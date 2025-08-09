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
 * @brief Temporal JSONB functions
 */

/*****************************************************************************
 * Temporal JSONB concatenation
 *****************************************************************************/

CREATE FUNCTION tjsonb_concat(jsonb, tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Jsonb_concat_jsonb_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_concat(tjsonb, jsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Jsonb_concat_tjsonb_jsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_concat(tjsonb, tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Jsonb_concat_tjsonb_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR || (
  PROCEDURE = tjsonb_concat,
  LEFTARG   = jsonb, RIGHTARG = tjsonb
);
CREATE OPERATOR || (
  PROCEDURE = tjsonb_concat,
  LEFTARG   = tjsonb, RIGHTARG = jsonb
);
CREATE OPERATOR || (
  PROCEDURE = tjsonb_concat,
  LEFTARG   = tjsonb, RIGHTARG = tjsonb
);


CREATE FUNCTION tjsonb_set_path(temp tjsonb, path text[], val jsonb, create_missing boolean DEFAULT true)
RETURNS tjsonb
AS 'MODULE_PATHNAME', 'Tjsonb_set_path'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
