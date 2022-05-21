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
 * @file pgis_call.c
 * @brief MobilityDB functions PGIS_func(...) corresponding to external
 * PostGIS functions func(PG_FUNCTION_ARGS). This avoids bypassing the
 * function manager fmgr.c.
 * @note These functions are only available for PGIS version >= 3.
 */

#if POSTGIS_VERSION_NUMBER >= 30000

#include "point/pgis_call.h"

/* C */
#include <assert.h>
#include <float.h>
/* GEOS */
#include <geos_c.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/elog.h>
/* PostGIS */
#include <liblwgeom.h>
#include <lwgeom_log.h>
#include <lwgeom_geos.h>
/* MobilityDB */
#include "point/tpoint_spatialfuncs.h"

/* To avoid including lwgeom_geos.h */
GSERIALIZED *GEOS2POSTGIS(GEOSGeom geom, char want3d);
GEOSGeometry *POSTGIS2GEOS(const GSERIALIZED *g);

/* To avoid including lwgeom_functions_analytic.h */
extern int point_in_polygon(LWPOLY *polygon, LWPOINT *point);
extern int point_in_multipolygon(LWMPOLY *mpolygon, LWPOINT *point);

/* Modified version of PG_PARSER_ERROR */

#if MEOS
#define PG_PARSER_ERROR(lwg_parser_result) \
  do { \
    elog(ERROR, "%s", lwg_parser_result.message); \
  } while(0);
#else
  #include <lwgeom_pg.h>
#endif

/*****************************************************************************
 * Functions adapted from lwgeom_box.c
 *****************************************************************************/

/**
 * @brief Create a geometry from a BOX2D
 * @note PostGIS function: Datum BOX2D_to_LWGEOM(PG_FUNCTION_ARGS)
 * @note With respect to the original PostGIS function we also set the SRID
 * which is passed as an additional argument
 */
GSERIALIZED *
PGIS_BOX2D_to_LWGEOM(GBOX *box, int srid)
{
  POINTARRAY *pa = ptarray_construct_empty(0, 0, 5);
  POINT4D pt;
  GSERIALIZED *result;

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
    result = geo_serialize(lwpoint_as_lwgeom(point));
    lwpoint_free(point);
  }
  else if ( (box->xmin == box->xmax) || (box->ymin == box->ymax) )
  {
    LWLINE *line;

    /* Assign coordinates to point array */
    pt.x = box->xmin;
    pt.y = box->ymin;
    ptarray_append_point(pa, &pt, LW_TRUE);
    pt.x = box->xmax;
    pt.y = box->ymax;
    ptarray_append_point(pa, &pt, LW_TRUE);

    /* Construct and serialize linestring */
    line = lwline_construct(srid, NULL, pa);
    /* MobilityDB: The above function does not set the geodetic flag */
    FLAGS_SET_GEODETIC(line->flags, FLAGS_GET_GEODETIC(box->flags));
    result = geo_serialize(lwline_as_lwgeom(line));
    lwline_free(line);
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
    result = geo_serialize(lwpoly_as_lwgeom(poly));
    lwpoly_free(poly);
  }

  return result;
}

/*****************************************************************************
 * Functions adapted from lwgeom_box3d.c
 *****************************************************************************/

/**
 * @brief Create a geometry from a BOX3D
 * @note PostGIS function: Datum BOX3D_to_LWGEOM(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
PGIS_BOX3D_to_LWGEOM(BOX3D *box)
{
  POINTARRAY *pa;
  GSERIALIZED *result;
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

    result = geo_serialize(lwpoint_as_lwgeom(lwpt));
    lwpoint_free(lwpt);
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

    result = geo_serialize(lwline_as_lwgeom(lwline));
    lwline_free(lwline);
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
    result = geo_serialize(lwpoly_as_lwgeom(lwpoly));
    lwpoly_free(lwpoly);
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
    result = geo_serialize(lwpoly_as_lwgeom(lwpoly));
    lwpoly_free(lwpoly);
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
    result = geo_serialize(lwpoly_as_lwgeom(lwpoly));
    lwpoly_free(lwpoly);
  }
  /* BOX3D is a polyhedron */
  else
  {
    POINT4D points[8];
    static const int ngeoms = 6;
    LWGEOM **geoms = (LWGEOM **)lwalloc(sizeof(LWGEOM *) * ngeoms);
    LWGEOM *geom = NULL;

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

    geom = (LWGEOM *)lwcollection_construct(POLYHEDRALSURFACETYPE, SRID_UNKNOWN, NULL, ngeoms, geoms);

    FLAGS_SET_SOLID(geom->flags, 1);

    result = geo_serialize(geom);
    lwcollection_free((LWCOLLECTION *)geom);
  }

  gserialized_set_srid(result, box->srid);

  return result;
}

