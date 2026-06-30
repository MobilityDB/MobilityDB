-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-------------------------------------------------------------------------------

-- Table-based tests for the static h3index type, exercised over tbl_h3index.

-------------------------------------------------------------------------------
-- Send / receive
-------------------------------------------------------------------------------

COPY tbl_h3index TO '/tmp/tbl_h3index' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_h3index_tmp;
CREATE TABLE tbl_h3index_tmp AS TABLE tbl_h3index WITH NO DATA;
COPY tbl_h3index_tmp FROM '/tmp/tbl_h3index' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_h3index t1, tbl_h3index_tmp t2 WHERE t1.k = t2.k AND t1.h3 <> t2.h3;
DROP TABLE tbl_h3index_tmp;

-------------------------------------------------------------------------------
-- (Hex)WKB round trip
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_h3index WHERE h3indexFromBinary(asBinary(h3)) <> h3;
SELECT COUNT(*) FROM tbl_h3index WHERE h3indexFromHexWKB(asHexWKB(h3)) <> h3;

-------------------------------------------------------------------------------
-- Validity predicates
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_h3index WHERE NOT isValidCell(h3);

-------------------------------------------------------------------------------
-- Casts to / from bigint (assignment)
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_h3index WHERE (h3::bigint)::h3index <> h3;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_h3index t1, tbl_h3index t2 WHERE t1.h3 = t2.h3;
SELECT COUNT(*) FROM tbl_h3index t1, tbl_h3index t2 WHERE t1.h3 <> t2.h3;
SELECT COUNT(*) FROM tbl_h3index t1, tbl_h3index t2 WHERE t1.h3 < t2.h3;
SELECT COUNT(*) FROM tbl_h3index t1, tbl_h3index t2 WHERE t1.h3 <= t2.h3;
SELECT COUNT(*) FROM tbl_h3index t1, tbl_h3index t2 WHERE t1.h3 > t2.h3;
SELECT COUNT(*) FROM tbl_h3index t1, tbl_h3index t2 WHERE t1.h3 >= t2.h3;

-------------------------------------------------------------------------------
