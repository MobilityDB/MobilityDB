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
 * @brief PostgreSQL functions of the raster sampling operators
 * @details The wrappers pass their arguments to MEOS, which samples a PostGIS
 * raster
 * through the vendored raster core: a `raster` column is detoasted into the
 * serialized form the MEOS functions take, so the operators answer the same
 * values to every binding of the library and not to PostgreSQL alone.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <funcapi.h>
#include <access/htup_details.h>
#include <catalog/pg_type.h>
#include <commands/extension.h>
#include <executor/executor.h>  /* GetAttributeByNum */
#include <utils/array.h>
#include <utils/guc.h>
#include <utils/lsyscache.h>
#include <utils/syscache.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_raster.h>
#include <pgtypes.h>          /* text_to_cstring, cstring_to_text */
#include "temporal/span.h"    /* PG_GETARG_SPAN_P */
#include "temporal/type_util.h" /* bstring2bytea */
#include "geo/stbox.h"        /* PG_RETURN_STBOX_P */
#include "raster/raquet.h"    /* Raquet, PG_GETARG_RAQUET_P, raquet_pixtype_size */
#include "raster/raster_quadbin.h"
#include "quadbin/quadbin.h"  /* PG_GETARG_QUADBIN, PG_RETURN_QUADBIN */
/* MobilityDB */
#include "pg_geo/postgis.h"   /* PG_GETARG_GSERIALIZED_P */
#include "pg_temporal/temporal.h"
#include "pg_temporal/type_util.h" /* raquetarr_extract */
#include "pg_raster/temporal_raster.h"

PGDLLEXPORT Datum Raster_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the values of a raster band read along a trajectory
 * @param[in] traj Trajectory
 * @param[in] rast Raster
 * @param[in] band Band number (1-based, default 1)
 * @param[in] exclude_nodata_value True to leave out the positions over a
 * nodata pixel (default true)
 * @param[in] resample Name of the read of a position, nearest or bilinear
 * (default nearest)
 * @sqlfn rasterValue()
 */
Datum
Raster_value(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  int32 band = PG_ARGISNULL(2) ? 1 : PG_GETARG_INT32(2);
  bool exclude_nodata = PG_GETARG_BOOL(3);
  char *resample = text_to_cstring(PG_GETARG_TEXT_P(4));

  Temporal *result = raster_value(traj, rast, band, exclude_nodata, resample);

  pfree(resample);
  PG_FREE_IF_COPY(traj, 0);
  PG_FREE_IF_COPY(rast, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * atRasterValue / minusRasterValue / eRasterValue / aRasterValue
 *****************************************************************************/

PGDLLEXPORT Datum Raster_at_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_at_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the instants of a trajectory where the sampled raster pixel
 * value falls inside a float range
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] rast Raster
 * @param[in] vspan Float value range (inclusive bounds)
 * @param[in] band Band number (1-based, default 1)
 * @sqlfn atRasterValue()
 */
Datum
Raster_at_value(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  Span *vspan = PG_GETARG_SPAN_P(2);
  int32 band = PG_ARGISNULL(3) ? 1 : PG_GETARG_INT32(3);

  Temporal *result = raster_at_value(traj, rast, band, vspan);

  PG_FREE_IF_COPY(traj, 0);
  PG_FREE_IF_COPY(rast, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_minus_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_minus_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the instants of a trajectory where the sampled raster pixel
 * value falls outside a float range
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] rast Raster
 * @param[in] vspan Float value range to exclude
 * @param[in] band Band number (1-based, default 1)
 * @sqlfn minusRasterValue()
 */
Datum
Raster_minus_value(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  Span *vspan = PG_GETARG_SPAN_P(2);
  int32 band = PG_ARGISNULL(3) ? 1 : PG_GETARG_INT32(3);

  Temporal *result = raster_minus_value(traj, rast, band, vspan);

  PG_FREE_IF_COPY(traj, 0);
  PG_FREE_IF_COPY(rast, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Eraster_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eraster_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if a trajectory ever samples a raster pixel value
 * inside a float range
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] rast Raster
 * @param[in] vspan Float value range
 * @param[in] band Band number (1-based, default 1)
 * @sqlfn eRasterValue()
 */
Datum
Eraster_value(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  Span *vspan = PG_GETARG_SPAN_P(2);
  int32 band = PG_ARGISNULL(3) ? 1 : PG_GETARG_INT32(3);

  int result = eraster_value(traj, rast, band, vspan);

  PG_FREE_IF_COPY(traj, 0);
  PG_FREE_IF_COPY(rast, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Araster_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Araster_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if every in-raster-extent instant of a trajectory
 * samples a pixel value inside a float range
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] rast Raster
 * @param[in] vspan Float value range
 * @param[in] band Band number (1-based, default 1)
 * @sqlfn aRasterValue()
 */
Datum
Araster_value(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  Span *vspan = PG_GETARG_SPAN_P(2);
  int32 band = PG_ARGISNULL(3) ? 1 : PG_GETARG_INT32(3);

  int result = araster_value(traj, rast, band, vspan);

  PG_FREE_IF_COPY(traj, 0);
  PG_FREE_IF_COPY(rast, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * The file forms: a raster file on the server read through GDAL
 *****************************************************************************/

/**
 * @brief Return true if a raster file on the server may be read, as PostGIS
 * allows a raster band stored outside the database to be read
 * @details PostGIS applies `postgis.gdal_enabled_drivers` to GDAL when its
 * raster library is loaded, so the library is loaded before the file is
 * opened. The file is then read where `postgis.enable_outdb_rasters` is on,
 * and a path through a GDAL virtual file system other than `/vsimem/` where
 * the drivers enabled include `VSICURL`, the tests PostGIS makes on a path
 * before it opens it.
 * @param[in] path Path of the raster file
 */
static bool
ensure_raster_file_readable(const char *path)
{
  /* Looking up the input function of the raster type loads the PostGIS
   * raster library, whose initialization applies the enabled drivers */
  Oid ext_oid = get_extension_oid("postgis_raster", true);
  Oid type_oid = OidIsValid(ext_oid) ?
    GetSysCacheOid2(TYPENAMENSP, Anum_pg_type_oid, CStringGetDatum("raster"),
      ObjectIdGetDatum(get_extension_schema(ext_oid))) : InvalidOid;
  if (OidIsValid(type_oid))
  {
    Oid infunc, ioparam;
    FmgrInfo finfo;
    getTypeInputInfo(type_oid, &infunc, &ioparam);
    fmgr_info(infunc, &finfo);
  }
  const char *outdb = GetConfigOption("postgis.enable_outdb_rasters", true,
    false);
  if (! outdb || strcmp(outdb, "on") != 0)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "Reading the raster file %s on the server is disabled by "
      "postgis.enable_outdb_rasters", path);
    return false;
  }
  const char *drivers = GetConfigOption("postgis.gdal_enabled_drivers", true,
    false);
  if (! drivers || strstr(drivers, "DISABLE_ALL"))
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "Cannot read the raster file %s: postgis.gdal_enabled_drivers disables "
      "every GDAL driver", path);
    return false;
  }
  if (! strstr(drivers, "ENABLE_ALL") && strstr(path, "/vsi") &&
      ! strstr(path, "/vsimem") && ! strstr(drivers, "VSICURL"))
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "Cannot read the raster file %s: postgis.gdal_enabled_drivers does not "
      "enable VSICURL", path);
    return false;
  }
  return true;
}

PGDLLEXPORT Datum Raster_value_gdal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_value_gdal);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the values of a band of a raster file on the server read
 * along a trajectory
 * @param[in] traj Trajectory
 * @param[in] path Path of a raster file on the server
 * @param[in] band Band number (1-based, default 1)
 * @sqlfn rasterValue()
 */
