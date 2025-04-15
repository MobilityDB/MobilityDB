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
 * @brief Functions for geometry/geography types corresponding to external 
 * PostGIS functions in order to bypass the function manager in @p fmgr.c
 */

#include "geo/postgis_funcs.h"

/* C */
#include <assert.h>
#include <float.h>
/* GEOS */
#include <geos_c.h>
/* PostgreSQL */
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* PostGIS */
#include <liblwgeom.h>
#include <lwgeom_log.h>
#include <lwgeom_geos.h>
/* MEOS */
#include <meos.h>
#include "geo/tgeo.h"
#include "geo/tgeo_spatialfuncs.h"

/* To avoid including lwgeom_functions_analytic.h */
extern int point_in_polygon(LWPOLY *polygon, LWPOINT *point);
extern int point_in_multipolygon(LWMPOLY *mpolygon, LWPOINT *point);

/* To avoid including lwgeom_transform.h */
void srid_check_latlong(int32_t srid);

/* Modified version of PG_PARSER_ERROR */
#if MEOS
#define PG_PARSER_ERROR(lwg_parser_result) \
  do { \
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, \
      "%s", lwg_parser_result.message); \
  } while(0);
#else
  #include <lwgeom_pg.h>
#endif

/* Function not exported in liblwgeom.h */
extern int spheroid_init_from_srid(int32_t srid, SPHEROID *s);

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @ingroup meos_geo_base_constructor
 * @brief Return a copy of a geometry
 * @note The @p gserialized_copy function is not available anymore in
 * PostGIS 3
 */
GSERIALIZED *
geo_copy(const GSERIALIZED *gs)
{
  VALIDATE_NOT_NULL(gs, NULL);
  GSERIALIZED *result = palloc(VARSIZE(gs));
  memcpy(result, gs, VARSIZE(gs));
  return result;
}

/*****************************************************************************
 * Functions borrowed from gserialized.c
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_geo_base_srid
 * @brief Get the SRID of a geometry/geography
 * @param[in] gs Geometry
 */
int32_t
geo_srid(const GSERIALIZED *gs)
{
  VALIDATE_NOT_NULL(gs, NULL);
  return gserialized_get_srid(gs);
}

/**
 * @ingroup meos_geo_base_srid
 * @brief Set the SRID of a geometry/geography
 * @param[in] gs Geometry
 * @param[in] srid SRID
 */
GSERIALIZED *
geo_set_srid(const GSERIALIZED *gs, int32_t srid)
{
  assert(gs);
  GSERIALIZED *result = geo_copy(gs);
  gserialized_set_srid(result, srid);
  return result;
}

/**
 * @ingroup meos_geo_base_accessor
 * @brief Get the SRID of a geometry/geography
 * @param[in] gs Geometry
 */
bool
geo_is_empty(const GSERIALIZED *gs)
{
  VALIDATE_NOT_NULL(gs, NULL);
  return gserialized_is_empty(gs);
}
#endif /* MEOS */

/*****************************************************************************
 * Functions adapted from lwgeom_box.c
 *****************************************************************************/

/**
 * @brief Create a geometry from a BOX2D
 * @note PostGIS function: @p BOX2D_to_LWGEOM(PG_FUNCTION_ARGS). With respect
 * to the original function, we also set the SRID which is passed as an
 * additional argument
 */
LWGEOM *
box2d_to_lwgeom(GBOX *box, int32_t srid)
{
  POINT4D pt;
  LWGEOM *result;

  /*
   * Alter BOX2D cast so that a valid geometry is always
   * returned depending upon the size of the BOX2D. The
   * code makes the following assumptions:
   *     - If the BOX2D is a single point then return a
   *     POINT geometry
   *     - If the BOX2D represents either a horizontal or
   *     vertical line, return a LINESTRING geometry
   *     - Otherwise return a POLYGON
   */

  if ( (box->xmin == box->xmax) && (box->ymin == box->ymax) )
  {
    /* Construct and serialize point */
    LWPOINT *point = lwpoint_make2d(srid, box->xmin, box->ymin);
    /* MobilityDB: The above function does not set the geodetic flag */
    FLAGS_SET_GEODETIC(point->flags, FLAGS_GET_GEODETIC(box->flags));
    result = lwpoint_as_lwgeom(point);
  }
  else if ( (box->xmin == box->xmax) || (box->ymin == box->ymax) )
  {
    LWLINE *line;

    /* Assign coordinates to point array */
    pt.x = box->xmin;
    pt.y = box->ymin;
    POINTARRAY *pa = ptarray_construct_empty(0, 0, 2);
    ptarray_append_point(pa, &pt, LW_TRUE);
    pt.x = box->xmax;
    pt.y = box->ymax;
    ptarray_append_point(pa, &pt, LW_TRUE);

    /* Construct and serialize linestring */
    line = lwline_construct(srid, NULL, pa);
    /* MobilityDB: The above function does not set the geodetic flag */
    FLAGS_SET_GEODETIC(line->flags, FLAGS_GET_GEODETIC(box->flags));
    result = lwline_as_lwgeom(line);
  }
  else
  {
    POINT4D points[4];
    LWPOLY *poly;

    /* Initialize the 4 vertices of the polygon */
    points[0] = (POINT4D) { box->xmin, box->ymin, 0.0, 0.0 };
    points[1] = (POINT4D) { box->xmin, box->ymax, 0.0, 0.0 };
    points[2] = (POINT4D) { box->xmax, box->ymax, 0.0, 0.0 };
    points[3] = (POINT4D) { box->xmax, box->ymin, 0.0, 0.0 };

    /* Construct polygon */
    poly = lwpoly_construct_rectangle(LW_FALSE, LW_FALSE, &points[0],
      &points[1], &points[2], &points[3]);
    lwgeom_set_srid(lwpoly_as_lwgeom(poly), srid);
    /* MobilityDB: The above function does not set the geodetic flag */
    FLAGS_SET_GEODETIC(poly->flags, FLAGS_GET_GEODETIC(box->flags));
    result = lwpoly_as_lwgeom(poly);
  }

  return result;
}

/*****************************************************************************
 * Functions adapted from lwgeom_box3d.c
 *****************************************************************************/

/**
 * @brief Create a geometry from a @p BOX3D
 * @note PostGIS function: @p BOX3D_to_LWGEOM(PG_FUNCTION_ARGS)
 */
LWGEOM *
box3d_to_lwgeom(BOX3D *box)
{
  POINTARRAY *pa;
  LWGEOM *result;
  POINT4D pt;

  /**
   * Alter BOX3D cast so that a valid geometry is always
   * returned depending upon the size of the BOX3D. The
   * code makes the following assumptions:
   *     - If the BOX3D is a single point then return a POINT geometry
   *     - If the BOX3D represents a line in any of X, Y or Z dimension,
   *       return a LINESTRING geometry
   *     - If the BOX3D represents a plane in the X, Y, or Z dimension,
   *       return a POLYGON geometry
   *     - Otherwise return a POLYHEDRALSURFACE geometry
   */

  pa = ptarray_construct_empty(LW_TRUE, LW_FALSE, 5);

  /* BOX3D is a point */
  if ((box->xmin == box->xmax) && (box->ymin == box->ymax) && (box->zmin == box->zmax))
  {
    LWPOINT *lwpt = lwpoint_construct(SRID_UNKNOWN, NULL, pa);

    pt.x = box->xmin;
    pt.y = box->ymin;
    pt.z = box->zmin;
    ptarray_append_point(pa, &pt, LW_TRUE);
    result = lwpoint_as_lwgeom(lwpt);
  }
  /* BOX3D is a line */
  else if (((box->xmin == box->xmax || box->ymin == box->ymax) && box->zmin == box->zmax) ||
     ((box->xmin == box->xmax || box->zmin == box->zmax) && box->ymin == box->ymax) ||
     ((box->ymin == box->ymax || box->zmin == box->zmax) && box->xmin == box->xmax))
  {
    LWLINE *lwline = lwline_construct(SRID_UNKNOWN, NULL, pa);

    pt.x = box->xmin;
    pt.y = box->ymin;
    pt.z = box->zmin;
    ptarray_append_point(pa, &pt, LW_TRUE);
    pt.x = box->xmax;
    pt.y = box->ymax;
    pt.z = box->zmax;
    ptarray_append_point(pa, &pt, LW_TRUE);
    result = lwline_as_lwgeom(lwline);
  }
  /* BOX3D is a polygon in the X plane */
  else if (box->xmin == box->xmax)
  {
    POINT4D points[4];
    LWPOLY *lwpoly;

    /* Initialize the 4 vertices of the polygon */
    points[0] = (POINT4D){box->xmin, box->ymin, box->zmin, 0.0};
    points[1] = (POINT4D){box->xmin, box->ymax, box->zmin, 0.0};
    points[2] = (POINT4D){box->xmin, box->ymax, box->zmax, 0.0};
    points[3] = (POINT4D){box->xmin, box->ymin, box->zmax, 0.0};
    lwpoly = lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[0], &points[1], &points[2], &points[3]);
    result = lwpoly_as_lwgeom(lwpoly);
  }
  /* BOX3D is a polygon in the Y plane */
  else if (box->ymin == box->ymax)
  {
    POINT4D points[4];
    LWPOLY *lwpoly;

    /* Initialize the 4 vertices of the polygon */
    points[0] = (POINT4D){box->xmin, box->ymin, box->zmin, 0.0};
    points[1] = (POINT4D){box->xmax, box->ymin, box->zmin, 0.0};
    points[2] = (POINT4D){box->xmax, box->ymin, box->zmax, 0.0};
    points[3] = (POINT4D){box->xmin, box->ymin, box->zmax, 0.0};
    lwpoly = lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[0], &points[1], &points[2], &points[3]);
    result = lwpoly_as_lwgeom(lwpoly);
  }
  /* BOX3D is a polygon in the Z plane */
  else if (box->zmin == box->zmax)
  {
    POINT4D points[4];
    LWPOLY *lwpoly;

    /* Initialize the 4 vertices of the polygon */
    points[0] = (POINT4D){box->xmin, box->ymin, box->zmin, 0.0};
    points[1] = (POINT4D){box->xmin, box->ymax, box->zmin, 0.0};
    points[2] = (POINT4D){box->xmax, box->ymax, box->zmin, 0.0};
    points[3] = (POINT4D){box->xmax, box->ymin, box->zmin, 0.0};
    lwpoly = lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[0], &points[1], &points[2], &points[3]);
    result = lwpoly_as_lwgeom(lwpoly);
  }
  /* BOX3D is a polyhedron */
  else
  {
    POINT4D points[8];
    static const int ngeoms = 6;
    LWGEOM **geoms = (LWGEOM **)lwalloc(sizeof(LWGEOM *) * ngeoms);

    /* Initialize the 8 vertices of the box */
    points[0] = (POINT4D){box->xmin, box->ymin, box->zmin, 0.0};
    points[1] = (POINT4D){box->xmin, box->ymax, box->zmin, 0.0};
    points[2] = (POINT4D){box->xmax, box->ymax, box->zmin, 0.0};
    points[3] = (POINT4D){box->xmax, box->ymin, box->zmin, 0.0};
    points[4] = (POINT4D){box->xmin, box->ymin, box->zmax, 0.0};
    points[5] = (POINT4D){box->xmin, box->ymax, box->zmax, 0.0};
    points[6] = (POINT4D){box->xmax, box->ymax, box->zmax, 0.0};
    points[7] = (POINT4D){box->xmax, box->ymin, box->zmax, 0.0};

    /* add bottom polygon */
    geoms[0] = lwpoly_as_lwgeom(
        lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[0], &points[1], &points[2], &points[3]));
    /* add top polygon */
    geoms[1] = lwpoly_as_lwgeom(
        lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[4], &points[7], &points[6], &points[5]));
    /* add left polygon */
    geoms[2] = lwpoly_as_lwgeom(
        lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[0], &points[4], &points[5], &points[1]));
    /* add right polygon */
    geoms[3] = lwpoly_as_lwgeom(
        lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[3], &points[2], &points[6], &points[7]));
    /* add front polygon */
    geoms[4] = lwpoly_as_lwgeom(
        lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[0], &points[3], &points[7], &points[4]));
    /* add back polygon */
    geoms[5] = lwpoly_as_lwgeom(
        lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[1], &points[5], &points[6], &points[2]));

    result = (LWGEOM *) lwcollection_construct(POLYHEDRALSURFACETYPE, SRID_UNKNOWN, NULL, ngeoms, geoms);
    FLAGS_SET_SOLID(result->flags, 1);
  }

  lwgeom_set_srid(result, box->srid);
  return result;
}

/*****************************************************************************
 * Functions adapted from lwgeom_functions_basic.c
 *****************************************************************************/

#if NPOINT
/**
 * @ingroup meos_geo_base_accessor
 * @brief Return the length of a geometry
 * @details Defined by
 *   - length(point) = 0
 *   - length(line) = length of line
 *   - length(polygon) = 0  -- could make sense to return sum(ring perimeter)
 *
 *  Uses Euclidean 3D/2D length depending on input dimensions.
 * @param[in] gs Geometry
 * @note PostGIS function: @p LWGEOM_length_linestring(PG_FUNCTION_ARGS)
 */
double
geom_length(const GSERIALIZED *gs)
{
  assert(gs);
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  double dist = lwgeom_length(lwgeom);
  lwgeom_free(lwgeom);
  return dist;
}
#endif /* NPOINT */

