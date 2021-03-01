/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * @file tpoint_spatialfuncs.c
 * Spatial functions for temporal points.
 */

#include "tpoint_spatialfuncs.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>

#include "period.h"
#include "periodset.h"
#include "timeops.h"
#include "doublen.h"
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

/**
 * Returns true if the segment of the temporal point value intersects
 * the base value at the timestamp
 *
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] value Base value
 * @param[out] t Timestamp
 */
bool
tpointseq_intersection_value(const TInstant *inst1, const TInstant *inst2,
  Datum value, TimestampTz *t)
{
  GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(value);
  if (gserialized_is_empty(gs))
  {
    POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(value));
    return false;
  }

  /* We are sure that the trajectory is a line */
  Datum start = tinstant_value(inst1);
  Datum end = tinstant_value(inst2);
  double dist;
  double fraction = geoseg_locate_point(start, end, value, &dist);
  if (fabs(dist) >= EPSILON)
    return false;

  if (t != NULL)
  {
    double duration = (inst2->t - inst1->t);
    *t = inst1->t + (long) (duration * fraction);
  }
  return true;
}

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * Ensure that the spatial constraints required for operating on two temporal
 * geometries are satisfied
 */
void
ensure_spatial_validity(const Temporal *temp1, const Temporal *temp2)
{
  if (tgeo_base_type(temp1->valuetypid))
  {
    ensure_same_srid_tpoint(temp1, temp2);
    ensure_same_dimensionality(temp1->flags, temp2->flags);
  }
  return;
}

/**
 * Ensure that the spatiotemporal boxes have the same type of coordinates,
 * either planar or geodetic
 */
void
ensure_same_geodetic(int16 flags1, int16 flags2)
{
  if (MOBDB_FLAGS_GET_X(flags1) && MOBDB_FLAGS_GET_X(flags2) &&
    MOBDB_FLAGS_GET_GEODETIC(flags1) != MOBDB_FLAGS_GET_GEODETIC(flags2))
    elog(ERROR, "The values must be both planar or both geodetic");
  return;
}

/**
 * Ensure that the spatiotemporal boxes have the same SRID
 */
void
ensure_same_srid_stbox(const STBOX *box1, const STBOX *box2)
{
  if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags) &&
    box1->srid != box2->srid)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The boxes must be in the same SRID")));
  return;
}

/**
 * Ensure that the temporal points have the same SRID
 */
void
ensure_same_srid_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal points must be in the same SRID")));
  return;
}

/**
 * Ensure that the temporal point and the spatiotemporal boxes have the same SRID
 */
void
ensure_same_srid_tpoint_stbox(const Temporal *temp, const STBOX *box)
{
  if (MOBDB_FLAGS_GET_X(box->flags) &&
    tpoint_srid_internal(temp) != box->srid)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal point and the box must be in the same SRID")));
  return;
}

/**
 * Ensure that the temporal point and the geometry/geography have the same SRID
 */
void
ensure_same_srid_tpoint_gs(const Temporal *temp, const GSERIALIZED *gs)
{
  if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal point and the geometry must be in the same SRID")));
  return;
}

/**
 * Ensure that the temporal point and the geometry/geography have the same SRID
 */
void
ensure_same_srid_stbox_gs(const STBOX *box, const GSERIALIZED *gs)
{
  if (box->srid != gserialized_get_srid(gs))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The spatiotemporal box and the geometry must be in the same SRID")));
  return;
}

/**
 * Ensure that the temporal values have the same dimensionality
 */
void
ensure_same_dimensionality(int16 flags1, int16 flags2)
{
  if (MOBDB_FLAGS_GET_X(flags1) != MOBDB_FLAGS_GET_X(flags2) ||
    MOBDB_FLAGS_GET_Z(flags1) != MOBDB_FLAGS_GET_Z(flags2) ||
    MOBDB_FLAGS_GET_T(flags1) != MOBDB_FLAGS_GET_T(flags2))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal values must be of the same dimensionality")));
  return;
}

/**
 * Ensure that the temporal values have the same spatial dimensionality
 */
void
ensure_same_spatial_dimensionality(int16 flags1, int16 flags2)
{
  if (MOBDB_FLAGS_GET_X(flags1) != MOBDB_FLAGS_GET_X(flags2) ||
    MOBDB_FLAGS_GET_Z(flags1) != MOBDB_FLAGS_GET_Z(flags2))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal values must be of the same spatial dimensionality")));
  return;
}

/**
 * Ensure that the temporal point and the geometry/geography have the same dimensionality
 */
void
ensure_same_dimensionality_tpoint_gs(const Temporal *temp, const GSERIALIZED *gs)
{
  if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal point and the geometry must be of the same dimensionality")));
  return;
}

/**
 * Ensure that the spatiotemporal boxes have the same spatial dimensionality
 */