Datum
Raster_value_gdal(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  char *path = text_to_cstring(PG_GETARG_TEXT_PP(1));
  int32 band = PG_ARGISNULL(2) ? 1 : PG_GETARG_INT32(2);
  if (! ensure_raster_file_readable(path))
    PG_RETURN_NULL();

  Temporal *result = raster_value_gdal(traj, path, band);

  pfree(path);
  PG_FREE_IF_COPY(traj, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_at_value_gdal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_at_value_gdal);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the instants of a trajectory where the value it reads from a
 * raster file on the server falls inside a float range
 * @param[in] traj Trajectory
 * @param[in] path Path of a raster file on the server
 * @param[in] vspan Float value range (inclusive bounds)
 * @param[in] band Band number (1-based, default 1)
 * @sqlfn atRasterValue()
 */
Datum
Raster_at_value_gdal(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  char *path = text_to_cstring(PG_GETARG_TEXT_PP(1));
  Span *vspan = PG_GETARG_SPAN_P(2);
  int32 band = PG_ARGISNULL(3) ? 1 : PG_GETARG_INT32(3);
  if (! ensure_raster_file_readable(path))
    PG_RETURN_NULL();

  Temporal *result = raster_at_value_gdal(traj, path, band, vspan);

  pfree(path);
  PG_FREE_IF_COPY(traj, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_minus_value_gdal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_minus_value_gdal);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the instants of a trajectory where the value it reads from a
 * raster file on the server falls outside a float range
 * @param[in] traj Trajectory
 * @param[in] path Path of a raster file on the server
 * @param[in] vspan Float value range to exclude
 * @param[in] band Band number (1-based, default 1)
 * @sqlfn minusRasterValue()
 */
Datum
Raster_minus_value_gdal(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  char *path = text_to_cstring(PG_GETARG_TEXT_PP(1));
  Span *vspan = PG_GETARG_SPAN_P(2);
  int32 band = PG_ARGISNULL(3) ? 1 : PG_GETARG_INT32(3);
  if (! ensure_raster_file_readable(path))
    PG_RETURN_NULL();

  Temporal *result = raster_minus_value_gdal(traj, path, band, vspan);

  pfree(path);
  PG_FREE_IF_COPY(traj, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Eraster_value_gdal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eraster_value_gdal);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if a trajectory ever reads a value inside a float range
 * from a raster file on the server
 * @param[in] traj Trajectory
 * @param[in] path Path of a raster file on the server
 * @param[in] vspan Float value range
 * @param[in] band Band number (1-based, default 1)
 * @sqlfn eRasterValue()
 */
Datum
Eraster_value_gdal(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  char *path = text_to_cstring(PG_GETARG_TEXT_PP(1));
  Span *vspan = PG_GETARG_SPAN_P(2);
  int32 band = PG_ARGISNULL(3) ? 1 : PG_GETARG_INT32(3);
  if (! ensure_raster_file_readable(path))
    PG_RETURN_NULL();

  int result = eraster_value_gdal(traj, path, band, vspan);

  pfree(path);
  PG_FREE_IF_COPY(traj, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Araster_value_gdal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Araster_value_gdal);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if every value a trajectory reads from a raster file on
 * the server falls inside a float range
 * @param[in] traj Trajectory
 * @param[in] path Path of a raster file on the server
 * @param[in] vspan Float value range
 * @param[in] band Band number (1-based, default 1)
 * @sqlfn aRasterValue()
 */
Datum
Araster_value_gdal(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  char *path = text_to_cstring(PG_GETARG_TEXT_PP(1));
  Span *vspan = PG_GETARG_SPAN_P(2);
  int32 band = PG_ARGISNULL(3) ? 1 : PG_GETARG_INT32(3);
  if (! ensure_raster_file_readable(path))
    PG_RETURN_NULL();

  int result = araster_value_gdal(traj, path, band, vspan);

  pfree(path);
  PG_FREE_IF_COPY(traj, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * raster_num_bands
 *****************************************************************************/

PGDLLEXPORT Datum Raster_num_bands(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_num_bands);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the number of bands of a raster
 * @param[in] rast Raster
 * @sqlfn numBands()
 */
Datum
Raster_num_bands(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  int result = raster_num_bands(rast);
  PG_RETURN_INT32(result);
}

/*****************************************************************************
 * Shape of a raster
 *****************************************************************************/

PGDLLEXPORT Datum Raster_width(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_width);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the width in pixels of a raster
 * @param[in] rast Raster
 * @sqlfn width()
 * @altsqlfn rasterWidth()
 */
Datum
Raster_width(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  int result = raster_width(rast);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Raster_height(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_height);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the height in pixels of a raster
 * @param[in] rast Raster
 * @sqlfn height()
 * @altsqlfn rasterHeight()
 */
Datum
Raster_height(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  int result = raster_height(rast);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Raster_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_srid);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the spatial reference system identifier of a raster
 * @param[in] rast Raster
 * @sqlfn SRID()
 */
Datum
Raster_srid(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  int32_t result = raster_srid(rast);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Raster_upper_left_x(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_upper_left_x);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the X coordinate of the upper left corner of a raster
 * @param[in] rast Raster
 * @sqlfn upperLeftX()
 */
Datum
Raster_upper_left_x(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  double result = raster_upper_left_x(rast);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Raster_upper_left_y(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_upper_left_y);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the Y coordinate of the upper left corner of a raster
 * @param[in] rast Raster
 * @sqlfn upperLeftY()
 */
Datum
Raster_upper_left_y(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  double result = raster_upper_left_y(rast);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Raster_scale_x(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_scale_x);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the pixel width of a raster, that is, the X component of its
 * scale
 * @param[in] rast Raster
 * @sqlfn scaleX()
 */
Datum
Raster_scale_x(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  double result = raster_scale_x(rast);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Raster_scale_y(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_scale_y);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the pixel height of a raster, that is, the Y component of its
 * scale
 * @param[in] rast Raster
 * @sqlfn scaleY()
 */
Datum
Raster_scale_y(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  double result = raster_scale_y(rast);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Raster_skew_x(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_skew_x);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the X component of the skew of a raster
 * @param[in] rast Raster
 * @sqlfn skewX()
 */
Datum
Raster_skew_x(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  double result = raster_skew_x(rast);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Raster_skew_y(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_skew_y);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the Y component of the skew of a raster
 * @param[in] rast Raster
 * @sqlfn skewY()
 */
Datum
Raster_skew_y(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  double result = raster_skew_y(rast);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Bands of a raster
 *****************************************************************************/

PGDLLEXPORT Datum Raster_band_pixel_type(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_band_pixel_type);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the name of the pixel data type of a raster band
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @sqlfn bandPixelType()
 * @altsqlfn rasterBandPixelType()
 */
Datum
Raster_band_pixel_type(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  int band = PG_GETARG_INT32(1);
  char *str = raster_band_pixel_type(rast, band);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Raster_band_has_nodata_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_band_has_nodata_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return whether a raster band states a nodata value
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @sqlfn bandHasNoDataValue()
 * @altsqlfn rasterBandHasNoDataValue()
 */
Datum
Raster_band_has_nodata_value(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  int band = PG_GETARG_INT32(1);
  bool result = raster_band_has_nodata_value(rast, band);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Raster_band_nodata_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_band_nodata_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the nodata value of a raster band, or NULL when the band
 * states none
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @sqlfn bandNoDataValue()
 * @altsqlfn rasterBandNoDataValue()
 */
Datum
Raster_band_nodata_value(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  int band = PG_GETARG_INT32(1);
  double result;
  if (! raster_band_nodata_value(rast, band, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Well-Known Binary representations of a raster
 *****************************************************************************/

PGDLLEXPORT Datum Raster_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_from_wkb);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster from its Well-Known Binary (WKB) representation
 * @sqlfn rasterFromBinary()
 */
Datum
Raster_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Raster *result = raster_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_from_hexwkb);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster from its ASCII hex-encoded Well-Known Binary
 * (HexWKB) representation
 * @sqlfn rasterFromHexWKB()
 */
Datum
Raster_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text_to_cstring(hexwkb_text);
  Raster *result = raster_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_as_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_as_wkb);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the Well-Known Binary (WKB) representation of a raster
 * @sqlfn asBinary()
 */
Datum
Raster_as_wkb(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  uint8_t variant = get_endian_variant(PG_GETARG_TEXT_P(1));
  size_t wkb_size;
  uint8_t *wkb = raster_as_wkb(rast, variant, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

PGDLLEXPORT Datum Raster_as_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_as_hexwkb);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a raster
 * @sqlfn asHexWKB()
 * @altsqlfn rasterAsHexWKB()
 */
Datum
Raster_as_hexwkb(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  uint8_t variant = get_endian_variant(PG_GETARG_TEXT_P(1));
  size_t hexwkb_size;
  char *hexwkb = raster_as_hexwkb(rast, variant, &hexwkb_size);
  text *result = cstring_to_text(hexwkb);
  pfree(hexwkb);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * A raster built from its pixels and its georeferencing
 *****************************************************************************/

PGDLLEXPORT Datum Raster_band_pixels(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_band_pixels);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the bytes of the pixels of a raster band
 * @details The twin of #Raquet_pixels for a band of a raster
 * @param[in] rast Raster
 * @param[in] nband Number of the band, starting at 1
 * @sqlfn pixels()
 * @altsqlfn rasterPixels()
 */
Datum
Raster_band_pixels(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  int band = PG_GETARG_INT32(1);
  size_t size;
  uint8_t *pixels = raster_band_pixels(rast, band, &size);
  if (! pixels)
    PG_RETURN_NULL();
  bytea *result = bstring2bytea(pixels, size);
  pfree(pixels);
  PG_RETURN_BYTEA_P(result);
}

PGDLLEXPORT Datum Raster_make(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_make);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster of one band from the bytes of its pixels and its
 * georeferencing
 * @details The twin of #Raquet_constructor, the georeferencing of a raster
 * standing for the cell of a tile
 * @param[in] pixels Bytes of the pixels, row by row
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] upperleftx X coordinate of the upper left corner
 * @param[in] upperlefty Y coordinate of the upper left corner
 * @param[in] scalex Width of a pixel
 * @param[in] scaley Height of a pixel
 * @param[in] skewx X component of the skew
 * @param[in] skewy Y component of the skew
 * @param[in] srid Spatial reference system identifier
 * @param[in] pixeltype Name of the pixel type
 * @param[in] nodataval Nodata value, absent where the band states none
 * @note Not STRICT: a nodata value left out states that the band carries none
 * @sqlfn raster()
 */
Datum
Raster_make(PG_FUNCTION_ARGS)
{
  for (int i = 0; i < 11; i++)
    if (PG_ARGISNULL(i))
      PG_RETURN_NULL();
  bytea *pixels = PG_GETARG_BYTEA_PP(0);
  char *pixeltype = text_to_cstring(PG_GETARG_TEXT_PP(10));
  bool has_nodata = ! PG_ARGISNULL(11);
  double nodataval = has_nodata ? PG_GETARG_FLOAT8(11) : 0.0;
  Raster *result = raster_make(PG_GETARG_INT32(1), PG_GETARG_INT32(2),
    PG_GETARG_FLOAT8(3), PG_GETARG_FLOAT8(4), PG_GETARG_FLOAT8(5),
    PG_GETARG_FLOAT8(6), PG_GETARG_FLOAT8(7), PG_GETARG_FLOAT8(8),
    PG_GETARG_INT32(9), pixeltype, nodataval, has_nodata,
    (const uint8_t *) VARDATA_ANY(pixels),
    (size_t) VARSIZE_ANY_EXHDR(pixels));
  pfree(pixeltype);
  PG_FREE_IF_COPY(pixels, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_make_empty(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_make_empty);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster with no band
 * @details The grid #Raster_make gives a band
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] upperleftx X coordinate of the upper left corner
 * @param[in] upperlefty Y coordinate of the upper left corner
 * @param[in] scalex Width of a pixel
 * @param[in] scaley Height of a pixel
 * @param[in] skewx X component of the skew
 * @param[in] skewy Y component of the skew
 * @param[in] srid Spatial reference system identifier
 * @sqlfn makeEmptyRaster()
 */
Datum
Raster_make_empty(PG_FUNCTION_ARGS)
{
  Raster *result = raster_make_empty(PG_GETARG_INT32(0), PG_GETARG_INT32(1),
    PG_GETARG_FLOAT8(2), PG_GETARG_FLOAT8(3), PG_GETARG_FLOAT8(4),
    PG_GETARG_FLOAT8(5), PG_GETARG_FLOAT8(6), PG_GETARG_FLOAT8(7),
    PG_GETARG_INT32(8));
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_add_band(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_add_band);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster with a band added, each of its pixels holding a value
 * @details The nodata value is read as #Raster_reclass reads it
 * @param[in] rast Raster
 * @param[in] index Number the band takes, starting at 1, absent to add it
 * after the last band
 * @param[in] pixeltype Name of the pixel type
 * @param[in] initialvalue Value of every pixel
 * @param[in] nodataval Nodata value, absent where the band states none
 * @note Not STRICT: a number left out adds the band after the last one, and a
 * nodata value left out states that the band carries none
 * @sqlfn addBand()
 */
Datum
Raster_add_band(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(2) || PG_ARGISNULL(3))
    PG_RETURN_NULL();
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  int index = PG_ARGISNULL(1) ? 0 : PG_GETARG_INT32(1);
  /* A band takes a number from 1, while 0 states the end to the kernel */
  if (! PG_ARGISNULL(1) && index < 1)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The number of a band starts at 1, not %d", index)));
  char *pixeltype = text_to_cstring(PG_GETARG_TEXT_PP(2));
  double initialvalue = PG_GETARG_FLOAT8(3);
  bool has_nodata = ! PG_ARGISNULL(4);
  double nodataval = has_nodata ? PG_GETARG_FLOAT8(4) : 0.0;
  Raster *result = raster_add_band(rast, index, pixeltype, initialvalue,
    nodataval, has_nodata);
  pfree(pixeltype);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_add_band_raster(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_add_band_raster);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster with a copy of the band of another raster added
 * @details The two rasters are read as #Raster_transform_raster reads them,
 * and the number the band takes as #Raster_add_band reads it
 * @param[in] torast Raster the band is added to
 * @param[in] fromrast Raster the band is copied from
 * @param[in] fromband Number of the band copied, starting at 1
 * @param[in] torastindex Number the band takes, starting at 1, absent to add
 * it after the last band
 * @note Not STRICT: a number left out adds the band after the last one
 * @sqlfn addBand()
 */
Datum
Raster_add_band_raster(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2))
    PG_RETURN_NULL();
  Raster *torast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  Raster *fromrast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  int fromband = PG_GETARG_INT32(2);
  int toindex = PG_ARGISNULL(3) ? 0 : PG_GETARG_INT32(3);
  /* A band takes a number from 1, while 0 states the end to the kernel */
  if (! PG_ARGISNULL(3) && toindex < 1)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The number of a band starts at 1, not %d", toindex)));
  Raster *result = raster_add_band_raster(torast, fromrast, fromband,
    toindex);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * @brief Return the elements of an array of one or two dimensions, its number
 * of rows and columns, and the elements that are NULL
 * @details An array of one dimension is a single row, as PostGIS reads it.
 * Unlike #datumarr_extract, which answers the elements alone, the shape and
 * the NULL elements are answered, which the block of pixels needs
 */
static Datum *
raster_block_extract(ArrayType *array, int *rows, int *cols, bool **nulls,
  int *count)
{
  int ndims = ARR_NDIM(array);
  if (ndims < 1 || ndims > 2)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("An array of values of a block of pixels has one or two "
        "dimensions, not %d", ndims)));
  int *dims = ARR_DIMS(array);
  *rows = (ndims == 2) ? dims[0] : 1;
  *cols = (ndims == 2) ? dims[1] : dims[0];
  Oid elemtype = ARR_ELEMTYPE(array);
  int16 typlen;
  bool typbyval;
  char typalign;
  get_typlenbyvalalign(elemtype, &typlen, &typbyval, &typalign);
  Datum *result;
  deconstruct_array(array, elemtype, typlen, typbyval, typalign, &result,
    nulls, count);
  return result;
}

PGDLLEXPORT Datum Raster_set_values(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_set_values);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster whose band takes the values of an array over a block
 * of pixels
 * @details The arrays are read as #Raster_clip_bands reads its arrays, with
 * the shape of the block
 * @param[in] rast Raster
 * @param[in] nband Number of the band, starting at 1
 * @param[in] x Column of the upper left pixel of the block, starting at 1
 * @param[in] y Row of the upper left pixel of the block, starting at 1
 * @param[in] newvalueset Values of the block, a NULL one setting the pixel to
 * nodata
 * @param[in] noset True for a value not to set, absent where every value is
 * set; a block of its own, read over the block of the values
 * @param[in] keepnodata True to leave the pixels holding the nodata value
 * @note Not STRICT: the flags left out set every value, and a NULL
 * @p keepnodata takes its default
 * @sqlfn setValues()
 */
Datum
Raster_set_values(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2) ||
      PG_ARGISNULL(3) || PG_ARGISNULL(4))
    PG_RETURN_NULL();
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  int band = PG_GETARG_INT32(1);
  int x = PG_GETARG_INT32(2);
  int y = PG_GETARG_INT32(3);
  int rows, cols, count;
  bool *nulls;
  Datum *elems = raster_block_extract(PG_GETARG_ARRAYTYPE_P(4), &rows, &cols,
    &nulls, &count);
  double *values = palloc(sizeof(double) * (size_t) (count > 0 ? count : 1));
  for (int i = 0; i < count; i++)
    values[i] = nulls[i] ? 0.0 : DatumGetFloat8(elems[i]);
  pfree(elems);
  /* The flags are a block of their own, which MEOS reads over the block of the
   * values; a NULL flag sets its value, and no flag sets every value */
  bool *noset = NULL;
  int nrows = 0, ncols = 0;
  if (! PG_ARGISNULL(5))
  {
    int ncount;
    bool *nnulls;
    Datum *nelems = raster_block_extract(PG_GETARG_ARRAYTYPE_P(5), &nrows,
      &ncols, &nnulls, &ncount);
    if (ncount > 0)
    {
      noset = palloc(sizeof(bool) * (size_t) ncount);
      for (int i = 0; i < ncount; i++)
        noset[i] = ! nnulls[i] && DatumGetBool(nelems[i]);
    }
    pfree(nelems); pfree(nnulls);
  }
  bool keepnodata = PG_ARGISNULL(6) ? false : PG_GETARG_BOOL(6);
  Raster *result = raster_set_values(rast, band, x, y, cols, rows, values,
    nulls, noset, ncols, nrows, count, keepnodata);
  pfree(values); pfree(nulls);
  if (noset)
    pfree(noset);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * raster_reclass
 *****************************************************************************/

PGDLLEXPORT Datum Raster_reclass(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_reclass);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster whose band states the classes an expression maps its
 * values onto
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @param[in] expr Reclassification expression
 * @param[in] pixeltype Name of the pixel type of the resulting band
 * @param[in] nodataval Nodata value of the resulting band, absent where the
 * band states none
 * @sqlfn reclass()
 */
Datum
Raster_reclass(PG_FUNCTION_ARGS)
{
  /* The function is not STRICT, so every argument is tested: only the nodata
   * value is optional, and the others reaching the kernel unread would be a
   * wild pointer rather than an error */
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2) ||
      PG_ARGISNULL(3))
    PG_RETURN_NULL();
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  int band = PG_GETARG_INT32(1);
  char *expr = text_to_cstring(PG_GETARG_TEXT_P(2));
  char *pixeltype = text_to_cstring(PG_GETARG_TEXT_P(3));
  /* A nodata value the caller leaves out is no value at all, which is what
   * the resulting band states rather than a zero standing in for it */
  bool has_nodata = ! PG_ARGISNULL(4);
  double nodataval = has_nodata ? PG_GETARG_FLOAT8(4) : 0.0;
  Raster *result = raster_reclass(rast, band, expr, pixeltype, has_nodata,
    nodataval);
  pfree(expr); pfree(pixeltype);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * raster_clip
 *****************************************************************************/

PGDLLEXPORT Datum Raster_clip(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_clip);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster keeping the pixels of another that a geometry covers
 * @param[in] rast Raster
 * @param[in] geom Geometry, in the reference system of the raster
 * @param[in] crop True to reduce the result to the extent the two share
 * @param[in] touched True to keep every pixel the geometry touches
 * @sqlfn clip()
 */
Datum
Raster_clip(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  bool crop = PG_GETARG_BOOL(2);
  bool touched = PG_GETARG_BOOL(3);
  Raster *result = raster_clip(rast, NULL, 0, gs, NULL, 0, crop, touched);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_clip_bands(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_clip_bands);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster keeping the pixels of some bands of another that a
 * geometry covers
 * @param[in] rast Raster
 * @param[in] nband Numbers of the bands to keep, NULL for every band
 * @param[in] geom Geometry, in the reference system of the raster
 * @param[in] nodataval Nodata values of the bands of the result, NULL for the
 * nodata value of each band
 * @param[in] crop True to reduce the result to the extent the two share
 * @param[in] touched True to keep every pixel the geometry touches
 * @note Not STRICT: a NULL array of bands keeps every band, and a NULL array
 * of nodata values keeps the nodata value of each band
 * @sqlfn clip()
 */
Datum
Raster_clip_bands(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(2))
    PG_RETURN_NULL();
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(2);
  int nbands = 0, nnodata = 0;
  int *bands = NULL;
  double *nodata = NULL;
  if (! PG_ARGISNULL(1))
  {
    Datum *elems = datumarr_extract(PG_GETARG_ARRAYTYPE_P(1), &nbands);
    bands = palloc(sizeof(int) * (size_t) (nbands > 0 ? nbands : 1));
    for (int i = 0; i < nbands; i++)
      bands[i] = DatumGetInt32(elems[i]);
    pfree(elems);
  }
  if (! PG_ARGISNULL(3))
  {
    Datum *elems = datumarr_extract(PG_GETARG_ARRAYTYPE_P(3), &nnodata);
    nodata = palloc(sizeof(double) * (size_t) (nnodata > 0 ? nnodata : 1));
    for (int i = 0; i < nnodata; i++)
      nodata[i] = DatumGetFloat8(elems[i]);
    pfree(elems);
  }
  /* A crop or a touch left unstated takes its default */
  bool crop = PG_ARGISNULL(4) ? true : PG_GETARG_BOOL(4);
  bool touched = PG_ARGISNULL(5) ? false : PG_GETARG_BOOL(5);
  Raster *result = raster_clip(rast, bands, nbands, gs, nodata, nnodata, crop,
    touched);
  if (bands)
    pfree(bands);
  if (nodata)
    pfree(nodata);
  PG_FREE_IF_COPY(gs, 2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * raster_transform, raster_rescale
 *****************************************************************************/

PGDLLEXPORT Datum Raster_transform(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_transform);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster stated in another spatial reference system
 * @param[in] rast Raster
 * @param[in] srid Target spatial reference system identifier
 * @param[in] algorithm Name of the resampling algorithm
 * @param[in] maxerr Error in input pixels the warp may commit
 * @param[in] scalex,scaley Pixel size of the result, 0 to let the warp derive
 * it
 * @sqlfn transform()
 * @altsqlfn rasterTransform()
 */
Datum
Raster_transform(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  int32_t srid = PG_GETARG_INT32(1);
  char *algorithm = text_to_cstring(PG_GETARG_TEXT_P(2));
  double maxerr = PG_GETARG_FLOAT8(3);
  double scalex = PG_GETARG_FLOAT8(4);
  double scaley = PG_GETARG_FLOAT8(5);
  Raster *result = raster_transform(rast, srid, algorithm, maxerr, scalex,
    scaley);
  pfree(algorithm);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_transform_pipeline(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_transform_pipeline);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster stated in another spatial reference system through a
 * coordinate operation
 * @param[in] rast Raster
 * @param[in] pipeline Coordinate operation
 * @param[in] srid Spatial reference system identifier of the result
 * @param[in] is_forward True to apply the operation, false its inverse
 * @param[in] algorithm Name of the resampling algorithm
 * @param[in] maxerr Error in input pixels the warp may commit
 * @param[in] scalex,scaley Pixel size of the result, 0 to let the warp derive
 * it
 * @sqlfn transformPipeline()
 * @altsqlfn rasterTransformPipeline()
 */
Datum
Raster_transform_pipeline(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  char *pipeline = text_to_cstring(PG_GETARG_TEXT_P(1));
  int32_t srid = PG_GETARG_INT32(2);
  bool is_forward = PG_GETARG_BOOL(3);
  char *algorithm = text_to_cstring(PG_GETARG_TEXT_P(4));
  double maxerr = PG_GETARG_FLOAT8(5);
  double scalex = PG_GETARG_FLOAT8(6);
  double scaley = PG_GETARG_FLOAT8(7);
  Raster *result = raster_transform_pipeline(rast, pipeline, srid, is_forward,
    algorithm, maxerr, scalex, scaley);
  pfree(pipeline); pfree(algorithm);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_transform_raster(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_transform_raster);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster stated on the grid of another raster
 * @param[in] rast Raster
 * @param[in] alignto Raster whose grid the result lies on
 * @param[in] algorithm Name of the resampling algorithm
 * @param[in] maxerr Error in input pixels the warp may commit
 * @sqlfn transform()
 * @altsqlfn rasterTransform()
 */
Datum
Raster_transform_raster(PG_FUNCTION_ARGS)
{
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  Raster *alignto = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  char *algorithm = text_to_cstring(PG_GETARG_TEXT_P(2));
  double maxerr = PG_GETARG_FLOAT8(3);
  Raster *result = raster_transform_raster(rast, alignto, algorithm, maxerr);
  pfree(algorithm);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_rescale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_rescale);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a raster resampled to another pixel size
 * @param[in] rast Raster
 * @param[in] scalex,scaley Pixel size in the units of the raster's reference
 * system
 * @param[in] algorithm Name of the resampling algorithm
 * @param[in] maxerr Error in input pixels the warp may commit
 * @sqlfn rescale()
 */
Datum
Raster_rescale(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  double scalex = PG_GETARG_FLOAT8(1);
  double scaley = PG_GETARG_FLOAT8(2);
  char *algorithm = text_to_cstring(PG_GETARG_TEXT_P(3));
  double maxerr = PG_GETARG_FLOAT8(4);
  Raster *result = raster_rescale(rast, scalex, scaley, algorithm, maxerr);
  pfree(algorithm);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * raster_summary_stats
 *****************************************************************************/

/**
 * @brief Return the summarystats record of the statistics of a band
 * @details The record #Raster_summary_stats answers, built as the PostGIS
 * RASTER_summaryStats builds it, and the one the aggregate answers. An error
 * in MEOS never returns here, so missing statistics are a band with no pixel
 * to count: the record PostGIS answers for it counts none and states no
 * statistic, rather than being no record at all. The statistics are freed
 */
static Datum
bandstats_record(FunctionCallInfo fcinfo, BandStats *stats)
{
  TupleDesc tupdesc;
  if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
    ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
      errmsg("function returning record called in context "
             "that cannot accept type record")));
  tupdesc = BlessTupleDesc(tupdesc);
  Datum values[6];
  bool isnull[6] = {false, false, false, false, false, false};
  if (! stats)
  {
    values[0] = Int64GetDatum(0);
    for (int i = 1; i < 6; i++)
      isnull[i] = true;
  }
  else
  {
    values[0] = Int64GetDatum(stats->count);
    values[1] = Float8GetDatum(stats->sum);
    values[2] = Float8GetDatum(stats->mean);
    values[3] = Float8GetDatum(stats->stddev);
    values[4] = Float8GetDatum(stats->min);
    values[5] = Float8GetDatum(stats->max);
    pfree(stats);
  }
  HeapTuple tuple = heap_form_tuple(tupdesc, values, isnull);
  return HeapTupleGetDatum(tuple);
}

PGDLLEXPORT Datum Raster_summary_stats(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_summary_stats);
/**
 * @ingroup mobilitydb_raster
 * @brief Return what the pixels of a raster band amount to
 * @details The record is built as the PostGIS RASTER_summaryStats builds it,
 * so the SQL function answers the summarystats record ST_SummaryStats answers
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @param[in] exclude_nodata True to leave the pixels the band states as nodata
 * out of the statistics
 * @sqlfn summaryStats()
 */
Datum
Raster_summary_stats(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  int band = PG_GETARG_INT32(1);
  bool exclude_nodata = PG_GETARG_BOOL(2);
  BandStats *stats = raster_summary_stats(rast, band, exclude_nodata);
  PG_RETURN_DATUM(bandstats_record(fcinfo, stats));
}

/**
 * @brief Read the statistics a summarystats record holds
 * @details The inverse of #bandstats_record, reading each field at its place
 * in the record, count, sum, mean, stddev, min and max, as #Create_trip reads
 * the fields of its records. A record counting no pixel states no statistic,
 * so its other fields read as 0 and the accessors answer NULL for them
 */
static void
record_bandstats(HeapTupleHeader td, BandStats *stats)
{
  double *fields[] = {&stats->sum, &stats->mean, &stats->stddev, &stats->min,
    &stats->max};
  bool isnull;
  Datum d = GetAttributeByNum(td, 1, &isnull);
  stats->count = isnull ? 0 : (uint32_t) DatumGetInt64(d);
  for (int i = 0; i < 5; i++)
  {
    d = GetAttributeByNum(td, (AttrNumber) (i + 2), &isnull);
    *fields[i] = (isnull || stats->count == 0) ? 0.0 : DatumGetFloat8(d);
  }
}

PGDLLEXPORT Datum Summarystats_count(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Summarystats_count);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the number of pixels of raster band statistics
 * @details The accessors of a summarystats record read its fields by a
 * function, the form every engine reads them in, beside the reading of a field
 * by its name PostgreSQL keeps, as for PostGIS's ST_SummaryStats
 * @sqlfn summarystatsCount()
 */
Datum
Summarystats_count(PG_FUNCTION_ARGS)
{
  BandStats stats;
  record_bandstats(PG_GETARG_HEAPTUPLEHEADER(0), &stats);
  PG_RETURN_INT64(bandstats_count(&stats));
}

PGDLLEXPORT Datum Summarystats_sum(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Summarystats_sum);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the sum of raster band statistics
 * @details NULL where the statistics count no pixel
 * @sqlfn summarystatsSum()
 */
Datum
Summarystats_sum(PG_FUNCTION_ARGS)
{
  BandStats stats;
  record_bandstats(PG_GETARG_HEAPTUPLEHEADER(0), &stats);
  double result;
  if (! bandstats_sum(&stats, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Summarystats_mean(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Summarystats_mean);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the mean of raster band statistics
 * @details NULL where the statistics count no pixel
 * @sqlfn summarystatsMean()
 */
Datum
Summarystats_mean(PG_FUNCTION_ARGS)
{
  BandStats stats;
  record_bandstats(PG_GETARG_HEAPTUPLEHEADER(0), &stats);
  double result;
  if (! bandstats_mean(&stats, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Summarystats_stddev(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Summarystats_stddev);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the standard deviation of raster band statistics
 * @details NULL where the statistics count no pixel
 * @sqlfn summarystatsStddev()
 */
Datum
Summarystats_stddev(PG_FUNCTION_ARGS)
{
  BandStats stats;
  record_bandstats(PG_GETARG_HEAPTUPLEHEADER(0), &stats);
  double result;
  if (! bandstats_stddev(&stats, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Summarystats_min(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Summarystats_min);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the minimum of raster band statistics
 * @details NULL where the statistics count no pixel
 * @sqlfn summarystatsMin()
 */
Datum
Summarystats_min(PG_FUNCTION_ARGS)
{
  BandStats stats;
  record_bandstats(PG_GETARG_HEAPTUPLEHEADER(0), &stats);
  double result;
  if (! bandstats_min(&stats, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Summarystats_max(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Summarystats_max);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the maximum of raster band statistics
 * @details NULL where the statistics count no pixel
 * @sqlfn summarystatsMax()
 */
Datum
Summarystats_max(PG_FUNCTION_ARGS)
{
  BandStats stats;
  record_bandstats(PG_GETARG_HEAPTUPLEHEADER(0), &stats);
  double result;
  if (! bandstats_max(&stats, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * raster_dump_as_polygons
 *****************************************************************************/

PGDLLEXPORT Datum Raster_dump_as_polygons(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_dump_as_polygons);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the polygons of a raster band, one for each group of pixels
 * carrying the same value
 * @details Set-returning function emitting one PostGIS geomval record per
 * polygon, as ST_DumpAsPolygons emits them
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @param[in] exclude_nodata True to leave the pixels the band states as nodata
 * out
 * @sqlfn dumpAsPolygons()
 */
Datum
Raster_dump_as_polygons(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;

  if (SRF_IS_FIRSTCALL())
  {
    funcctx = SRF_FIRSTCALL_INIT();
    /* The polygons are kept across the calls, so they are computed in the
     * context that outlives them */
    MemoryContext oldctx = MemoryContextSwitchTo(
      funcctx->multi_call_memory_ctx);
    Datum rast_datum = PG_GETARG_DATUM(0);
    Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
    int band = PG_GETARG_INT32(1);
    bool exclude_nodata = PG_GETARG_BOOL(2);
    int count = 0;
    funcctx->user_fctx = (void *) raster_dump_as_polygons(rast, band,
      exclude_nodata, &count);
    funcctx->max_calls = count;
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldctx);
  }

  funcctx = SRF_PERCALL_SETUP();
  GeomVal *gvarr = (GeomVal *) funcctx->user_fctx;
  if (funcctx->call_cntr >= funcctx->max_calls)
  {
    geomval_arr_free(gvarr, funcctx->max_calls);
    SRF_RETURN_DONE(funcctx);
  }

  const GeomVal *gv = &gvarr[funcctx->call_cntr];
  Datum values[2];
  bool isnull[2] = {false, false};
  values[0] = PointerGetDatum(gv->geom);
  values[1] = Float8GetDatum(gv->val);
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  SRF_RETURN_NEXT(funcctx, HeapTupleGetDatum(tuple));
}

PGDLLEXPORT Datum Raster_pixel_as_points(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_pixel_as_points);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the pixels of a raster band as points
 * @details Set-returning function emitting one record per pixel, as
 * ST_PixelAsPoints emits them
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @param[in] exclude_nodata True to leave the pixels the band states as nodata
 * out
 * @sqlfn pixelAsPoints()
 */
Datum
Raster_pixel_as_points(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;

  if (SRF_IS_FIRSTCALL())
  {
    funcctx = SRF_FIRSTCALL_INIT();
    /* The pixels are kept across the calls, so they are computed in the
     * context that outlives them */
    MemoryContext oldctx = MemoryContextSwitchTo(
      funcctx->multi_call_memory_ctx);
    Datum rast_datum = PG_GETARG_DATUM(0);
    Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
    int band = PG_GETARG_INT32(1);
    bool exclude_nodata = PG_GETARG_BOOL(2);
    int count = 0;
    funcctx->user_fctx = (void *) raster_pixel_as_points(rast, band,
      exclude_nodata, &count);
    funcctx->max_calls = count;
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldctx);
  }

  funcctx = SRF_PERCALL_SETUP();
  Pixel *pixarr = (Pixel *) funcctx->user_fctx;
  if (funcctx->call_cntr >= funcctx->max_calls)
  {
    pixel_arr_free(pixarr, funcctx->max_calls);
    SRF_RETURN_DONE(funcctx);
  }

  const Pixel *pix = &pixarr[funcctx->call_cntr];
  Datum values[4];
  bool isnull[4] = {false, false, false, false};
  values[0] = PointerGetDatum(pix->geom);
  values[1] = Float8GetDatum(pix->value);
  values[2] = Int32GetDatum(pix->x);
  values[3] = Int32GetDatum(pix->y);
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  SRF_RETURN_NEXT(funcctx, HeapTupleGetDatum(tuple));
}

/**
 * @brief Return the values of a band of a raster as a two-dimensional array,
 * a row per row of the raster, NULL where the raster has no pixel
 */
static ArrayType *
raster_values_array(const Raster *rast, int band, bool exclude_nodata)
{
  bool *nodata = NULL;
  int count = 0;
  double *values = raster_dump_values(rast, band, exclude_nodata, &nodata,
    &count);
  if (! values)
    return NULL;
  Datum *elems = palloc(sizeof(Datum) * (size_t) count);
  for (int i = 0; i < count; i++)
    elems[i] = Float8GetDatum(values[i]);
  int dims[2] = {raster_height(rast), raster_width(rast)};
  int lbs[2] = {1, 1};
  ArrayType *result = construct_md_array(elems, nodata, 2, dims, lbs,
    FLOAT8OID, sizeof(float8), FLOAT8PASSBYVAL, TYPALIGN_DOUBLE);
  pfree(elems); pfree(values); pfree(nodata);
  return result;
}

/**
 * @brief State of the set-returning function answering the values of the
 * bands of a raster
 */
typedef struct
{
  Raster *rast;         /**< Raster read */
  int *bands;           /**< Numbers of the bands, starting at 1 */
  bool exclude_nodata;  /**< True to answer NULL for the nodata pixels */
} RasterValuesState;

PGDLLEXPORT Datum Raster_dump_values(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_dump_values);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the values of the bands of a raster, one array per band
 * @details Set-returning function emitting one record per band, in the order
 * the bands are given, as ST_DumpValues emits them; a NULL array of bands
 * reads every band
 * @param[in] rast Raster
 * @param[in] nband Numbers of the bands, starting at 1
 * @param[in] exclude_nodata True to answer NULL for the pixels the band states
 * as nodata
 * @note Not STRICT: a NULL array of bands reads every band, and a NULL
 * exclusion takes its default
 * @sqlfn dumpValues()
 */
Datum
Raster_dump_values(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;

  if (SRF_IS_FIRSTCALL())
  {
    funcctx = SRF_FIRSTCALL_INIT();
    /* The raster and its bands are kept across the calls, so they are read
     * in the context that outlives them */
    MemoryContext oldctx = MemoryContextSwitchTo(
      funcctx->multi_call_memory_ctx);
    RasterValuesState *state = palloc0(sizeof(RasterValuesState));
    int count = 0;
    if (! PG_ARGISNULL(0))
    {
      state->rast = (Raster *) PG_DETOAST_DATUM_COPY(PG_GETARG_DATUM(0));
      state->exclude_nodata = PG_ARGISNULL(2) ? true : PG_GETARG_BOOL(2);
      if (PG_ARGISNULL(1))
      {
        count = raster_num_bands(state->rast);
        state->bands = palloc(sizeof(int) * (size_t) (count > 0 ? count : 1));
        for (int i = 0; i < count; i++)
          state->bands[i] = i + 1;
      }
      else
      {
        Datum *elems = datumarr_extract(PG_GETARG_ARRAYTYPE_P(1), &count);
        state->bands = palloc(sizeof(int) * (size_t) (count > 0 ? count : 1));
        for (int i = 0; i < count; i++)
          state->bands[i] = DatumGetInt32(elems[i]);
        pfree(elems);
      }
    }
    funcctx->user_fctx = (void *) state;
    funcctx->max_calls = count;
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldctx);
  }

  funcctx = SRF_PERCALL_SETUP();
  RasterValuesState *state = (RasterValuesState *) funcctx->user_fctx;
  if (funcctx->call_cntr >= funcctx->max_calls)
    SRF_RETURN_DONE(funcctx);

  int band = state->bands[funcctx->call_cntr];
  ArrayType *valarray = raster_values_array(state->rast, band,
    state->exclude_nodata);
  Datum values[2];
  bool isnull[2] = {false, valarray == NULL};
  values[0] = Int32GetDatum(band);
  values[1] = PointerGetDatum(valarray);
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  SRF_RETURN_NEXT(funcctx, HeapTupleGetDatum(tuple));
}

PGDLLEXPORT Datum Raster_dump_values_band(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_dump_values_band);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the values of a band of a raster as an array
 * @param[in] rast Raster
 * @param[in] nband Number of the band, starting at 1
 * @param[in] exclude_nodata True to answer NULL for the pixels the band states
 * as nodata
 * @sqlfn dumpValues()
 */
Datum
Raster_dump_values_band(PG_FUNCTION_ARGS)
{
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  int band = PG_GETARG_INT32(1);
  bool exclude_nodata = PG_GETARG_BOOL(2);
  ArrayType *result = raster_values_array(rast, band, exclude_nodata);
  PG_FREE_IF_COPY(rast, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * raster_tile_value_quadbin
 *****************************************************************************/

/** Map a pixtype name text argument to a MeosPixType code. */
static MeosPixType
text_to_pixtype(const text *pt)
{
  char *s = text_to_cstring(pt);
  MeosPixType result = raquet_pixtype_from_string(s);
  pfree(s);
  return result;
}

PGDLLEXPORT Datum Raster_tile_value_quadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_tile_value_quadbin);
/**
 * @ingroup mobilitydb_raster
 * @brief Sample a Raquet raster chip along a tgeompoint trajectory
 * @param[in] traj Trajectory (tgeompoint, SRID 4326)
 * @param[in] pixels Row-major pixel bytes (bytea)
 * @param[in] width Tile width in pixels
 * @param[in] height Tile height in pixels
 * @param[in] quadbin CARTO QUADBIN cell (bigint)
 * @param[in] pixtype Pixel type name: uint8, int8, uint16, int16, uint32, int32, uint64, int64, float16, float32, or float64
 * @param[in] nodata Nodata sentinel value
 * @param[in] has_nodata  Enable nodata filtering
 * @sqlfn rasterTileValueQuadbin()
 */
Datum
Raster_tile_value_quadbin(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  bytea *pxbytea = PG_GETARG_BYTEA_PP(1);
  int32 width = PG_GETARG_INT32(2);
  int32 height = PG_GETARG_INT32(3);
  Quadbin quadbin = PG_GETARG_QUADBIN(4);
  text *pixtype_t = PG_GETARG_TEXT_PP(5);
  float8 nodata = PG_GETARG_FLOAT8(6);
  bool has_nd = PG_GETARG_BOOL(7);

  const uint8_t *pixels = (const uint8_t *) VARDATA_ANY(pxbytea);
  size_t pixels_size = (size_t) VARSIZE_ANY_EXHDR(pxbytea);
  MeosPixType pixtype = text_to_pixtype(pixtype_t);

  Temporal *result = raster_tile_value_quadbin(traj, pixels, pixels_size,
    width, height, quadbin, pixtype, nodata, has_nd);

  PG_FREE_IF_COPY(traj, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Raquet type: input/output, constructor, and typed sampling
 *****************************************************************************/

PGDLLEXPORT Datum Raquet_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_in);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a Raquet tile from its HexWKB representation
 * @sqlfn raquet_in()
 */
Datum
Raquet_in(PG_FUNCTION_ARGS)
{
  const char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_RAQUET_P(raquet_in(str));
}

PGDLLEXPORT Datum Raquet_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_out);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the HexWKB representation of a Raquet tile
 * @sqlfn raquet_out()
 */
Datum
Raquet_out(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  char *result = raquet_out(rq);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_CSTRING(result);
}

PGDLLEXPORT Datum Raquet_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_recv);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a Raquet tile from its Well-Known Binary (WKB) representation
 * @sqlfn raquet_recv()
 */
Datum
Raquet_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  Raquet *result = raquet_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_RAQUET_P(result);
}

PGDLLEXPORT Datum Raquet_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_send);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the Well-Known Binary (WKB) representation of a Raquet tile
 * @sqlfn raquet_send()
 */
Datum
Raquet_send(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  size_t wkb_size;
  uint8_t *wkb = raquet_as_wkb(rq, (uint8_t) WKB_NDR, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_BYTEA_P(result);
}

PGDLLEXPORT Datum Raquet_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_from_wkb);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a Raquet tile from its Well-Known Binary (WKB) representation
 * @sqlfn raquetFromBinary()
 */
Datum
Raquet_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Raquet *result = raquet_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_RAQUET_P(result);
}

PGDLLEXPORT Datum Raquet_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_from_hexwkb);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a Raquet tile from its ASCII hex-encoded Well-Known Binary
 * (HexWKB) representation
 * @sqlfn raquetFromHexWKB()
 */
Datum
Raquet_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text_to_cstring(hexwkb_text);
  Raquet *result = raquet_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_RAQUET_P(result);
}

PGDLLEXPORT Datum Raquet_as_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_as_wkb);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the Well-Known Binary (WKB) representation of a Raquet tile
 * @sqlfn asBinary()
 */
Datum
Raquet_as_wkb(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  bytea *result = Datum_as_wkb(fcinfo, RaquetPGetDatum(rq), T_RAQUET, false);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_BYTEA_P(result);
}

PGDLLEXPORT Datum Raquet_as_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_as_hexwkb);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a Raquet tile
 * @sqlfn asHexWKB()
 * @altsqlfn raquetAsHexWKB()
 */
Datum
Raquet_as_hexwkb(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  text *result = Datum_as_hexwkb(fcinfo, RaquetPGetDatum(rq), T_RAQUET, false);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Raquet_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_constructor);
/**
 * @ingroup mobilitydb_raster
 * @brief Construct a Raquet tile from a QUADBIN cell, dimensions, a pixel type
 * name and a row-major packed pixel array
 * @param[in] pixels   Row-major pixel bytes (bytea)
 * @param[in] width    Tile width in pixels
 * @param[in] height   Tile height in pixels
 * @param[in] quadbin  CARTO QUADBIN cell (bigint)
 * @param[in] pixtype  Pixel type name: uint8, int8, uint16, int16, uint32, int32, uint64, int64, float16, float32, or float64
 * @param[in] nodata   Nodata sentinel value (NULL disables nodata filtering)
 * @sqlfn raquet()
 */
Datum
Raquet_constructor(PG_FUNCTION_ARGS)
{
  /* Non-strict: the nodata argument (5) may be NULL to disable nodata */
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2) ||
      PG_ARGISNULL(3) || PG_ARGISNULL(4))
    PG_RETURN_NULL();
  bytea *pxbytea = PG_GETARG_BYTEA_PP(0);
  int32 width = PG_GETARG_INT32(1);
  int32 height = PG_GETARG_INT32(2);
  Quadbin quadbin = PG_GETARG_QUADBIN(3);
  text *pixtype_t = PG_GETARG_TEXT_PP(4);
  bool has_nd = ! PG_ARGISNULL(5);
  float8 nodata = has_nd ? PG_GETARG_FLOAT8(5) : 0.0;
  MeosPixType pixtype = text_to_pixtype(pixtype_t);

  const uint8_t *pixels = (const uint8_t *) VARDATA_ANY(pxbytea);
  Raquet *result = raquet_make(quadbin, width, height, pixtype,
    nodata, has_nd, pixels, (size_t) VARSIZE_ANY_EXHDR(pxbytea));
  PG_RETURN_RAQUET_P(result);
}

PGDLLEXPORT Datum Raquet_read_bytes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_read_bytes);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a Raquet tile decoded from an in-memory raster file via GDAL
 * @details The raster file is supplied as bytes in any GDAL-supported format
 * and decoded through GDAL's `/vsimem/` virtual filesystem, so no server-side
 * file access is required.
 * @param[in] rasterfile Raster file bytes (bytea)
 * @param[in] quadbin CARTO QUADBIN cell (bigint), or NULL to derive it from the
 * raster geotransform and EPSG:3857 spatial reference
 * @sqlfn raquetRead()
 */
Datum
Raquet_read_bytes(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();
  bytea *rasterfile = PG_GETARG_BYTEA_PP(0);
  /* A NULL quadbin requests deriving the tile identifier from the raster
   * geotransform; raquet_read_bytes treats 0 as that request */
  Quadbin quadbin = PG_ARGISNULL(1) ? 0 : PG_GETARG_QUADBIN(1);
  const uint8_t *data = (const uint8_t *) VARDATA_ANY(rasterfile);
  size_t size = VARSIZE_ANY_EXHDR(rasterfile);
  Raquet *result = raquet_read_bytes(data, size, quadbin);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_RAQUET_P(result);
}

PGDLLEXPORT Datum Raquet_read(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_read);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a Raquet tile read from a raster file on the server via GDAL
 * @details The file is read where PostGIS allows a raster band stored outside
 * the database to be read.
 * @param[in] path Path of a raster file on the server (text)
 * @param[in] quadbin CARTO QUADBIN cell (bigint), or NULL to derive it from the
 * raster geotransform and EPSG:3857 spatial reference
 * @sqlfn raquetRead()
 */
Datum
Raquet_read(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();
  char *path = text_to_cstring(PG_GETARG_TEXT_PP(0));
  /* A NULL quadbin requests deriving the tile identifier from the raster
   * geotransform; raquet_read treats 0 as that request */
  Quadbin quadbin = PG_ARGISNULL(1) ? 0 : PG_GETARG_QUADBIN(1);
  if (! ensure_raster_file_readable(path))
    PG_RETURN_NULL();
  Raquet *result = raquet_read(path, quadbin);
  pfree(path);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_RAQUET_P(result);
}

PGDLLEXPORT Datum Raster_tile_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_tile_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Sample a Raquet tile along a tgeompoint trajectory
 * @param[in] traj Trajectory (tgeompoint)
 * @param[in] rq Raquet tile
 * @sqlfn rasterTileValue()
 */
Datum
Raster_tile_value(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  Raquet *rq = PG_GETARG_RAQUET_P(1);
  Temporal *result = raster_tile_value(traj, rq);
  PG_FREE_IF_COPY(traj, 0);
  PG_FREE_IF_COPY(rq, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_tile_value_array(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_tile_value_array);
/**
 * @ingroup mobilitydb_raster
 * @brief Sample an array of Raquet tiles along a tgeompoint trajectory
 * @param[in] traj Trajectory (tgeompoint)
 * @param[in] rqarr Array of Raquet tiles
 * @sqlfn rasterTileValue()
 */
Datum
Raster_tile_value_array(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(1);
  int count;
  Raquet **rqarr = raquetarr_extract(array, &count);
  Temporal *result = raster_tile_value_array(traj, (const Raquet **) rqarr,
    count);
  pfree(rqarr);
  PG_FREE_IF_COPY(traj, 0);
  PG_FREE_IF_COPY(array, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Raquet type: accessors
 *****************************************************************************/

PGDLLEXPORT Datum Raquet_quadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_quadbin);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the QUADBIN cell of a Raquet tile
 * @sqlfn quadbin()
 */
Datum
Raquet_quadbin(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  Quadbin result = raquet_quadbin(rq);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_QUADBIN(result);
}

PGDLLEXPORT Datum Raquet_width(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_width);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the width in pixels of a Raquet tile
 * @sqlfn width()
 * @altsqlfn raquetWidth()
 */
Datum
Raquet_width(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  int result = raquet_width(rq);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Raquet_height(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_height);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the height in pixels of a Raquet tile
 * @sqlfn height()
 * @altsqlfn raquetHeight()
 */
Datum
Raquet_height(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  int result = raquet_height(rq);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Raquet_band_pixel_type(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_band_pixel_type);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the name of the pixel data type of the band of a Raquet tile
 * @sqlfn bandPixelType()
 * @altsqlfn raquetBandPixelType()
 */
Datum
Raquet_band_pixel_type(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  char *str = raquet_band_pixel_type(rq);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Raquet_band_has_nodata_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_band_has_nodata_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return whether the band of a Raquet tile states a nodata value
 * @sqlfn bandHasNoDataValue()
 * @altsqlfn raquetBandHasNoDataValue()
 */
Datum
Raquet_band_has_nodata_value(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  bool result = raquet_band_has_nodata_value(rq);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Raquet_band_nodata_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_band_nodata_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the nodata value of the band of a Raquet tile, or NULL when
 * the band states none
 * @sqlfn bandNoDataValue()
 * @altsqlfn raquetBandNoDataValue()
 */
Datum
Raquet_band_nodata_value(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  double result;
  bool found = raquet_band_nodata_value(rq, &result);
  PG_FREE_IF_COPY(rq, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Raquet_pixels(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_pixels);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the pixel bytes of a Raquet tile
 * @sqlfn pixels()
 * @altsqlfn raquetPixels()
 */
Datum
Raquet_pixels(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  size_t size;
  uint8_t *pixels = raquet_pixels(rq, &size);
  bytea *result = palloc(VARHDRSZ + size);
  SET_VARSIZE(result, VARHDRSZ + size);
  memcpy(VARDATA(result), pixels, size);
  pfree(pixels);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Raquet type: comparison
 *****************************************************************************/

PGDLLEXPORT Datum Raquet_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_eq);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if the Raquet tiles are equal
 * @sqlfn eq()
 * @sqlop @p =
 */
Datum
Raquet_eq(PG_FUNCTION_ARGS)
{
  Raquet *rq1 = PG_GETARG_RAQUET_P(0);
  Raquet *rq2 = PG_GETARG_RAQUET_P(1);
  bool result = raquet_eq(rq1, rq2);
  PG_FREE_IF_COPY(rq1, 0); PG_FREE_IF_COPY(rq2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Raquet_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_ne);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if the Raquet tiles are different
 * @sqlfn ne()
 * @sqlop @p <>
 */
Datum
Raquet_ne(PG_FUNCTION_ARGS)
{
  Raquet *rq1 = PG_GETARG_RAQUET_P(0);
  Raquet *rq2 = PG_GETARG_RAQUET_P(1);
  bool result = raquet_ne(rq1, rq2);
  PG_FREE_IF_COPY(rq1, 0); PG_FREE_IF_COPY(rq2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Raquet_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_lt);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if the first Raquet tile is less than the second one
 * @sqlfn lt()
 * @sqlop @p <
 */
Datum
Raquet_lt(PG_FUNCTION_ARGS)
{
  Raquet *rq1 = PG_GETARG_RAQUET_P(0);
  Raquet *rq2 = PG_GETARG_RAQUET_P(1);
  bool result = raquet_lt(rq1, rq2);
  PG_FREE_IF_COPY(rq1, 0); PG_FREE_IF_COPY(rq2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Raquet_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_le);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if the first Raquet tile is less than or equal to the
 * second one
 * @sqlfn le()
 * @sqlop @p <=
 */
Datum
Raquet_le(PG_FUNCTION_ARGS)
{
  Raquet *rq1 = PG_GETARG_RAQUET_P(0);
  Raquet *rq2 = PG_GETARG_RAQUET_P(1);
  bool result = raquet_le(rq1, rq2);
  PG_FREE_IF_COPY(rq1, 0); PG_FREE_IF_COPY(rq2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Raquet_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_ge);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if the first Raquet tile is greater than or equal to the
 * second one
 * @sqlfn ge()
 * @sqlop @p >=
 */
Datum
Raquet_ge(PG_FUNCTION_ARGS)
{
  Raquet *rq1 = PG_GETARG_RAQUET_P(0);
  Raquet *rq2 = PG_GETARG_RAQUET_P(1);
  bool result = raquet_ge(rq1, rq2);
  PG_FREE_IF_COPY(rq1, 0); PG_FREE_IF_COPY(rq2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Raquet_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_gt);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if the first Raquet tile is greater than the second one
 * @sqlfn gt()
 * @sqlop @p >
 */
Datum
Raquet_gt(PG_FUNCTION_ARGS)
{
  Raquet *rq1 = PG_GETARG_RAQUET_P(0);
  Raquet *rq2 = PG_GETARG_RAQUET_P(1);
  bool result = raquet_gt(rq1, rq2);
  PG_FREE_IF_COPY(rq1, 0); PG_FREE_IF_COPY(rq2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Raquet_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_cmp);
/**
 * @ingroup mobilitydb_raster
 * @brief Return -1, 0, or 1 depending on whether the first Raquet tile is
 * less than, equal to, or greater than the second one
 * @sqlfn cmp()
 */
Datum
Raquet_cmp(PG_FUNCTION_ARGS)
{
  Raquet *rq1 = PG_GETARG_RAQUET_P(0);
  Raquet *rq2 = PG_GETARG_RAQUET_P(1);
  int result = raquet_cmp(rq1, rq2);
  PG_FREE_IF_COPY(rq1, 0); PG_FREE_IF_COPY(rq2, 1);
  PG_RETURN_INT32(result);
}

/*****************************************************************************
 * Raquet type: hash
 *****************************************************************************/

PGDLLEXPORT Datum Raquet_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_hash);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the 32-bit hash of a Raquet tile
 * @sqlfn hash()
 * @altsqlfn raquetHash()
 */
Datum
Raquet_hash(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  uint32 result = raquet_hash(rq);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_UINT32(result);
}

PGDLLEXPORT Datum Raquet_hash_extended(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_hash_extended);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the 64-bit hash of a Raquet tile using a seed
 * @sqlfn hashExtended()
 * @altsqlfn raquetHashExtended()
 */
Datum
Raquet_hash_extended(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = raquet_hash_extended(rq, seed);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_UINT64(result);
}

/*****************************************************************************
 * Raquet type: conversions
 *****************************************************************************/

PGDLLEXPORT Datum Raquet_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_to_stbox);
/**
 * @ingroup mobilitydb_raster
 * @brief Convert a Raquet tile into a spatiotemporal box
 * @sqlfn stbox()
 * @altsqlfn raquetStbox()
 */
Datum
Raquet_to_stbox(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  STBox *result = raquet_to_stbox(rq);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************
 * Raster type: mathematical functions
 *****************************************************************************/

PGDLLEXPORT Datum Add_float_raster(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Add_float_raster);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the addition of a float and a raster, pixel by pixel
 * @sqlfn rasterAdd()
 * @sqlop @p +
 */
Datum
Add_float_raster(PG_FUNCTION_ARGS)
{
  double d = PG_GETARG_FLOAT8(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  Raster *result = add_float_raster(d, rast);
  PG_FREE_IF_COPY(rast, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Add_raster_float(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Add_raster_float);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the addition of a raster and a float, pixel by pixel
 * @sqlfn rasterAdd()
 * @sqlop @p +
 */
Datum
Add_raster_float(PG_FUNCTION_ARGS)
{
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  double d = PG_GETARG_FLOAT8(1);
  Raster *result = add_raster_float(rast, d);
  PG_FREE_IF_COPY(rast, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Add_raster_raster(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Add_raster_raster);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the addition of two rasters, pixel by pixel
 * @details The rasters are operated over the extent they share
 * @sqlfn rasterAdd()
 * @sqlop @p +
 */
Datum
Add_raster_raster(PG_FUNCTION_ARGS)
{
  Raster *rast1 = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  Raster *rast2 = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  Raster *result = add_raster_raster(rast1, rast2);
  PG_FREE_IF_COPY(rast1, 0);
  PG_FREE_IF_COPY(rast2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Sub_float_raster(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Sub_float_raster);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the subtraction of a float and a raster, pixel by pixel
 * @sqlfn rasterSub()
 * @sqlop @p -
 */
Datum
Sub_float_raster(PG_FUNCTION_ARGS)
{
  double d = PG_GETARG_FLOAT8(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  Raster *result = sub_float_raster(d, rast);
  PG_FREE_IF_COPY(rast, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Sub_raster_float(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Sub_raster_float);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the subtraction of a raster and a float, pixel by pixel
 * @sqlfn rasterSub()
 * @sqlop @p -
 */
Datum
Sub_raster_float(PG_FUNCTION_ARGS)
{
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  double d = PG_GETARG_FLOAT8(1);
  Raster *result = sub_raster_float(rast, d);
  PG_FREE_IF_COPY(rast, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Sub_raster_raster(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Sub_raster_raster);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the subtraction of two rasters, pixel by pixel
 * @details The rasters are operated over the extent they share
 * @sqlfn rasterSub()
 * @sqlop @p -
 */
Datum
Sub_raster_raster(PG_FUNCTION_ARGS)
{
  Raster *rast1 = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  Raster *rast2 = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  Raster *result = sub_raster_raster(rast1, rast2);
  PG_FREE_IF_COPY(rast1, 0);
  PG_FREE_IF_COPY(rast2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Mul_float_raster(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Mul_float_raster);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the multiplication of a float and a raster, pixel by pixel
 * @sqlfn rasterMul()
 * @sqlop @p *
 */
Datum
Mul_float_raster(PG_FUNCTION_ARGS)
{
  double d = PG_GETARG_FLOAT8(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  Raster *result = mul_float_raster(d, rast);
  PG_FREE_IF_COPY(rast, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Mul_raster_float(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Mul_raster_float);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the multiplication of a raster and a float, pixel by pixel
 * @sqlfn rasterMul()
 * @sqlop @p *
 */
Datum
Mul_raster_float(PG_FUNCTION_ARGS)
{
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  double d = PG_GETARG_FLOAT8(1);
  Raster *result = mul_raster_float(rast, d);
  PG_FREE_IF_COPY(rast, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Mul_raster_raster(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Mul_raster_raster);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the multiplication of two rasters, pixel by pixel
 * @details The rasters are operated over the extent they share
 * @sqlfn rasterMul()
 * @sqlop @p *
 */
Datum
Mul_raster_raster(PG_FUNCTION_ARGS)
{
  Raster *rast1 = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  Raster *rast2 = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  Raster *result = mul_raster_raster(rast1, rast2);
  PG_FREE_IF_COPY(rast1, 0);
  PG_FREE_IF_COPY(rast2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Div_float_raster(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Div_float_raster);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the division of a float and a raster, pixel by pixel
 * @sqlfn rasterDiv()
 * @sqlop @p /
 */
Datum
Div_float_raster(PG_FUNCTION_ARGS)
{
  double d = PG_GETARG_FLOAT8(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  Raster *result = div_float_raster(d, rast);
  PG_FREE_IF_COPY(rast, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Div_raster_float(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Div_raster_float);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the division of a raster and a float, pixel by pixel
 * @sqlfn rasterDiv()
 * @sqlop @p /
 */
Datum
Div_raster_float(PG_FUNCTION_ARGS)
{
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  double d = PG_GETARG_FLOAT8(1);
  Raster *result = div_raster_float(rast, d);
  PG_FREE_IF_COPY(rast, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Div_raster_raster(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Div_raster_raster);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the division of two rasters, pixel by pixel
 * @details The rasters are operated over the extent they share
 * @sqlfn rasterDiv()
 * @sqlop @p /
 */
Datum
Div_raster_raster(PG_FUNCTION_ARGS)
{
  Raster *rast1 = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  Raster *rast2 = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  Raster *result = div_raster_raster(rast1, rast2);
  PG_FREE_IF_COPY(rast1, 0);
  PG_FREE_IF_COPY(rast2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Raster type: terrain functions
 *****************************************************************************/

PGDLLEXPORT Datum Raster_slope(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_slope);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the slope of a raster band, pixel by pixel
 * @sqlfn slope()
 */
Datum
Raster_slope(PG_FUNCTION_ARGS)
{
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  int band = PG_GETARG_INT32(1);
  char *pixeltype = text_to_cstring(PG_GETARG_TEXT_P(2));
  char *units = text_to_cstring(PG_GETARG_TEXT_P(3));
  double scale = PG_GETARG_FLOAT8(4);
  bool interpolate_nodata = PG_GETARG_BOOL(5);
  Raster *result = raster_slope(rast, band, NULL, pixeltype, units, scale,
    interpolate_nodata);
  pfree(pixeltype); pfree(units);
  PG_FREE_IF_COPY(rast, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_slope_extent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_slope_extent);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the slope of a raster band over the extent of another raster
 * @sqlfn slope()
 */
Datum
Raster_slope_extent(PG_FUNCTION_ARGS)
{
  /* The function is not STRICT, so that a custom extent left NULL is the
   * extent of the raster, as for PostGIS; every other argument is tested */
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(3) ||
      PG_ARGISNULL(4) || PG_ARGISNULL(5) || PG_ARGISNULL(6))
    PG_RETURN_NULL();
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  int band = PG_GETARG_INT32(1);
  Raster *extent = PG_ARGISNULL(2) ? NULL :
    (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(2));
  char *pixeltype = text_to_cstring(PG_GETARG_TEXT_P(3));
  char *units = text_to_cstring(PG_GETARG_TEXT_P(4));
  double scale = PG_GETARG_FLOAT8(5);
  bool interpolate_nodata = PG_GETARG_BOOL(6);
  Raster *result = raster_slope(rast, band, extent, pixeltype, units, scale,
    interpolate_nodata);
  pfree(pixeltype); pfree(units);
  PG_FREE_IF_COPY(rast, 0);
  if (extent)
    PG_FREE_IF_COPY(extent, 2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * @brief Return a terrain index of a raster band
 * @details The wrappers of #raster_tri and #raster_tpi, which take the same
 * arguments
 * @param[in] fcinfo Arguments of the function
 * @param[in] has_extent True where the second argument is the custom extent
 * @param[in] func MEOS function computing the index
 */
static Datum
Raster_terrain_index(FunctionCallInfo fcinfo, bool has_extent,
  Raster * (*func)(const Raster *, int, const Raster *, const char *, bool))
{
  int i = has_extent ? 1 : 0;
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2 + i) ||
      PG_ARGISNULL(3 + i))
    PG_RETURN_NULL();
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  int band = PG_GETARG_INT32(1);
  Raster *extent = (has_extent && ! PG_ARGISNULL(2)) ?
    (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(2)) : NULL;
  char *pixeltype = text_to_cstring(PG_GETARG_TEXT_P(2 + i));
  bool interpolate_nodata = PG_GETARG_BOOL(3 + i);
  Raster *result = func(rast, band, extent, pixeltype, interpolate_nodata);
  pfree(pixeltype);
  PG_FREE_IF_COPY(rast, 0);
  if (extent)
    PG_FREE_IF_COPY(extent, 2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_tri(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_tri);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the terrain ruggedness index of a raster band, pixel by pixel
 * @sqlfn tri()
 */
Datum
Raster_tri(PG_FUNCTION_ARGS)
{
  return Raster_terrain_index(fcinfo, false, &raster_tri);
}

PGDLLEXPORT Datum Raster_tri_extent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_tri_extent);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the terrain ruggedness index of a band over a custom extent
 * @sqlfn tri()
 */
Datum
Raster_tri_extent(PG_FUNCTION_ARGS)
{
  return Raster_terrain_index(fcinfo, true, &raster_tri);
}

PGDLLEXPORT Datum Raster_tpi(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_tpi);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the topographic position index of a raster band
 * @sqlfn tpi()
 */
Datum
Raster_tpi(PG_FUNCTION_ARGS)
{
  return Raster_terrain_index(fcinfo, false, &raster_tpi);
}

PGDLLEXPORT Datum Raster_tpi_extent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_tpi_extent);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the topographic position index of a band over a custom extent
 * @sqlfn tpi()
 */
Datum
Raster_tpi_extent(PG_FUNCTION_ARGS)
{
  return Raster_terrain_index(fcinfo, true, &raster_tpi);
}

/*****************************************************************************
 * Raster type: statistics over a neighbourhood
 *****************************************************************************/

PGDLLEXPORT Datum Raster_map_algebra(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_map_algebra);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a statistic of the neighbourhood of each pixel of a band
 * @sqlfn mapAlgebra()
 */
Datum
Raster_map_algebra(PG_FUNCTION_ARGS)
{
  /* The function is not STRICT, so that a pixel type and a custom extent
   * left NULL reach MEOS, as for PostGIS, as #Raster_slope_extent lets a
   * custom extent through; every other argument is tested */
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2) ||
      PG_ARGISNULL(6) || PG_ARGISNULL(7))
    PG_RETURN_NULL();
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  int band = PG_GETARG_INT32(1);
  char *stat = text_to_cstring(PG_GETARG_TEXT_P(2));
  char *pixeltype = PG_ARGISNULL(3) ? NULL :
    text_to_cstring(PG_GETARG_TEXT_P(3));
  char *extenttype = PG_ARGISNULL(4) ? NULL :
    text_to_cstring(PG_GETARG_TEXT_P(4));
  Raster *extent = PG_ARGISNULL(5) ? NULL :
    (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(5));
  int32 distancex = PG_GETARG_INT32(6);
  int32 distancey = PG_GETARG_INT32(7);
  Raster *result = raster_map_algebra(rast, band, stat, pixeltype, extenttype,
    extent, distancex, distancey);
  pfree(stat);
  if (pixeltype)
    pfree(pixeltype);
  if (extenttype)
    pfree(extenttype);
  PG_FREE_IF_COPY(rast, 0);
  if (extent)
    PG_FREE_IF_COPY(extent, 5);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Raster type: alignment
 *****************************************************************************/

PGDLLEXPORT Datum Raster_same_alignment(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_same_alignment);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if two rasters lie on the same grid
 * @details The two rasters are read as #Add_raster_raster reads them
 * @sqlfn sameAlignment()
 */
Datum
Raster_same_alignment(PG_FUNCTION_ARGS)
{
  Raster *rast1 = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  Raster *rast2 = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  bool result = raster_same_alignment(rast1, rast2);
  PG_FREE_IF_COPY(rast1, 0);
  PG_FREE_IF_COPY(rast2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Raster_not_same_alignment_reason(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_not_same_alignment_reason);
/**
 * @ingroup mobilitydb_raster
 * @brief Return why two rasters do not lie on the same grid
 * @details The text is returned as #Raster_band_pixel_type returns the name
 * of a pixel type
 * @sqlfn notSameAlignmentReason()
 */
Datum
Raster_not_same_alignment_reason(PG_FUNCTION_ARGS)
{
  Raster *rast1 = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  Raster *rast2 = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  char *reason = raster_not_same_alignment_reason(rast1, rast2);
  PG_FREE_IF_COPY(rast1, 0);
  PG_FREE_IF_COPY(rast2, 1);
  if (! reason)
    PG_RETURN_NULL();
  text *result = cstring_to_text(reason);
  pfree(reason);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Raster_same_alignment_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_same_alignment_transfn);
/**
 * @ingroup mobilitydb_raster
 * @brief Transition function of the aggregate testing whether rasters lie on
 * the same grid
 * @details The state lives in the aggregate context, as #Set_union_transfn
 * keeps its own
 * @sqlfn sameAlignmentTransition()
 * @sqlaggfn sameAlignment()
 * @altsqlfn sameAlignmentAgg()
 */
Datum
Raster_same_alignment_transfn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Raster_same_alignment_transfn called in non-aggregate context");
  RasterAlignState *state = PG_ARGISNULL(0) ? NULL :
    (RasterAlignState *) PG_GETARG_POINTER(0);
  /* Skip NULLs */
  if (! PG_ARGISNULL(1))
  {
    Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
    MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
    state = raster_same_alignment_transfn(state, rast);
    MemoryContextSwitchTo(oldctx);
  }
  if (! state)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Raster_same_alignment_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_same_alignment_combinefn);
/**
 * @ingroup mobilitydb_raster
 * @brief Combine function of the aggregate testing whether rasters lie on the
 * same grid
 * @details The states are combined as #Set_union_combinefn combines its own
 * @sqlfn sameAlignmentCombine()
 * @sqlaggfn sameAlignment()
 */
Datum
Raster_same_alignment_combinefn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR,
      "Raster_same_alignment_combinefn called in non-aggregate context");
  RasterAlignState *state1 = PG_ARGISNULL(0) ? NULL :
    (RasterAlignState *) PG_GETARG_POINTER(0);
  RasterAlignState *state2 = PG_ARGISNULL(1) ? NULL :
    (RasterAlignState *) PG_GETARG_POINTER(1);
  MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
  RasterAlignState *result = raster_same_alignment_combinefn(state1, state2);
  MemoryContextSwitchTo(oldctx);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_same_alignment_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_same_alignment_finalfn);
/**
 * @ingroup mobilitydb_raster
 * @brief Final function of the aggregate testing whether rasters lie on the
 * same grid
 * @details An aggregate over no raster answers NULL, as for PostGIS
 * @sqlfn sameAlignmentFinal()
 * @sqlaggfn sameAlignment()
 */
Datum
Raster_same_alignment_finalfn(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();
  RasterAlignState *state = (RasterAlignState *) PG_GETARG_POINTER(0);
  PG_RETURN_BOOL(raster_same_alignment_finalfn(state));
}

PGDLLEXPORT Datum Rasteralignstate_serialize(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Rasteralignstate_serialize);
/**
 * @ingroup mobilitydb_raster
 * @brief Serialize the state of the aggregate testing whether rasters lie on
 * the same grid
 * @details The bytes are wrapped as #Setstate_serialize wraps its own
 * @sqlfn sameAlignmentSerialize()
 * @sqlaggfn sameAlignment()
 */
Datum
Rasteralignstate_serialize(PG_FUNCTION_ARGS)
{
  RasterAlignState *state = (RasterAlignState *) PG_GETARG_POINTER(0);
  size_t size;
  uint8_t *bytes = rasteralignstate_serialize(state, &size);
  bytea *result = palloc(VARHDRSZ + size);
  SET_VARSIZE(result, VARHDRSZ + size);
  memcpy(VARDATA(result), bytes, size);
  pfree(bytes);
  PG_RETURN_BYTEA_P(result);
}

PGDLLEXPORT Datum Rasteralignstate_deserialize(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Rasteralignstate_deserialize);
/**
 * @ingroup mobilitydb_raster
 * @brief Deserialize the state of the aggregate testing whether rasters lie
 * on the same grid
 * @details The state is built in the aggregate context, as
 * #Setstate_deserialize builds its own
 * @sqlfn sameAlignmentDeserialize()
 * @sqlaggfn sameAlignment()
 */
Datum
Rasteralignstate_deserialize(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR,
      "Rasteralignstate_deserialize called in non-aggregate context");
  bytea *data = PG_GETARG_BYTEA_P(0);
  MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
  RasterAlignState *result = rasteralignstate_deserialize(
    (uint8_t *) VARDATA(data), VARSIZE(data) - VARHDRSZ);
  MemoryContextSwitchTo(oldctx);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Raster type: union
 *****************************************************************************/

PGDLLEXPORT Datum Raster_union_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_union_transfn);
/**
 * @ingroup mobilitydb_raster
 * @brief Transition function of the aggregate uniting rasters
 * @details The forms take the band and the pixel operation, the band alone,
 * or neither, and are told apart by their number of arguments, 4, 3 or 2, as
 * #Temporal_app_tinst_transfn tells apart those of appendInstant; the form
 * taking the pixel operation alone is #Raster_union_uniontype_transfn. A band
 * left out or NULL is 0, which unites every band, and a pixel operation left
 * out or NULL is `LAST`, as PostGIS's `RASTER_union_transfn` reads them. The
 * state lives in the aggregate context, as #Raster_same_alignment_transfn
 * keeps its own
 * @sqlfn rasterUnionTransition()
 * @sqlaggfn rasterUnion()
 */
Datum
Raster_union_transfn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Raster_union_transfn called in non-aggregate context");
  RasterUnionState *state = PG_ARGISNULL(0) ? NULL :
    (RasterUnionState *) PG_GETARG_POINTER(0);
  int band = 0;
  char *uniontype = NULL;
  if (PG_NARGS() > 2)
  {
    if (! PG_ARGISNULL(2))
      band = PG_GETARG_INT32(2);
    if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
      uniontype = text_to_cstring(PG_GETARG_TEXT_P(3));
  }
  /* Skip NULLs */
  if (! PG_ARGISNULL(1))
  {
    Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
    MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
    state = raster_union_transfn(state, rast, band, uniontype);
    MemoryContextSwitchTo(oldctx);
  }
  if (uniontype)
    pfree(uniontype);
  if (! state)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Raster_union_uniontype_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_union_uniontype_transfn);
/**
 * @ingroup mobilitydb_raster
 * @brief Transition function of the aggregate uniting rasters over every band
 * by a pixel operation
 * @details The form of #Raster_union_transfn taking the pixel operation
 * alone, which unites every band, as #Concat_jsonbset_jsonb is the form of
 * #Concat_jsonb_jsonbset taking its arguments the other way round. A NULL
 * pixel operation is `LAST`
 * @sqlfn rasterUnionTransition()
 * @sqlaggfn rasterUnion()
 */
Datum
Raster_union_uniontype_transfn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Raster_union_uniontype_transfn called in non-aggregate context");
  RasterUnionState *state = PG_ARGISNULL(0) ? NULL :
    (RasterUnionState *) PG_GETARG_POINTER(0);
  char *uniontype = PG_ARGISNULL(2) ? NULL :
    text_to_cstring(PG_GETARG_TEXT_P(2));
  /* Skip NULLs */
  if (! PG_ARGISNULL(1))
  {
    Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
    MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
    state = raster_union_transfn(state, rast, 0, uniontype);
    MemoryContextSwitchTo(oldctx);
  }
  if (uniontype)
    pfree(uniontype);
  if (! state)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Raster_union_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_union_combinefn);
/**
 * @ingroup mobilitydb_raster
 * @brief Combine function of the aggregate uniting rasters
 * @details The states are combined as #Raster_same_alignment_combinefn
 * combines its own
 * @sqlfn rasterUnionCombine()
 * @sqlaggfn rasterUnion()
 */
Datum
Raster_union_combinefn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Raster_union_combinefn called in non-aggregate context");
  RasterUnionState *state1 = PG_ARGISNULL(0) ? NULL :
    (RasterUnionState *) PG_GETARG_POINTER(0);
  RasterUnionState *state2 = PG_ARGISNULL(1) ? NULL :
    (RasterUnionState *) PG_GETARG_POINTER(1);
  MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
  RasterUnionState *result = raster_union_combinefn(state1, state2);
  MemoryContextSwitchTo(oldctx);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_union_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_union_finalfn);
/**
 * @ingroup mobilitydb_raster
 * @brief Final function of the aggregate uniting rasters
 * @details An aggregate over no raster answers NULL, as for PostGIS
 * @sqlfn rasterUnionFinal()
 * @sqlaggfn rasterUnion()
 */
Datum
Raster_union_finalfn(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();
  RasterUnionState *state = (RasterUnionState *) PG_GETARG_POINTER(0);
  Raster *result = raster_union_finalfn(state);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Rasterunionstate_serialize(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Rasterunionstate_serialize);
/**
 * @ingroup mobilitydb_raster
 * @brief Serialize the state of the aggregate uniting rasters
 * @details The bytes are wrapped as #Rasteralignstate_serialize wraps its own
 * @sqlfn rasterUnionSerialize()
 * @sqlaggfn rasterUnion()
 */
Datum
Rasterunionstate_serialize(PG_FUNCTION_ARGS)
{
  RasterUnionState *state = (RasterUnionState *) PG_GETARG_POINTER(0);
  size_t size;
  uint8_t *bytes = rasterunionstate_serialize(state, &size);
  bytea *result = palloc(VARHDRSZ + size);
  SET_VARSIZE(result, VARHDRSZ + size);
  memcpy(VARDATA(result), bytes, size);
  pfree(bytes);
  PG_RETURN_BYTEA_P(result);
}

PGDLLEXPORT Datum Rasterunionstate_deserialize(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Rasterunionstate_deserialize);
/**
 * @ingroup mobilitydb_raster
 * @brief Deserialize the state of the aggregate uniting rasters
 * @details The state is built in the aggregate context, as
 * #Rasteralignstate_deserialize builds its own
 * @sqlfn rasterUnionDeserialize()
 * @sqlaggfn rasterUnion()
 */
Datum
Rasterunionstate_deserialize(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR,
      "Rasterunionstate_deserialize called in non-aggregate context");
  bytea *data = PG_GETARG_BYTEA_P(0);
  MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
  RasterUnionState *result = rasterunionstate_deserialize(
    (uint8_t *) VARDATA(data), VARSIZE(data) - VARHDRSZ);
  MemoryContextSwitchTo(oldctx);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Raster type: summary statistics of many rasters
 *****************************************************************************/

PGDLLEXPORT Datum Raster_summary_stats_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_summary_stats_transfn);
/**
 * @ingroup mobilitydb_raster
 * @brief Transition function of the aggregate summarizing the pixels of the
 * first band of rasters
 * @details The form taking whether nodata is excluded alone, over band 1, as
 * #Raster_dump_values reads band 1 where #Raster_dump_values_band takes the
 * band; the form taking the band is #Raster_summary_stats_band_transfn. A
 * NULL exclusion is true, as PostGIS's `RASTER_summaryStats_transfn` reads
 * it. The state lives in the aggregate context, as
 * #Raster_same_alignment_transfn keeps its own
 * @sqlfn summaryStatsAggTransition()
 * @sqlaggfn summaryStatsAgg()
 */
Datum
Raster_summary_stats_transfn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Raster_summary_stats_transfn called in non-aggregate context");
  RasterStatsState *state = PG_ARGISNULL(0) ? NULL :
    (RasterStatsState *) PG_GETARG_POINTER(0);
  bool exclude_nodata = PG_ARGISNULL(2) ? true : PG_GETARG_BOOL(2);
  /* Skip NULLs */
  if (! PG_ARGISNULL(1))
  {
    Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
    MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
    state = raster_summary_stats_transfn(state, rast, 1, exclude_nodata);
    MemoryContextSwitchTo(oldctx);
  }
  if (! state)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Raster_summary_stats_band_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_summary_stats_band_transfn);
/**
 * @ingroup mobilitydb_raster
 * @brief Transition function of the aggregate summarizing the pixels of a
 * band of rasters
 * @details The form of #Raster_summary_stats_transfn taking the band. A NULL
 * band is 1 and a NULL exclusion is true, as PostGIS's
 * `RASTER_summaryStats_transfn` reads them
 * @sqlfn summaryStatsAggTransition()
 * @sqlaggfn summaryStatsAgg()
 */
Datum
Raster_summary_stats_band_transfn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Raster_summary_stats_band_transfn called in non-aggregate context");
  RasterStatsState *state = PG_ARGISNULL(0) ? NULL :
    (RasterStatsState *) PG_GETARG_POINTER(0);
  int band = PG_ARGISNULL(2) ? 1 : PG_GETARG_INT32(2);
  bool exclude_nodata = PG_ARGISNULL(3) ? true : PG_GETARG_BOOL(3);
  /* Skip NULLs */
  if (! PG_ARGISNULL(1))
  {
    Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
    MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
    state = raster_summary_stats_transfn(state, rast, band, exclude_nodata);
    MemoryContextSwitchTo(oldctx);
  }
  if (! state)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Raster_summary_stats_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_summary_stats_combinefn);
/**
 * @ingroup mobilitydb_raster
 * @brief Combine function of the aggregate summarizing the pixels of rasters
 * @details The states are combined as #Raster_same_alignment_combinefn
 * combines its own
 * @sqlfn summaryStatsAggCombine()
 * @sqlaggfn summaryStatsAgg()
 */
Datum
Raster_summary_stats_combinefn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR,
      "Raster_summary_stats_combinefn called in non-aggregate context");
  RasterStatsState *state1 = PG_ARGISNULL(0) ? NULL :
    (RasterStatsState *) PG_GETARG_POINTER(0);
  RasterStatsState *state2 = PG_ARGISNULL(1) ? NULL :
    (RasterStatsState *) PG_GETARG_POINTER(1);
  MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
  RasterStatsState *result = raster_summary_stats_combinefn(state1, state2);
  MemoryContextSwitchTo(oldctx);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_summary_stats_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_summary_stats_finalfn);
/**
 * @ingroup mobilitydb_raster
 * @brief Final function of the aggregate summarizing the pixels of rasters
 * @details An aggregate over no raster answers NULL, and one whose rasters
 * count no pixel the record counting none, as for PostGIS
 * @sqlfn summaryStatsAggFinal()
 * @sqlaggfn summaryStatsAgg()
 */
Datum
Raster_summary_stats_finalfn(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();
  RasterStatsState *state = (RasterStatsState *) PG_GETARG_POINTER(0);
  BandStats *stats = raster_summary_stats_finalfn(state);
  PG_RETURN_DATUM(bandstats_record(fcinfo, stats));
}

PGDLLEXPORT Datum Rasterstatsstate_serialize(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Rasterstatsstate_serialize);
/**
 * @ingroup mobilitydb_raster
 * @brief Serialize the state of the aggregate summarizing the pixels of
 * rasters
 * @details The bytes are wrapped as #Rasteralignstate_serialize wraps its own
 * @sqlfn summaryStatsAggSerialize()
 * @sqlaggfn summaryStatsAgg()
 */
Datum
Rasterstatsstate_serialize(PG_FUNCTION_ARGS)
{
  RasterStatsState *state = (RasterStatsState *) PG_GETARG_POINTER(0);
  size_t size;
  uint8_t *bytes = rasterstatsstate_serialize(state, &size);
  bytea *result = palloc(VARHDRSZ + size);
  SET_VARSIZE(result, VARHDRSZ + size);
  memcpy(VARDATA(result), bytes, size);
  pfree(bytes);
  PG_RETURN_BYTEA_P(result);
}

PGDLLEXPORT Datum Rasterstatsstate_deserialize(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Rasterstatsstate_deserialize);
/**
 * @ingroup mobilitydb_raster
 * @brief Deserialize the state of the aggregate summarizing the pixels of
 * rasters
 * @details The state is built in the aggregate context, as
 * #Rasteralignstate_deserialize builds its own
 * @sqlfn summaryStatsAggDeserialize()
 * @sqlaggfn summaryStatsAgg()
 */
Datum
Rasterstatsstate_deserialize(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR,
      "Rasterstatsstate_deserialize called in non-aggregate context");
  bytea *data = PG_GETARG_BYTEA_P(0);
  MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
  RasterStatsState *result = rasterstatsstate_deserialize(
    (uint8_t *) VARDATA(data), VARSIZE(data) - VARHDRSZ);
  MemoryContextSwitchTo(oldctx);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Raster type: value count
 *****************************************************************************/

/**
 * @brief Read the arguments of a value count, in the three forms of PostGIS's
 * @p ST_ValueCount and @p ST_ValuePercent
 * @details The forms take the band and whether nodata is excluded, the band
 * alone, or neither, before the search values and the rounding, and are told
 * apart by their number of arguments, 5, 4 or 3. The search values are read
 * as #Raster_clip_bands reads its nodata values. A band left out is 1 and an
 * exclusion left out is true, as for PostGIS
 * @param[in] fcinfo Arguments of the function
 * @param[in] scalar True where the search value is one number, false where it
 * is an array
 * @param[out] band Number of the band
 * @param[out] exclude_nodata Whether nodata is excluded
 * @param[out] searchvalues Search values, NULL for every value
 * @param[out] nsearch Number of search values
 * @param[out] roundto Multiple the values are rounded to
 * @return False where an argument the function requires is NULL
 */
static bool
value_count_args(FunctionCallInfo fcinfo, bool scalar, int *band,
  bool *exclude_nodata, double **searchvalues, int *nsearch, double *roundto)
{
  int nargs = PG_NARGS();
  /* The search values stand before the rounding, the last argument */
  int s = nargs - 2;
  if (PG_ARGISNULL(0) || PG_ARGISNULL(nargs - 1) ||
      (nargs >= 4 && PG_ARGISNULL(1)) || (nargs == 5 && PG_ARGISNULL(2)) ||
      (scalar && PG_ARGISNULL(s)))
    return false;
  *band = nargs >= 4 ? PG_GETARG_INT32(1) : 1;
  *exclude_nodata = nargs == 5 ? PG_GETARG_BOOL(2) : true;
  *roundto = PG_GETARG_FLOAT8(nargs - 1);
  *searchvalues = NULL;
  *nsearch = 0;
  if (scalar)
  {
    *searchvalues = palloc(sizeof(double));
    (*searchvalues)[0] = PG_GETARG_FLOAT8(s);
    *nsearch = 1;
  }
  else if (! PG_ARGISNULL(s))
  {
    Datum *elems = datumarr_extract(PG_GETARG_ARRAYTYPE_P(s), nsearch);
    if (*nsearch > 0)
    {
      *searchvalues = palloc(sizeof(double) * (size_t) *nsearch);
      for (int i = 0; i < *nsearch; i++)
        (*searchvalues)[i] = DatumGetFloat8(elems[i]);
    }
    pfree(elems);
  }
  return true;
}

/**
 * @brief Return the rows of a value count, with the count or with the share
 * of each value
 * @details The rows are returned as #Raster_dump_as_polygons returns its
 * polygons
 * @param[in] fcinfo Arguments of the function
 * @param[in] percent True for the share, false for the count
 */
static Datum
Raster_value_count_rows(FunctionCallInfo fcinfo, bool percent)
{
  FuncCallContext *funcctx;
  if (SRF_IS_FIRSTCALL())
  {
    funcctx = SRF_FIRSTCALL_INIT();
    /* The rows are kept across the calls, so they are computed in the
     * context that outlives them */
    MemoryContext oldctx = MemoryContextSwitchTo(
      funcctx->multi_call_memory_ctx);
    int band, nsearch, count = 0;
    bool exclude_nodata;
    double *searchvalues, roundto;
    ValueCount *rows = NULL;
    if (value_count_args(fcinfo, false, &band, &exclude_nodata, &searchvalues,
        &nsearch, &roundto))
    {
      Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
      rows = raster_value_count(rast, band, exclude_nodata, searchvalues,
        nsearch, roundto, &count);
    }
    funcctx->user_fctx = (void *) rows;
    funcctx->max_calls = count;
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldctx);
  }

  funcctx = SRF_PERCALL_SETUP();
  if (funcctx->call_cntr >= funcctx->max_calls)
    SRF_RETURN_DONE(funcctx);
  const ValueCount *row = &((ValueCount *) funcctx->user_fctx)[
    funcctx->call_cntr];
  Datum values[2];
  bool isnull[2] = {false, false};
  values[0] = Float8GetDatum(row->value);
  values[1] = percent ? Float8GetDatum(row->percent) :
    Int32GetDatum(row->count);
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  SRF_RETURN_NEXT(funcctx, HeapTupleGetDatum(tuple));
}

PGDLLEXPORT Datum Raster_value_count(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_value_count);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the number of pixels of a raster band holding each value
 * @sqlfn valueCount()
 */
Datum
Raster_value_count(PG_FUNCTION_ARGS)
{
  return Raster_value_count_rows(fcinfo, false);
}

PGDLLEXPORT Datum Raster_value_percent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_value_percent);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the share of the pixels of a raster band holding each value
 * @sqlfn valuePercent()
 */
Datum
Raster_value_percent(PG_FUNCTION_ARGS)
{
  return Raster_value_count_rows(fcinfo, true);
}

/**
 * @brief Return the value count of one search value, with the count or with
 * the share of the pixels
 * @param[in] fcinfo Arguments of the function
 * @param[in] percent True for the share, false for the count
 */
static Datum
Raster_value_count_one(FunctionCallInfo fcinfo, bool percent)
{
  int band, nsearch, count = 0;
  bool exclude_nodata;
  double *searchvalues, roundto;
  if (! value_count_args(fcinfo, true, &band, &exclude_nodata, &searchvalues,
      &nsearch, &roundto))
    PG_RETURN_NULL();
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  ValueCount *rows = raster_value_count(rast, band, exclude_nodata,
    searchvalues, nsearch, roundto, &count);
  pfree(searchvalues);
  PG_FREE_IF_COPY(rast, 0);
  /* A band answering no row holds the value in no pixel */
  ValueCount row = {0.0, 0, 0.0};
  if (rows && count > 0)
    row = rows[0];
  if (rows)
    pfree(rows);
  if (percent)
    PG_RETURN_FLOAT8(row.percent);
  PG_RETURN_INT32(row.count);
}

PGDLLEXPORT Datum Raster_value_count_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_value_count_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the number of pixels of a raster band holding a value
 * @sqlfn valueCount()
 */
Datum
Raster_value_count_value(PG_FUNCTION_ARGS)
{
  return Raster_value_count_one(fcinfo, false);
}

PGDLLEXPORT Datum Raster_value_percent_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_value_percent_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the share of the pixels of a raster band holding a value
 * @sqlfn valuePercent()
 */
Datum
Raster_value_percent_value(PG_FUNCTION_ARGS)
{
  return Raster_value_count_one(fcinfo, true);
}

/*****************************************************************************
 * Raster type: conversions
 *****************************************************************************/

PGDLLEXPORT Datum Raster_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_to_stbox);
/**
 * @ingroup mobilitydb_raster
 * @brief Convert a raster into a spatiotemporal box
 * @sqlfn stbox()
 * @altsqlfn rasterStbox()
 */
Datum
Raster_to_stbox(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  STBox *result = raster_to_stbox(rast);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************/
