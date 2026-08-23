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
 * @brief General functions for the Raquet raster-tile value type.
 *
 * A Raquet value is a GDAL-free, self-describing Web-Mercator raster chip
 * identified by a CARTO QUADBIN cell and carrying a row-major packed pixel
 * array. Serialization goes through the central MEOS WKB machinery keyed by
 * the ::T_RAQUET catalog type; the trajectory-sampling kernel in
 * `raster_quadbin.c` operates on the unpacked fields.
 */

#include "raster/raquet.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <pgtypes.h>
#include "common/hashfn.h"
#include <varatt.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include "raster/raster_quadbin.h"
#include "temporal/temporal.h"
#include "temporal/type_inout.h"

/*****************************************************************************
 * Pixel type catalog
 *
 * One row per pixel data type, indexed by its ::MeosPixType code, so that the
 * name and the size of a type are stated once and read from here rather than
 * repeated. The code indexes the catalog and is what a tile carries in its
 * header and in its Well-Known Binary representation, so a row keeps the index
 * it has.
 *****************************************************************************/

/**
 * @brief Structure to represent a row of the pixel type catalog
 */
typedef struct
{
  const char *name;   /**< Name the constructors accept and #raquet_pixtype() returns */
  const char *pgname; /**< Name PostGIS raster gives the type, also accepted */
  size_t size;        /**< Size in bytes of a single pixel */
} pixtype_catalog_struct;

/**
 * @brief Global constant array containing the pixel data types
 * @details The name is the one the RaQuet specification gives the type, and is
 * what a tile reports. The PostGIS name is the spelling `ST_BandPixelType`
 * returns for the same type, accepted so that the band type of a PostGIS
 * raster passes into the constructors as it stands. PostGIS raster carries no
 * 16-bit float and no 64-bit integer, so those three take the spelling its
 * grammar gives them.
 */
static const pixtype_catalog_struct MEOS_PIXTYPE_CATALOG[] =
{
  [MEOS_PT_UINT8]   = { "uint8",   "8BUI",  1 },
  [MEOS_PT_INT16]   = { "int16",   "16BSI", 2 },
  [MEOS_PT_INT32]   = { "int32",   "32BSI", 4 },
  [MEOS_PT_FLOAT32] = { "float32", "32BF",  4 },
  [MEOS_PT_FLOAT64] = { "float64", "64BF",  8 },
  [MEOS_PT_INT8]    = { "int8",    "8BSI",  1 },
  [MEOS_PT_UINT16]  = { "uint16",  "16BUI", 2 },
  [MEOS_PT_UINT32]  = { "uint32",  "32BUI", 4 },
  [MEOS_PT_INT64]   = { "int64",   "64BSI", 8 },
  [MEOS_PT_UINT64]  = { "uint64",  "64BUI", 8 },
  [MEOS_PT_FLOAT16] = { "float16", "16BF",  2 },
};

/**
 * @brief Names of the PostGIS raster pixel types that bound a pixel to less
 * than a byte
 * @details PostGIS raster stores each of them a byte a pixel, as
 * `rt_pixtype_size` gives them, in the same case group as `8BUI`. The bit
 * width in the name bounds the values a pixel may hold and says nothing about
 * how the band is stored, so the bytes of such a band are already those of an
 * unsigned 8-bit band and the constructors read them as `uint8`
 */
static const char *MEOS_PIXTYPE_PG_BYTE_BOUNDED[] = { "1BB", "2BUI", "4BUI" };

/**
 * @brief Return true when a pixel type code has a row in the catalog
 */
static bool
pixtype_known(uint8 pixtype)
{
  size_t n = sizeof(MEOS_PIXTYPE_CATALOG) / sizeof(pixtype_catalog_struct);
  return (size_t) pixtype < n && MEOS_PIXTYPE_CATALOG[pixtype].name;
}

/**
 * @brief Write into @p buf the names of the catalog as an enumeration reading
 * "A, B, or C", which is how the error messages name the types a caller may use
 * @param[out] buf Buffer receiving the enumeration
 * @param[in] size Size of @p buf
 */
