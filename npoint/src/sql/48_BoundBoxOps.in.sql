/*****************************************************************************
 *
 * BoundBoxOps.sql
 *	  Bounding box operators for temporal network-constrained points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Xinyang Li,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Temporal npoint to gbox
 *****************************************************************************/

CREATE FUNCTION gbox(tnpoint)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'tnpoint_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * Expand
 *****************************************************************************/
	
CREATE FUNCTION expandSpatial(tnpoint, float)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'tpoint_expand_spatial'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION expandTemporal(tnpoint, interval)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'tpoint_expand_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Contains
 *****************************************************************************/

CREATE FUNCTION contains_bbox(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(timestamptz, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(timestampset, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(period, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(periodset, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(gbox, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = geometry, RIGHTARG = tnpoint,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = timestamptz, RIGHTARG = tnpoint,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = timestampset, RIGHTARG = tnpoint,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = period, RIGHTARG = tnpoint,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = periodset, RIGHTARG = tnpoint,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = gbox, RIGHTARG = tnpoint,
	COMMUTATOR = <@,
	RESTRICT = contains_point_sel, JOIN = positionjoinseltemp
);

CREATE FUNCTION contains_bbox(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tnpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tnpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tnpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tnpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tnpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tnpoint, RIGHTARG = geometry,
	COMMUTATOR = <@,
	RESTRICT = contains_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tnpoint, RIGHTARG = timestamptz,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tnpoint, RIGHTARG = timestampset,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tnpoint, RIGHTARG = period,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tnpoint, RIGHTARG = periodset,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tnpoint, RIGHTARG = gbox,
	COMMUTATOR = <@,
	RESTRICT = contains_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	COMMUTATOR = <@,
	RESTRICT = contains_point_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * Contained
 *****************************************************************************/

CREATE FUNCTION contained_bbox(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(timestamptz, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(timestampset, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(period, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(periodset, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(gbox, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = geometry, RIGHTARG = tnpoint,
	COMMUTATOR = @>,
	RESTRICT = contained_point_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = timestamptz, RIGHTARG = tnpoint,
	COMMUTATOR = @>,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = timestampset, RIGHTARG = tnpoint,
	COMMUTATOR = @>,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = period, RIGHTARG = tnpoint,
	COMMUTATOR = @>,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = periodset, RIGHTARG = tnpoint,
	COMMUTATOR = @>,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = gbox, RIGHTARG = tnpoint,
	COMMUTATOR = @>,
	RESTRICT = contained_point_sel, JOIN = positionjoinseltemp
);

CREATE FUNCTION contained_bbox(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tnpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tnpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tnpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tnpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tnpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tnpoint, RIGHTARG = geometry,
	COMMUTATOR = @>,
	RESTRICT = contained_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tnpoint, RIGHTARG = timestamptz,
	COMMUTATOR = @>,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tnpoint, RIGHTARG = timestampset,
	COMMUTATOR = @>,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tnpoint, RIGHTARG = period,
	COMMUTATOR = @>,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tnpoint, RIGHTARG = periodset,
	COMMUTATOR = @>,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tnpoint, RIGHTARG = gbox,
	COMMUTATOR = @>,
	RESTRICT = contained_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	COMMUTATOR = @>,
	RESTRICT = contained_point_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(timestamptz, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(timestampset, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(period, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(periodset, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(gbox, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = geometry, RIGHTARG = tnpoint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_point_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = timestamptz, RIGHTARG = tnpoint,
	COMMUTATOR = &&,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = timestampset, RIGHTARG = tnpoint,
	COMMUTATOR = &&,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = period, RIGHTARG = tnpoint,
	COMMUTATOR = &&,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = periodset, RIGHTARG = tnpoint,
	COMMUTATOR = &&,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = gbox, RIGHTARG = tnpoint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_point_sel, JOIN = positionjoinseltemp
);

CREATE FUNCTION overlaps_bbox(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tnpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tnpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tnpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tnpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tnpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tnpoint, RIGHTARG = geometry,
	COMMUTATOR = &&,
	RESTRICT = overlaps_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tnpoint, RIGHTARG = timestamptz,
	COMMUTATOR = &&,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tnpoint, RIGHTARG = timestampset,
	COMMUTATOR = &&,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tnpoint, RIGHTARG = period,
	COMMUTATOR = &&,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tnpoint, RIGHTARG = periodset,
	COMMUTATOR = &&,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tnpoint, RIGHTARG = gbox,
	COMMUTATOR = &&,
	RESTRICT = overlaps_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_point_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * Same
 *****************************************************************************/

CREATE FUNCTION same_bbox(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(timestamptz, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(timestampset, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(period, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(periodset, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(gbox, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = geometry, RIGHTARG = tnpoint,
	COMMUTATOR = ~=,
	RESTRICT = same_point_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = timestamptz, RIGHTARG = tnpoint,
	COMMUTATOR = ~=,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = timestampset, RIGHTARG = tnpoint,
	COMMUTATOR = ~=,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = period, RIGHTARG = tnpoint,
	COMMUTATOR = ~=,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = periodset, RIGHTARG = tnpoint,
	COMMUTATOR = ~=,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = gbox, RIGHTARG = tnpoint,
	COMMUTATOR = ~=,
	RESTRICT = same_point_sel, JOIN = positionjoinseltemp
);

CREATE FUNCTION same_bbox(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tnpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tnpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tnpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tnpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tnpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tnpoint, RIGHTARG = geometry,
	COMMUTATOR = ~=,
	RESTRICT = same_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tnpoint, RIGHTARG = timestamptz,
	COMMUTATOR = ~=,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tnpoint, RIGHTARG = timestampset,
	COMMUTATOR = ~=,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tnpoint, RIGHTARG = period,
	COMMUTATOR = ~=,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tnpoint, RIGHTARG = periodset,
	COMMUTATOR = ~=,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tnpoint, RIGHTARG = gbox,
	COMMUTATOR = ~=,
	RESTRICT = same_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	COMMUTATOR = ~=,
	RESTRICT = same_point_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
