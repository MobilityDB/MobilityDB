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
-- tnpoint to tgeompoint, then to tgeometry, and delegate to tgeometry's
-- temporal spatial-rel kernel (072_tgeo_tempspatialrels.in.sql). Each block
-- checks that the direct tnpoint overload agrees with the equivalent manual
-- cast chain (::tgeompoint::tgeometry).
--
-- The delegate target tgeometry only supports STEP interpolation, so a
-- multi-instant sequence operand below is written 'Interp=Step;...' (a
-- LINEAR tnpoint sequence errors on the ::tgeometry step of the chain, the
-- same documented restriction already exercised for tgeompoint/tgeogpoint
-- in geo/056_tgeo_spatialfuncs.test.sql). Single-instant values have no
-- interpolation and are unaffected.
--
-------------------------------------------------------------------------------

-- Test for NULL inputs since the functions are not STRICT
SELECT tContains(NULL::geometry, tnpoint 'Npoint(1, 0.5)@2000-01-01');
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
  asText(tContains(seq1, big_region)) = asText(tContains(seq1::tgeompoint::tgeometry, big_region)) AS tc_np_geo,
  asText(tContains(seq1, seq2)) = asText(tContains(seq1::tgeompoint::tgeometry, seq2::tgeompoint::tgeometry)) AS tc_np_np,
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
