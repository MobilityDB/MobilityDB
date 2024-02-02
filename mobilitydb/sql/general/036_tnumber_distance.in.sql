/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * tnumber_distance.sql
 * Distance functions for temporal numbers.
 */

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/* integer <-> <TYPE> */

CREATE FUNCTION tnumber_distance(integer, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Distance_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = <->
);

/*****************************************************************************/

/* float <-> <TYPE> */

CREATE FUNCTION tnumber_distance(float, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = <->
);

/*****************************************************************************/
/* tint <-> <TYPE> */

CREATE FUNCTION tnumber_distance(tint, integer)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Distance_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_distance(tint, tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Distance_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = <->
);

/*****************************************************************************/
/* tfloat <-> <TYPE> */

CREATE FUNCTION tnumber_distance(tfloat, float)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnumber_distance(tfloat, tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tnumber_distance,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = <->
);

/*****************************************************************************
 * Nearest approach distance
 *****************************************************************************/

/* integer |=| <TYPE> */

CREATE FUNCTION nearestApproachDistance(integer, tint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'NAD_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = integer, RIGHTARG = tint,
  COMMUTATOR = |=|
);

/*****************************************************************************/

/* float |=| <TYPE> */

CREATE FUNCTION nearestApproachDistance(float, tfloat)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_number_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = float, RIGHTARG = tfloat,
  COMMUTATOR = |=|
);

/*****************************************************************************/

/* tbox |=| <TYPE> */

CREATE FUNCTION nearestApproachDistance(tbox, tbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tbox, tint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'NAD_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tbox, tfloat)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tbox_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tbox, RIGHTARG = tint,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tbox, RIGHTARG = tfloat,
  COMMUTATOR = |=|
);

/*****************************************************************************/
/* tint |=| <TYPE> */

CREATE FUNCTION nearestApproachDistance(tint, integer)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'NAD_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tint, tbox)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'NAD_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tint, tint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'NAD_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tint, RIGHTARG = integer,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tint, RIGHTARG = tbox,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tint, RIGHTARG = tint,
  COMMUTATOR = |=|
);

/*****************************************************************************/
/* tfloat |=| <TYPE> */

CREATE FUNCTION nearestApproachDistance(tfloat, float)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnumber_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tfloat, tbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnumber_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tfloat, tfloat)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnumber_tnumber'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tfloat, RIGHTARG = float,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tfloat, RIGHTARG = tbox,
  COMMUTATOR = |=|
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tfloat, RIGHTARG = tfloat,
  COMMUTATOR = |=|
);

/*****************************************************************************/
