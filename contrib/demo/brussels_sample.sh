#!/usr/bin/env bash
# brussels_sample.sh — End-to-end demo against a real-world LAS tile.
#
# Pulls one ~250 MB Brussels Region open-data LAS tile (Grand-Place /
# centre historique, 2021-09-10, ~67 pts/m², VQ1560II sensor; CC-BY 4.0,
# cite Paradigm / datastore.brussels), ingests it through PDAL into
# pgPointCloud, lifts it into a MobilityDB tpcpatch via the appendInstant
# aggregate, and runs four demo queries that exercise the pgPointCloud
# temporal surface end-to-end on real bytes.
#
# Prerequisites:
#   - PostgreSQL with mobilitydb (POINTCLOUD=ON build) + pointcloud +
#     pointcloud_postgis + postgis already loadable. The script CREATEs
#     them via CASCADE; it does not install binaries.
#   - PDAL >= 2.6 on PATH.
#   - curl, unzip.
#   - ~1 GB free on /tmp (zip ~70 MB, las ~250 MB, plus working space).
#
# Usage:
#   ./brussels_sample.sh [DBNAME=mobility]
#
# Exit codes: 0 on full PASS, non-zero on the first FAIL.

set -e -u -o pipefail

DB="${1:-mobility}"
PG_HOST="${PGHOST:-/tmp}"
WORK="${WORK:-$(mktemp -d /tmp/mdb_brussels_XXXXXX)}"
TILE_URL="https://urbisdownload.datastore.brussels/UrbIS/Vector/M8/PointCloud2021/LAS/PointCloud_31370_LAS_148173_20210910.zip"
TILE_NAME="PointCloud_31370_LAS_148173_20210910"
DECIMATE_KEEP_EVERY="${DECIMATE_KEEP_EVERY:-100}"  # 1 in N points kept; default 1% of ~17M points -> 170k
PCID_3D_4326="${PCID_3D_4326:-510}"                # WGS84 lon/lat/Z + intensity + classification + GpsTime

trap 'rm -rf "$WORK"; psql -h "$PG_HOST" -d "$DB" -X -q -c "
  DROP TABLE IF EXISTS brussels_pcp_raw, brussels_pcp_patches, brussels_traj;
  DELETE FROM pointcloud_formats WHERE pcid = '"$PCID_3D_4326"';
" >/dev/null 2>&1 || true' EXIT

step()  { echo; echo "=== $* ==="; }
pass()  { echo "  PASS"; }
fail()  { echo "  FAIL — $*"; exit 1; }

# -----------------------------------------------------------------------------
# 1. Tile fetch
# -----------------------------------------------------------------------------
step "fetch real Brussels LAS tile (Grand-Place 2021-09-10)"
if [ ! -f "$WORK/$TILE_NAME.las" ]; then
  curl -L --max-time 600 -o "$WORK/tile.zip" "$TILE_URL"
  unzip -o -q "$WORK/tile.zip" -d "$WORK"
  # Some tiles ship with sub-paths inside the zip; locate the .las.
  found="$(find "$WORK" -maxdepth 3 -type f -name '*.las' | head -1)"
  if [ -z "$found" ]; then fail "no .las extracted from $WORK/tile.zip"; fi
  if [ "$found" != "$WORK/$TILE_NAME.las" ]; then mv "$found" "$WORK/$TILE_NAME.las"; fi
fi
n_pts="$(pdal info --metadata "$WORK/$TILE_NAME.las" 2>/dev/null \
  | grep -oE '"count": [0-9]+' | head -1 | awk '{print $2}')"
if [ -z "${n_pts:-}" ]; then fail "pdal info returned no point count"; fi
echo "  source LAS has $n_pts points; will keep 1 in $DECIMATE_KEEP_EVERY for the demo"
pass

