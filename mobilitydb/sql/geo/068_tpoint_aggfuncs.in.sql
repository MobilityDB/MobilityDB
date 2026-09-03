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
 * @brief Aggregate functions for temporal points
 */

-- The function is not strict
CREATE FUNCTION tspatial_extent_transfn(stbox, tgeompoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tspatial_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tspatial_extent_transfn(stbox, tgeogpoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tspatial_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tgeompoint) (
  SFUNC = tspatial_extent_transfn,
  STYPE = stbox,
  COMBINEFUNC = stbox_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(tgeogpoint) (
  SFUNC = tspatial_extent_transfn,
  STYPE = stbox,
  COMBINEFUNC = stbox_extent_combinefn,
  PARALLEL = safe
);

/*****************************************************************************/

-- The function is not strict
CREATE FUNCTION tCountTransition(internal, tgeompoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tCountTransition(internal, tgeogpoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE tCount(tgeompoint) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tCount(tgeogpoint) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

-- The function is not strict
CREATE FUNCTION wCountTransition(internal, tgeompoint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wCountTransition(internal, tgeogpoint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE wCount(tgeompoint, interval) (
  SFUNC = wCountTransition,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE wCount(tgeogpoint, interval) (
  SFUNC = wCountTransition,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

-- The function is not strict
CREATE FUNCTION tcentroid_transfn(internal, tgeompoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tpoint_tcentroid_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcentroid_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tpoint_tcentroid_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcentroid_finalfn(internal)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Tpoint_tcentroid_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tCentroid(tgeompoint) (
  SFUNC = tcentroid_transfn,
  STYPE = internal,
  COMBINEFUNC = tcentroid_combinefn,
  FINALFUNC = tcentroid_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

-- The function is not strict
CREATE FUNCTION mergeTransition(internal, tgeompoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION mergeTransition(internal, tgeogpoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tgeompoint_tagg_finalfn(internal)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpoint_tagg_finalfn(internal)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE merge(tgeompoint) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tgeompoint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE mergeAgg(tgeompoint) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tgeompoint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE merge(tgeogpoint) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tgeogpoint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE mergeAgg(tgeogpoint) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tgeogpoint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

/*****************************************************************************
 * Append tinstant aggregate functions
 *****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION appendInstantTransition(tgeompoint, tgeompoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(tgeogpoint, tgeogpoint)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

-- The function is not STRICT
CREATE FUNCTION appendInstantTransition(tgeompoint, tgeompoint,
    interp text DEFAULT NULL)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(tgeogpoint, tgeogpoint,
    interp text DEFAULT NULL)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

-- The function is not STRICT
CREATE FUNCTION appendInstantTransition(tgeompoint, tgeompoint,
    interp text DEFAULT NULL, maxdist float DEFAULT NULL,
    maxt interval DEFAULT NULL)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(tgeogpoint, tgeogpoint,
    interp text DEFAULT NULL, maxdist float DEFAULT NULL, 
    maxt interval DEFAULT NULL)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION temporal_append_finalfn(tgeompoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_append_finalfn(tgeogpoint)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tgeompoint) (
  SFUNC = appendInstantTransition,
  STYPE = tgeompoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tgeompoint) (
  SFUNC = appendInstantTransition,
  STYPE = tgeompoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tgeogpoint) (
  SFUNC = appendInstantTransition,
  STYPE = tgeogpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tgeogpoint) (
  SFUNC = appendInstantTransition,
  STYPE = tgeogpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tgeompoint, text) (
  SFUNC = appendInstantTransition,
  STYPE = tgeompoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tgeompoint, text) (
  SFUNC = appendInstantTransition,
  STYPE = tgeompoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tgeogpoint, text) (
  SFUNC = appendInstantTransition,
  STYPE = tgeogpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tgeogpoint, text) (
  SFUNC = appendInstantTransition,
  STYPE = tgeogpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tgeompoint, text, float, interval) (
  SFUNC = appendInstantTransition,
  STYPE = tgeompoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tgeompoint, text, float, interval) (
  SFUNC = appendInstantTransition,
  STYPE = tgeompoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tgeogpoint, text, float, interval) (
  SFUNC = appendInstantTransition,
  STYPE = tgeogpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tgeogpoint, text, float, interval) (
  SFUNC = appendInstantTransition,
  STYPE = tgeogpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION appendSequenceTransition(tgeompoint, tgeompoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendSequenceTransition(tgeogpoint, tgeogpoint)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendSequence(tgeompoint) (
  SFUNC = appendSequenceTransition,
  STYPE = tgeompoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequenceAgg(tgeompoint) (
  SFUNC = appendSequenceTransition,
  STYPE = tgeompoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendSequence(tgeogpoint) (
  SFUNC = appendSequenceTransition,
  STYPE = tgeogpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequenceAgg(tgeogpoint) (
  SFUNC = appendSequenceTransition,
  STYPE = tgeogpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/
