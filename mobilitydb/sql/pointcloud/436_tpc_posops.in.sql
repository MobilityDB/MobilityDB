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

-- temporal_left
CREATE FUNCTION temporal_left(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_left(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_left(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_left(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_left(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_left(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (PROCEDURE = temporal_left,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR << (PROCEDURE = temporal_left,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR << (PROCEDURE = temporal_left,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR << (PROCEDURE = temporal_left,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR << (PROCEDURE = temporal_left,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR << (PROCEDURE = temporal_left,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

-- temporal_overleft
CREATE FUNCTION temporal_overleft(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (PROCEDURE = temporal_overleft,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &< (PROCEDURE = temporal_overleft,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &< (PROCEDURE = temporal_overleft,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &< (PROCEDURE = temporal_overleft,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &< (PROCEDURE = temporal_overleft,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &< (PROCEDURE = temporal_overleft,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly right (>>) and overlaps-or-right (&>)
 ******************************************************************************/

CREATE FUNCTION temporal_right(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (PROCEDURE = temporal_right,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR >> (PROCEDURE = temporal_right,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR >> (PROCEDURE = temporal_right,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR >> (PROCEDURE = temporal_right,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR >> (PROCEDURE = temporal_right,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR >> (PROCEDURE = temporal_right,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION temporal_overright(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (PROCEDURE = temporal_overright,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &> (PROCEDURE = temporal_overright,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &> (PROCEDURE = temporal_overright,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &> (PROCEDURE = temporal_overright,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &> (PROCEDURE = temporal_overright,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &> (PROCEDURE = temporal_overright,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly below (<<|) and overlaps-or-below (&<|)
 ******************************************************************************/

CREATE FUNCTION temporal_below(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<| (PROCEDURE = temporal_below,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<| (PROCEDURE = temporal_below,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<| (PROCEDURE = temporal_below,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<| (PROCEDURE = temporal_below,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<| (PROCEDURE = temporal_below,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<| (PROCEDURE = temporal_below,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION temporal_overbelow(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<| (PROCEDURE = temporal_overbelow,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<| (PROCEDURE = temporal_overbelow,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<| (PROCEDURE = temporal_overbelow,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<| (PROCEDURE = temporal_overbelow,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<| (PROCEDURE = temporal_overbelow,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<| (PROCEDURE = temporal_overbelow,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly above (|>>) and overlaps-or-above (|&>)
 ******************************************************************************/

CREATE FUNCTION temporal_above(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |>> (PROCEDURE = temporal_above,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |>> (PROCEDURE = temporal_above,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |>> (PROCEDURE = temporal_above,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |>> (PROCEDURE = temporal_above,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |>> (PROCEDURE = temporal_above,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |>> (PROCEDURE = temporal_above,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION temporal_overabove(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |&> (PROCEDURE = temporal_overabove,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |&> (PROCEDURE = temporal_overabove,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |&> (PROCEDURE = temporal_overabove,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |&> (PROCEDURE = temporal_overabove,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |&> (PROCEDURE = temporal_overabove,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |&> (PROCEDURE = temporal_overabove,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly front (<</) and overlaps-or-front (&</) — Z axis
 ******************************************************************************/

CREATE FUNCTION temporal_front(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <</ (PROCEDURE = temporal_front,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <</ (PROCEDURE = temporal_front,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <</ (PROCEDURE = temporal_front,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <</ (PROCEDURE = temporal_front,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <</ (PROCEDURE = temporal_front,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <</ (PROCEDURE = temporal_front,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION temporal_overfront(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &</ (PROCEDURE = temporal_overfront,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &</ (PROCEDURE = temporal_overfront,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &</ (PROCEDURE = temporal_overfront,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &</ (PROCEDURE = temporal_overfront,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &</ (PROCEDURE = temporal_overfront,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &</ (PROCEDURE = temporal_overfront,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly back (/>>) and overlaps-or-back (/&>) — Z axis
 ******************************************************************************/

CREATE FUNCTION temporal_back(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR />> (PROCEDURE = temporal_back,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR />> (PROCEDURE = temporal_back,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR />> (PROCEDURE = temporal_back,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR />> (PROCEDURE = temporal_back,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR />> (PROCEDURE = temporal_back,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR />> (PROCEDURE = temporal_back,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION temporal_overback(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR /&> (PROCEDURE = temporal_overback,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR /&> (PROCEDURE = temporal_overback,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR /&> (PROCEDURE = temporal_overback,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR /&> (PROCEDURE = temporal_overback,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR /&> (PROCEDURE = temporal_overback,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR /&> (PROCEDURE = temporal_overback,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly before (<<#) and overlaps-or-before (&<#) — time axis
 *
 * tstzspan-paired variants reuse the existing generic
 * Before_tstzspan_temporal / Before_temporal_tstzspan PG functions.
 ******************************************************************************/

CREATE FUNCTION temporal_before(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (PROCEDURE = temporal_before,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = temporal_before,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = temporal_before,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = temporal_before,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = temporal_before,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = temporal_before,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = temporal_before,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = temporal_before,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = temporal_before,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR <<# (PROCEDURE = temporal_before,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = '#>>',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

CREATE FUNCTION temporal_overbefore(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (PROCEDURE = temporal_overbefore,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = temporal_overbefore,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = temporal_overbefore,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = temporal_overbefore,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = temporal_overbefore,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = temporal_overbefore,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = temporal_overbefore,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = temporal_overbefore,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = temporal_overbefore,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR &<# (PROCEDURE = temporal_overbefore,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

/******************************************************************************
 * Strictly after (#>>) and overlaps-or-after (#&>) — time axis
 ******************************************************************************/

CREATE FUNCTION temporal_after(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (PROCEDURE = temporal_after,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = temporal_after,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = temporal_after,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = temporal_after,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = temporal_after,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = temporal_after,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = temporal_after,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = temporal_after,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = temporal_after,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #>> (PROCEDURE = temporal_after,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = '<<#',
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

CREATE FUNCTION temporal_overafter(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (PROCEDURE = temporal_overafter,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = temporal_overafter,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = temporal_overafter,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = temporal_overafter,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = temporal_overafter,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = temporal_overafter,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = temporal_overafter,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = temporal_overafter,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = temporal_overafter,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);
CREATE OPERATOR #&> (PROCEDURE = temporal_overafter,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel);

/*****************************************************************************/
