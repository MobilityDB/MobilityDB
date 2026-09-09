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
#include <ctype.h>
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
 * Processing functions
 *****************************************************************************/

/* Length of the longest reference system name the warp writes, EPSG: and a
 * signed 32 bit code */
#define MAX_SRS_LEN 32

/**
 * @brief Iterator callback keeping a pixel of the subject where the mask
 * covers it
 * @details The iterator is set with a neighbourhood of a single pixel, so the
 * value of each input stands at @p [0][0]. Raster 0 is the band being clipped
 * and raster 1 is the mask the geometry is burnt into: a pixel the mask does
 * not cover, and a pixel the subject states as nodata, alike answer nodata
 */
static int
raster_clip_callback(rt_iterator_arg arg, void *userarg __attribute__((unused)),
  double *value, int *nodata)
{
  /* The mask covers the pixel only where it carries a value of its own */
  if (arg->nodata[1][0][0] || arg->values[1][0][0] == 0.0)
  {
    *value = 0.0;
    *nodata = 1;
    return 1;
  }
  /* The subject decides the pixel wherever the mask covers it */
  if (arg->nodata[0][0][0])
  {
    *value = 0.0;
    *nodata = 1;
    return 1;
  }
  *value = arg->values[0][0][0];
  *nodata = 0;
  return 1;
}

/**
 * @brief Return the mask a geometry burns into the grid of a raster
 * @details The mask carries one band of a single covering value on the grid
 * the subject states, so that every pixel of the two answers the same cell.
 * The rasterizer accepts a null spatial reference system and leaves the
 * projection of its result unset, which is what the mask needs: it is read
 * against a raster it already shares a grid with, and never on its own
 * @param[in] raster Deserialized subject stating the grid
 * @param[in] gs Geometry to burn
 * @errval NULL
 */
static rt_raster
raster_geo_mask(rt_raster raster, const GSERIALIZED *gs)
{
  LWGEOM *geom = lwgeom_from_gserialized((GSERIALIZED *) gs);
  if (! geom)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not read the geometry to clip with");
    return NULL;
  }
  /* The rasterizer reads plain OGC WKB, which carries no SRID of its own */
  lwvarlena_t *wkb = lwgeom_to_wkb_varlena(geom, WKB_SFSQL);
  lwgeom_free(geom);
  if (! wkb)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not read the geometry to clip with");
    return NULL;
  }

  /* One band of an unsigned byte, initially empty, carrying 1 where the
   * geometry covers a pixel */
  rt_pixtype pixtype = PT_8BUI;
  double init = 0.0;
  double value = 1.0;
  double nodata = 0.0;
  uint8_t hasnodata = 1;
  double scale_x = rt_raster_get_x_scale(raster);
  double scale_y = rt_raster_get_y_scale(raster);
  double ul_x = rt_raster_get_x_offset(raster);
  double ul_y = rt_raster_get_y_offset(raster);
  double skew_x = rt_raster_get_x_skew(raster);
  double skew_y = rt_raster_get_y_skew(raster);

  rt_raster result = rt_raster_gdal_rasterize(
    (const unsigned char *) wkb->data, (uint32_t) (LWSIZE_GET(wkb->size) -
      LWVARHDRSZ), NULL, 1, &pixtype, &init, &value, &nodata, &hasnodata,
    NULL, NULL, &scale_x, &scale_y, &ul_x, &ul_y, NULL, NULL, &skew_x,
    &skew_y, NULL);
  lwfree(wkb);
  if (! result)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not burn the geometry into the grid of the raster");
    return NULL;
  }
  /* The mask shares the grid of the subject, so it shares its reference
   * system as well */
  rt_raster_set_srid(result, rt_raster_get_srid(raster));
  return result;
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Return a raster keeping the pixels of another that a geometry covers
 * @details Every band of the subject is read in turn against a mask the
 * geometry is burnt into. A pixel the geometry does not cover answers nodata.
 * When @p crop is true the result carries the extent the two share, and
 * otherwise it carries the extent of the subject
 * @param[in] rast Raster to clip
 * @param[in] gs Geometry to clip it to
 * @param[in] crop True to reduce the result to the extent the raster and the
 * geometry share
 * @errval NULL
 * @csqlfn None, the host answers this operation on its own raster type
 */
