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
 * @brief Similarity distance functions for temporal rigid geometries
 * @note The reference-point centroid trajectory is used as a proxy for pose
 * distance, consistent with all other trgeometry distance functions
 */

/*****************************************************************************
 * Frechet distance and path
 *****************************************************************************/

CREATE FUNCTION frechetDistance(trgeometry, trgeometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Trgeometry_frechet_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION frechetDistancePath(trgeometry, trgeometry)
  RETURNS SETOF warp
  AS 'MODULE_PATHNAME', 'Trgeometry_frechet_path'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Dynamic Time Warp distance and path
 *****************************************************************************/

CREATE FUNCTION dynTimeWarpDistance(trgeometry, trgeometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Trgeometry_dyntimewarp_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION dynTimeWarpPath(trgeometry, trgeometry)
  RETURNS SETOF warp
  AS 'MODULE_PATHNAME', 'Trgeometry_dyntimewarp_path'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Hausdorff distance
 *****************************************************************************/

CREATE FUNCTION hausdorffDistance(trgeometry, trgeometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Trgeometry_hausdorff_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
