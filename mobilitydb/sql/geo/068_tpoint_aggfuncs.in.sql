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
 * @brief Aggregate functions for temporal points
 */

-- The function is not strict
CREATE FUNCTION extentTransition(stbox, tgeompoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tspatial_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentTransition(stbox, tgeogpoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tspatial_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tgeompoint) (
  SFUNC = extentTransition,
  STYPE = stbox,
  COMBINEFUNC = extentCombine,
  PARALLEL = safe
);
CREATE AGGREGATE extent(tgeogpoint) (
  SFUNC = extentTransition,
  STYPE = stbox,
  COMBINEFUNC = extentCombine,
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

CREATE FUNCTION tCountFinal(internal, tgeompoint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE tCount(tgeompoint) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tCountCombine,
  FINALFUNC = tCountFinal,
  FINALFUNC_EXTRA,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE FUNCTION tCountFinal(internal, tgeogpoint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE tCount(tgeogpoint) (
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
CREATE FUNCTION wCountTransition(internal, tgeompoint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wCountTransition(internal, tgeogpoint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION wCountFinal(internal, tgeompoint, interval)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE wCount(tgeompoint, interval) (
  SFUNC = wCountTransition,
  STYPE = internal,
  COMBINEFUNC = wCountCombine,
  FINALFUNC = wCountFinal,
  FINALFUNC_EXTRA,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE FUNCTION wCountFinal(internal, tgeogpoint, interval)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE wCount(tgeogpoint, interval) (
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
CREATE FUNCTION tCentroidTransition(internal, tgeompoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tpoint_tcentroid_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tCentroidCombine(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tpoint_tcentroid_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tCentroidFinal(internal, tgeompoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Tpoint_tcentroid_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE tCentroid(tgeompoint) (
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
CREATE FUNCTION mergeTransition(internal, tgeompoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION mergeTransition(internal, tgeogpoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;


CREATE FUNCTION mergeFinal(internal, tgeompoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE merge(tgeompoint) (
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
CREATE FUNCTION mergeFinal(internal, tgeogpoint)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE merge(tgeogpoint) (
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
CREATE FUNCTION appendInstantTransition(tgeompoint, tgeompoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(tgeogpoint, tgeogpoint)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION appendInstantTransition(tgeompoint, tgeompoint, interp text)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(tgeogpoint, tgeogpoint, interp text)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION appendInstantTransition(tgeompoint, tgeompoint,
    interp text, maxdist float,
    maxt interval)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION appendInstantTransition(tgeogpoint, tgeogpoint,
    interp text, maxdist float,
    maxt interval)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;


CREATE FUNCTION appendInstantFinal(tgeompoint, tgeompoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendInstant(tgeompoint) (
  SFUNC = appendInstantTransition,
  STYPE = tgeompoint,
  FINALFUNC = appendInstantFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);
CREATE FUNCTION appendInstantFinal(tgeompoint, tgeompoint, interp text)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendInstant(tgeompoint, interp text) (
  SFUNC = appendInstantTransition,
  STYPE = tgeompoint,
  FINALFUNC = appendInstantFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);
CREATE FUNCTION appendInstantFinal(tgeompoint, tgeompoint, interp text, maxdist float,
    maxt interval)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendInstant(tgeompoint, interp text, maxdist float,
    maxt interval) (
  SFUNC = appendInstantTransition,
  STYPE = tgeompoint,
  FINALFUNC = appendInstantFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);

CREATE FUNCTION appendInstantFinal(tgeogpoint, tgeogpoint)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendInstant(tgeogpoint) (
  SFUNC = appendInstantTransition,
  STYPE = tgeogpoint,
  FINALFUNC = appendInstantFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);
CREATE FUNCTION appendInstantFinal(tgeogpoint, tgeogpoint, interp text)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendInstant(tgeogpoint, interp text) (
  SFUNC = appendInstantTransition,
  STYPE = tgeogpoint,
  FINALFUNC = appendInstantFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);
CREATE FUNCTION appendInstantFinal(tgeogpoint, tgeogpoint, interp text, maxdist float,
    maxt interval)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendInstant(tgeogpoint, interp text, maxdist float,
    maxt interval) (
  SFUNC = appendInstantTransition,
  STYPE = tgeogpoint,
  FINALFUNC = appendInstantFinal,
  FINALFUNC_EXTRA,
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

CREATE FUNCTION appendSequenceFinal(tgeompoint, tgeompoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendSequence(tgeompoint) (
  SFUNC = appendSequenceTransition,
  STYPE = tgeompoint,
  FINALFUNC = appendSequenceFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);
CREATE FUNCTION appendSequenceFinal(tgeogpoint, tgeogpoint)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendSequence(tgeogpoint) (
  SFUNC = appendSequenceTransition,
  STYPE = tgeogpoint,
  FINALFUNC = appendSequenceFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);

/*****************************************************************************/
