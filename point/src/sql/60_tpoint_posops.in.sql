/*****************************************************************************
 *
 * tpoint_relposops.sql
 *	  Relative position operators for 4D (2D/3D spatial value + 1D time value)
 *	  temporal points
 *
 * Temporal geometric points have associated operators for the spatial and
 * time dimensions while temporal geographic points only for the temporal
 * dimension.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * stbox
 *****************************************************************************/

CREATE FUNCTION temporal_left(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_stbox_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_stbox_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_stbox_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_stbox_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_stbox_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_stbox_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_stbox_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_stbox_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_stbox_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_stbox_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_stbox_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_stbox_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'front_stbox_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overfront_stbox_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'back_stbox_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overback_stbox_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	PROCEDURE = temporal_left,
	LEFTARG = stbox, RIGHTARG = stbox,
	COMMUTATOR = >>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &< (
	PROCEDURE = temporal_overleft,
	LEFTARG = stbox, RIGHTARG = stbox,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = stbox, RIGHTARG = stbox,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &> (
	PROCEDURE = temporal_overright,
	LEFTARG = stbox, RIGHTARG = stbox,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <<| (
	PROCEDURE = temporal_below,
	LEFTARG = stbox, RIGHTARG = stbox,
	COMMUTATOR = |>>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<| (
	PROCEDURE = temporal_overbelow,
	LEFTARG = stbox, RIGHTARG = stbox,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR |>> (
	PROCEDURE = temporal_above,
	LEFTARG = stbox, RIGHTARG = stbox,
	COMMUTATOR = <<|,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR |&> (
	PROCEDURE = temporal_overabove,
	LEFTARG = stbox, RIGHTARG = stbox,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = stbox, RIGHTARG = stbox,
	PROCEDURE = temporal_front,
	COMMUTATOR = />>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = stbox, RIGHTARG = stbox,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = stbox, RIGHTARG = stbox,
	PROCEDURE = temporal_back,
	COMMUTATOR = <</,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = stbox, RIGHTARG = stbox,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = stbox, RIGHTARG = stbox,
	COMMUTATOR = #>>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = stbox, RIGHTARG = stbox,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = stbox, RIGHTARG = stbox,
	COMMUTATOR = <<#,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = stbox, RIGHTARG = stbox,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
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
	COMMUTATOR = >>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overleft,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overright,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <<| (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_below,
	COMMUTATOR = |>>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<| (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbelow,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR |>> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_above,
	COMMUTATOR = <<|,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR |&> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overabove,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_front,
	COMMUTATOR = />>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_back,
	COMMUTATOR = <</,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/******************************************************************************/

CREATE FUNCTION temporal_left(stbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(stbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(stbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(stbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(stbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(stbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(stbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(stbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(stbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'front_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(stbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overfront_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(stbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'back_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(stbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overback_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(stbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(stbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(stbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(stbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = stbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = stbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overleft,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = stbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = stbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overright,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <<| (
	LEFTARG = stbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_below,
	COMMUTATOR = |>>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<| (
	LEFTARG = stbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbelow,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR |>> (
	LEFTARG = stbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_above,
	COMMUTATOR = <<|,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR |&> (
	LEFTARG = stbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overabove,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = stbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_front,
	COMMUTATOR = />>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = stbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = stbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_back,
	COMMUTATOR = <</,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = stbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = stbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = stbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = stbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = stbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_before(stbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(stbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(stbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(stbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_stbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = stbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = stbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = stbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = stbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
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
	COMMUTATOR = >>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overleft,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overright,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <<| (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_below,
	COMMUTATOR = |>>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<| (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overbelow,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR |>> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_above,
	COMMUTATOR = <<|,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR |&> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overabove,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_front,
	COMMUTATOR = />>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_back,
	COMMUTATOR = <</,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

/* tgeompoint op stbox */

CREATE FUNCTION temporal_left(tgeompoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tgeompoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tgeompoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tgeompoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tgeompoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tgeompoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tgeompoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tgeompoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tgeompoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'front_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tgeompoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overfront_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tgeompoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'back_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tgeompoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overback_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tgeompoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeompoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeompoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeompoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tgeompoint, RIGHTARG = stbox,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tgeompoint, RIGHTARG = stbox,
	PROCEDURE = temporal_overleft,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tgeompoint, RIGHTARG = stbox,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tgeompoint, RIGHTARG = stbox,
	PROCEDURE = temporal_overright,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <<| (
	LEFTARG = tgeompoint, RIGHTARG = stbox,
	PROCEDURE = temporal_below,
	COMMUTATOR = |>>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<| (
	LEFTARG = tgeompoint, RIGHTARG = stbox,
	PROCEDURE = temporal_overbelow,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR |>> (
	LEFTARG = tgeompoint, RIGHTARG = stbox,
	PROCEDURE = temporal_above,
	COMMUTATOR = <<|,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR |&> (
	LEFTARG = tgeompoint, RIGHTARG = stbox,
	PROCEDURE = temporal_overabove,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = tgeompoint, RIGHTARG = stbox,
	PROCEDURE = temporal_front,
	COMMUTATOR = />>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = tgeompoint, RIGHTARG = stbox,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = tgeompoint, RIGHTARG = stbox,
	PROCEDURE = temporal_back,
	COMMUTATOR = <</,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = tgeompoint, RIGHTARG = stbox,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = tgeompoint, RIGHTARG = stbox,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tgeompoint, RIGHTARG = stbox,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tgeompoint, RIGHTARG = stbox,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tgeompoint, RIGHTARG = stbox,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
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
	AS 'MODULE_PATHNAME', 'before_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overleft,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overright,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <<| (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_below,
	COMMUTATOR = |>>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<| (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbelow,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR |>> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_above,
	COMMUTATOR = <<|,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR |&> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overabove,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_front,
	COMMUTATOR = />>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_back,
	COMMUTATOR = <</,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************
 * tgeogpoint
 *****************************************************************************/

/* tgeogpoint op stbox */

CREATE FUNCTION temporal_before(tgeogpoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_tpoint_stbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = stbox,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = stbox,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = stbox,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = stbox,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

/* tgeogpoint op tgeogpoint */

CREATE FUNCTION temporal_before(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/