void
ensure_same_spatial_dimensionality_stbox_gs(const STBOX *box, const GSERIALIZED *gs)
{
  if (! MOBDB_FLAGS_GET_X(box->flags) ||
      MOBDB_FLAGS_GET_Z(box->flags) != FLAGS_GET_Z(gs->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The spatiotemporal box and the geometry must be of the same dimensionality")));
  return;
}

/**
 * Ensure that the temporal value has XY dimension
 */
void
ensure_has_X(int16 flags)
{
  if (! MOBDB_FLAGS_GET_X(flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal value must have XY dimension")));
  return;
}

/**
 * Ensure that the temporal value has Z dimension
 */
void
ensure_has_Z(int16 flags)
{
  if (! MOBDB_FLAGS_GET_Z(flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal value must have Z dimension")));
  return;
}

/**
 * Ensure that the temporal value has T dimension
 */
void
ensure_has_T(int16 flags)
{
  if (! MOBDB_FLAGS_GET_T(flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal value must have time dimension")));
  return;
}

/**
 * Ensure that the temporal point has Z dimension
 */
void
ensure_has_Z_tpoint(const Temporal *temp)
{
  if (! MOBDB_FLAGS_GET_Z(temp->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal point do not have Z dimension")));
  return;
}

/**
 * Ensure that the temporal point has not Z dimension
 */
void
ensure_has_not_Z(int16 flags)
{
  if (MOBDB_FLAGS_GET_Z(flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal point cannot have Z dimension")));
  return;
}

/**
 * Ensure that the geometry/geography has not Z dimension
 */
void
ensure_has_not_Z_gs(const GSERIALIZED *gs)
{
  if (FLAGS_GET_Z(gs->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Only geometries without Z dimension accepted")));
  return;
}

/**
 * Ensure that the geometry/geography has M dimension
 */
void
ensure_has_M_gs(const GSERIALIZED *gs)
{
  if (! FLAGS_GET_M(gs->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Only geometries with M dimension accepted")));
  return;
}

/**
 * Ensure that the geometry/geography has not M dimension
 */
void
ensure_has_not_M_gs(const GSERIALIZED *gs)
{
  if (FLAGS_GET_M(gs->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Only geometries without M dimension accepted")));
  return;
}

/**
 * Ensure that the geometry/geography is a point
 */
void
ensure_point_type(const GSERIALIZED *gs)
{
  if (gserialized_get_type(gs) != POINTTYPE)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Only point geometries accepted")));
  return;
}

/**
 * Ensure that the geometry/geography is not empty
 */
void
ensure_non_empty(const GSERIALIZED *gs)
{
  if (gserialized_is_empty(gs))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Only non-empty geometries accepted")));
  return;
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
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(geom);
  POINT2D *point = (POINT2D *)((uint8_t*)gs->data + 8);
  return *point;
}

/**
 * Returns a pointer to a 2D point from the datum
 */
const POINT2D *
datum_get_point2d_p(Datum geom)
{
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(geom);
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
    return FP_EQUALS(point1->x, point2->x) && FP_EQUALS(point1->y, point2->y) &&
      FP_EQUALS(point1->z, point2->z);
  }
  else
  {
    const POINT2D *point1 = gs_get_point2d_p(gs1);
    const POINT2D *point2 = gs_get_point2d_p(gs2);
    return FP_EQUALS(point1->x, point2->x) && FP_EQUALS(point1->y, point2->y);
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
 * Compute the trajectory from two geometry points
 *
 * @param[in] value1,value2 Points
 */
LWLINE *
geopoint_lwline(Datum value1, Datum value2)
{
  GSERIALIZED *gs1 = (GSERIALIZED *) DatumGetPointer(value1);
  GSERIALIZED *gs2 = (GSERIALIZED *) DatumGetPointer(value2);
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
 * Compute the trajectory from two points
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
 * type from the precomputed trajectories of its composing segments.
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
    GSERIALIZED *gstraj = (GSERIALIZED *) DatumGetPointer(traj);
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
 * type
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
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_value_copy((TInstant *)temp);
  else if (temp->temptype == INSTANTSET)
    result = tpointinstset_trajectory((TInstantSet *)temp);
  else if (temp->temptype == SEQUENCE)
    result = tpointseq_trajectory_copy((TSequence *)temp);
  else /* temp->temptype == SEQUENCESET */
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
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(tinstant_value_ptr(inst));
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
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tpointinst_srid((TInstant *)temp);
  else if (temp->temptype == INSTANTSET)
    result = tpointinstset_srid((TInstantSet *)temp);
  else if (temp->temptype == SEQUENCE)
    result = tpointseq_srid((TSequence *)temp);
  else /* temp->temptype == SEQUENCESET */
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
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(tinstant_value_ptr(result));
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
    GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(tinstant_value_ptr(inst));
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
  GSERIALIZED *gs;
  TSequence *result = tsequence_copy(seq);
  /* Set the SRID of the composing points */
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = tsequence_inst_n(result, i);
    gs = (GSERIALIZED *) DatumGetPointer(tinstant_value_ptr(inst));
    gserialized_set_srid(gs, srid);
  }
  /* Set the SRID of the precomputed trajectory */
  Datum traj = tpointseq_trajectory(result);
  gs = (GSERIALIZED *) DatumGetPointer(traj);
  gserialized_set_srid(gs, srid);
  /* Set the SRID of the bounding box */
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
  STBOX *box;
  TSequenceSet *result = tsequenceset_copy(ts);
  /* Loop for every composing sequence */
  for (int i = 0; i < ts->count; i++)
  {
    GSERIALIZED *gs;
    TSequence *seq = tsequenceset_seq_n(result, i);
    for (int j = 0; j < seq->count; j++)
    {
      /* Set the SRID of the composing points */
      TInstant *inst = tsequence_inst_n(seq, j);
      gs = (GSERIALIZED *) DatumGetPointer(tinstant_value_ptr(inst));
      gserialized_set_srid(gs, srid);
    }
    /* Set the SRID of the precomputed trajectory */
    Datum traj = tpointseq_trajectory(seq);
    gs = (GSERIALIZED *) DatumGetPointer(traj);
    gserialized_set_srid(gs, srid);
    /* Set the SRID of the bounding box */
    box = tsequence_bbox_ptr(seq);
    box->srid = srid;
  }
  /* Set the SRID of the bounding box */
  box = tsequenceset_bbox_ptr(result);
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
  if (temp->temptype == INSTANT)
    result = (Temporal *)tpointinst_set_srid((TInstant *)temp, srid);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *)tpointinstset_set_srid((TInstantSet *)temp, srid);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *)tpointseq_set_srid((TSequence *)temp, srid);
  else /* temp->temptype == SEQUENCESET */
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
  ensure_has_X(box->flags);
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
    TInstantSet *result = tinstantset_make(&inst, 1, MERGE_NO);
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

  return tinstantset_make_free(instants, ti->count, MERGE_NO);
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
    GSERIALIZED *gsvalue = (GSERIALIZED *) DatumGetPointer(value);
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
      GSERIALIZED *gsvalue = (GSERIALIZED *) DatumGetPointer(value);
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
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (Temporal *) tpointinst_transform((TInstant *)temp, srid);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tpointinstset_transform((TInstantSet *)temp, srid);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *) tpointseq_transform((TSequence *)temp, srid);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tpointseqset_transform((TSequenceSet *)temp, srid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 * Notice that a geometry point and a geography point are of different size
 * since the geography point keeps a bounding box
 *****************************************************************************/

/** Symbolic constants for transforming tgeompoint <-> tgeogpoint */
#define GEOG_FROM_GEOM        true
#define GEOM_FROM_GEOG        false

/**
 * Converts the temporal point to a geometry/geography point
 */
static TInstant *
tpointinst_convert_tgeom_tgeog(const TInstant *inst, bool oper)
{
    Datum point = (oper == GEOG_FROM_GEOM) ?
      call_function1(geography_from_geometry, tinstant_value(inst)) :
      call_function1(geometry_from_geography, tinstant_value(inst));
    return tinstant_make(point, inst->t, (oper == GEOG_FROM_GEOM) ?
      type_oid(T_GEOGRAPHY) : type_oid(T_GEOMETRY));
}

/**
 * Converts the temporal point to a geometry/geography point
 */
static TInstantSet *
tpointinstset_convert_tgeom_tgeog(const TInstantSet *ti, bool oper)
{
  /* Construct a multipoint with all the points */
  LWPOINT **points = palloc(sizeof(LWPOINT *) * ti->count);
  TInstant *inst;
  GSERIALIZED *gs;
  for (int i = 0; i < ti->count; i++)
  {
    inst = tinstantset_inst_n(ti, i);
    gs = (GSERIALIZED *) DatumGetPointer(tinstant_value_ptr(inst));
    points[i] = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
  }
  LWGEOM *lwresult = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
      points[0]->srid, NULL, (uint32_t) ti->count, (LWGEOM **) points);
  Datum mpoint_orig = PointerGetDatum(geo_serialize(lwresult));
  for (int i = 0; i < ti->count; i++)
    lwpoint_free(points[i]);
  pfree(points);
  /* Convert the multipoint geometry/geography */
  Datum mpoint_trans = (oper == GEOG_FROM_GEOM) ?
      call_function1(geography_from_geometry, mpoint_orig) :
      call_function1(geometry_from_geography, mpoint_orig);
  /* Construct the resulting tpoint from the multipoint geometry/geography */
  gs = (GSERIALIZED *) DatumGetPointer(mpoint_trans);
  LWMPOINT *lwmpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    inst = tinstantset_inst_n(ti, i);
    Datum point = PointerGetDatum(geo_serialize((LWGEOM *)(lwmpoint->geoms[i])));
    instants[i] = tinstant_make(point, inst->t, (oper == GEOG_FROM_GEOM) ?
      type_oid(T_GEOGRAPHY) : type_oid(T_GEOMETRY));
    pfree(DatumGetPointer(point));
  }
  lwmpoint_free(lwmpoint);
  return tinstantset_make_free(instants, ti->count, MERGE_NO);
}

/**
 * Converts the temporal point to a geometry/geography point
 */
static TSequence *
tpointseq_convert_tgeom_tgeog(const TSequence *seq, bool oper)
{
  /* Construct a multipoint with all the points */
  LWPOINT **points = palloc(sizeof(LWPOINT *) * seq->count);
  TInstant *inst;
  GSERIALIZED *gs;
  for (int i = 0; i < seq->count; i++)
  {
    inst = tsequence_inst_n(seq, i);
    gs = (GSERIALIZED *) DatumGetPointer(tinstant_value_ptr(inst));
    points[i] = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
  }
  LWGEOM *lwresult = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
      points[0]->srid, NULL, (uint32_t) seq->count, (LWGEOM **) points);
  Datum mpoint_orig = PointerGetDatum(geo_serialize(lwresult));
  for (int i = 0; i < seq->count; i++)
    lwpoint_free(points[i]);
  pfree(points);
  /* Convert the multipoint geometry/geography */
  Datum mpoint_trans = (oper == GEOG_FROM_GEOM) ?
      call_function1(geography_from_geometry, mpoint_orig) :
      call_function1(geometry_from_geography, mpoint_orig);
  /* Construct the resulting tpoint from the multipoint geometry/geography */
  gs = (GSERIALIZED *) DatumGetPointer(mpoint_trans);
  LWMPOINT *lwmpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    inst = tsequence_inst_n(seq, i);
    Datum point = PointerGetDatum(geo_serialize((LWGEOM *)(lwmpoint->geoms[i])));
    instants[i] = tinstant_make(point, inst->t, (oper == GEOG_FROM_GEOM) ?
      type_oid(T_GEOGRAPHY) : type_oid(T_GEOMETRY));
    pfree(DatumGetPointer(point));
  }
  lwmpoint_free(lwmpoint);
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MOBDB_FLAGS_GET_LINEAR(seq->flags), NORMALIZE_NO);
}

/**
 * Converts the temporal point to a geometry/geography point
 */
static TSequenceSet *
tpointseqset_convert_tgeom_tgeog(const TSequenceSet *ts, bool oper)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    sequences[i] = tpointseq_convert_tgeom_tgeog(seq, oper);
  }
  return tsequenceset_make_free(sequences, ts->count, NORMALIZE_NO);
}

/**
 * Converts the temporal point to a geometry/geography point
 * (dispatch function)
 */
Temporal *
tpoint_convert_tgeom_tgeog(const Temporal *temp, bool oper)
{
  Temporal *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (Temporal *) tpointinst_convert_tgeom_tgeog(
      (TInstant *)temp, oper);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *) tpointinstset_convert_tgeom_tgeog(
      (TInstantSet *)temp, oper);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *) tpointseq_convert_tgeom_tgeog(
      (TSequence *)temp, oper);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *) tpointseqset_convert_tgeom_tgeog(
      (TSequenceSet *)temp, oper);
  return result;
}

/**
 * Converts the temporal point to a geometry/geography point
 */
PG_FUNCTION_INFO_V1(tgeompoint_to_tgeogpoint);
PGDLLEXPORT Datum
tgeompoint_to_tgeogpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result = tpoint_convert_tgeom_tgeog(temp, GEOG_FROM_GEOM);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tgeogpoint_to_tgeompoint);
PGDLLEXPORT Datum
tgeogpoint_to_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result = tpoint_convert_tgeom_tgeog(temp, GEOM_FROM_GEOG);
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
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(value);
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
 * Functions for extracting coordinates.
 *****************************************************************************/

/**
 * Get the X coordinates of the temporal point
 */
static Datum
tpoint_get_x_internal(Datum point)
{
  POINT4D p = datum_get_point4d(point);
  return Float8GetDatum(p.x);
}

PG_FUNCTION_INFO_V1(tpoint_get_x);
/**
 * Get the X coordinates of the temporal point
 */
PGDLLEXPORT Datum
tpoint_get_x(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_tgeo_base_type(temp->valuetypid);
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) &tpoint_get_x_internal;
  lfinfo.numparam = 1;
  lfinfo.restypid = FLOAT8OID;
  Temporal *result = tfunc_temporal(temp, (Datum) NULL, lfinfo);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/**
 * Get the Y coordinates of the temporal point
 */
static Datum
tpoint_get_y_internal(Datum point)
{
  POINT4D p = datum_get_point4d(point);
  return Float8GetDatum(p.y);
}

PG_FUNCTION_INFO_V1(tpoint_get_y);
/**
 * Get the Y coordinates of the temporal point
 */
PGDLLEXPORT Datum
tpoint_get_y(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_tgeo_base_type(temp->valuetypid);
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) &tpoint_get_y_internal;
  lfinfo.numparam = 1;
  lfinfo.restypid = FLOAT8OID;
  Temporal *result = tfunc_temporal(temp, (Datum) NULL, lfinfo);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/**
 * Get the Z coordinates of the temporal point
 */
static Datum
tpoint_get_z_internal(Datum point)
{
  POINT4D p = datum_get_point4d(point);
  return Float8GetDatum(p.z);
}

PG_FUNCTION_INFO_V1(tpoint_get_z);
/**
 * Get the Z coordinates of the temporal point
 */
PGDLLEXPORT Datum
tpoint_get_z(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ensure_tgeo_base_type(temp->valuetypid);
  ensure_has_Z_tpoint(temp);
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) &tpoint_get_z_internal;
  lfinfo.numparam = 1;
  lfinfo.restypid = FLOAT8OID;
  Temporal *result = tfunc_temporal(temp, (Datum) NULL, lfinfo);
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
  GSERIALIZED *gstraj = (GSERIALIZED *) DatumGetPointer(traj);
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
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT || temp->temptype == INSTANTSET ||
    ! MOBDB_FLAGS_GET_LINEAR(temp->flags))
    ;
  else if (temp->temptype == SEQUENCE)
    result = tpointseq_length((TSequence *)temp);
  else /* temp->temptype == SEQUENCESET */
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
  return tinstantset_make_free(instants, ti->count, MERGE_NO);
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
  ensure_valid_temptype(temp->temptype);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  if (temp->temptype == INSTANT)
    result = (Temporal *)tpointinst_cumulative_length((TInstant *)temp);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *)tpointinstset_cumulative_length((TInstantSet *)temp);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *)tpointseq_cumulative_length((TSequence *)temp, 0);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *)tpointseqset_cumulative_length((TSequenceSet *)temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

/**
 * Returns the speed of the temporal point
 * @pre The temporal point has linear interpolation
 */
static TSequence *
tpointseq_speed(const TSequence *seq)
{
  assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));

  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  /* General case */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
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
    speed = datum_point_eq(value1, value2) ? 0 :
      DatumGetFloat8(func(value1, value2)) /
        ((double)(inst2->t - inst1->t) / 1000000);
    instants[i] = tinstant_make(Float8GetDatum(speed), inst1->t,
      FLOAT8OID);
    inst1 = inst2;
    value1 = value2;
  }
  instants[seq->count - 1] = tinstant_make(Float8GetDatum(speed),
    seq->period.upper, FLOAT8OID);
  /* The resulting sequence has step interpolation */
  TSequence *result = tsequence_make(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE);
  for (int i = 0; i < seq->count - 1; i++)
    pfree(instants[i]);
  pfree(instants);
  return result;
}

