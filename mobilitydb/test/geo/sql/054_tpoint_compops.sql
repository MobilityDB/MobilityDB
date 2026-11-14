-------------------------------------------------------------------------------
-- Ever/always equal 
-------------------------------------------------------------------------------

SELECT geometry 'Point(1 1)' ?= tgeompoint 'Point(1 1)@2001-01-01';
SELECT geometry 'Point(1 1)' ?<> tgeompoint 'Point(1 1)@2001-01-01';
SELECT geometry 'Point(1 1)' %= tgeompoint 'Point(1 1)@2001-01-01';
SELECT geometry 'Point(1 1)' %<> tgeompoint 'Point(1 1)@2001-01-01';

--NULL
SELECT geometry 'Polygon empty' ?= tgeompoint 'Point(1 1)@2001-01-01';

SELECT geography 'Point(1 1)' ?= tgeogpoint 'Point(1 1)@2001-01-01';
SELECT geography 'Point(1 1)' ?<> tgeogpoint 'Point(1 1)@2001-01-01';
SELECT geography 'Point(1 1)' %= tgeogpoint 'Point(1 1)@2001-01-01';
SELECT geography 'Point(1 1)' %<> tgeogpoint 'Point(1 1)@2001-01-01';

-------------------------------------------------------------------------------

SELECT tgeompoint 'Point(1 1)@2001-01-01' ?= tgeompoint 'Point(1 1)@2001-01-01';
SELECT tgeompoint 'Point(1 1)@2001-01-01' ?<> tgeompoint 'Point(1 1)@2001-01-01';
SELECT tgeompoint 'Point(1 1)@2001-01-01' %= tgeompoint 'Point(1 1)@2001-01-01';
SELECT tgeompoint 'Point(1 1)@2001-01-01' %<> tgeompoint 'Point(1 1)@2001-01-01';

SELECT tgeogpoint 'Point(1 1)@2001-01-01' ?= tgeogpoint 'Point(1 1)@2001-01-01';
SELECT tgeogpoint 'Point(1 1)@2001-01-01' ?<> tgeogpoint 'Point(1 1)@2001-01-01';
SELECT tgeogpoint 'Point(1 1)@2001-01-01' %= tgeogpoint 'Point(1 1)@2001-01-01';
SELECT tgeogpoint 'Point(1 1)@2001-01-01' %<> tgeogpoint 'Point(1 1)@2001-01-01';

-- Two points meeting only at a bound of their common period that the period
-- excludes are never equal
SELECT tgeompoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02)' ?= tgeompoint '[Point(2 0)@2001-01-01, Point(0 0)@2001-01-02)';
SELECT tgeompoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02)' %<> tgeompoint '[Point(2 0)@2001-01-01, Point(0 0)@2001-01-02)';
SELECT tgeogpoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02)' ?= tgeogpoint '[Point(2 0)@2001-01-01, Point(0 0)@2001-01-02)';
SELECT tgeogpoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02)' %<> tgeogpoint '[Point(2 0)@2001-01-01, Point(0 0)@2001-01-02)';
SELECT tgeogpoint '[Point(0 0)@2001-01-01, Point(0 0)@2001-01-02]' ?= tgeogpoint '[Point(2 0)@2001-01-01, Point(0 0)@2001-01-02]';

-- NULL
SELECT tgeompoint 'Point(1 1)@2001-01-01' ?= tgeompoint 'Point(1 1)@2001-01-02';

/* Errors */
SELECT tgeompoint 'Point(1 1)@2001-01-01' ?= tgeompoint 'SRID=5676;Point(1 1)@2001-01-01';

-------------------------------------------------------------------------------

-- Test index support function for ever/always equal and intersects<Time>

CREATE INDEX tbl_tgeompoint_rtree_idx ON tbl_tgeompoint USING gist(temp);
CREATE INDEX tbl_tgeogpoint_rtree_idx ON tbl_tgeogpoint USING gist(temp);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tgeompoint WHERE temp ?= geometry 'Point(1 1)';
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE temp ?= geography 'Point(1.5 1.5)';

SELECT COUNT(*) FROM tbl_tgeompoint WHERE temp %= geometry 'Point(1 1)';
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE temp %= geography 'Point(1.5 1.5)';

