-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp && tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp && stbox 'GEODSTBOX ZT(((1,40,1),(10,50,500)),[2001-01-01, 2001-02-01])';
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

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #&> tstzspan '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp && tgeogpoint '[Point(1 1 1)@2001-01-01, Point(10 10 10)@2001-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp && stbox 'GEODSTBOX ZT(((1,40,1),(10,50,500)),[2001-01-01, 2001-02-01])';
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
