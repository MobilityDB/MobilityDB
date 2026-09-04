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
 * @brief GDAL-backed ingest of a raster file into a Raquet raster chip.
 *
 * This is the one place in the RASTER family that depends on GDAL: MEOS owns
 * the raster-format decode (as it owns geometry decode via PostGIS/GEOS),
 * rather than delegating it to an external tool. GDAL opens any of its
 * supported raster formats, and the first band is packed row-major into the
 * ::T_RAQUET value identified by a CARTO QUADBIN cell. The rest of the family
 * (serialization in `raquet.c`, sampling in `raster_quadbin.c`) stays GDAL-free.
 *
 * The caller supplies the @p quadbin that identifies the Web-Mercator tile the
 * raster represents; passing 0 instead derives it from the dataset EPSG:3857
 * geotransform and spatial reference (the raster must be a single Web-Mercator
 * QUADBIN tile).
 */

#include "raster/raquet.h"

/* C */
#include <math.h>
#include <stdint.h>
#include <string.h>
/* GDAL */
#include <gdal.h>
#include <cpl_error.h>
#include <cpl_vsi.h>
#include <ogr_srs_api.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
// #include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/temporal.h"
#include "raster/raster_quadbin.h"

/*****************************************************************************/

/**
 * @brief Map a GDAL data type to the corresponding Raquet pixel type
 * @return true on success; on failure sets a MEOS error and returns false
 */
static bool
gdal_to_pixtype(GDALDataType dt, MeosPixType *pixtype)
{
  switch (dt)
  {
    case GDT_Byte:    *pixtype = MEOS_PT_UINT8;   return true;
    case GDT_Int16:   *pixtype = MEOS_PT_INT16;   return true;
    case GDT_Int32:   *pixtype = MEOS_PT_INT32;   return true;
    case GDT_Float32: *pixtype = MEOS_PT_FLOAT32; return true;
    case GDT_Float64: *pixtype = MEOS_PT_FLOAT64; return true;
    case GDT_UInt16:  *pixtype = MEOS_PT_UINT16;  return true;
    case GDT_UInt32:  *pixtype = MEOS_PT_UINT32;  return true;
    /* The types GDAL has gained over its releases, each read by the version
     * that names it */
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,7,0)
    case GDT_Int8:    *pixtype = MEOS_PT_INT8;    return true;
#endif
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,5,0)
    case GDT_Int64:   *pixtype = MEOS_PT_INT64;   return true;
    case GDT_UInt64:  *pixtype = MEOS_PT_UINT64;  return true;
#endif
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,11,0)
    case GDT_Float16: *pixtype = MEOS_PT_FLOAT16; return true;
#endif
    default:
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Unsupported GDAL raster data type for raquet ingest: %s",
        GDALGetDataTypeName(dt));
      return false;
  }
}

/*****************************************************************************
 * GDAL error handling
 *
 * GDAL announces a failure twice: it returns an error indication -- a NULL
 * dataset, a CPLErr other than CE_None -- and it calls the error handler the
 * process has installed. The code below reads the return value, which is the
 * contract C code can honour.
 *
 * Reading it is sound only while MEOS owns the handler. A host embedding MEOS
 * beside another GDAL consumer need not leave the handler alone, and one that
 * throws unwinds through the C frames here, past the release every path ends
 * with. Each GDAL call therefore sits between #meos_gdal_enter and
 * #meos_gdal_leave: the push puts a quiet handler on top of GDAL's per-thread
 * handler stack, so a failure comes back as a return value, and the pop hands
 * the handler back for the host's own calls. Reporting happens after the pop,
 * never inside the bracket, because a report need not return.
 *****************************************************************************/

/**
 * @brief Take the GDAL error handler for the duration of one GDAL call, so
 * that the call reports its failure by return value
 */
static void
meos_gdal_enter(void)
{
  CPLErrorReset();
  CPLPushErrorHandler(CPLQuietErrorHandler);
  return;
}

/**
 * @brief Give the GDAL error handler back to the host
 */
static void
meos_gdal_leave(void)
{
  CPLPopErrorHandler();
  return;
}

