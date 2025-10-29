/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief Functions for rounding the float coordinates of geometries
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "geo/postgis_funcs.h"
#include "geo/tgeo_spatialfuncs.h"

#include <utils/jsonb.h>
#include <utils/numeric.h>
#include <pgtypes.h>

/*****************************************************************************
 * Geometry/Geography
 *****************************************************************************/

/**
 * @brief Set the precision of the coordinates of the n-th point in a point
 * array to a number of decimal places
 */
static void
round_point_n(POINTARRAY *points, uint32_t n, int maxdd, bool hasz, bool hasm)
{
  /* N.B. lwpoint->point can be of 2, 3, or 4 dimensions depending on
   * the values of the arguments hasz and hasm !!! */
  POINT4D *pt = (POINT4D *) getPoint_internal(points, n);
  pt->x = float8_round(pt->x, maxdd);
  pt->y = float8_round(pt->y, maxdd);
  if (hasz && hasm)
  {
    pt->z = float8_round(pt->z, maxdd);
    pt->m = float8_round(pt->m, maxdd);
  }
  else if (hasz)
    pt->z = float8_round(pt->z, maxdd);
  else if (hasm)
    /* The m co ordinate is located at the third double of the point */
    pt->z = float8_round(pt->z, maxdd);
  return;
}

/**
 * @ingroup meos_internal_geo_base_transf
 * @brief Return a point with the coordinates set to a number of decimal places
 * @param[in] gs Geometry/geography
 * @param[in] maxdd Maximum number of decimal digits
 */
GSERIALIZED *
point_round(const GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == POINTTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWPOINT *point = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
  round_point_n(point->point, 0, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) point);
  lwpoint_free(point);
  return result;
}

/**
 * @brief Set the precision of the coordinates of a line to a number of
 * decimal places
 */
static void
round_lwline(LWLINE *line, int maxdd, bool hasz, bool hasm)
{
  int npoints = line->points->npoints;
  for (int i = 0; i < npoints; i++)
    round_point_n(line->points, i, maxdd, hasz, hasm);
  return;
}

/**
 * @brief Return a line with the coordinates set to a number of decimal places
 */
static GSERIALIZED *
round_linestring(const GSERIALIZED *gs,int maxdd)
{
  assert(gserialized_get_type(gs) == LINETYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWLINE *line = lwgeom_as_lwline(lwgeom_from_gserialized(gs));
  round_lwline(line, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) line);
  lwline_free(line);
  return result;
}

/**
 * @brief Set the precision of the coordinates of a triangle to a number of
 * decimal places
 */
static void
round_lwtriangle(LWTRIANGLE *triangle, int maxdd, bool hasz, bool hasm)
{
  int npoints = triangle->points->npoints;
  for (int i = 0; i < npoints; i++)
    round_point_n(triangle->points, i, maxdd, hasz, hasm);
  return;
}

/**
 * @brief Return a triangle with the precision of the coordinates set to a
 * number of decimal places
 */
static GSERIALIZED *
round_triangle(const GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == TRIANGLETYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWTRIANGLE *triangle = lwgeom_as_lwtriangle(lwgeom_from_gserialized(gs));
  round_lwtriangle(triangle, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) triangle);
  lwfree(triangle);
  return result;
}

/**
 * @brief Set the precision of the coordinates of a circular string to a number
 * of decimal places
 */
static void
round_lwcircstring(LWCIRCSTRING *circstring, int maxdd, bool hasz,
  bool hasm)
{
  int npoints = circstring->points->npoints;
  for (int i = 0; i < npoints; i++)
    round_point_n(circstring->points, i, maxdd, hasz, hasm);
  return;
}

/**
 * @brief Return a circular string with the precision of the coordinates set to
 * a number of decimal places
 */
static GSERIALIZED *
round_circularstring(const GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == CIRCSTRINGTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWCIRCSTRING *circstring = lwgeom_as_lwcircstring(lwgeom_from_gserialized(gs));
  round_lwcircstring(circstring, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) circstring);
  lwfree(circstring);
  return result;
}

/**
 * @brief Set the precision of the coordinates of a polygon to a number of
 * decimal places
 */
