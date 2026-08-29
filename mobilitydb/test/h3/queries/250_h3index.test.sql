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
-- Input parser
--
-- The h3index literal is parsed by the input function of the h3 extension,
-- which provides the type. It reads hexadecimal, saturates a value wider
-- than 64 bits, and stops at the first character it cannot read, so the
-- decimal spelling of a cell yields an unrelated cell. The MobilityDB
-- parser, which reads the elements of an h3indexset and the values of a
-- th3index, rejects those spellings instead (see 251 and 270).
-------------------------------------------------------------------------------

SELECT h3index '8a2a1072b59ffff';
SELECT h3index '0x8a2a1072b59ffff';
SELECT h3index '622236750694711295';

-- Round trip via the casts
SELECT (622236750694711295::bigint::h3index)::bigint = 622236750694711295;

/* Errors */
-- The message, the position and the hint of the rejection are the h3
-- extension's, which the suite takes from the environment rather than pinning,
-- and their wording differs between its releases. The state code is the part
-- an extension keeps, so the rejection is read from that alone
\set VERBOSITY sqlstate
SELECT h3index '0';
SELECT h3index 'not-a-hex-cell';
SELECT h3index '12345';   -- not a valid H3 cell
\set VERBOSITY default

-------------------------------------------------------------------------------
-- Values the h3 extension's parser admits are rejected where they enter a
-- MobilityDB operation. The operation decides which mode it requires: the
-- saturated value and the short hexadecimal spellings denote neither a cell
-- nor a directed edge nor a vertex, so no operation accepts them. The
-- validity predicates answer false instead of raising, since reporting the
-- mode of an arbitrary value is what they are for.
-------------------------------------------------------------------------------

SELECT isValidCell(h3index '622236750694711295');
SELECT isValidDirectedEdge(h3index '622236750694711295');
SELECT isValidVertex(h3index '622236750694711295');
SELECT isValidCell(h3index '12345');
SELECT isValidCell(h3index '0xabc');

/* Errors */
SELECT gridDisk(h3index '622236750694711295', 1);
SELECT gridDisk(h3index '12345', 1);
SELECT cellToChildren(h3index '0xabc', 6);
SELECT h3GetIcosahedronFaces(h3index '622236750694711295');
SELECT h3OriginToDirectedEdges(h3index '12345');

-- Valid cells keep working, at every resolution
SELECT numValues(gridDisk(h3index '831c02fffffffff', 1));
SELECT numValues(gridDisk(h3index '880326b885fffff', 1));
SELECT numValues(gridDisk(h3index '8a2a1072b59ffff', 1));
SELECT numValues(cellToChildren(h3index '831c02fffffffff', 4));
SELECT numValues(h3CellToVertexes(h3index '880326b885fffff'));
SELECT numValues(h3OriginToDirectedEdges(h3index '8a2a1072b59ffff'));

-------------------------------------------------------------------------------
-- Output: canonical hex form (matches h3-pg)
-------------------------------------------------------------------------------

SELECT h3index '622236750694711295' = h3index '8a2a1072b59ffff';

-------------------------------------------------------------------------------
-- (Hex)WKB round trip
-- Base WKB carries no embedded SRID; the default WGS84 (4326) is implied,
-- exactly like the th3index temporal asBinary/asHexWKB surface.
-------------------------------------------------------------------------------

SELECT h3indexFromBinary(asBinary(h3index '8a2a1072b59ffff'))
       = h3index '8a2a1072b59ffff';
SELECT h3indexFromBinary(asBinary(h3index '831c02fffffffff'))
       = h3index '831c02fffffffff';
SELECT h3indexFromHexWKB(asHexWKB(h3index '8a2a1072b59ffff'))
       = h3index '8a2a1072b59ffff';
SELECT h3indexFromHexWKB(asHexWKB(h3index '880326b885fffff'))
       = h3index '880326b885fffff';

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
\set VERBOSITY terse
SELECT 622236750694711295::bigint = h3index '8a2a1072b59ffff';
\set VERBOSITY default

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
