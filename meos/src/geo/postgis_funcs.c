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
 * @brief Functions for geometry/geography types corresponding to external
 * PostGIS functions in order to bypass the function manager in @p fmgr.c
 */

#include "geo/geo_funcs.h"
#include "geo/postgis_funcs.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* GEOS */
#include <geos_c.h>
/* PostgreSQL */
#include <postgres.h>
#include <varatt.h>
#include <pgtypes.h>
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
#include <lwgeom_log.h>
#include <intervaltree.h>
#include <lwgeom_geos.h>
#include <measures.h>
#include <measures3d.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/type_util.h"
#include "geo/geo_poly_clip.h"  /* clip_poly_poly fast-path for polygon ∩/− polygon */
#include "geo/meos_transform.h"
#include "geo/tgeo.h"
#include "geo/tgeo_spatialfuncs.h"

/* Modified version of PG_PARSER_ERROR */
#if MEOS
#define PG_PARSER_ERROR(lwg_parser_result) \
  do { \
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, \
      "%s", lwg_parser_result.message); \
  } while(0);
#else
  #include <lwgeom_pg.h>
#endif /* MEOS */

/* Function not exported in liblwgeom.h */
extern int spheroid_init_from_srid(int32_t srid, SPHEROID *s);

/*****************************************************************************
 * Output functions for GBOX and BOX3D
 * Original MEOS functions
 *****************************************************************************/

#define MAX_LEN_BOX3D    255
#define MAX_LEN_GBOX     255

/**
 * @ingroup meos_geo_base_inout
 * @brief Return a PostGIS GBOX from the arguments
 * @param[in] hasz True if there is a Z dimension
 * @param[in] hasm True if there is a M dimension
 * @param[in] geodetic True if geodetic
 * @param[in] xmin,ymin,zmin,mmin Minimum bounds for the spatial dimensions
 * @param[in] xmax,ymax,zmax,mmax Maximum bounds for the spatial dimensions
 */
GBOX *
gbox_make(bool hasz, bool hasm, bool geodetic, double xmin, double xmax,
  double ymin, double ymax, double zmin, double zmax, double mmin,
  double mmax)
{
  GBOX *result = gbox_new(lwflags(hasz, hasm, geodetic));
  /* Process X min/max */
  result->xmin = Min(xmin, xmax);
  result->xmax = Max(xmin, xmax);
  /* Process Y min/max */
  result->ymin = Min(ymin, ymax);
  result->ymax = Max(ymin, ymax);
  if (hasz)
  {
    /* Process Z min/max */
    result->zmin = Min(zmin, zmax);
    result->zmax = Max(zmin, zmax);
  }
  if (hasm)
  {
    /* Process M min/max */
    result->mmin = Min(mmin, mmax);
    result->mmax = Max(mmin, mmax);
  }
  return result;
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return the string representation of a PostGIS GBOX
 * @param[in] box Box
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
gbox_out(const GBOX *box, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  return gbox_to_string(box);
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return a PostGIS GBOX from its string representation
 * @param[in] str String
 * @details The format is `GBOX((xmin,ymin),(xmax,ymax))` for two dimensions
 * or `GBOX((xmin,ymin,zmin),(xmax,ymax,zmax))` for three dimensions, the
 * inverse of #gbox_out
 */
GBOX *
gbox_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);

  double xmin, ymin, zmin, xmax, ymax, zmax;
  if (sscanf(str, " GBOX((%lf,%lf,%lf),(%lf,%lf,%lf))",
      &xmin, &ymin, &zmin, &xmax, &ymax, &zmax) == 6)
    return gbox_make(true, false, false, xmin, xmax, ymin, ymax, zmin, zmax,
      0, 0);
  if (sscanf(str, " GBOX((%lf,%lf),(%lf,%lf))",
      &xmin, &ymin, &xmax, &ymax) == 4)
    return gbox_make(false, false, false, xmin, xmax, ymin, ymax, 0, 0, 0, 0);
  meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
    "Could not parse GBOX value: %s", str);
  return NULL;
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return a PostGIS BOX3D from the arguments
 * @param[in] xmin,ymin,zmin Minimum bounds for the spatial dimension
 * @param[in] xmax,ymax,zmax Maximum bounds for the spatial dimension
 * @param[in] srid SRID
 */
BOX3D *
box3d_make(double xmin, double xmax, double ymin, double ymax, double zmin,
  double zmax, int32_t srid)
{
  /* Ensure the validity of the arguments */
  if (srid == SRID_INVALID)
    return NULL;

  /* Note: zero-fill is required here, just as in heap tuples */
  BOX3D *result = palloc0(sizeof(BOX3D));
  /* Process X min/max */
  result->xmin = Min(xmin, xmax);
  result->xmax = Max(xmin, xmax);
  /* Process Y min/max */
  result->ymin = Min(ymin, ymax);
  result->ymax = Max(ymin, ymax);
  /* Process Z min/max */
  result->zmin = Min(zmin, zmax);
  result->zmax = Max(zmin, zmax);
  /* Process SRID */
  result->srid = srid;
  return result;
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return the string representation of a PostGIS BOX3D
 * @param[in] box Box
 * @param[in] maxdd Maximum number of decimal digits
 * @details The format is `BOX3D(xmin ymin zmin,xmax ymax zmax)`, the one
 * PostGIS gives the @p box3d type it declares, so that a value printed here
 * is read back by a PostgreSQL session and the other way round. As PostGIS
 * does, the Z ordinates are always printed and the SRID never is. PostGIS
 * prints the ordinates with at most #OUT_DEFAULT_DECIMAL_DIGITS decimal
 * digits, which is thus the value of @p maxdd that reproduces its output
 * character for character
 */
char *
box3d_out(const BOX3D *box, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  /* Six ordinates of at most #OUT_MAX_BYTES_DOUBLE characters each, the
   * `BOX3D(` prefix, the five separators, the closing parenthesis and the
   * terminating null all fit in #MAX_LEN_BOX3D + 1 characters */
  char buf[MAX_LEN_BOX3D + 1];
  int len = 6;
  memcpy(buf, "BOX3D(", 6);
  len += lwprint_double(box->xmin, maxdd, buf + len);
  buf[len++] = ' ';
  len += lwprint_double(box->ymin, maxdd, buf + len);
  buf[len++] = ' ';
  len += lwprint_double(box->zmin, maxdd, buf + len);
  buf[len++] = ',';
  len += lwprint_double(box->xmax, maxdd, buf + len);
  buf[len++] = ' ';
  len += lwprint_double(box->ymax, maxdd, buf + len);
  buf[len++] = ' ';
  len += lwprint_double(box->zmax, maxdd, buf + len);
  buf[len++] = ')';
  buf[len] = '\0';
  return strdup(buf);
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return a PostGIS BOX3D from its string representation
 * @param[in] str String
 * @details The format is `BOX3D(xmin ymin zmin,xmax ymax zmax)`, the inverse
 * of #box3d_out and the one PostGIS gives the @p box3d type it declares. As
 * PostGIS does, the Z ordinates may be left out, as in
 * `BOX3D(xmin ymin,xmax ymax)`, in which case they are zero, and the SRID of
 * the result is always unknown
 * @note The format `[SRID=#;]BOX3D((xmin,ymin,zmin),(xmax,ymax,zmax))` written
 * by previous versions of #box3d_out is still accepted, so that a value stored
 * by them can still be read. The two formats are told apart by the character
 * following `BOX3D(`, which is an opening parenthesis in the latter only
 */
BOX3D *
box3d_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);

  double xmin, ymin, zmin, xmax, ymax, zmax;
  /* Format written by PostGIS, with the Z ordinates omitted or not */
  if (sscanf(str, " BOX3D(%lf %lf %lf ,%lf %lf %lf)",
      &xmin, &ymin, &zmin, &xmax, &ymax, &zmax) == 6)
    return box3d_make(xmin, xmax, ymin, ymax, zmin, zmax, SRID_UNKNOWN);
  if (sscanf(str, " BOX3D(%lf %lf ,%lf %lf)", &xmin, &ymin, &xmax, &ymax) == 4)
    return box3d_make(xmin, xmax, ymin, ymax, 0.0, 0.0, SRID_UNKNOWN);

  /* Format written by previous versions of #box3d_out */
  int32_t srid = SRID_UNKNOWN;
  const char *ptr = str;
  /* Optional "SRID=#;" prefix */
  if (pg_strncasecmp(ptr, "SRID=", 5) == 0)
  {
    char *end;
    srid = (int32) strtol(ptr + 5, &end, 10);
    if (end == ptr + 5 || *end != ';')
    {
      meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
        "Could not parse BOX3D value: %s", str);
      return NULL;
    }
    ptr = end + 1;
  }
  if (sscanf(ptr, " BOX3D((%lf,%lf,%lf),(%lf,%lf,%lf))",
      &xmin, &ymin, &zmin, &xmax, &ymax, &zmax) != 6)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse BOX3D value: %s", str);
    return NULL;
  }
  return box3d_make(xmin, xmax, ymin, ymax, zmin, zmax, srid);
}

/*****************************************************************************
 * Interval tree functions
 * Functions copied from /postgis/lwgeom_itree.c
 *****************************************************************************/

/**
 * @brief A point must be fully inside (not on boundary) of
 * a polygon to be contained. A multipoint must have
 * at least one fully contained member and no members
 * outside the polygon to be contained.
 */
bool itree_pip_contains(const IntervalTree *itree, const LWGEOM *lwpoints)
{
  if (lwgeom_get_type(lwpoints) == POINTTYPE)
  {
    return itree_point_in_multipolygon(itree, lwgeom_as_lwpoint(lwpoints)) == ITREE_INSIDE;
  }
  else if (lwgeom_get_type(lwpoints) == MULTIPOINTTYPE)
  {
    bool found_completely_inside = false;
    LWMPOINT *mpoint = lwgeom_as_lwmpoint(lwpoints);
    for (uint32_t i = 0; i < mpoint->ngeoms; i++)
    {
      IntervalTreeResult pip_result;
      const LWPOINT* pt = mpoint->geoms[i];

      if (lwpoint_is_empty(pt))
        continue;
      /*
       * We need to find at least one point that's completely inside the
       * polygons (pip_result == 1).  As long as we have one point that's
       * completely inside, we can have as many as we want on the boundary
       * itself. (pip_result == 0)
       */
      pip_result = itree_point_in_multipolygon(itree, pt);

      if (pip_result == ITREE_INSIDE)
        found_completely_inside = true;

      if (pip_result == ITREE_OUTSIDE)
        return false;
    }
    return found_completely_inside;
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "%s got a non-point input", __func__);
    return false;
  }
}

/**
 * @brief If any point in the point/multipoint is outside
 * the polygon, then the polygon does not cover the point/multipoint.
 */
bool itree_pip_covers(const IntervalTree *itree, const LWGEOM *lwpoints)
{
  if (lwgeom_get_type(lwpoints) == POINTTYPE)
  {
    return itree_point_in_multipolygon(itree, lwgeom_as_lwpoint(lwpoints)) != ITREE_OUTSIDE;
  }
  else if (lwgeom_get_type(lwpoints) == MULTIPOINTTYPE)
  {
    LWMPOINT* mpoint = lwgeom_as_lwmpoint(lwpoints);
    for (uint32_t i = 0; i < mpoint->ngeoms; i++)
    {
      const LWPOINT *pt = mpoint->geoms[i];

      if (lwpoint_is_empty(pt))
        continue;

      if (itree_point_in_multipolygon(itree, pt) == ITREE_OUTSIDE)
        return false;
    }
    return true;
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "%s got a non-point input", __func__);
    return false;
  }
}

/**
 * @brief A.touches(B) implies that the point/multipoint meets the polygons
 * without any member reaching their interior, that is, at least one member
 * lies on the boundary and none inside.
 */
bool itree_pip_touches(const IntervalTree *itree, const LWGEOM *lwpoints)
{
  if (lwgeom_get_type(lwpoints) == POINTTYPE)
  {
    return itree_point_in_multipolygon(itree, lwgeom_as_lwpoint(lwpoints)) ==
      ITREE_BOUNDARY;
  }
  else if (lwgeom_get_type(lwpoints) == MULTIPOINTTYPE)
  {
    bool found_boundary = false;
    LWMPOINT *mpoint = lwgeom_as_lwmpoint(lwpoints);
    for (uint32_t i = 0; i < mpoint->ngeoms; i++)
    {
      const LWPOINT *pt = mpoint->geoms[i];

      if (lwpoint_is_empty(pt))
        continue;

      /*
       * A member inside the polygons makes the interiors meet, which no later
       * member can undo, so the answer is settled. A member on the boundary
       * supplies the required contact, and one outside contributes nothing.
       */
      IntervalTreeResult pip_result = itree_point_in_multipolygon(itree, pt);

      if (pip_result == ITREE_INSIDE)
        return false;

      if (pip_result == ITREE_BOUNDARY)
        found_boundary = true;
    }
    return found_boundary;
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "%s got a non-point input", __func__);
    return false;
  }
}

/**
 * @brief A.intersects(B) implies if any member of the point/multipoint
 * is not outside, then they intersect.
 */
bool itree_pip_intersects(const IntervalTree *itree, const LWGEOM *lwpoints)
{
  if (lwgeom_get_type(lwpoints) == POINTTYPE)
  {
    return itree_point_in_multipolygon(itree, lwgeom_as_lwpoint(lwpoints)) != ITREE_OUTSIDE;
  }
  else if (lwgeom_get_type(lwpoints) == MULTIPOINTTYPE)
  {
    LWMPOINT* mpoint = lwgeom_as_lwmpoint(lwpoints);
    for (uint32_t i = 0; i < mpoint->ngeoms; i++)
    {
      const LWPOINT *pt = mpoint->geoms[i];

      if (lwpoint_is_empty(pt))
        continue;

      if (itree_point_in_multipolygon(itree, pt) != ITREE_OUTSIDE)
        return true;
    }
    return false;
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "%s got a non-point input", __func__);
    return false;
  }
}

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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);

  GSERIALIZED *result = palloc(VARSIZE(gs));
  memcpy(result, gs, VARSIZE(gs));
  return result;
}

/**
 * @ingroup meos_geo_base_constructor
 * @brief Return a 2D geometry point constructed from the arguments
 */
GSERIALIZED *
geompoint_make2d(int32_t srid, double x, double y)
{
  /* Ensure the validity of the arguments */
  if (srid == SRID_INVALID)
    return NULL;

  LWPOINT *point = lwpoint_make2d(srid, x, y);
  GSERIALIZED *result = geo_serialize((LWGEOM *) point);
  lwpoint_free(point);
  return result;
}

/**
 * @ingroup meos_geo_base_constructor
 * @brief Return a 2D geography point constructed from the arguments
 */