Raster *
raster_clip(const Raster *rast, const GSERIALIZED *gs, bool crop)
{
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(gs, NULL);

  /* A raster and a geometry of different reference systems state positions
   * that cannot be compared */
  int32_t srid = raster_srid(rast);
  if (srid != gserialized_get_srid(gs))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "Operation on mixed SRID");
    return NULL;
  }

  /* The bands are read, so the subject is deserialized in full */
  rt_raster raster = rt_raster_deserialize((void *) rast, 0);
  if (! raster)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }
  int numbands = (int) rt_raster_get_num_bands(raster);
  if (numbands < 1)
  {
    raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The raster carries no band to clip");
    return NULL;
  }

  rt_raster mask = raster_geo_mask(raster, gs);
  if (! mask)
  {
    raster_destroy(raster);
    return NULL;
  }

  /* The result states the extent the two share, or that of the subject */
  rt_extenttype extent = crop ? ET_INTERSECTION : ET_FIRST;
  rt_raster result = NULL;
  for (int i = 0; i < numbands; i++)
  {
    rt_band band = rt_raster_get_band(raster, (uint32_t) i);
    if (! band)
    {
      raster_destroy(mask); raster_destroy(raster);
      if (result) raster_destroy(result);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Could not read band %d of the raster", i + 1);
      return NULL;
    }
    /* A band keeps its own pixel type and nodata value across the clip */
    rt_pixtype pixtype = rt_band_get_pixtype(band);
    int hasnodata = rt_band_get_hasnodata_flag(band);
    double nodataval = 0.0;
    if (hasnodata)
      rt_band_get_nodata(band, &nodataval);

    struct rt_iterator_t itrset[2];
    itrset[0].raster = raster;
    itrset[0].nband = (uint16_t) i;
    itrset[0].nbnodata = 1;
    itrset[1].raster = mask;
    itrset[1].nband = 0;
    itrset[1].nbnodata = 1;

    rt_raster banded = NULL;
    if (rt_raster_iterator(itrset, 2, extent, NULL, pixtype,
        (uint8_t) hasnodata, nodataval, 0, 0, NULL, NULL,
        raster_clip_callback, &banded) != ES_NONE || ! banded)
    {
      raster_destroy(mask); raster_destroy(raster);
      if (result) raster_destroy(result);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Could not clip band %d of the raster", i + 1);
      return NULL;
    }

    if (! result)
      /* The first band states the grid every later one is added to */
      result = banded;
    else
    {
      rt_band addband = rt_raster_get_band(banded, 0);
      if (! addband || rt_raster_add_band(result, addband, i) < 0)
      {
        raster_destroy(banded); raster_destroy(mask); raster_destroy(raster);
        raster_destroy(result);
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "Could not add band %d to the clipped raster", i + 1);
        return NULL;
      }
      /* The band is held by the result now, so only its carrier is released */
      rt_raster_destroy(banded);
    }
  }

  raster_destroy(mask);
  raster_destroy(raster);
  rt_raster_set_srid(result, srid);
  return raster_serialize_destroy(result);
}

/**
 * @brief Return the resampling algorithm a name states
 * @details The rt_core mapper answers nearest neighbour for a name it does not
 * know, which turns a misspelt algorithm into a silent resampling by another
 * one. The name is therefore tested against the set first and an unknown one
 * raises, since a resampling is an answer and not a preference
 * @param[in] algorithm Name of the algorithm, read without regard to case
 * @param[out] alg The algorithm the name states
 * @return True where the name states one of the algorithms
 */
static bool
raster_resample_alg(const char *algorithm, GDALResampleAlg *alg)
{
  static const char *names[] = {"NEARESTNEIGHBOUR", "NEARESTNEIGHBOR",
    "BILINEAR", "CUBIC", "CUBICSPLINE", "LANCZOS", "MAX", "MIN"};
  size_t len = strlen(algorithm);
  char *name = palloc(len + 1);
  for (size_t i = 0; i < len; i++)
    name[i] = (char) toupper((unsigned char) algorithm[i]);
  name[len] = '\0';

  bool found = false;
  for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++)
  {
    if (strcmp(name, names[i]) == 0)
    {
      found = true;
      break;
    }
  }
  if (found)
    *alg = rt_util_gdal_resample_alg(name);
  pfree(name);
  return found;
}