/**
 * @ingroup meos_geo_base_accessor
 * @brief Return the perimeter of a geometry
 * @details Defined by
 *   - perimeter(point) = 0
 *   - perimeter(line) = 0
 *   - perimeter(polygon) = sum of ring perimeters
 *
 * Uses Euclidian 2D computation even if input is 3D
 * @param[in] gs Geometry
 * @note PostGIS function: @p LWGEOM_perimeter2d_poly(PG_FUNCTION_ARGS)
 */
double
geom_perimeter(const GSERIALIZED *gs)
{
  assert(gs);
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  double perimeter = lwgeom_perimeter_2d(lwgeom);
  return perimeter;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the boundary of a geometry
 * @param[in] gs Geometry
 * @note PostGIS function: @p boundary(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geom_boundary(const GSERIALIZED *gs)
{
  assert(gs);
  /* Empty.Boundary() == Empty, but of other dimension, so can't shortcut */
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  LWGEOM *lwresult = lwgeom_boundary(geom);
  if (! lwresult)
  {
    lwgeom_free(geom);
    return NULL;
  }

  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(geom); lwgeom_free(lwresult);
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the shortest 2D line between two geometries
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p LWGEOM_shortestline2d(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geom_shortestline2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  assert(gs1); assert(gs2);
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));
  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  LWGEOM *line = lwgeom_closest_line(geom1, geom2);
  if (lwgeom_is_empty(line))
    return NULL;

  GSERIALIZED *result = geo_serialize(line);
  lwgeom_free(line); lwgeom_free(geom1); lwgeom_free(geom2);
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the shortest line between two 3D geometries
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p LWGEOM_shortestline3d(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geom_shortestline3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  assert(gs1); assert(gs2);
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  LWGEOM *line = lwgeom_closest_line_3d(geom1, geom2);
  if (lwgeom_is_empty(line))
    return NULL;

  GSERIALIZED *result = geo_serialize(line);
  lwgeom_free(line); lwgeom_free(geom1); lwgeom_free(geom2);
  return result;
}

/**
 * @ingroup meos_geo_base_distance
 * @brief Return the distance between two geometries
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p ST_Distance(PG_FUNCTION_ARGS)
 */
double
geom_distance2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  assert(gs1); assert(gs2);
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  double mindist = lwgeom_mindistance2d(geom1, geom2);
  lwgeom_free(geom1);
  lwgeom_free(geom2);
  /* if called with empty geometries the ingoing mindistance is untouched,
   * and makes us return NULL */
  if (mindist < FLT_MAX)
    return mindist;
  return -1;
}

/**
 * @ingroup meos_geo_base_distance
 * @brief Return the 3D distance between two geometries
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p ST_3DDistance(PG_FUNCTION_ARGS)
 */
double
geom_distance3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  assert(gs1); assert(gs2);
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  double mindist = lwgeom_mindistance3d(geom1, geom2);
  lwgeom_free(geom1);
  lwgeom_free(geom2);
  /* if called with empty geometries the ingoing mindistance is untouched,
   * and makes us return NULL */
  if (mindist < FLT_MAX)
    return mindist;
  return -1;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return true if the 3D geometries intersect
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p ST_3DIntersects(PG_FUNCTION_ARGS)
 */
bool
geom_intersects3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  assert(gs1); assert(gs2);
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  double mindist = lwgeom_mindistance3d_tolerance(geom1, geom2, 0.0);
  /* empty geometries cases should be right handled since return from
     underlying functions should be FLT_MAX which causes false as answer */
  return (mindist == 0.0);
}

/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if two geometries are within a distance
 * @param[in] gs1,gs2 Geometries
 * @param[in] tolerance Tolerance
 * @note PostGIS function: @p LWGEOM_dwithin(PG_FUNCTION_ARGS)
 */
bool
geom_dwithin2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  double tolerance)
{
  assert(gs1); assert(gs2);
  if (! ensure_not_negative_datum(Float8GetDatum(tolerance), T_FLOAT8) ||
      ! ensure_same_srid(gserialized_get_srid(gs1), gserialized_get_srid(gs2)) ||
      gserialized_is_empty(gs1) || gserialized_is_empty(gs2))
    return false;

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  double mindist = lwgeom_mindistance2d_tolerance(geom1, geom2, tolerance);
  /*empty geometries cases should be right handled since return from underlying
   functions should be FLT_MAX which causes false as answer*/
  return (tolerance >= mindist);
}

/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if two geometries are within a distance
 * @param[in] gs1,gs2 Geometries
 * @param[in] tolerance Tolerance
 * @note PostGIS function: @p LWGEOM_dwithin3d(PG_FUNCTION_ARGS)
 */
bool
geom_dwithin3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  double tolerance)
{
  assert(gs1); assert(gs2);
  if (! ensure_positive_datum(Float8GetDatum(tolerance), T_FLOAT8) ||
      ! ensure_same_srid(gserialized_get_srid(gs1), gserialized_get_srid(gs2)))
    return false;

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  double mindist = lwgeom_mindistance3d_tolerance(geom1, geom2, tolerance);
  /*empty geometries cases should be right handled since return from underlying
   functions should be FLT_MAX which causes false as answer*/
  return (tolerance >= mindist);
}

/**
 * @ingroup meos_geo_base_transf
 * @brief Reverse vertex order of geometry/geography
 * @param[in] gs Geometry/geography
 * @note PostGIS function: @p LWGEOM_reverse(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geo_reverse(const GSERIALIZED *gs)
{
  assert(gs);
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  lwgeom_reverse_in_place(geom);
  return geo_serialize(geom);
}

/**
 * @ingroup meos_geo_base_accessor
 * @brief Return in the last argument the azimuth of a segment defined by two
 * points
 * @param[in] gs1,gs2 Geometries
 * @param[out] result Result
 * @return Return false on exception (same point)
 */
bool
geom_azimuth(const GSERIALIZED *gs1, const GSERIALIZED *gs2, double *result)
{
  assert(gs1); assert(gs2);
  assert(gserialized_get_type(gs1) == POINTTYPE);
  assert(gserialized_get_type(gs2) == POINTTYPE);
  assert(! gserialized_is_empty(gs1)); assert(! gserialized_is_empty(gs2));

  POINT2D p1, p2;

  /* Extract first point */
  LWPOINT *point = (LWPOINT *) lwgeom_from_gserialized(gs1);
  if (! point)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR, "Error extracting point");
    return false;
  }
  int32_t srid = point->srid;
  if (!getPoint2d_p(point->point, 0, &p1))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR, "Error extracting point");
    return false;
  }
  lwpoint_free(point);

  /* Extract second point */
  point = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs2));
  if (! point)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR, "Error extracting point");
    return false;
  }
  if (point->srid != srid)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on mixed SRID geometries");
    return false;
  }
  if (! getPoint2d_p(point->point, 0, &p2))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR, "Error extracting point");
    return false;
  }
  lwpoint_free(point);

  /* Standard return value for equality case */
  if ((p1.x == p2.x) && (p1.y == p2.y))
  {
    return false;
  }

  /* Compute azimuth */
  if (! azimuth_pt_pt(&p1, &p2, result))
    return false;

  return true;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Collect the array of geometries/geographies into a geo collection
 * @param[in] gsarr Array of geometries/geographies
 * @param[in] nelems Number of elements in the array
 * @note PostGIS function: @p LWGEOM_collect_garray(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geo_collect_garray(GSERIALIZED **gsarr, int nelems)
{
  assert(gsarr); assert(nelems > 0);

  /* Singleton array */
  if (nelems == 1)
    return geo_copy(gsarr[0]);
    
  uint32 outtype = 0;
  int count = 0;
  int32_t srid = SRID_UNKNOWN;
  GBOX *box = NULL;
  LWGEOM **lwgeoms = palloc(sizeof(LWGEOM *) * nelems);
  for (int i = 0; i < nelems; i++)
  {
    GSERIALIZED *geom = gsarr[i];
    uint8_t intype = gserialized_get_type(geom);
    lwgeoms[count] = lwgeom_from_gserialized(geom);
    if (! count)
    {
      /* Get first geometry SRID */
      srid = lwgeoms[count]->srid;

      /* COMPUTE_BBOX WHEN_SIMPLE */
      if (lwgeoms[count]->bbox)
        box = gbox_copy(lwgeoms[count]->bbox);
    }
    else
    {
      /* Check SRID homogeneity */
      if (! ensure_same_srid(srid, gserialized_get_srid(geom)))
        return NULL;

      /* COMPUTE_BBOX WHEN_SIMPLE */
      if (box)
      {
        if (lwgeoms[count]->bbox)
          gbox_merge(lwgeoms[count]->bbox, box);
        else
        {
          pfree(box);
          box = NULL;
        }
      }
    }
    lwgeom_drop_srid(lwgeoms[count]);
    lwgeom_drop_bbox(lwgeoms[count]);

    /* Output type not initialized */
    if (! outtype)
      outtype = lwtype_get_collectiontype(intype);
    /* Input type not compatible with output */
    /* make output type a collection */
    else if (outtype != COLLECTIONTYPE && 
        lwtype_get_collectiontype(intype) != outtype)
      outtype = COLLECTIONTYPE;

    count++;
  }

  assert(outtype);
  LWGEOM *outlwg = (LWGEOM *) lwcollection_construct(outtype, srid, box, count,
    lwgeoms);
  GSERIALIZED *result = geom_serialize(outlwg);
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return a line from an array of geometries/geographies
 * @details Array elements that are not points are discarded.
 * @param[in] gsarr Array of geometries/geographies
 * @param[in] count Number of elements in the array
 * @note PostGIS function: @p LWGEOM_makeline_garray(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geo_makeline_garray(GSERIALIZED **gsarr, int count)
{
  assert(gsarr); assert(count > 0);

  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * count);
  int ngeoms = 0;
  int32_t srid = SRID_UNKNOWN;
  for (int i = 0; i < count; i++)
  {
    if (gserialized_get_type(gsarr[i]) != POINTTYPE && 
        gserialized_get_type(gsarr[i]) != LINETYPE &&
        gserialized_get_type(gsarr[i]) != MULTIPOINTTYPE)
      continue;
    geoms[ngeoms++] = lwgeom_from_gserialized(gsarr[i]);
    /* Check SRID homogeneity */
    if (ngeoms == 1)
    {
      /* Get first geometry SRID */
      srid = geoms[ngeoms - 1]->srid;
      /* TODO: also get ZMflags */
    }
    else
    {
      if (! ensure_same_srid(srid, geoms[ngeoms - 1]->srid))
      {
        for (int j = 0; j < ngeoms; j++)
          lwgeom_free(geoms[i]);
        pfree(geoms);
        return NULL;
      }
    }
  }

  /* Return null on 0-points input array */
  if (ngeoms == 0)
  {
    /* TODO: should we return LINESTRING EMPTY here ? */
    meos_error(NOTICE, MEOS_ERR_INVALID_ARG_VALUE,
      "No points or linestrings in input array");
    pfree(geoms);
    return NULL;
  }
  LWGEOM *outlwg = (LWGEOM *) lwline_from_lwgeom_array(srid, ngeoms, geoms);
  return geo_serialize(outlwg);
}

/*****************************************************************************
 * Functions adapted from lwgeom_geos.c
 *****************************************************************************/

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the centroid of a geometry
 * @note PostGIS function: @p centroid(PG_FUNCTION_ARGS). 
 */
GSERIALIZED *
geom_centroid(const GSERIALIZED *gs)
{
  assert(gs);
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  LWGEOM *lwresult = lwgeom_centroid(lwgeom);
  lwgeom_free(lwgeom);
  if (! lwresult)
    return NULL;
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult);
  return result;
}

/**
 * @brief Return true if a geometry is a point
 */
static char
gserialized_is_point(const GSERIALIZED* gs)
{
  int type = gserialized_get_type(gs);
  return (type == POINTTYPE || type == MULTIPOINTTYPE);
}

/**
 * @brief Return true if a geometry is a polygon
 */
static char
gserialized_is_poly(const GSERIALIZED* gs)
{
  int type = gserialized_get_type(gs);
  return (type == POLYGONTYPE || type == MULTIPOLYGONTYPE);
}

/**
 * @brief Return -1, 0, or 1 depending on whether a (multi)point is completely
 * outside, on the boundary, or completely inside a (multi)polygon
 * @note This function is based PostGIS function @p pip_short_circuit bypassing
 * the cache
 */
static int
meos_point_in_polygon(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  bool inter)
{
  const GSERIALIZED *gpoly = gserialized_is_poly(gs1) ? gs1 : gs2;
  const GSERIALIZED *gpoint = gserialized_is_point(gs1) ? gs1 : gs2;

  LWGEOM *poly = lwgeom_from_gserialized(gpoly);
  int32 polytype = lwgeom_get_type(poly);
  int retval = -1; /* Initialize to completely outside */
  if (gserialized_get_type(gpoint) == POINTTYPE)
  {
    LWGEOM *point = lwgeom_from_gserialized(gpoint);
    if ( polytype == POLYGONTYPE )
      retval = point_in_polygon(lwgeom_as_lwpoly(poly),
        lwgeom_as_lwpoint(point));
    else /* polytype == MULTIPOLYGONTYPE */
      retval = point_in_multipolygon(lwgeom_as_lwmpoly(poly),
        lwgeom_as_lwpoint(point));
    lwgeom_free(point);
    lwgeom_free(poly);
    return retval;
  }
  else /* gserialized_get_type(gpoint) == MULTIPOINTTYPE */
  {
    LWMPOINT* mpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(gpoint));
    for (uint32_t i = 0; i < mpoint->ngeoms; i++)
    {
      /* We need to find at least one point that's completely inside the
       * polygons (pip_result == 1).  As long as we have one point that's
       * completely inside, we can have as many as we want on the boundary
       * itself. (pip_result == 0)
       */
       int pip_result;
       if ( polytype == POLYGONTYPE )
        pip_result = point_in_polygon(lwgeom_as_lwpoly(poly), mpoint->geoms[i]);
      else /* polytype == MULTIPOLYGONTYPE */
        pip_result = point_in_multipolygon(lwgeom_as_lwmpoly(poly), mpoint->geoms[i]);
      /* Since we use the same function for intersects and contains we cannot
       * break on pip_result != -1 for intersects or pip_presult == 1 for
       * contains */
      retval = Max(retval, pip_result);
      if ((inter && retval != -1)|| (!inter && retval == 1))
        break;
    }
    lwmpoint_free(mpoint);
    lwgeom_free(poly);
    return retval;
  }
}

