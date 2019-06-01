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

CREATE FUNCTION tpoint_position_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tpoint_position_sel'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_position_joinsel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tpoint_position_joinsel'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

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
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &< (
	PROCEDURE = temporal_overleft,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &> (
	PROCEDURE = temporal_overright,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<| (
	PROCEDURE = temporal_below,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = '|>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<| (
	PROCEDURE = temporal_overbelow,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |>> (
	PROCEDURE = temporal_above,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = '<<|',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |&> (
	PROCEDURE = temporal_overabove,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = '#>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = '<<#',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);

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
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overleft,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overright,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<| (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<| (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbelow,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |>> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |&> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overabove,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
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
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overleft,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overright,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<| (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<| (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbelow,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |>> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |&> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overabove,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
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
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
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
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overleft,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overright,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<| (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<| (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overbelow,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |>> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |&> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overabove,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
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
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overleft,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overright,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<| (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<| (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overbelow,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |>> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |&> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overabove,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
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
	AS 'MODULE_PATHNAME', 'before_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_left,
	COMMUTATOR = '>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overleft,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overright,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<| (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<| (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbelow,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |>> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |&> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overabove,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);

/*****************************************************************************
 * tgeogpoint
 *****************************************************************************/

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
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);

/*****************************************************************************/

/* tgeogpoint op tgeogpoint */

CREATE FUNCTION temporal_before(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);

/*****************************************************************************/
