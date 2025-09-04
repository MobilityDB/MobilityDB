DROP TABLE IF EXISTS allDistances;
DROP TABLE IF EXISTS cluster_groups;
DROP TABLE IF EXISTS group_overrides;
-- 1) Logical groups for clusterId (unchanged)
CREATE TEMP TABLE cluster_groups(group_name text, cluster_id int);
INSERT INTO cluster_groups VALUES
  ('G_23_22_25', 23), ('G_23_22_25', 22), ('G_23_22_25', 25),
  ('G_13', 13),
  ('G_18', 18);

-- 2) Per-TripId overrides (unchanged)
CREATE TEMP TABLE group_overrides(tripid int primary key, group_name text);
INSERT INTO group_overrides(tripid, group_name) VALUES
  (385, 'G_23'),
  (386, 'G_23');

-- 3) Build final table
CREATE TABLE allDistances AS
WITH

-- Intra-group pairs (for per-group min/max), using resolved group names
pairs_intragroup AS (
  SELECT
    COALESCE(o1.group_name, cg1.group_name) AS group_name,
    a.TripId      AS trip1_id,
    b.TripId      AS trip2_id,
    a.TripSegment AS s1,
    b.TripSegment AS s2
  FROM smallsample a
  LEFT JOIN group_overrides o1 ON o1.tripid = a.TripId
  JOIN cluster_groups cg1 ON cg1.cluster_id = a.clusterId

  JOIN smallsample b ON a.TripId < b.TripId
  LEFT JOIN group_overrides o2 ON o2.tripid = b.TripId
  JOIN cluster_groups cg2 ON cg2.cluster_id = b.clusterId

  WHERE COALESCE(o1.group_name, cg1.group_name)
      = COALESCE(o2.group_name, cg2.group_name)
),
-- All pairs (cross-group allowed), carry resolved group names + segments
pairs_all AS (
  SELECT
    COALESCE(o1.group_name, cg1.group_name) AS g1_name,
    COALESCE(o2.group_name, cg2.group_name) AS g2_name,
    a.TripId      AS trip1_id,
    b.TripId      AS trip2_id,
    a.TripSegment AS s1,
    b.TripSegment AS s2
  FROM smallsample a
  LEFT JOIN group_overrides o1 ON o1.tripid = a.TripId
  JOIN cluster_groups cg1 ON cg1.cluster_id = a.clusterId

  JOIN smallsample b ON a.TripId < b.TripId
  LEFT JOIN group_overrides o2 ON o2.tripid = b.TripId
  JOIN cluster_groups cg2 ON cg2.cluster_id = b.clusterId
),
-- Metric values on intra-group pairs (to compute min/max)
metrics_intragroup AS (
  SELECT
    group_name,
    frechetDistance(s1, s2)               AS frechet,
    dynTimeWarpDistance(s1, s2)           AS dtw,
    HausdorffDistance(s1, s2)             AS hausdorff,
    averageHausdorffDistance(s1, s2)      AS avg_hausdorff
  FROM pairs_intragroup
),
-- Per-group min/max from intra-group distances  (rename to mm_raw)
mm_raw AS (
  SELECT
    group_name,
    MIN(frechet)       AS frechet_min,       MAX(frechet)       AS frechet_max,
    MIN(dtw)           AS dtw_min,           MAX(dtw)           AS dtw_max,
    MIN(hausdorff)     AS hausdorff_min,     MAX(hausdorff)     AS hausdorff_max,
    MIN(avg_hausdorff) AS avg_hausdorff_min, MAX(avg_hausdorff) AS avg_hausdorff_max
  FROM metrics_intragroup
  GROUP BY group_name
),

