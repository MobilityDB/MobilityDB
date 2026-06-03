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
 * @brief TPCBox bounding-box type.
 *
 * Fixed-size struct; no varlena. Mirrors STBox but carries an extra
 * `pcid` field so bboxes from different pgpointcloud schemas cannot
 * silently merge. Most work goes through per-type wrappers in
 * `mobilitydb/src/pointcloud/tpcbox.c`.
 *
 * Input is the byte-image hex form emitted by @c Tpcbox_out /
 * @c Tpcbox_send.
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
-- STABLE, not IMMUTABLE, because the schema lives in a PG catalog table.
CREATE FUNCTION tpcbox(pcpatch)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Pcpatch_to_tpcbox'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

-- Explicit SRID override — IMMUTABLE, takes no catalog detour.
CREATE FUNCTION tpcbox(pcpatch, srid integer)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Pcpatch_to_tpcbox_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- pcpoint → TPCBox: degenerate single-point bbox with spatial bounds =
-- the point's X/Y/[Z]. STABLE; needs the schema cache.
CREATE FUNCTION tpcbox(pcpoint)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Pcpoint_to_tpcbox'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

CREATE CAST (pcpatch AS tpcbox) WITH FUNCTION tpcbox(pcpatch);
CREATE CAST (pcpoint AS tpcbox) WITH FUNCTION tpcbox(pcpoint);

-- Project a tpcbox to an stbox by dropping the pcid. Lets tpcbox values
-- compose into stbox-only operators (extent aggregation across mixed
-- pcids, generic spatiotemporal predicates) without manual conversion.
CREATE FUNCTION stbox(tpcbox)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tpcbox_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tpcbox AS stbox) WITH FUNCTION stbox(tpcbox) AS IMPLICIT;

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

CREATE FUNCTION xmin(tpcbox)
  RETURNS float8 AS 'MODULE_PATHNAME', 'Tpcbox_xmin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION xmax(tpcbox)
  RETURNS float8 AS 'MODULE_PATHNAME', 'Tpcbox_xmax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ymin(tpcbox)
  RETURNS float8 AS 'MODULE_PATHNAME', 'Tpcbox_ymin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ymax(tpcbox)
  RETURNS float8 AS 'MODULE_PATHNAME', 'Tpcbox_ymax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION zmin(tpcbox)
  RETURNS float8 AS 'MODULE_PATHNAME', 'Tpcbox_zmin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION zmax(tpcbox)
  RETURNS float8 AS 'MODULE_PATHNAME', 'Tpcbox_zmax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tmin(tpcbox)
  RETURNS timestamptz AS 'MODULE_PATHNAME', 'Tpcbox_tmin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tmax(tpcbox)
  RETURNS timestamptz AS 'MODULE_PATHNAME', 'Tpcbox_tmax'
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

CREATE FUNCTION tpcbox_union(tpcbox, tpcbox)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Union_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_intersection(tpcbox, tpcbox)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Intersection_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tpcbox_union,
  LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = +
);
CREATE OPERATOR * (
  PROCEDURE = tpcbox_intersection,
  LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = *
);

/******************************************************************************
 * Topological predicates
 ******************************************************************************/

