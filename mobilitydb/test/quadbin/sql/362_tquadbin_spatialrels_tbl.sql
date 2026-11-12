-----------------------------------------------------------------------------
-- Bulk spatial-relationship tests over temporal QUADBIN cells (tquadbin/tquadbin).
-- The geometry-operand overloads are covered by the scalar test
-- 362_tquadbin_spatialrels.
-----------------------------------------------------------------------------

-----------------------------------------------------------------------------
-- Contains
-----------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE eContains(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE aContains(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE tContains(t1.temp, t2.temp) IS NOT NULL;

-----------------------------------------------------------------------------
-- Covers
-----------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE eCovers(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE aCovers(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE tCovers(t1.temp, t2.temp) IS NOT NULL;

-----------------------------------------------------------------------------
-- Disjoint
-----------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE eDisjoint(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE aDisjoint(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE tDisjoint(t1.temp, t2.temp) IS NOT NULL;

-----------------------------------------------------------------------------
-- Intersects
-----------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE eIntersects(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE aIntersects(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE tIntersects(t1.temp, t2.temp) IS NOT NULL;

-----------------------------------------------------------------------------
-- Touches
-----------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE eTouches(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE aTouches(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE tTouches(t1.temp, t2.temp) IS NOT NULL;

-----------------------------------------------------------------------------
-- Dwithin
-----------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE eDwithin(t1.temp, t2.temp, 0.05);
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE aDwithin(t1.temp, t2.temp, 0.05);
SELECT COUNT(*) FROM tbl_tquadbin t1, tbl_tquadbin t2 WHERE tDwithin(t1.temp, t2.temp, 0.05) IS NOT NULL;

-----------------------------------------------------------------------------