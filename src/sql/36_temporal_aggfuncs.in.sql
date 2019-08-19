/*****************************************************************************
 *
 * temporal_aggfuncs.sql
 *	  Temporal taggregate functions
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION tagg_serialize(internal)
	RETURNS bytea
	AS 'MODULE_PATHNAME', 'temporal_tagg_serialize'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tagg_deserialize(bytea, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tagg_deserialize'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tcount_transfn(internal, tbool)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tand_transfn(internal, tbool)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tbool_tand_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tand_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tbool_tand_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tor_transfn(internal, tbool)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tbool_tor_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tor_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tbool_tor_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tagg_finalfn(internal)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_tagg_finalfn'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tint_tagg_finalfn(internal)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_tagg_finalfn'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tcount(tbool) (
	SFUNC = tcount_transfn,
	STYPE = internal,
	COMBINEFUNC = tcount_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tand(tbool) (
	SFUNC = tbool_tand_transfn,
	STYPE = internal,
	COMBINEFUNC = tbool_tand_combinefn,
	FINALFUNC = tbool_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tor(tbool) (
	SFUNC = tbool_tor_transfn,
	STYPE = internal,
	COMBINEFUNC = tbool_tor_combinefn,
	FINALFUNC = tbool_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);

/*****************************************************************************/

CREATE FUNCTION tint_tmin_transfn(internal, tint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tmin_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tmin_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tmin_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tmax_transfn(internal, tint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tmax_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tmax_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tmax_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tsum_transfn(internal, tint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tsum_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tsum_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tsum_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn(internal, tint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tavg_transfn(internal, tint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tnumber_tavg_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tavg_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tnumber_tavg_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tavg_finalfn(internal)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tnumber_tavg_finalfn'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tmin(tint) (
	SFUNC = tint_tmin_transfn,
	STYPE = internal,
	COMBINEFUNC = tint_tmin_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tmax(tint) (
	SFUNC = tint_tmax_transfn,
	STYPE = internal,
	COMBINEFUNC = tint_tmax_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tsum(tint) (
	SFUNC = tint_tsum_transfn,
	STYPE = internal,
	COMBINEFUNC = tint_tsum_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tcount(tint) (
	SFUNC = tcount_transfn,
	STYPE = internal,
	COMBINEFUNC = tcount_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tavg(tint) (
	SFUNC = tavg_transfn,
	STYPE = internal,
	COMBINEFUNC = tavg_combinefn,
	FINALFUNC = tavg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);

CREATE FUNCTION tfloat_tmin_transfn(internal, tfloat)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tmin_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tmin_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tmin_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tmax_transfn(internal, tfloat)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tmax_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tmax_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tmax_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tsum_transfn(internal, tfloat)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tsum_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tsum_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tsum_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn(internal, tfloat)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tagg_finalfn(internal)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_tagg_finalfn'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tavg_transfn(internal, tfloat)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tnumber_tavg_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE tmin(tfloat) (
	SFUNC = tfloat_tmin_transfn,
	STYPE = internal,
	COMBINEFUNC = tfloat_tmin_combinefn,
	FINALFUNC = tfloat_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tmax(tfloat) (
	SFUNC = tfloat_tmax_transfn,
	STYPE = internal,
	COMBINEFUNC = tfloat_tmax_combinefn,
	FINALFUNC = tfloat_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tsum(tfloat) (
	SFUNC = tfloat_tsum_transfn,
	STYPE = internal,
	COMBINEFUNC = tfloat_tsum_combinefn,
	FINALFUNC = tfloat_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tcount(tfloat) (
	SFUNC = tcount_transfn,
	STYPE = internal,
	COMBINEFUNC = tcount_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tavg(tfloat) (
	SFUNC = tavg_transfn,
	STYPE = internal,
	COMBINEFUNC = tavg_combinefn,
	FINALFUNC = tavg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
 
/*****************************************************************************/

CREATE FUNCTION ttext_tmin_transfn(internal, ttext)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'ttext_tmin_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tmin_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'ttext_tmin_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tmax_transfn(internal, ttext)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'ttext_tmax_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tmax_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'ttext_tmax_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn(internal, ttext)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tagg_finalfn(internal)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_tagg_finalfn'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tmin(ttext) (
	SFUNC = ttext_tmin_transfn,
	STYPE = internal,
	COMBINEFUNC = ttext_tmin_combinefn,
	FINALFUNC = ttext_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tmax(ttext) (
	SFUNC = ttext_tmax_transfn,
	STYPE = internal,
	COMBINEFUNC = ttext_tmax_combinefn,
	FINALFUNC = ttext_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tcount(ttext) (
	SFUNC = tcount_transfn,
	STYPE = internal,
	COMBINEFUNC = tcount_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);

/*****************************************************************************/