/*****************************************************************************
 * Functions adapted from lwgeom_functions_basic.c
 *****************************************************************************/

/* The boundary function has changed its implementation in version 3.2.
 * This is the version in 3.2.1 */
LWGEOM *
PGIS_lwgeom_boundary(LWGEOM *lwgeom)
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
      LWMPOINT *points = lwgeom_as_lwmpoint(PGIS_lwgeom_boundary(
        (LWGEOM *)lwmline->geoms[i]));
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
      lwcollection_add_lwgeom(lwcol_boundary,
        PGIS_lwgeom_boundary(lwcol->geoms[i]));

    LWGEOM *lwout = lwgeom_homogenize((LWGEOM *)lwcol_boundary);
    lwgeom_free((LWGEOM *)lwcol_boundary);

    return lwout;
  }
  default:
    elog(ERROR, "unsupported geometry type: %s", lwtype_name(lwgeom->type));
    return NULL;
  }
}

/**
 * @brief Return the boundary of a geometry
 * @note PostGIS function: Datum boundary(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
PGIS_boundary(const GSERIALIZED *geom1)
{
  GSERIALIZED *result;
  LWGEOM *lwgeom, *lwresult;

  /* Empty.Boundary() == Empty, but of other dimension, so can't shortcut */

  lwgeom = lwgeom_from_gserialized(geom1);
  lwresult = PGIS_lwgeom_boundary(lwgeom);
  if (!lwresult)
  {
    lwgeom_free(lwgeom);
    return NULL;
  }

  result = geo_serialize(lwresult);

  lwgeom_free(lwgeom);
  lwgeom_free(lwresult);

  return result;
}

/**
 * @brief Return the shortest 2d line between two geometries
 * @note PostGIS function: Datum LWGEOM_shortestline2d(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
PGIS_LWGEOM_shortestline2d(const GSERIALIZED *geom1, const GSERIALIZED *geom2)
{
  ensure_same_srid(gserialized_get_srid(geom1), gserialized_get_srid(geom2));
  LWGEOM *lwgeom1 = lwgeom_from_gserialized(geom1);
  LWGEOM *lwgeom2 = lwgeom_from_gserialized(geom2);
  LWGEOM *theline = lwgeom_closest_line(lwgeom1, lwgeom2);
  if (lwgeom_is_empty(theline))
    return NULL;

  GSERIALIZED *result = geo_serialize(theline);
  lwgeom_free(theline);
  lwgeom_free(lwgeom1);
  lwgeom_free(lwgeom2);
  return result;
}

/**
 * @brief Return the shortest line between two geometries in 3D
 * @note PostGIS function: Datum LWGEOM_shortestline3d(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
PGIS_LWGEOM_shortestline3d(const GSERIALIZED *geom1, const GSERIALIZED *geom2)
{
  ensure_same_srid(gserialized_get_srid(geom1), gserialized_get_srid(geom2));
  LWGEOM *lwgeom1 = lwgeom_from_gserialized(geom1);
  LWGEOM *lwgeom2 = lwgeom_from_gserialized(geom2);
  LWGEOM *theline = lwgeom_closest_line_3d(lwgeom1, lwgeom2);
  if (lwgeom_is_empty(theline))
    return NULL;

  GSERIALIZED *result = geo_serialize(theline);
  lwgeom_free(theline);
  lwgeom_free(lwgeom1);
  lwgeom_free(lwgeom2);
  return result;
}

/**
 * @brief Return the distance between two geometries
 * @note PostGIS function: Datum ST_Distance(PG_FUNCTION_ARGS)
 */
double
PGIS_ST_Distance(const GSERIALIZED *geom1, const GSERIALIZED *geom2)
{
  ensure_same_srid(gserialized_get_srid(geom1), gserialized_get_srid(geom2));
  LWGEOM *lwgeom1 = lwgeom_from_gserialized(geom1);
  LWGEOM *lwgeom2 = lwgeom_from_gserialized(geom2);
  double mindist = lwgeom_mindistance2d(lwgeom1, lwgeom2);
  lwgeom_free(lwgeom1);
  lwgeom_free(lwgeom2);
  /* if called with empty geometries the ingoing mindistance is untouched,
   * and makes us return NULL */
  if (mindist < FLT_MAX)
    return mindist;
  return -1;
}

/**
 * @brief Return the distance between two geometries
 * @note PostGIS function: Datum ST_3DDistance(PG_FUNCTION_ARGS)
 */
double
PGIS_ST_3DDistance(const GSERIALIZED *geom1, const GSERIALIZED *geom2)
{
  ensure_same_srid(gserialized_get_srid(geom1), gserialized_get_srid(geom2));
  LWGEOM *lwgeom1 = lwgeom_from_gserialized(geom1);
  LWGEOM *lwgeom2 = lwgeom_from_gserialized(geom2);
  double mindist = lwgeom_mindistance3d(lwgeom1, lwgeom2);
  lwgeom_free(lwgeom1);
  lwgeom_free(lwgeom2);
  /* if called with empty geometries the ingoing mindistance is untouched,
   * and makes us return NULL */
  if (mindist < FLT_MAX)
    return mindist;
  return -1;
}

