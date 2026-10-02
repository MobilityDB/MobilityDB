/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Route operators for temporal network points
 */

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

CREATE FUNCTION overlapsRid(bigintset, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_rid_bigintset_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @@ (
  PROCEDURE = overlapsRid,
  LEFTARG = bigintset, RIGHTARG = tnpoint,
  COMMUTATOR = @@
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION overlapsRid(tnpoint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_rid_tnpoint_bigintset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlapsRid(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_rid_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @@ (
  PROCEDURE = overlapsRid,
  LEFTARG = tnpoint, RIGHTARG = bigintset,
  COMMUTATOR = @@
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @@ (
  PROCEDURE = overlapsRid,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = @@
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Contains
 *****************************************************************************/

CREATE FUNCTION containsRid(bigintset, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_rid_bigintset_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @? (
  PROCEDURE = containsRid,
  LEFTARG = bigintset, RIGHTARG = tnpoint,
  COMMUTATOR = ?@
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION containsRid(tnpoint, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_rid_tnpoint_bigint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION containsRid(tnpoint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_rid_tnpoint_bigintset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION containsRid(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_rid_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION containsRid(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_rid_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @? (
  PROCEDURE = containsRid,
  LEFTARG = tnpoint, RIGHTARG = bigint,
  COMMUTATOR = ?@
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @? (
  PROCEDURE = containsRid,
  LEFTARG = tnpoint, RIGHTARG = bigintset,
  COMMUTATOR = ?@
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @? (
  PROCEDURE = containsRid,
  LEFTARG = tnpoint, RIGHTARG = npoint,
  COMMUTATOR = ?@
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @? (
  PROCEDURE = containsRid,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = ?@
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Contained
 *****************************************************************************/

CREATE FUNCTION containedRid(bigint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_rid_bigint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION containedRid(bigintset, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_rid_bigintset_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION containedRid(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_rid_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?@ (
  PROCEDURE = containedRid,
  LEFTARG = bigint, RIGHTARG = tnpoint,
  COMMUTATOR = @?
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ?@ (
  PROCEDURE = containedRid,
  LEFTARG = bigintset, RIGHTARG = tnpoint,
  COMMUTATOR = @?
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ?@ (
  PROCEDURE = containedRid,
  LEFTARG = npoint, RIGHTARG = tnpoint,
  COMMUTATOR = @?
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION containedRid(tnpoint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_rid_tnpoint_bigintset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION containedRid(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_rid_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?@ (
  PROCEDURE = containedRid,
  LEFTARG = tnpoint, RIGHTARG = bigintset,
  COMMUTATOR = @?
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ?@ (
  PROCEDURE = containedRid,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = @?
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Same
 *****************************************************************************/

CREATE FUNCTION sameRid(bigint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_rid_bigint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sameRid(bigintset, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_rid_bigintset_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sameRid(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_rid_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @= (
  PROCEDURE = sameRid,
  LEFTARG = bigint, RIGHTARG = tnpoint,
  COMMUTATOR = @=
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @= (
  PROCEDURE = sameRid,
  LEFTARG = bigintset, RIGHTARG = tnpoint,
  COMMUTATOR = @=
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @= (
  PROCEDURE = sameRid,
  LEFTARG = npoint, RIGHTARG = tnpoint,
  COMMUTATOR = @=
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION sameRid(tnpoint, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_rid_tnpoint_bigint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sameRid(tnpoint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_rid_tnpoint_bigintset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sameRid(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_rid_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sameRid(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_rid_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @= (
  PROCEDURE = sameRid,
  LEFTARG = tnpoint, RIGHTARG = bigint,
  COMMUTATOR = @=
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @= (
  PROCEDURE = sameRid,
  LEFTARG = tnpoint, RIGHTARG = bigintset,
  COMMUTATOR = @=
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @= (
  PROCEDURE = sameRid,
  LEFTARG = tnpoint, RIGHTARG = npoint,
  COMMUTATOR = @=
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR @= (
  PROCEDURE = sameRid,
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  COMMUTATOR = @=
  -- , RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/
