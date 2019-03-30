/*****************************************************************************
 *
 * BoundBoxOps.sql
 *	  Bounding box operators for temporal types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION box(integer, timestamptz)
	RETURNS box
	AS 'MODULE_PATHNAME', 'base_timestamp_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(intrange, timestamptz)
	RETURNS box
	AS 'MODULE_PATHNAME', 'range_timestamp_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(float, timestamptz)
	RETURNS box
	AS 'MODULE_PATHNAME', 'base_timestamp_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(floatrange, timestamptz)
	RETURNS box
	AS 'MODULE_PATHNAME', 'range_timestamp_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(integer, period)
	RETURNS box
	AS 'MODULE_PATHNAME', 'base_period_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(intrange, period)
	RETURNS box
	AS 'MODULE_PATHNAME', 'range_period_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(float, period)
	RETURNS box
	AS 'MODULE_PATHNAME', 'base_period_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(floatrange, period)
	RETURNS box
	AS 'MODULE_PATHNAME', 'range_period_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION box(tint)
	RETURNS box
	AS 'MODULE_PATHNAME', 'tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(tfloat)
	RETURNS box
	AS 'MODULE_PATHNAME', 'tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Temporal boolean
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(timestamptz, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(timestampset, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(periodset, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = timestamptz, RIGHTARG = tbool,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = timestampset, RIGHTARG = tbool,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = period, RIGHTARG = tbool,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = periodset, RIGHTARG = tbool,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION overlaps_bbox(tbool, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tbool, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tbool, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tbool, RIGHTARG = timestamptz,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tbool, RIGHTARG = timestampset,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tbool, RIGHTARG = period,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tbool, RIGHTARG = periodset,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(timestamptz, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(timestampset, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(periodset, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = timestamptz, RIGHTARG = tbool,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = timestampset, RIGHTARG = tbool,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = period, RIGHTARG = tbool,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = periodset, RIGHTARG = tbool,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(tbool, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbool, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbool, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tbool, RIGHTARG = timestamptz,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tbool, RIGHTARG = timestampset,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tbool, RIGHTARG = period,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tbool, RIGHTARG = periodset,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(timestamptz, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(timestampset, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(periodset, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = timestamptz, RIGHTARG = tbool,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = timestampset, RIGHTARG = tbool,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = period, RIGHTARG = tbool,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = periodset, RIGHTARG = tbool,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(tbool, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbool, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbool, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tbool, RIGHTARG = timestamptz,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tbool, RIGHTARG = timestampset,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tbool, RIGHTARG = period,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tbool, RIGHTARG = periodset,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(timestamptz, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(timestampset, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(periodset, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = timestamptz, RIGHTARG = tbool,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = timestampset, RIGHTARG = tbool,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = period, RIGHTARG = tbool,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = periodset, RIGHTARG = tbool,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(tbool, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbool, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbool, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tbool, RIGHTARG = timestamptz,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tbool, RIGHTARG = timestampset,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tbool, RIGHTARG = period,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tbool, RIGHTARG = periodset,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * Temporal integer
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(integer, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_datum_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(float, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_datum_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(floatrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(timestamptz, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(timestampset, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(period, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(periodset, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_box_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = integer, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = intrange, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = float, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = floatrange, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = timestamptz, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = timestampset, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = period, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = periodset, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = box, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION overlaps_bbox(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(tint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = integer,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = intrange,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = float,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = floatrange,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = timestamptz,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = timestampset,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = period,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = periodset,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = box,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(integer, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_datum_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(float, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_datum_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(floatrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(timestamptz, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(timestampset, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(period, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(periodset, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_box_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = integer, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = intrange, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = float, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = floatrange, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = timestamptz, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = timestampset, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = period, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = periodset, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = box, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = integer,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = intrange,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = float,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = floatrange,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = timestamptz,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = timestampset,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = period,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = periodset,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = box,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(integer, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_datum_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(float, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_datum_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(floatrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(timestamptz, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(timestampset, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(period, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(periodset, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_box_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = integer, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = intrange, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = float, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = floatrange, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = timestamptz, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = timestampset, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = period, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = periodset, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = box, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = integer,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = intrange,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = float,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = floatrange,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = timestamptz,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = timestampset,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = period,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = periodset,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = box,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(integer, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_datum_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(float, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_datum_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(floatrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(timestamptz, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(timestampset, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(period, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(periodset, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_box_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = integer, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = intrange, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = float, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = floatrange, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = timestamptz, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = timestampset, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = period, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = periodset, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = box, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = integer,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = intrange,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = float,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = floatrange,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = timestamptz,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = timestampset,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = period,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = periodset,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = box,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * Temporal float
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(intrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(float, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_datum_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(timestamptz, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(timestampset, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(period, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(periodset, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_box_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = intrange, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = float, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = floatrange, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = timestamptz, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = timestampset, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = period, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = periodset, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = box, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION overlaps_bbox(tfloat, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = intrange,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = float,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = floatrange,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = timestamptz,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = timestampset,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = period,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = periodset,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = box,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(intrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(float, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_datum_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(timestamptz, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(timestampset, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(period, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(periodset, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contains_bbox(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_box_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = intrange, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = float, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = floatrange, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = timestamptz, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = timestampset, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = period, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = periodset, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = box, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(tfloat, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = intrange,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = float,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = floatrange,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = timestamptz,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = timestampset,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = period,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = periodset,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = box,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(intrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(float, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_datum_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(timestamptz, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(timestampset, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(period, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(periodset, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION contained_bbox(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_box_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = intrange, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = float, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = floatrange, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = timestamptz, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = timestampset, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = period, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = periodset, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = box, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(tfloat, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = intrange,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = float,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = floatrange,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = timestamptz,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = timestampset,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = period,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = periodset,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = box,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(intrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(float, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_datum_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(timestamptz, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(timestampset, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(period, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(periodset, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION same_bbox(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_box_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = intrange, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = float, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = floatrange, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = timestamptz, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = timestampset, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = period, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = periodset, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = box, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(tfloat, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = intrange,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = float,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = floatrange,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = timestamptz,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = timestampset,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = period,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = periodset,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = box,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * Temporal text
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(timestamptz, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(timestampset, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(periodset, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = timestamptz, RIGHTARG = ttext,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = timestampset, RIGHTARG = ttext,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = period, RIGHTARG = ttext,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = periodset, RIGHTARG = ttext,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

CREATE FUNCTION overlaps_bbox(ttext, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(ttext, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(ttext, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = ttext, RIGHTARG = timestamptz,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = ttext, RIGHTARG = timestampset,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = ttext, RIGHTARG = period,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = ttext, RIGHTARG = periodset,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(timestamptz, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(timestampset, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(periodset, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = timestamptz, RIGHTARG = ttext,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = timestampset, RIGHTARG = ttext,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = period, RIGHTARG = ttext,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = periodset, RIGHTARG = ttext,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(ttext, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(ttext, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(ttext, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = ttext, RIGHTARG = timestamptz,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = ttext, RIGHTARG = timestampset,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = ttext, RIGHTARG = period,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = ttext, RIGHTARG = periodset,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(timestamptz, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(timestampset, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(periodset, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = timestamptz, RIGHTARG = ttext,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = timestampset, RIGHTARG = ttext,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = period, RIGHTARG = ttext,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = periodset, RIGHTARG = ttext,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(ttext, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(ttext, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(ttext, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = ttext, RIGHTARG = timestamptz,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = ttext, RIGHTARG = timestampset,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = ttext, RIGHTARG = period,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = ttext, RIGHTARG = periodset,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(timestamptz, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(timestampset, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(periodset, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = timestamptz, RIGHTARG = ttext,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = timestampset, RIGHTARG = ttext,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = period, RIGHTARG = ttext,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = periodset, RIGHTARG = ttext,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(ttext, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(ttext, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(ttext, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = ttext, RIGHTARG = timestamptz,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = ttext, RIGHTARG = timestampset,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = ttext, RIGHTARG = period,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = ttext, RIGHTARG = periodset,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
