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
 * timestamptz
 *****************************************************************************/
/* timestamptz op tbool */

CREATE FUNCTION temporal_before(timestamptz, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = timestamptz, RIGHTARG = tbool,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = timestamptz, RIGHTARG = tbool,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = timestamptz, RIGHTARG = tbool,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = timestamptz, RIGHTARG = tbool,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* timestamptz op tint */

CREATE FUNCTION temporal_before(timestamptz, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = timestamptz, RIGHTARG = tint,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = timestamptz, RIGHTARG = tint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = timestamptz, RIGHTARG = tint,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = timestamptz, RIGHTARG = tint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* timestamptz op tfloat */

CREATE FUNCTION temporal_before(timestamptz, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = timestamptz, RIGHTARG = tfloat,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = timestamptz, RIGHTARG = tfloat,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = timestamptz, RIGHTARG = tfloat,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = timestamptz, RIGHTARG = tfloat,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* timestamptz op ttext */

CREATE FUNCTION temporal_before(timestamptz, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = timestamptz, RIGHTARG = ttext,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = timestamptz, RIGHTARG = ttext,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = timestamptz, RIGHTARG = ttext,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = timestamptz, RIGHTARG = ttext,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * timestampset
 *****************************************************************************/
/* timestampset op tbool */

CREATE FUNCTION temporal_before(timestampset, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = timestampset, RIGHTARG = tbool,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = timestampset, RIGHTARG = tbool,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = timestampset, RIGHTARG = tbool,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = timestampset, RIGHTARG = tbool,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* timestampset op tint */

CREATE FUNCTION temporal_before(timestampset, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = timestampset, RIGHTARG = tint,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = timestampset, RIGHTARG = tint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = timestampset, RIGHTARG = tint,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = timestampset, RIGHTARG = tint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* timestampset op tfloat */

CREATE FUNCTION temporal_before(timestampset, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = timestampset, RIGHTARG = tfloat,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = timestampset, RIGHTARG = tfloat,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = timestampset, RIGHTARG = tfloat,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = timestampset, RIGHTARG = tfloat,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* timestampset op ttext */

CREATE FUNCTION temporal_before(timestampset, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = timestampset, RIGHTARG = ttext,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = timestampset, RIGHTARG = ttext,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = timestampset, RIGHTARG = ttext,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = timestampset, RIGHTARG = ttext,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

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
/* period op tint */

CREATE FUNCTION temporal_before(period, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = period, RIGHTARG = tint,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = period, RIGHTARG = tint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = period, RIGHTARG = tint,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = period, RIGHTARG = tint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* period op tfloat */

CREATE FUNCTION temporal_before(period, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = period, RIGHTARG = tfloat,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = period, RIGHTARG = tfloat,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = period, RIGHTARG = tfloat,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = period, RIGHTARG = tfloat,
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
 * periodset
 *****************************************************************************/
/* periodset op tbool */

CREATE FUNCTION temporal_before(periodset, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = periodset, RIGHTARG = tbool,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = periodset, RIGHTARG = tbool,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = periodset, RIGHTARG = tbool,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = periodset, RIGHTARG = tbool,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* periodset op tint */

CREATE FUNCTION temporal_before(periodset, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = periodset, RIGHTARG = tint,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = periodset, RIGHTARG = tint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = periodset, RIGHTARG = tint,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = periodset, RIGHTARG = tint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* periodset op tfloat */

CREATE FUNCTION temporal_before(periodset, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = periodset, RIGHTARG = tfloat,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = periodset, RIGHTARG = tfloat,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = periodset, RIGHTARG = tfloat,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = periodset, RIGHTARG = tfloat,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* periodset op ttext */

CREATE FUNCTION temporal_before(periodset, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = periodset, RIGHTARG = ttext,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = periodset, RIGHTARG = ttext,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = periodset, RIGHTARG = ttext,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = periodset, RIGHTARG = ttext,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * integer 
 *****************************************************************************/
/* integer op tint */

CREATE FUNCTION temporal_left(integer, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_datum_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(integer, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_datum_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(integer, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_datum_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(integer, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_datum_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = integer, RIGHTARG = tint,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = integer, RIGHTARG = tint,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = integer, RIGHTARG = tint,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = integer, RIGHTARG = tint,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * intrange 
 *****************************************************************************/
/* intrange op tint */

CREATE FUNCTION temporal_left(r intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(r intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(r intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(r intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = intrange, RIGHTARG = tint,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = intrange, RIGHTARG = tint,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = intrange, RIGHTARG = tint,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = intrange, RIGHTARG = tint,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* intrange op tfloat */

CREATE FUNCTION temporal_left(intrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(intrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(intrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(intrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = intrange, RIGHTARG = tfloat,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = intrange, RIGHTARG = tfloat,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = intrange, RIGHTARG = tfloat,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = intrange, RIGHTARG = tfloat,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * float 
 *****************************************************************************/
/* float op tint */

CREATE FUNCTION temporal_left(float, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_datum_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(float, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_datum_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(float, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_datum_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(float, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_datum_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = float, RIGHTARG = tint,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = float, RIGHTARG = tint,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = float, RIGHTARG = tint,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = float, RIGHTARG = tint,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* float op tfloat */

CREATE FUNCTION temporal_left(float, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_datum_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(float, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_datum_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(float, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_datum_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(float, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_datum_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = float, RIGHTARG = tfloat,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = float, RIGHTARG = tfloat,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = float, RIGHTARG = tfloat,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = float, RIGHTARG = tfloat,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * floatrange 
 *****************************************************************************/
/* floatrange op tint */

CREATE FUNCTION temporal_left(floatrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(floatrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(floatrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(floatrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = floatrange, RIGHTARG = tint,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = floatrange, RIGHTARG = tint,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = floatrange, RIGHTARG = tint,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = floatrange, RIGHTARG = tint,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* floatrange op tfloat */

CREATE FUNCTION temporal_left(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = floatrange, RIGHTARG = tfloat,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = floatrange, RIGHTARG = tfloat,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = floatrange, RIGHTARG = tfloat,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = floatrange, RIGHTARG = tfloat,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
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
/* tbool op timestamptz */

CREATE FUNCTION temporal_before(tbool, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tbool, RIGHTARG = timestamptz,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tbool, RIGHTARG = timestamptz,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tbool, RIGHTARG = timestamptz,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tbool, RIGHTARG = timestamptz,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tbool op timestampset */

CREATE FUNCTION temporal_before(tbool, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tbool, RIGHTARG = timestampset,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tbool, RIGHTARG = timestampset,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tbool, RIGHTARG = timestampset,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tbool, RIGHTARG = timestampset,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
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
/* tbool op periodset */

CREATE FUNCTION temporal_before(tbool, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tbool, RIGHTARG = periodset,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tbool, RIGHTARG = periodset,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tbool, RIGHTARG = periodset,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tbool, RIGHTARG = periodset,
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
/* tint op int */

CREATE FUNCTION temporal_left(tint, int)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, int)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, int)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, int)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tint, RIGHTARG = int,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tint, RIGHTARG = int,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tint, RIGHTARG = int,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tint, RIGHTARG = int,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tint op float */

CREATE FUNCTION temporal_left(tint, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tint, RIGHTARG = float,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tint, RIGHTARG = float,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tint, RIGHTARG = float,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tint, RIGHTARG = float,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tint op intrange */

CREATE FUNCTION temporal_left(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tint, RIGHTARG = intrange,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tint, RIGHTARG = intrange,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tint, RIGHTARG = intrange,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tint, RIGHTARG = intrange,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tint op floatrange */

CREATE FUNCTION temporal_left(tint, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tint, RIGHTARG = floatrange,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tint, RIGHTARG = floatrange,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tint, RIGHTARG = floatrange,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tint, RIGHTARG = floatrange,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tint op timestamptz */

CREATE FUNCTION temporal_before(tint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tint op timestampset */

CREATE FUNCTION temporal_before(tint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tint, RIGHTARG = timestampset,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tint, RIGHTARG = timestampset,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tint, RIGHTARG = timestampset,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tint, RIGHTARG = timestampset,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tint op period */

CREATE FUNCTION temporal_before(tint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tint, RIGHTARG = period,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tint, RIGHTARG = period,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tint, RIGHTARG = period,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tint, RIGHTARG = period,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tint op periodset */

CREATE FUNCTION temporal_before(tint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tint, RIGHTARG = periodset,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tint, RIGHTARG = periodset,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tint, RIGHTARG = periodset,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tint, RIGHTARG = periodset,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
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
/* tfloat op float */

CREATE FUNCTION temporal_left(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_datum'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tfloat op intrange */

CREATE FUNCTION temporal_left(tfloat, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tfloat, RIGHTARG = intrange,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tfloat, RIGHTARG = intrange,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tfloat, RIGHTARG = intrange,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tfloat, RIGHTARG = intrange,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tfloat op floatrange */

CREATE FUNCTION temporal_left(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tfloat, RIGHTARG = floatrange,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = left_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tfloat, RIGHTARG = floatrange,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tfloat, RIGHTARG = floatrange,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = right_value_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tfloat, RIGHTARG = floatrange,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_value_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tfloat op timestamptz */

CREATE FUNCTION temporal_before(tfloat, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tfloat, RIGHTARG = timestamptz,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tfloat, RIGHTARG = timestamptz,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tfloat, RIGHTARG = timestamptz,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tfloat, RIGHTARG = timestamptz,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tfloat op timestampset */

CREATE FUNCTION temporal_before(tfloat, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tfloat, RIGHTARG = timestampset,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tfloat, RIGHTARG = timestampset,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tfloat, RIGHTARG = timestampset,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tfloat, RIGHTARG = timestampset,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tfloat op period */

CREATE FUNCTION temporal_before(tfloat, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tfloat, RIGHTARG = period,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tfloat, RIGHTARG = period,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tfloat, RIGHTARG = period,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tfloat, RIGHTARG = period,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
/* tfloat op periodset */

CREATE FUNCTION temporal_before(tfloat, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tfloat, RIGHTARG = periodset,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tfloat, RIGHTARG = periodset,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tfloat, RIGHTARG = periodset,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tfloat, RIGHTARG = periodset,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
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

/* ttext op timestamptz */

CREATE FUNCTION temporal_before(ttext, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = ttext, RIGHTARG = timestamptz,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = ttext, RIGHTARG = timestamptz,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = ttext, RIGHTARG = timestamptz,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = ttext, RIGHTARG = timestamptz,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* ttext op timestampset */

CREATE FUNCTION temporal_before(ttext, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = ttext, RIGHTARG = timestampset,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = ttext, RIGHTARG = timestampset,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = ttext, RIGHTARG = timestampset,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = ttext, RIGHTARG = timestampset,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
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
/* ttext op periodset */

CREATE FUNCTION temporal_before(ttext, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = ttext, RIGHTARG = periodset,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = ttext, RIGHTARG = periodset,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = ttext, RIGHTARG = periodset,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = ttext, RIGHTARG = periodset,
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
