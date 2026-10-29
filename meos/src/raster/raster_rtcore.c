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
 * @brief General functions for the PostGIS raster type, backed by the vendored
 * rt_core library
 * @details This is the only MEOS source file that includes `librtcore.h`. A
 * `Raster`
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
#include "rt_serialize.h"

/* C */
#include <ctype.h>
#include <string.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include <meos_raster.h>
#include "geo/geo_funcs.h"
#include "raster/raquet.h"
#include "raster/raster_quadbin.h"
#include "temporal/temporal.h"
#include "temporal/tnumber_mathfuncs.h"

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
 * @param[in] raster Raster to serialize and destroy @nullable
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
  void *serialized = rt_raster_serialize(raster);
  raster_destroy(raster);
  if (! serialized)
  {
    meos_error(ERROR, MEOS_ERR_WKB_INPUT, "Could not serialize raster");
    return NULL;
  }
  /* rt_core allocates through its own allocator, which is not the one MEOS
   * answers from, so the caller receives a copy it frees as every other MEOS
   * value, as #raster_wkb copies the bytes rt_core writes */
  size_t size = ((struct rt_raster_serialized_t *) serialized)->size;
  Raster *result = palloc(size);
  memcpy(result, serialized, size);
  rtdealloc(serialized);
  /* The serialized form opens with its own length, which is where the varlena
   * header stands, and rt_core writes that length as a plain number. A reader
   * that takes it as a varlena header -- which is every reader on the
   * PostgreSQL side -- needs it encoded as one, so it is stamped here, where
   * the form is produced, rather than by each caller. Without it a raster this
   * function answers reads as a band of no width and no height. */
  SET_VARSIZE(result, size);
  return result;
}

/**
 * @ingroup meos_raster_base_inout
 * @brief Return a raster from its Well-Known Binary (WKB) representation
 * @param[in] wkb WKB string
 * @param[in] size Size of the string
 * @errval NULL
 * @csqlfn #Raster_from_wkb()
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
 * @csqlfn #Raster_from_hexwkb()
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
 * @brief Return true when a Well-Known Binary variant asks for the
 * little-endian byte order
 * @details The order of the machine is taken when the variant names neither
 * order or both, as #datum_as_wkb() reads it.
 * @param[in] variant Output variant
 */
static bool
raster_wkb_little_endian(uint8_t variant)
{
  bool ndr = (variant & WKB_NDR) != 0;
  bool xdr = (variant & WKB_XDR) != 0;
  if (ndr != xdr)
    return ndr;
  return ! MEOS_IS_BIG_ENDIAN;
}

/**
 * @brief Reverse in place the bytes of a value
 * @param[in,out] ptr First byte of the value
 * @param[in] size Number of bytes of the value
 */
static void
raster_wkb_flip(uint8_t *ptr, size_t size)
{
  for (size_t i = 0, j = size - 1; i < j; i++, j--)
  {
    uint8_t b = ptr[i];
    ptr[i] = ptr[j];
    ptr[j] = b;
  }
}

/**
 * @brief Return the unsigned 16-bit integer at a position of a Well-Known
 * Binary representation, read in the byte order the representation states
 * @param[in] ptr First byte of the integer
 * @param[in] little True when the representation is little-endian
 */
static uint16_t
raster_wkb_uint16(const uint8_t *ptr, bool little)
{
  return little ? (uint16_t) (ptr[0] | (ptr[1] << 8)) :
    (uint16_t) ((ptr[0] << 8) | ptr[1]);
}

/**
 * @brief Rewrite in place the Well-Known Binary representation of a raster
 * from the byte order it states into the other one
 * @details The layout is the one rt_raster_to_wkb() writes: the byte order,
 * a header of fixed-width fields, and for each band a byte stating its pixel
 * type and flags, a nodata value as wide as a pixel, and either its pixels or,
 * for a band stored outside the database, a band number and a path, which are
 * bytes rather than numbers
 * @param[in,out] wkb Well-Known Binary representation
 * @param[in] size Size of the representation
 * @return False when the representation ends before the layout it states
 */
static bool
raster_wkb_swap(uint8_t *wkb, size_t size)
{
  /* Byte order, version, number of bands, the six numbers of the
   * geotransform, SRID, width and height */
  static const size_t header[] = {1, 2, 2, 8, 8, 8, 8, 8, 8, 4, 2, 2};
  const size_t nheader = sizeof(header) / sizeof(header[0]);
  size_t hsize = 0;
  for (size_t i = 0; i < nheader; i++)
    hsize += header[i];
  if (size < hsize)
    return false;
  bool little = (wkb[0] != 0);
  uint16_t nbands = raster_wkb_uint16(wkb + 3, little);
  uint16_t width = raster_wkb_uint16(wkb + hsize - 4, little);
  uint16_t height = raster_wkb_uint16(wkb + hsize - 2, little);
  wkb[0] = little ? 0 : 1;
  size_t pos = header[0];
  for (size_t i = 1; i < nheader; i++)
  {
    raster_wkb_flip(wkb + pos, header[i]);
    pos += header[i];
  }
  size_t npixels = (size_t) width * height;
  for (uint16_t b = 0; b < nbands; b++)
  {
    if (pos >= size)
      return false;
    uint8_t flags = wkb[pos++];
    int pixbytes = rt_pixtype_size((rt_pixtype) BANDTYPE_PIXTYPE(flags));
    if (pixbytes < 1 || size - pos < (size_t) pixbytes)
      return false;
    raster_wkb_flip(wkb + pos, (size_t) pixbytes);
    pos += (size_t) pixbytes;
    if (BANDTYPE_IS_OFFDB(flags))
    {
      /* The band number is one byte and the path a null-terminated string */
      if (pos >= size)
        return false;
      pos++;
      const uint8_t *nul = memchr(wkb + pos, '\0', size - pos);
      if (! nul)
        return false;
      pos = (size_t) (nul - wkb) + 1;
    }
    else
    {
      if ((size - pos) / (size_t) pixbytes < npixels)
        return false;
      if (pixbytes > 1)
      {
        for (size_t k = 0; k < npixels; k++)
          raster_wkb_flip(wkb + pos + k * (size_t) pixbytes, (size_t) pixbytes);
      }
      pos += npixels * (size_t) pixbytes;
    }
  }
  return pos == size;
}

/**
 * @brief Return the Well-Known Binary representation of a raster in the byte
 * order a variant asks for
 * @param[in] rast Raster
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @errval NULL
 */
static uint8_t *
raster_wkb(const Raster *rast, uint8_t variant, size_t *size_out)
{
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
  uint8_t *wkb = rt_raster_to_wkb(raster, 0, &wkb_size);
  raster_destroy(raster);
  if (! wkb)
  {
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Could not output the Well-Known Binary (WKB) representation of a "
      "raster");
    return NULL;
  }
  /* rt_core writes the byte order of the machine in memory it allocates
   * itself, so the bytes move to memory the caller releases as it releases
   * any other result, in the byte order asked for */
  uint8_t *result = palloc(wkb_size);
  memcpy(result, wkb, wkb_size);
  rtdealloc(wkb);
  if (raster_wkb_little_endian(variant) != (result[0] != 0) &&
      ! raster_wkb_swap(result, (size_t) wkb_size))
  {
    pfree(result);
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Could not output the Well-Known Binary (WKB) representation of a "
      "raster in the byte order asked for");
    return NULL;
  }
  *size_out = (size_t) wkb_size;
  return result;
}

/**
 * @ingroup meos_raster_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a raster
 * @param[in] rast Raster
 * @param[in] variant Output variant, whose WKB_NDR or WKB_XDR flag names the
 * byte order, the order of the machine when it names neither
 * @param[out] size_out Size of the output
 * @errval NULL
 * @csqlfn #Raster_as_wkb()
 */
uint8_t *
raster_as_wkb(const Raster *rast, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return raster_wkb(rast, variant, size_out);
}

/**
 * @ingroup meos_raster_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a raster
 * @param[in] rast Raster
 * @param[in] variant Output variant, whose WKB_NDR or WKB_XDR flag names the
 * byte order, the order of the machine when it names neither
 * @param[out] size_out Size of the output, not counting the null terminator
 * @errval NULL
 * @csqlfn #Raster_as_hexwkb()
 */
char *
raster_as_hexwkb(const Raster *rast, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  size_t wkb_size;
  uint8_t *wkb = raster_wkb(rast, variant, &wkb_size);
  if (! wkb)
    return NULL;
  static const char hexchr[] = "0123456789ABCDEF";
  char *result = palloc(2 * wkb_size + 1);
  for (size_t i = 0; i < wkb_size; i++)
  {
    result[2 * i] = hexchr[wkb[i] >> 4];
    result[2 * i + 1] = hexchr[wkb[i] & 0x0F];
  }
  result[2 * wkb_size] = '\0';
  pfree(wkb);
  *size_out = 2 * wkb_size;
  return result;
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_raster_base_constructor
 * @brief Return a copy of a raster
 * @param[in] rast Raster
 */
Raster *
raster_copy(const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL);
  Raster *result = palloc(VARSIZE(rast));
  memcpy(result, rast, VARSIZE(rast));
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the size in bytes of a raster
 * @details The size is the number of bytes of the flat memory image of the
 * value, which #raster_copy duplicates
 * @param[in] rast Raster
 * @errval INT_MAX
 */
int
raster_mem_size(const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, INT_MAX);
  return (int) VARSIZE(rast);
}

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
 * @csqlfn #Raster_width()
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
 * @csqlfn #Raster_height()
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
 * @csqlfn #Raster_srid()
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
 * @csqlfn #Raster_upper_left_x()
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
 * @csqlfn #Raster_upper_left_y()
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
 * @csqlfn #Raster_scale_x()
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
 * @csqlfn #Raster_scale_y()
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
 * @csqlfn #Raster_skew_x()
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
 * @csqlfn #Raster_skew_y()
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
 * what #raquet_band_pixel_type() answers for a tile, so a band and a tile of
 * the same type report the same name. The PostGIS spelling is read through
 * #raquet_pixtype_from_string(), so the one catalog states both vocabularies
 * and `1BB`, `2BUI` and `4BUI` report the `uint8` their bytes already are
 * @csqlfn #Raster_band_pixel_type()
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
 * @csqlfn #Raster_band_has_nodata_value()
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
 * @brief Return in the last argument the nodata value of a raster band
 * @param[in] rast Raster
 * @param[in] band Band number (1-based)
 * @param[out] result Result
 * @return True when the value is written, false when the band states no
 * nodata value
 * @errval false
 * @csqlfn #Raster_band_nodata_value()
 */
bool
raster_band_nodata_value(const Raster *rast, int band, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, false); VALIDATE_NOT_NULL(result, false);
  rt_raster raster;
  rt_band rtband = raster_band_of(rast, band, &raster);
  if (! rtband)
    return false;
  /* rt_band_get_nodata() reports an error for a band stating no nodata value,
   * which is an answer here rather than an error, so the flag is read first */
  bool found = (rt_band_get_hasnodata_flag(rtband) != 0);
  if (found)
    rt_band_get_nodata(rtband, result);
  raster_destroy(raster);
  return found;
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
 * @param[in] touched True to burn every pixel the geometry touches, false to
 * burn the pixels whose centre it covers
 * @errval NULL
 */
static rt_raster
raster_geo_mask(rt_raster raster, const GSERIALIZED *gs, bool touched)
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
  /* The touched pixels are the one option the rasterizer reads, as the
   * PostGIS RASTER_clip hands it over */
  char all_touched[] = "ALL_TOUCHED=TRUE";
  char *options[] = {all_touched, NULL};

  rt_raster result = rt_raster_gdal_rasterize(
    (const unsigned char *) wkb->data, (uint32_t) (LWSIZE_GET(wkb->size) -
      LWVARHDRSZ), NULL, 1, &pixtype, &init, &value, &nodata, &hasnodata,
    NULL, NULL, &scale_x, &scale_y, &ul_x, &ul_y, NULL, NULL, &skew_x,
    &skew_y, touched ? options : NULL);
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
 * @details The bands kept are read in turn against a mask the geometry is
 * burnt into, as the PostGIS `ST_Clip` reads them. A pixel the geometry does
 * not cover answers the nodata value of its band: the value @p nodata states,
 * one for every band or one per band with the last standing for the bands
 * after it, or otherwise the nodata value of the band, and the smallest value
 * its pixel type holds where the band states none. When @p crop is true the
 * result carries the extent the two share, and otherwise it carries the
 * extent of the subject
 * @param[in] rast Raster to clip
 * @param[in] bands Numbers of the bands to keep, starting at 1, in the order
 * of the result, or NULL for every band @nullable
 * @param[in] nbands Number of elements of @p bands
 * @param[in] gs Geometry to clip it to
 * @param[in] nodata Nodata values of the bands of the result, or NULL for the
 * nodata value of each band @nullable
 * @param[in] nnodata Number of elements of @p nodata
 * @param[in] crop True to reduce the result to the extent the raster and the
 * geometry share
 * @param[in] touched True to keep every pixel the geometry touches, false to
 * keep the pixels whose centre it covers
 * @errval NULL
 * @csqlfn #Raster_clip(), #Raster_clip_bands()
 */
Raster *
raster_clip(const Raster *rast, const int *bands, int nbands,
  const GSERIALIZED *gs, const double *nodata, int nnodata, bool crop,
  bool touched)
{
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if ((bands && nbands < 1) || (nodata && nnodata < 1))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "An array of bands or of nodata values cannot be empty");
    return NULL;
  }

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
  /* The bands named are checked before any of them is read */
  for (int i = 0; bands && i < nbands; i++)
  {
    if (bands[i] < 1 || bands[i] > numbands)
    {
      raster_destroy(raster);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Raster has no band %d, it has %d", bands[i], numbands);
      return NULL;
    }
  }
  int nresult = bands ? nbands : numbands;

  rt_raster mask = raster_geo_mask(raster, gs, touched);
  if (! mask)
  {
    raster_destroy(raster);
    return NULL;
  }

  /* The result states the extent the two share, or that of the subject */
  rt_extenttype extent = crop ? ET_INTERSECTION : ET_FIRST;
  rt_raster result = NULL;
  for (int i = 0; i < nresult; i++)
  {
    int b = bands ? bands[i] - 1 : i;
    rt_band band = rt_raster_get_band(raster, (uint32_t) b);
    if (! band)
    {
      raster_destroy(mask); raster_destroy(raster);
      if (result) raster_destroy(result);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Could not read band %d of the raster", b + 1);
      return NULL;
    }
    /* A band keeps its own pixel type across the clip, and its own nodata
     * value unless the caller states another */
    rt_pixtype pixtype = rt_band_get_pixtype(band);
    int hasnodata;
    double nodataval = 0.0;
    if (nodata)
    {
      hasnodata = 1;
      nodataval = nodata[i < nnodata ? i : nnodata - 1];
    }
    else
    {
      hasnodata = rt_band_get_hasnodata_flag(band);
      if (hasnodata)
        rt_band_get_nodata(band, &nodataval);
    }

    struct rt_iterator_t itrset[2];
    itrset[0].raster = raster;
    itrset[0].nband = (uint16_t) b;
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
        "Could not clip band %d of the raster", b + 1);
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
          "Could not add band %d to the clipped raster", b + 1);
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
 * strings are then left unstated, which is what rt_core reads as a regridding.
 * A coordinate operation states the transformation between the two systems,
 * which the warp then applies instead of the one GDAL derives from them, so a
 * warp given one states the system of the subject always and the target
 * system when it is known
 * @param[in] rast Raster to warp
 * @param[in] srid Target reference system, or the subject's own to keep it
 * @param[in] coord_op Coordinate operation from the system of the subject to
 * the target system, as a PROJ string, or NULL to let GDAL derive it @nullable
 * @param[in] scale_x,scale_y Pixel size in the units of the target system, or
 * NULL to let the warp choose it @nullable
 * @param[in] grid_x,grid_y Point of the target system a grid line of the result
 * passes through, or NULL to let the warp place the grid @nullable
 * @param[in] skew_x,skew_y Skew of the result, or NULL for none @nullable
 * @param[in] algorithm Name of the resampling algorithm
 * @param[in] max_err Error in input pixels the warp may commit, 0 for none
 * @errval NULL
 */
static Raster *
raster_warp(const Raster *rast, int32_t srid, const char *coord_op,
  double *scale_x, double *scale_y, double *grid_x, double *grid_y,
  double *skew_x, double *skew_y, const char *algorithm, double max_err)
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
  bool reproject = coord_op || srid != src_srid;
  if (src_srid == SRID_UNKNOWN && reproject)
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
  if (reproject)
  {
    src_srs = raster_srs_text(src_srid);
    if (! src_srs)
    {
      raster_destroy(raster);
      return NULL;
    }
    if (srid != SRID_UNKNOWN)
    {
      dst_srs = raster_srs_text(srid);
      if (! dst_srs)
      {
        pfree(src_srs); raster_destroy(raster);
        return NULL;
      }
    }
  }

  rt_raster result = rt_raster_gdal_warp(raster, src_srs, dst_srs, scale_x,
    scale_y, NULL, NULL, NULL, NULL, grid_x, grid_y, skew_x, skew_y, alg,
    max_err, coord_op);
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
 * through another system rather than the same pixels relabelled. A pixel size
 * stated for the result fixes its grid, as `ST_Transform` reads it
 * @param[in] rast Raster to reproject
 * @param[in] srid Target spatial reference system identifier
 * @param[in] algorithm Name of the resampling algorithm, NULL for nearest
 * neighbour; one of NearestNeighbour, Bilinear, Cubic, CubicSpline, Lanczos,
 * Max and Min, read without regard to case @nullable
 * @param[in] max_err Error in input pixels the warp may commit, 0 for an exact
 * calculation
 * @param[in] scale_x,scale_y Pixel size of the result in the units of the
 * target system, 0 to let the warp derive it
 * @errval NULL
 * @csqlfn #Raster_transform()
 */
Raster *
raster_transform(const Raster *rast, int32_t srid, const char *algorithm,
  double max_err, double scale_x, double scale_y)
{
  VALIDATE_NOT_NULL(rast, NULL);
  if (srid == SRID_UNKNOWN)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The target of a reprojection cannot be an unknown SRID");
    return NULL;
  }
  /* A pixel size of 0 leaves the axis to the warp, as the PostGIS
   * RASTER_GDALWarp reads it */
  return raster_warp(rast, srid, NULL, scale_x != 0.0 ? &scale_x : NULL,
    scale_y != 0.0 ? &scale_y : NULL, NULL, NULL, NULL, NULL, algorithm,
    max_err);
}

/**
 * @brief Return true when the first axis of a spatial reference system is
 * its latitude or its northing
 * @details GDAL hands a coordinate operation its coordinates in the order the
 * axes of the system are stated, which for EPSG:4326 is the latitude first
 * @param[in] srid Spatial reference system identifier
 */
static bool
raster_srid_north_first(int32_t srid)
{
  if (srid == SRID_UNKNOWN)
    return false;
  PJ_CONTEXT *ctx = meos_proj_get_context();
  char code[MAX_SRS_LEN];
  snprintf(code, MAX_SRS_LEN, "EPSG:%d", srid);
  PJ *crs = proj_create(ctx, code);
  if (! crs)
  {
    proj_errno_reset(NULL);
    return false;
  }
  bool result = false;
  PJ *cs = proj_crs_get_coordinate_system(ctx, crs);
  if (cs)
  {
    const char *dir = NULL;
    if (proj_cs_get_axis_info(ctx, cs, 0, NULL, NULL, &dir, NULL, NULL, NULL,
        NULL) && dir)
      result = (strcmp(dir, "north") == 0 || strcmp(dir, "south") == 0);
    proj_destroy(cs);
  }
  proj_destroy(crs);
  return result;
}

/**
 * @brief Return a PROJ operation string wrapped in the conversions between the
 * coordinates GDAL feeds and reads and the ones the operation reads and writes
 * @details GDAL feeds the operation the coordinates of the subject in the
 * order the axes of its system are stated and reads the result the same way,
 * while the operation reads and writes the longitude first, in radians where
 * it states so. PROJ does not nest pipelines, so the steps of an operation
 * that is a pipeline are spliced between the conversions, its global options
 * kept before the first step, and any other operation becomes the one step
 * between them
 * @param[in] str Operation as a PROJ string
 * @param[in] swap_in,swap_out True when the system of the subject, respectively
 * of the result, states its latitude or northing first
 * @param[in] ang_in,ang_out True when the operation reads, respectively writes,
 * angles in radians
 * @note The string is the caller's to release with #pfree()
 */
