-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2023, PostGIS contributors
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

COPY tbl_npointset TO '/tmp/tbl_npointset' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_npointset_tmp;
CREATE TABLE tbl_npointset_tmp AS TABLE tbl_npointset WITH NO DATA;
COPY tbl_npointset_tmp FROM '/tmp/tbl_npointset' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_npointset t1, tbl_npointset_tmp t2 WHERE t1.k = t2.k AND t1.n <> t2.n;
DROP TABLE tbl_npointset_tmp;

-- Input/output from/to WKB and HexWKB

SELECT COUNT(*) FROM tbl_npointset WHERE npointsetFromHexWKB(asHexWKB(n)) <> n;

-------------------------------------------------------------------------------
-- Constructor

SELECT numValues(set(array_agg(DISTINCT np ORDER BY np))) FROM tbl_npoint WHERE np IS NOT NULL;

-------------------------------------------------------------------------------
-- Cast

SELECT round(npointset '{"Npoint(1,0.5)", "Npoint(2,0.5)", "Npoint(3,0.5)"}'::stbox, 6);

SELECT COUNT(*) FROM tbl_npoint WHERE np::npointset IS NOT NULL;

-------------------------------------------------------------------------------
-- Transformation functions

SELECT MIN(getPosition(startValue(round(n, 3)))) FROM tbl_npointset;

-------------------------------------------------------------------------------
-- Accessor functions

SELECT MAX(memSize(n)) FROM tbl_npointset;

SELECT MIN(numValues(n)) FROM tbl_npointset;
SELECT MIN(getPosition(startValue(n))) FROM tbl_npointset;
SELECT MIN(getPosition(endValue(n))) FROM tbl_npointset;
SELECT MIN(getPosition(valueN(n, 1))) FROM tbl_npointset;
SELECT MIN(array_length(getValues(n), 1)) FROM tbl_npointset;

-------------------------------------------------------------------------------
-- Set_union and unnest functions

SELECT numValues(set_union(np)) FROM tbl_npoint;

WITH test1(k, n) AS (
  SELECT k, unnest(n) FROM tbl_npointset ),
test2 (k, n) AS (
  SELECT k, set_union(n) FROM test1 GROUP BY k )
SELECT COUNT(*) FROM test2 t1, tbl_npointset t2 WHERE t1.k = t2.k AND t1.n <> t2.n;

-------------------------------------------------------------------------------
-- Comparison functions

SELECT COUNT(*) FROM tbl_npointset t1, tbl_npointset t2 WHERE set_cmp(t1.n, t2.n) = -1;
SELECT COUNT(*) FROM tbl_npointset t1, tbl_npointset t2 WHERE t1.n = t2.n;
SELECT COUNT(*) FROM tbl_npointset t1, tbl_npointset t2 WHERE t1.n <> t2.n;
SELECT COUNT(*) FROM tbl_npointset t1, tbl_npointset t2 WHERE t1.n < t2.n;
SELECT COUNT(*) FROM tbl_npointset t1, tbl_npointset t2 WHERE t1.n <= t2.n;
SELECT COUNT(*) FROM tbl_npointset t1, tbl_npointset t2 WHERE t1.n > t2.n;
SELECT COUNT(*) FROM tbl_npointset t1, tbl_npointset t2 WHERE t1.n >= t2.n;

SELECT MAX(set_hash(n)) FROM tbl_npointset;

SELECT MAX(set_hash_extended(n, 1)) FROM tbl_npointset;

-------------------------------------------------------------------------------
-- Aggregation functions

SELECT numValues(set_union(np)) FROM tbl_npoint;

-------------------------------------------------------------------------------
