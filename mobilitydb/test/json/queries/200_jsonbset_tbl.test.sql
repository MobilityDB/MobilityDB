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
-- documentation for any purjsonb, without fee, and without a written
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

COPY tbl_jsonbset TO '/tmp/tbl_jsonbset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_jsonbset_tmp;
CREATE TABLE tbl_jsonbset_tmp AS TABLE tbl_jsonbset WITH NO DATA;
COPY tbl_jsonbset_tmp FROM '/tmp/tbl_jsonbset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_jsonbset t1, tbl_jsonbset_tmp t2 WHERE t1.k = t2.k AND t1.s <> t2.s;
DROP TABLE tbl_jsonbset_tmp;

-- Input/output from/to WKB and HexWKB

SELECT COUNT(*) FROM tbl_jsonbset WHERE jsonbsetFromHexWKB(asHexWKB(s)) <> s;

-------------------------------------------------------------------------------
-- Constructor

SELECT numValues(set(array_agg(DISTINCT jb ORDER BY jb))) FROM tbl_jsonb WHERE jb IS NOT NULL;

-------------------------------------------------------------------------------
-- Cast

SELECT COUNT(*) FROM tbl_jsonb WHERE jb::jsonbset IS NOT NULL;

SELECT MAX(memSize(s::textset)) FROM tbl_jsonbset;
SELECT COUNT(*) FROM tbl_jsonbset WHERE s = (s::textset)::jsonbset;

-------------------------------------------------------------------------------
-- Transformation functions

SELECT MAX(memSize(set(jb))) FROM tbl_jsonb;

SELECT COUNT(*) FROM tbl_jsonbset WHERE s @> startValue(s);
SELECT COUNT(*) FROM tbl_jsonbset WHERE startValue(s) <@ s;
SELECT COUNT(*) FROM tbl_jsonbset t1, tbl_jsonbset t2 WHERE t1.s && t2.s;
SELECT COUNT(*) FROM tbl_jsonbset t1, tbl_jsonbset t2 WHERE t1.s >> t2.s;
SELECT COUNT(*) FROM tbl_jsonbset t1, tbl_jsonbset t2 WHERE t1.s << t2.s;

SELECT MAX(memSize(s || jb)) FROM tbl_jsonbset, tbl_jsonb;
SELECT MAX(memSize(s - text 'timestamp')) FROM tbl_jsonbset;

SELECT MAX(memSize(s + jb)) FROM tbl_jsonbset, tbl_jsonb;
SELECT MAX(memSize(t1.s + t2.s)) FROM tbl_jsonbset t1, tbl_jsonbset t2;
SELECT MAX(memSize(s - jb)) FROM tbl_jsonbset, tbl_jsonb;
SELECT MAX(memSize(t1.s - t2.s)) FROM tbl_jsonbset t1, tbl_jsonbset t2;
SELECT MAX(memSize(t1.s * t2.s)) FROM tbl_jsonbset t1, tbl_jsonbset t2;

-------------------------------------------------------------------------------
-- Accessor functions

SELECT MAX(memSize(s)) FROM tbl_jsonbset;

SELECT MIN(numValues(s)) FROM tbl_jsonbset;
SELECT MIN(length(startValue(s)::text)) FROM tbl_jsonbset;
SELECT MIN(length(endValue(s)::text)) FROM tbl_jsonbset;
SELECT MIN(length(valueN(s, 1)::text)) FROM tbl_jsonbset;
SELECT MIN(array_length(getValues(s), 1)) FROM tbl_jsonbset;

-------------------------------------------------------------------------------
-- Set_union and unnest functions

SELECT numValues(setUnion(jb)) FROM tbl_jsonb;

WITH test1(k, s) AS (
  SELECT k, unnest(s) FROM tbl_jsonbset ),
test2 (k, s) AS (
  SELECT k, setUnion(s) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_jsonbset t2 WHERE t1.k = t2.k AND t1.s <> t2.s;

-------------------------------------------------------------------------------
-- Comparison functions

SELECT COUNT(*) FROM tbl_jsonbset t1, tbl_jsonbset t2 WHERE set_cmp(t1.s, t2.s) = -1;
SELECT COUNT(*) FROM tbl_jsonbset t1, tbl_jsonbset t2 WHERE t1.s = t2.s;
SELECT COUNT(*) FROM tbl_jsonbset t1, tbl_jsonbset t2 WHERE t1.s <> t2.s;
SELECT COUNT(*) FROM tbl_jsonbset t1, tbl_jsonbset t2 WHERE t1.s < t2.s;
SELECT COUNT(*) FROM tbl_jsonbset t1, tbl_jsonbset t2 WHERE t1.s <= t2.s;
SELECT COUNT(*) FROM tbl_jsonbset t1, tbl_jsonbset t2 WHERE t1.s > t2.s;
SELECT COUNT(*) FROM tbl_jsonbset t1, tbl_jsonbset t2 WHERE t1.s >= t2.s;

SELECT MAX(set_hash(s)) FROM tbl_jsonbset;

-------------------------------------------------------------------------------
-- Aggregation functions

SELECT numValues(setUnion(jb)) FROM tbl_jsonb;

-------------------------------------------------------------------------------
