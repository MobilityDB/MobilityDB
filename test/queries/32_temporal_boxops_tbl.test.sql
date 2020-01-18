-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tbool_gist_idx;
DROP INDEX IF EXISTS tbl_tint_gist_idx;
DROP INDEX IF EXISTS tbl_tfloat_gist_idx;
DROP INDEX IF EXISTS tbl_ttext_gist_idx;

#if MOBDB_PGSQL_VERSION >= 110000
DROP INDEX IF EXISTS tbl_tbool_spgist_idx;
DROP INDEX IF EXISTS tbl_tint_spgist_idx;
DROP INDEX IF EXISTS tbl_tfloat_spgist_idx;
DROP INDEX IF EXISTS tbl_ttext_spgist_idx;
#endif

-------------------------------------------------------------------------------

DROP TABLE IF EXISTS test_boundboxops;
CREATE TABLE test_boundboxops(
	op char(3), 
	leftarg text, 
	rightarg text, 
	noidx bigint,
	gistidx bigint
#if MOBDB_PGSQL_VERSION >= 110000
	, spgistidx bigint
#endif
);

-------------------------------------------------------------------------------
-- Overlaps
-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'timestamptz', 'tbool', count(*) FROM tbl_timestamptz, tbl_tbool WHERE t && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'timestamptz', 'tint', count(*) FROM tbl_timestamptz, tbl_tint WHERE t && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'timestamptz', 'tfloat', count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'timestamptz', 'ttext', count(*) FROM tbl_timestamptz, tbl_ttext WHERE t && temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'timestampset', 'tbool', count(*) FROM tbl_timestampset, tbl_tbool WHERE ts && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'timestampset', 'tint', count(*) FROM tbl_timestampset, tbl_tint WHERE ts && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'timestampset', 'tfloat', count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'timestampset', 'ttext', count(*) FROM tbl_timestampset, tbl_ttext WHERE ts && temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'period', 'tbool', count(*) FROM tbl_period, tbl_tbool WHERE p && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'period', 'tint', count(*) FROM tbl_period, tbl_tint WHERE p && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'period', 'tfloat', count(*) FROM tbl_period, tbl_tfloat WHERE p && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'period', 'ttext', count(*) FROM tbl_period, tbl_ttext WHERE p && temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'periodset', 'tbool', count(*) FROM tbl_periodset, tbl_tbool WHERE ps && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'periodset', 'tint', count(*) FROM tbl_periodset, tbl_tint WHERE ps && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'periodset', 'tfloat', count(*) FROM tbl_periodset, tbl_tfloat WHERE ps && temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'periodset', 'ttext', count(*) FROM tbl_periodset, tbl_ttext WHERE ps && temp;

-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tbool', 'timestamptz', count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp && t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tbool', 'timestampset', count(*) FROM tbl_tbool, tbl_timestampset WHERE temp && ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tbool', 'period', count(*) FROM tbl_tbool, tbl_period WHERE temp && p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tbool', 'periodset', count(*) FROM tbl_tbool, tbl_periodset WHERE temp && ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tbool', 'tbool', count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp && t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tint', 'int', count(*) FROM tbl_tint, tbl_int WHERE temp && i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tint', 'intrange', count(*) FROM tbl_tint, tbl_intrange WHERE temp && i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tint', 'float', count(*) FROM tbl_tint, tbl_float WHERE temp && f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tint', 'timestamptz', count(*) FROM tbl_tint, tbl_timestamptz WHERE temp && t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tint', 'timestampset', count(*) FROM tbl_tint, tbl_timestampset WHERE temp && ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tint', 'period', count(*) FROM tbl_tint, tbl_period WHERE temp && p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tint', 'periodset', count(*) FROM tbl_tint, tbl_periodset WHERE temp && ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tint', 'tbox', count(*) FROM tbl_tint, tbl_tbox WHERE temp && b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tint', 'tint', count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp && t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tint', 'tfloat', count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp && t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tfloat', 'int', count(*) FROM tbl_tfloat, tbl_int WHERE temp && i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tfloat', 'float', count(*) FROM tbl_tfloat, tbl_float WHERE temp && f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tfloat', 'floatrange', count(*) FROM tbl_tfloat, tbl_floatrange WHERE temp && f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tfloat', 'timestamptz', count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp && t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tfloat', 'timestampset', count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp && ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tfloat', 'period', count(*) FROM tbl_tfloat, tbl_period WHERE temp && p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tfloat', 'periodset', count(*) FROM tbl_tfloat, tbl_periodset WHERE temp && ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tfloat', 'tbox', count(*) FROM tbl_tfloat, tbl_tbox WHERE temp && b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tfloat', 'tint', count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp && t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'tfloat', 'tfloat', count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp && t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'ttext', 'timestamptz', count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp && t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'ttext', 'timestampset', count(*) FROM tbl_ttext, tbl_timestampset WHERE temp && ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'ttext', 'period', count(*) FROM tbl_ttext, tbl_period WHERE temp && p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'ttext', 'periodset', count(*) FROM tbl_ttext, tbl_periodset WHERE temp && ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '&&', 'ttext', 'ttext', count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp && t2.temp;

