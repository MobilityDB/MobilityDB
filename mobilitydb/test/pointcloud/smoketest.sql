---------------------------------------------------------------------
-- pgpointcloud end-to-end smoke test (self-contained, runnable via psql).
--
-- Requirements:
--   * PostgreSQL with postgis, pointcloud, and MobilityDB
--     (POINTCLOUD=ON build) installed.
--   * At least one row in pointcloud_formats (pgpointcloud's default
--     pcid=1 is enough — the smoke test uses PC_MakePoint with it).
--
-- Exercises the full pgpointcloud stack:
--   pcpoint, pcpatch, pcpointset, pcpatchset, tpcbox,
--   tpcpoint, tpcpatch, and the projection + cast machinery.
---------------------------------------------------------------------

\echo '=== pgpointcloud smoke: Base types (pcpoint/pcpatch accessors) ==='

SELECT pcid(PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0])) AS base_pcid_expect_1;

SELECT getX(PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0])) AS x_expect_10,
       getY(PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0])) AS y_expect_20,
       getZ(PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0])) AS z_expect_30,
       getDim(PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0]), 'X') AS dim_x_expect_10;

SELECT getZ(PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0])) IS NOT NULL
       AS getz_returns_value_when_z_present;

\echo '=== pgpointcloud smoke: Set types (pcpointset / pcpatchset dedup) ==='

SELECT numValues(set(ARRAY[
         PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0]),
         PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0]),
         PC_MakePoint(1, ARRAY[11.0, 21.0, 31.0])
       ])) AS pcpointset_distinct_expect_2;

SELECT numValues(set(ARRAY[
         PC_Patch(PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0])),
         PC_Patch(PC_MakePoint(1, ARRAY[11.0, 21.0, 31.0]))
       ])) AS pcpatchset_distinct_expect_2;

\echo '=== pgpointcloud smoke: TPCBox bounding box ==='

SELECT xmin(tpcbox(10, 20, 30, 40, 1, 0))         AS tpcbox_xmin_expect_10,
       xmax(tpcbox(10, 20, 30, 40, 1, 0))         AS tpcbox_xmax_expect_30,
       pcid(tpcbox(10, 20, 30, 40, 7, 0))         AS tpcbox_pcid_expect_7,
       hasZ(tpcbox_z(1, 2, 3, 4, 5, 6, 1, 0))     AS tpcbox_hasz_expect_true;

SELECT xmin(tpcbox(PC_Patch(PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0]))))
       AS patch_to_tpcbox_xmin_expect_10;

SELECT xmin(tpcbox(PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0])))
       AS point_to_tpcbox_xmin_expect_10,
       hasZ(tpcbox(PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0])))
       AS point_to_tpcbox_hasz_expect_true;

\echo '=== pgpointcloud smoke: TPCBox set operations ==='

SELECT xmin(tpcbox(0, 0, 5, 5, 1) + tpcbox(3, 3, 10, 10, 1)) AS union_xmin_0,
       xmax(tpcbox(0, 0, 5, 5, 1) + tpcbox(3, 3, 10, 10, 1)) AS union_xmax_10,
       xmin(tpcbox(0, 0, 5, 5, 1) * tpcbox(3, 3, 10, 10, 1)) AS inter_xmin_3;

SELECT tpcbox(0, 0, 10, 10, 1) @> tpcbox(2, 2, 8, 8, 1)   AS contains_true,
       tpcbox(0, 0, 5, 5, 1)   && tpcbox(3, 3, 10, 10, 1) AS overlaps_true;

\echo '=== pgpointcloud smoke: tpcpoint lifted temporal type ==='

SELECT numInstants(tpcpoint(
         PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0]),
         '2024-01-01 12:00:00+00'::timestamptz
       )) AS instants_expect_1;

SELECT pcid(tpcpoint(
         PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0]),
         '2024-01-01'::timestamptz
       )) AS pcid_expect_1;

-- Discrete sequence construction
SELECT numInstants(
         tpcpointSeq(ARRAY[
           tpcpoint(PC_MakePoint(1, ARRAY[1.0, 2.0, 3.0]), '2024-01-01'::timestamptz),
           tpcpoint(PC_MakePoint(1, ARRAY[4.0, 5.0, 6.0]), '2024-01-02'::timestamptz),
           tpcpoint(PC_MakePoint(1, ARRAY[7.0, 8.0, 9.0]), '2024-01-03'::timestamptz)
         ], 'step')
       ) AS seq_instants_expect_3;

\echo '=== pgpointcloud smoke: tpcpatch lifted temporal type ==='

SELECT numInstants(tpcpatch(
         PC_Patch(PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0])),
         '2024-02-01 12:00:00+00'::timestamptz
       )) AS tpcpatch_instants_expect_1;

SELECT startNumPoints(tpcpatch(
         PC_Patch(PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0])),
         '2024-02-01'::timestamptz
       )) AS tpcpatch_npoints_expect_1;

\echo '=== pgpointcloud smoke: tpcpoint per-dimension projections ==='

SELECT startValue(getX(tpcpoint(
         PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0]),
         '2024-01-01'::timestamptz
       ))) AS project_x_expect_10,
       startValue(getY(tpcpoint(
         PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0]),
         '2024-01-01'::timestamptz
       ))) AS project_y_expect_20,
       startValue(getZ(tpcpoint(
         PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0]),
         '2024-01-01'::timestamptz
       ))) AS project_z_expect_30;

\echo '=== pgpointcloud smoke: tpcpoint → tgeompoint XY projection cast ==='

SELECT ST_AsText(startValue(
         tpcpoint(PC_MakePoint(1, ARRAY[10.0, 20.0, 30.0]),
                  '2024-01-01'::timestamptz)::tgeompoint
       )) AS cast_result_expect_POINT_Z_10_20_30;

\echo '=== pgpointcloud smoke: DONE ==='
