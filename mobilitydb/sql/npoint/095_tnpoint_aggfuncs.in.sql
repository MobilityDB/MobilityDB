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
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief Aggregate functions for temporal network points
 */

CREATE FUNCTION tcount_transfn(internal, tnpoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE tcount(tnpoint) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE FUNCTION wcount_transfn(internal, tnpoint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE wcount(tnpoint, interval) (
  SFUNC = wcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE FUNCTION tcentroid_transfn(internal, tnpoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tnpoint_tcentroid_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE tcentroid(tnpoint) (
  SFUNC = tcentroid_transfn,
  STYPE = internal,
  COMBINEFUNC = tcentroid_combinefn,
  FINALFUNC = tcentroid_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

CREATE FUNCTION temporal_merge_transfn(internal, tnpoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tnpoint_tagg_finalfn(internal)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE merge(tnpoint) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
  COMBINEFUNC = temporal_merge_combinefn,
  FINALFUNC = tnpoint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

/*****************************************************************************
 * Append tinstant aggregate functions
 *****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION temporal_app_tinst_transfn(tnpoint, tnpoint)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

-- The function is not STRICT
CREATE FUNCTION temporal_app_tinst_transfn(tnpoint, tnpoint,
    interp text DEFAULT NULL)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

-- The function is not STRICT
CREATE FUNCTION temporal_app_tinst_transfn(tnpoint, tnpoint,
    interp text DEFAULT NULL, maxdist float DEFAULT NULL, 
    maxt interval DEFAULT NULL)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION temporal_append_finalfn(tnpoint)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE appendInstant(tnpoint) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tnpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE appendInstant(tnpoint, text) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tnpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE appendInstant(tnpoint, text, float, interval) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tnpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION temporal_app_tseq_transfn(tnpoint, tnpoint)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE appendSequence(tnpoint) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = tnpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/
