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
 * @brief Spatial and spatiotemporal grid tiling for temporal rigid geometries
 */

CREATE FUNCTION spaceBoxes(trgeometry, xsize float, ysize float, zsize float,
    sorigin geometry DEFAULT 'Point(0 0 0)', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Trgeometry_space_boxes'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceBoxes(trgeometry, xsize float,
    sorigin geometry DEFAULT 'Point(0 0 0)', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS stbox[]
  AS 'SELECT @extschema@.spaceBoxes($1, $2, $2, $2, $3, $4)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceBoxes(trgeometry, xsize float, ysize float,
    sorigin geometry DEFAULT 'Point(0 0 0)', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS stbox[]
  AS 'SELECT @extschema@.spaceBoxes($1, $2, $3, $2, $4, $5)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

CREATE FUNCTION timeBoxes(trgeometry, interval,
    torigin timestamptz DEFAULT '2000-01-03', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Trgeo_time_boxes'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

CREATE FUNCTION spaceTimeBoxes(trgeometry, xsize float, ysize float,
    zsize float, interval, sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Trgeometry_space_time_boxes'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceTimeBoxes(trgeometry, xsize float, interval,
    sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS stbox[]
  AS 'SELECT @extschema@.spaceTimeBoxes($1, $2, $2, $2, $3, $4, $5, $6)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceTimeBoxes(trgeometry, xsize float, ysize float, interval,
    sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS stbox[]
  AS 'SELECT @extschema@.spaceTimeBoxes($1, $2, $3, $2, $4, $5, $6, $7)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/