-------------------------------------------------------------------------------
-- Contains
-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'timestamptz', 'tbool', count(*) FROM tbl_timestamptz, tbl_tbool WHERE t @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'timestamptz', 'tint', count(*) FROM tbl_timestamptz, tbl_tint WHERE t @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'timestamptz', 'tfloat', count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'timestamptz', 'ttext', count(*) FROM tbl_timestamptz, tbl_ttext WHERE t @> temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'timestampset', 'tbool', count(*) FROM tbl_timestampset, tbl_tbool WHERE ts @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'timestampset', 'tint', count(*) FROM tbl_timestampset, tbl_tint WHERE ts @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'timestampset', 'tfloat', count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'timestampset', 'ttext', count(*) FROM tbl_timestampset, tbl_ttext WHERE ts @> temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'period', 'tbool', count(*) FROM tbl_period, tbl_tbool WHERE p @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'period', 'tint', count(*) FROM tbl_period, tbl_tint WHERE p @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'period', 'tfloat', count(*) FROM tbl_period, tbl_tfloat WHERE p @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'period', 'ttext', count(*) FROM tbl_period, tbl_ttext WHERE p @> temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'periodset', 'tbool', count(*) FROM tbl_periodset, tbl_tbool WHERE ps @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'periodset', 'tint', count(*) FROM tbl_periodset, tbl_tint WHERE ps @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'periodset', 'tfloat', count(*) FROM tbl_periodset, tbl_tfloat WHERE ps @> temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'periodset', 'ttext', count(*) FROM tbl_periodset, tbl_ttext WHERE ps @> temp;

-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tbool', 'timestamptz', count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp @> t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tbool', 'timestampset', count(*) FROM tbl_tbool, tbl_timestampset WHERE temp @> ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tbool', 'period', count(*) FROM tbl_tbool, tbl_period WHERE temp @> p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tbool', 'periodset', count(*) FROM tbl_tbool, tbl_periodset WHERE temp @> ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tbool', 'tbool', count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp @> t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tint', 'int', count(*) FROM tbl_tint, tbl_int WHERE temp @> i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tint', 'intrange', count(*) FROM tbl_tint, tbl_intrange WHERE temp @> i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tint', 'float', count(*) FROM tbl_tint, tbl_float WHERE temp @> f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tint', 'timestamptz', count(*) FROM tbl_tint, tbl_timestamptz WHERE temp @> t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tint', 'timestampset', count(*) FROM tbl_tint, tbl_timestampset WHERE temp @> ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tint', 'period', count(*) FROM tbl_tint, tbl_period WHERE temp @> p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tint', 'periodset', count(*) FROM tbl_tint, tbl_periodset WHERE temp @> ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tint', 'tbox', count(*) FROM tbl_tint, tbl_tbox WHERE temp @> b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tint', 'tint', count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tint', 'tfloat', count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp @> t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tfloat', 'int', count(*) FROM tbl_tfloat, tbl_int WHERE temp @> i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tfloat', 'float', count(*) FROM tbl_tfloat, tbl_float WHERE temp @> f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tfloat', 'floatrange', count(*) FROM tbl_tfloat, tbl_floatrange WHERE temp @> f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tfloat', 'timestamptz', count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp @> t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tfloat', 'timestampset', count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp @> ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tfloat', 'period', count(*) FROM tbl_tfloat, tbl_period WHERE temp @> p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tfloat', 'periodset', count(*) FROM tbl_tfloat, tbl_periodset WHERE temp @> ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tfloat', 'tbox', count(*) FROM tbl_tfloat, tbl_tbox WHERE temp @> b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tfloat', 'tint', count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp @> t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'tfloat', 'tfloat', count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp @> t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'ttext', 'timestamptz', count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp @> t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'ttext', 'timestampset', count(*) FROM tbl_ttext, tbl_timestampset WHERE temp @> ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'ttext', 'period', count(*) FROM tbl_ttext, tbl_period WHERE temp @> p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'ttext', 'periodset', count(*) FROM tbl_ttext, tbl_periodset WHERE temp @> ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '@>', 'ttext', 'ttext', count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp @> t2.temp;

-------------------------------------------------------------------------------
-- Contained
-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'timestamptz', 'tbool', count(*) FROM tbl_timestamptz, tbl_tbool WHERE t <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'timestamptz', 'tint', count(*) FROM tbl_timestamptz, tbl_tint WHERE t <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'timestamptz', 'tfloat', count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'timestamptz', 'ttext', count(*) FROM tbl_timestamptz, tbl_ttext WHERE t <@ temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'timestampset', 'tbool', count(*) FROM tbl_timestampset, tbl_tbool WHERE ts <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'timestampset', 'tint', count(*) FROM tbl_timestampset, tbl_tint WHERE ts <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'timestampset', 'tfloat', count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'timestampset', 'ttext', count(*) FROM tbl_timestampset, tbl_ttext WHERE ts <@ temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'period', 'tbool', count(*) FROM tbl_period, tbl_tbool WHERE p <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'period', 'tint', count(*) FROM tbl_period, tbl_tint WHERE p <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'period', 'tfloat', count(*) FROM tbl_period, tbl_tfloat WHERE p <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'period', 'ttext', count(*) FROM tbl_period, tbl_ttext WHERE p <@ temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'periodset', 'tbool', count(*) FROM tbl_periodset, tbl_tbool WHERE ps <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'periodset', 'tint', count(*) FROM tbl_periodset, tbl_tint WHERE ps <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'periodset', 'tfloat', count(*) FROM tbl_periodset, tbl_tfloat WHERE ps <@ temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'periodset', 'ttext', count(*) FROM tbl_periodset, tbl_ttext WHERE ps <@ temp;