GSERIALIZED *
geogpoint_make2d(int32_t srid, double x, double y)
{
  /* Ensure the validity of the arguments */
  if (srid == SRID_INVALID)
    return NULL;

  LWPOINT *point = lwpoint_make2d(srid, x, y);
  FLAGS_SET_GEODETIC(point->flags, true);
  GSERIALIZED *result = geo_serialize((LWGEOM *) point);
  lwpoint_free(point);
  return result;
}

/**
 * @ingroup meos_geo_base_constructor
 * @brief Return a 3DZ geometry point constructed from the arguments
 */
GSERIALIZED *
geompoint_make3dz(int32_t srid, double x, double y, double z)
{
  /* Ensure the validity of the arguments */
  if (srid == SRID_INVALID)
    return NULL;

  LWPOINT *point = lwpoint_make3dz(srid, x, y, z);
  GSERIALIZED *result = geo_serialize((LWGEOM *) point);
  lwpoint_free(point);
  return result;
}

/**
 * @ingroup meos_geo_base_constructor
 * @brief Return a 3DZ geography point constructed from the arguments
 */
GSERIALIZED *
geogpoint_make3dz(int32_t srid, double x, double y, double z)
{
  /* Ensure the validity of the arguments */
  if (srid == SRID_INVALID)
    return NULL;

  LWPOINT *point = lwpoint_make3dz(srid, x, y, z);
  FLAGS_SET_GEODETIC(point->flags, true);
  GSERIALIZED *result = geo_serialize((LWGEOM *) point);
  lwpoint_free(point);
  return result;
}

/*****************************************************************************
 * Functions borrowed from gserialized.c
 *****************************************************************************/

/**
 * @ingroup meos_geo_base_srid
 * @brief Get the SRID of a geometry/geography
 * @param[in] gs Geometry
 */
int32_t
geo_srid(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, SRID_INVALID);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, false);
  return gserialized_is_empty(gs);
}

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

  /* The point array below belongs to the point or the line that is built from
   * it, and the polygon and polyhedron arms build their own through
   * lwpoly_construct_rectangle. Constructing it for every arm left it owned by
   * nobody in the four that do not take it */

  /* BOX3D is a point */
  if ((box->xmin == box->xmax) && (box->ymin == box->ymax) &&
      (box->zmin == box->zmax))
  {
    POINTARRAY *pa = ptarray_construct_empty(LW_TRUE, LW_FALSE, 5);
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
    POINTARRAY *pa = ptarray_construct_empty(LW_TRUE, LW_FALSE, 5);
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
    lwpoly = lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[0],
      &points[1], &points[2], &points[3]);
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
    lwpoly = lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[0],
      &points[1], &points[2], &points[3]);
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
    lwpoly = lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[0],
      &points[1], &points[2], &points[3]);
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
      lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[0], &points[1],
        &points[2], &points[3]));
    /* add top polygon */
    geoms[1] = lwpoly_as_lwgeom(
      lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[4], &points[7],
        &points[6], &points[5]));
    /* add left polygon */
    geoms[2] = lwpoly_as_lwgeom(
      lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[0], &points[4],
        &points[5], &points[1]));
    /* add right polygon */
    geoms[3] = lwpoly_as_lwgeom(
      lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[3], &points[2],
        &points[6], &points[7]));
    /* add front polygon */
    geoms[4] = lwpoly_as_lwgeom(
      lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[0], &points[3],
        &points[7], &points[4]));
    /* add back polygon */
    geoms[5] = lwpoly_as_lwgeom(
      lwpoly_construct_rectangle(LW_TRUE, LW_FALSE, &points[1], &points[5],
        &points[6], &points[2]));

    result = (LWGEOM *) lwcollection_construct(POLYHEDRALSURFACETYPE,
      SRID_UNKNOWN, NULL, ngeoms, geoms);
    FLAGS_SET_SOLID(result->flags, 1);
  }

  lwgeom_set_srid(result, box->srid);
  return result;
}

/*****************************************************************************
 * Functions adapted from lwgeom_functions_basic.c
 *****************************************************************************/

/**
 * @ingroup meos_geo_base_accessor
 * @note Derived from PostGIS function: @p lwgeom_is_unitary(const LWGEOM *geom)
 */
bool
geo_is_unitary(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, false);
  if (gserialized_is_empty(gs))
    return false;

  switch (gserialized_get_type(gs))
  {
    case POINTTYPE:
    case LINETYPE:
    case POLYGONTYPE:
    case CURVEPOLYTYPE:
    case COMPOUNDTYPE:
    case CIRCSTRINGTYPE:
    case TRIANGLETYPE:
      return true;
    default:
      return false;
  }
}

/**
 * @ingroup meos_geo_base_accessor
 * @brief Return the area of a geometry
 * @details Defined by
 *   - area(point) = 0
 *   - area(line) = 0
 *   - area(polygon) = the area it encloses, its holes taken out
 *
 * Uses Euclidean 2D computation even if input is 3D.
 * @param[in] gs Geometry
 * @return On error return @p DBL_MAX
 * @note An empty geometry encloses nothing, so its area is 0. The question has
 * an answer, and #lwgeom_area gives it
 * @note PostGIS function: @p ST_Area(PG_FUNCTION_ARGS)
 */
double
geom_area(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, DBL_MAX);
  if (! ensure_not_geodetic_geo(gs))
    return DBL_MAX;

  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  double area = lwgeom_area(lwgeom);
  lwgeom_free(lwgeom);
  return area;
}

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
 * @return On error return @p DBL_MAX
 * @note An empty geometry draws no line, so its length is 0. The question has
 * an answer, and #lwgeom_length gives it
 * @note PostGIS function: @p LWGEOM_length_linestring(PG_FUNCTION_ARGS)
 */
double
geom_length(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, DBL_MAX);
  if (! ensure_not_geodetic_geo(gs))
    return DBL_MAX;

  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  double dist = lwgeom_length(lwgeom);
  lwgeom_free(lwgeom);
  return dist;
}

/**
 * @ingroup meos_geo_base_accessor
 * @brief Return the perimeter of a geometry
 * @details Defined by
 *   - perimeter(point) = 0
 *   - perimeter(line) = 0
 *   - perimeter(polygon) = sum of ring perimeters
 * Uses Euclidian 2D computation even if input is 3D
 * @param[in] gs Geometry
 * @return On error return @p DBL_MAX
 * @note An empty geometry bounds no area, so its perimeter is 0. The question
 * has an answer, and #lwgeom_perimeter_2d gives it
 * @note PostGIS function: @p LWGEOM_perimeter2d_poly(PG_FUNCTION_ARGS)
 */
double
geom_perimeter(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, DBL_MAX);
  if (! ensure_not_geodetic_geo(gs))
    return DBL_MAX;

  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  double perimeter = lwgeom_perimeter_2d(lwgeom);
  lwgeom_free(lwgeom);
  return perimeter;
}

/**
 * @brief Return the boundary of a compound curve, that is, its two end points,
 * or an empty multipoint if the curve is closed
 * @details The embedded PostGIS @p lwgeom_boundary does not handle
 * @p COMPOUNDTYPE (it errors on it), while it does handle circular strings; the
 * boundary is computed here from the first point of the first component and the
 * last point of the last component, mirroring the linestring/circularstring case
 */
static LWGEOM *
lwcompound_boundary(const LWGEOM *geom)
{
  int32_t srid = lwgeom_get_srid(geom);
  uint8_t hasz = lwgeom_has_z(geom);
  uint8_t hasm = lwgeom_has_m(geom);
  if (lwgeom_is_closed(geom) || lwgeom_is_empty(geom))
    return (LWGEOM *) lwmpoint_construct_empty(srid, hasz, hasm);
  /* A compound curve is a chain of (circular) lines sharing their end points */
  const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
  const LWLINE *first = (const LWLINE *) col->geoms[0];
  const LWLINE *last = (const LWLINE *) col->geoms[col->ngeoms - 1];
  LWMPOINT *result = lwmpoint_construct_empty(srid, hasz, hasm);
  POINT4D pt;
  getPoint4d_p(first->points, 0, &pt);
  lwmpoint_add_lwpoint(result, lwpoint_make(srid, hasz, hasm, &pt));
  getPoint4d_p(last->points, last->points->npoints - 1, &pt);
  lwmpoint_add_lwpoint(result, lwpoint_make(srid, hasz, hasm, &pt));
  return (LWGEOM *) result;
}

/**
 * @brief Return the boundary of a multisurface, that is, the union of the
 * boundaries of its member surfaces
 * @details The embedded PostGIS @p lwgeom_boundary does not handle
 * @p MULTISURFACETYPE (it errors on it), while it does handle its member curve
 * polygons; the boundaries of the members are collected and homogenized,
 * mirroring the multipolygon case
 */
static LWGEOM *
lwmsurface_boundary(const LWGEOM *geom)
{
  int32_t srid = lwgeom_get_srid(geom);
  uint8_t hasz = lwgeom_has_z(geom);
  uint8_t hasm = lwgeom_has_m(geom);
  const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
  LWCOLLECTION *result =
    lwcollection_construct_empty(COLLECTIONTYPE, srid, hasz, hasm);
  for (uint32_t i = 0; i < col->ngeoms; i++)
    result = lwcollection_add_lwgeom(result, lwgeom_boundary(col->geoms[i]));
  LWGEOM *lwout = lwgeom_homogenize((LWGEOM *) result);
  lwgeom_free((LWGEOM *) result);
  return lwout;
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (gserialized_is_empty(gs) || ! ensure_not_geodetic_geo(gs))
    return NULL;

  /* Empty.Boundary() == Empty, but of other dimension, so can't shortcut */
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  /* The embedded PostGIS lwgeom_boundary does not support compound curves or
   * multisurfaces; compute their boundary natively */
  LWGEOM *lwresult;
  if (geom->type == COMPOUNDTYPE)
    lwresult = lwcompound_boundary(geom);
  else if (geom->type == MULTISURFACETYPE)
    lwresult = lwmsurface_boundary(geom);
  else
    lwresult = lwgeom_boundary(geom);
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
  /* Ensure the validity of the arguments */
  if (! ensure_valid_geo_geo(gs1, gs2) || ! ensure_not_geodetic_geo(gs1) ||
      gserialized_is_empty(gs1) || gserialized_is_empty(gs2))
    return NULL;

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  LWGEOM *line = lwgeom_closest_line(geom1, geom2);
  lwgeom_free(geom1); lwgeom_free(geom2);
  GSERIALIZED *result = NULL;
  if (! lwgeom_is_empty(line))
    result = geo_serialize(line);
  lwgeom_free(line);
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
  /* Ensure the validity of the arguments */
  if (! ensure_valid_geo_geo(gs1, gs2) || ! ensure_not_geodetic_geo(gs1) ||
      gserialized_is_empty(gs1) || gserialized_is_empty(gs2))
    return NULL;

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
 * @ingroup meos_geo_base_dist
 * @brief Return the distance between two geometries
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p ST_Distance(PG_FUNCTION_ARGS)
 * @return On error or empty geometries return DBL_MAX
 */
double
geom_distance2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  /* Ensure the validity of the arguments */
  /* The Z dimension is not verified: this is the function selected for mixed
   * 2D/3D arguments, which are answered on their common dimensions */
  if (! ensure_valid_geo_geo(gs1, gs2) || ! ensure_not_geodetic_geo(gs1) ||
      gserialized_is_empty(gs1) || gserialized_is_empty(gs2))
    return DBL_MAX;

  /* Fast path: the distance between two points is computed from their
   * coordinates with the primitive that the general computation reaches, which
   * avoids deserializing the geometries. This keeps the deserialization out of
   * the temporal-distance hot path, where the lifting loop computes the
   * distance for every synchronized pair of instants */
  if (gserialized_get_type(gs1) == POINTTYPE &&
      gserialized_get_type(gs2) == POINTTYPE)
  {
    DISTPTS dl;
    lw_dist2d_distpts_init(&dl, DIST_MIN);
    lw_dist2d_pt_pt(GSERIALIZED_POINT2D_P(gs1), GSERIALIZED_POINT2D_P(gs2),
      &dl);
    return dl.distance;
  }

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  double mindist = lwgeom_mindistance2d(geom1, geom2);
  lwgeom_free(geom1);
  lwgeom_free(geom2);
  return mindist;
}

/**
 * @ingroup meos_geo_base_dist
 * @brief Return the maximum distance between two geometries
 * @details The maximum distance is the distance between the two points, one
 * on each geometry, that are farthest from each other
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p ST_MaxDistance(PG_FUNCTION_ARGS)
 * @note A geometry carrying a circular arc is not supported, since the
 * underlying computation implements the maximum only for straight edges
 * @return On error, on empty geometries, or on a geometry carrying a circular
 * arc return DBL_MAX
 */
double
geom_max_distance2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_geo_geo(gs1, gs2) || ! ensure_not_geodetic_geo(gs1) ||
      gserialized_is_empty(gs1) || gserialized_is_empty(gs2))
    return DBL_MAX;

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  double maxdist = (lwgeom_has_arc(geom1) || lwgeom_has_arc(geom2)) ?
    DBL_MAX : lwgeom_maxdistance2d(geom1, geom2);
  lwgeom_free(geom1);
  lwgeom_free(geom2);
  return maxdist;
}

/**
 * @ingroup meos_geo_base_dist
 * @brief Return the 3D distance between two geometries
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p ST_3DDistance(PG_FUNCTION_ARGS)
 * @return On error or empty geometries return DBL_MAX
 */
double
geom_distance3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  /* Ensure the validity of the arguments */
  /* The Z dimension is not verified: PostGIS function
   * #lwgeom_mindistance3d_tolerance reduces to its 2D counterpart when one of
   * the geometries has no Z, regarding the missing value as any value */
  if (! ensure_valid_geo_geo(gs1, gs2) || ! ensure_not_geodetic_geo(gs1) ||
      gserialized_is_empty(gs1) || gserialized_is_empty(gs2))
    return DBL_MAX;

  /* Fast path: the distance between two points with Z is computed from their
   * coordinates with the primitive that the general computation reaches, which
   * avoids deserializing the geometries */
  if (gserialized_get_type(gs1) == POINTTYPE && 
      gserialized_get_type(gs2) == POINTTYPE)
  {
    DISTPTS3D dl;
    memset(&dl, 0, sizeof(DISTPTS3D));
    dl.mode = DIST_MIN;
    dl.distance = DBL_MAX;
    dl.tolerance = 0.0;
    dl.twisted = -1;
    lw_dist3d_pt_pt(GSERIALIZED_POINT3DZ_P(gs1), GSERIALIZED_POINT3DZ_P(gs2),
      &dl);
    return dl.distance;
  }

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  double mindist = lwgeom_mindistance3d(geom1, geom2);
  lwgeom_free(geom1); lwgeom_free(geom2);
  return mindist;
}

