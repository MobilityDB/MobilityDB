/***********************************************************************
 *
 * tpoint_spatialfuncs.c
 *    Spatial functions for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint_spatialfuncs.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>

#if MOBDB_PGSQL_VERSION >= 120000
#include <utils/float.h>
#endif

#include "period.h"
#include "periodset.h"
#include "timeops.h"
#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "lifting.h"
#include "tnumber_mathfuncs.h"
#include "postgis.h"
#include "geography_funcs.h"
#include "tpoint.h"
#include "tpoint_boxops.h"
#include "tpoint_spatialrels.h"

/*****************************************************************************/
 
/**
 * Global variable to save the fcinfo when PostGIS functions need to access
 * the cache such as transform, geography_distance, or geography_azimuth
 */
FunctionCallInfo _FCINFO;

/**
 * Fetch from the cache the fcinfo of the external function
 */
FunctionCallInfo 
fetch_fcinfo()
{
  assert(_FCINFO);
  return _FCINFO;
}


/**
 * Store in the cache the fcinfo of the external function
 */
void 
store_fcinfo(FunctionCallInfo fcinfo)
{
  _FCINFO = fcinfo;
  return;
}

/*****************************************************************************
 * Functions derived from PostGIS
 *****************************************************************************/

/**
 * Returns a float between 0 and 1 representing the location of the closest 
 * point on the segment to the given point, as a fraction of total segment 
 * length (2D version).
 *
 * @note Function derived from the PostGIS function closest_point_on_segment.
 */
double
closest_point2d_on_segment_ratio(const POINT2D *p, const POINT2D *A, 
  const POINT2D *B, POINT2D *closest)
{
  if (FP_EQUALS(A->x, B->x) && FP_EQUALS(A->y, B->y))
  {
    *closest = *A;
    return 0.0;
  }

  /*
   * We use comp.graphics.algorithms Frequently Asked Questions method
   *
   * (1)          AC dot AB
   *         r = ----------
   *              ||AB||^2
   *  r has the following meaning:
   *  r=0 P = A
   *  r=1 P = B
   *  r<0 P is on the backward extension of AB
   *  r>1 P is on the forward extension of AB
   *  0<r<1 P is interior to AB
   *
   */
  double r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) ) /
    ( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) );

  if (r < 0)
  {
    *closest = *A;
    return 0.0;
  }
  if (r > 1)
  {
    *closest = *B;
    return 1.0;
  }

  closest->x = A->x + ( (B->x - A->x) * r );
  closest->y = A->y + ( (B->y - A->y) * r );
  return r;
}

/**
 * Returns a float between 0 and 1 representing the location of the closest 
 * point on the segment to the given point, as a fraction of total segment 
 * length (3D version).
 *
 * @note Function derived from the PostGIS function closest_point_on_segment.
 */
double
closest_point3dz_on_segment_ratio(const POINT3DZ *p, const POINT3DZ *A, 
  const POINT3DZ *B, POINT3DZ *closest)
{
  if (FP_EQUALS(A->x, B->x) && FP_EQUALS(A->y, B->y) && 
    FP_EQUALS(A->z, B->z))
  {
    *closest = *A;
    return 0.0;
  }

  /*
   * We use comp.graphics.algorithms Frequently Asked Questions method
   *
   * (1)          AC dot AB
   *         r = ----------
   *              ||AB||^2
   *  r has the following meaning:
   *  r=0 P = A
   *  r=1 P = B
   *  r<0 P is on the backward extension of AB
   *  r>1 P is on the forward extension of AB
   *  0<r<1 P is interior to AB
   *
   */
  double r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) + (p->z-A->z) * (B->z-A->z) ) /
    ( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) + (B->z-A->z) * (B->z-A->z) );

  if (r < 0)
  {
    *closest = *A;
    return 0.0;
  }
  if (r > 1)
  {
    *closest = *B;
    return 1.0;
  }

  closest->x = A->x + ( (B->x - A->x) * r );
  closest->y = A->y + ( (B->y - A->y) * r );
  closest->z = A->z + ( (B->z - A->z) * r );
  return r;
}

/**
 * Returns a float between 0 and 1 representing the location of the closest
 * point on the geography segment to the given point, as a fraction of total 
 * segment length.
 *
 *@param[in] p Reference point
 *@param[in] A,B Points defining the segment
 *@param[out] closest Closest point in the segment
 *@param[out] dist Distance between the closest point and the reference point 
 */
double
closest_point_on_segment_sphere(const POINT4D *p, const POINT4D *A,
  const POINT4D *B, POINT4D *closest, double *dist)
{
  GEOGRAPHIC_EDGE e;
  GEOGRAPHIC_POINT a, proj;
  double length, /* length from A to the closest point */
    seglength, /* length of the segment AB */
    result; /* ratio */

  /* Initialize target point */
  geographic_point_init(p->x, p->y, &a);

  /* Initialize edge */
  geographic_point_init(A->x, A->y, &(e.start));
  geographic_point_init(B->x, B->y, &(e.end));

  /* Get the spherical distance between point and edge */
  *dist = edge_distance_to_point(&e, &a, &proj);

  /* Compute distance from beginning of the segment to closest point */
  seglength = sphere_distance(&(e.start), &(e.end));
  length = sphere_distance(&(e.start), &proj);
  result = length / seglength;

  if (closest)
  {
    /* Copy nearest into returning argument */
    closest->x = rad2deg(proj.lon);
    closest->y = rad2deg(proj.lat);

    /* Compute Z and M values for closest point */
    closest->z = A->z + ((B->z - A->z) * result);
    closest->m = A->m + ((B->m - A->m) * result);
  }
  return result;
}

/*****************************************************************************/
 
/**
 * Compute the projected point and the distance between the closest point
 * (2D version).
 *
 * @note Function inspired by PostGIS function lw_dist2d_distancepoint 
 * from measures.c
 */
static double
lw_dist2d_point_dist(const LWGEOM *lw1, const LWGEOM *lw2, int mode,
  double *fraction)
{
  DISTPTS thedl;
  thedl.mode = mode;
  thedl.distance= FLT_MAX;
  thedl.tolerance = 0;
  lw_dist2d_recursive(lw1, lw2, &thedl);
  LWLINE *lwline = lwgeom_as_lwline(lw1);
  POINT2D a, b, closest;
  getPoint2d_p(lwline->points, 0, &a);
  getPoint2d_p(lwline->points, 1, &b);
  *fraction = closest_point2d_on_segment_ratio(&thedl.p1, &a, &b, &closest);
  return thedl.distance;
}

/**
 * Compute the projected point and the distance between the closest point
 * (3D version).
 *
 * @note Function inspired by PostGIS function lw_dist3d_distancepoint 
 * from measures3d.c
 */
static double
lw_dist3d_point_dist(const LWGEOM *lw1, const LWGEOM *lw2, int mode,
  double *fraction)
{
  assert(FLAGS_GET_Z(lw1->flags) && FLAGS_GET_Z(lw2->flags));
  DISTPTS3D thedl;
  thedl.mode = mode;
  thedl.distance= FLT_MAX;
  thedl.tolerance = 0;
  lw_dist3d_recursive(lw1, lw2, &thedl);
  LWLINE *lwline = lwgeom_as_lwline(lw1);
  POINT3DZ a, b, closest;
  getPoint3dz_p(lwline->points, 0, &a);
  getPoint3dz_p(lwline->points, 1, &b);
  *fraction = closest_point3dz_on_segment_ratio(&thedl.p1, &a, &b, &closest);
  return thedl.distance;
}

/**
 * Compute the projected point and the distance between the closest point
 * (geodetic version).
 */
double
lw_dist_sphere_point_dist(const LWGEOM *lw1, const LWGEOM *lw2, int mode,
  double *fraction)
{
  double min_dist = FLT_MAX;
  double max_dist = FLT_MAX;
  GEOGRAPHIC_POINT closest1, closest2, proj;
  GEOGRAPHIC_EDGE e;
  POINT4D a, b;

  CIRC_NODE *circ_tree1 = lwgeom_calculate_circ_tree(lw1);
  CIRC_NODE *circ_tree2 = lwgeom_calculate_circ_tree(lw2);
  circ_tree_distance_tree_internal(circ_tree1, circ_tree2, FP_TOLERANCE,
    &min_dist, &max_dist, &closest1, &closest2);
  double result = sphere_distance(&closest1, &closest2);

  /* Initialize edge */
  LWLINE *lwline = lwgeom_as_lwline(lw1);
  getPoint4d_p(lwline->points, 0, &a);
  getPoint4d_p(lwline->points, 1, &b);
  geographic_point_init(a.x, a.y, &(e.start));
  geographic_point_init(b.x, b.y, &(e.end));

  /* Get the spherical distance between point and edge */
  edge_distance_to_point(&e, &closest1, &proj);

  /* Compute distance from beginning of the segment to closest point */
  double seglength = sphere_distance(&(e.start), &(e.end));
  double length = sphere_distance(&(e.start), &closest1);
  *fraction = length / seglength;

  return result;
}

/*****************************************************************************
 * Functions specializing the PostGIS functions ST_LineInterpolatePoint and
 * ST_LineLocatePoint.
 *****************************************************************************/

/**
 * Returns a point interpolated from the geometry/geography segment with
 * respect to the fraction of its total length.
 *
 * @param[in] start,end Points defining the segment
 * @param[in] ratio Float between 0 and 1 representing the fraction of the 
 * total length of the segment where the point must be located
 */
Datum
geoseg_interpolate_point(Datum start, Datum end, double ratio)
{
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(start);
  int srid = gserialized_get_srid(gs);
  POINT4D p1 = datum_get_point4d(start);
  POINT4D p2 = datum_get_point4d(end);
  POINT4D p;
  bool geodetic = FLAGS_GET_GEODETIC(gs->flags);
  if (geodetic)
  {
    POINT3D q1, q2;
    GEOGRAPHIC_POINT g1, g2;
    geographic_point_init(p1.x, p1.y, &g1);
    geographic_point_init(p2.x, p2.y, &g2);
    geog2cart(&g1, &q1);
    geog2cart(&g2, &q2);
    interpolate_point4d_sphere(&q1, &q2, &p1, &p2, ratio, &p);
  }
  else
    interpolate_point4d(&p1, &p2, &p, ratio);

  LWPOINT *lwpoint = FLAGS_GET_Z(gs->flags) ?
    lwpoint_make3dz(srid, p.x, p.y, p.z) :
    lwpoint_make2d(srid, p.x, p.y);
  FLAGS_SET_GEODETIC(lwpoint->flags, geodetic);
  Datum result = PointerGetDatum(geo_serialize((LWGEOM *) lwpoint));
  lwpoint_free(lwpoint);
  POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(start));
  return result;
}

/**
 * Returns a float between 0 and 1 representing the location of the closest
 * point on the geometry segment to the given point, as a fraction of total 
 * segment length.
 *
 *@param[in] start,end Points defining the segment
 *@param[in] point Reference point
 *@param[out] dist Distance
 */
double
geoseg_locate_point(Datum start, Datum end, Datum point, double *dist)
{
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(start);
  double result;
  if (FLAGS_GET_GEODETIC(gs->flags))
  {
    POINT4D p1 = datum_get_point4d(start);
    POINT4D p2 = datum_get_point4d(end);
    POINT4D p = datum_get_point4d(point);
    POINT4D closest;
    double d;
    /* Get the closest point and the distance */
    result = closest_point_on_segment_sphere(&p, &p1, &p2, &closest, &d);
    /* For robustness, force 0/1 when closest point == start/endpoint */
    if (p4d_same(&p1, &closest))
      result = 0.0;
    else if (p4d_same(&p2, &closest))
      result = 1.0;
    /* Return the distance between the closest point and the point if requested */
    if (dist)
    {
      d = WGS84_RADIUS * d;
      /* Add to the distance the vertical displacement if we're in 3D */
      if (FLAGS_GET_Z(gs->flags))
        d = sqrt( (closest.z - p.z) * (closest.z - p.z) + d*d );
      *dist = d;
    }
  }
  else
  {
    if (FLAGS_GET_Z(gs->flags))
    {
      const POINT3DZ *p1 = datum_get_point3dz_p(start);
      const POINT3DZ *p2 = datum_get_point3dz_p(end);
      const POINT3DZ *p = datum_get_point3dz_p(point);
      POINT3DZ proj;
      result = closest_point3dz_on_segment_ratio(p, p1, p2, &proj);
      /* For robustness, force 0/1 when closest point == start/endpoint */
      if (p3d_same((POINT3D *) p1, (POINT3D *) &proj))
        result = 0.0;
      else if (p3d_same((POINT3D *) p2, (POINT3D *) &proj))
        result = 1.0;
      if (dist)
        *dist = distance3d_pt_pt((POINT3D *)p, (POINT3D *)&proj);
    }
    else
    {
      const POINT2D *p1 = datum_get_point2d_p(start);
      const POINT2D *p2 = datum_get_point2d_p(end);
      const POINT2D *p = datum_get_point2d_p(point);
      POINT2D proj;
      result = closest_point2d_on_segment_ratio(p, p1, p2, &proj);
      if (p2d_same(p1, &proj))
        result = 0.0;
      else if (p2d_same(p2, &proj))
        result = 1.0;
      if (dist)
        *dist = distance2d_pt_pt((POINT2D *)p, &proj);
    }
  }
  return result;
}

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * Ensures that the spatial constraints required for operating on two temporal
 * geometries are satisfied
 */
void
ensure_spatial_validity(const Temporal *temp1, const Temporal *temp2)
{
  if (tgeo_base_type(temp1->valuetypid))
  {
    ensure_same_srid_tpoint(temp1, temp2);
    ensure_same_dimensionality_tpoint(temp1, temp2);
  }
}

/**
 * Ensures that the spatiotemporal boxes have the same type of coordinates, 
 * either planar or geodetic
 */
void
ensure_same_geodetic_stbox(const STBOX *box1, const STBOX *box2)
{
  if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags) &&
    MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
    elog(ERROR, "The boxes must be both planar or both geodetic");
}

/**
 * Ensures that the temporal point and the spatiotemporal box have the same 
 * type of coordinates, either planar or geodetic
 */
void
ensure_same_geodetic_tpoint_stbox(const Temporal *temp, const STBOX *box)
{
  if (MOBDB_FLAGS_GET_X(box->flags) &&
    MOBDB_FLAGS_GET_GEODETIC(temp->flags) != MOBDB_FLAGS_GET_GEODETIC(box->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal point and the box must be both planar or both geodetic")));
}

/**
 * Ensures that the spatiotemporal boxes have the same SRID
 */
void
ensure_same_srid_stbox(const STBOX *box1, const STBOX *box2)
{
  if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags) &&
    box1->srid != box2->srid)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The boxes must be in the same SRID")));
}

/**
 * Ensures that the temporal points have the same SRID
 */
void
ensure_same_srid_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal points must be in the same SRID")));
}

/**
 * Ensures that the temporal point and the spatiotemporal boxes have the same SRID
 */
void
ensure_same_srid_tpoint_stbox(const Temporal *temp, const STBOX *box)
{
  if (MOBDB_FLAGS_GET_X(box->flags) &&
    tpoint_srid_internal(temp) != box->srid)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal point and the box must be in the same SRID")));
}

/**
 * Ensures that the temporal point and the geometry/geography have the same SRID
 */
void
ensure_same_srid_tpoint_gs(const Temporal *temp, const GSERIALIZED *gs)
{
  if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal point and the geometry must be in the same SRID")));
}

/**
 * Ensures that the spatiotemporal boxes have the same dimensionality
 */