-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tbool', 'timestamptz', count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp <@ t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tbool', 'timestampset', count(*) FROM tbl_tbool, tbl_timestampset WHERE temp <@ ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tbool', 'period', count(*) FROM tbl_tbool, tbl_period WHERE temp <@ p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tbool', 'periodset', count(*) FROM tbl_tbool, tbl_periodset WHERE temp <@ ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tbool', 'tbool', count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp <@ t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tint', 'int', count(*) FROM tbl_tint, tbl_int WHERE temp <@ i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tint', 'intrange', count(*) FROM tbl_tint, tbl_intrange WHERE temp <@ i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tint', 'float', count(*) FROM tbl_tint, tbl_float WHERE temp <@ f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tint', 'timestamptz', count(*) FROM tbl_tint, tbl_timestamptz WHERE temp <@ t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tint', 'timestampset', count(*) FROM tbl_tint, tbl_timestampset WHERE temp <@ ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tint', 'period', count(*) FROM tbl_tint, tbl_period WHERE temp <@ p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tint', 'periodset', count(*) FROM tbl_tint, tbl_periodset WHERE temp <@ ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tint', 'tbox', count(*) FROM tbl_tint, tbl_tbox WHERE temp <@ b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tint', 'tint', count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tint', 'tfloat', count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp <@ t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tfloat', 'int', count(*) FROM tbl_tfloat, tbl_int WHERE temp <@ i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tfloat', 'float', count(*) FROM tbl_tfloat, tbl_float WHERE temp <@ f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tfloat', 'floatrange', count(*) FROM tbl_tfloat, tbl_floatrange WHERE temp <@ f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tfloat', 'timestamptz', count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp <@ t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tfloat', 'timestampset', count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp <@ ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tfloat', 'period', count(*) FROM tbl_tfloat, tbl_period WHERE temp <@ p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tfloat', 'periodset', count(*) FROM tbl_tfloat, tbl_periodset WHERE temp <@ ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tfloat', 'tbox', count(*) FROM tbl_tfloat, tbl_tbox WHERE temp <@ b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tfloat', 'tint', count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp <@ t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'tfloat', 'tfloat', count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp <@ t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'ttext', 'timestamptz', count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp <@ t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'ttext', 'timestampset', count(*) FROM tbl_ttext, tbl_timestampset WHERE temp <@ ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'ttext', 'period', count(*) FROM tbl_ttext, tbl_period WHERE temp <@ p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'ttext', 'periodset', count(*) FROM tbl_ttext, tbl_periodset WHERE temp <@ ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '<@', 'ttext', 'ttext', count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp <@ t2.temp;

-------------------------------------------------------------------------------
-- Same
-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'timestamptz', 'tbool', count(*) FROM tbl_timestamptz, tbl_tbool WHERE t ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'timestamptz', 'tint', count(*) FROM tbl_timestamptz, tbl_tint WHERE t ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'timestamptz', 'tfloat', count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'timestamptz', 'ttext', count(*) FROM tbl_timestamptz, tbl_ttext WHERE t ~= temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'timestampset', 'tbool', count(*) FROM tbl_timestampset, tbl_tbool WHERE ts ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'timestampset', 'tint', count(*) FROM tbl_timestampset, tbl_tint WHERE ts ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'timestampset', 'tfloat', count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'timestampset', 'ttext', count(*) FROM tbl_timestampset, tbl_ttext WHERE ts ~= temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'period', 'tbool', count(*) FROM tbl_period, tbl_tbool WHERE p ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'period', 'tint', count(*) FROM tbl_period, tbl_tint WHERE p ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'period', 'tfloat', count(*) FROM tbl_period, tbl_tfloat WHERE p ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'period', 'ttext', count(*) FROM tbl_period, tbl_ttext WHERE p ~= temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'periodset', 'tbool', count(*) FROM tbl_periodset, tbl_tbool WHERE ps ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'periodset', 'tint', count(*) FROM tbl_periodset, tbl_tint WHERE ps ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'periodset', 'tfloat', count(*) FROM tbl_periodset, tbl_tfloat WHERE ps ~= temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'periodset', 'ttext', count(*) FROM tbl_periodset, tbl_ttext WHERE ps ~= temp;

-------------------------------------------------------------------------------

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tbool', 'timestamptz', count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp ~= t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tbool', 'timestampset', count(*) FROM tbl_tbool, tbl_timestampset WHERE temp ~= ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tbool', 'period', count(*) FROM tbl_tbool, tbl_period WHERE temp ~= p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tbool', 'periodset', count(*) FROM tbl_tbool, tbl_periodset WHERE temp ~= ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tbool', 'tbool', count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp ~= t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tint', 'int', count(*) FROM tbl_tint, tbl_int WHERE temp ~= i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tint', 'intrange', count(*) FROM tbl_tint, tbl_intrange WHERE temp ~= i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tint', 'float', count(*) FROM tbl_tint, tbl_float WHERE temp ~= f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tint', 'timestamptz', count(*) FROM tbl_tint, tbl_timestamptz WHERE temp ~= t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tint', 'timestampset', count(*) FROM tbl_tint, tbl_timestampset WHERE temp ~= ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tint', 'period', count(*) FROM tbl_tint, tbl_period WHERE temp ~= p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tint', 'periodset', count(*) FROM tbl_tint, tbl_periodset WHERE temp ~= ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tint', 'tbox', count(*) FROM tbl_tint, tbl_tbox WHERE temp ~= b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tint', 'tint', count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp ~= t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tint', 'tfloat', count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp ~= t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tfloat', 'int', count(*) FROM tbl_tfloat, tbl_int WHERE temp ~= i;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tfloat', 'float', count(*) FROM tbl_tfloat, tbl_float WHERE temp ~= f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tfloat', 'floatrange', count(*) FROM tbl_tfloat, tbl_floatrange WHERE temp ~= f;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tfloat', 'timestamptz', count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp ~= t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tfloat', 'timestampset', count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp ~= ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tfloat', 'period', count(*) FROM tbl_tfloat, tbl_period WHERE temp ~= p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tfloat', 'periodset', count(*) FROM tbl_tfloat, tbl_periodset WHERE temp ~= ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tfloat', 'tbox', count(*) FROM tbl_tfloat, tbl_tbox WHERE temp ~= b;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tfloat', 'tint', count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp ~= t2.temp;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'tfloat', 'tfloat', count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp ~= t2.temp;

INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'ttext', 'timestamptz', count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp ~= t;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'ttext', 'timestampset', count(*) FROM tbl_ttext, tbl_timestampset WHERE temp ~= ts;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'ttext', 'period', count(*) FROM tbl_ttext, tbl_period WHERE temp ~= p;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'ttext', 'periodset', count(*) FROM tbl_ttext, tbl_periodset WHERE temp ~= ps;
INSERT INTO test_boundboxops(op, leftarg, rightarg, noidx)
SELECT '~=', 'ttext', 'ttext', count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp ~= t2.temp;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tbool_gist_idx ON tbl_tbool USING GIST(temp);
CREATE INDEX tbl_tint_gist_idx ON tbl_tint USING GIST(temp);
CREATE INDEX tbl_tfloat_gist_idx ON tbl_tfloat USING GIST(temp);
CREATE INDEX tbl_ttext_gist_idx ON tbl_ttext USING GIST(temp);

-------------------------------------------------------------------------------
-- Overlaps
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tbool WHERE t && temp ) 
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'tbool';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tint WHERE t && temp ) 
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t && temp ) 
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_ttext WHERE t && temp ) 
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'ttext';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tbool WHERE ts && temp ) 
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'tbool';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tint WHERE ts && temp ) 
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts && temp ) 
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_ttext WHERE ts && temp ) 
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'ttext';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tbool WHERE p && temp ) 
WHERE op = '&&' and leftarg = 'period' and rightarg = 'tbool';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tint WHERE p && temp ) 
WHERE op = '&&' and leftarg = 'period' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tfloat WHERE p && temp ) 
WHERE op = '&&' and leftarg = 'period' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_ttext WHERE p && temp ) 
WHERE op = '&&' and leftarg = 'period' and rightarg = 'ttext';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tbool WHERE ps && temp ) 
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'tbool';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tint WHERE ps && temp ) 
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tfloat WHERE ps && temp ) 
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_ttext WHERE ps && temp ) 
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp && t ) 
WHERE op = '&&' and leftarg = 'tbool' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestampset WHERE temp && ts ) 
WHERE op = '&&' and leftarg = 'tbool' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_period WHERE temp && p ) 
WHERE op = '&&' and leftarg = 'tbool' and rightarg = 'period';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_periodset WHERE temp && ps ) 
WHERE op = '&&' and leftarg = 'tbool' and rightarg = 'periodset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp && t2.temp ) 
WHERE op = '&&' and leftarg = 'tbool' and rightarg = 'tbool';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_int WHERE temp && i ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'int';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_intrange WHERE temp && i ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'intrange';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_float WHERE temp && f ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'float';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestamptz WHERE temp && t ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestampset WHERE temp && ts ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_period WHERE temp && p ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'period';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_periodset WHERE temp && ps ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'periodset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp && b ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'tbox';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp && t2.temp ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp && t2.temp ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp && i ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'int';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp && f ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'float';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_floatrange WHERE temp && f ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'floatrange';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp && t ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp && ts ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_period WHERE temp && p ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'period';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_periodset WHERE temp && ps ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'periodset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp && b ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'tbox';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp && t2.temp ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp && t2.temp ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'tfloat';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp && t ) 
WHERE op = '&&' and leftarg = 'ttext' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestampset WHERE temp && ts ) 
WHERE op = '&&' and leftarg = 'ttext' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_period WHERE temp && p ) 
WHERE op = '&&' and leftarg = 'ttext' and rightarg = 'period';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_periodset WHERE temp && ps ) 
WHERE op = '&&' and leftarg = 'ttext' and rightarg = 'periodset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp && t2.temp ) 
WHERE op = '&&' and leftarg = 'ttext' and rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Contains
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tbool WHERE t @> temp ) 
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'tbool';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tint WHERE t @> temp ) 
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t @> temp ) 
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_ttext WHERE t @> temp ) 
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'ttext';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tbool WHERE ts @> temp ) 
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'tbool';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tint WHERE ts @> temp ) 
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts @> temp ) 
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_ttext WHERE ts @> temp ) 
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'ttext';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tbool WHERE p @> temp ) 
WHERE op = '@>' and leftarg = 'period' and rightarg = 'tbool';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tint WHERE p @> temp ) 
WHERE op = '@>' and leftarg = 'period' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tfloat WHERE p @> temp ) 
WHERE op = '@>' and leftarg = 'period' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_ttext WHERE p @> temp ) 
WHERE op = '@>' and leftarg = 'period' and rightarg = 'ttext';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tbool WHERE ps @> temp ) 
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'tbool';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tint WHERE ps @> temp ) 
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tfloat WHERE ps @> temp ) 
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_ttext WHERE ps @> temp ) 
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp @> t ) 
WHERE op = '@>' and leftarg = 'tbool' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestampset WHERE temp @> ts ) 
WHERE op = '@>' and leftarg = 'tbool' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_period WHERE temp @> p ) 
WHERE op = '@>' and leftarg = 'tbool' and rightarg = 'period';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_periodset WHERE temp @> ps ) 
WHERE op = '@>' and leftarg = 'tbool' and rightarg = 'periodset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp @> t2.temp ) 
WHERE op = '@>' and leftarg = 'tbool' and rightarg = 'tbool';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_int WHERE temp @> i ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'int';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_intrange WHERE temp @> i ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'intrange';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_float WHERE temp @> f ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'float';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestamptz WHERE temp @> t ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestampset WHERE temp @> ts ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_period WHERE temp @> p ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'period';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_periodset WHERE temp @> ps ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'periodset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp @> b ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'tbox';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp @> t2.temp ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp @> t2.temp ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'tfloat';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp @> i ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'int';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp @> f ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'float';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_floatrange WHERE temp @> f ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'floatrange';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp @> t ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp @> ts ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_period WHERE temp @> p ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'period';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_periodset WHERE temp @> ps ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'periodset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp @> b ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'tbox';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp @> t2.temp ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp @> t2.temp ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'tfloat';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp @> t ) 
WHERE op = '@>' and leftarg = 'ttext' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestampset WHERE temp @> ts ) 
WHERE op = '@>' and leftarg = 'ttext' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_period WHERE temp @> p ) 
WHERE op = '@>' and leftarg = 'ttext' and rightarg = 'period';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_periodset WHERE temp @> ps ) 
WHERE op = '@>' and leftarg = 'ttext' and rightarg = 'periodset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp @> t2.temp ) 
WHERE op = '@>' and leftarg = 'ttext' and rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Contained
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tbool WHERE t <@ temp ) 
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'tbool';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tint WHERE t <@ temp ) 
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t <@ temp ) 
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_ttext WHERE t <@ temp ) 
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'ttext';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tbool WHERE ts <@ temp ) 
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'tbool';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tint WHERE ts <@ temp ) 
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts <@ temp ) 
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_ttext WHERE ts <@ temp ) 
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'ttext';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tbool WHERE p <@ temp ) 
WHERE op = '<@' and leftarg = 'period' and rightarg = 'tbool';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tint WHERE p <@ temp ) 
WHERE op = '<@' and leftarg = 'period' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tfloat WHERE p <@ temp ) 
WHERE op = '<@' and leftarg = 'period' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_ttext WHERE p <@ temp ) 
WHERE op = '<@' and leftarg = 'period' and rightarg = 'ttext';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tbool WHERE ps <@ temp ) 
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'tbool';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tint WHERE ps <@ temp ) 
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tfloat WHERE ps <@ temp ) 
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_ttext WHERE ps <@ temp ) 
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp <@ t ) 
WHERE op = '<@' and leftarg = 'tbool' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestampset WHERE temp <@ ts ) 
WHERE op = '<@' and leftarg = 'tbool' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_period WHERE temp <@ p ) 
WHERE op = '<@' and leftarg = 'tbool' and rightarg = 'period';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_periodset WHERE temp <@ ps ) 
WHERE op = '<@' and leftarg = 'tbool' and rightarg = 'periodset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp <@ t2.temp ) 
WHERE op = '<@' and leftarg = 'tbool' and rightarg = 'tbool';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_int WHERE temp <@ i ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'int';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_intrange WHERE temp <@ i ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'intrange';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_float WHERE temp <@ f ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'float';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestamptz WHERE temp <@ t ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestampset WHERE temp <@ ts ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_period WHERE temp <@ p ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'period';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_periodset WHERE temp <@ ps ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'periodset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp <@ b ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'tbox';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp <@ t2.temp ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp <@ t2.temp ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp <@ i ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'int';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp <@ f ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'float';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_floatrange WHERE temp <@ f ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'floatrange';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp <@ t ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp <@ ts ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_period WHERE temp <@ p ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'period';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_periodset WHERE temp <@ ps ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'periodset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp <@ b ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'tbox';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp <@ t2.temp ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp <@ t2.temp ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp <@ t ) 
WHERE op = '<@' and leftarg = 'ttext' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestampset WHERE temp <@ ts ) 
WHERE op = '<@' and leftarg = 'ttext' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_period WHERE temp <@ p ) 
WHERE op = '<@' and leftarg = 'ttext' and rightarg = 'period';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_periodset WHERE temp <@ ps ) 
WHERE op = '<@' and leftarg = 'ttext' and rightarg = 'periodset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp <@ t2.temp ) 
WHERE op = '<@' and leftarg = 'ttext' and rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Contained
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tbool WHERE t ~= temp ) 
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'tbool';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tint WHERE t ~= temp ) 
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t ~= temp ) 
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_ttext WHERE t ~= temp ) 
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'ttext';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tbool WHERE ts ~= temp ) 
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'tbool';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tint WHERE ts ~= temp ) 
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts ~= temp ) 
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_ttext WHERE ts ~= temp ) 
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'ttext';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tbool WHERE p ~= temp ) 
WHERE op = '~=' and leftarg = 'period' and rightarg = 'tbool';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tint WHERE p ~= temp ) 
WHERE op = '~=' and leftarg = 'period' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_tfloat WHERE p ~= temp ) 
WHERE op = '~=' and leftarg = 'period' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_period, tbl_ttext WHERE p ~= temp ) 
WHERE op = '~=' and leftarg = 'period' and rightarg = 'ttext';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tbool WHERE ps ~= temp ) 
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'tbool';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tint WHERE ps ~= temp ) 
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tfloat WHERE ps ~= temp ) 
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_periodset, tbl_ttext WHERE ps ~= temp ) 
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp ~= t ) 
WHERE op = '~=' and leftarg = 'tbool' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestampset WHERE temp ~= ts ) 
WHERE op = '~=' and leftarg = 'tbool' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_period WHERE temp ~= p ) 
WHERE op = '~=' and leftarg = 'tbool' and rightarg = 'period';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool, tbl_periodset WHERE temp ~= ps ) 
WHERE op = '~=' and leftarg = 'tbool' and rightarg = 'periodset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp ~= t2.temp ) 
WHERE op = '~=' and leftarg = 'tbool' and rightarg = 'tbool';

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_int WHERE temp ~= i ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'int';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_intrange WHERE temp ~= i ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'intrange';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_float WHERE temp ~= f ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'float';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestamptz WHERE temp ~= t ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestampset WHERE temp ~= ts ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_period WHERE temp ~= p ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'period';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_periodset WHERE temp ~= ps ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'periodset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp ~= b ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'tbox';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp ~= t2.temp ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp ~= t2.temp ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp ~= i ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'int';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp ~= f ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'float';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_floatrange WHERE temp ~= f ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'floatrange';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp ~= t ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp ~= ts ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_period WHERE temp ~= p ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'period';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_periodset WHERE temp ~= ps ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'periodset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp ~= b ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'tbox';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp ~= t2.temp ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'tint';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp ~= t2.temp ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp ~= t ) 
WHERE op = '~=' and leftarg = 'ttext' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestampset WHERE temp ~= ts ) 
WHERE op = '~=' and leftarg = 'ttext' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_period WHERE temp ~= p ) 
WHERE op = '~=' and leftarg = 'ttext' and rightarg = 'period';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext, tbl_periodset WHERE temp ~= ps ) 
WHERE op = '~=' and leftarg = 'ttext' and rightarg = 'periodset';
UPDATE test_boundboxops
SET gistidx = ( SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp ~= t2.temp ) 
WHERE op = '~=' and leftarg = 'ttext' and rightarg = 'ttext';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tbool_gist_idx;
DROP INDEX IF EXISTS tbl_tint_gist_idx;
DROP INDEX IF EXISTS tbl_tfloat_gist_idx;
DROP INDEX IF EXISTS tbl_ttext_gist_idx;