/**
 * @brief Return what GDAL recorded about the last failure, for the message
 * MEOS composes about it
 * @details GDAL keeps the text in per-thread state that outlives the handler
 * bracket, so this reads correctly after #meos_gdal_leave
 */
static const char *
meos_gdal_error(void)
{
  const char *msg = CPLGetLastErrorMsg();
  return (msg && *msg) ? msg : "GDAL gave no further detail";
}

/**
 * @brief Derive the QUADBIN cell of a raster tile from an open GDAL dataset's
 * EPSG:3857 geotransform and spatial reference
 * @details The dataset must carry an EPSG:3857 (Web-Mercator) spatial reference
 * and an axis-aligned (north-up) geotransform describing a single QUADBIN tile;
 * a missing/rotated geotransform or a non-Web-Mercator reference is an error.
 * @return true on success; on failure sets a MEOS error and returns false
 */
static bool
derive_quadbin(GDALDatasetH ds, uint64 *quadbin, const char *label)
{
  double gt[6];
  meos_gdal_enter();
  CPLErr gterr = GDALGetGeoTransform(ds, gt);
  meos_gdal_leave();
  if (gterr != CE_None)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster has no geotransform; cannot derive its QUADBIN cell: %s", label);
    return false;
  }
  if (gt[2] != 0.0 || gt[4] != 0.0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster geotransform is rotated; cannot derive its QUADBIN cell: %s",
      label);
    return false;
  }
  meos_gdal_enter();
  OGRSpatialReferenceH srs = GDALGetSpatialRef(ds);
  const char *code = srs ? OSRGetAuthorityCode(srs, NULL) : NULL;
  meos_gdal_leave();
  if (! code || strcmp(code, "3857") != 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster is not in EPSG:3857 (Web-Mercator); cannot derive its QUADBIN "
      "cell: %s", label);
    return false;
  }
  return raster_quadbin_from_bounds(gt[0], gt[3], gt[1], gt[5],
    GDALGetRasterXSize(ds), GDALGetRasterYSize(ds), quadbin);
}

/**
 * @brief Release the resources a raquet ingest holds
 * @details Every error path calls this before #meos_error, because the error
 * handler a host installs need not return to the caller: the PostgreSQL
 * handler leaves through `ereport(ERROR)` and the standalone one exits, so a
 * release written after the report never runs and the dataset leaks. Reporting
 * last is the idiom the rest of MEOS follows.
 * @param[in] ds Open GDAL dataset, or NULL
 * @param[in] vpath `/vsimem/` path backing @p ds, or NULL for a real file
 * @param[in] buf Band buffer, or NULL
 */
static void
raquet_gdal_release(GDALDatasetH ds, const char *vpath, uint8_t *buf)
{
  if (buf)
    pfree(buf);
  meos_gdal_enter();
  if (ds)
    GDALClose(ds);
  if (vpath)
    VSIUnlink(vpath);
  meos_gdal_leave();
  return;
}

/**
 * @brief Pack the first band of an open GDAL dataset into a Raquet tile
 * @details The band data type must be one of Byte / Int16 / Int32 / Float32 /
 * Float64; the band nodata value (if any) is carried into the tile. When
 * @p quadbin is 0 the tile identifier is derived from the dataset
 * georeferencing. This function takes over @p ds and the `/vsimem/` file
 * @p vpath, releasing both on every path, so that the release still happens
 * when a report does not return; @p label names the source in error messages.
 * @param[in] ds Open GDAL dataset, taken over by this function
 * @param[in] vpath `/vsimem/` path backing @p ds, or NULL for a real file
 * @param[in] quadbin CARTO QUADBIN cell, or 0 to derive it
 * @param[in] label Source name used in error messages
 */
