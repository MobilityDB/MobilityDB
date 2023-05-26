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

/**
 * tnpoint_posops.sql
 * Relative position operators for temporal network points.
 */

/*****************************************************************************
 * tstzspan
 *****************************************************************************/

/* tstzspan op tnpoint */

CREATE FUNCTION temporal_before(tstzspan, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspan, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspan, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspan, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tnpoint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tnpoint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * stbox
 *****************************************************************************/

CREATE FUNCTION temporal_left(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(stbox, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = stbox, RIGHTARG = tnpoint,
  PROCEDURE = temporal_left,
  COMMUTATOR = '>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = stbox, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = stbox, RIGHTARG = tnpoint,
  PROCEDURE = temporal_right,
  COMMUTATOR = '<<',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = stbox, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overright,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = stbox, RIGHTARG = tnpoint,
  PROCEDURE = temporal_below,
  COMMUTATOR = '|>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = stbox, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overbelow,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = stbox, RIGHTARG = tnpoint,
  PROCEDURE = temporal_above,
  COMMUTATOR = '<<|',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = stbox, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overabove,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = stbox, RIGHTARG = tnpoint,
  PROCEDURE = temporal_before,
  COMMUTATOR = '#>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = stbox, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = stbox, RIGHTARG = tnpoint,
  PROCEDURE = temporal_after,
  COMMUTATOR = '<<#',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = stbox, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

/*****************************************************************************
 * tnpoint
 *****************************************************************************/

/* tnpoint op tstzspan */

CREATE FUNCTION temporal_before(tnpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tnpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tnpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tnpoint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tnpoint, RIGHTARG = tstzspan,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tnpoint, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tnpoint, RIGHTARG = tstzspan,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tnpoint, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

/* tnpoint op stbox */

CREATE FUNCTION temporal_left(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tnpoint, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tnpoint, RIGHTARG = stbox,
  PROCEDURE = temporal_left,
  COMMUTATOR = '>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tnpoint, RIGHTARG = stbox,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tnpoint, RIGHTARG = stbox,
  PROCEDURE = temporal_right,
  COMMUTATOR = '<<',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tnpoint, RIGHTARG = stbox,
  PROCEDURE = temporal_overright,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = tnpoint, RIGHTARG = stbox,
  PROCEDURE = temporal_below,
  COMMUTATOR = '|>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = tnpoint, RIGHTARG = stbox,
  PROCEDURE = temporal_overbelow,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = tnpoint, RIGHTARG = stbox,
  PROCEDURE = temporal_above,
  COMMUTATOR = '<<|',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = tnpoint, RIGHTARG = stbox,
  PROCEDURE = temporal_overabove,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tnpoint, RIGHTARG = stbox,
  PROCEDURE = temporal_before,
  COMMUTATOR = '#>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tnpoint, RIGHTARG = stbox,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tnpoint, RIGHTARG = stbox,
  PROCEDURE = temporal_after,
  COMMUTATOR = '<<#',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tnpoint, RIGHTARG = stbox,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

/*****************************************************************************/

/* tnpoint op tnpoint */

CREATE FUNCTION temporal_left(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_left,
  COMMUTATOR = '>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_right,
  COMMUTATOR = '<<',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overright,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_below,
  COMMUTATOR = '|>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overbelow,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_above,
  COMMUTATOR = '<<|',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overabove,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_before,
  COMMUTATOR = '#>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_after,
  COMMUTATOR = '<<#',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

/*****************************************************************************/
