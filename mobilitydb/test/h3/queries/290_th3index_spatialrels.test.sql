-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- Regression tests for the th3index spatial-rel surface
-- (290_th3index_spatialrels.in.sql).
--
-- Each block exercises one relation in all three argument shapes
-- (geometry/th3index, th3index/geometry, th3index/th3index) for both
-- the ever variant (returns bool) and the temporal variant (returns
-- tbool). Cell semantics are CELL-BOUNDARY (the cell's hexagonal
-- footprint), not cell-centre, so a query polygon that touches the
-- cell anywhere intersects it.
--
-- The cells used below are real Brussels-area H3 res-7 cells so any
-- geographer can verify the results from coordinates alone.
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Setup
-------------------------------------------------------------------------------

-- A H3 res-7 cell over Brussels (centre ~ 4.297, 50.804) and a region
-- polygon that strictly contains it.
WITH t AS (
  SELECT th3index '871fa44a8ffffff@2001-01-01' AS cell_inst,
         th3index '[871fa44a8ffffff@2001-01-01, 871fa44b1ffffff@2001-01-02]' AS cell_seq,
         geometry 'SRID=4326;POLYGON((4.20 50.70, 4.50 50.70, 4.50 50.90, 4.20 50.90, 4.20 50.70))' AS big_region,
         geometry 'SRID=4326;POLYGON((10.00 50.00, 11.00 50.00, 11.00 51.00, 10.00 51.00, 10.00 50.00))' AS far_region,
         geometry 'SRID=4326;POINT(4.297 50.804)' AS centre_point
)
SELECT
  -- Ever variants: scalar boolean
  eIntersects(big_region, cell_inst)  AS ei_geo_h3,
  eIntersects(cell_inst, big_region)  AS ei_h3_geo,
  eIntersects(cell_inst, cell_inst)   AS ei_h3_h3_self,
  eDisjoint  (far_region, cell_inst)  AS edj_far_geo,
  eContains  (big_region, cell_inst)  AS ec_big_contains_cell,
  eCovers    (big_region, cell_inst)  AS ecv_big_covers_cell,
  eDwithin   (centre_point, cell_inst, 1.0) AS edw_centre,
  -- Temporal variants: tbool over the cell's time period
  asText(tIntersects(big_region, cell_seq)) AS ti_seq,
  asText(tDisjoint(far_region, cell_seq))   AS tdj_seq,
  asText(tContains(big_region, cell_seq))   AS tc_seq
FROM t;

-------------------------------------------------------------------------------
-- Always variants — at every instant the cell is contained in the
-- big region, never contained in the far region.
-------------------------------------------------------------------------------

WITH t AS (
  SELECT th3index '[871fa44a8ffffff@2001-01-01, 871fa44b1ffffff@2001-01-02]' AS cell_seq,
         geometry 'SRID=4326;POLYGON((4.20 50.70, 4.50 50.70, 4.50 50.90, 4.20 50.90, 4.20 50.70))' AS big_region,
         geometry 'SRID=4326;POLYGON((10.00 50.00, 11.00 50.00, 11.00 51.00, 10.00 51.00, 10.00 50.00))' AS far_region
)
SELECT
  aIntersects(big_region, cell_seq)  AS ai_big,
  aDisjoint  (far_region, cell_seq)  AS adj_far,
  aContains  (big_region, cell_seq)  AS ac_big,
  aCovers    (big_region, cell_seq)  AS acv_big,
  aDwithin   (big_region, cell_seq, 0.001) AS adw_close
FROM t;

-------------------------------------------------------------------------------
-- Symmetry: tIntersects is symmetric (modulo arg order), so the two
-- swapped queries must agree at every instant.
-------------------------------------------------------------------------------

WITH t AS (
  SELECT th3index '[871fa44a8ffffff@2001-01-01, 871fa44b1ffffff@2001-01-02]' AS cell_seq,
         geometry 'SRID=4326;POLYGON((4.20 50.70, 4.50 50.70, 4.50 50.90, 4.20 50.90, 4.20 50.70))' AS big_region
)
SELECT asText(tIntersects(big_region, cell_seq)) = asText(tIntersects(cell_seq, big_region)) AS symmetric
FROM t;

-------------------------------------------------------------------------------