static char *
raster_coord_op_wrap(const char *str, bool swap_in, bool swap_out,
  bool ang_in, bool ang_out)
{
  static const char *PIPELINE = "+proj=pipeline";
  static const char *SWAP = " +step +proj=axisswap +order=2,1";
  static const char *TO_RAD = " +step +proj=unitconvert +xy_in=deg +xy_out=rad";
  static const char *TO_DEG = " +step +proj=unitconvert +xy_in=rad +xy_out=deg";
  const char *globals = "", *steps = str;
  size_t nglobals = 0;
  bool step_prefix = true;
  if (strncmp(str, PIPELINE, strlen(PIPELINE)) == 0)
  {
    globals = str + strlen(PIPELINE);
    const char *first = strstr(globals, " +step");
    nglobals = first ? (size_t) (first - globals) : strlen(globals);
    steps = globals + nglobals;
    step_prefix = false;
  }
  size_t size = strlen(PIPELINE) + nglobals + strlen(SWAP) * 2 +
    strlen(TO_RAD) + strlen(steps) + strlen(" +step ") + strlen(TO_DEG) + 1;
  char *result = palloc(size);
  snprintf(result, size, "%s%.*s%s%s%s%s%s%s", PIPELINE, (int) nglobals,
    globals, swap_in ? SWAP : "", ang_in ? TO_RAD : "",
    step_prefix ? " +step " : "", steps, ang_out ? TO_DEG : "",
    swap_out ? SWAP : "");
  return result;
}

/**
 * @brief Return the coordinate operation a pipeline states, as a PROJ string
 * GDAL applies in the order of the axes it feeds a warp
 * @details The operation is read as #lwproj_from_str_pipeline reads it for a
 * geometry: a coordinate reference system is not an operation, and the axes
 * are put in the order a geographic information system reads them, longitude
 * first. An inverse pipeline is the inverse operation, which GDAL then applies
 * forward. The operation is wrapped by #raster_coord_op_wrap in the
 * conversions between the coordinates GDAL feeds and reads, in the axis order
 * of each system, and the ones it reads and writes, longitude first and in
 * radians where it states so, as `ptarray_transform` converts the coordinates
 * of a geometry around it, so the raster and the geometry read one pipeline
 * the same way
 * @param[in] pipelinestr Coordinate operation, as a PROJ string, a WKT or an
 * authority code
 * @param[in] is_forward True to apply the operation, false its inverse
 * @param[in] src_srid,dst_srid Spatial reference systems of the subject and
 * of the result
 * @errval NULL
 * @note The string is the caller's to release with #pfree()
 */
static char *
raster_coord_op(const char *pipelinestr, bool is_forward, int32_t src_srid,
  int32_t dst_srid)
{
  PJ_CONTEXT *ctx = meos_proj_get_context();
  PJ *pj = proj_create(ctx, pipelinestr);
  if (! pj)
  {
    proj_errno_reset(NULL);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not parse coordinate operation '%s'", pipelinestr);
    return NULL;
  }
  if (proj_is_crs(pj))
  {
    proj_destroy(pj);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "'%s' is a coordinate reference system, not a coordinate operation",
      pipelinestr);
    return NULL;
  }
  /* Put the axes in the order a geographic information system reads them */
  PJ *op = proj_normalize_for_visualization(ctx, pj);
  if (op)
    proj_destroy(pj);
  else
  {
    proj_errno_reset(pj);
    op = pj;
  }
  if (! is_forward)
  {
    PJ *inv = proj_coordoperation_create_inverse(ctx, op);
    proj_destroy(op);
    if (! inv)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The coordinate operation '%s' has no inverse", pipelinestr);
      return NULL;
    }
    op = inv;
  }
  bool ang_in = proj_angular_input(op, PJ_FWD);
  bool ang_out = proj_angular_output(op, PJ_FWD);
  bool swap_in = raster_srid_north_first(src_srid);
  bool swap_out = raster_srid_north_first(dst_srid);
  bool wrap = ang_in || ang_out || swap_in || swap_out;
  const char *str = proj_as_proj_string(ctx, op, PJ_PROJ_5, NULL);
  char *result = NULL;
  if (str && wrap)
    result = raster_coord_op_wrap(str, swap_in, swap_out, ang_in, ang_out);
  else if (str)
    result = pstrdup(str);
  else if (! wrap)
  {
    /* An operation PROJ cannot write as a string is stated as a WKT */
    str = proj_as_wkt(ctx, op, PJ_WKT2_2019, NULL);
    result = str ? pstrdup(str) : NULL;
  }
  proj_destroy(op);
  if (! result)
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The coordinate operation '%s' cannot be stated to GDAL", pipelinestr);
  return result;
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Return a raster stated in another spatial reference system through a
 * coordinate operation
 * @details Every band is carried through the operation and resampled onto the
 * grid it implies, as #raster_transform carries it through the operation GDAL
 * derives from the two systems, so a pipeline the derivation would choose
 * answers as a reprojection to the same system does
 * @param[in] rast Raster to reproject
 * @param[in] pipelinestr Coordinate operation, as a PROJ string, a WKT or an
 * authority code
 * @param[in] srid Spatial reference system identifier of the result, 0 when
 * the operation leaves it unstated
 * @param[in] is_forward True to apply the operation, false its inverse
 * @param[in] algorithm Name of the resampling algorithm, NULL for nearest
 * neighbour; one of NearestNeighbour, Bilinear, Cubic, CubicSpline, Lanczos,
 * Max and Min, read without regard to case @nullable
 * @param[in] max_err Error in input pixels the warp may commit, 0 for an exact
 * calculation
 * @param[in] scale_x,scale_y Pixel size of the result in the units of the
 * target system, 0 to let the warp derive it
 * @errval NULL
 * @csqlfn #Raster_transform_pipeline()
 */
Raster *
raster_transform_pipeline(const Raster *rast, const char *pipelinestr,
  int32_t srid, bool is_forward, const char *algorithm, double max_err,
  double scale_x, double scale_y)
{
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(pipelinestr, NULL);
  char *coord_op = raster_coord_op(pipelinestr, is_forward, raster_srid(rast),
    srid);
  if (! coord_op)
    return NULL;
  Raster *result = raster_warp(rast, srid, coord_op,
    scale_x != 0.0 ? &scale_x : NULL, scale_y != 0.0 ? &scale_y : NULL, NULL,
    NULL, NULL, NULL, algorithm, max_err);
  pfree(coord_op);
  return result;
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Return a raster stated on the grid of another raster
 * @details The result takes the reference system, the pixel size, the grid
 * origin and the skew of @p alignto, as `ST_Transform` reads them from the
 * raster it aligns to, so every pixel of the result coincides with a pixel of
 * that grid
 * @param[in] rast Raster to reproject
 * @param[in] alignto Raster whose grid the result lies on
 * @param[in] algorithm Name of the resampling algorithm, NULL for nearest
 * neighbour; one of NearestNeighbour, Bilinear, Cubic, CubicSpline, Lanczos,
 * Max and Min, read without regard to case @nullable
 * @param[in] max_err Error in input pixels the warp may commit, 0 for an exact
 * calculation
 * @errval NULL
 * @csqlfn #Raster_transform_raster()
 */
Raster *
raster_transform_raster(const Raster *rast, const Raster *alignto,
  const char *algorithm, double max_err)
{
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(alignto, NULL);
  int32_t srid = raster_srid(alignto);
  if (srid == SRID_UNKNOWN)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The target of a reprojection cannot be an unknown SRID");
    return NULL;
  }
  double scale_x = raster_scale_x(alignto);
  double scale_y = raster_scale_y(alignto);
  double grid_x = raster_upper_left_x(alignto);
  double grid_y = raster_upper_left_y(alignto);
  double skew_x = raster_skew_x(alignto);
  double skew_y = raster_skew_y(alignto);
  /* A skew of 0 states no skew, as the PostGIS RASTER_GDALWarp reads it */
  return raster_warp(rast, srid, NULL, &scale_x, &scale_y, &grid_x, &grid_y,
    skew_x != 0.0 ? &skew_x : NULL, skew_y != 0.0 ? &skew_y : NULL,
    algorithm, max_err);
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
 * Max and Min, read without regard to case @nullable
 * @param[in] max_err Error in input pixels the warp may commit, 0 for an exact
 * calculation
 * @errval NULL
 * @csqlfn #Raster_rescale()
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
  return raster_warp(rast, raster_srid(rast), NULL, &scale_x, &scale_y, NULL,
    NULL, NULL, NULL, algorithm, max_err);
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Release an array of polygons and the values they carry
 * @param[in] gvarr Array answered by #raster_dump_as_polygons() @nullable
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
 * @csqlfn #Raster_dump_as_polygons()
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
  /* The raster core allocates the array with its own allocator, not the one
   * MEOS answers from, so it is released through the raster core as well */
  if (nelems < 1)
  {
    rtdealloc(geomval);
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
  rtdealloc(geomval);

  *count = nelems;
  return result;
}

/**
 * @brief Read a band of a raster, the raster deserialized in full
 * @details The band the accessors of this family read: one the raster does
 * not have is an error, as it is for #raster_dump_as_polygons, rather than
 * the empty set PostGIS answers
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @param[out] raster Raster deserialized, which the caller destroys
 * @return The band
 * @errval NULL
 */
static rt_band
raster_read_band(const Raster *rast, int band, rt_raster *raster)
{
  *raster = rt_raster_deserialize((void *) rast, 0);
  if (! *raster)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }
  int numbands = (int) rt_raster_get_num_bands(*raster);
  rt_band result = (band >= 1 && band <= numbands) ?
    rt_raster_get_band(*raster, (uint32_t) (band - 1)) : NULL;
  if (! result)
  {
    raster_destroy(*raster);
    *raster = NULL;
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The raster has no band %d, it has %d", band, numbands);
  }
  return result;
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Free an array of pixels
 * @param[in] pixarr Array of pixels @nullable
 * @param[in] count Number of elements in the array
 */
void
pixel_arr_free(Pixel *pixarr, int count)
{
  if (! pixarr)
    return;
  for (int i = 0; i < count; i++)
    if (pixarr[i].geom)
      pfree(pixarr[i].geom);
  pfree(pixarr);
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Return the pixels of a raster band as points
 * @details The points of PostGIS's `ST_PixelAsPoints`: for each pixel, row by
 * row, its upper-left corner in the reference system of the raster, its value
 * and its column and row, starting at 1. With @p exclude_nodata the pixels the
 * band states as nodata are left out, which a band stating no nodata value
 * leaves none of; otherwise a nodata pixel keeps the value the band holds
 * for it
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @param[in] exclude_nodata True to leave the nodata pixels out
 * @param[out] count Number of pixels answered
 * @return An array the caller releases with #pixel_arr_free(), or NULL where
 * no pixel is answered, in which case @p count is 0 and no error is stated
 * @errval NULL
 * @csqlfn #Raster_pixel_as_points()
 */
Pixel *
raster_pixel_as_points(const Raster *rast, int band, bool exclude_nodata,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  rt_raster raster;
  rt_band rtband = raster_read_band(rast, band, &raster);
  if (! rtband)
    return NULL;
  int width = (int) rt_raster_get_width(raster);
  int height = (int) rt_raster_get_height(raster);
  int32_t srid = (int32_t) rt_raster_get_srid(raster);
  if (width < 1 || height < 1)
  {
    raster_destroy(raster);
    return NULL;
  }

  Pixel *result = palloc0(sizeof(Pixel) * (size_t) width * (size_t) height);
  int n = 0;
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      double value, xw, yw;
      int isnodata = 0;
      if (rt_band_get_pixel(rtband, x, y, &value, &isnodata) != ES_NONE ||
          rt_raster_cell_to_geopoint(raster, x, y, &xw, &yw, NULL) != ES_NONE)
      {
        pixel_arr_free(result, n);
        raster_destroy(raster);
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "Could not read the pixel (%d, %d) of band %d", x + 1, y + 1, band);
        return NULL;
      }
      if (isnodata && exclude_nodata)
        continue;
      LWGEOM *point = lwpoint_as_lwgeom(lwpoint_make2d(srid, xw, yw));
      result[n].x = x + 1;
      result[n].y = y + 1;
      result[n].nodata = (isnodata != 0);
      result[n].value = value;
      result[n].geom = geo_serialize(point);
      lwgeom_free(point);
      n++;
    }
  }
  raster_destroy(raster);
  if (! n)
  {
    pfree(result);
    return NULL;
  }
  *count = n;
  return result;
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Return the values of a raster band
 * @details The values of PostGIS's `ST_DumpValues`, row by row, the
 * @p count of them the width times the height of the raster. With
 * @p exclude_nodata, @p nodata states the pixels the band states as nodata,
 * which PostGIS answers as NULL; otherwise every value is the one the band
 * holds and @p nodata states none
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @param[in] exclude_nodata True to state the nodata pixels
 * @param[out] nodata Array of the same length, true for a nodata pixel
 * stated, which the caller frees
 * @param[out] count Number of values answered
 * @return An array of values the caller frees, or NULL where the raster has
 * no pixel, in which case @p count is 0 and no error is stated
 * @errval NULL
 * @csqlfn #Raster_dump_values(), #Raster_dump_values_band()
 */
double *
raster_dump_values(const Raster *rast, int band, bool exclude_nodata,
  bool **nodata, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(nodata, NULL);
  VALIDATE_NOT_NULL(count, NULL);
  *nodata = NULL;
  *count = 0;
  rt_raster raster;
  rt_band rtband = raster_read_band(rast, band, &raster);
  if (! rtband)
    return NULL;
  int width = (int) rt_raster_get_width(raster);
  int height = (int) rt_raster_get_height(raster);
  if (width < 1 || height < 1)
  {
    raster_destroy(raster);
    return NULL;
  }

  size_t n = (size_t) width * (size_t) height;
  double *result = palloc(sizeof(double) * n);
  bool *isnull = palloc(sizeof(bool) * n);
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      size_t i = (size_t) y * (size_t) width + (size_t) x;
      int isnodata = 0;
      if (rt_band_get_pixel(rtband, x, y, &result[i], &isnodata) != ES_NONE)
      {
        pfree(result); pfree(isnull);
        raster_destroy(raster);
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "Could not read the pixel (%d, %d) of band %d", x + 1, y + 1, band);
        return NULL;
      }
      isnull[i] = exclude_nodata && isnodata;
    }
  }
  raster_destroy(raster);
  *nodata = isnull;
  *count = (int) n;
  return result;
}

/*****************************************************************************
 * A raster built from its pixels and its georeferencing
 *
 * The functions of PostGIS that build a raster and read and write the values
 * of its pixels, and the bytes of the pixels of a band, which keep every value
 * exactly where an array of doubles does not
 *****************************************************************************/

/**
 * @brief Return the pixel type of rt_core a name states
 * @details The name is either the one PostGIS gives a pixel type, such as
 * `8BUI`, or the one #raster_band_pixel_type answers, such as `uint8`, so the
 * type a raster states carries into the constructors as it stands
 * @param[in] name Name of the pixel type
 * @return The pixel type, or `PT_END` with an error stated
 */
static rt_pixtype
raster_pixtype(const char *name)
{
  rt_pixtype result = rt_pixtype_index_from_name(name);
  if (result != PT_END)
    return result;
  for (int i = 0; i <= (int) MEOS_PT_FLOAT16; i++)
  {
    const char *rqname = raquet_pixtype_name((MeosPixType) i);
    if (! rqname || pg_strcasecmp(name, rqname) != 0)
      continue;
    switch ((MeosPixType) i)
    {
      case MEOS_PT_UINT8:   return PT_8BUI;
      case MEOS_PT_INT8:    return PT_8BSI;
      case MEOS_PT_UINT16:  return PT_16BUI;
      case MEOS_PT_INT16:   return PT_16BSI;
      case MEOS_PT_UINT32:  return PT_32BUI;
      case MEOS_PT_INT32:   return PT_32BSI;
      case MEOS_PT_FLOAT32: return PT_32BF;
      case MEOS_PT_FLOAT64: return PT_64BF;
      default:
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "A raster band has no pixel type %s", name);
        return PT_END;
    }
  }
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "Unknown pixel type: %s",
    name);
  return PT_END;
}

/**
 * @brief Reverse in place the bytes of each pixel of a band on a big-endian
 * machine, which turns the order of the machine into the little-endian order
 * of the pixels a raster exchanges, and back
 * @param[in,out] data Pixels of the band
 * @param[in] count Number of pixels
 * @param[in] size Number of bytes of a pixel
 */
static void
raster_pixels_swap(uint8_t *data, size_t count, int size)
{
  if (! MEOS_IS_BIG_ENDIAN || size < 2)
    return;
  for (size_t i = 0; i < count; i++)
    raster_wkb_flip(data + i * (size_t) size, (size_t) size);
}

/**
 * @brief Return true when the dimensions of a raster are within the bounds
 * rt_core states, raising an error otherwise
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] min Least value of each dimension
 */
static bool
ensure_valid_raster_dims(int width, int height, int min)
{
  if (width < min || height < min || width > UINT16_MAX ||
      height > UINT16_MAX)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The width and height of a raster must be between %d and %d: %d x %d",
      min, UINT16_MAX, width, height);
    return false;
  }
  return true;
}

/**
 * @brief Return a raster with no band, its grid stated by its dimensions and
 * its georeferencing
 * @return The rt_core raster, which the caller releases, or NULL with an error
 * stated
 */
static rt_raster
raster_georef_new(int width, int height, double upperleftx, double upperlefty,
  double scalex, double scaley, double skewx, double skewy, int32_t srid)
{
  rt_raster result = rt_raster_new((uint32_t) width, (uint32_t) height);
  if (! result)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "Could not create raster");
    return NULL;
  }
  rt_raster_set_offsets(result, upperleftx, upperlefty);
  rt_raster_set_scale(result, scalex, scaley);
  rt_raster_set_skews(result, skewx, skewy);
  rt_raster_set_srid(result, srid);
  return result;
}

/**
 * @ingroup meos_raster_base_constructor
 * @brief Return a raster with no band, as PostGIS's `ST_MakeEmptyRaster`
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] upperleftx X coordinate of the upper left corner
 * @param[in] upperlefty Y coordinate of the upper left corner
 * @param[in] scalex Width of a pixel
 * @param[in] scaley Height of a pixel, negative where the rows go south
 * @param[in] skewx X component of the skew
 * @param[in] skewy Y component of the skew
 * @param[in] srid Spatial reference system identifier
 * @errval NULL
 * @csqlfn #Raster_make_empty()
 */
Raster *
raster_make_empty(int width, int height, double upperleftx,
  double upperlefty, double scalex, double scaley, double skewx, double skewy,
  int32_t srid)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_raster_dims(width, height, 0))
    return NULL;
  rt_raster raster = raster_georef_new(width, height, upperleftx, upperlefty,
    scalex, scaley, skewx, skewy, srid);
  if (! raster)
    return NULL;
  return raster_serialize_destroy(raster);
}

/**
 * @ingroup meos_raster_base_constructor
 * @brief Return a raster of one band from the bytes of its pixels and its
 * georeferencing
 * @details The pixels are the ones #raster_band_pixels answers: row by row,
 * each in the bytes of its type, little-endian beyond one byte, and one byte
 * for the types bounding a pixel to less than a byte. A raster of more bands
 * adds them with #raster_add_band_raster. The band states its nodata value
 * as rt_core reads it into the pixel type, and states the band as all
 * nodata where every pixel holds that value
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] upperleftx X coordinate of the upper left corner
 * @param[in] upperlefty Y coordinate of the upper left corner
 * @param[in] scalex Width of a pixel
 * @param[in] scaley Height of a pixel, negative where the rows go south
 * @param[in] skewx X component of the skew
 * @param[in] skewy Y component of the skew
 * @param[in] srid Spatial reference system identifier
 * @param[in] pixeltype Name of the pixel type, as PostGIS names it or as
 * #raster_band_pixel_type answers it
 * @param[in] nodataval Nodata value, read only where @p has_nodata is true
 * @param[in] has_nodata True where the band states a nodata value
 * @param[in] pixels Pixels of the band
 * @param[in] size Number of bytes of @p pixels
 * @errval NULL
 * @csqlfn #Raster_make()
 */
