-----------------------------------------------------------------------------
-- Bulk spatial-relationship tests over temporal S2 cells (ts2cell/ts2cell),
-- answered on the sphere over the temporal geography of the cell boundaries.
-- The geography-operand overloads are covered by the scalar test
-- 612_ts2cell_spatialrels.
-----------------------------------------------------------------------------

-----------------------------------------------------------------------------
-- Disjoint
-----------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE eDisjoint(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE aDisjoint(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE tDisjoint(t1.temp, t2.temp) IS NOT NULL;

-----------------------------------------------------------------------------
-- Intersects
-----------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE eIntersects(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE aIntersects(t1.temp, t2.temp);
SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE tIntersects(t1.temp, t2.temp) IS NOT NULL;

-----------------------------------------------------------------------------
-- Dwithin, in metres
-----------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE eDwithin(t1.temp, t2.temp, 5000.0);
SELECT COUNT(*) FROM tbl_ts2cell t1, tbl_ts2cell t2 WHERE aDwithin(t1.temp, t2.temp, 5000.0);

-----------------------------------------------------------------------------
