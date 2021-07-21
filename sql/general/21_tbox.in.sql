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
 * tbox.sql
 * Functions for temporal bounding boxes.
 */

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE TYPE tbox;

CREATE FUNCTION tbox_in(cstring)
  RETURNS tbox
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_out(tbox)
  RETURNS cstring
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
/*
CREATE FUNCTION tbox_recv(internal)
  RETURNS tbox
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_send(tbox)
  RETURNS bytea
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
*/

CREATE TYPE tbox (
  internallength = 40,
  input = tbox_in,
  output = tbox_out,
--  receive = tbox_recv,
--  send = tbox_send,
  storage = plain,
  alignment = double
);

/******************************************************************************
 * Constructors
 ******************************************************************************/

 CREATE FUNCTION tbox(float8, float8)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'tbox_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
 CREATE FUNCTION tboxt(timestamptz, timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'tbox_constructor_t'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
 CREATE FUNCTION tbox(float8, timestamptz, float8, timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'tbox_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Casting
 *****************************************************************************/

CREATE FUNCTION tbox(integer)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'int_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(float)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'float_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(numeric)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'numeric_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(intrange)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'range_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(floatrange)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'range_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'timestamp_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(period)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'period_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(timestampset)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'timestampset_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(periodset)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'periodset_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (int AS tbox) WITH FUNCTION tbox(int) AS IMPLICIT;
CREATE CAST (float AS tbox) WITH FUNCTION tbox(float) AS IMPLICIT;
CREATE CAST (numeric AS tbox) WITH FUNCTION tbox(numeric) AS IMPLICIT;
CREATE CAST (timestamptz AS tbox) WITH FUNCTION tbox(timestamptz) AS IMPLICIT;
CREATE CAST (timestampset AS tbox) WITH FUNCTION tbox(timestampset) AS IMPLICIT;
CREATE CAST (period AS tbox) WITH FUNCTION tbox(period) AS IMPLICIT;
CREATE CAST (periodset AS tbox) WITH FUNCTION tbox(periodset) AS IMPLICIT;

-- We cannot make the castings from range to tbox implicit since this produces
-- an ambiguity with the implicit castings to anyrange
CREATE CAST (intrange AS tbox) WITH FUNCTION tbox(intrange);
CREATE CAST (floatrange AS tbox) WITH FUNCTION tbox(floatrange);

CREATE FUNCTION tbox(integer, timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'int_timestamp_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(intrange, timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'range_timestamp_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(float, timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'float_timestamp_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(floatrange, timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'range_timestamp_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(integer, period)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'int_period_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(intrange, period)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'range_period_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(float, period)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'float_period_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(floatrange, period)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'range_period_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION floatrange(tbox)
  RETURNS floatrange
  AS 'MODULE_PATHNAME', 'tbox_to_floatrange'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(tbox)
  RETURNS period
  AS 'MODULE_PATHNAME', 'tbox_to_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tbox AS floatrange) WITH FUNCTION floatrange(tbox);
CREATE CAST (tbox AS period) WITH FUNCTION period(tbox);

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

CREATE FUNCTION hasX(tbox)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'tbox_hasx'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hasT(tbox)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'tbox_hast'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION Xmin(tbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'tbox_xmin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Xmax(tbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'tbox_xmax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Tmin(tbox)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'tbox_tmin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Tmax(tbox)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'tbox_tmax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

CREATE FUNCTION expandValue(tbox, float)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'tbox_expand_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION expandTemporal(tbox, interval)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'tbox_expand_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setPrecision(tbox, int)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'tbox_set_precision'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Selectivity functions
 *****************************************************************************/

CREATE FUNCTION tnumber_sel(internal, oid, internal, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'tnumber_sel'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnumber_joinsel(internal, oid, internal, smallint, internal)
  RETURNS float
  AS 'MODULE_PATHNAME', 'tnumber_joinsel'
  LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

CREATE FUNCTION tbox_contains(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_contained(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contained_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_overlaps(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_same(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'same_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_adjacent(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'adjacent_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = tbox_contains,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tbox_contained,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tbox_overlaps,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tbox_same,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = tbox_adjacent,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * Position operators
 *****************************************************************************/

CREATE FUNCTION temporal_left(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'left_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overleft_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'right_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overright_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'before_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overbefore_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'after_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overafter_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = temporal_left,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = temporal_overleft,
  LEFTARG = tbox, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbox, RIGHTARG = tbox,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = temporal_overright,
  LEFTARG = tbox, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = tbox, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = tbox, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * Set operators
 *****************************************************************************/

CREATE FUNCTION tbox_union(tbox, tbox)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'tbox_union'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_intersection(tbox, tbox)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'tbox_intersection'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tbox_union,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = +
);
CREATE OPERATOR * (
  PROCEDURE = tbox_intersection,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = *
);

/*****************************************************************************
 * Extent aggregation
 *****************************************************************************/

CREATE OR REPLACE FUNCTION tbox_extent_transfn(tbox, tbox)
  RETURNS tbox
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE OR REPLACE FUNCTION tbox_extent_combinefn(tbox, tbox)
  RETURNS tbox
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tbox) (
  SFUNC = tbox_extent_transfn,
  STYPE = tbox,
  COMBINEFUNC = tbox_extent_combinefn,
  PARALLEL = safe
);

/*****************************************************************************
 * Comparison
 *****************************************************************************/

CREATE FUNCTION tbox_eq(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'tbox_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_ne(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'tbox_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_lt(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'tbox_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_le(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'tbox_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_ge(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'tbox_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_gt(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'tbox_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_cmp(tbox, tbox)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'tbox_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = tbox, RIGHTARG = tbox,
  PROCEDURE = tbox_eq,
  COMMUTATOR = =,
  NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = tbox, RIGHTARG = tbox,
  PROCEDURE = tbox_ne,
  COMMUTATOR = <>,
  NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  PROCEDURE = tbox_lt,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR <= (
  PROCEDURE = tbox_le,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR >= (
  PROCEDURE = tbox_ge,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR > (
  PROCEDURE = tbox_gt,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = areasel, JOIN = areajoinsel
);

CREATE OPERATOR CLASS tbox_ops
  DEFAULT FOR TYPE tbox USING btree AS
  OPERATOR  1  < ,
  OPERATOR  2  <= ,
  OPERATOR  3  = ,
  OPERATOR  4  >= ,
  OPERATOR  5  > ,
  FUNCTION  1  tbox_cmp(tbox, tbox);

/*****************************************************************************/