/**
 * @brief Return the spatial reference system of an SRID as a string GDAL reads
 * @details The system is named by its authority and code rather than described
 * by a projection string, which is the first of the three forms PostGIS itself
 * offers GDAL for a raster and the one rt_core writes in its own calls. GDAL
 * resolves it against the PROJ database the raster family already requires, so
 * no catalog of this build's own is read and the answer is the same whether
 * MEOS stands alone or runs inside PostgreSQL.
 *
 * A code GDAL cannot resolve is refused by the caller rather than guessed at:
 * a raster carried into a reference system that is not the one asked for is a
 * wrong answer, and an error is not
 * @param[in] srid Spatial reference system identifier
 * @errval NULL
 * @note The string is the caller's to release with #pfree()
 */
static char *
raster_srs_text(int32_t srid)
{
  char *result = palloc(MAX_SRS_LEN);
  snprintf(result, MAX_SRS_LEN, "EPSG:%d", srid);
  if (! rt_util_gdal_supported_sr(result))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "GDAL cannot resolve the spatial reference system EPSG:%d", srid);
    pfree(result);
    return NULL;
  }
  return result;
}

/**
 * @brief Return a raster warped into another reference system, another pixel
 * size, or both
 * @details The two are one operation: the warp states the result in the target
 * system and samples the subject into the grid the scale asks for, so a call
 * asking only for a system keeps the pixel size the reprojection implies, and
 * a call asking only for a scale regrids the subject where it stands. A target
 * equal to the subject's own system is not a reprojection, and the reference
 * strings are then left unstated, which is what rt_core reads as a regridding
 * @param[in] rast Raster to warp
 * @param[in] srid Target reference system, or the subject's own to keep it
 * @param[in] scale_x,scale_y Pixel size in the units of the target system, or
 * NULL to let the warp choose it
 * @param[in] algorithm Name of the resampling algorithm
 * @param[in] max_err Error in input pixels the warp may commit, 0 for none
 * @errval NULL
 */
static Raster *
raster_warp(const Raster *rast, int32_t srid, double *scale_x, double *scale_y,
  const char *algorithm, double max_err)
{
  GDALResampleAlg alg = GRA_NearestNeighbour;
  if (algorithm && ! raster_resample_alg(algorithm, &alg))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Unknown resampling algorithm: %s", algorithm);
    return NULL;
  }
  if (max_err < 0.0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The error a warp may commit cannot be negative: %f", max_err);
    return NULL;
  }

  int32_t src_srid = raster_srid(rast);
  /* A subject standing in no reference system cannot be carried into one */
  if (src_srid == SRID_UNKNOWN && srid != src_srid)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The raster states no SRID to reproject from");
    return NULL;
  }

  rt_raster raster = rt_raster_deserialize((void *) rast, 0);
  if (! raster)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }

  /* The reference systems are read only where the warp reprojects: a warp
   * within one system states neither, which is how rt_core is told that the
   * grid alone moves */
  char *src_srs = NULL, *dst_srs = NULL;
  if (srid != src_srid)
  {
    src_srs = raster_srs_text(src_srid);
    if (! src_srs)
    {
      raster_destroy(raster);
      return NULL;
    }
    dst_srs = raster_srs_text(srid);
    if (! dst_srs)
    {
      pfree(src_srs); raster_destroy(raster);
      return NULL;
    }
  }

  rt_raster result = rt_raster_gdal_warp(raster, src_srs, dst_srs, scale_x,
    scale_y, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, alg, max_err);
  if (src_srs) pfree(src_srs);
  if (dst_srs) pfree(dst_srs);
  raster_destroy(raster);
  if (! result)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "Could not warp the raster");
    return NULL;
  }
  rt_raster_set_srid(result, srid);
  return raster_serialize_destroy(result);
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Return a raster stated in another spatial reference system
 * @details Every band is carried into the target system and resampled onto the
 * grid the reprojection implies, so the result states the same coverage read
 * through another system rather than the same pixels relabelled
 * @param[in] rast Raster to reproject
 * @param[in] srid Target spatial reference system identifier
 * @param[in] algorithm Name of the resampling algorithm, NULL for nearest
 * neighbour; one of NearestNeighbour, Bilinear, Cubic, CubicSpline, Lanczos,
 * Max and Min, read without regard to case
 * @param[in] max_err Error in input pixels the warp may commit, 0 for an exact
 * calculation
 * @errval NULL
 * @csqlfn None, the host answers this operation on its own raster type
 */