/**
 * Returns the speed of the temporal point
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
 * Returns the speed of the temporal point
 */
PGDLLEXPORT Datum
tpoint_speed(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Temporal *result = NULL;
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  ensure_linear_interpolation(temp->flags);
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT || temp->temptype == INSTANTSET)
    ;
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *)tpointseq_speed((TSequence *)temp);
  else /* temp->temptype == SEQUENCESET */
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
 * instant set type
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
  TInstantSet *tix = tinstantset_make_free(instantsx, ti->count, MERGE_NO);
  TInstantSet *tiy = tinstantset_make_free(instantsy, ti->count, MERGE_NO);
  TInstantSet *tiz = NULL; /* keep compiler quiet */
  if (hasz)
    tiz = tinstantset_make_free(instantsz, ti->count, MERGE_NO);
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
 * sequence type
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
 * sequence set type
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
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_value_copy((TInstant *)temp);
  else if (temp->temptype == INSTANTSET)
    result = tgeompointi_twcentroid((TInstantSet *)temp);
  else if (temp->temptype == SEQUENCE)
    result = tgeompointseq_twcentroid((TSequence *)temp);
  else /* temp->temptype == SEQUENCESET */
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
 * sequence type
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
 * of sequence type
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
 * of sequence set type
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
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT || temp->temptype == INSTANTSET ||
    (temp->temptype == SEQUENCE && ! MOBDB_FLAGS_GET_LINEAR(temp->flags)) ||
    (temp->temptype == SEQUENCESET && ! MOBDB_FLAGS_GET_LINEAR(temp->flags)))
    ;
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *)tpointseq_azimuth((TSequence *)temp);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *)tpointseqset_azimuth((TSequenceSet *)temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Find stationary segments or pairs of segments of a temporal point sequence
 * that spatially intersect
 *
 * The function uses a sweepline algorithm to find the intersections in a time
 * a bit higher than n logn. This algorithm is a variation of Algorithm 2.1 in
 * "BENTLEY, Algorithms for Reporting and Counting Geometric Intersections, 1979".
 * The main idea of the algorithm is that two segments can only intersect if
 * their x-ranges intersect. It works as follows:
 * 1) Extract from every segment its x-range [min(p1.x, p2.x), max(p1.x, p2.x)]
 * 2) Sort the ranges increasingly by their first component, the min x.
 * 3) Iteratively intersect every x-range with the ranges after it, until it
 *    doesn't overlap.
 * 4) For the x-ranges that overlap, compute the actual intersection of their
 *    spatial lines, and report the pairs that indeed intersect, except the
 *    cases mentioned above.
 *
 * @param[in] seq Temporal point
 * @param[out] count Number of elements in the resulting array
 */