DROP INDEX tbl_tgeompoint_rtree_idx;
DROP INDEX tbl_tgeogpoint_rtree_idx;

-------------------------------------------------------------------------------

-- Test index support function for ever/always equal and intersects<Time>

CREATE INDEX tbl_tgeompoint_quadtree_idx ON tbl_tgeompoint USING spgist(temp);
CREATE INDEX tbl_tgeogpoint_quadtree_idx ON tbl_tgeogpoint USING spgist(temp);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tgeompoint WHERE temp ?= geometry 'Point(1 1)';
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE temp ?= geography 'Point(1.5 1.5)';

SELECT COUNT(*) FROM tbl_tgeompoint WHERE temp %= geometry 'Point(1 1)';
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE temp %= geography 'Point(1.5 1.5)';

DROP INDEX tbl_tgeompoint_quadtree_idx;
DROP INDEX tbl_tgeogpoint_quadtree_idx;

-------------------------------------------------------------------------------
-- Temporal equal
-------------------------------------------------------------------------------

SELECT geometry 'Point(1 1)' #= tgeompoint 'Point(1 1)@2001-01-01';
SELECT geometry 'Point(1 1)' #= tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT geometry 'Point(1 1)' #= tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT geometry 'Point(1 1)' #= tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT geometry 'Point empty' #= tgeompoint 'Point(1 1)@2001-01-01';
SELECT geometry 'Point empty' #= tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT geometry 'Point empty' #= tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT geometry 'Point empty' #= tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT geometry 'Point(1 1)' #= tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT geometry 'Point(1.5 1.5)' #= tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';

SELECT geometry 'Point(1 1 1)' #= tgeompoint 'Point(1 1 1)@2001-01-01';
SELECT geometry 'Point(1 1 1)' #= tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT geometry 'Point(1 1 1)' #= tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT geometry 'Point(1 1 1)' #= tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT geometry 'Point Z empty' #= tgeompoint 'Point(1 1 1)@2001-01-01';
SELECT geometry 'Point Z empty' #= tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT geometry 'Point Z empty' #= tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT geometry 'Point Z empty' #= tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT tgeompoint 'Point(1 1)@2001-01-01' #= geometry 'Point(1 1)';
SELECT tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #= geometry 'Point(1 1)';
SELECT tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #= geometry 'Point(1 1)';
SELECT tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #= geometry 'Point(1 1)';

SELECT tgeompoint 'Point(1 1)@2001-01-01' #= geometry 'Point empty';
SELECT tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #= geometry 'Point empty';
SELECT tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #= geometry 'Point empty';
SELECT tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #= geometry 'Point empty';

SELECT tgeompoint 'Point(1 1 1)@2001-01-01' #= geometry 'Point(1 1 1)';
SELECT tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #= geometry 'Point(1 1 1)';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #= geometry 'Point(1 1 1)';
SELECT tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #= geometry 'Point(1 1 1)';

SELECT tgeompoint 'Point(1 1 1)@2001-01-01' #= geometry 'Point Z empty';
SELECT tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #= geometry 'Point Z empty';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #= geometry 'Point Z empty';
SELECT tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #= geometry 'Point Z empty';

SELECT tgeompoint 'Point(1 1)@2001-01-01' #= tgeompoint 'Point(1 1)@2001-01-01';
SELECT tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #= tgeompoint 'Point(1 1)@2001-01-01';
SELECT tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #= tgeompoint 'Point(1 1)@2001-01-01';
SELECT tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #= tgeompoint 'Point(1 1)@2001-01-01';
SELECT tgeompoint 'Point(1 1)@2001-01-01' #= tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #= tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #= tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #= tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeompoint 'Point(1 1)@2001-01-01' #= tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #= tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #= tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #= tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeompoint 'Point(1 1)@2001-01-01' #= tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #= tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #= tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #= tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT tgeompoint 'Interp=Step;[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #= tgeompoint 'Interp=Step;[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeompoint 'Interp=Step;{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #= tgeompoint 'Interp=Step;[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeompoint 'Interp=Step;[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #= tgeompoint 'Interp=Step;{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeompoint 'Interp=Step;{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #= tgeompoint 'Interp=Step;{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT tgeompoint '[Point(1 1)@2001-01-01, Point(1 1)@2001-01-03]' #= tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeompoint '[Point(1.5 1.5)@2001-01-01, Point(1.5 1.5)@2001-01-03]' #= tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(1 2 2)@2001-01-02]' #= tgeompoint '[Point(1 2 1)@2001-01-01, Point(1 1 2)@2001-01-02]';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 1 2)@2001-01-02]' #= tgeompoint '[Point(2 1 1)@2001-01-01, Point(1 1 2)@2001-01-02]';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 1)@2001-01-02]' #= tgeompoint '[Point(2 1 1)@2001-01-01, Point(1 2 1)@2001-01-02]';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 3 1)@2001-01-02]' #= tgeompoint '[Point(2 2 1)@2001-01-01, Point(1 2 1)@2001-01-02]';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 1)@2001-01-02]' #= tgeompoint '[Point(2 2 0.5)@2001-01-01, Point(3 3 1.5)@2001-01-02]';

