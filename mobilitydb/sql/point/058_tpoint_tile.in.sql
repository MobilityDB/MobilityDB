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
 * tpoint_tile.sql
 * Functions for spatial and spatiotemporal tiles.
 */

/******************************************************************************
 * Multidimensional tiling
 ******************************************************************************/

CREATE TYPE index_stbox AS (
  index integer,
  tile stbox
);

CREATE FUNCTION tileList(bounds stbox, xsize float, ysize float, zsize float,
    sorigin geometry DEFAULT 'Point(0 0 0)')
  RETURNS SETOF index_stbox
  AS 'MODULE_PATHNAME', 'Stbox_tile_list'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tileList(bounds stbox, size float,
    sorigin geometry DEFAULT 'Point(0 0 0)')
  RETURNS SETOF index_stbox
  AS 'SELECT @extschema@.tileList($1, $2, $2, $2, $3)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION tileList(bounds stbox, xsize float, ysize float,
    sorigin geometry DEFAULT 'Point(0 0 0)')
  RETURNS SETOF index_stbox
  AS 'SELECT @extschema@.tileList($1, $2, $3, $2, $4)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

CREATE FUNCTION tileList(bounds stbox, xsize float, ysize float, zsize float,
  duration interval, sorigin geometry DEFAULT 'Point(0 0 0)',
  timestamptz DEFAULT '2000-01-03')
  RETURNS SETOF index_stbox
  AS 'MODULE_PATHNAME', 'Stbox_tile_list'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tileList(bounds stbox, size float,
  duration interval, sorigin geometry DEFAULT 'Point(0 0 0)',
  timestamptz DEFAULT '2000-01-03')
  RETURNS SETOF index_stbox
  AS 'SELECT @extschema@.tileList($1, $2, $2, $2, $3, $4, $5)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION tileList(bounds stbox, xsize float, ysize float,
  duration interval, sorigin geometry DEFAULT 'Point(0 0 0)',
  timestamptz DEFAULT '2000-01-03')
  RETURNS SETOF index_stbox
  AS 'SELECT @extschema@.tileList($1, $2, $3, $2, $4, $5, $6)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

CREATE FUNCTION tile(point geometry, xsize float, ysize float, zsize float,
    sorigin geometry DEFAULT 'Point(0 0 0)')
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_tile'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tile(point geometry, size float,
    sorigin geometry DEFAULT 'Point(0 0 0)')
  RETURNS stbox
  AS 'SELECT @extschema@.tile($1, $2, $2, $2, $3)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION tile(point geometry, xsize float, ysize float,
    sorigin geometry DEFAULT 'Point(0 0 0)')
  RETURNS stbox
  AS 'SELECT @extschema@.tile($1, $2, $3, $2, $4)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

CREATE FUNCTION tile(point geometry, "time" timestamptz, xsize float,
    ysize float, zsize float, duration interval,
    sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03')
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_tile'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tile(point geometry, "time" timestamptz, size float,
    duration interval, sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03')
  RETURNS stbox
  AS 'SELECT @extschema@.tile($1, $2, $3, $3, $3, $4, $5, $6)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION tile(point geometry, "time" timestamptz, xsize float,
    ysize float, duration interval, sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03')
  RETURNS stbox
  AS 'SELECT @extschema@.tile($1, $2, $3, $4, $3, $5, $6, $7)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/

CREATE TYPE point_tpoint AS (
  point geometry,
  tpoint tgeompoint
);

CREATE FUNCTION spaceSplit(tgeompoint, xsize float, ysize float, zsize float,
    sorigin geometry DEFAULT 'Point(0 0 0)', bitmatrix boolean DEFAULT TRUE)
  RETURNS SETOF point_tpoint
  AS 'MODULE_PATHNAME', 'Tpoint_space_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceSplit(tgeompoint, size float,
    sorigin geometry DEFAULT 'Point(0 0 0)', bitmatrix boolean DEFAULT TRUE)
  RETURNS SETOF point_tpoint
  AS 'SELECT @extschema@.spaceSplit($1, $2, $2, $2, $3, $4)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceSplit(tgeompoint, sizeX float, sizeY float,
    sorigin geometry DEFAULT 'Point(0 0 0)', bitmatrix boolean DEFAULT TRUE)
  RETURNS SETOF point_tpoint
  AS 'SELECT @extschema@.spaceSplit($1, $2, $3, $2, $4, $5)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

CREATE TYPE point_time_tpoint AS (
  point geometry,
  time timestamptz,
  tpoint tgeompoint
);

CREATE FUNCTION spaceTimeSplit(tgeompoint, xsize float, ysize float,
    zsize float, interval, sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03', bitmatrix boolean DEFAULT TRUE)
  RETURNS SETOF point_time_tpoint
  AS 'MODULE_PATHNAME', 'Tpoint_space_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceTimeSplit(tgeompoint, size float, interval,
    sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03', bitmatrix boolean DEFAULT TRUE)
  RETURNS SETOF point_time_tpoint
  AS 'SELECT @extschema@.spaceTimeSplit($1, $2, $2, $2, $3, $4, $5, $6)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceTimeSplit(tgeompoint, xsize float, ysize float, interval,
    sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03', bitmatrix boolean DEFAULT TRUE)
  RETURNS SETOF point_time_tpoint
  AS 'SELECT @extschema@.spaceTimeSplit($1, $2, $3, $2, $4, $5, $6, $7)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/
