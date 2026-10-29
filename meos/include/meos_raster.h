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
 * @brief Public MEOS API for Raquet raster chip sampling along temporal point
 * trajectories via the CARTO QUADBIN grid
 * @details The raster family adds no temporal type of its own: it samples a
 * raster chip
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
#include <meos_quadbin.h>

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

extern Raquet *raquet_make(Quadbin quadbin, int32 width, int32 height,
  MeosPixType pixtype, double nodata, bool has_nodata, const uint8_t *pixels,
  size_t pixels_size);
extern Raquet *raquet_copy(const Raquet *rq);
extern Raquet *raquet_read(const char *path, Quadbin quadbin);
extern Raquet *raquet_read_bytes(const uint8_t *data, size_t size, Quadbin quadbin);

/* Accessor functions for Raquet tiles */

extern Quadbin raquet_quadbin(const Raquet *rq);
extern int raquet_width(const Raquet *rq);
extern int raquet_height(const Raquet *rq);
extern char *raquet_band_pixel_type(const Raquet *rq);
extern bool raquet_band_has_nodata_value(const Raquet *rq);
extern bool raquet_band_nodata_value(const Raquet *rq, double *result);
extern uint8_t *raquet_pixels(const Raquet *rq, size_t *size_out);
extern uint32 raquet_hash(const Raquet *rq);
extern uint64 raquet_hash_extended(const Raquet *rq, uint64 seed);
extern int raquet_mem_size(const Raquet *rq);

/* Opaque structure to represent a PostGIS raster */

typedef struct Raster Raster;

/**
 * @brief A polygon of a raster band together with the value its pixels share
 * @details The shape PostGIS states as its @p geomval composite type: a group
 * of pixels carrying one value, and that value
 */
typedef struct
{
  GSERIALIZED *geom;  /**< Polygon covering a group of pixels */
  double val;         /**< The value those pixels share */
} GeomVal;

/**
 * @brief A pixel of a raster band
 * @details The shape of PostGIS's @p rt_pixel and of the rows of its
 * @p ST_PixelAsPoints: the column and the row of the pixel, whether it is
 * nodata, its value and its upper-left corner
 */
typedef struct
{
  int x;              /**< Column of the pixel, starting at 1 */
  int y;              /**< Row of the pixel, starting at 1 */
  bool nodata;        /**< True when the pixel is nodata */
  double value;       /**< Value of the pixel */
  GSERIALIZED *geom;  /**< Upper-left corner of the pixel */
} Pixel;

/**
 * @brief What a raster band's pixels amount to, taken in one pass
 * @details The shape PostGIS states as the record of its @p ST_SummaryStats:
 * how many pixels the band states a value for, and what those values sum to,
 * average, spread and reach
 */
typedef struct
{
  uint32_t count;  /**< Number of pixels the statistics are taken over */
  double sum;      /**< Sum of their values */
  double mean;     /**< Mean of their values */
  double stddev;   /**< Standard deviation of their values */
  double min;      /**< Smallest of their values */
  double max;      /**< Largest of their values */
} BandStats;

/**
 * @brief How many pixels of a raster band hold a value
 * @details The shape PostGIS states as the rows of its @p ST_ValueCount and
 * @p ST_ValuePercent: a value, the number of pixels holding it and their share
 * of the pixels read
 */
typedef struct
{
  double value;     /**< The value */
  uint32_t count;   /**< Number of pixels holding it */
  double percent;   /**< Their share of the pixels read, between 0 and 1 */
} ValueCount;

/**
 * @brief State of the aggregate testing whether rasters lie on the same grid
 * @details The shape PostGIS states as its @p agg_samealignment type: the grid
 * of the first raster read, and whether every later raster is aligned with it
 */
typedef struct
{
  Raster *grid;    /**< Grid of the first raster, one pixel and no band */
  bool aligned;    /**< True while every raster read is aligned with it */
} RasterAlignState;

/**
 * @brief State of the aggregate uniting rasters
 * @details The shape of PostGIS's state of @p ST_Union: per band of the
 * union, the raster the pixel operation has built so far, and a second one
 * where the operation is `MEAN`, kept as a count and a sum, or `RANGE`, kept
 * as a minimum and a maximum
 */
typedef struct
{
  int band;          /**< Band united, starting at 1, or 0 for every band */
  int uniontype;     /**< Pixel operation, in the order `LAST`, `FIRST`,
                          `MIN`, `MAX`, `COUNT`, `SUM`, `MEAN`, `RANGE` */
  bool named;        /**< True when the pixel operation is named */
  int numbands;      /**< Number of bands of the union */
  Raster **rasters;  /**< Two parts per band, the second for `MEAN` and
                          `RANGE` only, NULL where none is built */
} RasterUnionState;

/**
 * @brief State of the aggregate summarizing the pixels of rasters
 * @details The shape of PostGIS's state of @p ST_SummaryStatsAgg: the band
 * read and whether nodata is left out, what the pixels read amount to, and
 * the running mean and sum of squared deviations from which the standard
 * deviation is taken in one pass
 */
