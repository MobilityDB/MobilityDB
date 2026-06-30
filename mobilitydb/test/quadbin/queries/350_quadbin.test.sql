-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
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

-- Static quadbin SQL type: parser, output, comparison operators, btree +
-- hash opclasses, ASSIGNMENT casts to/from bigint, validity predicates.

-------------------------------------------------------------------------------
-- Input parser: lowercase hex (canonical), optional 0x prefix
--
-- The parser accepts only the hexadecimal encoding of a valid quadbin cell
-- (mode 1). The z0 world cell 480fffffffffffff is the decimal value
-- 5192650370358181887 = quadbinTileToCell(0, 0, 0).
-------------------------------------------------------------------------------

SELECT quadbin '480fffffffffffff';
SELECT quadbin '0x480fffffffffffff';
SELECT quadbin '48427fffffffffff';

-- Round trip via the bigint casts
SELECT (5192650370358181887::bigint::quadbin)::bigint = 5192650370358181887;

/* Errors */
SELECT quadbin '0';
SELECT quadbin 'not-a-hex-cell';
SELECT quadbin '12345';   -- not a valid quadbin cell

-------------------------------------------------------------------------------
-- Output: canonical zero-padded lowercase hex
-------------------------------------------------------------------------------

SELECT quadbin '0x480fffffffffffff' = quadbin '480fffffffffffff';

-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT quadbin '480fffffffffffff' = quadbin '480fffffffffffff';
SELECT quadbin '480fffffffffffff' <> quadbin '48427fffffffffff';
SELECT quadbin '480fffffffffffff' < quadbin '48427fffffffffff';
SELECT quadbin '48427fffffffffff' >= quadbin '480fffffffffffff';

-- The <> can derive from = via NEGATOR
SELECT NOT (quadbin '480fffffffffffff' = quadbin '48427fffffffffff');

-------------------------------------------------------------------------------
-- ASSIGNMENT cast: explicit `::` works
-------------------------------------------------------------------------------

SELECT 5192650370358181887::bigint::quadbin;
SELECT (quadbin '480fffffffffffff')::bigint;

-- The cast is NOT implicit — direct comparison without `::` must error
/* Errors */
SELECT 5192650370358181887::bigint = quadbin '480fffffffffffff';

-------------------------------------------------------------------------------
-- Validity predicates
-------------------------------------------------------------------------------

SELECT isValidIndex(quadbin '480fffffffffffff');
SELECT isValidCell(quadbin '480fffffffffffff');
SELECT isValidCell(quadbin '48427fffffffffff');

-------------------------------------------------------------------------------
-- btree opclass: ORDER BY, DISTINCT, GROUP BY
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_quadbin_test;
CREATE TABLE tbl_quadbin_test(k int, c quadbin);
INSERT INTO tbl_quadbin_test VALUES
  (1, quadbin '480fffffffffffff'),
  (2, quadbin '48427fffffffffff'),
  (3, quadbin '480fffffffffffff'),  -- duplicate of row 1
  (4, quadbin '48a6227affffffff');

-- Sort + distinct exercise the btree opclass
SELECT COUNT(DISTINCT c) FROM tbl_quadbin_test;
-- expect: 3

-- GROUP BY exercises the hash opclass
SELECT (SELECT MAX(cnt) FROM (SELECT COUNT(*) AS cnt FROM tbl_quadbin_test GROUP BY c) sub);
-- expect: 2 (the cell with 2 occurrences)

DROP TABLE tbl_quadbin_test;

-------------------------------------------------------------------------------
