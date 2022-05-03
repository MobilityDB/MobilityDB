/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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

/*
 * tpoint_aggfuncs.sql
 * Aggregate functions for temporal points.
 */

CREATE FUNCTION tpoint_extent_transfn(stbox, tgeompoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tpoint_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tpoint_extent_transfn(stbox, tgeogpoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tpoint_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tgeompoint) (
  SFUNC = tpoint_extent_transfn,
  STYPE = stbox,
  COMBINEFUNC = stbox_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(tgeogpoint) (
  SFUNC = tpoint_extent_transfn,
  STYPE = stbox,
  COMBINEFUNC = stbox_extent_combinefn,
  PARALLEL = safe
);

/*****************************************************************************/

CREATE FUNCTION tcount_transfn(internal, tgeompoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn(internal, tgeogpoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE tcount(tgeompoint) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE tcount(tgeogpoint) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

CREATE FUNCTION wcount_transfn(internal, tgeompoint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wcount_transfn(internal, tgeogpoint, interval)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_wcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE wcount(tgeompoint, interval) (
  SFUNC = wcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);
CREATE AGGREGATE wcount(tgeogpoint, interval) (
  SFUNC = wcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tint_tsum_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

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

CREATE AGGREGATE tcentroid(tgeompoint) (
  SFUNC = tcentroid_transfn,
  STYPE = internal,
  COMBINEFUNC = tcentroid_combinefn,
  FINALFUNC = tcentroid_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

CREATE FUNCTION temporal_merge_transfn(internal, tgeompoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_merge_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION temporal_merge_transfn(internal, tgeogpoint)
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

CREATE AGGREGATE merge(tgeompoint) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
  COMBINEFUNC = temporal_merge_combinefn,
  FINALFUNC = tgeompoint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE merge(tgeogpoint) (
  SFUNC = temporal_merge_transfn,
  STYPE = internal,
  COMBINEFUNC = temporal_merge_combinefn,
  FINALFUNC = tgeogpoint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = safe
);

/*****************************************************************************/
