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
 *   rasterValue(tgeompoint, raster, band integer DEFAULT 1,
 *     exclude_nodata_value boolean DEFAULT true,
 *     resample text DEFAULT 'nearest') → tfloat
 *   rasterTileValueQuadbin(tgeompoint, bytea, ...) → tfloat
 *   quadbins(tgeompoint, integer) → bigint[]
 *
 * Restriction functions (SQL-defined, compose the sampling operators):
 *   atRasterValue(tgeompoint, raster, floatspan, band DEFAULT 1) → tgeompoint
 *   minusRasterValue(tgeompoint, raster, floatspan, band DEFAULT 1) → tgeompoint
 *
 * Ever/always predicates (SQL-defined):
 *   eRasterValue(tgeompoint, raster, floatspan, band DEFAULT 1) → boolean
 *   aRasterValue(tgeompoint, raster, floatspan, band DEFAULT 1) → boolean
 *
 * rasterValue and the four functions above also take the path of a raster
 * file on the server in place of the raster, which GDAL reads.
 *
 * This file is compiled into the mobilitydb extension only when
 * MobilityDB is built with `-DRASTER=ON`; the generated
 * `mobilitydb.control` then declares `requires = '...postgis_raster'`
 * so the extension stack is created in a single CASCADE:
 *
 *   CREATE EXTENSION mobilitydb CASCADE;
 */

/******************************************************************************
 * raquet type: a GDAL-free, self-describing Web-Mercator raster tile
 * identified by a QUADBIN cell and carrying a row-major packed pixel array
 ******************************************************************************/

CREATE TYPE raquet;

CREATE FUNCTION raquet_in(cstring)
  RETURNS raquet
  AS 'MODULE_PATHNAME', 'Raquet_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION raquet_out(raquet)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Raquet_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION raquet_recv(internal)
  RETURNS raquet
  AS 'MODULE_PATHNAME', 'Raquet_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION raquet_send(raquet)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Raquet_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE raquet (
  internallength = variable,
  input = raquet_in,
  output = raquet_out,
  receive = raquet_recv,
  send = raquet_send,
  storage = extended,
  alignment = double
);

-- GENERATED-REPRESENTATIONS-BEGIN raquet_base — tools/codegen/inherited/generate.py from templates/representations.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/representation_families.yaml and re-run.
/******************************************************************************
 * Well-Known Binary representations
 *
 * The tile carries its pixels and its QUADBIN georeferencing in one value, so
 * these round-trip a tile through a portable byte string with no spatial
 * extension involved, as the sibling h3index cell does.
 ******************************************************************************/

CREATE FUNCTION raquetFromBinary(bytea)
  RETURNS raquet
  AS 'MODULE_PATHNAME', 'Raquet_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION raquetFromHexWKB(text)
  RETURNS raquet
  AS 'MODULE_PATHNAME', 'Raquet_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(raquet, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Raquet_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(raquet, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Raquet_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- GENERATED-REPRESENTATIONS-END raquet_base

/******************************************************************************
 * raquet constructor
 ******************************************************************************/

CREATE FUNCTION raquet(pixels bytea, width integer, height integer,
    quadbin bigint, pixtype text, nodata float8 DEFAULT NULL)
  RETURNS raquet
  AS 'MODULE_PATHNAME', 'Raquet_constructor'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return a Raquet tile decoded from an in-memory raster file via GDAL
 * @param[in] rasterfile Raster file bytes in any GDAL-supported format
 * @param[in] quadbin CARTO QUADBIN cell, or NULL to derive it from the raster
 * geotransform and EPSG:3857 spatial reference
 */
CREATE FUNCTION raquetRead(
    rasterfile bytea,
    quadbin    bigint DEFAULT NULL
) RETURNS raquet
  AS 'MODULE_PATHNAME', 'Raquet_read_bytes'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return a Raquet tile read from a raster file on the server via GDAL
 * @param[in] path Path of a raster file on the server in any GDAL-supported
 * format
 * @param[in] quadbin CARTO QUADBIN cell, or NULL to derive it from the raster
 * geotransform and EPSG:3857 spatial reference
 */
CREATE FUNCTION raquetRead(
    path    text,
    quadbin bigint DEFAULT NULL
) RETURNS raquet
  AS 'MODULE_PATHNAME', 'Raquet_read'
  LANGUAGE C PARALLEL SAFE;

