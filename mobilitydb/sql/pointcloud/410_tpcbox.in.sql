/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief TPCBox bounding-box type.
 *
 * Fixed-size struct; no varlena. Mirrors STBox but carries an extra
 * `pcid` field so bboxes from different pgpointcloud schemas cannot
 * silently merge. Most work goes through per-type wrappers in
 * `mobilitydb/src/pointcloud/tpcbox.c`.
 *
 * The text form mirrors that of `stbox` inside a `TPCBOX(...)` wrapper with the
 * `pcid` as the last component, e.g. `TPCBOX(X((0,0),(10,10)), 1)`. Binary
 * interchange uses @c Tpcbox_recv / @c Tpcbox_send.
 */

/******************************************************************************
 * Type
 ******************************************************************************/

CREATE TYPE tpcbox;

CREATE FUNCTION tpcbox_in(cstring)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpcbox_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_out(tpcbox)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Tpcbox_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_recv(internal)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpcbox_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_send(tpcbox)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tpcbox_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE tpcbox (
  internallength = 88,
  input = tpcbox_in,
  output = tpcbox_out,
  receive = tpcbox_recv,
  send = tpcbox_send,
  alignment = double
);

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION tpcbox(xmin float8, ymin float8, xmax float8, ymax float8,
    pcid integer DEFAULT 0, srid integer DEFAULT 0)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpcbox_constructor_2d'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tpcbox_z(xmin float8, ymin float8, zmin float8,
    xmax float8, ymax float8, zmax float8,
    pcid integer DEFAULT 0, srid integer DEFAULT 0)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpcbox_constructor_3d'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tpcbox_t(period tstzspan, pcid integer DEFAULT 0)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpcbox_constructor_t'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tpcbox_xt(xmin float8, ymin float8, xmax float8, ymax float8,
    period tstzspan, pcid integer DEFAULT 0, srid integer DEFAULT 0)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpcbox_constructor_xt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tpcbox_zt(xmin float8, ymin float8, zmin float8,
    xmax float8, ymax float8, zmax float8, period tstzspan,
    pcid integer DEFAULT 0, srid integer DEFAULT 0)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpcbox_constructor_zt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Conversion
 ******************************************************************************/

-- SRID auto-filled from the pgpointcloud schema via the schema cache.
CREATE FUNCTION tpcbox(pcpatch)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Pcpatch_to_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Explicit SRID override — takes no catalog detour.
CREATE FUNCTION tpcbox(pcpatch, srid integer)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Pcpatch_to_tpcbox_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- pcpoint → TPCBox: degenerate single-point bbox with spatial bounds =
-- the point's X/Y/[Z]; needs the schema cache.
CREATE FUNCTION tpcbox(pcpoint)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Pcpoint_to_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (pcpatch AS tpcbox) WITH FUNCTION tpcbox(pcpatch);
CREATE CAST (pcpoint AS tpcbox) WITH FUNCTION tpcbox(pcpoint);

-- Project a tpcbox to an stbox by dropping the pcid. Write `value::stbox` to
-- compose a tpcbox into an stbox-only operator, as every other projection into
-- stbox is written.
CREATE FUNCTION stbox(tpcbox)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tpcbox_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- The cast is explicit. An implicit one lets a tpcbox reach the stbox
-- operators with no schema comparison at all, so `tpcbox && stbox` answers
-- where `tpcbox && tpcbox` reports the pcid mismatch, and a tpcbox assigns
-- into an stbox column dropping its schema silently.
CREATE CAST (tpcbox AS stbox) WITH FUNCTION stbox(tpcbox);

/******************************************************************************
 * Accessors
 ******************************************************************************/

CREATE FUNCTION hasX(tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_hasx'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hasZ(tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_hasz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hasT(tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_hast'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION xMin(tpcbox)
  RETURNS float8 AS 'MODULE_PATHNAME', 'Tpcbox_xmin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION xMax(tpcbox)
  RETURNS float8 AS 'MODULE_PATHNAME', 'Tpcbox_xmax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION yMin(tpcbox)
  RETURNS float8 AS 'MODULE_PATHNAME', 'Tpcbox_ymin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION yMax(tpcbox)
  RETURNS float8 AS 'MODULE_PATHNAME', 'Tpcbox_ymax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION zMin(tpcbox)
  RETURNS float8 AS 'MODULE_PATHNAME', 'Tpcbox_zmin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION zMax(tpcbox)
  RETURNS float8 AS 'MODULE_PATHNAME', 'Tpcbox_zmax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tMin(tpcbox)
  RETURNS timestamptz AS 'MODULE_PATHNAME', 'Tpcbox_tmin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tMinInc(tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_tmin_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tMax(tpcbox)
  RETURNS timestamptz AS 'MODULE_PATHNAME', 'Tpcbox_tmax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tMaxInc(tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_tmax_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION SRID(tpcbox)
  RETURNS integer AS 'MODULE_PATHNAME', 'Tpcbox_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pcid(tpcbox)
  RETURNS integer AS 'MODULE_PATHNAME', 'Tpcbox_pcid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Transformations
 ******************************************************************************/

CREATE FUNCTION round(tpcbox, integer DEFAULT 0)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpcbox_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setSRID(tpcbox, integer)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpcbox_set_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Set operations
 ******************************************************************************/

CREATE FUNCTION tpcboxUnion(tpcbox, tpcbox)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Union_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxIntersection(tpcbox, tpcbox)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Intersection_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tpcboxUnion,
  LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = +
);
CREATE OPERATOR * (
  PROCEDURE = tpcboxIntersection,
  LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = *
);