SELECT tgeompoint 'Point(1 1 1)@2001-01-01' #= tgeompoint 'Point(1 1 1)@2001-01-01';
SELECT tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #= tgeompoint 'Point(1 1 1)@2001-01-01';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #= tgeompoint 'Point(1 1 1)@2001-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #= tgeompoint 'Point(1 1 1)@2001-01-01';
SELECT tgeompoint 'Point(1 1 1)@2001-01-01' #= tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #= tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #= tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #= tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeompoint 'Point(1 1 1)@2001-01-01' #= tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #= tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #= tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #= tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeompoint 'Point(1 1 1)@2001-01-01' #= tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #= tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #= tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #= tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT tgeompoint 'Interp=Step;[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #= tgeompoint 'Interp=Step;[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeompoint 'Interp=Step;{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #= tgeompoint 'Interp=Step;[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeompoint 'Interp=Step;[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #= tgeompoint 'Interp=Step;{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeompoint 'Interp=Step;{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #= tgeompoint 'Interp=Step;{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT geography 'Point(1 1)' #= tgeogpoint 'Point(1.5 1.5)@2001-01-01';
SELECT geography 'Point(1 1)' #= tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}';
SELECT geography 'Point(1 1)' #= tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]';
SELECT geography 'Point(1 1)' #= tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}';

SELECT geography 'Point empty' #= tgeogpoint 'Point(1.5 1.5)@2001-01-01';
SELECT geography 'Point empty' #= tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}';
SELECT geography 'Point empty' #= tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]';
SELECT geography 'Point empty' #= tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}';

SELECT geography 'Point(1 1 1)' #= tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01';
SELECT geography 'Point(1 1 1)' #= tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}';
SELECT geography 'Point(1 1 1)' #= tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]';
SELECT geography 'Point(1 1 1)' #= tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}';

SELECT geography 'Point Z empty' #= tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01';
SELECT geography 'Point Z empty' #= tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}';
SELECT geography 'Point Z empty' #= tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]';
SELECT geography 'Point Z empty' #= tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}';

SELECT tgeogpoint 'Point(1.5 1.5)@2001-01-01' #= geography 'Point(1 1)';
SELECT tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}' #= geography 'Point(1 1)';
SELECT tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]' #= geography 'Point(1 1)';
SELECT tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}' #= geography 'Point(1 1)';

SELECT tgeogpoint 'Point(1.5 1.5)@2001-01-01' #= geography 'Point empty';
SELECT tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}' #= geography 'Point empty';
SELECT tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]' #= geography 'Point empty';
SELECT tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}' #= geography 'Point empty';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01' #= geography 'Point(1 1 1)';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}' #= geography 'Point(1 1 1)';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]' #= geography 'Point(1 1 1)';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}' #= geography 'Point(1 1 1)';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01' #= geography 'Point Z empty';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}' #= geography 'Point Z empty';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]' #= geography 'Point Z empty';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}' #= geography 'Point Z empty';