/******************************************************************************
 * rasterValue
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return the values of a raster band read along a trajectory
 * @param[in] traj Trajectory
 * @param[in] rast Raster
 * @param[in] band Band number (1-based, default 1)
 * @param[in] exclude_nodata_value True to leave out the positions over a
 * nodata pixel (default true)
 * @param[in] resample Name of the read of a position, nearest or bilinear
 * (default nearest)
 */
CREATE OR REPLACE FUNCTION rasterValue(
    traj  tgeompoint,
    rast  raster,
    band  integer DEFAULT 1,
    exclude_nodata_value boolean DEFAULT true,
    resample text DEFAULT 'nearest'
) RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Raster_value'
  LANGUAGE C STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return the values of a band of a raster file on the server read
 * along a trajectory
 * @param[in] traj Trajectory
 * @param[in] path Path of a raster file on the server in any GDAL-supported
 * format
 * @param[in] band Band number (1-based, default 1)
 */
CREATE OR REPLACE FUNCTION rasterValue(
    traj  tgeompoint,
    path  text,
    band  integer DEFAULT 1
) RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Raster_value_gdal'
  LANGUAGE C STRICT PARALLEL SAFE;

/******************************************************************************
 * rasterTileValueQuadbin
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return the values of a Raquet raster chip read along a trajectory,
 * using a QUADBIN cell to determine the tile georeferencing
 * @param[in] traj Trajectory (SRID 4326)
 * @param[in] pixels Row-major pixel bytes
 * @param[in] width Tile width in pixels
 * @param[in] height Tile height in pixels
 * @param[in] quadbin CARTO QUADBIN cell identifier
 * @param[in] pixtype Pixel type: uint8, int8, uint16, int16, uint32, int32, uint64, int64, float16, float32, or float64
 * @param[in] nodata Nodata sentinel value
 * @param[in] has_nodata Enable nodata filtering
 */
CREATE OR REPLACE FUNCTION rasterTileValueQuadbin(
    traj       tgeompoint,
    pixels     bytea,
    width      integer,
    height     integer,
    quadbin    bigint,
    pixtype    text,
    nodata     float8,
    has_nodata boolean
) RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Raster_tile_value_quadbin'
  LANGUAGE C STRICT PARALLEL SAFE;

/******************************************************************************
 * rasterTileValue
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Read a raquet raster tile along a trajectory
 * @param[in] traj Trajectory
 * @param[in] rast Raquet tile
 */
CREATE FUNCTION rasterTileValue(
    traj tgeompoint,
    rast raquet
) RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Raster_tile_value'
  LANGUAGE C STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Read an array of raquet raster tiles along a trajectory, keeping
 * the value of the tile of highest zoom where tiles overlap
 * @param[in] traj Trajectory
 * @param[in] rast Array of raquet tiles
 */
CREATE FUNCTION rasterTileValue(
    traj tgeompoint,
    rast raquet[]
) RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Raster_tile_value_array'
  LANGUAGE C STRICT PARALLEL SAFE;

/******************************************************************************
 * quadbins
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return the distinct QUADBIN cells at a zoom level covered by a
 * trajectory, suitable as a WHERE-clause join key against a Raquet table
 * @param[in] traj Trajectory (SRID 4326)
 * @param[in] zoom  QUADBIN zoom level (0–15)
 */
CREATE OR REPLACE FUNCTION quadbins(
    traj  tgeompoint,
    zoom  integer
) RETURNS bigint[]
  AS 'MODULE_PATHNAME', 'Trajectory_quadbins'
  LANGUAGE C STRICT PARALLEL SAFE;

/******************************************************************************
 * atRasterValue
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return the instants of a trajectory where the sampled raster pixel
 * value falls inside a float range
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] rast Raster
 * @param[in] vspan Float value range (inclusive bounds)
 * @param[in] band Band number (1-based, default 1)
 */
CREATE OR REPLACE FUNCTION atRasterValue(traj tgeompoint, rast raster,
    vspan floatspan, band integer DEFAULT 1)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Raster_at_value'
  LANGUAGE C STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return the instants of a trajectory where the value it reads from a
 * raster file on the server falls inside a float range
 * @param[in] traj Trajectory
 * @param[in] path Path of a raster file on the server in any GDAL-supported
 * format
 * @param[in] vspan Float value range (inclusive bounds)
 * @param[in] band Band number (1-based, default 1)
 */
CREATE OR REPLACE FUNCTION atRasterValue(traj tgeompoint, path text,
    vspan floatspan, band integer DEFAULT 1)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Raster_at_value_gdal'
  LANGUAGE C STRICT PARALLEL SAFE;

