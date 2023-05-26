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
 * geography_functions.sql
 * Spatial functions for PostGIS geography.
 */

-- Availability: 3.1.0
CREATE FUNCTION ST_LineInterpolatePoint(geography, float,
    use_spheroid boolean DEFAULT true)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'geography_line_interpolate_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE FUNCTION ST_LineInterpolatePoints(geography, float,
    use_spheroid boolean DEFAULT true, repeat boolean DEFAULT true)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'geography_line_interpolate_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE FUNCTION ST_LineLocatePoint(geography, geography,
    use_spheroid boolean DEFAULT true)
  RETURNS float
  AS 'MODULE_PATHNAME', 'geography_line_locate_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE FUNCTION ST_LineSubstring(geography, float, float)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'geography_line_substring'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-------------------------------------------------------------------------

-- Availability: 3.1.0
CREATE FUNCTION ST_ClosestPoint(geography, geography,
    use_spheroid boolean DEFAULT true)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'geography_closestpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE FUNCTION ST_ShortestLine(geography, geography,
    use_spheroid boolean DEFAULT true)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'geography_shortestline'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
