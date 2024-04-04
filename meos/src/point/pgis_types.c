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
 * @brief Functions for geometry types corresponding to external PostGIS
 * functions in order to bypass the function manager in @p fmgr.c
 */

#include "point/pgis_types.h"

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
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"

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

extern GSERIALIZED *geography_from_lwgeom(LWGEOM *geom, int32 typmod);

/*****************************************************************************/

#if MEOS
/**
 * @brief Return the srid of a geometry
 * @note PostGIS function: @p gserialized_get_srid(const GSERIALIZED *g).
 */
int32
geo_get_srid(const GSERIALIZED *g)
{
  return gserialized_get_srid(g);
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
box2d_to_lwgeom(GBOX *box, int srid)
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

/* The boundary function has changed its implementation in version 3.2.
 * This is the version in 3.2.3 */
LWGEOM *
lwgeom_boundary(LWGEOM *lwgeom)
{
  int32_t srid = lwgeom_get_srid(lwgeom);
  uint8_t hasz = lwgeom_has_z(lwgeom);
  uint8_t hasm = lwgeom_has_m(lwgeom);

  switch (lwgeom->type)
  {
  case POINTTYPE:
  case MULTIPOINTTYPE: {
    return lwgeom_construct_empty(lwgeom->type, srid, hasz, hasm);
  }
  case LINETYPE:
  case CIRCSTRINGTYPE: {
    if (lwgeom_is_closed(lwgeom) || lwgeom_is_empty(lwgeom))
      return (LWGEOM *)lwmpoint_construct_empty(srid, hasz, hasm);
    else
    {
      LWLINE *lwline = (LWLINE *)lwgeom;
      LWMPOINT *lwmpoint = lwmpoint_construct_empty(srid, hasz, hasm);
      POINT4D pt;
      getPoint4d_p(lwline->points, 0, &pt);
      lwmpoint_add_lwpoint(lwmpoint, lwpoint_make(srid, hasz, hasm, &pt));
      getPoint4d_p(lwline->points, lwline->points->npoints - 1, &pt);
      lwmpoint_add_lwpoint(lwmpoint, lwpoint_make(srid, hasz, hasm, &pt));

      return (LWGEOM *)lwmpoint;
    }
  }
  case MULTILINETYPE:
  case MULTICURVETYPE: {
    LWMLINE *lwmline = (LWMLINE *)lwgeom;
    POINT4D *out = lwalloc(sizeof(POINT4D) * lwmline->ngeoms * 2);
    uint32_t n = 0;

    for (uint32_t i = 0; i < lwmline->ngeoms; i++)
    {
      LWMPOINT *points = lwgeom_as_lwmpoint(lwgeom_boundary((LWGEOM *)lwmline->geoms[i]));
      if (!points)
        continue;

      for (uint32_t k = 0; k < points->ngeoms; k++)
      {
        POINT4D pt = getPoint4d(points->geoms[k]->point, 0);

        uint8_t seen = LW_FALSE;
        for (uint32_t j = 0; j < n; j++)
        {
          if (memcmp(&(out[j]), &pt, sizeof(POINT4D)) == 0)
          {
            seen = LW_TRUE;
            out[j] = out[--n];
            break;
          }
        }
        if (!seen)
          out[n++] = pt;
      }

      lwgeom_free((LWGEOM *)points);
    }

    LWMPOINT *lwmpoint = lwmpoint_construct_empty(srid, hasz, hasm);

    for (uint32_t i = 0; i < n; i++)
      lwmpoint_add_lwpoint(lwmpoint, lwpoint_make(srid, hasz, hasm, &(out[i])));

    lwfree(out);

    return (LWGEOM *)lwmpoint;
  }
  case TRIANGLETYPE: {
    LWTRIANGLE *lwtriangle = (LWTRIANGLE *)lwgeom;
    POINTARRAY *points = ptarray_clone_deep(lwtriangle->points);
    return (LWGEOM *)lwline_construct(srid, 0, points);
  }
  case POLYGONTYPE: {
    LWPOLY *lwpoly = (LWPOLY *)lwgeom;

    LWMLINE *lwmline = lwmline_construct_empty(srid, hasz, hasm);
    for (uint32_t i = 0; i < lwpoly->nrings; i++)
    {
      POINTARRAY *ring = ptarray_clone_deep(lwpoly->rings[i]);
      lwmline_add_lwline(lwmline, lwline_construct(srid, 0, ring));
    }

    /* Homogenize the multilinestring to hopefully get a single LINESTRING */
    LWGEOM *lwout = lwgeom_homogenize((LWGEOM *)lwmline);
    lwgeom_free((LWGEOM *)lwmline);
    return lwout;
  }
  case CURVEPOLYTYPE: {
    LWCURVEPOLY *lwcurvepoly = (LWCURVEPOLY *)lwgeom;
    LWCOLLECTION *lwcol = lwcollection_construct_empty(MULTICURVETYPE, srid, hasz, hasm);

    for (uint32_t i = 0; i < lwcurvepoly->nrings; i++)
      lwcol = lwcollection_add_lwgeom(lwcol, lwgeom_clone_deep(lwcurvepoly->rings[i]));

    return (LWGEOM *)lwcol;
  }
  case MULTIPOLYGONTYPE:
  case COLLECTIONTYPE:
  case TINTYPE: {
    LWCOLLECTION *lwcol = (LWCOLLECTION *)lwgeom;
    LWCOLLECTION *lwcol_boundary = lwcollection_construct_empty(COLLECTIONTYPE, srid, hasz, hasm);

    for (uint32_t i = 0; i < lwcol->ngeoms; i++)
      lwcollection_add_lwgeom(lwcol_boundary, lwgeom_boundary(lwcol->geoms[i]));

    LWGEOM *lwout = lwgeom_homogenize((LWGEOM *)lwcol_boundary);
    lwgeom_free((LWGEOM *)lwcol_boundary);

    return lwout;
  }
  default:
    lwerror("%s: unsupported geometry type: %s", __func__, lwtype_name(lwgeom->type));
    return NULL;
  }
}

/**
 * @brief Return the boundary of a geometry
 * @note PostGIS function: @p boundary(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geometry_boundary(const GSERIALIZED *gs)
{
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
 * @brief Return the shortest 2d line between two geometries
 * @note PostGIS function: @p LWGEOM_shortestline2d(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geo_shortestline2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
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
 * @brief Return the shortest line between two 3D geometries
 * @note PostGIS function: @p LWGEOM_shortestline3d(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geometry_shortestline3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
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
 * @brief Return the distance between two geometries
 * @note PostGIS function: @p ST_Distance(PG_FUNCTION_ARGS)
 */
double
geo_distance(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
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
 * @brief Return the 3D distance between two geometries
 * @note PostGIS function: @p ST_3DDistance(PG_FUNCTION_ARGS)
 */
double
geometry_3Ddistance(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
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
 * @brief Return true if the 3D geometries intersect
 * @note PostGIS function: @p ST_3DIntersects(PG_FUNCTION_ARGS)
 */
bool
geometry_3Dintersects(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  double mindist = lwgeom_mindistance3d_tolerance(geom1, geom2, 0.0);
  /* empty geometries cases should be right handled since return from
     underlying functions should be FLT_MAX which causes false as answer */
  return (mindist == 0.0);
}

/**
 * @brief Return true if two geometries are within a distance
 * @note PostGIS function: @p LWGEOM_dwithin(PG_FUNCTION_ARGS)
 */
bool
geometry_dwithin2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  double tolerance)
{
  if (! ensure_positive_datum(Float8GetDatum(tolerance), T_FLOAT8) ||
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
 * @brief Return true if two geometries are within a distance
 * @note PostGIS function: @p LWGEOM_dwithin3d(PG_FUNCTION_ARGS)
 */
bool
geometry_dwithin3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  double tolerance)
{
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
 * @brief  Reverse vertex order of geometry
 * @note PostGIS function: @p LWGEOM_reverse(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geo_reverse(const GSERIALIZED *gs)
{
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  lwgeom_reverse_in_place(geom);
  return geo_serialize(geom);
}

/**
 * @brief Return the last argument initialized with the azimuth of a segment
 * defined by two points
 * @return Return false on exception (same point)
 */
bool
gserialized_azimuth(GSERIALIZED *gs1, GSERIALIZED *gs2, double *result)
{
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

/*****************************************************************************
 * Functions adapted from lwgeom_geos.c
 *****************************************************************************/

static char
gserialized_is_point(const GSERIALIZED* gs)
{
  int type = gserialized_get_type(gs);
  return (type == POINTTYPE || type == MULTIPOINTTYPE);
}

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
 * @brief Return true if two geometries intersect or the first contains
 * the other, where the function called depend on the third argument
 * @param[in] gs1,gs2 Geometries
 * @param[in] rel Spatial relationship
 * @note PostGIS functions: @p ST_Intersects(PG_FUNCTION_ARGS),
 * @p contains(PG_FUNCTION_ARGS), @p touches(PG_FUNCTION_ARGS)
 */
bool
geometry_spatialrel(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  spatialRel rel)
{
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));

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
  assert(rel == INTERSECTS || rel == CONTAINS || rel == TOUCHES);
  switch (rel)
  {
    case INTERSECTS:
      return (bool) meos_call_geos2(gs1, gs2, &GEOSIntersects);
    case CONTAINS:
      return (bool) meos_call_geos2(gs1, gs2, &GEOSContains);
    case TOUCHES:
      return (bool) meos_call_geos2(gs1, gs2, &GEOSTouches);
    default:
      /* keep compiler quiet */
      return false;
  }
}

/**
 * @brief Return true if two geometries satisfy a spatial relationship given
 * by a pattern
 * @note PostGIS function: @p relate_pattern(PG_FUNCTION_ARGS)
 */
bool
geo_relate_pattern(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  char *patt)
{
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
 * @brief Return the intersection of two geometries
 * @note PostGIS function: @p ST_Intersection(PG_FUNCTION_ARGS). With respect
 * to the original function we do not use the @p prec argument.
 */
GSERIALIZED *
geometry_intersection(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  GSERIALIZED *result;
  LWGEOM *geom1, *geom2, *lwresult;
  double prec = -1;
  geom1 = lwgeom_from_gserialized(gs1);
  geom2 = lwgeom_from_gserialized(gs2);
  lwresult = lwgeom_intersection_prec(geom1, geom2, prec);
  result = geo_serialize(lwresult);
  lwgeom_free(geom1); lwgeom_free(geom2); lwgeom_free(lwresult);
  return result;
}

/**
 * @brief Return the union of an array of geometries
 * @details The funciton will iteratively call @p GEOSUnion on the
 * GEOS-converted versions of them and return PGIS-converted version back.
 * Changing the combination order *might* speed up performance.
 * @note PostGIS function: @p pgis_union_geometry_array(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geometry_array_union(GSERIALIZED **gsarr, int nelems)
{
  assert(nelems > 0);

  /* One geom geom? Return it */
  if (nelems == 1)
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
  GEOSGeometry **geoms = palloc(sizeof(GEOSGeometry *) * nelems);

  /*
  ** We need to convert the array of GSERIALIZED into a GEOS collection.
  ** First make an array of GEOS geometries.
  */
  for (int i = 0; i < nelems; i++)
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
 * @brief Return the convex hull of the geometry
 * @note PostGIS function: @p ST_ConvexHull(PG_FUNCTION_ARGS). With respect to
 * the original function we do not use the @p prec argument.
 */
GSERIALIZED *
geometry_convex_hull(const GSERIALIZED *gs)
{
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
 * @brief Return true if the coordinates of the geometry are finite
 */
int
lwgeom_isfinite(const LWGEOM *lwgeom)
{
  LWPOINTITERATOR* it = lwpointiterator_create(lwgeom);
  int hasz = lwgeom_has_z(lwgeom);
  int hasm = lwgeom_has_m(lwgeom);

  while (lwpointiterator_has_next(it))
  {
    POINT4D p;
    lwpointiterator_next(it, &p);
    int finite = isfinite(p.x) && isfinite(p.y) &&
      (hasz ? isfinite(p.z) : 1) && (hasm ? isfinite(p.m) : 1);

    if (!finite)
    {
      lwpointiterator_destroy(it);
      return LW_FALSE;
    }
  }
  lwpointiterator_destroy(it);
  return LW_TRUE;
}

/**
 * @brief Return a @p POLYGON or a @p MULTIPOLYGON that represents all points
 * whose distance from a geometry/geography is less than or equal to a given
 * distance
 * @param[in] gs Geometry
 * @param[in] size Distance
 * @param[in] params Buffer style parameters
 * @note PostGIS function: @p ST_Buffer(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geometry_buffer(const GSERIALIZED *gs, double size, char *params)
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
 * Functions adapted from geography_measurement.c
 *****************************************************************************/

/**
 * @brief Return double length in meters
 * @return On error return @p DBL_MAX
 * @note PostGIS function: @p geography_length(PG_FUNCTION_ARGS)
 */
double
pgis_geography_length(GSERIALIZED *gs, bool use_spheroid)
{
  /* EMPTY things have no length */
  int32 geo_type = gserialized_get_type(gs);
  if (gserialized_is_empty(gs) || geo_type == POLYGONTYPE ||
    geo_type == MULTIPOLYGONTYPE)
    return 0.0;

  /* Get our geometry object loaded into memory. */
  LWGEOM *geom = lwgeom_from_gserialized(gs);

  /* Initialize spheroid */
  /* We currently cannot use the following statement since PROJ4 API is not
   * available directly to MobilityDB. */
  // spheroid_init_from_srid(gserialized_get_srid(gs), &s);
  SPHEROID s;
  spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

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
 * @brief Return true if two geographies are within a distance
 * @note PostGIS function: @p geography_dwithin_uncached(PG_FUNCTION_ARGS)
 * where we use the WGS84 spheroid
 */
bool
pgis_geography_dwithin(GSERIALIZED *gs1, GSERIALIZED *gs2, double tolerance,
  bool use_spheroid)
{
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));

  /* Return FALSE on empty arguments. */
  if (gserialized_is_empty(gs1) || gserialized_is_empty(gs2))
    return false;

  /* Initialize spheroid */
  /* We currently cannot use the following statement since PROJ4 API is not
   * available directly to MobilityDB. */
  // spheroid_init_from_srid(gserialized_get_srid(gs1), &s);
  SPHEROID s;
  spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

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

/* Defined in liblwgeom_internal.h */
#define PGIS_FP_TOLERANCE 1e-12

/**
 * @brief Return the distance between two geographies
 * @note PostGIS function: @p geography_distance_uncached(PG_FUNCTION_ARGS).
 * We set by default both @p tolerance and @p use_spheroid and initialize the
 * spheroid to WGS84
 * @note Errors return -1 to replace return @p NULL
 */
double
pgis_geography_distance(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs1));

  /* Return NULL on empty arguments. */
  if (gserialized_is_empty(gs1) || gserialized_is_empty(gs2) )
    return -1;

  double tolerance = PGIS_FP_TOLERANCE;
  bool use_spheroid = true;

  /* Initialize spheroid */
  /* We currently cannot use the following statement since PROJ4 API is not
   * available directly to MobilityDB. */
  // spheroid_init_from_srid(gserialized_get_srid(g), &s);
  SPHEROID s;
  spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

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
* Check the consistency of the metadata we want to enforce in the typmod:
* srid, type and dimensionality. If things are inconsistent, shut down the query.
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
 * @ingroup meos_pgis_types
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
pgis_geometry_in(char *str, int32 typmod)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;

  char *str1 = str;
  LWGEOM_PARSER_RESULT lwg_parser_result;
  LWGEOM *lwgeom;
  GSERIALIZED *result;
  int32_t srid = 0;

  lwgeom_parser_result_init(&lwg_parser_result);

  /* Empty string. */
  if ( str1[0] == '\0' ) {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT, "parse error - invalid geometry");
    return NULL;
  }

  /* Starts with "SRID=" */
  if (pg_strncasecmp(str1, "SRID=", 5) == 0)
  {
    /* Roll forward to semi-colon */
    char *tmp = str1;
    while (tmp && *tmp != ';')
      tmp++;

    /* Check next character to see if we have WKB  */
    if (tmp && *(tmp+1) == '0')
    {
      /* Null terminate the SRID= string */
      *tmp = '\0';
      /* Set str1 to the start of the real WKB */
      str1 = tmp + 1;
      /* Move tmp to the start of the numeric part */
      tmp = str + 5;
      /* Parse out the SRID number */
      srid = atoi(tmp);
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
    if ( lwgeom_parse_wkt(&lwg_parser_result, str1, LW_PARSER_CHECK_ALL) == LW_FAILURE )
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
 * @ingroup meos_pgis_types
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs))
    return NULL;

  LWGEOM *geom = lwgeom_from_gserialized(gs);
  char *result = lwgeom_to_hexwkb_buffer(geom, WKB_EXTENDED);
  lwgeom_free(geom);
  return result;
}

#if MEOS || DEBUG_BUILD
/**
 * @brief Return a geometry/geography from its WKT representation (and
 * optionally a SRID)
 * @param[in] wkt WKT string
 * @param[in] srid SRID
 * @param[in] geography True if it is a geometry
 */
static GSERIALIZED *
geo_from_text(char *wkt, int srid, bool geography)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) wkt))
    return NULL;

  LWGEOM_PARSER_RESULT lwg_parser_result;
  GSERIALIZED *geo_result = NULL;
  LWGEOM *lwgeom;

  if (lwgeom_parse_wkt(&lwg_parser_result, wkt, LW_PARSER_CHECK_ALL) == LW_FAILURE )
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

  geo_result = geography ?
    geography_from_lwgeom(lwgeom, -1) : geo_serialize(lwgeom);
  /* Clean up */
  lwgeom_free(lwg_parser_result.geom);
  lwgeom_parser_result_free(&lwg_parser_result);
  pfree(wkt);

  return geo_result;
}