/**
 * @brief Tranform a PostGIS geometry to a GEOS one
 */
GEOSGeometry *
POSTGIS2GEOS(const GSERIALIZED *gs)
{
  GEOSGeometry *result;
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  if (! lwgeom)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "POSTGIS2GEOS: unable to deserialize input");
    return NULL;
  }
  result = LWGEOM2GEOS(lwgeom, 0);
  lwgeom_free(lwgeom);
  return result;
}

/**
 * @brief Tranform a GEOS geometry to a PostGIS one
 */
GSERIALIZED *
GEOS2POSTGIS(GEOSGeom geom, char want3d)
{
  LWGEOM *lwgeom = GEOS2LWGEOM(geom, want3d);
  if (! lwgeom)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "GEOS2LWGEOM returned NULL");
    return NULL;
  }

  if (lwgeom_needs_bbox(lwgeom))
    lwgeom_add_bbox(lwgeom);
  GSERIALIZED *result = geo_serialize(lwgeom);
  lwgeom_free(lwgeom);
  return result;
}

/**
 * @brief Transform two @p GSERIALIZED geometries into @p GEOSGeometry and
 * call the GEOS function passed as argument
 */
static char
meos_call_geos2(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  char (*func)(const GEOSGeometry *geos1, const GEOSGeometry *geos2))
{
  initGEOS(lwnotice, lwgeom_geos_error);

  GEOSGeometry *geos1 = POSTGIS2GEOS(gs1);
  if (! geos1)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "First argument geometry could not be converted to GEOS");
    return 2;
  }
  GEOSGeometry *geos2 = POSTGIS2GEOS(gs2);
  if (! geos2)
  {
    GEOSGeom_destroy(geos1);
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Second argument geometry could not be converted to GEOS");
    return 2;
  }

  char result = func(geos1, geos2);

  GEOSGeom_destroy(geos1); GEOSGeom_destroy(geos2);
  if (result == 2)
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "GEOS returned error");
  return result;
}

/**
 * @brief Return true if two geometries satisfy a given spatial relationship,
 * where the function called depend on the third argument
 * @param[in] gs1,gs2 Geometries
 * @param[in] rel Spatial relationship
 * @note PostGIS functions: @p ST_Intersects(PG_FUNCTION_ARGS),
 * @p contains(PG_FUNCTION_ARGS), @p touches(PG_FUNCTION_ARGS)
 */
bool
geom_spatialrel(const GSERIALIZED *gs1, const GSERIALIZED *gs2, spatialRel rel)
{
  if (! ensure_valid_geo_geo(gs1, gs2))
    return NULL;

  /* A.Intersects(Empty) == FALSE */
  if ( gserialized_is_empty(gs1) || gserialized_is_empty(gs2) )
    return false;

  /*
   * short-circuit 1: if gs2 bounding box does not overlap
   * gs1 bounding box we can return FALSE.
   */
  GBOX box1, box2;
  if (gserialized_get_gbox_p(gs1, &box1) &&
      gserialized_get_gbox_p(gs2, &box2))
  {
    if (gbox_overlaps_2d(&box1, &box2) == LW_FALSE)
      return false;
  }

  /*
   * short-circuit 2: if the geoms are a point and a polygon,
   * call the point_outside_polygon function.
   */
  if ((rel == INTERSECTS || rel == CONTAINS) && (
      (gserialized_is_point(gs1) && gserialized_is_poly(gs2)) ||
      (gserialized_is_poly(gs1) && gserialized_is_point(gs2))))
  {
    int pip_result = meos_point_in_polygon(gs1, gs2, rel == INTERSECTS);
    return (rel == INTERSECTS) ?
      (pip_result != -1) : /* not outside */
      (pip_result == 1); /* inside */
  }

  /* Call GEOS function */
  assert(rel == INTERSECTS || rel == CONTAINS || rel == TOUCHES ||
    rel == COVERS);
  switch (rel)
  {
    case INTERSECTS:
      return (bool) meos_call_geos2(gs1, gs2, &GEOSIntersects);
    case CONTAINS:
      return (bool) meos_call_geos2(gs1, gs2, &GEOSContains);
    case TOUCHES:
      return (bool) meos_call_geos2(gs1, gs2, &GEOSTouches);
    case COVERS:
      return (bool) meos_call_geos2(gs1, gs2, &GEOSCovers);
    default:
      /* keep compiler quiet */
      return false;
  }
}

/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if two geometries intersects
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS functions: @p ST_Intersects(PG_FUNCTION_ARGS)
 */
inline bool
geom_intersects2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  return geom_spatialrel(gs1, gs2, INTERSECTS);
}

/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if the first geometry contains the second one
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS functions: @p contains(PG_FUNCTION_ARGS)
 */
inline bool
geom_contains(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  return geom_spatialrel(gs1, gs2, CONTAINS);
}

#if MEOS
/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if the two geometries intersect on a border
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p ST_Covers(PG_FUNCTION_ARGS)
 */
inline bool
geom_touches(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  return geom_spatialrel(gs1, gs2, TOUCHES);
}

/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if the first geometry covers the second one
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p ST_Covers(PG_FUNCTION_ARGS)
 */
inline bool
geom_covers(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  return geom_spatialrel(gs1, gs2, COVERS);
}

/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if two geometries are disjoint in 2D
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p ST_Disjoint(PG_FUNCTION_ARGS)
 */
inline bool
geom_disjoint2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  return ! geom_spatialrel(gs1, gs2, INTERSECTS);
}
#endif /* MEOS */

/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if two geometries satisfy a spatial relationship given
 * by a pattern
 * @param[in] gs1,gs2 Geometries
 * @param[in] patt Pattern
 * @note PostGIS function: @p relate_pattern(PG_FUNCTION_ARGS)
 */
bool
geom_relate_pattern(const GSERIALIZED *gs1, const GSERIALIZED *gs2, char *patt)
{
  assert(gs1); assert(gs2); assert(patt);
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));

  /* TODO handle empty */

  initGEOS(lwnotice, lwgeom_geos_error);

  GEOSGeometry *geos1 = POSTGIS2GEOS(gs1);
  if (!geos1)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "First argument geometry could not be converted to GEOS");
    return false;
  }
  GEOSGeometry *geos2 = POSTGIS2GEOS(gs2);
  if (!geos2)
  {
    GEOSGeom_destroy(geos1);
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Second argument geometry could not be converted to GEOS");
    return false;
  }

  /*
  ** Need to make sure 't' and 'f' are upper-case before handing to GEOS
  */
  for (size_t i = 0; i < strlen(patt); i++ )
  {
    if ( patt[i] == 't' ) patt[i] = 'T';
    if ( patt[i] == 'f' ) patt[i] = 'F';
  }

  char result = GEOSRelatePattern(geos1, geos2, patt);
  GEOSGeom_destroy(geos1);
  GEOSGeom_destroy(geos2);

  if (result == 2)
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "GEOSRelatePattern returned error");

  return (bool) result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the intersection of two geometries
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p ST_Intersection(PG_FUNCTION_ARGS). With respect
 * to the original function we do not use the @p prec argument.
 */
GSERIALIZED *
geom_intersection2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  assert(gs1); assert(gs2);
  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  LWGEOM *lwresult = lwgeom_intersection_prec(geom1, geom2, -1);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(geom1); lwgeom_free(geom2); lwgeom_free(lwresult);
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the difference of two geometries
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p ST_Difference(PG_FUNCTION_ARGS). With respect
 * to the original function we do not use the @p prec argument.
 */
GSERIALIZED *
geom_difference2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  assert(gs1); assert(gs2);
  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  LWGEOM *lwresult = lwgeom_difference_prec(geom1, geom2, -1);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(geom1); lwgeom_free(geom2); lwgeom_free(lwresult);
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the union of an array of geometries
 * @details The function will iteratively call @p GEOSUnion on the
 * GEOS-converted versions of them and return PGIS-converted version back.
 * Changing the combination order *might* speed up performance.
 * @param[in] gsarr Array of geometries
 * @param[in] count Number of elements in the array
 * @note PostGIS function: @p pgis_union_geometry_array(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geom_array_union(GSERIALIZED **gsarr, int count)
{
  assert(gsarr); assert(count > 0);

  /* One geom geom? Return it */
  if (count == 1)
    return gsarr[0];

  bool is3d = false, gotsrid = false;
  int curgeom = 0;
  uint8_t empty_type = 0;
  int32_t srid = SRID_UNKNOWN;
  GSERIALIZED *result = NULL;
  GEOSGeometry *g = NULL;
  GEOSGeometry *g_union = NULL;

  initGEOS(lwnotice, lwgeom_geos_error);

  /* Collect the non-empty inputs and stuff them into a GEOS collection */
  GEOSGeometry **geoms = palloc(sizeof(GEOSGeometry *) * count);

  /*
  ** We need to convert the array of GSERIALIZED into a GEOS collection.
  ** First make an array of GEOS geometries.
  */
  for (int i = 0; i < count; i++)
  {
    /* Check for SRID mismatch in array elements */
    if (gotsrid)
      assert(gserialized_get_srid(gsarr[i]) == srid);
    else
    {
      /* Initialize SRID/dimensions info */
      srid = gserialized_get_srid(gsarr[i]);
      is3d = (bool) gserialized_has_z(gsarr[i]);
      gotsrid = true;
    }

    /* Don't include empties in the union */
    if (gserialized_is_empty(gsarr[i]))
    {
      uint8_t gser_type = (uint8_t) gserialized_get_type(gsarr[i]);
      if (gser_type > empty_type)
        empty_type = gser_type;
    }
    else
    {
      g = POSTGIS2GEOS(gsarr[i]);

      /* Uh oh! Exception thrown at construction... */
      if (! g)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
          "One of the geometries in the set could not be converted to GEOS");
        return NULL;
      }

      geoms[curgeom++] = g;
    }
  }

  /*
  ** Take our GEOS geometries and turn them into a GEOS collection,
  ** then pass that into cascaded union.
  */
  if (curgeom > 0)
  {
    g = GEOSGeom_createCollection(GEOS_GEOMETRYCOLLECTION, geoms, curgeom);
    if (! g)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Could not create GEOS COLLECTION from geometry array");
      return NULL;
    }

    g_union = GEOSUnaryUnion(g);
    GEOSGeom_destroy(g);
    if (! g_union)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "GEOSUnaryUnion");
      return NULL;
    }

    GEOSSetSRID(g_union, srid);
    result = GEOS2POSTGIS(g_union, is3d);
    GEOSGeom_destroy(g_union);
  }
  /* No real geometries in our array, any empties? */
  else
  {
    /* If it was only empties, we'll return the largest type number */
    if (empty_type > 0)
      return geo_serialize(lwgeom_construct_empty(empty_type, srid, is3d,
        0));
    /* Nothing but NULL, returns NULL */
    else
      return NULL;
  }

  if (! result)
    /* Union returned a NULL geometry */
    return NULL;
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the unary union of a geometry
 * @param[in] gs Geometry
 * @param[in] prec Precision
 * @note PostGIS function: @p ST_UnaryUnion(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geom_unary_union(GSERIALIZED *gs, double prec)
{
  assert(gs);
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs) ;
  LWGEOM *lwresult = lwgeom_unaryunion_prec(lwgeom, prec);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwgeom); lwgeom_free(lwresult);
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the convex hull of the geometry
 * @param[in] gs Geometry
 * @note PostGIS function: @p ST_ConvexHull(PG_FUNCTION_ARGS). With respect to
 * the original function we do not use the @p prec argument.
 */
