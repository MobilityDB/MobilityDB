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
 * @brief Spatial and spatiotemporal grid tiling for temporal rigid geometries
 *
 * A rigid geometry occupies the region its body covers, so the tiles are the
 * ones that body reaches, which #trgeo_restrict_stbox answers by solving for
 * the times the body meets a box. The grid itself is the `tgeo` one, reached
 * through #tspatial_space_time_boxes, so the two families lay values on the
 * same grid and differ only in how a value is read against one tile.
 *
 * Reading the centroid trajectory instead answers the tiles a POINT visits:
 * a value holding one placement of a 4 by 4 square answers a single box of
 * zero extent, and a square carried past four grid columns answers none of
 * them.
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_rgeo.h>
#include "geo/tgeo_tile.h"
#include "rgeo/trgeo.h"
#include "rgeo/trgeo_spatialfuncs.h"

/*****************************************************************************
 * Boxes functions
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_tile
 * @brief Return the spatial boxes of a temporal rigid geometry split with
 * respect to a spatial grid
 * @param[in] temp Temporal rigid geometry
 * @param[in] xsize,ysize,zsize Size of the corresponding dimension
 * @param[in] sorigin Origin of the spatial grid
 * @param[in] bitmatrix True when using a bitmatrix to speed up the computation
 * @param[in] border_inc True when the box contains the upper border
 * @param[out] count Number of elements in the output array
 * @csqlfn #Trgeometry_space_boxes()
 */
STBox *
trgeometry_space_boxes(const Temporal *temp, double xsize, double ysize,
  double zsize, const GSERIALIZED *sorigin, bool bitmatrix, bool border_inc,
  int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  return tspatial_space_time_boxes(temp, xsize, ysize, zsize, NULL, sorigin,
    0, bitmatrix, border_inc, &trgeo_restrict_stbox, count);
}

/**
 * @ingroup meos_rgeo_tile
 * @brief Return the spatiotemporal boxes of a temporal rigid geometry split
 * with respect to a spatiotemporal grid
 * @param[in] temp Temporal rigid geometry
 * @param[in] xsize,ysize,zsize Size of the corresponding dimension
 * @param[in] duration Size of the temporal dimension as an interval
 * @param[in] sorigin Origin of the spatial grid
 * @param[in] torigin Origin of the temporal grid
 * @param[in] bitmatrix True when using a bitmatrix to speed up the computation
 * @param[in] border_inc True when the box contains the upper border
 * @param[out] count Number of elements in the output array
 * @csqlfn #Trgeometry_space_time_boxes()
 */
STBox *
trgeometry_space_time_boxes(const Temporal *temp, double xsize, double ysize,
  double zsize, const Interval *duration, const GSERIALIZED *sorigin,
  TimestampTz torigin, bool bitmatrix, bool border_inc, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  return tspatial_space_time_boxes(temp, xsize, ysize, zsize, duration,
    sorigin, torigin, bitmatrix, border_inc, &trgeo_restrict_stbox, count);
}

/*****************************************************************************/
