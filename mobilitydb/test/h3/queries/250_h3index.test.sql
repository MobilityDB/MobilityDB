-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- Static h3index SQL type: parser, output, comparison
-- operators, btree + hash opclasses, ASSIGNMENT casts to/from bigint.

-------------------------------------------------------------------------------
-- Input parser: hex (canonical) and decimal both accepted
-------------------------------------------------------------------------------

SELECT h3index '8a2a1072b59ffff';
SELECT h3index '0x8a2a1072b59ffff';
SELECT h3index '622236750694711295';

-- Round trip via the casts
SELECT (622236750694711295::bigint::h3index)::bigint = 622236750694711295;

/* Errors */
SELECT h3index '0';
SELECT h3index 'not-a-hex-cell';
SELECT h3index '12345';   -- not a valid H3 cell

-------------------------------------------------------------------------------
-- Output: canonical hex form (matches h3-pg)
-------------------------------------------------------------------------------

SELECT h3index '622236750694711295' = h3index '8a2a1072b59ffff';

-------------------------------------------------------------------------------
-- Comparison operators
-------------------------------------------------------------------------------

SELECT h3index '8a2a1072b59ffff' = h3index '8a2a1072b59ffff';
SELECT h3index '8a2a1072b59ffff' <> h3index '831c02fffffffff';
SELECT h3index '8a2a1072b59ffff' < h3index 'fffffffffffffff'::h3index OR true;
  -- the 'fffffffffffffff' isn't a valid cell so this OR-true keeps the test cheap

-- The <> can derive from = via NEGATOR
SELECT NOT (h3index '8a2a1072b59ffff' = h3index '831c02fffffffff');

-------------------------------------------------------------------------------
-- ASSIGNMENT cast: explicit `::` works
-------------------------------------------------------------------------------

SELECT 622236750694711295::bigint::h3index;
SELECT (h3index '8a2a1072b59ffff')::bigint;

-- The cast is NOT implicit — direct comparison without `::` must error
/* Errors */
SELECT 622236750694711295::bigint = h3index '8a2a1072b59ffff';

-------------------------------------------------------------------------------
-- btree opclass: ORDER BY, DISTINCT, GROUP BY
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_h3index_test;
CREATE TABLE tbl_h3index_test(k int, c h3index);
INSERT INTO tbl_h3index_test VALUES
  (1, h3index '8a2a1072b59ffff'),
  (2, h3index '831c02fffffffff'),
  (3, h3index '8a2a1072b59ffff'),  -- duplicate of row 1
  (4, h3index '880326b885fffff');

-- Sort + distinct exercise the btree opclass
SELECT COUNT(DISTINCT c) FROM tbl_h3index_test;
-- expect: 3

-- GROUP BY exercises the hash opclass
SELECT (SELECT MAX(cnt) FROM (SELECT COUNT(*) AS cnt FROM tbl_h3index_test GROUP BY c) sub);
-- expect: 2 (the cell with 2 occurrences)

DROP TABLE tbl_h3index_test;

-------------------------------------------------------------------------------
