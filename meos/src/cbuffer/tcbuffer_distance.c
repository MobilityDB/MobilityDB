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
 * @brief Temporal distance for temporal circular buffers
 */

#include "cbuffer/tcbuffer_distance.h"

/* MEOS */
#include <meos.h>
#include <meos_cbuffer.h>
#include <meos_internal.h>
#include "point/pgis_types.h"
#include "point/tpoint_spatialfuncs.h"
#include "cbuffer/tcbuffer.h"
#include "cbuffer/tcbuffer_spatialfuncs.h"

/*****************************************************************************
 * Distance
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_dist
 * @brief Return the distance between two circular buffers
 */
Datum
datum_cbuffer_distance(Datum cbuf1, Datum cbuf2)
{
  GSERIALIZED *geom1 = cbuffer_geom(DatumGetCbufferP(cbuf1));
  GSERIALIZED *geom2 = cbuffer_geom(DatumGetCbufferP(cbuf2));
  return geom_distance2d(geom1, geom2);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_dist
 * @brief Return the distance between a circular buffer and a geometry
 * @return On error return -1.0
 * @csqlfn #Distance_cbuffer_geo()
 */
double
distance_cbuffer_geo(const Cbuffer *cbuf, const GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf) || ! ensure_not_null((void *) gs) ||
      gserialized_is_empty(gs))
    return -1.0;

  GSERIALIZED *geo = cbuffer_geom(cbuf);
  double result = geom_distance2d(geo, gs);
  pfree(geo);
  return result;
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the distance between a circular buffer and a spatiotemporal box
 * @return On error return -1.0
 * @csqlfn #Distance_cbuffer_stbox()
 */
double
distance_cbuffer_stbox(const Cbuffer *cbuf, const STBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf) || ! ensure_not_null((void *) box) ||
      ! ensure_has_X_stbox(box))
    return -1.0;

  GSERIALIZED *geo1 = cbuffer_geom(cbuf);
  GSERIALIZED *geo2 = stbox_to_geo(box);
  double result = geom_distance2d(geo1, geo2);
  pfree(geo1); pfree(geo2); 
  return result;
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the distance between a circular buffer and a geometry
 * @return On error return -1.0
 * @csqlfn #Distance_geo_cbuffer()
 */
double
distance_cbuffer_cbuffer(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf1) || ! ensure_not_null((void *) cbuf2))
    return -1.0;

  GSERIALIZED *geo1 = cbuffer_geom(cbuf1);
  GSERIALIZED *geo2 = cbuffer_geom(cbuf2);
  double result = geom_distance2d(geo1, geo2);
  pfree(geo1); pfree(geo2);
  return result;
}

/*****************************************************************************
 * Temporal distance
 * **** TO BE IMPLEMENTED ****  
 *****************************************************************************/

/**
 * @ingroup meos_temporal_dist
 * @brief Return the temporal distance between a geometry point and a temporal
 * circular buffer
 * @csqlfn #Distance_tcbuffer_point()
 */
Temporal *
distance_tcbuffer_point(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      gserialized_is_empty(gs) || ! ensure_point_type(gs))
    return NULL;

  Temporal *tpoint = tcbuffer_tgeompoint(temp);
  Temporal *result = distance_tpoint_point((const Temporal *) tpoint, gs);
  pfree(tpoint);
  return result;
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the temporal distance between a temporal circular buffer and
 * a circular buffer
 * @param[in] temp Temporal point
 * @param[in] cbuf Circular buffer
 * @csqlfn #Distance_tcbuffer_cbuffer()
 */
