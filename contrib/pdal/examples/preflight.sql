/*
 * preflight.sql — Sanity-check a database before running a real
 * `writers.tpcpatch` ingest. Catches the common first-flight failure
 * modes (missing extension, missing pcid, wrong column type, missing
 * id column for append/upsert) before PDAL streams a single point.
 *
 * Usage:
 *
 *   psql -d <yourdb> \
 *        -v pcid=500 \
 *        -v target_table=drone_traj \
 *        -v target_column=traj \
 *        -v id_column=id \
 *        -v want_pcl=1 \
 *        -f contrib/pdal/examples/preflight.sql
 *
 * Variables (all optional, with sensible defaults):
 *   pcid           — pcid the writer will use (default 500, the LiDAR template).
 *   target_table   — destination table (default 'drone_traj').
 *   target_column  — destination tpcpatch column (default 'traj').
 *   id_column      — pin to a row for mode=update/append/upsert ('' to skip).
 *   want_pcl       — set to 1 if you plan to use mobilitydb_pcl filters.
 *
 * Each check prints PASS on its own line. The script aborts on the
 * first FAIL via a RAISE EXCEPTION inside a DO block, so you see the
 * failing step's error message without scrolling.
 */

\set ON_ERROR_STOP on

\if :{?pcid}\else \set pcid 500 \endif
\if :{?target_table}\else \set target_table drone_traj \endif
\if :{?target_column}\else \set target_column traj \endif
\if :{?id_column}\else \set id_column '' \endif
\if :{?want_pcl}\else \set want_pcl 0 \endif

\echo
\echo '== preflight.sql =='
\echo '   pcid           =' :pcid
\echo '   target_table   =' :target_table
\echo '   target_column  =' :target_column
\echo '   id_column      =' :id_column
\echo '   want_pcl       =' :want_pcl
\echo

/* psql variable substitution does not reach inside $$-quoted DO bodies,
 * so we stash each parameter in a session GUC and read it via
 * current_setting() inside the assertions below. */
SELECT set_config('preflight.pcid',          :'pcid',          false),
       set_config('preflight.target_table',  :'target_table',  false),
       set_config('preflight.target_column', :'target_column', false),
       set_config('preflight.id_column',     :'id_column',     false),
       set_config('preflight.want_pcl',      :'want_pcl',      false)
\gset _preflight_

/* ---- 1. extensions ---- */
DO $$ BEGIN
  IF NOT EXISTS (SELECT 1 FROM pg_extension WHERE extname = 'postgis') THEN
    RAISE EXCEPTION 'postgis extension is not loaded. Run: CREATE EXTENSION postgis (or CREATE EXTENSION mobilitydb CASCADE).';
  END IF;
END $$;
\echo '  PASS — postgis present'

DO $$ BEGIN
  IF NOT EXISTS (SELECT 1 FROM pg_extension WHERE extname = 'pointcloud') THEN
    RAISE EXCEPTION 'pointcloud extension is not loaded. Run: CREATE EXTENSION pointcloud (or rebuild MobilityDB with -DPOINTCLOUD=ON and run CREATE EXTENSION mobilitydb CASCADE).';
  END IF;
END $$;
\echo '  PASS — pointcloud present'

DO $$ BEGIN
  IF NOT EXISTS (SELECT 1 FROM pg_extension WHERE extname = 'mobilitydb') THEN
    RAISE EXCEPTION 'mobilitydb extension is not loaded. Run: CREATE EXTENSION mobilitydb CASCADE.';
  END IF;
END $$;
\echo '  PASS — mobilitydb present'

/* ---- 2. mobilitydb_pcl (only if want_pcl=1) ---- */
DO $$ BEGIN
  IF current_setting('preflight.want_pcl')::int = 1
     AND NOT EXISTS (SELECT 1 FROM pg_extension WHERE extname = 'mobilitydb_pcl') THEN
    RAISE EXCEPTION 'mobilitydb_pcl extension is not loaded but want_pcl=1. Build the bridge under contrib/pcl/ (needs PCL >= 1.12) and run: CREATE EXTENSION mobilitydb_pcl.';
  END IF;
END $$;
\echo '  PASS — mobilitydb_pcl check (skipped when want_pcl=0)'

