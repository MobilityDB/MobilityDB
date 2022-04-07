-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2022, PostGIS contributors
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

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3D_big_rtree_idx ON tbl_tgeompoint3D_big USING GIST(temp);
CREATE INDEX tbl_tgeogpoint3D_big_rtree_idx ON tbl_tgeogpoint3D_big USING GIST(temp);

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp && geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp @> geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <@ geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp ~= geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp -|- geometry 'Linestring(1 1 1,10 10 10)';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp << geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &< geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp >> geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &> geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<| geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &<| geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |>> geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |&> geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <</ geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &</ geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp />> geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp /&> geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |&> geometry 'Linestring(1 1 1,10 10 10)';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<# period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &<# period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #>> period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #&> period '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp < tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <= tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp > tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp >= tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp && tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp << tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &< tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp >> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<| tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &<| tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |>> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |&> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <</ tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &</ tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp />> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp /&> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |&> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<# tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &<# tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #>> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #&> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp && geography 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp @> geography 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <@ geography 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp ~= geography 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp -|- geography 'Linestring(1 1 1,10 10 10)';

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <<# period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp &<# period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #>> period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #&> period '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp < tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <= tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp > tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp >= tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp && tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp @> tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <@ tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp ~= tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp -|- tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <<# tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp &<# tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #>> tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #&> tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

-- Test the commutator for the selectivity
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]' <<# temp;
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]' &<# temp;

DROP INDEX IF EXISTS tbl_tgeompoint3D_big_rtree_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint3D_big_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint3D_big_quadtree_idx ON tbl_tgeompoint3D_big USING SPGIST(temp);
CREATE INDEX tbl_tgeogpoint3D_big_quadtree_idx ON tbl_tgeogpoint3D_big USING SPGIST(temp);

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp && geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp @> geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <@ geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp ~= geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp -|- geometry 'Linestring(1 1 1,10 10 10)';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp && tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp << geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &< geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp >> geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &> geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<| geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &<| geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |>> geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |&> geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <</ geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &</ geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp />> geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp /&> geometry 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |&> geometry 'Linestring(1 1 1,10 10 10)';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<# period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &<# period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #>> period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #&> period '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp << tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &< tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp >> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<| tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &<| tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |>> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |&> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <</ tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &</ tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp />> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp /&> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp |&> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp <<# tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp &<# tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #>> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeompoint3D_big WHERE temp #&> tgeompoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp && geography 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp @> geography 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <@ geography 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp ~= geography 'Linestring(1 1 1,10 10 10)';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp -|- geography 'Linestring(1 1 1,10 10 10)';

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <<# period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp &<# period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #>> period '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #&> period '[2001-01-01, 2001-02-01]';

SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp && tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp @> tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <@ tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp ~= tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp -|- tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp <<# tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp &<# tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #>> tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';
SELECT COUNT(*) FROM tbl_tgeogpoint3D_big WHERE temp #&> tgeogpoint '[Point(1 1 1)@2000-01-01, Point(10 10 10)@2000-01-02]';

DROP INDEX IF EXISTS tbl_tgeompoint3D_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint3D_big_quadtree_idx;

-------------------------------------------------------------------------------
