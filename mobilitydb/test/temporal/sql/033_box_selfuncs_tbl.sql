-------------------------------------------------------------------------------
-- A column of temporal or spatiotemporal boxes has no statistics for the
-- estimator, so the planner estimates the rows an operator selects from it
-- with the default selectivity of the operator: 0.005 for overlaps, 0.002 for
-- contains and contained, 0.001 for same, one third for a position operator,
-- and the rows of a join on it with the default join selectivity, 0.001.
-------------------------------------------------------------------------------

CREATE FUNCTION test_estimated_rows(query text)
RETURNS bigint LANGUAGE plpgsql AS $$
DECLARE
  plan json;
BEGIN
  EXECUTE 'EXPLAIN (FORMAT JSON) ' || query INTO plan;
  RETURN (plan -> 0 -> 'Plan' ->> 'Plan Rows')::bigint;
END;
$$;

DROP TABLE IF EXISTS tbl_tbox_sel;
CREATE TABLE tbl_tbox_sel AS
SELECT t1.k * 1000 + t2.k AS k, t1.b
FROM tbl_tboxfloat t1, generate_series(1, 100) t2(k);
ANALYZE tbl_tbox_sel;

SELECT op, test_estimated_rows(format(
  'SELECT * FROM tbl_tbox_sel WHERE b %s tbox %L', op,
  'TBOXFLOAT XT([0, 50],[2001-03-01, 2001-06-01])')) AS rows
FROM unnest(ARRAY['&&', '@>', '<@', '~=', '-|-', '<<', '&<', '>>', '&>',
  '<<#', '&<#', '#>>', '#&>']) WITH ORDINALITY AS t(op, n)
ORDER BY n;

SELECT test_estimated_rows(
  'SELECT * FROM tbl_tbox_sel WHERE b && tbox ''TBOXFLOAT X([0, 50])''') AS rows;
SELECT test_estimated_rows(
  'SELECT * FROM tbl_tbox_sel WHERE b && tbox ''TBOX T([2001-03-01, 2001-06-01])''')
  AS rows;
SELECT test_estimated_rows(
  'SELECT * FROM tbl_tbox_sel t1, tbl_tbox_sel t2 WHERE t1.b && t2.b') AS rows;

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_stbox_sel;
CREATE TABLE tbl_stbox_sel AS
SELECT k, stboxXT(xmin(b), xmin(b), xmax(b), xmax(b), b::tstzspan) AS b
FROM tbl_tbox_sel;
ANALYZE tbl_stbox_sel;

SELECT op, test_estimated_rows(format(
  'SELECT * FROM tbl_stbox_sel WHERE b %s stbox %L', op,
  'STBOX XT(((0,0),(50,50)),[2001-03-01, 2001-06-01])')) AS rows
FROM unnest(ARRAY['&&', '@>', '<@', '~=', '-|-', '<<', '&<', '>>', '&>',
  '<<|', '&<|', '|>>', '|&>', '<<#', '&<#', '#>>', '#&>'])
  WITH ORDINALITY AS t(op, n)
ORDER BY n;

SELECT test_estimated_rows(
  'SELECT * FROM tbl_stbox_sel WHERE b && stbox ''STBOX X((0,0),(50,50))''')
  AS rows;
SELECT test_estimated_rows(
  'SELECT * FROM tbl_stbox_sel WHERE b && stbox ''STBOX T([2001-03-01, 2001-06-01])''')
  AS rows;
SELECT test_estimated_rows(
  'SELECT * FROM tbl_stbox_sel t1, tbl_stbox_sel t2 WHERE t1.b && t2.b') AS rows;

DROP TABLE tbl_stbox_sel;
DROP TABLE tbl_tbox_sel;
DROP FUNCTION test_estimated_rows(text);

-------------------------------------------------------------------------------
