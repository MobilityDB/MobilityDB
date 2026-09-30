-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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

-- Value-level tests for asMFJSON() on tpcpoint and tpcpatch.

\set p1 'tpcpoint(PC_MakePoint(1, ARRAY[1.0, 2.0, 3.0]::float[]), ''2024-01-01''::timestamptz)'
\set p2 'tpcpoint(PC_MakePoint(1, ARRAY[4.0, 5.0, 6.0]::float[]), ''2024-01-02''::timestamptz)'
\set patch 'PC_Patch(ARRAY[PC_MakePoint(1, ARRAY[1.0, 1.0, 1.0]::float[]), PC_MakePoint(1, ARRAY[2.0, 2.0, 2.0]::float[])])'
\set q1 'tpcpatch(:patch, ''2024-01-01''::timestamptz)'

-------------------------------------------------------------------------------
-- tpcpoint MF-JSON: instant + sequence with bbox = options 1.
-------------------------------------------------------------------------------

SELECT asMFJSON(:p1);
SELECT asMFJSON(tpcpointSeq(ARRAY[:p1, :p2]));
SELECT asMFJSON(:p1, 1);

-------------------------------------------------------------------------------
-- tpcpoint MF-JSON shape — structural assertions (mirror 432 for tpcpatch).
-------------------------------------------------------------------------------

-- type tag — MovingPCPoint on every subtype
SELECT asMFJSON(:p1)::jsonb ->> 'type' = 'MovingPCPoint';
SELECT asMFJSON(tpcpointSeq(ARRAY[:p1, :p2]))::jsonb ->> 'type' = 'MovingPCPoint';

-- coordinates — one [x, y, z] per instant, in input order
SELECT (asMFJSON(:p1)::jsonb -> 'coordinates' -> 0) = '[1, 2, 3]'::jsonb;
SELECT jsonb_array_length(asMFJSON(:p1)::jsonb -> 'coordinates') = 1;
SELECT jsonb_array_length(
  asMFJSON(tpcpointSeq(ARRAY[:p1, :p2]))::jsonb -> 'coordinates') = 2;
SELECT (asMFJSON(tpcpointSeq(ARRAY[:p1, :p2]))::jsonb
  -> 'coordinates' -> 1) = '[4, 5, 6]'::jsonb;

-- bbox embedding (options=1) — pcid + bbox + period at the top level
SELECT (asMFJSON(:p1, options := 1)::jsonb ->> 'pcid')::int = 1;
SELECT asMFJSON(:p1, options := 1)::jsonb ? 'bbox';
-- bbox is [[xmin, ymin, zmin], [xmax, ymax, zmax]] for 3D schemas
SELECT jsonb_array_length(asMFJSON(:p1, options := 1)::jsonb -> 'bbox') = 2;
SELECT jsonb_array_length(
  asMFJSON(:p1, options := 1)::jsonb -> 'bbox' -> 0) = 3;

-- bbox + period coexist
SELECT asMFJSON(:p1, options := 1)::jsonb ? 'period';
SELECT asMFJSON(:p1, options := 1)::jsonb -> 'period' ? 'begin';
SELECT asMFJSON(:p1, options := 1)::jsonb -> 'period' ? 'end';

-- datetimes / interpolation
SELECT asMFJSON(:p1)::jsonb ->> 'interpolation' = 'None';
SELECT asMFJSON(tpcpointSeq(ARRAY[:p1, :p2]))::jsonb ->> 'interpolation' = 'Step';
-- datetimes count matches coordinates count
SELECT jsonb_array_length(
  asMFJSON(tpcpointSeq(ARRAY[:p1, :p2]))::jsonb -> 'datetimes') =
  jsonb_array_length(
  asMFJSON(tpcpointSeq(ARRAY[:p1, :p2]))::jsonb -> 'coordinates');

-------------------------------------------------------------------------------
-- tpcpatch MF-JSON: pcid + npoints + bounds in the values array.
-------------------------------------------------------------------------------

SELECT asMFJSON(:q1);
SELECT asMFJSON(:q1, 1);

-------------------------------------------------------------------------------
-- tpcpoint MF-JSON round trip: the values member states every dimension of
-- the point, so the value reads back equal to the one written.
-------------------------------------------------------------------------------

-- The values member is pgpointcloud's text form of the point
SELECT asMFJSON(:p1)::jsonb -> 'values' -> 0 = '{"pcid": 1, "pt": [1, 2, 3]}'::jsonb;

SELECT tpcpointFromMFJSON(asMFJSON(:p1)) = :p1;
SELECT tpcpointFromMFJSON(asMFJSON(tpcpointSeq(ARRAY[:p1, :p2]))) =
  tpcpointSeq(ARRAY[:p1, :p2]);
SELECT tpcpointFromMFJSON(asMFJSON(tpcpointSeqSet(ARRAY[
  tpcpointSeq(ARRAY[:p1]), tpcpointSeq(ARRAY[:p2])]))) =
  tpcpointSeqSet(ARRAY[tpcpointSeq(ARRAY[:p1]), tpcpointSeq(ARRAY[:p2])]);
-- The bounding box and the precision of the coordinates leave the values whole
SELECT tpcpointFromMFJSON(asMFJSON(tpcpointSeq(ARRAY[:p1, :p2]), 1, 0, 0)) =
  tpcpointSeq(ARRAY[:p1, :p2]);

-- Scaled and offset integers, a double of 17 digits and the largest unsigned
-- 16-bit intensity read back exactly
INSERT INTO pointcloud_schemas (pcid, srid, compression) VALUES (92, 4326, 'none');
INSERT INTO pointcloud_dimensions
    (pcid, dim_no, dim_name, interpretation, dim_scale, dim_offset) VALUES
  (92, 1, 'X', 'int32_t', 0.01, 1000), (92, 2, 'Y', 'int32_t', 0.01, -500),
  (92, 3, 'Z', 'double', 1, 0), (92, 4, 'Intensity', 'uint16_t', 1, 0);
\set s1 'tpcpoint(pcpoint(92, ARRAY[1000.25, -499.99, 0.1, 17]::float[]), ''2024-01-01''::timestamptz)'
\set s2 'tpcpoint(pcpoint(92, ARRAY[1003.5, -497.25, 123456.78901234568, 65535]::float[]), ''2024-01-02''::timestamptz)'
SELECT asMFJSON(:s1)::jsonb -> 'values';
SELECT tpcpointFromMFJSON(asMFJSON(tpcpointSeq(ARRAY[:s1, :s2]))) =
  tpcpointSeq(ARRAY[:s1, :s2]);
SELECT getDim(getValue(tpcpointFromMFJSON(asMFJSON(:s2))), 'Z') =
    123456.78901234568::float,
  getDim(getValue(tpcpointFromMFJSON(asMFJSON(:s2))), 'Intensity');
DELETE FROM pointcloud_schemas WHERE pcid = 92;

-- A document stating the coordinates and not the values states no point
SELECT tpcpointFromMFJSON('{"type":"MovingPCPoint","coordinates":[[1,2,3]],'
  '"datetimes":["2024-01-01T00:00:00+00"],"interpolation":"None"}');
-- The values state as many dimensions as the schema of their pcid
SELECT tpcpointFromMFJSON('{"type":"MovingPCPoint","values":[{"pcid":1,"pt":[1,2]}],'
  '"datetimes":["2024-01-01T00:00:00+00"],"interpolation":"None"}');

-------------------------------------------------------------------------------