void
ensure_same_dimensionality_stbox(const STBOX *box1, const STBOX *box2)
{
  if (MOBDB_FLAGS_GET_X(box1->flags) != MOBDB_FLAGS_GET_X(box2->flags) ||
    MOBDB_FLAGS_GET_Z(box1->flags) != MOBDB_FLAGS_GET_Z(box2->flags) ||
    MOBDB_FLAGS_GET_T(box1->flags) != MOBDB_FLAGS_GET_T(box2->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The boxes must be of the same dimensionality")));
}

/**
 * Ensures that the temporal points have the same dimensionality
 */
void
ensure_same_dimensionality_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal points must be of the same dimensionality")));
}

/**
 * Ensures that the temporal point and the spatiotemporal boxes have the same spatial dimensionality
 */
void
ensure_same_spatial_dimensionality_tpoint_stbox(const Temporal *temp, const STBOX *box)
{
  if (MOBDB_FLAGS_GET_X(temp->flags) != MOBDB_FLAGS_GET_X(box->flags) ||
    MOBDB_FLAGS_GET_Z(temp->flags) != MOBDB_FLAGS_GET_Z(box->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal point and the box must be of the same spatial dimensionality")));
}

/**
 * Ensures that the spatiotemporal boxes have the same spatial dimensionality
 */
void
ensure_same_spatial_dimensionality_stbox(const STBOX *box1, const STBOX *box2)
{
  if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags) &&
    MOBDB_FLAGS_GET_Z(box1->flags) != MOBDB_FLAGS_GET_Z(box2->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The bounding boxes must be of the same spatial dimensionality")));
}

/**
 * Ensures that the temporal point and the spatiotemporal box have the same dimensionality
 */
void
ensure_same_dimensionality_tpoint_stbox(const Temporal *temp, const STBOX *box)
{
  if (MOBDB_FLAGS_GET_X(temp->flags) != MOBDB_FLAGS_GET_X(box->flags) ||
    MOBDB_FLAGS_GET_Z(temp->flags) != MOBDB_FLAGS_GET_Z(box->flags) ||
    MOBDB_FLAGS_GET_T(temp->flags) != MOBDB_FLAGS_GET_T(box->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal point and the box must be of the same dimensionality")));
}

/**
 * Ensures that the temporal point and the geometry/geography have the same dimensionality
 */
void
ensure_same_dimensionality_tpoint_gs(const Temporal *temp, const GSERIALIZED *gs)
{
  if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal point and the geometry must be of the same dimensionality")));
}

/**
 * Ensures that the spatiotemporal boxes have at least one common dimension
 */
void
ensure_common_dimension_stbox(const STBOX *box1, const STBOX *box2)
{
  if (MOBDB_FLAGS_GET_X(box1->flags) != MOBDB_FLAGS_GET_X(box2->flags) &&
    MOBDB_FLAGS_GET_T(box1->flags) != MOBDB_FLAGS_GET_T(box2->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The boxes must have at least one common dimension")));
}

/**
 * Ensures that the spatiotemporal box has XY dimension
 */
void
ensure_has_X_stbox(const STBOX *box)
{
  if (! MOBDB_FLAGS_GET_X(box->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The box must have XY dimension")));
}

/**
 * Ensures that the spatiotemporal box has Z dimension
 */
void
ensure_has_Z_stbox(const STBOX *box)
{
  if (! MOBDB_FLAGS_GET_Z(box->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The box must have Z dimension")));
}

/**
 * Ensures that the spatiotemporal box has T dimension
 */
void
ensure_has_T_stbox(const STBOX *box)
{
  if (! MOBDB_FLAGS_GET_T(box->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The box must have time dimension")));
}

/**
 * Ensures that the temporal point has not Z dimension
 */
void
ensure_has_not_Z_tpoint(const Temporal *temp)
{
  if (MOBDB_FLAGS_GET_Z(temp->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal point cannot have Z dimension")));
}

/**
 * Ensures that the geometry/geography has not Z dimension
 */
void
ensure_has_not_Z_gs(const GSERIALIZED *gs)
{
  if (FLAGS_GET_Z(gs->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Only geometries without Z dimension accepted")));
}

/**
 * Ensures that the geometry/geography has M dimension
 */
void
ensure_has_M_gs(const GSERIALIZED *gs)
{
  if (! FLAGS_GET_M(gs->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Only geometries with M dimension accepted")));
}

/**
 * Ensures that the geometry/geography has not M dimension
 */
void
ensure_has_not_M_gs(const GSERIALIZED *gs)
{
  if (FLAGS_GET_M(gs->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Only geometries without M dimension accepted")));
}

/**
 * Ensures that the geometry/geography is a point
 */
void
ensure_point_type(const GSERIALIZED *gs)
{
  if (gserialized_get_type(gs) != POINTTYPE)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Only point geometries accepted")));
}

/**
 * Ensures that the geometry/geography is not empty
 */
void
ensure_non_empty(const GSERIALIZED *gs)
{
  if (gserialized_is_empty(gs))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Only non-empty geometries accepted")));
}

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

/*
 * Obtain a geometry/geography point from the GSERIALIZED WITHOUT creating
 * the corresponding LWGEOM. These functions constitute a **SERIOUS**
 * break of encapsulation but it is the only way to achieve reasonable
 * performance when manipulating mobility data.
 * The datum_* functions suppose that the GSERIALIZED has been already
 * detoasted. This is typically the case when the datum is within a Temporal*
 * that has been already detoasted with PG_GETARG_TEMPORAL*
 * The first variant (e.g. datum_get_point2d) is slower than the second (e.g.
 * datum_get_point2d_p) since the point is passed by value and thus the bytes
 * are copied. The second version is declared const because you aren't allowed
 * to modify the values, only read them.
 */

/**
 * Returns a 2D point from the serialized geometry 
 */
const POINT2D *
gs_get_point2d_p(GSERIALIZED *gs)
{
  return (POINT2D *)((uint8_t*)gs->data + 8);
}

/**
 * Returns a 2D point from the datum 
 */
POINT2D
datum_get_point2d(Datum geom)
{
  GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(geom);
  POINT2D *point = (POINT2D *)((uint8_t*)gs->data + 8);
  return *point;
}

/**
 * Returns a pointer to a 2D point from the datum 
 */
const POINT2D *
datum_get_point2d_p(Datum geom)
{
  GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(geom);
  return (POINT2D *)((uint8_t*)gs->data + 8);
}

/**
 * Returns a 3DZ point from the serialized geometry 
 */
const POINT3DZ *
gs_get_point3dz_p(GSERIALIZED *gs)
{
  return (POINT3DZ *)((uint8_t*)gs->data + 8);
}

/**
 * Returns a 3DZ point from the datum 
 */
POINT3DZ
datum_get_point3dz(Datum geom)
{
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(geom);
  POINT3DZ *point = (POINT3DZ *)((uint8_t*)gs->data + 8);
  return *point;
}

/**
 * Returns a pointer to a 3DZ point from the datum 
 */
const POINT3DZ *
datum_get_point3dz_p(Datum geom)
{
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(geom);
  return (POINT3DZ *)((uint8_t*)gs->data + 8);
}

/**
 * Returns a 4D point from the datum 
 */
POINT4D
datum_get_point4d(Datum geom)
{
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(geom);
  POINT4D *point = (POINT4D *)((uint8_t*)gs->data + 8);
  return *point;
}

/**
 * Returns true if the two points are equal
 */
bool
datum_point_eq(Datum geopoint1, Datum geopoint2)
{
  GSERIALIZED *gs1 = (GSERIALIZED *) DatumGetPointer(geopoint1);
  GSERIALIZED *gs2 = (GSERIALIZED *) DatumGetPointer(geopoint2);
  if (gserialized_get_srid(gs1) != gserialized_get_srid(gs2) ||
    FLAGS_GET_Z(gs1->flags) != FLAGS_GET_Z(gs2->flags) ||
    FLAGS_GET_GEODETIC(gs1->flags) != FLAGS_GET_GEODETIC(gs2->flags))
    return false;
  if (FLAGS_GET_Z(gs1->flags))
  {
    const POINT3DZ *point1 = gs_get_point3dz_p(gs1);
    const POINT3DZ *point2 = gs_get_point3dz_p(gs2);
    return point1->x == point2->x && point1->y == point2->y &&
      point1->z == point2->z;
  }
  else
  {
    const POINT2D *point1 = gs_get_point2d_p(gs1);
    const POINT2D *point2 = gs_get_point2d_p(gs2);
    return point1->x == point2->x && point1->y == point2->y;
  }
}

/**
 * Returns true encoded as a datum if the two points are equal
 */
Datum
datum2_point_eq(Datum geopoint1, Datum geopoint2)
{
  return BoolGetDatum(datum_point_eq(geopoint1, geopoint2));
}

/**
 * Returns true encoded as a datum if the two points are different
 */
Datum
datum2_point_ne(Datum geopoint1, Datum geopoint2)
{
  return BoolGetDatum(! datum_point_eq(geopoint1, geopoint2));
}

/**
 * Serialize a geometry/geography
 *
 *@pre It is supposed that the flags such as Z and geodetic have been
 * set up before by the calling function
 */
GSERIALIZED *
geo_serialize(LWGEOM *geom)
{
  size_t size;
  GSERIALIZED *result = gserialized_from_lwgeom(geom, &size);
  SET_VARSIZE(result, size);
  return result;
}

/**
 * Call the PostGIS transform function. We need to use the fcinfo cached
 * in the external functions stbox_transform and tpoint_transform
 */
Datum
datum_transform(Datum value, Datum srid)
{
  return CallerFInfoFunctionCall2(transform, (fetch_fcinfo())->flinfo,
    InvalidOid, value, srid);
}

/**
 * Call the PostGIS geometry_from_geography function
 */
static Datum
geog_to_geom(Datum value)
{
  return call_function1(geometry_from_geography, value);
}

/**
 * Call the PostGIS geography_from_geometry function
 */
static Datum
geom_to_geog(Datum value)
{
  return call_function1(geography_from_geometry, value);
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/** 
 * Returns the 2D distance between the two geometries 
 */
Datum
geom_distance2d(Datum geom1, Datum geom2)
{
  return call_function2(distance, geom1, geom2);
}

/** 
 * Returns the 3D distance between the two geometries 
 */
Datum
geom_distance3d(Datum geom1, Datum geom2)
{
  return call_function2(distance3d, geom1, geom2);
}

/** 
 * Returns the distance between the two geographies 
 */
Datum
geog_distance(Datum geog1, Datum geog2)
{
  return CallerFInfoFunctionCall2(geography_distance, (fetch_fcinfo())->flinfo,
    InvalidOid, geog1, geog2);
}

/** 
 * Returns the 2D distance between the two geometric points 
 */
Datum
pt_distance2d(Datum geom1, Datum geom2)
{
  const POINT2D *p1 = datum_get_point2d_p(geom1);
  const POINT2D *p2 = datum_get_point2d_p(geom2);
  return Float8GetDatum(distance2d_pt_pt(p1, p2));
}

/** 
 * Returns the 3D distance between the two geometric points 
 */
Datum
pt_distance3d(Datum geom1, Datum geom2)
{
  const POINT3DZ *p1 = datum_get_point3dz_p(geom1);
  const POINT3DZ *p2 = datum_get_point3dz_p(geom2);
  return Float8GetDatum(distance3d_pt_pt((POINT3D *)p1, (POINT3D *)p2));
}


/*****************************************************************************
 * Trajectory functions.
 *****************************************************************************/

/**
 * Assemble the set of points of a temporal instant set geometry point as a
 * single geometry.
 * @note Duplicate points are removed.
 */
static Datum
tgeompointi_trajectory(const TInstantSet *ti)
{
  /* Singleton instant set */
  if (ti->count == 1)
    return tinstant_value_copy(tinstantset_inst_n(ti, 0));

  LWPOINT **points = palloc(sizeof(LWPOINT *) * ti->count);
  /* Remove all duplicate points */
  TInstant *inst = tinstantset_inst_n(ti, 0);
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(tinstant_value_ptr(inst));
  LWPOINT *value = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
  points[0] = value;
  int k = 1;
  for (int i = 1; i < ti->count; i++)
  {
    inst = tinstantset_inst_n(ti, i);
    gs = (GSERIALIZED *) DatumGetPointer(tinstant_value_ptr(inst));
    value = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
    bool found = false;
    for (int j = 0; j < k; j++)
    {
      if (lwpoint_same(value, points[j]) == LW_TRUE)
      {
        found = true;
        break;
      }
    }
    if (!found)
      points[k++] = value;
  }
  LWGEOM *lwresult;
  if (k == 1)
  {
    lwresult = (LWGEOM *) points[0];
  }
  else
  {
    lwresult = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
      points[0]->srid, NULL, (uint32_t) k, (LWGEOM **) points);
    for (int i = 0; i < k; i++)
      lwpoint_free(points[i]);
  }
  Datum result = PointerGetDatum(geo_serialize(lwresult));
  pfree(points);
  return result;
}

/**
 * Assemble the set of points of a temporal instant set as a
 * single geography/geography.
 */
Datum
tpointinstset_trajectory(const TInstantSet *ti)
{
  Datum result;
  if (MOBDB_FLAGS_GET_GEODETIC(ti->flags))
  {
    /* We only need to fill these parameters for tfunc_tinstantset */
    LiftedFunctionInfo lfinfo;
    lfinfo.func = (varfunc) &geog_to_geom;
    lfinfo.numparam = 1;
    lfinfo.restypid = type_oid(T_GEOMETRY);
    TInstantSet *tigeom = tfunc_tinstantset(ti, (Datum) NULL, lfinfo);
    Datum geomtraj = tgeompointi_trajectory(tigeom);
    result = call_function1(geography_from_geometry, geomtraj);
    pfree(DatumGetPointer(geomtraj));
  }
  else
    result = tgeompointi_trajectory(ti);
  return result;
}

/*****************************************************************************/

/**
 * Compute the trajectory from the two geometry points
 */
LWLINE *
geopoint_lwline(Datum value1, Datum value2)
{
  GSERIALIZED *gs1 = (GSERIALIZED *)DatumGetPointer(value1);
  GSERIALIZED *gs2 = (GSERIALIZED *)DatumGetPointer(value2);
  LWGEOM *geoms[2];
  geoms[0] = lwgeom_from_gserialized(gs1);
  geoms[1] = lwgeom_from_gserialized(gs2);
  LWLINE *result = lwline_from_lwgeom_array(geoms[0]->srid, 2, geoms);
  FLAGS_SET_Z(result->flags, FLAGS_GET_Z(geoms[0]->flags));
  FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(geoms[0]->flags));
  lwgeom_free(geoms[0]); lwgeom_free(geoms[1]);
  return result;
}

/**
 * Compute the trajectory from the two points
 *
 * @param[in] value1,value2 Points
 * @note Function called during normalization for determining whether three 
 * consecutive points are collinear, for computing the temporal distance,
 * the temporal spatial relationships, etc. 
 */
Datum
geopoint_line(Datum value1, Datum value2)
{
  LWGEOM *traj = (LWGEOM *)geopoint_lwline(value1, value2);
  GSERIALIZED *result = geo_serialize(traj);
  lwgeom_free(traj);
  return PointerGetDatum(result);
}

/*****************************************************************************/

/**
 * Compute a trajectory from a set of points. The result is either a line or
 * a multipoint depending on whether the interpolation is step or linear
 *
 * @param[in] lwpoints Array of points
 * @param[in] count Number of elements in the input array
 * @param[in] linear True when the interpolation is linear
 */
static Datum
lwpointarr_make_trajectory(LWGEOM **lwpoints, int count, bool linear)
{
  LWGEOM *lwgeom = linear ?
    (LWGEOM *) lwline_from_lwgeom_array(lwpoints[0]->srid,
      (uint32_t) count, lwpoints) :
    (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE, lwpoints[0]->srid,
      NULL, (uint32_t) count, lwpoints);
  FLAGS_SET_Z(lwgeom->flags, FLAGS_GET_Z(lwpoints[0]->flags));
  FLAGS_SET_GEODETIC(lwgeom->flags, FLAGS_GET_GEODETIC(lwpoints[0]->flags));
  Datum result = PointerGetDatum(geo_serialize(lwgeom));
  pfree(lwgeom);
  return result;
}

/**
 * Compute the trajectory of an array of instants.
 *
 * @note This function is called by the constructor of a temporal sequence
 * and returns a single Datum which is a geometry/geography.
 * Since the composing points have been already validated in the constructor
 * there is no verification of the input in this function, in particular
 * for geographies it is supposed that the composing points are geodetic 
 *
 * @param[in] instants Array of temporal instants
 * @param[in] count Number of elements in the input array
 * @param[in] linear True when the interpolation is linear
 */
Datum
tpointseq_make_trajectory(TInstant **instants, int count, bool linear)
{
  LWPOINT **points = palloc(sizeof(LWPOINT *) * count);
  LWPOINT *lwpoint;
  Datum value;
  GSERIALIZED *gs;
  int k;
  if (linear)
  {
    /* Remove two consecutive points if they are equal */
    value = tinstant_value(instants[0]);
    gs = (GSERIALIZED *) DatumGetPointer(value);
    points[0] = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
    k = 1;
    for (int i = 1; i < count; i++)
    {
      value = tinstant_value(instants[i]);
      gs = (GSERIALIZED *) DatumGetPointer(value);
      lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
      if (! lwpoint_same(lwpoint, points[k - 1]))
        points[k++] = lwpoint;
    }
  }
  else
  {
     /* Remove all duplicate points */
    k = 0;
    for (int i = 0; i < count; i++)
    {
      value = tinstant_value(instants[i]);
      gs = (GSERIALIZED *) DatumGetPointer(value);
      lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
      bool found = false;
      for (int j = 0; j < k; j++)
      {
        if (lwpoint_same(lwpoint, points[j]) == LW_TRUE)
        {
          found = true;
          break;
        }
      }
      if (!found)
        points[k++] = lwpoint;
    }
  }
  Datum result = (k == 1) ?
    PointerGetDatum(geo_serialize((LWGEOM *)points[0])) :
    lwpointarr_make_trajectory((LWGEOM **)points, k, linear);
  for (int i = 0; i < k; i++)
    lwpoint_free(points[i]);
  pfree(points);
  return result;
}

/**
 * Returns the precomputed trajectory of a temporal sequence point
 */
Datum
tpointseq_trajectory(const TSequence *seq)
{
  void *traj = (char *)(&seq->offsets[seq->count + 2]) +   /* start of data */
    seq->offsets[seq->count + 1];            /* offset */
  return PointerGetDatum(traj);
}

/**
 * Copy the precomputed trajectory of a temporal sequence point
 */
Datum
tpointseq_trajectory_copy(const TSequence *seq)
{
  void *traj = (char *)(&seq->offsets[seq->count + 2]) +   /* start of data */
      seq->offsets[seq->count + 1];          /* offset */
  return PointerGetDatum(gserialized_copy(traj));
}

/*****************************************************************************/

/**
 * Returns the trajectory of a temporal geography point with sequence set 
 * duration from the precomputed trajectories of its composing segments.
 * 
 * @note The resulting trajectory must be freed by the calling function.
 * The function removes duplicates points.
 */
static Datum
tgeompoints_trajectory(const TSequenceSet *ts)
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tpointseq_trajectory_copy(tsequenceset_seq_n(ts, 0));

  LWPOINT **points = palloc(sizeof(LWPOINT *) * ts->totalcount);
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ts->count);
  int k = 0, l = 0;
  for (int i = 0; i < ts->count; i++)
  {
    Datum traj = tpointseq_trajectory(tsequenceset_seq_n(ts, i));
    GSERIALIZED *gstraj = (GSERIALIZED *)DatumGetPointer(traj);
    LWPOINT *lwpoint;
    if (gserialized_get_type(gstraj) == POINTTYPE)
    {
      lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(gstraj));
      bool found = false;
      for (int j = 0; j < l; j++)
      {
        if (lwpoint_same(lwpoint, points[j]) == LW_TRUE)
        {
          found = true;
          break;
        }
      }
      if (!found)
        points[l++] = lwpoint;
    }
    else if (gserialized_get_type(gstraj) == MULTIPOINTTYPE)
    {
      LWMPOINT *lwmpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(gstraj));
      int count = lwmpoint->ngeoms;
      for (int m = 0; m < count; m++)
      {
        lwpoint = lwmpoint->geoms[m];
        bool found = false;
        for (int j = 0; j < l; j++)
        {
          if (lwpoint_same(lwpoint, points[j]) == LW_TRUE)
            {
              found = true;
              break;
            }
        }
        if (!found)
          points[l++] = lwpoint;
      }
    }
    /* gserialized_get_type(gstraj) == LINETYPE */
    else
    {
      geoms[k++] = lwgeom_from_gserialized(gstraj);
    }
  }
  Datum result;
  if (k == 0)
  {
    /* Only points */
    if (l == 1)
      result = PointerGetDatum(geo_serialize((LWGEOM *)points[0]));
    else
      result = lwpointarr_make_trajectory((LWGEOM **)points, l, false);
  }
  else if (l == 0)
  {
    /* Only lines */
    /* k > 1 since otherwise it is a singleton sequence set and this case
     * was taken care at the begining of the function */
    // TODO add the bounding box instead of ask PostGIS to compute it again
    // GBOX *box = stbox_to_gbox(tsequence_bbox_ptr(seq));
    LWGEOM *coll = (LWGEOM *) lwcollection_construct(MULTILINETYPE,
      geoms[0]->srid, NULL, (uint32_t) k, geoms);
    result = PointerGetDatum(geo_serialize(coll));
    /* We cannot lwgeom_free(geoms[i] or lwgeom_free(coll) */
  }
  else
  {
    /* Both points and lines */
    if (l == 1)
      geoms[k++] = (LWGEOM *)points[0];
    else
    {
      geoms[k++] = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
        points[0]->srid, NULL, (uint32_t) l, (LWGEOM **) points);
      for (int i = 0; i < l; i++)
        lwpoint_free(points[i]);
    }
    // TODO add the bounding box instead of ask PostGIS to compute it again
    // GBOX *box = stbox_to_gbox(tsequence_bbox_ptr(seq));
    LWGEOM *coll = (LWGEOM *) lwcollection_construct(COLLECTIONTYPE,
      geoms[0]->srid, NULL, (uint32_t) k, geoms);
    result = PointerGetDatum(geo_serialize(coll));
  }
  pfree(points); pfree(geoms);
  return result;
}

/**
 * Returns the trajectory of a temporal geography point with sequence set 
 * duration
 */
static Datum
tgeogpoints_trajectory(const TSequenceSet *ts)
{
  /* We only need to fill these parameters for tfunc_tsequenceset */
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) &geog_to_geom;
  lfinfo.numparam = 1;
  lfinfo.restypid = type_oid(T_GEOMETRY);
  TSequenceSet *tsgeom = tfunc_tsequenceset(ts, (Datum) NULL, lfinfo);
  Datum geomtraj = tgeompoints_trajectory(tsgeom);
  Datum result = call_function1(geography_from_geometry, geomtraj);
  pfree(DatumGetPointer(geomtraj));
  return result;
}

/**
 * Returns the trajectory of a temporal sequence set point
 */
Datum
tpointseqset_trajectory(const TSequenceSet *ts)
{
  return MOBDB_FLAGS_GET_GEODETIC(ts->flags) ?
    tgeogpoints_trajectory(ts) : tgeompoints_trajectory(ts);
}

/*****************************************************************************/

/**
 * Returns the trajectory of a temporal point (dispatch function)
 */
Datum
tpoint_trajectory_internal(const Temporal *temp)
{
  Datum result;
  ensure_valid_duration(temp->duration);
  if (temp->duration == INSTANT)
    result = tinstant_value_copy((TInstant *)temp);
  else if (temp->duration == INSTANTSET)
    result = tpointinstset_trajectory((TInstantSet *)temp);
  else if (temp->duration == SEQUENCE)
    result = tpointseq_trajectory_copy((TSequence *)temp);
  else /* temp->duration == SEQUENCESET */
    result = tpointseqset_trajectory((TSequenceSet *)temp);
  return result;
}

PG_FUNCTION_INFO_V1(tpoint_trajectory);
/**
 * Returns the trajectory of a temporal point
 */
PGDLLEXPORT Datum
tpoint_trajectory(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum result = tpoint_trajectory_internal(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

PG_FUNCTION_INFO_V1(stbox_srid);
/**
 * Returns the SRID of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_srid(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_INT32(box->srid);
}

PG_FUNCTION_INFO_V1(stbox_set_srid);
/**
 * Sets the SRID of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_set_srid(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  int32 srid = PG_GETARG_INT32(1);
  STBOX *result = stbox_copy(box);
  result->srid = srid;
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Returns the SRID of a temporal instant point 
 */
int
tpointinst_srid(const TInstant *inst)
{
  GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(tinstant_value_ptr(inst));
  return gserialized_get_srid(gs);
}

/**
 * Returns the SRID of a temporal instant set point
 */
int
tpointinstset_srid(const TInstantSet *ti)
{
  STBOX *box = tinstantset_bbox_ptr(ti);
  return box->srid;
}

/**
 * Returns the SRID of a temporal sequence point
 */
int
tpointseq_srid(const TSequence *seq)
{
  STBOX *box = tsequence_bbox_ptr(seq);
  return box->srid;
}

/**
 * Returns the SRID of a temporal sequence set point
 */
int
tpointseqset_srid(const TSequenceSet *ts)
{
  STBOX *box = tsequenceset_bbox_ptr(ts);
  return box->srid;
}

/**
 * Returns the SRID of a temporal point (dispatch function)
 */
int
tpoint_srid_internal(const Temporal *temp)
{
  int result;
  ensure_valid_duration(temp->duration);
  if (temp->duration == INSTANT)
    result = tpointinst_srid((TInstant *)temp);
  else if (temp->duration == INSTANTSET)
    result = tpointinstset_srid((TInstantSet *)temp);
  else if (temp->duration == SEQUENCE)
    result = tpointseq_srid((TSequence *)temp);
  else /* temp->duration == SEQUENCESET */
    result = tpointseqset_srid((TSequenceSet *)temp);
  return result;
}

PG_FUNCTION_INFO_V1(tpoint_srid);
/**
 * Returns the SRID of a temporal point
 */
PGDLLEXPORT Datum
tpoint_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  int result = tpoint_srid_internal(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

/*****************************************************************************/

/**
 * Set the SRID of a temporal instant point 
 */
static TInstant *
tpointinst_set_srid(TInstant *inst, int32 srid)
{
  TInstant *result = tinstant_copy(inst);
  GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(tinstant_value_ptr(result));
  gserialized_set_srid(gs, srid);
  return result;
}

/**
 * Set the SRID of a temporal instant set point 
 */
static TInstantSet *
tpointinstset_set_srid(TInstantSet *ti, int32 srid)
{
  TInstantSet *result = tinstantset_copy(ti);
  for (int i = 0; i < ti->count; i++)
  {
    TInstant *inst = tinstantset_inst_n(result, i);
    GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(tinstant_value_ptr(inst));
    gserialized_set_srid(gs, srid);
  }
  STBOX *box = tinstantset_bbox_ptr(result);
  box->srid = srid;
  return result;
}

/**
 * Set the SRID of a temporal sequence point 
 */
static TSequence *
tpointseq_set_srid(TSequence *seq, int32 srid)
{
  TSequence *result = tsequence_copy(seq);
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = tsequence_inst_n(result, i);
    GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(tinstant_value_ptr(inst));
    gserialized_set_srid(gs, srid);
  }
  STBOX *box = tsequence_bbox_ptr(result);
  box->srid = srid;
  return result;
}

/**
 * Set the SRID of a temporal sequence set point 
 */
static TSequenceSet *
tpointseqset_set_srid(TSequenceSet *ts, int32 srid)
{
  TSequenceSet *result = tsequenceset_copy(ts);
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(result, i);
    for (int j = 0; j < seq->count; j++)
    {
      TInstant *inst = tsequence_inst_n(seq, j);
      GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(tinstant_value_ptr(inst));
      gserialized_set_srid(gs, srid);
    }
  }
  STBOX *box = tsequenceset_bbox_ptr(result);
  box->srid = srid;
  return result;
}

/**
 * Set the SRID of a temporal point (dispatch function)
 */
Temporal *
tpoint_set_srid_internal(Temporal *temp, int32 srid)
{
  Temporal *result;
  if (temp->duration == INSTANT)
    result = (Temporal *)tpointinst_set_srid((TInstant *)temp, srid);
  else if (temp->duration == INSTANTSET)
    result = (Temporal *)tpointinstset_set_srid((TInstantSet *)temp, srid);
  else if (temp->duration == SEQUENCE)
    result = (Temporal *)tpointseq_set_srid((TSequence *)temp, srid);
  else /* temp->duration == SEQUENCESET */
    result = (Temporal *)tpointseqset_set_srid((TSequenceSet *)temp, srid);

  assert(result != NULL);
  return result;
}

PG_FUNCTION_INFO_V1(tpoint_set_srid);
/**
 * Set the SRID of a temporal point 
 */
PGDLLEXPORT Datum
tpoint_set_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  int32 srid = PG_GETARG_INT32(1);
  Temporal *result = tpoint_set_srid_internal(temp, srid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(stbox_transform);
/**
 * Transform a spatiotemporal box into another spatial reference system 
 */
PGDLLEXPORT Datum
stbox_transform(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Datum srid = PG_GETARG_DATUM(1);
  ensure_has_X_stbox(box);
  STBOX *result = stbox_copy(box);
  result->srid = DatumGetInt32(srid);
  bool hasz = MOBDB_FLAGS_GET_Z(box->flags);
  bool geodetic = MOBDB_FLAGS_GET_GEODETIC(box->flags);
  LWPOINT *ptmin, *ptmax;
  if (hasz)
  {
    ptmin = lwpoint_make3dz(box->srid, box->xmin, box->ymin, box->zmin);
    ptmax = lwpoint_make3dz(box->srid, box->xmax, box->ymax, box->zmax);
  }
  else
  {
    ptmin = lwpoint_make2d(box->srid, box->xmin, box->ymin);
    ptmax = lwpoint_make2d(box->srid, box->xmax, box->ymax);
  }
  lwgeom_set_geodetic((LWGEOM *)ptmin, geodetic);
  lwgeom_set_geodetic((LWGEOM *)ptmax, geodetic);
  Datum min = PointerGetDatum(geo_serialize((LWGEOM *)ptmin));
  Datum max = PointerGetDatum(geo_serialize((LWGEOM *)ptmax));
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Datum min1 = datum_transform(min, srid);
  Datum max1 = datum_transform(max, srid);
  if (hasz)
  {
    const POINT3DZ *ptmin1 = datum_get_point3dz_p(min1);
    const POINT3DZ *ptmax1 = datum_get_point3dz_p(max1);
    result->xmin = ptmin1->x;
    result->ymin = ptmin1->y;
    result->zmin = ptmin1->z;
    result->xmax = ptmax1->x;
    result->ymax = ptmax1->y;
    result->zmax = ptmax1->z;
  }
  else
  {
    const POINT2D *ptmin1 = datum_get_point2d_p(min1);
    const POINT2D *ptmax1 = datum_get_point2d_p(max1);
    result->xmin = ptmin1->x;
    result->ymin = ptmin1->y;
    result->xmax = ptmax1->x;
    result->ymax = ptmax1->y;
  }
  lwpoint_free(ptmin); lwpoint_free(ptmax);
  pfree(DatumGetPointer(min)); pfree(DatumGetPointer(max));
  pfree(DatumGetPointer(min1)); pfree(DatumGetPointer(max1));
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Transform a temporal instant point into another spatial reference system 
 */
TInstant *
tpointinst_transform(const TInstant *inst, Datum srid)
{
  Datum geo = datum_transform(tinstant_value(inst), srid);
  TInstant *result = tinstant_make(geo, inst->t, inst->valuetypid);
  pfree(DatumGetPointer(geo));
  return result;
}

/**
 * Transform a temporal instant set point into another spatial reference system 
 */
static TInstantSet *
tpointinstset_transform(const TInstantSet *ti, Datum srid)
{
  TInstant *inst;
  
  /* Singleton instant set */
  if (ti->count == 1)
  {
    inst = tpointinst_transform(tinstantset_inst_n(ti, 0), srid);
    TInstantSet *result = tinstantset_make(&inst, 1);
    pfree(inst);
    return result;
  }

  /* General case */
  LWGEOM **points = palloc(sizeof(LWGEOM *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    Datum value = tinstant_value(tinstantset_inst_n(ti, i));
    GSERIALIZED *gsvalue = (GSERIALIZED *) DatumGetPointer(value);
    points[i] = lwgeom_from_gserialized(gsvalue);
  }
  Datum multipoint = lwpointarr_make_trajectory(points, ti->count, false);
  Datum transf = datum_transform(multipoint, srid);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(transf);
  LWMPOINT *lwmpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    Datum point = PointerGetDatum(geo_serialize((LWGEOM *) (lwmpoint->geoms[i])));
    inst = tinstantset_inst_n(ti, i);
    instants[i] = tinstant_make(point, inst->t, inst->valuetypid);
    pfree(DatumGetPointer(point));
  }
  for (int i = 0; i < ti->count; i++)
    lwpoint_free((LWPOINT *) points[i]);
  pfree(points);
  pfree(DatumGetPointer(multipoint)); pfree(DatumGetPointer(transf));
  POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(gs));
  lwmpoint_free(lwmpoint);

  return tinstantset_make_free(instants, ti->count);
}

/**
 * Transform a temporal sequence point into another spatial reference system 
 */
static TSequence *
tpointseq_transform(const TSequence *seq, Datum srid)
{
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TInstant *inst = tpointinst_transform(tsequence_inst_n(seq, 0), srid);
    TSequence *result = tinstant_to_tsequence(inst, linear);
    pfree(inst);
    return result;
  }

  /* General case */
  LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    Datum value = tinstant_value(tsequence_inst_n(seq, i));
    GSERIALIZED *gsvalue = (GSERIALIZED *)DatumGetPointer(value);
    points[i] = lwgeom_from_gserialized(gsvalue);
  }
  Datum multipoint = lwpointarr_make_trajectory(points, seq->count, false);
  Datum transf = datum_transform(multipoint, srid);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(transf);
  LWMPOINT *lwmpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    Datum point = PointerGetDatum(geo_serialize((LWGEOM *) (lwmpoint->geoms[i])));
    TInstant *inst = tsequence_inst_n(seq, i);
    instants[i] = tinstant_make(point, inst->t, inst->valuetypid);
    pfree(DatumGetPointer(point));
  }

  for (int i = 0; i < seq->count; i++)
    lwpoint_free((LWPOINT *) points[i]);
  pfree(points);
  pfree(DatumGetPointer(multipoint)); pfree(DatumGetPointer(transf));
  POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(gs));
  lwmpoint_free(lwmpoint);

  return tsequence_make_free(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, linear, NORMALIZE_NO);
}

/**
 * Transform a temporal sequence set point into another spatial reference system
 *
 * @note In order to do a SINGLE call to the PostGIS transform function we do
 * not iterate through the sequences and call the transform for the sequence.
 */
static TSequenceSet *
tpointseqset_transform(const TSequenceSet *ts, Datum srid)
{
  /* Singleton sequence set */
  if (ts->count == 1)
  {
    TSequence *seq = tpointseq_transform(tsequenceset_seq_n(ts, 0), srid);
    TSequenceSet *result = tsequence_to_tsequenceset(seq);
    pfree(seq);
    return result;
  }

  /* General case */
  int k = 0;
  LWGEOM **points = palloc(sizeof(LWGEOM *) * ts->totalcount);
  int maxcount = -1; /* number of instants of the longest sequence */
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    maxcount = Max(maxcount, seq->count);
    for (int j = 0; j < seq->count; j++)
    {
      Datum value = tinstant_value(tsequence_inst_n(seq, j));
      GSERIALIZED *gsvalue = (GSERIALIZED *)DatumGetPointer(value);
      points[k++] = lwgeom_from_gserialized(gsvalue);
    }
  }
  Datum multipoint = lwpointarr_make_trajectory(points, ts->totalcount, false);
  Datum transf = datum_transform(multipoint, srid);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(transf);
  LWMPOINT *lwmpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  TInstant **instants = palloc(sizeof(TInstant *) * maxcount);
  bool linear = MOBDB_FLAGS_GET_LINEAR(ts->flags);
  k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    for (int j = 0; j < seq->count; j++)
    {
      Datum point = PointerGetDatum(geo_serialize((LWGEOM *) (lwmpoint->geoms[k++])));
      TInstant *inst = tsequence_inst_n(seq, j);
      instants[j] = tinstant_make(point, inst->t, inst->valuetypid);
      pfree(DatumGetPointer(point));
    }
    sequences[i] = tsequence_make(instants, seq->count,
      seq->period.lower_inc, seq->period.upper_inc, linear, NORMALIZE_NO);
    for (int j = 0; j < seq->count; j++)
      pfree(instants[j]);
  }
  TSequenceSet *result = tsequenceset_make_free(sequences, ts->count, NORMALIZE_NO);
  for (int i = 0; i < ts->totalcount; i++)
    lwpoint_free((LWPOINT *) points[i]);
  pfree(points); pfree(instants);
  pfree(DatumGetPointer(multipoint)); pfree(DatumGetPointer(transf));
  POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(gs));
  lwmpoint_free(lwmpoint);
  return result;
}

PG_FUNCTION_INFO_V1(tpoint_transform);
/**
 * Transform a temporal point into another spatial reference system 
 */
PGDLLEXPORT Datum
tpoint_transform(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum srid = PG_GETARG_DATUM(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);

  Temporal *result;
  ensure_valid_duration(temp->duration);
  if (temp->duration == INSTANT)
    result = (Temporal *) tpointinst_transform((TInstant *)temp, srid);
  else if (temp->duration == INSTANTSET)
    result = (Temporal *) tpointinstset_transform((TInstantSet *)temp, srid);
  else if (temp->duration == SEQUENCE)
    result = (Temporal *) tpointseq_transform((TSequence *)temp, srid);
  else /* temp->duration == SEQUENCESET */
    result = (Temporal *) tpointseqset_transform((TSequenceSet *)temp, srid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 * Notice that a geometry point and a geography point are of different size
 * since the geography point keeps a bounding box
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tgeompoint_to_tgeogpoint);
/**
 * Transform the geometry to a geography 
 */
PGDLLEXPORT Datum
tgeompoint_to_tgeogpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) &geom_to_geog;
  lfinfo.numparam = 1;
  lfinfo.restypid = type_oid(T_GEOGRAPHY);
  Temporal *result = tfunc_temporal(temp, (Datum) NULL, lfinfo);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tgeogpoint_to_tgeompoint);
/**
 * Transform the geography to a geometry
 */
PGDLLEXPORT Datum
tgeogpoint_to_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) &geog_to_geom;
  lfinfo.numparam = 1;
  lfinfo.restypid = type_oid(T_GEOMETRY);
  Temporal *result = tfunc_temporal(temp, (Datum) NULL, lfinfo);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set precision of the coordinates.
 *****************************************************************************/

/**
 * Set the precision of the point coordinates to the number
 * of decimal places
 */
static Datum
datum_set_precision(Datum value, Datum prec)
{
  GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value);
  int srid = gserialized_get_srid(gs);
  LWPOINT *lwpoint;
  if (FLAGS_GET_Z(gs->flags))
  {
    const POINT3DZ *point = gs_get_point3dz_p(gs);
    double x = DatumGetFloat8(datum_round(Float8GetDatum(point->x), prec));
    double y = DatumGetFloat8(datum_round(Float8GetDatum(point->y), prec));
    double z = DatumGetFloat8(datum_round(Float8GetDatum(point->z), prec));
    lwpoint = lwpoint_make3dz(srid, x, y, z);
  }
  else
  {
    const POINT2D *point = gs_get_point2d_p(gs);
    double x = DatumGetFloat8(datum_round(Float8GetDatum(point->x), prec));
    double y = DatumGetFloat8(datum_round(Float8GetDatum(point->y), prec));
    lwpoint = lwpoint_make2d(srid, x, y);
  }
  bool geodetic = FLAGS_GET_GEODETIC(gs->flags);
  FLAGS_SET_GEODETIC(lwpoint->flags, geodetic);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwpoint);
  pfree(lwpoint);
  return PointerGetDatum(result);
}

PG_FUNCTION_INFO_V1(tpoint_set_precision);
/**
 * Set the precision of the coordinates of the temporal point to the number
 * of decimal places
 */
PGDLLEXPORT Datum
tpoint_set_precision(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum size = PG_GETARG_DATUM(1);
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) &datum_set_precision;
  lfinfo.numparam = 2;
  lfinfo.restypid = temp->valuetypid;
  Temporal *result = tfunc_temporal(temp, size, lfinfo);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Length functions
 *****************************************************************************/

/**
 * Returns the length traversed by the temporal sequence point 
 */
static double
tpointseq_length(const TSequence *seq)
{
  assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
  Datum traj = tpointseq_trajectory(seq);
  GSERIALIZED *gstraj = (GSERIALIZED *)DatumGetPointer(traj);
  if (gserialized_get_type(gstraj) == POINTTYPE)
    return 0;

  /* We are sure that the trajectory is a line */
  double result = MOBDB_FLAGS_GET_GEODETIC(seq->flags) ?
    DatumGetFloat8(call_function2(geography_length, traj,
      BoolGetDatum(true))) :
    /* The next function call works for 2D and 3D */
    DatumGetFloat8(call_function1(LWGEOM_length_linestring, traj));
  return result;
}

/**
 * Returns the length traversed by the temporal sequence set point 
 */
static double
tpointseqset_length(const TSequenceSet *ts)
{
  assert(MOBDB_FLAGS_GET_LINEAR(ts->flags));
  double result = 0;
  for (int i = 0; i < ts->count; i++)
    result += tpointseq_length(tsequenceset_seq_n(ts, i));
  return result;
}

PG_FUNCTION_INFO_V1(tpoint_length);
/**
 * Returns the length traversed by the temporal sequence (set) point 
 */
PGDLLEXPORT Datum
tpoint_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  double result = 0.0;
  ensure_valid_duration(temp->duration);
  if (temp->duration == INSTANT || temp->duration == INSTANTSET ||
    ! MOBDB_FLAGS_GET_LINEAR(temp->flags))
    ;
  else if (temp->duration == SEQUENCE)
    result = tpointseq_length((TSequence *)temp);
  else /* temp->duration == SEQUENCESET */
    result = tpointseqset_length((TSequenceSet *)temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/

/**
 * Returns the cumulative length traversed by the temporal instant point 
 */
static TInstant *
tpointinst_cumulative_length(const TInstant *inst)
{
  return tinstant_make(Float8GetDatum(0.0), inst->t, FLOAT8OID);
}

/**
 * Returns the cumulative length traversed by the temporal instant set point 
 */
static TInstantSet *
tpointinstset_cumulative_length(const TInstantSet *ti)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  Datum length = Float8GetDatum(0.0);
  for (int i = 0; i < ti->count; i++)
  {
    TInstant *inst = tinstantset_inst_n(ti, i);
    instants[i] = tinstant_make(length, inst->t, FLOAT8OID);
  }
  return tinstantset_make_free(instants, ti->count);
}

/**
 * Returns the cumulative length traversed by the temporal sequence point 
 */
static TSequence *
tpointseq_cumulative_length(const TSequence *seq, double prevlength)
{
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TInstant *inst = tsequence_inst_n(seq, 0);
    TInstant *inst1 = tinstant_make(Float8GetDatum(0), inst->t,
      FLOAT8OID);
    TSequence *result = tinstant_to_tsequence(inst1, linear);
    pfree(inst1);
    return result;
  }

  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  /* Stepwise interpolation */
  if (! linear)
  {
    Datum length = Float8GetDatum(0.0);
    for (int i = 0; i < seq->count; i++)
    {
      TInstant *inst = tsequence_inst_n(seq, i);
      instants[i] = tinstant_make(length, inst->t, FLOAT8OID);
    }
  }
  else
  /* Linear interpolation */
  {
    Datum (*func)(Datum, Datum);
    if (MOBDB_FLAGS_GET_GEODETIC(seq->flags))
      func = &geog_distance;
    else
      func = MOBDB_FLAGS_GET_Z(seq->flags) ? &pt_distance3d :
        &pt_distance2d;

    TInstant *inst1 = tsequence_inst_n(seq, 0);
    Datum value1 = tinstant_value(inst1);
    double length = prevlength;
    instants[0] = tinstant_make(Float8GetDatum(length), inst1->t,
        FLOAT8OID);
    for (int i = 1; i < seq->count; i++)
    {
      TInstant *inst2 = tsequence_inst_n(seq, i);
      Datum value2 = tinstant_value(inst2);
      if (datum_ne(value1, value2, inst1->valuetypid))
        length += DatumGetFloat8(func(value1, value2));
      instants[i] = tinstant_make(Float8GetDatum(length), inst2->t,
        FLOAT8OID);
      inst1 = inst2;
      value1 = value2;
    }
  }
  TSequence *result = tsequence_make(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, linear, NORMALIZE);

  for (int i = 1; i < seq->count; i++)
    pfree(instants[i]);
  pfree(instants);

  return result;
}

/**
 * Returns the cumulative length traversed by the temporal sequence set point 
 */
static TSequenceSet *
tpointseqset_cumulative_length(const TSequenceSet *ts)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  double length = 0;
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    sequences[i] = tpointseq_cumulative_length(seq, length);
    TInstant *end = tsequence_inst_n(sequences[i], seq->count - 1);
    length = DatumGetFloat8(tinstant_value(end));
  }
  TSequenceSet *result = tsequenceset_make(sequences, ts->count,
    NORMALIZE_NO);

  for (int i = 1; i < ts->count; i++)
    pfree(sequences[i]);
  pfree(sequences);

  return result;
}

PG_FUNCTION_INFO_V1(tpoint_cumulative_length);
/**
 * Returns the cumulative length traversed by the temporal point 
 */
PGDLLEXPORT Datum
tpoint_cumulative_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result;
  ensure_valid_duration(temp->duration);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  if (temp->duration == INSTANT)
    result = (Temporal *)tpointinst_cumulative_length((TInstant *)temp);
  else if (temp->duration == INSTANTSET)
    result = (Temporal *)tpointinstset_cumulative_length((TInstantSet *)temp);
  else if (temp->duration == SEQUENCE)
    result = (Temporal *)tpointseq_cumulative_length((TSequence *)temp, 0);
  else /* temp->duration == SEQUENCESET */
    result = (Temporal *)tpointseqset_cumulative_length((TSequenceSet *)temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

/**
 * Returns the speed of the temporal point in the segment
 *
 * @param[in] inst1, inst2 Instants defining the segment
 * @param[in] func Distance function (2D, 3D, or geodetic)
 */
static double
tpointinst_speed(const TInstant *inst1, const TInstant *inst2,
  Datum (*func)(Datum, Datum))
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  return datum_point_eq(value1, value2) ? 0 :
    DatumGetFloat8(func(value1, value2)) / ((double)(inst2->t - inst1->t) / 1000000);
}

/**
 * Returns the speed of the temporal point in the temporal sequence point
 */
static TSequence *
tpointseq_speed(const TSequence *seq)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  /* Stepwise interpolation */
  if (! MOBDB_FLAGS_GET_LINEAR(seq->flags))
  {
    Datum length = Float8GetDatum(0.0);
    for (int i = 0; i < seq->count; i++)
    {
      TInstant *inst = tsequence_inst_n(seq, i);
      instants[i] = tinstant_make(length, inst->t, FLOAT8OID);
    }
  }
  else
  /* Linear interpolation */
  {
    Datum (*func)(Datum, Datum);
    if (MOBDB_FLAGS_GET_GEODETIC(seq->flags))
      func = &geog_distance;
    else
      func = MOBDB_FLAGS_GET_Z(seq->flags) ? 
        &pt_distance3d : &pt_distance2d;

    TInstant *inst1 = tsequence_inst_n(seq, 0);
    Datum value1 = tinstant_value(inst1);
    double speed;
    for (int i = 0; i < seq->count - 1; i++)
    {
      TInstant *inst2 = tsequence_inst_n(seq, i + 1);
      Datum value2 = tinstant_value(inst2);
      if (datum_point_eq(value1, value2))
        speed = 0;
      else
        speed = DatumGetFloat8(func(value1, value2)) / ((double)(inst2->t - inst1->t) / 1000000);
      instants[i] = tinstant_make(Float8GetDatum(speed), inst1->t,
        FLOAT8OID);
      inst1 = inst2;
      value1 = value2;
    }
    instants[seq->count - 1] = tinstant_make(Float8GetDatum(speed),
      seq->period.upper, FLOAT8OID);
  }
  /* The resulting sequence has step interpolation */
  TSequence *result = tsequence_make(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE);
  for (int i = 0; i < seq->count - 1; i++)
    pfree(instants[i]);
  pfree(instants);
  return result;
}

/**
 * Returns the speed of the temporal point in the temporal sequence set point
 */
static TSequenceSet *
tpointseqset_speed(const TSequenceSet *ts)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    if (seq->count > 1)
      sequences[k++] = tpointseq_speed(seq);
  }
  /* The resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

PG_FUNCTION_INFO_V1(tpoint_speed);
/**
 * Returns the speed of the temporal point in the temporal sequence (set) point
 */
