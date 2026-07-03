/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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

/**
 * @file
 * @brief Relative position operators for temporal geometries/geographies
 */

/*****************************************************************************
 * tstzspan
 *****************************************************************************/

/* tstzspan op tgeometry */

CREATE FUNCTION before(tstzspan, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tstzspan, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tstzspan, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tstzspan, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tgeometry,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tgeometry,
  PROCEDURE = overbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tgeometry,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tgeometry,
  PROCEDURE = overafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tstzspan op tgeography */

CREATE FUNCTION before(tstzspan, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tstzspan, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tstzspan, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tstzspan, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tgeography,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tgeography,
  PROCEDURE = overbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tgeography,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tgeography,
  PROCEDURE = overafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Stbox
 *****************************************************************************/

CREATE FUNCTION left(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION below(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbelow(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION above(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overabove(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION front(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overfront(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION back(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overback(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = overleft,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = overright,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = below,
  COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = overbelow,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = above,
  COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = overabove,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <</ (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = front,
  COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &</ (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = overfront,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR />> (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = back,
  COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR /&> (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = overback,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = overbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = overafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION before(stbox, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(stbox, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(stbox, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(stbox, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = stbox, RIGHTARG = tgeography,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = stbox, RIGHTARG = tgeography,
  PROCEDURE = overbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = stbox, RIGHTARG = tgeography,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = stbox, RIGHTARG = tgeography,
  PROCEDURE = overafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * tgeometry
 *****************************************************************************/

/* tgeometry op tstzspan */

CREATE FUNCTION before(tgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeometry, RIGHTARG = tstzspan,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeometry, RIGHTARG = tstzspan,
  PROCEDURE = overbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeometry, RIGHTARG = tstzspan,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeometry, RIGHTARG = tstzspan,
  PROCEDURE = overafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tgeometry op stbox */

CREATE FUNCTION left(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION below(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbelow(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION above(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overabove(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION front(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overfront(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION back(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overback(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = overleft,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = overright,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = below,
  COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = overbelow,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = above,
  COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = overabove,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <</ (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = front,
  COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &</ (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = overfront,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR />> (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = back,
  COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR /&> (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = overback,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = overbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = overafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tgeometry op tgeometry */

CREATE FUNCTION left(inst1 tgeometry, inst2 tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(inst1 tgeometry, inst2 tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(inst1 tgeometry, inst2 tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(inst1 tgeometry, inst2 tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION below(inst1 tgeometry, inst2 tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbelow(inst1 tgeometry, inst2 tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION above(inst1 tgeometry, inst2 tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overabove(inst1 tgeometry, inst2 tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION front(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overfront(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION back(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overback(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(inst1 tgeometry, inst2 tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(inst1 tgeometry, inst2 tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(inst1 tgeometry, inst2 tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(inst1 tgeometry, inst2 tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = overleft,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = overright,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = below,
  COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = overbelow,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = above,
  COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = overabove,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <</ (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = front,
  COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &</ (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = overfront,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR />> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = back,
  COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR /&> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = overback,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = overbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = overafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * tgeography
 *****************************************************************************/

/* tgeography op tstzspan */

CREATE FUNCTION before(tgeography, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tgeography, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tgeography, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tgeography, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeography, RIGHTARG = tstzspan,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeography, RIGHTARG = tstzspan,
  PROCEDURE = overbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeography, RIGHTARG = tstzspan,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeography, RIGHTARG = tstzspan,
  PROCEDURE = overafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tgeography op stbox */

CREATE FUNCTION before(tgeography, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tgeography, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tgeography, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tgeography, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeography, RIGHTARG = stbox,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeography, RIGHTARG = stbox,
  PROCEDURE = overbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeography, RIGHTARG = stbox,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeography, RIGHTARG = stbox,
  PROCEDURE = overafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tgeography op tgeography */

CREATE FUNCTION before(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeography, RIGHTARG = tgeography,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeography, RIGHTARG = tgeography,
  PROCEDURE = overbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeography, RIGHTARG = tgeography,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeography, RIGHTARG = tgeography,
  PROCEDURE = overafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/
