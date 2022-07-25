/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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

CREATE FUNCTION tagg_serialize(internal)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tagg_serialize'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tagg_deserialize(bytea, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tagg_deserialize'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION timestampset_extent_transfn(period, timestampset)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Timestampset_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION periodset_extent_transfn(period, periodset)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Periodset_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(timestampset) (
  SFUNC = timestampset_extent_transfn,
  STYPE = period,
  COMBINEFUNC = span_extent_combinefn
  -- , PARALLEL = safe
);
CREATE AGGREGATE extent(periodset) (
  SFUNC = periodset_extent_transfn,
  STYPE = period,
  COMBINEFUNC = span_extent_combinefn
  -- , PARALLEL = safe
);

/*****************************************************************************/

CREATE TYPE tint;

CREATE FUNCTION tcount_transfn(internal, timestampset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Timestampset_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn(internal, period)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Period_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn(internal, periodset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Periodset_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tagg_finalfn(internal)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tcount(timestampset) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize
  -- , PARALLEL = SAFE
);

CREATE AGGREGATE tcount(period) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize
  -- , PARALLEL = SAFE
);

CREATE AGGREGATE tcount(periodset) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize
  -- , PARALLEL = SAFE
);

/*****************************************************************************/

CREATE FUNCTION timestampset_tunion_transfn(internal, timestampset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Timestampset_tunion_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION period_tunion_transfn(internal, period)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Period_tunion_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION periodset_tunion_transfn(internal, periodset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Periodset_tunion_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION time_tunion_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Time_tunion_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION timestamp_tunion_finalfn(internal)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Timestamp_tunion_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_tunion_finalfn(internal)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Period_tunion_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tunion(timestampset) (
  SFUNC = timestampset_tunion_transfn,
  STYPE = internal,
  COMBINEFUNC = time_tunion_combinefn,
  FINALFUNC = timestamp_tunion_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize
  -- , PARALLEL = SAFE
);

CREATE AGGREGATE tunion(period) (
  SFUNC = period_tunion_transfn,
  STYPE = internal,
  COMBINEFUNC = time_tunion_combinefn,
  FINALFUNC = period_tunion_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize
  -- , PARALLEL = SAFE
);

CREATE AGGREGATE tunion(periodset) (
  SFUNC = periodset_tunion_transfn,
  STYPE = internal,
  COMBINEFUNC = time_tunion_combinefn,
  FINALFUNC = period_tunion_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize
  -- , PARALLEL = SAFE
);


/*****************************************************************************/