typedef struct
{
  int band;             /**< Band read, starting at 1 */
  bool exclude_nodata;  /**< True when nodata pixels are left out */
  uint64 count;         /**< Number of pixels read */
  double sum;           /**< Sum of their values */
  double min;           /**< Smallest of their values */
  double max;           /**< Largest of their values */
  double mean;          /**< Running mean of their values */
  double q;             /**< Running sum of squared deviations from the mean */
} RasterStatsState;

/* Input and output functions for PostGIS rasters */

extern Raster *raster_from_wkb(const uint8_t *wkb, size_t size);
extern Raster *raster_from_hexwkb(const char *hexwkb);
extern uint8_t *raster_as_wkb(const Raster *rast, uint8_t variant, size_t *size_out);
extern char *raster_as_hexwkb(const Raster *rast, uint8_t variant, size_t *size_out);

/* Constructor functions for PostGIS rasters */

extern Raster *raster_copy(const Raster *rast);
extern Raster *raster_make(int width, int height, double upperleftx,
  double upperlefty, double scalex, double scaley, double skewx, double skewy,
  int32_t srid, const char *pixeltype, double nodataval, bool has_nodata,
  const uint8_t *pixels, size_t size);
extern Raster *raster_make_empty(int width, int height, double upperleftx,
  double upperlefty, double scalex, double scaley, double skewx, double skewy,
  int32_t srid);
extern Raster *raster_add_band(const Raster *rast, int index,
  const char *pixeltype, double initialvalue, double nodataval,
  bool has_nodata);
extern Raster *raster_add_band_raster(const Raster *torast,
  const Raster *fromrast, int fromband, int toindex);

/* Accessor functions for PostGIS rasters */

extern int raster_mem_size(const Raster *rast);
extern int raster_num_bands(const Raster *rast);
extern int raster_width(const Raster *rast);
extern int raster_height(const Raster *rast);
extern int32_t raster_srid(const Raster *rast);
extern double raster_upper_left_x(const Raster *rast);
extern double raster_upper_left_y(const Raster *rast);
extern double raster_scale_x(const Raster *rast);
extern double raster_scale_y(const Raster *rast);
extern double raster_skew_x(const Raster *rast);
extern double raster_skew_y(const Raster *rast);
extern char *raster_band_pixel_type(const Raster *rast, int band);
extern bool raster_band_has_nodata_value(const Raster *rast, int band);
extern bool raster_band_nodata_value(const Raster *rast, int band,
  double *result);
extern uint8_t *raster_band_pixels(const Raster *rast, int band,
  size_t *size_out);
extern int64 bandstats_count(const BandStats *stats);
extern bool bandstats_sum(const BandStats *stats, double *result);
extern bool bandstats_mean(const BandStats *stats, double *result);
extern bool bandstats_stddev(const BandStats *stats, double *result);
extern bool bandstats_min(const BandStats *stats, double *result);
extern bool bandstats_max(const BandStats *stats, double *result);

/* Processing functions for PostGIS rasters */

extern Raster *raster_clip(const Raster *rast, const int *bands, int nbands,
  const GSERIALIZED *gs, const double *nodata, int nnodata, bool crop,
  bool touched);
extern Raster *raster_transform(const Raster *rast, int32_t srid,
  const char *algorithm, double max_err, double scale_x, double scale_y);
extern Raster *raster_transform_pipeline(const Raster *rast,
  const char *pipelinestr, int32_t srid, bool is_forward,
  const char *algorithm, double max_err, double scale_x, double scale_y);
extern Raster *raster_transform_raster(const Raster *rast,
  const Raster *alignto, const char *algorithm, double max_err);
extern Raster *raster_rescale(const Raster *rast, double scale_x,
  double scale_y, const char *algorithm, double max_err);
extern GeomVal *raster_dump_as_polygons(const Raster *rast, int band,
  bool exclude_nodata, int *count);
extern double *raster_dump_values(const Raster *rast, int band,
  bool exclude_nodata, bool **nodata, int *count);
extern Pixel *raster_pixel_as_points(const Raster *rast, int band,
  bool exclude_nodata, int *count);
extern BandStats *raster_summary_stats(const Raster *rast, int band,
  bool exclude_nodata);
extern ValueCount *raster_value_count(const Raster *rast, int band,
  bool exclude_nodata, const double *searchvalues, int nsearch, double roundto,
  int *count);
extern Raster *raster_set_values(const Raster *rast, int band, int x, int y,
  int width, int height, const double *values, const bool *nodata,
  const bool *noset, int nosetwidth, int nosetheight, int count,
  bool keepnodata);
extern Raster *raster_reclass(const Raster *rast, int band, const char *expr,
  const char *pixeltype, bool has_nodata, double nodataval);
extern Raster *raster_slope(const Raster *rast, int band,
  const Raster *customextent, const char *pixeltype, const char *units,
  double scale, bool interpolate_nodata);