PGDLLEXPORT Datum
tpoint_speed(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result = NULL;
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  ensure_valid_duration(temp->duration);
  if (temp->duration == INSTANT || temp->duration == INSTANTSET)
    ;
  else if (temp->duration == SEQUENCE)
    result = (Temporal *)tpointseq_speed((TSequence *)temp);
  else /* temp->duration == SEQUENCESET */
    result = (Temporal *)tpointseqset_speed((TSequenceSet *)temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Time-weighed centroid for temporal geometry points
 *****************************************************************************/

/**
 * Returns the time-weighed centroid of the temporal geometry point of 
 * instant set duration
 */
Datum
tgeompointi_twcentroid(const TInstantSet *ti)
{
  int srid = tpointinstset_srid(ti);
  TInstant **instantsx = palloc(sizeof(TInstant *) * ti->count);
  TInstant **instantsy = palloc(sizeof(TInstant *) * ti->count);
  TInstant **instantsz = NULL; /* keep compiler quiet */
  bool hasz = MOBDB_FLAGS_GET_Z(ti->flags);
  if (hasz)
    instantsz = palloc(sizeof(TInstant *) * ti->count);

  for (int i = 0; i < ti->count; i++)
  {
    TInstant *inst = tinstantset_inst_n(ti, i);
    POINT4D point = datum_get_point4d(tinstant_value(inst));
    instantsx[i] = tinstant_make(Float8GetDatum(point.x), inst->t,
      FLOAT8OID);
    instantsy[i] = tinstant_make(Float8GetDatum(point.y), inst->t,
      FLOAT8OID);
    if (hasz)
      instantsz[i] = tinstant_make(Float8GetDatum(point.z), inst->t,
        FLOAT8OID);
  }
  TInstantSet *tix = tinstantset_make_free(instantsx, ti->count);
  TInstantSet *tiy = tinstantset_make_free(instantsy, ti->count);
  TInstantSet *tiz = NULL; /* keep compiler quiet */
  if (hasz)
    tiz = tinstantset_make_free(instantsz, ti->count);
  double avgx = tnumberinstset_twavg(tix);
  double avgy = tnumberinstset_twavg(tiy);
  double avgz;
  if (hasz)
    avgz = tnumberinstset_twavg(tiz);
  LWPOINT *lwpoint;
  if (hasz)
    lwpoint = lwpoint_make3dz(srid, avgx, avgy, avgz);
  else
    lwpoint = lwpoint_make2d(srid, avgx, avgy);
  Datum result = PointerGetDatum(geo_serialize((LWGEOM *)lwpoint));

  pfree(lwpoint);
  pfree(tix); pfree(tiy);
  if (hasz)
    pfree(tiz);

  return result;
}

/**
 * Returns the time-weighed centroid of the temporal geometry point of 
 * sequence duration
 */
Datum
tgeompointseq_twcentroid(const TSequence *seq)
{
  int srid = tpointseq_srid(seq);
  TInstant **instantsx = palloc(sizeof(TInstant *) * seq->count);
  TInstant **instantsy = palloc(sizeof(TInstant *) * seq->count);
  TInstant **instantsz;
  bool hasz = MOBDB_FLAGS_GET_Z(seq->flags);
  if (hasz)
    instantsz = palloc(sizeof(TInstant *) * seq->count);

  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = tsequence_inst_n(seq, i);
    POINT4D point = datum_get_point4d(tinstant_value(inst));
    instantsx[i] = tinstant_make(Float8GetDatum(point.x), inst->t,
      FLOAT8OID);
    instantsy[i] = tinstant_make(Float8GetDatum(point.y), inst->t,
      FLOAT8OID);
    if (hasz)
      instantsz[i] = tinstant_make(Float8GetDatum(point.z), inst->t,
        FLOAT8OID);
  }
  TSequence *seqx = tsequence_make_free(instantsx, seq->count,
    seq->period.lower_inc, seq->period.upper_inc,
    MOBDB_FLAGS_GET_LINEAR(seq->flags), NORMALIZE);
  TSequence *seqy = tsequence_make_free(instantsy, seq->count,
    seq->period.lower_inc, seq->period.upper_inc,
    MOBDB_FLAGS_GET_LINEAR(seq->flags), NORMALIZE);
  TSequence *seqz;
  if (hasz)
    seqz = tsequence_make_free(instantsz, seq->count, seq->period.lower_inc,
      seq->period.upper_inc, MOBDB_FLAGS_GET_LINEAR(seq->flags), NORMALIZE);
  double twavgx = tnumberseq_twavg(seqx);
  double twavgy = tnumberseq_twavg(seqy);
  LWPOINT *lwpoint;
  if (hasz)
  {
    double twavgz = tnumberseq_twavg(seqz);
    lwpoint = lwpoint_make3dz(srid, twavgx, twavgy, twavgz);
  }
  else
    lwpoint = lwpoint_make2d(srid, twavgx, twavgy);
  Datum result = PointerGetDatum(geo_serialize((LWGEOM *)lwpoint));

  pfree(lwpoint);  pfree(seqx); pfree(seqy);
  if (hasz)
    pfree(seqz);

  return result;
}

/**
 * Returns the time-weighed centroid of the temporal geometry point of 
 * sequence set duration
 */
Datum
tgeompoints_twcentroid(const TSequenceSet *ts)
{
  int srid = tpointseqset_srid(ts);
  TSequence **sequencesx = palloc(sizeof(TSequence *) * ts->count);
  TSequence **sequencesy = palloc(sizeof(TSequence *) * ts->count);
  TSequence **sequencesz = NULL; /* keep compiler quiet */
  bool hasz = MOBDB_FLAGS_GET_Z(ts->flags);
  if (hasz)
    sequencesz = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    TInstant **instantsx = palloc(sizeof(TInstant *) * seq->count);
    TInstant **instantsy = palloc(sizeof(TInstant *) * seq->count);
    TInstant **instantsz;
    if (hasz)
      instantsz = palloc(sizeof(TInstant *) * seq->count);
    for (int j = 0; j < seq->count; j++)
    {
      TInstant *inst = tsequence_inst_n(seq, j);
      POINT4D point = datum_get_point4d(tinstant_value(inst));
      instantsx[j] = tinstant_make(Float8GetDatum(point.x),
        inst->t, FLOAT8OID);
      instantsy[j] = tinstant_make(Float8GetDatum(point.y),
        inst->t, FLOAT8OID);
      if (hasz)
        instantsz[j] = tinstant_make(Float8GetDatum(point.z),
          inst->t, FLOAT8OID);
    }
    sequencesx[i] = tsequence_make_free(instantsx, seq->count,
      seq->period.lower_inc, seq->period.upper_inc,
      MOBDB_FLAGS_GET_LINEAR(seq->flags), NORMALIZE);
    sequencesy[i] = tsequence_make_free(instantsy,
      seq->count, seq->period.lower_inc, seq->period.upper_inc,
      MOBDB_FLAGS_GET_LINEAR(seq->flags), NORMALIZE);
    if (hasz)
      sequencesz[i] = tsequence_make_free(instantsz, seq->count,
        seq->period.lower_inc, seq->period.upper_inc,
        MOBDB_FLAGS_GET_LINEAR(seq->flags), NORMALIZE);
  }
  TSequenceSet *tsx = tsequenceset_make_free(sequencesx, ts->count, NORMALIZE);
  TSequenceSet *tsy = tsequenceset_make_free(sequencesy, ts->count, NORMALIZE);
  TSequenceSet *tsz = NULL; /* keep compiler quiet */
  if (hasz)
    tsz = tsequenceset_make_free(sequencesz, ts->count, NORMALIZE);

  double twavgx = tnumberseqset_twavg(tsx);
  double twavgy = tnumberseqset_twavg(tsy);
  LWPOINT *lwpoint;
  if (hasz)
  {
    double twavgz = tnumberseqset_twavg(tsz);
    lwpoint = lwpoint_make3dz(srid, twavgx, twavgy, twavgz);
  }
  else
    lwpoint = lwpoint_make2d(srid, twavgx, twavgy);
  Datum result = PointerGetDatum(geo_serialize((LWGEOM *)lwpoint));

  pfree(lwpoint);
  pfree(tsx); pfree(tsy);
  if (hasz)
    pfree(tsz);

  return result;
}

