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
 * timeops.sql
 * Operators for time types.
 */

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION temporal_contains(timestampset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(timestampset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(periodset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contains(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = timestampset, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  COMMUTATOR = <@,
  RESTRICT = periodsel, JOIN = contjoinsel
);

CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = <@,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = <@,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = <@,
  RESTRICT = periodsel, JOIN = contjoinsel
);

CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = periodset, RIGHTARG = timestampset,
  COMMUTATOR = <@,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = <@,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR @> (
  PROCEDURE = temporal_contains,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = <@,
  RESTRICT = periodsel, JOIN = contjoinsel
);

CREATE FUNCTION temporal_contained(timestamptz, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(timestampset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(timestampset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_contained(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = timestamptz, RIGHTARG = timestampset,
  COMMUTATOR = @>,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = @>,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = @>,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  COMMUTATOR = @>,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = @>,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = timestampset, RIGHTARG = periodset,
  COMMUTATOR = @>,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = @>,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = @>,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = @>,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = temporal_contained,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = @>,
  RESTRICT = periodsel, JOIN = contjoinsel
);

CREATE FUNCTION temporal_overlaps(timestampset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(timestampset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(periodset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  COMMUTATOR = &&,
  RESTRICT = periodsel, JOIN = areajoinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = &&,
  RESTRICT = periodsel, JOIN = areajoinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = timestampset, RIGHTARG = periodset,
  COMMUTATOR = &&,
  RESTRICT = periodsel, JOIN = areajoinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = &&,
  RESTRICT = periodsel, JOIN = areajoinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = &&,
  RESTRICT = periodsel, JOIN = areajoinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = &&,
  RESTRICT = periodsel, JOIN = areajoinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = periodset, RIGHTARG = timestampset,
  COMMUTATOR = &&,
  RESTRICT = periodsel, JOIN = areajoinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = &&,
  RESTRICT = periodsel, JOIN = areajoinsel
);
CREATE OPERATOR && (
  PROCEDURE = temporal_overlaps,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = &&,
  RESTRICT = periodsel, JOIN = areajoinsel
);

CREATE FUNCTION temporal_before(timestamptz, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(timestampset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(timestampset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(timestampset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(periodset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = timestamptz, RIGHTARG = timestampset,
  COMMUTATOR = #>>,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = #>>,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = #>>,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = timestampset, RIGHTARG = timestamptz,
  COMMUTATOR = #>>,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  COMMUTATOR = #>>,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = #>>,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = timestampset, RIGHTARG = periodset,
  COMMUTATOR = #>>,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = #>>,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = #>>,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = #>>,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = #>>,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = #>>,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = periodset, RIGHTARG = timestampset,
  COMMUTATOR = #>>,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = #>>,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = #>>,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION temporal_after(timestamptz, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = timestamptz, RIGHTARG = timestampset,
  COMMUTATOR = <<#,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = <<#,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = <<#,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = timestampset, RIGHTARG = timestamptz,
  COMMUTATOR = <<#,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  COMMUTATOR = <<#,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = <<#,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = timestampset, RIGHTARG = periodset,
  COMMUTATOR = <<#,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = <<#,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = <<#,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = <<#,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = <<#,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = <<#,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = periodset, RIGHTARG = timestampset,
  COMMUTATOR = <<#,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = <<#,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = <<#,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);

CREATE FUNCTION temporal_overbefore(timestamptz, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = timestamptz, RIGHTARG = timestampset,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = timestamptz, RIGHTARG = period,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = timestampset, RIGHTARG = timestamptz,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = timestampset, RIGHTARG = period,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = timestampset, RIGHTARG = periodset,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = period, RIGHTARG = timestamptz,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = period, RIGHTARG = timestampset,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = period, RIGHTARG = period,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = period, RIGHTARG = periodset,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = periodset, RIGHTARG = timestampset,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = periodset, RIGHTARG = period,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = periodset, RIGHTARG = periodset,
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION temporal_overafter(timestamptz, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = timestamptz, RIGHTARG = timestampset,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = timestamptz, RIGHTARG = period,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = timestampset, RIGHTARG = timestamptz,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = timestampset, RIGHTARG = period,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = timestampset, RIGHTARG = periodset,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = period, RIGHTARG = timestamptz,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = period, RIGHTARG = timestampset,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = period, RIGHTARG = period,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = period, RIGHTARG = periodset,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = periodset, RIGHTARG = timestampset,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = periodset, RIGHTARG = period,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = periodset, RIGHTARG = periodset,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
);

CREATE FUNCTION temporal_adjacent(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(timestampset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_adjacent(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(periodset, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = -|-,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = -|-,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = -|-,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = timestampset, RIGHTARG = periodset,
  COMMUTATOR = -|-,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = -|-,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = -|-,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = -|-,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = -|-,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = -|-,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = periodset, RIGHTARG = timestampset,
  COMMUTATOR = -|-,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = -|-,
  RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = temporal_adjacent,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = -|-,
  RESTRICT = periodsel, JOIN = contjoinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_union(timestamptz, timestamptz)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'union_timestamp_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_union(timestamptz, timestampset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'union_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_union(timestamptz, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'union_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_union(timestamptz, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'union_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = temporal_union,
  LEFTARG = timestamptz, RIGHTARG = timestamptz,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = temporal_union,
  LEFTARG = timestamptz, RIGHTARG = timestampset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = temporal_union,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = temporal_union,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = +
);

CREATE FUNCTION temporal_union(timestampset, timestamptz)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'union_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_union(timestampset, timestampset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'union_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_union(timestampset, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'union_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_union(timestampset, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'union_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = temporal_union,
  LEFTARG = timestampset, RIGHTARG = timestamptz,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = temporal_union,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = temporal_union,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = temporal_union,
  LEFTARG = timestampset, RIGHTARG = periodset,
  COMMUTATOR = +
);

CREATE FUNCTION temporal_union(period, timestamptz)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'union_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_union(period, timestampset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'union_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_union(period, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'union_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_union(period, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'union_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = temporal_union,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = temporal_union,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = temporal_union,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = temporal_union,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = +
);

CREATE FUNCTION temporal_union(periodset, timestamptz)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'union_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_union(periodset, timestampset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'union_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_union(periodset, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'union_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_union(periodset, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'union_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = temporal_union,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = temporal_union,
  LEFTARG = periodset, RIGHTARG = timestampset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = temporal_union,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = temporal_union,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = +
);

/*****************************************************************************/

CREATE FUNCTION temporal_minus(timestamptz, timestamptz)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'minus_timestamp_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_minus(timestamptz, timestampset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'minus_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_minus(timestamptz, period)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'minus_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_minus(timestamptz, periodset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'minus_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = temporal_minus,
  LEFTARG = timestamptz, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
  PROCEDURE = temporal_minus,
  LEFTARG = timestamptz, RIGHTARG = timestampset
);
CREATE OPERATOR - (
  PROCEDURE = temporal_minus,
  LEFTARG = timestamptz, RIGHTARG = period
);
CREATE OPERATOR - (
  PROCEDURE = temporal_minus,
  LEFTARG = timestamptz, RIGHTARG = periodset
);

CREATE FUNCTION temporal_minus(timestampset, timestamptz)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'minus_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_minus(timestampset, timestampset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'minus_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_minus(timestampset, period)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'minus_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_minus(timestampset, periodset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'minus_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = temporal_minus,
  LEFTARG = timestampset, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
  PROCEDURE = temporal_minus,
  LEFTARG = timestampset, RIGHTARG = timestampset
);
CREATE OPERATOR - (
  PROCEDURE = temporal_minus,
  LEFTARG = timestampset, RIGHTARG = period
);
CREATE OPERATOR - (
  PROCEDURE = temporal_minus,
  LEFTARG = timestampset, RIGHTARG = periodset
);

CREATE FUNCTION temporal_minus(period, timestamptz)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'minus_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_minus(period, timestampset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'minus_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_minus(period, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'minus_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_minus(period, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'minus_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = temporal_minus,
  LEFTARG = period, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
  PROCEDURE = temporal_minus,
  LEFTARG = period, RIGHTARG = timestampset
);
CREATE OPERATOR - (
  PROCEDURE = temporal_minus,
  LEFTARG = period, RIGHTARG = period
);
CREATE OPERATOR - (
  PROCEDURE = temporal_minus,
  LEFTARG = period, RIGHTARG = periodset
);

CREATE FUNCTION temporal_minus(periodset, timestamptz)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'minus_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_minus(periodset, timestampset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'minus_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_minus(periodset, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'minus_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_minus(periodset, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'minus_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = temporal_minus,
  LEFTARG = periodset, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
  PROCEDURE = temporal_minus,
  LEFTARG = periodset, RIGHTARG = timestampset
);
CREATE OPERATOR - (
  PROCEDURE = temporal_minus,
  LEFTARG = periodset, RIGHTARG = period
);
CREATE OPERATOR - (
  PROCEDURE = temporal_minus,
  LEFTARG = periodset, RIGHTARG = periodset
);

/*****************************************************************************/

CREATE FUNCTION temporal_intersection(timestamptz, timestamptz)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'intersection_timestamp_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_intersection(timestamptz, timestampset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'intersection_timestamp_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_intersection(timestamptz, period)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'intersection_timestamp_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_intersection(timestamptz, periodset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'intersection_timestamp_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = temporal_intersection,
  LEFTARG = timestamptz, RIGHTARG = timestamptz,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = temporal_intersection,
  LEFTARG = timestamptz, RIGHTARG = timestampset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = temporal_intersection,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = temporal_intersection,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = *
);

CREATE FUNCTION temporal_intersection(timestampset, timestamptz)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'intersection_timestampset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_intersection(timestampset, timestampset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'intersection_timestampset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_intersection(timestampset, period)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'intersection_timestampset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_intersection(timestampset, periodset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'intersection_timestampset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = temporal_intersection,
  LEFTARG = timestampset, RIGHTARG = timestamptz,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = temporal_intersection,
  LEFTARG = timestampset, RIGHTARG = timestampset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = temporal_intersection,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = temporal_intersection,
  LEFTARG = timestampset, RIGHTARG = periodset,
  COMMUTATOR = *
);


CREATE FUNCTION temporal_intersection(period, timestamptz)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'intersection_period_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_intersection(period, timestampset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'intersection_period_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_intersection(period, period)
  RETURNS period
  AS 'MODULE_PATHNAME', 'intersection_period_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_intersection(period, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'intersection_period_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = temporal_intersection,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = temporal_intersection,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = temporal_intersection,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = temporal_intersection,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = *
);

CREATE FUNCTION temporal_intersection(periodset, timestamptz)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'intersection_periodset_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_intersection(periodset, timestampset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'intersection_periodset_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_intersection(periodset, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'intersection_periodset_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_intersection(periodset, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'intersection_periodset_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = temporal_intersection,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = temporal_intersection,
  LEFTARG = periodset, RIGHTARG = timestampset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = temporal_intersection,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = temporal_intersection,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = *
);

/*****************************************************************************/