static void
round_lwpoly(LWPOLY *poly, int maxdd, bool hasz, bool hasm)
{
  int nrings = poly->nrings;
  for (int i = 0; i < nrings; i++)
  {
    POINTARRAY *points = poly->rings[i];
    int npoints = points->npoints;
    for (int j = 0; j < npoints; j++)
      round_point_n(points, j, maxdd, hasz, hasm);
  }
  return;
}

/**
 * @brief Return a polygon with the precision of the coordinates set to a
 * number of decimal places
 */
static GSERIALIZED *
round_polygon(const GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == POLYGONTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWPOLY *poly = lwgeom_as_lwpoly(lwgeom_from_gserialized(gs));
  round_lwpoly(poly, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) poly);
  lwpoly_free(poly);
  return result;
}

/**
 * @brief Set the precision of the coordinates of a multipoint to a number of
 * decimal places
 */
static void
round_lwmpoint(LWMPOINT *mpoint, int maxdd, bool hasz, bool hasm)
{
  int ngeoms = mpoint->ngeoms;
  for (int i = 0; i < ngeoms; i++)
  {
    LWPOINT *point = mpoint->geoms[i];
    round_point_n(point->point, 0, maxdd, hasz, hasm);
  }
  return;
}

/**
 * @brief Return a multipoint with the precision of the coordinates set to a
 * number of decimal places
 */
static GSERIALIZED *
round_multipoint(const GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == MULTIPOINTTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWMPOINT *mpoint =  lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
  round_lwmpoint(mpoint, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) mpoint);
  lwfree(mpoint);
  return result;
}

/**
 * @brief Set the precision of the coordinates of a multilinestring to a
 * number of decimal places
 */
static void
round_lwmline(LWMLINE *mline, int maxdd, bool hasz, bool hasm)
{
  int ngeoms = mline->ngeoms;
  for (int i = 0; i < ngeoms; i++)
  {
    LWLINE *line = mline->geoms[i];
    int npoints = line->points->npoints;
    for (int j = 0; j < npoints; j++)
      round_point_n(line->points, j, maxdd, hasz, hasm);
  }
  return;
}

/**
 * @brief Return a multilinestring with the precision of the coordinates set to
 * a number of decimal places
 */
static GSERIALIZED *
round_multilinestring(const GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == MULTILINETYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWMLINE *mline = lwgeom_as_lwmline(lwgeom_from_gserialized(gs));
  round_lwmline(mline, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) mline);
  lwfree(mline);
  return result;
}

/**
 * @brief Set the precision of the coordinates of a multipolygon to a number of
 * decimal places
 */
static void
round_lwmpoly(LWMPOLY *mpoly, int maxdd, bool hasz, bool hasm)
{
  int ngeoms = mpoly->ngeoms;
  for (int i = 0; i < ngeoms; i++)
  {
    LWPOLY *poly = mpoly->geoms[i];
    round_lwpoly(poly, maxdd, hasz, hasm);
  }
  return;
}

/**
 * @brief Return a multipolygon with the precision of the coordinates set to a
 * number of decimal places
 */
static GSERIALIZED *
round_multipolygon(const GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == MULTIPOLYGONTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWMPOLY *mpoly = lwgeom_as_lwmpoly(lwgeom_from_gserialized(gs));
  round_lwmpoly(mpoly, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) mpoly);
  lwfree(mpoly);
  return result;
}

/**
 * @brief Set the precision of the coordinates of a compound curve to a
 * number of decimal places
 * @note A CompoundCurve is a single continuous curve that may contain both
 * circular arc segments and linear segments. That means that in addition to 
 * having well-formed components, the end point of every component (except the
 * last) must be coincident with the start point of the following component.
 * https://postgis.net/docs/using_postgis_dbmanagement.html#CompoundCurve
 */
static void
round_lwcompound(LWCOMPOUND *comp, int maxdd, bool hasz, bool hasm)
{
  int ngeoms = comp->ngeoms;
  for (int i = 0; i < ngeoms; i++)
  {
    LWGEOM *geom = comp->geoms[i];
    assert(geom->type == LINETYPE || geom->type == CIRCSTRINGTYPE);
    if (geom->type == LINETYPE)
      round_lwline(lwgeom_as_lwline(geom), maxdd, hasz, hasm);
    else /* geom->type == CIRCSTRINGTYPE */
      round_lwcircstring(lwgeom_as_lwcircstring(geom), maxdd, hasz, hasm);
  }
  return;
}

