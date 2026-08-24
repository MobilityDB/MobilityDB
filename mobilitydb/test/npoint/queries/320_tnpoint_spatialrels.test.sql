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
-- Regression tests for the tnpoint ever/always spatial-rel surface
-- (320_tnpoint_spatialrels.in.sql).
--
-- A temporal network point has no free-space geometry of its own: every
-- function here converts its tnpoint operand(s) to a temporal geometry
-- point via the standard tnpoint::tgeompoint cast (the point resolved
-- along its route) and delegates to the temporal geometry point
-- ever/always spatial relationship (070_tpoint_spatialrels.in.sql). Each
-- block checks that the direct tnpoint overload agrees with the
-- equivalent manual cast, which is how these relationships had to be
-- expressed before this file existed.
--
-- The operands below carry the DEFAULT linear interpolation, which the
-- temporal geometry point target represents, so a sequence is answered
-- along its route. A step sequence is exercised as a control.
--
-- The family declares the matrix its target declares: tContains and
-- tCovers take the geometry first only, and tTouches has no direction
-- between two network points.
--
-------------------------------------------------------------------------------

-- Test for NULL inputs since the functions are STRICT
SELECT eContains(NULL::geometry, tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT eContains(geometry 'SRID=5676;Point(1 1)', NULL::tnpoint);
SELECT tContains(NULL::geometry, tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tContains(geometry 'SRID=5676;Point(1 1)', NULL::tnpoint);

-------------------------------------------------------------------------------
-- Every declared cell agrees with the cast written out by hand, on the
-- default linear interpolation
-------------------------------------------------------------------------------

WITH t AS (
  SELECT tnpoint '[Npoint(1, 0.1)@2001-01-01, Npoint(1, 0.3)@2001-01-03]' AS seq1,
         tnpoint '[Npoint(2, 0.0)@2001-01-01, Npoint(2, 1)@2001-01-03]' AS seq2,
         geometry 'SRID=5676;Polygon((0 0,0 50,50 50,50 0,0 0))' AS big_region,
         geometry(npoint 'NPoint(2, 0.0)') AS far_point
)
SELECT
  eContains(big_region, seq1) = eContains(big_region, seq1::tgeompoint) AS ec_geo_np,
  eCovers(big_region, seq1) = eCovers(big_region, seq1::tgeompoint) AS ecv_geo_np,
  eDisjoint(far_point, seq1) = eDisjoint(far_point, seq1::tgeompoint) AS edj_geo_np,
  eDisjoint(seq1, far_point) = eDisjoint(seq1::tgeompoint, far_point) AS edj_np_geo,
  eDisjoint(seq1, seq2) = eDisjoint(seq1::tgeompoint, seq2::tgeompoint) AS edj_np_np,
  eIntersects(big_region, seq1) = eIntersects(big_region, seq1::tgeompoint) AS ei_geo_np,
  eIntersects(seq1, big_region) = eIntersects(seq1::tgeompoint, big_region) AS ei_np_geo,
  eIntersects(seq1, seq2) = eIntersects(seq1::tgeompoint, seq2::tgeompoint) AS ei_np_np,
  eTouches(big_region, seq1) = eTouches(big_region, seq1::tgeompoint) AS etc_geo_np,
  eTouches(seq1, big_region) = eTouches(seq1::tgeompoint, big_region) AS etc_np_geo,
  eDwithin(big_region, seq1, 1.0) = eDwithin(big_region, seq1::tgeompoint, 1.0) AS edw_geo_np,
  eDwithin(seq1, big_region, 1.0) = eDwithin(seq1::tgeompoint, big_region, 1.0) AS edw_np_geo,
  eDwithin(seq1, seq2, 1.0) = eDwithin(seq1::tgeompoint, seq2::tgeompoint, 1.0) AS edw_np_np
FROM t;

WITH t AS (
  SELECT tnpoint '[Npoint(1, 0.1)@2001-01-01, Npoint(1, 0.3)@2001-01-03]' AS seq1,
         tnpoint '[Npoint(2, 0.0)@2001-01-01, Npoint(2, 1)@2001-01-03]' AS seq2,
         geometry 'SRID=5676;Polygon((0 0,0 50,50 50,50 0,0 0))' AS big_region,
         geometry(npoint 'NPoint(2, 0.0)') AS far_point
)
SELECT
  aContains(big_region, seq1) = aContains(big_region, seq1::tgeompoint) AS ac_geo_np,
  aCovers(big_region, seq1) = aCovers(big_region, seq1::tgeompoint) AS acv_geo_np,
  aDisjoint(far_point, seq1) = aDisjoint(far_point, seq1::tgeompoint) AS adj_geo_np,
  aDisjoint(seq1, far_point) = aDisjoint(seq1::tgeompoint, far_point) AS adj_np_geo,
  aDisjoint(seq1, seq2) = aDisjoint(seq1::tgeompoint, seq2::tgeompoint) AS adj_np_np,
  aIntersects(big_region, seq1) = aIntersects(big_region, seq1::tgeompoint) AS ai_geo_np,
  aIntersects(seq1, big_region) = aIntersects(seq1::tgeompoint, big_region) AS ai_np_geo,
  aIntersects(seq1, seq2) = aIntersects(seq1::tgeompoint, seq2::tgeompoint) AS ai_np_np,
  aTouches(big_region, seq1) = aTouches(big_region, seq1::tgeompoint) AS atc_geo_np,
  aTouches(seq1, big_region) = aTouches(seq1::tgeompoint, big_region) AS atc_np_geo,
  aDwithin(big_region, seq1, 1.0) = aDwithin(big_region, seq1::tgeompoint, 1.0) AS adw_geo_np,
  aDwithin(seq1, big_region, 1.0) = aDwithin(seq1::tgeompoint, big_region, 1.0) AS adw_np_geo,
  aDwithin(seq1, seq2, 1.0) = aDwithin(seq1::tgeompoint, seq2::tgeompoint, 1.0) AS adw_np_np
FROM t;

-------------------------------------------------------------------------------
-- A linear sequence is answered along its route; the step sequence with the
-- same instants is the control
-------------------------------------------------------------------------------

SELECT eIntersects(tnpoint '[Npoint(1, 0.1)@2001-01-01, Npoint(1, 0.3)@2001-01-03]',
  geometry 'SRID=5676;Polygon((0 0,0 50,50 50,50 0,0 0))');
SELECT eIntersects(tnpoint 'Interp=Step;[Npoint(1, 0.1)@2001-01-01, Npoint(1, 0.3)@2001-01-03]',
  geometry 'SRID=5676;Polygon((0 0,0 50,50 50,50 0,0 0))');
SELECT eDisjoint(geometry(npoint 'NPoint(2, 0.0)'),
  tnpoint '[Npoint(1, 0.1)@2001-01-01, Npoint(1, 0.3)@2001-01-03]');
SELECT aDwithin(tnpoint '[Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-03]',
  tnpoint '[Npoint(1, 0.5)@2001-01-01, Npoint(1, 0.3)@2001-01-03]', 1);

-------------------------------------------------------------------------------
-- Direct values on single-instant operands
-------------------------------------------------------------------------------

SELECT eContains(geometry 'SRID=5676;Polygon((0 0,0 50,50 50,50 0,0 0))',
  tnpoint 'Npoint(1, 0.1)@2001-01-01');
SELECT eCovers(geometry 'SRID=5676;Polygon((0 0,0 50,50 50,50 0,0 0))',
  tnpoint 'Npoint(1, 0.1)@2001-01-01');
SELECT eDisjoint(geometry(npoint 'NPoint(2, 0.0)'),
  tnpoint 'Npoint(1, 0.1)@2001-01-01');
SELECT eTouches(tnpoint 'Npoint(1, 0.1)@2001-01-01',
  geometry 'SRID=5676;Polygon((0 0,0 50,50 50,50 0,0 0))');
SELECT eIntersects(
  tnpoint 'Npoint(1, 0.1)@2001-01-01',
  tnpoint 'Npoint(2, 0.0)@2001-01-01');

-------------------------------------------------------------------------------
