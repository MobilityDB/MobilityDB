/*****************************************************************************
 *
 * AggregateFuncs.sql
 *	  Temporal taggregate functions
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION tcount_transfn2(internal, tbool)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_transfn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_combinefn2(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_combinefn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tand_transfn2(internal, tbool)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tbool_tand_transfn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tand_combinefn2(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tbool_tand_combinefn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tor_transfn2(internal, tbool)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tbool_tor_transfn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tor_combinefn2(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tbool_tor_combinefn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tagg_finalfn2(internal)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_tagg_finalfn2'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tint_tagg_finalfn2(internal)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_tagg_finalfn2'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tcount2(tbool) (
	SFUNC = tcount_transfn2,
	STYPE = internal,
	COMBINEFUNC = tcount_combinefn2,
	FINALFUNC = tint_tagg_finalfn2,
	PARALLEL = SAFE
);
CREATE AGGREGATE tand2(tbool) (
	SFUNC = tbool_tand_transfn2,
	STYPE = internal,
	COMBINEFUNC = tbool_tand_combinefn2,
	FINALFUNC = tbool_tagg_finalfn2,
	PARALLEL = SAFE
);
CREATE AGGREGATE tor2(tbool) (
	SFUNC = tbool_tor_transfn2,
	STYPE = internal,
	COMBINEFUNC = tbool_tor_combinefn2,
	FINALFUNC = tbool_tagg_finalfn2,
	PARALLEL = SAFE
);

/*****************************************************************************/

CREATE FUNCTION tint_tmin_transfn2(internal, tint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tmin_transfn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tmin_combinefn2(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tmin_combinefn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tmax_transfn2(internal, tint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tmax_transfn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tmax_combinefn2(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tmax_combinefn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tsum_transfn2(internal, tint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tsum_transfn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tsum_combinefn2(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tsum_combinefn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn2(internal, tint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_transfn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tavg_transfn2(internal, tint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tavg_transfn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tavg_combinefn2(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tavg_combinefn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tavg_finalfn2(internal)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_tavg_finalfn2'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tmin2(tint) (
	SFUNC = tint_tmin_transfn2,
	STYPE = internal,
	COMBINEFUNC = tint_tmin_combinefn2,
	FINALFUNC = tint_tagg_finalfn2,
	PARALLEL = SAFE
);
CREATE AGGREGATE tmax2(tint) (
	SFUNC = tint_tmax_transfn2,
	STYPE = internal,
	COMBINEFUNC = tint_tmax_combinefn2,
	FINALFUNC = tint_tagg_finalfn2,
	PARALLEL = SAFE
);
CREATE AGGREGATE tsum2(tint) (
	SFUNC = tint_tsum_transfn2,
	STYPE = internal,
	COMBINEFUNC = tint_tsum_combinefn2,
	FINALFUNC = tint_tagg_finalfn2,
	PARALLEL = SAFE
);
CREATE AGGREGATE tcount2(tint) (
	SFUNC = tcount_transfn2,
	STYPE = internal,
	COMBINEFUNC = tcount_combinefn2,
	FINALFUNC = tint_tagg_finalfn2,
	PARALLEL = SAFE
);
CREATE AGGREGATE tavg2(tint) (
	SFUNC = tavg_transfn2,
	STYPE = internal,
	COMBINEFUNC = tavg_combinefn2,
	FINALFUNC = tavg_finalfn2,
	PARALLEL = SAFE
);

CREATE FUNCTION tfloat_tmin_transfn2(internal, tfloat)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tmin_transfn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tmin_combinefn2(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tmin_combinefn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tmax_transfn2(internal, tfloat)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tmax_transfn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tmax_combinefn2(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tmax_combinefn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tsum_transfn2(internal, tfloat)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tsum_transfn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tsum_combinefn2(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tsum_combinefn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn2(internal, tfloat)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_transfn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tagg_finalfn2(internal)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_tagg_finalfn2'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tavg_transfn2(internal, tfloat)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tavg_transfn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE tmin2(tfloat) (
	SFUNC = tfloat_tmin_transfn2,
	STYPE = internal,
	COMBINEFUNC = tfloat_tmin_combinefn2,
	FINALFUNC = tfloat_tagg_finalfn2,
	PARALLEL = SAFE
);
CREATE AGGREGATE tmax2(tfloat) (
	SFUNC = tfloat_tmax_transfn2,
	STYPE = internal,
	COMBINEFUNC = tfloat_tmax_combinefn2,
	FINALFUNC = tfloat_tagg_finalfn2,
	PARALLEL = SAFE
);
CREATE AGGREGATE tsum2(tfloat) (
	SFUNC = tfloat_tsum_transfn2,
	STYPE = internal,
	COMBINEFUNC = tfloat_tsum_combinefn2,
	FINALFUNC = tfloat_tagg_finalfn2,
	PARALLEL = SAFE
);
CREATE AGGREGATE tcount2(tfloat) (
	SFUNC = tcount_transfn2,
	STYPE = internal,
	COMBINEFUNC = tcount_combinefn2,
	FINALFUNC = tint_tagg_finalfn2,
	PARALLEL = SAFE
);
CREATE AGGREGATE tavg2(tfloat) (
	SFUNC = tavg_transfn2,
	STYPE = internal,
	COMBINEFUNC = tavg_combinefn2,
	FINALFUNC = tavg_finalfn2,
	PARALLEL = SAFE
);
 
/*****************************************************************************/

CREATE FUNCTION ttext_tmin_transfn2(internal, ttext)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'ttext_tmin_transfn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tmin_combinefn2(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'ttext_tmin_combinefn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tmax_transfn2(internal, ttext)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'ttext_tmax_transfn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tmax_combinefn2(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'ttext_tmax_combinefn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn2(internal, ttext)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_transfn2'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tagg_finalfn2(internal)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_tagg_finalfn2'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tmin2(ttext) (
	SFUNC = ttext_tmin_transfn2,
	STYPE = internal,
	COMBINEFUNC = ttext_tmin_combinefn2,
	FINALFUNC = ttext_tagg_finalfn2,
	PARALLEL = SAFE
);
CREATE AGGREGATE tmax2(ttext) (
	SFUNC = ttext_tmax_transfn2,
	STYPE = internal,
	COMBINEFUNC = ttext_tmax_combinefn2,
	FINALFUNC = ttext_tagg_finalfn2,
	PARALLEL = SAFE
);
CREATE AGGREGATE tcount2(ttext) (
	SFUNC = tcount_transfn2,
	STYPE = internal,
	COMBINEFUNC = tcount_combinefn2,
	FINALFUNC = tint_tagg_finalfn2,
	PARALLEL = SAFE
);

/*****************************************************************************/