/**
 * Returns the time-weighed centroid of the temporal geometry point
 * (dispatch function)
 */
Datum
tgeompoint_twcentroid_internal(Temporal *temp)
{
  Datum result;
  ensure_valid_duration(temp->duration);
  if (temp->duration == INSTANT)
    result = tinstant_value_copy((TInstant *)temp);
  else if (temp->duration == INSTANTSET)
    result = tgeompointi_twcentroid((TInstantSet *)temp);
  else if (temp->duration == SEQUENCE)
    result = tgeompointseq_twcentroid((TSequence *)temp);
  else /* temp->duration == SEQUENCESET */
    result = tgeompoints_twcentroid((TSequenceSet *)temp);
  return result;
}

PG_FUNCTION_INFO_V1(tgeompoint_twcentroid);
/**
 * Returns the time-weighed centroid of the temporal geometry point
 */
PGDLLEXPORT Datum
tgeompoint_twcentroid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum result = tgeompoint_twcentroid_internal(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

/**
 * Returns the azimuth of the two geometry points 
 */
static Datum
geom_azimuth(Datum geom1, Datum geom2)
{
  const POINT2D *p1 = datum_get_point2d_p(geom1);
  const POINT2D *p2 = datum_get_point2d_p(geom2);
  double result;
  azimuth_pt_pt(p1, p2, &result);
  return Float8GetDatum(result);
}

/**
 * Returns the azimuth the two geography points 
 */
static Datum
geog_azimuth(Datum geom1, Datum geom2)
{
  return CallerFInfoFunctionCall2(geography_azimuth, (fetch_fcinfo())->flinfo,
    InvalidOid, geom1, geom2);
}

/**
 * Returns the temporal azimuth of the temporal geometry point of 
 * sequence duration
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq Temporal value
 */
static int
tpointseq_azimuth1(TSequence **result, const TSequence *seq)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return 0;

  /* Determine the PostGIS function to call */
  Datum (*func)(Datum, Datum) = MOBDB_FLAGS_GET_GEODETIC(seq->flags) ?
    &geog_azimuth : &geom_azimuth;

  /* We are sure that there are at least 2 instants */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  TInstant *inst1 = tsequence_inst_n(seq, 0);
  Datum value1 = tinstant_value(inst1);
  int k = 0, l = 0;
  Datum azimuth = 0; /* Make the compiler quiet */
  bool lower_inc = seq->period.lower_inc, upper_inc;
  for (int i = 1; i < seq->count; i++)
  {
    TInstant *inst2 = tsequence_inst_n(seq, i);
    Datum value2 = tinstant_value(inst2);
    upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    if (datum_ne(value1, value2, seq->valuetypid))
    {
      azimuth = func(value1, value2);
      instants[k++] = tinstant_make(azimuth, inst1->t, FLOAT8OID);
    }
    else
    {
      if (k != 0)
      {
        instants[k++] = tinstant_make(azimuth, inst1->t, FLOAT8OID);
        upper_inc = true;
        /* Resulting sequence has step interpolation */
        result[l++] = tsequence_make(instants, k, lower_inc,
          upper_inc, STEP, NORMALIZE);
        for (int j = 0; j < k; j++)
          pfree(instants[j]);
        k = 0;
      }
      lower_inc = true;
    }
    inst1 = inst2;
    value1 = value2;
  }
  if (k != 0)
  {
    instants[k++] = tinstant_make(azimuth, inst1->t, FLOAT8OID);
    /* Resulting sequence has step interpolation */
    result[l++] = tsequence_make(instants, k, lower_inc, upper_inc,
      STEP, NORMALIZE);
  }

  pfree(instants);

  return l;
}

/**
 * Returns the temporal azimuth of the temporal geometry point
 * of sequence duration
 */
TSequenceSet *
tpointseq_azimuth(TSequence *seq)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count);
  int count = tpointseq_azimuth1(sequences, seq);
  /* Resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Returns the temporal azimuth of the temporal geometry point
 * of sequence set duration
 */
TSequenceSet *
tpointseqset_azimuth(TSequenceSet *ts)
{
  if (ts->count == 1)
    return tpointseq_azimuth(tsequenceset_seq_n(ts, 0));

  TSequence **sequences = palloc(sizeof(TSequence *) * ts->totalcount);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tpointseq_azimuth1(&sequences[k], seq);
  }
  /* Resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

PG_FUNCTION_INFO_V1(tpoint_azimuth);
/**
 * Returns the temporal azimuth of the temporal geometry point
 */
PGDLLEXPORT Datum
tpoint_azimuth(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = NULL;
  ensure_valid_duration(temp->duration);
  if (temp->duration == INSTANT || temp->duration == INSTANTSET ||
    (temp->duration == SEQUENCE && ! MOBDB_FLAGS_GET_LINEAR(temp->flags)) ||
    (temp->duration == SEQUENCESET && ! MOBDB_FLAGS_GET_LINEAR(temp->flags)))
    ;
  else if (temp->duration == SEQUENCE)
    result = (Temporal *)tpointseq_azimuth((TSequence *)temp);
  else /* temp->duration == SEQUENCESET */
    result = (Temporal *)tpointseqset_azimuth((TSequenceSet *)temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Restriction functions
 * N.B. In the current version of PostGIS (2.5) there is no true 
 * ST_Intersection function for geography
 *****************************************************************************/

/**
 * Restricts the temporal instant point to the geometry 
 */
static TInstant *
tpointinst_at_geometry(const TInstant *inst, Datum geom)
{
  if (!DatumGetBool(call_function2(intersects, tinstant_value(inst), geom)))
    return NULL;
  return tinstant_copy(inst);
}

/**
 * Restricts the temporal instant set point to the geometry 
 */
static TInstantSet *
tpointinstset_at_geometry(const TInstantSet *ti, Datum geom)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int k = 0;
  for (int i = 0; i < ti->count; i++)
  {
    TInstant *inst = tinstantset_inst_n(ti, i);
    Datum value = tinstant_value(inst);
    if (DatumGetBool(call_function2(intersects, value, geom)))
      instants[k++] = inst;
  }
  TInstantSet *result = NULL;
  if (k != 0)
    result = tinstantset_make(instants, k);
  /* We do not need to pfree the instants */
  pfree(instants);
  return result;
}

/**
 * Restricts the segment of a temporal sequence point to the geometry 
 *
 * @param[in] inst1,inst2 Instants defining the segment
 * @param[in] linear True when the segment has linear interpolation
 * @param[in] lower_inc,upper_inc State whether the bounds are inclusive
 * @param[in] geom Geometry
 * @param[out] count Number of elements in the resulting array
 * @pre The instants have the same SRID and the points and the geometry 
 * are in 2D
 */
static TSequence **
tpointseq_at_geometry1(const TInstant *inst1, const TInstant *inst2,
  bool linear, bool lower_inc, bool upper_inc, Datum geom, int *count)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  TInstant *instants[2];

  /* Constant segment or step interpolation */
  bool equal = datum_point_eq(value1, value2);
  if (equal || ! linear)
  {
    if (!DatumGetBool(call_function2(intersects, value1, geom)))
    {
      *count = 0;
      return NULL;
    }

    instants[0] = (TInstant *) inst1;
    instants[1] = linear ? (TInstant *) inst2 :
      tinstant_make(value1, inst2->t, inst1->valuetypid);
    /* Stepwise segment with inclusive upper bound must return 2 sequences */
    bool upper_inc1 = (linear) ? upper_inc : false;
    TSequence **result = palloc(sizeof(TSequence *) * 2);
    result[0] = tsequence_make(instants, 2, lower_inc, upper_inc1,
      linear, NORMALIZE_NO);
    int k = 1;
    if (upper_inc != upper_inc1 && 
      DatumGetBool(call_function2(intersects, value2, geom)))
    {
      result[1] = tinstant_to_tsequence(inst2, linear);
      k = 2;
    }
    if (! linear)
      pfree(instants[1]);
    *count = k;
    return result;
  }

  /* Look for intersections in linear segment */
  Datum line = geopoint_line(value1, value2);
  Datum inter = call_function2(intersection, line, geom);
  GSERIALIZED *gsinter = (GSERIALIZED *) PG_DETOAST_DATUM(inter);
  if (gserialized_is_empty(gsinter))
  {
    pfree(DatumGetPointer(line));
    pfree(DatumGetPointer(inter));
    POSTGIS_FREE_IF_COPY_P(gsinter, DatumGetPointer(gsinter));
    *count = 0;
    return NULL;
  }

  const POINT2D *start = datum_get_point2d_p(value1);
  const POINT2D *end = datum_get_point2d_p(value2);
  LWGEOM *lwgeom_inter = lwgeom_from_gserialized(gsinter);
  int type = lwgeom_inter->type;
  int countinter;
  LWPOINT *lwpoint_inter;
  LWLINE *lwline_inter;
  LWCOLLECTION *coll;
  if (type == POINTTYPE)
  {
    countinter = 1;
    lwpoint_inter = lwgeom_as_lwpoint(lwgeom_inter);

  }
  else if (type == LINETYPE)
  {
    countinter = 1;
    lwline_inter = lwgeom_as_lwline(lwgeom_inter);
  }
  else
  {
    coll = lwgeom_as_lwcollection(lwgeom_inter);
    countinter = coll->ngeoms;
  }
  TSequence **result = palloc(sizeof(TSequence *) * countinter);
  double duration = (inst2->t - inst1->t);
  int k = 0;
  for (int i = 0; i < countinter; i++)
  {
    if (countinter > 1)
    {
      /* Find the i-th intersection */
      LWGEOM *subgeom = coll->geoms[i];
      if (subgeom->type == POINTTYPE)
        lwpoint_inter = lwgeom_as_lwpoint(subgeom);
      else /* type == LINETYPE */
        lwline_inter = lwgeom_as_lwline(subgeom);
      type =   subgeom->type;
    }
    POINT2D p1, p2, closest;
    double fraction1;
    TimestampTz t1;
    Datum point1;
    /* Each intersection is either a point or a linestring with two points */
    if (type == POINTTYPE)
    {
      lwpoint_getPoint2d_p(lwpoint_inter, &p1);
      fraction1 = closest_point2d_on_segment_ratio(&p1, start, end, &closest);
      t1 = inst1->t + (long) (duration * fraction1);
      /* If the intersection is not at an exclusive bound */
      if ((lower_inc || t1 > inst1->t) && (upper_inc || t1 < inst2->t))
      {
        point1 = tsequence_value_at_timestamp1(inst1, inst2, true, t1);
        instants[0] = tinstant_make(point1, t1, inst1->valuetypid);
        result[k++] = tinstant_to_tsequence(instants[0], linear);
        pfree(DatumGetPointer(point1));
        pfree(instants[0]);
      }
    }
    else
    {
      LWPOINT *lwpoint1 = lwline_get_lwpoint(lwline_inter, 0);
      LWPOINT *lwpoint2 = lwline_get_lwpoint(lwline_inter, 1);
      lwpoint_getPoint2d_p(lwpoint1, &p1);
      lwpoint_getPoint2d_p(lwpoint2, &p2);
      fraction1 = closest_point2d_on_segment_ratio(&p1, start, end, &closest);
      double fraction2 = closest_point2d_on_segment_ratio(&p2, start, end, &closest);
      t1 = inst1->t + (long) (duration * fraction1);
      TimestampTz t2 = inst1->t + (long) (duration * fraction2);
      TimestampTz lower1 = Min(t1, t2);
      TimestampTz upper1 = Max(t1, t2);
      point1 = tsequence_value_at_timestamp1(inst1, inst2, true, lower1);
      Datum point2 = tsequence_value_at_timestamp1(inst1, inst2, true, upper1);
      instants[0] = tinstant_make(point1, lower1, inst1->valuetypid);
      instants[1] = tinstant_make(point2, upper1, inst1->valuetypid);
      bool lower_inc1 = (lower1 == inst1->t) ? lower_inc : true;
      bool upper_inc1 = (upper1 == inst2->t) ? upper_inc : true;
      result[k++] = tsequence_make(instants, 2, lower_inc1, upper_inc1,
        linear, NORMALIZE_NO);
      pfree(DatumGetPointer(point1)); pfree(DatumGetPointer(point2));
      pfree(instants[0]); pfree(instants[1]);
    }
  }

  pfree(DatumGetPointer(line));
  pfree(DatumGetPointer(inter));
  POSTGIS_FREE_IF_COPY_P(gsinter, DatumGetPointer(gsinter));
  lwgeom_free(lwgeom_inter);

  if (k == 0)
  {
    pfree(result);
    *count = 0;
    return NULL;
  }
  if (k > 1)
    tsequencearr_sort(result, k);
  *count = k;
  return result;
}

/**
 * Restricts the temporal sequence point to the geometry 
 *
 * @param[in] seq Temporal point
 * @param[in] geom Geometry
 * @param[out] count Number of elements in the resulting array
 */
TSequence **
tpointseq_at_geometry2(const TSequence *seq, Datum geom, int *count)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    /* Due to the bounding box test in the calling function we are sure
     * that the point intersects the geometry */
    TSequence **result = palloc(sizeof(TSequence *));
    result[0] = tsequence_copy(seq);
    *count = 1;
    return result;
  }

  /* Temporal sequence has at least 2 instants */
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  TSequence ***sequences = palloc(sizeof(TSequence *) * (seq->count - 1));
  int *countseqs = palloc0(sizeof(int) * (seq->count - 1));
  int totalseqs = 0;
  TInstant *inst1 = tsequence_inst_n(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  for (int i = 0; i < seq->count - 1; i++)
  {
    TInstant *inst2 = tsequence_inst_n(seq, i + 1);
    bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false;
    sequences[i] = tpointseq_at_geometry1(inst1, inst2, linear,
      lower_inc, upper_inc, geom, &countseqs[i]);
    totalseqs += countseqs[i];
    inst1 = inst2;
    lower_inc = true;
  }
  /* Set the output parameter */
  *count = totalseqs;
  if (totalseqs == 0)
  {
    pfree(sequences); pfree(countseqs);
    return NULL;
  }
  TSequence **result = tsequencearr2_to_tsequencearr(sequences,
    countseqs, seq->count - 1, totalseqs);
  return result;
}

/**
 * Restricts the temporal sequence point to the geometry 
 */
static TSequenceSet *
tpointseq_at_geometry(const TSequence *seq, Datum geom)
{
  int count;
  TSequence **sequences = tpointseq_at_geometry2(seq, geom, &count);
  if (sequences == NULL)
    return NULL;

  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Restricts the temporal sequence set point to the geometry 
 *
 * @param[in] ts Temporal point
 * @param[in] geom Geometry
 * @param[in] box Bounding box of the temporal point
 */
static TSequenceSet *
tpointseqset_at_geometry(const TSequenceSet *ts, Datum geom, const STBOX *box)
{
  /* palloc0 used due to the bounding box test in the for loop below */
  TSequence ***sequences = palloc0(sizeof(TSequence *) * ts->count);
  int *countseqs = palloc0(sizeof(int) * ts->count);
  int totalseqs = 0;
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    /* Bounding box test */
    STBOX *box1 = tsequence_bbox_ptr(seq);
    if (overlaps_stbox_stbox_internal(box1, box))
    {
      sequences[i] = tpointseq_at_geometry2(seq, geom,
        &countseqs[i]);
      totalseqs += countseqs[i];
    }
  }
  if (totalseqs == 0)
  {
    pfree(sequences); pfree(countseqs);
    return NULL;
  }
  TSequence **allsequences = tsequencearr2_to_tsequencearr(sequences,
    countseqs, ts->count, totalseqs);
  return tsequenceset_make_free(allsequences, totalseqs, NORMALIZE);
}

/**
 * Restricts the temporal point to the geometry 
 *
 * @pre The arguments are of the same dimensionality, have the same SRID,
 * and the geometry is not empty 
 */
