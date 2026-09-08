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
 * @brief General functions for the PostGIS raster type, backed by the
 * vendored rt_core library.
 *
 * This is the only MEOS source file that includes `librtcore.h`. A `Raster`
 * is the serialized on-disk form that PostgreSQL stores for the PostGIS
 * `raster` type; it is passed through unchanged and deserialized here on
 * demand.
 */

/* librtcore.h reads GDAL, which reads <sys/stat.h> under its own name, while
 * PostgreSQL's win32_port.h renames stat away and states the condition itself:
 * "We must pull in sys/stat.h before this part, else our overrides lose". The
 * header is therefore read RENAMED here first, which is PostgreSQL's own
 * prologue (pgtypes/port/win32_port.h), so its struct stat is the only one. */
#ifdef _WIN32
#define fstat microsoft_native_fstat
#define stat microsoft_native_stat
#include <sys/stat.h>
#undef fstat
#undef stat
#endif

#include "librtcore.h"

/* C */
#include <string.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_raster.h>
#include "geo/geo_funcs.h"
#include "raster/raquet.h"
#include "raster/raster_quadbin.h"
#include "temporal/temporal.h"

/*****************************************************************************
 * Input and output functions
 *
 * A `Raster` is the serialized form, while the interchange representation of
 * the PostGIS `raster` type is its WKB. The two are distinct byte streams, so
 * these functions convert rather than cast: rt_core parses the WKB against the
 * length it is given, and the resulting raster is then serialized.
 *****************************************************************************/

/**
 * @brief Destroy an rt_core raster together with its bands
 * @details rt_raster_destroy() releases the band registry but not the bands
 * it points to, which the caller of a function producing them owns. The
 * raster must carry its bands, that is, it must come from
 * rt_raster_from_wkb(), rt_raster_from_hexwkb(), or a full
 * rt_raster_deserialize(): after a header-only deserialization the registry
 * is empty while the band count is not, and rt_raster_get_band() reads the
 * registry without testing it
 * @param[in] raster Raster
 */
static void
raster_destroy(rt_raster raster)
{
  int numbands = rt_raster_get_num_bands(raster);
  for (int i = 0; i < numbands; i++)
    rt_band_destroy(rt_raster_get_band(raster, i));
  rt_raster_destroy(raster);
}

/**
 * @brief Return the serialized form of an rt_core raster, destroying the
 * raster
 * @param[in] raster Raster to serialize and destroy, may be @p NULL
 * @errval NULL
 */
static Raster *
raster_serialize_destroy(rt_raster raster)
{
  if (! raster)
  {
    meos_error(ERROR, MEOS_ERR_WKB_INPUT,
      "Could not parse the Well-Known Binary (WKB) representation of a raster");
    return NULL;
  }
  Raster *result = (Raster *) rt_raster_serialize(raster);
  raster_destroy(raster);
  if (! result)
  {
    meos_error(ERROR, MEOS_ERR_WKB_INPUT, "Could not serialize raster");
    return NULL;
  }
  return result;
}

/**
 * @ingroup meos_raster_base_inout
 * @brief Return a raster from its Well-Known Binary (WKB) representation
 * @param[in] wkb WKB string
 * @param[in] size Size of the string
 * @errval NULL
 */
Raster *
raster_from_wkb(const uint8_t *wkb, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb, NULL);
  /* rt_core takes the length as a uint32_t, which is also the widest raster it
   * can serialize, its size field being a uint32_t varlena header */
  if (size > UINT32_MAX)
  {
    meos_error(ERROR, MEOS_ERR_WKB_INPUT,
      "The Well-Known Binary (WKB) representation of a raster must have at "
      "most %u bytes: %zu", UINT32_MAX, size);
    return NULL;
  }
  return raster_serialize_destroy(rt_raster_from_wkb(wkb, (uint32_t) size));
}

/**
 * @ingroup meos_raster_base_inout
 * @brief Return a raster from its ASCII hex-encoded Well-Known Binary
 * (HexWKB) representation
 * @param[in] hexwkb HexWKB string
 * @errval NULL
 */
