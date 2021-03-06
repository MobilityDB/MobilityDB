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
 * time_aggfuncs.sql
 * Aggregate functions for time types
 */

/*****************************************************************************/

CREATE FUNCTION time_agg_serialize(internal)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'time_agg_serialize'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION time_agg_deserialize(bytea, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'time_agg_deserialize'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampset_tunion_transfn(internal, timestampset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'timestampset_tunion_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION period_tunion_transfn(internal, period)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'period_tunion_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION periodset_tunion_transfn(internal, periodset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'periodset_tunion_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION time_tunion_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'time_tunion_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION timestamp_tunion_finalfn(internal)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'timestamp_tunion_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_tunion_finalfn(internal)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'period_tunion_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tunion(timestampset) (
  SFUNC = timestampset_tunion_transfn,
  STYPE = internal,
  COMBINEFUNC = time_tunion_combinefn,
  FINALFUNC = timestamp_tunion_finalfn,
  SERIALFUNC = time_agg_serialize,
  DESERIALFUNC = time_agg_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tunion(period) (
  SFUNC = period_tunion_transfn,
  STYPE = internal,
  COMBINEFUNC = time_tunion_combinefn,
  FINALFUNC = period_tunion_finalfn,
  SERIALFUNC = time_agg_serialize,
  DESERIALFUNC = time_agg_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tunion(periodset) (
  SFUNC = periodset_tunion_transfn,
  STYPE = internal,
  COMBINEFUNC = time_tunion_combinefn,
  FINALFUNC = period_tunion_finalfn,
  SERIALFUNC = time_agg_serialize,
  DESERIALFUNC = time_agg_deserialize,
  PARALLEL = SAFE
);


/*****************************************************************************/