/**
 * @ingroup meos_pgis_types
 * @brief Return a geometry from its Well-Known Text (WKT) representation
 * @param[in] wkt WKT string
 * @param[in] srid SRID
 * @note This is a a stricter version of #pgis_geometry_in, where we refuse to
 * accept (HEX)WKB or EWKT.
 * @note PostGIS function: @p LWGEOM_from_text(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geometry_from_text(char *wkt, int srid)
{
  return geo_from_text(wkt, srid, false);
}

/**
 * @ingroup meos_pgis_types
 * @brief Return a geography from its Well-Known Text (WKT) representation
 * @note This is a a stricter version of #pgis_geography_in, where we refuse to
 * accept (HEX)WKB or EWKT.
 * @param[in] wkt WKT string
 * @param[in] srid SRID
 * @note PostGIS function: @p geography_from_text(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geography_from_text(char *wkt, int srid)
{
  return geo_from_text(wkt, srid, true);
}

/**
 * @ingroup meos_pgis_types
 * @brief Return the Well-Known Text (WKT) representation of a
 * geometry/geography
 * @param[in] gs Geometry/geography
 * @param[in] precision Maximum number of decimal digits
 * @note This is a a stricter version of #pgis_geometry_in, where we refuse to
 * accept (HEX)WKB or EWKT.
 * @note PostGIS function: @p LWGEOM_asText(PG_FUNCTION_ARGS)
 */