#if MOBDB_PGSQL_VERSION >= 110000

CREATE INDEX tbl_tbool_spgist_idx ON tbl_tbool USING SPGIST(temp);
CREATE INDEX tbl_tint_spgist_idx ON tbl_tint USING SPGIST(temp);
CREATE INDEX tbl_tfloat_spgist_idx ON tbl_tfloat USING SPGIST(temp);
CREATE INDEX tbl_ttext_spgist_idx ON tbl_ttext USING SPGIST(temp);

-------------------------------------------------------------------------------
-- Overlaps
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tbool WHERE t && temp ) 
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'tbool';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tint WHERE t && temp ) 
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t && temp ) 
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_ttext WHERE t && temp ) 
WHERE op = '&&' and leftarg = 'timestamptz' and rightarg = 'ttext';

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tbool WHERE ts && temp ) 
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'tbool';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tint WHERE ts && temp ) 
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts && temp ) 
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_ttext WHERE ts && temp ) 
WHERE op = '&&' and leftarg = 'timestampset' and rightarg = 'ttext';

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tbool WHERE p && temp ) 
WHERE op = '&&' and leftarg = 'period' and rightarg = 'tbool';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tint WHERE p && temp ) 
WHERE op = '&&' and leftarg = 'period' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tfloat WHERE p && temp ) 
WHERE op = '&&' and leftarg = 'period' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_ttext WHERE p && temp ) 
WHERE op = '&&' and leftarg = 'period' and rightarg = 'ttext';

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tbool WHERE ps && temp ) 
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'tbool';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tint WHERE ps && temp ) 
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tfloat WHERE ps && temp ) 
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_ttext WHERE ps && temp ) 
WHERE op = '&&' and leftarg = 'periodset' and rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp && t ) 
WHERE op = '&&' and leftarg = 'tbool' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestampset WHERE temp && ts ) 
WHERE op = '&&' and leftarg = 'tbool' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool, tbl_period WHERE temp && p ) 
WHERE op = '&&' and leftarg = 'tbool' and rightarg = 'period';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool, tbl_periodset WHERE temp && ps ) 
WHERE op = '&&' and leftarg = 'tbool' and rightarg = 'periodset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp && t2.temp ) 
WHERE op = '&&' and leftarg = 'tbool' and rightarg = 'tbool';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_int WHERE temp && i ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'int';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_intrange WHERE temp && i ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'intrange';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_float WHERE temp && f ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'float';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestamptz WHERE temp && t ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestampset WHERE temp && ts ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_period WHERE temp && p ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'period';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_periodset WHERE temp && ps ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'periodset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp && b ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'tbox';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp && t2.temp ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp && t2.temp ) 
WHERE op = '&&' and leftarg = 'tint' and rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp && i ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'int';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp && f ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'float';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_floatrange WHERE temp && f ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'floatrange';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp && t ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp && ts ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_period WHERE temp && p ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'period';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_periodset WHERE temp && ps ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'periodset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp && b ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'tbox';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp && t2.temp ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp && t2.temp ) 
WHERE op = '&&' and leftarg = 'tfloat' and rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp && t ) 
WHERE op = '&&' and leftarg = 'ttext' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestampset WHERE temp && ts ) 
WHERE op = '&&' and leftarg = 'ttext' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext, tbl_period WHERE temp && p ) 
WHERE op = '&&' and leftarg = 'ttext' and rightarg = 'period';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext, tbl_periodset WHERE temp && ps ) 
WHERE op = '&&' and leftarg = 'ttext' and rightarg = 'periodset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp && t2.temp ) 
WHERE op = '&&' and leftarg = 'ttext' and rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Contains
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tbool WHERE t @> temp ) 
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'tbool';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tint WHERE t @> temp ) 
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t @> temp ) 
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_ttext WHERE t @> temp ) 
WHERE op = '@>' and leftarg = 'timestamptz' and rightarg = 'ttext';

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tbool WHERE ts @> temp ) 
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'tbool';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tint WHERE ts @> temp ) 
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts @> temp ) 
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_ttext WHERE ts @> temp ) 
WHERE op = '@>' and leftarg = 'timestampset' and rightarg = 'ttext';

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tbool WHERE p @> temp ) 
WHERE op = '@>' and leftarg = 'period' and rightarg = 'tbool';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tint WHERE p @> temp ) 
WHERE op = '@>' and leftarg = 'period' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tfloat WHERE p @> temp ) 
WHERE op = '@>' and leftarg = 'period' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_ttext WHERE p @> temp ) 
WHERE op = '@>' and leftarg = 'period' and rightarg = 'ttext';

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tbool WHERE ps @> temp ) 
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'tbool';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tint WHERE ps @> temp ) 
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tfloat WHERE ps @> temp ) 
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_ttext WHERE ps @> temp ) 
WHERE op = '@>' and leftarg = 'periodset' and rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp @> t ) 
WHERE op = '@>' and leftarg = 'tbool' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestampset WHERE temp @> ts ) 
WHERE op = '@>' and leftarg = 'tbool' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool, tbl_period WHERE temp @> p ) 
WHERE op = '@>' and leftarg = 'tbool' and rightarg = 'period';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool, tbl_periodset WHERE temp @> ps ) 
WHERE op = '@>' and leftarg = 'tbool' and rightarg = 'periodset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp @> t2.temp ) 
WHERE op = '@>' and leftarg = 'tbool' and rightarg = 'tbool';

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_int WHERE temp @> i ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'int';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_intrange WHERE temp @> i ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'intrange';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_float WHERE temp @> f ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'float';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestamptz WHERE temp @> t ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestampset WHERE temp @> ts ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_period WHERE temp @> p ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'period';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_periodset WHERE temp @> ps ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'periodset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp @> b ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'tbox';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp @> t2.temp ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp @> t2.temp ) 
WHERE op = '@>' and leftarg = 'tint' and rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp @> i ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'int';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp @> f ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'float';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_floatrange WHERE temp @> f ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'floatrange';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp @> t ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp @> ts ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_period WHERE temp @> p ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'period';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_periodset WHERE temp @> ps ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'periodset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp @> b ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'tbox';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp @> t2.temp ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp @> t2.temp ) 
WHERE op = '@>' and leftarg = 'tfloat' and rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp @> t ) 
WHERE op = '@>' and leftarg = 'ttext' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestampset WHERE temp @> ts ) 
WHERE op = '@>' and leftarg = 'ttext' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext, tbl_period WHERE temp @> p ) 
WHERE op = '@>' and leftarg = 'ttext' and rightarg = 'period';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext, tbl_periodset WHERE temp @> ps ) 
WHERE op = '@>' and leftarg = 'ttext' and rightarg = 'periodset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp @> t2.temp ) 
WHERE op = '@>' and leftarg = 'ttext' and rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Contained
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tbool WHERE t <@ temp ) 
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'tbool';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tint WHERE t <@ temp ) 
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t <@ temp ) 
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_ttext WHERE t <@ temp ) 
WHERE op = '<@' and leftarg = 'timestamptz' and rightarg = 'ttext';

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tbool WHERE ts <@ temp ) 
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'tbool';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tint WHERE ts <@ temp ) 
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts <@ temp ) 
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_ttext WHERE ts <@ temp ) 
WHERE op = '<@' and leftarg = 'timestampset' and rightarg = 'ttext';

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tbool WHERE p <@ temp ) 
WHERE op = '<@' and leftarg = 'period' and rightarg = 'tbool';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tint WHERE p <@ temp ) 
WHERE op = '<@' and leftarg = 'period' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tfloat WHERE p <@ temp ) 
WHERE op = '<@' and leftarg = 'period' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_ttext WHERE p <@ temp ) 
WHERE op = '<@' and leftarg = 'period' and rightarg = 'ttext';

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tbool WHERE ps <@ temp ) 
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'tbool';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tint WHERE ps <@ temp ) 
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tfloat WHERE ps <@ temp ) 
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_ttext WHERE ps <@ temp ) 
WHERE op = '<@' and leftarg = 'periodset' and rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp <@ t ) 
WHERE op = '<@' and leftarg = 'tbool' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestampset WHERE temp <@ ts ) 
WHERE op = '<@' and leftarg = 'tbool' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool, tbl_period WHERE temp <@ p ) 
WHERE op = '<@' and leftarg = 'tbool' and rightarg = 'period';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool, tbl_periodset WHERE temp <@ ps ) 
WHERE op = '<@' and leftarg = 'tbool' and rightarg = 'periodset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp <@ t2.temp ) 
WHERE op = '<@' and leftarg = 'tbool' and rightarg = 'tbool';

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_int WHERE temp <@ i ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'int';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_intrange WHERE temp <@ i ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'intrange';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_float WHERE temp <@ f ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'float';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestamptz WHERE temp <@ t ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestampset WHERE temp <@ ts ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_period WHERE temp <@ p ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'period';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_periodset WHERE temp <@ ps ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'periodset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp <@ b ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'tbox';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp <@ t2.temp ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp <@ t2.temp ) 
WHERE op = '<@' and leftarg = 'tint' and rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp <@ i ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'int';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp <@ f ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'float';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_floatrange WHERE temp <@ f ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'floatrange';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp <@ t ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp <@ ts ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_period WHERE temp <@ p ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'period';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_periodset WHERE temp <@ ps ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'periodset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp <@ b ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'tbox';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp <@ t2.temp ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp <@ t2.temp ) 
WHERE op = '<@' and leftarg = 'tfloat' and rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp <@ t ) 
WHERE op = '<@' and leftarg = 'ttext' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestampset WHERE temp <@ ts ) 
WHERE op = '<@' and leftarg = 'ttext' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext, tbl_period WHERE temp <@ p ) 
WHERE op = '<@' and leftarg = 'ttext' and rightarg = 'period';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext, tbl_periodset WHERE temp <@ ps ) 
WHERE op = '<@' and leftarg = 'ttext' and rightarg = 'periodset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp <@ t2.temp ) 
WHERE op = '<@' and leftarg = 'ttext' and rightarg = 'ttext';

