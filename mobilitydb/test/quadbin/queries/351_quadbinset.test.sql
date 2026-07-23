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

-- quadbinset type: parser/output, WKB helpers, constructors, accessors,
-- comparison operators, btree/hash opclasses, unnest, setUnion aggregate.

-------------------------------------------------------------------------------
-- Input/output
-------------------------------------------------------------------------------

SELECT quadbinset '{480fffffffffffff, 48427fffffffffff}';
SELECT quadbinset '{48427fffffffffff, 480fffffffffffff}';  -- normalized + sorted

-- Singleton
SELECT quadbinset '{480fffffffffffff}';

/* Errors */
SELECT quadbinset '{480fffffffffffff, not-a-cell}';
SELECT quadbinset '{480fffffffffffff';

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT set(ARRAY[quadbin '480fffffffffffff', quadbin '48427fffffffffff']);
SELECT set(quadbin '480fffffffffffff');
SELECT (quadbin '480fffffffffffff')::quadbinset;

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT numValues(quadbinset '{480fffffffffffff, 48427fffffffffff, 48a6227affffffff}');
SELECT startValue(quadbinset '{480fffffffffffff, 48427fffffffffff}');
SELECT endValue(quadbinset '{480fffffffffffff, 48427fffffffffff}');
SELECT valueN(quadbinset '{480fffffffffffff, 48427fffffffffff}', 1);
SELECT valueN(quadbinset '{480fffffffffffff, 48427fffffffffff}', 2);
SELECT getValues(quadbinset '{480fffffffffffff, 48427fffffffffff}');
SELECT memSize(quadbinset '{480fffffffffffff, 48427fffffffffff}') > 0;

-------------------------------------------------------------------------------
-- WKB / HexWKB round-trip
-------------------------------------------------------------------------------

SELECT quadbinsetFromBinary(asBinary(quadbinset '{480fffffffffffff, 48427fffffffffff}'))
  = quadbinset '{480fffffffffffff, 48427fffffffffff}';
SELECT quadbinsetFromHexWKB(asHexWKB(quadbinset '{480fffffffffffff, 48427fffffffffff}'))
  = quadbinset '{480fffffffffffff, 48427fffffffffff}';
SELECT asText(quadbinset '{480fffffffffffff, 48427fffffffffff}');

-------------------------------------------------------------------------------
-- Comparison operators + opclasses
-------------------------------------------------------------------------------

SELECT quadbinset '{480fffffffffffff}' = quadbinset '{480fffffffffffff}';
SELECT quadbinset '{480fffffffffffff}' <> quadbinset '{48427fffffffffff}';
SELECT quadbinset '{480fffffffffffff}' < quadbinset '{48427fffffffffff}';

DROP TABLE IF EXISTS tbl_quadbinset_test;
CREATE TABLE tbl_quadbinset_test(k int, s quadbinset);
INSERT INTO tbl_quadbinset_test VALUES
  (1, quadbinset '{480fffffffffffff}'),
  (2, quadbinset '{48427fffffffffff}'),
  (3, quadbinset '{480fffffffffffff}');
SELECT COUNT(DISTINCT s) FROM tbl_quadbinset_test;
DROP TABLE tbl_quadbinset_test;

-------------------------------------------------------------------------------
-- unnest
-------------------------------------------------------------------------------

SELECT unnest(quadbinset '{480fffffffffffff, 48427fffffffffff}') ORDER BY 1;

-------------------------------------------------------------------------------
-- setUnion aggregate (scalar -> set, set -> set)
-------------------------------------------------------------------------------

SELECT setUnion(c) FROM (VALUES
  (quadbin '480fffffffffffff'),
  (quadbin '48427fffffffffff'),
  (quadbin '480fffffffffffff')) t(c);

SELECT setUnion(s) FROM (VALUES
  (quadbinset '{480fffffffffffff}'),
  (quadbinset '{48427fffffffffff, 48a6227affffffff}')) t(s);

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Set operations: union (+), difference (-), intersection (*)
-------------------------------------------------------------------------------

SELECT setUnion(quadbinset '{480fffffffffffff, 48427fffffffffff}', quadbinset '{48427fffffffffff, 48a6227affffffff}');
SELECT setUnion(quadbinset '{480fffffffffffff}', quadbin '48427fffffffffff');
SELECT setUnion(quadbin '480fffffffffffff', quadbinset '{48427fffffffffff}');
SELECT quadbinset '{480fffffffffffff, 48427fffffffffff}' + quadbinset '{48a6227affffffff}';

SELECT setMinus(quadbinset '{480fffffffffffff, 48427fffffffffff}', quadbinset '{48427fffffffffff}');
SELECT setMinus(quadbinset '{480fffffffffffff, 48427fffffffffff}', quadbin '480fffffffffffff');
SELECT quadbinset '{480fffffffffffff, 48427fffffffffff}' - quadbinset '{48427fffffffffff}';

SELECT setIntersection(quadbinset '{480fffffffffffff, 48427fffffffffff}', quadbinset '{48427fffffffffff, 48a6227affffffff}');
SELECT setIntersection(quadbinset '{480fffffffffffff, 48427fffffffffff}', quadbin '48427fffffffffff');
SELECT quadbinset '{480fffffffffffff, 48427fffffffffff}' * quadbinset '{48427fffffffffff}';

-------------------------------------------------------------------------------
-- Set topological operators: contains (@>), contained (<@), overlaps (&&)
-------------------------------------------------------------------------------

SELECT contains(quadbinset '{480fffffffffffff, 48427fffffffffff}', quadbin '480fffffffffffff');
SELECT contains(quadbinset '{480fffffffffffff, 48427fffffffffff}', quadbinset '{480fffffffffffff}');
SELECT quadbinset '{480fffffffffffff, 48427fffffffffff}' @> quadbin '48a6227affffffff';

SELECT contained(quadbin '480fffffffffffff', quadbinset '{480fffffffffffff, 48427fffffffffff}');
SELECT contained(quadbinset '{480fffffffffffff}', quadbinset '{480fffffffffffff, 48427fffffffffff}');
SELECT quadbin '48a6227affffffff' <@ quadbinset '{480fffffffffffff}';

SELECT overlaps(quadbinset '{480fffffffffffff, 48427fffffffffff}', quadbinset '{48427fffffffffff, 48a6227affffffff}');
SELECT quadbinset '{480fffffffffffff}' && quadbinset '{48427fffffffffff}';
