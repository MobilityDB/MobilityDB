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
-- Regression tests for the tpose temporal spatial-rel surface
-- (114_tpose_tempspatialrels.in.sql).
--
-- Each function returns a tbool whose value at instant t is the static
-- spatial relation applied to the position the pose holds at t: cast the
-- tpose to tgeompoint and delegate to the temporal spatial-rel kernel the geo
-- family owns (072_tgeo_tempspatialrels.in.sql). A pose holds a position, so
-- tgeompoint is the cast target its geometry names; the block below checks
-- that each direct tpose overload agrees with the longer manual chain
-- (::tgeompoint::tgeometry) that reaches the same kernel, which is what makes
-- the shorter route a pure gain rather than a different answer.
--
-- The family declares the matrix its target declares: tContains and tCovers
-- take the geometry first only, and tTouches has no direction between two
-- tposes, because a moving point neither contains nor covers a geometry and
-- two moving points do not touch.
--
-------------------------------------------------------------------------------

-- Test for NULL inputs since the functions are not STRICT
SELECT tContains(NULL::geometry, tpose 'Pose(Point(1 1), 0.2)@2001-01-01');
SELECT tContains(geometry 'Polygon((0 0,0 5,5 5,5 0,0 0))', NULL::tpose);

-------------------------------------------------------------------------------
-- tContains, tCovers, tDisjoint, tIntersects, tTouches, tDwithin
-------------------------------------------------------------------------------

WITH t AS (
  SELECT tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(2 2), 0.4)@2001-01-03]' AS seq1,
         tpose 'Interp=Step;[Pose(Point(2 2), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-03]' AS seq2,
         geometry 'Polygon((0 0,0 5,5 5,5 0,0 0))' AS region,
         geometry 'Point(50 50)' AS far_point
)
SELECT
  asText(tContains(region, seq1)) = asText(tContains(region, seq1::tgeompoint::tgeometry)) AS tc_geo_po,
  asText(tCovers(region, seq1)) = asText(tCovers(region, seq1::tgeompoint::tgeometry)) AS tcv_geo_po,
  asText(tDisjoint(far_point, seq1)) = asText(tDisjoint(far_point, seq1::tgeompoint::tgeometry)) AS tdj_geo_po,
  asText(tDisjoint(seq1, far_point)) = asText(tDisjoint(seq1::tgeompoint::tgeometry, far_point)) AS tdj_po_geo,
  asText(tDisjoint(seq1, seq2)) = asText(tDisjoint(seq1::tgeompoint::tgeometry, seq2::tgeompoint::tgeometry)) AS tdj_po_po,
  asText(tIntersects(region, seq1)) = asText(tIntersects(region, seq1::tgeompoint::tgeometry)) AS ti_geo_po,
  asText(tIntersects(seq1, region)) = asText(tIntersects(seq1::tgeompoint::tgeometry, region)) AS ti_po_geo,
  asText(tIntersects(seq1, seq2)) = asText(tIntersects(seq1::tgeompoint::tgeometry, seq2::tgeompoint::tgeometry)) AS ti_po_po,
  asText(tTouches(region, seq1)) = asText(tTouches(region, seq1::tgeompoint::tgeometry)) AS ttc_geo_po,
  asText(tTouches(seq1, region)) = asText(tTouches(seq1::tgeompoint::tgeometry, region)) AS ttc_po_geo,
  asText(tDwithin(region, seq1, 1.0)) = asText(tDwithin(region, seq1::tgeompoint::tgeometry, 1.0)) AS tdw_geo_po,
  asText(tDwithin(seq1, region, 1.0)) = asText(tDwithin(seq1::tgeompoint::tgeometry, region, 1.0)) AS tdw_po_geo,
  asText(tDwithin(seq1, seq2, 1.0)) = asText(tDwithin(seq1::tgeompoint::tgeometry, seq2::tgeompoint::tgeometry, 1.0)) AS tdw_po_po
FROM t;

-------------------------------------------------------------------------------
-- Direct values
-------------------------------------------------------------------------------

SELECT tContains(geometry 'Polygon((0 0,0 5,5 5,5 0,0 0))',
  tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(2 2), 0.4)@2001-01-03]');
SELECT tDisjoint(geometry 'Point(50 50)',
  tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(2 2), 0.4)@2001-01-03]');
SELECT tDwithin(
  tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(2 2), 0.4)@2001-01-03]',
  tpose 'Interp=Step;[Pose(Point(2 2), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-03]', 1.0);

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Linear interpolation, which the tgeompoint target carries
-------------------------------------------------------------------------------

WITH t AS (
  SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(4 4), 0.4)@2001-01-03]' AS lin1,
         tpose '[Pose(Point(4 4), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-03]' AS lin2,
         geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))' AS region
)
SELECT
  asText(tIntersects(region, lin1)) AS ti_geo_po,
  asText(tIntersects(lin1, region)) AS ti_po_geo,
  asText(tDisjoint(lin1, region)) AS tdj_po_geo,
  asText(tDwithin(lin1, region, 1.0)) AS tdw_po_geo,
  asText(tDwithin(lin1, lin2, 1.0)) AS tdw_po_po
FROM t;

-- The answer is the one the pose's own tgeompoint gives, turning points and all
WITH t AS (
  SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(4 4), 0.4)@2001-01-03]' AS lin1,
         geometry 'Polygon((0 0,0 2,2 2,2 0,0 0))' AS region
)
SELECT
  asText(tIntersects(lin1, region)) = asText(tIntersects(lin1::tgeompoint, region)) AS ti_agrees,
  asText(tDwithin(lin1, region, 1.0)) = asText(tDwithin(lin1::tgeompoint, region, 1.0)) AS tdw_agrees
FROM t;

-- A moving point neither contains nor covers a geometry, and two of them do
-- not touch, so those five signatures are not declared
SELECT to_regprocedure('tContains(tpose, geometry)') IS NULL AS tcontains_absent,
       to_regprocedure('tCovers(tpose, geometry)') IS NULL AS tcovers_absent,
       to_regprocedure('tTouches(tpose, tpose)') IS NULL AS ttouches_absent,
       to_regprocedure('tContains(geometry, tpose)') IS NOT NULL AS tcontains_present;

-------------------------------------------------------------------------------
