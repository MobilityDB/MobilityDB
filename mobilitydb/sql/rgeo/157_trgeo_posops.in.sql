/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief Bounding box relative position operators for temporal rigid geomtries
 */

/*****************************************************************************
 * tstzspan
 *****************************************************************************/

/* tstzspan op trgeometry */

CREATE FUNCTION temporal_before(tstzspan, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspan, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspan, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspan, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = trgeometry,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = trgeometry,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = trgeometry,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = trgeometry,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * stbox
 *****************************************************************************/

CREATE FUNCTION temporal_left(stbox, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(stbox, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(stbox, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(stbox, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(stbox, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(stbox, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(stbox, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(stbox, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(stbox, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(stbox, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(stbox, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(stbox, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_stbox_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = stbox, RIGHTARG = trgeometry,
  PROCEDURE = temporal_left,
  COMMUTATOR = '>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = stbox, RIGHTARG = trgeometry,
  PROCEDURE = temporal_overleft,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = stbox, RIGHTARG = trgeometry,
  PROCEDURE = temporal_right,
  COMMUTATOR = '<<',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = stbox, RIGHTARG = trgeometry,
  PROCEDURE = temporal_overright,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = stbox, RIGHTARG = trgeometry,
  PROCEDURE = temporal_below,
  COMMUTATOR = '|>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = stbox, RIGHTARG = trgeometry,
  PROCEDURE = temporal_overbelow,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = stbox, RIGHTARG = trgeometry,
  PROCEDURE = temporal_above,
  COMMUTATOR = '<<|',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = stbox, RIGHTARG = trgeometry,
  PROCEDURE = temporal_overabove,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = stbox, RIGHTARG = trgeometry,
  PROCEDURE = temporal_before,
  COMMUTATOR = '#>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = stbox, RIGHTARG = trgeometry,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = stbox, RIGHTARG = trgeometry,
  PROCEDURE = temporal_after,
  COMMUTATOR = '<<#',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = stbox, RIGHTARG = trgeometry,
  PROCEDURE = temporal_overafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * trgeometry
 *****************************************************************************/

/* trgeometry op tstzspan */

CREATE FUNCTION temporal_before(trgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(trgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(trgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(trgeometry, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = trgeometry, RIGHTARG = tstzspan,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = trgeometry, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = trgeometry, RIGHTARG = tstzspan,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = trgeometry, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

/* trgeometry op stbox */

CREATE FUNCTION temporal_left(trgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(trgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(trgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(trgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(trgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(trgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(trgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(trgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(trgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(trgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(trgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(trgeometry, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tspatial_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = trgeometry, RIGHTARG = stbox,
  PROCEDURE = temporal_left,
  COMMUTATOR = '>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = trgeometry, RIGHTARG = stbox,
  PROCEDURE = temporal_overleft,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = trgeometry, RIGHTARG = stbox,
  PROCEDURE = temporal_right,
  COMMUTATOR = '<<',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = trgeometry, RIGHTARG = stbox,
  PROCEDURE = temporal_overright,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = trgeometry, RIGHTARG = stbox,
  PROCEDURE = temporal_below,
  COMMUTATOR = '|>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = trgeometry, RIGHTARG = stbox,
  PROCEDURE = temporal_overbelow,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = trgeometry, RIGHTARG = stbox,
  PROCEDURE = temporal_above,
  COMMUTATOR = '<<|',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = trgeometry, RIGHTARG = stbox,
  PROCEDURE = temporal_overabove,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = trgeometry, RIGHTARG = stbox,
  PROCEDURE = temporal_before,
  COMMUTATOR = '#>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = trgeometry, RIGHTARG = stbox,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = trgeometry, RIGHTARG = stbox,
  PROCEDURE = temporal_after,
  COMMUTATOR = '<<#',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = trgeometry, RIGHTARG = stbox,
  PROCEDURE = temporal_overafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

/* trgeometry op trgeometry */

CREATE FUNCTION temporal_left(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tspatial_tspatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = temporal_left,
  COMMUTATOR = '>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = temporal_overleft,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = temporal_right,
  COMMUTATOR = '<<',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = temporal_overright,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = temporal_below,
  COMMUTATOR = '|>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = temporal_overbelow,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = temporal_above,
  COMMUTATOR = '<<|',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = temporal_overabove,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<# (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = temporal_before,
  COMMUTATOR = '#>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = temporal_after,
  COMMUTATOR = '<<#',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = temporal_overafter,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/
