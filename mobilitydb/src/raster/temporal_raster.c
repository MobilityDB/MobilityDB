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
 * @brief PostgreSQL functions of the raster sampling operators.
 *
 * The wrappers pass their arguments to MEOS, which samples a PostGIS raster
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
 * @sqlfn rasterValue()
 */
Datum
Raster_value(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  int32 band = PG_ARGISNULL(2) ? 1 : PG_GETARG_INT32(2);

  Temporal *result = raster_value(traj, rast, band);

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
 * @sqlfn clip()
 */
Datum
Raster_clip(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  bool crop = PG_GETARG_BOOL(2);
  Raster *result = raster_clip(rast, gs, crop);
  PG_FREE_IF_COPY(gs, 1);
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
 * @sqlfn transform()
 */
Datum
Raster_transform(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  int32_t srid = PG_GETARG_INT32(1);
  char *algorithm = text_to_cstring(PG_GETARG_TEXT_P(2));
  double maxerr = PG_GETARG_FLOAT8(3);
  Raster *result = raster_transform(rast, srid, algorithm, maxerr);
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
  TupleDesc tupdesc;
  if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
    ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
      errmsg("function returning record called in context "
             "that cannot accept type record")));
  tupdesc = BlessTupleDesc(tupdesc);

  BandStats *stats = raster_summary_stats(rast, band, exclude_nodata);
  Datum values[6];
  bool isnull[6] = {false, false, false, false, false, false};
  /* An error in MEOS never returns here, so a missing result is a band with no
   * pixel to count: the record PostGIS answers for it counts none and states
   * no statistic, rather than being no record at all */
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
  PG_RETURN_DATUM(HeapTupleGetDatum(tuple));
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
  int64 quadbin = PG_GETARG_INT64(4);
  text *pixtype_t = PG_GETARG_TEXT_PP(5);
  float8 nodata = PG_GETARG_FLOAT8(6);
  bool has_nd = PG_GETARG_BOOL(7);

  const uint8_t *pixels = (const uint8_t *) VARDATA_ANY(pxbytea);
  size_t pixels_size = (size_t) VARSIZE_ANY_EXHDR(pxbytea);
  MeosPixType pixtype = text_to_pixtype(pixtype_t);

  Temporal *result = raster_tile_value_quadbin(traj, pixels, pixels_size,
    width, height, (uint64) quadbin, pixtype, nodata, has_nd);

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
  int64 quadbin = PG_GETARG_INT64(3);
  text *pixtype_t = PG_GETARG_TEXT_PP(4);
  bool has_nd = ! PG_ARGISNULL(5);
  float8 nodata = has_nd ? PG_GETARG_FLOAT8(5) : 0.0;
  MeosPixType pixtype = text_to_pixtype(pixtype_t);

  const uint8_t *pixels = (const uint8_t *) VARDATA_ANY(pxbytea);
  Raquet *result = raquet_make((uint64) quadbin, width, height, pixtype,
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
  uint64 quadbin = PG_ARGISNULL(1) ? 0 : (uint64) PG_GETARG_INT64(1);
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
  uint64 quadbin = PG_ARGISNULL(1) ? 0 : (uint64) PG_GETARG_INT64(1);
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
  ensure_not_empty_array(array);
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
 * trajectory_quadbins
 *****************************************************************************/

PGDLLEXPORT Datum Trajectory_quadbins(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trajectory_quadbins);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the distinct QUADBIN cells at a zoom level covered by a
 * trajectory, for use as a WHERE-clause join key against a Raquet table
 * @param[in] traj  Trajectory (tgeompoint, SRID 4326)
 * @param[in] zoom  QUADBIN zoom level (0–15)
 * @sqlfn quadbins()
 */
Datum
Trajectory_quadbins(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  int32     zoom = PG_GETARG_INT32(1);

  int       ncells;
  uint64   *cells = trajectory_quadbins(traj, (uint32_t) zoom, &ncells);

  PG_FREE_IF_COPY(traj, 0);

  /* Build int8[] (bigint[]) from the uint64 cell array */
  Datum *elems = palloc(sizeof(Datum) * ncells);
  for (int i = 0; i < ncells; i++)
    elems[i] = Int64GetDatum((int64) cells[i]);
  pfree(cells);

  ArrayType *arr = construct_array(elems, ncells, INT8OID, 8, true, TYPALIGN_DOUBLE);
  pfree(elems);

  PG_RETURN_ARRAYTYPE_P(arr);
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
  uint64 result = raquet_quadbin(rq);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_INT64((int64) result);
}

PGDLLEXPORT Datum Raquet_width(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_width);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the width in pixels of a Raquet tile
 * @sqlfn width()
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
 * Raster type: conversions
 *****************************************************************************/

PGDLLEXPORT Datum Raster_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_to_stbox);
/**
 * @ingroup mobilitydb_raster
 * @brief Convert a raster into a spatiotemporal box
 * @sqlfn stbox()
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
