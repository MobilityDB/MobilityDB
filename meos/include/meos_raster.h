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
#include <meos_geo.h>

/*****************************************************************************/

/**
 * @brief Pixel data type of a Raquet band
 *
 * The names are the ones the RaQuet specification gives the `type` field of a
 * band. The code of a type indexes the pixel type catalog and is what a tile
 * carries in its header and in its Well-Known Binary representation, so a type
 * keeps the code it has and a new one takes the next free code. The codes are
 * MEOS's own: the PostGIS rt_pixtype codes number the types differently.
 */
typedef enum
{
  MEOS_PT_UINT8   = 0,   /**< Unsigned 8-bit integer  */
  MEOS_PT_INT16   = 1,   /**< Signed 16-bit integer   */
  MEOS_PT_INT32   = 2,   /**< Signed 32-bit integer   */
  MEOS_PT_FLOAT32 = 3,   /**< 32-bit IEEE float       */
  MEOS_PT_FLOAT64 = 4,   /**< 64-bit IEEE double      */
  MEOS_PT_INT8    = 5,   /**< Signed 8-bit integer    */
  MEOS_PT_UINT16  = 6,   /**< Unsigned 16-bit integer */
  MEOS_PT_UINT32  = 7,   /**< Unsigned 32-bit integer */
  MEOS_PT_INT64   = 8,   /**< Signed 64-bit integer   */
  MEOS_PT_UINT64  = 9,   /**< Unsigned 64-bit integer */
  MEOS_PT_FLOAT16 = 10,  /**< 16-bit IEEE half        */
} MeosPixType;

/* Pixel type utility functions */

extern size_t raquet_pixtype_size(MeosPixType pixtype);
extern MeosPixType raquet_pixtype_from_string(const char *str);

/* Opaque structure to represent a Raquet raster tile */

typedef struct Raquet Raquet;

/* Input and output functions for Raquet tiles */

extern Raquet *raquet_in(const char *str);
extern char *raquet_out(const Raquet *rq);
extern Raquet *raquet_from_wkb(const uint8_t *wkb, size_t size);
extern Raquet *raquet_from_hexwkb(const char *hexwkb);
extern uint8_t *raquet_as_wkb(const Raquet *rq, uint8_t variant, size_t *size_out);
extern char *raquet_as_hexwkb(const Raquet *rq, uint8_t variant, size_t *size_out);

/* Constructor functions for Raquet tiles */

extern Raquet *raquet_make(uint64 quadbin, int32 width, int32 height,
  MeosPixType pixtype, double nodata, bool has_nodata, const uint8_t *pixels,
  size_t pixels_size);
extern Raquet *raquet_copy(const Raquet *rq);
extern Raquet *raquet_read(const char *path, uint64 quadbin);
extern Raquet *raquet_read_bytes(const uint8_t *data, size_t size, uint64 quadbin);

/* Accessor functions for Raquet tiles */

extern uint64 raquet_quadbin(const Raquet *rq);
extern int raquet_width(const Raquet *rq);
extern int raquet_height(const Raquet *rq);
extern double raquet_nodata(const Raquet *rq);
extern char *raquet_pixtype(const Raquet *rq);
extern uint8_t *raquet_pixels(const Raquet *rq, size_t *size_out);
extern uint32 raquet_hash(const Raquet *rq);
extern uint64 raquet_hash_extended(const Raquet *rq, uint64 seed);

/* Opaque structure to represent a PostGIS raster */

typedef struct Raster Raster;

/* Input and output functions for PostGIS rasters */

extern Raster *raster_from_wkb(const uint8_t *wkb, size_t size);
extern Raster *raster_from_hexwkb(const char *hexwkb);
extern uint8_t *raster_as_wkb(const Raster *rast, size_t *size_out);
extern char *raster_as_hexwkb(const Raster *rast, size_t *size_out);

/* Accessor functions for PostGIS rasters */

extern int raster_num_bands(const Raster *rast);

/* Conversion functions for Raquet tiles */

extern STBox *raquet_to_stbox(const Raquet *rq);

/* Comparison functions for Raquet tiles */

extern int raquet_cmp(const Raquet *rq1, const Raquet *rq2);
extern bool raquet_eq(const Raquet *rq1, const Raquet *rq2);
extern bool raquet_ne(const Raquet *rq1, const Raquet *rq2);
extern bool raquet_lt(const Raquet *rq1, const Raquet *rq2);
extern bool raquet_le(const Raquet *rq1, const Raquet *rq2);
extern bool raquet_ge(const Raquet *rq1, const Raquet *rq2);
extern bool raquet_gt(const Raquet *rq1, const Raquet *rq2);

/* Sampling functions */

/* Sampling of a PostGIS raster: reads the band the position falls in with
 * nearest-neighbour resampling, and derives the bounding-box pre-filter from
 * the raster extent */

extern Temporal *raster_value(const Temporal *traj, const Raster *rast,
  int band);
extern Temporal *raster_at_value(const Temporal *traj, const Raster *rast,
  int band, const Span *vspan);
extern Temporal *raster_minus_value(const Temporal *traj, const Raster *rast,
  int band, const Span *vspan);
extern int eraster_value(const Temporal *traj, const Raster *rast, int band,
  const Span *vspan);
extern int araster_value(const Temporal *traj, const Raster *rast, int band,
  const Span *vspan);

/* GDAL-backed sampling of a raster file: opens the file, derives the
 * bounding-box pre-filter from its geotransform, and reads the band through
 * GDALRasterIO */

extern Temporal *raster_value_gdal(const Temporal *traj, const char *path,
  int band);
extern Temporal *raster_at_value_gdal(const Temporal *traj, const char *path,
  int band, const Span *vspan);
extern Temporal *raster_minus_value_gdal(const Temporal *traj,
  const char *path, int band, const Span *vspan);
extern int eraster_value_gdal(const Temporal *traj, const char *path, int band,
  const Span *vspan);
extern int araster_value_gdal(const Temporal *traj, const char *path, int band,
  const Span *vspan);

extern Temporal *raster_tile_value_quadbin(const Temporal *traj,
  const uint8_t *pixels, size_t pixels_size, int32 width, int32 height,
  uint64 quadbin, MeosPixType pixtype, double nodata, bool has_nodata);

extern Temporal *raster_tile_value(const Temporal *traj, const Raquet *rq);
extern Temporal *raster_tile_value_array(const Temporal *traj,
  const Raquet **rqarr, int count);

extern uint64 *trajectory_quadbins(const Temporal *traj, uint32_t zoom,
  int *count);

/*****************************************************************************/

#endif /* __MEOS_RASTER_H__ */