/******************************************************************************
 * Topological predicates
 ******************************************************************************/

CREATE FUNCTION contains(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Contains_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Contained_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overlaps_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Same_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION adjacent(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Adjacent_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = <@
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = @>
);
CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = &&
);
CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = ~=
);
CREATE OPERATOR -|- (
  PROCEDURE = adjacent,
  LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = -|-
);

/******************************************************************************
 * Comparison / B-tree
 ******************************************************************************/

CREATE FUNCTION eq(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lt(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(tpcbox, tpcbox)
  RETURNS integer AS 'MODULE_PATHNAME', 'Tpcbox_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = tpcbox, RIGHTARG = tpcbox, PROCEDURE = eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = tpcbox, RIGHTARG = tpcbox, PROCEDURE = ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  LEFTARG = tpcbox, RIGHTARG = tpcbox, PROCEDURE = lt,
  COMMUTATOR = >, NEGATOR = >=
);
CREATE OPERATOR <= (
  LEFTARG = tpcbox, RIGHTARG = tpcbox, PROCEDURE = le,
  COMMUTATOR = >=, NEGATOR = >
);
CREATE OPERATOR >= (
  LEFTARG = tpcbox, RIGHTARG = tpcbox, PROCEDURE = ge,
  COMMUTATOR = <=, NEGATOR = <
);
CREATE OPERATOR > (
  LEFTARG = tpcbox, RIGHTARG = tpcbox, PROCEDURE = gt,
  COMMUTATOR = <, NEGATOR = <=
);

CREATE OPERATOR CLASS tpcbox_btree_ops
  DEFAULT FOR TYPE tpcbox USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  cmp(tpcbox, tpcbox);

/******************************************************************************
 * Position predicates — strict + overlap variants across X / Y / Z / time.
 * A predicate only evaluates on dimensions both operands carry.
 ******************************************************************************/

CREATE FUNCTION left(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Left_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overleft_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Right_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overright_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION below(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Below_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbelow(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overbelow_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION above(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Above_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overabove(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overabove_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION front(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Front_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overfront(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overfront_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION back(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Back_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overback(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overback_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Before_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overbefore_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'After_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overafter_tpcbox_tpcbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = left, LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = >>
);
CREATE OPERATOR &< (
  PROCEDURE = overleft, LEFTARG = tpcbox, RIGHTARG = tpcbox
);
CREATE OPERATOR >> (
  PROCEDURE = right, LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = <<
);
CREATE OPERATOR &> (
  PROCEDURE = overright, LEFTARG = tpcbox, RIGHTARG = tpcbox
);
CREATE OPERATOR <<| (
  PROCEDURE = below, LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = |>>
);
CREATE OPERATOR &<| (
  PROCEDURE = overbelow, LEFTARG = tpcbox, RIGHTARG = tpcbox
);
CREATE OPERATOR |>> (
  PROCEDURE = above, LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = <<|
);
CREATE OPERATOR |&> (
  PROCEDURE = overabove, LEFTARG = tpcbox, RIGHTARG = tpcbox
);
CREATE OPERATOR <</ (
  PROCEDURE = front, LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = />>
);
CREATE OPERATOR &</ (
  PROCEDURE = overfront, LEFTARG = tpcbox, RIGHTARG = tpcbox
);
CREATE OPERATOR />> (
  PROCEDURE = back, LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = <</
);
CREATE OPERATOR /&> (
  PROCEDURE = overback, LEFTARG = tpcbox, RIGHTARG = tpcbox
);
CREATE OPERATOR <<# (
  PROCEDURE = before, LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = #>>
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore, LEFTARG = tpcbox, RIGHTARG = tpcbox
);
CREATE OPERATOR #>> (
  PROCEDURE = after, LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = <<#
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter, LEFTARG = tpcbox, RIGHTARG = tpcbox
);

/*****************************************************************************/