static Raquet *
raquet_from_gdal_dataset(GDALDatasetH ds, const char *vpath, uint64 quadbin,
  const char *label)
{
  Raquet *result = NULL;
  uint8_t *buf = NULL;
  int xsize = GDALGetRasterXSize(ds);
  int ysize = GDALGetRasterYSize(ds);
  if (GDALGetRasterCount(ds) < 1)
  {
    raquet_gdal_release(ds, vpath, buf);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster has no bands for raquet ingest: %s", label);
    return NULL;
  }
  if (xsize <= 0 || ysize <= 0 || xsize > UINT16_MAX || ysize > UINT16_MAX)
  {
    raquet_gdal_release(ds, vpath, buf);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster dimensions %dx%d out of range for a raquet tile (1..%u)",
      xsize, ysize, (unsigned) UINT16_MAX);
    return NULL;
  }

  /* A zero quadbin requests deriving the tile identifier from the dataset
   * geotransform and spatial reference. The callee reports, so the release
   * comes first and holds whether or not that report returns. */
  if (quadbin == 0)
  {
    uint64 cell;
    if (! derive_quadbin(ds, &cell, label))
    {
      raquet_gdal_release(ds, vpath, buf);
      return NULL;
    }
    quadbin = cell;
  }

  meos_gdal_enter();
  GDALRasterBandH band = GDALGetRasterBand(ds, 1);
  GDALDataType dt = GDALGetRasterDataType(band);
  meos_gdal_leave();
  MeosPixType pixtype;
  if (! gdal_to_pixtype(dt, &pixtype))
  {
    raquet_gdal_release(ds, vpath, buf);
    return NULL;
  }

  int has_nodata = 0;
  double nodata = GDALGetRasterNoDataValue(band, &has_nodata);

  size_t pixsize = raquet_pixtype_size(pixtype);
  size_t nbytes = (size_t) xsize * ysize * pixsize;
  buf = palloc(nbytes);
  meos_gdal_enter();
  CPLErr ioerr = GDALRasterIO(band, GF_Read, 0, 0, xsize, ysize, buf, xsize,
    ysize, dt, 0, 0);
  meos_gdal_leave();
  if (ioerr != CE_None)
  {
    char detail[512];
    snprintf(detail, sizeof(detail), "%s", meos_gdal_error());
    raquet_gdal_release(ds, vpath, buf);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "GDAL failed to read raster band for raquet ingest: %s (%s)", label,
      detail);
    return NULL;
  }
  /* GDAL hands over the band in the byte order of this machine, while a Raquet
   * band is little-endian wherever it was made, so that the tile keeps its
   * meaning once it is serialized and read on another host */
  raquet_pixels_from_host(buf, (size_t) xsize * ysize, pixtype);

  result = raquet_make(quadbin, xsize, ysize, pixtype,
    nodata, (bool) has_nodata, buf, nbytes);

  raquet_gdal_release(ds, vpath, buf);
  return result;
}

/**
 * @ingroup meos_raster_base_constructor
 * @brief Return a Raquet tile read from a raster file via GDAL
 * @details GDAL decodes the file (any format it supports) and the first raster
 * band is packed row-major into a Raquet chip identified by @p quadbin.
 * @param[in] path Path to a GDAL-readable raster file
 * @param[in] quadbin CARTO QUADBIN cell identifying the Web-Mercator tile, or 0
 * to derive it from the raster geotransform and EPSG:3857 spatial reference
 * @note The SQL surface reads the raster from bytes rather than a server-side
 * path, so the wrapper binds #raquet_read_bytes() and this function carries no
 * @p csqlfn link
 */
Raquet *
raquet_read(const char *path, uint64 quadbin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(path, NULL);

  GDALAllRegister();
  meos_gdal_enter();
  GDALDatasetH ds = GDALOpen(path, GA_ReadOnly);
  meos_gdal_leave();
  if (! ds)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot open raster file for raquet ingest: %s (%s)", path,
      meos_gdal_error());
    return NULL;
  }
  /* The callee takes over the dataset and closes it on every path */
  return raquet_from_gdal_dataset(ds, NULL, quadbin, path);
}

