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
 * @brief Relative position operators pour tjsonb
 */

-----------------------------------------------------------------------------

/* tstzspan op tjsonb */

CREATE FUNCTION temporal_before(tstzspan, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspan, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspan, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspan, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG    = tstzspan, RIGHTARG = tjsonb,
  PROCEDURE  = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT   = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG   = tstzspan, RIGHTARG = tjsonb,
  PROCEDURE = temporal_overbefore,
  RESTRICT  = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG    = tstzspan, RIGHTARG = tjsonb,
  PROCEDURE  = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT   = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG   = tstzspan, RIGHTARG = tjsonb,
  PROCEDURE = temporal_overafter,
  RESTRICT  = temporal_sel, JOIN = temporal_joinsel
);

/* tjsonb op tstzspan */

CREATE FUNCTION temporal_before(tjsonb, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tjsonb, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tjsonb, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tjsonb, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG    = tjsonb, RIGHTARG = tstzspan,
  PROCEDURE  = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT   = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG   = tjsonb, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overbefore,
  RESTRICT  = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG    = tjsonb, RIGHTARG = tstzspan,
  PROCEDURE  = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT   = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG   = tjsonb, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overafter,
  RESTRICT  = temporal_sel, JOIN = temporal_joinsel
);

/* tjsonb op tjsonb */

CREATE FUNCTION temporal_before(tjsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tjsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tjsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tjsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG    = tjsonb, RIGHTARG = tjsonb,
  PROCEDURE  = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT   = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG   = tjsonb, RIGHTARG = tjsonb,
  PROCEDURE = temporal_overbefore,
  RESTRICT  = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG    = tjsonb, RIGHTARG = tjsonb,
  PROCEDURE  = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT   = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG   = tjsonb, RIGHTARG = tjsonb,
  PROCEDURE = temporal_overafter,
  RESTRICT  = temporal_sel, JOIN = temporal_joinsel
);
