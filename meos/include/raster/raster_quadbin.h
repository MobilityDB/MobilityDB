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
 * @brief Callback returning the raster pixel value at a point: return true
 * and set @p value, or return false when the point lies outside the raster
 * or on a nodata pixel
 */
typedef bool (*raster_sample_fn)(void *ctx, const GSERIALIZED *point,
  double *value);

extern Temporal *raster_value_sampler(const Temporal *traj, const STBox *box,
  raster_sample_fn sample, void *ctx);
extern Temporal *raster_at_value_sampler(const Temporal *traj,
  const STBox *box, raster_sample_fn sample, void *ctx, const Span *vspan);
extern Temporal *raster_minus_value_sampler(const Temporal *traj,
  const STBox *box, raster_sample_fn sample, void *ctx, const Span *vspan);
extern int eraster_value_sampler(const Temporal *traj, const STBox *box,
  raster_sample_fn sample, void *ctx, const Span *vspan);
extern int araster_value_sampler(const Temporal *traj, const STBox *box,
  raster_sample_fn sample, void *ctx, const Span *vspan);

/*****************************************************************************/

#endif /* __RASTER_QUADBIN_H__ */