static double2 *
tgeompointseq_find_intersections(const TSequence *seq, int *count)
{
  bool hasz = MOBDB_FLAGS_GET_Z(seq->flags);
  /* We need to use palloc0 to set a 0 the z coordinates for 2D points */
  POINT3DZ *points = palloc0(sizeof(POINT3DZ) * seq->count);
  /* xranges is an array of double3 where, for each segment, a and b keep
   * the minimum and maximum value of x and c keeps the segment number.
   * A segment i in [0, seq->count - 1] is a pair of consecutive instants from seq */
  double3 *xranges = palloc(sizeof(double3) * (seq->count - 1));
  /* result is an array of double2 where a and b keep the segment number of
   * two segments that intersect. This array is expanded dynamically since
   * the number of intersections can be exponential. */
  double2 *result = palloc(sizeof(double2) * seq->count);
  int k = 0; /* number of elements in the output array */
  int l = 0; /* number of elements filled in the xranges array */
  if (hasz) /* 3D */
    points[0] = datum_get_point3dz(tinstant_value(tsequence_inst_n(seq, 0)));
  else
  {
    POINT2D point2D = datum_get_point2d(tinstant_value(tsequence_inst_n(seq, 0)));
    points[0].x = point2D.x;
    points[0].y = point2D.y;
  }
  for (int i = 1; i < seq->count; i++)
  {
    if (hasz) /* 3D */
      points[i] = datum_get_point3dz(tinstant_value(tsequence_inst_n(seq, i)));
    else
    {
      POINT2D point2D = datum_get_point2d(tinstant_value(tsequence_inst_n(seq, i)));
      points[i].x = point2D.x;
      points[i].y = point2D.y;
    }
    /* If stationary segment we need to split the sequence */
    if (points[i-1].x == points[i].x && points[i-1].y == points[i].y &&
      points[i-1].z == points[i].z)
    {
      /* If it is not the last segment */
      if (i < seq->count - 1)
        double2_set(&result[k++], i, i - 1);
      else
        double2_set(&result[k++], i - 1, i - 2);
      /* We are sure that the number of stationary segments will always be
       * less than the size of the result array (that is, seq->count) and thus
       * we do need to test whether the result array should be expanded */
    }
    else
      double3_set(&xranges[l++], Min(points[i-1].x, points[i].x),
        Max(points[i-1].x, points[i].x), i - 1);
  }
  /* If there are at least 2 non-stationary segments */
  if (l > 1)
  {
    double3arr_sort(xranges, l);
    for (int i = 0; i < l; i++)
    {
      for (int j = i + 1; j < l; j++)
      {
        /* If the segments overlap on x */
        if (xranges[i].a <= xranges[j].b && xranges[j].a <= xranges[i].b)
        {
          int seg1 = xranges[i].c;
          int seg2 = xranges[j].c;
          /* If the segments overlap on y */
          bool yoverlaps =
            Min(points[seg1].y, points[seg1 + 1].y) <=
              Max(points[seg2].y, points[seg2 + 1].y) &&
            Min(points[seg2].y, points[seg2 + 1].y) <=
              Max(points[seg1].y, points[seg1 + 1].y);
          bool zoverlaps = false;
          if (hasz)
            zoverlaps =
              Min(points[seg1].z, points[seg1 + 1].z) <=
                Max(points[seg2].z, points[seg2 + 1].z) &&
              Min(points[seg2].z, points[seg2 + 1].z) <=
                Max(points[seg1].z, points[seg1 + 1].z);
          if (!yoverlaps || (hasz && !zoverlaps))
            continue;
          /* Candidate for intersection
           * We are sure that the candidate segments are not stationary */
          Datum value1 = tinstant_value(tsequence_inst_n(seq, seg1));
          Datum value2 = tinstant_value(tsequence_inst_n(seq, seg1 + 1));
          Datum traj1 = geopoint_line(value1, value2);
          value1 = tinstant_value(tsequence_inst_n(seq, seg2));
          value2 = tinstant_value(tsequence_inst_n(seq, seg2 + 1));
          Datum traj2 = geopoint_line(value1, value2);
          Datum inter = call_function2(intersection, traj1, traj2);
          GSERIALIZED *gsinter = (GSERIALIZED *)PG_DETOAST_DATUM(inter);
          int intertype = gserialized_get_type(gsinter);
          bool isempty = gserialized_is_empty(gsinter);
          POSTGIS_FREE_IF_COPY_P(gsinter, DatumGetPointer(inter));
          pfree(DatumGetPointer(inter));
          pfree(DatumGetPointer(traj1)); pfree(DatumGetPointer(traj2));
          /* If the segments do not intersect or
           * the segments are consecutive and intersect in a point */
          if (isempty || (abs(seg1 - seg2) == 1 && intertype == POINTTYPE))
          {
            continue;
          }
          /* Output seg1 and seg2 in reverse order */
          double2_set(&result[k++], Max((double)seg1, (double)seg2),
            Min((double)seg1, (double)seg2));
          if (k % seq->count == 0)
          {
            /* Expand array of the result */
            double2 *tempresult = palloc0(sizeof(double2) * (k + seq->count));
            memcpy(tempresult, result, sizeof(double2) * k);
            pfree(result);
            result = tempresult;
          }
        }
        else
          break;
      }
    }
  }
  /* Sort the intersections to get the earliest intersecting segment at the
   * beginning */
  double2arr_sort(result, k);
  pfree(xranges); pfree(points);
  *count = k;
  return result;
}

