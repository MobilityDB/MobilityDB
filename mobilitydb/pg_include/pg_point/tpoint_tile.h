/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief Functions for spatiotemporal bounding boxes.
 */

#ifndef __TPOINT_TILE_H__
#define __TPOINT_TILE_H__

/* MobilityDB */
#include "general/temporal.h"

#define MAXDIMS 4

/*****************************************************************************/

/**
 * Structure for storing a bit matrix
 */
typedef struct
{
  int numdims;           /**< Number of dimensions */
  int count[MAXDIMS];    /**< Number of elements in each dimension */
  uint8_t byte[1];       /**< beginning of variable-length data */
} BitMatrix;

/**
 * Struct for storing the state that persists across multiple calls generating
 * a multidimensional grid
 */
typedef struct STboxGridState
{
  bool done;           /**< True when all tiles have been processed */
  int i;               /**< Number of current tile */
  double size;         /**< Size of the x, y, and z dimension */
  int64 tunits;        /**< Size of the time dimension */
  STBOX box;           /**< Bounding box of the grid */
  Temporal *temp;      /**< Optional temporal point to be split */
  BitMatrix *bm;       /**< Optional bit matrix for speeding up
                            the computation of the split functions */
  double x;            /**< Minimum x value of the current tile */
  double y;            /**< Minimum y value of the current tile */
  double z;            /**< Minimum z value of the current tile */
  TimestampTz t;       /**< Minimum t value of the current tile */
  int coords[MAXDIMS]; /**< Coordinates of the current tile */
} STboxGridState;

/*****************************************************************************/

#endif
