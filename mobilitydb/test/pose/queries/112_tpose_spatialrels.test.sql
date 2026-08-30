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
-- Regression tests for the tpose ever/always spatial-rel surface
-- (112_tpose_spatialrels.in.sql).
--
-- A temporal pose carries a position and an orientation, and these
-- relationships read the position only: every function converts its pose
-- operand(s) to a temporal geometry point through the standard
-- tpose::tgeompoint cast and delegates to the temporal geometry point
-- ever/always spatial relationship (070_tpoint_spatialrels.in.sql). Each
-- block checks that the direct tpose overload agrees with the equivalent
-- manual cast.
--
-- The operands below carry the DEFAULT linear interpolation, which the
-- temporal geometry point target represents, so a sequence is answered
-- along its trajectory. A step sequence is exercised as a control.
--
-- The family declares the matrix its target declares: eContains/aContains
-- and eCovers/aCovers take the geometry first only, and eTouches/aTouches
-- have no direction between two poses.
--
-------------------------------------------------------------------------------

-- Test for NULL inputs since the functions are STRICT
SELECT eContains(NULL::geometry, tpose 'Pose(Point(1 1), 0.1)@2001-01-01');
SELECT eContains(geometry 'Point(1 1)', NULL::tpose);
SELECT aDwithin(NULL::tpose, geometry 'Point(1 1)', 1.0);
SELECT eDwithin(tpose 'Pose(Point(1 1), 0.1)@2001-01-01', NULL::geometry, 1.0);

-------------------------------------------------------------------------------
-- Every declared cell agrees with the cast written out by hand, on the
-- default linear interpolation
-------------------------------------------------------------------------------

WITH t AS (
  SELECT tpose '[Pose(Point(1 1), 0.1)@2001-01-01, Pose(Point(3 3), 0.1)@2001-01-03]' AS seq1,
         tpose '[Pose(Point(5 5), 0.2)@2001-01-01, Pose(Point(7 7), 0.2)@2001-01-03]' AS seq2,
         geometry 'Polygon((0 0,0 4,4 4,4 0,0 0))' AS region,
         geometry 'Point(9 9)' AS far_point
)
SELECT
  eContains(region, seq1) = eContains(region, seq1::tgeompoint) AS ec_geo_ps,
  eCovers(region, seq1) = eCovers(region, seq1::tgeompoint) AS ecv_geo_ps,
  eDisjoint(far_point, seq1) = eDisjoint(far_point, seq1::tgeompoint) AS edj_geo_ps,
  eDisjoint(seq1, far_point) = eDisjoint(seq1::tgeompoint, far_point) AS edj_ps_geo,
  eDisjoint(seq1, seq2) = eDisjoint(seq1::tgeompoint, seq2::tgeompoint) AS edj_ps_ps,
  eIntersects(region, seq1) = eIntersects(region, seq1::tgeompoint) AS ei_geo_ps,
  eIntersects(seq1, region) = eIntersects(seq1::tgeompoint, region) AS ei_ps_geo,
  eIntersects(seq1, seq2) = eIntersects(seq1::tgeompoint, seq2::tgeompoint) AS ei_ps_ps,
  eTouches(region, seq1) = eTouches(region, seq1::tgeompoint) AS etc_geo_ps,
  eTouches(seq1, region) = eTouches(seq1::tgeompoint, region) AS etc_ps_geo,
  eDwithin(region, seq1, 1.0) = eDwithin(region, seq1::tgeompoint, 1.0) AS edw_geo_ps,
  eDwithin(seq1, region, 1.0) = eDwithin(seq1::tgeompoint, region, 1.0) AS edw_ps_geo,
  eDwithin(seq1, seq2, 1.0) = eDwithin(seq1::tgeompoint, seq2::tgeompoint, 1.0) AS edw_ps_ps
FROM t;