SELECT tgeogpoint 'Point(1.5 1.5)@2001-01-01' #= tgeogpoint 'Point(1.5 1.5)@2001-01-01';
SELECT tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}' #= tgeogpoint 'Point(1.5 1.5)@2001-01-01';
SELECT tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]' #= tgeogpoint 'Point(1.5 1.5)@2001-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}' #= tgeogpoint 'Point(1.5 1.5)@2001-01-01';
SELECT tgeogpoint 'Point(1.5 1.5)@2001-01-01' #= tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}' #= tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]' #= tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}' #= tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5)@2001-01-01' #= tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]';
SELECT tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}' #= tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]';
SELECT tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]' #= tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}' #= tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]';
SELECT tgeogpoint 'Point(1.5 1.5)@2001-01-01' #= tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}';
SELECT tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}' #= tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}';
SELECT tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]' #= tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}' #= tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01' #= tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}' #= tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]' #= tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}' #= tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01';
SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01' #= tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}' #= tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]' #= tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}' #= tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01' #= tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}' #= tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]' #= tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}' #= tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]';
SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01' #= tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}' #= tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]' #= tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}' #= tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}';

-------------------------------------------------------------------------------
-- Temporal ne
-------------------------------------------------------------------------------

SELECT geometry 'Point(1 1)' #<> tgeompoint 'Point(1 1)@2001-01-01';
SELECT geometry 'Point(1 1)' #<> tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT geometry 'Point(1 1)' #<> tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT geometry 'Point(1 1)' #<> tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT geometry 'Point empty' #<> tgeompoint 'Point(1 1)@2001-01-01';
SELECT geometry 'Point empty' #<> tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT geometry 'Point empty' #<> tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT geometry 'Point empty' #<> tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT geometry 'Point(1 1)' #<> tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT geometry 'Point(1.5 1.5)' #<> tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';

SELECT geometry 'Point(1 1 1)' #<> tgeompoint 'Point(1 1 1)@2001-01-01';
SELECT geometry 'Point(1 1 1)' #<> tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT geometry 'Point(1 1 1)' #<> tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT geometry 'Point(1 1 1)' #<> tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT geometry 'Point Z empty' #<> tgeompoint 'Point(1 1 1)@2001-01-01';
SELECT geometry 'Point Z empty' #<> tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT geometry 'Point Z empty' #<> tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT geometry 'Point Z empty' #<> tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT tgeompoint 'Point(1 1)@2001-01-01' #<> geometry 'Point(1 1)';
SELECT tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #<> geometry 'Point(1 1)';
SELECT tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #<> geometry 'Point(1 1)';
SELECT tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #<> geometry 'Point(1 1)';

SELECT tgeompoint 'Point(1 1)@2001-01-01' #<> geometry 'Point empty';
SELECT tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #<> geometry 'Point empty';
SELECT tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #<> geometry 'Point empty';
SELECT tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #<> geometry 'Point empty';

SELECT tgeompoint 'Point(1 1 1)@2001-01-01' #<> geometry 'Point(1 1 1)';
SELECT tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #<> geometry 'Point(1 1 1)';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #<> geometry 'Point(1 1 1)';
SELECT tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #<> geometry 'Point(1 1 1)';

SELECT tgeompoint 'Point(1 1 1)@2001-01-01' #<> geometry 'Point Z empty';
SELECT tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #<> geometry 'Point Z empty';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #<> geometry 'Point Z empty';
SELECT tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #<> geometry 'Point Z empty';

SELECT tgeompoint 'Point(1 1)@2001-01-01' #<> tgeompoint 'Point(1 1)@2001-01-01';
SELECT tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #<> tgeompoint 'Point(1 1)@2001-01-01';
SELECT tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #<> tgeompoint 'Point(1 1)@2001-01-01';
SELECT tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #<> tgeompoint 'Point(1 1)@2001-01-01';
SELECT tgeompoint 'Point(1 1)@2001-01-01' #<> tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #<> tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #<> tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #<> tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeompoint 'Point(1 1)@2001-01-01' #<> tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #<> tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #<> tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #<> tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeompoint 'Point(1 1)@2001-01-01' #<> tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeompoint '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #<> tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #<> tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #<> tgeompoint '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT tgeompoint '[Point(1 1)@2001-01-01, Point(1 1)@2001-01-03]' #<> tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeompoint '[Point(1.5 1.5)@2001-01-01, Point(1.5 1.5)@2001-01-03]' #<> tgeompoint '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';

