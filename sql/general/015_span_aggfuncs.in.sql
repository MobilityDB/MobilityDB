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
 * time_aggfuncs.sql
 * Aggregate functions for time types
 */

/*****************************************************************************/

CREATE FUNCTION span_extent_transfn(intspan, intspan)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_combinefn(intspan, intspan)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION span_extent_transfn(floatspan, floatspan)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_combinefn(floatspan, floatspan)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION span_extent_transfn(period, period)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Span_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_combinefn(period, period)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Span_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(intspan) (
  SFUNC = span_extent_transfn,
  STYPE = intspan,
  COMBINEFUNC = span_extent_combinefn
  -- , PARALLEL = safe
);
CREATE AGGREGATE extent(floatspan) (
  SFUNC = span_extent_transfn,
  STYPE = floatspan,
  COMBINEFUNC = span_extent_combinefn
  -- , PARALLEL = safe
);
CREATE AGGREGATE extent(period) (
  SFUNC = span_extent_transfn,
  STYPE = period,
  COMBINEFUNC = span_extent_combinefn
  -- , PARALLEL = safe
);

/*****************************************************************************/