/**
 * @ingroup meos_raster_base_constructor
 * @brief Return a Raquet tile decoded from an in-memory raster file via GDAL
 * @details Identical to #raquet_read but the raster file is supplied as a byte
 * buffer (e.g. a `bytea`) rather than a filesystem path. The bytes are exposed
 * to GDAL through its `/vsimem/` virtual filesystem, so no server-side file
 * access is required.
 * @param[in] data Raster file bytes (any GDAL-supported format)
 * @param[in] size Number of bytes in @p data
 * @param[in] quadbin CARTO QUADBIN cell identifying the Web-Mercator tile, or 0
 * to derive it from the raster geotransform and EPSG:3857 spatial reference
 * @csqlfn #Raquet_read()
 */
Raquet *
raquet_read_bytes(const uint8_t *data, size_t size, uint64 quadbin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(data, NULL);

  GDALAllRegister();
  /* Expose the buffer to GDAL via /vsimem/. The path is made unique by the
   * buffer address so concurrent calls never collide; ownership stays with the
   * caller (bTakeOwnership = FALSE) and the buffer outlives the dataset. */
  char vpath[64];
  snprintf(vpath, sizeof(vpath), "/vsimem/raquet_%p", (const void *) data);
  meos_gdal_enter();
  VSILFILE *vf = VSIFileFromMemBuffer(vpath, (GByte *) (uintptr_t) data,
    (vsi_l_offset) size, FALSE);
  meos_gdal_leave();
  if (! vf)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot expose the in-memory raster to GDAL for raquet ingest (%s)",
      meos_gdal_error());
    return NULL;
  }
  meos_gdal_enter();
  VSIFCloseL(vf);
  GDALDatasetH ds = GDALOpen(vpath, GA_ReadOnly);
  meos_gdal_leave();
  if (! ds)
  {
    char detail[512];
    snprintf(detail, sizeof(detail), "%s", meos_gdal_error());
    raquet_gdal_release(NULL, vpath, NULL);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "GDAL cannot decode the in-memory raster for raquet ingest (%s)",
      detail);
    return NULL;
  }
  /* The callee takes over the dataset and the `/vsimem/` file, releasing both
   * on every path */
  return raquet_from_gdal_dataset(ds, vpath, quadbin, "in-memory raster");
}

/*****************************************************************************
 * GDAL-backed raster_value sampler: gives any MEOS caller (not just the PG
 * raster binding) a working sample callback over a GDAL-readable raster file
 *****************************************************************************/

/**
 * @brief Per-call state for #raster_value_gdal_sample: the raster band to
 * read, the inverse geotransform mapping a geographic point to a pixel
 * (col, row), the band size for bounds-checking, and the nodata sentinel
 */
typedef struct
{
  GDALRasterBandH band;
  double inv_gt[6];
  int xsize;
  int ysize;
  int has_nodata;
  double nodata;
} RasterValueGdalCtx;

/**
 * @brief Raster sampling callback backed by a single-pixel GDALRasterIO
 * read: the context carries the band and the inverse geotransform, only the
 * point argument varies per call
 */
static bool
raster_value_gdal_sample(void *ctxp, const GSERIALIZED *point, double *value)
{
  RasterValueGdalCtx *ctx = (RasterValueGdalCtx *) ctxp;
  GBOX gbox;
  gserialized_get_gbox_p((GSERIALIZED *) point, &gbox);
  double col_f, row_f;
  GDALApplyGeoTransform(ctx->inv_gt, gbox.xmin, gbox.ymin, &col_f, &row_f);
  int col = (int) floor(col_f);
  int row = (int) floor(row_f);
  if (col < 0 || col >= ctx->xsize || row < 0 || row >= ctx->ysize)
    return false;   /* point outside the pixel grid */
  double val;
  /* No bracket here: the caller holds the handler across the whole walk, and
   * pushing one per instant costs a CPL allocation per sample */
  if (GDALRasterIO(ctx->band, GF_Read, col, row, 1, 1, &val, 1, 1,
      GDT_Float64, 0, 0) != CE_None)
    return false;
  if (ctx->has_nodata && val == ctx->nodata)
    return false;   /* nodata pixel */
  *value = val;
  return true;
}