-- Hardcode G_23 inside mm
mm AS (
  SELECT
    group_name,
    CASE WHEN group_name = 'G_23' THEN 0.0    ELSE frechet_min END AS frechet_min,
    CASE WHEN group_name = 'G_23' THEN 21700.0 ELSE frechet_max END AS frechet_max,
    CASE WHEN group_name = 'G_23' THEN 0.0    ELSE dtw_min     END AS dtw_min,
    CASE WHEN group_name = 'G_23' THEN 148112 ELSE dtw_max    END AS dtw_max,
    CASE WHEN group_name = 'G_23' THEN 0.0    ELSE hausdorff_min END AS hausdorff_min,
    CASE WHEN group_name = 'G_23' THEN 21700.0 ELSE hausdorff_max END AS hausdorff_max,
    CASE WHEN group_name = 'G_23' THEN 0.0    ELSE avg_hausdorff_min END AS avg_hausdorff_min,
    CASE WHEN group_name = 'G_23' THEN 11320.0 ELSE avg_hausdorff_max END AS avg_hausdorff_max
  FROM mm_raw
),
-- Metric values on all pairs; also precompute lengths for clean LCSS distance
metrics_all AS (
  SELECT
    g1_name, g2_name, trip1_id, trip2_id, s1, s2,
    numInstants(s1) AS n1,
    numInstants(s2) AS n2,
    frechetDistance(s1, s2)               AS frechet_raw,
    dynTimeWarpDistance(s1, s2)           AS dtw_raw,
    HausdorffDistance(s1, s2)             AS hausdorff_raw,
    averageHausdorffDistance(s1, s2)      AS avg_hausdorff_raw,
    lcssDistance(s1, s2, 2000.0)          AS lcss_len
  FROM pairs_all
)
SELECT
  m.g1_name  AS g1,
  m.g2_name  AS g2,
  m.trip1_id AS TripId1,
  m.trip2_id AS TripId2,

  -- Raw metrics (optional)
  m.frechet_raw,
  m.dtw_raw,
  m.hausdorff_raw,
  m.avg_hausdorff_raw,
  m.lcss_len,

  -- LCSS as distance in [0,1] (no min–max)
  1.0 - (m.lcss_len::float / NULLIF(GREATEST(m.n1, m.n2), 0)) AS lcss_dist,

  -- >>> NEW: normalize by the group of the LONGER trajectory (g1 if n1>=n2, else g2)
  -- We join the per-group min/max twice and select the appropriate one via CASE.
  CASE
    WHEN m.n1 >= m.n2 THEN  (m.frechet_raw - mm1.frechet_min)/ NULLIF(mm1.frechet_max - mm1.frechet_min,0)
    ELSE                    (m.frechet_raw - mm2.frechet_min)/ NULLIF(mm2.frechet_max - mm2.frechet_min,0)
  END AS frechet_norm_big,

  CASE
    WHEN m.n1 >= m.n2 THEN  (m.dtw_raw - mm1.dtw_min)/ NULLIF(mm1.dtw_max - mm1.dtw_min,0)
    ELSE                    (m.dtw_raw - mm2.dtw_min)/ NULLIF(mm2.dtw_max - mm2.dtw_min,0)
  END AS dtw_norm_big,

  CASE
    WHEN m.n1 >= m.n2 THEN  (m.hausdorff_raw - mm1.hausdorff_min)/ NULLIF(mm1.hausdorff_max - mm1.hausdorff_min,0)
    ELSE                    (m.hausdorff_raw - mm2.hausdorff_min)/ NULLIF(mm2.hausdorff_max - mm2.hausdorff_min,0)
  END AS hausdorff_norm_big,

  CASE
    WHEN m.n1 >= m.n2 THEN  (m.avg_hausdorff_raw - mm1.avg_hausdorff_min) / NULLIF(mm1.avg_hausdorff_max - mm1.avg_hausdorff_min, 0)
    ELSE                    (m.avg_hausdorff_raw - mm2.avg_hausdorff_min) / NULLIF(mm2.avg_hausdorff_max - mm2.avg_hausdorff_min, 0)
  END AS avg_hausdorff_norm_big

FROM metrics_all m
LEFT JOIN mm AS mm1 ON mm1.group_name = m.g1_name
LEFT JOIN mm AS mm2 ON mm2.group_name = m.g2_name
ORDER BY g1, TripId1, TripId2;
