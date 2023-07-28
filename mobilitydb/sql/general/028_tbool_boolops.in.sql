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

/*
 * temporal_boolops.sql
 * Temporal Boolean function and operators.
 */

/*****************************************************************************
 * Temporal and
 *****************************************************************************/

CREATE FUNCTION tbool_and(boolean, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tand_bool_tbool'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbool_and(tbool, boolean)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tand_tbool_bool'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbool_and(tbool, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tand_tbool_tbool'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR & (
  PROCEDURE = tbool_and,
  LEFTARG = boolean, RIGHTARG = tbool,
  COMMUTATOR = &
);
CREATE OPERATOR & (
  PROCEDURE = tbool_and,
  LEFTARG = tbool, RIGHTARG = boolean,
  COMMUTATOR = &
);
CREATE OPERATOR & (
  PROCEDURE = tbool_and,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = &
);

/*****************************************************************************
 * Temporal or
 *****************************************************************************/

CREATE FUNCTION tbool_or(boolean, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tor_bool_tbool'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbool_or(tbool, boolean)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tor_tbool_bool'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbool_or(tbool, tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tor_tbool_tbool'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR | (
  PROCEDURE = tbool_or,
  LEFTARG = boolean, RIGHTARG = tbool,
  COMMUTATOR = |
);
CREATE OPERATOR | (
  PROCEDURE = tbool_or,
  LEFTARG = tbool, RIGHTARG = boolean,
  COMMUTATOR = |
);
CREATE OPERATOR | (
  PROCEDURE = tbool_or,
  LEFTARG = tbool, RIGHTARG = tbool,
  COMMUTATOR = |
);

/*****************************************************************************
 * Temporal not
 *****************************************************************************/

CREATE FUNCTION tbool_not(tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tnot_tbool'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~ (
  PROCEDURE = tbool_not, RIGHTARG = tbool
);

/*****************************************************************************
 * Temporal when
 *****************************************************************************/

-- when is a reserved word in SQL
CREATE FUNCTION whenTrue(tbool)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Tbool_when'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