char *
geo_as_text(const GSERIALIZED *gs, int precision)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs))
    return NULL;

  LWGEOM *geom = lwgeom_from_gserialized(gs);
  return lwgeom_to_wkt(geom, WKT_ISO, precision, NULL);
}

/**
 * @ingroup meos_pgis_types
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * geometry/geography
 * @param[in] gs Geometry/geography
 * @param[in] precision Maximum number of decimal digits
 * @note This is a a stricter version of #pgis_geometry_in, where we refuse to
 * accept (HEX)WKB or EWKT.
 * @note PostGIS function: @p LWGEOM_asEWKT(PG_FUNCTION_ARGS)
 */
char *
geo_as_ewkt(const GSERIALIZED *gs, int precision)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs))
    return NULL;

  LWGEOM *geom = lwgeom_from_gserialized(gs);
  return lwgeom_to_wkt(geom, WKT_EXTENDED, precision, NULL);
}

/**
 * @ingroup meos_pgis_types
 * @brief Return a geometry from its hex-encoded ASCII Well-Known Binary
 * (HexEWKB) representation
 * @param[in] wkt WKT string
 * @note This is a a stricter version of #pgis_geometry_in, where we refuse to
 * accept (HEX)WKB or EWKT.
 * @note PostGIS function: @p LWGEOM_from_text(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geometry_from_hexewkb(const char *wkt)
{
  return pgis_geometry_in((char *) wkt, -1);
}

/**
 * @ingroup meos_pgis_types
 * @brief Return a geography from its hex-encoded ASCII Well-Known Binary
 * (HexEWKB) representation
 * @param[in] wkt WKT string
 * @note This is a a stricter version of #pgis_geography_in, where we refuse to
 * accept (HEX)WKB or EWKT.
 * @note PostGIS function: @p LWGEOM_from_text(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geography_from_hexewkb(const char *wkt)
{
  return pgis_geography_in((char *) wkt, -1);
}

/**
 * @ingroup meos_pgis_types
 * @brief Return the hex-encoded ASCII Well-Known Binary (HexWKB)
 * representation of a geometry/geography
 * @param[in] gs Geometry/geography
 * @param[in] endian Endianness
 * @note PostGIS function: @p AsHEXEWKB(gs, string)
 */