static char *
pixtype_names(char *buf, size_t size)
{
  size_t n = sizeof(MEOS_PIXTYPE_CATALOG) / sizeof(pixtype_catalog_struct);
  size_t last = 0;
  for (size_t i = 0; i < n; i++)
  {
    if (MEOS_PIXTYPE_CATALOG[i].name)
      last = i;
  }
  size_t pos = 0;
  buf[0] = '\0';
  for (size_t i = 0; i < n; i++)
  {
    if (! MEOS_PIXTYPE_CATALOG[i].name)
      continue;
    int len = snprintf(buf + pos, size - pos, "%s%s",
      pos ? (i == last ? ", or " : ", ") : "", MEOS_PIXTYPE_CATALOG[i].name);
    if (len < 0 || pos + (size_t) len >= size)
      break;
    pos += (size_t) len;
  }
  return buf;
}

/*****************************************************************************
 * Validity functions
 *****************************************************************************/

/**
 * @brief Ensure that a pixel type code is one of the supported values
 */
static bool
ensure_valid_pixtype(uint8 pixtype)
{
  if (! pixtype_known(pixtype))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Unknown raquet pixel type code: %u", pixtype);
    return false;
  }
  return true;
}

/*****************************************************************************
 * Pixel type functions
 *****************************************************************************/

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the size in bytes of a single pixel of the given type
 * @param[in] pixtype Pixel data type
 * @return On error return 0
 */
size_t
raquet_pixtype_size(MeosPixType pixtype)
{
  return pixtype_known((uint8) pixtype) ?
    MEOS_PIXTYPE_CATALOG[pixtype].size : 0;
}

/*****************************************************************************
 * Pixel payload codec
 *
 * A Raquet band is a little-endian byte stream whatever the machine that wrote
 * it, so that a tile keeps its meaning when it travels between hosts: the WKB
 * endianness flag describes the scalar header only, and the payload is copied
 * through verbatim by #raquet_from_wkb_state() and its output counterpart.
 * The bytes are therefore assembled and taken apart by arithmetic rather than
 * copied onto a machine-typed variable. That reads the same value on every
 * host with no compile-time endianness branch, so the path a big-endian
 * machine takes is the very one the tests exercise on a little-endian one.
 *****************************************************************************/

/**
 * @brief Return the unsigned integer of @p size bytes stored little-endian
 * at @p p
 */
static uint64
le_uint(const uint8 *p, size_t size)
{
  uint64 result = 0;
  for (size_t i = 0; i < size; i++)
    result |= (uint64) p[i] << (8 * i);
  return result;
}

/**
 * @brief Store an unsigned integer little-endian in the @p size bytes at @p p
 */
static void
le_uint_store(uint8 *p, uint64 value, size_t size)
{
  for (size_t i = 0; i < size; i++)
    p[i] = (uint8) (value >> (8 * i));
}

/**
 * @brief Return true when an unsigned integer is exactly representable in a
 * double, that is, when it is no greater than 2^53
 * @details A double carries 53 bits of significand, so beyond that the
 * integers stop being consecutive and a conversion rounds to a neighbour.
 */
static bool
exact_in_double(uint64 value)
{
  return value <= (UINT64_C(1) << 53);
}

/**
 * @brief Report a pixel that the sampling surface cannot carry and return zero
 * @details The sampling surface is double-valued, so a 64-bit integer band
 * holds values that no double names. Such a pixel is reported rather than
 * rounded to a neighbouring value, which would answer a question the caller
 * did not ask.
 */
static bool
pixel_out_of_domain(void)
{
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Pixel value is not exactly representable in a double precision number, "
    "which is the value domain of the raster sampling");
  return false;
}

/**
 * @brief Return the value of a 16-bit IEEE half precision number
 * @param[in] bits The sixteen bits of the number
 */