/*****************************************************************************
 * Functions for testing whether a temporal point is simple and for spliting
 * a temporal point into an array of temporal points that are simple.
 * A temporal point is simple if all its components are non self-intersecting.
 * - a temporal instant point is simple
 * - a temporal instant set point is simple if it is non self-intersecting
 * - a temporal sequence point is simple if it is non self-intersecting and
 *   do not have stationary segments
 * - a temporal sequence set point is simple if every composing sequence is
 *   simple even if two composing sequences intersect
 *****************************************************************************/

/**
 * Determine whether a temporal point is self-intersecting
 *
 * @param[in] ti Temporal point
 */
static int
tgeompointi_is_simple(const TInstantSet *ti)
{
  if (ti->count == 1)
    return true;

  Datum *points = palloc(sizeof(Datum) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    TInstant *inst = tinstantset_inst_n(ti, i);
    points[i] = tinstant_value(inst);
  }
  datumarr_sort(points, ti->count, ti->valuetypid);
  bool found = false;
  for (int i = 1; i < ti->count; i++)
  {
    if (datum_eq(points[i - 1], points[i], ti->valuetypid))
    {
      found = true;
      break;
    }
  }
  pfree(points);
  return ! found;
}

/**
 * Determine whether a temporal point is self-intersecting
 *
 * @param[in] seq Temporal point
 * @pre The sequence has more than two instants
 */
static bool
tgeompointseq_step_is_simple(const TSequence *seq)
{
  assert(seq->count > 2);
  Datum *points = palloc(sizeof(Datum) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = tsequence_inst_n(seq, i);
    points[i] = tinstant_value(inst);
  }
  datumarr_sort(points, seq->count, seq->valuetypid);
  bool found = false;
  for (int i = 1; i < seq->count; i++)
  {
    if (datum_eq(points[i - 1], points[i], seq->valuetypid))
    {
      found = true;
      break;
    }
  }
  pfree(points);
  return ! found;
}

/**
 * Determine whether a temporal point is self-intersecting
 *
 * @param[in] seq Temporal point
 */
static bool
tgeompointseq_is_simple(const TSequence *seq)
{
  if (seq->count <= 2)
    return true;

  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  /* Stepwise interpolation */
  if (! linear)
    return tgeompointseq_step_is_simple(seq);

  /* Linear interpolation */
  int countinter;
  double2 *intersections = tgeompointseq_find_intersections(seq, &countinter);
  pfree(intersections);
  return countinter == 0;
}

/**
 * Determine whether a temporal point is self-intersecting
 *
 * @param[in] ts Temporal point
 */
static bool
tgeompoints_is_simple(const TSequenceSet *ts)
{
  bool result = true;
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    result &= tgeompointseq_is_simple(seq);
    if (! result)
      break;
  }
  return result;
}

PG_FUNCTION_INFO_V1(tgeompoint_is_simple);
/**
 * Determine whether a temporal point is self-intersecting
 */
