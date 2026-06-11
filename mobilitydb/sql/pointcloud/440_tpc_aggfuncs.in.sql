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

CREATE FUNCTION tcount_transfn(internal, tpcpoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn(internal, tpcpatch)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE tcount(tpcpoint) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tcount(tpcpatch) (
  SFUNC = tcount_transfn,
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

CREATE FUNCTION wcount_transfn(internal, tpcpoint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wcount_transfn(internal, tpcpatch, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE wcount(tpcpoint, interval) (
  SFUNC = wcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE wcount(tpcpatch, interval) (
  SFUNC = wcount_transfn,
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

CREATE FUNCTION temporal_merge_transfn(internal, tpcpoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_merge_transfn(internal, tpcpatch)
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

CREATE AGGREGATE merge(tpcpoint) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
  COMBINEFUNC = temporal_merge_combinefn,
  FINALFUNC = tpcpoint_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

CREATE AGGREGATE merge(tpcpatch) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
  COMBINEFUNC = temporal_merge_combinefn,
  FINALFUNC = tpcpatch_tagg_finalfn,
  SERIALFUNC = taggstate_serialize,
  DESERIALFUNC = taggstate_deserialize,
  PARALLEL = safe
);

/*****************************************************************************
 * Append-instant aggregate (streaming trajectory construction)
 *****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION temporal_app_tinst_transfn(tpcpoint, tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
-- The function is not STRICT
CREATE FUNCTION temporal_app_tinst_transfn(tpcpoint, tpcpoint,
    interp text DEFAULT NULL, maxt interval DEFAULT NULL)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION temporal_append_finalfn(tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE appendInstant(tpcpoint) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tpcpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE appendInstant(tpcpoint, text, interval) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tpcpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

-- tpcpatch
-- The function is not STRICT
CREATE FUNCTION temporal_app_tinst_transfn(tpcpatch, tpcpatch)
  RETURNS tpcpatch
  AS 'MODULE_PATHNAME', 'Temporal_app_tinst_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION temporal_append_finalfn(tpcpatch)
  RETURNS tpcpatch
  AS 'MODULE_PATHNAME', 'Temporal_append_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE appendInstant(tpcpatch) (
  SFUNC = temporal_app_tinst_transfn,
  STYPE = tpcpatch,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************
 * Append-sequence aggregate
 *****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION temporal_app_tseq_transfn(tpcpoint, tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_app_tseq_transfn(tpcpatch, tpcpatch)
  RETURNS tpcpatch
  AS 'MODULE_PATHNAME', 'Temporal_app_tseq_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE appendSequence(tpcpoint) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = tpcpoint,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE appendSequence(tpcpatch) (
  SFUNC = temporal_app_tseq_transfn,
  STYPE = tpcpatch,
  FINALFUNC = temporal_append_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/
