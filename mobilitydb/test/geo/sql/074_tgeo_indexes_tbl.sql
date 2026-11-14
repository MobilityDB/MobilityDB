CREATE TABLE test AS
SELECT NULL::tgeometry FROM generate_series(1, 10);
ANALYZE test;
DROP TABLE test;

-------------------------------------------------------------------------------

ANALYZE tbl_tgeometry3D_big;
ANALYZE tbl_tgeography3D_big;

DROP INDEX IF EXISTS tbl_tgeometry3D_big_rtree_idx;
DROP INDEX IF EXISTS tbl_tgeography3D_big_rtree_idx;

DROP INDEX IF EXISTS tbl_tgeometry3D_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_tgeography3D_big_quadtree_idx;

DROP INDEX IF EXISTS tbl_tgeometry3D_big_kdtree_idx;
DROP INDEX IF EXISTS tbl_tgeography3D_big_kdtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeometry3D_big_rtree_idx ON tbl_tgeometry3D_big USING GIST(temp);
CREATE INDEX tbl_tgeography3D_big_rtree_idx ON tbl_tgeography3D_big USING GIST(temp);

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp < tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <= tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp > tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp >= tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp && tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp @> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <@ tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp ~= tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp -|- tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp << tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &< tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp >> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<| tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) > 0 FROM tbl_tgeometry3D_big WHERE temp &<| tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |>> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |&> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <</ tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &</ tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp />> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp /&> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |&> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<# tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &<# tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #>> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #&> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxLeft(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverleft(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxRight(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverright(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxBelow(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) > 0 FROM tbl_tgeometry3D_big WHERE stboxOverbelow(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxAbove(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverabove(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxFront(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverfront(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxBack(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverback(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxBefore(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverbefore(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxAfter(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverafter(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');

SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp < tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <= tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp > tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp >= tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp && tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp && stbox 'SRID=7844;GEODSTBOX ZT(((1,1,1),(10,10,10)),[2001-01-01, 2001-02-01])';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp @> tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <@ tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp ~= tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp -|- tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <<# tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp &<# tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #>> tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #&> tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

-- Test the commutator for the selectivity
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]' <<# temp;
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]' &<# temp;

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeometry3D_big_rtree_idx;
DROP INDEX IF EXISTS tbl_tgeography3D_big_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeometry3D_big_quadtree_idx ON tbl_tgeometry3D_big USING SPGIST(temp);
CREATE INDEX tbl_tgeography3D_big_quadtree_idx ON tbl_tgeography3D_big USING SPGIST(temp);

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp && tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp @> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <@ tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp ~= tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp -|- tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp << tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &< tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp >> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<| tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &<| tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |>> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |&> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <</ tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &</ tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp />> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp /&> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |&> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<# tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &<# tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #>> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #&> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxLeft(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverleft(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxRight(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverright(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxBelow(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverbelow(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxAbove(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverabove(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxFront(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverfront(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxBack(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverback(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxBefore(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverbefore(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxAfter(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverafter(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');

SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp && tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp && stbox 'SRID=7844;GEODSTBOX ZT(((1,1,1),(10,10,10)),[2001-01-01, 2001-02-01])';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp @> tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <@ tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp ~= tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp -|- tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <<# tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp &<# tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #>> tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #&> tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeometry3D_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_tgeography3D_big_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeometry3D_big_kdtree_idx ON tbl_tgeometry3D_big USING SPGIST(temp tgeometry_kdtree_ops);
CREATE INDEX tbl_tgeography3D_big_kdtree_idx ON tbl_tgeography3D_big USING SPGIST(temp tgeography_kdtree_ops);

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp && tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp @> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <@ tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp ~= tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp -|- tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp << tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &< tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp >> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<| tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &<| tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |>> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |&> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <</ tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &</ tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp />> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp /&> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |&> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<# tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &<# tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #>> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #&> tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxLeft(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverleft(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxRight(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverright(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxBelow(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverbelow(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxAbove(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverabove(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxFront(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverfront(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxBack(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverback(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxBefore(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverbefore(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxAfter(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE stboxOverafter(temp, tgeometry 'SRID=3812;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]');

SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp && tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp && stbox 'SRID=7844;GEODSTBOX ZT(((1,1,1),(10,10,10)),[2001-01-01, 2001-02-01])';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp @> tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <@ tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp ~= tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp -|- tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <<# tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp &<# tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #>> tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #&> tgeography 'SRID=7844;[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeometry3D_big_kdtree_idx;
DROP INDEX IF EXISTS tbl_tgeography3D_big_kdtree_idx;

-------------------------------------------------------------------------------

ANALYZE tbl_tgeometry;
ANALYZE tbl_tgeometry3D;

SELECT COUNT(*) FROM tbl_tgeometry WHERE temp ?&& stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tgeometry WHERE stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])' ?&& temp;
SELECT COUNT(*) FROM tbl_tgeometry WHERE temp %&& stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tgeometry WHERE stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])' %&& temp;

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeometry_rtree_idx;
DROP INDEX IF EXISTS tbl_tgeometry3D_rtree_idx;
CREATE INDEX tbl_tgeometry_rtree_idx ON tbl_tgeometry USING GIST(temp);
CREATE INDEX tbl_tgeometry3D_rtree_idx ON tbl_tgeometry3D USING GIST(temp);

-- EXPLAIN ANALYZE
WITH test AS (
  SELECT temp |=| tgeometry 'SRID=3812;[Point(1 1)@2001-06-01, Point(2 2)@2001-07-01]' AS distance FROM tbl_tgeometry ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| tgeometry 'SRID=3812;[Point(-1 -1 -1)@2001-06-01, Point(-2 -2 -2)@2001-07-01]' AS distance FROM tbl_tgeometry3D ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

WITH test AS (
  SELECT temp |=| geometry 'SRID=3812;Point(1 1)' AS distance FROM tbl_tgeometry ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| stbox 'SRID=3812;STBOX XT(((1,1),(2,2)),[2001-06-01, 2001-07-01])' AS distance FROM tbl_tgeometry ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

WITH test AS (
  SELECT temp |=| tgeography 'SRID=7844;[Point(1 1)@2001-06-01, Point(2 2)@2001-07-01]' AS distance FROM tbl_tgeography ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| geography 'SRID=7844;Point(1 1)' AS distance FROM tbl_tgeography ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

SELECT COUNT(*) FROM tbl_tgeometry WHERE temp ?&& stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tgeometry WHERE stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])' ?&& temp;
SELECT COUNT(*) FROM tbl_tgeometry WHERE temp %&& stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tgeometry WHERE stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])' %&& temp;
DROP INDEX tbl_tgeometry_rtree_idx;
DROP INDEX tbl_tgeometry3D_rtree_idx;

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeometry_quadtree_idx;
DROP INDEX IF EXISTS tbl_tgeometry3D_quadtree_idx;
CREATE INDEX tbl_tgeometry_quadtree_idx ON tbl_tgeometry USING SPGIST(temp);
CREATE INDEX tbl_tgeometry3D_quadtree_idx ON tbl_tgeometry3D USING SPGIST(temp);

-- EXPLAIN ANALYZE
WITH test AS (
  SELECT temp |=| tgeometry 'SRID=3812;[Point(1 1)@2001-06-01, Point(2 2)@2001-07-01]' AS distance FROM tbl_tgeometry ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| tgeometry 'SRID=3812;[Point(-1 -1 -1)@2001-06-01, Point(-2 -2 -2)@2001-07-01]' AS distance FROM tbl_tgeometry3D ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

WITH test AS (
  SELECT temp |=| geometry 'SRID=3812;Point(1 1)' AS distance FROM tbl_tgeometry ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| stbox 'SRID=3812;STBOX XT(((1,1),(2,2)),[2001-06-01, 2001-07-01])' AS distance FROM tbl_tgeometry ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

WITH test AS (
  SELECT temp |=| tgeography 'SRID=7844;[Point(1 1)@2001-06-01, Point(2 2)@2001-07-01]' AS distance FROM tbl_tgeography ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;
WITH test AS (
  SELECT temp |=| geography 'SRID=7844;Point(1 1)' AS distance FROM tbl_tgeography ORDER BY 1 LIMIT 3 )
SELECT round(distance, 6) FROM test;

SELECT COUNT(*) FROM tbl_tgeometry WHERE temp ?&& stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tgeometry WHERE stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])' ?&& temp;
SELECT COUNT(*) FROM tbl_tgeometry WHERE temp %&& stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])';
SELECT COUNT(*) FROM tbl_tgeometry WHERE stbox 'SRID=3812;STBOX XT(((0,0),(50,50)),[2001-01-01, 2001-12-31])' %&& temp;
DROP INDEX tbl_tgeometry_quadtree_idx;
DROP INDEX tbl_tgeometry3D_quadtree_idx;

