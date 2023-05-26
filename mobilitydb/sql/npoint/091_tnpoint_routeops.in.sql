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

/**
 * tnpoint_routeops.sql
 * Route operators for temporal network points.
 */

/*****************************************************************************/

-- CREATE FUNCTION tnpoint_sel(internal, oid, internal, integer)
  -- RETURNS float
  -- AS 'MODULE_PATHNAME', 'Tnpoint_sel'
  -- LANGUAGE C IMMUTABLE STRICT;

-- CREATE FUNCTION tnpoint_joinsel(internal, oid, internal, smallint, internal)
  -- RETURNS float
  -- AS 'MODULE_PATHNAME', 'Tnpoint_joinsel'
  -- LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

CREATE FUNCTION overlaps_rid(bigintset, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_rid_bigintset_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @@ (
  PROCEDURE = overlaps_rid,
  LEFTARG = bigintset, RIGHTARG = tnpoint,
  COMMUTATOR = @@
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

CREATE FUNCTION overlaps_rid(tnpoint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_rid_tnpoint_bigintset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_rid(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_rid_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @@ (
  PROCEDURE = overlaps_rid,
  LEFTARG = tnpoint, RIGHTARG = bigintset,
  COMMUTATOR = @@
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR @@ (
  PROCEDURE = overlaps_rid,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = @@
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

/*****************************************************************************
 * Contains
 *****************************************************************************/

CREATE FUNCTION contains_rid(bigintset, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_rid_bigintset_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @? (
  PROCEDURE = contains_rid,
  LEFTARG = bigintset, RIGHTARG = tnpoint,
  COMMUTATOR = ?@
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

CREATE FUNCTION contains_rid(tnpoint, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_rid_tnpoint_bigint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_rid(tnpoint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_rid_tnpoint_bigintset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_rid(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_rid_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_rid(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_rid_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @? (
  PROCEDURE = contains_rid,
  LEFTARG = tnpoint, RIGHTARG = bigint,
  COMMUTATOR = ?@
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR @? (
  PROCEDURE = contains_rid,
  LEFTARG = tnpoint, RIGHTARG = bigintset,
  COMMUTATOR = ?@
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR @? (
  PROCEDURE = contains_rid,
  LEFTARG = tnpoint, RIGHTARG = npoint,
  COMMUTATOR = ?@
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR @? (
  PROCEDURE = contains_rid,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = ?@
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

/*****************************************************************************
 * Contained
 *****************************************************************************/

CREATE FUNCTION contained_rid(bigint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_rid_bigint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_rid(bigintset, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_rid_bigintset_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_rid(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_rid_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?@ (
  PROCEDURE = contained_rid,
  LEFTARG = bigint, RIGHTARG = tnpoint,
  COMMUTATOR = @?
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR ?@ (
  PROCEDURE = contained_rid,
  LEFTARG = bigintset, RIGHTARG = tnpoint,
  COMMUTATOR = @?
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR ?@ (
  PROCEDURE = contained_rid,
  LEFTARG = npoint, RIGHTARG = tnpoint,
  COMMUTATOR = @?
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

CREATE FUNCTION contained_rid(tnpoint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_rid_tnpoint_bigintset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_rid(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_rid_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?@ (
  PROCEDURE = contained_rid,
  LEFTARG = tnpoint, RIGHTARG = bigintset,
  COMMUTATOR = @?
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR ?@ (
  PROCEDURE = contained_rid,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = @?
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

/*****************************************************************************
 * Same
 *****************************************************************************/

CREATE FUNCTION same_rid(bigint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_rid_bigint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_rid(bigintset, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_rid_bigintset_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_rid(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_rid_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @= (
  PROCEDURE = same_rid,
  LEFTARG = bigint, RIGHTARG = tnpoint,
  COMMUTATOR = @=
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR @= (
  PROCEDURE = same_rid,
  LEFTARG = bigintset, RIGHTARG = tnpoint,
  COMMUTATOR = @=
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR @= (
  PROCEDURE = same_rid,
  LEFTARG = npoint, RIGHTARG = tnpoint,
  COMMUTATOR = @=
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

CREATE FUNCTION same_rid(tnpoint, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_rid_tnpoint_bigint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_rid(tnpoint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_rid_tnpoint_bigintset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_rid(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_rid_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_rid(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_rid_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @= (
  PROCEDURE = same_rid,
  LEFTARG = tnpoint, RIGHTARG = bigint,
  COMMUTATOR = @=
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR @= (
  PROCEDURE = same_rid,
  LEFTARG = tnpoint, RIGHTARG = bigintset,
  COMMUTATOR = @=
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR @= (
  PROCEDURE = same_rid,
  LEFTARG = tnpoint, RIGHTARG = npoint,
  COMMUTATOR = @=
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);
CREATE OPERATOR @= (
  PROCEDURE = same_rid,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = @=
  -- , RESTRICT = tnpoint_sel, JOIN = tnpoint_joinsel
);

/*****************************************************************************/