char *
geo_as_hexewkb(const GSERIALIZED *gs, const char *endian)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs))
    return NULL;

  uint8_t variant = 0;
  /* If user specified endianness, respect it */
  if (endian != NULL)
  {
    if  (! strncmp(endian, "xdr", 3) || ! strncmp(endian, "XDR", 3))
      variant = variant | WKB_XDR;
    else
      variant = variant | WKB_NDR;
  }
  /* Create WKB hex string */
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  lwvarlena_t *hexwkb = lwgeom_to_hexwkb_varlena(geom, variant | WKB_EXTENDED);
  char *result = strdup(VARDATA(hexwkb));
  pfree(hexwkb);
  return result;
}

/**
 * @ingroup meos_pgis_types
 * @brief Return a geometry/geography from its EWKB representation
 * @details This function parses EWKB (extended form) which also contains SRID
 * info.
 * @param[in] bytea_wkb WKB string
 * @param[in] srid SRID
 * @note PostGIS function: @p LWGEOMFromEWKB(wkb, [SRID])
 * @note wkb is in *binary* not hex form
 */
GSERIALIZED *
geo_from_ewkb(const bytea *bytea_wkb, int32 srid)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) bytea_wkb))
    return NULL;

  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  LWGEOM *geom = lwgeom_from_wkb(wkb, VARSIZE_ANY_EXHDR(bytea_wkb),
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
 * @ingroup meos_pgis_types
 * @brief Return the Extended Well-Known Binary (EWKB) representation of a
 * geometry/geography
 * @param[in] gs Geometry/geography
 * @param[in] endian Endianness
 * @note PostGIS function: @p WKBFromLWGEOM(PG_FUNCTION_ARGS)
 */
bytea *
geo_as_ewkb(const GSERIALIZED *gs, char *endian)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs))
    return NULL;

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
  bytea *result = palloc(wkb->size - LWVARHDRSZ);
  memcpy(result, wkb->data, wkb->size - LWVARHDRSZ);
  pfree(geom); pfree(wkb);
  return result;
}