SELECT tgeompoint 'Point(1 1 1)@2001-01-01' #<> tgeompoint 'Point(1 1 1)@2001-01-01';
SELECT tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #<> tgeompoint 'Point(1 1 1)@2001-01-01';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #<> tgeompoint 'Point(1 1 1)@2001-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #<> tgeompoint 'Point(1 1 1)@2001-01-01';
SELECT tgeompoint 'Point(1 1 1)@2001-01-01' #<> tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #<> tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #<> tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #<> tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeompoint 'Point(1 1 1)@2001-01-01' #<> tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #<> tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #<> tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #<> tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeompoint 'Point(1 1 1)@2001-01-01' #<> tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeompoint '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #<> tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeompoint '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #<> tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #<> tgeompoint '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT geography 'Point(1 1)' #<> tgeogpoint 'Point(1.5 1.5)@2001-01-01';
SELECT geography 'Point(1 1)' #<> tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}';
SELECT geography 'Point(1 1)' #<> tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]';
SELECT geography 'Point(1 1)' #<> tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}';

SELECT geography 'Point empty' #<> tgeogpoint 'Point(1.5 1.5)@2001-01-01';
SELECT geography 'Point empty' #<> tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}';
SELECT geography 'Point empty' #<> tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]';
SELECT geography 'Point empty' #<> tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}';

SELECT geography 'Point(1 1 1)' #<> tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01';
SELECT geography 'Point(1 1 1)' #<> tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}';
SELECT geography 'Point(1 1 1)' #<> tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]';
SELECT geography 'Point(1 1 1)' #<> tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}';

SELECT geography 'Point Z empty' #<> tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01';
SELECT geography 'Point Z empty' #<> tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}';
SELECT geography 'Point Z empty' #<> tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]';
SELECT geography 'Point Z empty' #<> tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}';

SELECT tgeogpoint 'Point(1.5 1.5)@2001-01-01' #<> geography 'Point(1 1)';
SELECT tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}' #<> geography 'Point(1 1)';
SELECT tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]' #<> geography 'Point(1 1)';
SELECT tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}' #<> geography 'Point(1 1)';

SELECT tgeogpoint 'Point(1.5 1.5)@2001-01-01' #<> geography 'Point empty';
SELECT tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}' #<> geography 'Point empty';
SELECT tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]' #<> geography 'Point empty';
SELECT tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}' #<> geography 'Point empty';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01' #<> geography 'Point(1 1 1)';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}' #<> geography 'Point(1 1 1)';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]' #<> geography 'Point(1 1 1)';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}' #<> geography 'Point(1 1 1)';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01' #<> geography 'Point Z empty';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}' #<> geography 'Point Z empty';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]' #<> geography 'Point Z empty';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}' #<> geography 'Point Z empty';

SELECT tgeogpoint 'Point(1.5 1.5)@2001-01-01' #<> tgeogpoint 'Point(1.5 1.5)@2001-01-01';
SELECT tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}' #<> tgeogpoint 'Point(1.5 1.5)@2001-01-01';
SELECT tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]' #<> tgeogpoint 'Point(1.5 1.5)@2001-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}' #<> tgeogpoint 'Point(1.5 1.5)@2001-01-01';
SELECT tgeogpoint 'Point(1.5 1.5)@2001-01-01' #<> tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}' #<> tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]' #<> tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}' #<> tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5)@2001-01-01' #<> tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]';
SELECT tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}' #<> tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]';
SELECT tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]' #<> tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}' #<> tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]';
SELECT tgeogpoint 'Point(1.5 1.5)@2001-01-01' #<> tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}';
SELECT tgeogpoint '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}' #<> tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}';
SELECT tgeogpoint '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]' #<> tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}' #<> tgeogpoint '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01' #<> tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}' #<> tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]' #<> tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}' #<> tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01';
SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01' #<> tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}' #<> tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]' #<> tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}' #<> tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01' #<> tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}' #<> tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]' #<> tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}' #<> tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]';
SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2001-01-01' #<> tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03}' #<> tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03]' #<> tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}' #<> tgeogpoint '{[Point(1.5 1.5 1.5)@2001-01-01, Point(2.5 2.5 2.5)@2001-01-02, Point(1.5 1.5 1.5)@2001-01-03],[Point(3.5 3.5 3.5)@2001-01-04, Point(3.5 3.5 3.5)@2001-01-05]}';

-------------------------------------------------------------------------------

-- Ever and always overlaps: a temporal point and a box, and two temporal points

