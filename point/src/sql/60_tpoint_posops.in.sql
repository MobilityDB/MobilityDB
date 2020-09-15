/*****************************************************************************
 *
 * tpoint_relposops.sql
 *    Relative position operators for 4D (2D/3D spatial value + 1D time value)
 *    temporal points
 *
 * Temporal geometric points have associated operators for the spatial and
 * time dimensions while temporal geographic points only for the temporal
 * dimension.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Geometry
 *****************************************************************************/

CREATE FUNCTION temporal_left(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_left(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overleft(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_right(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overright(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_below(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overbelow(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_above(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overabove(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_front(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overfront(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_back(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overback(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

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
  AS 'SELECT temporal_left($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(stbox, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overleft($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(stbox, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_right($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(stbox, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overright($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(stbox, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_below($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(stbox, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overbelow($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(stbox, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_above($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(stbox, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overabove($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(stbox, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_front($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(stbox, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overfront($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(stbox, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_back($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(stbox, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overback($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(stbox, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_before($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(stbox, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overbefore($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(stbox, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_after($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(stbox, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overafter($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

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
  AS 'SELECT temporal_before($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(stbox, tgeogpoint)
  RETURNS boolean
  AS 'SELECT temporal_overbefore($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(stbox, tgeogpoint)
  RETURNS boolean
  AS 'SELECT temporal_after($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(stbox, tgeogpoint)
  RETURNS boolean
  AS 'SELECT temporal_overafter($1, CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

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
  AS 'SELECT temporal_left(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT temporal_overleft(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT temporal_right(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT temporal_overright(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT temporal_below(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT temporal_overbelow(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT temporal_above(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT temporal_overabove(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT temporal_front(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT temporal_overfront(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT temporal_back(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT temporal_overback(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

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
  AS 'SELECT temporal_left(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tgeompoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_overleft(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tgeompoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_right(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tgeompoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_overright(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tgeompoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_below(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tgeompoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_overbelow(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tgeompoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_above(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tgeompoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_overabove(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tgeompoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_front(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tgeompoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_overfront(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tgeompoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_back(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tgeompoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_overback(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tgeompoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_before(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeompoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_overbefore(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeompoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_after(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeompoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_overafter(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

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

CREATE FUNCTION temporal_left(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_left(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overleft(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_right(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overright(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_below(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overbelow(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_above(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overabove(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_front(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overfront(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_back(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overback(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_before(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overbefore(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_after(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT temporal_overafter(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

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
  AS 'SELECT temporal_before(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_overbefore(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_after(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, stbox)
  RETURNS boolean
  AS 'SELECT temporal_overafter(CAST($1 AS stbox), $2)'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

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
  AS 'SELECT temporal_before(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'SELECT temporal_overbefore(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'SELECT temporal_after(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'SELECT temporal_overafter(CAST($1 AS stbox), CAST($2 AS stbox))'
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE;

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