Raster *
raster_from_hexwkb(const char *hexwkb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(hexwkb, NULL);
  size_t size = strlen(hexwkb);
  if (size > UINT32_MAX)
  {
    meos_error(ERROR, MEOS_ERR_WKB_INPUT,
      "The ASCII hex-encoded Well-Known Binary (HexWKB) representation of a "
      "raster must have at most %u bytes: %zu", UINT32_MAX, size);
    return NULL;
  }
  return raster_serialize_destroy(rt_raster_from_hexwkb(hexwkb,
    (uint32_t) size));
}

/**
 * @ingroup meos_raster_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a raster
 * @param[in] rast Raster
 * @param[out] size_out Size of the output
 * @errval NULL
 */
uint8_t *
raster_as_wkb(const Raster *rast, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  /* The bands are needed, so the raster is fully deserialized. It keeps
   * pointers into `rast` without owning them, and is destroyed below before
   * `rast` is handed back to the caller */
  rt_raster raster = rt_raster_deserialize((void *) rast, 0);
  if (! raster)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }
  uint32_t wkb_size;
  uint8_t *result = rt_raster_to_wkb(raster, 0, &wkb_size);
  raster_destroy(raster);
  if (! result)
  {
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Could not output the Well-Known Binary (WKB) representation of a "
      "raster");
    return NULL;
  }
  *size_out = (size_t) wkb_size;
  return result;
}

/**
 * @ingroup meos_raster_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a raster
 * @param[in] rast Raster
 * @param[out] size_out Size of the output, not counting the null terminator
 * @errval NULL
 */
char *
raster_as_hexwkb(const Raster *rast, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  rt_raster raster = rt_raster_deserialize((void *) rast, 0);
  if (! raster)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }
  uint32_t hexwkb_size;
  char *result = rt_raster_to_hexwkb(raster, 0, &hexwkb_size);
  raster_destroy(raster);
  if (! result)
  {
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Could not output the ASCII hex-encoded Well-Known Binary (HexWKB) "
      "representation of a raster");
    return NULL;
  }
  *size_out = (size_t) hexwkb_size;
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @brief Return the header of a serialized raster, that is, its grid without
 * its bands
 * @details The shape of a raster is stated by its header alone, so an accessor
 * reading the grid deserializes with the band data left where it lies and
 * releases the result with rt_raster_destroy(). A band accessor cannot take
 * this path: after a header-only deserialization the band registry is empty
 * while the band count is not, so #raster_band_of() deserializes in full and
 * releases with ::raster_destroy()
 * @param[in] rast Raster
 * @errval NULL
 */
static rt_raster
raster_header(const Raster *rast)
{
  rt_raster raster = rt_raster_deserialize((void *) rast, 1);
  if (! raster)
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
  return raster;
}

/**
 * @brief Return a band of a serialized raster together with the raster
 * carrying it
 * @param[in] rast Raster
 * @param[in] band Band number (1-based)
 * @param[out] raster Raster the band is read from, which the caller releases
 * with ::raster_destroy() once it is done with the band
 * @errval NULL
 * @note A failure releases the raster and sets @p raster to @p NULL
 */
static rt_band
raster_band_of(const Raster *rast, int band, rt_raster *raster)
{
  *raster = NULL;
  rt_raster rt = rt_raster_deserialize((void *) rast, 0);
  if (! rt)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }
  int numbands = (int) rt_raster_get_num_bands(rt);
  if (band < 1 || band > numbands)
  {
    raster_destroy(rt);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster has no band %d, it has %d", band, numbands);
    return NULL;
  }
  /* Fetch the band using the 0-based internal index */
  rt_band result = rt_raster_get_band(rt, (uint32_t) (band - 1));
  if (! result)
  {
    raster_destroy(rt);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not read band %d of the raster", band);
    return NULL;
  }
  *raster = rt;
  return result;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the number of bands of a raster
 * @param[in] rast Raster
 * @errval -1
 * @csqlfn #Raster_num_bands()
 */