CREATE FUNCTION tpcbox_contains(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Contains_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_contained(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Contained_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_overlaps(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overlaps_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_same(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Same_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_adjacent(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Adjacent_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = tpcbox_contains,
  LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = <@
);
CREATE OPERATOR <@ (
  PROCEDURE = tpcbox_contained,
  LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = @>
);
CREATE OPERATOR && (
  PROCEDURE = tpcbox_overlaps,
  LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = &&
);
CREATE OPERATOR ~= (
  PROCEDURE = tpcbox_same,
  LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = ~=
);
CREATE OPERATOR -|- (
  PROCEDURE = tpcbox_adjacent,
  LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = -|-
);

/******************************************************************************
 * Comparison / B-tree
 ******************************************************************************/

CREATE FUNCTION tpcbox_eq(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_ne(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_lt(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_le(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_gt(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_ge(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Tpcbox_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_cmp(tpcbox, tpcbox)
  RETURNS integer AS 'MODULE_PATHNAME', 'Tpcbox_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = tpcbox, RIGHTARG = tpcbox, PROCEDURE = tpcbox_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = tpcbox, RIGHTARG = tpcbox, PROCEDURE = tpcbox_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  LEFTARG = tpcbox, RIGHTARG = tpcbox, PROCEDURE = tpcbox_lt,
  COMMUTATOR = >, NEGATOR = >=
);
CREATE OPERATOR <= (
  LEFTARG = tpcbox, RIGHTARG = tpcbox, PROCEDURE = tpcbox_le,
  COMMUTATOR = >=, NEGATOR = >
);
CREATE OPERATOR >= (
  LEFTARG = tpcbox, RIGHTARG = tpcbox, PROCEDURE = tpcbox_ge,
  COMMUTATOR = <=, NEGATOR = <
);
CREATE OPERATOR > (
  LEFTARG = tpcbox, RIGHTARG = tpcbox, PROCEDURE = tpcbox_gt,
  COMMUTATOR = <, NEGATOR = <=
);

CREATE OPERATOR CLASS tpcbox_btree_ops
  DEFAULT FOR TYPE tpcbox USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  tpcbox_cmp(tpcbox, tpcbox);

/******************************************************************************
 * Position predicates — strict + overlap variants across X / Y / Z / time.
 * A predicate only evaluates on dimensions both operands carry.
 ******************************************************************************/

CREATE FUNCTION tpcbox_left(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Left_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_overleft(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overleft_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_right(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Right_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_overright(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overright_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_below(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Below_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_overbelow(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overbelow_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_above(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Above_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_overabove(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overabove_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_front(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Front_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_overfront(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overfront_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_back(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Back_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_overback(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overback_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_before(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Before_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_overbefore(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overbefore_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_after(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'After_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcbox_overafter(tpcbox, tpcbox)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Overafter_tpcbox_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = tpcbox_left, LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = >>
);
CREATE OPERATOR &< (
  PROCEDURE = tpcbox_overleft, LEFTARG = tpcbox, RIGHTARG = tpcbox
);
CREATE OPERATOR >> (
  PROCEDURE = tpcbox_right, LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = <<
);
CREATE OPERATOR &> (
  PROCEDURE = tpcbox_overright, LEFTARG = tpcbox, RIGHTARG = tpcbox
);
CREATE OPERATOR <<| (
  PROCEDURE = tpcbox_below, LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = |>>
);
CREATE OPERATOR &<| (
  PROCEDURE = tpcbox_overbelow, LEFTARG = tpcbox, RIGHTARG = tpcbox
);
CREATE OPERATOR |>> (
  PROCEDURE = tpcbox_above, LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = <<|
);
CREATE OPERATOR |&> (
  PROCEDURE = tpcbox_overabove, LEFTARG = tpcbox, RIGHTARG = tpcbox
);
CREATE OPERATOR <</ (
  PROCEDURE = tpcbox_front, LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = />>
);
CREATE OPERATOR &</ (
  PROCEDURE = tpcbox_overfront, LEFTARG = tpcbox, RIGHTARG = tpcbox
);
CREATE OPERATOR />> (
  PROCEDURE = tpcbox_back, LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = <</
);
CREATE OPERATOR /&> (
  PROCEDURE = tpcbox_overback, LEFTARG = tpcbox, RIGHTARG = tpcbox
);
CREATE OPERATOR <<# (
  PROCEDURE = tpcbox_before, LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = #>>
);
CREATE OPERATOR &<# (
  PROCEDURE = tpcbox_overbefore, LEFTARG = tpcbox, RIGHTARG = tpcbox
);
CREATE OPERATOR #>> (
  PROCEDURE = tpcbox_after, LEFTARG = tpcbox, RIGHTARG = tpcbox,
  COMMUTATOR = <<#
);
CREATE OPERATOR #&> (
  PROCEDURE = tpcbox_overafter, LEFTARG = tpcbox, RIGHTARG = tpcbox
);

/*****************************************************************************/