PGDLLEXPORT Datum
tgeompoint_is_simple(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  bool result;
  if (temp->temptype == INSTANT)
    result = true;
  else if (temp->temptype == INSTANTSET)
    result = tgeompointi_is_simple((TInstantSet *)temp);
  else if (temp->temptype == SEQUENCE)
    result = tgeompointseq_is_simple((TSequence *)temp);
  else /* temp->temptype == SEQUENCESET */
    result = tgeompoints_is_simple((TSequenceSet *)temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/

/**
 * Split a temporal point into an array of non self-intersecting pieces
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * instant sets are stored
 * @param[in] ti Temporal point
 */
static int
tgeompointi_make_simple1(TInstantSet **result, const TInstantSet *ti)
{
  TInstant **instants = palloc(sizeof(TInstant) * ti->count);
  bool hasz = MOBDB_FLAGS_GET_Z(ti->flags);
  POINT3DZ *points = palloc0(sizeof(POINT3DZ) * ti->count);
  int count = 0, k = 1;
  TInstant *inst = instants[0] = tinstantset_inst_n(ti, 0);
  POINT3DZ p3dz; POINT2D p2d;
  if (hasz) /* 3D */
    points[0] = datum_get_point3dz(tinstant_value(inst));
  else
  {
    p2d = datum_get_point2d(tinstant_value(inst));
    points[0].x = p2d.x;
    points[0].y = p2d.y;
  }
  for (int i = 1; i < ti->count; i++)
  {
    inst = tinstantset_inst_n(ti, i);
    if (hasz) /* 3D */
      p3dz = datum_get_point3dz(tinstant_value(inst));
    else
    {
      p2d = datum_get_point2d(tinstant_value(inst));
      p3dz.x = p2d.x;
      p3dz.y = p2d.y;
      p3dz.z = 0;
    }
    bool found = false;
    for (int j = 0; j < k; j++)
    {
      if (p3dz.x == points[j].x && p3dz.y == points[j].y &&
        p3dz.z == points[j].z)
      {
        found = true;
        break;
      }
    }
    if (!found)
    {
      points[k] = p3dz;
      instants[k++] = inst;
    }
    else
    {
      result[count++] = tinstantset_make(instants, k, MERGE_NO);
      points[0] = p3dz;
      k = 1;
    }
  }
  result[count++] = tinstantset_make(instants, k, MERGE_NO);
  pfree(points);
  return count;
}

/**
 * Split a temporal point into an array of non self-intersecting pieces
 *
 * @param[in] ti Temporal point
 */
static ArrayType *
tgeompointi_make_simple(const TInstantSet *ti)
{
  /* Test whether the temporal point is simple */
  if (tgeompointi_is_simple(ti))
    return temporalarr_to_array((Temporal **)(&ti), 1);

  TInstantSet **instantsets = palloc(sizeof(TInstantSet *) * ti->count);
  int count = tgeompointi_make_simple1(instantsets, ti);
  ArrayType *result = temporalarr_to_array((Temporal **)instantsets, count);
  for (int i = 0; i < count; i++)
    pfree(instantsets[i]);
  pfree(instantsets);
  return result;
}

/**
 * Split a temporal point into an array of non self-intersecting pieces
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq Temporal point
 */
int
tgeompointseq_make_simple1(TSequence **result, const TSequence *seq)
{
  /* Special case when the input sequence has 1 or 2 instants */
  if (seq->count <= 2)
  {
    result[0] = tsequence_copy(seq);
    return 1;
  }

  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  /* Compute the non self-intersecting pieces of the sequence */
  int countinter;
  double2 *intersections = tgeompointseq_find_intersections(seq, &countinter);
  int k = 0, start = 0, curr = 0;
  bool lower_inc1;
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  while (curr < countinter)
  {
    /* Construct piece from start to intersections[0]->a; */
    lower_inc1 = (start == 0) ? seq->period.lower_inc : true;
    bool upper_inc1 = (((int)intersections[0].a) == seq->count - 1) ?
      seq->period.upper_inc : false;
    int j = 0;
    for (int i = start; i <= intersections[0].a; i++)
      instants[j++] = tsequence_inst_n(seq, i);
    /* The last two values of sequences with step interpolation and
     * exclusive upper bound must be equal */
    if (! linear)
    {
      Datum value = tinstant_value(instants[j - 2]);
      TimestampTz t = tsequence_inst_n(seq, intersections[0].a)->t;
      instants[j - 1] = tinstant_make(value, t, seq->valuetypid);
    }
    result[k++] = tsequence_make(instants, j, lower_inc1, upper_inc1,
      linear, NORMALIZE_NO);
    if (! linear)
      /* Free the last instant created for the step interpolation */
      pfree(instants[j - 1]);
    /* Save the initial segment of the next trajectory */
    start = intersections[0].a;
    /* Remove from intersections all entries that overlap the range
       from start to intersections[0].a - 1 */
    j = 0;
    for (int i = 0; i < countinter; i++)
    {
      if (intersections[i].a >= start && intersections[i].b >= start)
      {
        intersections[j++] = intersections[i];
      }
    }
    countinter = j;
  }
  /* Construct trajectory from start to end of instants */
  lower_inc1 = (start == 0) ? seq->period.lower_inc : true;
  for (int i = 0; i < seq->count - start; i++)
    instants[i] = tsequence_inst_n(seq, start + i);
  result[k++] = tsequence_make(instants, seq->count - start,
    lower_inc1, seq->period.upper_inc, linear, NORMALIZE_NO);
  pfree(intersections);
  return k;
}

/**
 * Split a temporal point into an array of non self-intersecting pieces
 *
 * @param[in] seq Temporal point
 */
static ArrayType *
tgeompointseq_make_simple(const TSequence *seq)
{
  /* Special case when the input sequence has 1 or 2 instants */
  if (seq->count <= 2)
    return temporalarr_to_array((Temporal **)(&seq), 1);

  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count - 1);
  int count = tgeompointseq_make_simple1(sequences, seq);
  ArrayType *result = temporalarr_to_array((Temporal **)sequences, count);
  for (int i = 0; i < count; i++)
    pfree(sequences[i]);
  pfree(sequences);
  return result;
}

/**
 * Split a temporal point into an array of non self-intersecting pieces
 *
 * @param[in] ts Temporal point
 */
static ArrayType *
tgeompoints_make_simple(const TSequenceSet *ts)
{
  /* Test whether the temporal point is simple */
  if (tgeompoints_is_simple(ts))
    return temporalarr_to_array((Temporal **)(&ts), 1);

  TSequence **sequences = palloc(sizeof(TSequence *) *
    ts->totalcount - ts->count);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tgeompointseq_make_simple1(&sequences[k], seq);
  }
  ArrayType *result = temporalarr_to_array((Temporal **)sequences, k);
  pfree(sequences);
  return result;
}

PG_FUNCTION_INFO_V1(tgeompoint_make_simple);
/**
 * Split a temporal point into an array of non self-intersecting pieces
 */
PGDLLEXPORT Datum
tgeompoint_make_simple(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ArrayType *result;
  if (temp->temptype == INSTANT)
    result = temporalarr_to_array(&temp, 1);
  else if (temp->temptype == INSTANTSET)
    result = tgeompointi_make_simple((TInstantSet *)temp);
  else if (temp->temptype == SEQUENCE)
    result = tgeompointseq_make_simple((TSequence *)temp);
  else /* temp->temptype == SEQUENCESET */
    result = tgeompoints_make_simple((TSequenceSet *)temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Restriction functions
 * N.B. In the PostGIS version currently used by MobilityDB (2.5) there is no
 * true ST_Intersection function for geography
 *****************************************************************************/

/**
 * Restricts the temporal instant point to the (complement of the) geometry
 */
static TInstant *
tpointinst_restrict_geometry(const TInstant *inst, Datum geom, bool atfunc)
{
  bool inter = DatumGetBool(call_function2(intersects, tinstant_value(inst), geom));
  if ((atfunc && !inter) || (!atfunc && inter))
    return NULL;
  return tinstant_copy(inst);
}

/**
 * Restricts the temporal instant set point to the (complement of the) geometry
 */
static TInstantSet *
tpointinstset_restrict_geometry(const TInstantSet *ti, Datum geom, bool atfunc)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int k = 0;
  for (int i = 0; i < ti->count; i++)
  {
    TInstant *inst = tinstantset_inst_n(ti, i);
    bool inter = DatumGetBool(call_function2(intersects, tinstant_value(inst), geom));
    if ((atfunc && inter) || (!atfunc && !inter))
      instants[k++] = inst;
  }
  TInstantSet *result = NULL;
  if (k != 0)
    result = tinstantset_make(instants, k, MERGE_NO);
  /* We cannot pfree the instants in the array */
  pfree(instants);
  return result;
}

/**
 * Restricts the temporal sequence point with step interpolation to the geometry
 *
 * @param[in] seq Temporal point
 * @param[in] gsinter Intersection of the temporal point and the geometry
 * @param[out] count Number of elements in the resulting array
 * @pre The temporal point is simple (that is, not self-intersecting)
 */
TSequence **
tpointseq_step_at_geometry(const TSequence *seq, GSERIALIZED *gsinter, int *count)
{
  /* Temporal sequence has at least 2 instants. Indeed, the test for
   * instantaneous full sequence was done in function tpointseq_at_geometry.
   * Furthermore, the simple components of a self-intersecting sequence
   * have at least two instants */

  LWGEOM *lwgeom_inter = lwgeom_from_gserialized(gsinter);
  int type = lwgeom_inter->type;
  int countinter;
  LWPOINT *lwpoint_inter;
  LWCOLLECTION *coll;
  if (type == POINTTYPE)
  {
    countinter = 1;
    lwpoint_inter = lwgeom_as_lwpoint(lwgeom_inter);
  }
  else
  /* It is a collection of type MULTIPOINTTYPE */
  {
    coll = lwgeom_as_lwcollection(lwgeom_inter);
    countinter = coll->ngeoms;
  }
  Datum *points = palloc(sizeof(Datum) * countinter);
  for (int i = 0; i < countinter; i++)
  {
    if (countinter > 1)
    {
      /* Find the i-th intersection */
      LWGEOM *subgeom = coll->geoms[i];
      lwpoint_inter = lwgeom_as_lwpoint(subgeom);
    }
    points[i] = PointerGetDatum(geo_serialize((LWGEOM *)lwpoint_inter));
  }
  /* Due to linear interpolation the maximum number of resulting sequences
   * is at most seq->count */
  TSequence **result = palloc(sizeof(TSequence *) * seq->count);
  int newcount = tsequence_at_values1(result, seq, points, countinter);

  for (int i = 0; i < countinter; i++)
    pfree(DatumGetPointer(points[i]));
  pfree(points);
  lwgeom_free(lwgeom_inter);

  *count = newcount;
  return result;
}

/**
 * Returns the timestamp at which the segment of the temporal value takes
 * the base value
 *
 * This function must take into account the roundoff errors and thus it uses
 * the datum_point_eq_eps for comparing two values so the coordinates of the 
 * values may differ by EPSILON. 
 *
 * @param[in] inst1,inst2 Temporal values
 * @param[in] value Base value
 * @param[out] projvalue Base value that is projected on the segment 
 * @param[out] t Timestamp
 * @result Returns true if the base value is found in the temporal value
 * @pre The segment is not constant and has linear interpolation
 * @note The resulting timestamp may be at an exclusive bound.
 */
static bool
tgeompointseq_timestamp_at_value1(const TInstant *inst1, const TInstant *inst2, 
  Datum value, Datum *projvalue, TimestampTz *t)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  /* Is the lower bound the answer? */
  if (datum_point_eq(value1, value))
  {
    *projvalue = tinstant_value_copy(inst1);
    *t = inst1->t;
    return true;
  }
  /* Is the upper bound the answer? */
  if (datum_point_eq(value2, value))
  {
    *projvalue = tinstant_value_copy(inst2);
    *t = inst2->t;
    return true;
  }
  /* For linear interpolation and not constant segment is the
   * value in the interior of the segment? */
  if (! tpointseq_intersection_value(inst1, inst2, value, t))
    return false;
  *projvalue = tsequence_value_at_timestamp1(inst1, inst2, true, *t);
  return true;
}