/**
 * @brief Set the precision of the coordinates of a geometry collection to a
 * number of decimal places
 * @note In PostGIS function lwgeom_free, the case for COMPOUNDTYPE is
 *   @p lwcollection_free((LWCOLLECTION *)lwgeom);
 */
static GSERIALIZED *
round_compoundcurve(const GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == COMPOUNDTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWCOMPOUND *comp = lwgeom_as_lwcompound(lwgeom_from_gserialized(gs));
  round_lwcompound(comp, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) comp);
  lwcollection_free((LWCOLLECTION *) comp);
  return result;
}

/**
 * @brief Set the precision of the coordinates of a compound curve to a
 * number of decimal places
 * @note A MultiCurve is a collection of curves which can include LineStrings, 
 * CircularStrings or CompoundCurves.
 * https://postgis.net/docs/using_postgis_dbmanagement.html#MultiCurve
 */
static void
round_lwmcurve(LWMCURVE *mcurve, int maxdd, bool hasz, bool hasm)
{
  int ngeoms = mcurve->ngeoms;
  for (int i = 0; i < ngeoms; i++)
  {
    LWGEOM *geom = mcurve->geoms[i];
    assert(geom->type == LINETYPE || geom->type == CIRCSTRINGTYPE || 
      geom->type == COMPOUNDTYPE);
    if (geom->type == LINETYPE)
      round_lwline(lwgeom_as_lwline(geom), maxdd, hasz, hasm);
    else if (geom->type == CIRCSTRINGTYPE)
      round_lwcircstring(lwgeom_as_lwcircstring(geom), maxdd, hasz, hasm);
    else /* geom->type == COMPOUNDTYPE */
      round_lwcompound(lwgeom_as_lwcompound(geom), maxdd, hasz, hasm);
  }
  return;
}

/**
 * @brief Set the precision of the coordinates of a geometry collection to a
 * number of decimal places
 * @note In PostGIS function lwgeom_free, the case for MULTICURVETYPE is
 *   @p lwcollection_free((LWCOLLECTION *)lwgeom);
 */
static GSERIALIZED *
round_multicurve(const GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == MULTICURVETYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWMCURVE *mcurv = (LWMCURVE *) lwgeom_from_gserialized(gs);
  round_lwmcurve(mcurv, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) mcurv);
  lwcollection_free((LWCOLLECTION *) mcurv);
  return result;
}

/**
 * @brief Set the precision of the coordinates of a curve polygon to a
 * number of decimal places
 * @note A CurvePolygon is like a polygon, with an outer ring and zero or more
 * inner rings. The difference is that a ring can be a CircularString or 
 * CompoundCurve as well as a LineString.
 * https://postgis.net/docs/using_postgis_dbmanagement.html#CurvePolygon
 */
static void
round_lwcurvepoly(LWCURVEPOLY *cpoly, int maxdd, bool hasz, bool hasm)
{
  int nrings = cpoly->nrings;
  for (int i = 0; i < nrings; i++)
  {
    LWGEOM *ring = cpoly->rings[i];
    assert(ring->type == LINETYPE || ring->type == CIRCSTRINGTYPE ||
      ring->type == COMPOUNDTYPE);
    if (ring->type == LINETYPE)
      round_lwline(lwgeom_as_lwline(ring), maxdd, hasz, hasm);
    else if (ring->type == CIRCSTRINGTYPE)
      round_lwcircstring(lwgeom_as_lwcircstring(ring), maxdd, hasz, hasm);
    else /* ring->type == COMPOUNDTYPE */
      round_lwcompound(lwgeom_as_lwcompound(ring), maxdd, hasz, hasm);
  }
  return;
}

/**
 * @brief Return a curve polygon with the precision of the coordinates set to
 * a number of decimal places
 * @note In PostGIS function lwgeom_free, the case for CURVEPOLYTYPE is
 *   @p lwcollection_free((LWCOLLECTION *)lwgeom);
 */
static GSERIALIZED *
round_curvepolygon(const GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == CURVEPOLYTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWCURVEPOLY *poly = lwgeom_as_lwcurvepoly(lwgeom_from_gserialized(gs));
  round_lwcurvepoly(poly, maxdd, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) poly);
  lwcollection_free((LWCOLLECTION *) poly);
  return result;
}

/**
 * @brief Set the precision of the coordinates of a geometry collection to a
 * number of decimal places
 */
