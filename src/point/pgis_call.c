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
#include <float.h>
// #include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/elog.h>
/* PostGIS */
#include <liblwgeom.h>
#include <lwgeom_pg.h>
#include <lwgeom_log.h>
/* MobilityDB */
#include "point/tpoint_spatialfuncs.h"

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
    result = geometry_serialize(lwpoint_as_lwgeom(point));
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
    result = geometry_serialize(lwline_as_lwgeom(line));
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
    result = geometry_serialize(lwpoly_as_lwgeom(poly));
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

    result = geometry_serialize(lwpoint_as_lwgeom(lwpt));
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

    result = geometry_serialize(lwline_as_lwgeom(lwline));
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
    result = geometry_serialize(lwpoly_as_lwgeom(lwpoly));
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
    result = geometry_serialize(lwpoly_as_lwgeom(lwpoly));
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
    result = geometry_serialize(lwpoly_as_lwgeom(lwpoly));
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

    result = geometry_serialize(geom);
    lwcollection_free((LWCOLLECTION *)geom);
  }

  gserialized_set_srid(result, box->srid);

  return result;
}

/*****************************************************************************
 * Functions adapted from lwgeom_functions_basic.c
 *****************************************************************************/

/**
 * @brief Return the shortest 2d line between two geometries
 * @note PostGIS function: Datum LWGEOM_shortestline2d(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
PGIS_LWGEOM_shortestline2d(const GSERIALIZED *geom1, const GSERIALIZED *geom2)
{
  GSERIALIZED *result;
  LWGEOM *theline;
  LWGEOM *lwgeom1 = lwgeom_from_gserialized(geom1);
  LWGEOM *lwgeom2 = lwgeom_from_gserialized(geom2);
  gserialized_error_if_srid_mismatch(geom1, geom2, __func__);

  theline = lwgeom_closest_line(lwgeom1, lwgeom2);

  if (lwgeom_is_empty(theline))
    return NULL;

  result = geometry_serialize(theline);
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
  GSERIALIZED *result;
  LWGEOM *theline;
  LWGEOM *lwgeom1 = lwgeom_from_gserialized(geom1);
  LWGEOM *lwgeom2 = lwgeom_from_gserialized(geom2);

  theline = lwgeom_closest_line_3d(lwgeom1, lwgeom2);
  // theline = lw_dist3d_distanceline(lwgeom1, lwgeom2, lwgeom1->srid, DIST_MIN);

  if (lwgeom_is_empty(theline))
    return NULL;

  result = geometry_serialize(theline);

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
  double mindist;
  LWGEOM *lwgeom1 = lwgeom_from_gserialized(geom1);
  LWGEOM *lwgeom2 = lwgeom_from_gserialized(geom2);
  mindist = lwgeom_mindistance2d(lwgeom1, lwgeom2);
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
  double mindist;
  LWGEOM *lwgeom1 = lwgeom_from_gserialized(geom1);
  LWGEOM *lwgeom2 = lwgeom_from_gserialized(geom2);
  mindist = lwgeom_mindistance3d(lwgeom1, lwgeom2);
  lwgeom_free(lwgeom1);
  lwgeom_free(lwgeom2);
  /* if called with empty geometries the ingoing mindistance is untouched,
   * and makes us return NULL */
  if (mindist < FLT_MAX)
    return mindist;
  return -1;
}

/**
 * @brief Reverse vertex order of geometry
 * @note PostGIS function: Datum LWGEOM_reverse(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
PGIS_LWGEOM_reverse(GSERIALIZED *geom)
{
  LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
  lwgeom_reverse_in_place(lwgeom);
  GSERIALIZED *result = geometry_serialize(lwgeom);
  return result;
}

/**
 * @brief Return true if the two 3D geometries intersect
 * @note PostGIS function: Datum LWGEOM_reverse(PG_FUNCTION_ARGS)
 */
bool
PGIS_ST_3DIntersects(GSERIALIZED *geom1, GSERIALIZED *geom2)
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
 * @brief Return the azimuth between the two geometries
 * @note PostGIS function: Datum LWGEOM_azimuth(PG_FUNCTION_ARGS)
 */
