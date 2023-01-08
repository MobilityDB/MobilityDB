/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

/*
 * tpoint_posops.sql
 * Relative position operators for temporal geometry points.
 */

/*****************************************************************************
 * timestamptz
 *****************************************************************************/
/* timestamptz op tgeompoint */

CREATE FUNCTION temporal_before(timestamptz, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = timestamptz, RIGHTARG = tgeompoint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = timestamptz, RIGHTARG = tgeompoint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = timestamptz, RIGHTARG = tgeompoint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = timestamptz, RIGHTARG = tgeompoint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/
/* timestamptz op tgeogpoint */

CREATE FUNCTION temporal_before(timestamptz, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************
 * tstzset
 *****************************************************************************/
/* tstzset op tgeompoint */

CREATE FUNCTION temporal_before(tstzset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzset, RIGHTARG = tgeompoint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzset, RIGHTARG = tgeompoint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzset, RIGHTARG = tgeompoint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzset, RIGHTARG = tgeompoint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/
/* tstzset op tgeogpoint */

CREATE FUNCTION temporal_before(tstzset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzset, RIGHTARG = tgeogpoint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzset, RIGHTARG = tgeogpoint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzset, RIGHTARG = tgeogpoint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzset, RIGHTARG = tgeogpoint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************
 * tstzspan
 *****************************************************************************/
/* tstzspan op tgeompoint */

CREATE FUNCTION temporal_before(tstzspan, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspan, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspan, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspan, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tgeompoint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tgeompoint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tgeompoint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tgeompoint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/
/* tstzspan op tgeogpoint */

CREATE FUNCTION temporal_before(tstzspan, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspan, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspan, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspan, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tgeogpoint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tgeogpoint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tgeogpoint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tgeogpoint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************
 * tstzspanset
 *****************************************************************************/
/* tstzspanset op tgeompoint */

CREATE FUNCTION temporal_before(tstzspanset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspanset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspanset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspanset, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspanset, RIGHTARG = tgeompoint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspanset, RIGHTARG = tgeompoint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspanset, RIGHTARG = tgeompoint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspanset, RIGHTARG = tgeompoint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/
/* tstzspanset op tgeogpoint */

CREATE FUNCTION temporal_before(tstzspanset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspanset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspanset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspanset, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspanset, RIGHTARG = tgeogpoint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspanset, RIGHTARG = tgeogpoint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspanset, RIGHTARG = tgeogpoint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspanset, RIGHTARG = tgeogpoint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************
 * Geometry
 *****************************************************************************/

CREATE FUNCTION temporal_left(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_geom_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_geom_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_geom_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_geom_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_geom_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_geom_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_geom_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_geom_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_geom_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_geom_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_geom_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_geom_tpoint'
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

/*****************************************************************************
 * Stbox
 *****************************************************************************/

CREATE FUNCTION temporal_left(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_stbox_tpoint'
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
  AS 'MODULE_PATHNAME', 'Before_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_stbox_tpoint'
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
/* tgeompoint op timestamptz */

CREATE FUNCTION temporal_before(tgeompoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeompoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeompoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeompoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeompoint, RIGHTARG = timestamptz,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeompoint, RIGHTARG = timestamptz,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeompoint, RIGHTARG = timestamptz,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeompoint, RIGHTARG = timestamptz,
  PROCEDURE = temporal_overafter,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/
/* tgeompoint op tstzset */

CREATE FUNCTION temporal_before(tgeompoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeompoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeompoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeompoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeompoint, RIGHTARG = tstzset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeompoint, RIGHTARG = tstzset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeompoint, RIGHTARG = tstzset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeompoint, RIGHTARG = tstzset,
  PROCEDURE = temporal_overafter,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/
/* tgeompoint op tstzspan */

CREATE FUNCTION temporal_before(tgeompoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeompoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeompoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeompoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeompoint, RIGHTARG = tstzspan,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeompoint, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeompoint, RIGHTARG = tstzspan,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeompoint, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overafter,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/
/* tgeompoint op tstzspanset */

CREATE FUNCTION temporal_before(tgeompoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeompoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeompoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeompoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeompoint, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeompoint, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeompoint, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeompoint, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_overafter,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/
 /* tgeompoint op geometry */

CREATE FUNCTION temporal_left(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpoint_geom'
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
  AS 'MODULE_PATHNAME', 'Left_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpoint_stbox'
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
  AS 'MODULE_PATHNAME', 'Left_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(inst1 tgeompoint, inst2 tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(inst1 tgeompoint, inst2 tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(inst1 tgeompoint, inst2 tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(inst1 tgeompoint, inst2 tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(inst1 tgeompoint, inst2 tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(inst1 tgeompoint, inst2 tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(inst1 tgeompoint, inst2 tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(inst1 tgeompoint, inst2 tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(inst1 tgeompoint, inst2 tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(inst1 tgeompoint, inst2 tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(inst1 tgeompoint, inst2 tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpoint_tpoint'
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

/* tgeogpoint op timestamptz */

CREATE FUNCTION temporal_before(tgeogpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
  PROCEDURE = temporal_overafter,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/
/* tgeogpoint op tstzset */

CREATE FUNCTION temporal_before(tgeogpoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeogpoint, RIGHTARG = tstzset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeogpoint, RIGHTARG = tstzset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeogpoint, RIGHTARG = tstzset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeogpoint, RIGHTARG = tstzset,
  PROCEDURE = temporal_overafter,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/
/* tgeogpoint op tstzspan */

CREATE FUNCTION temporal_before(tgeogpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeogpoint, RIGHTARG = tstzspan,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeogpoint, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeogpoint, RIGHTARG = tstzspan,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeogpoint, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overafter,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/
/* tgeogpoint op tstzspanset */

CREATE FUNCTION temporal_before(tgeogpoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeogpoint, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeogpoint, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeogpoint, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeogpoint, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_overafter,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/
/* tgeogpoint op stbox */

CREATE FUNCTION temporal_before(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpoint_stbox'
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
  AS 'MODULE_PATHNAME', 'Before_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpoint_tpoint'
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