/**
 * @ingroup meos_pgis_types
 * @brief Return the GeoJSON representation of a geometry
 * @param[in] geojson GeoJSON string
 * @note PostGIS function: @p geom_from_geojson(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geo_from_geojson(const char *geojson)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) geojson))
    return NULL;

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
 * @ingroup meos_pgis_types
 * @brief Return the GeoJSON representation of a geometry
 * @param[in] gs Geometry
 * @param[in] option Option
 * @param[in] precision Maximum number of decimal digits
 * @param[in] srs Spatial reference system
 * @note PostGIS function: @p LWGEOM_asGeoJson(PG_FUNCTION_ARGS)
 */
char *
geo_as_geojson(const GSERIALIZED *gs, int option, int precision, char *srs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs))
    return NULL;

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
#endif /* MEOS */

/**
 * @ingroup meos_pgis_types
 * @brief Return true if the geometries/geographies are the same
 * @param[in] gs1,gs2 Geometries/geographies
 */
bool
geo_same(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs1) || ! ensure_not_null((void *) gs2))
    return false;

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  char result = lwgeom_same(geom1, geom2);
  pfree(geom1); pfree(geom2);
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

GSERIALIZED *
geography_from_lwgeom(LWGEOM *lwgeom, int32 typmod)
{
  GSERIALIZED *g_ser = NULL;

  /* Set geodetic flag */
  lwgeom_set_geodetic(lwgeom, true);

  /* Check that this is a type we can handle */
  geography_valid_type(lwgeom->type);

  /* Force the geometry to have valid geodetic coordinate range. */
  lwgeom_nudge_geodetic(lwgeom);
  if (lwgeom_force_geodetic(lwgeom) == LW_TRUE)
  {
    meos_error(NOTICE, MEOS_ERR_TEXT_INPUT,
      "Coordinate values were coerced into range [-180 -90, 180 90] for GEOGRAPHY");
  }

  /* Force default SRID to the default */
  if ((int) lwgeom->srid <= 0)
    lwgeom->srid = SRID_DEFAULT;

  /*
  ** Serialize our lwgeom and set the geodetic flag so subsequent
  ** functions do the right thing.
  */
  g_ser = geo_serialize(lwgeom);

  /* Check for typmod agreement */
  if (typmod >= 0)
    g_ser = postgis_valid_typmod(g_ser, typmod);

  return g_ser;
}