/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if the 3D geometries intersect
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p ST_3DIntersects(PG_FUNCTION_ARGS)
 */
bool
geom_intersects3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  /* Ensure the validity of the arguments */
  /* The Z dimension is not verified: PostGIS function
   * #lwgeom_mindistance3d_tolerance reduces to its 2D counterpart when one of
   * the geometries has no Z, regarding the missing value as any value */
  if (! ensure_valid_geo_geo(gs1, gs2) || ! ensure_not_geodetic_geo(gs1) ||
      gserialized_is_empty(gs1) || gserialized_is_empty(gs2))
    return false;

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  double mindist = lwgeom_mindistance3d_tolerance(geom1, geom2, 0.0);
  lwgeom_free(geom1); lwgeom_free(geom2);
  /* empty geometries cases should be right handled since return from
     underlying functions should be DBL_MAX which causes false as answer */
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
  /* Ensure the validity of the arguments */
  /* The Z dimension is not verified: this is the function selected for mixed
   * 2D/3D arguments, which are answered on their common dimensions */
  if (! ensure_valid_geo_geo(gs1, gs2) ||
      gserialized_is_empty(gs1) || gserialized_is_empty(gs2) ||
      ! ensure_not_negative_datum(Float8GetDatum(tolerance), T_FLOAT8))
    return false;

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  double mindist = lwgeom_mindistance2d_tolerance(geom1, geom2, tolerance);
  lwgeom_free(geom1); lwgeom_free(geom2);
  /* empty geometries cases should be right handled since return from
   underlying functions should be DBL_MAX which causes false as answer */
  return (tolerance >= mindist);
}

/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if two geometries are within a distance
 * @details Bare name for the planar (2D) distance-within test, the portable
 * counterpart of @ref geog_dwithin() for geometry; equivalent to PostGIS
 * @p ST_DWithin.
 * @param[in] gs1,gs2 Geometries
 * @param[in] tolerance Tolerance
 * @note PostGIS function: @p LWGEOM_dwithin(PG_FUNCTION_ARGS)
 */
bool
geom_dwithin(const GSERIALIZED *gs1, const GSERIALIZED *gs2, double tolerance)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs1, false);
  return FLAGS_GET_Z(gs1->gflags) ?
    geom_dwithin2d(gs1, gs2, tolerance) : geom_dwithin3d(gs1, gs2, tolerance);
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
  /* Ensure the validity of the arguments */
  /* The Z dimension is not verified: PostGIS function
   * #lwgeom_mindistance3d_tolerance reduces to its 2D counterpart when one of
   * the geometries has no Z, regarding the missing value as any value */
  if (! ensure_valid_geo_geo(gs1, gs2) ||
      gserialized_is_empty(gs1) || gserialized_is_empty(gs2) ||
      ! ensure_not_negative_datum(Float8GetDatum(tolerance), T_FLOAT8))
    return false;

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  double mindist = lwgeom_mindistance3d_tolerance(geom1, geom2, tolerance);
  /*empty geometries cases should be right handled since return from underlying
   functions should be DBL_MAX which causes false as answer*/
  lwgeom_free(geom1); lwgeom_free(geom2);
  return (tolerance >= mindist);
}

/**
 * @ingroup meos_geo_base_transf
 * @brief Reverse vertex order of a geometry
 * @param[in] gs Geometry/geography
 * @note PostGIS function: @p LWGEOM_reverse(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geo_reverse(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);

  LWGEOM *geom = lwgeom_from_gserialized(gs);
  lwgeom_reverse_in_place(geom);
  GSERIALIZED *result = geo_serialize(geom);
  lwgeom_free(geom);
  return result;
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
  /* Ensure the validity of the arguments */
  /* The Z dimension is not verified: the azimuth is computed from the 2D
   * coordinates of the points, as PostGIS does */
  if (! ensure_valid_geo_geo(gs1, gs2) || ! ensure_not_null(result) ||
      ! ensure_point_type(gs1) || ! ensure_point_type(gs2) ||
      gserialized_is_empty(gs1) || gserialized_is_empty(gs2))
    return false;

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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gsarr, NULL);
  if (! ensure_positive(nelems))
    return NULL;

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
  /* MEOS: PostGIS function #lwcollection_construct does not set the GEODETIC
   * flag, which must be set BEFORE serialization since the bounding box of a
   * geodetic value is computed on the unit sphere and occupies 6 floats, while
   * the one of a planar value occupies 2 * ndims floats. The merged box above
   * is a planar one and is thus dropped for geodetic values so that a geodetic
   * box is recomputed at serialization */
  bool geodetic = FLAGS_GET_GEODETIC(gsarr[0]->gflags);
  if (geodetic && box)
  {
    pfree(box); box = NULL;
  }
  LWGEOM *outlwg = (LWGEOM *) lwcollection_construct(outtype, srid, box, count,
    lwgeoms);
  FLAGS_SET_GEODETIC(outlwg->flags, geodetic);
  GSERIALIZED *result = geo_serialize(outlwg);
  lwgeom_free(outlwg);
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return a line from an array of geometries/geographies
 * @details Array elements that are not points or linestrings are discarded
 * @param[in] gsarr Array of geometries/geographies
 * @param[in] count Number of elements in the array
 * @note PostGIS function: @p LWGEOM_makeline_garray(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geo_makeline_garray(GSERIALIZED **gsarr, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gsarr, NULL);
  if (! ensure_positive(count))
    return NULL;

  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * count);
  int ngeoms = 0;
  int32_t srid = SRID_UNKNOWN;
  for (int i = 0; i < count; i++)
  {
    uint32_t geotype = gserialized_get_type(gsarr[i]);
    if (geotype != POINTTYPE && geotype != LINETYPE &&
        geotype != MULTIPOINTTYPE)
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
    meos_error(WARNING, MEOS_ERR_INVALID_ARG_VALUE,
      "No points or linestrings in input array");
    for (int i = 0; i < ngeoms; i++)
      lwgeom_free(geoms[i]);
    return NULL;
  }
  LWGEOM *outlwg = (LWGEOM *) lwline_from_lwgeom_array(srid, ngeoms, geoms);
  /* MEOS: PostGIS function #lwline_from_lwgeom_array only propagates the Z and
   * M flags. The GEODETIC flag must be set BEFORE serialization, since the
   * bounding box of a geodetic value is computed on the unit sphere and
   * occupies 6 floats, while the one of a planar value occupies 2 * ndims
   * floats. Setting the flag on the serialized value would thus shift the
   * offset at which the geometry data is read */
  FLAGS_SET_GEODETIC(outlwg->flags, FLAGS_GET_GEODETIC(geoms[0]->flags));
  GSERIALIZED *result = geo_serialize(outlwg);
  /* Clean up and return */
  for (int i = 0; i < ngeoms; i++)
    lwgeom_free(geoms[i]);
  pfree(geoms); lwgeom_free(outlwg);
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return a MultiPoint containing all the coordinates of a geometry
 * @param[in] gs Geometry/geography
 * @note PostGIS function: @p ST_Points(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geo_points(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);

  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  LWMPOINT *res = lwmpoint_from_lwgeom(lwgeom);
  GSERIALIZED *result = geo_serialize(lwmpoint_as_lwgeom(res));
  lwgeom_free(lwgeom); lwmpoint_free(res);
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return a point array containing all the coordinates of a geometry
 * @param[in] gs Geometry/geography
 * @param[out] count Number of values in the resulting array
 * @note Derived from PostGIS function: @p ST_Points(PG_FUNCTION_ARGS)
 */
GSERIALIZED **
geo_pointarr(const GSERIALIZED *gs, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL); VALIDATE_NOT_NULL(count, NULL);

  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  LWMPOINT *res = lwmpoint_from_lwgeom(lwgeom);
  GSERIALIZED **result = palloc(sizeof(GSERIALIZED *) * res->ngeoms);
  for (uint32_t i = 0; i < res->ngeoms; i++)
    result[i] = geo_serialize((LWGEOM *) res->geoms[i]);
  *count = res->ngeoms;
  lwgeom_free(lwgeom);
  lwmpoint_free(res);
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the number of points of a geometry
 * @param[in] gs Geometry/geography
 * @note PostGIS function: @p ST_Points(PG_FUNCTION_ARGS)
 * @return On error return INT_MAX
 */
int
geo_num_points(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, INT_MAX);

  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  int npoints = lwgeom_count_vertices(lwgeom);
  lwgeom_free(lwgeom);
  return(npoints);
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the number of composing geometries of a geometry
 * @param[in] gs Geometry/geography
 * @note PostGIS function: @p LWGEOM_numgeometries_collection(PG_FUNCTION_ARGS)
 * @return On error return INT_MAX
 */
int
geo_num_geos(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, INT_MAX);

  int result = 0;
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  if (lwgeom_is_empty(lwgeom))
    result = 0;
  else if (lwgeom_is_unitary(lwgeom))
    result = 1;
  else
  {
    LWCOLLECTION *col = lwgeom_as_lwcollection(lwgeom);
    result = col->ngeoms;
  }
  lwgeom_free(lwgeom);
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return a copy of the n-th composing geometry of a geometry
 * @param[in] gs Geometry/geography
 * @param[in] n Number (1-based)
 * @note PostGIS function: @p LWGEOM_geometryn_collection(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geo_geo_n(const GSERIALIZED *gs, int n)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);

  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);

  /* Empty returns NULL */
  if (lwgeom_is_empty(lwgeom))
  {
    lwgeom_free(lwgeom);
    return NULL;
  }
  /* Unitary geometries just reflect back */
  if (lwgeom_is_unitary(lwgeom))
  {
    lwgeom_free(lwgeom);
    if (n == 1)
      return geo_copy(gs);
    else
      return NULL;
  }

  LWCOLLECTION *coll = lwgeom_as_lwcollection(lwgeom);
  if (! coll)
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "Unable to handle type %d in ST_GeometryN", lwgeom->type);

  /* Handle out-of-range index value */
  n -= 1;
  if (n < 0 || n >= (int32) coll->ngeoms)
    return NULL;

  LWGEOM *subgeom = coll->geoms[n];
  subgeom->srid = coll->srid;

  /* COMPUTE_BBOX==TAINTING */
  if (coll->bbox)
    lwgeom_add_bbox(subgeom);

  GSERIALIZED *result = geo_serialize(subgeom);
  lwgeom_free(lwgeom);
  return result;
}

/*****************************************************************************
 * Functions adapted from lwgeom_geos.c
 *****************************************************************************/

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the centroid of a geometry
 * @details The answer carries the ordinates of its input: the centroid is a
 * mean, and a mean is taken in every dimension the geometry states. PostGIS
 * answers a bare point in the plane, since the GEOS centroid it delegates to
 * has no ordinates to give back.
 * @note PostGIS function: @p centroid(PG_FUNCTION_ARGS).
 */
GSERIALIZED *
geom_centroid(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_geodetic_geo(gs))
    return NULL;

  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  LWGEOM *lwresult = meos_centroid(lwgeom);
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
  assert(gs);
  int type = gserialized_get_type(gs);
  return (type == POINTTYPE || type == MULTIPOINTTYPE);
}

/**
 * @brief Return true if a geometry is a polygon
 */
static char
gserialized_is_poly(const GSERIALIZED* gs)
{
  assert(gs);
  int type = gserialized_get_type(gs);
  return (type == POLYGONTYPE || type == MULTIPOLYGONTYPE);
}

/**
 * @brief Return -1, 0, or 1 depending on whether a (multi)point is completely
 * outside, on the boundary, or completely inside a (multi)polygon
 * @details The function selects the polygon and the point out of the pair
 * whatever order they arrive in, and always answers in the direction of the
 * polygon: whether the polygon intersects, contains, or covers the point. That
 * is the relationship asked for @p INTERSECTS, which is symmetric, in either
 * order. For @p CONTAINS and @p COVERS the caller must place the polygon
 * first, since the reverse direction is a different question
 * @note This function is based PostGIS function @p pip_short_circuit bypassing
 * the cache
 */
bool
meos_point_in_polygon(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  spatialRel rel)
{
  assert(gs1); assert(gs2);
  assert(rel == INTERSECTS || rel == CONTAINS || rel == COVERS ||
    rel == TOUCHES);
  const GSERIALIZED *gpoly = gserialized_is_poly(gs1) ? gs1 : gs2;
  const GSERIALIZED *gpoint = gserialized_is_point(gs1) ? gs1 : gs2;
  LWGEOM *poly = lwgeom_from_gserialized(gpoly);
  LWGEOM *point = lwgeom_from_gserialized(gpoint);
  IntervalTree *itree = itree_from_lwgeom(poly);
  bool result;
  if (rel == INTERSECTS)
    result = itree_pip_intersects(itree, point);
  else if (rel == CONTAINS)
    result = itree_pip_contains(itree, point);
  else if (rel == COVERS)
    result = itree_pip_covers(itree, point);
  else /* rel == TOUCHES */
    result = itree_pip_touches(itree, point);
  itree_free(itree);
  lwgeom_free(point);
  lwgeom_free(poly);
  return result;
}

#if GEOS
/**
 * @brief Tranform a PostGIS geometry to a GEOS one
 */
GEOSGeometry *
POSTGIS2GEOS(const GSERIALIZED *gs)
{
  assert(gs);
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
#endif /* GEOS */

/**
 * @brief Return the type that keeps a pair of geometries from the native engine
 * @details The native engine answers every type #geom_meos_supported() accepts,
 * which is every type the edge decomposition reaches, and a type it declines is
 * answered by nobody.
 * @param[in] geom1,geom2 Geometries the operation is asked about
 */
static uint8_t
geo_unsupported_type(const LWGEOM *geom1, const LWGEOM *geom2)
{
  return geom_meos_supported(geom1) ? geom2->type : geom1->type;
}

/**
 * @brief Report that an operation has no answer for a geometry type
 * @details The report does not depend on the build carrying GEOS, because a
 * type the edges decline is not handed to GEOS in either build.
 * @param[in] what Noun naming the operation
 * @param[in] type Type the operation has no answer for
 */
static void
geo_error_unsupported_type(const char *what, uint8_t type)
{
  meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
    "The %s of a geometry of type %s is not supported", what,
    lwtype_name(type));
  return;
}

