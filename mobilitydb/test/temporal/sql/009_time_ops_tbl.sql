-------------------------------------------------------------------------------
-- File time_ops.c
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Granularity modification with tprecision

SELECT MAX(tprecision(t, '15 minutes')) FROM tbl_timestamptz;
SELECT MAX(startValue(tprecision(t, '15 minutes'))) FROM tbl_tstzset;
SELECT MAX(lower(tprecision(t, '15 minutes'))) FROM tbl_tstzspan;
SELECT MAX(lower(tprecision(t, '15 minutes'))) FROM tbl_tstzspanset;

-------------------------------------------------------------------------------
-- Selectivity tests of time operators

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.t < t2.t;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.t <= t2.t;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.t > t2.t;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.t >= t2.t;

CREATE TEMP TABLE tbl_tstzspan_temp AS SELECT k, shift(t, '1 year') AS t FROM tbl_tstzspan;
SELECT COUNT(*) FROM tbl_tstzspan_temp WHERE t && tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan_temp t2 WHERE t1.t && t2.t;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan_temp t2 WHERE t1.t @> t2.t;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan_temp t2 WHERE t1.t <@ t2.t;

SELECT round(_mobdb_span_sel('tbl_intspan'::regclass, 'i', '&&(intspan,intspan)'::regoperator, intspan '[50, 55]'), 6);
SELECT round(_mobdb_span_sel('tbl_intspan'::regclass, 'i', '@>(intspan,intspan)'::regoperator, intspan '[50, 55]'), 6);
SELECT round(_mobdb_span_sel('tbl_floatspan'::regclass, 'f', '&&(floatspan,floatspan)'::regoperator, floatspan '[50, 55]'), 6);
SELECT round(_mobdb_span_sel('tbl_floatspan'::regclass, 'f', '@>(floatspan,floatspan)'::regoperator, floatspan '[50, 55]'), 6);
SELECT round(_mobdb_span_sel('tbl_tstzspan'::regclass, 't', '&&(tstzspan,tstzspan)'::regoperator, tstzspan '[2001-06-01, 2001-07-01]'), 6);
SELECT round(_mobdb_span_sel('tbl_tstzspan'::regclass, 't', '@>(tstzspan,tstzspan)'::regoperator, tstzspan '[2001-06-01 00:00:00, 2001-06-01:00:00:03]'), 6);

SELECT round(_mobdb_span_joinsel('tbl_intspan'::regclass, 'i', 'tbl_intspan'::regclass, 'i', '&&(intspan,intspan)'::regoperator), 6);
SELECT round(_mobdb_span_joinsel('tbl_intspan'::regclass, 'i', 'tbl_intspan'::regclass, 'i', '@>(intspan,intspan)'::regoperator), 6);
SELECT round(_mobdb_span_joinsel('tbl_floatspan'::regclass, 'f', 'tbl_floatspan'::regclass, 'f', '&&(floatspan,floatspan)'::regoperator), 6);
SELECT round(_mobdb_span_joinsel('tbl_floatspan'::regclass, 'f', 'tbl_floatspan'::regclass, 'f', '@>(floatspan,floatspan)'::regoperator), 6);
SELECT round(_mobdb_span_joinsel('tbl_tstzspan'::regclass, 't', 'tbl_tstzspan'::regclass, 't', '&&(tstzspan,tstzspan)'::regoperator), 6);
SELECT round(_mobdb_span_joinsel('tbl_tstzspan'::regclass, 't', 'tbl_tstzspan'::regclass, 't', '@>(tstzspan,tstzspan)'::regoperator), 6);

/* Errors */
SELECT round(_mobdb_span_sel(1184, 't', '&&(tstzspan,tstzspan)'::regoperator, tstzspan '[2001-06-01, 2001-07-01]'), 6);
SELECT _mobdb_span_sel('tbl_tstzspan'::regclass, 'X', '&&(tstzspan,tstzspan)'::regoperator, tstzspan '[2001-06-01, 2001-07-01]');
SELECT _mobdb_span_sel('tbl_tstzspan'::regclass, 't', '<(text,text)'::regoperator, tstzspan '[2001-06-01, 2001-07-01]');

SELECT _mobdb_span_joinsel(1184, 'X', 'tbl_tstzspan'::regclass, 't', '&&(tstzspan,tstzspan)'::regoperator);
SELECT _mobdb_span_joinsel('tbl_tstzspan'::regclass, 'X', 'tbl_tstzspan'::regclass, 't', '&&(tstzspan,tstzspan)'::regoperator);
SELECT _mobdb_span_joinsel('tbl_tstzspan'::regclass, 't', 1184, 't', '&&(tstzspan,tstzspan)'::regoperator);
SELECT _mobdb_span_joinsel('tbl_tstzspan'::regclass, 't', 'tbl_tstzspan'::regclass, 'X', '&&(tstzspan,tstzspan)'::regoperator);
SELECT _mobdb_span_joinsel('tbl_tstzspan'::regclass, 't', 'tbl_tstzspan'::regclass, 't', '<(text,text)'::regoperator);

SELECT _mobdb_span_sel('tbl_tstzspan_temp'::regclass, 't', '&&(tstzspan,tstzspan)'::regoperator, tstzspan '[2001-06-01, 2001-07-01]');
SELECT _mobdb_span_joinsel('tbl_tstzspan_temp'::regclass, 'X', 'tbl_tstzspan'::regclass, 't', '&&(tstzspan,tstzspan)'::regoperator);
DROP TABLE tbl_tstzspan_temp;

-------------------------------------------------------------------------------
