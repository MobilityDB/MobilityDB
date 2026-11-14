CREATE TABLE test AS
SELECT NULL::tgeompoint FROM generate_series(1, 10);
ANALYZE test;
DROP TABLE test;

-------------------------------------------------------------------------------

ANALYZE tbl_tgeompoint3D_big;
ANALYZE tbl_tgeogpoint3D_big;

DROP INDEX IF EXISTS tbl_tgeompoint3D_big_rtree_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint3D_big_rtree_idx;

DROP INDEX IF EXISTS tbl_tgeompoint3D_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint3D_big_quadtree_idx;

DROP INDEX IF EXISTS tbl_tgeompoint3D_big_kdtree_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint3D_big_kdtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3D_big_rtree_idx ON tbl_tgeompoint3D_big USING GIST(temp);
CREATE INDEX tbl_tgeogpoint3D_big_rtree_idx ON tbl_tgeogpoint3D_big USING GIST(temp);

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp < tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <= tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp > tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp >= tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp && tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp @> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <@ tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp ~= tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp -|- tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp << tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &< tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp >> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<| tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) > 0 FROM tbl_tgeompoint3D_big WHERE temp &<| tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |>> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |&> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <</ tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &</ tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp />> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp /&> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |&> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<# tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &<# tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #>> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #&> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxLeft(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverleft(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxRight(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverright(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxBelow(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) > 0 FROM tbl_tgeompoint3D_big WHERE stboxOverbelow(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxAbove(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverabove(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxFront(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverfront(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxBack(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverback(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxBefore(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverbefore(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxAfter(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverafter(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp < tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <= tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp > tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp >= tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp && tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp && stbox 'GEODSTBOX ZT(((1,40,1),(10,50,500)),[2001-01-01, 2001-02-01])';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp ?&& stbox 'GEODSTBOX ZT(((1,40,1),(10,50,500)),[2001-01-01, 2001-02-01])';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp %&& stbox 'GEODSTBOX ZT(((1,40,1),(10,50,500)),[2001-01-01, 2001-02-01])';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp @> tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <@ tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp ~= tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp -|- tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <<# tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp &<# tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #>> tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #&> tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

-- Test the commutator for the selectivity
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]' <<# temp;
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]' &<# temp;

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeompoint3D_big_rtree_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint3D_big_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3D_big_quadtree_idx ON tbl_tgeompoint3D_big USING SPGIST(temp);
CREATE INDEX tbl_tgeogpoint3D_big_quadtree_idx ON tbl_tgeogpoint3D_big USING SPGIST(temp);

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp && tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp @> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <@ tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp ~= tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp -|- tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp << tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &< tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp >> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<| tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &<| tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |>> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |&> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <</ tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &</ tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp />> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp /&> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |&> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<# tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &<# tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #>> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #&> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxLeft(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverleft(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxRight(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverright(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxBelow(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverbelow(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxAbove(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverabove(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxFront(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverfront(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxBack(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverback(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxBefore(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverbefore(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxAfter(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverafter(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp && tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp && stbox 'GEODSTBOX ZT(((1,40,1),(10,50,500)),[2001-01-01, 2001-02-01])';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp ?&& stbox 'GEODSTBOX ZT(((1,40,1),(10,50,500)),[2001-01-01, 2001-02-01])';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp %&& stbox 'GEODSTBOX ZT(((1,40,1),(10,50,500)),[2001-01-01, 2001-02-01])';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp @> tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <@ tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp ~= tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp -|- tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <<# tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp &<# tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #>> tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #&> tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeompoint3D_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint3D_big_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3D_big_kdtree_idx ON tbl_tgeompoint3D_big USING SPGIST(temp tgeompoint_kdtree_ops);
CREATE INDEX tbl_tgeogpoint3D_big_kdtree_idx ON tbl_tgeogpoint3D_big USING SPGIST(temp tgeogpoint_kdtree_ops);

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp && tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp @> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <@ tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp ~= tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp -|- tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp << tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &< tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp >> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<| tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &<| tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |>> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |&> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <</ tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &</ tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp />> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp /&> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |&> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<# tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &<# tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #>> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #&> tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxLeft(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverleft(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxRight(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverright(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxBelow(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverbelow(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxAbove(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverabove(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxFront(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverfront(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxBack(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverback(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxBefore(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverbefore(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxAfter(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE stboxOverafter(temp, tgeompoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp && tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp && stbox 'GEODSTBOX ZT(((1,40,1),(10,50,500)),[2001-01-01, 2001-02-01])';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp ?&& stbox 'GEODSTBOX ZT(((1,40,1),(10,50,500)),[2001-01-01, 2001-02-01])';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp %&& stbox 'GEODSTBOX ZT(((1,40,1),(10,50,500)),[2001-01-01, 2001-02-01])';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp @> tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <@ tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp ~= tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp -|- tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <<# tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp &<# tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #>> tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #&> tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeompoint3D_big_kdtree_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint3D_big_kdtree_idx;

