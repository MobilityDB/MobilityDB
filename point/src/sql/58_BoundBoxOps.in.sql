/*****************************************************************************
 *
 * BoundBoxOps.sql
 *	  Bounding box operators for temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION gbox(geometry)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geo_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(geography)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geo_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(timestamptz)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'timestamp_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(timestampset)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'timestampset_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(period)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'period_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(periodset)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'periodset_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(geometry, timestamptz)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geo_timestamp_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(geography, timestamptz)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geo_timestamp_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(geometry, period)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geo_period_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(geography, period)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geo_period_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(tgeompoint)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(tgeogpoint)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (geometry AS gbox) WITH FUNCTION gbox(geometry) AS IMPLICIT;
CREATE CAST (geography AS gbox) WITH FUNCTION gbox(geography) AS IMPLICIT;
CREATE CAST (timestamptz AS gbox) WITH FUNCTION gbox(timestamptz) AS IMPLICIT;
CREATE CAST (timestampset AS gbox) WITH FUNCTION gbox(timestampset) AS IMPLICIT;
CREATE CAST (period AS gbox) WITH FUNCTION gbox(period) AS IMPLICIT;
CREATE CAST (periodset AS gbox) WITH FUNCTION gbox(periodset) AS IMPLICIT;
CREATE CAST (tgeompoint AS gbox) WITH FUNCTION gbox(tgeompoint) AS IMPLICIT;
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

CREATE FUNCTION gbox_contains(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox_contained(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox_overlaps(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox_same(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
/*
CREATE FUNCTION gbox_distance(gbox, gbox)
	RETURNS float
	AS 'MODULE_PATHNAME', 'distance_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
*/

CREATE OPERATOR @> (
	PROCEDURE = gbox_contains,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = <@,
	RESTRICT = contains_point_sel, JOIN = tpoint_join_sel
);
CREATE OPERATOR <@ (
	PROCEDURE = gbox_contained,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = @>,
	RESTRICT = contained_point_sel, JOIN = tpoint_join_sel
);
CREATE OPERATOR && (
	PROCEDURE = gbox_overlaps,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = &&,
	RESTRICT = overlaps_point_sel, JOIN = tpoint_join_sel
);
CREATE OPERATOR ~= (
	PROCEDURE = gbox_same,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = ~=,
	RESTRICT = same_point_sel, JOIN = positionjoinseltemp
);
/*
CREATE OPERATOR <-> (
	PROCEDURE = gbox_distance,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = <->
);
*/

/*****************************************************************************/

CREATE FUNCTION expandSpatial(gbox, float)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gbox_expand_spatial'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION expandTemporal(gbox, interval)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gbox_expand_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION expandSpatial(tgeompoint, float)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'tpoint_expand_spatial'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION expandTemporal(tgeompoint, interval)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'tpoint_expand_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Contains
 *****************************************************************************/

CREATE FUNCTION contains_bbox(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(timestamptz, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(timestampset, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(period, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(periodset, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = tpoint_join_sel
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = timestamptz, RIGHTARG = tgeompoint,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = timestampset, RIGHTARG = tgeompoint,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = period, RIGHTARG = tgeompoint,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = periodset, RIGHTARG = tgeompoint,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	COMMUTATOR = <@,
	RESTRICT = contains_point_sel, JOIN = tpoint_join_sel
);

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

/*****************************************************************************/

CREATE FUNCTION contains_bbox(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	COMMUTATOR = <@,
	RESTRICT = contains_point_sel, JOIN = tpoint_join_sel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeompoint, RIGHTARG = timestamptz,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeompoint, RIGHTARG = timestampset,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeompoint, RIGHTARG = period,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeompoint, RIGHTARG = periodset,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	COMMUTATOR = <@,
	RESTRICT = contains_point_sel, JOIN = tpoint_join_sel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	COMMUTATOR = <@,
	RESTRICT = contains_point_sel, JOIN = tpoint_join_sel
);

/*****************************************************************************/

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

/*****************************************************************************
 * Contained
 *****************************************************************************/

CREATE FUNCTION contained_bbox(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(timestamptz, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(timestampset, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(period, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(periodset, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	COMMUTATOR = @>,
	RESTRICT = contained_point_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = timestamptz, RIGHTARG = tgeompoint,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = timestampset, RIGHTARG = tgeompoint,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = period, RIGHTARG = tgeompoint,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = periodset, RIGHTARG = tgeompoint,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	COMMUTATOR = @>,
	RESTRICT = contained_point_sel, JOIN = positionjoinseltemp
);

CREATE FUNCTION contained_bbox(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	COMMUTATOR = @>,
	RESTRICT = contained_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeompoint, RIGHTARG = timestamptz,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeompoint, RIGHTARG = timestampset,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeompoint, RIGHTARG = period,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeompoint, RIGHTARG = periodset,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	COMMUTATOR = @>,
	RESTRICT = contained_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	COMMUTATOR = @>,
	RESTRICT = contained_point_sel, JOIN = positionjoinseltemp
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

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(timestamptz, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(timestampset, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(period, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(periodset, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_point_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = timestamptz, RIGHTARG = tgeompoint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = timestampset, RIGHTARG = tgeompoint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = period, RIGHTARG = tgeompoint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = periodset, RIGHTARG = tgeompoint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_point_sel, JOIN = positionjoinseltemp
);

CREATE FUNCTION overlaps_bbox(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	COMMUTATOR = &&,
	RESTRICT = overlaps_point_sel, JOIN = tpoint_join_sel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeompoint, RIGHTARG = timestamptz,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeompoint, RIGHTARG = timestampset,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeompoint, RIGHTARG = period,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeompoint, RIGHTARG = periodset,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	COMMUTATOR = &&,
	RESTRICT = overlaps_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_point_sel, JOIN = positionjoinseltemp
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

/*****************************************************************************
 * Same
 *****************************************************************************/

CREATE FUNCTION same_bbox(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(timestamptz, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(timestampset, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(period, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(periodset, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	COMMUTATOR = ~=,
	RESTRICT = same_point_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = timestamptz, RIGHTARG = tgeompoint,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = timestampset, RIGHTARG = tgeompoint,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = period, RIGHTARG = tgeompoint,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = periodset, RIGHTARG = tgeompoint,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

CREATE FUNCTION same_bbox(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	COMMUTATOR = ~=,
	RESTRICT = same_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeompoint, RIGHTARG = timestamptz,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeompoint, RIGHTARG = timestampset,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeompoint, RIGHTARG = period,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeompoint, RIGHTARG = periodset,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	COMMUTATOR = ~=,
	RESTRICT = same_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	COMMUTATOR = ~=,
	RESTRICT = same_point_sel, JOIN = positionjoinseltemp
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

