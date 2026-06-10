/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * The boxes are computed on the temporal centroid trajectory
 * (`trgeometry_to_tpoint`) and thus reuse the `tgeo` tiling kernel, keeping the
 * trgeometry tiling surface aligned with the temporal geometry one.
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_rgeo.h>
#include "rgeo/trgeo.h"

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
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  Temporal *tpoint = trgeometry_to_tpoint(temp);
  STBox *result = tgeo_space_time_boxes(tpoint, xsize, ysize, zsize, NULL,
    sorigin, 0, bitmatrix, border_inc, count);
  pfree(tpoint);
  return result;
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
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  Temporal *tpoint = trgeometry_to_tpoint(temp);
  STBox *result = tgeo_space_time_boxes(tpoint, xsize, ysize, zsize, duration,
    sorigin, torigin, bitmatrix, border_inc, count);
  pfree(tpoint);
  return result;
}

/*****************************************************************************/
