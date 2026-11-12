-------------------------------------------------------------------------------
-- A position operator with the constant on its left is estimated as its
-- commutator with the column on its left, so the two estimates of each row
-- below are equal.
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

DROP TABLE IF EXISTS tbl_tfloat_possel;
CREATE TABLE tbl_tfloat_possel AS
SELECT t1.k * 1000 + t2.k AS k, t1.temp
FROM tbl_tfloat t1, generate_series(1, 100) t2(k);
ANALYZE tbl_tfloat_possel;

DROP TABLE IF EXISTS tbl_tbool_possel;
CREATE TABLE tbl_tbool_possel AS
SELECT t1.k * 1000 + t2.k AS k, t1.temp
FROM tbl_tbool t1, generate_series(1, 100) t2(k);
ANALYZE tbl_tbool_possel;

SELECT op, comm,
  test_estimated_rows(format(
    'SELECT * FROM tbl_tfloat_possel WHERE temp %s tbox %L', op,
    'TBOXFLOAT XT([30, 60],[2001-04-01, 2001-08-01])')) AS var_op_const,
  test_estimated_rows(format(
    'SELECT * FROM tbl_tfloat_possel WHERE tbox %L %s temp',
    'TBOXFLOAT XT([30, 60],[2001-04-01, 2001-08-01])', comm)) AS const_comm_var
FROM (VALUES (1, '<<', '>>'), (2, '>>', '<<'), (3, '<<#', '#>>'),
  (4, '#>>', '<<#')) AS t(n, op, comm)
ORDER BY n;

SELECT op, comm,
  test_estimated_rows(format(
    'SELECT * FROM tbl_tbool_possel WHERE temp %s tstzspan %L', op,
    '[2001-04-01, 2001-08-01]')) AS var_op_const,
  test_estimated_rows(format(
    'SELECT * FROM tbl_tbool_possel WHERE tstzspan %L %s temp',
    '[2001-04-01, 2001-08-01]', comm)) AS const_comm_var
FROM (VALUES (1, '<<#', '#>>'), (2, '#>>', '<<#')) AS t(n, op, comm)
ORDER BY n;

-- A position operator is estimated on the dimension it compares, so a box of
-- both dimensions estimates the rows of the box of that dimension alone, and
-- a box without it the default selectivity of the operator

SELECT op,
  test_estimated_rows(format(
    'SELECT * FROM tbl_tfloat_possel WHERE temp %s tbox %L', op,
    'TBOXFLOAT XT([30, 60],[2001-04-01, 2001-08-01])')) AS box_xt,
  test_estimated_rows(format(
    'SELECT * FROM tbl_tfloat_possel WHERE temp %s tbox %L', op,
    'TBOXFLOAT X([30, 60])')) AS box_x,
  test_estimated_rows(format(
    'SELECT * FROM tbl_tfloat_possel WHERE temp %s tbox %L', op,
    'TBOX T([2001-04-01, 2001-08-01])')) AS box_t
FROM unnest(ARRAY['<<', '&<', '>>', '&>', '<<#', '&<#', '#>>', '#&>'])
  WITH ORDINALITY AS t(op, n)
ORDER BY n;

DROP TABLE tbl_tbool_possel;
DROP TABLE tbl_tfloat_possel;
DROP FUNCTION test_estimated_rows(text);

-------------------------------------------------------------------------------