static double
half_to_double(uint16 bits)
{
  int sign = (bits >> 15) & 0x1;
  int exponent = (bits >> 10) & 0x1F;
  uint32 mantissa = bits & 0x3FF;
  double result;
  if (exponent == 0)
    /* Zero when the mantissa is zero, a subnormal otherwise, whose exponent is
     * the one of the smallest normal and whose leading bit is not implied */
    result = ldexp((double) mantissa, -24);
  else if (exponent == 0x1F)
    result = mantissa ? NAN : INFINITY;
  else
    /* A normal number carries an implied leading bit */
    result = ldexp((double) (mantissa | 0x400), exponent - 25);
  return sign ? -result : result;
}

/**
 * @brief Return in the last argument the value of the pixel at position
 * @p index of a Raquet band
 * @param[in] pixels Row-major packed pixel bytes
 * @param[in] index Position of the pixel, that is, `row * width + column`
 * @param[in] pixtype Pixel data type
 * @param[out] result Value of the pixel
 * @return true on success; on failure sets a MEOS error and returns false,
 * which a caller must not read past, since a value the sampling surface cannot
 * carry has no number to stand for it
 */
bool
raquet_pixel_value(const uint8 *pixels, size_t index, MeosPixType pixtype,
  double *result)
{
  const uint8 *p = pixels + index * raquet_pixtype_size(pixtype);
  switch (pixtype)
  {
    case MEOS_PT_UINT8:
      *result = (double) (uint8) le_uint(p, 1);
      return true;
    case MEOS_PT_INT16:
    {
      uint16 bits = (uint16) le_uint(p, 2);
      int16 value;
      memcpy(&value, &bits, sizeof(value));
      *result = (double) value;
      return true;
    }
    case MEOS_PT_INT32:
    {
      uint32 bits = (uint32) le_uint(p, 4);
      int32 value;
      memcpy(&value, &bits, sizeof(value));
      *result = (double) value;
      return true;
    }
    case MEOS_PT_FLOAT32:
    {
      uint32 bits = (uint32) le_uint(p, 4);
      float value;
      memcpy(&value, &bits, sizeof(value));
      *result = (double) value;
      return true;
    }
    case MEOS_PT_FLOAT64:
    {
      uint64 bits = le_uint(p, 8);
      memcpy(result, &bits, sizeof(*result));
      return true;
    }
    case MEOS_PT_INT8:
    {
      uint8 bits = (uint8) le_uint(p, 1);
      int8 value;
      memcpy(&value, &bits, sizeof(value));
      *result = (double) value;
      return true;
    }
    case MEOS_PT_UINT16:
      *result = (double) (uint16) le_uint(p, 2);
      return true;
    case MEOS_PT_UINT32:
      *result = (double) (uint32) le_uint(p, 4);
      return true;
    case MEOS_PT_INT64:
    {
      uint64 bits = le_uint(p, 8);
      int64 value;
      memcpy(&value, &bits, sizeof(value));
      /* The magnitude of the most negative value overflows a positive int64,
       * so it is taken in unsigned arithmetic */
      if (! exact_in_double(value < 0 ?
            (uint64) -(value + 1) + 1 : (uint64) value))
        return pixel_out_of_domain();
      *result = (double) value;
      return true;
    }
    case MEOS_PT_UINT64:
    {
      uint64 value = le_uint(p, 8);
      if (! exact_in_double(value))
        return pixel_out_of_domain();
      *result = (double) value;
      return true;
    }
    case MEOS_PT_FLOAT16:
      *result = half_to_double((uint16) le_uint(p, 2));
      return true;
    default:
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Unknown raquet pixel type code: %u", (uint8) pixtype);
      return false;
  }
}

/**
 * @brief Rewrite in place a band that a machine has just filled with its own
 * byte order, so that it is stored little-endian as a Raquet band is
 * @param[in,out] pixels Row-major packed pixel bytes
 * @param[in] count Number of pixels
 * @param[in] pixtype Pixel data type
 * @note Single-byte and unknown pixel types leave the band untouched
 */
