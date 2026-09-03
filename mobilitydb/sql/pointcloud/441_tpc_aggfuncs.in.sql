-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

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
 *****************************************************************************/

/**
 * @file
 * @brief Aggregate functions for the pgPointCloud temporal types.
 */

CREATE FUNCTION tpcbox_extent_transfn(tpcbox, tpcbox)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpcbox_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tpc_extent_transfn(tpcbox, tpcpoint)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpc_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tpc_extent_transfn(tpcbox, tpcpatch)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpc_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tpcbox) (
  SFUNC = tpcbox_extent_transfn,
  STYPE = tpcbox,
  COMBINEFUNC = tpcbox_extent_transfn,
  PARALLEL = safe
);

CREATE AGGREGATE extent(tpcpoint) (
  SFUNC = tpc_extent_transfn,
  STYPE = tpcbox,
  COMBINEFUNC = tpcbox_extent_transfn,
  PARALLEL = safe
);

CREATE AGGREGATE extent(tpcpatch) (
  SFUNC = tpc_extent_transfn,
  STYPE = tpcbox,
  COMBINEFUNC = tpcbox_extent_transfn,
  PARALLEL = safe
);

/*****************************************************************************
 * Temporal count and window count
 *****************************************************************************/

CREATE FUNCTION tCountTransition(internal, tpcpoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tCountTransition(internal, tpcpatch)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE tCount(tpcpoint) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tCount(tpcpatch) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

-- tnpoints(tpcpatch) — running temporal sum of per-instant pcpatch
-- npoints. Distinct from tcount(tpcpatch), which counts the number
-- of patches (always 1 per instant). For drone LiDAR this is "how
-- many points were in the cloud at time t".
CREATE FUNCTION tnpoints_transfn(internal, tpcpatch)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tpcpatch_tnpoints_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE tnpoints(tpcpatch) (
  SFUNC = tnpoints_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

-- tdensity(tpcpatch) — running temporal sum of per-instant density
-- (npoints / xy-bbox-area). 1-point or co-linear patches yield
-- +Infinity for that instant; filter with isfinite() if needed.
CREATE FUNCTION tdensity_transfn(internal, tpcpatch)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tpcpatch_tdensity_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE tdensity(tpcpatch) (
  SFUNC = tdensity_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tfloat_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE FUNCTION wCountTransition(internal, tpcpoint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wCountTransition(internal, tpcpatch, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE wCount(tpcpoint, interval) (
  SFUNC = wCountTransition,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE wCount(tpcpatch, interval) (
  SFUNC = wCountTransition,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************
 * Merge — combine multiple temporal values into one
 *****************************************************************************/

CREATE FUNCTION mergeTransition(internal, tpcpoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION mergeTransition(internal, tpcpatch)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tpcpoint_tagg_finalfn(internal)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcpatch_tagg_finalfn(internal)
  RETURNS tpcpatch
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE merge(tpcpoint) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tpcpoint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE mergeAgg(tpcpoint) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tpcpoint_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE merge(tpcpatch) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tpcpatch_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE mergeAgg(tpcpatch) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = tpcpatch_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

/*****************************************************************************
 * Append-instant aggregate (streaming trajectory construction)
 *****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION appendInstantTransition(tpcpoint, tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
-- The function is not STRICT
CREATE FUNCTION appendInstantTransition(tpcpoint, tpcpoint,
    interp text DEFAULT NULL, maxt interval DEFAULT NULL)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION temporal_append_finalfn(tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tpcpoint) (
  SFUNC = appendInstantTransition,
  STYPE = tpcpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tpcpoint) (
  SFUNC = appendInstantTransition,
  STYPE = tpcpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tpcpoint, text, interval) (
  SFUNC = appendInstantTransition,
  STYPE = tpcpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tpcpoint, text, interval) (
  SFUNC = appendInstantTransition,
  STYPE = tpcpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

-- tpcpatch
-- The function is not STRICT
CREATE FUNCTION appendInstantTransition(tpcpatch, tpcpatch)
  RETURNS tpcpatch
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION temporal_append_finalfn(tpcpatch)
  RETURNS tpcpatch
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendInstant(tpcpatch) (
  SFUNC = appendInstantTransition,
  STYPE = tpcpatch,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tpcpatch) (
  SFUNC = appendInstantTransition,
  STYPE = tpcpatch,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************
 * Append-sequence aggregate
 *****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION appendSequenceTransition(tpcpoint, tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendSequenceTransition(tpcpatch, tpcpatch)
  RETURNS tpcpatch
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendSequence(tpcpoint) (
  SFUNC = appendSequenceTransition,
  STYPE = tpcpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequenceAgg(tpcpoint) (
  SFUNC = appendSequenceTransition,
  STYPE = tpcpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/* Function deprecated in 1.4
   Some bindings require Agg suffix to disambiguate from the scalar function */
CREATE AGGREGATE appendSequence(tpcpatch) (
  SFUNC = appendSequenceTransition,
  STYPE = tpcpatch,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequenceAgg(tpcpatch) (
  SFUNC = appendSequenceTransition,
  STYPE = tpcpatch,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/
