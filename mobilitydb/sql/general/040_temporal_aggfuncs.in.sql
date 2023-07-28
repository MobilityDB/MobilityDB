/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Temporal aggregate functions
 */

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
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = temporal_extent_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  PARALLEL = safe
);
CREATE AGGREGATE extent(ttext) (
  SFUNC = temporal_extent_transfn,
  STYPE = tstzspan,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = temporal_extent_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  PARALLEL = safe
);

CREATE FUNCTION tnumber_extent_transfn(tbox, tint)
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
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tnumber_extent_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  PARALLEL = safe
);
CREATE AGGREGATE extent(tfloat) (
  SFUNC = tnumber_extent_transfn,
  STYPE = tbox,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tnumber_extent_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  PARALLEL = safe
);

/*****************************************************************************/

CREATE FUNCTION tcount_transfn(internal, tbool)
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

CREATE AGGREGATE tcount(tbool) (
  SFUNC = tcount_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tcount_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tand(tbool) (
  SFUNC = tbool_tand_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tbool_tand_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tbool_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tor(tbool) (
  SFUNC = tbool_tor_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tbool_tor_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tbool_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

CREATE FUNCTION tcount_transfn(internal, tint)
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

CREATE FUNCTION tavg_transfn(internal, tint)
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

CREATE AGGREGATE tcount(tint) (
  SFUNC = tcount_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tcount_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tmin(tint) (
  SFUNC = tint_tmin_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tint_tmin_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tmax(tint) (
  SFUNC = tint_tmax_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tint_tmax_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tsum(tint) (
  SFUNC = tint_tsum_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tint_tsum_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tavg(tint) (
  SFUNC = tavg_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tavg_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tavg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

CREATE FUNCTION tcount_transfn(internal, tfloat)
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
CREATE FUNCTION tavg_transfn(internal, tfloat)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tnumber_tavg_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE tcount(tfloat) (
  SFUNC = tcount_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tcount_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tmin(tfloat) (
  SFUNC = tfloat_tmin_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tfloat_tmin_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tmax(tfloat) (
  SFUNC = tfloat_tmax_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tfloat_tmax_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tsum(tfloat) (
  SFUNC = tfloat_tsum_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tfloat_tsum_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tavg(tfloat) (
  SFUNC = tavg_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tavg_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tavg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

CREATE FUNCTION tcount_transfn(internal, ttext)
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

CREATE AGGREGATE tcount(ttext) (
  SFUNC = tcount_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tcount_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tmin(ttext) (
  SFUNC = ttext_tmin_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = ttext_tmin_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = ttext_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tmax(ttext) (
  SFUNC = ttext_tmax_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = ttext_tmax_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = ttext_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

CREATE FUNCTION temporal_merge_transfn(internal, tbool)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_merge_transfn(internal, tint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_merge_transfn(internal, tfloat)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_merge_transfn(internal, ttext)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION temporal_merge_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE merge(tbool) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = temporal_merge_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tbool_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE merge(tint) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = temporal_merge_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE merge(tfloat) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = temporal_merge_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE merge(ttext) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = temporal_merge_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = ttext_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = safe
);

/*****************************************************************************
 * Append aggregate functions
 *****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION temporal_app_tinst_transfn(tbool, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_app_tinst_transfn(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_app_tinst_transfn(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_app_tinst_transfn(ttext, ttext)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

-- The function is not STRICT
CREATE FUNCTION temporal_app_tinst_transfn(tbool, tbool,
    maxt interval DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_app_tinst_transfn(tint, tint,
    maxdist float DEFAULT NULL, maxt interval DEFAULT NULL)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_app_tinst_transfn(tfloat, tfloat,
    maxdist float DEFAULT NULL, maxt interval DEFAULT NULL)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_app_tinst_transfn(ttext, ttext,
    maxt interval DEFAULT NULL)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION temporal_append_finalfn(tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_append_finalfn(tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_append_finalfn(tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_append_finalfn(ttext)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE appendInstant(tbool) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstant(tint) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstant(tfloat) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstant(ttext) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE appendInstant(tbool, interval) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstant(tint, float, interval) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstant(tfloat, float, interval) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstant(ttext, interval) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION temporal_app_tseq_transfn(tbool, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_app_tseq_transfn(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_app_tseq_transfn(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_app_tseq_transfn(ttext, ttext)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE appendSequence(tbool) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = tbool,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequence(tint) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = tint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequence(tfloat) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = tfloat,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequence(ttext) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = ttext,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/
