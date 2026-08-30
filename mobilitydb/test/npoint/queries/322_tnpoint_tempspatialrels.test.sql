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
-- Regression tests for the tnpoint temporal spatial-rel surface
-- (322_tnpoint_tempspatialrels.in.sql).
--
-- Each function returns a tbool whose value at instant t is the static
-- spatial relation applied to the network point resolved at t: cast the
-- tnpoint to tgeompoint and delegate to the temporal spatial-rel kernel the
-- geo family owns (072_tgeo_tempspatialrels.in.sql). A network point resolves
-- to a position, so tgeompoint is the cast target its geometry names; the
-- block below checks that each direct tnpoint overload agrees with the longer
-- manual chain (::tgeompoint::tgeometry) that reaches the same kernel.
--
-- The family declares the matrix its target declares: tContains and tCovers
-- take the geometry first only, and tTouches has no direction between two
-- tnpoints, because a moving point neither contains nor covers a geometry and
-- two moving points do not touch. The equivalence block below keeps its
-- sequences 'Interp=Step;...' so the longer chain it compares against, which
-- reaches tgeometry, can carry them.
--
-------------------------------------------------------------------------------

-- Test for NULL inputs since the functions are not STRICT
SELECT tContains(NULL::geometry, tnpoint 'Npoint(1, 0.5)@2001-01-01');
SELECT tContains(geometry 'SRID=5676;Point(1 1)', NULL::tnpoint);

-------------------------------------------------------------------------------
-- tContains, tCovers, tDisjoint, tIntersects, tTouches, tDwithin
-------------------------------------------------------------------------------

WITH t AS (
  SELECT tnpoint 'Interp=Step;[Npoint(1, 0.1)@2001-01-01, Npoint(1, 0.3)@2001-01-03]' AS seq1,
         tnpoint 'Interp=Step;[Npoint(2, 0.0)@2001-01-01, Npoint(2, 1)@2001-01-03]' AS seq2,
         geometry 'SRID=5676;Polygon((0 0,0 50,50 50,50 0,0 0))' AS big_region,
         geometry(npoint 'NPoint(2, 0.0)') AS far_point
)
SELECT
  asText(tContains(big_region, seq1)) = asText(tContains(big_region, seq1::tgeompoint::tgeometry)) AS tc_geo_np,
  asText(tCovers(big_region, seq1)) = asText(tCovers(big_region, seq1::tgeompoint::tgeometry)) AS tcv_geo_np,
  asText(tDisjoint(far_point, seq1)) = asText(tDisjoint(far_point, seq1::tgeompoint::tgeometry)) AS tdj_geo_np,
  asText(tIntersects(seq1, seq2)) = asText(tIntersects(seq1::tgeompoint::tgeometry, seq2::tgeompoint::tgeometry)) AS ti_np_np,
  asText(tTouches(big_region, seq1)) = asText(tTouches(big_region, seq1::tgeompoint::tgeometry)) AS ttc_geo_np,
  asText(tDwithin(seq1, seq2, 1.0)) = asText(tDwithin(seq1::tgeompoint::tgeometry, seq2::tgeompoint::tgeometry, 1.0)) AS tdw_np_np
FROM t;

-------------------------------------------------------------------------------
-- Direct values (not just cast-equivalence), matching the documented
-- manual-cast examples this file's functions now make direct
-------------------------------------------------------------------------------

SELECT tContains(geometry 'SRID=5676;Polygon((0 0,0 50,50 50,50 0,0 0))',
  tnpoint 'Interp=Step;[Npoint(1, 0.1)@2001-01-01, Npoint(1, 0.3)@2001-01-03]');
SELECT tDisjoint(geometry(npoint 'NPoint(2, 0.0)'),
  tnpoint 'Interp=Step;[Npoint(1, 0.1)@2001-01-01, Npoint(1, 0.3)@2001-01-03]');
SELECT tDwithin(
  tnpoint 'Interp=Step;[Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-03]',
  tnpoint 'Interp=Step;[Npoint(1, 0.5)@2001-01-01, Npoint(1, 0.3)@2001-01-03]', 1);

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Linear interpolation, which the tgeompoint target carries
-------------------------------------------------------------------------------

WITH t AS (
  SELECT tnpoint '[Npoint(1, 0.1)@2001-01-01, Npoint(1, 0.9)@2001-01-03]' AS lin1,
         tnpoint '[Npoint(1, 0.9)@2001-01-01, Npoint(1, 0.1)@2001-01-03]' AS lin2,
         geometry 'SRID=5676;Polygon((0 0,0 50,50 50,50 0,0 0))' AS region
)
SELECT
  asText(tIntersects(region, lin1)) AS ti_geo_np,
  asText(tIntersects(lin1, region)) AS ti_np_geo,
  asText(tDisjoint(lin1, region)) AS tdj_np_geo,
  asText(tDwithin(lin1, lin2, 1.0)) AS tdw_np_np
FROM t;

-- The answer is the one the network point's own tgeompoint gives
WITH t AS (
  SELECT tnpoint '[Npoint(1, 0.1)@2001-01-01, Npoint(1, 0.9)@2001-01-03]' AS lin1,
         geometry 'SRID=5676;Polygon((0 0,0 50,50 50,50 0,0 0))' AS region
)
SELECT
  asText(tIntersects(lin1, region)) = asText(tIntersects(lin1::tgeompoint, region)) AS ti_agrees,
  asText(tDwithin(lin1, region, 1.0)) = asText(tDwithin(lin1::tgeompoint, region, 1.0)) AS tdw_agrees
FROM t;

-- A moving point neither contains nor covers a geometry, and two of them do
-- not touch, so those five signatures are not declared
SELECT to_regprocedure('tContains(tnpoint, geometry)') IS NULL AS tcontains_absent,
       to_regprocedure('tCovers(tnpoint, geometry)') IS NULL AS tcovers_absent,
       to_regprocedure('tTouches(tnpoint, tnpoint)') IS NULL AS ttouches_absent,
       to_regprocedure('tContains(geometry, tnpoint)') IS NOT NULL AS tcontains_present;

-------------------------------------------------------------------------------
