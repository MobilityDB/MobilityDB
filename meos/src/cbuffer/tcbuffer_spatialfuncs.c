/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Spatial functons for temporal circular buffers
 */

// #include "cbuffer/tcbuffer_distance.h"

/* MEOS */
#include <meos.h>
#include <meos_cbuffer.h>
#include <meos_internal.h>
#include "geo/pgis_types.h"
#include "geo/tgeo_spatialfuncs.h"
#include "cbuffer/tcbuffer.h"
// #include "cbuffer/tcbuffer_spatialfuncs.h"

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

/**
 * @brief Return a cbuffer interpolated from a cbuffer segment with respect to 
 * the fraction of its total length
 * @param[in] start,end Circular buffers defining the segment
 * @param[in] ratio Float between 0 and 1 representing the fraction of the
 * total length of the segment where the interpolated buffer must be located
 */
Datum
cbuffersegm_interpolate(Datum start, Datum end, long double ratio)
{
  Cbuffer *cbuf1 = DatumGetCbufferP(start);
  Cbuffer *cbuf2 = DatumGetCbufferP(end);
  Datum d1 = PointerGetDatum(&cbuf1->point);
  Datum d2 = PointerGetDatum(&cbuf2->point);
  GSERIALIZED *point = 
    DatumGetGserializedP(geosegm_interpolate_point(d1, d2, ratio));
  double radius = cbuf1->radius + 
    (double) ((long double)(cbuf2->radius - cbuf1->radius) * ratio);
  Cbuffer *result = cbuffer_make(point, radius);
  return PointerGetDatum(result);
}

/**
 * @brief Ensure the validity of a temporal circular buffer and a circular buffer
 */
bool
ensure_valid_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf)
{
  if (ensure_not_null((void *) temp) && ensure_not_null((void *) cbuf) &&
      ensure_temporal_isof_type(temp, T_TCBUFFER) &&
      ensure_same_srid(tspatial_srid(temp), cbuffer_srid(cbuf)))
    return true;
  return false;
}

/**
 * @brief Ensure the validity of two temporal circular buffers
 */
bool
ensure_valid_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  if (ensure_not_null((void *) temp1) && ensure_not_null((void *) temp2) &&
      ensure_temporal_isof_type(temp1, T_TCBUFFER) &&
      ensure_temporal_isof_type(temp2, T_TCBUFFER) &&
      ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)))
    return true;
  return false;
}

/**
 * @brief Ensure the validity of a spatiotemporal box and a geometry
 */
bool
ensure_valid_stbox_cbuffer(const STBox *box, const Cbuffer *cbuf)
{
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) cbuf) ||
      ! ensure_has_X_stbox(box) || 
      ! ensure_same_srid(box->srid, cbuffer_srid(cbuf)))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a temporal circular buffer and a geometry
 * @note The geometry can be empty since some functions such atGeometry or
 * minusGeometry return different result on empty geometries.
 */
bool
ensure_valid_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_temporal_isof_type(temp, T_TCBUFFER) ||
      ! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)))
    return false;
  return true;
}

/*****************************************************************************
 * Traversed area 
 *****************************************************************************/

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer
 * @param[in] temp Temporal circular buffer
 * @csqlfn #Tcbuffer_traversed_area()
 */
GSERIALIZED *
tcbuffer_traversed_area(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || 
      ! ensure_temporal_isof_type(temp, T_TCBUFFER))
    return NULL;

  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Function %s not implemented", __func__);
  return NULL;
}

/*****************************************************************************/
