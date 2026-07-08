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
 * @brief Internal definition of the Raquet raster-tile value type.
 *
 * A Raquet value is a single CARTO Raquet raster chip: a self-describing
 * Web-Mercator tile identified by a QUADBIN cell, carrying a row-major packed
 * pixel array. It is a GDAL-free MEOS value type; the trajectory-sampling
 * kernel operates on it directly. See @ref meos_raster.h for the public API.
 */

#ifndef __RAQUET_H__
#define __RAQUET_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_raster.h>

/*****************************************************************************
 * Type definitions
 *****************************************************************************/

/**
 * @brief Structure to represent a Raquet raster tile.
 * @details Variable-length (varlena) value. The field order is chosen so that
 * the 8-byte members are naturally aligned and no internal padding is needed,
 * hence the pixel payload starts at a fixed offset. The pixels are stored
 * row-major, tightly packed, `width * height * raquet_pixtype_size(pixtype)`
 * bytes; the tile geotransform is fully determined by @p quadbin.
 */
struct Raquet
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  uint16 width;         /**< Tile width in pixels */
  uint16 height;        /**< Tile height in pixels */
  uint64 quadbin;       /**< CARTO QUADBIN cell identifying the Web-Mercator tile */
  double nodata;        /**< Nodata sentinel value */
  uint8 pixtype;        /**< Pixel data type (::MeosPixType) */
  bool has_nodata;      /**< Whether nodata filtering is active */
  uint8 pixels[FLEXIBLE_ARRAY_MEMBER]; /**< Row-major packed pixel bytes */
};

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

#if MEOS
  #define DatumGetRaquetP(X)     ((Raquet *) DatumGetPointer(X))
#else
  #define DatumGetRaquetP(X)     ((Raquet *) PG_DETOAST_DATUM(X))
#endif /* MEOS */
#define RaquetPGetDatum(X)       PointerGetDatum(X)
#define PG_GETARG_RAQUET_P(X)    ((Raquet *) PG_GETARG_VARLENA_P(X))
#define PG_RETURN_RAQUET_P(X)    PG_RETURN_POINTER(X)

/*****************************************************************************
 * Internal accessors
 *****************************************************************************/

/**
 * @brief Return the size in bytes of a single pixel of the given type
 */
static inline size_t
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
 * @brief Return the number of pixel bytes of a Raquet tile
 */
static inline size_t
raquet_pixels_size(const Raquet *rq)
{
  return (size_t) rq->width * rq->height *
    raquet_pixtype_size((MeosPixType) rq->pixtype);
}

#endif /* __RAQUET_H__ */