/**
 * @ingroup meos_internal_geo_base_rel
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
  /* Ensure the validity of the arguments */
  if (! ensure_valid_geo_geo(gs1, gs2))
    return false;

  /* A.Intersects(Empty) == FALSE */
  if ( gserialized_is_empty(gs1) || gserialized_is_empty(gs2) )
    return false;

  /*
   * short-circuit 1: if gs2 bounding box does not overlap
   * gs1 bounding box we can return FALSE.
   */
  GBOX box1, box2;
  memset(&box1, 0, sizeof(GBOX));
  memset(&box2, 0, sizeof(GBOX));
  if (gserialized_get_gbox_p(gs1, &box1) && gserialized_get_gbox_p(gs2, &box2))
  {
    if (gbox_overlaps_2d(&box1, &box2) == LW_FALSE)
      return false;
  }

  /*
   * short-circuit 2: if the geoms are a (multi)point and a (multi)polygon,
   * call the meos_point_in_polygon function. That function answers in the
   * direction of the polygon, so it settles INTERSECTS, which is symmetric,
   * with the pair in either order, and CONTAINS and COVERS only when the
   * polygon is the first argument. A (multi)point asked to contain or cover a
   * (multi)polygon is a different question and keeps the general path below.
   */
  bool poly_point = gserialized_is_poly(gs1) && gserialized_is_point(gs2);
  if ((rel == INTERSECTS && (poly_point ||
        (gserialized_is_point(gs1) && gserialized_is_poly(gs2)))) ||
      ((rel == CONTAINS || rel == COVERS) && poly_point))
    return meos_point_in_polygon(gs1, gs2, rel);

  /*
   * short-circuit 3: TOUCHES between a (multi)polygon and a (multi)point is
   * decided by the same point-in-polygon test, since the points touch exactly
   * when at least one lies on the boundary and none inside. Touching is
   * symmetric, so either order is accepted.
   */
  if (rel == TOUCHES && (poly_point ||
      (gserialized_is_point(gs1) && gserialized_is_poly(gs2))))
    return meos_point_in_polygon(gs1, gs2, TOUCHES);

  /* The relationship is read from the edges of the two geometries, so a
   * circular arc is met on its own circle rather than on the chords a
   * linearization would put in its place */
  assert(rel == INTERSECTS || rel == CONTAINS || rel == TOUCHES ||
    rel == COVERS);
  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  bool result;
  bool answered = meos_spatialrel(geom1, geom2, rel, &result);
  uint8_t badtype = answered ? 0 : geo_unsupported_type(geom1, geom2);
  lwgeom_free(geom1); lwgeom_free(geom2);
  if (! answered)
  {
    geo_error_unsupported_type("spatial relationship", badtype);
    return false;
  }
  return result;
}

/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if two geometries intersects
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS functions: @p ST_Intersects(PG_FUNCTION_ARGS)
 */
bool
geom_intersects2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  return geom_spatialrel(gs1, gs2, INTERSECTS);
}

/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if two geometries intersect
 * @details Bare name for the planar (2D) intersection test, the portable
 * counterpart of @ref geog_intersects() for geometry; equivalent to PostGIS
 * @p ST_Intersects.
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p ST_Intersects(PG_FUNCTION_ARGS)
 */
bool
geom_intersects(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  return geom_intersects2d(gs1, gs2);
}

/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if the first geometry contains the second one
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS functions: @p contains(PG_FUNCTION_ARGS)
 */
bool
geom_contains(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  return geom_spatialrel(gs1, gs2, CONTAINS);
}

/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if the two geometries intersect on a border
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p touches(PG_FUNCTION_ARGS)
 */
bool
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
bool
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
bool
geom_disjoint2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  return ! geom_spatialrel(gs1, gs2, INTERSECTS);
}

/**
 * @ingroup meos_geo_base_rel
 * @brief Return the DE-9IM intersection matrix of two geometries
 * @param[in] gs1,gs2 Geometries
 * @details The matrix is the native one, so a circular arc is met on its own
 * circle rather than on the chords a linearization would put in its place.
 * @ref geom_relate_pattern() answers whether a pattern matches it, which is
 * the question a caller holding a pattern asks; this entry answers the matrix
 * itself, which is what a caller without one needs
 * @return A newly allocated string of nine characters, @p NULL on error
 * @note PostGIS function: @p ST_Relate(geometry, geometry)
 * @csqlfn #Geom_relate()
 */
char *
geom_relate(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_geo_geo(gs1, gs2) || ! ensure_not_geodetic_geo(gs1))
    return NULL;

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  char matrix[10];
  bool covered = meos_relate(geom1, geom2, matrix);
  lwgeom_free(geom1); lwgeom_free(geom2);
  if (! covered)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "The intersection matrix of the geometries is not supported");
    return NULL;
  }
  char *result = palloc(10);
  strcpy(result, matrix);
  return result;
}

/**
 * @ingroup meos_geo_base_rel
 * @brief Return true if two geometries satisfy a spatial relationship given
 * by a pattern
 * @param[in] gs1,gs2 Geometries
 * @param[in] p Pattern
 * @details The pattern is matched against the native DE-9IM intersection
 * matrix, so a circular arc is met on its own circle rather than on the chords
 * a linearization would put in its place, and an empty operand meets nothing
 * @note PostGIS function: @p relate_pattern(PG_FUNCTION_ARGS)
 * Note also the the pattern may be modified in the function
 */
bool
geom_relate_pattern(const GSERIALIZED *gs1, const GSERIALIZED *gs2, char *p)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(p, false);
  if (! ensure_valid_geo_geo(gs1, gs2) || ! ensure_not_geodetic_geo(gs1))
    return false;

  /* The pattern alphabet is upper case */
  for (size_t i = 0; i < strlen(p); i++ )
  {
    if ( p[i] == 't' ) p[i] = 'T';
    if ( p[i] == 'f' ) p[i] = 'F';
  }

  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  bool result;
  bool covered = meos_relate_pattern(geom1, geom2, p, &result);
  uint8_t badtype = covered ? 0 : geo_unsupported_type(geom1, geom2);
  lwgeom_free(geom1); lwgeom_free(geom2);
  if (! covered)
  {
    geo_error_unsupported_type("relationship", badtype);
    return false;
  }
  return result;
}

/**
 * @brief Return @c true iff @p gs is a 2D POLYGON or MULTIPOLYGON.
 * @internal Used by #geom_intersection2d / #geom_difference2d to decide
 * whether to fast-path through the Clipper2-backed @c clip_poly_poly.
 * Geography and 3D inputs fall through to the GEOS path
 */
static bool
geo_is_planar_polygonal(const GSERIALIZED *gs)
{
  assert(gs);
  if (FLAGS_GET_Z(gs->gflags) || FLAGS_GET_GEODETIC(gs->gflags))
    return false;
  uint32_t t = gserialized_get_type(gs);
  return (t == POLYGONTYPE || t == MULTIPOLYGONTYPE);
}

/**
 * @brief Return what two areal geometries meet along where they share no area,
 * or @c NULL where they meet in nothing
 * @details The intersection of two point sets is a point set, and nothing
 * about it promises an area. An engine that assembles regions answers the
 * region, so for a pair meeting at a point or along a curve it answers an
 * empty one -- and "no area" and "nothing" are different sentences.
 *
 * Where the two share no area, no part of the first one's boundary reaches
 * the interior of the second: a point of it that did would carry a
 * neighbourhood of the first one's own interior into the second's, which is
 * area they would then share. So the part of that boundary the second
 * geometry covers is exactly the part lying ON its boundary -- the two
 * expressions denote one set -- and it is the SECOND BOUNDARY the clip is
 * asked about, because only that one is answered by the segment kernels
 * alone. Clipping against the second geometry as a SOLID asks additionally
 * where a point falls relative to its interior, a question the kernels answer
 * by locating constructed points, and the answer it gives is not always on
 * either operand. Asking it of the first operand's boundary also keeps the
 * answer running in that operand's own direction
 * @param[in] gs1,gs2 Geometries
 * @pre The two share no area, which is what the caller has just read
 */
static GSERIALIZED *
geom_areal_meeting(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  assert(gs1); assert(gs2);
  /* Two geometries whose bounding boxes lie apart meet nowhere, which spares
   * an ordinary spatial join a boundary of its own for every pair it rejects */
  GBOX box1, box2;
  memset(&box1, 0, sizeof(GBOX));
  memset(&box2, 0, sizeof(GBOX));
  if (gserialized_get_gbox_p(gs1, &box1) && gserialized_get_gbox_p(gs2, &box2)
      && gbox_overlaps_2d(&box1, &box2) == LW_FALSE)
    return NULL;

  GSERIALIZED *bound1 = geom_boundary(gs1);
  GSERIALIZED *bound2 = bound1 ? geom_boundary(gs2) : NULL;
  GSERIALIZED *result = (bound1 && bound2 && geo_clip_subject(bound1) &&
    geo_meos_supported(bound2)) ?
    geo_clip_linear_geom(bound1, bound2, true) : NULL;
  if (bound1) pfree(bound1);
  if (bound2) pfree(bound2);
  /* A meeting of nothing is the empty region the caller already holds */
  if (result && geo_is_empty(result))
  {
    pfree(result);
    result = NULL;
  }
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the intersection of two geometries
 * @param[in] gs1,gs2 Geometries
 * @note PostGIS function: @p ST_Intersection(PG_FUNCTION_ARGS). With respect
 * to the original function we do not use the @p prec argument.
 *
 * When both inputs are 2D POLYGON / MULTIPOLYGON the call routes through
 * the Clipper2-backed #clip_poly_poly, and where that answers a region of no
 * area #geom_areal_meeting answers what the two meet along. Other type
 * combinations fall through to PostGIS's GEOS-backed
 * @c lwgeom_intersection_prec.
 */
GSERIALIZED *
geom_intersection2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_geo_geo(gs1, gs2) || ! ensure_not_geodetic_geo(gs1))
    return NULL;

  /* Clipper2 fast-path for 2D polygonal inputs */
  if (geo_is_planar_polygonal(gs1) && geo_is_planar_polygonal(gs2))
  {
    GSERIALIZED *result = clip_poly_poly(gs1, gs2, CL_INTERSECTION);
    /* The region two surfaces share is empty where they meet without
     * overlapping, and what they meet along is still theirs in common */
    if (result && geo_is_empty(result))
    {
      GSERIALIZED *meeting = geom_areal_meeting(gs1, gs2);
      if (meeting)
      {
        pfree(result);
        return meeting;
      }
    }
    return result;
  }

  /* The points of a point set that the other geometry covers ARE the
   * intersection, whatever the other geometry draws */
  if (geo_is_point_set(gs1))
    return geo_points_covered(gs1, gs2, true);
  if (geo_is_point_set(gs2))
    return geo_points_covered(gs2, gs1, true);

  /* The part of a line inside the other geometry, read from the segment
   * kernels, which answer an arc of that geometry exactly */
  if (geo_clip_subject(gs1) && geo_meos_supported(gs2))
    return geo_clip_linear_geom(gs1, gs2, true);
  if (geo_clip_subject(gs2) && geo_meos_supported(gs1))
    return geo_clip_linear_geom(gs2, gs1, true);

  /* An areal pair the native overlay reads is answered on the circles its
   * operands carry, where the route below reads an arc as the chain of chords
   * a linearization puts in its place. It declines a pair whose boundaries run
   * along one another rather than crossing, and the route below answers that
   * one */
  if (geo_is_planar_areal(gs1) && geo_is_planar_areal(gs2))
  {
    LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
    LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
    LWGEOM *lwresult = meos_areal_intersection(geom1, geom2);
    GSERIALIZED *result = lwresult ? geo_serialize(lwresult) : NULL;
    if (lwresult)
      lwgeom_free(lwresult);
    lwgeom_free(geom1); lwgeom_free(geom2);
    if (result)
      return result;
  }

#if GEOS
  /* Other types fall through to GEOS */
  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  LWGEOM *lwresult = lwgeom_intersection_prec(geom1, geom2, -1);
  /* MEOS: the overlay answers NULL for a geometry it cannot read -- a
   * polyhedral surface reaches the default arm of #LWGEOM2GEOS, whose lwerror
   * the MEOS handler reports and RETURNS from -- so the answer is absent
   * rather than empty and serializing it would read a null pointer */
  if (! lwresult)
  {
    lwgeom_free(geom1); lwgeom_free(geom2);
    return NULL;
  }
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(geom1); lwgeom_free(geom2); lwgeom_free(lwresult);
  return result;
#else /* ! GEOS */
  meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
    "The intersection of two geometries that are not both planar and "
    "polygonal is answered by the GEOS library, which this build excludes: "
    "configure with -DGEOS=ON");
  return NULL;
#endif /* GEOS */
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
  /* Ensure the validity of the arguments */
  if (! ensure_valid_geo_geo(gs1, gs2) || ! ensure_not_geodetic_geo(gs1))
    return NULL;

  /* Clipper2 fast-path for 2D polygonal inputs */
  if (geo_is_planar_polygonal(gs1) && geo_is_planar_polygonal(gs2))
    return clip_poly_poly(gs1, gs2, CL_DIFFERENCE);

  /* Difference takes the FIRST operand apart, so only its own kind decides */
  if (geo_is_point_set(gs1))
    return geo_points_covered(gs1, gs2, false);
  if (geo_clip_subject(gs1) && geo_meos_supported(gs2))
    return geo_clip_linear_geom(gs1, gs2, false);

  /* A region loses no area to a clip that covers none. A point set and a curve
   * are of lower dimension than the plane, so what they take from a region is
   * of lower dimension too, and the region a difference answers is the subject
   * unchanged -- which also keeps the arcs of a curved subject, where reaching
   * the overlay would return them as the chain of chords it reads them by */
  if (geo_is_planar_areal(gs1) &&
      (geo_is_point_set(gs2) || geo_clip_subject(gs2)))
  {
    /* A part enclosing NO area is not a region but its own boundary, and a
     * clip of no area takes part of THAT: the line a flat ring lies along
     * removes the ring entirely. The rule holds of a region, so the guard asks
     * whether EVERY part of the subject is one -- a total would let a subject
     * mixing a region with a part of no area keep the part a clip covers, and
     * answer one point set two ways depending on how it is spelled */
    if (geo_every_part_bounds_area(gs1))
      return geo_copy(gs1);
  }

  /* An areal pair the native overlay reads is answered on the circles its
   * operands carry, where the route below reads an arc as the chain of chords
   * a linearization puts in its place. It declines a pair whose boundaries run
   * along one another rather than crossing, and the route below answers that
   * one */
  if (geo_is_planar_areal(gs1) && geo_is_planar_areal(gs2))
  {
    LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
    LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
    LWGEOM *lwresult = meos_areal_difference(geom1, geom2);
    GSERIALIZED *result = lwresult ? geo_serialize(lwresult) : NULL;
    if (lwresult)
      lwgeom_free(lwresult);
    lwgeom_free(geom1); lwgeom_free(geom2);
    if (result)
      return result;
  }

