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
 * @brief Aggregate functions for temporal network points
 */

CREATE FUNCTION extentTransition(stbox, tnpoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tspatial_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tnpoint) (
  SFUNC = extentTransition,
  STYPE = stbox,
  COMBINEFUNC = extentCombine,
  PARALLEL = safe
);

-- The function is not strict
CREATE FUNCTION tCountTransition(internal, tnpoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tCountFinal(internal, tnpoint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE tCount(tnpoint) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tCountCombine,
  FINALFUNC = tCountFinal,
  FINALFUNC_EXTRA,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

-- The function is not strict
CREATE FUNCTION wCountTransition(internal, tnpoint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION wCountFinal(internal, tnpoint, interval)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE wCount(tnpoint, interval) (
  SFUNC = wCountTransition,
  STYPE = internal,
  COMBINEFUNC = wCountCombine,
  FINALFUNC = wCountFinal,
  FINALFUNC_EXTRA,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

-- The function is not strict
CREATE FUNCTION tCentroidTransition(internal, tnpoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tnpoint_tcentroid_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tCentroidFinal(internal, tnpoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Tpoint_tcentroid_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE tCentroid(tnpoint) (
  SFUNC = tCentroidTransition,
  STYPE = internal,
  COMBINEFUNC = tCentroidCombine,
  FINALFUNC = tCentroidFinal,
  FINALFUNC_EXTRA,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

-- The function is not strict
CREATE FUNCTION mergeTransition(internal, tnpoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION mergeFinal(internal, tnpoint)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE merge(tnpoint) (
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
 * Append tinstant aggregate functions
 *****************************************************************************/

-- The functions are not STRICT, so that one row answers a sequence
CREATE FUNCTION appendInstantTransition(tnpoint, tnpoint)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION appendInstantTransition(tnpoint, tnpoint, interp text)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION appendInstantTransition(tnpoint, tnpoint,
    interp text, maxdist float,
    maxt interval)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;


CREATE FUNCTION appendInstantFinal(tnpoint, tnpoint)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendInstant(tnpoint) (
  SFUNC = appendInstantTransition,
  STYPE = tnpoint,
  FINALFUNC = appendInstantFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);
CREATE FUNCTION appendInstantFinal(tnpoint, tnpoint, interp text)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendInstant(tnpoint, interp text) (
  SFUNC = appendInstantTransition,
  STYPE = tnpoint,
  FINALFUNC = appendInstantFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);
CREATE FUNCTION appendInstantFinal(tnpoint, tnpoint, interp text, maxdist float,
    maxt interval)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendInstant(tnpoint, interp text, maxdist float,
    maxt interval) (
  SFUNC = appendInstantTransition,
  STYPE = tnpoint,
  FINALFUNC = appendInstantFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);

/*****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION appendSequenceTransition(tnpoint, tnpoint)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION appendSequenceFinal(tnpoint, tnpoint)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendSequence(tnpoint) (
  SFUNC = appendSequenceTransition,
  STYPE = tnpoint,
  FINALFUNC = appendSequenceFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);

/*****************************************************************************/
