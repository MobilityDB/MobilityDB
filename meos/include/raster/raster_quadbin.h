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
 * @brief Internal header for QUADBIN-keyed raster chip sampling.
 */

#ifndef __RASTER_QUADBIN_H__
#define __RASTER_QUADBIN_H__

/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_raster.h>

/*****************************************************************************/

extern bool raster_quadbin_from_bounds(double origin_x, double origin_y,
  double pixel_w, double pixel_h, int xsize, int ysize, uint64 *result);

extern void raster_quadbin_bounds(uint64 cell, double *xmin, double *ymin,
  double *xmax, double *ymax);

extern uint32_t raster_quadbin_zoom(uint64 cell);

/**
 * @brief Callback returning the grid coordinates of a position: its column
 * and its row as real numbers, whose floors name the pixel it falls in
 */
typedef void (*raster_grid_fn)(const void *ctx, double x, double y,
  double *col, double *row);

/**
 * @brief Callback returning the value of a pixel named by its column and its
 * row, both inside the grid: return true and set @p value, or return false
 * when the pixel carries no value
 */
typedef bool (*raster_pixel_fn)(void *ctx, int col, int row, double *value);

/**
 * @brief Callback returning the parameter at which the segment from
 * (@p x1, @p y1) to (@p x2, @p y2) reaches the grid line @p k of an axis,
 * 0 naming the columns and 1 the rows
 * @details The parameter is 0 at the first endpoint and 1 at the second, and
 * the grid coordinate of each axis is monotonic along the segment, so the
 * segment reaches the line at one parameter.
 */
typedef double (*raster_cross_fn)(const void *ctx, double x1, double y1,
  double x2, double y2, int axis, double k);

/**
 * @brief What a raster engine tells the sampling about its grid: where a
 * position lies in it, the value of a pixel, where a segment reaches one of
 * its lines, and where it lies
 * @details The descriptor plays for a raster grid the part `DggsCellOps`
 * plays for a DGGS: the sampling names one of these instead of repeating the
 * engine's arguments at every entry point, and an engine fills it in one
 * place. The PostGIS raster, a GDAL file and a Raquet tile answer the same
 * questions and differ only in how.
 */
typedef struct RasterGridOps
{
  raster_grid_fn grid;      /**< Grid coordinates of a position */
  raster_pixel_fn pixel;    /**< Value of a pixel */
  raster_cross_fn cross;    /**< Parameter at which a segment reaches a grid
                                 line, NULL when the grid coordinates are
                                 affine in the position */
  void *ctx;                /**< State of the engine, passed to the callbacks */
  int width;                /**< Number of columns of the grid */
  int height;               /**< Number of rows of the grid */
  STBox box;                /**< Extent of the grid, the pre-filter */
} RasterGridOps;

extern Temporal *raster_value_sampler(const Temporal *traj,
  const RasterGridOps *ops);
extern Temporal *raster_at_value_sampler(const Temporal *traj,
  const RasterGridOps *ops, const Span *vspan);
extern Temporal *raster_minus_value_sampler(const Temporal *traj,
  const RasterGridOps *ops, const Span *vspan);
extern int eraster_value_sampler(const Temporal *traj,
  const RasterGridOps *ops, const Span *vspan);
extern int araster_value_sampler(const Temporal *traj,
  const RasterGridOps *ops, const Span *vspan);

/*****************************************************************************/

#endif /* __RASTER_QUADBIN_H__ */
