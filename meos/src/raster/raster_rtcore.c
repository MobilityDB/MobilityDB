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

#include "librtcore.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_raster.h>
#include "temporal/temporal.h"

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the number of bands of a raster
 * @param[in] rast Raster
 * @return On error, return -1
 */
int
raster_num_bands(const Raster *rast)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rast, -1);
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