extern Raster *raster_tpi(const Raster *rast, int band,
  const Raster *customextent, const char *pixeltype, bool interpolate_nodata);
extern Raster *raster_tri(const Raster *rast, int band,
  const Raster *customextent, const char *pixeltype, bool interpolate_nodata);
extern Raster *raster_map_algebra(const Raster *rast, int band,
  const char *stat, const char *pixeltype, const char *extenttype,
  const Raster *customextent, int32 distancex, int32 distancey);
extern void geomval_arr_free(GeomVal *gvarr, int count);
extern void pixel_arr_free(Pixel *pixarr, int count);

/* Mathematical functions for PostGIS rasters */

extern Raster *add_float_raster(double d, const Raster *rast);
extern Raster *add_raster_float(const Raster *rast, double d);
extern Raster *add_raster_raster(const Raster *rast1, const Raster *rast2);
extern Raster *div_float_raster(double d, const Raster *rast);
extern Raster *div_raster_float(const Raster *rast, double d);
extern Raster *div_raster_raster(const Raster *rast1, const Raster *rast2);
extern Raster *mul_float_raster(double d, const Raster *rast);
extern Raster *mul_raster_float(const Raster *rast, double d);
extern Raster *mul_raster_raster(const Raster *rast1, const Raster *rast2);
extern Raster *sub_float_raster(double d, const Raster *rast);
extern Raster *sub_raster_float(const Raster *rast, double d);
extern Raster *sub_raster_raster(const Raster *rast1, const Raster *rast2);

/* Comparison functions for PostGIS rasters */

extern bool raster_same_alignment(const Raster *rast1, const Raster *rast2);
extern char *raster_not_same_alignment_reason(const Raster *rast1,
  const Raster *rast2);

/* Aggregate functions for PostGIS rasters */

extern RasterAlignState *raster_same_alignment_combinefn(
  RasterAlignState *state1, const RasterAlignState *state2);
extern bool raster_same_alignment_finalfn(RasterAlignState *state);
extern RasterAlignState *raster_same_alignment_transfn(
  RasterAlignState *state, const Raster *rast);
extern RasterStatsState *raster_summary_stats_combinefn(
  RasterStatsState *state1, const RasterStatsState *state2);
extern BandStats *raster_summary_stats_finalfn(RasterStatsState *state);
extern RasterStatsState *raster_summary_stats_transfn(RasterStatsState *state,
  const Raster *rast, int band, bool exclude_nodata);
extern RasterUnionState *raster_union_combinefn(RasterUnionState *state1,
  const RasterUnionState *state2);
extern Raster *raster_union_finalfn(RasterUnionState *state);
extern RasterUnionState *raster_union_transfn(RasterUnionState *state,
  const Raster *rast, int band, const char *uniontype);
extern RasterAlignState *rasteralignstate_deserialize(const uint8_t *bytes,
  size_t size);
extern uint8_t *rasteralignstate_serialize(const RasterAlignState *state,
  size_t *size_out);
extern RasterStatsState *rasterstatsstate_deserialize(const uint8_t *bytes,
  size_t size);
extern uint8_t *rasterstatsstate_serialize(const RasterStatsState *state,
  size_t *size_out);
extern RasterUnionState *rasterunionstate_deserialize(const uint8_t *bytes,
  size_t size);
extern uint8_t *rasterunionstate_serialize(const RasterUnionState *state,
  size_t *size_out);

/* Conversion functions for Raquet tiles */

extern STBox *raquet_to_stbox(const Raquet *rq);

/* Conversion functions for PostGIS rasters */

extern STBox *raster_to_stbox(const Raster *rast);

/* Comparison functions for Raquet tiles */

extern int raquet_cmp(const Raquet *rq1, const Raquet *rq2);
extern bool raquet_eq(const Raquet *rq1, const Raquet *rq2);
extern bool raquet_ne(const Raquet *rq1, const Raquet *rq2);
extern bool raquet_lt(const Raquet *rq1, const Raquet *rq2);
extern bool raquet_le(const Raquet *rq1, const Raquet *rq2);
extern bool raquet_ge(const Raquet *rq1, const Raquet *rq2);
extern bool raquet_gt(const Raquet *rq1, const Raquet *rq2);

/* Sampling functions */

/* Sampling of a PostGIS raster: reads the band the position falls in, and
 * derives the bounding-box pre-filter from the raster extent */

extern Temporal *raster_value(const Temporal *traj, const Raster *rast,
  int band, bool exclude_nodata, const char *resample);
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
  Quadbin quadbin, MeosPixType pixtype, double nodata, bool has_nodata);

extern Temporal *raster_tile_value(const Temporal *traj, const Raquet *rq);
extern Temporal *raster_tile_value_array(const Temporal *traj,
  const Raquet **rqarr, int count);

/*****************************************************************************/

#endif /* __MEOS_RASTER_H__ */
