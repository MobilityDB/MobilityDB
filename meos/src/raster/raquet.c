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
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <pgtypes.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/temporal.h"
#include "temporal/type_inout.h"

/*****************************************************************************
 * Validity functions
 *****************************************************************************/

/**
 * @brief Ensure that a pixel type code is one of the supported values
 */
static bool
ensure_valid_pixtype(uint8 pixtype)
{
  if (pixtype > (uint8) MEOS_PT_FLOAT64)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Unknown raquet pixel type code: %u", pixtype);
    return false;
  }
  return true;
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
 * raquet_pixtype_size(pixtype)` bytes)
 * @csqlfn #Raquet_constructor()
 */
Raquet *
raquet_make(uint64 quadbin, uint16 width, uint16 height, MeosPixType pixtype,
  double nodata, bool has_nodata, const uint8_t *pixels)
{
  VALIDATE_NOT_NULL(pixels, NULL);
  if (! ensure_valid_pixtype((uint8) pixtype))
    return NULL;
  if (width == 0 || height == 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The width and height of a raquet tile must be positive");
    return NULL;
  }

  size_t npixels = (size_t) width * height * raquet_pixtype_size(pixtype);
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
 * @csqlfn #Raquet_quadbin()
 */
uint64
raquet_quadbin(const Raquet *rq)
{
  VALIDATE_NOT_NULL(rq, 0);
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
  VALIDATE_NOT_NULL(rq, -1);
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
  VALIDATE_NOT_NULL(rq, -1);
  return (int) rq->height;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the nodata sentinel value of a Raquet tile
 * @csqlfn #Raquet_nodata()
 */
double
raquet_nodata(const Raquet *rq)
{
  VALIDATE_NOT_NULL(rq, 0.0);
  return rq->nodata;
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/**
 * @ingroup meos_raster_base_comp
 * @brief Return -1, 0, or 1 depending on whether the first Raquet tile is
 * less than, equal to, or greater than the second one
 * @param[in] rq1,rq2 Raquet tiles
 * @csqlfn #Raquet_cmp()
 */
int
raquet_cmp(const Raquet *rq1, const Raquet *rq2)
{
  assert(rq1); assert(rq2);
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
  assert(rq1); assert(rq2);
  return raquet_cmp(rq1, rq2) == 0;
}

/*****************************************************************************
 * Sampling functions
 *****************************************************************************/

/**
 * @ingroup meos_raster
 * @brief Return the values of a Raquet tile sampled at the instants of a
 * trajectory
 * @param[in] rq Raquet tile
 * @param[in] traj Trajectory (temporal geometry point)
 * @csqlfn #Raster_tile_value()
 */
Temporal *
raster_tile_value(const Raquet *rq, const Temporal *traj)
{
  VALIDATE_NOT_NULL(rq, NULL); VALIDATE_NOT_NULL(traj, NULL);
  return raster_tile_value_quadbin(rq->pixels, rq->width, rq->height,
    rq->quadbin, (MeosPixType) rq->pixtype, rq->nodata, rq->has_nodata, traj);
}

/*****************************************************************************/