bool
PGIS_LWGEOM_azimuth(GSERIALIZED *geom1, GSERIALIZED *geom2, double *result)
{
  LWPOINT *lwpoint;
  POINT2D p1, p2;
  int32_t srid;

  /* Extract first point */
  lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(geom1));
  if (! lwpoint)
    elog(ERROR, "Argument must be POINT geometries");
  srid = lwpoint->srid;
  if (! getPoint2d_p(lwpoint->point, 0, &p1))
    elog(ERROR, "Error extracting point");
  lwpoint_free(lwpoint);

  /* Extract second point */
  lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(geom2));
  if (! lwpoint)
    elog(ERROR, "Argument must be POINT geometries");
  if (lwpoint->srid != srid)
    elog(ERROR, "Operation on mixed SRID geometries");
  if (! getPoint2d_p(lwpoint->point, 0, &p2))
    elog(ERROR, "Error extracting point");
  lwpoint_free(lwpoint);

  /* Standard return value for equality case */
  if ((p1.x == p2.x) && (p1.y == p2.y))
    return false;

  /* Compute azimuth */
  if (! azimuth_pt_pt(&p1, &p2, result))
    return false;

  return true;
}

/*****************************************************************************
 * Functions adapted from lwgeom_functions_lrs.c
 *****************************************************************************/

/**
 * @brief Return the fraction in [0,1] where the point is located in the line
 * @note PostGIS function: Datum LWGEOM_line_locate_point(PG_FUNCTION_ARGS)
 */
double
PGIS_LWGEOM_line_locate_point(GSERIALIZED *geom1, GSERIALIZED *geom2)
{
  ensure_same_srid(gserialized_get_srid(geom1), gserialized_get_srid(geom2));
  if ( gserialized_get_type(geom1) != LINETYPE )
    elog(ERROR,"line_locate_point: 1st arg isn't a line");
  if ( gserialized_get_type(geom2) != POINTTYPE )
    elog(ERROR,"line_locate_point: 2st arg isn't a point");

  LWLINE *lwline = lwgeom_as_lwline(lwgeom_from_gserialized(geom1));
  LWPOINT *lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(geom2));
  POINTARRAY *pa = lwline->points;
  POINT4D p, p_proj;
  lwpoint_getPoint4d_p(lwpoint, &p);
  double result = ptarray_locate_point(pa, &p, NULL, &p_proj);
  return result;
}

/*****************************************************************************
 * Functions adapted from lwgeom_functions_analytic.c
 *****************************************************************************/