/**
 * @brief Return true if the 3D geometries intersect
 * @note PostGIS function: Datum LWGEOM_reverse(PG_FUNCTION_ARGS)
 */
bool
PGIS_ST_3DIntersects(const GSERIALIZED *geom1, const GSERIALIZED *geom2)
{
  ensure_same_srid(gserialized_get_srid(geom1), gserialized_get_srid(geom2));
  double mindist;
  LWGEOM *lwgeom1 = lwgeom_from_gserialized(geom1);
  LWGEOM *lwgeom2 = lwgeom_from_gserialized(geom2);
  mindist = lwgeom_mindistance3d_tolerance(lwgeom1, lwgeom2, 0.0);
  /*empty geometries cases should be right handled since return from underlying
    functions should be FLT_MAX which causes false as answer*/
  return (0.0 == mindist);
}

/**
 * @brief Return true if the geometries are within the given distance
 * @note PostGIS function: Datum LWGEOM_dwithin(PG_FUNCTION_ARGS)
 */
bool
PGIS_LWGEOM_dwithin(const GSERIALIZED *geom1, const GSERIALIZED *geom2,
  double tolerance)
{
  if (tolerance < 0)
    elog(ERROR, "Tolerance cannot be less than zero\n");
  ensure_same_srid(gserialized_get_srid(geom1), gserialized_get_srid(geom2));

  if (gserialized_is_empty(geom1) || gserialized_is_empty(geom2))
    return false;

  LWGEOM *lwgeom1 = lwgeom_from_gserialized(geom1);
  LWGEOM *lwgeom2 = lwgeom_from_gserialized(geom2);
  double mindist = lwgeom_mindistance2d_tolerance(lwgeom1, lwgeom2, tolerance);
  /*empty geometries cases should be right handled since return from underlying
   functions should be FLT_MAX which causes false as answer*/
  return (tolerance >= mindist);
}

/**
 * @brief Return true if the geometries are within the given distance
 * @note PostGIS function: Datum LWGEOM_dwithin3d(PG_FUNCTION_ARGS)
 */
bool
PGIS_LWGEOM_dwithin3d(const GSERIALIZED *geom1, const GSERIALIZED *geom2,
  double tolerance)
{
  if (tolerance < 0)
    elog(ERROR, "Tolerance cannot be less than zero\n");
  ensure_same_srid(gserialized_get_srid(geom1), gserialized_get_srid(geom2));

  LWGEOM *lwgeom1 = lwgeom_from_gserialized(geom1);
  LWGEOM *lwgeom2 = lwgeom_from_gserialized(geom2);

  double mindist = lwgeom_mindistance3d_tolerance(lwgeom1, lwgeom2, tolerance);

  /*empty geometries cases should be right handled since return from underlying
   functions should be FLT_MAX which causes false as answer*/
  return (tolerance >= mindist);
}

/*****************************************************************************
 * Functions adapted from lwgeom_geos.c
 *****************************************************************************/

static char
is_point(const GSERIALIZED* g)
{
  int type = gserialized_get_type(g);
  return type == POINTTYPE || type == MULTIPOINTTYPE;
}

static char
is_poly(const GSERIALIZED* g)
{
    int type = gserialized_get_type(g);
    return type == POLYGONTYPE || type == MULTIPOLYGONTYPE;
}

/**
 * @brief Return -1, 0, or 1 depending on whether a (multi)point is completely
 * outside, on the boundary, or completely inside a (multi)polygon
 * @note This function is based PostGIS function pip_short_circuit bypassing
 * the cache
 */
