/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written 
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO 
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

/*
 * temporal_aggfuncs.sql
 * Temporal aggregate functions
 */

CREATE OR REPLACE FUNCTION temporal_extent_transfn(period, tbool)
  RETURNS period
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE OR REPLACE FUNCTION temporal_extent_transfn(period, ttext)
  RETURNS period
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE OR REPLACE FUNCTION temporal_extent_combinefn(period, period)
  RETURNS period
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tbool) (
  SFUNC = temporal_extent_transfn,
  STYPE = period,
  COMBINEFUNC = temporal_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(ttext) (
  SFUNC = temporal_extent_transfn,
  STYPE = period,
  COMBINEFUNC = temporal_extent_combinefn,
  PARALLEL = safe
);

CREATE OR REPLACE FUNCTION tnumber_extent_transfn(tbox, tint)
  RETURNS tbox
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE OR REPLACE FUNCTION tnumber_extent_transfn(tbox, tfloat)
  RETURNS tbox
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE OR REPLACE FUNCTION tnumber_extent_combinefn(tbox, tbox)
  RETURNS tbox
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tint) (
  SFUNC = tnumber_extent_transfn,
  STYPE = tbox,
  COMBINEFUNC = tnumber_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(tfloat) (
  SFUNC = tnumber_extent_transfn,
  STYPE = tbox,
  COMBINEFUNC = tnumber_extent_combinefn,
  PARALLEL = safe
);

/*****************************************************************************/

CREATE FUNCTION tcount_transfn(internal, tbool)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'temporal_tcount_transfn'
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
