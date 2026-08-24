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
-------------------------------------------------------------------------------

-- Temporal spatial relationships for tpcpoint. Every one of them converts its
-- operands to the temporal geometry point the schema-aware projection gives
-- and re-delegates, the three predicates its Z-carrying values can answer.
-- That projection promotes step to linear, so tgeompoint — which carries
-- linear interpolation, unlike tgeometry — is what a sequence can reach.

\set square 'geometry ''SRID=0;Polygon((0 0,0 2,2 2,2 0,0 0))'''
\set inside 'tpcpoint(PC_MakePoint(1, ARRAY[1.0, 1.0, 0.0]::float[]), ''2001-01-01''::timestamptz)'
\set corner 'tpcpoint(PC_MakePoint(1, ARRAY[0.0, 0.0, 0.0]::float[]), ''2001-01-01''::timestamptz)'
\set far 'tpcpoint(PC_MakePoint(1, ARRAY[9.0, 9.0, 0.0]::float[]), ''2001-01-02''::timestamptz)'

-------------------------------------------------------------------------------
-- tIntersects and tDisjoint are negations of each other
-------------------------------------------------------------------------------

SELECT asText(tIntersects(:inside, :square));
SELECT asText(tIntersects(:square, :inside));
SELECT asText(tDisjoint(:inside, :square));
SELECT asText(tDisjoint(:far, :square));

-------------------------------------------------------------------------------
-- tDwithin
-------------------------------------------------------------------------------

SELECT asText(tDwithin(:inside, :square, 0.0));
SELECT asText(tDwithin(:inside, geometry 'SRID=0;Point(1 5)', 2.0));
SELECT asText(tDwithin(:inside, :inside, 0.0));

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- A sequence, whose projection carries linear interpolation
-------------------------------------------------------------------------------

\set seq 'tpcpointSeq(ARRAY[tpcpoint(PC_MakePoint(1, ARRAY[1.0, 1.0, 0.0]::float[]), ''2001-01-01''::timestamptz), tpcpoint(PC_MakePoint(1, ARRAY[3.0, 3.0, 0.0]::float[]), ''2001-01-02''::timestamptz)])'

SELECT asText(tIntersects(:seq, :square));
SELECT asText(tIntersects(:square, :seq));
SELECT asText(tDisjoint(:seq, :square));
SELECT asText(tDwithin(:seq, geometry 'SRID=0;Point(1 1)', 1.0));

-- The answer is the one the projection itself gives
SELECT asText(tIntersects(:seq, :square)) =
  asText(tIntersects(:seq::tgeompoint, :square)) AS ti_agrees;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- The matrix of the cast target
--
-- Z is one dimension a schema may state by name, not a property of the type,
-- so a schema declaring X and Y alone answers every planar relationship while
-- one carrying Z meets the engine's refusal, as a temporal geometry point does.
-------------------------------------------------------------------------------

INSERT INTO pointcloud_formats (pcid, srid, schema) VALUES (7, 0,
'<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <pc:dimension>
    <pc:position>1</pc:position>
    <pc:size>4</pc:size>
    <pc:description>X coordinate</pc:description>
    <pc:name>X</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>2</pc:position>
    <pc:size>4</pc:size>
    <pc:description>Y coordinate</pc:description>
    <pc:name>Y</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:metadata>
    <Metadata name="srid">0</Metadata>
  </pc:metadata>
</pc:PointCloudSchema>')
  ON CONFLICT (pcid) DO NOTHING;

\set flat 'tpcpoint(PC_MakePoint(7, ARRAY[1.0, 1.0]::float[]), ''2024-01-01''::timestamptz)'
\set flatsquare 'geometry ''SRID=0;Polygon((0 0,0 4,4 4,4 0,0 0))'''

SELECT asText(tContains(:flatsquare, :flat));
SELECT asText(tCovers(:flatsquare, :flat));
SELECT asText(tTouches(:flatsquare, :flat));
SELECT asText(tTouches(:flat, :flatsquare));

-- Every new cell agrees with the cast written out by hand
SELECT tContains(:flatsquare, :flat) = tContains(:flatsquare, (:flat)::tgeompoint) AS tc_agrees,
       tCovers(:flatsquare, :flat) = tCovers(:flatsquare, (:flat)::tgeompoint) AS tcv_agrees,
       tTouches(:flat, :flatsquare) = tTouches((:flat)::tgeompoint, :flatsquare) AS ttc_agrees;

/* Errors: a schema carrying Z in a planar relationship */
SELECT tContains(:flatsquare, :inside);

-------------------------------------------------------------------------------
