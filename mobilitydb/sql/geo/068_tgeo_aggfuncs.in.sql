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
 * @brief Aggregate functions for temporal geometries/geographies
 */

CREATE FUNCTION tspatial_extent_transfn(stbox, tgeometry)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tspatial_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tspatial_extent_transfn(stbox, tgeography)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tspatial_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tgeometry) (
  SFUNC = tspatial_extent_transfn,
  STYPE = stbox,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = stbox_extent_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  PARALLEL = safe
);
CREATE AGGREGATE extent(tgeography) (
  SFUNC = tspatial_extent_transfn,
  STYPE = stbox,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = stbox_extent_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  PARALLEL = safe
);

/*****************************************************************************/

CREATE FUNCTION tcount_transfn(internal, tgeometry)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tcount_transfn(internal, tgeography)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE tcount(tgeometry) (
  SFUNC = tcount_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tcount_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tcount(tgeography) (
  SFUNC = tcount_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tcount_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE FUNCTION wcount_transfn(internal, tgeometry, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wcount_transfn(internal, tgeography, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE wcount(tgeometry, interval) (
  SFUNC = wcount_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tint_tsum_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE wcount(tgeography, interval) (
  SFUNC = wcount_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = tint_tsum_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

CREATE FUNCTION temporal_merge_transfn(internal, tgeometry)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_merge_transfn(internal, tgeography)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tgeometry_tagg_finalfn(internal)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeography_tagg_finalfn(internal)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE merge(tgeometry) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = temporal_merge_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tgeometry_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE merge(tgeography) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 130000
  COMBINEFUNC = temporal_merge_combinefn,
#endif //POSTGRESQL_VERSION_NUMBER >= 130000
  FINALFUNC = tgeography_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

/*****************************************************************************
 * Append tinstant aggregate functions
 *****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION temporal_app_tinst_transfn(tgeometry, tgeometry)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_app_tinst_transfn(tgeography, tgeography)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

-- The function is not STRICT
CREATE FUNCTION temporal_app_tinst_transfn(tgeometry, tgeometry,
    interp text DEFAULT NULL)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_app_tinst_transfn(tgeography, tgeography,
    interp text DEFAULT NULL)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

-- The function is not STRICT
CREATE FUNCTION temporal_app_tinst_transfn(tgeometry, tgeometry,
    interp text DEFAULT NULL, maxdist float DEFAULT NULL,
    maxt interval DEFAULT NULL)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_app_tinst_transfn(tgeography, tgeography,
    interp text DEFAULT NULL, maxdist float DEFAULT NULL, 
    maxt interval DEFAULT NULL)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION temporal_append_finalfn(tgeometry)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_append_finalfn(tgeography)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE appendInstant(tgeometry) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeometry,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstant(tgeography) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeography,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE appendInstant(tgeometry, text) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeometry,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstant(tgeography, text) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeography,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE appendInstant(tgeometry, text, float, interval) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeometry,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendInstant(tgeography, text, float, interval) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tgeography,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION temporal_app_tseq_transfn(tgeometry, tgeometry)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_app_tseq_transfn(tgeography, tgeography)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE appendSequence(tgeometry) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = tgeometry,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE appendSequence(tgeography) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = tgeography,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/
