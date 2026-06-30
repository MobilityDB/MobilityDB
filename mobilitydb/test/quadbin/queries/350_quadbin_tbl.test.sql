-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- Table-based tests for the static quadbin type, exercised over tbl_quadbin.

-------------------------------------------------------------------------------
-- Send / receive
-------------------------------------------------------------------------------

COPY tbl_quadbin TO '/tmp/tbl_quadbin' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_quadbin_tmp;
CREATE TABLE tbl_quadbin_tmp AS TABLE tbl_quadbin WITH NO DATA;
COPY tbl_quadbin_tmp FROM '/tmp/tbl_quadbin' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_quadbin t1, tbl_quadbin_tmp t2 WHERE t1.k = t2.k AND t1.qb <> t2.qb;
DROP TABLE tbl_quadbin_tmp;

-------------------------------------------------------------------------------
-- Validity predicates
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_quadbin WHERE NOT isValidCell(qb);

-------------------------------------------------------------------------------
-- Casts to / from bigint (assignment)
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_quadbin WHERE (qb::bigint)::quadbin <> qb;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_quadbin t1, tbl_quadbin t2 WHERE t1.qb = t2.qb;
SELECT COUNT(*) FROM tbl_quadbin t1, tbl_quadbin t2 WHERE t1.qb <> t2.qb;
SELECT COUNT(*) FROM tbl_quadbin t1, tbl_quadbin t2 WHERE t1.qb < t2.qb;
SELECT COUNT(*) FROM tbl_quadbin t1, tbl_quadbin t2 WHERE t1.qb <= t2.qb;
SELECT COUNT(*) FROM tbl_quadbin t1, tbl_quadbin t2 WHERE t1.qb > t2.qb;
SELECT COUNT(*) FROM tbl_quadbin t1, tbl_quadbin t2 WHERE t1.qb >= t2.qb;

-------------------------------------------------------------------------------