/**
 * Returns the timestamp at which the temporal value takes the base value.
 *
 * This function is called by the atGeometry function to find the timestamp 
 * at which an intersection point found by PostGIS is located.
 *
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[out] projvalue Base value that is projected on the temporal value 
 * @param[out] t Timestamp
 * @result Returns true if the base value is found in the temporal value
 * @pre The base value is known to belong to the temporal value (taking into
 * account roundoff errors) and the temporal value has linear interpolation
 * @note The resulting timestamp may be at an exclusive bound.
 */
static bool
tgeompointseq_timestamp_at_value(const TSequence *seq, Datum value, 
  Datum *projvalue, TimestampTz *t)
{
  TInstant *inst1;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = tsequence_inst_n(seq, 0);
    if (! datum_point_eq(tinstant_value(inst1), value))
      return false;
    *projvalue = tinstant_value_copy(inst1);
    *t = inst1->t;
    return true;
  }

  /* Bounding box test */
  if (! temporal_bbox_restrict_value((Temporal *)seq, value))
    return false;

  /* General case */
  inst1 = tsequence_inst_n(seq, 0);
  for (int i = 1; i < seq->count; i++)
  {
    TInstant *inst2 = tsequence_inst_n(seq, i);
    if (tgeompointseq_timestamp_at_value1(inst1, inst2, value, projvalue, t))
      return true;
    inst1 = inst2;
  }
  return false;
}

/**
 * Restricts the temporal sequence point with linear interpolation to the geometry
 *
 * @param[in] seq Temporal point
 * @param[in] traj Trajectory of the temporal point
 * @param[in] gsinter Intersection of the temporal point and the geometry
 * @param[out] count Number of elements in the resulting array
 * @pre The temporal point is simple, that is, non self-intersecting
 */
TSequence **
tpointseq_linear_at_geometry(const TSequence *seq, Datum traj,
  GSERIALIZED *gsinter, int *count)
{
  /* Temporal sequence has at least 2 instants. Indeed, the test for
   * instantaneous full sequence was done in function tpointseq_at_geometry.
   * Furthermore, the simple components of a self-intersecting sequence
   * have at least two instants */

  TInstant *start = tsequence_inst_n(seq, 0);
  TInstant *end = tsequence_inst_n(seq, seq->count - 1);
  TSequence **result;

  /* If the trajectory is a point */
  if (seq->count == 2 &&
    datum_eq(tinstant_value(start), tinstant_value(end), seq->valuetypid))
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_copy(seq);
    *count = 1;
    return result;
  }

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
  /* It is a collection of type MULTIPOINTTYPE, MULTILINETYPE, or
   * COLLECTIONTYPE */
  {
    coll = lwgeom_as_lwcollection(lwgeom_inter);
    countinter = coll->ngeoms;
  }
  TSequence **sequences = palloc(sizeof(TSequence *) * countinter);
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
      type = subgeom->type;
    }
    TimestampTz t1, t2;
    Datum projpoint1, projpoint2;
    GSERIALIZED *gspoint;
    TInstant *inst;
    /* Each intersection is either a point or a linestring */
    if (type == POINTTYPE)
    {
      gspoint = geo_serialize((LWGEOM *)lwpoint_inter);
      if (! tgeompointseq_timestamp_at_value(seq, PointerGetDatum(gspoint), &projpoint1, &t1))
        /* We should never arrive here */
        elog(ERROR, "The value has not been found due to roundoff errors");
      pfree(gspoint);
      /* If the intersection is not at an exclusive bound */
      if ((seq->period.lower_inc || t1 > start->t) &&
        (seq->period.upper_inc || t1 < end->t))
      {
        inst = tinstant_make( projpoint1, t1, start->valuetypid);
        sequences[k++] = tinstant_to_tsequence(inst, LINEAR);
        pfree(DatumGetPointer( projpoint1)); pfree(inst);
      }
    }
    else
    {
      /* Get the fraction of the start point of the intersecting line */
      LWPOINT *lwpoint = lwline_get_lwpoint(lwline_inter, 0);
      gspoint = geo_serialize((LWGEOM *)lwpoint);
      if (! tgeompointseq_timestamp_at_value(seq, PointerGetDatum(gspoint), &projpoint1, &t1))
        /* We should never arrive here */
        elog(ERROR, "The value has not been found due to roundoff errors");
      pfree(gspoint);
      /* Get the fraction of the end point of the intersecting line */
      lwpoint = lwline_get_lwpoint(lwline_inter, lwline_inter->points->npoints - 1);
      gspoint = geo_serialize((LWGEOM *)lwpoint);
      if (! tgeompointseq_timestamp_at_value(seq, PointerGetDatum(gspoint), &projpoint2, &t2))
        /* We should never arrive here */
        elog(ERROR, "The value has not been found due to roundoff errors");
      pfree(gspoint); 
      /* If t1 == t2 and the intersection is not at an exclusive bound */
      if (t1 == t2 && (seq->period.lower_inc || t1 > start->t) &&
        (seq->period.upper_inc || t1 < end->t))
      {
        inst = tinstant_make(projpoint1, t1, start->valuetypid);
        sequences[k++] = tinstant_to_tsequence(inst, LINEAR);
        pfree(inst);
      }
      else
      {
        TimestampTz lower1 = Min(t1, t2);
        TimestampTz upper1 = Max(t1, t2);
        bool lower_inc1 = (lower1 == start->t) ? seq->period.lower_inc : true;
        bool upper_inc1 = (upper1 == end->t) ? seq->period.upper_inc : true;
        Period p;
        period_set(&p, lower1, upper1, lower_inc1, upper_inc1);
        sequences[k++] = tsequence_at_period(seq, &p);
      }
      pfree(DatumGetPointer(projpoint1)); pfree(DatumGetPointer(projpoint2));
    }
  }
  lwgeom_free(lwgeom_inter);

  if (k == 0)
  {
    pfree(sequences);
    *count = 0;
    return NULL;
  }
  else if (k == 1)
  {
    *count = 1;
    return sequences;
  }
  else /* (k > 1) */
  {
    /*
     * Since the result of st_intersection is not necessarily minimal, as in
     * the example below (as of PostGIS 2.5.5)
     *    select st_intersection(geometry 'LINESTRING(2 2,2 1,1 1)',
     *      'Polygon((0 0,0 2,2 2,2 0,0 0))'));
     *    -- MULTILINESTRING((2 2,2 1),(2 1,1 1))
     * we need to merge the resulting sequences since they may overlap on a
     * single instant.
     */
    tsequencearr_sort(sequences, k);
    result = tsequence_merge_array1(sequences, k, count);
    for (int i = 0; i < k; i++)
      pfree(sequences[i]);
    pfree(sequences);
    return result;
  }
}