int
raster_num_bands(const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, -1);
  rt_raster raster = raster_header(rast);
  if (! raster)
    return -1;
  int result = (int) rt_raster_get_num_bands(raster);
  rt_raster_destroy(raster);
  return result;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the width in pixels of a raster
 * @param[in] rast Raster
 * @errval INT_MAX
 */
int
raster_width(const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, INT_MAX);
  rt_raster raster = raster_header(rast);
  if (! raster)
    return INT_MAX;
  int result = (int) rt_raster_get_width(raster);
  rt_raster_destroy(raster);
  return result;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the height in pixels of a raster
 * @param[in] rast Raster
 * @errval INT_MAX
 */
int
raster_height(const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, INT_MAX);
  rt_raster raster = raster_header(rast);
  if (! raster)
    return INT_MAX;
  int result = (int) rt_raster_get_height(raster);
  rt_raster_destroy(raster);
  return result;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the spatial reference system identifier of a raster
 * @param[in] rast Raster
 * @errval SRID_INVALID
 */
int32_t
raster_srid(const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, SRID_INVALID);
  rt_raster raster = raster_header(rast);
  if (! raster)
    return SRID_INVALID;
  int32_t result = (int32_t) rt_raster_get_srid(raster);
  rt_raster_destroy(raster);
  return result;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the X coordinate of the upper left corner of a raster
 * @param[in] rast Raster
 * @errval DBL_MAX
 * @note This is the origin the geotransform states, which the skew rotates
 * the grid about, so it is a corner of the extent only when both skews are
 * zero
 */
double
raster_upper_left_x(const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, DBL_MAX);
  rt_raster raster = raster_header(rast);
  if (! raster)
    return DBL_MAX;
  double result = rt_raster_get_x_offset(raster);
  rt_raster_destroy(raster);
  return result;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the Y coordinate of the upper left corner of a raster
 * @param[in] rast Raster
 * @errval DBL_MAX
 * @note This is the origin the geotransform states, which the skew rotates
 * the grid about, so it is a corner of the extent only when both skews are
 * zero
 */
double
raster_upper_left_y(const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, DBL_MAX);
  rt_raster raster = raster_header(rast);
  if (! raster)
    return DBL_MAX;
  double result = rt_raster_get_y_offset(raster);
  rt_raster_destroy(raster);
  return result;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the pixel width of a raster, that is, the X component of its scale
 * @param[in] rast Raster
 * @errval DBL_MAX
 */
double
raster_scale_x(const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, DBL_MAX);
  rt_raster raster = raster_header(rast);
  if (! raster)
    return DBL_MAX;
  double result = rt_raster_get_x_scale(raster);
  rt_raster_destroy(raster);
  return result;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the pixel height of a raster, that is, the Y component of its scale
 * @param[in] rast Raster
 * @errval DBL_MAX
 * @note The value is negative for a grid whose rows run north to south,
 * which is how a raster is ordinarily written
 */
double
raster_scale_y(const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, DBL_MAX);
  rt_raster raster = raster_header(rast);
  if (! raster)
    return DBL_MAX;
  double result = rt_raster_get_y_scale(raster);
  rt_raster_destroy(raster);
  return result;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the X component of the skew of a raster
 * @param[in] rast Raster
 * @errval DBL_MAX
 * @note A raster whose two skews are zero is axis-aligned, so a reader that
 * assumes an axis-aligned grid states the assumption by testing them
 */
double
raster_skew_x(const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, DBL_MAX);
  rt_raster raster = raster_header(rast);
  if (! raster)
    return DBL_MAX;
  double result = rt_raster_get_x_skew(raster);
  rt_raster_destroy(raster);
  return result;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the Y component of the skew of a raster
 * @param[in] rast Raster
 * @errval DBL_MAX
 * @note A raster whose two skews are zero is axis-aligned, so a reader that
 * assumes an axis-aligned grid states the assumption by testing them
 */
