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

/*
 * temporal_posops.sql
 * Relative position operators for 1D (time) and 2D (1D value + 1D time)
 * temporal types.
 */

/*****************************************************************************
 * timestamptz
 *****************************************************************************/
/* timestamptz op tbool */

CREATE FUNCTION temporal_before(timestamptz, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = timestamptz, RIGHTARG = tbool,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = timestamptz, RIGHTARG = tbool,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = timestamptz, RIGHTARG = tbool,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = timestamptz, RIGHTARG = tbool,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* timestamptz op tint */

CREATE FUNCTION temporal_before(timestamptz, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = timestamptz, RIGHTARG = tint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = timestamptz, RIGHTARG = tint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = timestamptz, RIGHTARG = tint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = timestamptz, RIGHTARG = tint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* timestamptz op tfloat */

CREATE FUNCTION temporal_before(timestamptz, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = timestamptz, RIGHTARG = tfloat,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = timestamptz, RIGHTARG = tfloat,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = timestamptz, RIGHTARG = tfloat,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = timestamptz, RIGHTARG = tfloat,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* timestamptz op ttext */

CREATE FUNCTION temporal_before(timestamptz, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = timestamptz, RIGHTARG = ttext,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = timestamptz, RIGHTARG = ttext,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = timestamptz, RIGHTARG = ttext,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = timestamptz, RIGHTARG = ttext,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * tstzset
 *****************************************************************************/
/* tstzset op tbool */

CREATE FUNCTION temporal_before(tstzset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzset, RIGHTARG = tbool,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzset, RIGHTARG = tbool,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzset, RIGHTARG = tbool,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzset, RIGHTARG = tbool,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* tstzset op tint */

CREATE FUNCTION temporal_before(tstzset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzset, RIGHTARG = tint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzset, RIGHTARG = tint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzset, RIGHTARG = tint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzset, RIGHTARG = tint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tstzset op tfloat */

CREATE FUNCTION temporal_before(tstzset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzset, RIGHTARG = tfloat,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzset, RIGHTARG = tfloat,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzset, RIGHTARG = tfloat,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzset, RIGHTARG = tfloat,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tstzset op ttext */

CREATE FUNCTION temporal_before(tstzset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzset, RIGHTARG = ttext,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzset, RIGHTARG = ttext,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzset, RIGHTARG = ttext,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzset, RIGHTARG = ttext,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * tstzspan
 *****************************************************************************/
/* tstzspan op tbool */

CREATE FUNCTION temporal_before(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspan, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tbool,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tbool,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tbool,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tbool,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* tstzspan op tint */

CREATE FUNCTION temporal_before(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tstzspan op tfloat */

CREATE FUNCTION temporal_before(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = tfloat,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tstzspan op ttext */

CREATE FUNCTION temporal_before(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspan, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspan, RIGHTARG = ttext,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspan, RIGHTARG = ttext,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspan, RIGHTARG = ttext,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspan, RIGHTARG = ttext,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * tstzspanset
 *****************************************************************************/
/* tstzspanset op tbool */

CREATE FUNCTION temporal_before(tstzspanset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspanset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspanset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspanset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspanset, RIGHTARG = tbool,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspanset, RIGHTARG = tbool,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspanset, RIGHTARG = tbool,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspanset, RIGHTARG = tbool,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* tstzspanset op tint */

CREATE FUNCTION temporal_before(tstzspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspanset, RIGHTARG = tint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspanset, RIGHTARG = tint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspanset, RIGHTARG = tint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspanset, RIGHTARG = tint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tstzspanset op tfloat */

CREATE FUNCTION temporal_before(tstzspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspanset, RIGHTARG = tfloat,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspanset, RIGHTARG = tfloat,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspanset, RIGHTARG = tfloat,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspanset, RIGHTARG = tfloat,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tstzspanset op ttext */

CREATE FUNCTION temporal_before(tstzspanset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspanset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspanset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspanset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tstzspanset, RIGHTARG = ttext,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tstzspanset, RIGHTARG = ttext,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tstzspanset, RIGHTARG = ttext,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tstzspanset, RIGHTARG = ttext,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * int
 *****************************************************************************/
/* int op tint */

CREATE FUNCTION temporal_left(int, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(int, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(int, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(int, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = int, RIGHTARG = tint,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = int, RIGHTARG = tint,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = int, RIGHTARG = tint,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = int, RIGHTARG = tint,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* int op tfloat */

CREATE FUNCTION temporal_left(int, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(int, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(int, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(int, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = int, RIGHTARG = tfloat,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = int, RIGHTARG = tfloat,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = int, RIGHTARG = tfloat,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = int, RIGHTARG = tfloat,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * float
 *****************************************************************************/
/* float op tint */

CREATE FUNCTION temporal_left(float, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(float, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(float, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(float, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = float, RIGHTARG = tint,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = float, RIGHTARG = tint,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = float, RIGHTARG = tint,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = float, RIGHTARG = tint,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* float op tfloat */

CREATE FUNCTION temporal_left(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(float, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = float, RIGHTARG = tfloat,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * intspan
 *****************************************************************************/
/* intspan op tint */

CREATE FUNCTION temporal_left(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = intspan, RIGHTARG = tint,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = intspan, RIGHTARG = tint,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = intspan, RIGHTARG = tint,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = intspan, RIGHTARG = tint,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* intspan op tfloat */

CREATE FUNCTION temporal_left(intspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(intspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(intspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(intspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = intspan, RIGHTARG = tfloat,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = intspan, RIGHTARG = tfloat,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = intspan, RIGHTARG = tfloat,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = intspan, RIGHTARG = tfloat,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * floatspan
 *****************************************************************************/
/* floatspan op tint */

CREATE FUNCTION temporal_left(floatspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(floatspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(floatspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(floatspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = floatspan, RIGHTARG = tint,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = floatspan, RIGHTARG = tint,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = floatspan, RIGHTARG = tint,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = floatspan, RIGHTARG = tint,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* floatspan op tfloat */

CREATE FUNCTION temporal_left(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = floatspan, RIGHTARG = tfloat,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = floatspan, RIGHTARG = tfloat,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = floatspan, RIGHTARG = tfloat,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = floatspan, RIGHTARG = tfloat,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * intspanset
 *****************************************************************************/
/* intspanset op tint */

CREATE FUNCTION temporal_left(intspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(intspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(intspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(intspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = intspanset, RIGHTARG = tint,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = intspanset, RIGHTARG = tint,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = intspanset, RIGHTARG = tint,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = intspanset, RIGHTARG = tint,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* intspanset op tfloat */

CREATE FUNCTION temporal_left(intspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(intspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(intspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(intspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = intspanset, RIGHTARG = tfloat,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = intspanset, RIGHTARG = tfloat,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = intspanset, RIGHTARG = tfloat,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = intspanset, RIGHTARG = tfloat,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * floatspanset
 *****************************************************************************/
/* floatspanset op tint */

CREATE FUNCTION temporal_left(floatspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(floatspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(floatspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(floatspanset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = floatspanset, RIGHTARG = tint,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = floatspanset, RIGHTARG = tint,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = floatspanset, RIGHTARG = tint,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = floatspanset, RIGHTARG = tint,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* floatspanset op tfloat */

CREATE FUNCTION temporal_left(floatspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(floatspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(floatspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(floatspanset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspanset_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = floatspanset, RIGHTARG = tfloat,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = floatspanset, RIGHTARG = tfloat,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = floatspanset, RIGHTARG = tfloat,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = floatspanset, RIGHTARG = tfloat,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * tbox
 *****************************************************************************/
/* tbox op tint */

CREATE FUNCTION temporal_left(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbox, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tbox_tnumber'
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
  AS 'MODULE_PATHNAME', 'Left_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbox, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tbox_tnumber'
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
/* tbool op timestamptz */

CREATE FUNCTION temporal_before(tbool, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tbool, RIGHTARG = timestamptz,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbool, RIGHTARG = timestamptz,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbool, RIGHTARG = timestamptz,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbool, RIGHTARG = timestamptz,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* tbool op tstzset */

CREATE FUNCTION temporal_before(tbool, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tbool, RIGHTARG = tstzset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbool, RIGHTARG = tstzset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbool, RIGHTARG = tstzset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbool, RIGHTARG = tstzset,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

/* tbool op tstzspan */

CREATE FUNCTION temporal_before(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tbool, RIGHTARG = tstzspan,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbool, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbool, RIGHTARG = tstzspan,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbool, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* tbool op tstzspanset */

CREATE FUNCTION temporal_before(tbool, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tbool, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbool, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbool, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbool, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* tbool op tbool */

CREATE FUNCTION temporal_before(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_temporal'
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
/* tint op timestamptz */

CREATE FUNCTION temporal_before(tint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tint, RIGHTARG = timestamptz,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tint, RIGHTARG = timestamptz,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tint, RIGHTARG = timestamptz,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tint, RIGHTARG = timestamptz,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tint op tstzset */

CREATE FUNCTION temporal_before(tint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tint, RIGHTARG = tstzset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tint, RIGHTARG = tstzset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tint, RIGHTARG = tstzset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tint, RIGHTARG = tstzset,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tint op tstzspan */

CREATE FUNCTION temporal_before(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tint, RIGHTARG = tstzspan,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tint, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tint, RIGHTARG = tstzspan,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tint, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tint op tstzspanset */

CREATE FUNCTION temporal_before(tint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tint, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tint, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tint, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tint, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tint op int */

CREATE FUNCTION temporal_left(tint, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tint, RIGHTARG = int,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tint, RIGHTARG = int,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tint, RIGHTARG = int,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tint, RIGHTARG = int,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tint op float */

CREATE FUNCTION temporal_left(tint, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tint, RIGHTARG = float,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tint, RIGHTARG = float,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tint, RIGHTARG = float,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tint, RIGHTARG = float,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tint op intspan */

CREATE FUNCTION temporal_left(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tint, RIGHTARG = intspan,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tint, RIGHTARG = intspan,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tint, RIGHTARG = intspan,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tint, RIGHTARG = intspan,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tint op intspanset */

CREATE FUNCTION temporal_left(tint, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tint, RIGHTARG = intspanset,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tint, RIGHTARG = intspanset,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tint, RIGHTARG = intspanset,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tint, RIGHTARG = intspanset,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tint op tbox */

CREATE FUNCTION temporal_left(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tbox'
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
  AS 'MODULE_PATHNAME', 'Left_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tnumber'
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
  AS 'MODULE_PATHNAME', 'Left_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tnumber'
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
/* tfloat op timestamptz */

CREATE FUNCTION temporal_before(tfloat, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tfloat, RIGHTARG = timestamptz,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tfloat, RIGHTARG = timestamptz,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tfloat, RIGHTARG = timestamptz,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tfloat, RIGHTARG = timestamptz,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tfloat op tstzset */

CREATE FUNCTION temporal_before(tfloat, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tfloat, RIGHTARG = tstzset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tfloat, RIGHTARG = tstzset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tfloat, RIGHTARG = tstzset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tfloat, RIGHTARG = tstzset,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tfloat op tstzspan */

CREATE FUNCTION temporal_before(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tfloat, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tfloat op tstzspanset */

CREATE FUNCTION temporal_before(tfloat, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tfloat, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tfloat, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tfloat, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tfloat, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tfloat op int */

CREATE FUNCTION temporal_left(tfloat, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tfloat, RIGHTARG = int,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tfloat, RIGHTARG = int,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tfloat, RIGHTARG = int,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tfloat, RIGHTARG = int,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tfloat op float */

CREATE FUNCTION temporal_left(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tfloat, RIGHTARG = float,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tfloat op floatspan */

CREATE FUNCTION temporal_left(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tfloat, RIGHTARG = floatspan,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tfloat, RIGHTARG = floatspan,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tfloat, RIGHTARG = floatspan,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tfloat, RIGHTARG = floatspan,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tfloat op floatspanset */

CREATE FUNCTION temporal_left(tfloat, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_numspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tfloat, RIGHTARG = floatspanset,
  PROCEDURE = temporal_left,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tfloat, RIGHTARG = floatspanset,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tfloat, RIGHTARG = floatspanset,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tfloat, RIGHTARG = floatspanset,
  PROCEDURE = temporal_overright,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tfloat op tbox */

CREATE FUNCTION temporal_left(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tbox'
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
  AS 'MODULE_PATHNAME', 'Left_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tnumber'
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
  AS 'MODULE_PATHNAME', 'Left_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tnumber_tnumber'
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
/* ttext op timestamptz */

CREATE FUNCTION temporal_before(ttext, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = ttext, RIGHTARG = timestamptz,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = ttext, RIGHTARG = timestamptz,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = ttext, RIGHTARG = timestamptz,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = ttext, RIGHTARG = timestamptz,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* ttext op tstzset */

CREATE FUNCTION temporal_before(ttext, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = ttext, RIGHTARG = tstzset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = ttext, RIGHTARG = tstzset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = ttext, RIGHTARG = tstzset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = ttext, RIGHTARG = tstzset,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

/* ttext op tstzspan */

CREATE FUNCTION temporal_before(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = ttext, RIGHTARG = tstzspan,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = ttext, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = ttext, RIGHTARG = tstzspan,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = ttext, RIGHTARG = tstzspan,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* ttext op tstzspanset */

CREATE FUNCTION temporal_before(ttext, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = ttext, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = ttext, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = ttext, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = ttext, RIGHTARG = tstzspanset,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* ttext op ttext */

CREATE FUNCTION temporal_before(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_temporal'
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
