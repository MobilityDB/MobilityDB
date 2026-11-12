-------------------------------------------------------------------------------
-- Regression tests for the th3index temporal spatial relationships
-- (264_th3index_tempspatialrels.in.sql).
--
-- Each function returns a tbool whose value at instant t is the static
-- spatial relationship of the boundaries of the cells the values hold at t:
-- the boundary is the temporal geography cellToBoundary returns, and the
-- relationship is the one of two temporal geographies, answered on the
-- sphere. Each block checks that the th3index overload agrees with that
-- chain written out.
-------------------------------------------------------------------------------

-- Test for NULL inputs since the functions are not STRICT
SELECT tIntersects(NULL::th3index, th3index '831c00fffffffff@2001-01-01');
SELECT tIntersects(th3index '831c00fffffffff@2001-01-01', NULL::th3index);

-------------------------------------------------------------------------------
-- tDisjoint, tIntersects
-------------------------------------------------------------------------------

WITH t AS (
  SELECT th3index '{831c00fffffffff@2001-01-01, 831c02fffffffff@2001-01-02}' AS seq1,
         th3index '{831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02}' AS seq2
)
SELECT
  asText(tDisjoint(seq1, seq2)) = asText(tDisjoint(cellToBoundary(seq1), cellToBoundary(seq2))) AS tdj_c_c,
  asText(tIntersects(seq1, seq2)) = asText(tIntersects(cellToBoundary(seq1), cellToBoundary(seq2))) AS ti_c_c
FROM t;

-------------------------------------------------------------------------------
-- Direct values
-------------------------------------------------------------------------------

SELECT tIntersects(th3index '{831c00fffffffff@2001-01-01, 831c02fffffffff@2001-01-02}',
  th3index '{831c00fffffffff@2001-01-01, 831c02fffffffff@2001-01-02}');
SELECT tDisjoint(th3index '{831c00fffffffff@2001-01-01, 831c02fffffffff@2001-01-02}',
  th3index '{831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02}');

-------------------------------------------------------------------------------