double
raster_skew_y(const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, DBL_MAX);
  rt_raster raster = raster_header(rast);
  if (! raster)
    return DBL_MAX;
  double result = rt_raster_get_y_skew(raster);
  rt_raster_destroy(raster);
  return result;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the name of the pixel data type of a raster band
 * @param[in] rast Raster
 * @param[in] band Band number (1-based)
 * @errval NULL
 * @note The name returned is the one the RaQuet specification writes, which is
 * what #raquet_pixtype() answers for a tile, so a band and a tile of the same
 * type report the same name. The PostGIS spelling is read through
 * #raquet_pixtype_from_string(), so the one catalog states both vocabularies
 * and `1BB`, `2BUI` and `4BUI` report the `uint8` their bytes already are
 */
char *
raster_band_pixel_type(const Raster *rast, int band)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL);
  rt_raster raster;
  rt_band rtband = raster_band_of(rast, band, &raster);
  if (! rtband)
    return NULL;
  rt_pixtype pixtype = rt_band_get_pixtype(rtband);
  const char *pgname = rt_pixtype_name(pixtype);
  raster_destroy(raster);
  if (pixtype == PT_END || ! pgname)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Band %d of the raster has no known pixel type", band);
    return NULL;
  }
  const char *name = raquet_pixtype_name(raquet_pixtype_from_string(pgname));
  return name ? pstrdup(name) : NULL;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return whether a raster band states a nodata value
 * @param[in] rast Raster
 * @param[in] band Band number (1-based)
 * @errval false
 * @note A band that states none has no pixel to exclude, so every pixel it
 * holds carries a value
 */
bool
raster_band_has_nodata_value(const Raster *rast, int band)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, false);
  rt_raster raster;
  rt_band rtband = raster_band_of(rast, band, &raster);
  if (! rtband)
    return false;
  bool result = (rt_band_get_hasnodata_flag(rtband) != 0);
  raster_destroy(raster);
  return result;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the nodata value of a raster band
 * @param[in] rast Raster
 * @param[in] band Band number (1-based)
 * @errval DBL_MAX
 * @note A band stating no nodata value has none to return, which is an error
 * rather than a value: test it with #raster_band_has_nodata_value()
 */
double
raster_band_nodata_value(const Raster *rast, int band)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, DBL_MAX);
  rt_raster raster;
  rt_band rtband = raster_band_of(rast, band, &raster);
  if (! rtband)
    return DBL_MAX;
  double result;
  rt_errorstate state = rt_band_get_nodata(rtband, &result);
  raster_destroy(raster);
  if (state != ES_NONE)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Band %d of the raster states no nodata value", band);
    return DBL_MAX;
  }
  return result;
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_raster_base_conversion
 * @brief Convert a raster into a spatiotemporal box
 * @param[in] rast Raster
 * @return The X/Y bounding box of the raster in its own reference system, with
 * no T dimension
 * @errval NULL
 * @note The extent bears the rotation of the geotransform, so a skewed raster
 * answers the box that contains its four rotated corners rather than the one
 * the scale alone would give
 * @csqlfn #Raster_to_stbox()
 */
STBox *
raster_to_stbox(const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL);
  rt_raster raster = raster_header(rast);
  if (! raster)
    return NULL;
  rt_envelope env;
  if (rt_raster_get_envelope(raster, &env) != ES_NONE)
  {
    rt_raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not compute the extent of the raster");
    return NULL;
  }
  int32_t srid = (int32_t) rt_raster_get_srid(raster);
  rt_raster_destroy(raster);
  STBox box;
  memset(&box, 0, sizeof(STBox));
  box.xmin = env.MinX;
  box.xmax = env.MaxX;
  box.ymin = env.MinY;
  box.ymax = env.MaxY;
  box.srid = srid;
  MEOS_FLAGS_SET_X(box.flags, true);
  MEOS_FLAGS_SET_Z(box.flags, false);
  MEOS_FLAGS_SET_T(box.flags, false);
  MEOS_FLAGS_SET_GEODETIC(box.flags, false);
  return stbox_copy(&box);
}

/*****************************************************************************
 * Sampling functions
 *
 * The trajectory-sampling algorithm itself lives in #raster_value() and its
 * restriction and predicate companions; what follows is the pixel reader that
 * answers them for a PostGIS raster, so that the operators are computed by
 * MEOS for every binding rather than by PostgreSQL for one of them.
 *****************************************************************************/

