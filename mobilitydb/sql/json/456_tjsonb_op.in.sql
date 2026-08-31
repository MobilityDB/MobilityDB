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
 * @brief Operators for temporal JSONB
 */

/*****************************************************************************
 * Exists
 *****************************************************************************/

CREATE FUNCTION tjsonbExists(tjsonb, text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tjsonb_exists'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbExistsAny(tjsonb, text[])
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tjsonb_exists_any'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbExistsAll(tjsonb, text[])
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tjsonb_exists_all'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ? (
  PROCEDURE = tjsonbExists,
  LEFTARG   = tjsonb, RIGHTARG = text
);
CREATE OPERATOR ?| (
  PROCEDURE = tjsonbExistsAny,
  LEFTARG   = tjsonb, RIGHTARG = text[]
);
CREATE OPERATOR ?& (
  PROCEDURE = tjsonbExistsAll,
  LEFTARG   = tjsonb, RIGHTARG = text[]
);

/*****************************************************************************
 * Contains
 *****************************************************************************/

CREATE FUNCTION tjsonbContains(jsonb, tjsonb)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Contains_jsonb_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbContains(tjsonb, jsonb)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Contains_tjsonb_jsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbContains(tjsonb, tjsonb)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Contains_tjsonb_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = tjsonbContains,
  LEFTARG   = jsonb, RIGHTARG = tjsonb,
  COMMUTATOR = <@
);
CREATE OPERATOR @> (
  PROCEDURE = tjsonbContains,
  LEFTARG   = tjsonb, RIGHTARG = jsonb,
  COMMUTATOR = <@
);
CREATE OPERATOR @> (
  PROCEDURE = tjsonbContains,
  LEFTARG   = tjsonb, RIGHTARG = tjsonb,
  COMMUTATOR = <@
);

/*****************************************************************************
 * Contained
 *****************************************************************************/

CREATE FUNCTION tjsonbContained(jsonb, tjsonb)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Contained_jsonb_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbContained(tjsonb, jsonb)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Contained_tjsonb_jsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonbContained(tjsonb, tjsonb)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Contained_tjsonb_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = tjsonbContained,
  LEFTARG   = jsonb, RIGHTARG = tjsonb,
  COMMUTATOR = @>
);
CREATE OPERATOR <@ (
  PROCEDURE = tjsonbContained,
  LEFTARG   = tjsonb, RIGHTARG = jsonb,
  COMMUTATOR = @>
);
CREATE OPERATOR <@ (
  PROCEDURE = tjsonbContained,
  LEFTARG   = tjsonb, RIGHTARG = tjsonb,
  COMMUTATOR = @>
);

/*****************************************************************************/

