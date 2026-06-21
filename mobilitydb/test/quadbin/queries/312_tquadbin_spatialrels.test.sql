-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
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

-- Spatial relationships for tquadbin via the cell-boundary geometry.
-- cell '48a6227affffffff' is a res-10 tile over Brussels with bbox
-- x[4.219, 4.570], y[50.736, 50.958] (SRID 4326). big_region strictly
-- contains that bbox; far_region is on another continent (disjoint);
-- centre_point lies inside the cell. Relationships are known by construction.

WITH t AS (
  SELECT tquadbin '48a6227affffffff@2001-01-01' AS cell_inst,
         tquadbin '[48a6227affffffff@2001-01-01, 48a6227affffffff@2001-01-02]' AS cell_seq,
         geometry 'SRID=4326;POLYGON((4.0 50.6, 4.7 50.6, 4.7 51.0, 4.0 51.0, 4.0 50.6))' AS big_region,
         geometry 'SRID=4326;POLYGON((100 10, 110 10, 110 20, 100 20, 100 10))' AS far_region,
         geometry 'SRID=4326;POINT(4.4 50.85)' AS centre_point
)
SELECT
  eIntersects(big_region, cell_inst)        AS ei_geo_cell,
  eIntersects(cell_inst, big_region)        AS ei_cell_geo,
  eIntersects(cell_inst, cell_inst)         AS ei_self,
  eDisjoint  (far_region, cell_inst)        AS edj_far,
  eContains  (big_region, cell_inst)        AS ec_contains,
  eCovers    (big_region, cell_inst)        AS ecv_covers,
  eDwithin   (centre_point, cell_inst, 1.0) AS edw_inside,
  asText(tIntersects(big_region, cell_seq)) AS ti_seq,
  asText(tDisjoint(far_region, cell_seq))   AS tdj_seq,
  asText(tContains(big_region, cell_seq))   AS tc_seq
FROM t;

-- Always variants: constant-cell sequence -> always contained / never far
WITH t AS (
  SELECT tquadbin '[48a6227affffffff@2001-01-01, 48a6227affffffff@2001-01-02]' AS cell_seq,
         geometry 'SRID=4326;POLYGON((4.0 50.6, 4.7 50.6, 4.7 51.0, 4.0 51.0, 4.0 50.6))' AS big_region,
         geometry 'SRID=4326;POLYGON((100 10, 110 10, 110 20, 100 20, 100 10))' AS far_region
)
SELECT
  aContains (big_region, cell_seq) AS ac_always_contains,
  aDisjoint (far_region, cell_seq) AS adj_always_far,
  aIntersects(big_region, cell_seq) AS ai_always_intersects
FROM t;

-------------------------------------------------------------------------------