Raster *
raster_make(int width, int height, double upperleftx, double upperlefty,
  double scalex, double scaley, double skewx, double skewy, int32_t srid,
  const char *pixeltype, double nodataval, bool has_nodata,
  const uint8_t *pixels, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pixeltype, NULL); VALIDATE_NOT_NULL(pixels, NULL);
  if (! ensure_valid_raster_dims(width, height, 1))
    return NULL;
  rt_pixtype pixtype = raster_pixtype(pixeltype);
  if (pixtype == PT_END)
    return NULL;
  int pixsize = rt_pixtype_size(pixtype);
  size_t count = (size_t) width * (size_t) height;
  size_t expected = count * (size_t) pixsize;
  if (size != expected)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "A %d x %d raster band of type %s has %zu bytes, not %zu", width,
      height, rt_pixtype_name(pixtype), expected, size);
    return NULL;
  }
  /* A type bounding a pixel to less than a byte takes a byte a pixel, so a
   * byte beyond the bound is no value of the type */
  if (pixtype == PT_1BB || pixtype == PT_2BUI || pixtype == PT_4BUI)
  {
    uint8_t max = (pixtype == PT_1BB) ? 1 : (pixtype == PT_2BUI) ? 3 : 15;
    for (size_t i = 0; i < count; i++)
    {
      if (pixels[i] > max)
      {
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "The pixel %zu of a band of type %s holds %u, beyond %u", i + 1,
          rt_pixtype_name(pixtype), pixels[i], max);
        return NULL;
      }
    }
  }

  rt_raster raster = raster_georef_new(width, height, upperleftx, upperlefty,
    scalex, scaley, skewx, skewy, srid);
  if (! raster)
    return NULL;
  /* The band owns its data, which rt_core releases with the band */
  uint8_t *data = rtalloc(expected);
  memcpy(data, pixels, expected);
  raster_pixels_swap(data, count, pixsize);
  rt_band band = rt_band_new_inline((uint16_t) width, (uint16_t) height,
    pixtype, has_nodata ? 1 : 0, nodataval, data);
  if (! band)
  {
    rtdealloc(data);
    rt_raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not create the band of the raster");
    return NULL;
  }
  rt_band_set_ownsdata_flag(band, 1);
  if (rt_raster_add_band(raster, band, 0) < 0)
  {
    rt_band_destroy(band);
    rt_raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not add the band to the raster");
    return NULL;
  }
  if (has_nodata)
    rt_band_check_is_nodata(band);
  return raster_serialize_destroy(raster);
}

/**
 * @ingroup meos_raster_base_constructor
 * @brief Return a raster with a band added, each of its pixels holding a value,
 * as PostGIS's `ST_AddBand`
 * @param[in] rast Raster
 * @param[in] index Number the band takes, starting at 1, or 0 to add it after
 * the last band
 * @param[in] pixeltype Name of the pixel type, as PostGIS names it or as
 * #raster_band_pixel_type answers it
 * @param[in] initialvalue Value of every pixel
 * @param[in] nodataval Nodata value, read only where @p has_nodata is true
 * @param[in] has_nodata True where the band states a nodata value
 * @errval NULL
 * @csqlfn #Raster_add_band()
 */
Raster *
raster_add_band(const Raster *rast, int index, const char *pixeltype,
  double initialvalue, double nodataval, bool has_nodata)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(pixeltype, NULL);
  rt_pixtype pixtype = raster_pixtype(pixeltype);
  if (pixtype == PT_END)
    return NULL;
  rt_raster raster = rt_raster_deserialize((void *) rast, 0);
  if (! raster)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }
  int numbands = (int) rt_raster_get_num_bands(raster);
  if (index < 0 || index > numbands + 1)
  {
    raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "A band added to the raster takes a number from 1 to %d, not %d",
      numbands + 1, index);
    return NULL;
  }
  int pos = (index == 0) ? numbands : index - 1;
  if (rt_raster_generate_new_band(raster, pixtype, initialvalue,
      has_nodata ? 1 : 0, nodataval, pos) < 0)
  {
    raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not add a band to the raster");
    return NULL;
  }
  return raster_serialize_destroy(raster);
}

/**
 * @ingroup meos_raster_base_constructor
 * @brief Return a raster with a copy of the band of another raster added, as
 * PostGIS's `ST_AddBand`
 * @param[in] torast Raster the band is added to
 * @param[in] fromrast Raster the band is copied from, of the same width and
 * height
 * @param[in] fromband Number of the band copied, starting at 1
 * @param[in] toindex Number the band takes, starting at 1, or 0 to add it
 * after the last band
 * @errval NULL
 * @csqlfn #Raster_add_band_raster()
 */
Raster *
raster_add_band_raster(const Raster *torast, const Raster *fromrast,
  int fromband, int toindex)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(torast, NULL); VALIDATE_NOT_NULL(fromrast, NULL);
  rt_raster from;
  if (! raster_read_band(fromrast, fromband, &from))
    return NULL;
  rt_raster to = rt_raster_deserialize((void *) torast, 0);
  if (! to)
  {
    raster_destroy(from);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }
  int numbands = (int) rt_raster_get_num_bands(to);
  int width = (int) rt_raster_get_width(to);
  int height = (int) rt_raster_get_height(to);
  int fromwidth = (int) rt_raster_get_width(from);
  int fromheight = (int) rt_raster_get_height(from);
  if (width != fromwidth || height != fromheight)
  {
    raster_destroy(from); raster_destroy(to);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "A band of a %d x %d raster cannot be added to a %d x %d raster",
      fromwidth, fromheight, width, height);
    return NULL;
  }
  if (toindex < 0 || toindex > numbands + 1)
  {
    raster_destroy(from); raster_destroy(to);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "A band added to the raster takes a number from 1 to %d, not %d",
      numbands + 1, toindex);
    return NULL;
  }
  int pos = (toindex == 0) ? numbands : toindex - 1;
  int added = rt_raster_copy_band(to, from, fromband - 1, pos);
  raster_destroy(from);
  if (added < 0)
  {
    raster_destroy(to);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not add the band to the raster");
    return NULL;
  }
  return raster_serialize_destroy(to);
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the bytes of the pixels of a raster band
 * @details The pixels row by row, each in the bytes of its type, little-endian
 * beyond one byte, and one byte for the types bounding a pixel to less than a
 * byte: every value exactly, which #raster_make reads back
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @param[out] size_out Number of bytes answered
 * @return The bytes the caller frees, or NULL where the raster has no pixel,
 * in which case @p size_out is 0 and no error is stated
 * @errval NULL
 * @csqlfn #Raster_band_pixels()
 */
uint8_t *
raster_band_pixels(const Raster *rast, int band, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  *size_out = 0;
  rt_raster raster;
  rt_band rtband = raster_read_band(rast, band, &raster);
  if (! rtband)
    return NULL;
  size_t count = (size_t) rt_raster_get_width(raster) *
    (size_t) rt_raster_get_height(raster);
  if (! count)
  {
    raster_destroy(raster);
    return NULL;
  }
  int pixsize = rt_pixtype_size(rt_band_get_pixtype(rtband));
  const uint8_t *data = rt_band_get_data(rtband);
  if (pixsize < 1 || ! data)
  {
    raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not read the pixels of band %d", band);
    return NULL;
  }
  size_t size = count * (size_t) pixsize;
  uint8_t *result = palloc(size);
  memcpy(result, data, size);
  raster_destroy(raster);
  raster_pixels_swap(result, count, pixsize);
  *size_out = size;
  return result;
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Return a raster whose band takes the values of an array over a block
 * of pixels, as PostGIS's `ST_SetValues`
 * @details The values are the block row by row, its upper left pixel at
 * (@p x, @p y). A value stated as nodata sets the pixel to the nodata value
 * of the band, or to the least value of its type where the band states none;
 * a value stated as not to set leaves the pixel as it is, as does
 * @p keepnodata for a pixel holding the nodata value of the band. A pixel of
 * the block beyond the raster is left out, and a value beyond the pixel type
 * is read into it as rt_core reads it
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @param[in] x Column of the upper left pixel of the block, starting at 1
 * @param[in] y Row of the upper left pixel of the block, starting at 1
 * @param[in] width Number of columns of the block
 * @param[in] height Number of rows of the block
 * @param[in] values Values of the block
 * @param[in] nodata True for a value stated as nodata, NULL where none is @nullable
 * @param[in] noset True for a value not to set, row by row, NULL where every
 * value is set @nullable
 * @param[in] nosetwidth Number of columns of the flags not to set
 * @param[in] nosetheight Number of rows of the flags not to set
 * @param[in] count Number of values, the width times the height of the block
 * @param[in] keepnodata True to leave the pixels holding the nodata value
 * @errval NULL
 * @csqlfn #Raster_set_values()
 */
Raster *
raster_set_values(const Raster *rast, int band, int x, int y, int width,
  int height, const double *values, const bool *nodata, const bool *noset,
  int nosetwidth, int nosetheight, int count, bool keepnodata)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(values, NULL);
  if (width < 1 || height < 1 || (int64) width * height != count)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "A block of %d x %d pixels takes as many values, not %d", width, height,
      count);
    return NULL;
  }
  if (noset && (nosetwidth < 1 || nosetheight < 1))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "A block of %d x %d flags not to set states none", nosetwidth,
      nosetheight);
    return NULL;
  }
  /* rt_core writes the pixels where the band lies, which in a raster read in
   * full is the serialized form itself, so it is a copy that is written */
  Raster *copy = raster_copy(rast);
  rt_raster raster;
  rt_band rtband = raster_read_band(copy, band, &raster);
  if (! rtband)
  {
    pfree(copy);
    return NULL;
  }
  int rwidth = (int) rt_raster_get_width(raster);
  int rheight = (int) rt_raster_get_height(raster);
  if (x < 1 || x > rwidth || y < 1 || y > rheight)
  {
    raster_destroy(raster); pfree(copy);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The pixel (%d, %d) is not in a %d x %d raster", x, y, rwidth, rheight);
    return NULL;
  }
  bool hasnodata = (rt_band_get_hasnodata_flag(rtband) != 0);
  double nodataval;
  if (hasnodata)
    rt_band_get_nodata(rtband, &nodataval);
  else
    nodataval = rt_band_get_min_value(rtband);

  for (int j = 0; j < height; j++)
  {
    int py = y - 1 + j;
    for (int i = 0; i < width; i++)
    {
      int k = j * width + i;
      int px = x - 1 + i;
      if ((noset && i < nosetwidth && j < nosetheight &&
            noset[j * nosetwidth + i]) || px >= rwidth || py >= rheight)
        continue;
      if (hasnodata && keepnodata)
      {
        double value;
        int isnodata = 0;
        if (rt_band_get_pixel(rtband, px, py, &value, &isnodata) != ES_NONE)
        {
          raster_destroy(raster); pfree(copy);
          meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
            "Could not read the pixel (%d, %d) of band %d", px + 1, py + 1,
            band);
          return NULL;
        }
        if (isnodata)
          continue;
      }
      double value = (nodata && nodata[k]) ? nodataval : values[k];
      if (rt_band_set_pixel(rtband, px, py, value, NULL) != ES_NONE)
      {
        raster_destroy(raster); pfree(copy);
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "Could not set the pixel (%d, %d) of band %d", px + 1, py + 1, band);
        return NULL;
      }
    }
  }
  Raster *result = raster_serialize_destroy(raster);
  pfree(copy);
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
 * @csqlfn #Raster_summary_stats()
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

  /* The statistics are allocated by the raster core, so its own deallocator
   * releases them: in the extension build that allocator is not PostgreSQL's,
   * and pfree() on them raises "pfree called with invalid pointer" */

  /* A band whose every pixel is nodata states no value to summarize, which is
   * an answer rather than an error; the statistics of no pixels have no mean */
  if (stats->count < 1)
  {
    rtdealloc(stats);
    return NULL;
  }

  BandStats *result = palloc0(sizeof(BandStats));
  result->count = stats->count;
  result->sum = stats->sum;
  result->mean = stats->mean;
  result->stddev = stats->stddev;
  result->min = stats->min;
  result->max = stats->max;
  rtdealloc(stats);
  return result;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the number of pixels of raster band statistics
 * @details The accessors of the statistics read the fields of the one value
 * #raster_summary_stats answers, as a query reads them in every engine, by a
 * function rather than by the name of a field
 * @param[in] stats Statistics, as #raster_summary_stats answers them
 * @errval INT64_MAX
 * @csqlfn #Summarystats_count()
 */
int64
bandstats_count(const BandStats *stats)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(stats, INT64_MAX);
  return (int64) stats->count;
}
/**
 * @ingroup meos_raster_base_accessor
 * @brief Return in the last argument the sum of raster band statistics
 * @param[in] stats Statistics, as #raster_summary_stats answers them
 * @param[out] result Sum
 * @return False where the statistics count no pixel, which state no value
 * @csqlfn #Summarystats_sum()
 */
bool
bandstats_sum(const BandStats *stats, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(stats, false); VALIDATE_NOT_NULL(result, false);
  if (stats->count == 0)
    return false;
  *result = stats->sum;
  return true;
}
/**
 * @ingroup meos_raster_base_accessor
 * @brief Return in the last argument the mean of raster band statistics
 * @param[in] stats Statistics, as #raster_summary_stats answers them
 * @param[out] result Mean
 * @return False where the statistics count no pixel, which state no value
 * @csqlfn #Summarystats_mean()
 */
bool
bandstats_mean(const BandStats *stats, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(stats, false); VALIDATE_NOT_NULL(result, false);
  if (stats->count == 0)
    return false;
  *result = stats->mean;
  return true;
}
/**
 * @ingroup meos_raster_base_accessor
 * @brief Return in the last argument the standard deviation of band statistics
 * @param[in] stats Statistics, as #raster_summary_stats answers them
 * @param[out] result Standard deviation
 * @return False where the statistics count no pixel, which state no value
 * @csqlfn #Summarystats_stddev()
 */
bool
bandstats_stddev(const BandStats *stats, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(stats, false); VALIDATE_NOT_NULL(result, false);
  if (stats->count == 0)
    return false;
  *result = stats->stddev;
  return true;
}
/**
 * @ingroup meos_raster_base_accessor
 * @brief Return in the last argument the minimum of raster band statistics
 * @param[in] stats Statistics, as #raster_summary_stats answers them
 * @param[out] result Minimum
 * @return False where the statistics count no pixel, which state no value
 * @csqlfn #Summarystats_min()
 */
bool
bandstats_min(const BandStats *stats, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(stats, false); VALIDATE_NOT_NULL(result, false);
  if (stats->count == 0)
    return false;
  *result = stats->min;
  return true;
}
/**
 * @ingroup meos_raster_base_accessor
 * @brief Return in the last argument the maximum of raster band statistics
 * @param[in] stats Statistics, as #raster_summary_stats answers them
 * @param[out] result Maximum
 * @return False where the statistics count no pixel, which state no value
 * @csqlfn #Summarystats_max()
 */
bool
bandstats_max(const BandStats *stats, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(stats, false); VALIDATE_NOT_NULL(result, false);
  if (stats->count == 0)
    return false;
  *result = stats->max;
  return true;
}

/**
 * @brief Remove every space from a string, in place
 * @details The grammar below is stated without regard to spacing, so the
 * expression is closed up before it is read rather than each reader skipping
 * whitespace of its own
 */
static char *
reclass_removespaces(char *str)
{
  char *w = str;
  for (char *r = str; *r; r++)
    if (! isspace((unsigned char) *r))
      *w++ = *r;
  *w = '\0';
  return str;
}

/**
 * @brief Split a string on one separator character, the way the grammar reads
 * @details EMPTY PIECES ARE SKIPPED, which is load bearing rather than
 * incidental: PostGIS splits with @p strtok, so a run of separators counts as
 * one and a leading separator opens no empty piece. That is what lets a range
 * of negative bounds be read at all -- `-9999--1` states two pieces, `9999`
 * and `1`, whose signs are restored afterwards from the text they came from,
 * where a split keeping empties would state four and be refused as malformed
 * @param[in] str String to split
 * @param[in] sep Separator character
 * @param[out] n Number of pieces
 * @return An array of pieces, each and the array itself the caller's to free
 */
static char **
reclass_strsplit(const char *str, char sep, int *n)
{
  /* An upper bound on the pieces: one more than the separators */
  int cap = 1;
  for (const char *p = str; *p; p++)
    if (*p == sep)
      cap++;
  char **result = palloc(sizeof(char *) * (size_t) cap);

  int k = 0;
  const char *p = str;
  while (*p)
  {
    while (*p == sep)
      p++;
    if (! *p)
      break;
    const char *start = p;
    while (*p && *p != sep)
      p++;
    size_t len = (size_t) (p - start);
    result[k] = palloc(len + 1);
    memcpy(result[k], start, len);
    result[k][len] = '\0';
    k++;
  }
  /* A string holding nothing but separators states one empty piece, as the
   * splitter this follows does for an empty string */
  if (! k)
  {
    result[0] = palloc(1);
    result[0][0] = '\0';
    k = 1;
  }
  *n = k;
  return result;
}

/**
 * @brief Release the pieces of a split and the array holding them
 */
static void
reclass_strsplit_free(char **pieces, int n)
{
  for (int i = 0; i < n; i++)
    pfree(pieces[i]);
  pfree(pieces);
}

/**
 * @brief Remove any of a set of characters from both ends of a string
 */
static void
reclass_chartrim(char *str, const char *chars)
{
  size_t len = strlen(str);
  size_t start = 0;
  while (start < len && strchr(chars, str[start]))
    start++;
  while (len > start && strchr(chars, str[len - 1]))
    len--;
  memmove(str, str + start, len - start);
  str[len - start] = '\0';
}

/**
 * @brief Return the LAST occurrence of one string within another
 */
static const char *
reclass_strrstr(const char *haystack, const char *needle)
{
  size_t nlen = strlen(needle);
  if (! nlen)
    return haystack;
  const char *found = NULL;
  for (const char *p = haystack; (p = strstr(p, needle)) != NULL; p++)
    found = p;
  return found;
}

/**
 * @brief Read a reclassification expression into the ranges it states
 * @details THE GRAMMAR IS POSTGIS'S, NOT INVENTED HERE. An expression is a
 * comma separated list of mappings; each mapping is a source range, a colon,
 * and a destination range; each range is one value or two around a dash. A
 * bracket states how an end is bounded: on the low end `(` excludes the value
 * and `)` or `]` marks the range as reaching beyond it, while on the high end
 * `]` includes the value and `(` or `[` marks it as reaching beyond. A dash
 * belonging to a negative number is told from the separating dash by what
 * stands before it, which is why the pieces are matched back against the text
 * they came from
 * @param[in] expr Expression, already closed up of spaces
 * @param[out] count Number of ranges read
 * @errval NULL
 */
static rt_reclassexpr *
reclass_parse(const char *expr, int *count)
{
  int comma_n;
  char **comma_set = reclass_strsplit(expr, ',', &comma_n);
  if (comma_n < 1)
  {
    reclass_strsplit_free(comma_set, comma_n);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The reclassification expression states no mapping: %s", expr);
    return NULL;
  }

  rt_reclassexpr *exprset = palloc0(sizeof(rt_reclassexpr) * (size_t) comma_n);
  int j = 0;
  bool failed = false;

  for (int a = 0; a < comma_n && ! failed; a++)
  {
    int colon_n;
    char **colon_set = reclass_strsplit(comma_set[a], ':', &colon_n);
    if (colon_n != 2)
    {
      reclass_strsplit_free(colon_set, colon_n);
      failed = true;
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "A mapping states a source and a destination around one colon: %s",
        comma_set[a]);
      break;
    }

    exprset[j] = palloc0(sizeof(struct rt_reclassexpr_t));
    for (int b = 0; b < colon_n && ! failed; b++)
    {
      int dash_n;
      char **dash_set = reclass_strsplit(colon_set[b], '-', &dash_n);

      /* A bracket standing alone before the first value belongs to it: the
       * low end of `(-9999-100` splits as "(", "9999", "100" and the bracket
       * has to be given back to the number it bounds */
      if (dash_n > 1 && strlen(dash_set[0]) == 1 &&
          strchr("()[]", dash_set[0][0]))
      {
        size_t len = strlen(dash_set[1]) + 2;
        char *glued = palloc(len);
        snprintf(glued, len, "%s%s", dash_set[0], dash_set[1]);
        pfree(dash_set[0]); pfree(dash_set[1]);
        dash_set[1] = glued;
        for (int t = 1; t < dash_n; t++)
          dash_set[t - 1] = dash_set[t];
        dash_n--;
      }

      if (dash_n < 1 || dash_n > 2)
      {
        /* This break leaves the enclosing loop, so its release is not reached
         * and the pieces are freed here */
        reclass_strsplit_free(dash_set, dash_n);
        failed = true;
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "A range states one value or two around a dash: %s", colon_set[b]);
        break;
      }

      for (int c = 0; c < dash_n && ! failed; c++)
      {
        /* How this end is bounded, read from the bracket that marks it. The
         * low end and the high end read the same characters differently */
        int exc_val = 0, inc_val = 1;
        if (dash_n != 1)
        {
          if (c < 1)
          {
            if (strchr(dash_set[c], ')') || strchr(dash_set[c], ']'))
            {
              exc_val = 1; inc_val = 1;
            }
            else if (strchr(dash_set[c], '('))
              inc_val = 0;
            else
              inc_val = 1;
          }
          else
          {
            if (strrchr(dash_set[c], '(') || strrchr(dash_set[c], '['))
            {
              exc_val = 1; inc_val = 0;
            }
            else if (strrchr(dash_set[c], ']'))
              inc_val = 1;
            else
              inc_val = 0;
          }
        }

        reclass_chartrim(dash_set[c], "()[]");

        char *end = NULL;
        errno = 0;
        double val = strtod(dash_set[c], &end);
        if (errno != 0 || end == dash_set[c])
        {
          /* The pieces are released once, where this loop ends: freeing them
           * here as well would release them twice, since breaking leaves the
           * enclosing loop's own release still to come */
          failed = true;
          meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
            "A range bound is not a number: %s", colon_set[b]);
          break;
        }

        /* The split consumed the dash of a negative number along with the one
         * that separates the bounds, so the sign is read back from the text
         * the piece came from: a dash before it that itself opens the string,
         * or follows another dash or an opening bracket, is a sign */
        const char *at = (c < 1) ? strstr(colon_set[b], dash_set[c]) :
          reclass_strrstr(colon_set[b], dash_set[c]);
        if (at && at != colon_set[b] && *(at - 1) == '-' &&
            (((at - 1) == colon_set[b]) || *(at - 2) == '-' ||
             *(at - 2) == '[' || *(at - 2) == '('))
          val *= -1.0;

        if (b < 1)
        {
          if (dash_n == 1)
          {
            exprset[j]->src.exc_min = exprset[j]->src.exc_max = exc_val;
            exprset[j]->src.inc_min = exprset[j]->src.inc_max = inc_val;
            exprset[j]->src.min = exprset[j]->src.max = val;
          }
          else if (c < 1)
          {
            exprset[j]->src.exc_min = exc_val;
            exprset[j]->src.inc_min = inc_val;
            exprset[j]->src.min = val;
          }
          else
          {
            exprset[j]->src.exc_max = exc_val;
            exprset[j]->src.inc_max = inc_val;
            exprset[j]->src.max = val;
          }
        }
        else
        {
          if (dash_n == 1)
            exprset[j]->dst.min = exprset[j]->dst.max = val;
          else if (c < 1)
            exprset[j]->dst.min = val;
          else
            exprset[j]->dst.max = val;
        }
      }
      reclass_strsplit_free(dash_set, dash_n);
    }
    reclass_strsplit_free(colon_set, colon_n);
    if (! failed)
      j++;
  }
  reclass_strsplit_free(comma_set, comma_n);

  if (failed)
  {
    for (int k = 0; k <= j && k < comma_n; k++)
      if (exprset[k])
        pfree(exprset[k]);
    pfree(exprset);
    return NULL;
  }

  *count = j;
  return exprset;
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Return a raster whose band states the classes an expression maps its
 * values onto
 * @details The values of a band are read as the ranges an expression names and
 * written as the ranges it maps them to, so a coverage of measurements becomes
 * a coverage of classes that a trajectory can be read against. The grammar is
 * PostGIS's: a comma separated list of mappings, each a source range and a
 * destination range around a colon, each range one value or two around a dash,
 * with brackets stating how an end is bounded
 * @param[in] rast Raster to reclassify
 * @param[in] band Number of the band, starting at 1
 * @param[in] expr Expression, such as `0-100:1, 101-200:2, 201-300:3`
 * @param[in] pixeltype Name of the pixel type of the resulting band
 * @param[in] has_nodata True where the resulting band states a nodata value
 * @param[in] nodataval That value, read only where @p has_nodata is true
 * @errval NULL
 * @csqlfn #Raster_reclass()
 */