void
raquet_pixels_from_host(uint8 *pixels, size_t count, MeosPixType pixtype)
{
  size_t size = raquet_pixtype_size(pixtype);
  if (size < 2)
    return;
  for (size_t i = 0; i < count; i++)
  {
    uint8 *p = pixels + i * size;
    /* Reading through a variable of the very width of the pixel is what makes
     * this the value the machine meant: a copy onto a wider one would land on
     * its high-order bytes on a big-endian host */
    uint64 bits;
    if (size == 2)
    {
      uint16 value;
      memcpy(&value, p, sizeof(value));
      bits = value;
    }
    else if (size == 4)
    {
      uint32 value;
      memcpy(&value, p, sizeof(value));
      bits = value;
    }
    else
      memcpy(&bits, p, sizeof(bits));
    le_uint_store(p, bits, size);
  }
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the pixel data type corresponding to a name
 * @param[in] str Pixel type name: uint8, int8, uint16, int16, uint32, int32, uint64, int64, float16, float32, or float64
 * @note This is the parser counterpart of #raquet_pixtype()
 * @note The name is read without regard to case, so a name written either way
 * carries into the constructors. The name #raquet_pixtype() returns is the one
 * the RaQuet specification writes, in lower case, so it compares equal to the
 * `type` field of the tile a file holds
 * @note The name PostGIS raster gives a pixel type is accepted for the same
 * type, so the band type an `ST_BandPixelType` call reports carries into the
 * constructors as it stands. Its `1BB`, `2BUI` and `4BUI` bound a pixel to
 * less than a byte while occupying one, so they name a `uint8` band
 */
MeosPixType
raquet_pixtype_from_string(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, MEOS_PT_UINT8);
  size_t n = sizeof(MEOS_PIXTYPE_CATALOG) / sizeof(pixtype_catalog_struct);
  for (size_t i = 0; i < n; i++)
  {
    if (! MEOS_PIXTYPE_CATALOG[i].name)
      continue;
    if (pg_strcasecmp(str, MEOS_PIXTYPE_CATALOG[i].name) == 0 ||
        pg_strcasecmp(str, MEOS_PIXTYPE_CATALOG[i].pgname) == 0)
      return (MeosPixType) i;
  }
  /* A PostGIS pixel type bounded to less than a byte occupies a byte a pixel,
   * so its band is already the bytes of an unsigned 8-bit band and the bound
   * is the only thing its name adds */
  size_t nb = sizeof(MEOS_PIXTYPE_PG_BYTE_BOUNDED) /
    sizeof(MEOS_PIXTYPE_PG_BYTE_BOUNDED[0]);
  for (size_t i = 0; i < nb; i++)
  {
    if (pg_strcasecmp(str, MEOS_PIXTYPE_PG_BYTE_BOUNDED[i]) == 0)
      return MEOS_PT_UINT8;
  }
  char names[128];
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Unknown pixel type \"%s\": use %s, or the name PostGIS raster gives one "
    "of them", str, pixtype_names(names, sizeof(names)));
  return MEOS_PT_UINT8; /* make compiler quiet */
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_raster_base_inout
 * @brief Return a Raquet tile from its ASCII hex-encoded Well-Known Binary
 * (HexWKB) representation
 * @param[in] str HexWKB string
 * @csqlfn #Raquet_in()
 */
Raquet *
raquet_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return DatumGetRaquetP(type_from_hexwkb(str, strlen(str), T_RAQUET));
}

/**
 * @ingroup meos_raster_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a Raquet tile
 * @param[in] rq Raquet tile
 * @csqlfn #Raquet_out()
 */
char *
raquet_out(const Raquet *rq)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq, NULL);
  size_t size;
  return (char *) datum_as_wkb(RaquetPGetDatum(rq), T_RAQUET,
    (uint8_t) (WKB_NDR | WKB_HEX), &size);
}

/**
 * @ingroup meos_raster_base_inout
 * @brief Return a Raquet tile from its Well-Known Binary (WKB) representation
 * @param[in] wkb WKB string
 * @param[in] size Size of the string
 * @csqlfn #Raquet_recv(), #Raquet_from_wkb()
 */
