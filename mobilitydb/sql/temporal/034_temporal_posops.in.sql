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
 * @brief Relative position operators for 1D (time) and 2D (1D value + 1D time)
 * temporal types
 */

/*****************************************************************************
 * tstzspan
 *****************************************************************************/

/* tstzspan op tbool */

CREATE FUNCTION spanBefore(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverbefore(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAfter(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverafter(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tbool,
  PROCEDURE = spanBefore,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tbool,
  PROCEDURE = spanOverbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tbool,
  PROCEDURE = spanAfter,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tbool,
  PROCEDURE = spanOverafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

/* tstzspan op tint */

CREATE FUNCTION tboxBefore(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tint,
  PROCEDURE = tboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tint,
  PROCEDURE = tboxOverbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tint,
  PROCEDURE = tboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tint,
  PROCEDURE = tboxOverafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tstzspan op tbigint */

CREATE FUNCTION tboxBefore(tstzspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tstzspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tstzspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tstzspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tbigint,
  PROCEDURE = tboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tbigint,
  PROCEDURE = tboxOverbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tbigint,
  PROCEDURE = tboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tbigint,
  PROCEDURE = tboxOverafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tstzspan op tfloat */

CREATE FUNCTION tboxBefore(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  PROCEDURE = tboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  PROCEDURE = tboxOverbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  PROCEDURE = tboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  PROCEDURE = tboxOverafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tstzspan op ttext */

CREATE FUNCTION spanBefore(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverbefore(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAfter(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverafter(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = ttext,
  PROCEDURE = spanBefore,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = ttext,
  PROCEDURE = spanOverbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = ttext,
  PROCEDURE = spanAfter,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = ttext,
  PROCEDURE = spanOverafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * intspan
 *****************************************************************************/

/* intspan op tint */

CREATE FUNCTION tboxLeft(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = intspan, RIGHTARG = tint,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = intspan, RIGHTARG = tint,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = intspan, RIGHTARG = tint,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = intspan, RIGHTARG = tint,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* intspan op tbigint */

CREATE FUNCTION tboxLeft(intspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(intspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(intspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(intspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = intspan, RIGHTARG = tbigint,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = intspan, RIGHTARG = tbigint,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = intspan, RIGHTARG = tbigint,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = intspan, RIGHTARG = tbigint,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* intspan op tfloat */

CREATE FUNCTION tboxLeft(intspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(intspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(intspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(intspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = intspan, RIGHTARG = tfloat,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = intspan, RIGHTARG = tfloat,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = intspan, RIGHTARG = tfloat,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = intspan, RIGHTARG = tfloat,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * bigintspan
 *****************************************************************************/

/* bigintspan op tint */

CREATE FUNCTION tboxLeft(bigintspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(bigintspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(bigintspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(bigintspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = bigintspan, RIGHTARG = tint,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = bigintspan, RIGHTARG = tint,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = bigintspan, RIGHTARG = tint,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = bigintspan, RIGHTARG = tint,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/


/* bigintspan op tbigint */

CREATE FUNCTION tboxLeft(bigintspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(bigintspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(bigintspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(bigintspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = bigintspan, RIGHTARG = tbigint,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = bigintspan, RIGHTARG = tbigint,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = bigintspan, RIGHTARG = tbigint,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = bigintspan, RIGHTARG = tbigint,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* bigintspan op tfloat */

CREATE FUNCTION tboxLeft(bigintspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(bigintspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(bigintspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(bigintspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = bigintspan, RIGHTARG = tfloat,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = bigintspan, RIGHTARG = tfloat,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = bigintspan, RIGHTARG = tfloat,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = bigintspan, RIGHTARG = tfloat,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * floatspan
 *****************************************************************************/

/* floatspan op tint */

CREATE FUNCTION tboxLeft(floatspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(floatspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(floatspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(floatspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = floatspan, RIGHTARG = tint,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = floatspan, RIGHTARG = tint,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = floatspan, RIGHTARG = tint,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = floatspan, RIGHTARG = tint,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* floatspan op tbigint */

CREATE FUNCTION tboxLeft(floatspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(floatspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(floatspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(floatspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = floatspan, RIGHTARG = tbigint,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = floatspan, RIGHTARG = tbigint,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = floatspan, RIGHTARG = tbigint,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = floatspan, RIGHTARG = tbigint,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* floatspan op tfloat */

CREATE FUNCTION tboxLeft(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = floatspan, RIGHTARG = tfloat,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = floatspan, RIGHTARG = tfloat,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = floatspan, RIGHTARG = tfloat,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = floatspan, RIGHTARG = tfloat,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * tbox
 *****************************************************************************/

/* tbox op tint */

CREATE FUNCTION tboxLeft(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxBefore(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = tboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = tboxOverbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = tboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = tboxOverafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tbox op tbigint */

CREATE FUNCTION tboxLeft(tbox, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(tbox, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(tbox, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(tbox, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxBefore(tbox, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tbox, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tbox, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tbox, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tbox, RIGHTARG = tbigint,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tbox, RIGHTARG = tbigint,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbox, RIGHTARG = tbigint,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tbox, RIGHTARG = tbigint,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tbox, RIGHTARG = tbigint,
  PROCEDURE = tboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbox, RIGHTARG = tbigint,
  PROCEDURE = tboxOverbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbox, RIGHTARG = tbigint,
  PROCEDURE = tboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbox, RIGHTARG = tbigint,
  PROCEDURE = tboxOverafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tbox op tfloat */

CREATE FUNCTION tboxLeft(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxBefore(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tbox_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = tboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = tboxOverbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = tboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = tboxOverafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * tbool
 *****************************************************************************/

/* tbool op tstzspan */

CREATE FUNCTION spanBefore(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverbefore(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAfter(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverafter(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tbool, RIGHTARG = tstzspan,
  PROCEDURE = spanBefore,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbool, RIGHTARG = tstzspan,
  PROCEDURE = spanOverbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbool, RIGHTARG = tstzspan,
  PROCEDURE = spanAfter,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbool, RIGHTARG = tstzspan,
  PROCEDURE = spanOverafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

/* tbool op tbool */

CREATE FUNCTION spanBefore(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverbefore(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAfter(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverafter(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = spanBefore,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = spanOverbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = spanAfter,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = spanOverafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * tint
 *****************************************************************************/

/* tint op tstzspan */

CREATE FUNCTION tboxBefore(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tint, RIGHTARG = tstzspan,
  PROCEDURE = tboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tint, RIGHTARG = tstzspan,
  PROCEDURE = tboxOverbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tint, RIGHTARG = tstzspan,
  PROCEDURE = tboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tint, RIGHTARG = tstzspan,
  PROCEDURE = tboxOverafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tint op intspan */

CREATE FUNCTION tboxLeft(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_numspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_numspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_numspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_numspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tint, RIGHTARG = intspan,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tint, RIGHTARG = intspan,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tint, RIGHTARG = intspan,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tint, RIGHTARG = intspan,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tint op tbox */

CREATE FUNCTION tboxLeft(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxBefore(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = tboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = tboxOverbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = tboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = tboxOverafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tint op tint */

CREATE FUNCTION tboxLeft(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxBefore(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = tboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = tboxOverbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = tboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = tboxOverafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * tbigint
 *****************************************************************************/

/* tbigint op tstzspan */

CREATE FUNCTION tboxBefore(tbigint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tbigint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tbigint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tbigint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tbigint, RIGHTARG = tstzspan,
  PROCEDURE = tboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbigint, RIGHTARG = tstzspan,
  PROCEDURE = tboxOverbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbigint, RIGHTARG = tstzspan,
  PROCEDURE = tboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbigint, RIGHTARG = tstzspan,
  PROCEDURE = tboxOverafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tbigint op bigintspan */

CREATE FUNCTION tboxLeft(tbigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_numspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(tbigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_numspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(tbigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_numspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(tbigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_numspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tbigint, RIGHTARG = bigintspan,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tbigint, RIGHTARG = bigintspan,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbigint, RIGHTARG = bigintspan,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tbigint, RIGHTARG = bigintspan,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tbigint op tbox */

CREATE FUNCTION tboxLeft(tbigint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(tbigint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(tbigint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(tbigint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxBefore(tbigint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tbigint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tbigint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tbigint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tbigint, RIGHTARG = tbox,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tbigint, RIGHTARG = tbox,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbigint, RIGHTARG = tbox,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tbigint, RIGHTARG = tbox,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tbigint, RIGHTARG = tbox,
  PROCEDURE = tboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbigint, RIGHTARG = tbox,
  PROCEDURE = tboxOverbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbigint, RIGHTARG = tbox,
  PROCEDURE = tboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbigint, RIGHTARG = tbox,
  PROCEDURE = tboxOverafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tbigint op tbigint */

CREATE FUNCTION tboxLeft(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxBefore(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = tboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = tboxOverbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = tboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = tboxOverafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * tfloat
 *****************************************************************************/

/* tfloat op tstzspan */

CREATE FUNCTION tboxBefore(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  PROCEDURE = tboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  PROCEDURE = tboxOverbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  PROCEDURE = tboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  PROCEDURE = tboxOverafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tfloat op floatspan */

CREATE FUNCTION tboxLeft(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_numspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_numspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_numspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_numspan'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tfloat, RIGHTARG = floatspan,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tfloat, RIGHTARG = floatspan,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tfloat, RIGHTARG = floatspan,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tfloat, RIGHTARG = floatspan,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tfloat op tbox */

CREATE FUNCTION tboxLeft(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxBefore(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tbox'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = tboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = tboxOverbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = tboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = tboxOverafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tfloat op tfloat */

CREATE FUNCTION tboxLeft(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxBefore(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tnumber'
  SUPPORT tnumber_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = tboxLeft,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = tboxOverleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = tboxOverright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = tboxBefore,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = tboxOverbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = tboxAfter,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = tboxOverafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * ttext
 *****************************************************************************/

/* ttext op tstzspan */

CREATE FUNCTION spanBefore(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverbefore(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAfter(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverafter(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = ttext, RIGHTARG = tstzspan,
  PROCEDURE = spanBefore,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = ttext, RIGHTARG = tstzspan,
  PROCEDURE = spanOverbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = ttext, RIGHTARG = tstzspan,
  PROCEDURE = spanAfter,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = ttext, RIGHTARG = tstzspan,
  PROCEDURE = spanOverafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

/* ttext op ttext */

CREATE FUNCTION spanBefore(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverbefore(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAfter(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverafter(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = spanBefore,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = spanOverbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = spanAfter,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = spanOverafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
