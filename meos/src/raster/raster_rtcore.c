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
#include <meos_internal.h>
#include <meos_raster.h>
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
 * @return On error, return @p NULL
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
 * @return On error, return @p NULL
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
 * @return On error, return @p NULL
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
 * @return On error, return @p NULL
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
 * @return On error, return @p NULL
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
 * @ingroup meos_raster_base_accessor
 * @brief Return the number of bands of a raster
 * @param[in] rast Raster
 * @return On error, return -1
 * @csqlfn #Raster_num_bands()
 */
int
raster_num_bands(const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, INT_MAX);
  rt_raster raster = rt_raster_deserialize((void *) rast, 1);
  if (! raster)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not deserialize raster");
    return -1;
  }
  int result = (int) rt_raster_get_num_bands(raster);
  rt_raster_destroy(raster);
  return result;
}

/*****************************************************************************/