Temporal *
tpoint_at_geometry_internal(const Temporal *temp, Datum geom)
{
  /* Bounding box test */
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  /* Non-empty geometries have a bounding box */
  assert(geo_to_stbox_internal(&box2, (GSERIALIZED *) DatumGetPointer(geom)));
  if (!overlaps_stbox_stbox_internal(&box1, &box2))
    return NULL;

  Temporal *result;
  ensure_valid_duration(temp->duration);
  if (temp->duration == INSTANT)
    result = (Temporal *)tpointinst_at_geometry((TInstant *)temp, geom);
  else if (temp->duration == INSTANTSET)
    result = (Temporal *)tpointinstset_at_geometry((TInstantSet *)temp, geom);
  else if (temp->duration == SEQUENCE)
    result = (Temporal *)tpointseq_at_geometry((TSequence *)temp, geom);
  else /* temp->duration == SEQUENCESET */
    result = (Temporal *)tpointseqset_at_geometry((TSequenceSet *)temp, geom, &box2);

  return result;
}

/*****************************************************************************/

/**
 * Restrict the temporal instant point to the complement of the geometry
 */
static TInstant *
tpointinst_minus_geometry(const TInstant *inst, Datum geom)
{
  if (DatumGetBool(call_function2(intersects, tinstant_value(inst), geom)))
    return NULL;
  return tinstant_copy(inst);
}

/**
 * Restrict the temporal instant set point to the complement of the geometry
 */
static TInstantSet *
tpointinstset_minus_geometry(const TInstantSet *ti, Datum geom)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int k = 0;
  for (int i = 0; i < ti->count; i++)
  {
    TInstant *inst = tinstantset_inst_n(ti, i);
    Datum value = tinstant_value(inst);
    if (!DatumGetBool(call_function2(intersects, value, geom)))
      instants[k++] = inst;
  }
  TInstantSet *result = NULL;
  if (k != 0)
    result = tinstantset_make(instants, k);
  /* We cannot pfree the instants */
  pfree(instants);
  return result;
}

/**
 * Restrict the temporal sequence point to the complement of the geometry
 *
 * It is not possible to use a similar approach as for tpointseq_at_geometry1
 * where instead of computing the intersections we compute the difference since
 * in PostGIS the following query
 * @code
 * select st_astext(st_difference(geometry 'Linestring(0 0,3 3)',
 *     geometry 'MultiPoint((1 1),(2 2),(3 3))'))
 * @endcode
 * returns `LINESTRING(0 0,3 3)`. Therefore we compute tpointseq_at_geometry1
 * and then compute the complement of the value obtained.
 *
 * @param[in] seq Temporal point
 * @param[in] geom Geometry
 * @param[out] count Number of elements in the resulting array
 */
static TSequence **
tpointseq_minus_geometry1(const TSequence *seq, Datum geom, int *count)
{
  int countinter;
  TSequence **sequences = tpointseq_at_geometry2(seq, geom, &countinter);
  if (countinter == 0)
  {
    TSequence **result = palloc(sizeof(TSequence *));
    result[0] = tsequence_copy(seq);
    *count = 1;
    return result;
  }

  Period **periods = palloc(sizeof(Period) * countinter);
  for (int i = 0; i < countinter; i++)
    periods[i] = &sequences[i]->period;
  PeriodSet *ps1 = periodset_make(periods, countinter, NORMALIZE_NO);
  PeriodSet *ps2 = minus_period_periodset_internal(&seq->period, ps1);
  pfree(ps1); pfree(periods);
  if (ps2 == NULL)
  {
    *count = 0;
    return NULL;
  }
  TSequence **result = palloc(sizeof(TSequence *) * ps2->count);
  *count = tsequence_at_periodset(result, seq, ps2);
  pfree(ps2);
  return result;
}

/**
 * Restrict the temporal sequence point to the complement of the geometry
 */
static TSequenceSet *
tpointseq_minus_geometry(const TSequence *seq, Datum geom)
{
  int count;
  TSequence **sequences = tpointseq_minus_geometry1(seq, geom, &count);
  if (sequences == NULL)
    return NULL;

  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Restrict the temporal sequence set point to the complement of the geometry
 */
static TSequenceSet *
tpointseqset_minus_geometry(const TSequenceSet *ts, Datum geom, STBOX *box2)
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tpointseq_minus_geometry(tsequenceset_seq_n(ts, 0),
      geom);

  TSequence ***sequences = palloc(sizeof(TSequence *) * ts->count);
  int *countseqs = palloc0(sizeof(int) * ts->count);
  int totalseqs = 0;
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    /* Bounding box test */
    STBOX *box1 = tsequence_bbox_ptr(seq);
    if (!overlaps_stbox_stbox_internal(box1, box2))
    {
      sequences[i] = palloc(sizeof(TSequence *));
      sequences[i][0] = tsequence_copy(seq);
      countseqs[i] = 1;
      totalseqs ++;
    }
    else
    {
      sequences[i] = tpointseq_minus_geometry1(seq, geom,
        &countseqs[i]);
      totalseqs += countseqs[i];
    }
  }
  if (totalseqs == 0)
  {
    pfree(sequences); pfree(countseqs);
    return NULL;
  }
  TSequence **allsequences = tsequencearr2_to_tsequencearr(sequences,
    countseqs, ts->count, totalseqs);
  return tsequenceset_make_free(allsequences, totalseqs, NORMALIZE);
}

/**
 * Restrict the temporal point to the complement of the geometry
 * (dispatch function)
 *
 * @pre The arguments are of the same dimensionality,
 * have the same SRID, and the geometry is not empty 
 */
Temporal *
tpoint_minus_geometry_internal(const Temporal *temp, Datum geom)
{
  /* Bounding box test */
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  /* Non-empty geometries have a bounding box */
  assert(geo_to_stbox_internal(&box2, (GSERIALIZED *) DatumGetPointer(geom)));
  if (!overlaps_stbox_stbox_internal(&box1, &box2))
    return temporal_copy(temp);

  Temporal *result;
  ensure_valid_duration(temp->duration);
  if (temp->duration == INSTANT)
    result = (Temporal *)tpointinst_minus_geometry((TInstant *)temp, geom);
  else if (temp->duration == INSTANTSET)
    result = (Temporal *)tpointinstset_minus_geometry((TInstantSet *)temp, geom);
  else if (temp->duration == SEQUENCE)
    result = (Temporal *)tpointseq_minus_geometry((TSequence *)temp, geom);
  else /* temp->duration == SEQUENCESET */
    result = (Temporal *)tpointseqset_minus_geometry((TSequenceSet *)temp, geom, &box2);

  return result;
}

/*****************************************************************************/

/**
 * Restricts the temporal point to the (complement of the) geometry 
 */
Datum
tpoint_restrict_geometry(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
  {
    PG_FREE_IF_COPY(gs, 1);
    if (atfunc)
    {
      PG_FREE_IF_COPY(temp, 0);
      PG_RETURN_NULL();
    }
    else
    {
      Temporal *result = temporal_copy(temp);
      PG_FREE_IF_COPY(temp, 0);
      PG_RETURN_POINTER(result);
    }
  }
  ensure_same_srid_tpoint_gs(temp, gs);
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  Temporal *result = atfunc ?
    tpoint_at_geometry_internal(temp, PointerGetDatum(gs)) :
    tpoint_minus_geometry_internal(temp, PointerGetDatum(gs));
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tpoint_at_geometry);
/**
 * Restricts the temporal point to the geometry 
 */
PGDLLEXPORT Datum
tpoint_at_geometry(PG_FUNCTION_ARGS)
{
  return tpoint_restrict_geometry(fcinfo, REST_AT);
}


PG_FUNCTION_INFO_V1(tpoint_minus_geometry);
/**
 * Restrict the temporal point to the complement of the geometry
 */
PGDLLEXPORT Datum
tpoint_minus_geometry(PG_FUNCTION_ARGS)
{
  return tpoint_restrict_geometry(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * Restrict the temporal point to the spatiotemporal box 
 *
 * @pre The arguments are of the same dimensionality and
 * have the same SRID 
 */
Temporal *
tpoint_at_stbox_internal(const Temporal *temp, const STBOX *box)
{
  /* Bounding box test */
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  if (!overlaps_stbox_stbox_internal(box, &box1))
    return NULL;

  /* At least one of MOBDB_FLAGS_GET_T and MOBDB_FLAGS_GET_X is true */
  Temporal *temp1;
  if (MOBDB_FLAGS_GET_T(box->flags))
  {
    Period p;
    period_set(&p, box->tmin, box->tmax, true, true);
    temp1 = temporal_at_period_internal(temp, &p);
  }
  else
    temp1 = temporal_copy(temp);

  Temporal *result;
  if (MOBDB_FLAGS_GET_X(box->flags))
  {
    Datum gbox = PointerGetDatum(stbox_to_gbox(box));
    Datum geom = MOBDB_FLAGS_GET_Z(box->flags) ?
      call_function1(BOX3D_to_LWGEOM, gbox) :
      call_function1(BOX2D_to_LWGEOM, gbox);
    Datum geom1 = call_function2(LWGEOM_set_srid, geom,
      Int32GetDatum(box->srid));
    result = tpoint_at_geometry_internal(temp1, geom1);
    pfree(DatumGetPointer(gbox)); pfree(DatumGetPointer(geom));
    pfree(DatumGetPointer(geom1));
    pfree(temp1);
  }
  else
    result = temp1;
  return result;
}

/**
 * Restrict the temporal point to the complement of the spatiotemporal box.
 * (internal function).
 * We cannot make the difference from each dimension separately, i.e.,
 * restrict at the period and then restrict to the polygon. Therefore, we
 * compute the atStbox and then compute the complement of the value obtained.
 *
 * @pre The arguments are of the same dimensionality and have the same SRID 
 */
Temporal *
tpoint_minus_stbox_internal(const Temporal *temp, const STBOX *box)
{
  /* Bounding box test */
  STBOX box1;
  memset(&box1, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  if (!overlaps_stbox_stbox_internal(box, &box1))
    return temporal_copy(temp);

  Temporal *result = NULL;
  Temporal *temp1 = tpoint_at_stbox_internal(temp, box);
  if (temp1 != NULL)
  {
    PeriodSet *ps1 = temporal_get_time_internal(temp);
    PeriodSet *ps2 = temporal_get_time_internal(temp1);
    PeriodSet *ps = minus_periodset_periodset_internal(ps1, ps2);
    if (ps != NULL)
    {
      result = temporal_restrict_periodset_internal(temp, ps, true);
      pfree(ps);
    }
    pfree(temp1); pfree(ps1); pfree(ps2); 
  }
  return result;
}

/**
 * Restrict the temporal point to (the complement of) the spatiotemporal box
 */
Datum
tpoint_restrict_stbox(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  STBOX *box = PG_GETARG_STBOX_P(1);
  ensure_same_geodetic_tpoint_stbox(temp, box);
  ensure_same_srid_tpoint_stbox(temp, box);
  if (MOBDB_FLAGS_GET_X(box->flags))
    ensure_same_spatial_dimensionality_tpoint_stbox(temp, box);
  Temporal *result = atfunc ? tpoint_at_stbox_internal(temp, box) :
    tpoint_minus_stbox_internal(temp, box);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tpoint_at_stbox);
/**
 * Restricts the temporal value to the spatiotemporal box
 */
PGDLLEXPORT Datum
tpoint_at_stbox(PG_FUNCTION_ARGS)
{
  return tpoint_restrict_stbox(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(tpoint_minus_stbox);
/**
 * Restricts the temporal value to the complement of the spatiotemporal box
 */
PGDLLEXPORT Datum
tpoint_minus_stbox(PG_FUNCTION_ARGS)
{
  return tpoint_restrict_stbox(fcinfo, REST_MINUS);
}

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/
 
/**
 * Returns the temporal distance between the temporal sequence point and
 * the geometry/geography point
 *
 * @param[in] seq Temporal point
 * @param[in] point Point
 * @param[in] func Distance function
 */
static TSequence *
distance_tpointseq_geo(const TSequence *seq, Datum point,
  Datum (*func)(Datum, Datum))
{
  int k = 0;
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count * 2);
  TInstant *inst1 = tsequence_inst_n(seq, 0);
  Datum value1 = tinstant_value(inst1);
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  for (int i = 1; i < seq->count; i++)
  {
    /* Each iteration of the loop adds between one and three points */
    TInstant *inst2 = tsequence_inst_n(seq, i);
    Datum value2 = tinstant_value(inst2);

    /* Constant segment or step interpolation */
    if (datum_point_eq(value1, value2) || ! linear)
    {
      instants[k++] = tinstant_make(func(point, value1),
        inst1->t, FLOAT8OID);
    }
    else
    {
      /* The trajectory is a line */
      long double fraction;
      long double duration = (long double) (inst2->t - inst1->t);
      double dist;
      fraction = (long double) geoseg_locate_point(value1, value2, point, &dist);

      if (fraction == 0.0 || fraction == 1.0)
      {
        instants[k++] = tinstant_make(func(point, value1),
          inst1->t, FLOAT8OID);
      }
      else
      {
        TimestampTz time = inst1->t + (long) (duration * fraction);
        instants[k++] = tinstant_make(func(point, value1),
          inst1->t, FLOAT8OID);
        instants[k++] = tinstant_make(Float8GetDatum(dist),
          time, FLOAT8OID);
      }
    }
    inst1 = inst2; value1 = value2;
  }
  instants[k++] = tinstant_make(func(point, value1), inst1->t, FLOAT8OID);
  
  return tsequence_make_free(instants, k, seq->period.lower_inc,
    seq->period.upper_inc, linear, NORMALIZE);
}

/**
 * Returns the temporal distance between the temporal sequence set point and
 * the geometry/geography point 
 *
 * @param[in] ts Temporal point
 * @param[in] point Point
 * @param[in] func Distance function
 */
static TSequenceSet *
distance_tpointseqset_geo(const TSequenceSet *ts, Datum point, 
  Datum (*func)(Datum, Datum))
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    sequences[i] = distance_tpointseq_geo(seq, point, func);
  }
  return tsequenceset_make_free(sequences, ts->count, NORMALIZE);
}

/**
 * Returns the single timestamp at which the two temporal geometric point
 * segments are at the minimum distance. These are the turning points 
 * when computing the temporal distance.
 * 
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[out] t Timestamp
 * @note The PostGIS functions `lw_dist2d_seg_seg` and `lw_dist3d_seg_seg`
 * cannot be used since they do not take time into consideration and would
 * return, e.g., that the minimum distance between the two following segments
 * `[Point(2 2)@t1, Point(1 1)@t2]` and `[Point(3 1)@t1, Point(1 1)@t2]`
 * is at `Point(2 2)@t2` instead of `Point(1.5 1.5)@(t1 + (t2 - t1)/2)`.
 * @pre The segments are not both constants.
 * @note 
 */
bool
tgeompointseq_min_dist_at_timestamp(const TInstant *start1, 
  const TInstant *end1, const TInstant *start2, 
  const TInstant *end2, TimestampTz *t)
{
  long double denum, fraction;
  long double dx1, dy1, dx2, dy2, f1, f2, f3, f4;
  long double duration = (long double) (end1->t - start1->t);

  if (MOBDB_FLAGS_GET_Z(start1->flags)) /* 3D */
  {
    long double dz1, dz2, f5, f6;
    const POINT3DZ *p1 = datum_get_point3dz_p(tinstant_value(start1));
    const POINT3DZ *p2 = datum_get_point3dz_p(tinstant_value(end1));
    const POINT3DZ *p3 = datum_get_point3dz_p(tinstant_value(start2));
    const POINT3DZ *p4 = datum_get_point3dz_p(tinstant_value(end2));
    /* The following basically computes d/dx (Euclidean distance) = 0->
       To reduce problems related to floating point arithmetic, t1 and t2
       are shifted, respectively, to 0 and 1 before computing d/dx */
    dx1 = p2->x - p1->x;
    dy1 = p2->y - p1->y;
    dz1 = p2->z - p1->z;
    dx2 = p4->x - p3->x;
    dy2 = p4->y - p3->y;
    dz2 = p4->z - p3->z;
    
    f1 = p3->x * (dx1 - dx2);
    f2 = p1->x * (dx2 - dx1);
    f3 = p3->y * (dy1 - dy2);
    f4 = p1->y * (dy2 - dy1);
    f5 = p3->z * (dz1 - dz2);
    f6 = p1->z * (dz2 - dz1);

    denum = dx1*(dx1-2*dx2) + dy1*(dy1-2*dy2) + dz1*(dz1-2*dz2) + 
      dx2*dx2 + dy2*dy2 + dz2*dz2;
    if (denum == 0)
      return false;

    fraction = (f1 + f2 + f3 + f4 + f5 + f6) / denum;
  }
  else /* 2D */
  {
    const POINT2D *p1 = datum_get_point2d_p(tinstant_value(start1));
    const POINT2D *p2 = datum_get_point2d_p(tinstant_value(end1));
    const POINT2D *p3 = datum_get_point2d_p(tinstant_value(start2));
    const POINT2D *p4 = datum_get_point2d_p(tinstant_value(end2));
    /* The following basically computes d/dx (Euclidean distance) = 0.
       To reduce problems related to floating point arithmetic, t1 and t2
       are shifted, respectively, to 0 and 1 before computing d/dx */
    dx1 = p2->x - p1->x;
    dy1 = p2->y - p1->y;
    dx2 = p4->x - p3->x;
    dy2 = p4->y - p3->y;
    
    f1 = p3->x * (dx1 - dx2);
    f2 = p1->x * (dx2 - dx1);
    f3 = p3->y * (dy1 - dy2);
    f4 = p1->y * (dy2 - dy1);

    denum = dx1*(dx1-2*dx2) + dy1*(dy1-2*dy2) + dy2*dy2 + dx2*dx2;
    /* If the segments are parallel */
    if (denum == 0)
      return false;

    fraction = (f1 + f2 + f3 + f4) / denum;
  }
  if (fraction <= EPSILON || fraction >= (1.0 - EPSILON))
    return false;
  *t = start1->t + (long) (duration * fraction);
  return true;
}

/**
 * Returns the single timestamp at which the two temporal geographic point
 * segments are at the minimum distance. These are the turning points 
 * when computing the temporal distance.
 * 
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[out] mindist Minimum distance
 * @param[out] t Timestamp
 * @pre The segments are not both constants.
 */
bool
tgeogpointseq_min_dist_at_timestamp(const TInstant *start1, 
  const TInstant *end1, const TInstant *start2, 
  const TInstant *end2, double *mindist, TimestampTz *t)
{
  const POINT2D *p1 = datum_get_point2d_p(tinstant_value(start1));
  const POINT2D *p2 = datum_get_point2d_p(tinstant_value(end1));
  const POINT2D *p3 = datum_get_point2d_p(tinstant_value(start2));
  const POINT2D *p4 = datum_get_point2d_p(tinstant_value(end2));
  GEOGRAPHIC_EDGE e1, e2;
  GEOGRAPHIC_POINT close1, close2;
  POINT3D A1, A2, B1, B2;
  geographic_point_init(p1->x, p1->y, &(e1.start));
  geographic_point_init(p2->x, p2->y, &(e1.end));
  geographic_point_init(p3->x, p3->y, &(e2.start));
  geographic_point_init(p4->x, p4->y, &(e2.end));
  geog2cart(&(e1.start), &A1);
  geog2cart(&(e1.end), &A2);
  geog2cart(&(e2.start), &B1);
  geog2cart(&(e2.end), &B2);
  double fraction;
  if (edge_intersects(&A1, &A2, &B1, &B2))
  {
    /* We know that the distance is 0 */
    *mindist = 0.0;
    /* In this case we must take the temporality into account */
    long double dx1, dy1, dz1, dx2, dy2, dz2, f1, f2, f3, f4, f5, f6, denum;
    dx1 = A2.x - A1.x;
    dy1 = A2.y - A1.y;
    dz1 = A2.z - A1.z;
    dx2 = B2.x - B1.x;
    dy2 = B2.y - B1.y;
    dz2 = B2.z - B1.z;

    f1 = B1.x * (dx1 - dx2);
    f2 = A1.x * (dx2 - dx1);
    f3 = B1.y * (dy1 - dy2);
    f4 = A1.y * (dy2 - dy1);
    f5 = B1.z * (dz1 - dz2);
    f6 = A1.z * (dz2 - dz1);

    denum = dx1*(dx1-2*dx2) + dy1*(dy1-2*dy2) + dz1*(dz1-2*dz2) +
      dx2*dx2 + dy2*dy2 + dz2*dz2;
    if (denum == 0)
      return false;

    fraction = (double) ((f1 + f2 + f3 + f4 + f5 + f6) / denum);
  }
  else
  {
    /* Compute closest points en each segment */
    edge_distance_to_edge(&e1, &e2, &close1, &close2);
    if (geographic_point_equals(&e1.start, &close1) ||
      geographic_point_equals(&e1.end, &close1))
      return false;
    /* Compute distance fbetween closest points */
    *mindist = WGS84_RADIUS * sphere_distance(&close1, &close2);
    /* Compute distance from beginning of the segment to one closest point */
    long double seglength = sphere_distance(&(e1.start), &(e1.end));
    long double length = sphere_distance(&(e1.start), &close1);
    fraction = (double) (length / seglength);
  }

  if (fraction <= EPSILON || fraction >= (1.0 - EPSILON))
    return false;
  long double duration = (long double) (end1->t - start1->t);
  *t = start1->t + (long) (duration * fraction);
  return true;
}

/**
 * Returns the single timestamp at which the two temporal point segments 
 * are at the minimum distance (dispatch function). 
 * 
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[out] t Timestamp
 * @pre The segments are not both constants.
 */
bool
tpointseq_min_dist_at_timestamp(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, TimestampTz *t)
{
  double d;
  if (MOBDB_FLAGS_GET_GEODETIC(start1->flags))
    return tgeogpointseq_min_dist_at_timestamp(start1, end1, start2, end2, &d, t);
  else
    return tgeompointseq_min_dist_at_timestamp(start1, end1, start2, end2, t);
}

/*****************************************************************************/

/**
 * Returns the temporal distance between the temporal point and the 
 * geometry/geography point (distpatch function)
 */
Temporal *
distance_tpoint_geo_internal(const Temporal *temp, Datum geo)
{
  Datum (*func)(Datum, Datum);
  if (MOBDB_FLAGS_GET_GEODETIC(temp->flags))
    func = &geog_distance;
  else
    func = MOBDB_FLAGS_GET_Z(temp->flags) ?
      &pt_distance3d : &pt_distance2d;
  LiftedFunctionInfo lfinfo;
  ensure_valid_duration(temp->duration);
  if (temp->duration == INSTANT || temp->duration == INSTANTSET)
  {
    lfinfo.func = (varfunc) func;
    lfinfo.numparam = 2;
    lfinfo.restypid = FLOAT8OID;
    lfinfo.reslinear = MOBDB_FLAGS_GET_LINEAR(temp->flags);
    lfinfo.invert = INVERT_NO;
    lfinfo.discont = CONTINUOUS;
    lfinfo.tpfunc = NULL;
  }
  Temporal *result;
  if (temp->duration == INSTANT)
    result = (Temporal *)tfunc_tinstant_base((TInstant *)temp, geo,
      temp->valuetypid, (Datum) NULL, lfinfo);
  else if (temp->duration == INSTANTSET)
    result = (Temporal *)tfunc_tinstantset_base((TInstantSet *)temp, geo,
      temp->valuetypid, (Datum) NULL, lfinfo);
  else if (temp->duration == SEQUENCE)
    result = (Temporal *)distance_tpointseq_geo((TSequence *)temp, geo, func);
  else /* temp->duration == SEQUENCESET */
    result = (Temporal *)distance_tpointseqset_geo((TSequenceSet *)temp, geo, func);
  return result;
}

PG_FUNCTION_INFO_V1(distance_geo_tpoint);
/**
 * Returns the temporal distance between the geometry/geography point
 * and the temporal point
 */
PGDLLEXPORT Datum
distance_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_point_type(gs);
  ensure_same_srid_tpoint_gs(temp, gs);
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = distance_tpoint_geo_internal(temp, PointerGetDatum(gs));
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(distance_tpoint_geo);
/**
 * Returns the temporal distance between the temporal point and the 
 * geometry/geography point
 */
PGDLLEXPORT Datum
distance_tpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_point_type(gs);
  ensure_same_srid_tpoint_gs(temp, gs);
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = distance_tpoint_geo_internal(temp, PointerGetDatum(gs));
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_POINTER(result);
}

/**
 * Returns the temporal distance between the two temporal points
 * (dispatch function)
 */
Temporal *
distance_tpoint_tpoint_internal(const Temporal *temp1, const Temporal *temp2)
{
  LiftedFunctionInfo lfinfo;
  if (MOBDB_FLAGS_GET_GEODETIC(temp1->flags))
    lfinfo.func = (varfunc) &geog_distance;
  else
    lfinfo.func = MOBDB_FLAGS_GET_Z(temp1->flags) ? 
      (varfunc) &pt_distance3d : (varfunc) &pt_distance2d;
  lfinfo.numparam = 2;
  lfinfo.restypid = FLOAT8OID;
  lfinfo.reslinear = MOBDB_FLAGS_GET_LINEAR(temp1->flags) || 
    MOBDB_FLAGS_GET_LINEAR(temp2->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc = lfinfo.reslinear ? &tpointseq_min_dist_at_timestamp : NULL;
  Temporal *result = sync_tfunc_temporal_temporal(temp1, temp2, (Datum) NULL,
    lfinfo);
  return result;
}

PG_FUNCTION_INFO_V1(distance_tpoint_tpoint);
/**
 * Returns the temporal distance between the two temporal points
 */
PGDLLEXPORT Datum
distance_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tpoint(temp1, temp2);
  ensure_same_dimensionality_tpoint(temp1, temp2);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = distance_tpoint_tpoint_internal(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach instant
 *****************************************************************************/

/**
 * Returns the nearest approach instant between the temporal instant set point
 * and the geometry/geography
 *
 * @param[in] ti Temporal point
 * @param[in] geo Geometry
 * @param[in] func Distance function
 */
static TInstant *
NAI_tpointinstset_geo(const TInstantSet *ti, Datum geo, Datum (*func)(Datum, Datum))
{
  double mindist = DBL_MAX;
  int number = 0; /* keep compiler quiet */
  for (int i = 0; i < ti->count; i++)
  {
    TInstant *inst = tinstantset_inst_n(ti, i);
    Datum value = tinstant_value(inst);
    double dist = DatumGetFloat8(func(value, geo));
    if (dist < mindist)
    {
      mindist = dist;
      number = i;
    }
  }
  return tinstant_copy(tinstantset_inst_n(ti, number));
}

/*****************************************************************************/

/* NAI between temporal sequence point with step interpolation and a
 * geometry/geography */

/**
 * Returns the new current nearest approach instant between the temporal 
 * sequence point with stepwise interpolation and the geometry/geography
 *
 * @param[in] seq Temporal point
 * @param[in] geo Geometry
 * @param[in] mindist Current minimum distance, it is set at DBL_MAX at the
 * begining but contains the minimum distance found in the previous
 * sequences of a temporal sequence set
 * @param[in] func Distance function
 * @param[out] mininst Instant with the minimum distance
 */
static double
NAI_tpointseq_step_geo1(const TSequence *seq, Datum geo, double mindist,
  Datum (*func)(Datum, Datum), TInstant **mininst)
{
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = tsequence_inst_n(seq, i);
    double dist = DatumGetFloat8(func(tinstant_value(inst), geo));
    if (dist < mindist)
    {
      mindist = dist;
      *mininst = inst;
    }
  }
  return mindist;
}

/**
 * Returns the nearest approach instant between the temporal sequence
 * point with stepwise interpolation and the geometry/geography
 *
 * @param[in] seq Temporal point
 * @param[in] geo Geometry
 * @param[in] func Distance function
 */
static TInstant *
NAI_tpointseq_step_geo(const TSequence *seq, Datum geo,
  Datum (*func)(Datum, Datum))
{
  TInstant *inst;
  NAI_tpointseq_step_geo1(seq, geo, DBL_MAX, func, &inst);
  return tinstant_copy(inst);
}

/**
 * Returns the nearest approach instant between the temporal sequence set 
 * point with stepwise interpolation and the geometry/geography
 *
 * @param[in] ts Temporal point
 * @param[in] geo Geometry
 * @param[in] func Distance function
 */
static TInstant *
NAI_tpointseqset_step_geo(const TSequenceSet *ts, Datum geo, 
  Datum (*func)(Datum, Datum))
{
  TInstant *inst;
  double mindist = DBL_MAX;
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    mindist = NAI_tpointseq_step_geo1(seq, geo, mindist, func, &inst);
  }
  assert(inst != NULL);
  return tinstant_copy(inst);
}

/*****************************************************************************/

/**
 * Returns the nearest approach instant between the segment of a temporal 
 * sequence point with linear interpolation and the geometry
 *
 * @param[in] inst1,inst2 Temporal segment
 * @param[in] lwgeom Geometry
 * @param[out] closest Closest point
 * @param[out] t Timestamp
 * @param[out] tofree True when the resulting instant should be freed
 */
static double
NAI_tpointseq_linear_geo1(const TInstant *inst1, const TInstant *inst2,
  LWGEOM *lwgeom, Datum *closest, TimestampTz *t, bool *tofree)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  *tofree = false;
  double dist, fraction;

  /* Constant segment */
  if (datum_point_eq(value1, value2))
  {
    *closest = value1;
    *t = inst1->t;
    return 0.0;
  }

  /* The trajectory is a line */
  LWLINE *lwline = geopoint_lwline(value1, value2);
  if (MOBDB_FLAGS_GET_GEODETIC(inst1->flags))
    dist = lw_dist_sphere_point_dist((LWGEOM *) lwline, lwgeom, DIST_MIN, &fraction);
  else
    dist = MOBDB_FLAGS_GET_Z(inst1->flags) ?
      lw_dist3d_point_dist((LWGEOM *) lwline, lwgeom, DIST_MIN, &fraction) :
      lw_dist2d_point_dist((LWGEOM *) lwline, lwgeom, DIST_MIN, &fraction);
  lwline_free(lwline);

  if (fabs(fraction) < EPSILON)
  {
    *closest = value1;
    *t = inst1->t;
    return 0.0;
  }
  if (fabs(fraction - 1.0) < EPSILON)
  {
    *closest = value2;
    *t = inst2->t;
    return 0.0;
  }

  double duration = (inst2->t - inst1->t);
  *t = inst1->t + (long)(duration * fraction);
  *tofree = true;
  /* We are sure that it is linear interpolation */
  *closest =  tsequence_value_at_timestamp1(inst1, inst2, true, *t);
  return dist;
}

