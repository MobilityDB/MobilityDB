/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * doublen.sql
 * Internal types used for the temporal average and centroid aggregates.
 */

CREATE TYPE double2;
CREATE TYPE double3;
CREATE TYPE double4;

CREATE FUNCTION double2_in(cstring)
  RETURNS double2
   AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double2_out(double2)
  RETURNS cstring
   AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double2_send(double2)
  RETURNS bytea
   AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double2_recv(internal)
  RETURNS double2
   AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE double2 (
  internallength = 16,
  input = double2_in,
  output = double2_out,
  send = double2_send,
  receive = double2_recv,
  alignment = double
);

/******************************************************************************/

CREATE FUNCTION double3_in(cstring)
  RETURNS double3
   AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double3_out(double3)
  RETURNS cstring
   AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double3_send(double3)
  RETURNS bytea
   AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double3_recv(internal)
  RETURNS double3
   AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE double3 (
  internallength = 24,
  input = double3_in,
  output = double3_out,
  send = double3_send,
  receive = double3_recv,
  alignment = double
);

/******************************************************************************/

CREATE FUNCTION double4_in(cstring)
  RETURNS double4
   AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double4_out(double4)
  RETURNS cstring
   AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double4_send(double4)
  RETURNS bytea
   AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double4_recv(internal)
  RETURNS double4
   AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE double4 (
  internallength = 32,
  input = double4_in,
  output = double4_out,
  send = double4_send,
  receive = double4_recv,
  alignment = double
);

/******************************************************************************
 * Catalog
 ******************************************************************************/

CREATE TYPE tdouble2;
CREATE TYPE tdouble3;
CREATE TYPE tdouble4;

/* temporal, base, contbase, box */
SELECT register_temporal_type('tdouble2', 'double2', true, '');
SELECT register_temporal_type('tdouble3', 'double3', true, '');
SELECT register_temporal_type('tdouble4', 'double4', true, '');

/******************************************************************************/

CREATE FUNCTION tdouble2_in(cstring, oid, integer)
  RETURNS tdouble2
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_out(tdouble2)
  RETURNS cstring
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE tdouble2 (
  internallength = variable,
  input = tdouble2_in,
  output = temporal_out,
  alignment = double
);

/******************************************************************************/

CREATE FUNCTION tdouble3_in(cstring, oid, integer)
  RETURNS tdouble3
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_out(tdouble3)
  RETURNS cstring
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE tdouble3 (
  internallength = variable,
  input = tdouble3_in,
  output = temporal_out,
  alignment = double
);

/******************************************************************************/

CREATE FUNCTION tdouble4_in(cstring, oid, integer)
  RETURNS tdouble4
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_out(tdouble4)
  RETURNS cstring
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE tdouble4 (
  internallength = variable,
  input = tdouble4_in,
  output = temporal_out,
  alignment = double
);

/******************************************************************************/
