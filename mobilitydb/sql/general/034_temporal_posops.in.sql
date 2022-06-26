/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * timestampset
 *****************************************************************************/
/* timestampset op tbool */

CREATE FUNCTION temporal_before(timestampset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = timestampset, RIGHTARG = tbool,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = timestampset, RIGHTARG = tbool,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = timestampset, RIGHTARG = tbool,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = timestampset, RIGHTARG = tbool,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* timestampset op tint */

CREATE FUNCTION temporal_before(timestampset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = timestampset, RIGHTARG = tint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = timestampset, RIGHTARG = tint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = timestampset, RIGHTARG = tint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = timestampset, RIGHTARG = tint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* timestampset op tfloat */

CREATE FUNCTION temporal_before(timestampset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = timestampset, RIGHTARG = tfloat,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = timestampset, RIGHTARG = tfloat,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = timestampset, RIGHTARG = tfloat,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = timestampset, RIGHTARG = tfloat,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* timestampset op ttext */

CREATE FUNCTION temporal_before(timestampset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = timestampset, RIGHTARG = ttext,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = timestampset, RIGHTARG = ttext,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = timestampset, RIGHTARG = ttext,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = timestampset, RIGHTARG = ttext,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * period
 *****************************************************************************/
/* period op tbool */

CREATE FUNCTION temporal_before(period, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_period_temporal'
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
/* period op tint */

CREATE FUNCTION temporal_before(period, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = period, RIGHTARG = tint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = period, RIGHTARG = tint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = period, RIGHTARG = tint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = period, RIGHTARG = tint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* period op tfloat */

CREATE FUNCTION temporal_before(period, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = period, RIGHTARG = tfloat,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = period, RIGHTARG = tfloat,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = period, RIGHTARG = tfloat,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = period, RIGHTARG = tfloat,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* period op ttext */

CREATE FUNCTION temporal_before(period, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_period_temporal'
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
 * periodset
 *****************************************************************************/
/* periodset op tbool */

CREATE FUNCTION temporal_before(periodset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, tbool)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = periodset, RIGHTARG = tbool,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = periodset, RIGHTARG = tbool,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = periodset, RIGHTARG = tbool,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = periodset, RIGHTARG = tbool,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* periodset op tint */

CREATE FUNCTION temporal_before(periodset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = periodset, RIGHTARG = tint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = periodset, RIGHTARG = tint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = periodset, RIGHTARG = tint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = periodset, RIGHTARG = tint,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* periodset op tfloat */

CREATE FUNCTION temporal_before(periodset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = periodset, RIGHTARG = tfloat,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = periodset, RIGHTARG = tfloat,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = periodset, RIGHTARG = tfloat,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = periodset, RIGHTARG = tfloat,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* periodset op ttext */

CREATE FUNCTION temporal_before(periodset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, ttext)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = periodset, RIGHTARG = ttext,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = periodset, RIGHTARG = ttext,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = periodset, RIGHTARG = ttext,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = periodset, RIGHTARG = ttext,
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
  AS 'MODULE_PATHNAME', 'Left_span_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(intspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_tnumber'
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
  AS 'MODULE_PATHNAME', 'Left_span_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(intspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(intspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(intspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_tnumber'
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
  AS 'MODULE_PATHNAME', 'Left_span_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(floatspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(floatspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(floatspan, tint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_tnumber'
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
  AS 'MODULE_PATHNAME', 'Left_span_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(floatspan, tfloat)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_tnumber'
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
/* tbool op timestampset */

CREATE FUNCTION temporal_before(tbool, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tbool, RIGHTARG = timestampset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbool, RIGHTARG = timestampset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbool, RIGHTARG = timestampset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbool, RIGHTARG = timestampset,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

/* tbool op period */

CREATE FUNCTION temporal_before(tbool, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_period'
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
/* tbool op periodset */

CREATE FUNCTION temporal_before(tbool, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tbool, RIGHTARG = periodset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tbool, RIGHTARG = periodset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tbool, RIGHTARG = periodset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tbool, RIGHTARG = periodset,
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
/* tint op timestampset */

CREATE FUNCTION temporal_before(tint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tint, RIGHTARG = timestampset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tint, RIGHTARG = timestampset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tint, RIGHTARG = timestampset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tint, RIGHTARG = timestampset,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tint op period */

CREATE FUNCTION temporal_before(tint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tint, RIGHTARG = period,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tint, RIGHTARG = period,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tint, RIGHTARG = period,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tint, RIGHTARG = period,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tint op periodset */

CREATE FUNCTION temporal_before(tint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tint, RIGHTARG = periodset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tint, RIGHTARG = periodset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tint, RIGHTARG = periodset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tint, RIGHTARG = periodset,
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
  AS 'MODULE_PATHNAME', 'Left_tnumber_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_span'
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
/* tfloat op timestampset */

CREATE FUNCTION temporal_before(tfloat, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tfloat, RIGHTARG = timestampset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tfloat, RIGHTARG = timestampset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tfloat, RIGHTARG = timestampset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tfloat, RIGHTARG = timestampset,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/

/* tfloat op period */

CREATE FUNCTION temporal_before(tfloat, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tfloat, RIGHTARG = period,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tfloat, RIGHTARG = period,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tfloat, RIGHTARG = period,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tfloat, RIGHTARG = period,
  PROCEDURE = temporal_overafter,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************/
/* tfloat op periodset */

CREATE FUNCTION temporal_before(tfloat, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tfloat, RIGHTARG = periodset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tfloat, RIGHTARG = periodset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tfloat, RIGHTARG = periodset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tfloat, RIGHTARG = periodset,
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
  AS 'MODULE_PATHNAME', 'Left_tnumber_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnumber_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnumber_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnumber_span'
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
/* ttext op timestampset */

CREATE FUNCTION temporal_before(ttext, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = ttext, RIGHTARG = timestampset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = ttext, RIGHTARG = timestampset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = ttext, RIGHTARG = timestampset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = ttext, RIGHTARG = timestampset,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/

/* ttext op period */

CREATE FUNCTION temporal_before(ttext, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = ttext, RIGHTARG = period,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = ttext, RIGHTARG = period,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = ttext, RIGHTARG = period,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = ttext, RIGHTARG = period,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* ttext op periodset */

CREATE FUNCTION temporal_before(ttext, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = ttext, RIGHTARG = periodset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = ttext, RIGHTARG = periodset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = ttext, RIGHTARG = periodset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = ttext, RIGHTARG = periodset,
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