/**
 * @ingroup meos_pgis_types
 * @brief Return a geography from its Well-Known Text or Binary (WKT or Binary)
 * representation
 * @param[in] str String
 * @param[in] typmod Typmod
 * @note PostGIS function: @p geography_in(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
pgis_geography_in(char *str, int32 typmod)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;

  LWGEOM_PARSER_RESULT lwg_parser_result;
  LWGEOM *lwgeom = NULL;
  GSERIALIZED *g_ser = NULL;

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
    if ( lwgeom_parse_wkt(&lwg_parser_result, str, LW_PARSER_CHECK_ALL) == LW_FAILURE )
      PG_PARSER_ERROR(lwg_parser_result);

    lwgeom = lwg_parser_result.geom;
  }

#if ! MEOS
  /* Error on any SRID != default */
  srid_check_latlong(lwgeom->srid);
#endif /* ! MEOS */

  /* Convert to gserialized */
  g_ser = geography_from_lwgeom(lwgeom, typmod);

  /* Clean up temporary object */
  lwgeom_free(lwgeom);

  return g_ser;
}

#if MEOS
/*
** geography_from_binary(*char) returns *GSERIALIZED
*/
GSERIALIZED *
pgis_geography_from_binary(const char *wkb_bytea)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) wkb_bytea))
    return NULL;

  size_t wkb_size = VARSIZE(wkb_bytea);
  uint8_t *wkb = (uint8_t *) VARDATA(wkb_bytea);
  LWGEOM *geom = lwgeom_from_wkb(wkb, wkb_size, LW_PARSER_CHECK_NONE);

  if (!  geom )
  {
    meos_error(ERROR, MEOS_ERR_WKB_INPUT, "Unable to parse WKB string");
    return NULL;
  }

  /* Error on any SRID != default */
  // srid_check_latlong(lwgeom->srid);

  GSERIALIZED *result = geography_from_lwgeom(geom, -1);
  lwgeom_free(geom);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/