Raster *
raster_transform(const Raster *rast, int32_t srid, const char *algorithm,
  double max_err)
{
  VALIDATE_NOT_NULL(rast, NULL);
  if (srid == SRID_UNKNOWN)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The target of a reprojection cannot be an unknown SRID");
    return NULL;
  }
  return raster_warp(rast, srid, NULL, NULL, algorithm, max_err);
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Return a raster resampled to another pixel size
 * @details The result keeps the reference system and the upper left corner of
 * the subject and states its coverage on a grid of the pixel size asked for,
 * so a coarser scale reads fewer pixels over the same ground
 * @param[in] rast Raster to resample
 * @param[in] scale_x,scale_y Pixel size in the units of the raster's own
 * reference system, the Y component being negative for a north-up grid as the
 * geotransform states it
 * @param[in] algorithm Name of the resampling algorithm, NULL for nearest
 * neighbour; one of NearestNeighbour, Bilinear, Cubic, CubicSpline, Lanczos,
 * Max and Min, read without regard to case
 * @param[in] max_err Error in input pixels the warp may commit, 0 for an exact
 * calculation
 * @errval NULL
 * @csqlfn None, the host answers this operation on its own raster type
 */
Raster *
raster_rescale(const Raster *rast, double scale_x, double scale_y,
  const char *algorithm, double max_err)
{
  VALIDATE_NOT_NULL(rast, NULL);
  /* A pixel of no width or no height covers no ground, so the grid it would
   * state has no cell to carry a value */
  if (scale_x == 0.0 || scale_y == 0.0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The pixel size of a rescaled raster cannot be zero");
    return NULL;
  }
  return raster_warp(rast, raster_srid(rast), &scale_x, &scale_y, algorithm,
    max_err);
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Release an array of polygons and the values they carry
 * @param[in] gvarr Array answered by #raster_dump_as_polygons()
 * @param[in] count Number of elements it holds
 * @note Each polygon is released with it, so a caller keeping one copies it
 * first
 */
void
geomval_arr_free(GeomVal *gvarr, int count)
{
  if (! gvarr)
    return;
  for (int i = 0; i < count; i++)
    if (gvarr[i].geom)
      pfree(gvarr[i].geom);
  pfree(gvarr);
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Return the polygons of a raster band, one for each group of pixels
 * carrying the same value
 * @details A band states a value per pixel; this states the same band as the
 * regions those values cover, so a coverage becomes geometry a spatial
 * operation can read. Each polygon carries the reference system of the raster,
 * as every spatial value this family derives from a raster does, so it can be
 * compared with the trajectories the coverage is read along
 * @param[in] rast Raster to read
 * @param[in] band Number of the band, starting at 1
 * @param[in] exclude_nodata True to leave the pixels the band states as nodata
 * out of the answer, false to give them a region of their own
 * @param[out] count Number of polygons answered
 * @return An array the caller releases with #geomval_arr_free(), or NULL where
 * the band covers nothing, in which case @p count is 0 and no error is stated
 * @errval NULL
 * @csqlfn None, the host answers this operation on its own raster type
 */
GeomVal *
raster_dump_as_polygons(const Raster *rast, int band, bool exclude_nodata,
  int *count)
{
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(count, NULL);
  *count = 0;

  /* The bands are read, so the subject is deserialized in full */
  rt_raster raster = rt_raster_deserialize((void *) rast, 0);
  if (! raster)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }

  /* A band the raster does not have is an error here, as it is for every other
   * accessor of this family, rather than the empty set PostGIS answers */
  int numbands = (int) rt_raster_get_num_bands(raster);
  if (band < 1 || band > numbands)
  {
    raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The raster has no band %d, it has %d", band, numbands);
    return NULL;
  }

  int32_t srid = (int32_t) rt_raster_get_srid(raster);

  /* A band that is entirely nodata covers nothing, which is an answer */
  rt_band rtband = rt_raster_get_band(raster, (uint32_t) (band - 1));
  if (rtband && rt_band_get_isnodata_flag(rtband))
  {
    raster_destroy(raster);
    return NULL;
  }

  int nelems = 0;
  rt_geomval geomval = rt_raster_gdal_polygonize(raster, band - 1,
    exclude_nodata ? 1 : 0, &nelems);
  raster_destroy(raster);
  if (! geomval)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not read band %d of the raster as polygons", band);
    return NULL;
  }
  if (nelems < 1)
  {
    pfree(geomval);
    return NULL;
  }

  /* Each polygon arrives as plain WKB and states no reference system of its
   * own, so it is given the one the raster states, which is the system its
   * coordinates are already in */
  GeomVal *result = palloc0(sizeof(GeomVal) * (size_t) nelems);
  for (int i = 0; i < nelems; i++)
  {
    LWGEOM *geom = lwpoly_as_lwgeom(geomval[i].geom);
    lwgeom_set_srid(geom, srid);
    result[i].geom = geo_serialize(geom);
    result[i].val = geomval[i].val;
    lwgeom_free(geom);
  }
  pfree(geomval);

  *count = nelems;
  return result;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return what the pixels of a raster band amount to
 * @details The band is read ONCE and answers one value carrying every
 * statistic, rather than a function per statistic each of which would rescan
 * it. Every pixel is read: rt_core samples a band only when asked for a
 * fraction below one, and this asks for all of them, so the answer is exact
 * rather than estimated
 * @param[in] rast Raster to read
 * @param[in] band Number of the band, starting at 1
 * @param[in] exclude_nodata True to leave the pixels the band states as nodata
 * out of the statistics, false to count them as the values they hold
 * @return The statistics, which the caller releases with @p free(), or NULL
 * where the band states no pixel to count, in which case no error is stated
 * @errval NULL
 * @csqlfn None, the host answers this operation on its own raster type
 */
BandStats *
raster_summary_stats(const Raster *rast, int band, bool exclude_nodata)
{
  VALIDATE_NOT_NULL(rast, NULL);

  /* The band values are read, so the subject is deserialized in full */
  rt_raster raster = rt_raster_deserialize((void *) rast, 0);
  if (! raster)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }

  /* A band the raster does not have is an error here, as it is for every
   * other accessor of this family */
  int numbands = (int) rt_raster_get_num_bands(raster);
  if (band < 1 || band > numbands)
  {
    raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The raster has no band %d, it has %d", band, numbands);
    return NULL;
  }

  rt_band rtband = rt_raster_get_band(raster, (uint32_t) (band - 1));
  if (! rtband)
  {
    raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not read band %d of the raster", band);
    return NULL;
  }

  /* A sample of one is every pixel: rt_core reads the whole band unless it is
   * asked for a fraction strictly between zero and one. The values themselves
   * are not wanted, only what they amount to, and no coverage is accumulated
   * across rasters, so the three coverage accumulators are unstated */
  rt_bandstats stats = rt_band_get_summary_stats(rtband,
    exclude_nodata ? 1 : 0, 1.0, 0, NULL, NULL, NULL);
  raster_destroy(raster);
  if (! stats)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not summarize band %d of the raster", band);
    return NULL;
  }

  /* A band whose every pixel is nodata states no value to summarize, which is
   * an answer rather than an error; the statistics of no pixels have no mean */
  if (stats->count < 1)
  {
    pfree(stats);
    return NULL;
  }

  BandStats *result = palloc0(sizeof(BandStats));
  result->count = stats->count;
  result->sum = stats->sum;
  result->mean = stats->mean;
  result->stddev = stats->stddev;
  result->min = stats->min;
  result->max = stats->max;
  pfree(stats);
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
