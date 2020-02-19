/*****************************************************************************
 *
 * temporal_boxops.sql
 *	  Bounding tbox operators for temporal types.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Casting for tbox
 *****************************************************************************/

CREATE FUNCTION floatrange(tbox)
	RETURNS floatrange
	AS 'MODULE_PATHNAME', 'tbox_to_floatrange'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION period(tbox)
	RETURNS period
	AS 'MODULE_PATHNAME', 'tbox_to_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE CAST (tbox AS floatrange) WITH FUNCTION floatrange(tbox);
CREATE CAST (tbox AS period) WITH FUNCTION period(tbox);

/*****************************************************************************/

CREATE FUNCTION tbox(integer)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'int_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(float)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'float_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(numeric)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'numeric_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(intrange)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'intrange_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(floatrange)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'floatrange_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(timestamptz)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'timestamp_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(period)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'period_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(timestampset)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'timestampset_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(periodset)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'periodset_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(tint)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'tnumber_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(tfloat)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'tnumber_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (int AS tbox) WITH FUNCTION tbox(int) AS IMPLICIT;
CREATE CAST (float AS tbox) WITH FUNCTION tbox(float) AS IMPLICIT;
CREATE CAST (numeric AS tbox) WITH FUNCTION tbox(numeric) AS IMPLICIT;
CREATE CAST (timestamptz AS tbox) WITH FUNCTION tbox(timestamptz) AS IMPLICIT;
CREATE CAST (timestampset AS tbox) WITH FUNCTION tbox(timestampset) AS IMPLICIT;
CREATE CAST (period AS tbox) WITH FUNCTION tbox(period) AS IMPLICIT;
CREATE CAST (periodset AS tbox) WITH FUNCTION tbox(periodset) AS IMPLICIT;
CREATE CAST (tint AS tbox) WITH FUNCTION tbox(tint);
CREATE CAST (tfloat AS tbox) WITH FUNCTION tbox(tfloat);
-- We cannot make the castings from range to tbox implicit since this produces
-- an ambiguity with the implicit castings to anyrange
CREATE CAST (intrange AS tbox) WITH FUNCTION tbox(intrange);
CREATE CAST (floatrange AS tbox) WITH FUNCTION tbox(floatrange);

CREATE FUNCTION tbox(integer, timestamptz)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'int_timestamp_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(intrange, timestamptz)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'intrange_timestamp_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(float, timestamptz)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'float_timestamp_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(floatrange, timestamptz)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'floatrange_timestamp_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(integer, period)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'int_period_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(intrange, period)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'intrange_period_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(float, period)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'float_period_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(floatrange, period)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'floatrange_period_to_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Selectively functions for operators
 *****************************************************************************/

CREATE FUNCTION temporal_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'temporal_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION temporal_joinsel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'temporal_joinsel'
	LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************/

CREATE FUNCTION tnumber_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tnumber_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnumber_joinsel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tnumber_joinsel'
	LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************
 * tbox operators
 *****************************************************************************/

CREATE FUNCTION tbox_contains(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_contained(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_overlaps(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_same(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_adjacent(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'adjacent_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = tbox_contains,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = <@,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = tbox_contained,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = @>,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = tbox_overlaps,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = &&,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = tbox_same,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = ~=,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = tbox_adjacent,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = -|-,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * Temporal boolean
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = period, RIGHTARG = tbool,
	COMMUTATOR = &&,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tbool, RIGHTARG = period,
	COMMUTATOR = &&,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = &&,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = period, RIGHTARG = tbool,
	COMMUTATOR = <@,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tbool, RIGHTARG = period,
	COMMUTATOR = <@,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = <@,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = period, RIGHTARG = tbool,
	COMMUTATOR = @>,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tbool, RIGHTARG = period,
	COMMUTATOR = @>,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = @>,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = period, RIGHTARG = tbool,
	COMMUTATOR = ~=,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tbool, RIGHTARG = period,
	COMMUTATOR = ~=,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = ~=,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * Temporal integer
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(tbox, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tbox_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_tbox'
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
	LEFTARG = intrange, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = intrange,
	COMMUTATOR = &&,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tbox, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = tbox,
	COMMUTATOR = &&,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbox, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tbox_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_tbox'
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
	LEFTARG = intrange, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = intrange,
	COMMUTATOR = <@,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tbox, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = tbox,
	COMMUTATOR = <@,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbox, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tbox_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_tbox'
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
	LEFTARG = intrange, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = intrange,
	COMMUTATOR = @>,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tbox, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = tbox,
	COMMUTATOR = @>,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbox, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tbox_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_tbox'
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
	LEFTARG = intrange, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = intrange,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tbox, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = tbox,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * Temporal float
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(tbox, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tbox_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_tbox'
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
	LEFTARG = floatrange, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = floatrange,
	COMMUTATOR = &&,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tbox, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = tbox,
	COMMUTATOR = &&,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbox, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tbox_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_tbox'
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
	LEFTARG = floatrange, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = floatrange,
	COMMUTATOR = <@,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tbox, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = tbox,
	COMMUTATOR = <@,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbox, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tbox_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_tbox'
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
	LEFTARG = floatrange, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = floatrange,
	COMMUTATOR = @>,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tbox, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = tbox,
	COMMUTATOR = @>,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbox, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tbox_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_tbox'
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
	LEFTARG = floatrange, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = floatrange,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tbox, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = tbox,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * Temporal text
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = period, RIGHTARG = ttext,
	COMMUTATOR = &&,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = ttext, RIGHTARG = period,
	COMMUTATOR = &&,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = &&,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = period, RIGHTARG = ttext,
	COMMUTATOR = <@,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = ttext, RIGHTARG = period,
	COMMUTATOR = <@,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = <@,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = period, RIGHTARG = ttext,
	COMMUTATOR = @>,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = ttext, RIGHTARG = period,
	COMMUTATOR = @>,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = @>,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = period, RIGHTARG = ttext,
	COMMUTATOR = ~=,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = ttext, RIGHTARG = period,
	COMMUTATOR = ~=,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = ~=,
	RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
