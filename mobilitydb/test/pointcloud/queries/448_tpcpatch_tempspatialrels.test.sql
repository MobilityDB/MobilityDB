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

-- Temporal spatial relationships for tpcpatch. A patch reaches the geometry
-- engine as the multipoint of the positions its points occupy, and answers the
-- three predicates its Z-carrying values can answer.

\set square 'geometry ''SRID=0;Polygon((0 0,0 2,2 2,2 0,0 0))'''
\set inside 'tpcpatch(pcpatch(1, pcpoint(1, 1, 1, 1)), ''2001-01-01''::timestamptz)'
\set straddling 'tpcpatch(pcpatch(1, pcpoint(1, 1, 1, 1), pcpoint(1, 9, 9, 9)), ''2001-01-01''::timestamptz)'
\set far 'tpcpatch(pcpatch(1, pcpoint(1, 9, 9, 9)), ''2001-01-02''::timestamptz)'

-------------------------------------------------------------------------------
-- The geometry a patch reaches the engine as
-------------------------------------------------------------------------------

SELECT ST_AsText(geometry(pcpatch(1, pcpoint(1, 1, 1, 1), pcpoint(1, 2, 2, 2))));
SELECT ST_AsText(pcpatch(1, pcpoint(1, 1, 1, 1))::geometry);
SELECT asText(:straddling::tgeometry);

-------------------------------------------------------------------------------
-- tIntersects and tDisjoint are negations of each other
-------------------------------------------------------------------------------

SELECT asText(tIntersects(:inside, :square));
SELECT asText(tIntersects(:square, :inside));
SELECT asText(tIntersects(:far, :square));
SELECT asText(tDisjoint(:inside, :square));
SELECT asText(tDisjoint(:far, :square));

-- One point of the patch inside the square is enough for the whole patch to
-- intersect it
SELECT asText(tIntersects(:straddling, :square));
SELECT asText(tDisjoint(:straddling, :square));

-------------------------------------------------------------------------------
-- tDwithin
-------------------------------------------------------------------------------

SELECT asText(tDwithin(:inside, :square, 0.0));
SELECT asText(tDwithin(:inside, geometry 'SRID=0;Point(1 2)', 2.0));
SELECT asText(tDwithin(:inside, geometry 'SRID=0;Point(1 5)', 2.0));
SELECT asText(tDwithin(:inside, :inside, 0.0));

-------------------------------------------------------------------------------
-- A sequence answers instant by instant
-------------------------------------------------------------------------------

SELECT asText(tIntersects(tpcpatchSeq(ARRAY[:inside, :far]), :square));

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- The matrix of the cast target
--
-- A patch reaches the engine as the multipoint of the positions its points
-- occupy, so the temporal geometry is its cast target and the family declares
-- every predicate in every direction. Z is one dimension a schema states by
-- name: a schema declaring X and Y alone answers the planar relationships,
-- while one carrying Z meets the engine's refusal.
-------------------------------------------------------------------------------

INSERT INTO pointcloud_formats (pcid, srid, schema) VALUES (8, 0,
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

\set flatpatch 'tpcpatch(PC_Patch(ARRAY[PC_MakePoint(8, ARRAY[1.0, 1.0]::float[]), PC_MakePoint(8, ARRAY[2.0, 2.0]::float[])]), ''2001-01-01''::timestamptz)'

SELECT asText(tContains(:square, :flatpatch));
SELECT asText(tCovers(:square, :flatpatch));
SELECT asText(tTouches(:square, :flatpatch));
SELECT asText(tTouches(:flatpatch, :square));
SELECT asText(tContains(:flatpatch, :square));

-- Every new cell agrees with the cast written out by hand
SELECT tContains(:square, :flatpatch) = tContains(:square, (:flatpatch)::tgeometry) AS tc_agrees,
       tCovers(:square, :flatpatch) = tCovers(:square, (:flatpatch)::tgeometry) AS tcv_agrees,
       tTouches(:flatpatch, :square) = tTouches((:flatpatch)::tgeometry, :square) AS ttc_agrees,
       tContains(:flatpatch, :square) = tContains((:flatpatch)::tgeometry, :square) AS tc_rev_agrees;

/* Errors: a schema carrying Z in a planar relationship */
SELECT tContains(:square, :inside);

-------------------------------------------------------------------------------
