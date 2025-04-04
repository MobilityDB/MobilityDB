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
 * @brief Functions for spatial and spatiotemporal tiles
 */

/******************************************************************************
 * Multidimensional tiling
 ******************************************************************************/

CREATE TYPE index_stbox AS (
  index integer,
  tile stbox
);

CREATE FUNCTION spaceTiles(bounds stbox, xsize float, ysize float, zsize float,
    sorigin geometry DEFAULT 'Point(0 0 0)', borderIinc boolean DEFAULT TRUE)
  RETURNS SETOF index_stbox
  AS 'MODULE_PATHNAME', 'Stbox_space_tiles'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spaceTiles(bounds stbox, xsize float,
    sorigin geometry DEFAULT 'Point(0 0 0)', borderInc boolean DEFAULT TRUE)
  RETURNS SETOF index_stbox
  AS 'SELECT @extschema@.spaceTiles($1, $2, $2, $2, $3, $4)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceTiles(bounds stbox, xsize float, ysize float,
    sorigin geometry DEFAULT 'Point(0 0 0)', borderInc boolean DEFAULT TRUE)
  RETURNS SETOF index_stbox
  AS 'SELECT @extschema@.spaceTiles($1, $2, $3, $2, $4, $5)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

CREATE FUNCTION timeTiles(bounds stbox, duration interval,
  timestamptz DEFAULT '2000-01-03', borderInc boolean DEFAULT TRUE)
  RETURNS SETOF index_stbox
  AS 'MODULE_PATHNAME', 'Stbox_time_tiles'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION spaceTimeTiles(bounds stbox, xsize float, ysize float, zsize float,
  duration interval, sorigin geometry DEFAULT 'Point(0 0 0)',
  timestamptz DEFAULT '2000-01-03', borderInc boolean DEFAULT TRUE)
  RETURNS SETOF index_stbox
  AS 'MODULE_PATHNAME', 'Stbox_space_time_tiles'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spaceTimeTiles(bounds stbox, xsize float,
  duration interval, sorigin geometry DEFAULT 'Point(0 0 0)',
  timestamptz DEFAULT '2000-01-03', borderInc boolean DEFAULT TRUE)
  RETURNS SETOF index_stbox
  AS 'SELECT @extschema@.spaceTimeTiles($1, $2, $2, $2, $3, $4, $5, $6)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceTimeTiles(bounds stbox, xsize float, ysize float,
  duration interval, sorigin geometry DEFAULT 'Point(0 0 0)',
  timestamptz DEFAULT '2000-01-03', borderInc boolean DEFAULT TRUE)
  RETURNS SETOF index_stbox
  AS 'SELECT @extschema@.spaceTimeTiles($1, $2, $3, $2, $4, $5, $6, $7)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

/******************************************************************************/

CREATE FUNCTION getSpaceTile(point geometry, xsize float, ysize float, 
    zsize float, sorigin geometry DEFAULT 'Point(0 0 0)')
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_get_space_tile'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getSpaceTile(point geometry, xsize float,
    sorigin geometry DEFAULT 'Point(0 0 0)')
  RETURNS stbox
  AS 'SELECT @extschema@.getSpaceTile($1, $2, $2, $2, $3)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION getSpaceTile(point geometry, xsize float, ysize float,
    sorigin geometry DEFAULT 'Point(0 0 0)')
  RETURNS stbox
  AS 'SELECT @extschema@.getSpaceTile($1, $2, $3, $2, $4)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

CREATE FUNCTION getStboxTimeTile(t timestamptz, duration interval,
    torigin timestamptz DEFAULT '2000-01-03')
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_get_time_tile'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getSpaceTimeTile(point geometry, t timestamptz, xsize float,
    ysize float, zsize float, duration interval,
    sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03')
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_get_space_time_tile'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getSpaceTimeTile(point geometry, t timestamptz, xsize float,
    duration interval, sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03')
  RETURNS stbox
  AS 'SELECT @extschema@.getSpaceTimeTile($1, $2, $3, $3, $3, $4, $5, $6)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION getSpaceTimeTile(point geometry, t timestamptz, xsize float,
    ysize float, duration interval, sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03')
  RETURNS stbox
  AS 'SELECT @extschema@.getSpaceTimeTile($1, $2, $3, $4, $3, $5, $6, $7)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/

CREATE FUNCTION spaceBoxes(tgeompoint, xsize float, ysize float, zsize float,
    sorigin geometry DEFAULT 'Point(0 0 0)', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Tgeo_space_boxes'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceBoxes(tgeompoint, xsize float,
    sorigin geometry DEFAULT 'Point(0 0 0)', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS stbox[]
  AS 'SELECT @extschema@.spaceBoxes($1, $2, $2, $2, $3, $4)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceBoxes(tgeompoint, xsize float, ysize float,
    sorigin geometry DEFAULT 'Point(0 0 0)', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS stbox[]
  AS 'SELECT @extschema@.spaceBoxes($1, $2, $3, $2, $4, $5)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

CREATE FUNCTION timeBoxes(tgeompoint, interval,
    torigin timestamptz DEFAULT '2000-01-03', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Tgeo_time_boxes'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

CREATE FUNCTION spaceTimeBoxes(tgeompoint, xsize float, ysize float,
    zsize float, interval, sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Tgeo_space_time_boxes'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceTimeBoxes(tgeompoint, xsize float, interval,
    sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS stbox[]
  AS 'SELECT @extschema@.spaceTimeBoxes($1, $2, $2, $2, $3, $4, $5, $6)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceTimeBoxes(tgeompoint, xsize float, ysize float, interval,
    sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS stbox[]
  AS 'SELECT @extschema@.spaceTimeBoxes($1, $2, $3, $2, $4, $5, $6, $7)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/

CREATE TYPE point_tpoint AS (
  point geometry,
  tpoint tgeompoint
);

CREATE FUNCTION spaceSplit(tgeompoint, xsize float, ysize float, zsize float,
    sorigin geometry DEFAULT 'Point(0 0 0)', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS SETOF point_tpoint
  AS 'MODULE_PATHNAME', 'Tgeo_space_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceSplit(tgeompoint, size float,
    sorigin geometry DEFAULT 'Point(0 0 0)', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS SETOF point_tpoint
  AS 'SELECT @extschema@.spaceSplit($1, $2, $2, $2, $3, $4)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceSplit(tgeompoint, sizeX float, sizeY float,
    sorigin geometry DEFAULT 'Point(0 0 0)', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
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
    torigin timestamptz DEFAULT '2000-01-03', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS SETOF point_time_tpoint
  AS 'MODULE_PATHNAME', 'Tgeo_space_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceTimeSplit(tgeompoint, size float, interval,
    sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS SETOF point_time_tpoint
  AS 'SELECT @extschema@.spaceTimeSplit($1, $2, $2, $2, $3, $4, $5, $6)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION spaceTimeSplit(tgeompoint, xsize float, ysize float, interval,
    sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03', bitmatrix boolean DEFAULT TRUE,
    borderInc boolean DEFAULT TRUE)
  RETURNS SETOF point_time_tpoint
  AS 'SELECT @extschema@.spaceTimeSplit($1, $2, $3, $2, $4, $5, $6, $7)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/
