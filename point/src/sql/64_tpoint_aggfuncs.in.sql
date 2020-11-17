/*****************************************************************************
 *
 * tpoint_aggfuncs.sql
 *    Aggregate functions for temporal points.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB contributors
 *
 * Permission to use, copy, modify, and distribute this software and its documentation for any purpose, without fee, and without a written agreement is hereby
 * granted, provided that the above copyright notice and this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
 * PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
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
