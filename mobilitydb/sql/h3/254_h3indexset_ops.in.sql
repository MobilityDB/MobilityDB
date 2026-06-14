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
 * @brief Set-theoretic operators on `h3indexset`.
 *
 * Mirrors the `bigintset` / `textset` sections of
 * `mobilitydb/sql/temporal/002_set_ops.in.sql`, but only for the
 * set-theoretic operators that do not depend on a total order of
 * the basetype: contains (`@>`), contained by (`<@`), overlaps
 * (`&&`), union (`+`), difference (`-`), intersection (`*`).
 *
 * The value-dim ordering operators (`<<`, `&<`, `>>`, `&>`) that
 * other `*set` types ship are intentionally NOT declared here.
 * H3 cell ids have no meaningful total order — the int64 bitwise
 * comparison that the framework would use for "strictly-left"
 * queries has no spatial or hierarchical meaning (same rationale
 * as the bbox-operator pruning for `th3index`).
 *
 * All C implementations behind these operators (`Contains_set_*`,
 * `Overlaps_set_set`, `Union_*`, `Minus_*`, `Intersection_*`) are
 * type-generic — they dispatch on the operand's MeosType and
 * route through `datum_cmp` / `datum_eq` from `type_util.c`,
 * where `T_H3INDEX` is already wired.
 */

/******************************************************************************
 * contains @>
 ******************************************************************************/

CREATE FUNCTION set_contains(h3indexset, h3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contains(h3indexset, h3indexset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = h3indexset, RIGHTARG = h3index,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = h3indexset, RIGHTARG = h3indexset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************
 * contained by <@
 ******************************************************************************/

CREATE FUNCTION set_contained(h3index, h3indexset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contained(h3indexset, h3indexset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = h3index, RIGHTARG = h3indexset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = h3indexset, RIGHTARG = h3indexset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************
 * overlaps &&
 ******************************************************************************/

CREATE FUNCTION set_overlaps(h3indexset, h3indexset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = set_overlaps,
  LEFTARG = h3indexset, RIGHTARG = h3indexset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************
 * set union +
 ******************************************************************************/

CREATE FUNCTION set_union(h3index, h3indexset)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(h3indexset, h3index)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(h3indexset, h3indexset)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = h3index, RIGHTARG = h3indexset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = h3indexset, RIGHTARG = h3index,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = h3indexset, RIGHTARG = h3indexset,
  COMMUTATOR = +
);

/******************************************************************************
 * set difference -
 ******************************************************************************/

CREATE FUNCTION set_minus(h3index, h3indexset)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(h3indexset, h3index)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(h3indexset, h3indexset)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = h3index, RIGHTARG = h3indexset
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = h3indexset, RIGHTARG = h3index
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = h3indexset, RIGHTARG = h3indexset
);

/******************************************************************************
 * set intersection *
 ******************************************************************************/

CREATE FUNCTION set_intersection(h3index, h3indexset)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(h3indexset, h3index)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(h3indexset, h3indexset)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = h3index, RIGHTARG = h3indexset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = h3indexset, RIGHTARG = h3index,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = h3indexset, RIGHTARG = h3indexset,
  COMMUTATOR = *
);

/******************************************************************************/
