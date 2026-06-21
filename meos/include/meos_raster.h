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
 * @brief Public MEOS API for Raquet raster chip sampling along temporal point
 * trajectories via the CARTO QUADBIN grid.
 *
 * The raster family adds no temporal type of its own: it samples a raster chip
 * (a QUADBIN-tiled pixel array in MEOS, or a PostGIS raster band on the PG
 * side) at every instant of a `tgeompoint` trajectory, yielding a `tfloat`.
 *
 * Implementations live in meos/src/raster/. The PG V1 wrappers in
 * mobilitydb/src/raster/ call these symbols.
 */

#ifndef __MEOS_RASTER_H__
#define __MEOS_RASTER_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
/* MEOS */
#include <meos.h>

/**
 * @brief Pixel data type for raster chip sampling.
 *
 * Values are assigned to be compatible with the corresponding PostGIS
 * rt_pixtype codes where they overlap.
 */
typedef enum
{
  MEOS_PT_UINT8   = 0,   /**< Unsigned 8-bit integer  */
  MEOS_PT_INT16   = 1,   /**< Signed 16-bit integer   */
  MEOS_PT_INT32   = 2,   /**< Signed 32-bit integer   */
  MEOS_PT_FLOAT32 = 3,   /**< 32-bit IEEE float       */
  MEOS_PT_FLOAT64 = 4,   /**< 64-bit IEEE double      */
} MeosPixType;

extern Temporal *raster_tile_value_quadbin(const uint8_t *pixels,
  uint16_t width, uint16_t height, uint64 quadbin, MeosPixType pixtype,
  double nodata, bool has_nodata, const Temporal *traj);

extern uint64 *trajectory_quadbins(const Temporal *traj, uint32_t zoom,
  int *count);

#endif /* __MEOS_RASTER_H__ */
