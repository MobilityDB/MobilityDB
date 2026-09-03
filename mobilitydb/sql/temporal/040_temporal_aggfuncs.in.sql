/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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

/**
 * @file
 * @brief Temporal aggregate functions
 */

-- The function is not strict
CREATE FUNCTION temporal_extent_transfn(tstzspan, tbool)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Temporal_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_extent_transfn(tstzspan, ttext)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Temporal_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_extent_combinefn(tstzspan, tstzspan)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Span_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tbool) (
  SFUNC = temporal_extent_transfn,
  STYPE = tstzspan,
  COMBINEFUNC = temporal_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(ttext) (
  SFUNC = temporal_extent_transfn,
  STYPE = tstzspan,
  COMBINEFUNC = temporal_extent_combinefn,
  PARALLEL = safe
);

-- The function is not strict
CREATE FUNCTION tnumber_extent_transfn(tbox, tint)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tnumber_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tnumber_extent_transfn(tbox, tbigint)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tnumber_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tnumber_extent_transfn(tbox, tfloat)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tnumber_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tnumber_extent_combinefn(tbox, tbox)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tint) (
  SFUNC = tnumber_extent_transfn,
  STYPE = tbox,
  COMBINEFUNC = tnumber_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(tbigint) (
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

CREATE FUNCTION taggstate_serialize(internal)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Taggstate_serialize'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION taggstate_deserialize(bytea, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Taggstate_deserialize'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

-- The function is not strict
CREATE FUNCTION tCountTransition(internal, timestamptz)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Timestamptz_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tCountTransition(internal, tstzset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tstzset_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tCountTransition(internal, tstzspan)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tstzspan_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tCountTransition(internal, tstzspanset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tstzspanset_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tcount_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tagg_finalfn(internal)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbigint_tagg_finalfn(internal)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tCount(timestamptz) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tCount(tstzset) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tCount(tstzspan) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tCount(tstzspanset) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

-- The function is not strict
CREATE FUNCTION tCountTransition(internal, tbool)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tbool_tand_transfn(internal, tbool)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbool_tand_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tand_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbool_tand_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tor_transfn(internal, tbool)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbool_tor_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tor_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbool_tor_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tagg_finalfn(internal)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tCount(tbool) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tAnd(tbool) (
  SFUNC = tbool_tand_transfn,
  STYPE = internal,
  COMBINEFUNC = tbool_tand_combinefn,
  FINALFUNC = tbool_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tOr(tbool) (
  SFUNC = tbool_tor_transfn,
  STYPE = internal,
  COMBINEFUNC = tbool_tor_combinefn,
  FINALFUNC = tbool_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

-- The function is not strict
CREATE FUNCTION tCountTransition(internal, tint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tCountTransition(internal, tbigint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tint_tmin_transfn(internal, tint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tint_tmin_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tmin_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tint_tmin_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tmax_transfn(internal, tint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tint_tmax_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tmax_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tint_tmax_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tsum_transfn(internal, tint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tint_tsum_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tsum_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tint_tsum_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tbigint_tmin_transfn(internal, tbigint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbigint_tmin_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbigint_tmin_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbigint_tmin_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbigint_tmax_transfn(internal, tbigint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbigint_tmax_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbigint_tmax_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbigint_tmax_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbigint_tsum_transfn(internal, tbigint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbigint_tsum_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbigint_tsum_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbigint_tsum_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tavg_transfn(internal, tint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tnumber_tavg_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tavg_transfn(internal, tbigint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tnumber_tavg_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tavg_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tnumber_tavg_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tavg_finalfn(internal)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tnumber_tavg_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tCount(tint) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tCount(tbigint) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE tMin(tint) (
  SFUNC = tint_tmin_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tmin_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tMinAgg(tint) (
  SFUNC = tint_tmin_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tmin_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE tMax(tint) (
  SFUNC = tint_tmax_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tmax_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tMaxAgg(tint) (
  SFUNC = tint_tmax_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tmax_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tSum(tint) (
  SFUNC = tint_tsum_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tAvg(tint) (
  SFUNC = tavg_transfn,
  STYPE = internal,
  COMBINEFUNC = tavg_combinefn,
  FINALFUNC = tavg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE tMin(tbigint) (
  SFUNC = tbigint_tmin_transfn,
  STYPE = internal,
  COMBINEFUNC = tbigint_tmin_combinefn,
  FINALFUNC = tbigint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tMinAgg(tbigint) (
  SFUNC = tbigint_tmin_transfn,
  STYPE = internal,
  COMBINEFUNC = tbigint_tmin_combinefn,
  FINALFUNC = tbigint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE tMax(tbigint) (
  SFUNC = tbigint_tmax_transfn,
  STYPE = internal,
  COMBINEFUNC = tbigint_tmax_combinefn,
  FINALFUNC = tbigint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tMaxAgg(tbigint) (
  SFUNC = tbigint_tmax_transfn,
  STYPE = internal,
  COMBINEFUNC = tbigint_tmax_combinefn,
  FINALFUNC = tbigint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tSum(tbigint) (
  SFUNC = tbigint_tsum_transfn,
  STYPE = internal,
  COMBINEFUNC = tbigint_tsum_combinefn,
  FINALFUNC = tbigint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tAvg(tbigint) (
  SFUNC = tavg_transfn,
  STYPE = internal,
  COMBINEFUNC = tavg_combinefn,
  FINALFUNC = tavg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

-- The function is not strict
CREATE FUNCTION tCountTransition(internal, tfloat)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tfloat_tmin_transfn(internal, tfloat)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tfloat_tmin_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tmin_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tfloat_tmin_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tmax_transfn(internal, tfloat)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tfloat_tmax_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tmax_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tfloat_tmax_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tsum_transfn(internal, tfloat)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tfloat_tsum_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tsum_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tfloat_tsum_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tagg_finalfn(internal)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION tavg_transfn(internal, tfloat)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tnumber_tavg_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE tCount(tfloat) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE tMin(tfloat) (
  SFUNC = tfloat_tmin_transfn,
  STYPE = internal,
  COMBINEFUNC = tfloat_tmin_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tMinAgg(tfloat) (
  SFUNC = tfloat_tmin_transfn,
  STYPE = internal,
  COMBINEFUNC = tfloat_tmin_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE tMax(tfloat) (
  SFUNC = tfloat_tmax_transfn,
  STYPE = internal,
  COMBINEFUNC = tfloat_tmax_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tMaxAgg(tfloat) (
  SFUNC = tfloat_tmax_transfn,
  STYPE = internal,
  COMBINEFUNC = tfloat_tmax_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tSum(tfloat) (
  SFUNC = tfloat_tsum_transfn,
  STYPE = internal,
  COMBINEFUNC = tfloat_tsum_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tAvg(tfloat) (
  SFUNC = tavg_transfn,
  STYPE = internal,
  COMBINEFUNC = tavg_combinefn,
  FINALFUNC = tavg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

-- The function is not strict
CREATE FUNCTION tCountTransition(internal, ttext)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION ttext_tmin_transfn(internal, ttext)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Ttext_tmin_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tmin_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Ttext_tmin_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tmax_transfn(internal, ttext)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Ttext_tmax_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tmax_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Ttext_tmax_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tagg_finalfn(internal)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tCount(ttext) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE tMin(ttext) (
  SFUNC = ttext_tmin_transfn,
  STYPE = internal,
  COMBINEFUNC = ttext_tmin_combinefn,
  FINALFUNC = ttext_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tMinAgg(ttext) (
  SFUNC = ttext_tmin_transfn,
  STYPE = internal,
  COMBINEFUNC = ttext_tmin_combinefn,
  FINALFUNC = ttext_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE tMax(ttext) (
  SFUNC = ttext_tmax_transfn,
  STYPE = internal,
  COMBINEFUNC = ttext_tmax_combinefn,
  FINALFUNC = ttext_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tMaxAgg(ttext) (
  SFUNC = ttext_tmax_transfn,
  STYPE = internal,
  COMBINEFUNC = ttext_tmax_combinefn,
  FINALFUNC = ttext_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

-- The function is not strict
CREATE FUNCTION mergeTransition(internal, tbool)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION mergeTransition(internal, tint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION mergeTransition(internal, tbigint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION mergeTransition(internal, tfloat)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION mergeTransition(internal, ttext)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION mergeCombine(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE merge(tbool) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tbool_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE mergeAgg(tbool) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tbool_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE merge(tint) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE mergeAgg(tint) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE merge(tbigint) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tbigint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE mergeAgg(tbigint) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tbigint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE merge(tfloat) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tfloat_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE mergeAgg(tfloat) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tfloat_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE merge(ttext) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = ttext_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE mergeAgg(ttext) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = ttext_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

/*****************************************************************************
 * Append aggregate functions
 *****************************************************************************/

-- Default interpolation based on the base type
CREATE FUNCTION appendInstantTransition(tbool, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(tbigint, tbigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(ttext, ttext)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Interpolation given by the user
CREATE FUNCTION appendInstantTransition(tbool, tbool, interp text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(tint, tint, interp text)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(tbigint, tbigint, interp text)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(tfloat, tfloat, interp text)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(ttext, ttext, interp text)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- These functions are not strict
CREATE FUNCTION appendInstantTransition(tbool, tbool, interp text,
    maxt interval)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(tint, tint, interp text,
    maxdist float, maxt interval)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(tbigint, tbigint, interp text,
    maxdist float, maxt interval)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(tfloat, tfloat, interp text,
    maxdist float, maxt interval)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(ttext, ttext, interp text,
    maxt interval)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION temporal_append_finalfn(tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_append_finalfn(tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_append_finalfn(tbigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_append_finalfn(tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_append_finalfn(ttext)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tbool) (
  SFUNC = appendInstantTransition(tbool, tbool),
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tbool) (
  SFUNC = appendInstantTransition(tbool, tbool),
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tbool, interp text) (
  SFUNC = appendInstantTransition(tbool, tbool, text),
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tbool, interp text) (
  SFUNC = appendInstantTransition(tbool, tbool, text),
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tbool, interp text, maxt interval) (
  SFUNC = appendInstantTransition(tbool, tbool, text, maxt),
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tbool, interp text, maxt interval) (
  SFUNC = appendInstantTransition(tbool, tbool, text, maxt),
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tint) (
  SFUNC = appendInstantTransition(tint, tint),
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tint) (
  SFUNC = appendInstantTransition(tint, tint),
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tint, interp text) (
  SFUNC = appendInstantTransition(tint, tint, text),
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tint, interp text) (
  SFUNC = appendInstantTransition(tint, tint, text),
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tint, interp text, maxdist float, 
    maxt interval) (
  SFUNC = appendInstantTransition(tint, tint, text, maxdist, maxt),
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tint, interp text, maxdist float, 
    maxt interval) (
  SFUNC = appendInstantTransition(tint, tint, text, maxdist, maxt),
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tbigint) (
  SFUNC = appendInstantTransition(tbigint, tbigint),
  STYPE = tbigint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tbigint) (
  SFUNC = appendInstantTransition(tbigint, tbigint),
  STYPE = tbigint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tbigint, interp text) (
  SFUNC = appendInstantTransition(tbigint, tbigint, text),
  STYPE = tbigint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tbigint, interp text) (
  SFUNC = appendInstantTransition(tbigint, tbigint, text),
  STYPE = tbigint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tbigint, interp text, maxdist float, 
    maxt interval) (
  SFUNC = appendInstantTransition(tbigint, tbigint, text, maxdist, maxt),
  STYPE = tbigint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tbigint, interp text, maxdist float, 
    maxt interval) (
  SFUNC = appendInstantTransition(tbigint, tbigint, text, maxdist, maxt),
  STYPE = tbigint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tfloat) (
  SFUNC = appendInstantTransition(tfloat, tfloat),
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tfloat) (
  SFUNC = appendInstantTransition(tfloat, tfloat),
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tfloat, interp text) (
  SFUNC = appendInstantTransition(tfloat, tfloat, text),
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tfloat, interp text) (
  SFUNC = appendInstantTransition(tfloat, tfloat, text),
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tfloat, interp text, maxdist float, 
    maxt interval) (
  SFUNC = appendInstantTransition(tfloat, tfloat, text, maxdist, maxt),
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tfloat, interp text, maxdist float, 
    maxt interval) (
  SFUNC = appendInstantTransition(tfloat, tfloat, text, maxdist, maxt),
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(ttext) (
  SFUNC = appendInstantTransition(ttext, ttext),
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(ttext) (
  SFUNC = appendInstantTransition(ttext, ttext),
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(ttext, interp text) (
  SFUNC = appendInstantTransition(ttext, ttext, text),
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(ttext, interp text) (
  SFUNC = appendInstantTransition(ttext, ttext, text),
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(ttext, interp text, maxt interval) (
  SFUNC = appendInstantTransition(ttext, ttext, text, maxt),
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(ttext, interp text, maxt interval) (
  SFUNC = appendInstantTransition(ttext, ttext, text, maxt),
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION appendSequenceTransition(tbool, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendSequenceTransition(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendSequenceTransition(tbigint, tbigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendSequenceTransition(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendSequenceTransition(ttext, ttext)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendSequence(tbool) (
  SFUNC = appendSequenceTransition,
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequenceAgg(tbool) (
  SFUNC = appendSequenceTransition,
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendSequence(tint) (
  SFUNC = appendSequenceTransition,
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequenceAgg(tint) (
  SFUNC = appendSequenceTransition,
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendSequence(tbigint) (
  SFUNC = appendSequenceTransition,
  STYPE = tbigint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequenceAgg(tbigint) (
  SFUNC = appendSequenceTransition,
  STYPE = tbigint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendSequence(tfloat) (
  SFUNC = appendSequenceTransition,
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequenceAgg(tfloat) (
  SFUNC = appendSequenceTransition,
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendSequence(ttext) (
  SFUNC = appendSequenceTransition,
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequenceAgg(ttext) (
  SFUNC = appendSequenceTransition,
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/