Raster *
raster_reclass(const Raster *rast, int band, const char *expr,
  const char *pixeltype, bool has_nodata, double nodataval)
{
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(expr, NULL);
  VALIDATE_NOT_NULL(pixeltype, NULL);

  rt_pixtype pixtype = rt_pixtype_index_from_name(pixeltype);
  if (pixtype == PT_END)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Unknown pixel type: %s", pixeltype);
    return NULL;
  }

  /* The band values are read and written, so the subject is deserialized in
   * full */
  rt_raster raster = rt_raster_deserialize((void *) rast, 0);
  if (! raster)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }

  int numbands = (int) rt_raster_get_num_bands(raster);
  if (band < 1 || band > numbands)
  {
    raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The raster has no band %d, it has %d", band, numbands);
    return NULL;
  }

  char *clean = pstrdup(expr);
  reclass_removespaces(clean);
  int nexpr = 0;
  rt_reclassexpr *exprset = reclass_parse(clean, &nexpr);
  pfree(clean);
  if (! exprset)
  {
    raster_destroy(raster);
    return NULL;
  }
  if (nexpr < 1)
  {
    pfree(exprset); raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The reclassification expression states no mapping: %s", expr);
    return NULL;
  }

  rt_band rtband = rt_raster_get_band(raster, (uint32_t) (band - 1));
  rt_band newband = rtband ? rt_band_reclass(rtband, pixtype,
    has_nodata ? 1 : 0, nodataval, exprset, (uint32_t) nexpr) : NULL;
  for (int k = 0; k < nexpr; k++)
    pfree(exprset[k]);
  pfree(exprset);
  if (! newband)
  {
    raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not reclassify band %d of the raster", band);
    return NULL;
  }

  /* The reclassified band stands where the one it was read from stood, so the
   * result keeps the grid and every other band of the subject */
  rt_band oldband = rt_raster_replace_band(raster, newband,
    (uint32_t) (band - 1));
  if (! oldband)
  {
    rt_band_destroy(newband); raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not put the reclassified band %d back", band);
    return NULL;
  }
  rt_band_destroy(oldband);
  return raster_serialize_destroy(raster);
}

/*****************************************************************************
 * Value count
 *****************************************************************************/

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the number of pixels of a raster band holding each value
 * @details The histogram of PostGIS's `ST_ValueCount` and `ST_ValuePercent`,
 * read by `rt_band_get_value_count`: one row for each distinct value of the
 * band, or one for each search value where some are given, with the number of
 * pixels holding it and their share of the pixels read. A value is rounded to
 * the nearest multiple of @p roundto before it is counted, where @p roundto is
 * positive. A band whose every pixel is nodata, as the band itself states it,
 * answers no row where nodata is excluded, and one row holding the nodata
 * value otherwise. Where no pixel is read, the share of a search value is
 * zero divided by zero, NaN, as for PostGIS
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @param[in] exclude_nodata True to leave the nodata pixels out
 * @param[in] searchvalues Values to count, or NULL to count every value @nullable
 * @param[in] nsearch Number of search values
 * @param[in] roundto Multiple the values are rounded to, 0 for none
 * @param[out] count Number of rows of the result
 * @return An array of value counts, which may be empty
 * @errval NULL
 * @csqlfn #Raster_value_count(), #Raster_value_percent()
 */
ValueCount *
raster_value_count(const Raster *rast, int band, bool exclude_nodata,
  const double *searchvalues, int nsearch, double roundto, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  if (nsearch < 0 || (nsearch > 0 && ! searchvalues))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The search values of a value count are not stated");
    return NULL;
  }

  /* The pixels are read, so the subject is deserialized in full */
  rt_raster raster = rt_raster_deserialize((void *) rast, 0);
  if (! raster)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }
  /* A band the raster does not have is an error, as it is for every other
   * accessor of this family, rather than the NULL PostGIS answers */
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

  /* A band stating that every pixel is nodata is answered here: rt_core
   * warns and leaves the count unset where nodata is excluded, and writes the
   * share of the first search value alone where it is not */
  if (rt_band_get_hasnodata_flag(rtband) &&
      rt_band_get_isnodata_flag(rtband))
  {
    ValueCount *result = NULL;
    if (! exclude_nodata)
    {
      double nodataval;
      rt_band_get_nodata(rtband, &nodataval);
      uint32_t npixels = (uint32_t) (rt_band_get_width(rtband) *
        rt_band_get_height(rtband));
      int n = nsearch > 0 ? nsearch : 1;
      result = palloc(sizeof(ValueCount) * n);
      for (int i = 0; i < n; i++)
      {
        result[i].value = nsearch > 0 ? searchvalues[i] : nodataval;
        bool match = nsearch == 0 || searchvalues[i] == nodataval;
        result[i].count = match ? npixels : 0;
        result[i].percent = match ? 1.0 : 0.0;
      }
      *count = n;
    }
    raster_destroy(raster);
    return result;
  }

  uint32_t nrows = 0;
  rt_valuecount vcnts = rt_band_get_value_count(rtband,
    exclude_nodata ? 1 : 0, (double *) searchvalues, (uint32_t) nsearch,
    roundto, NULL, &nrows);
  raster_destroy(raster);
  if (! vcnts && nrows > 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not count the values of band %d of the raster", band);
    return NULL;
  }
  ValueCount *result = NULL;
  if (nrows > 0)
  {
    result = palloc(sizeof(ValueCount) * nrows);
    for (uint32_t i = 0; i < nrows; i++)
    {
      result[i].value = vcnts[i].value;
      result[i].count = vcnts[i].count;
      result[i].percent = vcnts[i].percent;
    }
  }
  if (vcnts)
    rtdealloc(vcnts);
  *count = (int) nrows;
  return result;
}

/*****************************************************************************
 * Mathematical functions
 *****************************************************************************/

/**
 * @brief Arguments of the callback computing the arithmetic of rasters
 */
typedef struct
{
  TArithmetic oper;   /**< Operation */
  bool number;        /**< True where the second operand is a number */
  bool invert;        /**< True where the number is the first operand */
  double value;       /**< The number, read only where @p number is true */
  bool divzero;       /**< Set where a pixel is divided by zero */
} RasterArithArg;

/**
 * @brief Compute a pixel of an arithmetic operation over rasters
 * @details The callback of #raster_clip_callback's shape: a pixel is nodata in
 * the result where it is nodata in either operand. A division by zero is
 * recorded in the arguments rather than reported here, so that the iteration
 * ends with an answer the caller turns into the error, and rt_core writes no
 * message of its own
 */
static int
raster_arith_callback(rt_iterator_arg arg, void *userarg, double *value,
  int *nodata)
{
  RasterArithArg *args = (RasterArithArg *) userarg;
  *value = 0.0;
  if (arg->nodata[0][0][0] || (! args->number && arg->nodata[1][0][0]))
  {
    *nodata = 1;
    return 1;
  }
  double x = arg->values[0][0][0];
  double y = args->number ? args->value : arg->values[1][0][0];
  if (args->invert)
  {
    double swap = x;
    x = y;
    y = swap;
  }
  switch (args->oper)
  {
    case ADD:
      *value = x + y;
      break;
    case SUB:
      *value = x - y;
      break;
    case MUL:
      *value = x * y;
      break;
    default: /* DIV */
      if (y == 0.0)
      {
        args->divzero = true;
        *nodata = 1;
        return 1;
      }
      *value = x / y;
  }
  *nodata = 0;
  return 1;
}

/**
 * @brief Return true if two rasters of the same reference system lie on the
 * same grid
 * @details The test of #rt_raster_same_alignment, which rt_core states in a
 * file answering other relationships with GEOS, read here over the
 * conversions of rt_raster.c, which reach none: the scales and the skews are
 * equal and the upper left corner of the first raster is a corner of the
 * pixels of the second
 * @param[in] raster1,raster2 Rasters
 * @param[out] reason Why the rasters are not aligned, set where they are not
 */
static bool
raster_grids_aligned(rt_raster raster1, rt_raster raster2,
  const char **reason)
{
  if (FLT_NEQ(fabs(rt_raster_get_x_scale(raster1)),
      fabs(rt_raster_get_x_scale(raster2))))
  {
    *reason = "The rasters have different scales on the X axis";
    return false;
  }
  if (FLT_NEQ(fabs(rt_raster_get_y_scale(raster1)),
      fabs(rt_raster_get_y_scale(raster2))))
  {
    *reason = "The rasters have different scales on the Y axis";
    return false;
  }
  if (FLT_NEQ(rt_raster_get_x_skew(raster1), rt_raster_get_x_skew(raster2)))
  {
    *reason = "The rasters have different skews on the X axis";
    return false;
  }
  if (FLT_NEQ(rt_raster_get_y_skew(raster1), rt_raster_get_y_skew(raster2)))
  {
    *reason = "The rasters have different skews on the Y axis";
    return false;
  }
  double ulx = rt_raster_get_x_offset(raster1);
  double uly = rt_raster_get_y_offset(raster1);
  double xr, yr, xw, yw;
  if (rt_raster_geopoint_to_cell(raster2, ulx, uly, &xr, &yr, NULL) !=
        ES_NONE ||
      rt_raster_cell_to_geopoint(raster2, xr, yr, &xw, &yw, NULL) != ES_NONE ||
      ! FLT_EQ(xw, ulx) || ! FLT_EQ(yw, uly))
  {
    *reason = "The rasters (pixel corner coordinates) are not aligned";
    return false;
  }
  return true;
}

/**
 * @brief Read the alignment of two rasters, refusing two reference systems
 * @param[in] rast1,rast2 Rasters
 * @param[out] aligned True where the rasters are aligned
 * @param[out] reason Why they are not, or that they are
 * @return True when the alignment is read
 * @errval false
 */
static bool
raster_alignment(const Raster *rast1, const Raster *rast2, bool *aligned,
  const char **reason)
{
  /* Only the grids are read, so the headers suffice */
  rt_raster raster1 = raster_header(rast1);
  if (! raster1)
    return false;
  rt_raster raster2 = raster_header(rast2);
  if (! raster2)
  {
    rt_raster_destroy(raster1);
    return false;
  }
  /* Two rasters in different reference systems are refused, as every
   * operation over two spatial values is */
  bool valid = ensure_same_srid((int32_t) rt_raster_get_srid(raster1),
    (int32_t) rt_raster_get_srid(raster2));
  if (valid)
  {
    *reason = "The rasters are aligned";
    *aligned = raster_grids_aligned(raster1, raster2, reason);
  }
  rt_raster_destroy(raster1); rt_raster_destroy(raster2);
  return valid;
}

/**
 * @ingroup meos_raster_base_comp
 * @brief Return true if two rasters lie on the same grid
 * @details The test of PostGIS's `ST_SameAlignment`: the scales and the skews
 * are equal, and the upper left corner of the first raster is a corner of the
 * pixels of the second. Two rasters in different reference systems raise an
 * error, where PostGIS answers false
 * @param[in] rast1,rast2 Rasters
 * @errval false
 * @csqlfn #Raster_same_alignment()
 */
bool
raster_same_alignment(const Raster *rast1, const Raster *rast2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast1, false); VALIDATE_NOT_NULL(rast2, false);
  bool aligned = false;
  const char *reason = NULL;
  if (! raster_alignment(rast1, rast2, &aligned, &reason))
    return false;
  return aligned;
}

/**
 * @ingroup meos_raster_base_comp
 * @brief Return why two rasters do not lie on the same grid
 * @details The reason of PostGIS's `ST_NotSameAlignmentReason`, which states
 * that the rasters are aligned where they are. Two rasters in different
 * reference systems raise an error, where PostGIS states it as the reason
 * @param[in] rast1,rast2 Rasters
 * @errval NULL
 * @csqlfn #Raster_not_same_alignment_reason()
 */
char *
raster_not_same_alignment_reason(const Raster *rast1, const Raster *rast2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast1, NULL); VALIDATE_NOT_NULL(rast2, NULL);
  bool aligned = false;
  const char *reason = NULL;
  if (! raster_alignment(rast1, rast2, &aligned, &reason))
    return NULL;
  return pstrdup(reason);
}

/**
 * @brief Return the grid of a raster as a raster of one pixel and no band
 * @param[in] rast Raster
 * @errval NULL
 */
static Raster *
raster_grid(const Raster *rast)
{
  rt_raster header = raster_header(rast);
  if (! header)
    return NULL;
  rt_raster grid = rt_raster_new(1, 1);
  if (! grid)
  {
    rt_raster_destroy(header);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not read the grid of the raster");
    return NULL;
  }
  rt_raster_set_scale(grid, rt_raster_get_x_scale(header),
    rt_raster_get_y_scale(header));
  rt_raster_set_skews(grid, rt_raster_get_x_skew(header),
    rt_raster_get_y_skew(header));
  rt_raster_set_offsets(grid, rt_raster_get_x_offset(header),
    rt_raster_get_y_offset(header));
  rt_raster_set_srid(grid, rt_raster_get_srid(header));
  rt_raster_destroy(header);
  return raster_serialize_destroy(grid);
}

/**
 * @ingroup meos_raster_base_agg
 * @brief Transition function of the aggregate testing whether rasters lie on
 * the same grid
 * @details The aggregate of PostGIS's `ST_SameAlignment(raster)`, in the
 * shape of #set_union_transfn: the state keeps the grid of the first raster
 * and whether every raster read is aligned with it, so a raster is tested
 * only while every one before it is aligned. A NULL raster leaves the state as
 * it is, and two rasters in different reference systems raise an error
 * @param[in,out] state Current aggregate state, NULL before the first raster @nullable
 * @param[in] rast Raster @nullable
 * @errval NULL
 * @csqlfn #Raster_same_alignment_transfn()
 * @csqlaggfn #sameAlignmentTransition()
 */
RasterAlignState *
raster_same_alignment_transfn(RasterAlignState *state, const Raster *rast)
{
  /* Null raster: return state */
  if (! rast)
    return state;
  if (! state)
  {
    Raster *grid = raster_grid(rast);
    if (! grid)
      return NULL;
    state = palloc(sizeof(RasterAlignState));
    state->grid = grid;
    state->aligned = true;
    return state;
  }
  if (state->aligned)
  {
    const char *reason = NULL;
    if (! raster_alignment(state->grid, rast, &state->aligned, &reason))
      return NULL;
  }
  return state;
}

/**
 * @ingroup meos_raster_base_agg
 * @brief Combine function of the aggregate testing whether rasters lie on
 * the same grid
 * @details The two states are aligned where each is and their grids are,
 * as #set_union_combinefn combines two sets
 * @param[in,out] state1 Current aggregate state @nullable
 * @param[in] state2 Aggregate state to combine with the first one @nullable
 * @return The first state, or a copy of the second where the first is NULL;
 * the second state is left to the caller
 * @errval NULL
 * @csqlfn #Raster_same_alignment_combinefn()
 */
RasterAlignState *
raster_same_alignment_combinefn(RasterAlignState *state1,
  const RasterAlignState *state2)
{
  if (! state2)
    return state1;
  if (! state1)
  {
    RasterAlignState *result = palloc(sizeof(RasterAlignState));
    result->grid = raster_copy(state2->grid);
    result->aligned = state2->aligned;
    return result;
  }
  if (state1->aligned)
  {
    if (! state2->aligned)
      state1->aligned = false;
    else
    {
      const char *reason = NULL;
      if (! raster_alignment(state1->grid, state2->grid, &state1->aligned,
          &reason))
        return NULL;
    }
  }
  return state1;
}

/**
 * @ingroup meos_raster_base_agg
 * @brief Final function of the aggregate testing whether rasters lie on the
 * same grid
 * @details The state is freed, as #set_union_finalfn frees its own. A NULL
 * state, an aggregate over no raster, answers false
 * @param[in] state Aggregate state @nullable
 * @csqlfn #Raster_same_alignment_finalfn()
 */
bool
raster_same_alignment_finalfn(RasterAlignState *state)
{
  if (! state)
    return false;
  bool result = state->aligned;
  pfree(state->grid);
  pfree(state);
  return result;
}

/**
 * @ingroup meos_raster_base_agg
 * @brief Return the bytes of the state of the aggregate testing whether
 * rasters lie on the same grid, read back by #rasteralignstate_deserialize
 * @details The bytes are a byte stating whether the rasters read are aligned,
 * followed by the Well-Known Binary of the grid, as #setstate_serialize writes
 * the Well-Known Binary of its set
 * @param[in] state State
 * @param[out] size_out Size of the result in bytes
 * @errval NULL
 * @csqlfn #Rasteralignstate_serialize()
 */
uint8_t *
rasteralignstate_serialize(const RasterAlignState *state, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(state, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  size_t wkb_size;
  uint8_t *wkb = raster_as_wkb(state->grid, 0, &wkb_size);
  if (! wkb)
    return NULL;
  uint8_t *result = palloc(wkb_size + 1);
  result[0] = state->aligned ? 1 : 0;
  memcpy(result + 1, wkb, wkb_size);
  pfree(wkb);
  *size_out = wkb_size + 1;
  return result;
}

/**
 * @ingroup meos_raster_base_agg
 * @brief Return the state of the aggregate testing whether rasters lie on
 * the same grid, written by #rasteralignstate_serialize
 * @param[in] bytes Bytes
 * @param[in] size Size of the bytes
 * @errval NULL
 * @csqlfn #Rasteralignstate_deserialize()
 */
RasterAlignState *
rasteralignstate_deserialize(const uint8_t *bytes, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(bytes, NULL);
  if (size < 2)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The bytes of an alignment state are too short");
    return NULL;
  }
  Raster *grid = raster_from_wkb(bytes + 1, size - 1);
  if (! grid)
    return NULL;
  RasterAlignState *result = palloc(sizeof(RasterAlignState));
  result->grid = grid;
  result->aligned = bytes[0] != 0;
  return result;
}