-------------------------------------------------------------------------------
-- Coverage of all the same and order by logic in SP-GiST indexes

CREATE TABLE tbl_tgeometry3D_big_allthesame AS SELECT k, tgeometry(geometry 'Point(5 5 5)', t) AS temp FROM tbl_tstzspan_big;
CREATE INDEX tbl_tgeometry3D_big_allthesame_quadtree_idx ON tbl_tgeometry3D_big_allthesame USING SPGIST(temp);
ANALYZE tbl_tgeometry3D_big_allthesame;

-- EXPLAIN ANALYZE

DROP TABLE tbl_tgeometry3D_big_allthesame;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- A column of stbox values indexed by GiST, quad-tree and k-d tree answers the
-- queries of a tgeometry as the scan does: <<, &<, &&, &>, >>, ~=, @>, <@, &<|, <<|, |>>, |&>, -|-, &<#, <<#, #>>, #&>, &</, <</, />>, /&>, |=|
-------------------------------------------------------------------------------

CREATE TABLE tbl_stbox_tgeometry AS
SELECT k, temp::stbox AS b FROM tbl_tgeometry3d WHERE temp IS NOT NULL;
CREATE TABLE tbl_tgeometry_q AS
SELECT temp AS q FROM tbl_tgeometry3d WHERE temp IS NOT NULL ORDER BY k LIMIT 3;
CREATE FUNCTION stbox_tgeometry_answers() RETURNS text AS $$
  SELECT string_agg(coalesce(c::text, 'null'), ',' ORDER BY o) FROM (
    SELECT 1, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b << q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 2, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b &< q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 3, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b && q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 4, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b &> q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 5, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b >> q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 6, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b ~= q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 7, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b @> q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 8, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b <@ q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 9, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b &<| q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 10, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b <<| q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 11, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b |>> q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 12, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b |&> q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 13, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b -|- q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 14, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b &<# q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 15, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b <<# q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 16, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b #>> q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 17, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b #&> q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 18, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b &</ q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 19, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b <</ q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 20, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b />> q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 21, (SELECT count(*) FROM tbl_stbox_tgeometry WHERE b /&> q) FROM tbl_tgeometry_q
    UNION ALL
    SELECT 22, (SELECT sum(k) FROM (SELECT k FROM tbl_stbox_tgeometry
      ORDER BY b |=| q, k LIMIT 5) s) FROM tbl_tgeometry_q) t(o, c);
$$ LANGUAGE SQL;
CREATE FUNCTION stbox_tgeometry_probes_index(query text)
RETURNS boolean LANGUAGE plpgsql AS $$
DECLARE
  line text;
BEGIN
  FOR line IN EXECUTE 'EXPLAIN (COSTS OFF) ' || query LOOP
    IF line LIKE '%Index Cond%' OR line LIKE '%Order By%' THEN
      RETURN true;
    END IF;
  END LOOP;
  RETURN false;