/**
 * Returns the nearest approach instant between the temporal sequence 
 * point with linear interpolation and the geometry
 *
 * @param[in] seq Temporal point
 * @param[in] geo Geometry
 * @param[in] mindist Minimum distance found so far, or DBL_MAX at the beginning
 * @param[in] func Distance function
 * @param[out] closest Closest point
 * @param[out] t Timestamp
 * @param[out] tofree True when the resulting instant should be freed
 */
static double
NAI_tpointseq_linear_geo2(const TSequence *seq, Datum geo, double mindist,
  Datum (*func)(Datum, Datum), Datum *closest, TimestampTz *t, bool *tofree)
{
  TInstant *inst1;
  double dist;
  Datum point;
  TimestampTz t1;
  bool tofree1;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = tsequence_inst_n(seq, 0);
    point = tinstant_value(inst1);
    dist =  DatumGetFloat8(func(point, geo));
    if (dist < mindist)
    {
      mindist = dist;
      *closest = point;
      *t = inst1->t;
      *tofree = false;
    }
    return mindist;
  }

  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geo);
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  inst1 = tsequence_inst_n(seq, 0);
  *tofree = false;
  for (int i = 0; i < seq->count - 1; i++)
  {
    TInstant *inst2 = tsequence_inst_n(seq, i + 1);
    dist = NAI_tpointseq_linear_geo1(inst1, inst2, lwgeom, &point, &t1, &tofree1);
    if (dist < mindist)
    {
      if (*tofree)
        pfree(DatumGetPointer(*closest));
      mindist = dist;
      *closest = point;
      *t = t1;
      *tofree = tofree1;
    }
    if (mindist == 0.0)
      break;
    inst1 = inst2;
  }
  return mindist;
}

/**
 * Returns the nearest approach instant between the temporal sequence 
 * point with linear interpolation and the geometry
 */
static TInstant *
NAI_tpointseq_linear_geo(const TSequence *seq, Datum geo,
  Datum (*func)(Datum, Datum))
{
  Datum closest;
  TimestampTz t;
  bool tofree;
  NAI_tpointseq_linear_geo2(seq, geo, DBL_MAX, func, &closest, &t, &tofree);
  TInstant *result = tinstant_make(closest, t, seq->valuetypid);
  if (tofree)
    pfree(DatumGetPointer(closest));
  return result;
}

/**
 * Returns the nearest approach instant between the temporal sequence set 
 * point with linear interpolation and the geometry
 */
static TInstant *
NAI_tpointseqset_linear_geo(const TSequenceSet *ts, Datum geo,
  Datum (*func)(Datum, Datum))
{
  Datum closest, point;
  TimestampTz t, t1;
  bool tofree = false, tofree1;
  double mindist = DBL_MAX;
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    double dist = NAI_tpointseq_linear_geo2(seq, geo, mindist, func,
      &point, &t1, &tofree1);
    if (dist < mindist)
    {
      if (tofree)
        pfree(DatumGetPointer(closest));
      mindist = dist;
      closest = point;
      t = t1;
      tofree = tofree1;
    }
    if (mindist == 0.0)
      break;
  }
  TInstant *result = tinstant_make(closest, t, ts->valuetypid);
  if (tofree)
    pfree(DatumGetPointer(closest));
  return result;
}

/*****************************************************************************/

/**
 * Returns the nearest approach instant between the temporal point and
 * the geometry (dispatch function)
 */
TInstant *
NAI_tpoint_geo_internal(FunctionCallInfo fcinfo, const Temporal *temp, 
  GSERIALIZED *gs)
{
  ensure_same_srid_tpoint_gs(temp, gs);
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Datum (*func)(Datum, Datum);
  if (MOBDB_FLAGS_GET_GEODETIC(temp->flags))
    func = &geog_distance;
  else
    func = &geom_distance2d;
  TInstant *result;
  ensure_valid_duration(temp->duration);
  if (temp->duration == INSTANT)
    result = tinstant_copy((TInstant *)temp);
  else if (temp->duration == INSTANTSET)
    result = NAI_tpointinstset_geo((TInstantSet *)temp, PointerGetDatum(gs), func);
  else if (temp->duration == SEQUENCE)
    result = MOBDB_FLAGS_GET_LINEAR(temp->flags) ?
      NAI_tpointseq_linear_geo((TSequence *)temp, PointerGetDatum(gs), func) :
      NAI_tpointseq_step_geo((TSequence *)temp, PointerGetDatum(gs), func);
  else /* temp->duration == SEQUENCESET */
    result = MOBDB_FLAGS_GET_LINEAR(temp->flags) ?
      NAI_tpointseqset_linear_geo((TSequenceSet *)temp, PointerGetDatum(gs), func) :
      NAI_tpointseqset_step_geo((TSequenceSet *)temp, PointerGetDatum(gs), func);
  return result;
}

PG_FUNCTION_INFO_V1(NAI_geo_tpoint);
/**
 * Returns the nearest approach instant between the geometry and
 * the temporal point
 */
PGDLLEXPORT Datum
NAI_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  TInstant *result = NAI_tpoint_geo_internal(fcinfo, temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tpoint_geo);
/**
 * Returns the nearest approach instant between the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
NAI_tpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  TInstant *result = NAI_tpoint_geo_internal(fcinfo, temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tpoint_tpoint);
/**
 * Returns the nearest approach instant between the temporal points
 */
PGDLLEXPORT Datum
NAI_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tpoint(temp1, temp2);
  ensure_same_dimensionality_tpoint(temp1, temp2);
  TInstant *result = NULL;
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *dist = distance_tpoint_tpoint_internal(temp1, temp2);
  if (dist != NULL)
  {
    TInstant *min = temporal_min_instant(dist);
    result = (TInstant *) temporal_restrict_timestamp_internal(temp1, 
      min->t, REST_AT);
    pfree(dist);
    if (result == NULL)
    {
      if (temp1->duration == SEQUENCE)
        result = tsequence_find_timestamp_excl((TSequence *)temp1,
          min->t);
      else /* temp->duration == SEQUENCESET */
        result = tsequenceset_find_timestamp_excl((TSequenceSet *)temp1,
          min->t);
    }
  }
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach distance
 *****************************************************************************/

/**
 * Returns the nearest approach distance between the temporal point and the 
 * geometry (internal function)
 */
Datum
NAD_tpoint_geo_internal(FunctionCallInfo fcinfo, Temporal *temp,
  GSERIALIZED *gs)
{
  ensure_same_srid_tpoint_gs(temp, gs);
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Datum (*func)(Datum, Datum);
  if (MOBDB_FLAGS_GET_GEODETIC(temp->flags))
    func = &geog_distance;
  else
    func = MOBDB_FLAGS_GET_Z(temp->flags) ? &geom_distance3d :
      &geom_distance2d;
  Datum traj = tpoint_trajectory_internal(temp);
  Datum result = func(traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  return result;
}

PG_FUNCTION_INFO_V1(NAD_geo_tpoint);
/**
 * Returns the nearest approach distance between the geometry and
 * the temporal point
 */
PGDLLEXPORT Datum
NAD_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Datum result = NAD_tpoint_geo_internal(fcinfo, temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(NAD_tpoint_geo);
/**
 * Returns the nearest approach distance between the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
NAD_tpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum result = NAD_tpoint_geo_internal(fcinfo, temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(NAD_tpoint_tpoint);
/**
 * Returns the nearest approach distance between the temporal points
 */
PGDLLEXPORT Datum
NAD_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tpoint(temp1, temp2);
  ensure_same_dimensionality_tpoint(temp1, temp2);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *dist = distance_tpoint_tpoint_internal(temp1, temp2);
  if (dist == NULL)
  {
    PG_FREE_IF_COPY(temp1, 0);
    PG_FREE_IF_COPY(temp2, 1);
    PG_RETURN_NULL();
  }

  Datum result = temporal_min_value_internal(dist);
  pfree(dist);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

/**
 * Returns the line connecting the nearest approach point between the
 * temporal instant point and the geometry (internal function)
 */
Datum
shortestline_tpoint_geo_internal(Temporal *temp, GSERIALIZED *gs)
{
  ensure_same_srid_tpoint_gs(temp, gs);
  bool geodetic = MOBDB_FLAGS_GET_GEODETIC(temp->flags);
  if (geodetic)
    ensure_has_not_Z_gs(gs);
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  Datum traj = tpoint_trajectory_internal(temp);
  Datum result;
  if (geodetic)
    result = call_function2(geography_shortestline, traj, PointerGetDatum(gs));
  else
    result = MOBDB_FLAGS_GET_Z(temp->flags) ? 
      call_function2(LWGEOM_shortestline3d, traj, PointerGetDatum(gs)) :
      call_function2(LWGEOM_shortestline2d, traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  return result;
}

PG_FUNCTION_INFO_V1(shortestline_geo_tpoint);
/**
 * Returns the line connecting the nearest approach point between the
 * geometry and the temporal instant point
 */
PGDLLEXPORT Datum
shortestline_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Datum result = shortestline_tpoint_geo_internal(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(shortestline_tpoint_geo);
/**
 * Returns the line connecting the nearest approach point between the
 * temporal instant point and the geometry/geography
 */
PGDLLEXPORT Datum
shortestline_tpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum result = shortestline_tpoint_geo_internal(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_DATUM(result);
}

/**
 * Returns the line connecting the nearest approach point between the
 * temporal points
 */
bool
shortestline_tpoint_tpoint_internal(const Temporal *temp1, 
  const Temporal *temp2, Datum *line)
{
  Temporal *dist = distance_tpoint_tpoint_internal(temp1, temp2);
  if (dist == NULL)
    return false;
  TInstant *inst = temporal_min_instant(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value1, value2;
  bool found1 = temporal_value_at_timestamp_inc(temp1, inst->t, &value1);
  bool found2 = temporal_value_at_timestamp_inc(temp2, inst->t, &value2);
  assert (found1 && found2);
  *line = geopoint_line(value1, value2);
  return true;
}

PG_FUNCTION_INFO_V1(shortestline_tpoint_tpoint);
/**
 * Returns the line connecting the nearest approach point between the
 * temporal points
 */
PGDLLEXPORT Datum
shortestline_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  ensure_same_srid_tpoint(temp1, temp2);
  ensure_same_dimensionality_tpoint(temp1, temp2);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Datum result;
  bool found = shortestline_tpoint_tpoint_internal(temp1, temp2, &result);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (!found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Convert a temporal point into a PostGIS trajectory geometry/geography 
 * The M coordinates encode the timestamps in number of seconds since '1970-01-01'
 *****************************************************************************/

/**
 * Converts the point and the timestamp into a PostGIS geometry/geography 
 * point where the M coordinate encodes the timestamp in number of seconds
 * since '1970-01-01' 
 */
static LWPOINT *
point_to_trajpoint(Datum point, TimestampTz t)
{
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(point);
  int32 srid = gserialized_get_srid(gs);
  /* The internal representation of timestamps in PostgreSQL is in 
   * microseconds since '2000-01-01'. Therefore we need to compute
   * select date_part('epoch', timestamp '2000-01-01' - timestamp '1970-01-01')
   * which results in 946684800 */
  double epoch = ((double) t / 1e6) + 946684800;
  LWPOINT *result;
  if (FLAGS_GET_Z(gs->flags))
  {
    const POINT3DZ *point = gs_get_point3dz_p(gs);
    result = lwpoint_make4d(srid, point->x, point->y, point->z, epoch);
  }
  else
  {
    const POINT2D *point = gs_get_point2d_p(gs);
    result = lwpoint_make3dm(srid, point->x, point->y, epoch);
  }
  FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(gs->flags));
  return result;
}

/**
 * Converts the temporal instant point into a PostGIS trajectory 
 * geometry/geography where the M coordinates encode the timestamps in 
 * number of seconds since '1970-01-01' 
 */
static Datum
tpointinst_to_geo(const TInstant *inst)
{
  LWPOINT *point = point_to_trajpoint(tinstant_value(inst), inst->t);
  GSERIALIZED *result = geo_serialize((LWGEOM *)point);
  pfree(point);
  return PointerGetDatum(result);
}

/**
 * Converts the temporal instant set point into a PostGIS trajectory 
 * geometry/geography where the M coordinates encode the timestamps in 
 * number of seconds since '1970-01-01' 
 */
static Datum
tpointinstset_to_geo(const TInstantSet *ti)
{
  TInstant *inst = tinstantset_inst_n(ti, 0);
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(tinstant_value_ptr(inst));
  int32 srid = gserialized_get_srid(gs);
  LWGEOM **points = palloc(sizeof(LWGEOM *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    inst = tinstantset_inst_n(ti, i);
    points[i] = (LWGEOM *)point_to_trajpoint(tinstant_value(inst), inst->t);
  }
  GSERIALIZED *result;
  if (ti->count == 1)
    result = geo_serialize(points[0]);
  else
  {
    LWGEOM *mpoint = (LWGEOM *)lwcollection_construct(MULTIPOINTTYPE, srid,
      NULL, (uint32_t) ti->count, points);
    result = geo_serialize(mpoint);
    pfree(mpoint);
  }

  for (int i = 0; i < ti->count; i++)
    pfree(points[i]);
  pfree(points);
  return PointerGetDatum(result);
}

/**
 * Converts the temporal sequence point into a PostGIS trajectory 
 * geometry/geography where the M coordinates encode the timestamps in 
 * number of seconds since '1970-01-01' 
 */
static LWGEOM *
tpointseq_to_geo1(const TSequence *seq)
{
  LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = tsequence_inst_n(seq, i);
    points[i] = (LWGEOM *) point_to_trajpoint(tinstant_value(inst), inst->t);
  }
  LWGEOM *result;
  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    result = points[0];
    pfree(points);
  }
  else
  {
    if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
      result = (LWGEOM *) lwline_from_lwgeom_array(points[0]->srid,
        (uint32_t) seq->count, points);
    else
      result = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
        points[0]->srid, NULL, (uint32_t) seq->count, points);
    for (int i = 0; i < seq->count; i++)
      lwpoint_free((LWPOINT *) points[i]);
    pfree(points);
  }
  return result;
}

/**
 * Converts the temporal sequence point into a PostGIS trajectory 
 * geometry/geography where the M coordinates encode the timestamps in 
 * number of seconds since '1970-01-01' 
 */
static Datum
tpointseq_to_geo(const TSequence *seq)
{
  LWGEOM *lwgeom = tpointseq_to_geo1(seq);
  GSERIALIZED *result = geo_serialize(lwgeom);
  return PointerGetDatum(result);
}

/**
 * Converts the temporal sequence set point into a PostGIS trajectory 
 * geometry/geography where the M coordinates encode the timestamps in 
 * number of seconds since '1970-01-01' 
 */
static Datum
tpointseqset_to_geo(const TSequenceSet *ts)
{
  /* Instantaneous sequence */
  if (ts->count == 1)
  {
    TSequence *seq = tsequenceset_seq_n(ts, 0);
    return tpointseq_to_geo(seq);
  }
  uint32_t colltype = 0;
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    geoms[i] = tpointseq_to_geo1(seq);
    /* Output type not initialized */
    if (! colltype)
      colltype = lwtype_get_collectiontype(geoms[i]->type);
      /* Input type not compatible with output */
      /* make output type a collection */
    else if (colltype != COLLECTIONTYPE &&
      lwtype_get_collectiontype(geoms[i]->type) != colltype)
      colltype = COLLECTIONTYPE;
  }
  // TODO add the bounding box instead of ask PostGIS to compute it again
  // GBOX *box = stbox_to_gbox(tsequence_bbox_ptr(seq));
  LWGEOM *coll = (LWGEOM *) lwcollection_construct((uint8_t) colltype,
    geoms[0]->srid, NULL, (uint32_t) ts->count, geoms);
  Datum result = PointerGetDatum(geo_serialize(coll));
  /* We cannot lwgeom_free(geoms[i] or lwgeom_free(coll) */
  pfree(geoms);
  return result;
}

/*****************************************************************************/

/**
 * Converts the temporal sequence point into a PostGIS trajectory 
 * geometry/geography where the M coordinates encode the timestamps in 
 * number of seconds since '1970-01-01' 
 *
 * Version when the resulting value is a MultiLinestring M, where each
 * component is a segment of two points.
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq Temporal point
 */
static int
tpointseq_to_geo_segmentize1(LWGEOM **result, const TSequence *seq)
{
  TInstant *inst = tsequence_inst_n(seq, 0);
  LWPOINT *points[2];

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    result[0] = (LWGEOM *) point_to_trajpoint(tinstant_value(inst), inst->t);
    return 1;
  }

  /* General case */
  for (int i = 0; i < seq->count - 1; i++)
  {
    points[0] = point_to_trajpoint(tinstant_value(inst), inst->t);
    inst = tsequence_inst_n(seq, i + 1);
    points[1] = point_to_trajpoint(tinstant_value(inst), inst->t);
    result[i] = (LWGEOM *) lwline_from_lwgeom_array(points[0]->srid, 2, 
      (LWGEOM **) points);
    lwpoint_free(points[0]); lwpoint_free(points[1]);
  }
  return seq->count - 1;
}

/**
 * Converts the temporal sequence point into a PostGIS trajectory 
 * geometry/geography where the M coordinates encode the timestamps in 
 * number of seconds since '1970-01-01' 
 *
 * Version when the resulting value is a MultiLinestring M, where each
 * component is a segment of two points.
 */
static Datum
tpointseq_to_geo_segmentize(const TSequence *seq)
{
  int count = (seq->count == 1) ? 1 : seq->count - 1;
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * count);
  tpointseq_to_geo_segmentize1(geoms, seq);
  Datum result;
  /* Instantaneous sequence */
  if (seq->count == 1)
    result = PointerGetDatum(geo_serialize(geoms[0]));
  else
  {
    // TODO add the bounding box instead of ask PostGIS to compute it again
    // GBOX *box = stbox_to_gbox(tsequence_bbox_ptr(seq));
    LWGEOM *segcoll = (LWGEOM *) lwcollection_construct(MULTILINETYPE,
      geoms[0]->srid, NULL, (uint32_t)(seq->count - 1), geoms);
    result = PointerGetDatum(geo_serialize(segcoll));
  }
  for (int i = 0; i < count; i++)
    lwgeom_free(geoms[i]);
  pfree(geoms);
  return result;
}