/**
 * @brief Return an arithmetic operation over rasters, pixel by pixel
 * @details The operands are two rasters, or a raster and a number. The raster
 * counterpart of #arithop_tnumber_tnumber and
 * #arithop_tnumber_number, its bands iterated as #raster_clip iterates them.
 * Every band of the result is a 64-bit float band, since a pixel holds the
 * arithmetic as computed: a difference of two unsigned bands that is negative,
 * a product beyond the range of the operands, or a ratio between zero and
 * one, all of which a band of the type of the operands would clamp or
 * truncate. Two rasters are operated band by band over the extent they share,
 * as temporal numbers are over the time they share, so they are to have the
 * same reference system, the same grid and the same number of bands
 * @param[in] rast1,rast2 Rasters, @p rast2 being NULL where the second
 * operand is the number
 * @param[in] d Number, read only where @p rast2 is NULL
 * @param[in] oper Operation
 * @param[in] invert True where the number is the first operand
 * @return NULL, with no error, where the two rasters share no pixel
 * @errval NULL
 */
static Raster *
raster_arithop(const Raster *rast1, const Raster *rast2, double d,
  TArithmetic oper, bool invert)
{
  /* A division by a number is tested once, before any pixel is read, as a
   * temporal number divided by a number is */
  if (! rast2 && oper == DIV && ! invert && fabs(d) < MEOS_EPSILON)
  {
    meos_error(ERROR, MEOS_ERR_DIVISION_BY_ZERO, "Division by zero");
    return NULL;
  }

  /* The bands are read, so the operands are deserialized in full */
  rt_raster raster1 = rt_raster_deserialize((void *) rast1, 0);
  rt_raster raster2 = rast2 ? rt_raster_deserialize((void *) rast2, 0) : NULL;
  if (! raster1 || (rast2 && ! raster2))
  {
    if (raster1) raster_destroy(raster1);
    if (raster2) raster_destroy(raster2);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }

  int32_t srid = (int32_t) rt_raster_get_srid(raster1);
  int numbands = (int) rt_raster_get_num_bands(raster1);
  if (numbands < 1)
  {
    raster_destroy(raster1);
    if (raster2) raster_destroy(raster2);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The raster carries no band to operate on");
    return NULL;
  }
  if (raster2)
  {
    int numbands2 = (int) rt_raster_get_num_bands(raster2);
    const char *reason = NULL;
    bool valid = ensure_same_srid(srid,
      (int32_t) rt_raster_get_srid(raster2));
    if (valid && numbands != numbands2)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The rasters have %d and %d bands", numbands, numbands2);
      valid = false;
    }
    if (valid && ! raster_grids_aligned(raster1, raster2, &reason))
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "%s", reason);
      valid = false;
    }
    if (! valid)
    {
      raster_destroy(raster1); raster_destroy(raster2);
      return NULL;
    }
    /* Two rasters sharing no pixel have no result, as two temporal numbers
     * sharing no time have none */
    rt_raster shared = NULL;
    if (rt_raster_from_two_rasters(raster1, raster2, ET_INTERSECTION, &shared,
        NULL) != ES_NONE || ! shared)
    {
      raster_destroy(raster1); raster_destroy(raster2);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Could not compute the extent the rasters share");
      return NULL;
    }
    bool empty = rt_raster_is_empty(shared);
    rt_raster_destroy(shared);
    if (empty)
    {
      raster_destroy(raster1); raster_destroy(raster2);
      return NULL;
    }
  }

  RasterArithArg args;
  args.oper = oper;
  args.number = (raster2 == NULL);
  args.invert = invert;
  args.value = d;
  args.divzero = false;

  rt_raster result = NULL;
  for (int i = 0; i < numbands; i++)
  {
    rt_band band1 = rt_raster_get_band(raster1, (uint32_t) i);
    rt_band band2 = raster2 ? rt_raster_get_band(raster2, (uint32_t) i) :
      NULL;
    if (! band1 || (raster2 && ! band2))
    {
      raster_destroy(raster1);
      if (raster2) raster_destroy(raster2);
      if (result) raster_destroy(result);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Could not read band %d of the rasters", i + 1);
      return NULL;
    }
    /* The result states a nodata value where an operand does, that of the
     * first operand having one */
    int hasnodata = 0;
    double nodataval = 0.0;
    if (rt_band_get_hasnodata_flag(band1))
    {
      hasnodata = 1;
      rt_band_get_nodata(band1, &nodataval);
    }
    else if (band2 && rt_band_get_hasnodata_flag(band2))
    {
      hasnodata = 1;
      rt_band_get_nodata(band2, &nodataval);
    }

    struct rt_iterator_t itrset[2];
    itrset[0].raster = raster1;
    itrset[0].nband = (uint16_t) i;
    itrset[0].nbnodata = 1;
    if (raster2)
    {
      itrset[1].raster = raster2;
      itrset[1].nband = (uint16_t) i;
      itrset[1].nbnodata = 1;
    }

    rt_raster banded = NULL;
    if (rt_raster_iterator(itrset, raster2 ? 2 : 1,
        raster2 ? ET_INTERSECTION : ET_FIRST, NULL, PT_64BF,
        (uint8_t) hasnodata, nodataval, 0, 0, NULL, &args,
        raster_arith_callback, &banded) != ES_NONE || ! banded)
    {
      raster_destroy(raster1);
      if (raster2) raster_destroy(raster2);
      if (result) raster_destroy(result);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Could not compute band %d of the result", i + 1);
      return NULL;
    }
    if (args.divzero)
    {
      raster_destroy(banded); raster_destroy(raster1);
      if (raster2) raster_destroy(raster2);
      if (result) raster_destroy(result);
      meos_error(ERROR, MEOS_ERR_DIVISION_BY_ZERO, "Division by zero");
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
        raster_destroy(banded); raster_destroy(raster1);
        if (raster2) raster_destroy(raster2);
        raster_destroy(result);
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "Could not add band %d to the result", i + 1);
        return NULL;
      }
      /* The band is held by the result now, so only its carrier is released */
      rt_raster_destroy(banded);
    }
  }

  raster_destroy(raster1);
  if (raster2) raster_destroy(raster2);
  rt_raster_set_srid(result, srid);
  return raster_serialize_destroy(result);
}

/**
 * @ingroup meos_raster_base_math
 * @brief Return the addition of a float and a raster, pixel by pixel
 * @param[in] d Value
 * @param[in] rast Raster
 * @return A raster of 64-bit float bands
 * @errval NULL
 * @csqlfn #Add_float_raster()
 */
Raster *
add_float_raster(double d, const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL);
  return raster_arithop(rast, NULL, d, ADD, INVERT);
}

/**
 * @ingroup meos_raster_base_math
 * @brief Return the addition of a raster and a float, pixel by pixel
 * @param[in] rast Raster
 * @param[in] d Value
 * @return A raster of 64-bit float bands
 * @errval NULL
 * @csqlfn #Add_raster_float()
 */
Raster *
add_raster_float(const Raster *rast, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL);
  return raster_arithop(rast, NULL, d, ADD, INVERT_NO);
}

/**
 * @ingroup meos_raster_base_math
 * @brief Return the addition of two rasters, pixel by pixel
 * @details The rasters are operated over the extent they share
 * @param[in] rast1,rast2 Rasters
 * @return A raster of 64-bit float bands, or NULL where the rasters share no
 * pixel
 * @errval NULL
 * @csqlfn #Add_raster_raster()
 */
Raster *
add_raster_raster(const Raster *rast1, const Raster *rast2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast1, NULL); VALIDATE_NOT_NULL(rast2, NULL);
  return raster_arithop(rast1, rast2, 0.0, ADD, INVERT_NO);
}

/**
 * @ingroup meos_raster_base_math
 * @brief Return the subtraction of a float and a raster, pixel by pixel
 * @param[in] d Value
 * @param[in] rast Raster
 * @return A raster of 64-bit float bands
 * @errval NULL
 * @csqlfn #Sub_float_raster()
 */
Raster *
sub_float_raster(double d, const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL);
  return raster_arithop(rast, NULL, d, SUB, INVERT);
}

/**
 * @ingroup meos_raster_base_math
 * @brief Return the subtraction of a raster and a float, pixel by pixel
 * @param[in] rast Raster
 * @param[in] d Value
 * @return A raster of 64-bit float bands
 * @errval NULL
 * @csqlfn #Sub_raster_float()
 */
Raster *
sub_raster_float(const Raster *rast, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL);
  return raster_arithop(rast, NULL, d, SUB, INVERT_NO);
}

/**
 * @ingroup meos_raster_base_math
 * @brief Return the subtraction of two rasters, pixel by pixel
 * @details The rasters are operated over the extent they share
 * @param[in] rast1,rast2 Rasters
 * @return A raster of 64-bit float bands, or NULL where the rasters share no
 * pixel
 * @errval NULL
 * @csqlfn #Sub_raster_raster()
 */
Raster *
sub_raster_raster(const Raster *rast1, const Raster *rast2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast1, NULL); VALIDATE_NOT_NULL(rast2, NULL);
  return raster_arithop(rast1, rast2, 0.0, SUB, INVERT_NO);
}

/**
 * @ingroup meos_raster_base_math
 * @brief Return the multiplication of a float and a raster, pixel by pixel
 * @param[in] d Value
 * @param[in] rast Raster
 * @return A raster of 64-bit float bands
 * @errval NULL
 * @csqlfn #Mul_float_raster()
 */
Raster *
mul_float_raster(double d, const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL);
  return raster_arithop(rast, NULL, d, MUL, INVERT);
}

/**
 * @ingroup meos_raster_base_math
 * @brief Return the multiplication of a raster and a float, pixel by pixel
 * @param[in] rast Raster
 * @param[in] d Value
 * @return A raster of 64-bit float bands
 * @errval NULL
 * @csqlfn #Mul_raster_float()
 */
Raster *
mul_raster_float(const Raster *rast, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL);
  return raster_arithop(rast, NULL, d, MUL, INVERT_NO);
}

/**
 * @ingroup meos_raster_base_math
 * @brief Return the multiplication of two rasters, pixel by pixel
 * @details The rasters are operated over the extent they share
 * @param[in] rast1,rast2 Rasters
 * @return A raster of 64-bit float bands, or NULL where the rasters share no
 * pixel
 * @errval NULL
 * @csqlfn #Mul_raster_raster()
 */
Raster *
mul_raster_raster(const Raster *rast1, const Raster *rast2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast1, NULL); VALIDATE_NOT_NULL(rast2, NULL);
  return raster_arithop(rast1, rast2, 0.0, MUL, INVERT_NO);
}

/**
 * @ingroup meos_raster_base_math
 * @brief Return the division of a float and a raster, pixel by pixel
 * @param[in] d Value
 * @param[in] rast Raster
 * @return A raster of 64-bit float bands
 * @errval NULL
 * @csqlfn #Div_float_raster()
 */
Raster *
div_float_raster(double d, const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL);
  return raster_arithop(rast, NULL, d, DIV, INVERT);
}

/**
 * @ingroup meos_raster_base_math
 * @brief Return the division of a raster and a float, pixel by pixel
 * @param[in] rast Raster
 * @param[in] d Value
 * @return A raster of 64-bit float bands
 * @errval NULL
 * @csqlfn #Div_raster_float()
 */
Raster *
div_raster_float(const Raster *rast, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL);
  return raster_arithop(rast, NULL, d, DIV, INVERT_NO);
}

/**
 * @ingroup meos_raster_base_math
 * @brief Return the division of two rasters, pixel by pixel
 * @details The rasters are operated over the extent they share
 * @param[in] rast1,rast2 Rasters
 * @return A raster of 64-bit float bands, or NULL where the rasters share no
 * pixel
 * @errval NULL
 * @csqlfn #Div_raster_raster()
 */
Raster *
div_raster_raster(const Raster *rast1, const Raster *rast2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast1, NULL); VALIDATE_NOT_NULL(rast2, NULL);
  return raster_arithop(rast1, rast2, 0.0, DIV, INVERT_NO);
}


/*****************************************************************************
 * Terrain functions
 *****************************************************************************/

/**
 * @brief Callback of rt_raster_iterator computing a pixel
 */
typedef int (*RasterPixelFn)(rt_iterator_arg arg, void *userarg,
  double *value, int *nodata);

/**
 * @brief Units of a slope
 */
typedef enum
{
  SLOPE_DEGREES,
  SLOPE_RADIANS,
  SLOPE_PERCENT,
} SlopeUnits;

/**
 * @brief Arguments of the callback computing the slope of a pixel
 */
typedef struct
{
  double pixwidth;    /**< Width of a pixel in the units of the raster */
  double pixheight;   /**< Height of a pixel in the units of the raster */
  double scale;       /**< Ratio of the vertical units to the horizontal ones */
  SlopeUnits units;   /**< Units of the result */
} RasterSlopeArg;

/**
 * @brief Read the 3x3 window of a pixel for a terrain function
 * @details The window of PostGIS's `_st_slope4ma`, `_st_tri4ma` and
 * `_st_tpi4ma`: a neighbour that is nodata, or that lies outside the raster,
 * takes the value of the center pixel, so an edge pixel has a value
 * @param[in] arg Neighbourhood the iterator reads, of one raster
 * @param[out] z Values of the window, row by row from the north
 * @return False where the center pixel is nodata, and the result with it
 */
static bool
raster_terrain_window(rt_iterator_arg arg, double z[3][3])
{
  if (arg->nodata[0][1][1])
    return false;
  double center = arg->values[0][1][1];
  for (int row = 0; row < 3; row++)
    for (int col = 0; col < 3; col++)
      z[row][col] = arg->nodata[0][row][col] ? center :
        arg->values[0][row][col];
  return true;
}

/**
 * @brief Compute the slope of a pixel
 * @details The formula of PostGIS's `_st_slope4ma`: the gradients are Horn's,
 * the rows weighted 1, 2, 1 across the pixel height and the columns across its
 * width, and the slope is their norm divided by 8 and by the scale. PostGIS
 * compares the upper case units to `'rad'`, so its radians answer degrees;
 * here they answer radians
 */
static int
/* cppcheck-suppress constParameterCallback */
raster_slope_callback(rt_iterator_arg arg, void *userarg, double *value,
  int *nodata)
{
  const RasterSlopeArg *args = (const RasterSlopeArg *) userarg;
  double z[3][3];
  *value = 0.0;
  if (! raster_terrain_window(arg, z))
  {
    *nodata = 1;
    return 1;
  }
  double dz_dy = ((z[2][0] + 2.0 * z[2][1] + z[2][2]) -
    (z[0][0] + 2.0 * z[0][1] + z[0][2])) / args->pixheight;
  double dz_dx = ((z[0][2] + 2.0 * z[1][2] + z[2][2]) -
    (z[0][0] + 2.0 * z[1][0] + z[2][0])) / args->pixwidth;
  double slope = sqrt(dz_dx * dz_dx + dz_dy * dz_dy) / (8.0 * args->scale);
  switch (args->units)
  {
    case SLOPE_PERCENT:
      *value = 100.0 * slope;
      break;
    case SLOPE_RADIANS:
      *value = atan(slope);
      break;
    default: /* SLOPE_DEGREES */
      *value = atan(slope) * 180.0 / M_PI;
  }
  *nodata = 0;
  return 1;
}

/**
 * @brief Compute the topographic position index of a pixel
 * @details The formula of PostGIS's `_st_tpi4ma`: the value of the pixel
 * minus the mean of its eight neighbours
 */
static int
raster_tpi_callback(rt_iterator_arg arg,
  void *userarg __attribute__((unused)), double *value, int *nodata)
{
  double z[3][3];
  *value = 0.0;
  if (! raster_terrain_window(arg, z))
  {
    *nodata = 1;
    return 1;
  }
  double sum = 0.0;
  for (int row = 0; row < 3; row++)
    for (int col = 0; col < 3; col++)
      if (row != 1 || col != 1)
        sum += z[row][col];
  *value = z[1][1] - sum / 8.0;
  *nodata = 0;
  return 1;
}

/**
 * @brief Compute the terrain ruggedness index of a pixel
 * @details The formula of PostGIS's `_st_tri4ma`: the mean of the absolute
 * differences between the pixel and its eight neighbours
 */
static int
raster_tri_callback(rt_iterator_arg arg,
  void *userarg __attribute__((unused)), double *value, int *nodata)
{
  double z[3][3];
  *value = 0.0;
  if (! raster_terrain_window(arg, z))
  {
    *nodata = 1;
    return 1;
  }
  double sum = 0.0;
  for (int row = 0; row < 3; row++)
    for (int col = 0; col < 3; col++)
      if (row != 1 || col != 1)
        sum += fabs(z[row][col] - z[1][1]);
  *value = sum / 8.0;
  *nodata = 0;
  return 1;
}

/**
 * @brief Interpolate a nodata pixel from its neighbours
 * @details The formula of PostGIS's `st_invdistweight4ma` with the arguments
 * its terrain functions pass, a power of 1 and no weight for the center
 * pixel: a pixel with a value keeps it, and a nodata pixel takes the mean of
 * the neighbours with a value, each weighted by the inverse of its distance to
 * the pixel, or stays nodata where none has one
 */
static int
raster_idw_callback(rt_iterator_arg arg,
  void *userarg __attribute__((unused)), double *value, int *nodata)
{
  *value = 0.0;
  if (! arg->nodata[0][1][1])
  {
    *value = arg->values[0][1][1];
    *nodata = 0;
    return 1;
  }
  double sum = 0.0, weights = 0.0;
  for (int row = 0; row < 3; row++)
  {
    for (int col = 0; col < 3; col++)
    {
      if ((row == 1 && col == 1) || arg->nodata[0][row][col])
        continue;
      double weight = 1.0 / hypot((double) (col - 1), (double) (row - 1));
      sum += weight * arg->values[0][row][col];
      weights += weight;
    }
  }
  if (weights == 0.0)
  {
    *nodata = 1;
    return 1;
  }
  *value = sum / weights;
  *nodata = 0;
  return 1;
}

/**
 * @brief Return a terrain function of a raster band, pixel by pixel
 * @details The terrain functions of PostGIS, `ST_Slope`, `ST_TRI` and
 * `ST_TPI`, read each pixel with its eight neighbours through
 * `rt_raster_iterator`, as #raster_arithop reads one pixel. Where nodata
 * pixels are interpolated, the band is first read by #raster_idw_callback, as
 * PostGIS reads it by `st_invdistweight4ma`, and the terrain function then
 * reads the interpolated band. The result keeps the nodata value of the band
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @param[in] customextent Raster whose extent the result takes, or NULL for
 * the extent of @p rast @nullable
 * @param[in] pixeltype Name of the pixel type of the result
 * @param[in] interpolate_nodata True to interpolate the nodata pixels first
 * @param[in] callback Terrain function of a pixel
 * @param[in] userarg Arguments of @p callback
 * @errval NULL
 */
static Raster *
raster_terrain(const Raster *rast, int band, const Raster *customextent,
  const char *pixeltype, bool interpolate_nodata, RasterPixelFn callback,
  void *userarg)
{
  rt_pixtype pixtype = rt_pixtype_index_from_name(pixeltype);
  if (pixtype == PT_END)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Unknown pixel type: %s", pixeltype);
    return NULL;
  }

  /* The bands are read, so the rasters are deserialized in full */
  rt_raster raster = rt_raster_deserialize((void *) rast, 0);
  if (! raster)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }
  int numbands = (int) rt_raster_get_num_bands(raster);
  if (band < 1 || band > numbands)
  {
    raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The raster has no band %d, it has %d", band, numbands);
    return NULL;
  }
  int32_t srid = (int32_t) rt_raster_get_srid(raster);

  rt_raster extent = NULL;
  if (customextent)
  {
    /* Only the extent is read, so the header suffices */
    extent = raster_header(customextent);
    const char *reason = NULL;
    if (! extent)
    {
      raster_destroy(raster);
      return NULL;
    }
    if (! ensure_same_srid(srid, (int32_t) rt_raster_get_srid(extent)) ||
        ! raster_grids_aligned(raster, extent, &reason))
    {
      rt_raster_destroy(extent); raster_destroy(raster);
      if (reason)
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "%s", reason);
      return NULL;
    }
  }

  /* The result states a nodata value where the band does */
  rt_band rtband = rt_raster_get_band(raster, (uint32_t) (band - 1));
  int hasnodata = rtband ? rt_band_get_hasnodata_flag(rtband) : 0;
  double nodataval = 0.0;
  if (hasnodata)
    rt_band_get_nodata(rtband, &nodataval);

  struct rt_iterator_t itrset[1];
  itrset[0].raster = raster;
  itrset[0].nband = (uint16_t) (band - 1);
  itrset[0].nbnodata = 1;

  rt_raster interpolated = NULL;
  if (interpolate_nodata)
  {
    if (rt_raster_iterator(itrset, 1, ET_FIRST, NULL, pixtype,
        (uint8_t) hasnodata, nodataval, 1, 1, NULL, NULL,
        raster_idw_callback, &interpolated) != ES_NONE || ! interpolated)
    {
      if (extent) rt_raster_destroy(extent);
      raster_destroy(raster);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Could not interpolate the nodata pixels of band %d", band);
      return NULL;
    }
    itrset[0].raster = interpolated;
    itrset[0].nband = 0;
  }

  rt_raster result = NULL;
  rt_errorstate status = rt_raster_iterator(itrset, 1,
    extent ? ET_CUSTOM : ET_FIRST, extent, pixtype, (uint8_t) hasnodata,
    nodataval, 1, 1, NULL, userarg, callback, &result);
  if (interpolated) raster_destroy(interpolated);
  if (extent) rt_raster_destroy(extent);
  raster_destroy(raster);
  if (status != ES_NONE || ! result)
  {
    if (result) raster_destroy(result);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not compute band %d of the result", band);
    return NULL;
  }
  rt_raster_set_srid(result, srid);
  return raster_serialize_destroy(result);
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Return the slope of a raster band, pixel by pixel
 * @details The slope of PostGIS's `ST_Slope`, from the gradients of each pixel
 * and its eight neighbours, in degrees, radians or percent. The units are read
 * as PostGIS reads them, by their first three letters regardless of case,
 * and units it does not know raise an error
 * @param[in] rast Raster of elevations
 * @param[in] band Number of the band, starting at 1
 * @param[in] customextent Raster whose extent the result takes, or NULL for
 * the extent of @p rast @nullable
 * @param[in] pixeltype Name of the pixel type of the result, such as `32BF`
 * @param[in] units Units of the result: `DEGREES`, `RADIANS` or `PERCENT`
 * @param[in] scale Ratio of the vertical units to the horizontal ones, such as
 * 111120 for elevations in meters over a grid in degrees
 * @param[in] interpolate_nodata True to interpolate the nodata pixels first
 * @errval NULL
 * @csqlfn #Raster_slope(), #Raster_slope_extent()
 */
