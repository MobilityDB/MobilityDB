-------------------------------------------------------------------------------
-- Temporal distance
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint, tbl_int WHERE i <-> temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_int WHERE i <-> temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_float WHERE f <-> temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_float WHERE f <-> temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_int WHERE temp <-> i IS NOT NULL;
SELECT count(*) FROM tbl_tint, tbl_float WHERE temp <-> f IS NOT NULL;

SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp <-> t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp <-> t2.temp IS NOT NULL;

SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp <-> i IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp <-> f IS NOT NULL;

SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp <-> t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp <-> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Nearest approach distance
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint, tbl_int WHERE i |=| temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_int WHERE i |=| temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_float WHERE f |=| temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_float WHERE f |=| temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_tbox WHERE b |=| temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE b |=| temp IS NOT NULL;
SELECT count(*) FROM tbl_tbox t1, tbl_tbox t2 WHERE t1.b |=| t2.b IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_int WHERE temp |=| i IS NOT NULL;
SELECT count(*) FROM tbl_tint, tbl_float WHERE temp |=| f IS NOT NULL;

SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp |=| t2.temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp |=| b IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp |=| b IS NOT NULL;

SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp |=| i IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp |=| f IS NOT NULL;

SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp |=| t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp |=| t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