static GSERIALIZED *
round_geometrycollection(const GSERIALIZED *gs, int maxdd)
{
  assert(gserialized_get_type(gs) == COLLECTIONTYPE);
  LWCOLLECTION *coll = lwgeom_as_lwcollection(lwgeom_from_gserialized(gs));
  int ngeoms = coll->ngeoms;
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  for (int i = 0; i < ngeoms; i++)
  {
    LWGEOM *geom = coll->geoms[i];
    switch (geom->type)
    {
      case POINTTYPE:
        round_point_n((lwgeom_as_lwpoint(geom))->point, 0, maxdd, hasz, hasm);
        break;
      case LINETYPE:
        round_lwline(lwgeom_as_lwline(geom), maxdd, hasz, hasm);
        break;
      case TRIANGLETYPE:
        round_lwtriangle(lwgeom_as_lwtriangle(geom), maxdd, hasz, hasm);
        break;
      case CIRCSTRINGTYPE:
        round_lwcircstring(lwgeom_as_lwcircstring(geom), maxdd, hasz, hasm);
        break;
      case COMPOUNDTYPE:
        round_lwcompound(lwgeom_as_lwcompound(geom), maxdd, hasz, hasm);
        break;
      case MULTICURVETYPE:
        round_lwmcurve((LWMCURVE *) geom, maxdd, hasz, hasm);
        break;
      case POLYGONTYPE:
        round_lwpoly(lwgeom_as_lwpoly(geom), maxdd, hasz, hasm);
        break;
      case CURVEPOLYTYPE:
        round_lwcurvepoly(lwgeom_as_lwcurvepoly(geom), maxdd, hasz, hasm);
        break;
      case MULTIPOINTTYPE:
        round_lwmpoint(lwgeom_as_lwmpoint(geom), maxdd, hasz, hasm);
        break;
      case MULTILINETYPE:
        round_lwmline(lwgeom_as_lwmline(geom), maxdd, hasz, hasm);
        break;
      case MULTIPOLYGONTYPE:
        round_lwmpoly(lwgeom_as_lwmpoly(geom), maxdd, hasz, hasm);\
        break;
      default:
      {
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "Unsupported geometry type in round function: %s",
          geo_typename(geom->type));
        lwcollection_free(coll);
        return NULL;
      }
    }    
  }
  GSERIALIZED *result = geo_serialize((LWGEOM *) coll);
  lwcollection_free(coll);
  return result;
}

/**
 * @ingroup meos_geo_base_transf
 * @brief Return a geometry with the precision of the coordinates set to a
 * number of decimal places
 * @param[in] gs Geometry/geography
 * @param[in] maxdd Maximum number of decimal digits
 * @note Currently not all geometry types are allowed
 */
GSERIALIZED *
geo_round(const GSERIALIZED *gs, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  if (gserialized_is_empty(gs))
    return geo_copy(gs);

  uint32_t type = gserialized_get_type(gs);
  switch (type)
  {
    case POINTTYPE:
      return point_round(gs, maxdd);
    case LINETYPE:
      return round_linestring(gs, maxdd);
    case POLYGONTYPE:
      return round_polygon(gs, maxdd);
    case MULTIPOINTTYPE:
      return round_multipoint(gs, maxdd);
    case MULTILINETYPE:
      return round_multilinestring(gs, maxdd);
    case MULTIPOLYGONTYPE:
      return round_multipolygon(gs, maxdd);
    case COLLECTIONTYPE:
      return round_geometrycollection(gs, maxdd);
    case CIRCSTRINGTYPE:
      return round_circularstring(gs, maxdd);
    case MULTICURVETYPE:
      return round_multicurve(gs, maxdd);
    case COMPOUNDTYPE:
      return round_compoundcurve(gs, maxdd);
    case CURVEPOLYTYPE:
      return round_curvepolygon(gs, maxdd);
    case TRIANGLETYPE:
      return round_triangle(gs, maxdd);
    default:
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Unsupported geometry type in round function: %s", geo_typename(type));
      return NULL;
  }
}

/**
 * @brief Return a geometry with the precision of the coordinates set to a
 * number of decimal places
 * @note Currently not all geometry types are allowed
 */
Datum
datum_geo_round(Datum value, Datum size)
{
  return GserializedPGetDatum(
    geo_round(DatumGetGserializedP(value), DatumGetInt32(size)));
}

/*****************************************************************************/