Raster *
raster_slope(const Raster *rast, int band, const Raster *customextent,
  const char *pixeltype, const char *units, double scale,
  bool interpolate_nodata)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(pixeltype, NULL);
  VALIDATE_NOT_NULL(units, NULL);
  if (scale == 0.0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The scale of a slope cannot be zero");
    return NULL;
  }

  /* PostGIS reads the first three letters of the units after any leading
   * space */
  const char *start = units;
  while (*start == ' ')
    start++;
  char prefix[4] = {0};
  for (int i = 0; i < 3 && start[i]; i++)
    prefix[i] = (char) toupper((unsigned char) start[i]);
  RasterSlopeArg args;
  if (strcmp(prefix, "DEG") == 0)
    args.units = SLOPE_DEGREES;
  else if (strcmp(prefix, "RAD") == 0)
    args.units = SLOPE_RADIANS;
  else if (strcmp(prefix, "PER") == 0)
    args.units = SLOPE_PERCENT;
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Unknown units of a slope: %s", units);
    return NULL;
  }

  /* The width and the height of a pixel are those of PostGIS's ST_PixelWidth
   * and ST_PixelHeight, which a skew lengthens */
  rt_raster header = raster_header(rast);
  if (! header)
    return NULL;
  args.pixwidth = hypot(rt_raster_get_x_scale(header),
    rt_raster_get_y_skew(header));
  args.pixheight = hypot(rt_raster_get_y_scale(header),
    rt_raster_get_x_skew(header));
  rt_raster_destroy(header);
  args.scale = scale;

  return raster_terrain(rast, band, customextent, pixeltype,
    interpolate_nodata, raster_slope_callback, &args);
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Return the terrain ruggedness index of a raster band, pixel by pixel
 * @details The index of PostGIS's `ST_TRI`: the mean of the absolute
 * differences between a pixel and its eight neighbours
 * @param[in] rast Raster of elevations
 * @param[in] band Number of the band, starting at 1
 * @param[in] customextent Raster whose extent the result takes, or NULL for
 * the extent of @p rast @nullable
 * @param[in] pixeltype Name of the pixel type of the result, such as `32BF`
 * @param[in] interpolate_nodata True to interpolate the nodata pixels first
 * @errval NULL
 * @csqlfn #Raster_tri(), #Raster_tri_extent()
 */
Raster *
raster_tri(const Raster *rast, int band, const Raster *customextent,
  const char *pixeltype, bool interpolate_nodata)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(pixeltype, NULL);
  return raster_terrain(rast, band, customextent, pixeltype,
    interpolate_nodata, raster_tri_callback, NULL);
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Return the topographic position index of a raster band
 * @details The index of PostGIS's `ST_TPI`: the value of a pixel minus the
 * mean of its eight neighbours
 * @param[in] rast Raster of elevations
 * @param[in] band Number of the band, starting at 1
 * @param[in] customextent Raster whose extent the result takes, or NULL for
 * the extent of @p rast @nullable
 * @param[in] pixeltype Name of the pixel type of the result, such as `32BF`
 * @param[in] interpolate_nodata True to interpolate the nodata pixels first
 * @errval NULL
 * @csqlfn #Raster_tpi(), #Raster_tpi_extent()
 */
Raster *
raster_tpi(const Raster *rast, int band, const Raster *customextent,
  const char *pixeltype, bool interpolate_nodata)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(pixeltype, NULL);
  return raster_terrain(rast, band, customextent, pixeltype,
    interpolate_nodata, raster_tpi_callback, NULL);
}

/*****************************************************************************
 * Statistics over a neighbourhood
 *****************************************************************************/

/**
 * @brief Statistic of the pixels of a neighbourhood
 */
typedef enum
{
  MAPALG_MIN,
  MAPALG_MAX,
  MAPALG_SUM,
  MAPALG_MEAN,
  MAPALG_RANGE,
  MAPALG_STDDEV,
} RasterMapAlgStat;

/**
 * @brief Compute a statistic of the neighbourhood of a pixel
 * @details A callback of `rt_raster_iterator` as #raster_tpi_callback is, over
 * a neighbourhood of any size. The statistics of PostGIS's `st_min4ma`,
 * `st_max4ma`, `st_sum4ma`, `st_mean4ma`, `st_range4ma` and `st_stddev4ma`
 * without user arguments: the nodata pixels of the neighbourhood, and those
 * outside the raster, are skipped; the sum of no pixel is 0, every other
 * statistic of no pixel is nodata, and the standard deviation is the sample
 * one of the SQL aggregate `stddev`, nodata for fewer than two pixels
 */
static int
/* cppcheck-suppress constParameterCallback */
raster_map_algebra_callback(rt_iterator_arg arg, void *userarg, double *value,
  int *nodata)
{
  RasterMapAlgStat stat = *(const RasterMapAlgStat *) userarg;
  /* The squared deviations accumulate as PostgreSQL's float8_accum does, by
   * the update of Youngs and Cramer, row by row as unnest reads the window */
  double min = INFINITY, max = -INFINITY, sum = 0.0, sqdev = 0.0;
  int count = 0;
  for (uint32_t row = 0; row < arg->rows; row++)
  {
    for (uint32_t col = 0; col < arg->columns; col++)
    {
      if (arg->nodata[0][row][col])
        continue;
      double v = arg->values[0][row][col];
      if (v < min)
        min = v;
      if (v > max)
        max = v;
      sum += v;
      count++;
      if (count > 1)
      {
        double tmp = v * count - sum;
        sqdev += tmp * tmp / ((double) count * (count - 1));
      }
    }
  }

  *value = 0.0;
  *nodata = 0;
  if (stat == MAPALG_SUM)
  {
    *value = sum;
    return 1;
  }
  if (count == 0 || (stat == MAPALG_STDDEV && count < 2))
  {
    *nodata = 1;
    return 1;
  }
  switch (stat)
  {
    case MAPALG_MIN:
      *value = min;
      break;
    case MAPALG_MAX:
      *value = max;
      break;
    case MAPALG_MEAN:
      *value = sum / count;
      break;
    case MAPALG_RANGE:
      *value = max - min;
      break;
    default: /* MAPALG_STDDEV */
      *value = sqrt(sqdev / (count - 1));
  }
  return 1;
}

/**
 * @brief Return the position of a name in a list of names, regardless of case
 * @details The upper case lookup #raster_resample_alg makes of an algorithm,
 * over any list of names; the family holds no lookup of its own to call
 * @return The position, or -1 where the list does not hold the name
 */
static int
raster_name_index(const char *name, const char * const *names, int count)
{
  size_t len = strlen(name);
  char *upper = palloc(len + 1);
  for (size_t i = 0; i < len; i++)
    upper[i] = (char) toupper((unsigned char) name[i]);
  upper[len] = '\0';
  int result = -1;
  for (int i = 0; i < count; i++)
  {
    if (strcmp(upper, names[i]) == 0)
    {
      result = i;
      break;
    }
  }
  pfree(upper);
  return result;
}

/**
 * @ingroup meos_raster_base_transf
 * @brief Return a statistic of the neighbourhood of each pixel of a band
 * @details The focal statistics PostGIS computes by `ST_MapAlgebra` with the
 * callbacks `st_min4ma`, `st_max4ma`, `st_sum4ma`, `st_mean4ma`,
 * `st_range4ma` and `st_stddev4ma`, each named here by its statistic as
 * `ST_Union` names its operations, read through `rt_raster_iterator` as
 * #raster_terrain reads a 3x3 neighbourhood. The neighbourhood of a pixel
 * extends @p distancex pixels to the west and to the east and @p distancey
 * pixels to the north and to the south; its nodata pixels, and those outside
 * the raster, are skipped. The sum of no pixel is 0, every other statistic of
 * no pixel is nodata, and the standard deviation is the sample one, nodata for
 * fewer than two pixels. As for PostGIS, which reads the neighbourhood through
 * the same iterator, a neighbourhood whose distance is zero along one axis
 * only is the pixel alone. The result states the nodata value of the band, or
 * its minimum value where it states none, as PostGIS's does.
 *
 * Over one raster every extent type but `CUSTOM` is the extent of the raster;
 * `CUSTOM` takes the extent of @p customextent, which is otherwise not read.
 * A statistic or an extent type outside those raises an error, where PostGIS
 * reads an unknown extent type as `INTERSECTION`
 * @param[in] rast Raster
 * @param[in] band Number of the band, starting at 1
 * @param[in] stat Statistic: `MIN`, `MAX`, `SUM`, `MEAN`, `RANGE` or
 * `STDDEV`, regardless of case
 * @param[in] pixeltype Name of the pixel type of the result, or NULL for the
 * pixel type of the band @nullable
 * @param[in] extenttype Extent of the result: `INTERSECTION`, `UNION`,
 * `FIRST`, `SECOND`, `LAST` or `CUSTOM`, regardless of case, or NULL for
 * `FIRST` @nullable
 * @param[in] customextent Raster whose extent the result takes where @nullable
 * @p extenttype is `CUSTOM`
 * @param[in] distancex Number of pixels of the neighbourhood on each side of
 * a pixel along X
 * @param[in] distancey Number of pixels of the neighbourhood on each side of
 * a pixel along Y
 * @errval NULL
 * @csqlfn #Raster_map_algebra()
 */
Raster *
raster_map_algebra(const Raster *rast, int band, const char *stat,
  const char *pixeltype, const char *extenttype, const Raster *customextent,
  int32 distancex, int32 distancey)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, NULL); VALIDATE_NOT_NULL(stat, NULL);
  if (distancex < 0 || distancey < 0 || distancex > UINT16_MAX ||
      distancey > UINT16_MAX)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The distances of a neighbourhood must be between 0 and %d",
      UINT16_MAX);
    return NULL;
  }
  static const char * const statnames[] = {"MIN", "MAX", "SUM", "MEAN",
    "RANGE", "STDDEV"};
  int statnum = raster_name_index(stat, statnames,
    (int) (sizeof(statnames) / sizeof(statnames[0])));
  if (statnum < 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Unknown statistic of a neighbourhood: %s", stat);
    return NULL;
  }
  RasterMapAlgStat statarg = (RasterMapAlgStat) statnum;
  bool custom = false;
  if (extenttype)
  {
    static const char * const extentnames[] = {"INTERSECTION", "UNION",
      "FIRST", "SECOND", "LAST", "CUSTOM"};
    int extentnum = raster_name_index(extenttype, extentnames,
      (int) (sizeof(extentnames) / sizeof(extentnames[0])));
    if (extentnum < 0)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Unknown extent type: %s", extenttype);
      return NULL;
    }
    custom = (extentnum == 5);
  }
  if (custom && ! customextent)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The extent type CUSTOM needs a custom extent");
    return NULL;
  }
  rt_pixtype pixtype = PT_END;
  if (pixeltype)
  {
    pixtype = rt_pixtype_index_from_name(pixeltype);
    if (pixtype == PT_END)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Unknown pixel type: %s", pixeltype);
      return NULL;
    }
  }

  /* The bands are read, so the raster is deserialized in full */
  rt_raster raster = rt_raster_deserialize((void *) rast, 0);
  if (! raster)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }
  int numbands = (int) rt_raster_get_num_bands(raster);
  if (band < 1 || band > numbands)
  {
    raster_destroy(raster);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The raster has no band %d, it has %d", band, numbands);
    return NULL;
  }
  int32_t srid = (int32_t) rt_raster_get_srid(raster);

  rt_raster extent = NULL;
  if (custom)
  {
    /* Only the extent is read, so the header suffices */
    extent = raster_header(customextent);
    const char *reason = NULL;
    if (! extent)
    {
      raster_destroy(raster);
      return NULL;
    }
    if (! ensure_same_srid(srid, (int32_t) rt_raster_get_srid(extent)) ||
        ! raster_grids_aligned(raster, extent, &reason))
    {
      rt_raster_destroy(extent); raster_destroy(raster);
      if (reason)
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "%s", reason);
      return NULL;
    }
  }

  /* The result states the nodata value of the band, or its minimum value */
  rt_band rtband = rt_raster_get_band(raster, (uint32_t) (band - 1));
  if (pixtype == PT_END)
    pixtype = rt_band_get_pixtype(rtband);
  double nodataval;
  if (rt_band_get_hasnodata_flag(rtband))
    rt_band_get_nodata(rtband, &nodataval);
  else
    nodataval = rt_band_get_min_value(rtband);

  struct rt_iterator_t itrset[1];
  itrset[0].raster = raster;
  itrset[0].nband = (uint16_t) (band - 1);
  itrset[0].nbnodata = 1;

  rt_raster result = NULL;
  rt_errorstate status = rt_raster_iterator(itrset, 1,
    extent ? ET_CUSTOM : ET_FIRST, extent, pixtype, 1, nodataval,
    (uint16_t) distancex, (uint16_t) distancey, NULL, &statarg,
    raster_map_algebra_callback, &result);
  if (extent) rt_raster_destroy(extent);
  raster_destroy(raster);
  if (status != ES_NONE || ! result)
  {
    if (result) raster_destroy(result);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not compute band %d of the result", band);
    return NULL;
  }
  rt_raster_set_srid(result, srid);
  return raster_serialize_destroy(result);
}

/*****************************************************************************
 * Union aggregate
 *****************************************************************************/

/**
 * @brief Pixel operations of a union of rasters
 * @details Those PostGIS's `ST_Union` names, in its order
 */
typedef enum
{
  UNIONTYPE_LAST,
  UNIONTYPE_FIRST,
  UNIONTYPE_MIN,
  UNIONTYPE_MAX,
  UNIONTYPE_COUNT,
  UNIONTYPE_SUM,
  UNIONTYPE_MEAN,
  UNIONTYPE_RANGE,
} RasterUnionType;

/**
 * @brief Unite a pixel of the union built so far with a pixel of a raster
 * @details A callback of `rt_raster_iterator` over two rasters, the union
 * built so far and the raster read, computing the pixel operation of
 * PostGIS's `rtpg_union_callback`: where one of the two pixels is nodata the
 * other one is the result, and where both are the result is nodata, but for
 * `COUNT`, which counts the pixels holding a value. `MEAN` and `RANGE` are
 * united as their two parts, a count and a sum, a minimum and a maximum
 */
static int
/* cppcheck-suppress constParameterCallback */
raster_union_callback(rt_iterator_arg arg, void *userarg, double *value,
  int *nodata)
{
  RasterUnionType utype = *(RasterUnionType *) userarg;
  double value0 = arg->values[0][0][0], value1 = arg->values[1][0][0];
  int nodata0 = arg->nodata[0][0][0], nodata1 = arg->nodata[1][0][0];
  *value = 0;
  *nodata = 0;
  if (utype != UNIONTYPE_COUNT)
  {
    if (nodata0 && nodata1)
    {
      *nodata = 1;
      return 1;
    }
    if (nodata1)
    {
      *value = value0;
      return 1;
    }
    if (nodata0)
    {
      *value = value1;
      return 1;
    }
  }
  switch (utype)
  {
    case UNIONTYPE_LAST:
      *value = value1;
      break;
    case UNIONTYPE_FIRST:
      *value = value0;
      break;
    case UNIONTYPE_MIN:
      *value = value0 < value1 ? value0 : value1;
      break;
    case UNIONTYPE_MAX:
      *value = value0 > value1 ? value0 : value1;
      break;
    case UNIONTYPE_COUNT:
      if (nodata0 && nodata1)
        *value = 0;
      else if (nodata1)
        *value = value0;
      else if (nodata0)
        *value = 1;
      else
        *value = value0 + 1;
      break;
    case UNIONTYPE_SUM:
      *value = value0 + value1;
      break;
    case UNIONTYPE_MEAN:
    case UNIONTYPE_RANGE:
      /* Never reached: a mean and a range are united as their two parts */
      return 0;
  }
  return 1;
}

/**
 * @brief Compute the mean or the range of a pixel of a union from its parts
 * @details A callback of `rt_raster_iterator` over the two parts of a union,
 * the mean of PostGIS's `rtpg_union_mean_callback`, the sum divided by the
 * count, and the range of its `rtpg_union_range_callback`, the maximum minus
 * the minimum. A pixel missing a part, or counted by no raster, is nodata
 */
static int
/* cppcheck-suppress constParameterCallback */
raster_union_final_callback(rt_iterator_arg arg, void *userarg,
  double *value, int *nodata)
{
  RasterUnionType utype = *(RasterUnionType *) userarg;
  *value = 0;
  *nodata = 1;
  if (arg->nodata[0][0][0] || arg->nodata[1][0][0])
    return 1;
  if (utype == UNIONTYPE_MEAN)
  {
    if (FLT_NEQ(arg->values[0][0][0], 0.0))
    {
      *value = arg->values[1][0][0] / arg->values[0][0][0];
      *nodata = 0;
    }
  }
  else
  {
    *value = arg->values[1][0][0] - arg->values[0][0][0];
    *nodata = 0;
  }
  return 1;
}

/**
 * @brief Copy the pixels of a band into another band from a pixel on
 */
static bool
raster_union_copy_band(rt_band from, uint16_t width, uint16_t height,
  rt_band to, int x, int y)
{
  for (uint16_t row = 0; row < height; row++)
  {
    void *vals = NULL;
    uint16_t nvals = 0;
    if (rt_band_get_pixel_line(from, 0, row, width, &vals, &nvals) != ES_NONE)
      return false;
    rt_errorstate status = rt_band_set_pixel_line(to, x, y + row, vals,
      nvals);
    rtdealloc(vals);
    if (status != ES_NONE)
      return false;
  }
  return true;
}

/**
 * @brief Return the union built so far of a band, united with a band of a
 * raster
 * @details The step of PostGIS's `RASTER_union_transfn` for one band and one
 * part. Where both rasters hold the band, the result covers the union of
 * their extents, holding the union built so far and, over the extent of the
 * raster read, the pixel operation of the two; otherwise the iterator reads
 * the union of their extents, a missing band read as nodata where
 * @p nbnodata is true. The pixel type and the nodata value are those of the
 * band of the union, or of the band read, or of a 64-bit float band, its
 * minimum value standing for nodata where the band states none, but for a
 * count, an unsigned 32-bit band without a nodata value
 * @param[in] work Union built so far, NULL before the first raster
 * @param[in] raster Raster read
 * @param[in] nband Band of the raster read, starting at 0
 * @param[in] utype Pixel operation
 * @param[in] counts True when both bands are counts, united by their sum
 * @param[in] nbnodata True when a missing band is read as nodata
 * @return The new union, NULL when neither raster is given
 * @errval NULL
 */