GSERIALIZED *
geom_convex_hull(const GSERIALIZED *gs)
{
  assert(gs);
  /* Empty.ConvexHull() == Empty */
  if ( gserialized_is_empty(gs) )
    return geo_copy(gs);

  int32_t srid = gserialized_get_srid(gs);

  initGEOS(lwnotice, lwgeom_geos_error);

  GEOSGeometry *geos1 = POSTGIS2GEOS(gs);
  if (!geos1)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "First argument geometry could not be converted to GEOS");
    return NULL;
  }

  GEOSGeometry *geos2 = GEOSConvexHull(geos1);
  GEOSGeom_destroy(geos1);

  if (! geos2)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "GEOS convexhull() threw an error !");
    return NULL;
  }

  GEOSSetSRID(geos2, srid);

  LWGEOM *lwout = GEOS2LWGEOM(geos2, (uint8_t) gserialized_has_z(gs));
  GEOSGeom_destroy(geos2);

  if (!lwout)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "convexhull() failed to convert GEOS geometry to LWGEOM");
    return NULL;
  }

  /* Copy input bbox if any */
  GBOX bbox;
  if ( gserialized_get_gbox_p(gs, &bbox) )
  {
    /* Force the box to have the same dimensionality as the lwgeom */
    bbox.flags = lwout->flags;
    lwout->bbox = gbox_copy(&bbox);
  }

  GSERIALIZED *result = geo_serialize(lwout);
  lwgeom_free(lwout);
  if (!result)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "GEOS convexhull() threw an error !");
    return NULL;
  }
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return a @p POLYGON or a @p MULTIPOLYGON that represents all points
 * whose distance from a geometry/geography is less than or equal to a given
 * distance
 * @param[in] gs Geometry
 * @param[in] size Distance
 * @param[in] params Buffer style parameters
 * @note PostGIS function: @p ST_Buffer(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geom_buffer(const GSERIALIZED *gs, double size, char *params)
{
  assert(gs); assert(params);

  GEOSBufferParams *bufferparams;
  GEOSGeometry *g1, *g3 = NULL;
  LWGEOM *lwg;
  int quadsegs = 8; /* the default */
  int singleside = 0; /* the default */
  enum
  {
    ENDCAP_ROUND = 1,
    ENDCAP_FLAT = 2,
    ENDCAP_SQUARE = 3
  };
  enum
  {
    JOIN_ROUND = 1,
    JOIN_MITRE = 2,
    JOIN_BEVEL = 3
  };
  const double DEFAULT_MITRE_LIMIT = 5.0;
  const int DEFAULT_ENDCAP_STYLE = ENDCAP_ROUND;
  const int DEFAULT_JOIN_STYLE = JOIN_ROUND;
  double mitreLimit = DEFAULT_MITRE_LIMIT;
  int endCapStyle = DEFAULT_ENDCAP_STYLE;
  int joinStyle  = DEFAULT_JOIN_STYLE;

  char *param;
  for (param = params; ; param = NULL)
  {
    char *key, *val;
    param = strtok(param, " ");
    if (! param)
      break;

    key = param;
    val = strchr(key, '=');
    if (! val || *(val + 1) == '\0')
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Missing value for buffer parameter %s", key);
      return NULL;
    }
    *val = '\0';
    ++val;

    if (! strcmp(key, "endcap"))
    {
      /* Supported end cap styles: "round", "flat", "square" */
      if (! strcmp(val, "round"))
        endCapStyle = ENDCAP_ROUND;
      else if (! strcmp(val, "flat") || ! strcmp(val, "butt"))
        endCapStyle = ENDCAP_FLAT;
      else if (! strcmp(val, "square"))
        endCapStyle = ENDCAP_SQUARE;
      else
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
          "Invalid buffer end cap style: %s (accept: 'round', 'flat', "
          "'butt' or 'square')", val);
        return NULL;
      }
    }
    else if (! strcmp(key, "join"))
    {
      if (! strcmp(val, "round"))
        joinStyle = JOIN_ROUND;
      else if (! strcmp(val, "mitre") || ! strcmp(val, "miter"))
        joinStyle = JOIN_MITRE;
      else if (! strcmp(val, "bevel"))
        joinStyle = JOIN_BEVEL;
      else
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
          "Invalid buffer end cap style: %s (accept: 'round', 'mitre', "
          "'miter'  or 'bevel')", val);
        return NULL;
      }
    }
    else if (! strcmp(key, "mitre_limit") || ! strcmp(key, "miter_limit"))
      /* mitreLimit is a float */
      mitreLimit = atof(val);
    else if (! strcmp(key, "quad_segs"))
      /* quadrant segments is an int */
      quadsegs = atoi(val);
    else if (! strcmp(key, "side"))
    {
      if (! strcmp(val, "both"))
        singleside = 0;
      else if (! strcmp(val, "left"))
        singleside = 1;
      else if (! strcmp(val, "right"))
      {
        singleside = 1;
        size *= -1;
      }
      else
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
          "Invalid side parameter: %s (accept: 'right', 'left', 'both')",
          val);
        return NULL;
      }
    }
    else
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Invalid buffer parameter: %s (accept: 'endcap', 'join', "
        "'mitre_limit', 'miter_limit', 'quad_segs' and 'side')", key);
      return NULL;
    }
  }

  /* Empty.Buffer() == Empty[polygon] */
  if (gserialized_is_empty(gs))
  {
    lwg = lwpoly_as_lwgeom(lwpoly_construct_empty(gserialized_get_srid(gs),
      0, 0)); // buffer wouldn't give back z or m anyway
    GSERIALIZED *result = geo_serialize(lwg);
    lwgeom_free(lwg);
    return result;
  }

  lwg = lwgeom_from_gserialized(gs);

  if (! lwgeom_isfinite(lwg))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "Geometry contains invalid coordinates");
    return NULL;
  }

  lwgeom_free(lwg);

  initGEOS(lwnotice, lwgeom_geos_error);

  g1 = POSTGIS2GEOS(gs);
  if (! g1)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "First argument geometry could not be converted to GEOS");
    return NULL;
  }

  bufferparams = GEOSBufferParams_create();
  if (bufferparams)
  {
    if (GEOSBufferParams_setEndCapStyle(bufferparams, endCapStyle) &&
      GEOSBufferParams_setJoinStyle(bufferparams, joinStyle) &&
      GEOSBufferParams_setMitreLimit(bufferparams, mitreLimit) &&
      GEOSBufferParams_setQuadrantSegments(bufferparams, quadsegs) &&
      GEOSBufferParams_setSingleSided(bufferparams, singleside))
    {
      g3 = GEOSBufferWithParams(g1, bufferparams, size);
    }
    else
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Error setting buffer parameters.");
    }
    GEOSBufferParams_destroy(bufferparams);
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Error setting buffer parameters.");
  }

  GEOSGeom_destroy(g1);

  if (! g3)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "GEOSBuffer returned error");
    return NULL;
  }

  GEOSSetSRID(g3, gserialized_get_srid(gs));

  GSERIALIZED *result = GEOS2POSTGIS(g3, gserialized_has_z(gs));
  GEOSGeom_destroy(g3);
  if (! result)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "GEOS buffer() threw an error (result postgis geometry formation)!");
    return NULL; /* never get here */
  }
  return result;
}

/*****************************************************************************
 * Functions adapted from lwgeom_geos_predicates.c
 *****************************************************************************/

/**
 * @ingroup meos_geo_base_comp
 * @brief Return true if the geometries/geographies are equal, false otherwise
 * @param[in] gs1,gs2 Geometries/geographies
 * @note PostGIS function: @p ST_Equals(PG_FUNCTION_ARGS)
 */
int
geo_equals(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  assert(gs1); assert(gs2); 
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));

  /* Empty == Empty */
  if ( gserialized_is_empty(gs1) && gserialized_is_empty(gs2) )
    return 1;

  /*
   * Short-circuit: If gs1 and gs2 do not have the same bounding box
   * we can return FALSE.
   */
  GBOX box1, box2;
  if ( gserialized_get_gbox_p(gs1, &box1) &&
       gserialized_get_gbox_p(gs2, &box2) )
  {
    // ORIGINAL DEFINITION: TODO verify that the generalization to 3D is OK
    // if ( gbox_same_2d_float(&box1, &box2) == LW_FALSE )
    if ( gbox_same(&box1, &box2) == LW_FALSE )
      return 0;
  }

  /*
   * Short-circuit: if gs1 and gs2 are binary-equivalent, we can return
   * TRUE.  This is much faster than doing the comparison using GEOS.
   */
  if (VARSIZE(gs1) == VARSIZE(gs2) && ! memcmp(gs1, gs2, VARSIZE(gs1)))
      return 1;

  initGEOS(lwnotice, lwgeom_geos_error);

  GEOSGeometry *geos1 = POSTGIS2GEOS(gs1);
  if (! geos1)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "First argument geometry could not be converted to GEOS");
    return -1;
  }

  GEOSGeometry *geos2 = POSTGIS2GEOS(gs2);
  if (! geos2)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "First argument geometry could not be converted to GEOS");
    GEOSGeom_destroy(geos1);
    return -1;
  }

  int result = GEOSEquals(geos1, geos2);
  GEOSGeom_destroy(geos1);
  GEOSGeom_destroy(geos2);

  if (result == 2)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "GEOS equals() threw an error !");
    return -1;
  }

  return result;
}

/*****************************************************************************
 * Functions borrowed from lwgeom_pg.c
 *****************************************************************************/

/**
 * @brief Utility method to call the serialization and then set the
 * PgSQL varsize header appropriately with the serialized size.
 */
GSERIALIZED *
geom_serialize(LWGEOM *lwgeom)
{
  size_t ret_size;
  GSERIALIZED *result = gserialized_from_lwgeom(lwgeom, &ret_size);
  SET_VARSIZE(result, ret_size);
  return result;
}

/**
 * @brief Utility method to call the serialization and then set the
 * PgSQL varsize header appropriately with the serialized size.
 */
GSERIALIZED *
geog_serialize(LWGEOM *lwgeom)
{
  /** force to geodetic in case it's not **/
  lwgeom_set_geodetic(lwgeom, true);
  size_t ret_size;
  GSERIALIZED *result = gserialized_from_lwgeom(lwgeom,  &ret_size);
  SET_VARSIZE(result, ret_size);
  return result;
}

/**
 * @brief Serialize a geometry/geography
 * @pre It is supposed that the flags such as Z and geodetic have been
 * set up before by the calling function
 */
GSERIALIZED *
geo_serialize(const LWGEOM *geom)
{
  GSERIALIZED *result = FLAGS_GET_GEODETIC(geom->flags) ?
    geog_serialize((LWGEOM *) geom) : geom_serialize((LWGEOM *) geom);
  return result;
}

/*****************************************************************************
 * Functions adapted from lwgeom_transform.c
 *****************************************************************************/

/**
 * @ingroup meos_geo_base_srid
 * @brief Returns the geometry/geography transformed to an SRID
 * @return On error return @p NULL
 * @param[in] gs Geometry/geography
 * @param[in] srid_to Target SRID
 * @note PostGIS function: @p transform(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geo_transform(GSERIALIZED *gs, int32_t srid_to)
{
  if (srid_to == SRID_UNKNOWN)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "geo_transform: %d is an invalid target SRID", SRID_UNKNOWN);
    return NULL;
  }

  int32_t srid_from = gserialized_get_srid(gs);

  if (srid_from == SRID_UNKNOWN)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "geo_transform: Input geometry has unknown (%d) SRID", SRID_UNKNOWN);
    return NULL;
  }

  /* Input SRID and output SRID are equal, noop */
  if (srid_from == srid_to)
    return gs;

  LWPROJ *pj = lwproj_get(srid_from, srid_to);
  if (! pj)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "geo_transform: Error when getting projection.");
    return NULL;
  }

  /* now we have a geometry, and input/output PJ structs. */
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  lwgeom_transform(lwgeom, pj);
  lwgeom->srid = srid_to;

  /* Re-compute bbox if input had one (COMPUTE_BBOX TAINTING) */
  if (lwgeom->bbox)
  {
    lwgeom_refresh_bbox(lwgeom);
  }

  GSERIALIZED *result = geo_serialize(lwgeom);
  lwgeom_free(lwgeom);
  return result; /* new geometry */
}

/**
 * @ingroup meos_geo_base_srid
 * @brief Return a geometry/geography transformed to another SRID using a pipeline
 * @param[in] gs Geometry/geography
 * @param[in] pipeline Pipeline string
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN
 * @param[in] is_forward True when the transformation is forward
 * @note PostGIS function: @p transform_pipeline_geom(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geo_transform_pipeline(const GSERIALIZED *gs, char *pipeline, int32_t srid_to,
  bool is_forward)
{
  assert(gs); assert(pipeline);
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  int rv = lwgeom_transform_pipeline(geom, pipeline, is_forward);

  if (rv == LW_FAILURE)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Coordinate transformation failed");
    return NULL;
  }

  /* Re-compute bbox if input had one (COMPUTE_BBOX TAINTING) */
  geom->srid = srid_to;
  if (geom->bbox)
    lwgeom_refresh_bbox(geom);

  GSERIALIZED *result = geo_serialize(geom);
  lwgeom_free(geom);
  return (result); /* new geometry */
}

/*****************************************************************************
 * Functions adapted from geography_centroid.c
 *****************************************************************************/

POINT3D *
lonlat_to_cart(const double_t raw_lon, const double_t raw_lat)
{
  POINT3D *point = lwalloc(sizeof(POINT3D));
  // prepare coordinate for trigonometric functions from [-90, 90] -> [0, pi]
  double_t lat = (raw_lat + 90) / 180 * M_PI;
  // prepare coordinate for trigonometric functions from [-180, 180] -> [-pi, pi]
  double_t lon = raw_lon / 180 * M_PI;
  /* calculate value only once */
  double_t sin_lat = sinl(lat);
  /* convert to 3D cartesian coordinates */
  point->x = sin_lat * cosl(lon);
  point->y = sin_lat * sinl(lon);
  point->z = cosl(lat);
  return point;
}

LWPOINT *
cart_to_lwpoint(const double_t x_sum, const double_t y_sum,
  const double_t z_sum, const double_t weight_sum, const int32_t srid)
{
  double_t x = x_sum / weight_sum;
  double_t y = y_sum / weight_sum;
  double_t z = z_sum / weight_sum;
  /* x-y-z vector length */
  double_t r = sqrtl(powl(x, 2) + powl(y, 2) + powl(z, 2));
  double_t lon = atan2l(y, x) * 180 / M_PI;
  double_t lat = acosl(z / r) * 180 / M_PI - 90;
  return lwpoint_make2d(srid, lon, lat);
}

