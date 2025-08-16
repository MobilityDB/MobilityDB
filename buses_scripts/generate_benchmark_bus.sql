-- ============================================================
-- STEP 1 — Sample: two trips per route, plus a short numeric ID
-- ============================================================
-- Build a tiny Brussels sample with trip in meters (ETRS89 / UTM 31N = EPSG:25831)
DROP TABLE IF EXISTS smallsample_bxl;

CREATE TABLE smallsample_bxl AS
WITH routes_35 AS (
  SELECT route_id
  FROM trips_mdb
  GROUP BY route_id
  ORDER BY route_id            -- or ORDER BY random()
  LIMIT 40
),
ranked AS (
  SELECT
    t.route_id, t.trip_id, t.trip, t.traj,   -- trip is tgeompoint SRID 4326 in source
    row_number() OVER (PARTITION BY t.route_id ORDER BY t.trip_id) AS rn
  FROM trips_mdb t
  JOIN routes_35 r USING (route_id)
),
picked AS (
  SELECT route_id, trip_id, trip, traj
  FROM ranked
  WHERE rn <= 2
)
SELECT
  route_id,
  trip_id,
  -- convert temporal geometry to EPSG:25831 so all distances are in meters
  tgeompoint_transform(trip, 25831) AS trip,            -- <<< meters now
  row_number() OVER (ORDER BY route_id, trip_id) AS trip_no,
  traj AS traj_geom                                     -- QGIS display only (still 4326)
FROM picked;

-- Nice-to-haves
COMMENT ON TABLE  smallsample_bxl IS 'Brussels sample: trip in EPSG:25831 (meters), traj_geom is display-only (4326).';
COMMENT ON COLUMN smallsample_bxl.trip     IS 'tgeompoint SRID 25831 (meters)';
COMMENT ON COLUMN smallsample_bxl.traj_geom IS 'geometry(LineString, 4326) for QGIS display only; not used in analytics.';

CREATE INDEX IF NOT EXISTS smallsample_bxl_route_idx ON smallsample_bxl(route_id);
CREATE INDEX IF NOT EXISTS smallsample_bxl_trip_idx  ON smallsample_bxl(trip_id);
CREATE INDEX IF NOT EXISTS smallsample_bxl_traj_gix  ON smallsample_bxl USING GIST (traj_geom);


-- =========================
-- K-means trip clustering (meters; uses EPSG:25831 trips)
-- =========================
DROP TABLE IF EXISTS trip_clusters_bxl;

CREATE TABLE trip_clusters_bxl AS
WITH mids AS (
  SELECT
    sb.trip_id,
    -- trip is already 25831; trajectory() yields a 25831 LineString
    ST_LineInterpolatePoint(trajectory(sb.trip), 0.5) AS mid_m
  FROM smallsample_bxl sb
),
k AS (
  SELECT
    trip_id,
    ST_ClusterKMeans(mid_m, 5) OVER () AS kid   -- set K as needed
  FROM mids
)
SELECT
  trip_id,
  ('K' || (kid + 1)) AS group_name
FROM k;

CREATE INDEX ON trip_clusters_bxl(group_name);
CREATE INDEX ON trip_clusters_bxl(trip_id);

-- =========================
-- Stamp cluster label back on the sample table
-- =========================
ALTER TABLE smallsample_bxl
  ADD COLUMN IF NOT EXISTS cluster TEXT;

UPDATE smallsample_bxl sb
SET cluster = tc.group_name
FROM trip_clusters_bxl tc
WHERE tc.trip_id = sb.trip_id;

UPDATE smallsample_bxl
SET cluster = 'UNCL'
WHERE cluster IS NULL;

CREATE INDEX IF NOT EXISTS smallsample_bxl_cluster_idx ON smallsample_bxl(cluster);

-- =========================
-- Pairwise distances + per-cluster min–max normalization (all in meters)
-- NOTE: LCSS epsilon below (50.0) is now 50 meters since trip is 25831
-- =========================
DROP TABLE IF EXISTS alldistances_bxl;

CREATE TABLE alldistances_bxl AS
WITH
  p AS (
    SELECT
      tc1.group_name AS g1, tc2.group_name AS g2,
      a.trip_id      AS trip_id1, b.trip_id AS trip_id2,
      a.trip         AS t1,       b.trip    AS t2,
      numInstants(a.trip) AS n1,  numInstants(b.trip) AS n2,
      frechetDistance(a.trip, b.trip)          AS frechet_raw,
      dynTimeWarpDistance(a.trip, b.trip)      AS dtw_raw,
      HausdorffDistance(a.trip, b.trip)        AS hausdorff_raw,
      averageHausdorffDistance(a.trip, b.trip) AS avg_hausdorff_raw,
      lcssDistance(a.trip, b.trip, 50.0)       AS lcss_len   -- 50 meters
    FROM smallsample_bxl a
    JOIN trip_clusters_bxl tc1 USING (trip_id)
    JOIN smallsample_bxl b ON a.trip_id < b.trip_id
    JOIN trip_clusters_bxl tc2 ON tc2.trip_id = b.trip_id
  ),
  pp AS (
    SELECT
      p.*,
      (p.dtw_raw::double precision / NULLIF(GREATEST(p.n1,p.n2),0)) AS dtw_ps,
      1.0 - (p.lcss_len::double precision / NULLIF(GREATEST(p.n1,p.n2),0)) AS lcss_dist,
      CASE WHEN p.n1 >= p.n2 THEN p.g1 ELSE p.g2 END AS ref_g
    FROM p
  ),
  mm AS (
    SELECT
      g1 AS g,
      MIN(frechet_raw)       AS frechet_min,       MAX(frechet_raw)       AS frechet_max,
      MIN(dtw_ps)            AS dtw_min,           MAX(dtw_ps)            AS dtw_max,
      MIN(hausdorff_raw)     AS hausdorff_min,     MAX(hausdorff_raw)     AS hausdorff_max,
      MIN(avg_hausdorff_raw) AS avg_hausdorff_min, MAX(avg_hausdorff_raw) AS avg_hausdorff_max
    FROM pp
    WHERE g1 = g2
    GROUP BY g1
  )
SELECT
  -- identifiers
  pp.g1, pp.g2, pp.trip_id1, pp.trip_id2,

  -- raw + helpers
  pp.n1, pp.n2,
  pp.frechet_raw,
  pp.dtw_raw,
  pp.dtw_ps,
  pp.hausdorff_raw,
  pp.avg_hausdorff_raw,
  pp.lcss_len    AS lcss_raw,
  pp.lcss_dist   AS lcss_norm,

  -- normalized by longer trip’s cluster (ties → g1)
  (pp.frechet_raw       - mm.frechet_min)       / NULLIF(mm.frechet_max       - mm.frechet_min,0)       AS frechet_norm,
  (pp.dtw_ps            - mm.dtw_min)           / NULLIF(mm.dtw_max           - mm.dtw_min,0)           AS dtw_norm,
  (pp.hausdorff_raw     - mm.hausdorff_min)     / NULLIF(mm.hausdorff_max     - mm.hausdorff_min,0)     AS hausdorff_norm,
  (pp.avg_hausdorff_raw - mm.avg_hausdorff_min) / NULLIF(mm.avg_hausdorff_max - mm.avg_hausdorff_min,0) AS avg_hausdorff_norm
FROM pp
LEFT JOIN mm ON mm.g = pp.ref_g
ORDER BY pp.trip_id1, pp.trip_id2;