/**
 * Converts the temporal sequence set point into a PostGIS trajectory 
 * geometry/geography where the M coordinates encode the timestamps in 
 * number of seconds since '1970-01-01' 
 *
 * Version when the resulting value is a MultiLinestring M, where each
 * component is a segment of two points.
 */
static Datum
tpointseqset_to_geo_segmentize(const TSequenceSet *ts)
{
  /* Instantaneous sequence */
  if (ts->count == 1)
  {
    TSequence *seq = tsequenceset_seq_n(ts, 0);
    return tpointseq_to_geo_segmentize(seq);
  }

  uint8_t colltype = 0;
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ts->totalcount);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tpointseq_to_geo_segmentize1(&geoms[k], seq);
    /* Output type not initialized */
    if (! colltype)
      colltype = (uint8_t) lwtype_get_collectiontype(geoms[k - 1]->type);
      /* Input type not compatible with output */
      /* make output type a collection */
    else if (colltype != COLLECTIONTYPE &&
      lwtype_get_collectiontype(geoms[k - 1]->type) != colltype)
      colltype = COLLECTIONTYPE;
  }
  Datum result;
  // TODO add the bounding box instead of ask PostGIS to compute it again
  // GBOX *box = stbox_to_gbox(tsequenceset_bbox_ptr(seq));
  LWGEOM *coll = (LWGEOM *) lwcollection_construct(colltype,
    geoms[0]->srid, NULL, (uint32_t) k, geoms);
  result = PointerGetDatum(geo_serialize(coll));
  for (int i = 0; i < k; i++)
    lwgeom_free(geoms[i]);
  pfree(geoms);
  return result;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_to_geo);
/**
 * Converts the temporal point into a PostGIS trajectory geometry/geography 
 * where the M coordinates encode the timestamps in number of seconds since 
 * '1970-01-01' 
 */
PGDLLEXPORT Datum
tpoint_to_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  bool segmentize = (PG_NARGS() == 2) ? PG_GETARG_BOOL(1) : false;
  Datum result;
  ensure_valid_duration(temp->duration);
  if (temp->duration == INSTANT)
    result = tpointinst_to_geo((TInstant *)temp);
  else if (temp->duration == INSTANTSET)
    result = tpointinstset_to_geo((TInstantSet *)temp);
  else if (temp->duration == SEQUENCE)
    result = segmentize ?
         tpointseq_to_geo_segmentize((TSequence *) temp) :
         tpointseq_to_geo((TSequence *) temp);
  else /* temp->duration == SEQUENCESET */
    result = segmentize ?
         tpointseqset_to_geo_segmentize((TSequenceSet *) temp) :
         tpointseqset_to_geo((TSequenceSet *) temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Convert trajectory geometry/geography where the M coordinates encode the
 * timestamps in number of seconds since '1970-01-01' into a temporal point.
 *****************************************************************************/

/**
 * Converts the PostGIS trajectory geometry/geography where the M coordinates 
 * encode the timestamps in number of seconds since '1970-01-01' into a 
 * temporal instant point.
 */
static TInstant *
trajpoint_to_tpointinst(LWPOINT *lwpoint)
{
  bool hasz = (bool) FLAGS_GET_Z(lwpoint->flags);
  bool geodetic = (bool) FLAGS_GET_GEODETIC(lwpoint->flags);
  LWPOINT *lwpoint1;
  TimestampTz t;
  if (hasz)
  {
    POINT4D point = getPoint4d(lwpoint->point, 0);
    t = (long) ((point.m - 946684800) * 1e6);
    lwpoint1 = lwpoint_make3dz(lwpoint->srid, point.x, point.y, point.z);
  }
  else
  {
    POINT3DM point = getPoint3dm(lwpoint->point, 0);
    t = (long) ((point.m - 946684800) * 1e6);
    lwpoint1 = lwpoint_make2d(lwpoint->srid, point.x, point.y);
  }
  FLAGS_SET_GEODETIC(lwpoint1->flags, geodetic);
  GSERIALIZED *gs = geo_serialize((LWGEOM *)lwpoint1);
  Oid valuetypid = geodetic ? type_oid(T_GEOGRAPHY) : type_oid(T_GEOMETRY);
  TInstant *result = tinstant_make(PointerGetDatum(gs), t,
    valuetypid);
  pfree(gs);
  return result;
}

/**
 * Converts the PostGIS trajectory geometry/geography where the M coordinates 
 * encode the timestamps in number of seconds since '1970-01-01' into a 
 * temporal instant point.
 */
static TInstant *
geo_to_tpointinst(GSERIALIZED *gs)
{
  /* Geometry is a POINT */
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  TInstant *result = trajpoint_to_tpointinst((LWPOINT *)lwgeom);
  lwgeom_free(lwgeom);
  return result;
}

/**
 * Converts the PostGIS trajectory geometry/geography where the M coordinates 
 * encode the timestamps in number of seconds since '1970-01-01' into a 
 * temporal instant set point.
 */
static TInstantSet *
geo_to_tpointinstset(GSERIALIZED *gs)
{
  /* Geometry is a MULTIPOINT */
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  bool hasz = (bool) FLAGS_GET_Z(gs->flags);
  /* Verify that is a valid set of trajectory points */
  LWCOLLECTION *lwcoll = lwgeom_as_lwcollection(lwgeom);
  double m1 = -1 * DBL_MAX, m2;
  int npoints = lwcoll->ngeoms;
  for (int i = 0; i < npoints; i++)
  {
    LWPOINT *lwpoint = (LWPOINT *)lwcoll->geoms[i];
    if (hasz)
    {
      POINT4D point = getPoint4d(lwpoint->point, 0);
      m2 = point.m;
    }
    else
    {
      POINT3DM point = getPoint3dm(lwpoint->point, 0);
      m2 = point.m;
    }
    if (m1 >= m2)
    {
      lwgeom_free(lwgeom);
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Trajectory must be valid")));
    }
    m1 = m2;
  }
  TInstant **instants = palloc(sizeof(TInstant *) * npoints);
  for (int i = 0; i < npoints; i++)
    instants[i] = trajpoint_to_tpointinst((LWPOINT *)lwcoll->geoms[i]);
  lwgeom_free(lwgeom);

  return tinstantset_make_free(instants, npoints);
}

/**
 * Converts the PostGIS trajectory geometry/geography where the M coordinates 
 * encode the timestamps in number of seconds since '1970-01-01' into a 
 * temporal sequence point.
 */
static TSequence *
geo_to_tpointseq(GSERIALIZED *gs)
{
  /* Geometry is a LINESTRING */
  bool hasz =(bool)  FLAGS_GET_Z(gs->flags);
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  LWLINE *lwline = lwgeom_as_lwline(lwgeom);
  int npoints = lwline->points->npoints;
  /*
   * Verify that the trajectory is valid.
   * Since calling lwgeom_is_trajectory causes discrepancies with regression
   * tests because of the error message depends on PostGIS version,
   * the verification is made here.
   */
  double m1 = -1 * DBL_MAX, m2;
  for (int i = 0; i < npoints; i++)
  {
    if (hasz)
    {
      POINT4D point = getPoint4d(lwline->points, (uint32_t) i);
      m2 = point.m;
    }
    else
    {
      POINT3DM point = getPoint3dm(lwline->points, (uint32_t) i);
      m2 = point.m;
    }
    if (m1 >= m2)
    {
      lwgeom_free(lwgeom);
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Trajectory must be valid")));
    }
    m1 = m2;
  }
  TInstant **instants = palloc(sizeof(TInstant *) * npoints);
  for (int i = 0; i < npoints; i++)
  {
    /* Returns freshly allocated LWPOINT */
    LWPOINT *lwpoint = lwline_get_lwpoint(lwline, (uint32_t) i);
    instants[i] = trajpoint_to_tpointinst(lwpoint);
    lwpoint_free(lwpoint);
  }
  lwgeom_free(lwgeom);
  /* The resulting sequence assumes linear interpolation */
  return tsequence_make_free(instants, npoints, true, true,
    LINEAR, NORMALIZE);
}

/**
 * Converts the PostGIS trajectory geometry/geography where the M coordinates 
 * encode the timestamps in number of seconds since '1970-01-01' into a 
 * temporal sequence set point.
 */
static TSequenceSet *
geo_to_tpointseqset(GSERIALIZED *gs)
{
  /* Geometry is a MULTILINESTRING or a COLLECTION */
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  LWCOLLECTION *lwcoll = lwgeom_as_lwcollection(lwgeom);
  int ngeoms = lwcoll->ngeoms;
  for (int i = 0; i < ngeoms; i++)
  {
    LWGEOM *lwgeom1 = lwcoll->geoms[i];
    if (lwgeom1->type != POINTTYPE && lwgeom1->type != LINETYPE)
    {
      lwgeom_free(lwgeom);
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Component geometry/geography must be of type Point(Z)M or Linestring(Z)M")));
    }
  }

  TSequence **sequences = palloc(sizeof(TSequence *) * ngeoms);
  for (int i = 0; i < ngeoms; i++)
  {
    LWGEOM *lwgeom1 = lwcoll->geoms[i];
    GSERIALIZED *gs1 = geo_serialize(lwgeom1);
    if (lwgeom1->type == POINTTYPE)
    {
      TInstant *inst = geo_to_tpointinst(gs1);
      /* The resulting sequence assumes linear interpolation */
      sequences[i] = tinstant_to_tsequence(inst, LINEAR);
      pfree(inst);
    }
    else /* lwgeom1->type == LINETYPE */
      sequences[i] = geo_to_tpointseq(gs1);
    pfree(gs1);
  }
  lwgeom_free(lwgeom);
  /* The resulting sequence set assumes linear interpolation */
  return tsequenceset_make_free(sequences, ngeoms, NORMALIZE_NO);
}

PG_FUNCTION_INFO_V1(geo_to_tpoint);
/**
 * Converts the PostGIS trajectory geometry/geography where the M coordinates 
 * encode the timestamps in number of seconds since '1970-01-01' into a 
 * temporal point.
 */
PGDLLEXPORT Datum
geo_to_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  ensure_non_empty(gs);
  ensure_has_M_gs(gs);

  Temporal *result = NULL; /* Make compiler quiet */
  if (gserialized_get_type(gs) == POINTTYPE)
    result = (Temporal *)geo_to_tpointinst(gs);
  else if (gserialized_get_type(gs) == MULTIPOINTTYPE)
    result = (Temporal *)geo_to_tpointinstset(gs);
  else if (gserialized_get_type(gs) == LINETYPE)
    result = (Temporal *)geo_to_tpointseq(gs);
  else if (gserialized_get_type(gs) == MULTILINETYPE ||
    gserialized_get_type(gs) == COLLECTIONTYPE)
    result = (Temporal *)geo_to_tpointseqset(gs);
  else
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Invalid geometry type for trajectory")));

  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Convert a temporal point into a LinestringM geometry/geography where the M
 * coordinates values are given by a temporal float.
 *****************************************************************************/

static LWPOINT *
point_measure_to_geo_measure(Datum point, Datum measure)
{
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(point);
  int32 srid = gserialized_get_srid(gs);
  double d = DatumGetFloat8(measure);
  LWPOINT *result;
  if (FLAGS_GET_Z(gs->flags))
  {
    const POINT3DZ *point = gs_get_point3dz_p(gs);
    result = lwpoint_make4d(srid, point->x, point->y, point->z, d);
  }
  else
  {
    const POINT2D *point = gs_get_point2d_p(gs);
    result = lwpoint_make3dm(srid, point->x, point->y, d);
  }
  FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(gs->flags));
  return result;
}

/**
 * Construct a geometry/geography with M measure from the temporal instant
 * point and the temporal float. 
 *
 * @param[in] inst Temporal point
 * @param[in] measure Temporal float
 */
static Datum
tpointinst_to_geo_measure(const TInstant *inst, const TInstant *measure)
{
  LWPOINT *point = point_measure_to_geo_measure(tinstant_value(inst),
    tinstant_value(measure));
  GSERIALIZED *result = geo_serialize((LWGEOM *)point);
  pfree(point);
  return PointerGetDatum(result);
}

/**
 * Construct a geometry/geography with M measure from the temporal instant set
 * point and the temporal float. 
 *
 * @param[in] ti Temporal point
 * @param[in] measure Temporal float
 */
static Datum
tpointinstset_to_geo_measure(const TInstantSet *ti, const TInstantSet *measure)
{
  LWGEOM **points = palloc(sizeof(LWGEOM *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    TInstant *inst = tinstantset_inst_n(ti, i);
    TInstant *m = tinstantset_inst_n(measure, i);
    points[i] = (LWGEOM *) point_measure_to_geo_measure(
      tinstant_value(inst), tinstant_value(m));
  }
  GSERIALIZED *result;
  if (ti->count == 1)
    result = geo_serialize(points[0]);
  else
  {
    LWGEOM *mpoint = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE, 
      points[0]->srid, NULL, (uint32_t) ti->count, points);
    result = geo_serialize(mpoint);
    pfree(mpoint);
  }

  for (int i = 0; i < ti->count; i++)
    pfree(points[i]);
  pfree(points);
  return PointerGetDatum(result);
}

/**
 * Construct a geometry/geography with M measure from the temporal sequence
 * point and the temporal float. The function removes one point if two 
 * consecutive points are equal
 *
 * @param[in] seq Temporal point
 * @param[in] measure Temporal float
 * @pre The temporal point and the measure are synchronized
 */
static LWGEOM *
tpointseq_to_geo_measure1(const TSequence *seq, const TSequence *measure)
{
  LWPOINT **points = palloc(sizeof(LWPOINT *) * seq->count);
  /* Remove two consecutive points if they are equal */
  TInstant *inst = tsequence_inst_n(seq, 0);
  TInstant *m = tsequence_inst_n(measure, 0);
  LWPOINT *value1 = point_measure_to_geo_measure(tinstant_value(inst),
    tinstant_value(m));
  points[0] = value1;
  int k = 1;
  for (int i = 1; i < seq->count; i++)
  {
    inst = tsequence_inst_n(seq, i);
    m = tsequence_inst_n(measure, i);
    LWPOINT *value2 = point_measure_to_geo_measure(tinstant_value(inst),
      tinstant_value(m));
    /* Add point only if previous point is diffrent from the current one */
    if (lwpoint_same(value1, value2) != LW_TRUE)
      points[k++] = value2;
    value1 = value2;
  }
  LWGEOM *result;
  if (k == 1)
  {
    result = (LWGEOM *) points[0];
    pfree(points);
  }
  else
  {
    result = MOBDB_FLAGS_GET_LINEAR(seq->flags) ?
      (LWGEOM *) lwline_from_lwgeom_array(points[0]->srid, (uint32_t) k,
        (LWGEOM **) points) :
      (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
        points[0]->srid, NULL, (uint32_t) k, (LWGEOM **) points);
    for (int i = 0; i < k; i++)
      lwpoint_free(points[i]);
    pfree(points);
  }
  return result;
}

/**
 * Construct a geometry/geography with M measure from the temporal sequence
 * point and the temporal float. 
 *
 * @param[in] seq Temporal point
 * @param[in] measure Temporal float
 */
static Datum
tpointseq_to_geo_measure(const TSequence *seq, const TSequence *measure)
{
  LWGEOM *lwgeom = tpointseq_to_geo_measure1(seq, measure);
  GSERIALIZED *result = geo_serialize(lwgeom);
  return PointerGetDatum(result);
}

/**
 * Construct a geometry/geography with M measure from the temporal sequence
 * point and the temporal float. 
 *
 * @param[in] ts Temporal point
 * @param[in] measure Temporal float
 */
static Datum
tpointseqset_to_geo_measure(const TSequenceSet *ts, const TSequenceSet *measure)
{
  /* Instantaneous sequence */
  if (ts->count == 1)
  {
    TSequence *seq1 = tsequenceset_seq_n(ts, 0);
    TSequence *seq2 = tsequenceset_seq_n(measure, 0);
    return tpointseq_to_geo_measure(seq1, seq2);
  }

  uint8_t colltype = 0;
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    TSequence *m = tsequenceset_seq_n(measure, i);
    geoms[i] = tpointseq_to_geo_measure1(seq, m);
    /* Output type not initialized */
    if (! colltype)
      colltype = (uint8_t) lwtype_get_collectiontype(geoms[i]->type);
    /* Input type not compatible with output */
    /* make output type a collection */
    else if (colltype != COLLECTIONTYPE &&
      lwtype_get_collectiontype(geoms[i]->type) != colltype)
      colltype = COLLECTIONTYPE;
  }
  // TODO add the bounding box instead of ask PostGIS to compute it again
  // GBOX *box = stbox_to_gbox(tsequence_bbox_ptr(seq));
  LWGEOM *coll = (LWGEOM *) lwcollection_construct(colltype,
    geoms[0]->srid, NULL, (uint32_t) ts->count, geoms);
  Datum result = PointerGetDatum(geo_serialize(coll));
  /* We cannot lwgeom_free(geoms[i] or lwgeom_free(coll) */
  pfree(geoms);
  return result;
}

/*****************************************************************************/

/**
 * Construct a geometry/geography with M measure from the temporal sequence 
 * point and the temporal float. 
 *
 * Version that produces a Multilinestring when each composing linestring
 * corresponds to a segment of the orginal temporal point.
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq Temporal point
 * @param[in] measure Temporal float
 */
static int
tpointseq_to_geo_measure_segmentize1(LWGEOM **result, const TSequence *seq,
  const TSequence *measure)
{
  TInstant *inst = tsequence_inst_n(seq, 0);
  TInstant *m = tsequence_inst_n(measure, 0);
  LWPOINT *points[2];

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    result[0] = (LWGEOM *) point_measure_to_geo_measure(
      tinstant_value(inst), tinstant_value(m));
    return 1;
  }

  /* General case */
  for (int i = 0; i < seq->count - 1; i++)
  {
    points[0] = point_measure_to_geo_measure(tinstant_value(inst), 
      tinstant_value(m));
    inst = tsequence_inst_n(seq, i + 1);
    points[1] = point_measure_to_geo_measure(tinstant_value(inst), 
      tinstant_value(m));
    result[i] = (LWGEOM *) lwline_from_lwgeom_array(points[0]->srid, 2, 
      (LWGEOM **) points);
    lwpoint_free(points[0]); lwpoint_free(points[1]);
    m = tsequence_inst_n(measure, i + 1);
  }
  return seq->count - 1;
}

