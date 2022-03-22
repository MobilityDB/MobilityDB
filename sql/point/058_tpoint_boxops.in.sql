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
 * tpoint_boxops.sql
 * Bounding box operators for temporal points.
 */

/*****************************************************************************
 * Casting
 *****************************************************************************/

CREATE FUNCTION stbox(tgeompoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'tpoint_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(tgeogpoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'tpoint_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeompoint AS stbox) WITH FUNCTION stbox(tgeompoint);
CREATE CAST (tgeogpoint AS stbox) WITH FUNCTION stbox(tgeogpoint);

/*****************************************************************************/

CREATE FUNCTION expandSpatial(geometry, float)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'geo_expand_spatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION expandSpatial(geography, float)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'geo_expand_spatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION expandSpatial(tgeompoint, float)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'tpoint_expand_spatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION expandSpatial(tgeogpoint, float)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'tpoint_expand_spatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION stboxes(tgeompoint)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'tpoint_stboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Contains
 *****************************************************************************/

CREATE FUNCTION contains_bbox(timestamptz, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(timestampset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(period, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(periodset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = timestamptz, RIGHTARG = tgeompoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeompoint, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = timestampset, RIGHTARG = tgeompoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeompoint, RIGHTARG = timestampset,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = period, RIGHTARG = tgeompoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeompoint, RIGHTARG = period,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = periodset, RIGHTARG = tgeompoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeompoint, RIGHTARG = periodset,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(timestamptz, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(timestampset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(period, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(periodset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = timestampset, RIGHTARG = tgeogpoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = timestampset,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = period, RIGHTARG = tgeogpoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = period,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = periodset, RIGHTARG = tgeogpoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = periodset,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = geometry, RIGHTARG = tgeompoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeompoint, RIGHTARG = geometry,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = geography, RIGHTARG = tgeogpoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = stbox, RIGHTARG = tgeogpoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = geography,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = stbox,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************
 * Contained
 *****************************************************************************/

CREATE FUNCTION contained_bbox(timestamptz, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(timestampset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(period, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(periodset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = timestamptz, RIGHTARG = tgeompoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeompoint, RIGHTARG = timestamptz,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = timestampset, RIGHTARG = tgeompoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeompoint, RIGHTARG = timestampset,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = period, RIGHTARG = tgeompoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeompoint, RIGHTARG = period,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = periodset, RIGHTARG = tgeompoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeompoint, RIGHTARG = periodset,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = geometry, RIGHTARG = tgeompoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeompoint, RIGHTARG = geometry,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(timestamptz, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(timestampset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(period, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(periodset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = timestampset, RIGHTARG = tgeogpoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = timestampset,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = period, RIGHTARG = tgeogpoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = period,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = periodset, RIGHTARG = tgeogpoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = periodset,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = geography, RIGHTARG = tgeogpoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = stbox, RIGHTARG = tgeogpoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = geography,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = stbox,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(timestamptz, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(timestampset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(period, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(periodset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = timestamptz, RIGHTARG = tgeompoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeompoint, RIGHTARG = timestamptz,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = timestampset, RIGHTARG = tgeompoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeompoint, RIGHTARG = timestampset,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = period, RIGHTARG = tgeompoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeompoint, RIGHTARG = period,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = periodset, RIGHTARG = tgeompoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeompoint, RIGHTARG = periodset,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION overlaps_bbox(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = geometry, RIGHTARG = tgeompoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeompoint, RIGHTARG = geometry,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION overlaps_bbox(timestamptz, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(timestampset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(period, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(periodset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = timestampset, RIGHTARG = tgeogpoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = timestampset,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = period, RIGHTARG = tgeogpoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = period,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = periodset, RIGHTARG = tgeogpoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = periodset,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION overlaps_bbox(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = geography, RIGHTARG = tgeogpoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = stbox, RIGHTARG = tgeogpoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = geography,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = stbox,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************
 * Same
 *****************************************************************************/

CREATE FUNCTION same_bbox(timestamptz, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(timestampset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(period, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(periodset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = timestamptz, RIGHTARG = tgeompoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeompoint, RIGHTARG = timestamptz,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = timestampset, RIGHTARG = tgeompoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeompoint, RIGHTARG = timestampset,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = period, RIGHTARG = tgeompoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeompoint, RIGHTARG = period,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = periodset, RIGHTARG = tgeompoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeompoint, RIGHTARG = periodset,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = geometry, RIGHTARG = tgeompoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeompoint, RIGHTARG = geometry,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(timestamptz, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(timestampset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(period, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(periodset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = timestampset, RIGHTARG = tgeogpoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = timestampset,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = period, RIGHTARG = tgeogpoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = period,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = periodset, RIGHTARG = tgeogpoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = periodset,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_bbox_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = geography, RIGHTARG = tgeogpoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = stbox, RIGHTARG = tgeogpoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = geography,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = stbox,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************
 * Adjacent
 *****************************************************************************/

CREATE FUNCTION adjacent_bbox(timestamptz, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeompoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(timestampset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeompoint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(period, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeompoint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(periodset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeompoint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = timestamptz, RIGHTARG = tgeompoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeompoint, RIGHTARG = timestamptz,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = timestampset, RIGHTARG = tgeompoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeompoint, RIGHTARG = timestampset,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = period, RIGHTARG = tgeompoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeompoint, RIGHTARG = period,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = periodset, RIGHTARG = tgeompoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeompoint, RIGHTARG = periodset,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION adjacent_bbox(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = geometry, RIGHTARG = tgeompoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeompoint, RIGHTARG = geometry,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION adjacent_bbox(timestamptz, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeogpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(timestampset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeogpoint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(period, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeogpoint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(periodset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeogpoint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = timestampset, RIGHTARG = tgeogpoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = timestampset,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = period, RIGHTARG = tgeogpoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = period,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = periodset, RIGHTARG = tgeogpoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = periodset,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION adjacent_bbox(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = geography, RIGHTARG = tgeogpoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = stbox, RIGHTARG = tgeogpoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = geography,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = stbox,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/
