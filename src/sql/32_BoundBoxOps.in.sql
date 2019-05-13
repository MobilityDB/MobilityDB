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

 /*****************************************************************************
 * Period
 *****************************************************************************/

CREATE FUNCTION period(tbool)
	RETURNS period
	AS 'MODULE_PATHNAME', 'temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(ttext)
	RETURNS period
	AS 'MODULE_PATHNAME', 'temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tbool AS period) WITH FUNCTION period(tbool);
CREATE CAST (ttext AS period) WITH FUNCTION period(ttext);

 /*****************************************************************************
 * BOX
 *****************************************************************************/

CREATE FUNCTION box(integer)
	RETURNS box
	AS 'MODULE_PATHNAME', 'int_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(float)
	RETURNS box
	AS 'MODULE_PATHNAME', 'float_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(numeric)
	RETURNS box
	AS 'MODULE_PATHNAME', 'numeric_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(intrange)
	RETURNS box
	AS 'MODULE_PATHNAME', 'intrange_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(floatrange)
	RETURNS box
	AS 'MODULE_PATHNAME', 'floatrange_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(timestamptz)
	RETURNS box
	AS 'MODULE_PATHNAME', 'timestamp_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(period)
	RETURNS box
	AS 'MODULE_PATHNAME', 'period_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(timestampset)
	RETURNS box
	AS 'MODULE_PATHNAME', 'timestampset_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(periodset)
	RETURNS box
	AS 'MODULE_PATHNAME', 'periodset_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(tint)
	RETURNS box
	AS 'MODULE_PATHNAME', 'tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(tfloat)
	RETURNS box
	AS 'MODULE_PATHNAME', 'tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (int AS box) WITH FUNCTION box(int) AS IMPLICIT;
CREATE CAST (float AS box) WITH FUNCTION box(float) AS IMPLICIT;
CREATE CAST (numeric AS box) WITH FUNCTION box(numeric) AS IMPLICIT;
-- We cannot make the castings from range to box implicit since this produces
-- an ambiguity with the implicit castings to anyrange TODO !!!
CREATE CAST (intrange AS box) WITH FUNCTION box(intrange) AS IMPLICIT;
CREATE CAST (floatrange AS box) WITH FUNCTION box(floatrange) AS IMPLICIT;
CREATE CAST (timestamptz AS box) WITH FUNCTION box(timestamptz) AS IMPLICIT;
CREATE CAST (timestampset AS box) WITH FUNCTION box(timestampset) AS IMPLICIT;
CREATE CAST (period AS box) WITH FUNCTION box(period) AS IMPLICIT;
CREATE CAST (periodset AS box) WITH FUNCTION box(periodset) AS IMPLICIT;
CREATE CAST (tint AS box) WITH FUNCTION box(tint);
CREATE CAST (tfloat AS box) WITH FUNCTION box(tfloat);

CREATE FUNCTION box(integer, timestamptz)
	RETURNS box
	AS 'MODULE_PATHNAME', 'int_timestamp_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(intrange, timestamptz)
	RETURNS box
	AS 'MODULE_PATHNAME', 'intrange_timestamp_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(float, timestamptz)
	RETURNS box
	AS 'MODULE_PATHNAME', 'float_timestamp_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(floatrange, timestamptz)
	RETURNS box
	AS 'MODULE_PATHNAME', 'floatrange_timestamp_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(integer, period)
	RETURNS box
	AS 'MODULE_PATHNAME', 'int_period_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(intrange, period)
	RETURNS box
	AS 'MODULE_PATHNAME', 'intrange_period_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(float, period)
	RETURNS box
	AS 'MODULE_PATHNAME', 'float_period_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(floatrange, period)
	RETURNS box
	AS 'MODULE_PATHNAME', 'floatrange_period_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Comparison
 *****************************************************************************/

CREATE FUNCTION box_eq(box, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'box_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box_ne(box, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'box_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box_lt(box, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'box_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION box_le(box, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'box_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION box_ge(box, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'box_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION box_gt(box, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'box_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION box_cmp(box, box)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'box_cmp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR = (
	LEFTARG = box, RIGHTARG = box,
	PROCEDURE = box_eq,
	COMMUTATOR = =,
	NEGATOR = <>,
	RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
	LEFTARG = box, RIGHTARG = box,
	PROCEDURE = box_ne,
	COMMUTATOR = <>,
	NEGATOR = =,
	RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
	PROCEDURE = box_lt,
	LEFTARG = box, RIGHTARG = box,
	COMMUTATOR = >, NEGATOR = >=,
	RESTRICT = areasel, JOIN = areajoinsel 
);
CREATE OPERATOR <= (
	PROCEDURE = box_le,
	LEFTARG = box, RIGHTARG = box,
	COMMUTATOR = >=, NEGATOR = >,
	RESTRICT = areasel, JOIN = areajoinsel 
);
CREATE OPERATOR >= (
	PROCEDURE = box_ge,
	LEFTARG = box, RIGHTARG = box,
	COMMUTATOR = <=, NEGATOR = <,
	RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR > (
	PROCEDURE = box_gt,
	LEFTARG = box, RIGHTARG = box,
	COMMUTATOR = <, NEGATOR = <=,
	RESTRICT = areasel, JOIN = areajoinsel
);

CREATE OPERATOR CLASS box_ops
	DEFAULT FOR TYPE box USING btree	AS
	OPERATOR	1	< ,
	OPERATOR	2	<= ,
	OPERATOR	3	= ,
	OPERATOR	4	>= ,
	OPERATOR	5	> ,
	FUNCTION	1	box_cmp(box, box);

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
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
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
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
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
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
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
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * Temporal integer
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_box_tnumber'
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
	LEFTARG = box, RIGHTARG = tint,
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

CREATE FUNCTION contains_bbox(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_box_tnumber'
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
	LEFTARG = box, RIGHTARG = tint,
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

CREATE FUNCTION contained_bbox(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_box_tnumber'
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
	LEFTARG = box, RIGHTARG = tint,
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

CREATE FUNCTION same_bbox(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_box_tnumber'
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
	LEFTARG = box, RIGHTARG = tint,
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

CREATE FUNCTION overlaps_bbox(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_box_tnumber'
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
	LEFTARG = box, RIGHTARG = tfloat,
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

CREATE FUNCTION contains_bbox(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_box_tnumber'
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
	LEFTARG = box, RIGHTARG = tfloat,
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

CREATE FUNCTION contained_bbox(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_box_tnumber'
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
	LEFTARG = box, RIGHTARG = tfloat,
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

CREATE FUNCTION same_bbox(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_box_tnumber'
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
	LEFTARG = box, RIGHTARG = tfloat,
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
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = &&,
	RESTRICT = overlaps_bbox_sel, JOIN = positionjoinseltemp
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
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = <@,
	RESTRICT = contains_bbox_sel, JOIN = positionjoinseltemp
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
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = @>,
	RESTRICT = contained_bbox_sel, JOIN = positionjoinseltemp
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
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = ~=,
	RESTRICT = same_bbox_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