/**
 * @brief Convert lat-lon-points to x-y-z-coordinates, calculate a weighted
 * average point and return lat-lon-coordinated
 */
LWPOINT *
geography_centroid_from_wpoints(const int32_t srid, const POINT3DM *points,
  const uint32_t size)
{
  double_t x_sum = 0;
  double_t y_sum = 0;
  double_t z_sum = 0;
  double_t weight_sum = 0;
  double_t weight = 1;
  POINT3D* point;
  for (uint32_t i = 0; i < size; i++ )
  {
    point = lonlat_to_cart(points[i].x, points[i].y);
    weight = points[i].m;
    x_sum += point->x * weight;
    y_sum += point->y * weight;
    z_sum += point->z * weight;
    weight_sum += weight;
    lwfree(point);
  }
  return cart_to_lwpoint(x_sum, y_sum, z_sum, weight_sum, srid);
}

/**
 * @brief Split lines into segments and calculate with middle of segment as
 * weighted point
 */
LWPOINT *
geography_centroid_from_mline(const LWMLINE* mline, SPHEROID *s)
{
  double_t tolerance = 0.0;
  uint32_t size = 0;
  uint32_t i, k, j = 0;
  POINT3DM* points;
  LWPOINT* result;

  /* get total number of points */
  for (i = 0; i < mline->ngeoms; i++)
    size += (mline->geoms[i]->points->npoints - 1) * 2;
  points = palloc(size*sizeof(POINT3DM));
  for (i = 0; i < mline->ngeoms; i++)
  {
    LWLINE* line = mline->geoms[i];
    /* add both points of line segment as weighted point */
    for (k = 0; k < line->points->npoints - 1; k++)
    {
      const POINT2D* p1 = getPoint2d_cp(line->points, k);
      const POINT2D* p2 = getPoint2d_cp(line->points, k+1);
      double_t weight;

      /* use line-segment length as weight */
      LWPOINT* lwp1 = lwpoint_make2d(mline->srid, p1->x, p1->y);
      LWPOINT* lwp2 = lwpoint_make2d(mline->srid, p2->x, p2->y);
      LWGEOM* lwgeom1 = lwpoint_as_lwgeom(lwp1);
      LWGEOM* lwgeom2 = lwpoint_as_lwgeom(lwp2);
      lwgeom_set_geodetic(lwgeom1, LW_TRUE);
      lwgeom_set_geodetic(lwgeom2, LW_TRUE);

      /* use point distance as weight */
      weight = lwgeom_distance_spheroid(lwgeom1, lwgeom2, s, tolerance);
      points[j].x = p1->x;
      points[j].y = p1->y;
      points[j].m = weight;
      j++;
      points[j].x = p2->x;
      points[j].y = p2->y;
      points[j].m = weight;
      j++;
      lwgeom_free(lwgeom1);
      lwgeom_free(lwgeom2);
    }
  }

  result = geography_centroid_from_wpoints(mline->srid, points, size);
  pfree(points);
  return result;
}

/**
 * Split polygons into triangles and use centroid of the triangle with the
 * triangle area as weight to calculate the centroid of a (multi)polygon.
 */
LWPOINT *
geography_centroid_from_mpoly(const LWMPOLY* mpoly, bool use_spheroid,
  SPHEROID *s)
{
  uint32_t size = 0;
  uint32_t i, ir, ip, j = 0;
  POINT3DM* points;
  POINT4D* reference_point = NULL;
  LWPOINT* result = NULL;

  for (ip = 0; ip < mpoly->ngeoms; ip++)
    for (ir = 0; ir < mpoly->geoms[ip]->nrings; ir++)
      size += mpoly->geoms[ip]->rings[ir]->npoints - 1;

  points = palloc(size*sizeof(POINT3DM));

  /* use first point as reference to create triangles */
  reference_point = (POINT4D*) getPoint2d_cp(mpoly->geoms[0]->rings[0], 0);

  for (ip = 0; ip < mpoly->ngeoms; ip++)
  {
    LWPOLY* poly = mpoly->geoms[ip];
    for (ir = 0; ir < poly->nrings; ir++)
    {
      POINTARRAY* ring = poly->rings[ir];

      /* split into triangles (two points + reference point) */
      for (i = 0; i < ring->npoints - 1; i++)
      {
        const POINT4D* p1 = (const POINT4D*) getPoint2d_cp(ring, i);
        const POINT4D* p2 = (const POINT4D*) getPoint2d_cp(ring, i+1);
        LWPOLY* poly_tri;
        LWGEOM* geom_tri;
        double_t weight;
        POINT3DM triangle[3];
        LWPOINT* tri_centroid;

        POINTARRAY* pa = ptarray_construct_empty(0, 0, 4);
        ptarray_insert_point(pa, p1, 0);
        ptarray_insert_point(pa, p2, 1);
        ptarray_insert_point(pa, reference_point, 2);
        ptarray_insert_point(pa, p1, 3);

        poly_tri = lwpoly_construct_empty(mpoly->srid, 0, 0);
        lwpoly_add_ring(poly_tri, pa);

        geom_tri = lwpoly_as_lwgeom(poly_tri);
        lwgeom_set_geodetic(geom_tri, LW_TRUE);

        /* Calculate the weight of the triangle. If counter clockwise,
         * the weight is negative (e.g. for holes in polygons) */

        if (use_spheroid)
          weight = lwgeom_area_spheroid(geom_tri, s);
        else
          weight = lwgeom_area_sphere(geom_tri, s);


        triangle[0].x = p1->x;
        triangle[0].y = p1->y;
        triangle[0].m = 1;

        triangle[1].x = p2->x;
        triangle[1].y = p2->y;
        triangle[1].m = 1;

        triangle[2].x = reference_point->x;
        triangle[2].y = reference_point->y;
        triangle[2].m = 1;

        /* get center of triangle */
        tri_centroid = geography_centroid_from_wpoints(mpoly->srid, triangle, 3);

        points[j].x = lwpoint_get_x(tri_centroid);
        points[j].y = lwpoint_get_y(tri_centroid);
        points[j].m = weight;
        j++;

        lwpoint_free(tri_centroid);
        lwgeom_free(geom_tri);
       }
    }
  }
  result = geography_centroid_from_wpoints(mpoly->srid, points, size);
  pfree(points);
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the centroid of a geometry
 * @note PostGIS function: @p geography_centroid(PG_FUNCTION_ARGS). 
 */
GSERIALIZED *
geog_centroid(const GSERIALIZED *g, bool use_spheroid)
{
  LWGEOM *lwgeom_out = NULL;
  LWPOINT *lwpoint_out = NULL;
  GSERIALIZED *g_out = NULL;
  SPHEROID s;

  /* Get our geometry object loaded into memory. */
  LWGEOM *lwgeom = lwgeom_from_gserialized(g);
  int32_t srid = lwgeom_get_srid(lwgeom);

  /* on empty input, return empty output */
  if (gserialized_is_empty(g))
  {
    LWCOLLECTION *empty = lwcollection_construct_empty(COLLECTIONTYPE, srid,
      0, 0);
    lwgeom_out = lwcollection_as_lwgeom(empty);
    g_out = geo_serialize(lwgeom_out);
    return g_out;
  }

  /* Initialize spheroid */
  spheroid_init_from_srid(srid, &s);

  /* Set to sphere if requested */
  if (! use_spheroid)
    s.a = s.b = s.radius;

  switch (lwgeom_get_type(lwgeom))
  {
    case POINTTYPE:
    {
      /* centroid of a point is itself */
      return geo_copy(g);
    }
    case MULTIPOINTTYPE:
    {
      LWMPOINT* mpoints = lwgeom_as_lwmpoint(lwgeom);

      /* average between all points */
      uint32_t size = mpoints->ngeoms;
      POINT3DM* points = palloc(size*sizeof(POINT3DM));
      for (uint32_t i = 0; i < size; i++)
      {
        points[i].x = lwpoint_get_x(mpoints->geoms[i]);
        points[i].y = lwpoint_get_y(mpoints->geoms[i]);
        points[i].m = 1;
      }
      lwpoint_out = geography_centroid_from_wpoints(srid, points, size);
      pfree(points);
      break;
    }
    case LINETYPE:
    {
      LWLINE* line = lwgeom_as_lwline(lwgeom);

      /* reuse mline function */
      LWMLINE* mline = lwmline_construct_empty(srid, 0, 0);
      lwmline_add_lwline(mline, line);

      lwpoint_out = geography_centroid_from_mline(mline, &s);
      lwmline_free(mline);
      break;
    }
    case MULTILINETYPE:
    {
      LWMLINE* mline = lwgeom_as_lwmline(lwgeom);
      lwpoint_out = geography_centroid_from_mline(mline, &s);
      break;
    }
    case POLYGONTYPE:
    {
      LWPOLY* poly = lwgeom_as_lwpoly(lwgeom);
      /* reuse mpoly function */
      LWMPOLY* mpoly = lwmpoly_construct_empty(srid, 0, 0);
      lwmpoly_add_lwpoly(mpoly, poly);
      lwpoint_out = geography_centroid_from_mpoly(mpoly, use_spheroid, &s);
      lwmpoly_free(mpoly);
      break;
    }
    case MULTIPOLYGONTYPE:
    {
      LWMPOLY* mpoly = lwgeom_as_lwmpoly(lwgeom);
      lwpoint_out = geography_centroid_from_mpoly(mpoly, use_spheroid, &s);
      break;
    }
    default:
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "ST_Centroid(geography) unhandled geography type");
      return NULL;
    }
  }
  lwgeom_out = lwpoint_as_lwgeom(lwpoint_out);
  g_out = geo_serialize(lwgeom_out);
  return g_out;
}

/*****************************************************************************
 * Functions adapted from geography_measurement.c
 *****************************************************************************/

/**
 * @ingroup meos_geo_base_accessor
 * @brief Returns the area of a geography in square meters
 * @param[in] gs Geography
 * @param[in] use_spheroid True when using a spheroid
 * @return On error return @p DBL_MAX
 * @note PostGIS function: @p geography_area(PG_FUNCTION_ARGS)
 */
double
geog_area(const GSERIALIZED *gs, bool use_spheroid)
{
  LWGEOM *lwgeom = NULL;
  GBOX gbox;
  double area;

  /* Initialize spheroid */
  /* We currently cannot use the next statement since it uses PostGIS cache */
  SPHEROID s;
  spheroid_init_from_srid(gserialized_get_srid(gs), &s);

  lwgeom = lwgeom_from_gserialized(gs);

  /* EMPTY things have no area */
  if ( lwgeom_is_empty(lwgeom) )
  {
    lwgeom_free(lwgeom);
    return 0.0;
  }

  if ( lwgeom->bbox )
    gbox = *(lwgeom->bbox);
  else
    lwgeom_calculate_gbox_geodetic(lwgeom, &gbox);

#ifndef PROJ_GEODESIC
  /* Test for cases that are currently not handled by spheroid code */
  if ( use_spheroid )
  {
    /* We can't circle the poles right now */
    if ( FP_GTEQ(gbox.zmax,1.0) || FP_LTEQ(gbox.zmin,-1.0) )
      use_spheroid = LW_FALSE;
    /* We can't cross the equator right now */
    if ( gbox.zmax > 0.0 && gbox.zmin < 0.0 )
      use_spheroid = LW_FALSE;
  }
#endif /* ifndef PROJ_GEODESIC */

  /* User requests spherical calculation, turn our spheroid into a sphere */
  if ( ! use_spheroid )
    s.a = s.b = s.radius;

  /* Calculate the area */
  if ( use_spheroid )
    area = lwgeom_area_spheroid(lwgeom, &s);
  else
    area = lwgeom_area_sphere(lwgeom, &s);

  /* Clean up */
  lwgeom_free(lwgeom);

  /* Something went wrong... */
  if ( area < 0.0 )
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "lwgeom_area_spheroid returned length < 0.0");
    return DBL_MAX;
  }

  return area;
}

/**
 * @ingroup meos_geo_base_accessor
 * @brief Returns the perimeter of a geography in meters
 * @param[in] gs Geography
 * @param[in] use_spheroid True when using a spheroid
 * @return On error return @p DBL_MAX
 * @note PostGIS function: @p geography_perimeter(PG_FUNCTION_ARGS)
 */
double
geog_perimeter(const GSERIALIZED *gs, bool use_spheroid)
{
  LWGEOM *lwgeom = NULL;
  double length;
  int type;

  /* Only return for area features. */
  type = gserialized_get_type(gs);
  if ( ! (type == POLYGONTYPE || type == MULTIPOLYGONTYPE || type == COLLECTIONTYPE) )
  {
    return 0.0;
  }

  lwgeom = lwgeom_from_gserialized(gs);

  /* EMPTY things have no perimeter */
  if ( lwgeom_is_empty(lwgeom) )
  {
    lwgeom_free(lwgeom);
    return 0.0;
  }

  /* Initialize spheroid */
  SPHEROID s;
  spheroid_init_from_srid(gserialized_get_srid(gs), &s);

  /* User requests spherical calculation, turn our spheroid into a sphere */
  if ( ! use_spheroid )
    s.a = s.b = s.radius;

  /* Calculate the length */
  length = lwgeom_length_spheroid(lwgeom, &s);

  /* Something went wrong... */
  if ( length < 0.0 )
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "lwgeom_perimeter_spheroid returned length < 0.0");
    return DBL_MAX;
  }

  /* Clean up, but not all the way to the point arrays */
  lwgeom_free(lwgeom);

  return length;
}