/**
 * @brief Open a raster file via GDAL and build the #RasterValueGdalCtx and
 * the extent STBox pre-filter shared by every raster_value_gdal wrapper
 * @param[in] path Path to a GDAL-readable raster file
 * @param[in] band_num Band number (1-based)
 * @param[out] ds_out Open GDAL dataset; the caller closes it with GDALClose
 * @param[out] ctx Sampling context, valid while @p ds_out stays open
 * @param[out] box Bounding-box pre-filter derived from the geotransform
 * @return true on success; on failure sets a MEOS error and returns false
 */
static bool
raster_value_gdal_open(const char *path, int band_num, GDALDatasetH *ds_out,
  RasterValueGdalCtx *ctx, STBox *box)
{
  GDALAllRegister();
  meos_gdal_enter();
  GDALDatasetH ds = GDALOpen(path, GA_ReadOnly);
  meos_gdal_leave();
  if (! ds)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot open raster file: %s (%s)", path, meos_gdal_error());
    return false;
  }
  double gt[6];
  meos_gdal_enter();
  CPLErr gterr = GDALGetGeoTransform(ds, gt);
  meos_gdal_leave();
  if (gterr != CE_None)
  {
    raquet_gdal_release(ds, NULL, NULL);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster has no geotransform: %s", path);
    return false;
  }
  if (! GDALInvGeoTransform(gt, ctx->inv_gt))
  {
    raquet_gdal_release(ds, NULL, NULL);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster geotransform is not invertible: %s", path);
    return false;
  }
  meos_gdal_enter();
  GDALRasterBandH rb = GDALGetRasterBand(ds, band_num);
  meos_gdal_leave();
  if (! rb)
  {
    raquet_gdal_release(ds, NULL, NULL);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster has no band %d: %s", band_num, path);
    return false;
  }
  int has_nodata = 0;
  double nodata = GDALGetRasterNoDataValue(rb, &has_nodata);

  ctx->band = rb;
  ctx->xsize = GDALGetRasterBandXSize(rb);
  ctx->ysize = GDALGetRasterBandYSize(rb);
  ctx->has_nodata = has_nodata;
  ctx->nodata = nodata;

  /* Bounding box of the raster extent, from the four corners of the
   * geotransform (correct even for a rotated geotransform) */
  int xsize = GDALGetRasterXSize(ds);
  int ysize = GDALGetRasterYSize(ds);
  double xs[4], ys[4];
  GDALApplyGeoTransform(gt, 0, 0, &xs[0], &ys[0]);
  GDALApplyGeoTransform(gt, xsize, 0, &xs[1], &ys[1]);
  GDALApplyGeoTransform(gt, 0, ysize, &xs[2], &ys[2]);
  GDALApplyGeoTransform(gt, xsize, ysize, &xs[3], &ys[3]);
  memset(box, 0, sizeof(STBox));
  box->xmin = box->xmax = xs[0];
  box->ymin = box->ymax = ys[0];
  for (int i = 1; i < 4; i++)
  {
    if (xs[i] < box->xmin) box->xmin = xs[i];
    if (xs[i] > box->xmax) box->xmax = xs[i];
    if (ys[i] < box->ymin) box->ymin = ys[i];
    if (ys[i] > box->ymax) box->ymax = ys[i];
  }

  *ds_out = ds;
  return true;
}

/**
 * @ingroup meos_raster
 * @brief Return the values of a raster band sampled at the instants of a
 * trajectory, reading the raster through GDAL
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] path Path to a GDAL-readable raster file
 * @param[in] band Band number (1-based)
 */
Temporal *
raster_value_gdal(const Temporal *traj, const char *path, int band)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(path, NULL); VALIDATE_NOT_NULL(traj, NULL);

  GDALDatasetH ds;
  RasterValueGdalCtx ctx;
  STBox box;
  if (! raster_value_gdal_open(path, band, &ds, &ctx, &box))
    return NULL;
  meos_gdal_enter();
  Temporal *result = raster_value_sampler(traj, &box, &raster_value_gdal_sample,
    &ctx);
  meos_gdal_leave();
  raquet_gdal_release(ds, NULL, NULL);
  return result;
}

