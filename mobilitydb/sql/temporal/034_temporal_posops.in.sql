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

CREATE FUNCTION before(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tbool,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tbool,
  PROCEDURE = overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tbool,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tbool,
  PROCEDURE = overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

/* tstzspan op tint */

CREATE FUNCTION before(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tint,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tint,
  PROCEDURE = overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tint,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tint,
  PROCEDURE = overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tstzspan op tbigint */

CREATE FUNCTION before(tstzspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tstzspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tstzspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tstzspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tbigint,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tbigint,
  PROCEDURE = overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tbigint,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tbigint,
  PROCEDURE = overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tstzspan op tfloat */

CREATE FUNCTION before(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  PROCEDURE = overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  PROCEDURE = overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tstzspan op ttext */

CREATE FUNCTION before(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = ttext,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = ttext,
  PROCEDURE = overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = ttext,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = ttext,
  PROCEDURE = overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * intspan
 *****************************************************************************/

/* intspan op tint */

CREATE FUNCTION left(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = intspan, RIGHTARG = tint,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = intspan, RIGHTARG = tint,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = intspan, RIGHTARG = tint,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = intspan, RIGHTARG = tint,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* intspan op tbigint */

CREATE FUNCTION left(intspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(intspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(intspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(intspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = intspan, RIGHTARG = tbigint,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = intspan, RIGHTARG = tbigint,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = intspan, RIGHTARG = tbigint,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = intspan, RIGHTARG = tbigint,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* intspan op tfloat */

CREATE FUNCTION left(intspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(intspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(intspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(intspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = intspan, RIGHTARG = tfloat,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = intspan, RIGHTARG = tfloat,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = intspan, RIGHTARG = tfloat,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = intspan, RIGHTARG = tfloat,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * bigintspan
 *****************************************************************************/

/* bigintspan op tint */

CREATE FUNCTION left(bigintspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(bigintspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(bigintspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(bigintspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = bigintspan, RIGHTARG = tint,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = bigintspan, RIGHTARG = tint,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = bigintspan, RIGHTARG = tint,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = bigintspan, RIGHTARG = tint,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/


/* bigintspan op tbigint */

CREATE FUNCTION left(bigintspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(bigintspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(bigintspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(bigintspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = bigintspan, RIGHTARG = tbigint,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = bigintspan, RIGHTARG = tbigint,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = bigintspan, RIGHTARG = tbigint,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = bigintspan, RIGHTARG = tbigint,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* bigintspan op tfloat */

CREATE FUNCTION left(bigintspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(bigintspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(bigintspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(bigintspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = bigintspan, RIGHTARG = tfloat,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = bigintspan, RIGHTARG = tfloat,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = bigintspan, RIGHTARG = tfloat,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = bigintspan, RIGHTARG = tfloat,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * floatspan
 *****************************************************************************/

/* floatspan op tint */

CREATE FUNCTION left(floatspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(floatspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(floatspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(floatspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = floatspan, RIGHTARG = tint,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = floatspan, RIGHTARG = tint,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = floatspan, RIGHTARG = tint,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = floatspan, RIGHTARG = tint,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* floatspan op tbigint */

CREATE FUNCTION left(floatspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(floatspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(floatspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(floatspan, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = floatspan, RIGHTARG = tbigint,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = floatspan, RIGHTARG = tbigint,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = floatspan, RIGHTARG = tbigint,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = floatspan, RIGHTARG = tbigint,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* floatspan op tfloat */

CREATE FUNCTION left(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = floatspan, RIGHTARG = tfloat,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = floatspan, RIGHTARG = tfloat,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = floatspan, RIGHTARG = tfloat,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = floatspan, RIGHTARG = tfloat,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * tbox
 *****************************************************************************/

/* tbox op tint */

CREATE FUNCTION left(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tbox op tbigint */

CREATE FUNCTION left(tbox, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tbox, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tbox, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tbox, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tbox, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tbox, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tbox, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tbox, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tbox, RIGHTARG = tbigint,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tbox, RIGHTARG = tbigint,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbox, RIGHTARG = tbigint,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tbox, RIGHTARG = tbigint,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tbox, RIGHTARG = tbigint,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbox, RIGHTARG = tbigint,
  PROCEDURE = overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbox, RIGHTARG = tbigint,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbox, RIGHTARG = tbigint,
  PROCEDURE = overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tbox op tfloat */

CREATE FUNCTION left(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * tbool
 *****************************************************************************/

/* tbool op tstzspan */

CREATE FUNCTION before(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tbool, RIGHTARG = tstzspan,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbool, RIGHTARG = tstzspan,
  PROCEDURE = overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbool, RIGHTARG = tstzspan,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbool, RIGHTARG = tstzspan,
  PROCEDURE = overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

/* tbool op tbool */

CREATE FUNCTION before(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * tint
 *****************************************************************************/

/* tint op tstzspan */

CREATE FUNCTION before(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tint, RIGHTARG = tstzspan,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tint, RIGHTARG = tstzspan,
  PROCEDURE = overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tint, RIGHTARG = tstzspan,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tint, RIGHTARG = tstzspan,
  PROCEDURE = overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tint op intspan */

CREATE FUNCTION left(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tint, RIGHTARG = intspan,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tint, RIGHTARG = intspan,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tint, RIGHTARG = intspan,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tint, RIGHTARG = intspan,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tint op tbox */

CREATE FUNCTION left(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tint op tint */

CREATE FUNCTION left(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * tbigint
 *****************************************************************************/

/* tbigint op tstzspan */

CREATE FUNCTION before(tbigint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tbigint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tbigint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tbigint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tbigint, RIGHTARG = tstzspan,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbigint, RIGHTARG = tstzspan,
  PROCEDURE = overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbigint, RIGHTARG = tstzspan,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbigint, RIGHTARG = tstzspan,
  PROCEDURE = overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tbigint op bigintspan */

CREATE FUNCTION left(tbigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tbigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tbigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tbigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tbigint, RIGHTARG = bigintspan,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tbigint, RIGHTARG = bigintspan,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbigint, RIGHTARG = bigintspan,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tbigint, RIGHTARG = bigintspan,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tbigint op tbox */

CREATE FUNCTION left(tbigint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tbigint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tbigint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tbigint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tbigint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tbigint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tbigint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tbigint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tbigint, RIGHTARG = tbox,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tbigint, RIGHTARG = tbox,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbigint, RIGHTARG = tbox,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tbigint, RIGHTARG = tbox,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tbigint, RIGHTARG = tbox,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbigint, RIGHTARG = tbox,
  PROCEDURE = overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbigint, RIGHTARG = tbox,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbigint, RIGHTARG = tbox,
  PROCEDURE = overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tbigint op tbigint */

CREATE FUNCTION left(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tbigint, tbigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbigint, RIGHTARG = tbigint,
  PROCEDURE = overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * tfloat
 *****************************************************************************/

/* tfloat op tstzspan */

CREATE FUNCTION before(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  PROCEDURE = overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  PROCEDURE = overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tfloat op floatspan */

CREATE FUNCTION left(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tfloat, RIGHTARG = floatspan,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tfloat, RIGHTARG = floatspan,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tfloat, RIGHTARG = floatspan,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tfloat, RIGHTARG = floatspan,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tfloat op tbox */

CREATE FUNCTION left(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tfloat op tfloat */

CREATE FUNCTION left(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * ttext
 *****************************************************************************/

/* ttext op tstzspan */

CREATE FUNCTION before(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = ttext, RIGHTARG = tstzspan,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = ttext, RIGHTARG = tstzspan,
  PROCEDURE = overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = ttext, RIGHTARG = tstzspan,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = ttext, RIGHTARG = tstzspan,
  PROCEDURE = overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

/* ttext op ttext */

CREATE FUNCTION before(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = overbefore,
  RESTRICT = temporal_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = overafter,
  RESTRICT = temporal_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
