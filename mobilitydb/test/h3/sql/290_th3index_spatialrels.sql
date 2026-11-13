-------------------------------------------------------------------------------
-- Ever and always spatial relationships between a th3index and a geography,
-- or two th3index values.
--
-- An H3 cell is a region of the sphere bounded by great-circle arcs, so each
-- relationship is the one of the temporal geography of the cell boundary,
-- answered on the sphere; the operands are geographies and a distance is in
-- metres.
-------------------------------------------------------------------------------

-- A res-7 cell over Brussels (centre ~ 4.297, 50.804) and a region that
-- strictly contains it.
WITH t AS (
  SELECT th3index '871fa44a8ffffff@2001-01-01' AS cell_inst,
         geography 'POLYGON((4.20 50.70, 4.50 50.70, 4.50 50.90, 4.20 50.90, 4.20 50.70))' AS big_region,
         geography 'POLYGON((10.00 50.00, 11.00 50.00, 11.00 51.00, 10.00 51.00, 10.00 50.00))' AS far_region,
         geography 'POINT(4.297 50.804)' AS centre_point
)
SELECT
  eIntersects(big_region, cell_inst)  AS ei_geo_h3,
  eIntersects(cell_inst, big_region)  AS ei_h3_geo,
  eIntersects(cell_inst, cell_inst)   AS ei_h3_h3_self,
  eDisjoint  (far_region, cell_inst)  AS edj_far_geo,
  eDisjoint  (cell_inst, far_region)  AS edj_h3_far,
  eDwithin   (centre_point, cell_inst, 1000.0) AS edw_centre,
  eDwithin   (cell_inst, far_region, 1000.0) AS edw_far
FROM t;

-------------------------------------------------------------------------------
-- An empty geography is ever reached by nothing, in either order
-------------------------------------------------------------------------------

SELECT eIntersects(geography 'Point EMPTY', th3index '871fa44a8ffffff@2001-01-01');
SELECT eIntersects(th3index '871fa44a8ffffff@2001-01-01', geography 'Point EMPTY');
SELECT eDwithin(geography 'Point EMPTY', th3index '871fa44a8ffffff@2001-01-01', 1000.0);
SELECT eDwithin(th3index '871fa44a8ffffff@2001-01-01', geography 'Point EMPTY', 1000.0);

-------------------------------------------------------------------------------
-- Always variants: at every instant the cell lies in the big region and away
-- from the far one.
-------------------------------------------------------------------------------

WITH t AS (
  SELECT th3index '[871fa44a8ffffff@2001-01-01, 871fa44b1ffffff@2001-01-02]' AS cell_seq,
         geography 'POLYGON((4.20 50.70, 4.50 50.70, 4.50 50.90, 4.20 50.90, 4.20 50.70))' AS big_region,
         geography 'POLYGON((10.00 50.00, 11.00 50.00, 11.00 51.00, 10.00 51.00, 10.00 50.00))' AS far_region
)
SELECT
  aIntersects(big_region, cell_seq)  AS ai_big,
  aDisjoint  (far_region, cell_seq)  AS adj_far,
  aDwithin   (cell_seq, cell_seq, 1.0) AS adw_self
FROM t;

-------------------------------------------------------------------------------
-- A geodetic cell is the region H3 assigns its points to, which eIntersects
-- of a geography reads; the cell of a geometry point is the planar cell, the
-- polygon joining its vertices with straight lines in longitude and latitude,
-- which latLngToCell of a geometry reads. The first point lies in the
-- resolution-0 cell of the queries below and 91 km outside that polygon; the
-- second lies 53 km inside the polygon and outside the cell.
-------------------------------------------------------------------------------

SELECT latLngToCell(geometry 'SRID=4326;POINT(16.6328 71.9281)', 0) = h3index '8009fffffffffff',
  eIntersects(th3index '8009fffffffffff@2001-01-01', geography 'POINT(16.6328 71.9281)'),
  eDisjoint(th3index '8009fffffffffff@2001-01-01', geography 'POINT(16.6328 71.9281)');
SELECT latLngToCell(geometry 'SRID=4326;POINT(-0.8367 59.2711)', 0) = h3index '8009fffffffffff',
  eIntersects(th3index '8009fffffffffff@2001-01-01', geography 'POINT(-0.8367 59.2711)'),
  eDisjoint(th3index '8009fffffffffff@2001-01-01', geography 'POINT(-0.8367 59.2711)');

-------------------------------------------------------------------------------
-- A temporal geography declares no contains relationship, and neither does
-- a th3index; a geometry operand is read as the geography it converts to.
-------------------------------------------------------------------------------

SELECT eContains(geography 'POINT(4.297 50.804)', th3index '871fa44a8ffffff@2001-01-01');
SELECT eIntersects(th3index '871fa44a8ffffff@2001-01-01', geometry 'SRID=4326;POINT(4.297 50.804)');

-------------------------------------------------------------------------------
-- Symmetry: the relationship of two cells does not depend on their order.
-------------------------------------------------------------------------------

WITH t AS (
  SELECT th3index '[871fa44a8ffffff@2001-01-01, 871fa44b1ffffff@2001-01-02]' AS seq1,
         th3index '[871fa44b1ffffff@2001-01-01, 871fa44a8ffffff@2001-01-02]' AS seq2
)
SELECT eIntersects(seq1, seq2) = eIntersects(seq2, seq1) AS symmetric
FROM t;

-------------------------------------------------------------------------------