static int
MOBDB_point_in_polygon(const GSERIALIZED *geom1, const GSERIALIZED *geom2,
  bool inter)
{
  const GSERIALIZED *gpoly = is_poly(geom1) ? geom1 : geom2;
  const GSERIALIZED *gpoint = is_point(geom1) ? geom1 : geom2;

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
 * @brief Transform the GSERIALIZED geometries into GEOSGeometry and
 * call the GEOS function passed as argument
 */
static char
MOBDB_call_geos(const GSERIALIZED *geom1, const GSERIALIZED *geom2,
  char (*func)(const GEOSGeometry *g1, const GEOSGeometry *g2))
{
  initGEOS(lwnotice, lwgeom_geos_error);

  GEOSGeometry *g1;
  GEOSGeometry *g2;
  g1 = POSTGIS2GEOS(geom1);
  if (!g1)
    elog(ERROR, "First argument geometry could not be converted to GEOS");
  g2 = POSTGIS2GEOS(geom2);
  if (!g2)
  {
    GEOSGeom_destroy(g1);
    elog(ERROR, "Second argument geometry could not be converted to GEOS");
  }

  char result = func(g1, g2);

  GEOSGeom_destroy(g1);
  GEOSGeom_destroy(g2);

  if (result == 2)
    elog(ERROR, "GEOS returned error");

  return result;
}

/**
 * @brief Return true if the geometries intersect or the first contains
 * the other
 *
 * @param[in] geom1,geom2, Geometries
 * @param[in] inter: True when performing intersection, fals for contains
 * @note PostGIS functions: Datum ST_Intersects(PG_FUNCTION_ARGS) and
 * Datum contains(PG_FUNCTION_ARGS)
 */
bool
PGIS_inter_contains(const GSERIALIZED *geom1, const GSERIALIZED *geom2,
  bool inter)
{
  GBOX box1, box2;

  ensure_same_srid(gserialized_get_srid(geom1), gserialized_get_srid(geom2));

  /* A.Intersects(Empty) == FALSE */
  if ( gserialized_is_empty(geom1) || gserialized_is_empty(geom2) )
      return false;

  /*
   * short-circuit 1: if geom2 bounding box does not overlap
   * geom1 bounding box we can return FALSE.
   */
  if ( gserialized_get_gbox_p(geom1, &box1) &&
          gserialized_get_gbox_p(geom2, &box2) )
  {
    if ( gbox_overlaps_2d(&box1, &box2) == LW_FALSE )
      return false;
  }

  /*
   * short-circuit 2: if the geoms are a point and a polygon,
   * call the point_outside_polygon function.
   */
  if ((is_point(geom1) && is_poly(geom2)) || (is_poly(geom1) && is_point(geom2)))
  {
    int pip_result = MOBDB_point_in_polygon(geom1, geom2, inter);
    return inter ?
      (pip_result != -1) : /* not outside */
      (pip_result == 1); /* inside */
  }

  /* Call GEOS function */
  bool result = (bool) MOBDB_call_geos(geom1, geom2, &GEOSIntersects);

  return result;
}

/**
 * @brief Return true if the geometries touch
 * @note PostGIS function: Datum touches(PG_FUNCTION_ARGS)
  */
bool
PGIS_touches(const GSERIALIZED *geom1, const GSERIALIZED *geom2)
{
  ensure_same_srid(gserialized_get_srid(geom1), gserialized_get_srid(geom2));

  /* A.Touches(Empty) == FALSE */
  if ( gserialized_is_empty(geom1) || gserialized_is_empty(geom2) )
    return false;

  /*
   * short-circuit 1: if geom2 bounding box does not overlap
   * geom1 bounding box we can return FALSE.
   */
  GBOX box1, box2;
  if ( gserialized_get_gbox_p(geom1, &box1) &&
      gserialized_get_gbox_p(geom2, &box2) )
  {
    if ( gbox_overlaps_2d(&box1, &box2) == LW_FALSE )
    {
      return false;
    }
  }

  /* Call GEOS function */
  bool result = (bool) MOBDB_call_geos(geom1, geom2, &GEOSTouches);

  return result;
}

/**
 * @brief Return true if the 3D geometries intersect
 * @note PostGIS function: Datum relate_pattern(PG_FUNCTION_ARGS)
 */
bool
PGIS_relate_pattern(const GSERIALIZED *geom1, const GSERIALIZED *geom2,
  char *patt)
{
  ensure_same_srid(gserialized_get_srid(geom1), gserialized_get_srid(geom2));

  /* TODO handle empty */

  initGEOS(lwnotice, lwgeom_geos_error);

  GEOSGeometry *g1 = POSTGIS2GEOS(geom1);
  if (!g1)
    elog(ERROR, "First argument geometry could not be converted to GEOS");
  GEOSGeometry *g2 = POSTGIS2GEOS(geom2);
  if (!g2)
  {
    GEOSGeom_destroy(g1);
    elog(ERROR, "Second argument geometry could not be converted to GEOS");
  }

  /*
  ** Need to make sure 't' and 'f' are upper-case before handing to GEOS
  */
  for (size_t i = 0; i < strlen(patt); i++ )
  {
    if ( patt[i] == 't' ) patt[i] = 'T';
    if ( patt[i] == 'f' ) patt[i] = 'F';
  }

  char result = GEOSRelatePattern(g1, g2, patt);
  GEOSGeom_destroy(g1);
  GEOSGeom_destroy(g2);

  if (result == 2)
    elog(ERROR, "GEOSRelatePattern returned error");

  return (bool) result;
}

/**
 * @brief Return true if the 3D geometries intersect
 * @note PostGIS function: Datum LWGEOM_reverse(PG_FUNCTION_ARGS)
 * @note With respect to the original function we do not use the prec
 * argument
 */
GSERIALIZED *
PGIS_ST_Intersection(GSERIALIZED *geom1, GSERIALIZED *geom2)
{
  GSERIALIZED *result;
  LWGEOM *lwgeom1, *lwgeom2, *lwresult;
  double prec = -1;
  lwgeom1 = lwgeom_from_gserialized(geom1);
  lwgeom2 = lwgeom_from_gserialized(geom2);
  lwresult = lwgeom_intersection_prec(lwgeom1, lwgeom2, prec);
  result = geo_serialize(lwresult);
  lwgeom_free(lwgeom1);
  lwgeom_free(lwgeom2);
  lwgeom_free(lwresult);
  return result;
}

/*****************************************************************************
 * Functions adapted from geography_measurement.c
 *****************************************************************************/

/**
 * @brief Return double length in meters
 * @note PostGIS function: Datum geography_length(PG_FUNCTION_ARGS)
 */
double
PGIS_geography_length(GSERIALIZED *g, bool use_spheroid)
{
  /* EMPTY things have no length */
  int32 geo_type = gserialized_get_type(g);
  if (gserialized_is_empty(g) || geo_type == POLYGONTYPE ||
    geo_type == MULTIPOLYGONTYPE)
    return 0.0;

  /* Get our geometry object loaded into memory. */
  LWGEOM *lwgeom = lwgeom_from_gserialized(g);

  /* Initialize spheroid */
  /* We currently cannot use the following statement since PROJ4 API is not
   * available directly to MobilityDB. */
  // spheroid_init_from_srid(gserialized_get_srid(g), &s);
  SPHEROID s;
  spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

  /* User requests spherical calculation, turn our spheroid into a sphere */
  if ( ! use_spheroid )
    s.a = s.b = s.radius;

  /* Calculate the length */
  double length = lwgeom_length_spheroid(lwgeom, &s);

  /* Something went wrong... */
  if ( length < 0.0 )
  {
    elog(ERROR, "lwgeom_length_spheroid returned length < 0.0");
  }

  /* Clean up */
  lwgeom_free(lwgeom);

  return length;
}

/**
 * @brief Return true if the geographies are within the given distance
 * @note PostGIS function: Datum geography_dwithin_uncached(PG_FUNCTION_ARGS)
 * where we use the WGS84 spheroid
 */
bool
PGIS_geography_dwithin(GSERIALIZED *g1, GSERIALIZED *g2, double tolerance,
  bool use_spheroid)
{
  ensure_same_srid(gserialized_get_srid(g1), gserialized_get_srid(g2));
  /* Return FALSE on empty arguments. */
  if (gserialized_is_empty(g1) || gserialized_is_empty(g2))
    return false;

  /* Initialize spheroid */
  /* We currently cannot use the following statement since PROJ4 API is not
   * available directly to MobilityDB. */
  // spheroid_init_from_srid(gserialized_get_srid(g1), &s);
  SPHEROID s;
  spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

  /* Set to sphere if requested */
  if ( ! use_spheroid )
    s.a = s.b = s.radius;

  LWGEOM *lwgeom1 = lwgeom_from_gserialized(g1);
  LWGEOM *lwgeom2 = lwgeom_from_gserialized(g2);
  double distance = lwgeom_distance_spheroid(lwgeom1, lwgeom2, &s, tolerance);

  /* Clean up */
  lwgeom_free(lwgeom1);
  lwgeom_free(lwgeom2);

  /* Something went wrong... should already be eloged, return FALSE */
  if ( distance < 0.0 )
  {
    elog(ERROR, "lwgeom_distance_spheroid returned negative!");
    return false;
  }

  return (distance <= tolerance);
}

/* Defined in liblwgeom_internal.h */
#define PGIS_FP_TOLERANCE 1e-12

/**
 * @brief Return the distance between two geographies
 * @note PostGIS function: Datum geography_distance_uncached(PG_FUNCTION_ARGS)
 * @note We set by defaultboth tolerance and use_spheroid and initialize the
 * spheroid to WGS84
 * @note Errors return -1 to replace PG_RETURN_NULL()
 */
double
PGIS_geography_distance(const GSERIALIZED *g1, const GSERIALIZED *g2)
{
  ensure_same_srid(gserialized_get_srid(g1), gserialized_get_srid(g1));
  /* Return NULL on empty arguments. */
  if (gserialized_is_empty(g1) || gserialized_is_empty(g2) )
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
  if ( ! use_spheroid )
    s.a = s.b = s.radius;

  LWGEOM *lwgeom1 = lwgeom_from_gserialized(g1);
  LWGEOM *lwgeom2 = lwgeom_from_gserialized(g2);

  /* Make sure we have boxes attached */
  lwgeom_add_bbox_deep(lwgeom1, NULL);
  lwgeom_add_bbox_deep(lwgeom2, NULL);

  double distance = lwgeom_distance_spheroid(lwgeom1, lwgeom2, &s, tolerance);

  /* Clean up */
  lwgeom_free(lwgeom1);
  lwgeom_free(lwgeom2);

  /* Something went wrong, negative return... should already be eloged, return NULL */
  if ( distance < 0.0 )
    elog(ERROR, "PGIS_geography_distance returned distance < 0.0");

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
GSERIALIZED * postgis_valid_typmod(GSERIALIZED *gser, int32_t typmod)
{
  int32 geom_srid = gserialized_get_srid(gser);
  int32 geom_type = gserialized_get_type(gser);
  int32 geom_z = gserialized_has_z(gser);
  int32 geom_m = gserialized_has_m(gser);
  int32 typmod_srid = TYPMOD_GET_SRID(typmod);
  int32 typmod_type = TYPMOD_GET_TYPE(typmod);
  int32 typmod_z = TYPMOD_GET_Z(typmod);
  int32 typmod_m = TYPMOD_GET_M(typmod);

  /* No typmod (-1) => no preferences */
  if (typmod < 0) return gser;

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
       gserialized_is_empty(gser) )
  {
    LWPOINT *empty_point = lwpoint_construct_empty(geom_srid, geom_z, geom_m);
    geom_type = POINTTYPE;
    pfree(gser);
    if ( gserialized_is_geodetic(gser) )
      gser = geo_serialize(lwpoint_as_lwgeom(empty_point));
    else
      gser = geo_serialize(lwpoint_as_lwgeom(empty_point));
  }

  /* Typmod has a preference for SRID, but geometry does not? Harmonize the geometry SRID. */
  if ( typmod_srid > 0 && geom_srid == 0 )
  {
    gserialized_set_srid(gser, typmod_srid);
    geom_srid = typmod_srid;
  }

  /* Typmod has a preference for SRID? Geometry SRID had better match. */
  if ( typmod_srid > 0 && typmod_srid != geom_srid )
  {
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Geometry SRID (%d) does not match column SRID (%d)", geom_srid, typmod_srid) ));
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
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Geometry type (%s) does not match column type (%s)", lwtype_name(geom_type), lwtype_name(typmod_type)) ));
  }

  /* Mismatched Z dimensionality. */
  if ( typmod_z && ! geom_z )
  {
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Column has Z dimension but geometry does not" )));
  }

  /* Mismatched Z dimensionality (other way). */
  if ( geom_z && ! typmod_z )
  {
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Geometry has Z dimension but column does not" )));
  }

  /* Mismatched M dimensionality. */
  if ( typmod_m && ! geom_m )
  {
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Column has M dimension but geometry does not" )));
  }

  /* Mismatched M dimensionality (other way). */
  if ( geom_m && ! typmod_m )
  {
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Geometry has M dimension but column does not" )));
  }

  return gser;
}

/**
 * @brief Get a geometry from a string
 *
 * format is '[SRID=#;]wkt|wkb'
 *  LWGEOM_in( 'SRID=99;POINT(0 0)')
 *  LWGEOM_in( 'POINT(0 0)')            --> assumes SRID=SRID_UNKNOWN
 *  LWGEOM_in( 'SRID=99;0101000000000000000000F03F000000000000004')
 *  LWGEOM_in( '0101000000000000000000F03F000000000000004')
 *  LWGEOM_in( '{"type":"Point","coordinates":[1,1]}')
 *  returns a GSERIALIZED object

 * @note PostGIS function: Datum LWGEOM_in(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
PGIS_LWGEOM_in(char *input, int32 geom_typmod)
{
  char *str = input;
  LWGEOM_PARSER_RESULT lwg_parser_result;
  LWGEOM *lwgeom;
  GSERIALIZED *result;
  int32_t srid = 0;

  lwgeom_parser_result_init(&lwg_parser_result);

  /* Empty string. */
  if ( str[0] == '\0' ) {
    ereport(ERROR,(errmsg("parse error - invalid geometry")));
  }

  /* Starts with "SRID=" */
  if( strncasecmp(str,"SRID=",5) == 0 )
  {
    /* Roll forward to semi-colon */
    char *tmp = str;
    while ( tmp && *tmp != ';' )
      tmp++;

    /* Check next character to see if we have WKB  */
    if ( tmp && *(tmp+1) == '0' )
    {
      /* Null terminate the SRID= string */
      *tmp = '\0';
      /* Set str to the start of the real WKB */
      str = tmp + 1;
      /* Move tmp to the start of the numeric part */
      tmp = input + 5;
      /* Parse out the SRID number */
      srid = atoi(tmp);
    }
  }

  /* WKB? Let's find out. */
  if ( str[0] == '0' )
  {
    size_t hexsize = strlen(str);
    unsigned char *wkb = bytes_from_hexbytes(str, hexsize);
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
  else if (str[0] == '{')
  {
    char *srs = NULL;
    lwgeom = lwgeom_from_geojson(str, &srs);
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
    if ( lwgeom_parse_wkt(&lwg_parser_result, str, LW_PARSER_CHECK_ALL) == LW_FAILURE )
    {
      PG_PARSER_ERROR(lwg_parser_result);
    }
    lwgeom = lwg_parser_result.geom;
    if ( lwgeom_needs_bbox(lwgeom) )
      lwgeom_add_bbox(lwgeom);
    result = geo_serialize(lwgeom);
    lwgeom_parser_result_free(&lwg_parser_result);
  }

  if ( geom_typmod >= 0 )
  {
    result = postgis_valid_typmod(result, geom_typmod);
  }

  /* Don't free the parser result (and hence lwgeom) until we have done */
  /* the typemod check with lwgeom */
  return result;
}

/**
 * @brief Output function for geometries
 *
 * LWGEOM_out(lwgeom) --> cstring
 * output is 'SRID=#;<wkb in hex form>'
 * ie. 'SRID=-99;0101000000000000000000F03F0000000000000040'
 * WKB is machine endian
 * if SRID=-1, the 'SRID=-1;' will probably not be present.
 * @note PostGIS function: Datum LWGEOM_out(PG_FUNCTION_ARGS)
 */
char *
PGIS_LWGEOM_out(GSERIALIZED *geom)
{
  LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
  return lwgeom_to_hexwkb_buffer(lwgeom, WKB_EXTENDED);
}

/**
 * @brief Get a geometry from its binary representation
 *
 * This function must advance the StringInfo.cursor pointer
 * and leave it at the end of StringInfo.buf. If it fails
 * to do so the backend will raise an exception with message:
 * ERROR:  incorrect binary data format in bind parameter #
 * @note PostGIS function: Datum LWGEOM_send(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
PGIS_LWGEOM_recv(StringInfo buf)
{
  // We do not use the typmod
  int32 geom_typmod = -1;
  GSERIALIZED *geom;
  LWGEOM *lwgeom;

  lwgeom = lwgeom_from_wkb((uint8_t*)buf->data, buf->len, LW_PARSER_CHECK_ALL);

  if ( lwgeom_needs_bbox(lwgeom) )
    lwgeom_add_bbox(lwgeom);

  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;

  geom = geo_serialize(lwgeom);
  lwgeom_free(lwgeom);

  if ( geom_typmod >= 0 )
  {
    geom = postgis_valid_typmod(geom, geom_typmod);
  }

  return geom;
}

/*
 * WKBFromLWGEOM(lwgeom) --> wkb
 * this will have no 'SRID=#;'
 */
bytea *
PGIS_WKBFromLWGEOM(GSERIALIZED *geom)
{
  LWGEOM *lwgeom;
  uint8_t variant = 0;

  // We do not accept user specified endianness

  /* Create WKB hex string */
  lwgeom = lwgeom_from_gserialized(geom);
  return (bytea *) lwgeom_to_wkb_varlena(lwgeom, variant | WKB_EXTENDED);
}

/**
 * @brief Get the binary representation of a geometry
 * @note PostGIS function: Datum LWGEOM_send(PG_FUNCTION_ARGS)
 */
bytea *
PGIS_LWGEOM_send(GSERIALIZED *geo)
{
  return PGIS_WKBFromLWGEOM(geo);
}

/*****************************************************************************
 * Functions adapted from lwgeom_btree.c
 *****************************************************************************/

/**
 * @brief Return true if the first geometry is less than the second one
 * @note PostGIS function: Datum lwgeom_lt(PG_FUNCTION_ARGS)
 */
// bool
// PGIS_lwgeom_lt(GSERIALIZED *g1, GSERIALIZED *g2)
// {
  // int cmp = gserialized_cmp(g1, g2);
  // if (cmp < 0)
    // return true;
  // else
    // return false;
// }

/*****************************************************************************
 * Functions adapted from geography_inout.c
 *****************************************************************************/

/**
* The geography type only support POINT, LINESTRING, POLYGON, MULTI* variants
* of same, and GEOMETRYCOLLECTION. If the input type is not one of those, shut
* down the query.
*/
void
geography_valid_type(uint8_t type)
{
  if ( ! (type == POINTTYPE || type == LINETYPE || type == POLYGONTYPE ||
          type == MULTIPOINTTYPE || type == MULTILINETYPE ||
          type == MULTIPOLYGONTYPE || type == COLLECTIONTYPE) )
  {
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Geography type does not support %s", lwtype_name(type) )));
  }
}

GSERIALIZED *
gserialized_geography_from_lwgeom(LWGEOM *lwgeom, int32 geog_typmod)
{
  GSERIALIZED *g_ser = NULL;

  /* Set geodetic flag */
  lwgeom_set_geodetic(lwgeom, true);

  /* Check that this is a type we can handle */
  geography_valid_type(lwgeom->type);

  /* Force the geometry to have valid geodetic coordinate range. */
  lwgeom_nudge_geodetic(lwgeom);
  if ( lwgeom_force_geodetic(lwgeom) == LW_TRUE )
  {
    ereport(NOTICE, (errmsg_internal(
      "Coordinate values were coerced into range [-180 -90, 180 90] for GEOGRAPHY" ))
    );
  }

  /* Force default SRID to the default */
  if ( (int)lwgeom->srid <= 0 )
    lwgeom->srid = SRID_DEFAULT;

  /*
  ** Serialize our lwgeom and set the geodetic flag so subsequent
  ** functions do the right thing.
  */
  g_ser = geo_serialize(lwgeom);

  /* Check for typmod agreement */
  if ( geog_typmod >= 0 )
    g_ser = postgis_valid_typmod(g_ser, geog_typmod);

  return g_ser;
}

/**
 * @brief Get a geography from in string
 * @note PostGIS function: Datum geography_in(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
PGIS_geography_in(char *str, int32 geog_typmod)
{
  LWGEOM_PARSER_RESULT lwg_parser_result;
  LWGEOM *lwgeom = NULL;
  GSERIALIZED *g_ser = NULL;

  lwgeom_parser_result_init(&lwg_parser_result);

  /* Empty string. */
  if ( str[0] == '\0' )
    ereport(ERROR,(errmsg("parse error - invalid geometry")));

  /* WKB? Let's find out. */
  if ( str[0] == '0' )
  {
    /* TODO: 20101206: No parser checks! This is inline with current 1.5 behavior,
     * but needs discussion */
    lwgeom = lwgeom_from_hexwkb(str, LW_PARSER_CHECK_NONE);
    /* Error out if something went sideways */
    if ( ! lwgeom )
      ereport(ERROR,(errmsg("parse error - invalid geometry")));
  }
  /* WKT then. */
  else
  {
    if ( lwgeom_parse_wkt(&lwg_parser_result, str, LW_PARSER_CHECK_ALL) == LW_FAILURE )
      PG_PARSER_ERROR(lwg_parser_result);

    lwgeom = lwg_parser_result.geom;
  }

  /* Error on any SRID != default */
  // srid_check_latlong(lwgeom->srid);

  /* Convert to gserialized */
  g_ser = gserialized_geography_from_lwgeom(lwgeom, geog_typmod);

  /* Clean up temporary object */
  lwgeom_free(lwgeom);

  return g_ser;
}

