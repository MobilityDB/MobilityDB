/*****************************************************************************
 *
 * tpoint_aggfuncs.sql
 *    Aggregate functions for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE OR REPLACE FUNCTION tpoint_extent_transfn(stbox, tgeompoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE OR REPLACE FUNCTION tpoint_extent_transfn(stbox, tgeogpoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE OR REPLACE FUNCTION tpoint_extent_combinefn(stbox, stbox)
  RETURNS stbox
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tgeompoint) (
  SFUNC = tpoint_extent_transfn,
  STYPE = stbox,
  COMBINEFUNC = tpoint_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(tgeogpoint) (
  SFUNC = tpoint_extent_transfn,
  STYPE = stbox,
  COMBINEFUNC = tpoint_extent_combinefn,
  PARALLEL = safe
);

/*****************************************************************************/

CREATE FUNCTION tcount_transfn(internal, tgeompoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn(internal, tgeogpoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE tcount(tgeompoint) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tcount(tgeogpoint) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

CREATE FUNCTION wcount_transfn(internal, tgeompoint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wcount_transfn(internal, tgeogpoint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE wcount(tgeompoint, interval) (
  SFUNC = wcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE wcount(tgeogpoint, interval) (
  SFUNC = wcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);
  
CREATE FUNCTION tcentroid_transfn(internal, tgeompoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'tpoint_tcentroid_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcentroid_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'tpoint_tcentroid_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcentroid_finalfn(internal)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'tpoint_tcentroid_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tcentroid(tgeompoint) (
  SFUNC = tcentroid_transfn,
  STYPE = internal,
  COMBINEFUNC = tcentroid_combinefn,
  FINALFUNC = tcentroid_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/
