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
 *   rasterValue(tgeompoint, raster, band integer DEFAULT 1) → tfloat
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
  AS 'MODULE_PATHNAME', 'Raquet_read'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/******************************************************************************
 * rasterValue
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return the values of a raster band sampled at the instants of a
 * trajectory
 * @param[in] traj Trajectory
 * @param[in] rast Raster
 * @param[in] band Band number (1-based, default 1)
 */
CREATE OR REPLACE FUNCTION rasterValue(
    traj  tgeompoint,
    rast  raster,
    band  integer DEFAULT 1
) RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Raster_value'
  LANGUAGE C STRICT PARALLEL SAFE;

/******************************************************************************
 * rasterTileValueQuadbin
 *****************************************************************************/

/**
 * @ingroup mobilitydb_raster
 * @brief Return the values of a Raquet raster chip sampled at the instants of
 * a trajectory, using a QUADBIN cell to determine the tile georeferencing
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
 * @brief Sample a raquet raster tile at the instants of a trajectory
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
 * @brief Sample an array of raquet raster tiles at the instants of a 
 * trajectory, keeping the value of the tile of highest zoom where tiles overlap
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
CREATE FUNCTION nodata(raquet)
  RETURNS float8
  AS 'MODULE_PATHNAME', 'Raquet_nodata'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pixtype(raquet)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Raquet_pixtype'
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

/*****************************************************************************/