Temporal *
distance_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf)
{
  GSERIALIZED *geom = cbuffer_geom(cbuf);
  Temporal *tpoint = tcbuffer_tgeompoint(temp);
  Temporal *result = distance_tpoint_point(tpoint, geom);
  pfree(geom);
  return result;
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the temporal distance between two temporal circular buffers
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Distance_tcbuffer_tcbuffer()
 */
Temporal *
distance_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *tpoint1 = tcbuffer_tgeompoint(temp1);
  Temporal *tpoint2 = tcbuffer_tgeompoint(temp2);
  Temporal *result = distance_tpoint_tpoint(tpoint1, tpoint2);
  pfree(tpoint1); pfree(tpoint2);
  return result;
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between a spatiotemporal box
 * and a geometry
 * @param[in] box Spatiotemporal box
 * @param[in] cbuf Circular buffer
 * @csqlfn #NAD_stbox_cbuffer()
 */
double
nad_stbox_cbuffer(const STBox *box, const Cbuffer *cbuf)
{
  /* Ensure validity of the arguments */
  if (! ensure_valid_stbox_cbuffer(box, cbuf))
      // ! ensure_same_spatial_dimensionality_stbox_gs(box, cbuf))
    return -1.0;

  Datum geobox = PointerGetDatum(stbox_to_geo(box));
  Datum geocbuf = PointerGetDatum(cbuffer_geom(cbuf));
  double result = DatumGetFloat8(datum_geom_distance2d(geobox, geocbuf));
  pfree(DatumGetPointer(geobox)); pfree(DatumGetPointer(geocbuf));
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach instant of the temporal circular buffer
 * and a geometry
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #NAI_tcbuffer_geo()
 */
TInstant *
nai_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  if (gserialized_is_empty(gs))
    return NULL;
  Temporal *tpoint = tcbuffer_tgeompoint(temp);
  TInstant *resultgeom = nai_tpoint_geo(tpoint, gs);
  /* We do not call the function tgeompointinst_tcbufferinst to avoid
   * roundoff errors. The closest point may be at an exclusive bound. */
  Datum value;
  temporal_value_at_timestamptz(temp, resultgeom->t, false, &value);
  TInstant *result = tinstant_make_free(value, temp->temptype, resultgeom->t);
  pfree(tpoint); pfree(resultgeom);
  return result;
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach instant of the circular buffer and a
 * temporal circular buffer
 * @param[in] temp Temporal point
 * @param[in] cbuf Circular buffer
 * @csqlfn #NAI_tcbuffer_cbuffer()
 */
TInstant *
nai_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf)
{
  GSERIALIZED *geom = cbuffer_geom(cbuf);
  Temporal *tpoint = tcbuffer_tgeompoint(temp);
  TInstant *resultgeom = nai_tpoint_geo(tpoint, geom);
  /* We do not call the function tgeompointinst_tcbufferinst to avoid
   * roundoff errors. The closest point may be at an exclusive bound. */
  Datum value;
  temporal_value_at_timestamptz(temp, resultgeom->t, false, &value);
  TInstant *result = tinstant_make_free(value, temp->temptype, resultgeom->t);
  pfree(tpoint); pfree(resultgeom); pfree(geom);
  return result;
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach instant of two temporal circular buffers
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #NAI_tcbuffer_tcbuffer()
 */
TInstant *
nai_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *dist = distance_tcbuffer_tcbuffer(temp1, temp2);
  if (dist == NULL)
    return NULL;

  const TInstant *min = temporal_min_instant((const Temporal *) dist);
  pfree(dist);
  /* The closest point may be at an exclusive bound. */
  Datum value;
  temporal_value_at_timestamptz(temp1, min->t, false, &value);
  return tinstant_make_free(value, temp1->temptype, min->t);
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance of a temporal circular buffer
 * and a geometry
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #NAD_tcbuffer_geo()
 */
double
nad_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_temporal_isof_type(temp, T_TCBUFFER) || 
      ! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return -1.0;

  GSERIALIZED *trav = tcbuffer_traversed_area(temp);
  double result = geom_distance2d(trav, gs);
  pfree(trav);
  return result;
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance of a temporal circular buffer
 * and a spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #NAD_tcbuffer_geo()
 */
double
nad_tcbuffer_stbox(const Temporal *temp, const STBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) box) ||
      ! ensure_temporal_isof_type(temp, T_TCBUFFER) || 
      ! ensure_has_X_stbox(box))
    return -1.0;

  GSERIALIZED *trav = tcbuffer_traversed_area(temp);
  GSERIALIZED *geo = stbox_to_geo(box);
  double result = geom_distance2d(trav, geo);
  pfree(trav);
  return result;
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance of a temporal circular buffer
 * and a circular buffer
 * @param[in] temp Temporal point
 * @param[in] cbuf Circular buffer
 * @csqlfn #NAD_tcbuffer_cbuffer()
 */
double
nad_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf)
{
  GSERIALIZED *geom = cbuffer_geom(cbuf);
  GSERIALIZED *trav = tcbuffer_traversed_area(temp);
  double result = geom_distance2d(trav, geom);
  pfree(trav); pfree(geom);
  return result;
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance of two temporal circular buffers
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #NAD_tcbuffer_tcbuffer()
 */
double
nad_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *dist = distance_tcbuffer_tcbuffer(temp1, temp2);
  if (dist == NULL)
    return -1;
  return DatumGetFloat8(temporal_min_value(dist));
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

/**
 * @ingroup meos_temporal_dist
 * @brief Return the line connecting the nearest approach point between a
 * geometry and a temporal circular buffer
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #Shortestline_tcbuffer_geo()
 */
GSERIALIZED *
shortestline_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  if (gserialized_is_empty(gs))
    return NULL;
  GSERIALIZED *trav = tcbuffer_traversed_area(temp);
  GSERIALIZED *result = geom_shortestline2d(trav, gs);
  pfree(trav);
  return result;
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the line connecting the nearest approach point between a
 * circular buffer and a temporal circular buffer
 * @param[in] temp Temporal point
 * @param[in] cbuf Circular buffer
 * @csqlfn #Shortestline_tcbuffer_cbuffer()
 */
GSERIALIZED *
shortestline_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf)
{
  GSERIALIZED *geom = cbuffer_geom(cbuf);
  GSERIALIZED *trav = tcbuffer_traversed_area(temp);
  GSERIALIZED *result = geom_shortestline2d(trav, geom);
  pfree(geom); pfree(trav);
  return result;
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the line connecting the nearest approach point between two
 * temporal circular buffers
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Shortestline_tcbuffer_tcbuffer()
 */
GSERIALIZED *
shortestline_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *tpoint1 = tcbuffer_tgeompoint(temp1);
  Temporal *tpoint2 = tcbuffer_tgeompoint(temp2);
  GSERIALIZED *result = shortestline_tpoint_tpoint(tpoint1, tpoint2);
  pfree(tpoint1); pfree(tpoint2);
  return result;
}

/*****************************************************************************/
