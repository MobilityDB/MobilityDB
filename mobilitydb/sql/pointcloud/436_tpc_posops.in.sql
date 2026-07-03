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
 *****************************************************************************/

/**
 * @file
 * @brief Position operators (strictly left / right / below / above /
 *   front / back / before / after, and their "overlaps-or-X"
 *   variants) for tpcpoint / tpcpatch paired against tpcbox,
 *   tstzspan, and the temporal type itself. Mirrors the cbuffer /
 *   npoint posops surface.
 */

/******************************************************************************
 * Strictly left (<<) and overlaps-or-left (&<)
 ******************************************************************************/

-- left
CREATE FUNCTION left(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (PROCEDURE = left,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR << (PROCEDURE = left,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR << (PROCEDURE = left,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR << (PROCEDURE = left,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR << (PROCEDURE = left,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR << (PROCEDURE = left,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

-- overleft
CREATE FUNCTION overleft(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (PROCEDURE = overleft,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &< (PROCEDURE = overleft,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &< (PROCEDURE = overleft,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &< (PROCEDURE = overleft,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &< (PROCEDURE = overleft,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &< (PROCEDURE = overleft,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly right (>>) and overlaps-or-right (&>)
 ******************************************************************************/

CREATE FUNCTION right(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (PROCEDURE = right,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR >> (PROCEDURE = right,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR >> (PROCEDURE = right,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR >> (PROCEDURE = right,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR >> (PROCEDURE = right,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR >> (PROCEDURE = right,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION overright(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (PROCEDURE = overright,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &> (PROCEDURE = overright,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &> (PROCEDURE = overright,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &> (PROCEDURE = overright,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &> (PROCEDURE = overright,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &> (PROCEDURE = overright,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly below (<<|) and overlaps-or-below (&<|)
 ******************************************************************************/

CREATE FUNCTION below(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION below(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION below(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION below(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION below(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION below(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<| (PROCEDURE = below,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<| (PROCEDURE = below,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<| (PROCEDURE = below,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<| (PROCEDURE = below,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<| (PROCEDURE = below,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<| (PROCEDURE = below,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION overbelow(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbelow(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbelow(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbelow(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbelow(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbelow(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<| (PROCEDURE = overbelow,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<| (PROCEDURE = overbelow,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<| (PROCEDURE = overbelow,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<| (PROCEDURE = overbelow,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<| (PROCEDURE = overbelow,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<| (PROCEDURE = overbelow,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly above (|>>) and overlaps-or-above (|&>)
 ******************************************************************************/

CREATE FUNCTION above(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION above(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION above(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION above(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION above(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION above(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |>> (PROCEDURE = above,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |>> (PROCEDURE = above,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |>> (PROCEDURE = above,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |>> (PROCEDURE = above,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |>> (PROCEDURE = above,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |>> (PROCEDURE = above,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION overabove(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overabove(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overabove(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overabove(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overabove(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overabove(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |&> (PROCEDURE = overabove,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |&> (PROCEDURE = overabove,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |&> (PROCEDURE = overabove,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |&> (PROCEDURE = overabove,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |&> (PROCEDURE = overabove,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |&> (PROCEDURE = overabove,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly front (<</) and overlaps-or-front (&</) — Z axis
 ******************************************************************************/

CREATE FUNCTION front(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION front(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION front(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION front(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION front(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION front(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <</ (PROCEDURE = front,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <</ (PROCEDURE = front,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <</ (PROCEDURE = front,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <</ (PROCEDURE = front,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <</ (PROCEDURE = front,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <</ (PROCEDURE = front,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION overfront(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overfront(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overfront(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overfront(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overfront(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overfront(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &</ (PROCEDURE = overfront,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &</ (PROCEDURE = overfront,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &</ (PROCEDURE = overfront,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &</ (PROCEDURE = overfront,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &</ (PROCEDURE = overfront,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &</ (PROCEDURE = overfront,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly back (/>>) and overlaps-or-back (/&>) — Z axis
 ******************************************************************************/

CREATE FUNCTION back(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION back(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION back(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION back(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION back(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION back(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR />> (PROCEDURE = back,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR />> (PROCEDURE = back,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR />> (PROCEDURE = back,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR />> (PROCEDURE = back,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR />> (PROCEDURE = back,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR />> (PROCEDURE = back,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION overback(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overback(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overback(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overback(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overback(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overback(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR /&> (PROCEDURE = overback,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR /&> (PROCEDURE = overback,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR /&> (PROCEDURE = overback,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR /&> (PROCEDURE = overback,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR /&> (PROCEDURE = overback,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR /&> (PROCEDURE = overback,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly before (<<#) and overlaps-or-before (&<#) — time axis
 *
 * tstzspan-paired variants reuse the existing generic
 * Before_tstzspan_temporal / Before_temporal_tstzspan PG functions.
 ******************************************************************************/

CREATE FUNCTION before(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (PROCEDURE = before,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = before,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = before,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = before,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = before,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = before,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = before,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = before,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = before,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = before,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

CREATE FUNCTION overbefore(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (PROCEDURE = overbefore,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = overbefore,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = overbefore,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = overbefore,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = overbefore,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = overbefore,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = overbefore,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = overbefore,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = overbefore,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = overbefore,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

/******************************************************************************
 * Strictly after (#>>) and overlaps-or-after (#&>) — time axis
 ******************************************************************************/

CREATE FUNCTION after(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (PROCEDURE = after,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = after,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = after,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = after,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = after,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = after,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = after,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = after,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = after,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = after,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

CREATE FUNCTION overafter(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (PROCEDURE = overafter,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = overafter,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = overafter,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = overafter,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = overafter,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = overafter,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = overafter,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = overafter,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = overafter,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = overafter,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

/*****************************************************************************/