Raquet *
raquet_from_wkb(const uint8_t *wkb, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb, NULL);
  return DatumGetRaquetP(type_from_wkb(wkb, size, T_RAQUET));
}

/**
 * @ingroup meos_raster_base_inout
 * @brief Return a Raquet tile from its ASCII hex-encoded Well-Known Binary
 * (HexWKB) representation
 * @param[in] hexwkb HexWKB string
 * @csqlfn #Raquet_from_hexwkb()
 */
Raquet *
raquet_from_hexwkb(const char *hexwkb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(hexwkb, NULL);
  return DatumGetRaquetP(type_from_hexwkb(hexwkb, strlen(hexwkb), T_RAQUET));
}

/**
 * @ingroup meos_raster_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a Raquet tile
 * @param[in] rq Raquet tile
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Raquet_send(), #Raquet_as_wkb()
 */
uint8_t *
raquet_as_wkb(const Raquet *rq, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return datum_as_wkb(RaquetPGetDatum(rq), T_RAQUET, variant, size_out);
}

/**
 * @ingroup meos_raster_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a Raquet tile
 * @param[in] rq Raquet tile
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Raquet_as_hexwkb()
 */
char *
raquet_as_hexwkb(const Raquet *rq, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return (char *) datum_as_wkb(RaquetPGetDatum(rq), T_RAQUET,
    variant | (uint8_t) WKB_HEX, size_out);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_raster_base_constructor
 * @brief Construct a Raquet tile from a QUADBIN cell, its dimensions, a pixel
 * type and a row-major packed pixel array
 * @param[in] quadbin CARTO QUADBIN cell identifying the Web-Mercator tile
 * @param[in] width Tile width in pixels
 * @param[in] height Tile height in pixels
 * @param[in] pixtype Pixel data type
 * @param[in] nodata Nodata sentinel value
 * @param[in] has_nodata Whether nodata filtering is active
 * @param[in] pixels Row-major packed pixel bytes (`width * height *
 * raquet_pixtype_size(pixtype)` bytes), little-endian beyond one byte a pixel
 * @param[in] pixels_size Number of bytes available at @p pixels
 * @csqlfn #Raquet_constructor()
 */
Raquet *
raquet_make(uint64 quadbin, int32 width, int32 height, MeosPixType pixtype,
  double nodata, bool has_nodata, const uint8_t *pixels, size_t pixels_size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pixels, NULL);
  if (! ensure_valid_pixtype((uint8) pixtype))
    return NULL;
  /* The dimensions are taken in the type the SQL surface uses and validated
   * before the narrowing to the uint16 fields below, so that a negative value
   * is rejected here instead of wrapping to a large positive one */
  if (width <= 0 || height <= 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The width and height of a raquet tile must be positive");
    return NULL;
  }
  if (width > UINT16_MAX || height > UINT16_MAX)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The width and height of a raquet tile must be at most %d: %d x %d",
      UINT16_MAX, width, height);
    return NULL;
  }

  size_t npixels = (size_t) width * height * raquet_pixtype_size(pixtype);
  /* The band arrives as a bare pointer, so its length must be given: without it
   * the memcpy below reads past the end of a buffer shorter than the dimensions
   * claim. Mirrors the check in #raster_tile_value_quadbin() */
  if (pixels_size < npixels)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The pixel array has %zu bytes but %zu are required for a %d x %d tile",
      pixels_size, npixels, width, height);
    return NULL;
  }
  size_t size = offsetof(struct Raquet, pixels) + npixels;
  Raquet *result = palloc0(size);
  SET_VARSIZE(result, size);
  result->width = width;
  result->height = height;
  result->quadbin = quadbin;
  result->nodata = nodata;
  result->pixtype = (uint8) pixtype;
  result->has_nodata = has_nodata;
  memcpy(result->pixels, pixels, npixels);
  return result;
}

/**
 * @ingroup meos_raster_base_constructor
 * @brief Return a copy of a Raquet tile
 * @param[in] rq Raquet tile
 */