#if GEOS
  /* Other types fall through to GEOS */
  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  LWGEOM *lwresult = lwgeom_difference_prec(geom1, geom2, -1);
  /* MEOS: the overlay answers NULL for a geometry it cannot read -- a
   * polyhedral surface reaches the default arm of #LWGEOM2GEOS, whose lwerror
   * the MEOS handler reports and RETURNS from -- so the answer is absent
   * rather than empty and serializing it would read a null pointer */
  if (! lwresult)
  {
    lwgeom_free(geom1); lwgeom_free(geom2);
    return NULL;
  }
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(geom1); lwgeom_free(geom2); lwgeom_free(lwresult);
  return result;
#else /* ! GEOS */
  meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
    "The difference of two geometries that are not both planar and polygonal "
    "is answered by the GEOS library, which this build excludes: configure "
    "with -DGEOS=ON");
  return NULL;
#endif /* GEOS */
}

/**
 * @brief Return the collection a set of geometries stands for
 * @details #lwcollection_construct() reads the Z and M dimensions of the
 * collection from its first member and refuses a set whose members do not
 * share them, answering @p NULL. That is not a collection an arm can be given:
 * the arms read their members through it, so a null one is dereferenced rather
 * than reported. The members and the array are released with the refusal, so a
 * caller answered @p NULL has nothing left to free
 * @param[in] geoms Array of geometries, owned by this function
 * @param[in] ngeoms Number of elements in the array, at least one
 * @param[in] srid Spatial reference identifier
 * @return The collection, which owns the array and its members, or @p NULL
 * where the members do not share their dimensions
 */
static LWCOLLECTION *
union_collection_make(LWGEOM **geoms, uint32_t ngeoms, int32_t srid)
{
  assert(geoms); assert(ngeoms > 0);
  LWCOLLECTION *result = lwcollection_construct(COLLECTIONTYPE, srid, NULL,
    ngeoms, geoms);
  if (! result)
  {
    for (uint32_t i = 0; i < ngeoms; i++)
      lwgeom_free(geoms[i]);
    pfree(geoms);
  }
  return result;
}

/**
 * @brief Return the union of an array of geometries whose members are all
 * surfaces, read from their boundaries
 * @details The array is presented to #meos_areal_union() as the collection it
 * stands for, so the answer is the one the unary union of that collection
 * gives: a pair whose interiors meet becomes one surface and a pair that only
 * touches stays apart. An empty member carries no area and is left out
 * @param[in] gsarr Array of geometries
 * @param[in] count Number of elements in the array
 * @return The union, or @p NULL where a member is not a surface or the
 * boundary overlay does not cover the topology of a pair, which leaves the
 * caller to answer it another way
 */
static GSERIALIZED *
geom_array_areal_union(GSERIALIZED **gsarr, int count)
{
  assert(gsarr); assert(count > 1);
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * count);
  int ngeoms = 0;
  for (int i = 0; i < count; i++)
    if (! gserialized_is_empty(gsarr[i]))
      geoms[ngeoms++] = lwgeom_from_gserialized(gsarr[i]);
  if (ngeoms == 0)
  {
    pfree(geoms);
    return NULL;
  }
  /* #union_collection_make() takes ownership of the array it is given, so
   * geoms must not be freed after this call */
  LWCOLLECTION *coll = union_collection_make(geoms, (uint32_t) ngeoms,
    gserialized_get_srid(gsarr[0]));
  if (! coll)
    return NULL;
  LWGEOM *lwresult = meos_areal_union(lwcollection_as_lwgeom(coll));
  GSERIALIZED *result = lwresult ? geo_serialize(lwresult) : NULL;
  if (lwresult)
    lwgeom_free(lwresult);
  lwcollection_free(coll);
  return result;
}

/**
 * @brief Return true if every member of an array is a point
 * @details Which points of a set are equal is read from their coordinates
 * alone, so the union of an array of points -- the set with its duplicates
 * removed -- is the same answer whether the coordinates are measured on the
 * plane or on the sphere. That is what lets a GEODETIC array of points be
 * answered by a planar engine, where an array carrying a curve or a surface
 * cannot be
 */
static bool
geom_array_point_only(GSERIALIZED **gsarr, int count)
{
  assert(gsarr);
  for (int i = 0; i < count; i++)
  {
    uint32_t gtype = gserialized_get_type(gsarr[i]);
    if (gtype != POINTTYPE && gtype != MULTIPOINTTYPE)
      return false;
  }
  return true;
}

/**
 * @brief Return the union of an array of geometries whose members carry
 * linework and points
 * @details The array is presented to #meos_linear_union() as the collection it
 * stands for, so the answer is the one the unary union of that collection
 * gives: a component another one covers is left out and what remains is
 * collected.  An empty member carries nothing and is left out
 * @param[in] gsarr Array of geometries
 * @param[in] count Number of elements in the array
 * @param[in] geodetic True when the array is measured on the sphere, which
 * the answer carries: a collection built here does not inherit the flag its
 * members hold
 * @return The union, or @p NULL where a member is not linear or a point, or
 * where a pair shares a curve, which leaves the caller to answer it another way
 */
static GSERIALIZED *
geom_array_linear_union(GSERIALIZED **gsarr, int count, bool geodetic)
{
  assert(gsarr); assert(count > 1);
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * count);
  int ngeoms = 0;
  for (int i = 0; i < count; i++)
    if (! gserialized_is_empty(gsarr[i]))
      geoms[ngeoms++] = lwgeom_from_gserialized(gsarr[i]);
  if (ngeoms == 0)
  {
    pfree(geoms);
    return NULL;
  }
  /* #union_collection_make() takes ownership of the array it is given, so
   * geoms must not be freed after this call */
  LWCOLLECTION *coll = union_collection_make(geoms, (uint32_t) ngeoms,
    gserialized_get_srid(gsarr[0]));
  if (! coll)
    return NULL;
  LWGEOM *lwresult = meos_linear_union(lwcollection_as_lwgeom(coll));
  GSERIALIZED *result = NULL;
  if (lwresult)
  {
    /* The members carry the flag already; a collection built here does not,
     * and a geography whose collection reads as a geometry is a different
     * value. Only the flag is set: #lwgeom_set_geodetic() walks the tree and
     * refuses a type this answer may legitimately be */
    FLAGS_SET_GEODETIC(lwresult->flags, geodetic);
    result = geo_serialize(lwresult);
    lwgeom_free(lwresult);
  }
  lwcollection_free(coll);
  return result;
}

/**
 * @brief Return the components a union answer is read as, one entry per piece
 * that stands on its own
 * @details A multi-geometry stands for its members and a general collection
 * for its components, so the answer is assembled from those rather than from
 * the wrapper.
 * ⛔ The test is the TYPE, not #lwgeom_is_collection(), which answers true for
 * a curve polygon and a compound curve as well -- their rings and pieces are
 * sub-geometries, so reading THEM as components takes a surface apart into its
 * own boundary
 * @param[in] geom Geometry
 * @param[out] comps Newly allocated array of components, which the geometry
 * keeps ownership of and which the caller releases with @p pfree()
 * @return The number of components
 */
static int
union_components(const LWGEOM *geom, const LWGEOM ***comps)
{
  uint8_t type = geom->type;
  if (type == MULTIPOINTTYPE || type == MULTILINETYPE ||
      type == MULTICURVETYPE || type == MULTIPOLYGONTYPE ||
      type == MULTISURFACETYPE || type == COLLECTIONTYPE)
  {
    const LWCOLLECTION *coll = (const LWCOLLECTION *) geom;
    const LWGEOM **result = palloc(sizeof(LWGEOM *) * coll->ngeoms);
    for (uint32_t i = 0; i < coll->ngeoms; i++)
      result[i] = coll->geoms[i];
    *comps = result;
    return (int) coll->ngeoms;
  }
  const LWGEOM **one = palloc(sizeof(LWGEOM *));
  one[0] = geom;
  *comps = one;
  return 1;
}

/**
 * @brief Return the union of an array of geometries whose members fall on both
 * sides of the areal boundary
 * @details The array is split on #relate_is_areal() and each half is answered
 * by the arm that already answers it -- #meos_areal_union() for the surfaces,
 * #meos_linear_union() for the linework and the points. What the two answers
 * share is then read: a non-areal piece the surfaces COVER contributes nothing
 * of its own and is left out, and what remains is collected beside them.
 *
 * A piece the surfaces cover only in PART stays whole, which is the rule the
 * union already keeps for its linework: both spellings cover the same points,
 * and keeping the piece whole keeps a circular arc on its own circle. That is
 * where this answer differs from the one GEOS gives, which cuts the piece at
 * the boundary
 * @param[in] gsarr Array of geometries
 * @param[in] count Number of elements in the array
 * @return The union, or @p NULL where the array does not span the boundary, or
 * where either half is one its arm does not answer, which leaves the caller to
 * answer it another way
 */
static GSERIALIZED *
geom_array_mixed_union(GSERIALIZED **gsarr, int count)
{
  assert(gsarr); assert(count > 1);
  int32_t srid = gserialized_get_srid(gsarr[0]);
  /* An empty member carries no points and is left out, as it is in both arms */
  LWGEOM **areal = palloc(sizeof(LWGEOM *) * count);
  LWGEOM **other = palloc(sizeof(LWGEOM *) * count);
  uint32_t nareal = 0, nother = 0;
  for (int i = 0; i < count; i++)
  {
    if (gserialized_is_empty(gsarr[i]))
      continue;
    LWGEOM *geom = lwgeom_from_gserialized(gsarr[i]);
    if (relate_is_areal(geom))
      areal[nareal++] = geom;
    else
      other[nother++] = geom;
  }
  /* An array that stays on one side of the boundary is what the two arms
   * already answered, and this one has nothing to add to it */
  if (nareal == 0 || nother == 0)
  {
    for (uint32_t i = 0; i < nareal; i++)
      lwgeom_free(areal[i]);
    for (uint32_t i = 0; i < nother; i++)
      lwgeom_free(other[i]);
    pfree(areal); pfree(other);
    return NULL;
  }

  /* #union_collection_make() takes ownership of the array it is given and of
   * its members, so neither is freed other than through the collection. A half
   * holding a single member is the collection of one, which each arm answers
   * by returning that member -- there is no count the arms must be spared.
   * A half whose members do not share their dimensions is no collection, and
   * the other half is then released here rather than through one */
  LWCOLLECTION *acoll = union_collection_make(areal, nareal, srid);
  LWCOLLECTION *ocoll = acoll ?
    union_collection_make(other, nother, srid) : NULL;
  if (! acoll || ! ocoll)
  {
    if (! acoll)
    {
      for (uint32_t i = 0; i < nother; i++)
        lwgeom_free(other[i]);
      pfree(other);
    }
    if (acoll)
      lwcollection_free(acoll);
    return NULL;
  }
  LWGEOM *lwareal = meos_areal_union(lwcollection_as_lwgeom(acoll));
  LWGEOM *lwother = lwareal ?
    meos_linear_union(lwcollection_as_lwgeom(ocoll)) : NULL;
  lwcollection_free(acoll); lwcollection_free(ocoll);
  if (! lwother)
  {
    if (lwareal)
      lwgeom_free(lwareal);
    return NULL;
  }

  /* The surfaces are asked about every piece, so their edges are extracted
   * once and read again for each question rather than once per question */
  void *actx = relate_ctx_make(lwareal);
  const LWGEOM **ocomps;
  int nocomps = union_components(lwother, &ocomps);
  bool *dropped = palloc0(sizeof(bool) * nocomps);
  int nkept = 0;
  for (int i = 0; i < nocomps && actx; i++)
  {
    void *octx = relate_ctx_make(ocomps[i]);
    bool covered = false;
    if (! octx || ! meos_spatialrel_ctx(actx, octx, COVERS, &covered))
    {
      /* A pair the engine does not cover leaves the whole answer to the
       * caller: a piece kept without knowing it is covered would be a
       * component the union does not have */
      relate_ctx_free(octx);
      relate_ctx_free(actx);
      actx = NULL;
      break;
    }
    relate_ctx_free(octx);
    dropped[i] = covered;
    if (! covered)
      nkept++;
  }
  if (! actx)
  {
    pfree(dropped); pfree(ocomps);
    lwgeom_free(lwareal); lwgeom_free(lwother);
    return NULL;
  }
  relate_ctx_free(actx);

  /* Every piece is covered, so the surfaces are the whole answer and they
   * carry their own type rather than being wrapped in a collection */
  GSERIALIZED *result;
  if (nkept == 0)
  {
    result = geo_serialize(lwareal);
    pfree(dropped); pfree(ocomps);
    lwgeom_free(lwareal); lwgeom_free(lwother);
    return result;
  }

  /* The answer lists what remains before the surfaces, the order a collection
   * is read in, and lists each piece on its own: a collection of surfaces
   * stands for its members here, not for itself */
  const LWGEOM **acomps;
  int nacomps = union_components(lwareal, &acomps);
  LWGEOM **members = palloc(sizeof(LWGEOM *) * (nkept + nacomps));
  int nmembers = 0;
  for (int i = 0; i < nocomps; i++)
    if (! dropped[i])
      members[nmembers++] = lwgeom_clone_deep(ocomps[i]);
  for (int i = 0; i < nacomps; i++)
    members[nmembers++] = lwgeom_clone_deep(acomps[i]);
  pfree(dropped); pfree(ocomps); pfree(acomps);
  /* #union_collection_make() takes ownership of the array it is given. The two
   * halves are each a collection, and yet what they draw together need not be:
   * a surface carrying Z and a line that does not are two answers no single
   * collection holds, and that array is left to the caller as the halves were */
  LWCOLLECTION *coll = union_collection_make(members, (uint32_t) nmembers,
    srid);
  if (! coll)
  {
    lwgeom_free(lwareal); lwgeom_free(lwother);
    return NULL;
  }
  result = geo_serialize(lwcollection_as_lwgeom(coll));
  lwcollection_free(coll);
  lwgeom_free(lwareal); lwgeom_free(lwother);
  return result;
}

/**
 * @brief Return the members of an array read on the dimensions they share
 * @details A planar union answers the point set its members cover, and the Z
 * and M ordinates of that answer are READ from the members rather than
 * computed: a member carrying no elevation determines none for the points it
 * contributes, and an answer declaring one would publish a value the array
 * does not hold. The answer therefore carries an ordinate only where EVERY
 * member contributing points carries it. An empty member contributes none, so
 * it does not take an ordinate away from the members that do
 * @param[in] gsarr Array of geometries
 * @param[in] count Number of elements in the array
 * @return A new array holding the members read on their shared dimensions, or
 * @p NULL where they already share them and the array itself is what the arms
 * read. The caller releases the array and its members
 */
