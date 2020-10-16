/*****************************************************************************
 *
 * temporal_boxops.sql
 *    Bounding tbox operators for temporal types.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Casting
 *****************************************************************************/

CREATE FUNCTION tbox(tint)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'tnumber_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(tfloat)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'tnumber_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tint AS tbox) WITH FUNCTION tbox(tint) AS IMPLICIT;
CREATE CAST (tfloat AS tbox) WITH FUNCTION tbox(tfloat) AS IMPLICIT;

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

/*****************************************************************************/

CREATE FUNCTION adjacent_bbox(period, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tbool, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = period, RIGHTARG = tbool,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tbool, RIGHTARG = period,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = -|-,
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

/*****************************************************************************/

CREATE FUNCTION adjacent_bbox(intrange, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tint, intrange)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tnumber_range'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = intrange, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tint, RIGHTARG = intrange,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tbox, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tint, RIGHTARG = tbox,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tint, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
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

/*****************************************************************************/

CREATE FUNCTION adjacent_bbox(floatrange, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tfloat, floatrange)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tnumber_range'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = floatrange, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tfloat, RIGHTARG = floatrange,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tbox, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tfloat, RIGHTARG = tbox,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tfloat, RIGHTARG = tint,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = -|-,
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

CREATE FUNCTION adjacent_bbox(period, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(ttext, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent_bbox(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_bbox_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = period, RIGHTARG = ttext,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = ttext, RIGHTARG = period,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent_bbox,
  LEFTARG = ttext, RIGHTARG = ttext,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
