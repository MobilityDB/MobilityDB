/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
  AS 'MODULE_PATHNAME', 'Tpoint_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(tgeogpoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tpoint_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeompoint AS stbox) WITH FUNCTION stbox(tgeompoint);
CREATE CAST (tgeogpoint AS stbox) WITH FUNCTION stbox(tgeogpoint);

/*****************************************************************************/

CREATE FUNCTION expandSpatial(geometry, float)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geo_expand_spatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION expandSpatial(geography, float)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geo_expand_spatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION expandSpatial(tgeompoint, float)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tpoint_expand_spatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION expandSpatial(tgeogpoint, float)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tpoint_expand_spatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION stboxes(tgeompoint)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Tpoint_stboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Contains
 *****************************************************************************/

CREATE FUNCTION contains_bbox(timestamptz, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzspan, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzspanset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_periodset'
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
  LEFTARG = tstzset, RIGHTARG = tgeompoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tstzset,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tstzspan, RIGHTARG = tgeompoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tgeompoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tstzspanset,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(timestamptz, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzspan, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tstzspanset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_periodset'
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
  LEFTARG = tstzset, RIGHTARG = tgeogpoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tstzset,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tstzspan, RIGHTARG = tgeogpoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tgeogpoint,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tstzspanset,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_bbox_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpoint_tpoint'
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
  AS 'MODULE_PATHNAME', 'Contains_bbox_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tpoint_tpoint'
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
  AS 'MODULE_PATHNAME', 'Contained_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzspan, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzspanset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_periodset'
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
  LEFTARG = tstzset, RIGHTARG = tgeompoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tstzset,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tstzspan, RIGHTARG = tgeompoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tgeompoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tstzspanset,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpoint_tpoint'
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
  AS 'MODULE_PATHNAME', 'Contained_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzspan, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tstzspanset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_periodset'
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
  LEFTARG = tstzset, RIGHTARG = tgeogpoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tstzset,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tstzspan, RIGHTARG = tgeogpoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tgeogpoint,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tstzspanset,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tpoint_tpoint'
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
  AS 'MODULE_PATHNAME', 'Overlaps_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzspan, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzspanset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_periodset'
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
  LEFTARG = tstzset, RIGHTARG = tgeompoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tstzset,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tstzspan, RIGHTARG = tgeompoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tgeompoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tstzspanset,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION overlaps_bbox(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpoint_tpoint'
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
  AS 'MODULE_PATHNAME', 'Overlaps_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzspan, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tstzspanset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_periodset'
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
  LEFTARG = tstzset, RIGHTARG = tgeogpoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tstzset,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tstzspan, RIGHTARG = tgeogpoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tgeogpoint,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tstzspanset,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION overlaps_bbox(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tpoint_tpoint'
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
  AS 'MODULE_PATHNAME', 'Same_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzspan, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzspanset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_periodset'
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
  LEFTARG = tstzset, RIGHTARG = tgeompoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tstzset,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tstzspan, RIGHTARG = tgeompoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tgeompoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tstzspanset,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpoint_tpoint'
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
  AS 'MODULE_PATHNAME', 'Same_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzspan, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tstzspanset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_periodset'
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
  LEFTARG = tstzset, RIGHTARG = tgeogpoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tstzset,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tstzspan, RIGHTARG = tgeogpoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tgeogpoint,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = same_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tstzspanset,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tpoint_tpoint'
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
  AS 'MODULE_PATHNAME', 'Adjacent_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeompoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeompoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzspan, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeompoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzspanset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeompoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_periodset'
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
  LEFTARG = tstzset, RIGHTARG = tgeompoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tstzset,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tstzspan, RIGHTARG = tgeompoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tgeompoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeompoint, RIGHTARG = tstzspanset,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION adjacent_bbox(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpoint_tpoint'
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
  AS 'MODULE_PATHNAME', 'Adjacent_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeogpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeogpoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzspan, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeogpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tstzspanset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeogpoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_periodset'
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
  LEFTARG = tstzset, RIGHTARG = tgeogpoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tstzset,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tstzspan, RIGHTARG = tgeogpoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tstzspanset, RIGHTARG = tgeogpoint,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tgeogpoint, RIGHTARG = tstzspanset,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION adjacent_bbox(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tpoint_tpoint'
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
