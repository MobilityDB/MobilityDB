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

CREATE FUNCTION stboxBefore(tstzspan, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(tstzspan, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(tstzspan, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(tstzspan, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tgeometry,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tgeometry,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tstzspan op tgeography */

CREATE FUNCTION stboxBefore(tstzspan, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(tstzspan, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(tstzspan, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(tstzspan, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tgeography,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tgeography,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tgeography,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tgeography,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Stbox
 *****************************************************************************/

CREATE FUNCTION stboxLeft(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverleft(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxRight(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverright(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBelow(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbelow(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAbove(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverabove(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxFront(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverfront(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBack(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverback(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBefore(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(stbox, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = stboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverleft,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = stboxRight,
  COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverright,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = stboxBelow,
  COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverbelow,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = stboxAbove,
  COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverabove,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <</ (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = stboxFront,
  COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &</ (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverfront,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR />> (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = stboxBack,
  COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR /&> (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverback,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION stboxBefore(stbox, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(stbox, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(stbox, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(stbox, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_stbox_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = stbox, RIGHTARG = tgeography,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = stbox, RIGHTARG = tgeography,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = stbox, RIGHTARG = tgeography,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = stbox, RIGHTARG = tgeography,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * tgeometry
 *****************************************************************************/

/* tgeometry op tstzspan */

CREATE FUNCTION stboxBefore(tgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(tgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(tgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(tgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeometry, RIGHTARG = tstzspan,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeometry, RIGHTARG = tstzspan,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeometry, RIGHTARG = tstzspan,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeometry, RIGHTARG = tstzspan,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tgeometry op stbox */

CREATE FUNCTION stboxLeft(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverleft(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxRight(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverright(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBelow(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbelow(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAbove(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverabove(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxFront(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverfront(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBack(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverback(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBefore(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(tgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = stboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = stboxOverleft,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = stboxRight,
  COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = stboxOverright,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = stboxBelow,
  COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = stboxOverbelow,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = stboxAbove,
  COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = stboxOverabove,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <</ (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = stboxFront,
  COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &</ (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = stboxOverfront,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR />> (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = stboxBack,
  COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR /&> (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = stboxOverback,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tgeometry op tgeometry */

CREATE FUNCTION stboxLeft(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverleft(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxRight(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverright(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBelow(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbelow(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAbove(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverabove(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxFront(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverfront(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBack(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverback(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxBefore(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = stboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverleft,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = stboxRight,
  COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverright,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = stboxBelow,
  COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverbelow,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = stboxAbove,
  COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverabove,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <</ (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = stboxFront,
  COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &</ (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverfront,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR />> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = stboxBack,
  COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR /&> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverback,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * tgeography
 *****************************************************************************/

/* tgeography op tstzspan */

CREATE FUNCTION stboxBefore(tgeography, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(tgeography, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(tgeography, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(tgeography, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeography, RIGHTARG = tstzspan,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeography, RIGHTARG = tstzspan,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeography, RIGHTARG = tstzspan,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeography, RIGHTARG = tstzspan,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tgeography op stbox */

CREATE FUNCTION stboxBefore(tgeography, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(tgeography, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(tgeography, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(tgeography, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tspatial_stbox'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeography, RIGHTARG = stbox,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeography, RIGHTARG = stbox,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeography, RIGHTARG = stbox,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeography, RIGHTARG = stbox,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* tgeography op tgeography */

CREATE FUNCTION stboxBefore(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverbefore(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxAfter(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxOverafter(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tspatial_tspatial'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tgeography, RIGHTARG = tgeography,
  PROCEDURE = stboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tgeography, RIGHTARG = tgeography,
  PROCEDURE = stboxOverbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tgeography, RIGHTARG = tgeography,
  PROCEDURE = stboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tgeography, RIGHTARG = tgeography,
  PROCEDURE = stboxOverafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/
