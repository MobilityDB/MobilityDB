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
 * @brief Aggregate functions for temporal quadbin cells
 */

-- The function is not STRICT
CREATE FUNCTION tspatial_extent_transfn(stbox, tquadbin)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tspatial_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tquadbin) (
  SFUNC = tspatial_extent_transfn,
  STYPE = stbox,
  COMBINEFUNC = stbox_extent_combinefn,
  PARALLEL = safe
);

-- The function is not STRICT
CREATE FUNCTION tcount_transfn(internal, tquadbin)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tquadbin_tagg_finalfn(internal)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tCount(tquadbin) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

-- The function is not STRICT
CREATE FUNCTION wcount_transfn(internal, tquadbin, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE wCount(tquadbin, interval) (
  SFUNC = wcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

-- The function is not STRICT
CREATE FUNCTION temporal_merge_transfn(internal, tquadbin)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE mergeAgg(tquadbin) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
  COMBINEFUNC = temporal_merge_combinefn,
  FINALFUNC = tquadbin_tagg_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

CREATE FUNCTION temporal_app_tinst_transfn(tquadbin, tquadbin)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_app_tinst_transfn(tquadbin, tquadbin, interp text)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- The function is not STRICT
CREATE FUNCTION temporal_app_tinst_transfn(tquadbin, tquadbin, interp text,
    maxt interval)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION temporal_append_finalfn(tquadbin)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE appendInstantAgg(tquadbin) (
  SFUNC = temporal_app_tinst_transfn(tquadbin, tquadbin),
  STYPE = tquadbin,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tquadbin, interp text) (
  SFUNC = temporal_app_tinst_transfn(tquadbin, tquadbin, text),
  STYPE = tquadbin,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstantAgg(tquadbin, interp text, maxt interval) (
  SFUNC = temporal_app_tinst_transfn(tquadbin, tquadbin, text, maxt),
  STYPE = tquadbin,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION temporal_app_tseq_transfn(tquadbin, tquadbin)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE appendSequenceAgg(tquadbin) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = tquadbin,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/
