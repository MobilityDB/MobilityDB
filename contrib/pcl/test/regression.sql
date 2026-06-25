/*
 * Regression suite for the mobilitydb_pcl bridge.
 *
 * Runs against any database that has mobilitydb + pointcloud +
 * mobilitydb_pcl loaded as extensions, e.g.:
 *
 *   psql -d <db> -c "CREATE EXTENSION mobilitydb_pcl"
 *   psql -d <db> -f contrib/pcl/test/regression.sql
 *
 * Each test prints PASS / FAIL and the script exits at the first FAIL.
 * Output is deterministic (no timestamps, no random seeds without a
 * controlled setseed).
 */

\set ON_ERROR_STOP on

-- Schemas: X/Y/Z, X/Y/Z+I, X/Y/Z+R+G+B.
INSERT INTO pointcloud_formats (pcid, srid, schema) VALUES
  (9001, 0,
   '<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1">
  <pc:dimension><pc:position>1</pc:position><pc:size>8</pc:size><pc:name>X</pc:name><pc:interpretation>double</pc:interpretation><pc:scale>1</pc:scale></pc:dimension>
  <pc:dimension><pc:position>2</pc:position><pc:size>8</pc:size><pc:name>Y</pc:name><pc:interpretation>double</pc:interpretation><pc:scale>1</pc:scale></pc:dimension>
  <pc:dimension><pc:position>3</pc:position><pc:size>8</pc:size><pc:name>Z</pc:name><pc:interpretation>double</pc:interpretation><pc:scale>1</pc:scale></pc:dimension>
  <pc:metadata><Metadata name="compression">none</Metadata></pc:metadata>
</pc:PointCloudSchema>'),
  (9002, 0,
   '<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1">
  <pc:dimension><pc:position>1</pc:position><pc:size>8</pc:size><pc:name>X</pc:name><pc:interpretation>double</pc:interpretation><pc:scale>1</pc:scale></pc:dimension>
  <pc:dimension><pc:position>2</pc:position><pc:size>8</pc:size><pc:name>Y</pc:name><pc:interpretation>double</pc:interpretation><pc:scale>1</pc:scale></pc:dimension>
  <pc:dimension><pc:position>3</pc:position><pc:size>8</pc:size><pc:name>Z</pc:name><pc:interpretation>double</pc:interpretation><pc:scale>1</pc:scale></pc:dimension>
  <pc:dimension><pc:position>4</pc:position><pc:size>2</pc:size><pc:name>Intensity</pc:name><pc:interpretation>uint16_t</pc:interpretation><pc:scale>1</pc:scale></pc:dimension>
  <pc:metadata><Metadata name="compression">none</Metadata></pc:metadata>
</pc:PointCloudSchema>'),
  (9003, 0,
   '<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1">
  <pc:dimension><pc:position>1</pc:position><pc:size>8</pc:size><pc:name>X</pc:name><pc:interpretation>double</pc:interpretation><pc:scale>1</pc:scale></pc:dimension>
  <pc:dimension><pc:position>2</pc:position><pc:size>8</pc:size><pc:name>Y</pc:name><pc:interpretation>double</pc:interpretation><pc:scale>1</pc:scale></pc:dimension>
  <pc:dimension><pc:position>3</pc:position><pc:size>8</pc:size><pc:name>Z</pc:name><pc:interpretation>double</pc:interpretation><pc:scale>1</pc:scale></pc:dimension>
  <pc:dimension><pc:position>4</pc:position><pc:size>1</pc:size><pc:name>Red</pc:name><pc:interpretation>uint8_t</pc:interpretation><pc:scale>1</pc:scale></pc:dimension>
  <pc:dimension><pc:position>5</pc:position><pc:size>1</pc:size><pc:name>Green</pc:name><pc:interpretation>uint8_t</pc:interpretation><pc:scale>1</pc:scale></pc:dimension>
  <pc:dimension><pc:position>6</pc:position><pc:size>1</pc:size><pc:name>Blue</pc:name><pc:interpretation>uint8_t</pc:interpretation><pc:scale>1</pc:scale></pc:dimension>
  <pc:metadata><Metadata name="compression">none</Metadata></pc:metadata>
</pc:PointCloudSchema>')
ON CONFLICT (pcid) DO UPDATE SET schema = EXCLUDED.schema;

------------------------------------------------------------------------
-- 1. PCD round-trip preserves dim values for all three schema flavours.
------------------------------------------------------------------------

-- XYZ
SELECT 'XYZ round-trip' AS test,
  CASE WHEN PC_AsText(pcd_to_pcpatch_wkb_hex(pcpatch_to_pcd(PC_Patch(ARRAY[
      PC_MakePoint(9001, ARRAY[1.0, 2.0, 3.0]),
      PC_MakePoint(9001, ARRAY[4.0, 5.0, 6.0])
    ])), 9001)::pcpatch)
       = '{"pcid":9001,"pts":[[1,2,3],[4,5,6]]}'
       THEN 'PASS' ELSE 'FAIL' END AS result;

-- XYZI
SELECT 'XYZI round-trip' AS test,
  CASE WHEN PC_AsText(pcd_to_pcpatch_wkb_hex(pcpatch_to_pcd(PC_Patch(ARRAY[
      PC_MakePoint(9002, ARRAY[1.0, 2.0, 3.0, 99]),
      PC_MakePoint(9002, ARRAY[4.0, 5.0, 6.0, 199])
    ])), 9002)::pcpatch)
       = '{"pcid":9002,"pts":[[1,2,3,99],[4,5,6,199]]}'
       THEN 'PASS' ELSE 'FAIL' END AS result;

-- XYZRGB
SELECT 'XYZRGB round-trip' AS test,
  CASE WHEN PC_AsText(pcd_to_pcpatch_wkb_hex(pcpatch_to_pcd(PC_Patch(ARRAY[
      PC_MakePoint(9003, ARRAY[1.0, 2.0, 3.0, 200, 100, 50]),
      PC_MakePoint(9003, ARRAY[4.0, 5.0, 6.0, 50, 100, 200])
    ])), 9003)::pcpatch)
       = '{"pcid":9003,"pts":[[1,2,3,200,100,50],[4,5,6,50,100,200]]}'
       THEN 'PASS' ELSE 'FAIL' END AS result;

------------------------------------------------------------------------
-- 2. VoxelGrid: a 6-point fixture clustered at (0, 5, 10) with a 1 m
--    leaf collapses to 3 centroids; a 1 mm leaf preserves all 6.
------------------------------------------------------------------------

SELECT 'VoxelGrid 1 m leaf collapses clusters' AS test,
  CASE WHEN PC_NumPoints(pcpatch_voxel_grid_wkb_hex(PC_Patch(ARRAY[
      PC_MakePoint(9001, ARRAY[0.0, 0.0, 0.0]),
      PC_MakePoint(9001, ARRAY[0.01, 0.01, 0.01]),
      PC_MakePoint(9001, ARRAY[0.02, 0.02, 0.02]),
      PC_MakePoint(9001, ARRAY[5.0, 5.0, 5.0]),
      PC_MakePoint(9001, ARRAY[5.05, 5.05, 5.05]),
      PC_MakePoint(9001, ARRAY[10.0, 10.0, 10.0])
    ]), 1.0::double precision)::pcpatch) = 3
       THEN 'PASS' ELSE 'FAIL' END AS result;

SELECT 'VoxelGrid 1 mm leaf preserves all points' AS test,
  CASE WHEN PC_NumPoints(pcpatch_voxel_grid_wkb_hex(PC_Patch(ARRAY[
      PC_MakePoint(9001, ARRAY[0.0, 0.0, 0.0]),
      PC_MakePoint(9001, ARRAY[0.01, 0.01, 0.01]),
      PC_MakePoint(9001, ARRAY[0.02, 0.02, 0.02]),
      PC_MakePoint(9001, ARRAY[5.0, 5.0, 5.0]),
      PC_MakePoint(9001, ARRAY[5.05, 5.05, 5.05]),
      PC_MakePoint(9001, ARRAY[10.0, 10.0, 10.0])
    ]), 0.001::double precision)::pcpatch) = 6
       THEN 'PASS' ELSE 'FAIL' END AS result;

------------------------------------------------------------------------
-- 3. SOR: 50 deterministic in-cube points + 1 obvious outlier; SOR
--    drops the outlier.
------------------------------------------------------------------------

SELECT setseed(0.5);  -- deterministic random()

WITH cluster AS (
  SELECT array_agg(PC_MakePoint(9001, ARRAY[
    random()::double precision,
    random()::double precision,
    random()::double precision])) AS pts
  FROM generate_series(1, 50)
)
SELECT 'SOR drops far outlier' AS test,
  CASE WHEN PC_NumPoints(pcpatch_sor_wkb_hex(
    PC_Patch(pts || PC_MakePoint(9001, ARRAY[1000.0, 1000.0, 1000.0])),
    20::integer, 1.0::double precision)::pcpatch) = 50
       THEN 'PASS' ELSE 'FAIL' END AS result
FROM cluster;

------------------------------------------------------------------------
-- 4. VoxelGrid preserves intensity / RGB (PCL averages within each
--    voxel). Two-point cluster with I=100, 200 → one centroid with
--    averaged I=150.
------------------------------------------------------------------------

SELECT 'VoxelGrid averages intensity within voxel' AS test,
  CASE WHEN PC_AsText(pcpatch_voxel_grid_wkb_hex(PC_Patch(ARRAY[
      PC_MakePoint(9002, ARRAY[0.0, 0.0, 0.0, 100]),
      PC_MakePoint(9002, ARRAY[0.01, 0.01, 0.01, 200]),
      PC_MakePoint(9002, ARRAY[5.0, 5.0, 5.0, 50]),
      PC_MakePoint(9002, ARRAY[5.05, 5.05, 5.05, 150]),
      PC_MakePoint(9002, ARRAY[10.0, 10.0, 10.0, 99])
    ]), 1.0::double precision)::pcpatch)
       = '{"pcid":9002,"pts":[[0.005,0.005,0.005,150],[5.025,5.025,5.025,100],[10,10,10,99]]}'
       THEN 'PASS' ELSE 'FAIL' END AS result;

SELECT 'VoxelGrid averages RGB within voxel' AS test,
  CASE WHEN PC_AsText(pcpatch_voxel_grid_wkb_hex(PC_Patch(ARRAY[
      PC_MakePoint(9003, ARRAY[0.0, 0.0, 0.0, 100, 100, 100]),
      PC_MakePoint(9003, ARRAY[0.01, 0.01, 0.01, 200, 200, 200]),
      PC_MakePoint(9003, ARRAY[5.0, 5.0, 5.0, 50, 50, 50]),
      PC_MakePoint(9003, ARRAY[5.05, 5.05, 5.05, 150, 150, 150]),
      PC_MakePoint(9003, ARRAY[10.0, 10.0, 10.0, 99, 99, 99])
    ]), 1.0::double precision)::pcpatch)
       = '{"pcid":9003,"pts":[[0.005,0.005,0.005,150,150,150],[5.025,5.025,5.025,100,100,100],[10,10,10,99,99,99]]}'
       THEN 'PASS' ELSE 'FAIL' END AS result;

------------------------------------------------------------------------
-- 5. SOR is a keep/drop filter on points: surviving points retain
--    their non-spatial dim values exactly.
------------------------------------------------------------------------

SELECT 'SOR preserves intensity on survivors' AS test,
  CASE WHEN PC_AsText(pcpatch_sor_wkb_hex(PC_Patch(ARRAY[
      PC_MakePoint(9002, ARRAY[0.0, 0.0, 0.0, 7]),
      PC_MakePoint(9002, ARRAY[0.01, 0.01, 0.01, 8]),
      PC_MakePoint(9002, ARRAY[0.02, 0.02, 0.02, 9]),
      PC_MakePoint(9002, ARRAY[0.03, 0.03, 0.03, 10]),
      PC_MakePoint(9002, ARRAY[0.04, 0.04, 0.04, 11]),
      PC_MakePoint(9002, ARRAY[0.05, 0.05, 0.05, 12])
    ]), 3::integer, 100.0::double precision)::pcpatch)
    LIKE '%[0,0,0,7]%[0.05,0.05,0.05,12]%'
       THEN 'PASS' ELSE 'FAIL' END AS result;

------------------------------------------------------------------------
-- 6. ICP recovers a known translation between source and target.
--    Target = source translated by (+0.5, +0.5, +0.5); ICP must return
--    the same translation, an identity quaternion, and near-zero
--    fitness.
------------------------------------------------------------------------

WITH icp AS (
  SELECT pcpatch_icp(
    PC_Patch(ARRAY[
      PC_MakePoint(9001, ARRAY[0.0, 0.0, 0.0]),
      PC_MakePoint(9001, ARRAY[1.0, 0.0, 0.0]),
      PC_MakePoint(9001, ARRAY[0.0, 1.0, 0.0]),
      PC_MakePoint(9001, ARRAY[0.0, 0.0, 1.0]),
      PC_MakePoint(9001, ARRAY[1.0, 1.0, 1.0])]),
    PC_Patch(ARRAY[
      PC_MakePoint(9001, ARRAY[0.5, 0.5, 0.5]),
      PC_MakePoint(9001, ARRAY[1.5, 0.5, 0.5]),
      PC_MakePoint(9001, ARRAY[0.5, 1.5, 0.5]),
      PC_MakePoint(9001, ARRAY[0.5, 0.5, 1.5]),
      PC_MakePoint(9001, ARRAY[1.5, 1.5, 1.5])]),
    50, 5.0::double precision) AS r
)
SELECT 'ICP recovers known translation' AS test,
  CASE WHEN
    abs(r[1] - 0.5) < 1e-3 AND
    abs(r[2] - 0.5) < 1e-3 AND
    abs(r[3] - 0.5) < 1e-3 AND
    abs(r[4] - 1.0) < 1e-6 AND   -- qw = 1 (identity)
    abs(r[8])       < 1e-6        -- fitness near zero
       THEN 'PASS' ELSE 'FAIL' END AS result
FROM icp;

------------------------------------------------------------------------
-- 7. GICP recovers the same known translation. Needs ~200 points for
--    covariance estimation to be well-posed.
------------------------------------------------------------------------

DO $$BEGIN PERFORM setseed(0.5); END$$;
WITH src AS (
  SELECT array_agg(PC_MakePoint(9001, ARRAY[
    random()::double precision * 10,
    random()::double precision * 10,
    random()::double precision * 10])) AS pts
  FROM generate_series(1, 200)
),
tgt AS (
  SELECT array_agg(PC_MakePoint(9001, ARRAY[
    PC_Get(p, 'X') + 0.5,
    PC_Get(p, 'Y') + 0.5,
    PC_Get(p, 'Z') + 0.5])) AS pts
  FROM (SELECT unnest(pts) AS p FROM src) sub
)
SELECT 'GICP recovers known translation' AS test,
  CASE WHEN
    abs(r[1] - 0.5) < 1e-3 AND
    abs(r[2] - 0.5) < 1e-3 AND
    abs(r[3] - 0.5) < 1e-3 AND
    abs(r[8])       < 1e-3
       THEN 'PASS' ELSE 'FAIL' END AS result
FROM src, tgt,
     LATERAL (SELECT pcpatch_gicp(PC_Patch(src.pts),
                                   PC_Patch(tgt.pts),
                                   50, 5.0::double precision) AS r) x;

------------------------------------------------------------------------
-- 12. pcpatch_normals: planar grid yields unit normal (0, 0, ±1).
------------------------------------------------------------------------

WITH plane AS (
  SELECT pcpatch_normals(
    PC_Patch(ARRAY[
      PC_MakePoint(9001, ARRAY[0.0, 0.0, 0.0]), PC_MakePoint(9001, ARRAY[1.0, 0.0, 0.0]),
      PC_MakePoint(9001, ARRAY[2.0, 0.0, 0.0]), PC_MakePoint(9001, ARRAY[0.0, 1.0, 0.0]),
      PC_MakePoint(9001, ARRAY[1.0, 1.0, 0.0]), PC_MakePoint(9001, ARRAY[2.0, 1.0, 0.0]),
      PC_MakePoint(9001, ARRAY[0.0, 2.0, 0.0]), PC_MakePoint(9001, ARRAY[1.0, 2.0, 0.0]),
      PC_MakePoint(9001, ARRAY[2.0, 2.0, 0.0])
    ]), 5
  ) AS n
)
SELECT 'pcpatch_normals z=0 plane yields normal (0,0,1)' AS test,
  CASE WHEN
    array_length(n, 1) = 36 AND  -- 4 * 9 points
    abs(n[1]) < 1e-3 AND abs(n[2]) < 1e-3 AND abs(abs(n[3]) - 1.0) < 1e-3 AND
    abs(n[4]) < 1e-3
       THEN 'PASS' ELSE 'FAIL' END AS result
FROM plane;

------------------------------------------------------------------------
-- Cleanup
------------------------------------------------------------------------

DELETE FROM pointcloud_formats WHERE pcid IN (9001, 9002, 9003);