SELECT tgeompoint '[Point(0 0)@2001-01-01 00:00, Point(10 10)@2001-01-01 10:00]' ?&& stbox 'STBOX XT(((8,0),(10,2)),[2001-01-01 00:00, 2001-01-01 10:00])';
SELECT tgeompoint '[Point(0 0)@2001-01-01 00:00, Point(10 10)@2001-01-01 10:00]' ?&& stbox 'STBOX XT(((4,4),(6,6)),[2001-01-01 04:00, 2001-01-01 06:00])';
SELECT tgeompoint '[Point(0 0)@2001-01-01 00:00, Point(10 10)@2001-01-01 10:00]' ?&& stbox 'STBOX XT(((4,4),(6,6)),[2001-01-01 08:00, 2001-01-01 10:00])';
SELECT stbox 'STBOX T([2001-01-01 09:00, 2001-01-01 12:00])' ?&& tgeompoint '[Point(0 0)@2001-01-01 00:00, Point(10 10)@2001-01-01 10:00]';
SELECT tgeompoint '[Point(0 0)@2001-01-01 00:00, Point(10 10)@2001-01-01 10:00]' %&& stbox 'STBOX X((0,0),(10,10))';
SELECT tgeompoint '[Point(0 0)@2001-01-01 00:00, Point(10 10)@2001-01-01 10:00]' %&& stbox 'STBOX XT(((0,0),(10,10)),[2001-01-01 00:00, 2001-01-01 05:00])';
SELECT stbox 'STBOX X((0,0),(10,10))' %&& tgeompoint '{Point(1 1)@2001-01-01, Point(11 11)@2001-01-02}';
SELECT tgeogpoint '[Point(0 0)@2001-01-01, Point(10 10)@2001-01-02]' ?&& stbox 'SRID=4326;GEODSTBOX ZT(((4,4,0),(6,6,0)),[2001-01-01, 2001-01-02])';
-- Two temporal points crossing between their instants
SELECT tgeompoint '[Point(0 0)@2001-01-01, Point(2 2)@2001-01-03]' ?&& tgeompoint '[Point(2 0)@2001-01-01, Point(0 2)@2001-01-03]';
SELECT tgeompoint '[Point(0 0)@2001-01-01, Point(2 2)@2001-01-03]' %&& tgeompoint '[Point(2 0)@2001-01-01, Point(0 2)@2001-01-03]';
SELECT tgeompoint '[Point(0 0)@2001-01-01, Point(2 2)@2001-01-03]' ?&& tgeompoint '[Point(0 1)@2001-01-01, Point(2 3)@2001-01-03]';
-- Two temporal points meeting halfway between two microseconds, dated by the
-- even one, and a third of a microsecond after the start, dated by the start,
-- an instant at which they differ
SELECT tgeompoint '[Point(0 0)@2001-01-01 00:00:00, Point(2 0)@2001-01-01 00:00:00.000003]' #= tgeompoint '[Point(2 0)@2001-01-01 00:00:00, Point(0 0)@2001-01-01 00:00:00.000003]';
SELECT tgeompoint '[Point(0 0)@2001-01-01, Point(1 0)@2001-01-02]' #= tgeompoint '[Point(-1 0)@2001-01-01, Point(259200000000 0)@2001-01-02]';
SELECT tgeompoint '[Point(0 0)@2001-01-01, Point(1 0)@2001-01-02]' ?= tgeompoint '[Point(-1 0)@2001-01-01, Point(259200000000 0)@2001-01-02]';
SELECT tgeompoint '(Point(0 0)@2001-01-01, Point(1 0)@2001-01-02]' ?= tgeompoint '(Point(-1 0)@2001-01-01, Point(259200000000 0)@2001-01-02]';
SELECT tgeompoint '(Point(0 0)@2001-01-01, Point(1 0)@2001-01-02]' %<> tgeompoint '(Point(-1 0)@2001-01-01, Point(259200000000 0)@2001-01-02]';
-- Errors
SELECT tgeompoint 'SRID=4326;Point(1 1)@2001-01-01' ?&& stbox 'STBOX X((0,0),(2,2))';
SELECT tgeogpoint 'Point(1 1)@2001-01-01' ?&& stbox 'STBOX X((0,0),(2,2))';

-------------------------------------------------------------------------------
