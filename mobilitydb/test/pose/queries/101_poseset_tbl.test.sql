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

-------------------------------------------------------------------------------
-- Tests for the set data type
-- File set.c
-------------------------------------------------------------------------------

-- Send/receive functions

COPY tbl_poseset2d TO '/tmp/tbl_poseset2d' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_poseset2d_tmp;
CREATE TABLE tbl_poseset2d_tmp AS TABLE tbl_poseset2d WITH NO DATA;
COPY tbl_poseset2d_tmp FROM '/tmp/tbl_poseset2d' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_poseset2d t1, tbl_poseset2d_tmp t2 WHERE t1.k = t2.k AND t1.s <> t2.s;
DROP TABLE tbl_poseset2d_tmp;

-- Input/output from/to WKB and HexWKB

SELECT COUNT(*) FROM tbl_poseset2d WHERE posesetFromHexWKB(asHexWKB(s)) <> s;

-------------------------------------------------------------------------------
-- Constructor

SELECT numValues(set(array_agg(DISTINCT pose ORDER BY pose))) FROM tbl_pose2d WHERE pose IS NOT NULL;

-------------------------------------------------------------------------------
-- Cast

SELECT round(poseset '{"Pose(Point(1 1),0.5)", "Pose(Point(2 2),0.5)", "Pose(Point(3 3),0.5)"}'::stbox, 6);

SELECT COUNT(*) FROM tbl_pose2d WHERE pose::poseset IS NOT NULL;

-------------------------------------------------------------------------------
-- Transformation functions

SELECT MIN(rotation(startValue(round(s, 3)))) FROM tbl_poseset2d;

-------------------------------------------------------------------------------
-- Accessor functions

SELECT MAX(memSize(s)) FROM tbl_poseset2d;

SELECT MIN(numValues(s)) FROM tbl_poseset2d;
SELECT round(MIN(rotation(startValue(s)))::numeric, 6) FROM tbl_poseset2d;
SELECT round(MIN(rotation(endValue(s)))::numeric, 6) FROM tbl_poseset2d;
SELECT round(MIN(rotation(valueN(s, 1)))::numeric, 6) FROM tbl_poseset2d;
SELECT MIN(array_length(getValues(s), 1)) FROM tbl_poseset2d;

-------------------------------------------------------------------------------
-- Set_union and unnest functions

SELECT numValues(setUnion(pose)) FROM tbl_pose2d;

WITH test1(k, s) AS (
  SELECT k, unnest(s) FROM tbl_poseset2d ),
test2 (k, s) AS (
  SELECT k, setUnion(s) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_poseset2d t2 WHERE t1.k = t2.k AND t1.s <> t2.s;

-------------------------------------------------------------------------------
-- Comparison functions

SELECT COUNT(*) FROM tbl_poseset2d t1, tbl_poseset2d t2 WHERE set_cmp(t1.s, t2.s) = -1;
SELECT COUNT(*) FROM tbl_poseset2d t1, tbl_poseset2d t2 WHERE t1.s = t2.s;
SELECT COUNT(*) FROM tbl_poseset2d t1, tbl_poseset2d t2 WHERE t1.s <> t2.s;
SELECT COUNT(*) FROM tbl_poseset2d t1, tbl_poseset2d t2 WHERE t1.s < t2.s;
SELECT COUNT(*) FROM tbl_poseset2d t1, tbl_poseset2d t2 WHERE t1.s <= t2.s;
SELECT COUNT(*) FROM tbl_poseset2d t1, tbl_poseset2d t2 WHERE t1.s > t2.s;
SELECT COUNT(*) FROM tbl_poseset2d t1, tbl_poseset2d t2 WHERE t1.s >= t2.s;

SELECT MAX(set_hash(s)) FROM tbl_poseset2d;

-------------------------------------------------------------------------------
-- Aggregation functions

SELECT numValues(setUnion(pose)) FROM tbl_pose2d;

-------------------------------------------------------------------------------