-------------------------------------------------------------------------------
-- Contained
-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tbool WHERE t ~= temp ) 
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'tbool';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tint WHERE t ~= temp ) 
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tfloat WHERE t ~= temp ) 
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_ttext WHERE t ~= temp ) 
WHERE op = '~=' and leftarg = 'timestamptz' and rightarg = 'ttext';

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tbool WHERE ts ~= temp ) 
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'tbool';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tint WHERE ts ~= temp ) 
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tfloat WHERE ts ~= temp ) 
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_ttext WHERE ts ~= temp ) 
WHERE op = '~=' and leftarg = 'timestampset' and rightarg = 'ttext';

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tbool WHERE p ~= temp ) 
WHERE op = '~=' and leftarg = 'period' and rightarg = 'tbool';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tint WHERE p ~= temp ) 
WHERE op = '~=' and leftarg = 'period' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tfloat WHERE p ~= temp ) 
WHERE op = '~=' and leftarg = 'period' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_ttext WHERE p ~= temp ) 
WHERE op = '~=' and leftarg = 'period' and rightarg = 'ttext';

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tbool WHERE ps ~= temp ) 
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'tbool';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tint WHERE ps ~= temp ) 
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tfloat WHERE ps ~= temp ) 
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'tfloat';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_ttext WHERE ps ~= temp ) 
WHERE op = '~=' and leftarg = 'periodset' and rightarg = 'ttext';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestamptz WHERE temp ~= t ) 
WHERE op = '~=' and leftarg = 'tbool' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool, tbl_timestampset WHERE temp ~= ts ) 
WHERE op = '~=' and leftarg = 'tbool' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool, tbl_period WHERE temp ~= p ) 
WHERE op = '~=' and leftarg = 'tbool' and rightarg = 'period';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool, tbl_periodset WHERE temp ~= ps ) 
WHERE op = '~=' and leftarg = 'tbool' and rightarg = 'periodset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp ~= t2.temp ) 
WHERE op = '~=' and leftarg = 'tbool' and rightarg = 'tbool';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_int WHERE temp ~= i ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'int';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_intrange WHERE temp ~= i ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'intrange';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_float WHERE temp ~= f ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'float';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestamptz WHERE temp ~= t ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_timestampset WHERE temp ~= ts ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_period WHERE temp ~= p ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'period';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_periodset WHERE temp ~= ps ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'periodset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint, tbl_tbox WHERE temp ~= b ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'tbox';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp ~= t2.temp ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp ~= t2.temp ) 
WHERE op = '~=' and leftarg = 'tint' and rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp ~= i ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'int';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp ~= f ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'float';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_floatrange WHERE temp ~= f ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'floatrange';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestamptz WHERE temp ~= t ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_timestampset WHERE temp ~= ts ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_period WHERE temp ~= p ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'period';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_periodset WHERE temp ~= ps ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'periodset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat, tbl_tbox WHERE temp ~= b ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'tbox';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp ~= t2.temp ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'tint';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp ~= t2.temp ) 
WHERE op = '~=' and leftarg = 'tfloat' and rightarg = 'tfloat';

