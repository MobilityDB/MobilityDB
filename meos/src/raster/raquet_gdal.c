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
 * ::T_RAQUET value identified by the supplied CARTO QUADBIN cell. The rest of
 * the family (serialization in `raquet.c`, sampling in `raster_quadbin.c`)
 * stays GDAL-free.
 *
 * The caller supplies the @p quadbin that identifies the Web-Mercator tile the
 * raster represents, so the decode reads pixels, type and nodata from the band
 * without interpreting the dataset geotransform or SRS.
 */

#include "raster/raquet.h"

/* C */
#include <stdint.h>
/* GDAL */
#include <gdal.h>
#include <cpl_error.h>
#include <cpl_vsi.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/temporal.h"

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
    default:
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Unsupported GDAL raster data type for raquet ingest: %s",
        GDALGetDataTypeName(dt));
      return false;
  }
}

/**
 * @brief Pack the first band of an open GDAL dataset into a Raquet tile
 * @details The band data type must be one of Byte / Int16 / Int32 / Float32 /
 * Float64; the band nodata value (if any) is carried into the tile. The caller
 * owns @p ds and closes it; @p label names the source in error messages.
 */
static Raquet *
raquet_from_gdal_dataset(GDALDatasetH ds, uint64 quadbin, const char *label)
{
  Raquet *result = NULL;
  uint8_t *buf = NULL;
  int xsize = GDALGetRasterXSize(ds);
  int ysize = GDALGetRasterYSize(ds);
  if (GDALGetRasterCount(ds) < 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster has no bands for raquet ingest: %s", label);
    goto cleanup;
  }
  if (xsize <= 0 || ysize <= 0 || xsize > UINT16_MAX || ysize > UINT16_MAX)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster dimensions %dx%d out of range for a raquet tile (1..%u)",
      xsize, ysize, (unsigned) UINT16_MAX);
    goto cleanup;
  }

  GDALRasterBandH band = GDALGetRasterBand(ds, 1);
  GDALDataType dt = GDALGetRasterDataType(band);
  MeosPixType pixtype;
  if (! gdal_to_pixtype(dt, &pixtype))
    goto cleanup;

  int has_nodata = 0;
  double nodata = GDALGetRasterNoDataValue(band, &has_nodata);

  size_t pixsize = raquet_pixtype_size(pixtype);
  size_t nbytes = (size_t) xsize * ysize * pixsize;
  buf = palloc(nbytes);
  if (GDALRasterIO(band, GF_Read, 0, 0, xsize, ysize, buf, xsize, ysize,
      dt, 0, 0) != CE_None)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "GDAL failed to read raster band for raquet ingest: %s", label);
    goto cleanup;
  }

  result = raquet_make(quadbin, (uint16) xsize, (uint16) ysize, pixtype,
    nodata, (bool) has_nodata, buf);

cleanup:
  if (buf)
    pfree(buf);
  return result;
}

/**
 * @ingroup meos_raster_base_constructor
 * @brief Return a Raquet tile read from a raster file via GDAL
 * @details GDAL decodes the file (any format it supports) and the first raster
 * band is packed row-major into a Raquet chip identified by @p quadbin.
 * @param[in] path Path to a GDAL-readable raster file
 * @param[in] quadbin CARTO QUADBIN cell identifying the Web-Mercator tile
 */
Raquet *
raquet_read(const char *path, uint64 quadbin)
{
  VALIDATE_NOT_NULL(path, NULL);

  GDALAllRegister();
  GDALDatasetH ds = GDALOpen(path, GA_ReadOnly);
  if (! ds)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot open raster file for raquet ingest: %s", path);
    return NULL;
  }
  Raquet *result = raquet_from_gdal_dataset(ds, quadbin, path);
  GDALClose(ds);
  return result;
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
 * @param[in] quadbin CARTO QUADBIN cell identifying the Web-Mercator tile
 */
Raquet *
raquet_read_bytes(const uint8_t *data, size_t size, uint64 quadbin)
{
  VALIDATE_NOT_NULL(data, NULL);

  GDALAllRegister();
  /* Expose the buffer to GDAL via /vsimem/. The path is made unique by the
   * buffer address so concurrent calls never collide; ownership stays with the
   * caller (bTakeOwnership = FALSE) and the buffer outlives the dataset. */
  char vpath[64];
  snprintf(vpath, sizeof(vpath), "/vsimem/raquet_%p", (const void *) data);
  VSILFILE *vf = VSIFileFromMemBuffer(vpath, (GByte *) (uintptr_t) data,
    (vsi_l_offset) size, FALSE);
  if (! vf)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot expose the in-memory raster to GDAL for raquet ingest");
    return NULL;
  }
  VSIFCloseL(vf);

  GDALDatasetH ds = GDALOpen(vpath, GA_ReadOnly);
  if (! ds)
  {
    VSIUnlink(vpath);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "GDAL cannot decode the in-memory raster for raquet ingest");
    return NULL;
  }
  Raquet *result = raquet_from_gdal_dataset(ds, quadbin, "in-memory raster");
  GDALClose(ds);
  VSIUnlink(vpath);
  return result;
}

/*****************************************************************************/
