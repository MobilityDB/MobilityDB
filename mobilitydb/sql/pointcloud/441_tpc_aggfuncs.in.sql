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
 * @brief Aggregate functions for the pgPointCloud temporal types
 */

CREATE FUNCTION extentTransition(tpcbox, tpcbox)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpcbox_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tpcbox_extent_combinefn(tpcbox, tpcbox)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpcbox_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION extentTransition(tpcbox, tpcpoint)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpc_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION extentTransition(tpcbox, tpcpatch)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpc_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tpcbox) (
  SFUNC = extentTransition,
  STYPE = tpcbox,
  COMBINEFUNC = tpcbox_extent_combinefn,
  PARALLEL = safe
);

CREATE AGGREGATE extent(tpcpoint) (
  SFUNC = extentTransition,
  STYPE = tpcbox,
  COMBINEFUNC = tpcbox_extent_combinefn,
  PARALLEL = safe
);

CREATE AGGREGATE extent(tpcpatch) (
  SFUNC = extentTransition,
  STYPE = tpcbox,
  COMBINEFUNC = tpcbox_extent_combinefn,
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

CREATE FUNCTION tCountFinal(internal, tpcpoint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE tCount(tpcpoint) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tCountFinal,
  FINALFUNC_EXTRA,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE FUNCTION tCountFinal(internal, tpcpatch)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE tCount(tpcpatch) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tCountFinal,
  FINALFUNC_EXTRA,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

-- tnpoints(tpcpatch) — running temporal sum of per-instant pcpatch
-- npoints. Distinct from tcount(tpcpatch), which counts the number
-- of patches (always 1 per instant). For drone LiDAR this is "how
-- many points were in the cloud at time t".
CREATE FUNCTION tnpointsTransition(internal, tpcpatch)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tpcpatch_tnpoints_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tnpointsFinal(internal, tpcpatch)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE tnpoints(tpcpatch) (
  SFUNC = tnpointsTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tnpointsFinal,
  FINALFUNC_EXTRA,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

-- tdensity(tpcpatch) — running temporal sum of per-instant density
-- (npoints / xy-bbox-area). 1-point or co-linear patches yield
-- +Infinity for that instant; filter with isfinite() if needed.
CREATE FUNCTION tdensityTransition(internal, tpcpatch)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tpcpatch_tdensity_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tdensityFinal(internal, tpcpatch)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE tdensity(tpcpatch) (
  SFUNC = tdensityTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tdensityFinal,
  FINALFUNC_EXTRA,
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

CREATE FUNCTION wCountFinal(internal, tpcpoint, interval)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE wCount(tpcpoint, interval) (
  SFUNC = wCountTransition,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = wCountFinal,
  FINALFUNC_EXTRA,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE FUNCTION wCountFinal(internal, tpcpatch, interval)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE wCount(tpcpatch, interval) (
  SFUNC = wCountTransition,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = wCountFinal,
  FINALFUNC_EXTRA,
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


CREATE FUNCTION mergeFinal(internal, tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE merge(tpcpoint) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = mergeFinal,
  FINALFUNC_EXTRA,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

CREATE FUNCTION mergeFinal(internal, tpcpatch)
  RETURNS tpcpatch
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE merge(tpcpatch) (
  SFUNC = mergeTransition,
  STYPE = internal,
  COMBINEFUNC = mergeCombine,
  FINALFUNC = mergeFinal,
  FINALFUNC_EXTRA,
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


CREATE FUNCTION appendInstantFinal(tpcpoint, tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendInstant(tpcpoint) (
  SFUNC = appendInstantTransition,
  STYPE = tpcpoint,
  FINALFUNC = appendInstantFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);

CREATE FUNCTION appendInstantFinal(tpcpoint, tpcpoint, text, interval)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendInstant(tpcpoint, text, interval) (
  SFUNC = appendInstantTransition,
  STYPE = tpcpoint,
  FINALFUNC = appendInstantFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);

-- tpcpatch
-- The function is not STRICT
CREATE FUNCTION appendInstantTransition(tpcpatch, tpcpatch)
  RETURNS tpcpatch
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;


CREATE FUNCTION appendInstantFinal(tpcpatch, tpcpatch)
  RETURNS tpcpatch
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendInstant(tpcpatch) (
  SFUNC = appendInstantTransition,
  STYPE = tpcpatch,
  FINALFUNC = appendInstantFinal,
  FINALFUNC_EXTRA,
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

CREATE FUNCTION appendSequenceFinal(tpcpoint, tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendSequence(tpcpoint) (
  SFUNC = appendSequenceTransition,
  STYPE = tpcpoint,
  FINALFUNC = appendSequenceFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);

CREATE FUNCTION appendSequenceFinal(tpcpatch, tpcpatch)
  RETURNS tpcpatch
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendSequence(tpcpatch) (
  SFUNC = appendSequenceTransition,
  STYPE = tpcpatch,
  FINALFUNC = appendSequenceFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);

/*****************************************************************************/