END;
$$;
CREATE FUNCTION stbox_tgeometry_indexed() RETURNS text AS $$
  SELECT string_agg(CASE WHEN stbox_tgeometry_probes_index(query) THEN 't' ELSE 'f' END, '' ORDER BY o)
  FROM (VALUES
    (1, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b << (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (2, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b &< (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (3, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b && (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (4, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b &> (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (5, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b >> (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (6, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b ~= (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (7, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b @> (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (8, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b <@ (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (9, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b &<| (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (10, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b <<| (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (11, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b |>> (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (12, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b |&> (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (13, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b -|- (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (14, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b &<# (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (15, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b <<# (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (16, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b #>> (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (17, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b #&> (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (18, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b &</ (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (19, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b <</ (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (20, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b />> (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (21, 'SELECT count(*) FROM tbl_stbox_tgeometry WHERE b /&> (SELECT q FROM tbl_tgeometry_q LIMIT 1)'),
    (22, 'SELECT k FROM tbl_stbox_tgeometry ORDER BY b |=| (SELECT q FROM tbl_tgeometry_q LIMIT 1) LIMIT 5')) t(o, query);
$$ LANGUAGE SQL;
CREATE TABLE tbl_stbox_tgeometry_seq AS SELECT stbox_tgeometry_answers() AS a;
SELECT a FROM tbl_stbox_tgeometry_seq;
SET enable_seqscan = off;
CREATE INDEX tbl_stbox_tgeometry_gist_idx ON tbl_stbox_tgeometry USING gist(b);
SELECT stbox_tgeometry_indexed(), stbox_tgeometry_answers() = a
FROM tbl_stbox_tgeometry_seq;
DROP INDEX tbl_stbox_tgeometry_gist_idx;
CREATE INDEX tbl_stbox_tgeometry_quad_idx ON tbl_stbox_tgeometry USING spgist(b);
SELECT stbox_tgeometry_indexed(), stbox_tgeometry_answers() = a
FROM tbl_stbox_tgeometry_seq;
DROP INDEX tbl_stbox_tgeometry_quad_idx;
CREATE INDEX tbl_stbox_tgeometry_kd_idx ON tbl_stbox_tgeometry
  USING spgist(b stbox_kdtree_ops);
SELECT stbox_tgeometry_indexed(), stbox_tgeometry_answers() = a
FROM tbl_stbox_tgeometry_seq;
RESET enable_seqscan;
DROP TABLE tbl_stbox_tgeometry, tbl_tgeometry_q, tbl_stbox_tgeometry_seq;
DROP FUNCTION stbox_tgeometry_indexed(), stbox_tgeometry_answers(), stbox_tgeometry_probes_index(text);

-------------------------------------------------------------------------------
-- A column of stbox values indexed by GiST, quad-tree and k-d tree answers the
-- queries of a tgeography as the scan does: &&, ~=, @>, <@, -|-, &<#, <<#, #>>, #&>, |=|
-------------------------------------------------------------------------------

CREATE TABLE tbl_stbox_tgeography AS
SELECT k, temp::stbox AS b FROM tbl_tgeography3d WHERE temp IS NOT NULL;
CREATE TABLE tbl_tgeography_q AS
SELECT temp AS q FROM tbl_tgeography3d WHERE temp IS NOT NULL ORDER BY k LIMIT 3;
CREATE FUNCTION stbox_tgeography_answers() RETURNS text AS $$
  SELECT string_agg(coalesce(c::text, 'null'), ',' ORDER BY o) FROM (
    SELECT 1, (SELECT count(*) FROM tbl_stbox_tgeography WHERE b && q) FROM tbl_tgeography_q
    UNION ALL
    SELECT 2, (SELECT count(*) FROM tbl_stbox_tgeography WHERE b ~= q) FROM tbl_tgeography_q
    UNION ALL
    SELECT 3, (SELECT count(*) FROM tbl_stbox_tgeography WHERE b @> q) FROM tbl_tgeography_q
    UNION ALL
    SELECT 4, (SELECT count(*) FROM tbl_stbox_tgeography WHERE b <@ q) FROM tbl_tgeography_q
    UNION ALL
    SELECT 5, (SELECT count(*) FROM tbl_stbox_tgeography WHERE b -|- q) FROM tbl_tgeography_q
    UNION ALL
    SELECT 6, (SELECT count(*) FROM tbl_stbox_tgeography WHERE b &<# q) FROM tbl_tgeography_q
    UNION ALL
    SELECT 7, (SELECT count(*) FROM tbl_stbox_tgeography WHERE b <<# q) FROM tbl_tgeography_q
    UNION ALL
    SELECT 8, (SELECT count(*) FROM tbl_stbox_tgeography WHERE b #>> q) FROM tbl_tgeography_q
    UNION ALL
    SELECT 9, (SELECT count(*) FROM tbl_stbox_tgeography WHERE b #&> q) FROM tbl_tgeography_q
    UNION ALL
    SELECT 10, (SELECT sum(k) FROM (SELECT k FROM tbl_stbox_tgeography
      ORDER BY b |=| q, k LIMIT 5) s) FROM tbl_tgeography_q) t(o, c);
$$ LANGUAGE SQL;
CREATE FUNCTION stbox_tgeography_probes_index(query text)
RETURNS boolean LANGUAGE plpgsql AS $$
DECLARE
  line text;
BEGIN
  FOR line IN EXECUTE 'EXPLAIN (COSTS OFF) ' || query LOOP
    IF line LIKE '%Index Cond%' OR line LIKE '%Order By%' THEN
      RETURN true;
    END IF;
  END LOOP;
  RETURN false;
END;
$$;
CREATE FUNCTION stbox_tgeography_indexed() RETURNS text AS $$
  SELECT string_agg(CASE WHEN stbox_tgeography_probes_index(query) THEN 't' ELSE 'f' END, '' ORDER BY o)
  FROM (VALUES
    (1, 'SELECT count(*) FROM tbl_stbox_tgeography WHERE b && (SELECT q FROM tbl_tgeography_q LIMIT 1)'),
    (2, 'SELECT count(*) FROM tbl_stbox_tgeography WHERE b ~= (SELECT q FROM tbl_tgeography_q LIMIT 1)'),
    (3, 'SELECT count(*) FROM tbl_stbox_tgeography WHERE b @> (SELECT q FROM tbl_tgeography_q LIMIT 1)'),
    (4, 'SELECT count(*) FROM tbl_stbox_tgeography WHERE b <@ (SELECT q FROM tbl_tgeography_q LIMIT 1)'),
    (5, 'SELECT count(*) FROM tbl_stbox_tgeography WHERE b -|- (SELECT q FROM tbl_tgeography_q LIMIT 1)'),
    (6, 'SELECT count(*) FROM tbl_stbox_tgeography WHERE b &<# (SELECT q FROM tbl_tgeography_q LIMIT 1)'),
    (7, 'SELECT count(*) FROM tbl_stbox_tgeography WHERE b <<# (SELECT q FROM tbl_tgeography_q LIMIT 1)'),
    (8, 'SELECT count(*) FROM tbl_stbox_tgeography WHERE b #>> (SELECT q FROM tbl_tgeography_q LIMIT 1)'),
    (9, 'SELECT count(*) FROM tbl_stbox_tgeography WHERE b #&> (SELECT q FROM tbl_tgeography_q LIMIT 1)'),
    (10, 'SELECT k FROM tbl_stbox_tgeography ORDER BY b |=| (SELECT q FROM tbl_tgeography_q LIMIT 1) LIMIT 5')) t(o, query);
$$ LANGUAGE SQL;
CREATE TABLE tbl_stbox_tgeography_seq AS SELECT stbox_tgeography_answers() AS a;
SELECT a FROM tbl_stbox_tgeography_seq;
SET enable_seqscan = off;
CREATE INDEX tbl_stbox_tgeography_gist_idx ON tbl_stbox_tgeography USING gist(b);
SELECT stbox_tgeography_indexed(), stbox_tgeography_answers() = a
FROM tbl_stbox_tgeography_seq;
DROP INDEX tbl_stbox_tgeography_gist_idx;
CREATE INDEX tbl_stbox_tgeography_quad_idx ON tbl_stbox_tgeography USING spgist(b);
SELECT stbox_tgeography_indexed(), stbox_tgeography_answers() = a
FROM tbl_stbox_tgeography_seq;
DROP INDEX tbl_stbox_tgeography_quad_idx;
CREATE INDEX tbl_stbox_tgeography_kd_idx ON tbl_stbox_tgeography
  USING spgist(b stbox_kdtree_ops);
SELECT stbox_tgeography_indexed(), stbox_tgeography_answers() = a
FROM tbl_stbox_tgeography_seq;
RESET enable_seqscan;
DROP TABLE tbl_stbox_tgeography, tbl_tgeography_q, tbl_stbox_tgeography_seq;
DROP FUNCTION stbox_tgeography_indexed(), stbox_tgeography_answers(), stbox_tgeography_probes_index(text);
