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
 * @brief PostGIS raster sampling along tgeompoint trajectories.
 *
 * `raster_value(raster, tgeompoint, band integer DEFAULT 1) → tfloat`
 * evaluates a raster band at each instant of a trajectory and returns
 * the sampled values as a `tfloat` (instant set). Instants that fall
 * outside the raster extent or land on a nodata pixel are silently
 * dropped; NULL is returned when no instants survive.
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