static GSERIALIZED **
geom_array_shared_dims(GSERIALIZED **gsarr, int count)
{
  assert(gsarr); assert(count > 0);
  bool hasz = true, hasm = true, mixed = false;
  int nonempty = 0;
  for (int i = 0; i < count; i++)
  {
    if (gserialized_is_empty(gsarr[i]))
      continue;
    nonempty++;
    if (! gserialized_has_z(gsarr[i]))
      hasz = false;
    if (! gserialized_has_m(gsarr[i]))
      hasm = false;
  }
  /* An array of empties carries no points to read an ordinate for */
  if (nonempty == 0)
    return NULL;
  for (int i = 0; i < count && ! mixed; i++)
  {
    if (gserialized_is_empty(gsarr[i]))
      continue;
    mixed = ((bool) gserialized_has_z(gsarr[i]) != hasz ||
      (bool) gserialized_has_m(gsarr[i]) != hasm);
  }
  /* Every member already carries what they share, and the array reads as it is */
  if (! mixed)
    return NULL;

  GSERIALIZED **result = palloc(sizeof(GSERIALIZED *) * count);
  for (int i = 0; i < count; i++)
  {
    LWGEOM *geom = lwgeom_from_gserialized(gsarr[i]);
    /* The two ordinates are never both dropped: they are shared unless a
     * member lacks one, and a member lacking both leaves nothing to force */
    LWGEOM *shared = hasz ? lwgeom_force_3dz(geom, 0) :
      (hasm ? lwgeom_force_3dm(geom, 0) : lwgeom_force_2d(geom));
    result[i] = geo_serialize(shared);
    lwgeom_free(geom); lwgeom_free(shared);
  }
  return result;
}

/**
 * @brief Return the union of an array of geometries whose members share their
 * dimensions
 * @details The function will iteratively call @p GEOSUnion on the
 * GEOS-converted versions of them and return PGIS-converted version back.
 * Changing the combination order *might* speed up performance.
 * @param[in] gsarr Array of geometries
 * @param[in] count Number of elements in the array
 * @note PostGIS function: @p pgis_union_geometry_array(PG_FUNCTION_ARGS)
 * MEOS modified the PostGIS function since does not cope with geographies
 * by setting the geodetic flag for geographies.
 */
static GSERIALIZED *
geom_array_union_shared(GSERIALIZED **gsarr, int count)
{
  assert(gsarr); assert(count > 1);

  /* An array holding nothing but empties has an empty union, which is read
   * from the array alone. It is answered here so that a build carrying no
   * GEOS answers it too */
  int nonempty = 0;
  uint8_t largest_empty = 0;
  for (int i = 0; i < count; i++)
  {
    if (! gserialized_is_empty(gsarr[i]))
      nonempty++;
    else
    {
      uint8_t gser_type = (uint8_t) gserialized_get_type(gsarr[i]);
      if (gser_type > largest_empty)
        largest_empty = gser_type;
    }
  }
  if (nonempty == 0)
  {
    if (largest_empty == 0)
      return NULL;
    LWGEOM *empty = lwgeom_construct_empty(largest_empty,
      gserialized_get_srid(gsarr[0]), (bool) gserialized_has_z(gsarr[0]), 0);
    GSERIALIZED *result = geo_serialize(empty);
    lwgeom_free(empty);
    return result;
  }

  /* The dissolve of an array whose members are all surfaces is read from their
   * boundaries, which keeps a circular arc on its own circle where a
   * linearization would put the chain of segments approximating it in its
   * place. A member that is not a surface is not what it answers, and neither
   * is a geodetic value, whose surfaces are bounded by geodesics rather than
   * by the segments a planar overlay reads */
  bool geodetic = FLAGS_GET_GEODETIC(gsarr[0]->gflags);
  if (! geodetic)
  {
    GSERIALIZED *native = geom_array_areal_union(gsarr, count);
    if (native)
      return native;
  }
  /* An array whose members carry linework and points is read the same way, by
   * #meos_linear_union(), which keeps a circular arc on its own circle. A
   * geodetic array reaches it only when every member is a point, the one case
   * whose answer does not depend on the metric */
  if (! geodetic || geom_array_point_only(gsarr, count))
  {
    GSERIALIZED *native = geom_array_linear_union(gsarr, count, geodetic);
    if (native)
      return native;
  }
  /* An array whose members fall on BOTH sides of the areal boundary is
   * answered by the two arms together: each half is dissolved by the arm that
   * answers it, and a non-areal piece the surfaces cover is left out */
  if (! geodetic)
  {
    GSERIALIZED *native = geom_array_mixed_union(gsarr, count);
    if (native)
      return native;
  }

#if GEOS
  bool is3d = false, gotsrid = false;
  int curgeom = 0;
  uint8_t empty_type = 0;
  int32_t srid = SRID_UNKNOWN;
  GSERIALIZED *result = NULL;
  GEOSGeometry *g = NULL;
  GEOSGeometry *g_union = NULL;

  GEOSContextHandle_t ctx = geos_get_context();

  /* Collect the non-empty inputs and stuff them into a GEOS collection */
  GEOSGeometry **geoms = palloc(sizeof(GEOSGeometry *) * count);

  /*
  ** We need to convert the array of GSERIALIZED into a GEOS collection.
  ** First make an array of GEOS geometries.
  */
  for (int i = 0; i < count; i++)
  {
    if (! gotsrid)
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
    g = GEOSGeom_createCollection_r(ctx, GEOS_GEOMETRYCOLLECTION, geoms, curgeom);
    if (! g)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Could not create GEOS COLLECTION from geometry array");
      return NULL;
    }

    g_union = GEOSUnaryUnion_r(ctx, g);
    GEOSGeom_destroy_r(ctx, g);
    if (! g_union)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "GEOSUnaryUnion");
      return NULL;
    }

    GEOSSetSRID_r(ctx, g_union, srid);
    result = GEOS2POSTGIS(g_union, is3d);
    /* MEOS: GEOS DOES NOT SET THE GEODETIC FLAG. The flag cannot be set on the
     * serialized value, since #GEOS2POSTGIS has already added a planar
     * bounding box, which occupies 2 * ndims floats while a geodetic one
     * occupies 6 floats. The value is thus deserialized, flagged as geodetic
     * after dropping the planar box, and serialized again */
    if (FLAGS_GET_GEODETIC(gsarr[0]->gflags))
    {
      LWGEOM *lwgeom = lwgeom_from_gserialized(result);
      lwgeom_drop_bbox(lwgeom);
      lwgeom_set_geodetic(lwgeom, true);
      /* The deserialized value reads its coordinates OUT OF @p result, so the
       * geodetic serialization below walks that buffer. It is released once
       * the new value is built, never before */
      GSERIALIZED *planar = result;
      result = geo_serialize(lwgeom);
      lwgeom_free(lwgeom);
      pfree(planar);
    }
    GEOSGeom_destroy_r(ctx, g_union);
  }
  /* No real geometries in our array, any empties? */
  else
  {
    /* If it was only empties, we'll return the largest type number */
    if (empty_type > 0)
    {
      LWGEOM *geom = lwgeom_construct_empty(empty_type, srid, is3d, 0);
      GSERIALIZED *result = geo_serialize(geom);
      lwgeom_free(geom);
      return result;
    }
    /* Nothing but NULL, returns NULL */
    else
      return NULL;
  }

  pfree(geoms);
  if (! result)
    /* Union returned a NULL geometry */
    return NULL;
  return result;
#else /* ! GEOS */
  meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
    "The union of a geodetic array, and of one whose linework coincides over a "
    "curve, is answered by the GEOS library, which this build excludes: "
    "configure with -DGEOS=ON");
  return NULL;
#endif /* GEOS */
}

/**
 * @brief Return the answer of an overlay carrying the ordinates its members
 * determine
 * @details The answer of an overlay is computed on the PLANE while its Z and M
 * are a lift of that plane, so the ordinates are read back from the members
 * once the planar figure is known -- see #meos_lift_ordinates(). An answer of
 * members carrying neither ordinate is already the whole answer, and is left
 * untouched rather than walked
 * @param[in] result Answer of the overlay, owned by this function
 * @param[in] gsarr Array of geometries the answer is read from
 * @param[in] count Number of elements in the array
 * @return The answer carrying the ordinates its members determine, or @p NULL
 * where the overlay gave none
 */
static GSERIALIZED *
union_lifted(GSERIALIZED *result, GSERIALIZED **gsarr, int count)
{
  assert(gsarr); assert(count > 0);
  if (! result)
    return NULL;
  if (! gserialized_has_z(gsarr[0]) && ! gserialized_has_m(gsarr[0]))
    return result;

  const LWGEOM **geoms = palloc(sizeof(LWGEOM *) * count);
  for (int i = 0; i < count; i++)
    geoms[i] = lwgeom_from_gserialized(gsarr[i]);
  LWGEOM *lwresult = lwgeom_from_gserialized(result);
  LWGEOM *lifted = meos_lift_ordinates(lwresult, geoms, count);
  GSERIALIZED *answer = geo_serialize(lifted);
  for (int i = 0; i < count; i++)
    lwgeom_free((LWGEOM *) geoms[i]);
  pfree(geoms);
  lwgeom_free(lwresult); lwgeom_free(lifted); pfree(result);
  return answer;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the union of an array of geometries
 * @details The answer covers the points the members cover, read on the
 * dimensions they share: an array mixing a member that carries an elevation
 * with one that does not is answered without one, since no member determines
 * an elevation for the points the flat one contributes. That reading is what
 * the array is answered from, so the answer does not depend on which member
 * the array happens to list first
 * @param[in] gsarr Array of geometries
 * @param[in] count Number of elements in the array
 * @return On error return @p NULL
 */
GSERIALIZED *
geom_array_union(GSERIALIZED **gsarr, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gsarr, NULL);
  if (! ensure_positive(count))
    return NULL;
  /* This entry answers on the plane; #geog_array_union() answers a geodetic
   * array. The flag is read from one member, so the array is turned away
   * before the SRID test walks all of them */
  if (! ensure_not_geodetic_geo(gsarr[0]))
    return NULL;
  /* The members of the array carry one SRID */
  if (! ensure_same_srid_geoarr((const GSERIALIZED **) gsarr, count))
    return NULL;

  /* A single member is its own union, returned as a value of its own: the
   * array belongs to the caller, and a result aliasing a member of it is
   * released when the caller releases the array. The member carries whatever
   * dimensions it has, since there is no other member to share them with */
  if (count == 1)
    return geo_copy(gsarr[0]);

  GSERIALIZED **shared = geom_array_shared_dims(gsarr, count);
  GSERIALIZED **members = shared ? shared : gsarr;
  GSERIALIZED *result = union_lifted(geom_array_union_shared(members, count),
    members, count);
  if (shared)
  {
    for (int i = 0; i < count; i++)
      pfree(shared[i]);
    pfree(shared);
  }
  return result;
}

/**
 * @ingroup meos_geo_base_transf
 * @brief Return the union of an array of geographies
 * @param[in] gsarr Array of geographies
 * @param[in] count Number of elements in the array
 * @return On error return @p NULL
 * @details The union of a set of positions is the set with its duplicates
 * removed, and two positions are the same when their coordinates are, so the
 * answer is read without measuring anything and holds on the spheroid as it
 * does on the plane. A geodetic array carrying anything else is a different
 * question: a geodesic is not the segment joining two positions in degree
 * space, and two geodesics meet where those segments need not, so the answer
 * awaits an overlay that works on the spheroid.
 */
GSERIALIZED *
geog_array_union(GSERIALIZED **gsarr, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gsarr, NULL);
  if (! ensure_positive(count))
    return NULL;
  if (! ensure_geodetic_geo(gsarr[0]))
    return NULL;
  /* The members of the array carry one SRID */
  if (! ensure_same_srid_geoarr((const GSERIALIZED **) gsarr, count))
    return NULL;

  /* A single member is its own union, returned as a value of its own, as on
   * the planar side */
  if (count == 1)
    return geo_copy(gsarr[0]);

  if (! geom_array_point_only(gsarr, count))
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "The union of a geodetic array is answered for positions only");
    return NULL;
  }

  /* Which positions of a set are equal is read from their coordinates, and an
   * elevation none of them carries is no more available here than on the
   * plane, so the array is read on the dimensions its members share */
  GSERIALIZED **shared = geom_array_shared_dims(gsarr, count);
  if (! shared)
    return geom_array_linear_union(gsarr, count, true);
  GSERIALIZED *result = geom_array_linear_union(shared, count, true);
  for (int i = 0; i < count; i++)
    pfree(shared[i]);
  pfree(shared);
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
geom_unary_union(const GSERIALIZED *gs, double prec)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_geodetic_geo(gs))
    return NULL;

  LWGEOM *lwgeom = lwgeom_from_gserialized(gs) ;
  /* The dissolve of a geometry whose components are surfaces is read from
   * their boundaries, which keeps a circular arc on its own circle where a
   * linearization would put the chain of segments approximating it in its
   * place. A precision model is not something it answers for, and neither is a
   * geometry carrying a component that is not a surface */
  LWGEOM *lwresult = (prec < 0) ? meos_areal_union(lwgeom) : NULL;
  if (! lwresult)
  {
#if GEOS
    lwresult = lwgeom_unaryunion_prec(lwgeom, prec);
    /* MEOS: the overlay answers NULL for a geometry it cannot read -- a
     * polyhedral surface reaches the default arm of #LWGEOM2GEOS, whose
     * lwerror the MEOS handler reports and RETURNS from -- so the answer is
     * absent rather than empty and every step below would read a null pointer */
    if (! lwresult)
    {
      lwgeom_free(lwgeom);
      return NULL;
    }
    /* MEOS: PostGIS function #lwgeom_unaryunion_prec only propagates the SRID
     * and the Z flag. The GEODETIC flag must be set BEFORE serialization,
     * since the bounding box of a geodetic value is computed on the unit
     * sphere and occupies 6 floats, while the one of a planar value occupies
     * 2 * ndims floats. The answer read from the boundaries needs none of
     * this: it is built with the flags it carries, and a geodetic geometry
     * does not reach here */
    lwgeom_set_geodetic(lwresult, FLAGS_GET_GEODETIC(lwgeom->flags));
#else /* ! GEOS */
    lwgeom_free(lwgeom);
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "The unary union of a geometry whose components are not all surfaces, "
      "and the one asked for on a precision grid, are answered by the GEOS "
      "library, which this build excludes: configure with -DGEOS=ON");
    return NULL;
