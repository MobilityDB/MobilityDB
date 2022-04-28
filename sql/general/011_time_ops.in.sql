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
 * time_ops.sql
 * Operators for time types.
 */

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION time_contains(timestampset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contains(timestampset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contains(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contains(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contains(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contains(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contains(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contains(periodset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contains(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contains(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = time_contains,
  LEFTARG = timestampset, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = time_contains,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE OPERATOR @> (
  PROCEDURE = time_contains,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = time_contains,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = time_contains,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = time_contains,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE OPERATOR @> (
  PROCEDURE = time_contains,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = time_contains,
  LEFTARG = periodset, RIGHTARG = timestampset,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = time_contains,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = time_contains,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE FUNCTION time_contained(timestamptz, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contained(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contained(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contained(timestampset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contained(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contained(timestampset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contained(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contained(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contained(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_contained(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = time_contained,
  LEFTARG = timestamptz, RIGHTARG = timestampset,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = time_contained,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = time_contained,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = time_contained,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = time_contained,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = time_contained,
  LEFTARG = timestampset, RIGHTARG = periodset,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = time_contained,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = time_contained,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = time_contained,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = time_contained,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE FUNCTION time_overlaps(timestampset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overlaps(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overlaps(timestampset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overlaps(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overlaps(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overlaps(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overlaps(periodset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overlaps(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overlaps(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = time_overlaps,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = time_overlaps,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = time_overlaps,
  LEFTARG = timestampset, RIGHTARG = periodset,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = time_overlaps,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = time_overlaps,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = time_overlaps,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = time_overlaps,
  LEFTARG = periodset, RIGHTARG = timestampset,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = time_overlaps,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = time_overlaps,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE FUNCTION time_before(timestamptz, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_before(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_before(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_before(timestampset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_before(timestampset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_before(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_before(timestampset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_before(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_before(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_before(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_before(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_before(periodset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_before(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_before(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_before(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  PROCEDURE = time_before,
  LEFTARG = timestamptz, RIGHTARG = timestampset,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = time_before,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = time_before,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = time_before,
  LEFTARG = timestampset, RIGHTARG = timestamptz,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = time_before,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = time_before,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = time_before,
  LEFTARG = timestampset, RIGHTARG = periodset,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = time_before,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = time_before,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = time_before,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = time_before,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = time_before,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = time_before,
  LEFTARG = periodset, RIGHTARG = timestampset,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = time_before,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = time_before,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE FUNCTION time_after(timestamptz, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_after(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_after(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_after(timestampset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_after(timestampset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_after(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_after(timestampset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_after(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_after(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_after(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_after(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_after(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_after(periodset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_after(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_after(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (
  PROCEDURE = time_after,
  LEFTARG = timestamptz, RIGHTARG = timestampset,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = time_after,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = time_after,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = time_after,
  LEFTARG = timestampset, RIGHTARG = timestamptz,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = time_after,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = time_after,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = time_after,
  LEFTARG = timestampset, RIGHTARG = periodset,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = time_after,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = time_after,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = time_after,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = time_after,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = time_after,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = time_after,
  LEFTARG = periodset, RIGHTARG = timestampset,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = time_after,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = time_after,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE FUNCTION time_overbefore(timestamptz, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overbefore(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overbefore(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overbefore(timestampset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overbefore(timestampset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overbefore(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overbefore(timestampset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overbefore(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overbefore(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overbefore(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overbefore(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overbefore(periodset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overbefore(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overbefore(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overbefore(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (
  PROCEDURE = time_overbefore,
  LEFTARG = timestamptz, RIGHTARG = timestampset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = time_overbefore,
  LEFTARG = timestamptz, RIGHTARG = period,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = time_overbefore,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = time_overbefore,
  LEFTARG = timestampset, RIGHTARG = timestamptz,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = time_overbefore,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = time_overbefore,
  LEFTARG = timestampset, RIGHTARG = period,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = time_overbefore,
  LEFTARG = timestampset, RIGHTARG = periodset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = time_overbefore,
  LEFTARG = period, RIGHTARG = timestamptz,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = time_overbefore,
  LEFTARG = period, RIGHTARG = timestampset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = time_overbefore,
  LEFTARG = period, RIGHTARG = period,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = time_overbefore,
  LEFTARG = period, RIGHTARG = periodset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = time_overbefore,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = time_overbefore,
  LEFTARG = periodset, RIGHTARG = timestampset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = time_overbefore,
  LEFTARG = periodset, RIGHTARG = period,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = time_overbefore,
  LEFTARG = periodset, RIGHTARG = periodset,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE FUNCTION time_overafter(timestamptz, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overafter(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overafter(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overafter(timestampset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overafter(timestampset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overafter(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overafter(timestampset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overafter(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overafter(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overafter(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overafter(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overafter(periodset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overafter(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overafter(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_overafter(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (
  PROCEDURE = time_overafter,
  LEFTARG = timestamptz, RIGHTARG = timestampset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = time_overafter,
  LEFTARG = timestamptz, RIGHTARG = period,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = time_overafter,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = time_overafter,
  LEFTARG = timestampset, RIGHTARG = timestamptz,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = time_overafter,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = time_overafter,
  LEFTARG = timestampset, RIGHTARG = period,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = time_overafter,
  LEFTARG = timestampset, RIGHTARG = periodset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = time_overafter,
  LEFTARG = period, RIGHTARG = timestamptz,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = time_overafter,
  LEFTARG = period, RIGHTARG = timestampset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = time_overafter,
  LEFTARG = period, RIGHTARG = period,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = time_overafter,
  LEFTARG = period, RIGHTARG = periodset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = time_overafter,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = time_overafter,
  LEFTARG = periodset, RIGHTARG = timestampset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = time_overafter,
  LEFTARG = periodset, RIGHTARG = period,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = time_overafter,
  LEFTARG = periodset, RIGHTARG = periodset,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE FUNCTION time_adjacent(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_adjacent(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_adjacent(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_adjacent(timestampset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_adjacent(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_adjacent(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_adjacent(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_adjacent(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION time_adjacent(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_adjacent(periodset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_adjacent(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_adjacent(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = time_adjacent,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = time_adjacent,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = time_adjacent,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = time_adjacent,
  LEFTARG = timestampset, RIGHTARG = periodset,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = time_adjacent,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = time_adjacent,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = time_adjacent,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = time_adjacent,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = time_adjacent,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = time_adjacent,
  LEFTARG = periodset, RIGHTARG = timestampset,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = time_adjacent,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = time_adjacent,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION time_union(timestamptz, timestamptz)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Union_timestamp_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_union(timestamptz, timestampset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Union_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_union(timestamptz, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_union(timestamptz, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = time_union,
  LEFTARG = timestamptz, RIGHTARG = timestamptz,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = time_union,
  LEFTARG = timestamptz, RIGHTARG = timestampset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = time_union,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = time_union,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = +
);

CREATE FUNCTION time_union(timestampset, timestamptz)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Union_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_union(timestampset, timestampset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Union_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_union(timestampset, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_union(timestampset, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = time_union,
  LEFTARG = timestampset, RIGHTARG = timestamptz,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = time_union,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = time_union,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = time_union,
  LEFTARG = timestampset, RIGHTARG = periodset,
  COMMUTATOR = +
);

CREATE FUNCTION time_union(period, timestamptz)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_union(period, timestampset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_union(period, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_union(period, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = time_union,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = time_union,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = time_union,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = time_union,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = +
);

CREATE FUNCTION time_union(periodset, timestamptz)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_union(periodset, timestampset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_union(periodset, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_union(periodset, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = time_union,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = time_union,
  LEFTARG = periodset, RIGHTARG = timestampset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = time_union,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = time_union,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = +
);

/*****************************************************************************/

CREATE FUNCTION time_minus(timestamptz, timestamptz)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Minus_timestamp_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_minus(timestamptz, timestampset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Minus_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_minus(timestamptz, period)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Minus_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_minus(timestamptz, periodset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Minus_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = time_minus,
  LEFTARG = timestamptz, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
  PROCEDURE = time_minus,
  LEFTARG = timestamptz, RIGHTARG = timestampset
);
CREATE OPERATOR - (
  PROCEDURE = time_minus,
  LEFTARG = timestamptz, RIGHTARG = period
);
CREATE OPERATOR - (
  PROCEDURE = time_minus,
  LEFTARG = timestamptz, RIGHTARG = periodset
);

CREATE FUNCTION time_minus(timestampset, timestamptz)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Minus_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_minus(timestampset, timestampset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Minus_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_minus(timestampset, period)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Minus_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_minus(timestampset, periodset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Minus_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = time_minus,
  LEFTARG = timestampset, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
  PROCEDURE = time_minus,
  LEFTARG = timestampset, RIGHTARG = timestampset
);
CREATE OPERATOR - (
  PROCEDURE = time_minus,
  LEFTARG = timestampset, RIGHTARG = period
);
CREATE OPERATOR - (
  PROCEDURE = time_minus,
  LEFTARG = timestampset, RIGHTARG = periodset
);

CREATE FUNCTION time_minus(period, timestamptz)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Minus_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_minus(period, timestampset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Minus_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_minus(period, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Minus_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_minus(period, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Minus_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = time_minus,
  LEFTARG = period, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
  PROCEDURE = time_minus,
  LEFTARG = period, RIGHTARG = timestampset
);
CREATE OPERATOR - (
  PROCEDURE = time_minus,
  LEFTARG = period, RIGHTARG = period
);
CREATE OPERATOR - (
  PROCEDURE = time_minus,
  LEFTARG = period, RIGHTARG = periodset
);

CREATE FUNCTION time_minus(periodset, timestamptz)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Minus_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_minus(periodset, timestampset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Minus_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_minus(periodset, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Minus_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_minus(periodset, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Minus_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = time_minus,
  LEFTARG = periodset, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
  PROCEDURE = time_minus,
  LEFTARG = periodset, RIGHTARG = timestampset
);
CREATE OPERATOR - (
  PROCEDURE = time_minus,
  LEFTARG = periodset, RIGHTARG = period
);
CREATE OPERATOR - (
  PROCEDURE = time_minus,
  LEFTARG = periodset, RIGHTARG = periodset
);

/*****************************************************************************/

CREATE FUNCTION time_intersection(timestamptz, timestamptz)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Intersection_timestamp_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_intersection(timestamptz, timestampset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Intersection_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_intersection(timestamptz, period)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Intersection_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_intersection(timestamptz, periodset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Intersection_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = time_intersection,
  LEFTARG = timestamptz, RIGHTARG = timestamptz,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = time_intersection,
  LEFTARG = timestamptz, RIGHTARG = timestampset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = time_intersection,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = time_intersection,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = *
);

CREATE FUNCTION time_intersection(timestampset, timestamptz)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Intersection_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_intersection(timestampset, timestampset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Intersection_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_intersection(timestampset, period)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Intersection_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_intersection(timestampset, periodset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Intersection_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = time_intersection,
  LEFTARG = timestampset, RIGHTARG = timestamptz,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = time_intersection,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = time_intersection,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = time_intersection,
  LEFTARG = timestampset, RIGHTARG = periodset,
  COMMUTATOR = *
);


CREATE FUNCTION time_intersection(period, timestamptz)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Intersection_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_intersection(period, timestampset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Intersection_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_intersection(period, period)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Intersection_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_intersection(period, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Intersection_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = time_intersection,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = time_intersection,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = time_intersection,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = time_intersection,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = *
);

CREATE FUNCTION time_intersection(periodset, timestamptz)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Intersection_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_intersection(periodset, timestampset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Intersection_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_intersection(periodset, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Intersection_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_intersection(periodset, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Intersection_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = time_intersection,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = time_intersection,
  LEFTARG = periodset, RIGHTARG = timestampset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = time_intersection,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = time_intersection,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = *
);

/*****************************************************************************/

CREATE FUNCTION time_distance(timestamptz, timestamptz)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_timestamp_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(timestamptz, timestampset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(timestamptz, period)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(timestamptz, periodset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = timestamptz, RIGHTARG = timestamptz,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = timestamptz, RIGHTARG = timestampset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = <->
);

CREATE FUNCTION time_distance(timestampset, timestamptz)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(timestampset, timestampset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(timestampset, period)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(timestampset, periodset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = timestampset, RIGHTARG = timestamptz,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = timestampset, RIGHTARG = periodset,
  COMMUTATOR = <->
);

CREATE FUNCTION time_distance(period, timestamptz)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(period, timestampset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(period, period)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(period, periodset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = <->
);

CREATE FUNCTION time_distance(periodset, timestamptz)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(periodset, timestampset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(periodset, period)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(periodset, periodset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = periodset, RIGHTARG = timestampset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = <->
);

/*****************************************************************************/