Raquet *
raquet_copy(const Raquet *rq)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq, NULL);
  Raquet *result = palloc(VARSIZE(rq));
  memcpy(result, rq, VARSIZE(rq));
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the QUADBIN cell of a Raquet tile
 * @return On error return @p UINT64_MAX
 * @csqlfn #Raquet_quadbin()
 */
uint64
raquet_quadbin(const Raquet *rq)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq, UINT64_MAX);
  return rq->quadbin;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the width in pixels of a Raquet tile
 * @csqlfn #Raquet_width()
 */
int
raquet_width(const Raquet *rq)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq, INT_MAX);
  return (int) rq->width;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the height in pixels of a Raquet tile
 * @csqlfn #Raquet_height()
 */
int
raquet_height(const Raquet *rq)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq, INT_MAX);
  return (int) rq->height;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the nodata sentinel value of a Raquet tile
 * @return On error return @p DBL_MAX
 * @csqlfn #Raquet_nodata()
 */
double
raquet_nodata(const Raquet *rq)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq, DBL_MAX);
  return rq->nodata;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the name of the pixel data type of a Raquet tile
 * @param[in] rq Raquet tile
 * @return On error return @p NULL
 * @note The returned name is the one the RaQuet specification writes for the
 * type, that is, one of uint8, int8, uint16, int16, uint32, int32, uint64,
 * int64, float16, float32, or float64, so it compares equal to the `type`
 * field of the tile a RaQuet file holds
 * @csqlfn #Raquet_pixtype()
 */
char *
raquet_pixtype(const Raquet *rq)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq, NULL);
  if (! ensure_valid_pixtype(rq->pixtype))
    return NULL;
  return pstrdup(MEOS_PIXTYPE_CATALOG[rq->pixtype].name);
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return a copy of the pixel bytes of a Raquet tile
 * @param[in] rq Raquet tile
 * @param[out] size_out Number of bytes returned
 * @return On error return @p NULL
 * @note The bytes are row-major and packed, @p width * @p height pixels of
 * @p raquet_pixtype_size() bytes each, which is the layout the tile
 * constructors accept
 * @csqlfn #Raquet_pixels()
 */
uint8_t *
raquet_pixels(const Raquet *rq, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  size_t size = raquet_pixels_size(rq);
  uint8_t *result = palloc(size);
  memcpy(result, rq->pixels, size);
  *size_out = size;
  return result;
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument the spatiotemporal box of a Raquet tile
 * @param[in] rq Raquet tile
 * @param[out] box Spatiotemporal box
 */
void
raquet_set_stbox(const Raquet *rq, STBox *box)
{
  assert(rq); assert(box);
  memset(box, 0, sizeof(STBox));
  double xmin, ymin, xmax, ymax;
  raster_quadbin_bounds(rq->quadbin, &xmin, &ymin, &xmax, &ymax);
  box->xmin = xmin;
  box->xmax = xmax;
  box->ymin = ymin;
  box->ymax = ymax;
  /* A Raquet tile is emitted as planar lon/lat (EPSG:4326), the reference
   * system of the trajectories the sampling functions evaluate it against */
  box->srid = SRID_DEFAULT;
  MEOS_FLAGS_SET_X(box->flags, true);
  MEOS_FLAGS_SET_Z(box->flags, false);
  MEOS_FLAGS_SET_T(box->flags, false);
  MEOS_FLAGS_SET_GEODETIC(box->flags, false);
}

/**
 * @ingroup meos_raster_base_conversion
 * @brief Convert a Raquet tile into a spatiotemporal box
 * @param[in] rq Raquet tile
 * @return The planar X/Y bounding box of the tile (SRID 4326, no T dimension)
 * @csqlfn #Raquet_to_stbox()
 */
STBox *
raquet_to_stbox(const Raquet *rq)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq, NULL);
  STBox box;
  raquet_set_stbox(rq, &box);
  return stbox_copy(&box);
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/**
 * @ingroup meos_raster_base_comp
 * @brief Return -1, 0, or 1 depending on whether the first Raquet tile is
 * less than, equal to, or greater than the second one
 * @param[in] rq1,rq2 Raquet tiles
 * @return On error return @p INT_MAX
 * @csqlfn #Raquet_cmp()
 */
