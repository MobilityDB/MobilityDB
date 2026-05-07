/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS
 * TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief Forward-compatible aggregate aliases per RFC #827.
 *
 * Each aggregate registered by the previous SQL files is exposed under a
 * Pascal-cased *Agg name in addition to its original lowercase or
 * camelCase name. This eliminates the case-folding collision between the
 * scalar accessors `Tmin(tbox|stbox)` / `Tmax(tbox|stbox)` and the
 * temporal-min / temporal-max aggregates, which previously differed only
 * in the casing of their first letter and so collapsed to the same
 * canonical name in catalogs that are case-insensitive but key on
 * `(name, kind)` rather than `(name, argtypes, kind)` (DuckDB, BigQuery,
 * Snowflake, Trino, ...).
 *
 * The original names remain valid for backward compatibility. Once
 * downstream tools have migrated, a future major version may drop the
 * un-suffixed aliases.
 *
 * The new aggregates share the same internal C transition functions as
 * the originals, so the only runtime cost is one extra row in
 * `pg_aggregate` per aggregate.
 */

-- From 015_span_aggfuncs.in.sql
CREATE AGGREGATE SpanUnionAgg(intspan) (
  SFUNC = array_agg_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = intspan_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SpanUnionAgg(bigintspan) (
  SFUNC = array_agg_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = bigintspan_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SpanUnionAgg(floatspan) (
  SFUNC = array_agg_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = floatspan_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SpanUnionAgg(datespan) (
  SFUNC = array_agg_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = datespan_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SpanUnionAgg(tstzspan) (
  SFUNC = array_agg_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = tstzspan_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SpanUnionAgg(intspanset) (
  SFUNC = spanset_union_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = intspan_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SpanUnionAgg(bigintspanset) (
  SFUNC = spanset_union_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = bigintspan_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SpanUnionAgg(floatspanset) (
  SFUNC = spanset_union_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = floatspan_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SpanUnionAgg(datespanset) (
  SFUNC = spanset_union_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = datespan_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SpanUnionAgg(tstzspanset) (
  SFUNC = spanset_union_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = tstzspan_union_finalfn,
  PARALLEL = safe
);

-- From 001_set.in.sql
CREATE AGGREGATE SetUnionAgg(integer) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = intset_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SetUnionAgg(bigint) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = bigintset_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SetUnionAgg(float) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = floatset_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SetUnionAgg(text) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = textset_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SetUnionAgg(date) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = dateset_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SetUnionAgg(timestamptz) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = tstzset_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SetUnionAgg(intset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = intset_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SetUnionAgg(bigintset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = bigintset_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SetUnionAgg(floatset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = floatset_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SetUnionAgg(textset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = textset_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SetUnionAgg(dateset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = dateset_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE SetUnionAgg(tstzset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = tstzset_union_finalfn,
  PARALLEL = safe
);

-- From 040_temporal_aggfuncs.in.sql
CREATE AGGREGATE TcountAgg(timestamptz) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TcountAgg(tstzset) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TcountAgg(tstzspan) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TcountAgg(tstzspanset) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TcountAgg(tbool) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TandAgg(tbool) (
  SFUNC = tbool_tand_transfn,
  STYPE = internal,
  COMBINEFUNC = tbool_tand_combinefn,
  FINALFUNC = tbool_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TorAgg(tbool) (
  SFUNC = tbool_tor_transfn,
  STYPE = internal,
  COMBINEFUNC = tbool_tor_combinefn,
  FINALFUNC = tbool_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TcountAgg(tint) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TminAgg(tint) (
  SFUNC = tint_tmin_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tmin_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TmaxAgg(tint) (
  SFUNC = tint_tmax_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tmax_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TsumAgg(tint) (
  SFUNC = tint_tsum_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TavgAgg(tint) (
  SFUNC = tavg_transfn,
  STYPE = internal,
  COMBINEFUNC = tavg_combinefn,
  FINALFUNC = tavg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TcountAgg(tfloat) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TminAgg(tfloat) (
  SFUNC = tfloat_tmin_transfn,
  STYPE = internal,
  COMBINEFUNC = tfloat_tmin_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TmaxAgg(tfloat) (
  SFUNC = tfloat_tmax_transfn,
  STYPE = internal,
  COMBINEFUNC = tfloat_tmax_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TsumAgg(tfloat) (
  SFUNC = tfloat_tsum_transfn,
  STYPE = internal,
  COMBINEFUNC = tfloat_tsum_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TavgAgg(tfloat) (
  SFUNC = tavg_transfn,
  STYPE = internal,
  COMBINEFUNC = tavg_combinefn,
  FINALFUNC = tavg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TcountAgg(ttext) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TminAgg(ttext) (
  SFUNC = ttext_tmin_transfn,
  STYPE = internal,
  COMBINEFUNC = ttext_tmin_combinefn,
  FINALFUNC = ttext_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE TmaxAgg(ttext) (
  SFUNC = ttext_tmax_transfn,
  STYPE = internal,
  COMBINEFUNC = ttext_tmax_combinefn,
  FINALFUNC = ttext_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE MergeAgg(tbool) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
  COMBINEFUNC = temporal_merge_combinefn,
  FINALFUNC = tbool_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

CREATE AGGREGATE MergeAgg(tint) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
  COMBINEFUNC = temporal_merge_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

CREATE AGGREGATE MergeAgg(tfloat) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
  COMBINEFUNC = temporal_merge_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

CREATE AGGREGATE MergeAgg(ttext) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
  COMBINEFUNC = temporal_merge_combinefn,
  FINALFUNC = ttext_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tbool) (
  SFUNC = temporal_app_tinst_transfn(tbool, tbool),
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tbool, interp text) (
  SFUNC = temporal_app_tinst_transfn(tbool, tbool, text),
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tbool, interp text, maxt interval) (
  SFUNC = temporal_app_tinst_transfn(tbool, tbool, text, maxt),
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tint) (
  SFUNC = temporal_app_tinst_transfn(tint, tint),
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tint, interp text) (
  SFUNC = temporal_app_tinst_transfn(tint, tint, text),
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tint, interp text, maxdist float, 
    maxt interval) (
  SFUNC = temporal_app_tinst_transfn(tint, tint, text, maxdist, maxt),
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tfloat) (
  SFUNC = temporal_app_tinst_transfn(tfloat, tfloat),
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tfloat, interp text) (
  SFUNC = temporal_app_tinst_transfn(tfloat, tfloat, text),
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(tfloat, interp text, maxdist float, 
    maxt interval) (
  SFUNC = temporal_app_tinst_transfn(tfloat, tfloat, text, maxdist, maxt),
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(ttext) (
  SFUNC = temporal_app_tinst_transfn(ttext, ttext),
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(ttext, interp text) (
  SFUNC = temporal_app_tinst_transfn(ttext, ttext, text),
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendInstantAgg(ttext, interp text, maxt interval) (
  SFUNC = temporal_app_tinst_transfn(ttext, ttext, text, maxt),
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendSequenceAgg(tbool) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendSequenceAgg(tint) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendSequenceAgg(tfloat) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE AppendSequenceAgg(ttext) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

-- From 042_temporal_waggfuncs.in.sql
CREATE AGGREGATE WminAgg(tint, interval) (
  SFUNC = tint_wmin_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tmin_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE WmaxAgg(tint, interval) (
  SFUNC = tint_wmax_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tmax_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE WsumAgg(tint, interval) (
  SFUNC = tint_wsum_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE WcountAgg(tint, interval) (
  SFUNC = wcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE WavgAgg(tint, interval) (
  SFUNC = wavg_transfn,
  STYPE = internal,
  COMBINEFUNC = tavg_combinefn,
  FINALFUNC = tavg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE WminAgg(tfloat, interval) (
  SFUNC = tfloat_wmin_transfn,
  STYPE = internal,
  COMBINEFUNC = tfloat_tmin_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE WmaxAgg(tfloat, interval) (
  SFUNC = tfloat_wmax_transfn,
  STYPE = internal,
  COMBINEFUNC = tfloat_tmax_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE WsumAgg(tfloat, interval) (
  SFUNC = tfloat_wsum_transfn,
  STYPE = internal,
  COMBINEFUNC = tfloat_tsum_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE WcountAgg(tfloat, interval) (
  SFUNC = wcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE WavgAgg(tfloat, interval) (
  SFUNC = wavg_transfn,
  STYPE = internal,
  COMBINEFUNC = tavg_combinefn,
  FINALFUNC = tavg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

