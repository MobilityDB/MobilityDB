/*****************************************************************************
 *
 * BoundBoxOpsG.sql
 *	  Bounding box operators for temporal geography points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION gbox(geography)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geo_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(geography, timestamptz)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geo_timestamp_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(geography, period)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geo_period_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(tgeogpoint)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (geography AS gbox) WITH FUNCTION gbox(geography) AS IMPLICIT;
CREATE CAST (tgeogpoint AS gbox) WITH FUNCTION gbox(tgeogpoint) AS IMPLICIT;

/*****************************************************************************/

CREATE FUNCTION overlaps_point_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'overlaps_point_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION contains_point_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'contains_point_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION contained_point_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'contained_point_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION same_point_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'same_point_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tpoint_join_sel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tpoint_join_sel'
	LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************/

CREATE FUNCTION expandSpatial(tgeogpoint, float)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'tpoint_expand_spatial'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION expandTemporal(tgeogpoint, interval)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'tpoint_expand_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION contains_bbox(geography, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(timestamptz, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(timestampset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(period, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(periodset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = geography, RIGHTARG = tgeogpoint,
	COMMUTATOR = <@,
	RESTRICT = contains_point_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = timestampset, RIGHTARG = tgeogpoint,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = period, RIGHTARG = tgeogpoint,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = periodset, RIGHTARG = tgeogpoint,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	COMMUTATOR = <@,
	RESTRICT = contains_point_sel, JOIN = positionjoinseltemp
);

CREATE FUNCTION contains_bbox(tgeogpoint, geography)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = geography,
	COMMUTATOR = <@,
	RESTRICT = contains_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = timestampset,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = period,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = periodset,
	COMMUTATOR = <@,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	COMMUTATOR = <@,
	RESTRICT = contains_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	COMMUTATOR = <@,
	RESTRICT = contains_point_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(geography, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(timestamptz, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(timestampset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(period, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(periodset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = geography, RIGHTARG = tgeogpoint,
	COMMUTATOR = @>,
	RESTRICT = contained_point_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
	COMMUTATOR = @>,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = timestampset, RIGHTARG = tgeogpoint,
	COMMUTATOR = @>,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = period, RIGHTARG = tgeogpoint,
	COMMUTATOR = @>,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = periodset, RIGHTARG = tgeogpoint,
	COMMUTATOR = @>,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	COMMUTATOR = @>,
	RESTRICT = contained_point_sel, JOIN = positionjoinseltemp
);

CREATE FUNCTION contained_bbox(tgeogpoint, geography)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = geography,
	COMMUTATOR = @>,
	RESTRICT = contained_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
	COMMUTATOR = @>,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = timestampset,
	COMMUTATOR = @>,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = period,
	COMMUTATOR = @>,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = periodset,
	COMMUTATOR = @>,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	COMMUTATOR = @>,
	RESTRICT = contained_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	COMMUTATOR = @>,
	RESTRICT = contained_point_sel, JOIN = positionjoinseltemp
);


/*****************************************************************************/


CREATE FUNCTION overlaps_bbox(geography, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(timestamptz, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(timestampset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(period, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(periodset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = geography, RIGHTARG = tgeogpoint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_point_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
	COMMUTATOR = &&,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = timestampset, RIGHTARG = tgeogpoint,
	COMMUTATOR = &&,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = period, RIGHTARG = tgeogpoint,
	COMMUTATOR = &&,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = periodset, RIGHTARG = tgeogpoint,
	COMMUTATOR = &&,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_point_sel, JOIN = positionjoinseltemp
);

CREATE FUNCTION overlaps_bbox(tgeogpoint, geography)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = geography,
	COMMUTATOR = &&,
	RESTRICT = overlaps_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
	COMMUTATOR = &&,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = timestampset,
	COMMUTATOR = &&,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = period,
	COMMUTATOR = &&,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = periodset,
	COMMUTATOR = &&,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	COMMUTATOR = &&,
	RESTRICT = overlaps_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_point_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(geography, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(timestamptz, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(timestampset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(period, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(periodset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = geography, RIGHTARG = tgeogpoint,
	COMMUTATOR = ~=,
	RESTRICT = same_point_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
	COMMUTATOR = ~=,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = timestampset, RIGHTARG = tgeogpoint,
	COMMUTATOR = ~=,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = period, RIGHTARG = tgeogpoint,
	COMMUTATOR = ~=,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = periodset, RIGHTARG = tgeogpoint,
	COMMUTATOR = ~=,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	COMMUTATOR = ~=,
	RESTRICT = same_point_sel, JOIN = positionjoinseltemp
);

CREATE FUNCTION same_bbox(tgeogpoint, geography)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = geography,
	COMMUTATOR = ~=,
	RESTRICT = same_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
	COMMUTATOR = ~=,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = timestampset,
	COMMUTATOR = ~=,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = period,
	COMMUTATOR = ~=,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = periodset,
	COMMUTATOR = ~=,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	COMMUTATOR = ~=,
	RESTRICT = same_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	COMMUTATOR = ~=,
	RESTRICT = same_point_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
