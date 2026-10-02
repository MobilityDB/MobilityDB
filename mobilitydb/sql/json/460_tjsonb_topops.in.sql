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
 * @brief Topological operators for temporal JSONB
 */

/*****************************************************************************
 * Contains
 *****************************************************************************/


CREATE FUNCTION spanContains(tstzspan, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzspan_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanContains(tjsonb, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_tstzspan'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanContains(tjsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_temporal_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #@> (
  PROCEDURE = spanContains,
  LEFTARG = tstzspan, RIGHTARG = tjsonb,
  COMMUTATOR = <@#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #@> (
  PROCEDURE = spanContains,
  LEFTARG = tjsonb, RIGHTARG = tstzspan,
  COMMUTATOR = <@#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR #@> (
  PROCEDURE = spanContains,
  LEFTARG = tjsonb, RIGHTARG = tjsonb,
  COMMUTATOR = <@#,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * Contained
 *****************************************************************************/

CREATE FUNCTION spanContained(tstzspan, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzspan_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanContained(tjsonb, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_tstzspan'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanContained(tjsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_temporal_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@# (
  PROCEDURE = spanContained,
  LEFTARG = tstzspan, RIGHTARG = tjsonb,
  COMMUTATOR = #@>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@# (
  PROCEDURE = spanContained,
  LEFTARG = tjsonb, RIGHTARG = tstzspan,
  COMMUTATOR = #@>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR <@# (
  PROCEDURE = spanContained,
  LEFTARG = tjsonb, RIGHTARG = tjsonb,
  COMMUTATOR = #@>,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

CREATE FUNCTION spanOverlaps(tstzspan, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzspan_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverlaps(tjsonb, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_tstzspan'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverlaps(tjsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_temporal_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = spanOverlaps,
  LEFTARG = tstzspan, RIGHTARG = tjsonb,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = spanOverlaps,
  LEFTARG = tjsonb, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = spanOverlaps,
  LEFTARG    = tjsonb, RIGHTARG = tjsonb,
  COMMUTATOR = &&,
  RESTRICT   = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * Same
 *****************************************************************************/

CREATE FUNCTION spanSame(tstzspan, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzspan_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanSame(tjsonb, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_tstzspan'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanSame(tjsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_temporal_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = spanSame,
  LEFTARG = tstzspan, RIGHTARG = tjsonb,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = spanSame,
  LEFTARG = tjsonb, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = spanSame,
  LEFTARG    = tjsonb, RIGHTARG = tjsonb,
  COMMUTATOR = ~=,
  RESTRICT   = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************
 * Adjacent
 *****************************************************************************/

CREATE FUNCTION spanAdjacent(tstzspan, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzspan_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAdjacent(tjsonb, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_tstzspan'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAdjacent(tjsonb, tjsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_temporal_temporal'
  SUPPORT temporal_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = tstzspan, RIGHTARG = tjsonb,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = tjsonb, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = temporal_sel, JOIN = temporal_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG    = tjsonb, RIGHTARG = tjsonb,
  COMMUTATOR = -|-,
  RESTRICT   = temporal_sel, JOIN = temporal_joinsel
);

/*****************************************************************************/
