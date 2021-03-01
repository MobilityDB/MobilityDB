/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * temporal_posops.sql
 * Relative position operators for 1D (time) and 2D (1D value + 1D time)
 * temporal types.
 */

/*****************************************************************************
 * period
 *****************************************************************************/
/* period op tbool */

CREATE FUNCTION temporal_before(period, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = period, RIGHTARG = tbool,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = period, RIGHTARG = tbool,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = period, RIGHTARG = tbool,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = period, RIGHTARG = tbool,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* period op ttext */

CREATE FUNCTION temporal_before(period, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = period, RIGHTARG = ttext,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = period, RIGHTARG = ttext,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = period, RIGHTARG = ttext,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = period, RIGHTARG = ttext,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * intrange
 *****************************************************************************/
/* intrange op tint */

CREATE FUNCTION temporal_left(intrange, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'left_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(intrange, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overleft_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(intrange, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'right_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(intrange, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overright_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = intrange, RIGHTARG = tint,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = intrange, RIGHTARG = tint,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = intrange, RIGHTARG = tint,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = intrange, RIGHTARG = tint,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* intrange op tfloat */

CREATE FUNCTION temporal_left(intrange, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'left_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(intrange, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overleft_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(intrange, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'right_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(intrange, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overright_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = intrange, RIGHTARG = tfloat,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = intrange, RIGHTARG = tfloat,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = intrange, RIGHTARG = tfloat,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = intrange, RIGHTARG = tfloat,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * floatrange
 *****************************************************************************/
/* floatrange op tint */

CREATE FUNCTION temporal_left(floatrange, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'left_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(floatrange, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overleft_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(floatrange, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'right_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(floatrange, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overright_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = floatrange, RIGHTARG = tint,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = floatrange, RIGHTARG = tint,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = floatrange, RIGHTARG = tint,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = floatrange, RIGHTARG = tint,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* floatrange op tfloat */

CREATE FUNCTION temporal_left(floatrange, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'left_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(floatrange, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overleft_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(floatrange, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'right_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(floatrange, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overright_range_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = floatrange, RIGHTARG = tfloat,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = floatrange, RIGHTARG = tfloat,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = floatrange, RIGHTARG = tfloat,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = floatrange, RIGHTARG = tfloat,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * tbox
 *****************************************************************************/
/* tbox op tint */

CREATE FUNCTION temporal_left(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'left_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overleft_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'right_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overright_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbox, RIGHTARG = tint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tbox op tfloat */

CREATE FUNCTION temporal_left(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'left_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overleft_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'right_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overright_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbox, RIGHTARG = tfloat,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * tbool
 *****************************************************************************/
/* tbool op period */

CREATE FUNCTION temporal_before(tbool, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tbool, RIGHTARG = period,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbool, RIGHTARG = period,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbool, RIGHTARG = period,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbool, RIGHTARG = period,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* tbool op tbool */

CREATE FUNCTION temporal_before(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbool, RIGHTARG = tbool,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * tint
 *****************************************************************************/
/* tint op intrange */

CREATE FUNCTION temporal_left(tint, intrange)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'left_tnumber_range'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, intrange)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overleft_tnumber_range'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, intrange)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'right_tnumber_range'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, intrange)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overright_tnumber_range'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tint, RIGHTARG = intrange,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tint, RIGHTARG = intrange,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tint, RIGHTARG = intrange,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tint, RIGHTARG = intrange,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tint op tbox */

CREATE FUNCTION temporal_left(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'left_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overleft_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'right_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overright_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tint, RIGHTARG = tbox,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tint op tint */

CREATE FUNCTION temporal_left(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'left_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overleft_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'right_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overright_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tint, RIGHTARG = tint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tint op tfloat */

CREATE FUNCTION temporal_left(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'left_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overleft_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'right_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overright_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tint, RIGHTARG = tfloat,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tint, RIGHTARG = tfloat,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tint, RIGHTARG = tfloat,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tint, RIGHTARG = tfloat,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tint, RIGHTARG = tfloat,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tint, RIGHTARG = tfloat,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tint, RIGHTARG = tfloat,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tint, RIGHTARG = tfloat,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * tfloat
 *****************************************************************************/
/* tfloat op floatrange */

CREATE FUNCTION temporal_left(tfloat, floatrange)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'left_tnumber_range'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, floatrange)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overleft_tnumber_range'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, floatrange)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'right_tnumber_range'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, floatrange)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overright_tnumber_range'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tfloat, RIGHTARG = floatrange,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tfloat, RIGHTARG = floatrange,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tfloat, RIGHTARG = floatrange,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tfloat, RIGHTARG = floatrange,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tfloat op tbox */

CREATE FUNCTION temporal_left(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'left_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overleft_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'right_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overright_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tfloat, RIGHTARG = tbox,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tfloat op tint */

CREATE FUNCTION temporal_left(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'left_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overleft_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'right_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overright_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tfloat, RIGHTARG = tint,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tfloat, RIGHTARG = tint,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tfloat, RIGHTARG = tint,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tfloat, RIGHTARG = tint,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tfloat, RIGHTARG = tint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tfloat, RIGHTARG = tint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tfloat, RIGHTARG = tint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tfloat, RIGHTARG = tint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tfloat op tfloat */

CREATE FUNCTION temporal_left(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'left_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overleft_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'right_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overright_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tfloat, RIGHTARG = tfloat,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * ttext
 *****************************************************************************/

/* ttext op period */

CREATE FUNCTION temporal_before(ttext, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = ttext, RIGHTARG = period,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = ttext, RIGHTARG = period,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = ttext, RIGHTARG = period,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = ttext, RIGHTARG = period,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* ttext op ttext */

CREATE FUNCTION temporal_before(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = ttext, RIGHTARG = ttext,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
