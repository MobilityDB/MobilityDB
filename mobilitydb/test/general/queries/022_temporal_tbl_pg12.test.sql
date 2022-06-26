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

-- RESTRICTION SELECTIVITY
-- Test index support function

CREATE INDEX tbl_tbool_big_rtree_idx ON tbl_tbool_big USING gist(temp);
CREATE INDEX tbl_tint_big_rtree_idx ON tbl_tint_big USING gist(temp);
CREATE INDEX tbl_tfloat_big_rtree_idx ON tbl_tfloat_big USING gist(temp);
CREATE INDEX tbl_ttext_rtree_idx ON tbl_ttext_big USING gist(temp);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tint_big WHERE temp ?= 1;
SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp ?= 1.5;
SELECT COUNT(*) FROM tbl_ttext_big WHERE temp ?= 'AAA';

SELECT COUNT(*) FROM tbl_tint_big WHERE temp %= 1;
SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp %= 1.5;
SELECT COUNT(*) FROM tbl_ttext_big WHERE temp %= 'AAA';

SELECT COUNT(*) FROM tbl_tbool_big WHERE intersectsTimestamp(temp, '2001-06-01');
SELECT COUNT(*) FROM tbl_tint_big WHERE intersectsTimestamp(temp, '2001-06-01');
SELECT COUNT(*) FROM tbl_tfloat_big WHERE intersectsTimestamp(temp, '2001-06-01');
SELECT COUNT(*) FROM tbl_ttext_big WHERE intersectsTimestamp(temp, '2001-06-01');

SELECT COUNT(*) FROM tbl_tbool_big WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');
SELECT COUNT(*) FROM tbl_tint_big WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');
SELECT COUNT(*) FROM tbl_tfloat_big WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');
SELECT COUNT(*) FROM tbl_ttext_big WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');

SELECT COUNT(*) FROM tbl_tbool_big WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');
SELECT COUNT(*) FROM tbl_tint_big WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');
SELECT COUNT(*) FROM tbl_tfloat_big WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');
SELECT COUNT(*) FROM tbl_ttext_big WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');

SELECT COUNT(*) FROM tbl_tbool_big WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');
SELECT COUNT(*) FROM tbl_tint_big WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');
SELECT COUNT(*) FROM tbl_tfloat_big WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');
SELECT COUNT(*) FROM tbl_ttext_big WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');

DROP INDEX tbl_tbool_big_rtree_idx;
DROP INDEX tbl_tint_big_rtree_idx;
DROP INDEX tbl_tfloat_big_rtree_idx;
DROP INDEX tbl_ttext_rtree_idx;

-------------------------------------------------------------------------------

-- RESTRICTION SELECTIVITY
-- Test index support function

CREATE INDEX tbl_tbool_big_quadtree_idx ON tbl_tbool_big USING spgist(temp);
CREATE INDEX tbl_tint_big_quadtree_idx ON tbl_tint_big USING spgist(temp);
CREATE INDEX tbl_tfloat_big_quadtree_idx ON tbl_tfloat_big USING spgist(temp);
CREATE INDEX tbl_ttext_quadtree_idx ON tbl_ttext_big USING spgist(temp);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tint_big WHERE temp ?= 1;
SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp ?= 1.5;
SELECT COUNT(*) FROM tbl_tint_big WHERE temp %= 1;
SELECT COUNT(*) FROM tbl_tfloat_big WHERE temp %= 1.5;

SELECT COUNT(*) FROM tbl_tbool_big WHERE intersectsTimestamp(temp, '2001-06-01');
SELECT COUNT(*) FROM tbl_tint_big WHERE intersectsTimestamp(temp, '2001-06-01');
SELECT COUNT(*) FROM tbl_tfloat_big WHERE intersectsTimestamp(temp, '2001-06-01');
SELECT COUNT(*) FROM tbl_ttext_big WHERE intersectsTimestamp(temp, '2001-06-01');

SELECT COUNT(*) FROM tbl_tbool_big WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');
SELECT COUNT(*) FROM tbl_tint_big WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');
SELECT COUNT(*) FROM tbl_tfloat_big WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');
SELECT COUNT(*) FROM tbl_ttext_big WHERE intersectsTimestampSet(temp, '{2001-06-01, 2001-07-01}');

SELECT COUNT(*) FROM tbl_tbool_big WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');
SELECT COUNT(*) FROM tbl_tint_big WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');
SELECT COUNT(*) FROM tbl_tfloat_big WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');
SELECT COUNT(*) FROM tbl_ttext_big WHERE intersectsPeriod(temp, '[2001-06-01, 2001-07-01]');

SELECT COUNT(*) FROM tbl_tbool_big WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');
SELECT COUNT(*) FROM tbl_tint_big WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');
SELECT COUNT(*) FROM tbl_tfloat_big WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');
SELECT COUNT(*) FROM tbl_ttext_big WHERE intersectsPeriodSet(temp, '{[2001-06-01, 2001-07-01]}');

DROP INDEX tbl_tbool_big_quadtree_idx;
DROP INDEX tbl_tint_big_quadtree_idx;
DROP INDEX tbl_tfloat_big_quadtree_idx;
DROP INDEX tbl_ttext_quadtree_idx;

-------------------------------------------------------------------------------