WITH t AS (
  SELECT tpose '[Pose(Point(1 1), 0.1)@2001-01-01, Pose(Point(3 3), 0.1)@2001-01-03]' AS seq1,
         tpose '[Pose(Point(5 5), 0.2)@2001-01-01, Pose(Point(7 7), 0.2)@2001-01-03]' AS seq2,
         geometry 'Polygon((0 0,0 4,4 4,4 0,0 0))' AS region,
         geometry 'Point(9 9)' AS far_point
)
SELECT
  aContains(region, seq1) = aContains(region, seq1::tgeompoint) AS ac_geo_ps,
  aCovers(region, seq1) = aCovers(region, seq1::tgeompoint) AS acv_geo_ps,
  aDisjoint(far_point, seq1) = aDisjoint(far_point, seq1::tgeompoint) AS adj_geo_ps,
  aDisjoint(seq1, far_point) = aDisjoint(seq1::tgeompoint, far_point) AS adj_ps_geo,
  aDisjoint(seq1, seq2) = aDisjoint(seq1::tgeompoint, seq2::tgeompoint) AS adj_ps_ps,
  aIntersects(region, seq1) = aIntersects(region, seq1::tgeompoint) AS ai_geo_ps,
  aIntersects(seq1, region) = aIntersects(seq1::tgeompoint, region) AS ai_ps_geo,
  aIntersects(seq1, seq2) = aIntersects(seq1::tgeompoint, seq2::tgeompoint) AS ai_ps_ps,
  aTouches(region, seq1) = aTouches(region, seq1::tgeompoint) AS atc_geo_ps,
  aTouches(seq1, region) = aTouches(seq1::tgeompoint, region) AS atc_ps_geo,
  aDwithin(region, seq1, 1.0) = aDwithin(region, seq1::tgeompoint, 1.0) AS adw_geo_ps,
  aDwithin(seq1, region, 1.0) = aDwithin(seq1::tgeompoint, region, 1.0) AS adw_ps_geo,
  aDwithin(seq1, seq2, 1.0) = aDwithin(seq1::tgeompoint, seq2::tgeompoint, 1.0) AS adw_ps_ps
FROM t;

-------------------------------------------------------------------------------
-- A linear sequence is answered along its trajectory; the step sequence with
-- the same instants is the control
-------------------------------------------------------------------------------

SELECT eIntersects(tpose '[Pose(Point(1 1), 0.1)@2001-01-01, Pose(Point(3 3), 0.1)@2001-01-03]',
  geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))');
SELECT eIntersects(tpose '{Pose(Point(1 1), 0.1)@2001-01-01, Pose(Point(3 3), 0.1)@2001-01-03}',
  geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))');
SELECT aDisjoint(geometry 'Point(9 9)',
  tpose '[Pose(Point(1 1), 0.1)@2001-01-01, Pose(Point(3 3), 0.1)@2001-01-03]');
SELECT eDwithin(tpose '[Pose(Point(1 1), 0.1)@2001-01-01, Pose(Point(3 3), 0.1)@2001-01-03]',
  geometry 'Point(9 9)', 1.0);

-------------------------------------------------------------------------------
-- Direct values on single-instant operands
-------------------------------------------------------------------------------

SELECT eContains(geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))',
  tpose 'Pose(Point(0 0), 0.1)@2001-01-01');
SELECT eCovers(geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))',
  tpose 'Pose(Point(0 0), 0.1)@2001-01-01');
SELECT eTouches(tpose 'Pose(Point(0 0), 0.1)@2001-01-01',
  geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))');
SELECT eIntersects(tpose 'Pose(Point(1 1), 0.1)@2001-01-01',
  tpose 'Pose(Point(1 1), 0.5)@2001-01-01');
SELECT eDwithin(tpose 'Pose(Point(1 1), 0.1)@2001-01-01',
  tpose 'Pose(Point(1 2), 0.5)@2001-01-01', 2.0);

-------------------------------------------------------------------------------
