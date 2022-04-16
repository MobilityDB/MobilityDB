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

-- Test index support function for ever/always equal and intersects<Time>

CREATE INDEX tbl_tgeompoint_rtree_idx ON tbl_tgeompoint USING gist(temp);
CREATE INDEX tbl_tgeogpoint_rtree_idx ON tbl_tgeogpoint USING gist(temp);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tgeompoint WHERE temp ?= 'Point(1 1)';
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE temp ?= 'Point(1.5 1.5)';

SELECT COUNT(*) FROM tbl_tgeompoint WHERE temp %= 'Point(1 1)';
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE temp %= 'Point(1.5 1.5)';

SELECT COUNT(*) FROM tbl_tgeompoint WHERE intersectsTimestamp(temp, '2001-06-01');
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE intersectsTimestamp(temp, '2001-06-01');

SELECT COUNT(*) FROM tbl_tgeompoint WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');

SELECT COUNT(*) FROM tbl_tgeompoint WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');

SELECT COUNT(*) FROM tbl_tgeompoint WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');

DROP INDEX tbl_tgeompoint_rtree_idx;
DROP INDEX tbl_tgeogpoint_rtree_idx;

-------------------------------------------------------------------------------

-- Test index support function for ever/always equal and intersects<Time>

CREATE INDEX tbl_tgeompoint_quadtree_idx ON tbl_tgeompoint USING spgist(temp);
CREATE INDEX tbl_tgeogpoint_quadtree_idx ON tbl_tgeogpoint USING spgist(temp);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tgeompoint WHERE temp ?= 'Point(1 1)';
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE temp ?= 'Point(1.5 1.5)';

SELECT COUNT(*) FROM tbl_tgeompoint WHERE temp %= 'Point(1 1)';
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE temp %= 'Point(1.5 1.5)';

SELECT COUNT(*) FROM tbl_tgeompoint WHERE intersectsTimestamp(temp, '2001-06-01');
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE intersectsTimestamp(temp, '2001-06-01');

SELECT COUNT(*) FROM tbl_tgeompoint WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');

SELECT COUNT(*) FROM tbl_tgeompoint WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');

SELECT COUNT(*) FROM tbl_tgeompoint WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');
SELECT COUNT(*) FROM tbl_tgeogpoint WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');

DROP INDEX tbl_tgeompoint_quadtree_idx;
DROP INDEX tbl_tgeogpoint_quadtree_idx;

-------------------------------------------------------------------------------