/**
 * @ingroup meos_geo_base_accessor
 * @brief Return double length in meters
 * @param[in] gs Geography
 * @param[in] use_spheroid True when using a spheroid
 * @return On error return @p DBL_MAX
 * @note PostGIS function: @p geography_length(PG_FUNCTION_ARGS)
 */
double
geog_length(const GSERIALIZED *gs, bool use_spheroid)
{
  /* EMPTY things have no length */
  int32 geo_type = gserialized_get_type(gs);
  if (gserialized_is_empty(gs) || geo_type == POLYGONTYPE ||
    geo_type == MULTIPOLYGONTYPE)
    return 0.0;

  /* Get our geometry object loaded into memory. */
  LWGEOM *geom = lwgeom_from_gserialized(gs);

  /* Initialize spheroid */
  SPHEROID s;
  spheroid_init_from_srid(gserialized_get_srid(gs), &s);

  /* User requests spherical calculation, turn our spheroid into a sphere */
  if (!  use_spheroid )
    s.a = s.b = s.radius;

  /* Calculate the length */
  double length = lwgeom_length_spheroid(geom, &s);

  /* Something went wrong... */
  if ( length < 0.0 )
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "lwgeom_length_spheroid returned length < 0.0");
    return DBL_MAX;
  }

  /* Clean up */
  lwgeom_free(geom);

  return length;
}

/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if two geographies are within a distance
 * @param[in] gs1,gs2 Geographies
 * @param[in] tolerance Tolerance
 * @param[in] use_spheroid True when using a spheroid
 * @note PostGIS function: @p geography_dwithin_uncached(PG_FUNCTION_ARGS)
 * where we use the WGS84 spheroid
 */
bool
geog_dwithin(const GSERIALIZED *gs1, const GSERIALIZED *gs2, double tolerance,
  bool use_spheroid)
{
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));

  /* Return FALSE on empty arguments. */
  if (gserialized_is_empty(gs1) || gserialized_is_empty(gs2))
    return false;

  /* Initialize spheroid */
  SPHEROID s;
  spheroid_init_from_srid(gserialized_get_srid(gs1), &s);

  /* Set to sphere if requested */
  if (! use_spheroid)
    s.a = s.b = s.radius;

  LWGEOM *lwgeom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *lwgeom2 = lwgeom_from_gserialized(gs2);
  double distance = lwgeom_distance_spheroid(lwgeom1, lwgeom2, &s, tolerance);

  /* Clean up */
  lwgeom_free(lwgeom1);
  lwgeom_free(lwgeom2);

  /* Something went wrong... should already be eloged, return FALSE */
  if (distance < 0.0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "lwgeom_distance_spheroid returned negative!");
    return false;
  }

  return (distance <= tolerance);
}

/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if the geographies intersect
 * @param[in] gs1,gs2 Geographies
 * @param[in] use_spheroid True when using a spheroid
 * @note PostGIS function: @p geography_intersects(PG_FUNCTION_ARGS)
 */
inline bool
geog_intersects(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  bool use_spheroid)
{
  return geog_dwithin(gs1, gs2, 0.0, use_spheroid);
}

/* Defined in liblwgeom_internal.h */
#define PGIS_FP_TOLERANCE 1e-12

/**
 * @ingroup meos_geo_base_distance
 * @brief Return the distance between two geographies
 * @param[in] gs1,gs2 Geographies
 * @note PostGIS function: @p geography_distance_uncached(PG_FUNCTION_ARGS).
 * We set by default both @p tolerance and @p use_spheroid and initialize the
 * spheroid to WGS84
 * @note Errors return -1 to replace return @p NULL
 */
double
geog_distance(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs1));

  /* Return NULL on empty arguments. */
  if (gserialized_is_empty(gs1) || gserialized_is_empty(gs2) )
    return -1;

  double tolerance = PGIS_FP_TOLERANCE;
  bool use_spheroid = true;

  /* Initialize spheroid */
  SPHEROID s;
  spheroid_init_from_srid(gserialized_get_srid(gs1), &s);

  /* Set to sphere if requested */
  if (!  use_spheroid )
    s.a = s.b = s.radius;

  LWGEOM *lwgeom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *lwgeom2 = lwgeom_from_gserialized(gs2);

  /* Make sure we have boxes attached */
  lwgeom_add_bbox_deep(lwgeom1, NULL);
  lwgeom_add_bbox_deep(lwgeom2, NULL);

  double distance = lwgeom_distance_spheroid(lwgeom1, lwgeom2, &s, tolerance);

  /* Clean up */
  lwgeom_free(lwgeom1);
  lwgeom_free(lwgeom2);

  /* Something went wrong, negative return... should already be eloged, return NULL */
  if ( distance < 0.0 )
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "geography_distance returned distance < 0.0");
    return -1;
  }

  return distance;
}

/*****************************************************************************
 * Functions adapted from lwgeom_inout.c
 *****************************************************************************/

/**
* @brief Check the consistency of the metadata to enforce in the typmod:
* SRID, type, and dimensionality. If things are inconsistent, return NULL
* @note Function from gserialized_typmod.c
*/
GSERIALIZED *
postgis_valid_typmod(GSERIALIZED *gs, int32_t typmod)
{
  int32 geom_srid = gserialized_get_srid(gs);
  int32 geom_type = gserialized_get_type(gs);
  int32 geom_z = gserialized_has_z(gs);
  int32 geom_m = gserialized_has_m(gs);
  int32 typmod_srid = TYPMOD_GET_SRID(typmod);
  int32 typmod_type = TYPMOD_GET_TYPE(typmod);
  int32 typmod_z = TYPMOD_GET_Z(typmod);
  int32 typmod_m = TYPMOD_GET_M(typmod);

  /* No typmod (-1) => no preferences */
  if (typmod < 0) return gs;

  /*
  * #3031: If a user is handing us a MULTIPOINT EMPTY but trying to fit it into
  * a POINT geometry column, there's a strong chance the reason she has
  * a MULTIPOINT EMPTY because we gave it to her during data dump,
  * converting the internal POINT EMPTY into a EWKB MULTIPOINT EMPTY
  * (because EWKB doesn't have a clean way to represent POINT EMPTY).
  * In such a case, it makes sense to turn the MULTIPOINT EMPTY back into a
  * point EMPTY, rather than throwing an error.
  */
  if ( typmod_type == POINTTYPE && geom_type == MULTIPOINTTYPE &&
       gserialized_is_empty(gs) )
  {
    LWPOINT *empty_point = lwpoint_construct_empty(geom_srid, geom_z, geom_m);
    geom_type = POINTTYPE;
    pfree(gs);
    /* MEOS: use internal geo_serialize that copes with both geom and geog */
    gs = geo_serialize(lwpoint_as_lwgeom(empty_point));
  }

  /* Typmod has a preference for SRID, but geometry does not? Harmonize the geometry SRID. */
  if ( typmod_srid > 0 && geom_srid == 0 )
  {
    gserialized_set_srid(gs, typmod_srid);
    geom_srid = typmod_srid;
  }

  /* Typmod has a preference for SRID? Geometry SRID had better match. */
  if ( typmod_srid > 0 && typmod_srid != geom_srid )
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Geometry SRID (%d) does not match column SRID (%d)",
      geom_srid, typmod_srid);
    return NULL;
  }

  /* Typmod has a preference for geometry type. */
  if ( typmod_type > 0 &&
          /* GEOMETRYCOLLECTION column can hold any kind of collection */
          ( (typmod_type == COLLECTIONTYPE && !
              (geom_type == COLLECTIONTYPE || geom_type == MULTIPOLYGONTYPE ||
               geom_type == MULTIPOINTTYPE || geom_type == MULTILINETYPE )) ||
           /* Other types must be strictly equal. */
           (typmod_type != geom_type)) )
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Geometry type (%s) does not match column type (%s)",
      lwtype_name(geom_type), lwtype_name(typmod_type));
    return NULL;
  }

  /* Mismatched Z dimensionality. */
  if ( typmod_z && ! geom_z )
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Column has Z dimension but geometry does not");
    return NULL;
  }

  /* Mismatched Z dimensionality (other way). */
  if ( geom_z && ! typmod_z )
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Geometry has Z dimension but column does not");
    return NULL;
  }

  /* Mismatched M dimensionality. */
  if ( typmod_m && ! geom_m )
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Column has M dimension but geometry does not");
    return NULL;
  }

  /* Mismatched M dimensionality (other way). */
  if ( geom_m && ! typmod_m )
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Geometry has M dimension but column does not");
    return NULL;
  }

  return gs;
}

/**
 * @ingroup meos_geo_base_input
 * @brief Return a geometry from its Well-Known Text (WKT), Well-Known Binary
 * (WKB) or GeoJSON representation
 * @details The format is @p '[SRID=#;]wkt|wkb'. Examples of input are as
 * follows:
 * @code
 * 'SRID=99;POINT(0 0)'
 * 'POINT(0 0)' --> assumes SRID=SRID_UNKNOWN
 * 'SRID=99;0101000000000000000000F03F000000000000004'
 * '0101000000000000000000F03F000000000000004'
 * '{"type":"Point","coordinates":[1,1]}'
 * @endcode
 * @param[in] str String
 * @param[in] typmod Typmod
 * @note PostGIS function: @p LWGEOM_in(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geom_in(const char *str, int32 typmod)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);

  LWGEOM_PARSER_RESULT lwg_parser_result;
  LWGEOM *lwgeom;
  GSERIALIZED *result;
  int32_t srid = 0;

  lwgeom_parser_result_init(&lwg_parser_result);

  /* Empty string. */
  if (str[0] == '\0')
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT, "parse error - invalid geometry");
    return NULL;
  }

  /* Starts with "SRID=" */
  const char *str1 = str;
  if (pg_strncasecmp(str1, "SRID=", 5) == 0)
  {
    /* Roll forward to semi-colon */
    int delim = 0;
    while ((str1)[delim] != ';' && (str1)[delim] != '\0')
      delim++;
    if ((str1)[delim] == '\0')
    {
      meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
        "Could not parse geometry value: %s", str);
      return false;
    }

    /* Check next character to see if we have WKB */
    if ((str1)[delim + 1] == '0')
    {
      char *tmp = palloc(sizeof(char) * (delim + 1));
      strncpy(tmp, str1, delim);
      /* Null terminate the SRID= string */
      tmp[delim] = '\0';
      /* Set str1 to the start of the real WKB */
      str1 += delim + 1;
      /* Move str to the start of the numeric part and parse the SRID number */
      srid = atoi(tmp + 5);
      pfree(tmp);
    }
  }

  /* WKB? Let's find out. */
  if (str1[0] == '0')
  {
    size_t hexsize = strlen(str1);
    unsigned char *wkb = bytes_from_hexbytes(str1, hexsize);
    /* TODO: 20101206: No parser checks! This is inline with current 1.5 behavior, but needs discussion */
    lwgeom = lwgeom_from_wkb(wkb, hexsize/2, LW_PARSER_CHECK_NONE);
    /* If we picked up an SRID at the head of the WKB set it manually */
    if ( srid ) lwgeom_set_srid(lwgeom, srid);
    /* Add a bbox if necessary */
    if ( lwgeom_needs_bbox(lwgeom) ) lwgeom_add_bbox(lwgeom);
    lwfree(wkb);
    result = geo_serialize(lwgeom);
    lwgeom_free(lwgeom);
  }
  else if (str1[0] == '{')
  {
    char *srs = NULL;
    lwgeom = lwgeom_from_geojson(str1, &srs);
    if (srs)
    {
      srid = SRID_DEFAULT; // TODO
      // srid = GetSRIDCacheBySRS(fcinfo, srs);
      lwfree(srs);
      lwgeom_set_srid(lwgeom, srid);
    }
    result = geo_serialize(lwgeom);
    lwgeom_free(lwgeom);
  }
  /* WKT then. */
  else
  {
    if ( lwgeom_parse_wkt(&lwg_parser_result, (char *) str1, 
      LW_PARSER_CHECK_ALL) == LW_FAILURE )
    {
      PG_PARSER_ERROR(lwg_parser_result);
      return NULL;
    }
    lwgeom = lwg_parser_result.geom;
    if ( lwgeom_needs_bbox(lwgeom) )
      lwgeom_add_bbox(lwgeom);
    result = geo_serialize(lwgeom);
    lwgeom_parser_result_free(&lwg_parser_result);
  }

  if (typmod >= 0)
    result = postgis_valid_typmod(result, typmod);

  /* Don't free the parser result (and hence lwgeom) until we have done */
  /* the typemod check with lwgeom */
  return result;
}

/**
 * @ingroup meos_geo_base_input
 * @brief Return the hex-encoded ASCII Well-Known Binary (HexWKB)
 * representation of a geometry/geography
 * @details The output is `'SRID=#;{wkb in hex form}'`,
 * e.g., @p 'SRID=-99;0101000000000000000000F03F0000000000000040'.
 * The WKB otput is in the machine endian.
 * If SRID=-1, the @p 'SRID=-1;' will probably not be present.
 * @param[in] gs Geometry/geography
 * @note PostGIS function: @p LWGEOM_out(PG_FUNCTION_ARGS)
 */
char *
geo_out(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, false);

  LWGEOM *geom = lwgeom_from_gserialized(gs);
  char *result = lwgeom_to_hexwkb_buffer(geom, WKB_EXTENDED);
  lwgeom_free(geom);
  return result;
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return a geometry/geography from its WKT representation (and
 * optionally a SRID)
 * @param[in] wkt WKT string
 * @param[in] srid SRID
 */