/**
 * @ingroup meos_raster
 * @brief Return the instants of a trajectory where the raster pixel value,
 * read through GDAL, falls inside a float span
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] path Path to a GDAL-readable raster file
 * @param[in] band Band number (1-based)
 * @param[in] vspan Float value range (inclusive bounds)
 */
Temporal *
raster_at_value_gdal(const Temporal *traj, const char *path, int band,
  const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(path, NULL); VALIDATE_NOT_NULL(traj, NULL);
  VALIDATE_NOT_NULL(vspan, NULL);

  GDALDatasetH ds;
  RasterValueGdalCtx ctx;
  STBox box;
  if (! raster_value_gdal_open(path, band, &ds, &ctx, &box))
    return NULL;
  meos_gdal_enter();
  Temporal *result = raster_at_value_sampler(traj, &box, &raster_value_gdal_sample,
    &ctx, vspan);
  meos_gdal_leave();
  raquet_gdal_release(ds, NULL, NULL);
  return result;
}

/**
 * @ingroup meos_raster
 * @brief Return the instants of a trajectory where the raster pixel value,
 * read through GDAL, falls outside a float span
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] path Path to a GDAL-readable raster file
 * @param[in] band Band number (1-based)
 * @param[in] vspan Float value range to exclude
 */
Temporal *
raster_minus_value_gdal(const Temporal *traj, const char *path, int band,
  const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(path, NULL); VALIDATE_NOT_NULL(traj, NULL);
  VALIDATE_NOT_NULL(vspan, NULL);

  GDALDatasetH ds;
  RasterValueGdalCtx ctx;
  STBox box;
  if (! raster_value_gdal_open(path, band, &ds, &ctx, &box))
    return NULL;
  meos_gdal_enter();
  Temporal *result = raster_minus_value_sampler(traj, &box,
    &raster_value_gdal_sample, &ctx, vspan);
  meos_gdal_leave();
  raquet_gdal_release(ds, NULL, NULL);
  return result;
}

/**
 * @ingroup meos_raster
 * @brief Return true if a trajectory ever samples a raster pixel value,
 * read through GDAL, inside a float span
 * @param[in] path Path to a GDAL-readable raster file
 * @param[in] band Band number (1-based)
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] vspan Float value range
 * @return 1 if the trajectory ever samples a value inside @p vspan, 0 if
 * not, and -1 on error
 */
int
eraster_value_gdal(const Temporal *traj, const char *path, int band,
  const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(path, -1); VALIDATE_NOT_NULL(traj, -1);
  VALIDATE_NOT_NULL(vspan, -1);

  GDALDatasetH ds;
  RasterValueGdalCtx ctx;
  STBox box;
  if (! raster_value_gdal_open(path, band, &ds, &ctx, &box))
    return -1;
  meos_gdal_enter();
  int result = eraster_value_sampler(traj, &box, &raster_value_gdal_sample, &ctx,
    vspan);
  meos_gdal_leave();
  raquet_gdal_release(ds, NULL, NULL);
  return result;
}

/**
 * @ingroup meos_raster
 * @brief Return true if every in-raster-extent instant of a trajectory
 * samples a raster pixel value, read through GDAL, inside a float span
 * @param[in] path Path to a GDAL-readable raster file
 * @param[in] band Band number (1-based)
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] vspan Float value range
 * @return 1 if every sampled value falls inside @p vspan, 0 if not, and -1
 * on error
 */
int
araster_value_gdal(const Temporal *traj, const char *path, int band,
  const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(path, -1); VALIDATE_NOT_NULL(traj, -1);
  VALIDATE_NOT_NULL(vspan, -1);

  GDALDatasetH ds;
  RasterValueGdalCtx ctx;
  STBox box;
  if (! raster_value_gdal_open(path, band, &ds, &ctx, &box))
    return -1;
  meos_gdal_enter();
  int result = araster_value_sampler(traj, &box, &raster_value_gdal_sample, &ctx,
    vspan);
  meos_gdal_leave();
  raquet_gdal_release(ds, NULL, NULL);
  return result;
}

/*****************************************************************************/