-------------------------------------------------------------------------------

UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestamptz WHERE temp ~= t ) 
WHERE op = '~=' and leftarg = 'ttext' and rightarg = 'timestamptz';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext, tbl_timestampset WHERE temp ~= ts ) 
WHERE op = '~=' and leftarg = 'ttext' and rightarg = 'timestampset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext, tbl_period WHERE temp ~= p ) 
WHERE op = '~=' and leftarg = 'ttext' and rightarg = 'period';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext, tbl_periodset WHERE temp ~= ps ) 
WHERE op = '~=' and leftarg = 'ttext' and rightarg = 'periodset';
UPDATE test_boundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp ~= t2.temp ) 
WHERE op = '~=' and leftarg = 'ttext' and rightarg = 'ttext';

#endif

-------------------------------------------------------------------------------

SELECT * FROM test_boundboxops
WHERE noidx <> gistidx 
#if MOBDB_PGSQL_VERSION >= 110000
OR noidx <> spgistidx OR gistidx <> spgistidx
#endif
ORDER BY op, leftarg, rightarg;

DROP INDEX tbl_tbool_spgist_idx;
DROP INDEX tbl_tint_spgist_idx;
DROP INDEX tbl_tfloat_spgist_idx;
DROP INDEX tbl_ttext_spgist_idx;

DROP TABLE test_boundboxops;

-------------------------------------------------------------------------------