#endif /* GEOS */
  }
  /* The dissolve is computed on the plane, and the ordinates the components
   * carry are read back onto it: two of them crossing at one elevation give
   * the answer that elevation, and two crossing at different ones leave the
   * point with none */
  const LWGEOM *inputs[1] = { lwgeom };
  LWGEOM *lifted = meos_lift_ordinates(lwresult, inputs, 1);
  GSERIALIZED *result = geo_serialize(lifted);
  lwgeom_free(lwgeom); lwgeom_free(lwresult); lwgeom_free(lifted);
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return true if a geometry has no anomalous point, which is a point
 * at which it crosses or touches itself
 * @details A point is always simple, a multipoint is simple when it repeats
 * no point, a line is simple when it meets itself only where two of its
 * segments follow one another and, when it closes, at the point where it
 * closes, and an areal geometry is simple when each of its rings is. The
 * lines of a multiline may additionally meet at a point that ends both.
 * @param[in] gs Geometry
 * @note PostGIS function: @p ST_IsSimple(PG_FUNCTION_ARGS). With respect to
 * the original function we do not use the @p flags argument.
 * @csqlfn #Geom_is_simple()
 */
bool
geom_is_simple(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, false);
  if (! ensure_not_geodetic_geo(gs))
    return false;

  LWGEOM *geom = lwgeom_from_gserialized(gs);
  bool result;
  bool covered = meos_is_simple(geom, &result);
  lwgeom_free(geom);
  if (covered)
    return result;

  /* #meos_is_simple answers every type #geom_meos_supported admits, and a
   * geodetic geometry is refused above, so a geometry reaching here carries a
   * type the engine does not read at all */
  meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
    "Unsupported geometry type");
  return false;
}

/*****************************************************************************/


/**
 * @ingroup meos_geo_base_spatial
 * @brief Call function #geom_intersection2d for each element of the collection
 * if the arguments are collections
 * @details Some GEOS operations, e.g., intersects or intersection do not
 * support collections. In this cases, we need to extract the elements of the
 * collection and iterate over the elements
 */
GSERIALIZED *
geom_intersection2d_coll(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_geo_geo(gs1, gs2))
    return NULL;

  /* Extract the elements of the arguments, if they are collections */
  int count1, count2, count = 0;
  GSERIALIZED **elems1 = geo_extract_elements(gs1, &count1);
  GSERIALIZED **elems2 = geo_extract_elements(gs2, &count2);
  GSERIALIZED **res = palloc(sizeof(GSERIALIZED *) * count1 * count2);
  /* Perform the iterations for the elements in the collections if any */
  for (int i = 0; i < count1; i++)
  {
    for (int j = 0; j < count2; j++)
    {
      GSERIALIZED *inter = geom_intersection2d(elems1[i], elems2[j]);
      if (gserialized_is_empty(inter))
        pfree(inter);
      else
        res[count++] = inter;
    }
  }
  /* Construct the result */
  GSERIALIZED *result = NULL;
  if (count)
  {
    if (count == 1)
    {
      result = res[0];
      pfree(res);
    }
    else
    {
      result = geo_collect_garray(res, count);
      pfree_array((void *) res, count);
    }
  }
  else
    /* The two cover nothing in common, so the array holds no element to keep
     * and no element to release -- only itself */
    pfree(res);
  /* Clean up and return */
  pfree_array((void *) elems1, count1);
  pfree_array((void *) elems2, count2);
  return result;
}

/*****************************************************************************
 * Functions adapted from lwgeom_geos_predicates.c
 *****************************************************************************/

/**
 * @ingroup meos_geo_base_comp
 * @brief Return true if the geometries/geographies are equal, false otherwise
 * @param[in] gs1,gs2 Geometries/geographies
 * @details The answer is read from the native DE-9IM intersection matrix, so a
 * circular arc is met on its own circle rather than on the chords a
 * linearization would put in its place
 * @note PostGIS function: @p ST_Equals(PG_FUNCTION_ARGS)
 */
int
geo_equals(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs1, -1); VALIDATE_NOT_NULL(gs2, -1);
  if (! ensure_same_srid(gserialized_get_srid(gs1),
        gserialized_get_srid(gs2)))
    return -1;

  /* Empty == Empty */
  if ( gserialized_is_empty(gs1) && gserialized_is_empty(gs2) )
    return 1;

  /*
   * Short-circuit: If gs1 and gs2 do not have the same bounding box
   * we can return FALSE.
   */
  GBOX box1, box2;
  memset(&box1, 0, sizeof(GBOX));
  memset(&box2, 0, sizeof(GBOX));
  if (gserialized_get_gbox_p(gs1, &box1) && gserialized_get_gbox_p(gs2, &box2))
  {
    // ORIGINAL DEFINITION: TODO verify that the generalization to 3D is OK
    // if ( gbox_same_2d_float(&box1, &box2) == LW_FALSE )
    if ( gbox_same(&box1, &box2) == LW_FALSE )
      return 0;
  }

  /*
   * Short-circuit: if gs1 and gs2 are binary-equivalent, we can return
   * TRUE.  This is much faster than computing the intersection matrix.
   */
  if (VARSIZE(gs1) == VARSIZE(gs2) && ! memcmp(gs1, gs2, VARSIZE(gs1)))
      return 1;

  /* Two geometries are equal where their interiors meet, neither interior
   * reaches the exterior of the other and neither boundary does, which is the
   * pattern the standard gives ST_Equals */
  LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
  char matrix[10];
  bool covered = meos_relate(geom1, geom2, matrix);
  uint8_t badtype = covered ? 0 : geo_unsupported_type(geom1, geom2);
  lwgeom_free(geom1); lwgeom_free(geom2);
  if (! covered)
  {
    geo_error_unsupported_type("equality", badtype);
    return -1;
  }
  return de9im_match(matrix, "T*F**FFF*") ? 1 : 0;
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
  /** Force to geodetic in case it's not **/
  lwgeom_set_geodetic(lwgeom, true);
  size_t ret_size;
  GSERIALIZED *result = gserialized_from_lwgeom(lwgeom, &ret_size);
  /** Set geodetic **/
  FLAGS_SET_GEODETIC(result->gflags, 1);
  SET_VARSIZE(result, ret_size);
  return result;
}


/*****************************************************************************
 * Functions adapted from lwgeom_transform.c
 *****************************************************************************/

/**
 * @ingroup meos_geo_base_srid
 * @brief Return the geometry/geography transformed to an SRID
 * @return On error return @p NULL
 * @param[in] gs Geometry/geography
 * @param[in] srid_to Target SRID
 * @note PostGIS function: @p transform(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geo_transform(const GSERIALIZED *gs, int32_t srid_to)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
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
    return geo_copy(gs);

  LWPROJ *pj;
  if (! lwproj_lookup(srid_from, srid_to, &pj))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "geo_transform: Error when getting projection.");
    return NULL;
  }

  /* now we have a geometry, and input/output PJ structs. */
  GSERIALIZED *gs1 = geo_copy(gs);
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs1);
  if (lwgeom_transform(lwgeom, pj) == LW_FAILURE)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Coordinate transformation failed");
    lwgeom_free(lwgeom);
    pfree(gs1);
    return NULL;
  }
  lwgeom->srid = srid_to;

  /* Re-compute bbox if input had one (COMPUTE_BBOX TAINTING) */
  if (lwgeom->bbox)
  {
    lwgeom_refresh_bbox(lwgeom);
  }

  GSERIALIZED *result = geo_serialize(lwgeom);
  lwgeom_free(lwgeom);
  pfree(gs1);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL); VALIDATE_NOT_NULL(pipeline, NULL);
  if (srid_to == SRID_UNKNOWN)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "geo_transform_pipeline: %d is an invalid target SRID", SRID_UNKNOWN);
    return NULL;
  }

  GSERIALIZED *gs1 = geo_copy(gs);
  LWGEOM *geom = lwgeom_from_gserialized(gs1);
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
  lwgeom_free(geom); pfree(gs1);
  return (result); /* new geometry */
}

/*****************************************************************************
 * Functions adapted from geography_centroid.c
 *****************************************************************************/

/**
 * @brief Convert lonlat coordinates to Cartesien coordinates
 */
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

/**
 * @brief Convert Cartesian coordinates to LWPOINT
 */
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
 * @brief Split polygons into triangles and use centroid of the triangle with
 * the triangle area as weight to calculate the centroid of a (multi)polygon.
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
 * @ingroup meos_geo_base_accessor
 * @brief Return the centroid of a geometry
 * @note PostGIS function: @p geography_centroid(PG_FUNCTION_ARGS).
 */
GSERIALIZED *
geog_centroid(const GSERIALIZED *gs, bool use_spheroid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_geodetic_geo(gs))
    return NULL;

  LWGEOM *lwgeom_out = NULL;
  LWPOINT *lwpoint_out = NULL;
  GSERIALIZED *g_out = NULL;
  SPHEROID s;

  /* Get our geometry object loaded into memory. */
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  int32_t srid = lwgeom_get_srid(lwgeom);

  /* on empty input, return empty output */
  if (gserialized_is_empty(gs))
  {
    LWCOLLECTION *empty = lwcollection_construct_empty(COLLECTIONTYPE, srid,
      0, 0);
    lwgeom_out = lwcollection_as_lwgeom(empty);
    g_out = geo_serialize(lwgeom_out);
    lwgeom_free(lwgeom_out); lwgeom_free(lwgeom);
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
      lwgeom_free(lwgeom);
      return geo_copy(gs);
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
      /* #lwmline_add_lwline stores the line it is given rather than copying
       * it, so this releases the operand along with the collection holding
       * it and the release below must not reach it a second time */
      lwmline_free(mline);
      lwgeom = NULL;
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
      /* #lwmpoly_add_lwpoly stores the polygon it is given rather than
       * copying it, so this releases the operand along with the collection
       * holding it and the release below must not reach it a second time */
      lwmpoly_free(mpoly);
      lwgeom = NULL;
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
      lwgeom_free(lwgeom);
      return NULL;
    }
  }
  lwgeom_out = lwpoint_as_lwgeom(lwpoint_out);
  g_out = geo_serialize(lwgeom_out);
  /* MEOS: GEOS DOES NOT SET THE GEODETIC FLAG */
  FLAGS_SET_GEODETIC(g_out->gflags, FLAGS_GET_GEODETIC(gs->gflags));
  lwgeom_free(lwgeom_out); lwgeom_free(lwgeom);
  return g_out;
}

/*****************************************************************************
 * Functions adapted from geography_measurement.c
 *****************************************************************************/

/**
 * @ingroup meos_geo_base_accessor
 * @brief Return the area of a geography in square meters
 * @param[in] gs Geography
 * @param[in] use_spheroid True when using a spheroid
 * @return On error return @p DBL_MAX
 * @note PostGIS function: @p geography_area(PG_FUNCTION_ARGS)
 */
double
geog_area(const GSERIALIZED *gs, bool use_spheroid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, DBL_MAX);
  if (! ensure_geodetic_geo(gs))
    return DBL_MAX;

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
  if (! use_spheroid )
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
 * @brief Return the perimeter of a geography in meters
 * @param[in] gs Geography
 * @param[in] use_spheroid True when using a spheroid
 * @return On error return @p DBL_MAX
 * @note PostGIS function: @p geography_perimeter(PG_FUNCTION_ARGS)
 */
double
geog_perimeter(const GSERIALIZED *gs, bool use_spheroid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, DBL_MAX);
  if (! ensure_geodetic_geo(gs))
    return DBL_MAX;

  LWGEOM *lwgeom = NULL;
  double length;
  int type;

  /* Only return for area features. */
  type = gserialized_get_type(gs);
  if (type != POLYGONTYPE && type != MULTIPOLYGONTYPE &&
      type != COLLECTIONTYPE)
    return 0.0;

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
  if (! use_spheroid )
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, DBL_MAX);
  if (! ensure_geodetic_geo(gs))
    return DBL_MAX;

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
  /* Ensure the validity of the arguments */
  if (! ensure_valid_geo_geo(gs1, gs2) || ! ensure_geodetic_geo(gs1))
    return false;

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
bool
geog_intersects(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  bool use_spheroid)
{
  return geog_dwithin(gs1, gs2, 0.0, use_spheroid);
}

/**
 * @ingroup meos_geo_base_dist
 * @brief Return the distance between two geographies
 * @param[in] gs1,gs2 Geographies
 * @note PostGIS function: @p geography_distance_uncached(PG_FUNCTION_ARGS).
 * We set by default both @p tolerance and @p use_spheroid and initialize the
 * spheroid to WGS84
 * @return On error or empty geometries return DBL_MAX
 */
double
geog_distance(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_geo_geo(gs1, gs2) || ! ensure_geodetic_geo(gs1))
    return DBL_MAX;

  /* Return NULL on empty arguments. */
  if (gserialized_is_empty(gs1) || gserialized_is_empty(gs2) )
    return DBL_MAX;

  double tolerance = FP_TOLERANCE;
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

  /* Something went wrong, negative or infinite return... */
  if (distance < 0.0 || distance == DBL_MAX)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "geography_distance returned distance < 0.0");
    return DBL_MAX;
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
    lwpoint_free(empty_point);
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
 * @ingroup meos_geo_base_inout
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
      return NULL;
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
    /* A string of an odd length, or one carrying a character that is not a
     * hexadecimal digit, encodes nothing */
    if (! wkb)
    {
      meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
        "Could not parse geometry value: %s", str);
      return NULL;
    }
    lwgeom = lwgeom_from_wkb(wkb, hexsize/2, LW_PARSER_CHECK_NONE);
    lwfree(wkb);
    /* The reader is asked for no parser checks, so it answers NULL for bytes
     * that do not spell a geometry */
    if (! lwgeom)
    {
      meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
        "Could not parse geometry value: %s", str);
      return NULL;
    }
    /* If we picked up an SRID at the head of the WKB set it manually */
    if ( srid ) lwgeom_set_srid(lwgeom, srid);
    /* Add a bbox if necessary */
    if ( lwgeom_needs_bbox(lwgeom) ) lwgeom_add_bbox(lwgeom);
    result = geo_serialize(lwgeom);
    lwgeom_free(lwgeom);
  }
  else if (str1[0] == '{')
  {
    char *srs = NULL;
    lwgeom = lwgeom_from_geojson(str1, &srs);
    if (! lwgeom)
    {
      if (srs)
        lwfree(srs);
      meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
        "Could not parse geometry value: %s", str);
      return NULL;
    }
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
    lwgeom_free(lwgeom);
  }

  if (typmod >= 0)
    result = postgis_valid_typmod(result, typmod);

  /* Don't free the parser result (and hence lwgeom) until we have done */
  /* the typemod check with lwgeom */
  return result;
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
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
  VALIDATE_NOT_NULL(gs, NULL);

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
  {
    PG_PARSER_ERROR(lwg_parser_result);
    return NULL;
  }

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
  lwgeom_parser_result_free(&lwg_parser_result);
  return geo_result;
}

