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
-- Regression tests for the tpcpatch ever/always spatial-rel surface
-- (449_tpcpatch_spatialrels.in.sql).
--
-- A patch reaches the geometry engine as the multipoint of the positions its
-- points occupy, so the temporal geometry is its cast target and the family
-- declares the matrix that target declares: every predicate in every
-- direction. Each block checks that the direct tpcpatch overload agrees with
-- the equivalent cast written out by hand.
--
-- Z is one dimension a schema states by name, so pcid 8 declaring X and Y
-- alone answers the planar relationships while the fixture's pcid 1, which
-- carries Z, meets the engine's refusal.
--
-------------------------------------------------------------------------------

\set square 'geometry ''SRID=0;Polygon((0 0,0 4,4 4,4 0,0 0))'''
\set zpatch 'tpcpatch(pcpatch(1, pcpoint(1, 1, 1, 1)), ''2001-01-01''::timestamptz)'

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

\set flat 'tpcpatch(PC_Patch(ARRAY[PC_MakePoint(8, ARRAY[1.0, 1.0]::float[]), PC_MakePoint(8, ARRAY[2.0, 2.0]::float[])]), ''2001-01-01''::timestamptz)'
\set flatfar 'tpcpatch(PC_Patch(ARRAY[PC_MakePoint(8, ARRAY[9.0, 9.0]::float[])]), ''2001-01-01''::timestamptz)'

-- Test for NULL inputs since the functions are STRICT
SELECT eContains(NULL::geometry, :flat);
SELECT eIntersects(:flat, NULL::geometry);

-------------------------------------------------------------------------------
-- Every declared cell agrees with the cast written out by hand
-------------------------------------------------------------------------------

SELECT eContains(:square, :flat) = eContains(:square, (:flat)::tgeometry) AS ec_geo_pp,
       eContains(:flat, :square) = eContains((:flat)::tgeometry, :square) AS ec_pp_geo,
       eContains(:flat, :flatfar) = eContains((:flat)::tgeometry, (:flatfar)::tgeometry) AS ec_pp_pp,
       eCovers(:square, :flat) = eCovers(:square, (:flat)::tgeometry) AS ecv_geo_pp,
       eDisjoint(:flat, :flatfar) = eDisjoint((:flat)::tgeometry, (:flatfar)::tgeometry) AS edj_pp_pp,
       eIntersects(:square, :flat) = eIntersects(:square, (:flat)::tgeometry) AS ei_geo_pp,
       eTouches(:flat, :square) = eTouches((:flat)::tgeometry, :square) AS etc_pp_geo,
       eDwithin(:flat, :flatfar, 1.0) = eDwithin((:flat)::tgeometry, (:flatfar)::tgeometry, 1.0) AS edw_pp_pp;

SELECT aContains(:square, :flat) = aContains(:square, (:flat)::tgeometry) AS ac_geo_pp,
       aCovers(:square, :flat) = aCovers(:square, (:flat)::tgeometry) AS acv_geo_pp,
       aDisjoint(:flat, :flatfar) = aDisjoint((:flat)::tgeometry, (:flatfar)::tgeometry) AS adj_pp_pp,
       aIntersects(:flat, :square) = aIntersects((:flat)::tgeometry, :square) AS ai_pp_geo,
       aTouches(:square, :flat) = aTouches(:square, (:flat)::tgeometry) AS atc_geo_pp,
       aDwithin(:square, :flat, 1.0) = aDwithin(:square, (:flat)::tgeometry, 1.0) AS adw_geo_pp;

-------------------------------------------------------------------------------
-- Direct values
-------------------------------------------------------------------------------

SELECT eContains(:square, :flat);
SELECT eCovers(:square, :flat);
SELECT eDisjoint(:flat, :flatfar);
SELECT eIntersects(:square, :flat);
SELECT eTouches(:flat, :square);
SELECT eDwithin(:flat, :flatfar, 1.0);

/* Errors: a schema carrying Z in a planar relationship */
SELECT eContains(:square, :zpatch);

-------------------------------------------------------------------------------
