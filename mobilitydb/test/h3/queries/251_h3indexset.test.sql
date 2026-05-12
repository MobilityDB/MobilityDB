-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- h3indexset — set of h3 cells. All operations delegate to
-- the generic Set_* C symbols; the dispatch arms in basetype_in /
-- basetype_out (catalog work) make the per-element parser
-- and formatter route through h3index_parse / h3index_to_string.

-------------------------------------------------------------------------------
-- Input / Output
-------------------------------------------------------------------------------

SELECT h3indexset '{8a2a1072b59ffff, 831c02fffffffff, 880326b885fffff}';

-- Singleton
SELECT h3indexset '{8a2a1072b59ffff}';

-- Decimal element form is also accepted (h3index_parse handles both)
SELECT h3indexset '{622236750694711295}';

/* Errors */
SELECT h3indexset '{0}';                     -- the 0 sentinel is rejected
SELECT h3indexset '{not-a-cell}';

-------------------------------------------------------------------------------
-- Conversions
-------------------------------------------------------------------------------

-- Singleton set from a cell (convention: set(basetype), matches set(bigint) etc.)
SELECT set(h3index '8a2a1072b59ffff');

-- Cast form of the same
SELECT (h3index '8a2a1072b59ffff')::h3indexset;

-- Array -> set constructor
SELECT set(ARRAY[h3index '8a2a1072b59ffff', h3index '831c02fffffffff']);

-------------------------------------------------------------------------------
-- WKB / HexWKB round-trips
-------------------------------------------------------------------------------

-- asText round-trip: asText(...) should equal the canonical literal form
SELECT asText(h3indexset '{8a2a1072b59ffff}') IS NOT NULL;

-- Binary round-trip
SELECT h3indexsetFromBinary(asBinary(h3indexset '{8a2a1072b59ffff, 831c02fffffffff}'))
       = h3indexset '{8a2a1072b59ffff, 831c02fffffffff}';

-- HexWKB round-trip
SELECT h3indexsetFromHexWKB(asHexWKB(h3indexset '{8a2a1072b59ffff}'))
       = h3indexset '{8a2a1072b59ffff}';

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT numValues(h3indexset '{8a2a1072b59ffff, 831c02fffffffff, 880326b885fffff}');
SELECT startValue(h3indexset '{8a2a1072b59ffff, 831c02fffffffff, 880326b885fffff}');
SELECT endValue(h3indexset '{8a2a1072b59ffff, 831c02fffffffff, 880326b885fffff}');
SELECT valueN(h3indexset '{8a2a1072b59ffff, 831c02fffffffff}', 2);

-- Out-of-range yields NULL
SELECT valueN(h3indexset '{8a2a1072b59ffff}', 99);

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT h3indexset '{8a2a1072b59ffff}' = h3indexset '{8a2a1072b59ffff}';
SELECT h3indexset '{8a2a1072b59ffff}' <> h3indexset '{831c02fffffffff}';

-------------------------------------------------------------------------------
-- btree + hash opclasses (DISTINCT and GROUP BY)
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_h3indexset_test;
CREATE TABLE tbl_h3indexset_test(k int, s h3indexset);
INSERT INTO tbl_h3indexset_test VALUES
  (1, h3indexset '{8a2a1072b59ffff}'),
  (2, h3indexset '{831c02fffffffff}'),
  (3, h3indexset '{8a2a1072b59ffff}'),
  (4, h3indexset '{8a2a1072b59ffff, 831c02fffffffff}');

SELECT COUNT(DISTINCT s) FROM tbl_h3indexset_test;
DROP TABLE tbl_h3indexset_test;

-------------------------------------------------------------------------------
-- unnest: SETOF expansion
-------------------------------------------------------------------------------

-- Row count matches numValues
SELECT COUNT(*) = numValues(h3indexset '{8a2a1072b59ffff, 831c02fffffffff, 880326b885fffff}')
FROM unnest(h3indexset '{8a2a1072b59ffff, 831c02fffffffff, 880326b885fffff}');

-------------------------------------------------------------------------------
-- setUnion aggregate (scalar → set, set → set)
-------------------------------------------------------------------------------

-- Aggregate scalars into a single h3indexset
SELECT setUnion(v) = h3indexset '{8a2a1072b59ffff, 831c02fffffffff}'
FROM (VALUES (h3index '8a2a1072b59ffff'), (h3index '831c02fffffffff'),
             (h3index '8a2a1072b59ffff')) AS t(v);

-- Aggregate sets into a single h3indexset (duplicates collapse)
SELECT setUnion(s) = h3indexset '{8a2a1072b59ffff, 831c02fffffffff, 880326b885fffff}'
FROM (VALUES (h3indexset '{8a2a1072b59ffff}'),
             (h3indexset '{831c02fffffffff, 880326b885fffff}'),
             (h3indexset '{8a2a1072b59ffff}')) AS t(s);

-------------------------------------------------------------------------------
