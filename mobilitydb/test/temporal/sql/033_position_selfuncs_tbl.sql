-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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