/**
 * @ingroup meos_internal_geo_base_inout
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
  if (! ensure_positive(precision))
    return NULL;

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
char *
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
char *
geo_as_ewkt(const GSERIALIZED *gs, int precision)
{
  return geo_as_wkt(gs, precision, true);
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return a geometry from its ASCII hex-encoded Well-Known Binary
 * (HexEWKB) representation
 * @param[in] wkt WKT string
 * @note This is a a stricter version of #geom_in, where we refuse to
 * accept (HEX)WKB or EWKT.
 * @note PostGIS function: @p LWGEOM_from_text(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geom_from_hexewkb(const char *wkt)
{
  return geom_in(wkt, -1);
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return a geography from its ASCII hex-encoded Well-Known Binary
 * (HexEWKB) representation
 * @param[in] wkt WKT string
 * @note This is a a stricter version of #geog_in, where we refuse to
 * accept (HEX)WKB or EWKT.
 * @note PostGIS function: @p LWGEOM_from_text(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geog_from_hexewkb(const char *wkt)
{
  return geog_in(wkt, -1);
}

/**
 * @ingroup meos_geo_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a geometry/geography
 * @param[in] gs Geometry/geography
 * @param[in] endian Endianness
 * @note PostGIS function: @p AsHEXEWKB(gs, string)
 */
char *
geo_as_hexewkb(const GSERIALIZED *gs, const char *endian)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);

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
geo_from_ewkb(const uint8_t *wkb, size_t wkb_size, int32_t srid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb, NULL);

  LWGEOM *geom = lwgeom_from_wkb(wkb, wkb_size, LW_PARSER_CHECK_ALL);
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
 * @param[out] size Size of result
 * @note PostGIS function: @p WKBFromLWGEOM(PG_FUNCTION_ARGS)
 */
uint8_t *
geo_as_ewkb(const GSERIALIZED *gs, const char *endian, size_t *size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);

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

  size_t data_size = VARSIZE(wkb) - LWVARHDRSZ;
  uint8_t *result = palloc(data_size);
  memcpy(result, wkb->data, data_size);
  lwgeom_free(geom); pfree(wkb);
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
    lwfree(srs);
    return NULL;
  }

  // if (srs)
  // {
    // srid = GetSRIDCacheBySRS(fcinfo, srs);
    lwfree(srs);
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
 * @param[in] srs Spatial reference system, may be `NULL`
 * @note PostGIS function: @p LWGEOM_asGeoJson(PG_FUNCTION_ARGS)
 */
char *
geo_as_geojson(const GSERIALIZED *gs, int option, int precision,
  const char *srs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);

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
  /* The result of lwgeom_to_geojson is a length-prefixed lwvarlena_t whose data
   * is not null-terminated; copy exactly its length so the string does not read
   * past the end into adjacent memory */
  char *result = pnstrdup(VARDATA(txt), LWSIZE_GET(txt->size) - LWVARHDRSZ);
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
    {
      PG_PARSER_ERROR(lwg_parser_result);
      return NULL;
    }
    lwgeom = lwg_parser_result.geom;
  }

  GSERIALIZED *result = NULL;
  /* Error on any SRID != default */
  if (lwgeom->srid == SRID_UNKNOWN || ensure_srid_is_latlong(lwgeom->srid))
    /* Convert to gserialized */
    result = geog_from_lwgeom(lwgeom, typmod);

  /* Clean up and return */
  lwgeom_free(lwgeom);
  return result;
}

/**
 * @ingroup meos_geo_base_conversion
 * @brief Return a geography from a geometry
 * @param[in] gs Geometry
 * @note PostGIS function: @p geography_from_geometry(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
geom_to_geog(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
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
geog_to_geom(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);

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
  {
    result = lwmpoint_as_lwgeom(lwmpoint_construct(srid, opa));
    ptarray_free(opa);
  }
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
line_interpolate_point(const GSERIALIZED *gs, double fraction, bool repeat)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
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

  int32_t srid = gserialized_get_srid(gs);
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  LWGEOM *lwresult = lwgeom_line_interpolate_point(lwgeom, fraction, srid,
    repeat);

  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwgeom); lwgeom_free(lwresult);
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return a subline from a line
 * @param[in] gs Geometry
 * @param[in] from,to Values in [0,1] representing the fractional locations
 * where the subline starts and ends
 * @note PostGIS function: @p LWGEOM_line_substring(PG_FUNCTION_ARGS)
 */
GSERIALIZED *
line_substring(const GSERIALIZED *gs, double from, double to)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
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
      // lwline_release(iline); /* MEOS remove memory leaks */
      lwline_free(iline);
      return NULL;
    }

    POINTARRAY *ipa = iline->points;
    opa = ptarray_substring(ipa, from, to, 0);
    if (opa->npoints == 1) /* Point returned */
      olwgeom = (LWGEOM *)lwpoint_construct(iline->srid, NULL, opa);
    else
      olwgeom = (LWGEOM *)lwline_construct(iline->srid, NULL, opa);
    lwline_free(iline); /* MEOS remove memory leaks */
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
      // lwline_release(iline); /* MEOS remove memory leaks */
      lwmline_free(iline);
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
    lwmline_free(iline); /* MEOS remove memory leaks */
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
 * Minimum Enclosing Circle implementation improving the performance of the
 * PostGIS function ST_MinimumBoundingCircle
 *****************************************************************************/

/**
 * @brief Definition of the 2D circle structure
 * @note Equivalent of PostGIS structure LWBOUNDINGCIRCLE
 */
typedef struct
{
  POINT2D center;
  double radius;
} Circle;

/**
 * @brief Return the distance between two points
 */
static inline double
distance_point2d(POINT2D a, POINT2D b)
{
  double dx = a.x - b.x;
  double dy = a.y - b.y;
  return sqrt(dx * dx + dy * dy);
}

/**
 * @brief Return true if a point is inside a circle
 */
static inline bool
mec_inside(POINT2D p, Circle c)
{
  return distance_point2d(p, c.center) <= c.radius + MEOS_GEOM_TOLERANCE;
}

/**
 * @brief Circle constructor for 2 points
 */
static inline Circle
mec_circle2(POINT2D a, POINT2D b)
{
  Circle c;
  c.center.x = (a.x + b.x) * 0.5;
  c.center.y = (a.y + b.y) * 0.5;
  c.radius = distance_point2d(a,b) * 0.5;
  return c;
}

/**
 * @brief Circle constructor for 3 points
 */
static Circle
mec_circle3(POINT2D a, POINT2D b, POINT2D c)
{
  double A = b.x - a.x;
  double B = b.y - a.y;
  double C = c.x - a.x;
  double D = c.y - a.y;
  double E = A * (a.x + b.x) + B * (a.y + b.y);
  double F = C * (a.x + c.x) + D * (a.y + c.y);
  double G = 2.0 * (A * (c.y - b.y) - B * (c.x - b.x));

  /* Zero-init so the early-exit return doesn't leave circ.center
   * uninitialised — cppcheck flags this as `uninitvar`, and a downstream
   * caller that ignored circ.radius == -1 would read garbage. */
  Circle circ = { .center = {0.0, 0.0}, .radius = 0.0 };
  if (fabs(G) < MEOS_GEOM_TOLERANCE)
  {
    circ.radius = -1;
    return circ;
  }
  circ.center.x = (D * E - B * F) / G;
  circ.center.y = (A * F - C * E) / G;
  circ.radius = distance_point2d(circ.center,a);
  return circ;
}

/**
 * @brief Return the minimum enclosing circle for 3 points, handling the
 * collinear case by falling back to the best 2-point circle
 */
static Circle
mec_circle3_safe(POINT2D a, POINT2D b, POINT2D c)
{
  Circle circ = mec_circle3(a, b, c);
  if (circ.radius >= 0)
    return circ;
  /* Collinear: pick the largest 2-point circle */
  Circle c1 = mec_circle2(a, b);
  Circle c2 = mec_circle2(a, c);
  Circle c3 = mec_circle2(b, c);
  if (c2.radius > c1.radius) c1 = c2;
  if (c3.radius > c1.radius) c1 = c3;
  return c1;
}

/**
 * @brief Iterative Welzl algorithm for the minimum enclosing circle
 * @details Equivalent to the recursive Welzl algorithm but uses constant
 * stack space. The three nested loops correspond to the three levels of
 * recursion (boundary set size 0, 1, 2). Despite appearing O(n^3), the
 * expected runtime is O(n) with random shuffling.
 * @param[in] P Array of points (must be shuffled beforehand)
 * @param[in] n Number of points
 * @pre n >= 1
 */
static Circle
mec_welzl(POINT2D *P, int n)
{
  Circle C = (Circle){P[0], 0};
  for (int i = 1; i < n; i++)
  {
    if (! mec_inside(P[i], C))
    {
      C = (Circle){P[i], 0};
      for (int j = 0; j < i; j++)
      {
        if (! mec_inside(P[j], C))
        {
          C = mec_circle2(P[i], P[j]);
          for (int k = 0; k < j; k++)
          {
            if (! mec_inside(P[k], C))
              C = mec_circle3_safe(P[i], P[j], P[k]);
          }
        }
      }
    }
  }
  return C;
}

/**
 * @brief Extract coordinates from LWGEOM
 * @pre The geometry type is one of the supported types as given by function
 * #lwgeom_mec_supported_type
 */
static void
lwgeom_collect_points(const LWGEOM *geom, MeosArray *array)
{
  POINT2D point;
  if (geom->type == POINTTYPE)
  {
    const LWPOINT *p = (LWPOINT *) geom;
    const POINT2D *pt = getPoint2d_cp(p->point, 0);
    point = (POINT2D){pt->x, pt->y};
    meos_array_add(array, &point);
  }
  else if (geom->type == LINETYPE)
  {
    const LWLINE *l = (LWLINE *) geom;
    for (int i = 0; i < (int) l->points->npoints; i++)
    {
      const POINT2D *pt = getPoint2d_cp(l->points, i);
      point = (POINT2D){pt->x, pt->y};
      meos_array_add(array, &point);
    }
  }
  else if (geom->type == TRIANGLETYPE)
  {
    const LWTRIANGLE *tr = (LWTRIANGLE *) geom;
    for (int i = 0; i < (int) tr->points->npoints; i++)
    {
      const POINT2D *pt = getPoint2d_cp(tr->points, i);
      point = (POINT2D){pt->x, pt->y};
      meos_array_add(array, &point);
    }
  }
  else if (geom->type == POLYGONTYPE)
  {
    const LWPOLY *poly = (LWPOLY *) geom;
    for (int r = 0; r < (int) poly->nrings; r++)
    {
      POINTARRAY *pa = poly->rings[r];
      for (int i = 0; i < (int) pa->npoints; i++)
      {
        const POINT2D *pt = getPoint2d_cp(pa, i);
        point = (POINT2D){pt->x, pt->y};
        meos_array_add(array, &point);
      }
    }
  }
  else if (lwgeom_is_collection(geom))
  {
    const LWCOLLECTION *col = (LWCOLLECTION *) geom;
    for (int i = 0; i < (int) col->ngeoms; i++)
      lwgeom_collect_points(col->geoms[i], array);
  }
  return;
}

/**
 * @brief Computation of the Minimum Enclosing Circle
 * @pre The geometry is not empty and is one of the supported gemetry types
 */
static Circle
lwgeom_mec(const LWGEOM *geom)
{
  MeosArray *array = meos_array_create(sizeof(POINT2D));
  lwgeom_collect_points(geom, array);
  /* Ensure that there is at least one point given the precondition */
  assert(array->count > 0);
  /* Reuse the array->elems array that contains the points */
  POINT2D *pts = (POINT2D *) array->elems;

  /* Fisher-Yates shuffle */
  for (int i = array->count - 1; i > 0; i--)
  {
    int j = rand() % (i + 1);
    POINT2D tmp = pts[i];
    pts[i] = pts[j];
    pts[j] = tmp;
  }
  Circle result = mec_welzl(pts, array->count);
  meos_array_destroy(array);
  return result;
}

/**
 * @brief Return true if the geometry type is one of the supported types
 * for the MEOS fast Minimum Bounding Circle
 */
static bool
lwgeom_mec_supported_type(const LWGEOM *geom)
{
  if (geom->type == POINTTYPE || geom->type == LINETYPE ||
      geom->type == TRIANGLETYPE || geom->type == POLYGONTYPE)
    return true;
  else if (lwgeom_is_collection(geom))
  {
    const LWCOLLECTION *col = (LWCOLLECTION *) geom;
    for (int i = 0; i < (int) col->ngeoms; i++)
    {
      if (! lwgeom_mec_supported_type(col->geoms[i]))
        return false;
    }
    return true;
  }
  else
    return false;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the center point and radius of the smallest circle that
 * contains a geometry
 * @param[in] geom Geometry
 * @param[out] radius Radius
 * @note The corresponding PostGIS function ST_MinimumBoundingCircle is much
 * slower despite it uses the same algorithm
 *   Welzl, Emo (1991), "Smallest enclosing disks (balls and elipsoids)."
 *   New Results and Trends in Computer Science (H. Maurer, Ed.), Lecture Notes
 *   in Computer Science, 555 (1991) 359-370.
 */
GSERIALIZED *
geom_min_bounding_radius(const GSERIALIZED *geom, double *radius)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(geom, NULL); VALIDATE_NOT_NULL(radius, NULL);

  LWGEOM *input = lwgeom_from_gserialized(geom);
  LWGEOM *center;

  if (lwgeom_is_empty(input))
  {
    center = (LWGEOM *) lwpoint_construct_empty(input->srid, LW_FALSE, LW_FALSE);
    *radius = 0;
  }
  else if (lwgeom_mec_supported_type(input))
  {
    Circle c = lwgeom_mec(input);
    center = (LWGEOM *) lwpoint_make2d(input->srid, c.center.x, c.center.y);
    *radius = c.radius;
  }
  else
  {
    LWBOUNDINGCIRCLE *mbc = lwgeom_calculate_mbc(input);
    if (! (mbc && mbc->center))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Error calculating minimum bounding circle");
      lwgeom_free(input);
      return NULL;
    }
    center = (LWGEOM *) lwpoint_make2d(input->srid, mbc->center->x,
      mbc->center->y);
    *radius = mbc->radius;
    lwboundingcircle_destroy(mbc);
  }

  GSERIALIZED *result = geo_serialize(center);
  lwgeom_free(center);
  lwgeom_free(input);
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
  /* Ensure the validity of the arguments */
  if (! ensure_valid_geo_geo(gs1, gs2))
    return -1.0;

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
  lwline_free(lwline); lwpoint_free(lwpoint);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);

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
  GSERIALIZED *result = geo_serialize(lwpoint_as_lwgeom(point));
  lwpoint_free(point);
  return result;
}

/**
 * @ingroup meos_geo_base_accessor
 * @brief Return the number of points of a line
 * @param[in] gs Geometry
 * @return On error return INT_MAX
*/
int
line_numpoints(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, INT_MAX);

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
