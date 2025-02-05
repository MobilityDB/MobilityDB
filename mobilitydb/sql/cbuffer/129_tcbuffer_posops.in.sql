/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * tcbuffer_posops.sql
 * Relative position operators for temporal circular buffers.
 */

/*****************************************************************************
 * tstzspan
 *****************************************************************************/

/* tstzspan op tcbuffer */

CREATE FUNCTION temporal_before(tstzspan, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspan, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspan, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspan, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * stbox
 *****************************************************************************/

CREATE FUNCTION temporal_left(stbox, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_stbox_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(stbox, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_stbox_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(stbox, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_stbox_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(stbox, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_stbox_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(stbox, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_stbox_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(stbox, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_stbox_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(stbox, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_stbox_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(stbox, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_stbox_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(stbox, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_stbox_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(stbox, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_stbox_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(stbox, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_stbox_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(stbox, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_stbox_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = stbox, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_left,
  COMMUTATOR = '>>',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = stbox, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_overleft,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = stbox, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_right,
  COMMUTATOR = '<<',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = stbox, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_overright,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = stbox, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_below,
  COMMUTATOR = '|>>',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = stbox, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_overbelow,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = stbox, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_above,
  COMMUTATOR = '<<|',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = stbox, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_overabove,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = stbox, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_before,
  COMMUTATOR = '#>>',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = stbox, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = stbox, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_after,
  COMMUTATOR = '<<#',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = stbox, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_overafter,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);

/*****************************************************************************
 * tcbuffer
 *****************************************************************************/

/* tcbuffer op tstzspan */

CREATE FUNCTION temporal_before(tcbuffer, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tcbuffer, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tcbuffer, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tcbuffer, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tcbuffer, RIGHTARG = tstzspan,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tcbuffer, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tcbuffer, RIGHTARG = tstzspan,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tcbuffer, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

/* tcbuffer op stbox */

CREATE FUNCTION temporal_left(tcbuffer, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tcbuffer_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tcbuffer, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tcbuffer_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tcbuffer, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tcbuffer_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tcbuffer, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tcbuffer_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tcbuffer, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tcbuffer_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tcbuffer, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tcbuffer_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tcbuffer, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tcbuffer_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tcbuffer, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tcbuffer_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tcbuffer, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tcbuffer_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tcbuffer, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tcbuffer_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tcbuffer, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tcbuffer_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tcbuffer, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tcbuffer_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tcbuffer, RIGHTARG = stbox,
  PROCEDURE = temporal_left,
  COMMUTATOR = '>>',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tcbuffer, RIGHTARG = stbox,
  PROCEDURE = temporal_overleft,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tcbuffer, RIGHTARG = stbox,
  PROCEDURE = temporal_right,
  COMMUTATOR = '<<',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tcbuffer, RIGHTARG = stbox,
  PROCEDURE = temporal_overright,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = tcbuffer, RIGHTARG = stbox,
  PROCEDURE = temporal_below,
  COMMUTATOR = '|>>',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = tcbuffer, RIGHTARG = stbox,
  PROCEDURE = temporal_overbelow,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = tcbuffer, RIGHTARG = stbox,
  PROCEDURE = temporal_above,
  COMMUTATOR = '<<|',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = tcbuffer, RIGHTARG = stbox,
  PROCEDURE = temporal_overabove,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tcbuffer, RIGHTARG = stbox,
  PROCEDURE = temporal_before,
  COMMUTATOR = '#>>',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tcbuffer, RIGHTARG = stbox,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tcbuffer, RIGHTARG = stbox,
  PROCEDURE = temporal_after,
  COMMUTATOR = '<<#',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tcbuffer, RIGHTARG = stbox,
  PROCEDURE = temporal_overafter,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);

/*****************************************************************************/

/* tcbuffer op tcbuffer */

CREATE FUNCTION temporal_left(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_left,
  COMMUTATOR = '>>',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_overleft,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_right,
  COMMUTATOR = '<<',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_overright,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_below,
  COMMUTATOR = '|>>',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_overbelow,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_above,
  COMMUTATOR = '<<|',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_overabove,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_before,
  COMMUTATOR = '#>>',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_after,
  COMMUTATOR = '<<#',
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  PROCEDURE = temporal_overafter,
  RESTRICT = tcbuffer_sel, JOIN = tcbuffer_joinsel
);

/*****************************************************************************/