/**
 * @brief Extract a line fraction from a line and two doubles in [0,1]
 * @note PostGIS function: Datum LWGEOM_line_substring(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
PGIS_LWGEOM_line_substring(GSERIALIZED *geom, double from, double to)
{
  LWGEOM *olwgeom;
  POINTARRAY *ipa, *opa;
  GSERIALIZED *result;
  int type = gserialized_get_type(geom);

  if ( from < 0 || from > 1 )
  {
    elog(ERROR,"line_interpolate_point: 2nd arg isn't within [0,1]");
    return NULL;
  }
  if ( to < 0 || to > 1 )
  {
    elog(ERROR,"line_interpolate_point: 3rd arg isn't within [0,1]");
    return NULL;
  }
  if ( from > to )
  {
    elog(ERROR, "2nd arg must be smaller then 3rd arg");
    return NULL;
  }

  if ( type == LINETYPE )
  {
    LWLINE *iline = lwgeom_as_lwline(lwgeom_from_gserialized(geom));

    if ( lwgeom_is_empty((LWGEOM*)iline) )
    {
      /* TODO return empty line */
      lwline_release(iline);
      return NULL;
    }

    ipa = iline->points;

    opa = ptarray_substring(ipa, from, to, 0);

    if ( opa->npoints == 1 ) /* Point returned */
      olwgeom = (LWGEOM *)lwpoint_construct(iline->srid, NULL, opa);
    else
      olwgeom = (LWGEOM *)lwline_construct(iline->srid, NULL, opa);

  }
  else if ( type == MULTILINETYPE )
  {
    LWMLINE *iline;
    uint32_t i = 0, g = 0;
    int homogeneous = LW_TRUE;
    LWGEOM **geoms = NULL;
    double length = 0.0, sublength = 0.0, minprop = 0.0, maxprop = 0.0;

    iline = lwgeom_as_lwmline(lwgeom_from_gserialized(geom));

    if ( lwgeom_is_empty((LWGEOM*)iline) )
    {
      /* TODO return empty collection */
      lwmline_release(iline);
      return NULL;
    }

    /* Calculate the total length of the mline */
    for ( i = 0; i < iline->ngeoms; i++ )
    {
      LWLINE *subline = (LWLINE*)iline->geoms[i];
      if ( subline->points && subline->points->npoints > 1 )
        length += ptarray_length_2d(subline->points);
    }

    geoms = lwalloc(sizeof(LWGEOM*) * iline->ngeoms);

    /* Slice each sub-geometry of the multiline */
    for ( i = 0; i < iline->ngeoms; i++ )
    {
      LWLINE *subline = (LWLINE*)iline->geoms[i];
      double subfrom = 0.0, subto = 0.0;

      if ( subline->points && subline->points->npoints > 1 )
        sublength += ptarray_length_2d(subline->points);

      /* Calculate proportions for this subline */
      minprop = maxprop;
      maxprop = sublength / length;

      /* This subline doesn't reach the lowest proportion requested
         or is beyond the highest proporton */
      if ( from > maxprop || to < minprop )
        continue;

      if ( from <= minprop )
        subfrom = 0.0;
      if ( to >= maxprop )
        subto = 1.0;

      if ( from > minprop && from <= maxprop )
        subfrom = (from - minprop) / (maxprop - minprop);

      if ( to < maxprop && to >= minprop )
        subto = (to - minprop) / (maxprop - minprop);


      opa = ptarray_substring(subline->points, subfrom, subto, 0);
      if ( opa && opa->npoints > 0 )
      {
        if ( opa->npoints == 1 ) /* Point returned */
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
    if ( ! homogeneous )
      type = COLLECTIONTYPE;

    olwgeom = (LWGEOM*)lwcollection_construct(type, iline->srid, NULL, g, geoms);
  }
  else
  {
    elog(ERROR,"line_substring: 1st arg isn't a line");
    return NULL;
  }

  result = geometry_serialize(olwgeom);
  lwgeom_free(olwgeom);
  return result;

}

/**
 * @brief Extract a line fraction from a line and two doubles in [0,1]
 * @note PostGIS function: Datum LWGEOM_line_interpolate_point(PG_FUNCTION_ARGS)
 * @note With respect to the original function we do not use the repeat
 * argument
 */
GSERIALIZED *
PGIS_LWGEOM_line_interpolate_point(GSERIALIZED *gser, double distance_fraction)
{
  GSERIALIZED *result;
  int repeat = 0;
  int32_t srid = gserialized_get_srid(gser);
  LWLINE* lwline;
  LWGEOM* lwresult;
  POINTARRAY* opa;

  if ( distance_fraction < 0 || distance_fraction > 1 )
  {
    elog(ERROR,"line_interpolate_point: 2nd arg isn't within [0,1]");
  }

  if ( gserialized_get_type(gser) != LINETYPE )
  {
    elog(ERROR,"line_interpolate_point: 1st arg isn't a line");
  }

  lwline = lwgeom_as_lwline(lwgeom_from_gserialized(gser));
  opa = lwline_interpolate_points(lwline, distance_fraction, repeat);

  lwgeom_free(lwline_as_lwgeom(lwline));

  if (opa->npoints <= 1)
  {
    lwresult = lwpoint_as_lwgeom(lwpoint_construct(srid, NULL, opa));
  } else {
    lwresult = lwmpoint_as_lwgeom(lwmpoint_construct(srid, opa));
  }

  result = geometry_serialize(lwresult);
  lwgeom_free(lwresult);

  return result;
}

/*****************************************************************************
 * Functions adapted from lwgeom_geos.c
 *****************************************************************************/

/**
 * @brief Return true if the two 3D geometries intersect
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
  result = geometry_serialize(lwresult);
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
  SPHEROID s;

  /* Get our geometry object loaded into memory. */
  LWGEOM *lwgeom = lwgeom_from_gserialized(g);

  /* EMPTY things have no length */
  if ( lwgeom_is_empty(lwgeom) || lwgeom->type == POLYGONTYPE ||
    lwgeom->type == MULTIPOLYGONTYPE )
  {
    lwgeom_free(lwgeom);
    return 0.0;
  }

  /* Initialize spheroid */
  /* We currently cannot use the following statement since PROJ4 API is not
   * available directly to MobilityDB. */
  // spheroid_init_from_srid(gserialized_get_srid(g), &s);
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

  POSTGIS_DEBUG(2, "Entered function");

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
      gser = geography_serialize(lwpoint_as_lwgeom(empty_point));
    else
      gser = geometry_serialize(lwpoint_as_lwgeom(empty_point));
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
    result = geometry_serialize(lwgeom);
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
    result = geometry_serialize(lwgeom);
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
    result = geometry_serialize(lwgeom);
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

  geom = geometry_serialize(lwgeom);
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
  g_ser = geography_serialize(lwgeom);

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
  if ( (int)lwgeom->srid <= 0 )
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
  GSERIALIZED *result = geography_serialize(lwgeom);
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

  GSERIALIZED *result = geometry_serialize(lwgeom);
  lwgeom_free(lwgeom);
  return result;
}

/*****************************************************************************/

#endif /* POSTGIS_VERSION_NUMBER >= 30000 */