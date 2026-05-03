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

-- Coverage fixtures for the in-house Welzl minimum-bounding-circle path.
-- Single-arg cbuffer(geometry) routes non-POINT / non-CURVEPOLY inputs
-- through geom_min_bounding_radius -> lwgeom_mec / lwgeom_calculate_mbc,
-- exercising mec_welzl, mec_circle3, mec_circle3_safe, lwgeom_mec_supported_type
-- and lwgeom_collect_points in meos/src/geo/postgis_funcs.c.

-------------------------------------------------------------------------------
-- Welzl base cases (n = 1, 2, 3)
-------------------------------------------------------------------------------

SELECT round(radius(cbuffer('LINESTRING(0 0, 4 0)'::geometry)), 6);
SELECT round(radius(cbuffer('LINESTRING(0 0, 4 0, 2 1.7320508)'::geometry)), 6);

-- Three colinear points -> mec_circle3 colinear fallback
SELECT round(radius(cbuffer('LINESTRING(0 0, 1 0, 2 0)'::geometry)), 6);

-- Multipoint with N > 3, multiple boundary candidates -> recursion depth
SELECT round(radius(cbuffer('MULTIPOINT(0 0, 4 0, 4 3, 0 3, 2 1.5)'::geometry)), 6);
SELECT round(radius(cbuffer('MULTIPOINT(0 0, 10 0, 0 10, 10 10, 5 5)'::geometry)), 6);

-------------------------------------------------------------------------------
-- Polygon dispatch (lwgeom_mec POLYGON branch + ring iteration)
-------------------------------------------------------------------------------

-- Single-ring polygon
SELECT round(radius(cbuffer('POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))'::geometry)), 6);

-- Polygon with hole — exercises ring iteration in lwgeom_collect_points;
-- only the outer ring contributes to the MEC, but both rings are visited.
SELECT round(radius(cbuffer(
  'POLYGON((0 0, 10 0, 10 10, 0 10, 0 0), (3 3, 7 3, 7 7, 3 7, 3 3))'::geometry)), 6);

-------------------------------------------------------------------------------
-- MULTI* dispatch
-------------------------------------------------------------------------------

SELECT round(radius(cbuffer('MULTILINESTRING((0 0, 4 0), (1 1, 3 3))'::geometry)), 6);
SELECT round(radius(cbuffer(
  'MULTIPOLYGON(((0 0, 4 0, 4 4, 0 4, 0 0)), ((6 6, 10 6, 10 10, 6 10, 6 6)))'::geometry)), 6);

-------------------------------------------------------------------------------
-- GEOMETRYCOLLECTION branches in lwgeom_mec_supported_type / lwgeom_collect_points
-------------------------------------------------------------------------------

-- All members supported -> Welzl on the union of points
SELECT round(radius(cbuffer(
  'GEOMETRYCOLLECTION(POINT(0 0), LINESTRING(2 0, 4 0), POLYGON((1 1, 3 1, 3 3, 1 3, 1 1)))'::geometry)), 6);

-- Member is unsupported (CIRCULARSTRING) -> falls through to lwgeom_calculate_mbc
SELECT round(radius(cbuffer(
  'GEOMETRYCOLLECTION(POINT(0 0), CIRCULARSTRING(0 0, 1 1, 2 0))'::geometry)), 6);

-------------------------------------------------------------------------------
-- Empty geometry (returns empty point + radius 0)
-------------------------------------------------------------------------------

SELECT radius(cbuffer('LINESTRING EMPTY'::geometry));
SELECT ST_AsText(geometry(cbuffer('LINESTRING EMPTY'::geometry)));

-------------------------------------------------------------------------------
