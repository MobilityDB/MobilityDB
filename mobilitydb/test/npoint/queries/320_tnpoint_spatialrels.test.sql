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
-- point via the standard tnpoint::tgeompoint::tgeometry cast (the point
-- resolved along its route) and delegates to the tgeometry ever/always
-- spatial relationship (070_tgeo_spatialrels.in.sql). Each block checks
-- that the direct tnpoint overload agrees with the equivalent manual cast
-- chain (::tgeompoint::tgeometry), which is how these relationships had to
-- be expressed before this file existed.
--
-- The delegate target tgeometry only supports STEP interpolation, so a
-- multi-instant sequence operand below is written 'Interp=Step;...' (a
-- LINEAR tnpoint sequence errors on the ::tgeometry step of the chain, the
-- same documented restriction already exercised for tgeompoint/tgeogpoint
-- in geo/056_tgeo_spatialfuncs.test.sql). Single-instant values have no
-- interpolation and are unaffected.
--
-------------------------------------------------------------------------------

-- Test for NULL inputs since the functions are STRICT
SELECT eContains(NULL::geometry, tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT eContains(geometry 'SRID=5676;Point(1 1)', NULL::tnpoint);
SELECT tContains(NULL::geometry, tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT tContains(geometry 'SRID=5676;Point(1 1)', NULL::tnpoint);

-------------------------------------------------------------------------------
-- Ever/always contains/covers/disjoint/intersects/touches/dwithin
-------------------------------------------------------------------------------

WITH t AS (
  SELECT tnpoint 'Interp=Step;[Npoint(1, 0.1)@2001-01-01, Npoint(1, 0.3)@2001-01-03]' AS seq1,
         tnpoint 'Interp=Step;[Npoint(2, 0.0)@2001-01-01, Npoint(2, 1)@2001-01-03]' AS seq2,
         geometry 'SRID=5676;Polygon((0 0,0 50,50 50,50 0,0 0))' AS big_region,
         geometry(npoint 'NPoint(2, 0.0)') AS far_point
)
SELECT
  eContains(big_region, seq1) = eContains(big_region, seq1::tgeompoint::tgeometry) AS ec_geo_np,
  eContains(seq1, big_region) = eContains(seq1::tgeompoint::tgeometry, big_region) AS ec_np_geo,
  eContains(seq1, seq2) = eContains(seq1::tgeompoint::tgeometry, seq2::tgeompoint::tgeometry) AS ec_np_np,
  eCovers(big_region, seq1) = eCovers(big_region, seq1::tgeompoint::tgeometry) AS ecv_geo_np,
  eDisjoint(far_point, seq1) = eDisjoint(far_point, seq1::tgeompoint::tgeometry) AS edj_geo_np,
  eIntersects(seq1, seq2) = eIntersects(seq1::tgeompoint::tgeometry, seq2::tgeompoint::tgeometry) AS ei_np_np,
  eTouches(big_region, seq1) = eTouches(big_region, seq1::tgeompoint::tgeometry) AS etc_geo_np,
  eDwithin(seq1, seq2, 1.0) = eDwithin(seq1::tgeompoint::tgeometry, seq2::tgeompoint::tgeometry, 1.0) AS edw_np_np
FROM t;

WITH t AS (
  SELECT tnpoint 'Interp=Step;[Npoint(1, 0.1)@2001-01-01, Npoint(1, 0.3)@2001-01-03]' AS seq1,
         tnpoint 'Interp=Step;[Npoint(2, 0.0)@2001-01-01, Npoint(2, 1)@2001-01-03]' AS seq2,
         geometry 'SRID=5676;Polygon((0 0,0 50,50 50,50 0,0 0))' AS big_region
)
SELECT
  aContains(big_region, seq1) = aContains(big_region, seq1::tgeompoint::tgeometry) AS ac_geo_np,
  aCovers(big_region, seq1) = aCovers(big_region, seq1::tgeompoint::tgeometry) AS acv_geo_np,
  aDisjoint(seq1, seq2) = aDisjoint(seq1::tgeompoint::tgeometry, seq2::tgeompoint::tgeometry) AS adj_np_np,
  aIntersects(big_region, seq1) = aIntersects(big_region, seq1::tgeompoint::tgeometry) AS ai_geo_np,
  aTouches(seq1, big_region) = aTouches(seq1::tgeompoint::tgeometry, big_region) AS atc_np_geo,
  aDwithin(big_region, seq1, 1.0) = aDwithin(big_region, seq1::tgeompoint::tgeometry, 1.0) AS adw_geo_np
FROM t;

-------------------------------------------------------------------------------
-- Direct values (not just cast-equivalence), on single-instant operands (no
-- interpolation restriction), matching the documented manual-cast examples
-- this file's functions now make direct
-------------------------------------------------------------------------------

SELECT eContains(geometry 'SRID=5676;Polygon((0 0,0 50,50 50,50 0,0 0))',
  tnpoint 'Npoint(1, 0.1)@2001-01-01');
SELECT eDisjoint(geometry(npoint 'NPoint(2, 0.0)'),
  tnpoint 'Npoint(1, 0.1)@2001-01-01');
SELECT eIntersects(
  tnpoint 'Npoint(1, 0.1)@2001-01-01',
  tnpoint 'Npoint(2, 0.0)@2001-01-01');

-------------------------------------------------------------------------------