GSERIALIZED *
geo_from_text(const char *wkt, int32_t srid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkt, NULL);

  LWGEOM_PARSER_RESULT lwg_parser_result;
  GSERIALIZED *geo_result = NULL;
  LWGEOM *lwgeom;

  if (lwgeom_parse_wkt(&lwg_parser_result, (char *) wkt,
      LW_PARSER_CHECK_ALL) == LW_FAILURE )
    PG_PARSER_ERROR(lwg_parser_result);

  lwgeom = lwg_parser_result.geom;

  if ( lwgeom->srid != SRID_UNKNOWN )
  {
    meos_error(WARNING, MEOS_ERR_TEXT_INPUT,
      "OGC WKT expected, EWKT provided - use GeomFromEWKT() for this");
    return NULL;
  }

  /* read user-requested SRID if any */
  if ( srid > 0 )
    lwgeom_set_srid(lwgeom, srid);

  geo_result = geo_serialize(lwgeom);
  /* Clean up */
  lwgeom_free(lwg_parser_result.geom);
  lwgeom_parser_result_free(&lwg_parser_result);

  return geo_result;
}

/**
 * @brief Return the (Extended) Well-Known Text (EWKT or WKT) representation of
 * a geometry/geography
 * @param[in] gs Geometry/geography
 * @param[in] precision Maximum number of decimal digits
 * @param[in] extended True for the EWKT representation, false for the WKT one
 * @note This is a a stricter version of #geom_in, where we refuse to
 * accept (HEX)WKB or EWKT.
 */
char *
geo_as_wkt(const GSERIALIZED *gs, int precision, bool extended)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);

  LWGEOM *geom = lwgeom_from_gserialized(gs);
  char *result = lwgeom_to_wkt(geom, extended ? WKT_EXTENDED : WKT_ISO, 
    precision, NULL);
  lwgeom_free(geom);
  return result;
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return the Well-Known Text (WKT) representation of a
 * geometry/geography
 * @param[in] gs Geometry/geography
 * @param[in] precision Maximum number of decimal digits
 * @note PostGIS function: @p LWGEOM_asText(PG_FUNCTION_ARGS)
 */
inline char *
geo_as_text(const GSERIALIZED *gs, int precision)
{
  return geo_as_wkt(gs, precision, false);
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * geometry/geography
 * @param[in] gs Geometry/geography
 * @param[in] precision Maximum number of decimal digits
 * @note This is a a stricter version of #geom_in, where we refuse to
 * accept (HEX)WKB or EWKT.
 * @note PostGIS function: @p LWGEOM_asEWKT(PG_FUNCTION_ARGS)
 */
inline char *
geo_as_ewkt(const GSERIALIZED *gs, int precision)
{
  return geo_as_wkt(gs, precision, true);
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return a geometry from its hex-encoded ASCII Well-Known Binary
 * (HexEWKB) representation
 * @param[in] wkt WKT string
 * @note This is a a stricter version of #geom_in, where we refuse to
 * accept (HEX)WKB or EWKT.
 * @note PostGIS function: @p LWGEOM_from_text(PG_FUNCTION_ARGS)
 */
inline GSERIALIZED *
geom_from_hexewkb(const char *wkt)
{
  return geom_in(wkt, -1);
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return a geography from its hex-encoded ASCII Well-Known Binary
 * (HexEWKB) representation
 * @param[in] wkt WKT string
 * @note This is a a stricter version of #geog_in, where we refuse to
 * accept (HEX)WKB or EWKT.
 * @note PostGIS function: @p LWGEOM_from_text(PG_FUNCTION_ARGS)
 */
inline GSERIALIZED *
geog_from_hexewkb(const char *wkt)
{
  return geog_in(wkt, -1);
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return the hex-encoded ASCII Well-Known Binary (HexWKB)
 * representation of a geometry/geography
 * @param[in] gs Geometry/geography
 * @param[in] endian Endianness
 * @note PostGIS function: @p AsHEXEWKB(gs, string)
 */
char *
geo_as_hexewkb(const GSERIALIZED *gs, const char *endian)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, false);

  uint8_t variant = 0;
  /* If user specified endianness, respect it */
  if (endian)
  {
    if  (! strncmp(endian, "xdr", 3) || ! strncmp(endian, "XDR", 3))
      variant = variant | WKB_XDR;
    else
      variant = variant | WKB_NDR;
  }
  /* Create WKB hex string */
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  char *result = lwgeom_to_hexwkb_buffer(geom, variant | WKB_EXTENDED);
  lwgeom_free(geom);
  return result;
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return a geometry/geography from its EWKB representation
 * @details This function parses EWKB (extended form) which also contains SRID
 * info.
 * @param[in] wkb WKB bytes
 * @param[in] wkb_size Number of WKB bytes
 * @param[in] srid SRID
 * @note PostGIS function: @p LWGEOMFromEWKB(wkb, [SRID])
 * @note wkb is in *binary* not hex form
 */
GSERIALIZED *
geo_from_ewkb(const uint8_t *wkb, size_t wkb_size, int32 srid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb, NULL);

  LWGEOM *geom = lwgeom_from_wkb(wkb, wkb_size,
    LW_PARSER_CHECK_ALL);
  if (!geom)
  {
    meos_error(ERROR, MEOS_ERR_WKB_INPUT, "Unable to parse WKB string");
    return NULL;
  }

  if (srid > 0)
    lwgeom_set_srid(geom, srid);

  if (lwgeom_needs_bbox(geom))
    lwgeom_add_bbox(geom);

  GSERIALIZED *result = geo_serialize(geom);
  lwgeom_free(geom);
  return result;
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return the Extended Well-Known Binary (EWKB) representation of a
 * geometry/geography
 * @param[in] gs Geometry/geography
 * @param[in] endian Endianness
 * @param[in] size Size of result
 * @note PostGIS function: @p WKBFromLWGEOM(PG_FUNCTION_ARGS)
 */
uint8_t *
geo_as_ewkb(const GSERIALIZED *gs, const char *endian, size_t *size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, false);

  uint8_t variant = 0;

  /* If user specified endianness, respect it */
  if (endian)
  {
    if (! strncmp(endian, "xdr", 3) || ! strncmp(endian, "XDR", 3))
      variant = variant | WKB_XDR;
    else
      variant = variant | WKB_NDR;
  }

  /* Create WKB hex string */
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  lwvarlena_t *wkb = lwgeom_to_wkb_varlena(geom, variant | WKB_EXTENDED);

  size_t data_size = wkb->size - LWVARHDRSZ;
  uint8_t *result = palloc(data_size);
  memcpy(result, wkb->data, data_size);
  pfree(geom); pfree(wkb);
  *size = data_size;
  return result;
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return a geometry/geography from its GeoJSON representation
 * @param[in] geojson GeoJSON string
 * @note PostGIS function: @p geom_from_geojson(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geo_from_geojson(const char *geojson)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(geojson, NULL);

  char *srs = NULL;
  int32_t srid = WGS84_SRID;

  LWGEOM *geom = lwgeom_from_geojson(geojson, &srs);
  if (!geom)
  {
    /* Shouldn't get here */
    meos_error(ERROR, MEOS_ERR_GEOJSON_INPUT,
      "lwgeom_from_geojson returned NULL");
    return NULL;
  }

  // if (srs)
  // {
    // srid = GetSRIDCacheBySRS(fcinfo, srs);
    // lwfree(srs);
  // }

  lwgeom_set_srid(geom, srid);
  GSERIALIZED *result = geo_serialize(geom);
  lwgeom_free(geom);
  return result;
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return the GeoJSON representation of a geometry/geography
 * @param[in] gs Geometry/geography
 * @param[in] option Option
 * @param[in] precision Maximum number of decimal digits
 * @param[in] srs Spatial reference system
 * @note PostGIS function: @p LWGEOM_asGeoJson(PG_FUNCTION_ARGS)
 */
char *
geo_as_geojson(const GSERIALIZED *gs, int option, int precision,
  const char *srs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, false);

  // int precision = OUT_DEFAULT_DECIMAL_DIGITS;
  int output_bbox = LW_FALSE;
  // int output_long_crs = LW_FALSE;
  // int output_short_crs = LW_FALSE;
  // int output_guess_short_srid = LW_FALSE;
  // const char *srs = NULL;

  // int32_t srid = gserialized_get_srid(gs);

  /* Retrieve output option
   * 0 = without option
   * 1 = bbox
   * 2 = short crs
   * 4 = long crs
   * 8 = guess if CRS is needed (default)
   */
  // output_guess_short_srid = (option & 8) ? LW_TRUE : LW_FALSE;
  // output_short_crs = (option & 2) ? LW_TRUE : LW_FALSE;
  // output_long_crs = (option & 4) ? LW_TRUE : LW_FALSE;
  output_bbox = (option & 1) ? LW_TRUE : LW_FALSE;

  // if (output_guess_short_srid && srid != WGS84_SRID && srid != SRID_UNKNOWN)
    // output_short_crs = LW_TRUE;

  // if (srid != SRID_UNKNOWN && (output_short_crs || output_long_crs))
  // {
    // srs = GetSRSCacheBySRID(fcinfo, srid, !output_long_crs);

    // if (!srs)
    // {
      // meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          // "SRID %i unknown in spatial_ref_sys table", srid);
      // return NULL;
    // }
  // }

  LWGEOM *geom = lwgeom_from_gserialized(gs);
  lwvarlena_t *txt = lwgeom_to_geojson(geom, srs, precision, output_bbox);
  char *result = pstrdup(VARDATA(txt));
  lwgeom_free(geom); pfree(txt);
  return result;
}

/**
 * @ingroup meos_geo_base_comp
 * @brief Return true if the geometries/geographies are the same
 * @param[in] gs1,gs2 Geometries/geographies
 */
bool
geo_same(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs1, false); VALIDATE_NOT_NULL(gs2, false);

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  char result = lwgeom_same(geom1, geom2);
  lwgeom_free(geom1); lwgeom_free(geom2);
  return (result == LW_TRUE);
}

/*****************************************************************************
 * Functions adapted from geography_inout.c
 *****************************************************************************/

/**
 * @brief Ensure that the geography type is valid
 * The geography type only support POINT, LINESTRING, POLYGON, MULTI* variants
 * of same, and GEOMETRYCOLLECTION. If the input type is not one of those, shut
 * down the query.
 */
void
geography_valid_type(uint8_t type)
{
  if (! (type == POINTTYPE || type == LINETYPE || type == POLYGONTYPE ||
          type == MULTIPOINTTYPE || type == MULTILINETYPE ||
          type == MULTIPOLYGONTYPE || type == COLLECTIONTYPE) )
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Geography type does not support %s", lwtype_name(type));
  return;
}

/**
 * @brief Return a geography from a LWGEOM
 * @note Function derived from
 *   GSERIALIZED* gserialized_geography_from_lwgeom(LWGEOM *lwgeom,
 *   int32 geog_typmod)
 */
GSERIALIZED *
geog_from_lwgeom(LWGEOM *lwgeom, int32 typmod)
{
  GSERIALIZED *result = NULL;

  /* Set geodetic flag */
  lwgeom_set_geodetic(lwgeom, true);

  /* Check that this is a type we can handle */
  geography_valid_type(lwgeom->type);

  /* Force the geometry to have valid geodetic coordinate range. */
  lwgeom_nudge_geodetic(lwgeom);
  /* Contrary to PostGIS, we do not issue a warning if the coordinate values
   * were coerced into the range [-180 -90, 180 90] for geography */
  lwgeom_force_geodetic(lwgeom);

  /* Force default SRID to the default */
  if ((int) lwgeom->srid <= 0)
    lwgeom->srid = SRID_DEFAULT;

  /*
   * Serialize our lwgeom and set the geodetic flag so subsequent
   * functions do the right thing.
  */
  result = geog_serialize(lwgeom);

  /* Check for typmod agreement */
  if (typmod >= 0)
    result = postgis_valid_typmod(result, typmod);

  return result;
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return a geography from its Well-Known Text or Binary (WKT or Binary)
 * representation
 * @param[in] str String
 * @param[in] typmod Typmod
 * @note PostGIS function: @p geography_in(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geog_in(const char *str, int32 typmod)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);

  LWGEOM_PARSER_RESULT lwg_parser_result;
  LWGEOM *lwgeom = NULL;

  lwgeom_parser_result_init(&lwg_parser_result);

  /* Empty string. */
  if ( str[0] == '\0' )
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT, "parse error - invalid geography");
    return NULL;
  }

  /* WKB? Let's find out. */
  if ( str[0] == '0' )
  {
    /* TODO: 20101206: No parser checks! This is inline with current 1.5 behavior,
     * but needs discussion */
    lwgeom = lwgeom_from_hexwkb(str, LW_PARSER_CHECK_NONE);
    /* Error out if something went sideways */
    if (!  lwgeom )
    {
      meos_error(ERROR, MEOS_ERR_TEXT_INPUT, "parse error - invalid geometry");
      return NULL;
    }
  }
  /* WKT then. */
  else
  {
    if ( lwgeom_parse_wkt(&lwg_parser_result, (char *) str, 
        LW_PARSER_CHECK_ALL) == LW_FAILURE )
      PG_PARSER_ERROR(lwg_parser_result);
      lwgeom = lwg_parser_result.geom;
  }

  GSERIALIZED *result = NULL;
  /* Error on any SRID != default */
#if MEOS 
  /* TODO Determine whether we can reuse PostGIS cache */
  if (lwgeom->srid == SRID_UNKNOWN || ensure_srid_is_latlong(lwgeom->srid))
    /* Convert to gserialized */
    result = geog_from_lwgeom(lwgeom, typmod);
#else
  /* Use PostGIS cache, throw an error if not geodetic */
  srid_check_latlong(lwgeom->srid);
  /* Convert to gserialized */
  result = geog_from_lwgeom(lwgeom, typmod);
#endif /* MEOS */

  /* Clean up and return */
  lwgeom_free(lwgeom);
  return result;
}