#if 0 /* not used  */
/**
 * @brief Get a geography from a geometry
 * @note PostGIS function: @p geography_from_geometry(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
gserialized_geog_from_geom(GSERIALIZED *geom)
{
  LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
  geography_valid_type(lwgeom_get_type(lwgeom));

  /* Force default SRID */
  if ( (int) lwgeom->srid <= 0 )
  {
    lwgeom->srid = SRID_DEFAULT;
  }

  /* Error on any SRID != default */
  // Cannot test this in MobilityDB since we do not have access to PROJ
  // srid_check_latlong(lwgeom->srid);

  /* Force the geometry to have valid geodetic coordinate range. */
  lwgeom_nudge_geodetic(lwgeom);
  if ( lwgeom_force_geodetic(lwgeom) == LW_TRUE )
  {
    meos_error(NOTICE, MEOS_ERR_TEXT_INPUT,
      "Coordinate values were coerced into range [-180 -90, 180 90] for GEOGRAPHY");
    return NULL;
  }

  /* force recalculate of box by dropping */
  lwgeom_drop_bbox(lwgeom);

  lwgeom_set_geodetic(lwgeom, true);
  /* We are trusting geography_serialize will add a box if needed */
  GSERIALIZED *result = geo_serialize(lwgeom);
  lwgeom_free(lwgeom);
  return result;
}

