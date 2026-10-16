/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
CREATE FUNCTION extentTransition(tstzspan, tbool)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Temporal_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentTransition(tstzspan, ttext)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Temporal_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tbool) (
  SFUNC = extentTransition,
  STYPE = tstzspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(ttext) (
  SFUNC = extentTransition,
  STYPE = tstzspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);

-- The function is not strict
CREATE FUNCTION extentTransition(tbox, tint)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tnumber_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentTransition(tbox, tbigint)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tnumber_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentTransition(tbox, tfloat)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tnumber_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tint) (
  SFUNC = extentTransition,
  STYPE = tbox,
  COMBINEFUNC = tbox_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(tbigint) (
  SFUNC = extentTransition,
  STYPE = tbox,
  COMBINEFUNC = tbox_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(tfloat) (
  SFUNC = extentTransition,
  STYPE = tbox,
  COMBINEFUNC = tbox_extent_combinefn,
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

CREATE FUNCTION tAndTransition(internal, tbool)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbool_tand_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tand_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbool_tand_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tOrTransition(internal, tbool)
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
  SFUNC = tAndTransition,
  STYPE = internal,
  COMBINEFUNC = tbool_tand_combinefn,
  FINALFUNC = tbool_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tOr(tbool) (
  SFUNC = tOrTransition,
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

CREATE FUNCTION tMinAggTransition(internal, tint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tint_tmin_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tmin_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tint_tmin_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tMaxAggTransition(internal, tint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tint_tmax_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tmax_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tint_tmax_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tSumTransition(internal, tint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tint_tsum_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tsum_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tint_tsum_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tMinAggTransition(internal, tbigint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbigint_tmin_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbigint_tmin_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbigint_tmin_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tMaxAggTransition(internal, tbigint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbigint_tmax_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbigint_tmax_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbigint_tmax_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tSumTransition(internal, tbigint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbigint_tsum_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbigint_tsum_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbigint_tsum_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tAvgTransition(internal, tint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tnumber_tavg_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tAvgTransition(internal, tbigint)
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
  SFUNC = tMinAggTransition,
  STYPE = internal,
  COMBINEFUNC = tint_tmin_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tMinAgg(tint) (
  SFUNC = tMinAggTransition,
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
  SFUNC = tMaxAggTransition,
  STYPE = internal,
  COMBINEFUNC = tint_tmax_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tMaxAgg(tint) (
  SFUNC = tMaxAggTransition,
  STYPE = internal,
  COMBINEFUNC = tint_tmax_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tSum(tint) (
  SFUNC = tSumTransition,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tAvg(tint) (
  SFUNC = tAvgTransition,
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
  SFUNC = tMinAggTransition,
  STYPE = internal,
  COMBINEFUNC = tbigint_tmin_combinefn,
  FINALFUNC = tbigint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tMinAgg(tbigint) (
  SFUNC = tMinAggTransition,
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
  SFUNC = tMaxAggTransition,
  STYPE = internal,
  COMBINEFUNC = tbigint_tmax_combinefn,
  FINALFUNC = tbigint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tMaxAgg(tbigint) (
  SFUNC = tMaxAggTransition,
  STYPE = internal,
  COMBINEFUNC = tbigint_tmax_combinefn,
  FINALFUNC = tbigint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tSum(tbigint) (
  SFUNC = tSumTransition,
  STYPE = internal,
  COMBINEFUNC = tbigint_tsum_combinefn,
  FINALFUNC = tbigint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tAvg(tbigint) (
  SFUNC = tAvgTransition,
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

CREATE FUNCTION tMinAggTransition(internal, tfloat)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tfloat_tmin_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tmin_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tfloat_tmin_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tMaxAggTransition(internal, tfloat)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tfloat_tmax_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tmax_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tfloat_tmax_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tSumTransition(internal, tfloat)
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
CREATE FUNCTION tAvgTransition(internal, tfloat)
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
  SFUNC = tMinAggTransition,
  STYPE = internal,
  COMBINEFUNC = tfloat_tmin_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tMinAgg(tfloat) (
  SFUNC = tMinAggTransition,
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
  SFUNC = tMaxAggTransition,
  STYPE = internal,
  COMBINEFUNC = tfloat_tmax_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tMaxAgg(tfloat) (
  SFUNC = tMaxAggTransition,
  STYPE = internal,
  COMBINEFUNC = tfloat_tmax_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tSum(tfloat) (
  SFUNC = tSumTransition,
  STYPE = internal,
  COMBINEFUNC = tfloat_tsum_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tAvg(tfloat) (
  SFUNC = tAvgTransition,
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

CREATE FUNCTION tMinAggTransition(internal, ttext)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Ttext_tmin_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tmin_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Ttext_tmin_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tMaxAggTransition(internal, ttext)
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
  SFUNC = tMinAggTransition,
  STYPE = internal,
  COMBINEFUNC = ttext_tmin_combinefn,
  FINALFUNC = ttext_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tMinAgg(ttext) (
  SFUNC = tMinAggTransition,
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
  SFUNC = tMaxAggTransition,
  STYPE = internal,
  COMBINEFUNC = ttext_tmax_combinefn,
  FINALFUNC = ttext_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tMaxAgg(ttext) (
  SFUNC = tMaxAggTransition,
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
CREATE FUNCTION mergeAggTransition(internal, tbool)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION mergeAggTransition(internal, tint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION mergeAggTransition(internal, tbigint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION mergeAggTransition(internal, tfloat)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION mergeAggTransition(internal, ttext)
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
  SFUNC = mergeAggTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tbool_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE mergeAgg(tbool) (
  SFUNC = mergeAggTransition,
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
  SFUNC = mergeAggTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE mergeAgg(tint) (
  SFUNC = mergeAggTransition,
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
  SFUNC = mergeAggTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tbigint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE mergeAgg(tbigint) (
  SFUNC = mergeAggTransition,
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
  SFUNC = mergeAggTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tfloat_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE mergeAgg(tfloat) (
  SFUNC = mergeAggTransition,
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
  SFUNC = mergeAggTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = ttext_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE mergeAgg(ttext) (
  SFUNC = mergeAggTransition,
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
CREATE FUNCTION appendInstantAggTransition(tbool, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstantAggTransition(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstantAggTransition(tbigint, tbigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstantAggTransition(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstantAggTransition(ttext, ttext)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Interpolation given by the user
CREATE FUNCTION appendInstantAggTransition(tbool, tbool, interp text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstantAggTransition(tint, tint, interp text)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstantAggTransition(tbigint, tbigint, interp text)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstantAggTransition(tfloat, tfloat, interp text)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstantAggTransition(ttext, ttext, interp text)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- These functions are not strict
CREATE FUNCTION appendInstantAggTransition(tbool, tbool, interp text,
    maxt interval)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendInstantAggTransition(tint, tint, interp text,
    maxdist float, maxt interval)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendInstantAggTransition(tbigint, tbigint, interp text,
    maxdist float, maxt interval)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendInstantAggTransition(tfloat, tfloat, interp text,
    maxdist float, maxt interval)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendInstantAggTransition(ttext, ttext, interp text,
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
  SFUNC = appendInstantAggTransition(tbool, tbool),
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tbool) (
  SFUNC = appendInstantAggTransition(tbool, tbool),
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tbool, interp text) (
  SFUNC = appendInstantAggTransition(tbool, tbool, text),
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tbool, interp text) (
  SFUNC = appendInstantAggTransition(tbool, tbool, text),
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tbool, interp text, maxt interval) (
  SFUNC = appendInstantAggTransition(tbool, tbool, text, maxt),
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tbool, interp text, maxt interval) (
  SFUNC = appendInstantAggTransition(tbool, tbool, text, maxt),
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tint) (
  SFUNC = appendInstantAggTransition(tint, tint),
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tint) (
  SFUNC = appendInstantAggTransition(tint, tint),
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tint, interp text) (
  SFUNC = appendInstantAggTransition(tint, tint, text),
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tint, interp text) (
  SFUNC = appendInstantAggTransition(tint, tint, text),
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tint, interp text, maxdist float, 
    maxt interval) (
  SFUNC = appendInstantAggTransition(tint, tint, text, maxdist, maxt),
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tint, interp text, maxdist float, 
    maxt interval) (
  SFUNC = appendInstantAggTransition(tint, tint, text, maxdist, maxt),
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tbigint) (
  SFUNC = appendInstantAggTransition(tbigint, tbigint),
  STYPE = tbigint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tbigint) (
  SFUNC = appendInstantAggTransition(tbigint, tbigint),
  STYPE = tbigint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tbigint, interp text) (
  SFUNC = appendInstantAggTransition(tbigint, tbigint, text),
  STYPE = tbigint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tbigint, interp text) (
  SFUNC = appendInstantAggTransition(tbigint, tbigint, text),
  STYPE = tbigint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tbigint, interp text, maxdist float, 
    maxt interval) (
  SFUNC = appendInstantAggTransition(tbigint, tbigint, text, maxdist, maxt),
  STYPE = tbigint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tbigint, interp text, maxdist float, 
    maxt interval) (
  SFUNC = appendInstantAggTransition(tbigint, tbigint, text, maxdist, maxt),
  STYPE = tbigint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tfloat) (
  SFUNC = appendInstantAggTransition(tfloat, tfloat),
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tfloat) (
  SFUNC = appendInstantAggTransition(tfloat, tfloat),
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tfloat, interp text) (
  SFUNC = appendInstantAggTransition(tfloat, tfloat, text),
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tfloat, interp text) (
  SFUNC = appendInstantAggTransition(tfloat, tfloat, text),
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tfloat, interp text, maxdist float, 
    maxt interval) (
  SFUNC = appendInstantAggTransition(tfloat, tfloat, text, maxdist, maxt),
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tfloat, interp text, maxdist float, 
    maxt interval) (
  SFUNC = appendInstantAggTransition(tfloat, tfloat, text, maxdist, maxt),
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(ttext) (
  SFUNC = appendInstantAggTransition(ttext, ttext),
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(ttext) (
  SFUNC = appendInstantAggTransition(ttext, ttext),
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(ttext, interp text) (
  SFUNC = appendInstantAggTransition(ttext, ttext, text),
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(ttext, interp text) (
  SFUNC = appendInstantAggTransition(ttext, ttext, text),
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(ttext, interp text, maxt interval) (
  SFUNC = appendInstantAggTransition(ttext, ttext, text, maxt),
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(ttext, interp text, maxt interval) (
  SFUNC = appendInstantAggTransition(ttext, ttext, text, maxt),
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION appendSequenceAggTransition(tbool, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendSequenceAggTransition(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendSequenceAggTransition(tbigint, tbigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendSequenceAggTransition(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendSequenceAggTransition(ttext, ttext)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendSequence(tbool) (
  SFUNC = appendSequenceAggTransition,
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequenceAgg(tbool) (
  SFUNC = appendSequenceAggTransition,
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendSequence(tint) (
  SFUNC = appendSequenceAggTransition,
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequenceAgg(tint) (
  SFUNC = appendSequenceAggTransition,
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendSequence(tbigint) (
  SFUNC = appendSequenceAggTransition,
  STYPE = tbigint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequenceAgg(tbigint) (
  SFUNC = appendSequenceAggTransition,
  STYPE = tbigint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendSequence(tfloat) (
  SFUNC = appendSequenceAggTransition,
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequenceAgg(tfloat) (
  SFUNC = appendSequenceAggTransition,
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendSequence(ttext) (
  SFUNC = appendSequenceAggTransition,
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequenceAgg(ttext) (
  SFUNC = appendSequenceAggTransition,
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/