/******************************************************************************
 * minusRasterValue
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return the instants of a trajectory where the sampled raster pixel
 * value falls outside a float range
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] rast Raster
 * @param[in] vspan Float value range to exclude
 * @param[in] band Band number (1-based, default 1)
 */
CREATE OR REPLACE FUNCTION minusRasterValue(traj tgeompoint, rast raster,
    vspan floatspan, band integer DEFAULT 1)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Raster_minus_value'
  LANGUAGE C STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return the instants of a trajectory where the value it reads from a
 * raster file on the server falls outside a float range
 * @param[in] traj Trajectory
 * @param[in] path Path of a raster file on the server in any GDAL-supported
 * format
 * @param[in] vspan Float value range to exclude
 * @param[in] band Band number (1-based, default 1)
 */
CREATE OR REPLACE FUNCTION minusRasterValue(traj tgeompoint, path text,
    vspan floatspan, band integer DEFAULT 1)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Raster_minus_value_gdal'
  LANGUAGE C STRICT PARALLEL SAFE;

/******************************************************************************
 * eRasterValue
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return true if the trajectory ever samples a raster pixel value
 * inside a float range
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] rast Raster
 * @param[in] vspan Float value range
 * @param[in] band Band number (1-based, default 1)
 */
CREATE OR REPLACE FUNCTION eRasterValue(traj tgeompoint, rast raster,
    vspan floatspan, band integer DEFAULT 1)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eraster_value'
  LANGUAGE C STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return true if the trajectory ever reads a value inside a float range
 * from a raster file on the server
 * @param[in] traj Trajectory
 * @param[in] path Path of a raster file on the server in any GDAL-supported
 * format
 * @param[in] vspan Float value range
 * @param[in] band Band number (1-based, default 1)
 */
CREATE OR REPLACE FUNCTION eRasterValue(traj tgeompoint, path text,
    vspan floatspan, band integer DEFAULT 1)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eraster_value_gdal'
  LANGUAGE C STRICT PARALLEL SAFE;

/******************************************************************************
 * aRasterValue
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return true if every in-raster-extent instant of the trajectory
 * samples a pixel value inside a float range
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] rast Raster
 * @param[in] vspan Float value range
 * @param[in] band Band number (1-based, default 1)
 */
CREATE OR REPLACE FUNCTION aRasterValue(traj tgeompoint, rast raster,
    vspan floatspan, band integer DEFAULT 1) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Araster_value'
  LANGUAGE C STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return true if every value the trajectory reads from a raster file on
 * the server falls inside a float range
 * @param[in] traj Trajectory
 * @param[in] path Path of a raster file on the server in any GDAL-supported
 * format
 * @param[in] vspan Float value range
 * @param[in] band Band number (1-based, default 1)
 */
CREATE OR REPLACE FUNCTION aRasterValue(traj tgeompoint, path text,
    vspan floatspan, band integer DEFAULT 1) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Araster_value_gdal'
  LANGUAGE C STRICT PARALLEL SAFE;

/******************************************************************************
 * numBands
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return the number of bands of a raster
 * @param[in] rast Raster
 */
CREATE FUNCTION numBands(raster)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Raster_num_bands'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Shape of a raster
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return the width in pixels of a raster
 * @param[in] rast Raster
 */
CREATE FUNCTION width(raster)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Raster_width'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return the height in pixels of a raster
 * @param[in] rast Raster
 */
CREATE FUNCTION height(raster)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Raster_height'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return the spatial reference system identifier of a raster
 * @param[in] rast Raster
 */
CREATE FUNCTION SRID(raster)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Raster_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return the X coordinate of the upper left corner of a raster
 * @param[in] rast Raster
 */
CREATE FUNCTION upperLeftX(raster)
  RETURNS float8
  AS 'MODULE_PATHNAME', 'Raster_upper_left_x'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return the Y coordinate of the upper left corner of a raster
 * @param[in] rast Raster
 */
CREATE FUNCTION upperLeftY(raster)
  RETURNS float8
  AS 'MODULE_PATHNAME', 'Raster_upper_left_y'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return the pixel width of a raster, that is, the X component of its
 * scale
 * @param[in] rast Raster
 */
CREATE FUNCTION scaleX(raster)
  RETURNS float8
  AS 'MODULE_PATHNAME', 'Raster_scale_x'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return the pixel height of a raster, that is, the Y component of its
 * scale
 * @param[in] rast Raster
 */
CREATE FUNCTION scaleY(raster)
  RETURNS float8
  AS 'MODULE_PATHNAME', 'Raster_scale_y'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return the X component of the skew of a raster
 * @param[in] rast Raster
 */
