-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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

-- h3indexset — set of h3 cells. All operations delegate to
-- the generic Set_* C symbols; the dispatch arms in basetype_in /
-- basetype_out (catalog work) make the per-element parser
-- and formatter route through h3index_in / h3index_out.

-------------------------------------------------------------------------------
-- Input / Output
-------------------------------------------------------------------------------

SELECT h3indexset '{8a2a1072b59ffff, 831c02fffffffff, 880326b885fffff}';

-- Singleton
SELECT h3indexset '{8a2a1072b59ffff}';

-- An optional "0x" prefix and insignificant leading zeros are accepted
SELECT h3indexset '{0x8a2a1072b59ffff}';
SELECT h3indexset '{0x0000000008a2a1072b59ffff}';
SELECT h3indexset '{00000008a2a1072b59ffff}';

/* Errors */
SELECT h3indexset '{0}';                     -- the 0 sentinel is rejected
SELECT h3indexset '{not-a-cell}';
-- A cell literal is read as hexadecimal, so the decimal spelling of a cell
-- has more than 16 significant hexadecimal digits and is rejected instead
-- of being silently narrowed to another cell
SELECT h3indexset '{622236750694711295}';
SELECT h3indexset '{ffffffffffffffffff}';
SELECT h3indexset '{0xffffffffffffffffffffffff}';
-- Characters that are not hexadecimal digits are rejected instead of
-- being silently ignored
SELECT h3indexset '{8928308280fffffZZ}';
-- Short hexadecimal is well formed but denotes no cell, directed edge or
-- vertex, so it is rejected rather than stored as a value with no location
SELECT h3indexset '{abc}';
SELECT h3indexset '{0xabc}';
SELECT h3indexset '{12345}';

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