static rt_raster
raster_union_step(rt_raster work, rt_raster raster, uint16_t nband,
  RasterUnionType utype, bool counts, bool nbnodata)
{
  bool hasband0 = ! rt_raster_is_empty(work) && rt_raster_has_band(work, 0);
  bool hasband1 = ! rt_raster_is_empty(raster) &&
    rt_raster_has_band(raster, nband);
  rt_band band = hasband0 ? rt_raster_get_band(work, 0) :
    (hasband1 ? rt_raster_get_band(raster, nband) : NULL);
  rt_pixtype pixtype = PT_64BF;
  int hasnodata = 1;
  double nodataval = rt_pixtype_get_min_value(PT_64BF);
  if (band)
  {
    pixtype = rt_band_get_pixtype(band);
    if (rt_band_get_hasnodata_flag(band))
      rt_band_get_nodata(band, &nodataval);
    else
      nodataval = rt_band_get_min_value(band);
  }
  if (utype == UNIONTYPE_COUNT)
  {
    pixtype = PT_32BUI;
    hasnodata = 0;
    nodataval = 0;
  }
  RasterUnionType op = counts ? UNIONTYPE_SUM : utype;

  struct rt_iterator_t itrset[2];
  itrset[0].raster = work;
  itrset[0].nband = 0;
  itrset[0].nbnodata = nbnodata ? 1 : 0;
  itrset[1].raster = raster;
  itrset[1].nband = nband;
  itrset[1].nbnodata = nbnodata ? 1 : 0;

  rt_raster result = NULL;
  if (! hasband0 || ! hasband1)
  {
    if (rt_raster_iterator(itrset, 2, ET_UNION, NULL, pixtype, hasnodata,
        nodataval, 0, 0, NULL, &op, raster_union_callback, &result) !=
          ES_NONE)
    {
      if (result) raster_destroy(result);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Could not unite the rasters");
      return NULL;
    }
    return result;
  }

  /* Both rasters hold the band: the union of their extents receives the
   * union built so far, then the pixel operation over the raster read */
  double offset[4] = {0};
  if (rt_raster_from_two_rasters(work, raster, ET_UNION, &result, offset) !=
        ES_NONE || ! result)
  {
    if (result) rt_raster_destroy(result);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not compute the extent of the union");
    return NULL;
  }
  rt_raster piece = NULL;
  bool ok = rt_raster_generate_new_band(result, pixtype, nodataval,
    hasnodata, nodataval, 0) != -1;
  rt_band rband = ok ? rt_raster_get_band(result, 0) : NULL;
  ok = ok && raster_union_copy_band(band, rt_raster_get_width(work),
    rt_raster_get_height(work), rband, (int) offset[0], (int) offset[1]);
  ok = ok && rt_raster_iterator(itrset, 2, ET_LAST, NULL, pixtype,
    hasnodata, nodataval, 0, 0, NULL, &op, raster_union_callback, &piece) ==
      ES_NONE && piece;
  ok = ok && raster_union_copy_band(rt_raster_get_band(piece, 0),
    rt_raster_get_width(piece), rt_raster_get_height(piece), rband,
    (int) offset[2], (int) offset[3]);
  if (piece) raster_destroy(piece);
  if (! ok)
  {
    raster_destroy(result);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not unite the rasters");
    return NULL;
  }
  return result;
}

/**
 * @brief Return the raster of a part of a union, NULL where none is built
 */
static rt_raster
raster_union_part(const RasterUnionState *state, int i)
{
  if (! state->rasters[i])
    return NULL;
  rt_raster result = rt_raster_deserialize((void *) state->rasters[i], 0);
  if (! result)
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
  return result;
}

/**
 * @brief Replace a part of a union by a raster, which is freed
 */
static bool
raster_union_set_part(RasterUnionState *state, int i, rt_raster raster)
{
  Raster *part = NULL;
  if (raster)
  {
    part = raster_serialize_destroy(raster);
    if (! part)
      return false;
  }
  if (state->rasters[i])
    pfree(state->rasters[i]);
  state->rasters[i] = part;
  return true;
}

/**
 * @brief Return the number of parts of each band of a union
 */
static inline int
raster_union_numparts(const RasterUnionState *state)
{
  return (state->uniontype == UNIONTYPE_MEAN ||
    state->uniontype == UNIONTYPE_RANGE) ? 2 : 1;
}

/**
 * @brief Return the pixel operation of a part of a band of a union
 * @details A mean is a count and a sum, a range a minimum and a maximum, as
 * PostGIS keeps them
 */
static RasterUnionType
raster_union_part_type(const RasterUnionState *state, int part)
{
  if (state->uniontype == UNIONTYPE_MEAN)
    return part == 0 ? UNIONTYPE_COUNT : UNIONTYPE_SUM;
  if (state->uniontype == UNIONTYPE_RANGE)
    return part == 0 ? UNIONTYPE_MIN : UNIONTYPE_MAX;
  return (RasterUnionType) state->uniontype;
}

/**
 * @brief Give a union more bands
 * @details A band added after the first raster starts from the extent of the
 * union of the first band, without pixels, as PostGIS starts it from a
 * shallow clone of that union
 */
static bool
raster_union_grow(RasterUnionState *state, int numbands)
{
  if (numbands <= state->numbands)
    return true;
  Raster **rasters = palloc0(sizeof(Raster *) * 2 * numbands);
  if (state->numbands)
  {
    memcpy(rasters, state->rasters, sizeof(Raster *) * 2 * state->numbands);
    pfree(state->rasters);
  }
  state->rasters = rasters;
  Raster *extent = NULL;
  if (state->numbands && state->rasters[0])
  {
    rt_raster header = raster_header(state->rasters[0]);
    if (! header)
      return false;
    if (! rt_raster_is_empty(header))
    {
      rt_raster clone = rt_raster_clone(header, 0);
      rt_raster_destroy(header);
      extent = raster_serialize_destroy(clone);
      if (! extent)
        return false;
    }
    else
      rt_raster_destroy(header);
  }
  int numparts = raster_union_numparts(state);
  for (int i = state->numbands; i < numbands; i++)
    for (int j = 0; j < numparts; j++)
      state->rasters[2 * i + j] = extent ? raster_copy(extent) : NULL;
  if (extent)
    pfree(extent);
  state->numbands = numbands;
  return true;
}

/**
 * @brief Ensure that a raster can be united with the union built so far
 */
static bool
raster_union_ensure_valid(const RasterUnionState *state, rt_raster raster)
{
  if (rt_raster_is_empty(raster))
    return true;
  for (int i = 0; i < 2 * state->numbands; i++)
  {
    if (! state->rasters[i])
      continue;
    rt_raster header = raster_header(state->rasters[i]);
    if (! header)
      return false;
    const char *reason = NULL;
    bool valid = ensure_same_srid((int32_t) rt_raster_get_srid(header),
      (int32_t) rt_raster_get_srid(raster));
    if (valid && ! rt_raster_is_empty(header) &&
        ! raster_grids_aligned(header, raster, &reason))
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "%s", reason);
      valid = false;
    }
    rt_raster_destroy(header);
    return valid;
  }
  return true;
}

/**
 * @brief Free the state of the aggregate uniting rasters
 */
static void
rasterunionstate_free(RasterUnionState *state)
{
  for (int i = 0; i < 2 * state->numbands; i++)
    if (state->rasters[i])
      pfree(state->rasters[i]);
  if (state->rasters)
    pfree(state->rasters);
  pfree(state);
}

/**
 * @ingroup meos_raster_base_agg
 * @brief Transition function of the aggregate uniting rasters
 * @details The aggregate of PostGIS's `ST_Union(raster)`, keeping per band
 * the union built so far as PostGIS's `RASTER_union_transfn` keeps it: the
 * union of the extents of the rasters read, each pixel the pixel operation
 * of the pixels of the rasters holding a value there. A `MEAN` keeps a count
 * and a sum and a `RANGE` a minimum and a maximum, which the final function
 * reads.
 *
 * With @p band 0 every band is united, a band a raster lacks read as nodata;
 * otherwise only that band, which every raster must hold. The band and the
 * pixel operation are those of the first raster read. A NULL raster leaves
 * the state as it is, rasters in different reference systems or on different
 * grids raise an error, and so does a pixel operation outside those named,
 * where PostGIS reads it as `LAST`
 * @param[in,out] state Current aggregate state, NULL before the first raster @nullable
 * @param[in] rast Raster @nullable
 * @param[in] band Number of the band, starting at 1, or 0 for every band
 * @param[in] uniontype Pixel operation: `LAST`, `FIRST`, `MIN`, `MAX`,
 * `COUNT`, `SUM`, `MEAN` or `RANGE`, regardless of case, or NULL for `LAST` @nullable
 * @errval NULL
 * @csqlfn #Raster_union_transfn(), #Raster_union_uniontype_transfn()
 * @csqlaggfn #rasterUnionTransition()
 */
RasterUnionState *
raster_union_transfn(RasterUnionState *state, const Raster *rast, int band,
  const char *uniontype)
{
  if (band < 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The band of a union starts at 1, or is 0 for every band");
    return NULL;
  }
  int utype = UNIONTYPE_LAST;
  if (uniontype)
  {
    static const char * const unionnames[] = {"LAST", "FIRST", "MIN", "MAX",
      "COUNT", "SUM", "MEAN", "RANGE"};
    utype = raster_name_index(uniontype, unionnames,
      (int) (sizeof(unionnames) / sizeof(unionnames[0])));
    if (utype < 0)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Unknown union type: %s", uniontype);
      return NULL;
    }
  }
  /* Null raster: return state */
  if (! rast)
    return state;
  rt_raster raster = rt_raster_deserialize((void *) rast, 0);
  if (! raster)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }
  /* A state created here is freed on error, a state of the caller is left
   * to it */
  RasterUnionState *created = NULL;
  if (! state)
  {
    state = created = palloc0(sizeof(RasterUnionState));
    state->band = band;
    state->uniontype = utype;
    state->named = (uniontype != NULL);
  }
  bool ok = created || raster_union_ensure_valid(state, raster);

  /* The bands of the union: the one asked for, or as many as the rasters
   * read have, at least one where a pixel operation is named, as PostGIS
   * counts them */
  int numbands = (int) rt_raster_get_num_bands(raster);
  bool empty = rt_raster_is_empty(raster);
  int target = state->numbands;
  if (state->band > 0)
  {
    if (ok && ! empty && state->band > numbands)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The raster has no band %d, it has %d", state->band, numbands);
      ok = false;
    }
    target = 1;
  }
  else if (state->named)
    target = Max(Max(target, numbands), 1);
  else if (! empty)
    target = Max(target, numbands);
  ok = ok && raster_union_grow(state, target);

  int numparts = raster_union_numparts(state);
  for (int i = 0; ok && i < state->numbands; i++)
  {
    uint16_t nband = (uint16_t) (state->band > 0 ? state->band - 1 : i);
    for (int j = 0; ok && j < numparts; j++)
    {
      rt_raster work = raster_union_part(state, 2 * i + j);
      if (state->rasters[2 * i + j] && ! work)
      {
        ok = false;
        break;
      }
      rt_raster result = raster_union_step(work, raster, nband,
        raster_union_part_type(state, j), false, state->band == 0);
      ok = (result || (rt_raster_is_empty(work) && empty));
      if (work) raster_destroy(work);
      ok = ok && raster_union_set_part(state, 2 * i + j, result);
    }
  }
  raster_destroy(raster);
  if (! ok)
  {
    if (created)
      rasterunionstate_free(created);
    return NULL;
  }
  return state;
}

/**
 * @ingroup meos_raster_base_agg
 * @brief Combine function of the aggregate uniting rasters
 * @details The union of the second state is united with the union of the
 * first, part by part, by the pixel operation of the aggregate, the rasters
 * read by the first state taken as read before those of the second: two
 * counts are united by their sum. PostGIS's `ST_Union` has no combine
 * function
 * @param[in,out] state1 Current aggregate state @nullable
 * @param[in] state2 Aggregate state to combine with the first one @nullable
 * @return The first state, or a copy of the second where the first is NULL;
 * the second state is left to the caller
 * @errval NULL
 * @csqlfn #Raster_union_combinefn()
 */
RasterUnionState *
raster_union_combinefn(RasterUnionState *state1,
  const RasterUnionState *state2)
{
  if (! state2)
    return state1;
  if (! state1)
  {
    RasterUnionState *result = palloc(sizeof(RasterUnionState));
    *result = *state2;
    result->rasters = NULL;
    if (state2->numbands)
    {
      result->rasters = palloc0(sizeof(Raster *) * 2 * state2->numbands);
      for (int i = 0; i < 2 * state2->numbands; i++)
        if (state2->rasters[i])
          result->rasters[i] = raster_copy(state2->rasters[i]);
    }
    return result;
  }
  if (state1->band != state2->band || state1->uniontype != state2->uniontype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The states of a union unite different bands or pixel operations");
    return NULL;
  }
  int numparts = raster_union_numparts(state1);
  for (int k = 0; k < 2 * state2->numbands; k++)
  {
    if (! state2->rasters[k])
      continue;
    rt_raster other = raster_union_part(state2, k);
    if (! other)
      return NULL;
    bool valid = raster_union_ensure_valid(state1, other);
    raster_destroy(other);
    if (! valid)
      return NULL;
    break;
  }
  if (! raster_union_grow(state1, state2->numbands))
    return NULL;
  for (int i = 0; i < state2->numbands; i++)
  {
    for (int j = 0; j < numparts; j++)
    {
      int k = 2 * i + j;
      if (! state2->rasters[k])
        continue;
      rt_raster work = raster_union_part(state1, k);
      rt_raster other = raster_union_part(state2, k);
      if ((state1->rasters[k] && ! work) || ! other)
      {
        if (work) raster_destroy(work);
        if (other) raster_destroy(other);
        return NULL;
      }
      RasterUnionType utype = raster_union_part_type(state1, j);
      rt_raster result = raster_union_step(work, other, 0, utype,
        utype == UNIONTYPE_COUNT, true);
      bool failed = (! result && ! (rt_raster_is_empty(work) &&
        rt_raster_is_empty(other)));
      if (work) raster_destroy(work);
      raster_destroy(other);
      if (failed || ! raster_union_set_part(state1, k, result))
        return NULL;
    }
  }
  return state1;
}

/**
 * @ingroup meos_raster_base_agg
 * @brief Final function of the aggregate uniting rasters
 * @details The bands of the union, a mean or a range computed from its two
 * parts as PostGIS's `RASTER_union_finalfn` computes them. The state is
 * freed, as #raster_same_alignment_finalfn frees its own. A NULL state, an
 * aggregate over no raster, answers NULL
 * @param[in] state Aggregate state @nullable
 * @errval NULL
 * @csqlfn #Raster_union_finalfn()
 */
Raster *
raster_union_finalfn(RasterUnionState *state)
{
  if (! state)
    return NULL;
  rt_raster result = NULL;
  bool ok = true;
  for (int i = 0; ok && i < state->numbands; i++)
  {
    rt_raster bandrast = NULL;
    if (raster_union_numparts(state) == 2)
    {
      rt_raster part0 = raster_union_part(state, 2 * i);
      rt_raster part1 = raster_union_part(state, 2 * i + 1);
      rt_band band1 = rt_raster_is_empty(part1) ? NULL :
        rt_raster_get_band(part1, 0);
      if (part0 && band1)
      {
        /* The result takes the pixel type of the sum or of the maximum */
        rt_pixtype pixtype = rt_band_get_pixtype(band1);
        int hasnodata = rt_band_get_hasnodata_flag(band1);
        double nodataval = 0;
        if (hasnodata)
          rt_band_get_nodata(band1, &nodataval);
        struct rt_iterator_t itrset[2];
        itrset[0].raster = part0;
        itrset[0].nband = 0;
        itrset[0].nbnodata = 0;
        itrset[1].raster = part1;
        itrset[1].nband = 0;
        itrset[1].nbnodata = 0;
        RasterUnionType utype = (RasterUnionType) state->uniontype;
        if (rt_raster_iterator(itrset, 2, ET_UNION, NULL, pixtype, hasnodata,
            nodataval, 0, 0, NULL, &utype, raster_union_final_callback,
            &bandrast) != ES_NONE)
          ok = false;
      }
      if (part0) raster_destroy(part0);
      if (part1) raster_destroy(part1);
    }
    else
    {
      bandrast = raster_union_part(state, 2 * i);
      if (state->rasters[2 * i] && ! bandrast)
        ok = false;
    }
    if (! ok || ! bandrast)
      continue;
    if (! result)
    {
      uint32_t bandnums[1] = {0};
      result = rt_raster_from_band(bandrast, bandnums, 1);
      ok = (result != NULL);
    }
    else
      ok = rt_raster_copy_band(result, bandrast, 0,
        (int) rt_raster_get_num_bands(result)) >= 0;
    raster_destroy(bandrast);
  }
  rasterunionstate_free(state);
  if (! ok)
  {
    if (result) raster_destroy(result);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not build the bands of the union");
    return NULL;
  }
  if (! result)
    return NULL;
  return raster_serialize_destroy(result);
}

/**
 * @ingroup meos_raster_base_agg
 * @brief Return the bytes of the state of the aggregate uniting rasters,
 * read back by #rasterunionstate_deserialize
 * @details The bytes are the band, the pixel operation, whether it is named
 * and the number of bands, followed for each part of each band by its size
 * and the Well-Known Binary of its raster, a size of 0 for a part not built,
 * as #rasteralignstate_serialize writes the Well-Known Binary of its grid
 * @param[in] state State
 * @param[out] size_out Size of the result in bytes
 * @errval NULL
 * @csqlfn #Rasterunionstate_serialize()
 */
uint8_t *
rasterunionstate_serialize(const RasterUnionState *state, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(state, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  int count = 2 * state->numbands;
  uint8_t **wkbs = count ? palloc0(sizeof(uint8_t *) * count) : NULL;
  size_t *sizes = count ? palloc0(sizeof(size_t) * count) : NULL;
  size_t size = 3 * sizeof(int32) + 1 + count * sizeof(uint32);
  for (int i = 0; i < count; i++)
  {
    if (! state->rasters[i])
      continue;
    wkbs[i] = raster_as_wkb(state->rasters[i], 0, &sizes[i]);
    if (! wkbs[i])
    {
      for (int j = 0; j < i; j++)
        if (wkbs[j]) pfree(wkbs[j]);
      pfree(wkbs); pfree(sizes);
      return NULL;
    }
    size += sizes[i];
  }
  uint8_t *result = palloc(size);
  uint8_t *ptr = result;
  int32 ints[3] = {state->band, state->uniontype, state->numbands};
  memcpy(ptr, ints, sizeof(ints));
  ptr += sizeof(ints);
  *ptr++ = state->named ? 1 : 0;
  for (int i = 0; i < count; i++)
  {
    uint32 partsize = (uint32) sizes[i];
    memcpy(ptr, &partsize, sizeof(uint32));
    ptr += sizeof(uint32);
    if (wkbs[i])
    {
      memcpy(ptr, wkbs[i], sizes[i]);
      ptr += sizes[i];
      pfree(wkbs[i]);
    }
  }
  if (wkbs) pfree(wkbs);
  if (sizes) pfree(sizes);
  *size_out = size;
  return result;
}

/**
 * @ingroup meos_raster_base_agg
 * @brief Return the state of the aggregate uniting rasters, written by
 * #rasterunionstate_serialize
 * @param[in] bytes Bytes
 * @param[in] size Size of the bytes
 * @errval NULL
 * @csqlfn #Rasterunionstate_deserialize()
 */
RasterUnionState *
rasterunionstate_deserialize(const uint8_t *bytes, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(bytes, NULL);
  int32 ints[3];
  if (size < sizeof(ints) + 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The bytes of a union state are too short");
    return NULL;
  }
  memcpy(ints, bytes, sizeof(ints));
  const uint8_t *ptr = bytes + sizeof(ints);
  const uint8_t *end = bytes + size;
  if (ints[0] < 0 || ints[1] < UNIONTYPE_LAST || ints[1] > UNIONTYPE_RANGE ||
      ints[2] < 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The bytes of a union state are invalid");
    return NULL;
  }
  RasterUnionState *result = palloc0(sizeof(RasterUnionState));
  result->band = ints[0];
  result->uniontype = ints[1];
  result->named = (*ptr++ != 0);
  int count = 2 * ints[2];
  if (count)
    result->rasters = palloc0(sizeof(Raster *) * count);
  result->numbands = ints[2];
  for (int i = 0; i < count; i++)
  {
    uint32 partsize;
    if ((size_t) (end - ptr) < sizeof(uint32))
      break;
    memcpy(&partsize, ptr, sizeof(uint32));
    ptr += sizeof(uint32);
    if (! partsize)
      continue;
    if ((size_t) (end - ptr) < partsize)
    {
      ptr = NULL;
      break;
    }
    result->rasters[i] = raster_from_wkb(ptr, partsize);
    if (! result->rasters[i])
    {
      rasterunionstate_free(result);
      return NULL;
    }
    ptr += partsize;
  }
  if (ptr != end)
  {
    rasterunionstate_free(result);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The bytes of a union state are invalid");
    return NULL;
  }
  return result;
}

/**
 * @ingroup meos_raster_base_agg
 * @brief Transition function of the aggregate summarizing the pixels of
 * rasters
 * @details The aggregate of PostGIS's `ST_SummaryStatsAgg`, keeping what
 * PostGIS's `RASTER_summaryStats_transfn` keeps: the count, the sum, the
 * smallest and the largest value of the pixels read, and the running mean and
 * sum of squared deviations from it that @p rt_band_get_summary_stats updates
 * pixel by pixel across rasters, from which the final function takes the
 * standard deviation in one pass. Every pixel is read, as
 * #raster_summary_stats reads them, so the statistics are exact rather than
 * estimated from a sample.
 *
 * The band and whether nodata is left out are those of the first raster read.
 * A NULL raster leaves the state as it is, and a band the raster lacks raises
 * an error, as it does for #raster_summary_stats, where PostGIS skips the
 * raster
 * @param[in,out] state Current aggregate state, NULL before the first raster @nullable
 * @param[in] rast Raster @nullable
 * @param[in] band Number of the band, starting at 1
 * @param[in] exclude_nodata True to leave the pixels the band states as nodata
 * out of the statistics, false to count them as the values they hold
 * @errval NULL
 * @csqlfn #Raster_summary_stats_transfn(),
 *   #Raster_summary_stats_band_transfn()
 * @csqlaggfn #summaryStatsAggTransition()
 */
RasterStatsState *
raster_summary_stats_transfn(RasterStatsState *state, const Raster *rast,
  int band, bool exclude_nodata)
{
  if (band < 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The band of the statistics starts at 1");
    return NULL;
  }
  /* Null raster: return state */
  if (! rast)
    return state;
  rt_raster raster = rt_raster_deserialize((void *) rast, 0);
  if (! raster)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return NULL;
  }
  /* A state created here is freed on error, a state of the caller is left
   * to it */
  RasterStatsState *created = NULL;
  if (! state)
  {
    state = created = palloc0(sizeof(RasterStatsState));
    state->band = band;
    state->exclude_nodata = exclude_nodata;
  }
  int numbands = (int) rt_raster_get_num_bands(raster);
  rt_band rtband = (state->band <= numbands) ?
    rt_raster_get_band(raster, (uint32_t) (state->band - 1)) : NULL;
  if (! rtband)
  {
    int nband = state->band;
    raster_destroy(raster);
    if (created)
      pfree(created);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The raster has no band %d, it has %d", nband, numbands);
    return NULL;
  }

  /* A sample of one is every pixel, and the running mean and sum of squared
   * deviations continue from those of the rasters read before */
  uint64_t count = state->count;
  rt_bandstats stats = rt_band_get_summary_stats(rtband,
    state->exclude_nodata ? 1 : 0, 1.0, 0, &count, &state->mean, &state->q);
  raster_destroy(raster);
  if (! stats)
  {
    int nband = state->band;
    if (created)
      pfree(created);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not summarize band %d of the raster", nband);
    return NULL;
  }
  /* The values of a band with no pixel to count leave the state as it is */
  if (stats->count > 0)
  {
    if (state->count < 1)
    {
      state->min = stats->min;
      state->max = stats->max;
    }
    else
    {
      if (stats->min < state->min)
        state->min = stats->min;
      if (stats->max > state->max)
        state->max = stats->max;
    }
    state->sum += stats->sum;
    state->count = (uint64) count;
  }
  /* The statistics are allocated by the raster core, so its own deallocator
   * releases them */
  rtdealloc(stats);
  return state;
}

