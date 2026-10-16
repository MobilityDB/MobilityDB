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
 * @brief Aggregate functions for temporal pose chains
 */

-- The function is not strict
CREATE FUNCTION tCountTransition(internal, tposechain)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tCountFinal(internal, tposechain)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE tCount(tposechain) (
  SFUNC = tCountTransition,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tCountFinal,
  FINALFUNC_EXTRA,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE FUNCTION extentTransition(stbox, tposechain)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tspatial_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE extent(tposechain) (
  SFUNC = extentTransition,
  STYPE = stbox,
  COMBINEFUNC = stbox_extent_combinefn,
  PARALLEL = safe
);

-- The function is not strict
CREATE FUNCTION wCountTransition(internal, tposechain, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION wCountFinal(internal, tposechain, interval)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE wCount(tposechain, interval) (
  SFUNC = wCountTransition,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = wCountFinal,
  FINALFUNC_EXTRA,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

-- The function is not strict
CREATE FUNCTION mergeTransition(internal, tposechain)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION mergeFinal(internal, tposechain)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE merge(tposechain) (
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

-- The function is not STRICT
CREATE FUNCTION appendInstantTransition(tposechain, tposechain)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

-- The function is not STRICT
CREATE FUNCTION appendInstantTransition(tposechain, tposechain,
    interp text DEFAULT NULL)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

-- The function is not STRICT
CREATE FUNCTION appendInstantTransition(tposechain, tposechain,
    interp text DEFAULT NULL, maxdist float DEFAULT NULL, 
    maxt interval DEFAULT NULL)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;


CREATE FUNCTION appendInstantFinal(tposechain, tposechain)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendInstant(tposechain) (
  SFUNC = appendInstantTransition,
  STYPE = tposechain,
  FINALFUNC = appendInstantFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);

CREATE FUNCTION appendInstantFinal(tposechain, tposechain, text)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendInstant(tposechain, text) (
  SFUNC = appendInstantTransition,
  STYPE = tposechain,
  FINALFUNC = appendInstantFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);

CREATE FUNCTION appendInstantFinal(tposechain, tposechain, text, float, interval)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendInstant(tposechain, text, float, interval) (
  SFUNC = appendInstantTransition,
  STYPE = tposechain,
  FINALFUNC = appendInstantFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);

/*****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION appendSequenceTransition(tposechain, tposechain)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION appendSequenceFinal(tposechain, tposechain)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE AGGREGATE appendSequence(tposechain) (
  SFUNC = appendSequenceTransition,
  STYPE = tposechain,
  FINALFUNC = appendSequenceFinal,
  FINALFUNC_EXTRA,
  PARALLEL = safe
);

/*****************************************************************************/
