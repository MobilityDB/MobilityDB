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
 * @brief Position operators of the temporal pgpointcloud types
 * @details Position operators (strictly left / right / below / above /
 * front / back / before / after, and their "overlaps-or-X"
 * variants) for tpcpoint / tpcpatch paired against tpcbox,
 * tstzspan, and the temporal type itself. Mirrors the cbuffer /
 * npoint posops surface.
 */

/******************************************************************************
 * Strictly left (<<) and overlaps-or-left (&<)
 ******************************************************************************/

-- left
CREATE FUNCTION tpcboxLeft(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxLeft(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxLeft(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxLeft(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxLeft(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxLeft(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (PROCEDURE = tpcboxLeft,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR << (PROCEDURE = tpcboxLeft,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR << (PROCEDURE = tpcboxLeft,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR << (PROCEDURE = tpcboxLeft,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR << (PROCEDURE = tpcboxLeft,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR << (PROCEDURE = tpcboxLeft,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  COMMUTATOR = >>, RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

-- overleft
CREATE FUNCTION tpcboxOverleft(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverleft(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverleft(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverleft(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverleft(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverleft(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (PROCEDURE = tpcboxOverleft,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &< (PROCEDURE = tpcboxOverleft,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &< (PROCEDURE = tpcboxOverleft,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &< (PROCEDURE = tpcboxOverleft,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &< (PROCEDURE = tpcboxOverleft,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &< (PROCEDURE = tpcboxOverleft,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly right (>>) and overlaps-or-right (&>)
 ******************************************************************************/

CREATE FUNCTION tpcboxRight(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxRight(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxRight(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxRight(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxRight(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxRight(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (PROCEDURE = tpcboxRight,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR >> (PROCEDURE = tpcboxRight,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR >> (PROCEDURE = tpcboxRight,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR >> (PROCEDURE = tpcboxRight,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR >> (PROCEDURE = tpcboxRight,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR >> (PROCEDURE = tpcboxRight,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION tpcboxOverright(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverright(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverright(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverright(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverright(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverright(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (PROCEDURE = tpcboxOverright,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &> (PROCEDURE = tpcboxOverright,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &> (PROCEDURE = tpcboxOverright,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &> (PROCEDURE = tpcboxOverright,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &> (PROCEDURE = tpcboxOverright,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &> (PROCEDURE = tpcboxOverright,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly below (<<|) and overlaps-or-below (&<|)
 ******************************************************************************/

CREATE FUNCTION tpcboxBelow(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBelow(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBelow(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBelow(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBelow(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBelow(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<| (PROCEDURE = tpcboxBelow,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<| (PROCEDURE = tpcboxBelow,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<| (PROCEDURE = tpcboxBelow,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<| (PROCEDURE = tpcboxBelow,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<| (PROCEDURE = tpcboxBelow,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<| (PROCEDURE = tpcboxBelow,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION tpcboxOverbelow(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverbelow(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverbelow(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverbelow(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverbelow(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverbelow(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<| (PROCEDURE = tpcboxOverbelow,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<| (PROCEDURE = tpcboxOverbelow,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<| (PROCEDURE = tpcboxOverbelow,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<| (PROCEDURE = tpcboxOverbelow,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<| (PROCEDURE = tpcboxOverbelow,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<| (PROCEDURE = tpcboxOverbelow,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly above (|>>) and overlaps-or-above (|&>)
 ******************************************************************************/

CREATE FUNCTION tpcboxAbove(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAbove(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAbove(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAbove(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAbove(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAbove(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |>> (PROCEDURE = tpcboxAbove,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |>> (PROCEDURE = tpcboxAbove,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |>> (PROCEDURE = tpcboxAbove,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |>> (PROCEDURE = tpcboxAbove,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |>> (PROCEDURE = tpcboxAbove,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |>> (PROCEDURE = tpcboxAbove,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION tpcboxOverabove(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverabove(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverabove(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverabove(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverabove(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverabove(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |&> (PROCEDURE = tpcboxOverabove,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |&> (PROCEDURE = tpcboxOverabove,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |&> (PROCEDURE = tpcboxOverabove,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |&> (PROCEDURE = tpcboxOverabove,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |&> (PROCEDURE = tpcboxOverabove,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR |&> (PROCEDURE = tpcboxOverabove,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly front (<</) and overlaps-or-front (&</) — Z axis
 ******************************************************************************/

CREATE FUNCTION tpcboxFront(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxFront(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxFront(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxFront(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxFront(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxFront(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <</ (PROCEDURE = tpcboxFront,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <</ (PROCEDURE = tpcboxFront,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <</ (PROCEDURE = tpcboxFront,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <</ (PROCEDURE = tpcboxFront,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <</ (PROCEDURE = tpcboxFront,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <</ (PROCEDURE = tpcboxFront,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION tpcboxOverfront(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverfront(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverfront(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverfront(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverfront(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverfront(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &</ (PROCEDURE = tpcboxOverfront,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &</ (PROCEDURE = tpcboxOverfront,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &</ (PROCEDURE = tpcboxOverfront,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &</ (PROCEDURE = tpcboxOverfront,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &</ (PROCEDURE = tpcboxOverfront,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &</ (PROCEDURE = tpcboxOverfront,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly back (/>>) and overlaps-or-back (/&>) — Z axis
 ******************************************************************************/

CREATE FUNCTION tpcboxBack(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBack(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBack(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBack(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBack(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBack(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR />> (PROCEDURE = tpcboxBack,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR />> (PROCEDURE = tpcboxBack,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR />> (PROCEDURE = tpcboxBack,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR />> (PROCEDURE = tpcboxBack,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR />> (PROCEDURE = tpcboxBack,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR />> (PROCEDURE = tpcboxBack,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION tpcboxOverback(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverback(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverback(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverback(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverback(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverback(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR /&> (PROCEDURE = tpcboxOverback,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR /&> (PROCEDURE = tpcboxOverback,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR /&> (PROCEDURE = tpcboxOverback,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR /&> (PROCEDURE = tpcboxOverback,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR /&> (PROCEDURE = tpcboxOverback,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR /&> (PROCEDURE = tpcboxOverback,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly before (<<#) and overlaps-or-before (&<#) — time axis
 *
 * tstzspan-paired variants reuse the existing generic
 * Before_tstzspan_temporal / Before_temporal_tstzspan PG functions.
 ******************************************************************************/

CREATE FUNCTION tpcboxBefore(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBefore(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBefore(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBefore(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBefore(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBefore(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBefore(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBefore(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBefore(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxBefore(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (PROCEDURE = tpcboxBefore,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint, COMMUTATOR = '#>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<# (PROCEDURE = tpcboxBefore,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan, COMMUTATOR = '#>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<# (PROCEDURE = tpcboxBefore,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = '#>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<# (PROCEDURE = tpcboxBefore,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = '#>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<# (PROCEDURE = tpcboxBefore,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = '#>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<# (PROCEDURE = tpcboxBefore,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch, COMMUTATOR = '#>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<# (PROCEDURE = tpcboxBefore,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan, COMMUTATOR = '#>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<# (PROCEDURE = tpcboxBefore,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = '#>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<# (PROCEDURE = tpcboxBefore,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = '#>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR <<# (PROCEDURE = tpcboxBefore,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = '#>>',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION tpcboxOverbefore(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverbefore(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverbefore(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverbefore(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverbefore(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverbefore(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverbefore(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverbefore(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverbefore(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverbefore(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (PROCEDURE = tpcboxOverbefore,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<# (PROCEDURE = tpcboxOverbefore,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<# (PROCEDURE = tpcboxOverbefore,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<# (PROCEDURE = tpcboxOverbefore,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<# (PROCEDURE = tpcboxOverbefore,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<# (PROCEDURE = tpcboxOverbefore,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<# (PROCEDURE = tpcboxOverbefore,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<# (PROCEDURE = tpcboxOverbefore,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<# (PROCEDURE = tpcboxOverbefore,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR &<# (PROCEDURE = tpcboxOverbefore,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/******************************************************************************
 * Strictly after (#>>) and overlaps-or-after (#&>) — time axis
 ******************************************************************************/

CREATE FUNCTION tpcboxAfter(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAfter(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAfter(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAfter(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAfter(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAfter(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAfter(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAfter(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAfter(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxAfter(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (PROCEDURE = tpcboxAfter,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint, COMMUTATOR = '<<#',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #>> (PROCEDURE = tpcboxAfter,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan, COMMUTATOR = '<<#',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #>> (PROCEDURE = tpcboxAfter,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = '<<#',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #>> (PROCEDURE = tpcboxAfter,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = '<<#',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #>> (PROCEDURE = tpcboxAfter,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = '<<#',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #>> (PROCEDURE = tpcboxAfter,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch, COMMUTATOR = '<<#',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #>> (PROCEDURE = tpcboxAfter,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan, COMMUTATOR = '<<#',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #>> (PROCEDURE = tpcboxAfter,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = '<<#',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #>> (PROCEDURE = tpcboxAfter,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = '<<#',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #>> (PROCEDURE = tpcboxAfter,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = '<<#',
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

CREATE FUNCTION tpcboxOverafter(tstzspan, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverafter(tpcpoint, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverafter(tpcbox, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverafter(tpcpoint, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverafter(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverafter(tstzspan, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_temporal' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverafter(tpcpatch, tstzspan) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_temporal_tstzspan' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverafter(tpcbox, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverafter(tpcpatch, tpcbox) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcboxOverafter(tpcpatch, tpcpatch) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (PROCEDURE = tpcboxOverafter,
  LEFTARG = tstzspan, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #&> (PROCEDURE = tpcboxOverafter,
  LEFTARG = tpcpoint, RIGHTARG = tstzspan,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #&> (PROCEDURE = tpcboxOverafter,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #&> (PROCEDURE = tpcboxOverafter,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #&> (PROCEDURE = tpcboxOverafter,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #&> (PROCEDURE = tpcboxOverafter,
  LEFTARG = tstzspan, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #&> (PROCEDURE = tpcboxOverafter,
  LEFTARG = tpcpatch, RIGHTARG = tstzspan,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #&> (PROCEDURE = tpcboxOverafter,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #&> (PROCEDURE = tpcboxOverafter,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);
CREATE OPERATOR #&> (PROCEDURE = tpcboxOverafter,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel);

/*****************************************************************************/