/* ---- 3. pcid present ---- */
DO $$
DECLARE pcid_val int := current_setting('preflight.pcid')::int;
BEGIN
  IF NOT EXISTS (SELECT 1 FROM pointcloud_formats WHERE pcid = pcid_val) THEN
    RAISE EXCEPTION 'pcid % is not registered in pointcloud_formats. Register it first — see contrib/pdal/examples/drone_lidar_ingest.sql or drone_photogrammetry_ingest.sql for templates.', pcid_val;
  END IF;
END $$;
\echo '  PASS — pcid' :pcid 'is registered'

\echo
\echo '  schema dim summary (compare with: pdal info /path/to/your.laz --schema):'
SELECT
  (xpath('//pc:dimension/pc:name/text()',
         schema::xml,
         ARRAY[ARRAY['pc','http://pointcloud.org/schemas/PC/1.1']]))::text
    AS dim_names,
  srid
FROM pointcloud_formats WHERE pcid = current_setting('preflight.pcid')::int;

/* ---- 4. target table + column exist with correct type ---- */
DO $$
DECLARE
  tbl  text := current_setting('preflight.target_table');
  col  text := current_setting('preflight.target_column');
  typname text;
BEGIN
  SELECT format_type(a.atttypid, a.atttypmod) INTO typname
  FROM pg_attribute a
  JOIN pg_class c ON c.oid = a.attrelid
  WHERE c.relname = tbl
    AND a.attname = col
    AND a.attnum > 0
    AND NOT a.attisdropped;
  IF typname IS NULL THEN
    RAISE EXCEPTION 'column %.% does not exist. Run: CREATE TABLE % (id serial primary key, % tpcpatch);', tbl, col, tbl, col;
  END IF;
  IF typname NOT LIKE 'tpcpatch%' THEN
    RAISE EXCEPTION 'column %.% is type %, expected tpcpatch', tbl, col, typname;
  END IF;
END $$;
\echo '  PASS — column' :target_table'.':target_column 'exists and is tpcpatch'

/* ---- 5. id column (only if specified) ---- */
DO $$
DECLARE
  tbl text := current_setting('preflight.target_table');
  idc text := current_setting('preflight.id_column');
  typname text;
BEGIN
  IF idc = '' THEN RETURN; END IF;
  SELECT format_type(a.atttypid, a.atttypmod) INTO typname
  FROM pg_attribute a
  JOIN pg_class c ON c.oid = a.attrelid
  WHERE c.relname = tbl
    AND a.attname = idc
    AND a.attnum > 0
    AND NOT a.attisdropped;
  IF typname IS NULL THEN
    RAISE EXCEPTION 'id column %.% does not exist (required for mode=update/append/upsert)', tbl, idc;
  END IF;
  IF typname NOT IN ('integer', 'bigint', 'smallint') THEN
    RAISE WARNING 'id column %.% is type % — works but integer family is conventional', tbl, idc, typname;
  END IF;
END $$;
\echo '  PASS — id column check (skipped when id_column is empty)'

/* ---- 6. recommended GiST/SP-GiST index (warn, do not fail) ---- */
DO $$
DECLARE
  tbl text := current_setting('preflight.target_table');
  col text := current_setting('preflight.target_column');
  n   int;
BEGIN
  SELECT count(*) INTO n
  FROM pg_index i
  JOIN pg_class c ON c.oid = i.indrelid
  JOIN pg_class ic ON ic.oid = i.indexrelid
  JOIN pg_am am ON am.oid = ic.relam
  JOIN pg_attribute a ON a.attrelid = i.indrelid AND a.attnum = ANY(i.indkey)
  WHERE c.relname = tbl
    AND a.attname = col
    AND am.amname IN ('gist', 'spgist');
  IF n = 0 THEN
    RAISE WARNING 'no GiST/SP-GiST index on %.% — spatial-temporal predicates will be slow on large tables. Consider: CREATE INDEX %_%_gist ON % USING gist (%);',
      tbl, col, tbl, col, tbl, col;
  END IF;
END $$;
\echo '  PASS — index check (warning printed above if absent)'

\echo
\echo '== all preflight checks passed =='
\echo
\echo '  Next step: edit contrib/pdal/examples/drone_lidar_ingest.json (or'
\echo '  drone_photogrammetry_ingest.json) to point at your input file +'
\echo '  connection string, then:'
\echo
\echo '      PDAL_DRIVER_PATH=$(pwd)/contrib/pdal/build/reader:$(pwd)/contrib/pdal/build/writer \\'
\echo '        pdal pipeline contrib/pdal/examples/drone_lidar_ingest.json'
\echo
