-------------------------------------------------------------------------------
-- Send/receive functions
-------------------------------------------------------------------------------

\copy tbl_npoint TO 'tbl_npoint' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_npoint_tmp;
CREATE TABLE tbl_npoint_tmp AS TABLE tbl_npoint WITH NO DATA;
\copy tbl_npoint_tmp FROM 'tbl_npoint' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_npoint t1, tbl_npoint_tmp t2 WHERE t1.k = t2.k AND t1.np <> t2.np;
DROP TABLE tbl_npoint_tmp;

\copy tbl_nsegment TO 'tbl_nsegment' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_nsegment_tmp;
CREATE TABLE tbl_nsegment_tmp AS TABLE tbl_nsegment WITH NO DATA;
\copy tbl_nsegment_tmp FROM 'tbl_nsegment' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_nsegment t1, tbl_nsegment_tmp t2 WHERE t1.k = t2.k AND t1.ns <> t2.ns;
DROP TABLE tbl_nsegment_tmp;

-------------------------------------------------------------------------------
-- Accessing values
-------------------------------------------------------------------------------

SELECT MAX(route(np)) FROM tbl_npoint;
SELECT MAX(getPosition(np)) FROM tbl_npoint;

SELECT MAX(route(ns)) FROM tbl_nsegment;
SELECT MAX(startPosition(ns)) FROM tbl_nsegment;
SELECT MAX(endPosition(ns)) FROM tbl_nsegment;

-------------------------------------------------------------------------------
-- Cast functions between network and space
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_npoint WHERE np::geometry IS NOT NULL;
SELECT COUNT(*) FROM tbl_nsegment WHERE ns::geometry IS NOT NULL;

-- The cast to a geometry interpolates the point along its route and the cast
-- back locates it, so the route comes back and the position comes back within
-- the rounding of a parameter along a segment
SELECT COUNT(*) FROM tbl_npoint WHERE route(np) = route((np::geometry)::npoint) AND
  abs(getPosition(np) - getPosition((np::geometry)::npoint)) <= 1e-12;
SELECT COUNT(*) FROM tbl_nsegment WHERE route(ns) = route((ns::geometry)::nsegment) AND
  abs(startPosition(ns) - startPosition((ns::geometry)::nsegment)) <= 1e-12 AND
  abs(endPosition(ns) - endPosition((ns::geometry)::nsegment)) <= 1e-12;

SELECT COUNT(*) FROM tbl_geom_point WHERE CASE WHEN NOT ST_IsEmpty(g) THEN ST_SetSRID(g, 5676)::npoint IS NOT NULL END;
SELECT COUNT(*) FROM tbl_geom_linestring WHERE CASE WHEN NOT ST_IsEmpty(g) THEN ST_SetSRID(g, 5676)::nsegment IS NOT NULL END;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_npoint t1, tbl_npoint t2 WHERE t1.np = t2.np;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_npoint t2 WHERE t1.np != t2.np;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_npoint t2 WHERE t1.np < t2.np;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_npoint t2 WHERE t1.np <= t2.np;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_npoint t2 WHERE t1.np > t2.np;
SELECT COUNT(*) FROM tbl_npoint t1, tbl_npoint t2 WHERE t1.np >= t2.np;

SELECT COUNT(*) FROM tbl_nsegment t1, tbl_nsegment t2 WHERE t1.ns = t2.ns;
SELECT COUNT(*) FROM tbl_nsegment t1, tbl_nsegment t2 WHERE t1.ns != t2.ns;
SELECT COUNT(*) FROM tbl_nsegment t1, tbl_nsegment t2 WHERE t1.ns < t2.ns;
SELECT COUNT(*) FROM tbl_nsegment t1, tbl_nsegment t2 WHERE t1.ns <= t2.ns;
SELECT COUNT(*) FROM tbl_nsegment t1, tbl_nsegment t2 WHERE t1.ns > t2.ns;
SELECT COUNT(*) FROM tbl_nsegment t1, tbl_nsegment t2 WHERE t1.ns >= t2.ns;

-------------------------------------------------------------------------------/
