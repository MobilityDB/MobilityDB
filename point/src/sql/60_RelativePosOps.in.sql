/*****************************************************************************
 *
 * RelativePosOps.sql
 *	  Relative position operators for 4D (2D/3D spatial value + 1D time)
 *	  temporal points
 *
 * Temporal geometric points have associated operators for the spatial and
 * temporal dimensions while temporal geographic points only for the temporal
 * dimension.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Geometry
 *****************************************************************************/

CREATE FUNCTION temporal_left(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'front_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overfront_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'back_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overback_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_left,
	COMMUTATOR = '>>',
	RESTRICT = left_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = right_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<| (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = below_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<| (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbelow,
	RESTRICT = overbelow_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |>> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = above_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |&> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overabove,
	RESTRICT = overabove_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <</ (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = below_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &</ (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overfront,
	RESTRICT = overbelow_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR />> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = above_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR /&> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overback,
	RESTRICT = overabove_point_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * gbox
 *****************************************************************************/

CREATE FUNCTION temporal_left(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'front_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overfront_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'back_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overback_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	PROCEDURE = temporal_left,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = '>>',
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	PROCEDURE = temporal_overleft,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	PROCEDURE = temporal_overright,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<| (
	PROCEDURE = temporal_below,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = '|>>',
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<| (
	PROCEDURE = temporal_overbelow,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR |>> (
	PROCEDURE = temporal_above,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = '<<|',
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR |&> (
	PROCEDURE = temporal_overabove,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR <</ (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = below_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &</ (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = temporal_overfront,
	RESTRICT = overbelow_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR />> (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = above_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR /&> (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = temporal_overback,
	RESTRICT = overabove_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = '#>>',
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = '<<#',
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = positionseltemp, JOIN = positionjoinseltemp
);

/******************************************************************************/

CREATE FUNCTION temporal_left(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'front_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overfront_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'back_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overback_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_left,
	COMMUTATOR = '>>',
	RESTRICT = left_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = right_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<| (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = below_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<| (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbelow,
	RESTRICT = overbelow_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |>> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = above_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |&> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overabove,
	RESTRICT = overabove_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <</ (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = below_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &</ (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overfront,
	RESTRICT = overbelow_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR />> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = above_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR /&> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overback,
	RESTRICT = overabove_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<# (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION temporal_before(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * timestamptz
 *****************************************************************************/

CREATE FUNCTION temporal_before(timestamptz, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = timestamptz, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = timestamptz, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = timestamptz, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = timestamptz, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION temporal_before(timestamptz, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * timestampset
 *****************************************************************************/

CREATE FUNCTION temporal_before(timestampset, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = timestampset, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = timestampset, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = timestampset, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = timestampset, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION temporal_before(timestampset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = timestampset, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = timestampset, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = timestampset, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = timestampset, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * Period
 *****************************************************************************/

CREATE FUNCTION temporal_before(period, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = period, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = period, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = period, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = period, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION temporal_before(period, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = period, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = period, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = period, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = period, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * PeriodSet
 *****************************************************************************/

CREATE FUNCTION temporal_before(periodset, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = periodset, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = periodset, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = periodset, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<</',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = periodset, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

CREATE FUNCTION temporal_before(periodset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = periodset, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = periodset, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = periodset, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = periodset, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * tgeompoint
 *****************************************************************************/

 /* tgeompoint op geometry */

CREATE FUNCTION temporal_left(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'front_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overfront_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'back_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overback_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_left,
	COMMUTATOR = '>>',
	RESTRICT = left_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = right_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<| (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = below_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<| (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overbelow,
	RESTRICT = overbelow_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |>> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = above_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |&> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overabove,
	RESTRICT = overabove_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <</ (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = below_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &</ (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overfront,
	RESTRICT = overbelow_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR />> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = above_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR /&> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overback,
	RESTRICT = overabove_point_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tgeompoint op gbox */

CREATE FUNCTION temporal_left(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'front_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overfront_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'back_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overback_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_left,
	COMMUTATOR = '>>',
	RESTRICT = left_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = right_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<| (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = below_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<| (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overbelow,
	RESTRICT = overbelow_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |>> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = above_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |&> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overabove,
	RESTRICT = overabove_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <</ (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = below_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &</ (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overfront,
	RESTRICT = overbelow_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR />> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = above_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR /&> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overback,
	RESTRICT = overabove_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<# (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tgeompoint op timestamptz */

CREATE FUNCTION temporal_before(tgeompoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeompoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeompoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeompoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeompoint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeompoint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeompoint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeompoint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tgeompoint op timestampset */

CREATE FUNCTION temporal_before(tgeompoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeompoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeompoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeompoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeompoint, RIGHTARG = timestampset,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeompoint, RIGHTARG = timestampset,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeompoint, RIGHTARG = timestampset,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeompoint, RIGHTARG = timestampset,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tgeompoint op period */

CREATE FUNCTION temporal_before(tgeompoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeompoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeompoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeompoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeompoint, RIGHTARG = period,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeompoint, RIGHTARG = period,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeompoint, RIGHTARG = period,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeompoint, RIGHTARG = period,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tgeompoint op periodset */

CREATE FUNCTION temporal_before(tgeompoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeompoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeompoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeompoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeompoint, RIGHTARG = periodset,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeompoint, RIGHTARG = periodset,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeompoint, RIGHTARG = periodset,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeompoint, RIGHTARG = periodset,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tgeompoint op tgeompoint */

CREATE FUNCTION temporal_left(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'front_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overfront_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'back_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overback_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_left,
	COMMUTATOR = '>>',
	RESTRICT = left_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &< (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overleft,
	RESTRICT = overleft_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR >> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = right_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overright,
	RESTRICT = overright_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<| (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = below_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<| (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbelow,
	RESTRICT = overbelow_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |>> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = above_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR |&> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overabove,
	RESTRICT = overabove_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <</ (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = below_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &</ (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overfront,
	RESTRICT = overbelow_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR />> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = above_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR /&> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overback,
	RESTRICT = overabove_point_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR <<# (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * tgeogpoint
 *****************************************************************************/

/* tgeogpoint op timestamptz */

CREATE FUNCTION temporal_before(tgeogpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tgeogpoint op timestampset */

CREATE FUNCTION temporal_before(tgeogpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = timestampset,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = timestampset,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = timestampset,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = timestampset,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tgeogpoint op period */

CREATE FUNCTION temporal_before(tgeogpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = period,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = period,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = period,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = period,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tgeogpoint op periodset */

CREATE FUNCTION temporal_before(tgeogpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = periodset,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = periodset,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = periodset,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = periodset,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tgeogpoint op gbox */

CREATE FUNCTION temporal_before(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tgeogpoint op tgeogpoint */

CREATE FUNCTION temporal_before(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