int
raquet_cmp(const Raquet *rq1, const Raquet *rq2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq1, INT_MAX); VALIDATE_NOT_NULL(rq2, INT_MAX);

  if (rq1->quadbin != rq2->quadbin)
    return (rq1->quadbin < rq2->quadbin) ? -1 : 1;
  if (rq1->pixtype != rq2->pixtype)
    return (rq1->pixtype < rq2->pixtype) ? -1 : 1;
  if (rq1->width != rq2->width)
    return (rq1->width < rq2->width) ? -1 : 1;
  if (rq1->height != rq2->height)
    return (rq1->height < rq2->height) ? -1 : 1;
  size_t size1 = raquet_pixels_size(rq1);
  int c = memcmp(rq1->pixels, rq2->pixels, size1);
  return (c < 0) ? -1 : ((c > 0) ? 1 : 0);
}

/**
 * @ingroup meos_raster_base_comp
 * @brief Return true if two Raquet tiles are equal
 * @param[in] rq1,rq2 Raquet tiles
 * @csqlfn #Raquet_eq()
 */
bool
raquet_eq(const Raquet *rq1, const Raquet *rq2)
{
  return raquet_cmp(rq1, rq2) == 0;
}

/**
 * @ingroup meos_raster_base_comp
 * @brief Return true if two Raquet tiles are different
 * @param[in] rq1,rq2 Raquet tiles
 * @csqlfn #Raquet_ne()
 */
bool
raquet_ne(const Raquet *rq1, const Raquet *rq2)
{
  return raquet_cmp(rq1, rq2) != 0;
}

/**
 * @ingroup meos_raster_base_comp
 * @brief Return true if the first Raquet tile is less than the second one
 * @param[in] rq1,rq2 Raquet tiles
 * @csqlfn #Raquet_lt()
 */
bool
raquet_lt(const Raquet *rq1, const Raquet *rq2)
{
  return raquet_cmp(rq1, rq2) < 0;
}

/**
 * @ingroup meos_raster_base_comp
 * @brief Return true if the first Raquet tile is less than or equal to the
 * second one
 * @param[in] rq1,rq2 Raquet tiles
 * @csqlfn #Raquet_le()
 */
bool
raquet_le(const Raquet *rq1, const Raquet *rq2)
{
  return raquet_cmp(rq1, rq2) <= 0;
}

/**
 * @ingroup meos_raster_base_comp
 * @brief Return true if the first Raquet tile is greater than or equal to the
 * second one
 * @param[in] rq1,rq2 Raquet tiles
 * @csqlfn #Raquet_ge()
 */
bool
raquet_ge(const Raquet *rq1, const Raquet *rq2)
{
  return raquet_cmp(rq1, rq2) >= 0;
}

/**
 * @ingroup meos_raster_base_comp
 * @brief Return true if the first Raquet tile is greater than the second one
 * @param[in] rq1,rq2 Raquet tiles
 * @csqlfn #Raquet_gt()
 */
bool
raquet_gt(const Raquet *rq1, const Raquet *rq2)
{
  return raquet_cmp(rq1, rq2) > 0;
}

/*****************************************************************************
 * Hash functions
 *****************************************************************************/

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the 32-bit hash of a Raquet tile
 * @param[in] rq Raquet tile
 * @csqlfn #Raquet_hash()
 * @return On error return @p UINT32_MAX
 */
uint32
raquet_hash(const Raquet *rq)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq, UINT32_MAX);
  return hash_any(((const unsigned char *) rq) + VARHDRSZ,
    (int) raquet_meaningful_size(rq));
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the 64-bit hash of a Raquet tile using a seed
 * @param[in] rq Raquet tile
 * @param[in] seed Seed
 * @csqlfn #Raquet_hash_extended()
 * @return On error return @p UINT64_MAX
 */
uint64
raquet_hash_extended(const Raquet *rq, uint64 seed)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq, UINT64_MAX);
  return hash_any_extended(((const unsigned char *) rq) + VARHDRSZ,
    (int) raquet_meaningful_size(rq), seed);
}