/**
 * @brief Get a geometry from a geography
 * @note PostGIS function: @p geometry_from_geography(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
gserialized_geom_from_geog(GSERIALIZED *geom)
{
  LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
  /* Recalculate the boxes after re-setting the geodetic bit */
  lwgeom_set_geodetic(lwgeom, false);
  lwgeom_refresh_bbox(lwgeom);
  /* We want "geometry" to think all our "geography" has an SRID, and the
     implied SRID is the default, so we fill that in if our SRID is actually unknown. */
  if ( (int)lwgeom->srid <= 0 )
    lwgeom->srid = SRID_DEFAULT;

  GSERIALIZED *result = geo_serialize(lwgeom);
  lwgeom_free(lwgeom);
  return result;
}
#endif /* not used  */

/*****************************************************************************
 * Functions adapted from lwgeom_functions_analytic.c
 *****************************************************************************/

/**
 * @brief Get a geometry from a geography
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
 * @brief Interpolate a point from a line
 * @note PostGIS function: @p LWGEOM_line_interpolate_point(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
linestring_line_interpolate_point(GSERIALIZED *gs, double distance_fraction,
  char repeat)
{
  if (distance_fraction < 0 || distance_fraction > 1)
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
  POINTARRAY *opa = lwline_interpolate_points(lwline, distance_fraction,
   repeat);

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
 * @brief Get a subline from a line
 * @note PostGIS function: @p LWGEOM_line_substring(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
linestring_substring(GSERIALIZED *geom, double from, double to)
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
  uint8_t type = (uint8_t) gserialized_get_type(geom);
  if (type == LINETYPE)
  {
    LWLINE *iline = lwgeom_as_lwline(lwgeom_from_gserialized(geom));
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
    LWMLINE *iline = lwgeom_as_lwmline(lwgeom_from_gserialized(geom));
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
 * @brief Locate a point into a line
 * @return On error return -1.0
 */
double
linestring_locate_point(GSERIALIZED *gs1, GSERIALIZED *gs2)
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
 * PointN(GEOMETRY,INTEGER) -- find the first linestring in GEOMETRY,
 * @return the point at index INTEGER (1 is 1st point).  Return @p NULL if
 * there is no LINESTRING(..) in GEOMETRY or INTEGER is out of bounds.
 */
GSERIALIZED *
linestring_point_n(const GSERIALIZED *gs, int where)
{
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  LWPOINT *point = NULL;
  int type = geom->type;

  /* If index is negative, count backward */
  if (where < 1)
  {
    int count = -1;
    if ( type == LINETYPE || type == CIRCSTRINGTYPE || type == COMPOUNDTYPE )
      count = lwgeom_count_vertices(geom);
    if (count >0)
    {
      /* only work if we found the total point number */
      /* converting where to positive backward indexing, +1 because 1 indexing */
      where = where + count + 1;
    }
    if (where < 1)
      return NULL;
  }

  if ( type == LINETYPE || type == CIRCSTRINGTYPE )
  {
    /* OGC index starts at one, so we substract first. */
    point = lwline_get_lwpoint((LWLINE*) geom, where - 1);
  }
  else if ( type == COMPOUNDTYPE )
  {
    point = lwcompound_get_lwpoint((LWCOMPOUND*) geom, where - 1);
  }

  lwgeom_free(geom);

  if (!  point )
    return NULL;

  return geo_serialize(lwpoint_as_lwgeom(point));
}

/**
 * numpoints(LINESTRING) -- return the number of points in the
 * linestring, or NULL if it is not a linestring
 * @return On error return -1.0
*/
int
linestring_numpoints(const GSERIALIZED *gs)
{
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  int count = -1;
  int type = geom->type;

  if ( type == LINETYPE || type == CIRCSTRINGTYPE || type == COMPOUNDTYPE )
    count = lwgeom_count_vertices(geom);

  lwgeom_free(geom);

  /* OGC says this functions is only valid on LINESTRING */
  if ( count < 0 )
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "Error in computing number of points of a linestring");
    return -1;
  }

  return count;
}

/*****************************************************************************/
