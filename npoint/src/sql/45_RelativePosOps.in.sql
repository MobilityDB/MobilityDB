/*****************************************************************************
 *
 * RelativePosOps.sql
 *	  Relative position operators for temporal network-constrained points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Xinyang Li,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Geometry
 *****************************************************************************/

CREATE FUNCTION temporal_left(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = geometry, RIGHTARG = tnpoint,
	PROCEDURE = temporal_left,
	COMMUTATOR = '>>',
	RESTRICT = left_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = geometry, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = geometry, RIGHTARG = tnpoint,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = right_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = geometry, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<| (
	LEFTARG = geometry, RIGHTARG = tnpoint,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = below_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<| (
	LEFTARG = geometry, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overbelow,
	RESTRICT = overbelow_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |>> (
	LEFTARG = geometry, RIGHTARG = tnpoint,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = above_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |&> (
	LEFTARG = geometry, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overabove,
	RESTRICT = overabove_point_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * gbox
 *****************************************************************************/
 
CREATE FUNCTION temporal_left(gbox, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(gbox, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(gbox, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(gbox, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(gbox, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(gbox, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(gbox, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(gbox, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(gbox, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(gbox, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(gbox, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(gbox, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = gbox, RIGHTARG = tnpoint,
	PROCEDURE = temporal_left,
	COMMUTATOR = '>>',
	RESTRICT = left_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = gbox, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = gbox, RIGHTARG = tnpoint,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = right_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = gbox, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<| (
	LEFTARG = gbox, RIGHTARG = tnpoint,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = below_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<| (
	LEFTARG = gbox, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overbelow,
	RESTRICT = overbelow_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |>> (
	LEFTARG = gbox, RIGHTARG = tnpoint,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = above_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |&> (
	LEFTARG = gbox, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overabove,
	RESTRICT = overabove_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<# (
	LEFTARG = gbox, RIGHTARG = tnpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = gbox, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overbefore,
	COMMUTATOR = '#&>',
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = gbox, RIGHTARG = tnpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = gbox, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overafter,
	COMMUTATOR = '&<#',
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * timestamptz
 *****************************************************************************/

CREATE FUNCTION temporal_before(timestamptz, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = timestamptz, RIGHTARG = tnpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = timestamptz, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overbefore,
	COMMUTATOR = '#&>',
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = timestamptz, RIGHTARG = tnpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = timestamptz, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overafter,
	COMMUTATOR = '&<#',
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * timestampset
 *****************************************************************************/

CREATE FUNCTION temporal_before(timestampset, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = timestampset, RIGHTARG = tnpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = timestampset, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overbefore,
	COMMUTATOR = '#&>',
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = timestampset, RIGHTARG = tnpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = timestampset, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overafter,
	COMMUTATOR = '&<#',
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * Period
 *****************************************************************************/

CREATE FUNCTION temporal_before(period, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = period, RIGHTARG = tnpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = period, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overbefore,
	COMMUTATOR = '#&>',
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = period, RIGHTARG = tnpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = period, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overafter,
	COMMUTATOR = '&<#',
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * PeriodSet
 *****************************************************************************/

CREATE FUNCTION temporal_before(periodset, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = periodset, RIGHTARG = tnpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = periodset, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overbefore,
	COMMUTATOR = '#&>',
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = periodset, RIGHTARG = tnpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = periodset, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overafter,
	COMMUTATOR = '&<#',
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * tnpoint
 *****************************************************************************/

/* tnpoint op geometry */

CREATE FUNCTION temporal_left(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tnpoint, RIGHTARG = geometry,
	PROCEDURE = temporal_left,
	COMMUTATOR = '>>',
	RESTRICT = left_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tnpoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tnpoint, RIGHTARG = geometry,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = right_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tnpoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<| (
	LEFTARG = tnpoint, RIGHTARG = geometry,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = below_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<| (
	LEFTARG = tnpoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overbelow,
	RESTRICT = overbelow_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |>> (
	LEFTARG = tnpoint, RIGHTARG = geometry,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = above_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |&> (
	LEFTARG = tnpoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overabove,
	RESTRICT = overabove_point_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tnpoint op gbox */

CREATE FUNCTION temporal_left(tnpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tnpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tnpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tnpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tnpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tnpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tnpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tnpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tnpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tnpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tnpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tnpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tnpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_left,
	COMMUTATOR = '>>',
	RESTRICT = left_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tnpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tnpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = right_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tnpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<| (
	LEFTARG = tnpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = below_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<| (
	LEFTARG = tnpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overbelow,
	RESTRICT = overbelow_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |>> (
	LEFTARG = tnpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = above_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |&> (
	LEFTARG = tnpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overabove,
	RESTRICT = overabove_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<# (
	LEFTARG = tnpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tnpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overbefore,
	COMMUTATOR = '#&>',
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tnpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tnpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overafter,
	COMMUTATOR = '&<#',
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tnpoint op timestamptz */

CREATE FUNCTION temporal_before(tnpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tnpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tnpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tnpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tnpoint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tnpoint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_overbefore,
	COMMUTATOR = '#&>',
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tnpoint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tnpoint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_overafter,
	COMMUTATOR = '&<#',
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tnpoint op timestampset */

CREATE FUNCTION temporal_before(tnpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tnpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tnpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tnpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tnpoint, RIGHTARG = timestampset,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tnpoint, RIGHTARG = timestampset,
	PROCEDURE = temporal_overbefore,
	COMMUTATOR = '#&>',
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tnpoint, RIGHTARG = timestampset,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tnpoint, RIGHTARG = timestampset,
	PROCEDURE = temporal_overafter,
	COMMUTATOR = '&<#',
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tnpoint op period */

CREATE FUNCTION temporal_before(tnpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tnpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tnpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tnpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tnpoint, RIGHTARG = period,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tnpoint, RIGHTARG = period,
	PROCEDURE = temporal_overbefore,
	COMMUTATOR = '#&>',
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tnpoint, RIGHTARG = period,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tnpoint, RIGHTARG = period,
	PROCEDURE = temporal_overafter,
	COMMUTATOR = '&<#',
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tnpoint op periodset */

CREATE FUNCTION temporal_before(tnpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tnpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tnpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tnpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tnpoint, RIGHTARG = periodset,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tnpoint, RIGHTARG = periodset,
	PROCEDURE = temporal_overbefore,
	COMMUTATOR = '#&>',
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tnpoint, RIGHTARG = periodset,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tnpoint, RIGHTARG = periodset,
	PROCEDURE = temporal_overafter,
	COMMUTATOR = '&<#',
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tnpoint op tnpoint */

CREATE FUNCTION temporal_left(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = temporal_left,
	COMMUTATOR = '>>',
	RESTRICT = left_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = right_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<| (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = below_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<| (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overbelow,
	RESTRICT = overbelow_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |>> (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = above_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |&> (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overabove,
	RESTRICT = overabove_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<# (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overbefore,
	COMMUTATOR = '#&>',
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = temporal_overafter,
	COMMUTATOR = '&<#',
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