/**
 * @brief State a raster sampling call keeps for the length of a trajectory:
 * the deserialized raster, the band the values are read from, and the inverse
 * geotransform, computed once and handed to every point conversion
 */
typedef struct
{
  rt_raster raster;   /**< Raster carrying its bands */
  rt_band band;       /**< Band the pixel values are read from */
  double igt[6];      /**< Inverse geotransform of the raster */
  double step;        /**< Distance between two positions of the walk */
} RasterSampleState;

/**
 * @brief Raster sampling callback reading one pixel of a PostGIS raster
 * through the vendored raster core
 * @details The point is converted to raster coordinates with the inverse
 * geotransform and read with nearest-neighbour resampling. A point outside
 * the pixel grid and a nodata pixel alike answer that there is no value,
 * which is the contract of ::raster_sample_fn
 */
static bool
raster_value_sample(void *ctxp, double x, double y, double *value)
{
  RasterSampleState *state = (RasterSampleState *) ctxp;
  double xr, yr;
  if (rt_raster_geopoint_to_rasterpoint(state->raster, x, y, &xr, &yr,
      state->igt) != ES_NONE)
    return false;
  int isnodata;
  if (rt_band_get_pixel_resample(state->band, xr, yr, RT_NEAREST, value,
      &isnodata) != ES_NONE)
    return false;
  return ! isnodata;
}

/**
 * @brief Build the sampling state and the extent pre-filter shared by every
 * raster sampling function
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rast Raster
 * @param[in] band Band number (1-based)
 * @param[out] state Sampling state; on success its raster is owned by the
 * caller, which releases it with #raster_destroy()
 * @param[out] ops Descriptor of the raster grid
 * @return true on success; on failure sets a MEOS error and returns false
 */
static bool
raster_rtcore_gridops(const Temporal *traj, const Raster *rast, int band,
  RasterSampleState *state, RasterGridOps *ops)
{
  /* The bands are needed, so the raster is fully deserialized. It keeps
   * pointers into `rast` without owning them, and the caller destroys it
   * before `rast` is handed back to its own caller */
  rt_raster raster = rt_raster_deserialize((void *) rast, 0);
  if (! raster)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return false;
  }
  /* A raster and a trajectory in different reference systems state their
   * positions in different units, which the sampling cannot reconcile */
  if (! ensure_same_srid(tspatial_srid(traj), rt_raster_get_srid(raster)))
  {
    raster_destroy(raster);
    return false;
  }
  int numbands = (int) rt_raster_get_num_bands(raster);
  if (band < 1 || band > numbands)
  {
    raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster has no band %d, it has %d", band, numbands);
    return false;
  }
  /* Fetch the band using the 0-based internal index */
  rt_band rtband = rt_raster_get_band(raster, (uint32_t) (band - 1));
  if (! rtband ||
      rt_raster_get_inverse_geotransform_matrix(raster, NULL,
        state->igt) != ES_NONE)
  {
    raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not read band %d of the raster", band);
    return false;
  }
  state->raster = raster;
  state->band = rtband;
  /* Half the smaller pixel side, in the units the trajectory states its
   * positions in, so the walk cannot step over a pixel */
  double gt[6];
  rt_raster_get_geotransform_matrix(raster, gt);
  state->step = raster_sample_step(gt);

  /* Bounding box of the raster extent, which bears the rotation of the
   * geotransform */
  rt_envelope env;
  if (rt_raster_get_envelope(raster, &env) != ES_NONE)
  {
    raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not compute the extent of the raster");
    return false;
  }
  ops->sample = &raster_value_sample;
  ops->ctx = state;
  ops->step = state->step;
  memset(&ops->box, 0, sizeof(STBox));
  ops->box.xmin = env.MinX; ops->box.xmax = env.MaxX;
  ops->box.ymin = env.MinY; ops->box.ymax = env.MaxY;
  return true;
}

/**
 * @ingroup meos_raster
 * @brief Return the values of a raster band sampled at the instants of a
 * trajectory
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rast Raster
 * @param[in] band Band number (1-based)
 * @return A temporal float, or @p NULL when no instant of @p traj falls
 * inside the raster or survives nodata filtering
 * @csqlfn #Raster_value()
 */
