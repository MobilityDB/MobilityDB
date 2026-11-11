-------------------------------------------------------------------------------
-- Regression tests for the tquadbin temporal spatial-rel surface
-- (364_tquadbin_tempspatialrels.in.sql).
--
-- Each function returns a tbool whose value at instant t is the static
-- spatial relation applied to the boundary of the cell the value holds at t:
-- take the boundary with cellToBoundary, cast it to tgeometry, and delegate to
-- the temporal spatial-rel kernel of tgeometry
-- (072_tgeo_tempspatialrels.in.sql). Each block checks that the direct tquadbin
-- overload agrees with the equivalent manual chain.
--
-- The geometry operand is the boundary of one of the cells, so that the
-- relations answer over the same ground the cells cover.
--
-------------------------------------------------------------------------------

-- Test for NULL inputs since the functions are not STRICT
SELECT tIntersects(NULL::geometry, tquadbin '480fffffffffffff@2001-01-01');
SELECT tIntersects(getValue(cellToBoundary(tquadbin '480fffffffffffff@2001-01-01')::tgeometry), NULL::tquadbin);

-------------------------------------------------------------------------------
-- tContains, tCovers, tDisjoint, tIntersects, tTouches, tDwithin
-------------------------------------------------------------------------------

WITH t AS (
  SELECT tquadbin '{480fffffffffffff@2001-01-01, 481fffffffffffff@2001-01-02}' AS seq1,
         tquadbin '{481fffffffffffff@2001-01-01, 480fffffffffffff@2001-01-02}' AS seq2,
         getValue(cellToBoundary(tquadbin '480fffffffffffff@2001-01-01')::tgeometry) AS cell_geom
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

SELECT tIntersects(tquadbin '{480fffffffffffff@2001-01-01, 481fffffffffffff@2001-01-02}',
  tquadbin '{480fffffffffffff@2001-01-01, 481fffffffffffff@2001-01-02}');
SELECT tDisjoint(tquadbin '{480fffffffffffff@2001-01-01, 481fffffffffffff@2001-01-02}',
  tquadbin '{481fffffffffffff@2001-01-01, 480fffffffffffff@2001-01-02}');

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- The conversions between geometry and geography
-------------------------------------------------------------------------------

SELECT asEWKT(tquadbin '480fffffffffffff@2001-01-01'::tgeogpoint, 6);
SELECT asEWKT(tquadbin '480fffffffffffff@2001-01-01'::tgeompoint, 6);