-------------------------------------------------------------------------------

ANALYZE tbl_tgeompoint;
ANALYZE tbl_tgeompoint3D;

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeompoint_rtree_idx;
DROP INDEX IF EXISTS tbl_tgeompoint3D_rtree_idx;
CREATE INDEX tbl_tgeompoint_rtree_idx ON tbl_tgeompoint USING GIST(temp);
CREATE INDEX tbl_tgeompoint3D_rtree_idx ON tbl_tgeompoint3D USING GIST(temp);

-- EXPLAIN ANALYZE
WITH test AS (
  SELECT temp |=| tgeompoint '[Point(1 1)@2001-06-01, Point(2 2)@2001-07-01]' AS distance FROM tbl_tgeompoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| tgeompoint '[Point(-1 -1 -1)@2001-06-01, Point(-2 -2 -2)@2001-07-01]' AS distance FROM tbl_tgeompoint3D ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

WITH test AS (
  SELECT temp |=| geometry 'Point(1 1)' AS distance FROM tbl_tgeompoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| stbox 'STBOX XT(((1,1),(2,2)),[2001-06-01, 2001-07-01])' AS distance FROM tbl_tgeompoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

WITH test AS (
  SELECT temp |=| tgeogpoint '[Point(1 1)@2001-06-01, Point(2 2)@2001-07-01]' AS distance FROM tbl_tgeogpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| geography 'Point(1 1)' AS distance FROM tbl_tgeogpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
-- An empty geometry has no bounding box for the index to read
WITH test AS (
  SELECT temp |=| geometry 'Polygon empty' AS distance FROM tbl_tgeompoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
DROP INDEX tbl_tgeompoint_rtree_idx;
DROP INDEX tbl_tgeompoint3D_rtree_idx;

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeompoint_quadtree_idx;
DROP INDEX IF EXISTS tbl_tgeompoint3D_quadtree_idx;
CREATE INDEX tbl_tgeompoint_quadtree_idx ON tbl_tgeompoint USING SPGIST(temp);
CREATE INDEX tbl_tgeompoint3D_quadtree_idx ON tbl_tgeompoint3D USING SPGIST(temp);

-- EXPLAIN ANALYZE
WITH test AS (
  SELECT temp |=| tgeompoint '[Point(1 1)@2001-06-01, Point(2 2)@2001-07-01]' AS distance FROM tbl_tgeompoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| tgeompoint '[Point(-1 -1 -1)@2001-06-01, Point(-2 -2 -2)@2001-07-01]' AS distance FROM tbl_tgeompoint3D ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

WITH test AS (
  SELECT temp |=| geometry 'Point(1 1)' AS distance FROM tbl_tgeompoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| stbox 'STBOX XT(((1,1),(2,2)),[2001-06-01, 2001-07-01])' AS distance FROM tbl_tgeompoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

WITH test AS (
  SELECT temp |=| tgeogpoint '[Point(1 1)@2001-06-01, Point(2 2)@2001-07-01]' AS distance FROM tbl_tgeogpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| geography 'Point(1 1)' AS distance FROM tbl_tgeogpoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
-- An empty geometry has no bounding box for the index to read
WITH test AS (
  SELECT temp |=| geometry 'Polygon empty' AS distance FROM tbl_tgeompoint ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
DROP INDEX tbl_tgeompoint_quadtree_idx;
DROP INDEX tbl_tgeompoint3D_quadtree_idx;

-------------------------------------------------------------------------------
-- Coverage of all the same and order by logic in SP-GiST indexes

CREATE TABLE tbl_tgeompoint3D_big_allthesame AS SELECT k, tgeompoint(geometry 'Point(5 5 5)', t) AS temp FROM tbl_tstzspan_big;
CREATE INDEX tbl_tgeompoint3D_big_allthesame_quadtree_idx ON tbl_tgeompoint3D_big_allthesame USING SPGIST(temp);
ANALYZE tbl_tgeompoint3D_big_allthesame;

-- EXPLAIN ANALYZE

DROP TABLE tbl_tgeompoint3D_big_allthesame;

-------------------------------------------------------------------------------


-------------------------------------------------------------------------------
-- A quad-tree node whose children are all the same computes one ordering
-- distance per ordering key from the parent node box. A table of identical
-- values is what builds such a node.
-------------------------------------------------------------------------------

CREATE TABLE tbl_tgeompoint_allthesame AS
  SELECT k, tgeompoint 'Point(5 5)@2001-01-01' AS temp
  FROM generate_series(1, 1000) k;
CREATE INDEX tbl_tgeompoint_allthesame_quadtree_idx ON tbl_tgeompoint_allthesame
  USING SPGIST(temp);
ANALYZE tbl_tgeompoint_allthesame;

SET enable_seqscan = off;
WITH test AS (
  SELECT temp |=| geometry 'Point(0 0)' AS distance
  FROM tbl_tgeompoint_allthesame ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
RESET enable_seqscan;

DROP TABLE tbl_tgeompoint_allthesame;

--------------------------------------------------------------------------------
-- A node key must keep the inclusivity of the bound it is expanded to, and
-- two keys that differ only in that inclusivity are not the same key. Every
-- value below ends on an EXCLUSIVE upper bound and the last one is an instant
-- sitting exactly on the largest of them, so a key that takes the bound value
-- while keeping the exclusivity of the entry it starts from excludes that
-- instant and the scan prunes the page that holds it. Several sizes are built
-- because which node ends on that bound follows from how the pages split.

CREATE FUNCTION gist_bound_inc_disagreements() RETURNS bigint AS $$
DECLARE n int; s bigint; i bigint; bad bigint := 0;
BEGIN
  FOREACH n IN ARRAY ARRAY[200, 300, 400, 800, 1200] LOOP
    DROP TABLE IF EXISTS tbl_gist_bound_inc;
    EXECUTE format($f$CREATE TABLE tbl_gist_bound_inc AS
      SELECT i AS k, tgeompoint(format('[Point(%%s %%s)@%%s, Point(%%s %%s)@%%s)',
        i %% 97, (i * 7) %% 89,
        timestamptz '2001-01-01' + (i || ' minutes')::interval,
        (i + 1) %% 97, (i * 7 + 3) %% 89,
        timestamptz '2001-01-01' + ((i + 300) || ' minutes')::interval)) AS temp
      FROM generate_series(1, %s) AS i$f$, n);
    INSERT INTO tbl_gist_bound_inc
    SELECT 0, tgeompoint(format('Point(50 50)@%s',
      (SELECT max(endTimestamp(temp)) FROM tbl_gist_bound_inc)));
    CREATE INDEX tbl_gist_bound_inc_rtree_idx
      ON tbl_gist_bound_inc USING gist(temp);
    ANALYZE tbl_gist_bound_inc;
    SET LOCAL enable_seqscan = on;
    SET LOCAL enable_bitmapscan = off;
    SET LOCAL enable_indexscan = off;
    SELECT COUNT(*) INTO s FROM tbl_gist_bound_inc
    WHERE temp #&> (SELECT temp FROM tbl_gist_bound_inc WHERE k = 0);
    SET LOCAL enable_seqscan = off;
    SET LOCAL enable_bitmapscan = on;
    SET LOCAL enable_indexscan = on;
    SELECT COUNT(*) INTO i FROM tbl_gist_bound_inc
    WHERE temp #&> (SELECT temp FROM tbl_gist_bound_inc WHERE k = 0);
    IF s <> i THEN bad := bad + 1; END IF;
  END LOOP;
  RETURN bad;
END;
$$ LANGUAGE plpgsql;

SELECT gist_bound_inc_disagreements() AS sizes_where_the_index_disagrees;

DROP FUNCTION gist_bound_inc_disagreements();
DROP TABLE tbl_gist_bound_inc;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- An instant [t, t] is after a span [a, t): the bound they share belongs to
-- the instant only. A node of a space-partitioning index whose values reach t
-- and no further therefore holds an answer. Every sequence below ends on the
-- same timestamp, so every centroid ends there and so does every node below
-- one; the instants sitting on that bound are the answers of a query excluding
-- it. The answer of each operator class is compared with the sequential scan.
-------------------------------------------------------------------------------

CREATE FUNCTION spgist_shared_bound_disagreements(opclass text)
RETURNS bigint AS $$
DECLARE scan int; s bigint; i bigint; bad bigint := 0;
  t timestamptz := timestamptz '2001-01-01';
BEGIN
  DROP TABLE IF EXISTS tbl_spgist_shared_bound;
  CREATE TABLE tbl_spgist_shared_bound AS
    SELECT k, CASE WHEN k % 10 = 0
      THEN tgeompoint(format('Point(%s %s)@%s', k % 89, k % 83,
        t + interval '100 s'))
      ELSE tgeompoint(format('[Point(%s %s)@%s, Point(%s %s)@%s]', k % 89,
        k % 83, t + (k % 97) * interval '1 s', k % 83, k % 89,
        t + interval '100 s')) END AS temp
    FROM generate_series(1, 5000) AS k;
  EXECUTE format('CREATE INDEX tbl_spgist_shared_bound_idx ON '
    'tbl_spgist_shared_bound USING spgist(temp %s)', opclass);
  ANALYZE tbl_spgist_shared_bound;
  FOR scan IN 0..1 LOOP
    IF scan = 0 THEN
      SET LOCAL enable_seqscan = on;
      SET LOCAL enable_bitmapscan = off;
      SET LOCAL enable_indexscan = off;
    ELSE
      SET LOCAL enable_seqscan = off;
      SET LOCAL enable_bitmapscan = on;
      SET LOCAL enable_indexscan = on;
    END IF;
    SELECT COUNT(*) INTO i FROM tbl_spgist_shared_bound WHERE temp #>>
      span(t + interval '95 s', t + interval '100 s', true, false);
    IF scan = 0 THEN s := i; ELSE bad := s - i; END IF;
  END LOOP;
  DROP TABLE tbl_spgist_shared_bound;
  RETURN bad;
END;
$$ LANGUAGE plpgsql;

SELECT spgist_shared_bound_disagreements('tgeompoint_quadtree_ops')
  AS quadtree_answers_the_index_misses;
SELECT spgist_shared_bound_disagreements('tgeompoint_kdtree_ops')
  AS kdtree_answers_the_index_misses;

DROP FUNCTION spgist_shared_bound_disagreements(text);

-------------------------------------------------------------------------------
-- An index over boxes of several structures answers as the sequential scan:
-- the same count, or an error where the scan raises one, for every operator
-- and every query, and the same 25 nearest distances for |=|. Where the boxes
-- of several structures raise different errors, the error is the one of the
-- first box a plan meets, which no plan orders. Per operator class: whether
-- the plan reads the index, how many answers are errors, and how many differ
-- from the scan
-------------------------------------------------------------------------------

CREATE FUNCTION index_structures(tbl text, typ text, opclasses text[],
  queries text[], ops text[])
RETURNS TABLE(opclass text, uses_index boolean, errors int,
  disagreements int) AS $$
DECLARE
  a text; o text; q text; r bigint; ans text; line text;
  seqans jsonb := '{}';
BEGIN
  FOREACH a IN ARRAY array_prepend('seq', opclasses) LOOP
    EXECUTE format('DROP INDEX IF EXISTS %I', tbl || '_idx');
    IF a = 'seq' THEN
      SET LOCAL enable_seqscan = on;
      SET LOCAL enable_indexscan = off;
      SET LOCAL enable_bitmapscan = off;
    ELSE
      EXECUTE format('CREATE INDEX %I ON %I USING %s', tbl || '_idx', tbl, a);
      SET LOCAL enable_seqscan = off;
      SET LOCAL enable_indexscan = on;
      SET LOCAL enable_bitmapscan = on;
    END IF;
    opclass := a; uses_index := false; errors := 0; disagreements := 0;
    FOR line IN EXECUTE format('EXPLAIN SELECT count(*) FROM %I WHERE b && %L::%s',
      tbl, queries[1], typ) LOOP
      uses_index := uses_index OR line LIKE '%' || tbl || '_idx%';
    END LOOP;
    FOREACH o IN ARRAY ops LOOP
      FOREACH q IN ARRAY queries LOOP
        BEGIN
          IF o = '|=|' THEN
            EXECUTE format('SELECT string_agg(coalesce(round(d::numeric, 6)::text, '
              '''null''), '','' ORDER BY d) FROM (SELECT b |=| %L::%s AS d '
              'FROM %I ORDER BY b |=| %L::%s LIMIT 25) s', q, typ, tbl, q, typ)
              INTO ans;
          ELSE
            EXECUTE format('SELECT count(*) FROM %I WHERE b %s %L::%s',
              tbl, o, q, typ) INTO r;
            ans := r::text;
          END IF;
        EXCEPTION WHEN OTHERS THEN
          ans := 'ERROR: ' || SQLERRM;
          errors := errors + 1;
        END;
        IF a = 'seq' THEN
          seqans := seqans || jsonb_build_object(o || ' ' || q, ans);
        ELSIF seqans ->> (o || ' ' || q) IS DISTINCT FROM ans AND
            NOT (seqans ->> (o || ' ' || q) LIKE 'ERROR: %' AND
              ans LIKE 'ERROR: %') THEN
          disagreements := disagreements + 1;
        END IF;
      END LOOP;
    END LOOP;
    RETURN NEXT;
  END LOOP;
  EXECUTE format('DROP INDEX IF EXISTS %I', tbl || '_idx');
END;
$$ LANGUAGE plpgsql;

CREATE FUNCTION day_of(d int) RETURNS timestamptz AS $$
  SELECT timestamptz '2001-01-01' + d * interval '1 day'
$$ LANGUAGE sql IMMUTABLE;

CREATE TABLE tbl_index_structures AS
  SELECT k, CASE k % 7
    WHEN 0 THEN stbox(format('SRID=3857;STBOX XT(((%s,%s),(%s,%s)),[%s, %s])',
      k % 50, k % 47, k % 50 + 2, k % 47 + 2, day_of(k % 40), day_of(k % 40 + 2)))
    WHEN 1 THEN stbox(format('SRID=5676;STBOX XT(((%s,%s),(%s,%s)),[%s, %s])',
      k % 50, k % 47, k % 50 + 2, k % 47 + 2, day_of(k % 40), day_of(k % 40 + 2)))
    WHEN 2 THEN stbox(format('SRID=4326;GEODSTBOX XT(((%s,%s),(%s,%s)),[%s, %s])',
      k % 50, k % 47, k % 50 + 2, k % 47 + 2, day_of(k % 40), day_of(k % 40 + 2)))
    WHEN 3 THEN stbox(format('SRID=3857;STBOX X((%s,%s),(%s,%s))',
      k % 50, k % 47, k % 50 + 2, k % 47 + 2))
    WHEN 4 THEN stbox(format('SRID=3857;STBOX ZT(((%s,%s,%s),(%s,%s,%s)),[%s, %s])',
      k % 50, k % 47, k % 43, k % 50 + 2, k % 47 + 2, k % 43 + 2,
      day_of(k % 40), day_of(k % 40 + 2)))
    WHEN 5 THEN stbox(format('SRID=3857;STBOX Z((%s,%s,%s),(%s,%s,%s))',
      k % 50, k % 47, k % 43, k % 50 + 2, k % 47 + 2, k % 43 + 2))
    ELSE stbox(format('STBOX T([%s, %s])', day_of(k % 40), day_of(k % 40 + 2)))
    END AS b
  FROM generate_series(1, 4000) AS k;

SELECT * FROM index_structures('tbl_index_structures', 'stbox',
  ARRAY['gist(b stbox_rtree_ops)', 'spgist(b stbox_quadtree_ops)',
    'spgist(b stbox_kdtree_ops)'],
  ARRAY[format('SRID=3857;STBOX XT(((10,10),(20,20)),[%s, %s])', day_of(9), day_of(19)),
    format('SRID=5676;STBOX XT(((10,10),(20,20)),[%s, %s])', day_of(9), day_of(19)),
    format('SRID=4326;GEODSTBOX XT(((10,10),(20,20)),[%s, %s])', day_of(9), day_of(19)),
    'SRID=3857;STBOX X((10,10),(20,20))',
    format('SRID=3857;STBOX ZT(((10,10,10),(20,20,20)),[%s, %s])', day_of(9), day_of(19)),
    'SRID=3857;STBOX Z((10,10,10),(20,20,20))',
    format('STBOX T([%s, %s])', day_of(9), day_of(19))],
  ARRAY['&&', '@>', '<@', '~=', '-|-', '<<', '&<', '>>', '&>', '<<|', '&<|',
    '|>>', '|&>', '<</', '&</', '/>>', '/&>', '<<#', '&<#', '#>>', '#&>',
    '|=|']);

DROP TABLE tbl_index_structures;
DROP FUNCTION day_of(int);
DROP FUNCTION index_structures(text, text, text[], text[], text[]);

-------------------------------------------------------------------------------
