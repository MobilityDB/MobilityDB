-------------------------------------------------------------------------------
-- The planner estimates the rows a box-by-box operator of the tpcbox selects
-- with the spatiotemporal selectivity functions, as for the stbox: each
-- estimate is the default selectivity of the operator's kind (overlaps,
-- contains, same, position), never the half of the table the planner assumes
-- for an operator declaring no selectivity function.
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

DROP TABLE IF EXISTS tbl_tpcbox_sel;
CREATE TABLE tbl_tpcbox_sel AS
SELECT t1.k * 1000 + t2.k AS k, t1.b
FROM tbl_tpcbox t1, generate_series(1, 100) t2(k);
ANALYZE tbl_tpcbox_sel;

SELECT op, test_estimated_rows(format(
  'SELECT * FROM tbl_tpcbox_sel WHERE b %s tpcboxZT(0, 0, 0, 50, 50, 50, '
  'tstzspan ''[2001-03-01, 2001-06-01]'', 1)', op)) AS rows
FROM unnest(ARRAY['&&', '@>', '<@', '~=', '-|-', '<<', '&<', '>>', '&>',
  '<<|', '&<|', '|>>', '|&>', '<</', '&</', '/>>', '/&>', '<<#', '&<#', '#>>',
  '#&>']) WITH ORDINALITY AS t(op, n)
ORDER BY n;

SELECT test_estimated_rows(
  'SELECT * FROM tbl_tpcbox_sel t1, tbl_tpcbox_sel t2 WHERE t1.b && t2.b')
  AS rows;

DROP TABLE tbl_tpcbox_sel;
DROP FUNCTION test_estimated_rows(text);

-------------------------------------------------------------------------------