/**
 * @brief Output a geography in string format
 * @note PostGIS function: Datum geography_out(PG_FUNCTION_ARGS)
 */
char *
PGIS_geography_out(GSERIALIZED *g)
{
  LWGEOM *lwgeom = lwgeom_from_gserialized(g);
  return lwgeom_to_hexwkb_buffer(lwgeom, WKB_EXTENDED);
}

/**
 * @brief Get a geography from its binary representation
 * @note PostGIS function: Datum geography_recv(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
PGIS_geography_recv(StringInfo buf)
{
  // We do not use typmod
  int32 geog_typmod = -1;
  LWGEOM *lwgeom = NULL;
  GSERIALIZED *g_ser = NULL;

  lwgeom = lwgeom_from_wkb((uint8_t*)buf->data, buf->len, LW_PARSER_CHECK_ALL);

  // We cannot perform the following check
  /* Error on any SRID != default */
  // srid_check_latlong(lwgeom->srid);

  g_ser = gserialized_geography_from_lwgeom(lwgeom, geog_typmod);

  /* Clean up temporary object */
  lwgeom_free(lwgeom);

  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;

  return g_ser;
}

/**
 * @brief Get the binary representation of a geography
 * @note PostGIS function: Datum geography_send(PG_FUNCTION_ARGS)
 */
bytea *
PGIS_geography_send(GSERIALIZED *g)
{
  LWGEOM *lwgeom = lwgeom_from_gserialized(g);
  return (bytea *) (lwgeom_to_wkb_varlena(lwgeom, WKB_EXTENDED));
}

/*****************************************************************************/

/**
 * @brief Get a geography from a geometry
 * @note PostGIS function: Datum geography_from_geometry(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
PGIS_geography_from_geometry(GSERIALIZED *geom)
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
    ereport(NOTICE, (errmsg_internal(
      "Coordinate values were coerced into range [-180 -90, 180 90] for GEOGRAPHY" ))
    );
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
 * @note PostGIS function: Datum geometry_from_geography(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
PGIS_geometry_from_geography(GSERIALIZED *geom)
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

/*****************************************************************************/

#endif /* POSTGIS_VERSION_NUMBER >= 30000 */
