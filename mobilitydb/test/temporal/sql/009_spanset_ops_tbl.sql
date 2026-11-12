-------------------------------------------------------------------------------
-- File span_ops.c
-- Tests of operators that do not involve indexes for span types.
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i + t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i + t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i + t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i + t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i + t2.i IS NOT NULL;

SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b + t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b + t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b + t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b + t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b + t2.b IS NOT NULL;

SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f + t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f + t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f + t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f + t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f + t2.f IS NOT NULL;

SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d + t2.d IS NOT NULL;
SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d + t2.d IS NOT NULL;
SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d + t2.d IS NOT NULL;
SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d + t2.d IS NOT NULL;
SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d + t2.d IS NOT NULL;

SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t + t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t + t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t + t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t + t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t + t2.t IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i - t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i - t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i - t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i - t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i - t2.i IS NOT NULL;

SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b - t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b - t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b - t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b - t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b - t2.b IS NOT NULL;

SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f - t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f - t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f - t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f - t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f - t2.f IS NOT NULL;

SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d - t2.d IS NOT NULL;
SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d - t2.d IS NOT NULL;
SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d - t2.d IS NOT NULL;
SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d - t2.d IS NOT NULL;
SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d - t2.d IS NOT NULL;

SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t - t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t - t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t - t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t - t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t - t2.t IS NOT NULL;

-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_int t1, tbl_intspanset t2 WHERE t1.i * t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspan t1, tbl_intspanset t2 WHERE t1.i * t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_int t2 WHERE t1.i * t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspan t2 WHERE t1.i * t2.i IS NOT NULL;
SELECT COUNT(*) FROM tbl_intspanset t1, tbl_intspanset t2 WHERE t1.i * t2.i IS NOT NULL;

SELECT COUNT(*) FROM tbl_bigint t1, tbl_bigintspanset t2 WHERE t1.b * t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspan t1, tbl_bigintspanset t2 WHERE t1.b * t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigint t2 WHERE t1.b * t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspan t2 WHERE t1.b * t2.b IS NOT NULL;
SELECT COUNT(*) FROM tbl_bigintspanset t1, tbl_bigintspanset t2 WHERE t1.b * t2.b IS NOT NULL;

SELECT COUNT(*) FROM tbl_float t1, tbl_floatspanset t2 WHERE t1.f * t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspan t1, tbl_floatspanset t2 WHERE t1.f * t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_float t2 WHERE t1.f * t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspan t2 WHERE t1.f * t2.f IS NOT NULL;
SELECT COUNT(*) FROM tbl_floatspanset t1, tbl_floatspanset t2 WHERE t1.f * t2.f IS NOT NULL;

SELECT COUNT(*) FROM tbl_date t1, tbl_datespanset t2 WHERE t1.d * t2.d IS NOT NULL;
SELECT COUNT(*) FROM tbl_datespan t1, tbl_datespanset t2 WHERE t1.d * t2.d IS NOT NULL;
SELECT COUNT(*) FROM tbl_datespanset t1, tbl_date t2 WHERE t1.d * t2.d IS NOT NULL;
SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespan t2 WHERE t1.d * t2.d IS NOT NULL;
SELECT COUNT(*) FROM tbl_datespanset t1, tbl_datespanset t2 WHERE t1.d * t2.d IS NOT NULL;

SELECT COUNT(*) FROM tbl_timestamptz t1, tbl_tstzspanset t2 WHERE t1.t * t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.t * t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_timestamptz t2 WHERE t1.t * t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.t * t2.t IS NOT NULL;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.t * t2.t IS NOT NULL;

-------------------------------------------------------------------------------

SELECT MIN(t1.i <-> t2.i) FROM tbl_int t1, tbl_intspanset t2;
SELECT MIN(t1.i <-> t2.i) FROM tbl_intspan t1, tbl_intspanset t2;
SELECT MIN(t1.i <-> t2.i) FROM tbl_intspanset t1, tbl_int t2;
SELECT MIN(t1.i <-> t2.i) FROM tbl_intspanset t1, tbl_intspan t2;
SELECT MIN(t1.i <-> t2.i) FROM tbl_intspanset t1, tbl_intspanset t2;

SELECT MIN(t1.b <-> t2.b) FROM tbl_bigint t1, tbl_bigintspanset t2;
SELECT MIN(t1.b <-> t2.b) FROM tbl_bigintspan t1, tbl_bigintspanset t2;
SELECT MIN(t1.b <-> t2.b) FROM tbl_bigintspanset t1, tbl_bigint t2;
SELECT MIN(t1.b <-> t2.b) FROM tbl_bigintspanset t1, tbl_bigintspan t2;
SELECT MIN(t1.b <-> t2.b) FROM tbl_bigintspanset t1, tbl_bigintspanset t2;

SELECT MIN(t1.f <-> t2.f) FROM tbl_float t1, tbl_floatspanset t2;
SELECT MIN(t1.f <-> t2.f) FROM tbl_floatspan t1, tbl_floatspanset t2;
SELECT MIN(t1.f <-> t2.f) FROM tbl_floatspanset t1, tbl_float t2;
SELECT MIN(t1.f <-> t2.f) FROM tbl_floatspanset t1, tbl_floatspan t2;
SELECT MIN(t1.f <-> t2.f) FROM tbl_floatspanset t1, tbl_floatspanset t2;

SELECT MIN(t1.d <-> t2.d) FROM tbl_date t1, tbl_datespanset t2;
SELECT MIN(t1.d <-> t2.d) FROM tbl_datespan t1, tbl_datespanset t2;
SELECT MIN(t1.d <-> t2.d) FROM tbl_datespanset t1, tbl_date t2;
SELECT MIN(t1.d <-> t2.d) FROM tbl_datespanset t1, tbl_datespan t2;
SELECT MIN(t1.d <-> t2.d) FROM tbl_datespanset t1, tbl_datespanset t2;

SELECT MIN(t1.t <-> t2.t) FROM tbl_timestamptz t1, tbl_tstzspanset t2;
SELECT MIN(t1.t <-> t2.t) FROM tbl_tstzspan t1, tbl_tstzspanset t2;
SELECT MIN(t1.t <-> t2.t) FROM tbl_tstzspanset t1, tbl_timestamptz t2;
SELECT MIN(t1.t <-> t2.t) FROM tbl_tstzspanset t1, tbl_tstzspan t2;
SELECT MIN(t1.t <-> t2.t) FROM tbl_tstzspanset t1, tbl_tstzspanset t2;

-- The named portable form of the <-> operator tested above
SELECT spansetDistance(1, intspanset '{[3, 7]}');
SELECT spanDistance(intspan '[1, 2]', intspanset '{[5, 7]}');
SELECT spansetDistance(intspanset '{[3, 7]}', 1);
SELECT spansetDistance(intspanset '{[3, 7]}', intspan '[10, 20]');
SELECT spansetDistance(intspanset '{[3, 7]}', intspanset '{[10, 20]}');
SELECT round(spansetDistance(1.0, floatspanset '{[3.5, 7.5]}')::numeric, 6);
SELECT spansetDistance(date '2001-01-01', datespanset '{[2001-01-05, 2001-01-10]}');

-------------------------------------------------------------------------------