CREATE FUNCTION skewX(raster)
  RETURNS float8
  AS 'MODULE_PATHNAME', 'Raster_skew_x'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return the Y component of the skew of a raster
 * @param[in] rast Raster
 */
CREATE FUNCTION skewY(raster)
  RETURNS float8
  AS 'MODULE_PATHNAME', 'Raster_skew_y'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Bands of a raster
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return the name of the pixel data type of a raster band
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 */
CREATE FUNCTION bandPixelType(raster, integer DEFAULT 1)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Raster_band_pixel_type'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return whether a raster band states a nodata value
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 */
CREATE FUNCTION bandHasNoDataValue(raster, integer DEFAULT 1)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Raster_band_has_nodata_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return the nodata value of a raster band, or NULL when the band
 * states none
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 */
CREATE FUNCTION bandNoDataValue(raster, integer DEFAULT 1)
  RETURNS float8
  AS 'MODULE_PATHNAME', 'Raster_band_nodata_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- GENERATED-REPRESENTATIONS-BEGIN raster_base — tools/codegen/inherited/generate.py from templates/representations.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/representation_families.yaml and re-run.
/******************************************************************************
 * Well-Known Binary representations of a raster
 *
 * A raster round-trips through the Well-Known Binary PostGIS writes for it,
 * in the byte order asked for, so a binding reads and writes the rasters of a
 * PostGIS column with no PostgreSQL in between.
 ******************************************************************************/

CREATE FUNCTION rasterFromBinary(bytea)
  RETURNS raster
  AS 'MODULE_PATHNAME', 'Raster_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION rasterFromHexWKB(text)
  RETURNS raster
  AS 'MODULE_PATHNAME', 'Raster_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(raster, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Raster_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(raster, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Raster_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- GENERATED-REPRESENTATIONS-END raster_base

/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster whose band states the classes an expression maps its
 * values onto
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @param[in] expr Reclassification expression
 * @param[in] pixeltype Name of the pixel type of the resulting band
 * @param[in] nodataval Nodata value of the resulting band, absent where the
 * band states none
 * @note Not STRICT: a nodata value left out states that the resulting band
 * carries none, which is a different answer from a band whose nodata value is
 * zero
 */
CREATE FUNCTION reclass(raster, integer, text, text,
    float8 DEFAULT NULL)
  RETURNS raster
  AS 'MODULE_PATHNAME', 'Raster_reclass'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster keeping the pixels of another that a geometry covers
 * @param[in] rast Raster
 * @param[in] geom Geometry, in the reference system of the raster
 * @param[in] crop True to reduce the result to the extent the two share
 * @param[in] touched True to keep every pixel the geometry touches
 */
CREATE FUNCTION clip(rast raster, geom geometry, crop boolean DEFAULT true,
    touched boolean DEFAULT false)
  RETURNS raster
  AS 'MODULE_PATHNAME', 'Raster_clip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster keeping the pixels of some bands of another that a
 * geometry covers
 * @param[in] rast Raster
 * @param[in] nband Numbers of the bands to keep, NULL for every band
 * @param[in] geom Geometry, in the reference system of the raster
 * @param[in] nodataval Nodata values of the bands of the result, one for every
 * band or one per band, NULL for the nodata value of each band
 * @param[in] crop True to reduce the result to the extent the two share
 * @param[in] touched True to keep every pixel the geometry touches
 * @note Not STRICT: a NULL array of bands keeps every band, and a NULL array
 * of nodata values keeps the nodata value of each band
 */
CREATE FUNCTION clip(rast raster, nband integer[], geom geometry,
    nodataval float8[] DEFAULT NULL, crop boolean DEFAULT true,
    touched boolean DEFAULT false)
  RETURNS raster
  AS 'MODULE_PATHNAME', 'Raster_clip_bands'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster stated in another spatial reference system
 * @param[in] rast Raster
 * @param[in] srid Target spatial reference system identifier
 * @param[in] algorithm Name of the resampling algorithm
 * @param[in] maxerr Error in input pixels the warp may commit
 * @param[in] scalex,scaley Pixel size of the result in the units of the target
 * system, 0 to let the warp derive it
 */
CREATE FUNCTION transform(rast raster, srid integer,
    algorithm text DEFAULT 'NearestNeighbour', maxerr float8 DEFAULT 0.125,
    scalex float8 DEFAULT 0, scaley float8 DEFAULT 0)
  RETURNS raster
  AS 'MODULE_PATHNAME', 'Raster_transform'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster stated on the grid of another raster
 * @param[in] rast Raster
 * @param[in] alignto Raster whose grid the result lies on
 * @param[in] algorithm Name of the resampling algorithm
 * @param[in] maxerr Error in input pixels the warp may commit
 */
