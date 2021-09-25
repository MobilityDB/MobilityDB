/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2021, PostGIS contributors
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
  box stbox
);

CREATE OR REPLACE FUNCTION multidimGrid(bounds stbox, size float,
    sorigin geometry DEFAULT 'Point(0 0 0)')
  RETURNS SETOF index_stbox
  AS 'MODULE_PATHNAME', 'stbox_multidim_grid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION multidimGrid(bounds stbox, size float, 
  duration interval, sorigin geometry DEFAULT 'Point(0 0 0)',
  timestamptz DEFAULT '2000-01-03')
  RETURNS SETOF index_stbox
  AS 'MODULE_PATHNAME', 'stbox_multidim_grid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION multidimTile(point geometry, size float, 
    sorigin geometry DEFAULT 'Point(0 0 0)')
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'stbox_multidim_tile'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION multidimTile(point geometry, "time" timestamptz, 
    size float, duration interval, sorigin geometry DEFAULT 'Point(0 0 0)', 
    torigin timestamptz DEFAULT '2000-01-03')
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'stbox_multidim_tile'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE TYPE point_tpoint AS (
  point geometry,
  tpoint tgeompoint
);

CREATE OR REPLACE FUNCTION spaceSplit(tgeompoint, float,
    sorigin geometry DEFAULT 'Point(0 0 0)')
  RETURNS SETOF point_tpoint
  AS 'MODULE_PATHNAME', 'tpoint_space_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

CREATE TYPE point_time_tpoint AS (
  point geometry,
  time timestamptz,
  tpoint tgeompoint
);

CREATE OR REPLACE FUNCTION spaceTimeSplit(tgeompoint, float, interval,
    sorigin geometry DEFAULT 'Point(0 0 0)',
    torigin timestamptz DEFAULT '2000-01-03')
  RETURNS SETOF point_time_tpoint
  AS 'MODULE_PATHNAME', 'tpoint_space_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/
