/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Relative position operators for temporal geometry points
 */

/*****************************************************************************
 * tstzspan
 *****************************************************************************/

/* tstzspan op tgeompoint */

CREATE FUNCTION stboxBefore(tstzspan, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(tstzspan, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(tstzspan, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(tstzspan, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tgeompoint,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tgeompoint,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tstzspan op tgeogpoint */

CREATE FUNCTION stboxBefore(tstzspan, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(tstzspan, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(tstzspan, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(tstzspan, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tgeogpoint,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tgeogpoint,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tgeogpoint,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tgeogpoint,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Stbox
 *****************************************************************************/

CREATE FUNCTION stboxLeft(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverleft(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxRight(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverright(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBelow(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbelow(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAbove(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverabove(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxFront(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverfront(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBack(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverback(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBefore(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(stbox, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = stboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverleft,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = stboxRight,
  COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverright,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = stboxBelow,
  COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverbelow,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = stboxAbove,
  COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverabove,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <</ (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = stboxFront,
  COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &</ (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverfront,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR />> (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = stboxBack,
  COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR /&> (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverback,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION stboxBefore(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(stbox, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = stbox, RIGHTARG = tgeogpoint,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = stbox, RIGHTARG = tgeogpoint,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = stbox, RIGHTARG = tgeogpoint,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = stbox, RIGHTARG = tgeogpoint,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * tgeompoint
 *****************************************************************************/

/* tgeompoint op tstzspan */

CREATE FUNCTION stboxBefore(tgeompoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(tgeompoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(tgeompoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(tgeompoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeompoint, RIGHTARG = tstzspan,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeompoint, RIGHTARG = tstzspan,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeompoint, RIGHTARG = tstzspan,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeompoint, RIGHTARG = tstzspan,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tgeompoint op stbox */

CREATE FUNCTION stboxLeft(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverleft(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxRight(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverright(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBelow(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbelow(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAbove(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverabove(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxFront(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverfront(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBack(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverback(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBefore(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(tgeompoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = stboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = stboxOverleft,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = stboxRight,
  COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = stboxOverright,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = stboxBelow,
  COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = stboxOverbelow,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = stboxAbove,
  COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = stboxOverabove,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <</ (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = stboxFront,
  COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &</ (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = stboxOverfront,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR />> (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = stboxBack,
  COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR /&> (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = stboxOverback,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tgeompoint op tgeompoint */

CREATE FUNCTION stboxLeft(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverleft(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxRight(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverright(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBelow(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbelow(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAbove(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverabove(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxFront(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverfront(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBack(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverback(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBefore(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = stboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverleft,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = stboxRight,
  COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverright,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = stboxBelow,
  COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverbelow,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = stboxAbove,
  COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverabove,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <</ (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = stboxFront,
  COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &</ (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverfront,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR />> (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = stboxBack,
  COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR /&> (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverback,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * tgeogpoint
 *****************************************************************************/

/* tgeogpoint op tstzspan */

CREATE FUNCTION stboxBefore(tgeogpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(tgeogpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(tgeogpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(tgeogpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeogpoint, RIGHTARG = tstzspan,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeogpoint, RIGHTARG = tstzspan,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeogpoint, RIGHTARG = tstzspan,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeogpoint, RIGHTARG = tstzspan,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tgeogpoint op stbox */

CREATE FUNCTION stboxBefore(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(tgeogpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeogpoint, RIGHTARG = stbox,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeogpoint, RIGHTARG = stbox,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeogpoint, RIGHTARG = stbox,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeogpoint, RIGHTARG = stbox,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tgeogpoint op tgeogpoint */

CREATE FUNCTION stboxBefore(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/
