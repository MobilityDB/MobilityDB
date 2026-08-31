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
-- Regression tests for the th3index temporal spatial-rel surface
-- (264_th3index_tempspatialrels.in.sql).
--
-- Each function returns a tbool whose value at instant t is the static
-- spatial relation applied to the boundary of the cell the value holds at t:
-- take the boundary with cellToBoundary, cast it to tgeometry, and delegate to
-- the temporal spatial-rel kernel of tgeometry
-- (072_tgeo_tempspatialrels.in.sql). Each block checks that the direct th3index
-- overload agrees with the equivalent manual chain.
--
-- The geometry operand is the boundary of one of the cells, so that the
-- relations answer over the same ground the cells cover.
--
-------------------------------------------------------------------------------

-- Test for NULL inputs since the functions are not STRICT
SELECT tIntersects(NULL::geometry, th3index '831c00fffffffff@2001-01-01');
SELECT tIntersects(getValue(cellToBoundary(th3index '831c00fffffffff@2001-01-01')::tgeometry), NULL::th3index);

-------------------------------------------------------------------------------
-- tContains, tCovers, tDisjoint, tIntersects, tTouches, tDwithin
-------------------------------------------------------------------------------

WITH t AS (
  SELECT th3index '{831c00fffffffff@2001-01-01, 831c02fffffffff@2001-01-02}' AS seq1,
         th3index '{831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02}' AS seq2,
         getValue(cellToBoundary(th3index '831c00fffffffff@2001-01-01')::tgeometry) AS cell_geom
)
SELECT
  asText(tContains(cell_geom, seq1)) = asText(tContains(cell_geom, cellToBoundary(seq1)::tgeometry)) AS tc_geo_c,
  asText(tContains(seq1, cell_geom)) = asText(tContains(cellToBoundary(seq1)::tgeometry, cell_geom)) AS tc_c_geo,
  asText(tContains(seq1, seq2)) = asText(tContains(cellToBoundary(seq1)::tgeometry, cellToBoundary(seq2)::tgeometry)) AS tc_c_c,
  asText(tCovers(cell_geom, seq1)) = asText(tCovers(cell_geom, cellToBoundary(seq1)::tgeometry)) AS tcv_geo_c,
  asText(tCovers(seq1, cell_geom)) = asText(tCovers(cellToBoundary(seq1)::tgeometry, cell_geom)) AS tcv_c_geo,
  asText(tCovers(seq1, seq2)) = asText(tCovers(cellToBoundary(seq1)::tgeometry, cellToBoundary(seq2)::tgeometry)) AS tcv_c_c,
  asText(tDisjoint(cell_geom, seq1)) = asText(tDisjoint(cell_geom, cellToBoundary(seq1)::tgeometry)) AS tdj_geo_c,
  asText(tDisjoint(seq1, cell_geom)) = asText(tDisjoint(cellToBoundary(seq1)::tgeometry, cell_geom)) AS tdj_c_geo,
  asText(tDisjoint(seq1, seq2)) = asText(tDisjoint(cellToBoundary(seq1)::tgeometry, cellToBoundary(seq2)::tgeometry)) AS tdj_c_c,
  asText(tIntersects(cell_geom, seq1)) = asText(tIntersects(cell_geom, cellToBoundary(seq1)::tgeometry)) AS ti_geo_c,
  asText(tIntersects(seq1, cell_geom)) = asText(tIntersects(cellToBoundary(seq1)::tgeometry, cell_geom)) AS ti_c_geo,
  asText(tIntersects(seq1, seq2)) = asText(tIntersects(cellToBoundary(seq1)::tgeometry, cellToBoundary(seq2)::tgeometry)) AS ti_c_c,
  asText(tTouches(cell_geom, seq1)) = asText(tTouches(cell_geom, cellToBoundary(seq1)::tgeometry)) AS ttc_geo_c,
  asText(tTouches(seq1, cell_geom)) = asText(tTouches(cellToBoundary(seq1)::tgeometry, cell_geom)) AS ttc_c_geo,
  asText(tTouches(seq1, seq2)) = asText(tTouches(cellToBoundary(seq1)::tgeometry, cellToBoundary(seq2)::tgeometry)) AS ttc_c_c,
  asText(tDwithin(cell_geom, seq1, 0.1)) = asText(tDwithin(cell_geom, cellToBoundary(seq1)::tgeometry, 0.1)) AS tdw_geo_c,
  asText(tDwithin(seq1, cell_geom, 0.1)) = asText(tDwithin(cellToBoundary(seq1)::tgeometry, cell_geom, 0.1)) AS tdw_c_geo,
  asText(tDwithin(seq1, seq2, 0.1)) = asText(tDwithin(cellToBoundary(seq1)::tgeometry, cellToBoundary(seq2)::tgeometry, 0.1)) AS tdw_c_c
FROM t;

-------------------------------------------------------------------------------
-- Direct values
-------------------------------------------------------------------------------

SELECT tIntersects(th3index '{831c00fffffffff@2001-01-01, 831c02fffffffff@2001-01-02}',
  th3index '{831c00fffffffff@2001-01-01, 831c02fffffffff@2001-01-02}');
SELECT tDisjoint(th3index '{831c00fffffffff@2001-01-01, 831c02fffffffff@2001-01-02}',
  th3index '{831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02}');

-------------------------------------------------------------------------------
