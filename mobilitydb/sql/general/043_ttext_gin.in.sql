/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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

/*
 * ttext_gin.sql
 * GIN index for temporal text
 */

/******************************************************************************/

CREATE FUNCTION ttext_gin_extract_value(text, internal)
RETURNS internal
AS 'MODULE_PATHNAME', 'Ttext_gin_extract_value'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ttext_gin_extract_query(text, internal, int2, internal, internal, internal, internal)
RETURNS internal
AS 'MODULE_PATHNAME', 'Ttext_gin_extract_query'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ttext_gin_consistent(internal, int2, text, int4, internal, internal, internal, internal)
RETURNS bool
AS 'MODULE_PATHNAME', 'Ttext_gin_consistent'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ttext_gin_triconsistent(internal, int2, text, int4, internal, internal, internal)
RETURNS "char"
AS 'MODULE_PATHNAME', 'Ttext_gin_triconsistent'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS ttext_gin_ops
  DEFAULT FOR TYPE ttext USING gin AS
  STORAGE text,
  -- overlaps
  OPERATOR  3    && (ttext, ttext),
    -- same
  OPERATOR  6    = (ttext, ttext),
  -- contains
  -- OPERATOR  7    @> (ttext, text),
  OPERATOR  7    @> (ttext, ttext),
  -- contained by
  -- OPERATOR  8    <@ (text, ttext),
  OPERATOR  8    <@ (ttext, ttext),
  -- functions
  FUNCTION   2    ttext_gin_extract_value(text, internal),
  FUNCTION   3    ttext_gin_extract_query(text, internal, int2, internal, internal, internal, internal),
  FUNCTION   4    ttext_gin_consistent(internal, int2, text, int4, internal, internal, internal, internal),
  FUNCTION   6    ttext_gin_triconsistent(internal, int2, text, int4, internal, internal, internal);

/******************************************************************************/