/**
 * Restricts the temporal sequence point to the geometry.
 *
 * The function splits the temporal point in an array of temporal
 * points that are simple (that is, not self-intersecting) and
 * call the function tpointseq_at_geometry2 for each piece.
 * @param[in] seq Temporal point
 * @param[in] geom Geometry
 * @param[out] count Number of elements in the resulting array
 * @pre The temporal point is simple, that is, not self-intersecting
 */
TSequence **
tpointseq_at_geometry(const TSequence *seq, Datum geom, int *count)
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
  /* Split the temporal point in an array of non self-intersecting
   * temporal points */
  TSequence **simpleseqs = palloc(sizeof(TSequence *) * seq->count - 1);
  int countsimple = tgeompointseq_make_simple1(simpleseqs, seq);
  /* Allocate memory for the result */
  TSequence ***sequences = palloc(sizeof(TSequence *) * countsimple);
  int *countseqs = palloc0(sizeof(int) * (seq->count - 1));
  int totalseqs = 0;
  /* Loop for every simple piece of the trip */
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  for (int i = 0; i < countsimple; i++)
  {
    Datum traj = tpointseq_trajectory(simpleseqs[i]);
    Datum inter = call_function2(intersection, traj, geom);
    GSERIALIZED *gsinter = (GSERIALIZED *) PG_DETOAST_DATUM(inter);
    if (! gserialized_is_empty(gsinter))
    {
      sequences[i] = linear ?
        tpointseq_linear_at_geometry(simpleseqs[i], traj, gsinter, &countseqs[i]) :
        tpointseq_step_at_geometry(simpleseqs[i], gsinter, &countseqs[i]);
      totalseqs += countseqs[i];
    }
    pfree(DatumGetPointer(inter));
    POSTGIS_FREE_IF_COPY_P(gsinter, DatumGetPointer(gsinter));
  }
  *count = totalseqs;
  if (totalseqs == 0)
  {
    pfree(sequences); pfree(countseqs);
    return NULL;
  }
  TSequence **result = tsequencearr2_to_tsequencearr(sequences, countseqs,
    countsimple, totalseqs);
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
  TSequence **sequences = tpointseq_at_geometry(seq, geom, &countinter);
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
 * Restricts the temporal sequence point to the (complement of the) geometry
 */
static TSequenceSet *
tpointseq_restrict_geometry(const TSequence *seq, Datum geom, bool atfunc)
{
  int count;
  TSequence **sequences = atfunc ? tpointseq_at_geometry(seq, geom, &count) :
    tpointseq_minus_geometry1(seq, geom, &count);
  if (sequences == NULL)
    return NULL;

  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Restricts the temporal sequence set point to the (complement of the) geometry
 *
 * @param[in] ts Temporal point
 * @param[in] geom Geometry
 * @param[in] box Bounding box of the temporal point
 * @param[in] atfunc True when the restriction is at, false for minus
 */
static TSequenceSet *
tpointseqset_restrict_geometry(const TSequenceSet *ts, Datum geom,
  const STBOX *box, bool atfunc)
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tpointseq_restrict_geometry(tsequenceset_seq_n(ts, 0), geom, atfunc);

  /* palloc0 used due to the bounding box test in the for loop below */
  TSequence ***sequences = palloc0(sizeof(TSequence *) * ts->count);
  int *countseqs = palloc0(sizeof(int) * ts->count);
  int totalseqs = 0;
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    /* Bounding box test */
    STBOX *box1 = tsequence_bbox_ptr(seq);
    bool overlaps = overlaps_stbox_stbox_internal(box1, box);
    if (atfunc)
    {
      if (overlaps)
      {
        sequences[i] = tpointseq_at_geometry(seq, geom,
          &countseqs[i]);
        totalseqs += countseqs[i];
      }
    }
    else
    {
      if (!overlaps)
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
 * Restricts the temporal point to the (complement of the) geometry
 * (dispatch function)
 *
 * @pre The arguments are of the same dimensionality, have the same SRID,
 * and the geometry is not empty
 */
Temporal *
tpoint_restrict_geometry_internal(const Temporal *temp, Datum geom, bool atfunc)
{
  /* Bounding box test */
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  /* Non-empty geometries have a bounding box */
  assert(geo_to_stbox_internal(&box2, (GSERIALIZED *) DatumGetPointer(geom)));
  if (!overlaps_stbox_stbox_internal(&box1, &box2))
    return atfunc ? NULL : temporal_copy(temp);

  Temporal *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = (Temporal *)tpointinst_restrict_geometry((TInstant *)temp, geom, atfunc);
  else if (temp->temptype == INSTANTSET)
    result = (Temporal *)tpointinstset_restrict_geometry((TInstantSet *)temp, geom, atfunc);
  else if (temp->temptype == SEQUENCE)
    result = (Temporal *)tpointseq_restrict_geometry((TSequence *)temp, geom, atfunc);
  else /* temp->temptype == SEQUENCESET */
    result = (Temporal *)tpointseqset_restrict_geometry((TSequenceSet *)temp, geom, &box2, atfunc);

  return result;
}

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
  Temporal *result = tpoint_restrict_geometry_internal(temp,
    PointerGetDatum(gs), atfunc);
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
    result = tpoint_restrict_geometry_internal(temp1, geom1, REST_AT);
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
  ensure_common_dimension(temp->flags, box->flags);
  if (MOBDB_FLAGS_GET_X(box->flags))
  {
    ensure_same_spatial_dimensionality(temp->flags, box->flags);
    ensure_same_geodetic(temp->flags, box->flags);
    ensure_same_srid_tpoint_stbox(temp, box);
  }
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

/*****************************************************************************/
