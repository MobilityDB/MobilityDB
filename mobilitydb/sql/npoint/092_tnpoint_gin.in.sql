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
 * tnpoint_gin.sql
 * GIN index for temporal network points
 */

/******************************************************************************/

CREATE FUNCTION tnpoint_gin_extract_value(bigint, internal)
RETURNS internal
AS 'MODULE_PATHNAME', 'Tnpoint_gin_extract_value'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tnpoint_gin_extract_query(bigint, internal, int2, internal, internal, internal, internal)
RETURNS internal
AS 'MODULE_PATHNAME', 'Tnpoint_gin_extract_query'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS tnpoint_gin_ops
  DEFAULT FOR TYPE tnpoint USING gin AS
  STORAGE bigint,
  -- overlap
  OPERATOR  1    @@ (tnpoint, tnpoint),
  -- contains value
  OPERATOR  2    @? (tnpoint, bigint),
  -- contains set
  OPERATOR  3    @? (tnpoint, tnpoint),
  -- contained
  OPERATOR  4    ?@ (tnpoint, tnpoint),
  -- equal
  OPERATOR  5    @= (tnpoint, tnpoint),
  -- functions
  FUNCTION   2    tnpoint_gin_extract_value(bigint, internal),
  FUNCTION   3    tnpoint_gin_extract_query(bigint, internal, int2, internal, internal, internal, internal),
  FUNCTION   6    set_gin_triconsistent(internal, int2, bigint, int4, internal, internal, internal);

/******************************************************************************/
