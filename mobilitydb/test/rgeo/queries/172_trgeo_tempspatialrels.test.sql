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
-- Regression tests for the trgeometry temporal spatial-rel surface
-- (172_trgeo_tempspatialrels.in.sql).
--
-- Each function returns a tbool whose value at instant t is the static
-- spatial relation applied to the reference geometry placed at the pose the
-- temporal rigid geometry holds at t: cast the trgeometry to tgeometry and
-- delegate to the temporal spatial-rel kernel of tgeometry
-- (072_tgeo_tempspatialrels.in.sql). Each block checks that the direct
-- trgeometry overload agrees with the equivalent manual cast chain
-- (::tgeometry).
--
-- The cast keeps the value the trgeometry holds at each instant, so a
-- sequence written with linear interpolation casts to the same temporal
-- geometry as the one written with step interpolation, and the operands
-- below carry step interpolation to state that granularity.
--
-------------------------------------------------------------------------------

-- Test for NULL inputs since the functions are not STRICT
SELECT tContains(NULL::geometry, trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));Pose(Point(0 0),0)@2001-01-01');
SELECT tContains(geometry 'Polygon((-1 -1,-1 5,10 5,10 -1,-1 -1))', NULL::trgeometry);

-------------------------------------------------------------------------------
-- tContains, tCovers, tDisjoint, tIntersects, tTouches, tDwithin
-------------------------------------------------------------------------------

WITH t AS (
  SELECT trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-03]' AS seq1,
         trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(2 0),0)@2001-01-01, Pose(Point(2 2),0)@2001-01-03]' AS seq2,
         geometry 'Polygon((-1 -1,-1 5,10 5,10 -1,-1 -1))' AS big_region,
         geometry 'Point(50 50)' AS far_point
)
SELECT
  asText(tContains(big_region, seq1)) = asText(tContains(big_region, seq1::tgeometry)) AS tc_geo_rg,
  asText(tContains(seq1, big_region)) = asText(tContains(seq1::tgeometry, big_region)) AS tc_rg_geo,
  asText(tContains(seq1, seq2)) = asText(tContains(seq1::tgeometry, seq2::tgeometry)) AS tc_rg_rg,
  asText(tCovers(big_region, seq1)) = asText(tCovers(big_region, seq1::tgeometry)) AS tcv_geo_rg,
  asText(tCovers(seq1, big_region)) = asText(tCovers(seq1::tgeometry, big_region)) AS tcv_rg_geo,
  asText(tCovers(seq1, seq2)) = asText(tCovers(seq1::tgeometry, seq2::tgeometry)) AS tcv_rg_rg,
  asText(tDisjoint(far_point, seq1)) = asText(tDisjoint(far_point, seq1::tgeometry)) AS tdj_geo_rg,
  asText(tDisjoint(seq1, far_point)) = asText(tDisjoint(seq1::tgeometry, far_point)) AS tdj_rg_geo,
  asText(tDisjoint(seq1, seq2)) = asText(tDisjoint(seq1::tgeometry, seq2::tgeometry)) AS tdj_rg_rg,
  asText(tIntersects(big_region, seq1)) = asText(tIntersects(big_region, seq1::tgeometry)) AS ti_geo_rg,
  asText(tIntersects(seq1, big_region)) = asText(tIntersects(seq1::tgeometry, big_region)) AS ti_rg_geo,
  asText(tIntersects(seq1, seq2)) = asText(tIntersects(seq1::tgeometry, seq2::tgeometry)) AS ti_rg_rg,
  asText(tTouches(big_region, seq1)) = asText(tTouches(big_region, seq1::tgeometry)) AS ttc_geo_rg,
  asText(tTouches(seq1, big_region)) = asText(tTouches(seq1::tgeometry, big_region)) AS ttc_rg_geo,
  asText(tTouches(seq1, seq2)) = asText(tTouches(seq1::tgeometry, seq2::tgeometry)) AS ttc_rg_rg,
  asText(tDwithin(big_region, seq1, 1.0)) = asText(tDwithin(big_region, seq1::tgeometry, 1.0)) AS tdw_geo_rg,
  asText(tDwithin(seq1, big_region, 1.0)) = asText(tDwithin(seq1::tgeometry, big_region, 1.0)) AS tdw_rg_geo,
  asText(tDwithin(seq1, seq2, 1.0)) = asText(tDwithin(seq1::tgeometry, seq2::tgeometry, 1.0)) AS tdw_rg_rg
FROM t;

-------------------------------------------------------------------------------
-- Direct values
-------------------------------------------------------------------------------

SELECT tContains(geometry 'Polygon((-1 -1,-1 5,10 5,10 -1,-1 -1))',
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-03]');
SELECT tCovers(geometry 'Polygon((-1 -1,-1 5,10 5,10 -1,-1 -1))',
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-03]');
SELECT tDisjoint(geometry 'Point(50 50)',
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-03]');
SELECT tIntersects(
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-03]',
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(2 0),0)@2001-01-01, Pose(Point(2 2),0)@2001-01-03]');
SELECT tDwithin(
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-03]',
  trgeometry 'Interp=Step;Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(2 0),0)@2001-01-01, Pose(Point(2 2),0)@2001-01-03]', 1.0);

-------------------------------------------------------------------------------