/**
 * Construct a geometry/geography with M measure from the temporal sequence 
 * point and the temporal float. 
 *
 * Version that produces a Multilinestring when each composing linestring
 * corresponds to a segment of the orginal temporal point.
 */
static Datum
tpointseq_to_geo_measure_segmentize(const TSequence *seq, 
  const TSequence *measure)
{
  int count = (seq->count == 1) ? 1 : seq->count - 1;
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * count);
  tpointseq_to_geo_measure_segmentize1(geoms, seq, measure);
  Datum result;
  /* Instantaneous sequence */
  if (seq->count == 1)
    result = PointerGetDatum(geo_serialize(geoms[0]));
  else
  {
    // TODO add the bounding box instead of ask PostGIS to compute it again
    // GBOX *box = stbox_to_gbox(tsequence_bbox_ptr(seq));
    LWGEOM *segcoll = (LWGEOM *) lwcollection_construct(MULTILINETYPE,
      geoms[0]->srid, NULL, (uint32_t)(seq->count - 1), geoms);
    result = PointerGetDatum(geo_serialize(segcoll));
  }
  for (int i = 0; i < count; i++)
    lwgeom_free(geoms[i]);
  pfree(geoms);
  return result;
}

/**
 * Construct a geometry/geography with M measure from the temporal sequence set 
 * point and the temporal float. 
 *
 * Version that produces a Multilinestring when each composing linestring
 * corresponds to a segment of the orginal temporal point.
 */
static Datum
tpointseqset_to_geo_measure_segmentize(const TSequenceSet *ts, 
  const TSequenceSet *measure)
{
  /* Instantaneous sequence */
  if (ts->count == 1)
  {
    TSequence *seq1 = tsequenceset_seq_n(ts, 0);
    TSequence *seq2 = tsequenceset_seq_n(measure, 0);
    return tpointseq_to_geo_measure_segmentize(seq1, seq2);
  }

  uint8_t colltype = 0;
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ts->totalcount);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {

    TSequence *seq = tsequenceset_seq_n(ts, i);
    TSequence *m = tsequenceset_seq_n(measure, i);
    k += tpointseq_to_geo_measure_segmentize1(&geoms[k], seq, m);
    /* Output type not initialized */
    if (! colltype)
      colltype = (uint8_t) lwtype_get_collectiontype(geoms[k - 1]->type);
      /* Input type not compatible with output */
      /* make output type a collection */
    else if (colltype != COLLECTIONTYPE &&
         lwtype_get_collectiontype(geoms[k - 1]->type) != colltype)
      colltype = COLLECTIONTYPE;
  }
  Datum result;
  // TODO add the bounding box instead of ask PostGIS to compute it again
  // GBOX *box = stbox_to_gbox(tsequenceset_bbox_ptr(seq));
  LWGEOM *coll = (LWGEOM *) lwcollection_construct(colltype,
    geoms[0]->srid, NULL, (uint32_t) k, geoms);
  result = PointerGetDatum(geo_serialize(coll));
  for (int i = 0; i < k; i++)
    lwgeom_free(geoms[i]);
  pfree(geoms);
  return result;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_to_geo_measure);
/**
 * Construct a geometry/geography with M measure from the temporal point and 
 * the temporal float
 */
PGDLLEXPORT Datum
tpoint_to_geo_measure(PG_FUNCTION_ARGS)
{
  Temporal *tpoint = PG_GETARG_TEMPORAL(0);
  Temporal *measure = PG_GETARG_TEMPORAL(1);
  bool segmentize = PG_GETARG_BOOL(2);
  ensure_tgeo_base_type(tpoint->valuetypid);
  ensure_tnumber_base_type(measure->valuetypid);

  Temporal *sync1, *sync2;
  /* Return false if the temporal values do not intersect in time
     The last parameter crossing must be set to false  */
  if (!intersection_temporal_temporal(tpoint, measure, SYNCHRONIZE,
    &sync1, &sync2))
  {
    PG_FREE_IF_COPY(tpoint, 0);
    PG_FREE_IF_COPY(measure, 1);
    PG_RETURN_NULL();
  }

  Temporal *result;
  ensure_valid_duration(sync1->duration);
  if (sync1->duration == INSTANT)
    result = (Temporal *) tpointinst_to_geo_measure(
      (TInstant *) sync1, (TInstant *) sync2);
  else if (sync1->duration == INSTANTSET)
    result = (Temporal *) tpointinstset_to_geo_measure(
      (TInstantSet *) sync1, (TInstantSet *) sync2);
  else if (sync1->duration == SEQUENCE)
    result = segmentize ?
      (Temporal *) tpointseq_to_geo_measure_segmentize(
        (TSequence *) sync1, (TSequence *) sync2) :
      (Temporal *) tpointseq_to_geo_measure(
        (TSequence *) sync1, (TSequence *) sync2);
  else /* sync1->duration == SEQUENCESET */
    result = segmentize ?
      (Temporal *) tpointseqset_to_geo_measure_segmentize(
        (TSequenceSet *) sync1, (TSequenceSet *) sync2) :
      (Temporal *) tpointseqset_to_geo_measure(
        (TSequenceSet *) sync1, (TSequenceSet *) sync2);

  pfree(sync1); pfree(sync2);
  PG_FREE_IF_COPY(tpoint, 0);
  PG_FREE_IF_COPY(measure, 1);
  PG_RETURN_POINTER(result);
}

/***********************************************************************
 * Simple spatio-temporal Douglas-Peucker line simplification.
 * No checks are done to avoid introduction of self-intersections.
 * No topology relations are considered.
 ***********************************************************************/

/**
 * Determine the 3D hypotenuse.
 *
 * If required, x, y, and z are swapped to make x the larger number. The
 * traditional formula of x^2+y^2+z^2 is rearranged to factor x outside the
 * sqrt. This allows computation of the hypotenuse for significantly
 * larger values, and with a higher precision than when using the naive
 * formula. In particular, this cannot overflow unless the final result
 * would be out-of-range.
 * @code
 * sqrt( x^2 + y^2 + z^2 ) = sqrt( x^2( 1 + y^2/x^2 + z^2/x^2) )
 *                         = x * sqrt( 1 + y^2/x^2 + z^2/x^2)
 *                         = x * sqrt( 1 + y/x * y/x + z/x * z/x)
 * @endcode
 */
double
hypot3d(double x, double y, double z)
{
  double yx;
  double zx;
  double temp;

  /* Handle INF and NaN properly */
  if (isinf(x) || isinf(y) || isinf(z))
    return get_float8_infinity();

  if (isnan(x) || isnan(y) || isnan(z))
    return get_float8_nan();

  /* Else, drop any minus signs */
  x = fabs(x);
  y = fabs(y);
  z = fabs(z);

  /* Swap x, y and z if needed to make x the larger one */
  if (x < y)
  {
    temp = x;
    x = y;
    y = temp;
  }
  if (x < z)
  {
    temp = x;
    x = z;
    z = temp;
  }
  /*
   * If x is zero, the hypotenuse is computed with the 2D case.
   * This test saves a few cycles in such cases, but more importantly
   * it also protects against divide-by-zero errors, since now x >= y.
   */
  if (x == 0)
    return hypot(y, z);

  /* Determine the hypotenuse */
  yx = y / x;
  zx = z / x;
  return x * sqrt(1.0 + (yx * yx) + (zx * zx));
}

/**
 * Determine the 4D hypotenuse.
 *
 * @see The function is a generalization of the 3D case in the function hypot3d
 */
double
hypot4d(double x, double y, double z, double m)
{
  double yx;
  double zx;
  double mx;
  double temp;

  /* Handle INF and NaN properly */
  if (isinf(x) || isinf(y) || isinf(z) || isinf(m))
    return get_float8_infinity();

  if (isnan(x) || isnan(y) || isnan(z) || isnan(m))
    return get_float8_nan();

  /* Else, drop any minus signs */
  x = fabs(x);
  y = fabs(y);
  z = fabs(z);
  m = fabs(m);

  /* Swap x, y, z, and m if needed to make x the larger one */
  if (x < y)
  {
    temp = x;
    x = y;
    y = temp;
  }
  if (x < z)
  {
    temp = x;
    x = z;
    z = temp;
  }
  if (x < m)
  {
    temp = x;
    x = m;
    m = temp;
  }
  /*
   * If x is zero, the hypotenuse is computed with the 3D case.
   * This test saves a few cycles in such cases, but more importantly
   * it also protects against divide-by-zero errors, since now x >= y.
   */
  if (x == 0)
    return hypot3d(y, z, m);

  /* Determine the hypotenuse */
  yx = y / x;
  zx = z / x;
  mx = m / x;
  return x * sqrt(1.0 + (yx * yx) + (zx * zx) + (mx * mx));
}

/**
 * Returns the 2D distance between the points
 */
double
dist2d_pt_pt(POINT2D *p1, POINT2D *p2)
{
  double dx = p2->x - p1->x;
  double dy = p2->y - p1->y;
  return hypot(dx, dy);
}

/**
 * Returns the 3D distance between the points
 */
double
dist3d_pt_pt(POINT3DZ *p1, POINT3DZ *p2)
{
  double dx = p2->x - p1->x;
  double dy = p2->y - p1->y;
  double dz = p2->z - p1->z;
  return hypot3d(dx, dy, dz);
}

/**
 * Returns the 4D distance between the points
 */
double
dist4d_pt_pt(POINT4D *p1, POINT4D *p2)
{
  double dx = p2->x - p1->x;
  double dy = p2->y - p1->y;
  double dz = p2->z - p1->z;
  double dm = p2->m - p1->m;
  return hypot4d(dx, dy, dz, dm);
}

/**
 * Returns the 2D distance between the point the segment
 * 
 * @param[in] p Point
 * @param[in] A,B Points defining the segment
 * @see http://geomalgorithms.com/a02-_lines.html 
 * @note Derived from the PostGIS function lw_dist2d_pt_seg in 
 * file measures.c
 */
double
dist2d_pt_seg(POINT2D *p, POINT2D *A, POINT2D *B)
{
  POINT2D c;
  double r;
  /* If start==end, then use pt distance */
  if (A->x == B->x && A->y == B->y)
    return dist2d_pt_pt(p, A);

  r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) ) /
    ( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) );

  if (r < 0) /* If the first vertex A is closest to the point p */
    return dist2d_pt_pt(p, A);
  if (r > 1)  /* If the second vertex B is closest to the point p */
    return dist2d_pt_pt(p, B);

  /* else if the point p is closer to some point between a and b
  then we find that point and send it to dist2d_pt_pt */
  c.x = A->x + r * (B->x - A->x);
  c.y = A->y + r * (B->y - A->y);

  return dist2d_pt_pt(p, &c);
}

/**
 * Returns the 3D distance between the point the segment
 * 
 * @param[in] p Point
 * @param[in] A,B Points defining the segment
 * @note Derived from the PostGIS function lw_dist3d_pt_seg in file 
 * measures3d.c
 * @see http://geomalgorithms.com/a02-_lines.html 
 */
double
dist3d_pt_seg(POINT3DZ *p, POINT3DZ *A, POINT3DZ *B)
{
  POINT3DZ c;
  double r;
  /* If start==end, then use pt distance */
  if (A->x == B->x && A->y == B->y && A->z == B->z)
    return dist3d_pt_pt(p, A);

  r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) + 
      (p->z-A->z) * (B->z-A->z) ) /
    ( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) + 
      (B->z-A->z) * (B->z-A->z) );

  if (r < 0) /* If the first vertex A is closest to the point p */
    return dist3d_pt_pt(p, A);
  if (r > 1) /* If the second vertex B is closest to the point p */
    return dist3d_pt_pt(p, B);

  /* else if the point p is closer to some point between a and b
  then we find that point and send it to dist3d_pt_pt */
  c.x = A->x + r * (B->x - A->x);
  c.y = A->y + r * (B->y - A->y);
  c.z = A->z + r * (B->z - A->z);

  return dist3d_pt_pt(p, &c);
}

/**
 * Returns the 4D distance between the point the segment
 * 
 * @param[in] p Point
 * @param[in] A,B Points defining the segment
 * @note Derived from the PostGIS function lw_dist3d_pt_seg in file 
 * measures3d.c
 * @see http://geomalgorithms.com/a02-_lines.html 
 */
double
dist4d_pt_seg(POINT4D *p, POINT4D *A, POINT4D *B)
{
  POINT4D c;
  double r;
  /* If start==end, then use pt distance */
  if (A->x == B->x && A->y == B->y && A->z == B->z && A->m == B->m)
    return dist4d_pt_pt(p, A);

  r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) + 
      (p->z-A->z) * (B->z-A->z) + (p->m-A->m) * (B->m-A->m) ) /
    ( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) + 
      (B->z-A->z) * (B->z-A->z) + (B->m-A->m) * (B->m-A->m) );

  if (r < 0) /* If the first vertex A is closest to the point p */
    return dist4d_pt_pt(p, A);
  if (r > 1) /* If the second vertex B is closest to the point p */
    return dist4d_pt_pt(p, B);

  /* else if the point p is closer to some point between a and b
  then we find that point and send it to dist3d_pt_pt */
  c.x = A->x + r * (B->x - A->x);
  c.y = A->y + r * (B->y - A->y);
  c.z = A->z + r * (B->z - A->z);
  c.m = A->m + r * (B->m - A->m);

  return dist4d_pt_pt(p, &c);
}

/**
 * Finds a split when simplifying the temporal sequence point using a 
 * spatio-temporal extension of the Douglas-Peucker line simplification 
 * algorithm.
 *
 * @param[in] seq Temporal sequence
 * @param[in] p1,p2 Reference points
 * @param[in] withspeed True when the delta in the speed must be considered
 * @param[out] split Location of the split
 * @param[out] dist Distance at the split
 * @param[out] delta_speed Delta speed at the split
 */
static void
tpointseq_dp_findsplit(const TSequence *seq, int p1, int p2, bool withspeed,
  int *split, double *dist, double *delta_speed)
{
  POINT2D p2k, p2k_tmp, p2a, p2b;
  POINT3DZ p3k, p3k_tmp, p3a, p3b;
  POINT4D p4k, p4a, p4b;
  double d;
  bool hasz = MOBDB_FLAGS_GET_Z(seq->flags);
  *split = p1;
  d = -1;
  if (p1 + 1 < p2)
  {
    double speed_seg;
    Datum (*func)(Datum, Datum);
    if (withspeed)
      func = hasz ? &pt_distance3d : &pt_distance2d;
    TInstant *inst1 = tsequence_inst_n(seq, p1);
    TInstant *inst2 = tsequence_inst_n(seq, p2);
    if (withspeed)
      speed_seg = tpointinst_speed(inst1, inst2, func);
    if (hasz)
    {
      p3a = datum_get_point3dz(tinstant_value(inst1));
      p3b = datum_get_point3dz(tinstant_value(inst2));
      if (withspeed)
      {
        p4a.x = p3a.x; p4a.y = p3a.y; 
        p4a.z = p3a.z; p4a.m = speed_seg;
        p4b.x = p3b.x; p4b.y = p3b.y; 
        p4b.z = p3b.z; p4b.m = speed_seg;
      }
    }
    else
    {
      p2a = datum_get_point2d(tinstant_value(inst1));
      p2b = datum_get_point2d(tinstant_value(inst2));
      if (withspeed)
      {
        p3a.x = p2a.x; p3a.y = p2a.y; p3a.z = speed_seg;
        p3b.x = p2b.x; p3b.y = p2b.y; p3b.z = speed_seg;
      }
    }
    for (int k = p1 + 1; k < p2; k++)
    {
      double d_tmp, speed_pt;
      inst2 = tsequence_inst_n(seq, k);
      if (withspeed)
        speed_pt = tpointinst_speed(inst1, inst2, func);
      if (hasz)
      {
        p3k_tmp = datum_get_point3dz(tinstant_value(inst2));
        if (withspeed)
        {
          p4k.x = p3k_tmp.x; p4k.y = p3k_tmp.y; 
          p4k.z = p3k_tmp.z; p4k.m = speed_pt;
          d_tmp = dist4d_pt_seg(&p4k, &p4a, &p4b);
        }
        else
          d_tmp = dist3d_pt_seg(&p3k_tmp, &p3a, &p3b);
      }
      else
      {
        p2k_tmp = datum_get_point2d(tinstant_value(inst2));
        if (withspeed)
        {
          p3k.x = p2k_tmp.x; p3k.y = p2k_tmp.y; p3k.z = speed_pt;
          d_tmp = dist3d_pt_seg(&p3k, &p3a, &p3b);
        }
        else
          d_tmp = dist2d_pt_seg(&p2k_tmp, &p2a, &p2b);
      }
      if (d_tmp > d)
      {
        /* record the maximum */
        d = d_tmp;
        if (hasz)
          p3k = p3k_tmp;
        else
          p2k = p2k_tmp;
        if (withspeed)
          *delta_speed = fabs(speed_seg - speed_pt);
        *split = k;
      }
      inst1 = inst2;
    }
    *dist = hasz ? dist3d_pt_seg(&p3k, &p3a, &p3b) :
      distance2d_pt_seg(&p2k, &p2a, &p2b);
  }
  else
    *dist = -1;
}

/***********************************************************************/

/**
 * Simplifies the temporal sequence point using a spatio-temporal 
 * extension of the Douglas-Peucker line simplification algorithm.
 *
 * @param[in] seq Temporal point
 * @param[in] eps_dist Epsilon speed
 * @param[in] eps_speed Epsilon speed
 * @param[in] minpts Minimum number of points
 */
TSequence *
tpointseq_simplify(const TSequence *seq, double eps_dist, 
  double eps_speed, uint32_t minpts)
{
  static size_t stack_size = 256;
  int *stack, *outlist; /* recursion stack */
  int stack_static[stack_size];
  int outlist_static[stack_size];
  int sp = -1; /* recursion stack pointer */
  int p1, split;
  uint32_t outn = 0;
  uint32_t i;
  double dist, delta_speed;
  bool withspeed = eps_speed > 0;

  /* Do not try to simplify really short things */
  if (seq->count < 3)
    return tsequence_copy(seq);

  /* Only heap allocate book-keeping arrays if necessary */
  if ((unsigned int) seq->count > stack_size)
  {
    stack = palloc(sizeof(int) * seq->count);
    outlist = palloc(sizeof(int) * seq->count);
  }
  else
  {
    stack = stack_static;
    outlist = outlist_static;
  }

  p1 = 0;
  stack[++sp] = seq->count - 1;
  /* Add first point to output list */
  outlist[outn++] = 0;
  do
  {
    tpointseq_dp_findsplit(seq, p1, stack[sp], withspeed, &split, &dist, &delta_speed);
    bool dosplit;
    if (withspeed)
      dosplit = (dist >= 0 &&
        (dist > eps_dist || delta_speed > eps_speed || outn + sp + 1 < minpts));
    else
      dosplit = (dist >= 0 &&
        (dist > eps_dist || outn + sp + 1 < minpts));
    if (dosplit)
      stack[++sp] = split;
    else
    {
      outlist[outn++] = stack[sp];
      p1 = stack[sp--];
    }
  }
  while (sp >= 0);

  /* Put list of retained points into order */
  qsort(outlist, outn, sizeof(int), int_cmp);
  /* Create new TSequence */
  TInstant **instants = palloc(sizeof(TInstant *) * outn);
  for (i = 0; i < outn; i++)
    instants[i] = tsequence_inst_n(seq, outlist[i]);
  TSequence *result = tsequence_make(instants, outn,
    seq->period.lower_inc, seq->period.upper_inc,
    MOBDB_FLAGS_GET_LINEAR(seq->flags), NORMALIZE);
  pfree(instants);

  /* Only free if arrays are on heap */
  if (stack != stack_static)
    pfree(stack);
  if (outlist != outlist_static)
    pfree(outlist);

  return result;
}

/**
 * Simplifies the temporal sequence set point using a spatio-temporal 
 * extension of the Douglas-Peucker line simplification algorithm.
 *
 * @param[in] ts Temporal point
 * @param[in] eps_dist Epsilon speed
 * @param[in] eps_speed Epsilon speed
 * @param[in] minpts Minimum number of points
 */
TSequenceSet *
tpointseqset_simplify(const TSequenceSet *ts, double eps_dist, 
  double eps_speed, uint32_t minpts)
{
  TSequenceSet *result;
  /* Singleton sequence set */
  if (ts->count == 1)
  {
    TSequence *seq = tpointseq_simplify(tsequenceset_seq_n(ts, 0), 
      eps_dist, eps_speed, minpts);
    result = tsequence_to_tsequenceset(seq);
    pfree(seq);
    return result;
  }

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
    sequences[i] = tpointseq_simplify(tsequenceset_seq_n(ts, i), 
      eps_dist, eps_speed, minpts);
  return tsequenceset_make_free(sequences, ts->count, NORMALIZE);
}

PG_FUNCTION_INFO_V1(tpoint_simplify);
/**
 * Simplifies the temporal sequence (set) point using a spatio-temporal 
 * extension of the Douglas-Peucker line simplification algorithm.
 */
Datum
tpoint_simplify(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  double eps_dist = PG_GETARG_FLOAT8(1);
  double eps_speed = PG_GETARG_FLOAT8(2);

  Temporal *result;
  ensure_valid_duration(temp->duration);
  if (temp->duration == INSTANT || temp->duration == INSTANTSET ||
    ! MOBDB_FLAGS_GET_LINEAR(temp->flags))
    result = temporal_copy(temp);
  else if (temp->duration == SEQUENCE)
    result = (Temporal *) tpointseq_simplify((TSequence *)temp,
      eps_dist, eps_speed, 2);
  else /* temp->duration == SEQUENCESET */
    result = (Temporal *) tpointseqset_simplify((TSequenceSet *)temp,
      eps_dist, eps_speed, 2);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
