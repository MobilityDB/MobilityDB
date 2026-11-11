-------------------------------------------------------------------------------
-- Temporal addition
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i + temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp + i IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f + temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp + f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp + t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp + t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal subtraction
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i - temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp - i IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f - temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp - f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp - t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp - t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal multiplication
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i * temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp * i IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f * temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp * f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp * t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp * t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal division
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE i / temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tint, tbl_int WHERE temp / i IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE f / temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat, tbl_float WHERE temp / f IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp / t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp / t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal round, deltaValue, trend, derivative
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE deltaValue(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat WHERE deltaValue(temp) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat_seq WHERE trend(seq) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat_seqset WHERE trend(ss) IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal round, degrees, radians, abs, ln, log10
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tint WHERE abs(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat WHERE abs(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat_step_seq WHERE abs(seq) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat WHERE round(temp, 1) IS NOT NULL;
SELECT COUNT(*) FROM tbl_float WHERE degrees(f, true) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat WHERE degrees(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat WHERE radians(temp) IS NOT NULL;
SELECT round(MAX(maxValue(derivative(temp))), 6) FROM tbl_tfloat WHERE interp(temp) = 'Linear';

SELECT COUNT(*) FROM tbl_tfloat WHERE exp(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat WHERE ln(abs(temp)) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat WHERE log10(abs(temp)) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tfloat WHERE sin(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat WHERE cos(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat WHERE tan(temp / 100.0) IS NOT NULL;

SELECT COUNT(*) FROM tbl_tint WHERE deltaValue(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_tfloat WHERE deltaValue(temp) IS NOT NULL;


SELECT round(array_agg(inst ORDER BY k), 2) FROM tbl_tfloat_inst WHERE inst IS NOT NULL AND k % 20 = 1;

-------------------------------------------------------------------------------