/**
 * @ingroup meos_raster_base_agg
 * @brief Combine function of the aggregate summarizing the pixels of rasters
 * @details The counts, sums, smallest and largest values are combined as
 * they are, and the running means and sums of squared deviations of the two
 * states by the pairwise formula of Chan, Golub and LeVeque, so the standard
 * deviation of the combined state is that of every pixel the two have read.
 * PostGIS's `ST_SummaryStatsAgg` has no combine function. Two states of
 * different bands, or one leaving nodata out and one counting it, raise an
 * error
 * @param[in,out] state1 Current aggregate state @nullable
 * @param[in] state2 Aggregate state to combine with the first one @nullable
 * @return The first state, or a copy of the second where the first is NULL;
 * the second state is left to the caller
 * @errval NULL
 * @csqlfn #Raster_summary_stats_combinefn()
 */
RasterStatsState *
raster_summary_stats_combinefn(RasterStatsState *state1,
  const RasterStatsState *state2)
{
  if (! state2)
    return state1;
  if (! state1)
  {
    RasterStatsState *result = palloc(sizeof(RasterStatsState));
    memcpy(result, state2, sizeof(RasterStatsState));
    return result;
  }
  if (state1->band != state2->band ||
      state1->exclude_nodata != state2->exclude_nodata)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The statistics of two states of different bands or of a different "
      "handling of nodata cannot be combined");
    return NULL;
  }
  if (state2->count < 1)
    return state1;
  if (state1->count < 1)
  {
    memcpy(state1, state2, sizeof(RasterStatsState));
    return state1;
  }
  double n1 = (double) state1->count, n2 = (double) state2->count;
  double n = n1 + n2;
  double delta = state2->mean - state1->mean;
  state1->mean += delta * n2 / n;
  state1->q += state2->q + delta * delta * n1 * n2 / n;
  state1->sum += state2->sum;
  if (state2->min < state1->min)
    state1->min = state2->min;
  if (state2->max > state1->max)
    state1->max = state2->max;
  state1->count += state2->count;
  return state1;
}

/**
 * @ingroup meos_raster_base_agg
 * @brief Final function of the aggregate summarizing the pixels of rasters
 * @details The mean is the sum over the count and the standard deviation the
 * square root of the sum of squared deviations over the count, as PostGIS's
 * `RASTER_summaryStats_finalfn` takes them when every pixel is read. The
 * state is freed, as #raster_same_alignment_finalfn frees its own
 * @param[in] state Aggregate state @nullable
 * @return The statistics, or NULL where the state is NULL, an aggregate over
 * no raster, or counts no pixel, in which case no error is stated
 * @errval NULL
 * @csqlfn #Raster_summary_stats_finalfn()
 */
BandStats *
raster_summary_stats_finalfn(RasterStatsState *state)
{
  if (! state)
    return NULL;
  if (state->count > UINT32_MAX)
  {
    pfree(state);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The statistics count more pixels than their count can state");
    return NULL;
  }
  BandStats *result = NULL;
  if (state->count > 0)
  {
    result = palloc0(sizeof(BandStats));
    result->count = (uint32_t) state->count;
    result->sum = state->sum;
    result->mean = state->sum / (double) state->count;
    result->stddev = sqrt(state->q / (double) state->count);
    result->min = state->min;
    result->max = state->max;
  }
  pfree(state);
  return result;
}

/**
 * @ingroup meos_raster_base_agg
 * @brief Return the bytes of the state of the aggregate summarizing the pixels
 * of rasters, read back by #rasterstatsstate_deserialize
 * @details The bytes are the band and whether nodata is left out, followed by
 * the count, the sum, the smallest and largest value, the running mean and
 * the sum of squared deviations, each in the byte order of the machine, as
 * the state crosses only between workers of one server
 * @param[in] state State
 * @param[out] size_out Size of the result in bytes
 * @errval NULL
 * @csqlfn #Rasterstatsstate_serialize()
 */
uint8_t *
rasterstatsstate_serialize(const RasterStatsState *state, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(state, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  int32 band = state->band;
  double values[5] = {state->sum, state->min, state->max, state->mean,
    state->q};
  size_t size = sizeof(int32) + 1 + sizeof(uint64) + sizeof(values);
  uint8_t *result = palloc(size);
  uint8_t *ptr = result;
  memcpy(ptr, &band, sizeof(int32));
  ptr += sizeof(int32);
  *ptr++ = state->exclude_nodata ? 1 : 0;
  memcpy(ptr, &state->count, sizeof(uint64));
  ptr += sizeof(uint64);
  memcpy(ptr, values, sizeof(values));
  *size_out = size;
  return result;
}

/**
 * @ingroup meos_raster_base_agg
 * @brief Return the state of the aggregate summarizing the pixels of rasters,
 * written by #rasterstatsstate_serialize
 * @param[in] bytes Bytes
 * @param[in] size Size of the bytes
 * @errval NULL
 * @csqlfn #Rasterstatsstate_deserialize()
 */
RasterStatsState *
rasterstatsstate_deserialize(const uint8_t *bytes, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(bytes, NULL);
  int32 band;
  double values[5];
  if (size != sizeof(int32) + 1 + sizeof(uint64) + sizeof(values))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The bytes of a statistics state are not of its size");
    return NULL;
  }
  memcpy(&band, bytes, sizeof(int32));
  if (band < 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The bytes of a statistics state are invalid");
    return NULL;
  }
  const uint8_t *ptr = bytes + sizeof(int32);
  RasterStatsState *result = palloc(sizeof(RasterStatsState));
  result->band = band;
  result->exclude_nodata = (*ptr++ != 0);
  memcpy(&result->count, ptr, sizeof(uint64));
  ptr += sizeof(uint64);
  memcpy(values, ptr, sizeof(values));
  result->sum = values[0];
  result->min = values[1];
  result->max = values[2];
  result->mean = values[3];
  result->q = values[4];
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
 * @brief State a raster sampling call keeps for the length of a trajectory
 * @details The state holds the deserialized raster, the band the values are
 * read from, the geotransform and its inverse, computed once and handed to
 * every crossing and every point conversion, whether a nodata pixel carries no
 * value, and whether a pixel of the band could not be read.
 */
typedef struct
{
  rt_raster raster;   /**< Raster carrying its bands */
  rt_band band;       /**< Band the pixel values are read from */
  double gt[6];       /**< Geotransform of the raster */
  double igt[6];      /**< Inverse geotransform of the raster */
  bool exclude_nodata; /**< Whether a nodata pixel carries no value */
  bool unreadable;    /**< Whether a pixel of the band could not be read */
} RasterSampleState;

/**
 * @brief Raster grid callback converting a position to the raster
 * coordinates of a PostGIS raster with its inverse geotransform
 * @details The position is converted as `rt_raster_geopoint_to_rasterpoint`
 * converts one. `GDALApplyGeoTransform` takes a writable geotransform, so it
 * reads a copy of the one the state holds.
 */
static void
raster_value_grid(const void *ctxp, double x, double y, double *col,
  double *row)
{
  const RasterSampleState *state = (const RasterSampleState *) ctxp;
  double igt[6];
  memcpy(igt, state->igt, sizeof(igt));
  GDALApplyGeoTransform(igt, x, y, col, row);
  return;
}

/**
 * @brief Raster crossing callback placing the grid lines of a PostGIS raster
 * with its geotransform, see #raster_affine_cross()
 */
static double
raster_value_cross(const void *ctxp, double x1, double y1, double x2,
  double y2, int axis, double k)
{
  const RasterSampleState *state = (const RasterSampleState *) ctxp;
  return raster_affine_cross(state->gt, x1, y1, x2, y2, axis, k);
}

/**
 * @brief Raster pixel callback reading one pixel of a PostGIS raster through
 * the vendored raster core
 * @details Reading the pixel a position falls in is the nearest-neighbour
 * read of that position. A nodata pixel answers that there is no value,
 * which is the contract of ::raster_pixel_fn, unless the state keeps the
 * nodata value, which the pixel then answers as `ST_Value` answers it with
 * `exclude_nodata_value` false. A pixel the raster core cannot
 * read, as when the band is stored outside the database in a file that
 * cannot be opened, carries no value either, and the state records it so
 * that the caller raises an error once it has released the raster; the
 * pixels after it are not read.
 */
static bool
raster_value_pixel(void *ctxp, int col, int row, double *value)
{
  RasterSampleState *state = (RasterSampleState *) ctxp;
  if (state->unreadable)
    return false;
  int isnodata;
  if (rt_band_get_pixel(state->band, col, row, value, &isnodata) != ES_NONE)
  {
    state->unreadable = true;
    return false;
  }
  return ! isnodata || ! state->exclude_nodata;
}

/**
 * @brief Raster value callback reading a PostGIS raster at a position by
 * bilinear interpolation of the four pixels around it
 * @details The value is the one `ST_Value` answers with `resample` set to
 * bilinear, read by the same `rt_band_get_pixel_resample`. That read takes a
 * neighbour it cannot read for a nodata one, so the pixel the position falls
 * in is read first: a band whose file cannot be opened then raises an error
 * as the read of that pixel alone raises it.
 */
static bool
raster_value_bilinear(void *ctxp, double col, double row, double *value)
{
  RasterSampleState *state = (RasterSampleState *) ctxp;
  if (state->unreadable)
    return false;
  int isnodata;
  if (rt_band_get_pixel_resample(state->band, col, row, RT_NEAREST, value,
        &isnodata) != ES_NONE ||
      rt_band_get_pixel_resample(state->band, col, row, RT_BILINEAR, value,
        &isnodata) != ES_NONE)
  {
    state->unreadable = true;
    return false;
  }
  return ! isnodata || ! state->exclude_nodata;
}

/**
 * @brief Raster data callback answering whether a pixel of a PostGIS raster
 * holds data, as the bilinear read of the raster core tells a pixel holding
 * data from one it replaces by the pixel a position falls in
 */
static bool
raster_value_data(void *ctxp, int col, int row)
{
  RasterSampleState *state = (RasterSampleState *) ctxp;
  if (state->unreadable)
    return false;
  double value;
  int isnodata;
  if (rt_band_get_pixel(state->band, col, row, &value, &isnodata) != ES_NONE)
  {
    state->unreadable = true;
    return false;
  }
  return ! isnodata;
}

/**
 * @brief Build the sampling state and the extent pre-filter shared by every
 * raster sampling function
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rast Raster
 * @param[in] band Band number (1-based)
 * @param[in] exclude_nodata True when a nodata pixel carries no value
 * @param[in] bilinear True to read a position by bilinear interpolation of
 * the pixels around it, false to read the pixel it falls in
 * @param[out] state Sampling state; on success its raster is owned by the
 * caller, which releases it with #raster_destroy()
 * @param[out] ops Descriptor of the raster grid
 * @return true on success; on failure sets a MEOS error and returns false
 */
static bool
raster_rtcore_gridops(const Temporal *traj, const Raster *rast, int band,
  bool exclude_nodata, bool bilinear, RasterSampleState *state,
  RasterGridOps *ops)
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
  state->gt[0] = rt_raster_get_x_offset(raster);
  state->gt[1] = rt_raster_get_x_scale(raster);
  state->gt[2] = rt_raster_get_x_skew(raster);
  state->gt[3] = rt_raster_get_y_offset(raster);
  state->gt[4] = rt_raster_get_y_skew(raster);
  state->gt[5] = rt_raster_get_y_scale(raster);
  state->raster = raster;
  state->band = rtband;
  state->exclude_nodata = exclude_nodata;
  state->unreadable = false;

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
  ops->grid = &raster_value_grid;
  ops->pixel = &raster_value_pixel;
  ops->point = bilinear ? &raster_value_bilinear : NULL;
  ops->data = bilinear ? &raster_value_data : NULL;
  ops->cross = &raster_value_cross;
  ops->ctx = state;
  ops->width = (int) rt_raster_get_width(raster);
  ops->height = (int) rt_raster_get_height(raster);
  memset(&ops->box, 0, sizeof(STBox));
  ops->box.xmin = env.MinX; ops->box.xmax = env.MaxX;
  ops->box.ymin = env.MinY; ops->box.ymax = env.MaxY;
  return true;
}

/**
 * @brief Release the raster a sampling call deserialized, raising an error
 * when a pixel of its band could not be read
 * @details A band stored outside the database is read from its file when its
 * first pixel is read, so a file that cannot be opened surfaces there, in the
 * middle of the walk. The walk carries on without reading, the raster is
 * released, and the error names the file.
 * @param[in] state Sampling state, whose raster is released
 * @param[in] band Band number (1-based)
 * @return true when every pixel read answered, false after raising the error
 */
static bool
raster_rtcore_release(RasterSampleState *state, int band)
{
  if (! state->unreadable)
  {
    raster_destroy(state->raster);
    return true;
  }
  /* The path belongs to the raster, so it is copied before the release */
  char path[1024] = "";
  const char *ext = rt_band_is_offline(state->band) ?
    rt_band_get_ext_path(state->band) : NULL;
  if (ext)
    snprintf(path, sizeof(path), "%s", ext);
  raster_destroy(state->raster);
  if (path[0])
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not read band %d of the raster from %s", band, path);
    return false;
  }
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Could not read band %d of the raster", band);
  return false;
}

/**
 * @brief Return whether the name of a read of a position states the bilinear
 * one, raising an error for a name that states neither read
 * @details The names are those `ST_Value` reads, without regard to case
 * @param[in] resample Name of the read
 * @param[out] bilinear True for the bilinear read, false for the nearest one
 * @return False after raising an error for a name that states no read
 */
static bool
raster_resample_bilinear(const char *resample, bool *bilinear)
{
  size_t len = strlen(resample);
  char *name = palloc(len + 1);
  for (size_t i = 0; i < len; i++)
    name[i] = (char) tolower((unsigned char) resample[i]);
  name[len] = '\0';
  bool found = true;
  if (strcmp(name, "bilinear") == 0)
    *bilinear = true;
  else if (strcmp(name, "nearest") == 0)
    *bilinear = false;
  else
    found = false;
  pfree(name);
  if (! found)
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Unknown resample type: %s", resample);
  return found;
}

/**
 * @ingroup meos_raster
 * @brief Return the values of a raster band read along a trajectory
 * @details A position reads the pixel it falls in, or, with @p resample
 * stating the bilinear read, the value the four pixels around it
 * interpolate, as `ST_Value` reads it. A bilinear value varies within a
 * pixel, and along a segment of a moving trajectory it is a polynomial of
 * degree two in time over each half-pixel square the segment crosses. The
 * answer then holds the value at the instants of the trajectory, where it
 * crosses the pixel edges and the lines through the pixel centres, and at
 * the extremum of each of those pieces, and is linear between them, as the
 * product of two temporal floats is. It is the value itself where the
 * trajectory moves along a grid axis, over which the value is linear
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rast Raster
 * @param[in] band Band number (1-based)
 * @param[in] exclude_nodata True to leave out the positions over a nodata
 * pixel, false to answer the nodata value there
 * @param[in] resample Name of the read of a position, nearest or bilinear,
 * read without regard to case, NULL for nearest @nullable
 * @return A temporal float, or @p NULL when no instant of @p traj falls
 * inside the raster or survives nodata filtering
 * @errval NULL
 * @csqlfn #Raster_value()
 */
Temporal *
raster_value(const Temporal *traj, const Raster *rast, int band,
  bool exclude_nodata, const char *resample)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(traj, NULL); VALIDATE_NOT_NULL(rast, NULL);
  bool bilinear = false;
  if (resample && ! raster_resample_bilinear(resample, &bilinear))
    return NULL;

  RasterSampleState state;
  RasterGridOps ops;
  if (! raster_rtcore_gridops(traj, rast, band, exclude_nodata, bilinear,
      &state, &ops))
    return NULL;
  Temporal *result = raster_value_sampler(traj, &ops);
  if (! raster_rtcore_release(&state, band))
  {
    if (result)
      pfree(result);
    return NULL;
  }
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
 * @errval NULL
 * @csqlfn #Raster_at_value()
 */
Temporal *
raster_at_value(const Temporal *traj, const Raster *rast, int band,
  const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(traj, NULL); VALIDATE_NOT_NULL(rast, NULL);
  VALIDATE_NOT_NULL(vspan, NULL);

  RasterSampleState state;
  RasterGridOps ops;
  if (! raster_rtcore_gridops(traj, rast, band, true, false, &state, &ops))
    return NULL;
  Temporal *result = raster_at_value_sampler(traj, &ops, vspan);
  if (! raster_rtcore_release(&state, band))
  {
    if (result)
      pfree(result);
    return NULL;
  }
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
 * @errval NULL
 * @csqlfn #Raster_minus_value()
 */
Temporal *
raster_minus_value(const Temporal *traj, const Raster *rast, int band,
  const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(traj, NULL); VALIDATE_NOT_NULL(rast, NULL);
  VALIDATE_NOT_NULL(vspan, NULL);

  RasterSampleState state;
  RasterGridOps ops;
  if (! raster_rtcore_gridops(traj, rast, band, true, false, &state, &ops))
    return NULL;
  Temporal *result = raster_minus_value_sampler(traj, &ops, vspan);
  if (! raster_rtcore_release(&state, band))
  {
    if (result)
      pfree(result);
    return NULL;
  }
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
  VALIDATE_TGEOMPOINT(traj, -1); VALIDATE_NOT_NULL(rast, -1);
  VALIDATE_NOT_NULL(vspan, -1);

  RasterSampleState state;
  RasterGridOps ops;
  if (! raster_rtcore_gridops(traj, rast, band, true, false, &state, &ops))
    return -1;
  int result = eraster_value_sampler(traj, &ops, vspan);
  return raster_rtcore_release(&state, band) ? result : -1;
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
  VALIDATE_TGEOMPOINT(traj, -1); VALIDATE_NOT_NULL(rast, -1);
  VALIDATE_NOT_NULL(vspan, -1);

  RasterSampleState state;
  RasterGridOps ops;
  if (! raster_rtcore_gridops(traj, rast, band, true, false, &state, &ops))
    return -1;
  int result = araster_value_sampler(traj, &ops, vspan);
  return raster_rtcore_release(&state, band) ? result : -1;
}

/*****************************************************************************/
