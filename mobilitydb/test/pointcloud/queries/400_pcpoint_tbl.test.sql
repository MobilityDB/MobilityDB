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

-- Table-level tests for the static pcpoint / pcpatch types and the
-- pcpointset / pcpatchset set types.  Every query collapses to a single
-- scalar (COUNT, MIN, MAX, bool_and, etc.) so the diff is human-readable.

-- Note: pgpointcloud's static pcpoint / pcpatch do not expose binary
-- send/receive functions or B-tree comparison operators, so the
-- COPY BINARY round-trip and `=`/`<>`/`<` patterns used by the
-- temporal types are not exercised here.

-------------------------------------------------------------------------------
-- pcid uniformity — every row in the datagen tables must use pcid = 1.
-------------------------------------------------------------------------------

SELECT bool_and(pcid(pt) = 1) FROM tbl_pcpoint;
SELECT bool_and(pcid(pa) = 1) FROM tbl_pcpatch;

-------------------------------------------------------------------------------
-- Coordinate accessors range-check.
-------------------------------------------------------------------------------

SELECT bool_and(getX(pt) BETWEEN -100 AND 100) FROM tbl_pcpoint;
SELECT bool_and(getY(pt) BETWEEN -100 AND 100) FROM tbl_pcpoint;
SELECT bool_and(getZ(pt) BETWEEN    0 AND 100) FROM tbl_pcpoint;

-------------------------------------------------------------------------------
-- Set types — basic invariants.
-------------------------------------------------------------------------------

SELECT bool_and(numValues(s) BETWEEN 1 AND 10)
FROM tbl_pcpointset WHERE s IS NOT NULL;
SELECT COUNT(*) FROM tbl_pcpointset WHERE s IS NULL;

SELECT bool_and(numValues(s) BETWEEN 1 AND 10)
FROM tbl_pcpatchset WHERE s IS NOT NULL;
SELECT COUNT(*) FROM tbl_pcpatchset WHERE s IS NULL;

-------------------------------------------------------------------------------
-- Set types — input/output from/to WKB, EWKB, HexWKB, and HexEWKB.
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_pcpointset
WHERE s IS NOT NULL AND pcpointsetFromBinary(asBinary(s)) <> s;
SELECT COUNT(*) FROM tbl_pcpointset
WHERE s IS NOT NULL AND pcpointsetFromBinary(asEWKB(s)) <> s;
SELECT COUNT(*) FROM tbl_pcpointset
WHERE s IS NOT NULL AND pcpointsetFromHexWKB(asHexWKB(s)) <> s;
SELECT COUNT(*) FROM tbl_pcpointset
WHERE s IS NOT NULL AND pcpointsetFromHexWKB(asHexEWKB(s)) <> s;

SELECT COUNT(*) FROM tbl_pcpatchset
WHERE s IS NOT NULL AND pcpatchsetFromBinary(asBinary(s)) <> s;
SELECT COUNT(*) FROM tbl_pcpatchset
WHERE s IS NOT NULL AND pcpatchsetFromBinary(asEWKB(s)) <> s;
SELECT COUNT(*) FROM tbl_pcpatchset
WHERE s IS NOT NULL AND pcpatchsetFromHexWKB(asHexWKB(s)) <> s;
SELECT COUNT(*) FROM tbl_pcpatchset
WHERE s IS NOT NULL AND pcpatchsetFromHexWKB(asHexEWKB(s)) <> s;

-- The extended form carries the SRID the plain one omits, and a round trip
-- holds either way, so only a comparison of the two forms sees it. The fixture
-- registers its only schema with SRID 0, where the two forms agree by
-- definition, so a schema carrying an SRID is what makes the question
-- answerable.
SELECT COUNT(*) FROM tbl_pcpointset
WHERE s IS NOT NULL AND asEWKB(s) = asBinary(s);
SELECT COUNT(*) FROM tbl_pcpatchset
WHERE s IS NOT NULL AND asEWKB(s) = asBinary(s);

INSERT INTO pointcloud_formats (pcid, srid, schema)
SELECT 3, 4326, schema FROM pointcloud_formats WHERE pcid = 1;

-- Of a schema carrying an SRID: the extended form is the four bytes of the
-- SRID longer, differs from the plain one, and both still round-trip.
WITH t AS (SELECT set(ARRAY[pcpoint(3, 1.0, 1.0, 1.0),
  pcpoint(3, 2.0, 2.0, 2.0)]) AS s)
SELECT asEWKB(s) <> asBinary(s) AS differ,
  octet_length(asEWKB(s)) - octet_length(asBinary(s)) AS extra_bytes,
  pcpointsetFromBinary(asEWKB(s)) = s AS ewkb_roundtrips,
  pcpointsetFromBinary(asBinary(s)) = s AS wkb_roundtrips
FROM t;

DELETE FROM pointcloud_formats WHERE pcid = 3;

-------------------------------------------------------------------------------
