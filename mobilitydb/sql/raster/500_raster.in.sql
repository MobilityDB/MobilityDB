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
 * @file
 * @brief PostGIS raster and Raquet raster sampling along tgeompoint
 * trajectories.
 *
 * Sampling functions:
 *   raster_value(raster, tgeompoint, band integer DEFAULT 1) → tfloat
 *   raster_tile_value_quadbin(bytea, ..., tgeompoint) → tfloat
 *   trajectory_quadbins(tgeompoint, integer) → bigint[]
 *
 * Restriction functions (SQL-defined, compose the sampling operators):
 *   atRasterValue(tgeompoint, raster, floatspan, band DEFAULT 1) → tgeompoint
 *   minusRasterValue(tgeompoint, raster, floatspan, band DEFAULT 1) → tgeompoint
 *
 * Ever/always predicates (SQL-defined):
 *   eRasterValue(raster, tgeompoint, floatspan, band DEFAULT 1) → boolean
 *   aRasterValue(raster, tgeompoint, floatspan, band DEFAULT 1) → boolean
 *
 * This file is compiled into the mobilitydb extension only when
 * MobilityDB is built with `-DRASTER=ON`; the generated
 * `mobilitydb.control` then declares `requires = '...postgis_raster'`
 * so the extension stack is created in a single CASCADE:
 *
 *   CREATE EXTENSION mobilitydb CASCADE;
 */

/******************************************************************************
 * raster_value
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return the values of a raster band sampled at the instants of a
 * trajectory
 * @param[in] rast Raster
 * @param[in] traj Trajectory
 * @param[in] band Band number (1-based, default 1)
 * @csqlfn #raster_value()
 */
CREATE OR REPLACE FUNCTION raster_value(
    rast  raster,
    traj  tgeompoint,
    band  integer DEFAULT 1
) RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Raster_value'
  LANGUAGE C STRICT;

/******************************************************************************
 * raster_tile_value_quadbin
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return the values of a Raquet raster chip sampled at the instants of
 * a trajectory, using a QUADBIN cell to determine the tile georeferencing
 * @param[in] pixels    Row-major pixel bytes
 * @param[in] width     Tile width in pixels
 * @param[in] height    Tile height in pixels
 * @param[in] quadbin   CARTO QUADBIN cell identifier
 * @param[in] pixtype   Pixel type: UINT8 | INT16 | INT32 | FLOAT32 | FLOAT64
 * @param[in] nodata    Nodata sentinel value
 * @param[in] has_nodata Enable nodata filtering
 * @param[in] traj      Trajectory (SRID 4326)
 * @csqlfn #Raster_tile_value_quadbin()
 */
CREATE OR REPLACE FUNCTION raster_tile_value_quadbin(
    pixels     bytea,
    width      integer,
    height     integer,
    quadbin    bigint,
    pixtype    text,
    nodata     float8,
    has_nodata boolean,
    traj       tgeompoint
) RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Raster_tile_value_quadbin'
  LANGUAGE C STRICT;

/******************************************************************************
 * trajectory_quadbins
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return the distinct QUADBIN cells at a zoom level covered by a
 * trajectory, suitable as a WHERE-clause join key against a Raquet table
 * @param[in] traj  Trajectory (SRID 4326)
 * @param[in] zoom  QUADBIN zoom level (0–15)
 * @csqlfn #Trajectory_quadbins()
 */
CREATE OR REPLACE FUNCTION trajectory_quadbins(
    traj  tgeompoint,
    zoom  integer
) RETURNS bigint[]
  AS 'MODULE_PATHNAME', 'Trajectory_quadbins'
  LANGUAGE C STRICT;

/******************************************************************************
 * atRasterValue
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return the instants of a trajectory where the sampled raster pixel
 * value falls inside a float range
 * @param[in] traj  Trajectory (SRID matching the raster)
 * @param[in] rast  Raster
 * @param[in] vspan Float value range (inclusive bounds)
 * @param[in] band  Band number (1-based, default 1)
 * @csqlfn #atRasterValue()
 */
CREATE OR REPLACE FUNCTION atRasterValue(
    traj  tgeompoint,
    rast  raster,
    vspan floatspan,
    band  integer DEFAULT 1
) RETURNS tgeompoint AS $$
  SELECT atTime($1, getTime(atValues(v, $3)))
  FROM (SELECT raster_value($2, $1, $4) AS v) t
$$ LANGUAGE SQL STRICT;

/******************************************************************************
 * minusRasterValue
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return the instants of a trajectory where the sampled raster pixel
 * value falls outside a float range
 * @param[in] traj  Trajectory (SRID matching the raster)
 * @param[in] rast  Raster
 * @param[in] vspan Float value range to exclude
 * @param[in] band  Band number (1-based, default 1)
 * @csqlfn #minusRasterValue()
 */
CREATE OR REPLACE FUNCTION minusRasterValue(
    traj  tgeompoint,
    rast  raster,
    vspan floatspan,
    band  integer DEFAULT 1
) RETURNS tgeompoint AS $$
  SELECT atTime($1, getTime(minusValues(v, $3)))
  FROM (SELECT raster_value($2, $1, $4) AS v) t
$$ LANGUAGE SQL STRICT;

/******************************************************************************
 * eRasterValue
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return true if the trajectory ever samples a raster pixel value
 * inside a float range
 * @param[in] rast  Raster
 * @param[in] traj  Trajectory (SRID matching the raster)
 * @param[in] vspan Float value range
 * @param[in] band  Band number (1-based, default 1)
 * @csqlfn #eRasterValue()
 */
CREATE OR REPLACE FUNCTION eRasterValue(
    rast  raster,
    traj  tgeompoint,
    vspan floatspan,
    band  integer DEFAULT 1
) RETURNS boolean AS $$
  SELECT atValues(raster_value($1, $2, $4), $3) IS NOT NULL
$$ LANGUAGE SQL STRICT;

/******************************************************************************
 * aRasterValue
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return true if every in-raster-extent instant of the trajectory
 * samples a pixel value inside a float range
 * @param[in] rast  Raster
 * @param[in] traj  Trajectory (SRID matching the raster)
 * @param[in] vspan Float value range
 * @param[in] band  Band number (1-based, default 1)
 * @csqlfn #aRasterValue()
 */
CREATE OR REPLACE FUNCTION aRasterValue(
    rast  raster,
    traj  tgeompoint,
    vspan floatspan,
    band  integer DEFAULT 1
) RETURNS boolean AS $$
  SELECT v IS NOT NULL AND minusValues(v, $3) IS NULL
  FROM (SELECT raster_value($1, $2, $4) AS v) t
$$ LANGUAGE SQL STRICT;
