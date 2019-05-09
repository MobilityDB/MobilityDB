/*****************************************************************************
 *
 * RelativePosOps.sql
 *	  Relative position operations for 2D (1D value + 1D time) temporal types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/* operators selectively functions */

CREATE FUNCTION left_value_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'left_value_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION overleft_value_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'overleft_value_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION right_value_sel(internal, oid, internal, integer)
    RETURNS float
    AS 'MODULE_PATHNAME', 'right_value_sel'
    LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION overright_value_sel(internal, oid, internal, integer)
    RETURNS float
    AS 'MODULE_PATHNAME', 'overright_value_sel'
    LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION before_temporal_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'before_temporal_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION overbefore_temporal_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'overbefore_temporal_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION after_temporal_sel(internal, oid, internal, integer)
    RETURNS float
    AS 'MODULE_PATHNAME', 'after_temporal_sel'
    LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION overafter_temporal_sel(internal, oid, internal, integer)
    RETURNS float
    AS 'MODULE_PATHNAME', 'overafter_temporal_sel'
    LANGUAGE C IMMUTABLE STRICT;


CREATE FUNCTION left_point_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'left_point_sel'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overleft_point_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'overleft_point_sel'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION right_point_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'right_point_sel'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overright_point_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'overright_point_sel'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION above_point_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'above_point_sel'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overabove_point_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'overabove_point_sel'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION below_point_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'below_point_sel'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overbelow_point_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'overbelow_point_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION back_point_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'back_point_sel'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overback_point_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'overback_point_sel'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION front_point_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'front_point_sel'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overfront_point_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'overfront_point_sel'
	LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************
 * period
 *****************************************************************************/
/* period op tbool */

CREATE FUNCTION temporal_before(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = period, RIGHTARG = tbool,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = period, RIGHTARG = tbool,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = period, RIGHTARG = tbool,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = period, RIGHTARG = tbool,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* period op ttext */

CREATE FUNCTION temporal_before(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = period, RIGHTARG = ttext,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = period, RIGHTARG = ttext,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = period, RIGHTARG = ttext,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = period, RIGHTARG = ttext,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * box 
 *****************************************************************************/
/* box op tint */

CREATE FUNCTION temporal_left(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = box, RIGHTARG = tint,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = box, RIGHTARG = tint,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = box, RIGHTARG = tint,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = box, RIGHTARG = tint,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<# (
	LEFTARG = box, RIGHTARG = tint,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = box, RIGHTARG = tint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = box, RIGHTARG = tint,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = box, RIGHTARG = tint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);  

/*****************************************************************************/
/* box op tfloat */

CREATE FUNCTION temporal_left(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = box, RIGHTARG = tfloat,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = box, RIGHTARG = tfloat,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = box, RIGHTARG = tfloat,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = box, RIGHTARG = tfloat,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<# (
	LEFTARG = box, RIGHTARG = tfloat,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = box, RIGHTARG = tfloat,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = box, RIGHTARG = tfloat,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = box, RIGHTARG = tfloat,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);  

/*****************************************************************************
 * tbool
 *****************************************************************************/
/* tbool op period */

CREATE FUNCTION temporal_before(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tbool, RIGHTARG = period,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tbool, RIGHTARG = period,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tbool, RIGHTARG = period,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tbool, RIGHTARG = period,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tbool op tbool */

CREATE FUNCTION temporal_before(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tbool, RIGHTARG = tbool,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tbool, RIGHTARG = tbool,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tbool, RIGHTARG = tbool,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tbool, RIGHTARG = tbool,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * tint
 *****************************************************************************/
/* tint op box */

CREATE FUNCTION temporal_left(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tint, RIGHTARG = box,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tint, RIGHTARG = box,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tint, RIGHTARG = box,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tint, RIGHTARG = box,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<# (
	LEFTARG = tint, RIGHTARG = box,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tint, RIGHTARG = box,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tint, RIGHTARG = box,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tint, RIGHTARG = box,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tint op tint */

CREATE FUNCTION temporal_left(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<# (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tint op tfloat */

CREATE FUNCTION temporal_left(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tint, RIGHTARG = tfloat,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tint, RIGHTARG = tfloat,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tint, RIGHTARG = tfloat,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tint, RIGHTARG = tfloat,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<# (
	LEFTARG = tint, RIGHTARG = tfloat,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tint, RIGHTARG = tfloat,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tint, RIGHTARG = tfloat,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tint, RIGHTARG = tfloat,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * tfloat
 *****************************************************************************/
/* tfloat op box */

CREATE FUNCTION temporal_left(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tfloat, RIGHTARG = box,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tfloat, RIGHTARG = box,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tfloat, RIGHTARG = box,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tfloat, RIGHTARG = box,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<# (
	LEFTARG = tfloat, RIGHTARG = box,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tfloat, RIGHTARG = box,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tfloat, RIGHTARG = box,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tfloat, RIGHTARG = box,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tfloat op tint */

CREATE FUNCTION temporal_left(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tfloat, RIGHTARG = tint,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tfloat, RIGHTARG = tint,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tfloat, RIGHTARG = tint,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tfloat, RIGHTARG = tint,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<# (
	LEFTARG = tfloat, RIGHTARG = tint,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tfloat, RIGHTARG = tint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tfloat, RIGHTARG = tint,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tfloat, RIGHTARG = tint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tfloat op tfloat */

CREATE FUNCTION temporal_left(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<# (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * ttext
 *****************************************************************************/

/* ttext op period */

CREATE FUNCTION temporal_before(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = ttext, RIGHTARG = period,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = ttext, RIGHTARG = period,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = ttext, RIGHTARG = period,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = ttext, RIGHTARG = period,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* ttext op ttext */

CREATE FUNCTION temporal_before(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = ttext, RIGHTARG = ttext,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = ttext, RIGHTARG = ttext,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = ttext, RIGHTARG = ttext,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = ttext, RIGHTARG = ttext,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