CREATE FUNCTION transform(rast raster, alignto raster,
    algorithm text DEFAULT 'NearestNeighbour', maxerr float8 DEFAULT 0.125)
  RETURNS raster
  AS 'MODULE_PATHNAME', 'Raster_transform_raster'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster resampled to another pixel size
 * @param[in] rast Raster
 * @param[in] scalex,scaley Pixel size in the units of the raster's reference
 * system
 * @param[in] algorithm Name of the resampling algorithm
 * @param[in] maxerr Error in input pixels the warp may commit
 */
CREATE FUNCTION rescale(raster, float8, float8,
    text DEFAULT 'NearestNeighbour', float8 DEFAULT 0.125)
  RETURNS raster
  AS 'MODULE_PATHNAME', 'Raster_rescale'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return what the pixels of a raster band amount to
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @param[in] exclude_nodata True to leave the pixels the band states as nodata
 * out of the statistics
 */
CREATE FUNCTION summaryStats(raster, integer DEFAULT 1, boolean DEFAULT true)
  RETURNS summarystats
  AS 'MODULE_PATHNAME', 'Raster_summary_stats'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/**
 * @ingroup mobilitydb_raster
 * @brief Return the polygons of a raster band, one for each group of pixels
 * carrying the same value
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @param[in] exclude_nodata True to leave the pixels the band states as nodata
 * out
 */
CREATE FUNCTION dumpAsPolygons(raster, integer DEFAULT 1, boolean DEFAULT true)
  RETURNS SETOF geomval
  AS 'MODULE_PATHNAME', 'Raster_dump_as_polygons'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Accessors for raquet tiles
 *****************************************************************************/

CREATE FUNCTION quadbin(raquet)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Raquet_quadbin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION width(raquet)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Raquet_width'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION height(raquet)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Raquet_height'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bandPixelType(raquet)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Raquet_band_pixel_type'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bandHasNoDataValue(raquet)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Raquet_band_has_nodata_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bandNoDataValue(raquet)
  RETURNS float8
  AS 'MODULE_PATHNAME', 'Raquet_band_nodata_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pixels(raquet)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Raquet_pixels'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Comparison of raquet tiles
 *****************************************************************************/

CREATE FUNCTION eq(raquet, raquet)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Raquet_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(raquet, raquet)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Raquet_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lt(raquet, raquet)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Raquet_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(raquet, raquet)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Raquet_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(raquet, raquet)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Raquet_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(raquet, raquet)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Raquet_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(raquet, raquet)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Raquet_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = raquet, RIGHTARG = raquet,
  PROCEDURE = eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = raquet, RIGHTARG = raquet,
  PROCEDURE = ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  PROCEDURE = lt,
  LEFTARG = raquet, RIGHTARG = raquet,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR <= (
  PROCEDURE = le,
  LEFTARG = raquet, RIGHTARG = raquet,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR >= (
  PROCEDURE = ge,
  LEFTARG = raquet, RIGHTARG = raquet,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR > (
  PROCEDURE = gt,
  LEFTARG = raquet, RIGHTARG = raquet,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = areasel, JOIN = areajoinsel
);

CREATE OPERATOR CLASS raquet_btree_ops
  DEFAULT FOR TYPE raquet USING btree AS
  OPERATOR  1  < ,
  OPERATOR  2  <= ,
  OPERATOR  3  = ,
  OPERATOR  4  >= ,
  OPERATOR  5  > ,
  FUNCTION  1  cmp(raquet, raquet);

/*****************************************************************************/

CREATE FUNCTION hash(raquet)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Raquet_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION hashExtended(raquet, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Raquet_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS raquet_hash_ops
  DEFAULT FOR TYPE raquet USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   hash(raquet),
    FUNCTION    2   hashExtended(raquet, bigint);

/*****************************************************************************/

/******************************************************************************
 * Conversions of raquet tiles
 *****************************************************************************/

CREATE FUNCTION stbox(raquet)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Raquet_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (raquet AS stbox) WITH FUNCTION stbox(raquet);

/******************************************************************************
 * Conversions of rasters
 *
 * PostGIS has no stbox, so this conversion adds the temporal layer's box to
 * the raster a user already holds.
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Convert a raster into a spatiotemporal box
 * @param[in] rast Raster
 */
CREATE FUNCTION stbox(raster)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Raster_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (raster AS stbox) WITH FUNCTION stbox(raster);

/*****************************************************************************/