Temporal *
raster_value(const Temporal *traj, const Raster *rast, int band)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(traj, NULL); VALIDATE_NOT_NULL(rast, NULL);

  RasterSampleState state;
  RasterGridOps ops;
  if (! raster_rtcore_gridops(traj, rast, band, &state, &ops))
    return NULL;
  Temporal *result = raster_value_sampler(traj, &ops);
  raster_destroy(state.raster);
  return result;
}

/**
 * @ingroup meos_raster
 * @brief Return the instants of a trajectory where the sampled raster pixel
 * value falls inside a float span
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rast Raster
 * @param[in] band Band number (1-based)
 * @param[in] vspan Float value range (inclusive bounds)
 * @return A trajectory restricted to the qualifying instants, or @p NULL
 * when none qualify
 * @csqlfn #Raster_at_value()
 */
Temporal *
raster_at_value(const Temporal *traj, const Raster *rast, int band,
  const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(traj, NULL); VALIDATE_NOT_NULL(rast, NULL);
  VALIDATE_NOT_NULL(vspan, NULL);

  RasterSampleState state;
  RasterGridOps ops;
  if (! raster_rtcore_gridops(traj, rast, band, &state, &ops))
    return NULL;
  Temporal *result = raster_at_value_sampler(traj, &ops, vspan);
  raster_destroy(state.raster);
  return result;
}

/**
 * @ingroup meos_raster
 * @brief Return the instants of a trajectory where the sampled raster pixel
 * value falls outside a float span
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rast Raster
 * @param[in] band Band number (1-based)
 * @param[in] vspan Float value range to exclude
 * @return A trajectory restricted to the qualifying instants, or @p NULL
 * when none qualify
 * @csqlfn #Raster_minus_value()
 */
Temporal *
raster_minus_value(const Temporal *traj, const Raster *rast, int band,
  const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(traj, NULL); VALIDATE_NOT_NULL(rast, NULL);
  VALIDATE_NOT_NULL(vspan, NULL);

  RasterSampleState state;
  RasterGridOps ops;
  if (! raster_rtcore_gridops(traj, rast, band, &state, &ops))
    return NULL;
  Temporal *result = raster_minus_value_sampler(traj, &ops, vspan);
  raster_destroy(state.raster);
  return result;
}

/**
 * @ingroup meos_raster
 * @brief Return true if a trajectory ever samples a raster pixel value inside
 * a float span
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rast Raster
 * @param[in] band Band number (1-based)
 * @param[in] vspan Float value range (inclusive bounds)
 * @errval -1
 * @csqlfn #Eraster_value()
 */
int
eraster_value(const Temporal *traj, const Raster *rast, int band,
  const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(traj, -1); VALIDATE_NOT_NULL(rast, -1);
  VALIDATE_NOT_NULL(vspan, -1);

  RasterSampleState state;
  RasterGridOps ops;
  if (! raster_rtcore_gridops(traj, rast, band, &state, &ops))
    return -1;
  int result = eraster_value_sampler(traj, &ops, vspan);
  raster_destroy(state.raster);
  return result;
}

/**
 * @ingroup meos_raster
 * @brief Return true if every instant of a trajectory that falls inside a
 * raster samples a pixel value inside a float span
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rast Raster
 * @param[in] band Band number (1-based)
 * @param[in] vspan Float value range (inclusive bounds)
 * @errval -1
 * @csqlfn #Araster_value()
 */
int
araster_value(const Temporal *traj, const Raster *rast, int band,
  const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(traj, -1); VALIDATE_NOT_NULL(rast, -1);
  VALIDATE_NOT_NULL(vspan, -1);

  RasterSampleState state;
  RasterGridOps ops;
  if (! raster_rtcore_gridops(traj, rast, band, &state, &ops))
    return -1;
  int result = araster_value_sampler(traj, &ops, vspan);
  raster_destroy(state.raster);
  return result;
}

/*****************************************************************************/