/*****************************************************************************
 * Sampling functions
 *****************************************************************************/

/**
 * @ingroup meos_raster
 * @brief Return the values of a Raquet tile sampled at the instants of a
 * trajectory
 * @note The sampling surface is double-valued whatever the pixel type of the
 * band, as #raster_tile_value_quadbin() describes
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] rq Raquet tile
 * @csqlfn #Raster_tile_value()
 */
Temporal *
raster_tile_value(const Temporal *traj, const Raquet *rq)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq, NULL); VALIDATE_NOT_NULL(traj, NULL);
  return raster_tile_value_quadbin(traj, rq->pixels, raquet_pixels_size(rq),
    rq->width, rq->height, rq->quadbin, (MeosPixType) rq->pixtype, rq->nodata,
    rq->has_nodata);
}

/**
 * @ingroup meos_raster
 * @brief Return the values of an array of Raquet tiles sampled at the instants
 * of a trajectory
 * @details A trajectory that leaves a single tile is sampled from the whole set
 * of tiles covering it, each tile contributing the instants that fall inside
 * it. Tiles of one zoom level partition the plane, so they contribute disjoint
 * instants. Tiles of different zoom levels overlap, and where two of them
 * sample the same instant the value of the tile of higher zoom is kept, that
 * being the one carrying the finer resolution.
 * @param[in] rqarr Array of Raquet tiles
 * @param[in] count Number of tiles in the array
 * @param[in] traj Trajectory (temporal geometry point)
 * @return A temporal float, or @p NULL when no instant of @p traj falls inside
 * a tile or survives nodata filtering
 * @csqlfn #Raster_tile_value_array()
 */
Temporal *
raster_tile_value_array(const Temporal *traj, const Raquet **rqarr, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rqarr, NULL); VALIDATE_NOT_NULL(traj, NULL);
  if (! ensure_positive(count))
    return NULL;

  /* The instants of the trajectory are the slots the tiles compete for, since a
   * sampled instant carries the timestamp of the instant it was sampled at */
  int ninsts;
  const TInstant **insts = temporal_insts_p(traj, &ninsts);
  double *values = palloc(sizeof(double) * ninsts);
  int *zooms = palloc(sizeof(int) * ninsts);
  for (int i = 0; i < ninsts; i++)
    zooms[i] = -1;                  /* no tile has covered this instant yet */

  for (int i = 0; i < count; i++)
  {
    if (rqarr[i] == NULL)
      continue;
    Temporal *sampled = raster_tile_value(traj, rqarr[i]);
    if (sampled == NULL)
      continue;
    int zoom = (int) raster_quadbin_zoom(rqarr[i]->quadbin);
    int nsampled;
    const TInstant **sinsts = temporal_insts_p(sampled, &nsampled);
    /* Both arrays are ordered by timestamp, so one walk places every sampled
     * value in its slot */
    int k = 0;
    for (int j = 0; j < nsampled; j++)
    {
      while (k < ninsts && insts[k]->t < sinsts[j]->t)
        k++;
      if (k == ninsts)
        break;
      if (insts[k]->t == sinsts[j]->t && zoom > zooms[k])
      {
        zooms[k] = zoom;
        values[k] = DatumGetFloat8(tinstant_value(sinsts[j]));
      }
    }
    pfree(sinsts); pfree(sampled);
  }

  TInstant **result_insts = palloc(sizeof(TInstant *) * ninsts);
  int nresult = 0;
  for (int i = 0; i < ninsts; i++)
    if (zooms[i] >= 0)
      result_insts[nresult++] =
        tinstant_make(Float8GetDatum(values[i]), T_TFLOAT, insts[i]->t);
  pfree(insts); pfree(values); pfree(zooms);

  if (nresult == 0)
  {
    pfree(result_insts);
    return NULL;
  }
  return (Temporal *) tsequence_make_free(result_insts, nresult, true, true,
    DISCRETE, NORMALIZE);
}

/*****************************************************************************/