# -----------------------------------------------------------------------------
# 2. Schema setup
# -----------------------------------------------------------------------------
step "ensure extensions + register pcid $PCID_3D_4326"
psql -h "$PG_HOST" -d "$DB" -X -q >/dev/null <<EOF
  CREATE EXTENSION IF NOT EXISTS mobilitydb CASCADE;
  CREATE EXTENSION IF NOT EXISTS pointcloud_postgis;
  INSERT INTO pointcloud_formats (pcid, srid, schema) VALUES (
    $PCID_3D_4326, 4326,
    '<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1">
  <pc:dimension>
    <pc:position>1</pc:position><pc:size>4</pc:size><pc:name>X</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.0000001</pc:scale><pc:offset>4.35</pc:offset>
  </pc:dimension>
  <pc:dimension>
    <pc:position>2</pc:position><pc:size>4</pc:size><pc:name>Y</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.0000001</pc:scale><pc:offset>50.85</pc:offset>
  </pc:dimension>
  <pc:dimension>
    <pc:position>3</pc:position><pc:size>4</pc:size><pc:name>Z</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.001</pc:scale><pc:offset>0</pc:offset>
  </pc:dimension>
  <pc:dimension>
    <pc:position>4</pc:position><pc:size>2</pc:size><pc:name>Intensity</pc:name>
    <pc:interpretation>uint16_t</pc:interpretation><pc:scale>1</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>5</pc:position><pc:size>1</pc:size><pc:name>Classification</pc:name>
    <pc:interpretation>uint8_t</pc:interpretation><pc:scale>1</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>6</pc:position><pc:size>8</pc:size><pc:name>GpsTime</pc:name>
    <pc:interpretation>double</pc:interpretation><pc:scale>1</pc:scale>
  </pc:dimension>
  <pc:metadata><Metadata name="compression">none</Metadata></pc:metadata>
</pc:PointCloudSchema>')
  ON CONFLICT (pcid) DO UPDATE SET schema = EXCLUDED.schema;
  DROP TABLE IF EXISTS brussels_pcp_raw, brussels_pcp_patches, brussels_traj;
  CREATE TABLE brussels_pcp_raw (id serial PRIMARY KEY, pcp pcpatch($PCID_3D_4326));
EOF
pass

# -----------------------------------------------------------------------------
# 3. PDAL ingest: reproject 31370 -> 4326, decimate, chip into pcpatch rows
# -----------------------------------------------------------------------------
step "PDAL ingest: 31370 -> 4326, decimate 1/$DECIMATE_KEEP_EVERY, chip via pgpointcloud"
cat > "$WORK/ingest.json" <<EOF
{
  "pipeline": [
    { "type": "readers.las",
      "filename": "$WORK/$TILE_NAME.las",
      "spatialreference": "EPSG:31370" },
    { "type": "filters.reprojection",
      "in_srs": "EPSG:31370",
      "out_srs": "EPSG:4326" },
    { "type": "filters.decimation",
      "step": $DECIMATE_KEEP_EVERY },
    { "type": "writers.pgpointcloud",
      "connection": "host=$PG_HOST dbname=$DB",
      "table": "brussels_pcp_raw",
      "column": "pcp",
      "pcid": $PCID_3D_4326,
      "compression": "none",
      "overwrite": "false" }
  ]
}
EOF
pdal pipeline "$WORK/ingest.json" >"$WORK/pdal.log" 2>&1
n_patches="$(psql -h "$PG_HOST" -d "$DB" -X -tAc \
  "SELECT count(*) FROM brussels_pcp_raw;")"
if [ "$n_patches" -lt 1 ]; then fail "PDAL ingest produced 0 patches (see $WORK/pdal.log)"; fi
echo "  ingested into $n_patches pcpatch rows"
pass

# -----------------------------------------------------------------------------
# 4. Lift to tpcpoint trajectory via per-point GpsTime
# -----------------------------------------------------------------------------
step "lift to tpcpoint via PC_Explode + tpcpoint() + appendInstant"
psql -h "$PG_HOST" -d "$DB" -X -q >/dev/null <<EOF
  -- Per-point timestamps: GpsTime is "Adjusted GPS Standard Time" in
  -- LAS-1.4 ASGT mode, i.e. seconds since 1980-01-06 GPS epoch minus 1e9.
  -- Convert to absolute timestamptz: + 1e9 + 315964782 - 18 leap seconds.
  -- (See contrib/pdal/writer/TpcpatchWriter.cpp::toUnixMicroseconds.)
  CREATE TABLE brussels_pcp_patches AS
  SELECT row_number() OVER () AS id,
         (PC_Explode(pcp)).*  AS pt
  FROM   brussels_pcp_raw;

  -- One trajectory per source patch; keep ordering deterministic.
  CREATE TABLE brussels_traj AS
  SELECT 1 AS id,
         appendInstant(
           tpcpoint(pt,
             to_timestamp((PC_Get(pt, 'GpsTime') + 1e9 + 315964782 - 18))::timestamptz)
           ORDER BY PC_Get(pt, 'GpsTime')
         ) AS traj
  FROM   brussels_pcp_patches;
EOF
n_inst="$(psql -h "$PG_HOST" -d "$DB" -X -tAc \
  "SELECT numInstants(traj) FROM brussels_traj;")"
if [ "$n_inst" -lt 1000 ]; then fail "expected >1000 instants in tpcpoint, got $n_inst"; fi
echo "  tpcpoint trajectory has $n_inst instants"
pass

# -----------------------------------------------------------------------------
# 5. Demo queries
# -----------------------------------------------------------------------------
step "demo: getX/getY/getZ as tfloat (extract per-instant coordinate)"
psql -h "$PG_HOST" -d "$DB" -X -q -c "
  SELECT 'tfloat instants: x=' || numInstants(getX(traj))
       ||              ', y=' || numInstants(getY(traj))
       ||              ', z=' || numInstants(getZ(traj))
  FROM   brussels_traj;
"
pass

step "demo: cast tpcpoint -> tgeompoint and check trajectory is non-empty"
n_geom_inst="$(psql -h "$PG_HOST" -d "$DB" -X -tAc "
  SELECT numInstants(traj::tgeompoint)
  FROM   brussels_traj;
")"
if [ "$n_geom_inst" != "$n_inst" ]; then
  fail "tpcpoint->tgeompoint cast lost instants ($n_inst -> $n_geom_inst)"
fi
echo "  tgeompoint cast preserves all $n_geom_inst instants"
pass

step "demo: atGeometry restriction to a 200x200 m sub-window over Grand-Place"
# Grand-Place de Bruxelles ~ 4.3525 E, 50.8467 N (WGS84). 200 m at this
# latitude is ~0.0028° lon, ~0.0018° lat.
n_clip="$(psql -h "$PG_HOST" -d "$DB" -X -tAc "
  SELECT numInstants(atGeometry(traj::tgeompoint,
    ST_GeomFromText(
      'POLYGON((4.3511 50.8458, 4.3539 50.8458, 4.3539 50.8476,
                4.3511 50.8476, 4.3511 50.8458))', 4326)))
  FROM   brussels_traj;
")"
echo "  $n_clip of $n_inst instants fall inside the Grand-Place sub-window"
pass

step "demo: spatial bbox of the trajectory"
psql -h "$PG_HOST" -d "$DB" -X -q -c "
  SELECT extent(traj) FROM brussels_traj;
"
pass

echo
echo "== brussels_sample.sh: all PASS =="
echo "   working directory:  $WORK"
echo "   tpcpoint instants:  $n_inst"
echo "   tile points kept:   1 in $DECIMATE_KEEP_EVERY"
echo
echo "Tip: export DECIMATE_KEEP_EVERY=10 to ingest 10x more points (~1.7 M)"
echo "     and observe scaling on a real corpus."
