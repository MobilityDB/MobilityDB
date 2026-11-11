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
 * @file mobilitydb--1.3.1--1.3.2.sql
 * @brief Upgrade of the MobilityDB extension from version 1.3.1 to 1.3.2
 * @details The intersection of a date span set and a date returns a date span
 * set, as its commutator does. A function cannot change its result type in
 * place, so the operator and the function are dropped and created again; the
 * operator names its commutator, which links the two operators back.
 */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "ALTER EXTENSION mobilitydb UPDATE TO '1.3.2'" to load this file. \quit

DROP OPERATOR * (datespanset, date);
DROP FUNCTION span_intersection(datespanset, date);

CREATE FUNCTION span_intersection(datespanset, date)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = datespanset, RIGHTARG = date,
  COMMUTATOR = *
);
