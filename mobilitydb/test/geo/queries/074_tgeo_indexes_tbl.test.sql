-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

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

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp < tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <= tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp > tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp >= tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp && tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp @> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <@ tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp ~= tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp -|- tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp << tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &< tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp >> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<| tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) > 0 FROM tbl_tgeometry3D_big WHERE temp &<| tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |>> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |&> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <</ tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &</ tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp />> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp /&> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |&> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<# tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &<# tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #>> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #&> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp < tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <= tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp > tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp >= tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp && tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp @> tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <@ tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp ~= tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp -|- tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <<# tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp &<# tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #>> tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #&> tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

-- Test the commutator for the selectivity
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]' <<# temp;
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]' &<# temp;

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeometry3D_big_rtree_idx;
DROP INDEX IF EXISTS tbl_tgeography3D_big_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeometry3D_big_quadtree_idx ON tbl_tgeometry3D_big USING SPGIST(temp);
CREATE INDEX tbl_tgeography3D_big_quadtree_idx ON tbl_tgeography3D_big USING SPGIST(temp);

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp && tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp @> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <@ tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp ~= tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp -|- tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp << tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &< tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp >> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<| tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &<| tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |>> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |&> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <</ tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &</ tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp />> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp /&> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |&> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<# tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &<# tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #>> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #&> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp && tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp @> tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <@ tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp ~= tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp -|- tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <<# tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp &<# tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #>> tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #&> tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeometry3D_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_tgeography3D_big_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeometry3D_big_kdtree_idx ON tbl_tgeometry3D_big USING SPGIST(temp tgeometry_kdtree_ops);
CREATE INDEX tbl_tgeography3D_big_kdtree_idx ON tbl_tgeography3D_big USING SPGIST(temp tgeography_kdtree_ops);

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp && tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp @> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <@ tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp ~= tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp -|- tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp << tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &< tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp >> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<| tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &<| tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |>> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |&> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <</ tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &</ tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp />> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp /&> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp |&> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp <<# tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp &<# tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #>> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeometry3D_big WHERE temp #&> tgeometry 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp && tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp @> tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <@ tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp ~= tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp -|- tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp <<# tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp &<# tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #>> tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeography3D_big WHERE temp #&> tgeography 'SRID=7844;[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeometry3D_big_kdtree_idx;
DROP INDEX IF EXISTS tbl_tgeography3D_big_kdtree_idx;

-------------------------------------------------------------------------------

ANALYZE tbl_tgeometry;
ANALYZE tbl_tgeometry3D;

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

