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

/**
 * tnpoint_posops.sql
 * Relative position operators for temporal network points.
 */

/*****************************************************************************
 * timestamptz
 *****************************************************************************/
/* timestamptz op tnpoint */

CREATE FUNCTION temporal_before(timestamptz, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestamp_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = timestamptz, RIGHTARG = tnpoint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = timestamptz, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = timestamptz, RIGHTARG = tnpoint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = timestamptz, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * timestampset
 *****************************************************************************/
/* timestampset op tnpoint */

CREATE FUNCTION temporal_before(timestampset, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestampset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = timestampset, RIGHTARG = tnpoint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = timestampset, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = timestampset, RIGHTARG = tnpoint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = timestampset, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * period
 *****************************************************************************/
/* period op tnpoint */

CREATE FUNCTION temporal_before(period, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_period_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = period, RIGHTARG = tnpoint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = period, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = period, RIGHTARG = tnpoint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = period, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * periodset
 *****************************************************************************/
/* periodset op tnpoint */

CREATE FUNCTION temporal_before(periodset, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_periodset_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = periodset, RIGHTARG = tnpoint,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = periodset, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = periodset, RIGHTARG = tnpoint,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = periodset, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * Geometry
 *****************************************************************************/

CREATE FUNCTION temporal_left(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_geom_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_geom_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_geom_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_geom_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_geom_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_geom_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_geom_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_geom_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = geometry, RIGHTARG = tnpoint,
  PROCEDURE = temporal_left,
  COMMUTATOR = '>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = geometry, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = geometry, RIGHTARG = tnpoint,
  PROCEDURE = temporal_right,
  COMMUTATOR = '<<',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = geometry, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overright,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = geometry, RIGHTARG = tnpoint,
  PROCEDURE = temporal_below,
  COMMUTATOR = '|>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = geometry, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overbelow,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = geometry, RIGHTARG = tnpoint,
  PROCEDURE = temporal_above,
  COMMUTATOR = '<<|',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = geometry, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overabove,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
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
 * npoint
 *****************************************************************************/

CREATE FUNCTION temporal_left(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = npoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_left,
  COMMUTATOR = '>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = npoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = npoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_right,
  COMMUTATOR = '<<',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = npoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overright,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = npoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_below,
  COMMUTATOR = '|>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = npoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overbelow,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = npoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_above,
  COMMUTATOR = '<<|',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = npoint, RIGHTARG = tnpoint,
  PROCEDURE = temporal_overabove,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

/*****************************************************************************
 * tnpoint
 *****************************************************************************/
/* tnpoint op timestamptz */

CREATE FUNCTION temporal_before(tnpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tnpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tnpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tnpoint, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tnpoint, RIGHTARG = timestamptz,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tnpoint, RIGHTARG = timestamptz,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tnpoint, RIGHTARG = timestamptz,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tnpoint, RIGHTARG = timestamptz,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* tnpoint op timestampset */

CREATE FUNCTION temporal_before(tnpoint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tnpoint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tnpoint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tnpoint, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tnpoint, RIGHTARG = timestampset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tnpoint, RIGHTARG = timestampset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tnpoint, RIGHTARG = timestampset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tnpoint, RIGHTARG = timestampset,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* tnpoint op period */

CREATE FUNCTION temporal_before(tnpoint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tnpoint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tnpoint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tnpoint, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tnpoint, RIGHTARG = period,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tnpoint, RIGHTARG = period,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tnpoint, RIGHTARG = period,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tnpoint, RIGHTARG = period,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* tnpoint op periodset */

CREATE FUNCTION temporal_before(tnpoint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tnpoint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tnpoint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tnpoint, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  LEFTARG = tnpoint, RIGHTARG = periodset,
  PROCEDURE = temporal_before,
  COMMUTATOR = #>>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR &<# (
  LEFTARG = tnpoint, RIGHTARG = periodset,
  PROCEDURE = temporal_overbefore,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #>> (
  LEFTARG = tnpoint, RIGHTARG = periodset,
  PROCEDURE = temporal_after,
  COMMUTATOR = <<#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #&> (
  LEFTARG = tnpoint, RIGHTARG = periodset,
  PROCEDURE = temporal_overafter,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
/* tnpoint op geometry */

CREATE FUNCTION temporal_left(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tnpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tnpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tnpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tnpoint_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tnpoint, RIGHTARG = geometry,
  PROCEDURE = temporal_left,
  COMMUTATOR = '>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tnpoint, RIGHTARG = geometry,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tnpoint, RIGHTARG = geometry,
  PROCEDURE = temporal_right,
  COMMUTATOR = '<<',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tnpoint, RIGHTARG = geometry,
  PROCEDURE = temporal_overright,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = tnpoint, RIGHTARG = geometry,
  PROCEDURE = temporal_below,
  COMMUTATOR = '|>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = tnpoint, RIGHTARG = geometry,
  PROCEDURE = temporal_overbelow,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = tnpoint, RIGHTARG = geometry,
  PROCEDURE = temporal_above,
  COMMUTATOR = '<<|',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = tnpoint, RIGHTARG = geometry,
  PROCEDURE = temporal_overabove,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
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

/* tnpoint op npoint */

CREATE FUNCTION temporal_left(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  LEFTARG = tnpoint, RIGHTARG = npoint,
  PROCEDURE = temporal_left,
  COMMUTATOR = '>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &< (
  LEFTARG = tnpoint, RIGHTARG = npoint,
  PROCEDURE = temporal_overleft,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tnpoint, RIGHTARG = npoint,
  PROCEDURE = temporal_right,
  COMMUTATOR = '<<',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &> (
  LEFTARG = tnpoint, RIGHTARG = npoint,
  PROCEDURE = temporal_overright,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR <<| (
  LEFTARG = tnpoint, RIGHTARG = npoint,
  PROCEDURE = temporal_below,
  COMMUTATOR = '|>>',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR &<| (
  LEFTARG = tnpoint, RIGHTARG = npoint,
  PROCEDURE = temporal_overbelow,
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR |>> (
  LEFTARG = tnpoint, RIGHTARG = npoint,
  PROCEDURE = temporal_above,
  COMMUTATOR = '<<|',
  RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR |&> (
  LEFTARG = tnpoint, RIGHTARG = npoint,
  PROCEDURE = temporal_overabove,
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
