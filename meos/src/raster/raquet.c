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
#include <limits.h>
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
 * Pixel type functions
 *****************************************************************************/

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the size in bytes of a single pixel of the given type
 * @param[in] pixtype Pixel data type
 */
size_t
raquet_pixtype_size(MeosPixType pixtype)
{
  switch (pixtype)
  {
    case MEOS_PT_UINT8:   return 1;
    case MEOS_PT_INT16:   return 2;
    case MEOS_PT_INT32:   return 4;
    case MEOS_PT_FLOAT32: return 4;
    case MEOS_PT_FLOAT64: return 8;
    default:              return 0;
  }
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the pixel data type corresponding to a name
 * @param[in] str Pixel type name: UINT8, INT16, INT32, FLOAT32, or FLOAT64
 * @note This is the parser counterpart of #raquet_pixtype()
 */
MeosPixType
raquet_pixtype_from_string(const char *str)
{
  VALIDATE_NOT_NULL(str, MEOS_PT_UINT8);
  if (strcmp(str, "UINT8") == 0)   return MEOS_PT_UINT8;
  if (strcmp(str, "INT16") == 0)   return MEOS_PT_INT16;
  if (strcmp(str, "INT32") == 0)   return MEOS_PT_INT32;
  if (strcmp(str, "FLOAT32") == 0) return MEOS_PT_FLOAT32;
  if (strcmp(str, "FLOAT64") == 0) return MEOS_PT_FLOAT64;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Unknown pixel type \"%s\": use UINT8, INT16, INT32, FLOAT32, or FLOAT64",
    str);
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
 * @param[in] pixels_size Number of bytes available at @p pixels
 * @csqlfn #Raquet_constructor()
 */
Raquet *
raquet_make(uint64 quadbin, int32 width, int32 height, MeosPixType pixtype,
  double nodata, bool has_nodata, const uint8_t *pixels, size_t pixels_size)
{
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

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the name of the pixel data type of a Raquet tile
 * @param[in] rq Raquet tile
 * @return On error return @p NULL
 * @note The returned name is the one accepted by the tile constructors, that
 * is, one of UINT8, INT16, INT32, FLOAT32, or FLOAT64
 * @csqlfn #Raquet_pixtype()
 */
char *
raquet_pixtype(const Raquet *rq)
{
  VALIDATE_NOT_NULL(rq, NULL);
  switch ((MeosPixType) rq->pixtype)
  {
    case MEOS_PT_UINT8:   return pstrdup("UINT8");
    case MEOS_PT_INT16:   return pstrdup("INT16");
    case MEOS_PT_INT32:   return pstrdup("INT32");
    case MEOS_PT_FLOAT32: return pstrdup("FLOAT32");
    case MEOS_PT_FLOAT64: return pstrdup("FLOAT64");
    default:
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Unknown raquet pixel type code: %u", rq->pixtype);
      return NULL;
  }
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
 * @csqlfn #Raquet_cmp()
 */
int
raquet_cmp(const Raquet *rq1, const Raquet *rq2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq1, false); VALIDATE_NOT_NULL(rq2, false);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rq1, false); VALIDATE_NOT_NULL(rq2, false);
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
 */
uint32
raquet_hash(const Raquet *rq)
{
  VALIDATE_NOT_NULL(rq, INT_MAX);
  return hash_any(((const unsigned char *) rq) + VARHDRSZ,
    (int) raquet_meaningful_size(rq));
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the 64-bit hash of a Raquet tile using a seed
 * @param[in] rq Raquet tile
 * @param[in] seed Seed
 * @csqlfn #Raquet_hash_extended()
 */
uint64
raquet_hash_extended(const Raquet *rq, uint64 seed)
{
  VALIDATE_NOT_NULL(rq, LONG_MAX);
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
 * @param[in] rq Raquet tile
 * @param[in] traj Trajectory (temporal geometry point)
 * @csqlfn #Raster_tile_value()
 */
Temporal *
raster_tile_value(const Raquet *rq, const Temporal *traj)
{
  VALIDATE_NOT_NULL(rq, NULL); VALIDATE_NOT_NULL(traj, NULL);
  return raster_tile_value_quadbin(rq->pixels, raquet_pixels_size(rq),
    rq->width, rq->height, rq->quadbin, (MeosPixType) rq->pixtype, rq->nodata,
    rq->has_nodata, traj);
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
raster_tile_value_array(const Raquet **rqarr, int count, const Temporal *traj)
{
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
    Temporal *sampled = raster_tile_value(rqarr[i], traj);
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
