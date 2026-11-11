-------------------------------------------------------------------------------
-- Discrete Frechet distance and path
-------------------------------------------------------------------------------

SELECT round(MAX(frechetDistance(t1.temp, t2.temp)), 6) FROM tbl_tint t1, tbl_tint t2 WHERE t1.k < t2.k;
SELECT round(MAX(frechetDistance(t1.temp, t2.temp)), 6) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.k < t2.k;

WITH temp AS (
  SELECT frechetDistancePath(t1.temp, t2.temp) FROM tbl_tint t1, tbl_tint t2 WHERE t1.k < t2.k )
SELECT COUNT(*) FROM temp;
WITH temp AS (
  SELECT frechetDistancePath(t1.temp, t2.temp) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.k < t2.k )
SELECT COUNT(*) FROM temp;

-------------------------------------------------------------------------------
-- Dynamic Time Warp (DTW) distance and path
-------------------------------------------------------------------------------

SELECT round(MAX(dynTimeWarpDistance(t1.temp, t2.temp)), 6) FROM tbl_tint t1, tbl_tint t2 WHERE t1.k < t2.k;
SELECT round(MAX(dynTimeWarpDistance(t1.temp, t2.temp)), 6) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.k < t2.k;

WITH temp AS (
  SELECT dynTimeWarpPath(t1.temp, t2.temp) FROM tbl_tint t1, tbl_tint t2 WHERE t1.k < t2.k )
SELECT COUNT(*) FROM temp;
WITH temp AS (
  SELECT dynTimeWarpPath(t1.temp, t2.temp) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.k < t2.k )
SELECT COUNT(*) FROM temp;

-------------------------------------------------------------------------------
-- Hausdorff distance
-------------------------------------------------------------------------------

SELECT round(MAX(hausdorffDistance(t1.temp, t2.temp)), 6) FROM tbl_tint t1, tbl_tint t2 WHERE t1.k < t2.k;
SELECT round(MAX(hausdorffDistance(t1.temp, t2.temp)), 6) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.k < t2.k;

-------------------------------------------------------------------------------