#if MEOS
/**
 * @ingroup meos_geo_base_inout
 * @brief Return a geography from its binary representation
 * @param[in] wkb_bytea Byte striing
 * geography_from_binary(*char) returns *GSERIALIZED
 */
GSERIALIZED *
geog_from_binary(const char *wkb_bytea)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb_bytea, NULL);

  size_t wkb_size = VARSIZE(wkb_bytea);
  uint8_t *wkb = (uint8_t *) VARDATA(wkb_bytea);
  LWGEOM *geom = lwgeom_from_wkb(wkb, wkb_size, LW_PARSER_CHECK_NONE);

  if (! geom)
  {
    meos_error(ERROR, MEOS_ERR_WKB_INPUT, "Unable to parse WKB string");
    return NULL;
  }

  GSERIALIZED *result = NULL;
  /* Error on any SRID != default */
  if (ensure_srid_is_latlong(geom->srid))
    result = geog_serialize(geom);

  lwgeom_free(geom);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup meos_geo_base_conversion
 * @brief Return a geography from a geometry
 * @param[in] gs Geometry
 * @note PostGIS function: @p geography_from_geometry(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geog_from_geom(const GSERIALIZED *gs)
{
  VALIDATE_NOT_NULL(gs, NULL);
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  geography_valid_type(lwgeom_get_type(lwgeom));
  /* Force default SRID */
  if ((int) lwgeom->srid <= 0)
    lwgeom->srid = SRID_DEFAULT;

  GSERIALIZED *result = NULL;
  /* Error on any SRID != default */
  if (ensure_srid_is_latlong(lwgeom->srid))
  {
    /* Force the geometry to have valid geodetic coordinate range. */
    lwgeom_nudge_geodetic(lwgeom);
    /* Contrary to PostGIS, we do not issue a warning if the coordinate values
     * were coerced into the range [-180 -90, 180 90] for geography */
    lwgeom_force_geodetic(lwgeom);

    /* force recalculate of box by dropping */
    lwgeom_drop_bbox(lwgeom);

    lwgeom_set_geodetic(lwgeom, true);
    /* We are trusting geography_serialize will add a box if needed */
    result = geo_serialize(lwgeom);
  }
  lwgeom_free(lwgeom);
  return result;
}

/**
 * @ingroup meos_geo_base_conversion
 * @brief Return a geometry from a geography
 * @param[in] gs Geography
 * @note PostGIS function: @p geometry_from_geography(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geom_from_geog(const GSERIALIZED *gs)
{
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  /* Recalculate the boxes after re-setting the geodetic bit */
  lwgeom_set_geodetic(lwgeom, false);
  lwgeom_refresh_bbox(lwgeom);
  /* We want "geometry" to think all our "geography" has an SRID, and the
     implied SRID is the default, so we fill that in if our SRID is actually
     unknown. */
  if (lwgeom->srid <= 0)
    lwgeom->srid = SRID_DEFAULT;

  GSERIALIZED *result = geom_serialize(lwgeom);
  lwgeom_free(lwgeom);
  return result;
}

/*****************************************************************************
 * Functions adapted from lwgeom_functions_analytic.c
 *****************************************************************************/

/**
 * @brief Interpolate a point from a line
 * @pre The argument @p fraction is in [0,1] and the type of the geometry is
 * @p LINETYPE
 * @note PostGIS function: @p LWGEOM_line_interpolate_point(PG_FUNCTION_ARGS)
 */
LWGEOM *
lwgeom_line_interpolate_point(LWGEOM *lwgeom, double fraction, int32_t srid,
  char repeat)
{
  assert(fraction >= 0 && fraction <= 1);
  assert(lwgeom->type == LINETYPE);
  LWLINE *lwline = lwgeom_as_lwline(lwgeom);
  POINTARRAY *opa = lwline_interpolate_points(lwline, fraction, repeat);
  LWGEOM *result;
  if (opa->npoints <= 1)
    result = lwpoint_as_lwgeom(lwpoint_construct(srid, NULL, opa));
  else
    result = lwmpoint_as_lwgeom(lwmpoint_construct(srid, opa));
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Interpolate one or several points from a line
 * @param[in] gs Geometry
 * @param[in] fraction Value in [0,1] representing the distance where the point
 * is located
 * @param[in] repeat True when obtaining several points 
 * @note PostGIS function: @p LWGEOM_line_interpolate_point(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
line_interpolate_point(GSERIALIZED *gs, double fraction, bool repeat)
{
  if (fraction < 0 || fraction > 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Second argument is not within [0,1]");
    return NULL;
  }
  if (gserialized_get_type(gs) != LINETYPE)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "First argument is not a line");
    return NULL;
  }

  LWLINE *lwline = lwgeom_as_lwline(lwgeom_from_gserialized(gs));
  POINTARRAY *opa = lwline_interpolate_points(lwline, fraction, repeat);

  lwgeom_free(lwline_as_lwgeom(lwline));

  LWGEOM *lwresult;
  int32_t srid = gserialized_get_srid(gs);
  if (opa->npoints <= 1)
    lwresult = lwpoint_as_lwgeom(lwpoint_construct(srid, NULL, opa));
  else
    lwresult = lwmpoint_as_lwgeom(lwmpoint_construct(srid, opa));

  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult);
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @param[in] gs Geometry
 * @param[in] from,to Values in [0,1] representing the fractional locations 
 * where the subline starts and ends
 * @brief Return a subline from a line
 * @note PostGIS function: @p LWGEOM_line_substring(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
line_substring(const GSERIALIZED *gs, double from, double to)
{
  if (from < 0 || from > 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Second argument is not within [0,1]");
    return NULL;
  }
  if (to < 0 || to > 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Third argument is not within [0,1]");
    return NULL;
  }
  if (from > to)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Second argument must be smaller then the third one");
    return NULL;
  }

  LWGEOM *olwgeom;
  POINTARRAY *opa;
  uint8_t type = (uint8_t) gserialized_get_type(gs);
  if (type == LINETYPE)
  {
    LWLINE *iline = lwgeom_as_lwline(lwgeom_from_gserialized(gs));
    if (lwgeom_is_empty((LWGEOM *) iline))
    {
      /* TODO return empty line */
      lwline_release(iline);
      return NULL;
    }

    POINTARRAY *ipa = iline->points;
    opa = ptarray_substring(ipa, from, to, 0);
    if (opa->npoints == 1) /* Point returned */
      olwgeom = (LWGEOM *)lwpoint_construct(iline->srid, NULL, opa);
    else
      olwgeom = (LWGEOM *)lwline_construct(iline->srid, NULL, opa);
  }
  else if (type == MULTILINETYPE)
  {
    uint32_t i = 0, g = 0;
    int homogeneous = LW_TRUE;
    double length = 0.0, sublength = 0.0, maxprop = 0.0;
    LWMLINE *iline = lwgeom_as_lwmline(lwgeom_from_gserialized(gs));
    if ( lwgeom_is_empty((LWGEOM*)iline) )
    {
      /* TODO return empty collection */
      lwmline_release(iline);
      return NULL;
    }

    /* Calculate the total length of the mline */
    for (i = 0; i < iline->ngeoms; i++)
    {
      LWLINE *subline = (LWLINE*)iline->geoms[i];
      if ( subline->points && subline->points->npoints > 1 )
        length += ptarray_length_2d(subline->points);
    }

    LWGEOM **geoms = lwalloc(sizeof(LWGEOM*) * iline->ngeoms);

    /* Slice each sub-geometry of the multiline */
    for ( i = 0; i < iline->ngeoms; i++ )
    {
      LWLINE *subline = (LWLINE*)iline->geoms[i];
      double subfrom = 0.0, subto = 0.0;

      if ( subline->points && subline->points->npoints > 1 )
        sublength += ptarray_length_2d(subline->points);

      /* Calculate proportions for this subline */
      double minprop = 0.0;
      minprop = maxprop;
      maxprop = sublength / length;

      /* This subline doesn't reach the lowest proportion requested
         or is beyond the highest proporton */
      if ( from > maxprop || to < minprop )
        continue;

      if (from <= minprop)
        subfrom = 0.0;
      if (to >= maxprop)
        subto = 1.0;

      if (from > minprop && from <= maxprop)
        subfrom = (from - minprop) / (maxprop - minprop);

      if (to < maxprop && to >= minprop)
        subto = (to - minprop) / (maxprop - minprop);

      opa = ptarray_substring(subline->points, subfrom, subto, 0);
      if (opa && opa->npoints > 0)
      {
        if (opa->npoints == 1) /* Point returned */
        {
          geoms[g] = (LWGEOM *)lwpoint_construct(SRID_UNKNOWN, NULL, opa);
          homogeneous = LW_FALSE;
        }
        else
        {
          geoms[g] = (LWGEOM *)lwline_construct(SRID_UNKNOWN, NULL, opa);
        }
        g++;
      }

    }
    /* If we got any points, we need to return a GEOMETRYCOLLECTION */
    if (!  homogeneous )
      type = COLLECTIONTYPE;

    olwgeom = (LWGEOM *) lwcollection_construct(type, iline->srid, NULL, g, geoms);
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "First argument is not a line");
    return NULL;
  }

  GSERIALIZED *result = geo_serialize(olwgeom);
  lwgeom_free(olwgeom);
  return result;
}

/*****************************************************************************
 * Functions adapted from lwgeom_lrs.c
 *****************************************************************************/

/**
 * @ingroup meos_geo_base_spatial
 * @brief Locate a point into a line
 * @param[in] gs1 Line
 * @param[in] gs2 Point
 * @return On error return -1.0
 */
double
line_locate_point(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  LWLINE *lwline;
  LWPOINT *lwpoint;
  POINTARRAY *pa;
  POINT4D p, p_proj;
  double ret;

  if ( gserialized_get_type(gs1) != LINETYPE )
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "First argument is not a line");
    return -1.0;
  }
  if ( gserialized_get_type(gs2) != POINTTYPE )
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Second argument is not a point");
    return -1.0;
  }

  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));

  lwline = lwgeom_as_lwline(lwgeom_from_gserialized(gs1));
  lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs2));

  pa = lwline->points;
  lwpoint_getPoint4d_p(lwpoint, &p);

  ret = ptarray_locate_point(pa, &p, NULL, &p_proj);

  return ret;
}

/*****************************************************************************
 * Functions adapted from lwgeom_ogc.c
 *****************************************************************************/

/**
 * @brief Global constant array containing the geometry type strings
 */
static const char * _GEO_TYPENAME[] =
{
  "Unknown",
  "Point",
  "LineString",
  "Polygon",
  "MultiPoint",
  "MultiLineString",
  "MultiPolygon",
  "GeometryCollection",
  "CircularString",
  "CompoundCurve",
  "CurvePolygon",
  "MultiCurve",
  "MultiSurface",
  "PolyhedralSurface",
  "Triangle",
  "Tin",
};

/**
 * @ingroup meos_geo_base_accessor
 * @brief Return a string representation of a geometry's type
 */
const char *
geo_typename(int type)
{
  /* NUMTYPES is defined in liblwgeom.h */
  if (type < 0 || type >= NUMTYPES)
    return "";
  return _GEO_TYPENAME[type];
}

/**
 * @ingroup meos_geo_base_accessor
 * @brief Return the n-th point of a line
 * @param[in] gs Geometry
 * @param[in] n Number (1-based)
 * @return  Return @p NULL if there is no LINESTRING(..) in GEOMETRY or @p n
 * is out of bounds.
 */
GSERIALIZED *
line_point_n(const GSERIALIZED *gs, int n)
{
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  LWPOINT *point = NULL;
  int type = geom->type;

  /* If index is negative, count backward */
  if (n < 1)
  {
    int count = -1;
    if ( type == LINETYPE || type == CIRCSTRINGTYPE || type == COMPOUNDTYPE )
      count = lwgeom_count_vertices(geom);
    if (count > 0)
    {
      /* only work if we found the total point number */
      /* converting nf to positive backward indexing, +1 because 1 indexing */
      n = n + count + 1;
    }
    if (n < 1)
      return NULL;
  }

  if (type == LINETYPE || type == CIRCSTRINGTYPE)
  {
    /* OGC index starts at one, so we substract first. */
    point = lwline_get_lwpoint((LWLINE*) geom, n - 1);
  }
  else if (type == COMPOUNDTYPE)
  {
    point = lwcompound_get_lwpoint((LWCOMPOUND*) geom, n - 1);
  }

  lwgeom_free(geom);
  if (! point)
    return NULL;
  return geo_serialize(lwpoint_as_lwgeom(point));
}

/**
 * @ingroup meos_geo_base_accessor
 * @brief Return the number of points of a line
 * @param[in] gs Geometry 
 * @return On error return -1.0
*/
int
line_numpoints(const GSERIALIZED *gs)
{
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  int count = -1;
  int type = geom->type;
  if (type == LINETYPE || type == CIRCSTRINGTYPE || type == COMPOUNDTYPE)
    count = lwgeom_count_vertices(geom);
  lwgeom_free(geom);
  /* OGC says this functions is only valid on LINESTRING */
  if (count < 0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "Error in computing number of points of a linestring");
    return -1;
  }
  return count;
}

/*****************************************************************************/
