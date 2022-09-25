/***********************************************************************
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
 * @brief Spatial functions for temporal points.
 */

#include "point/tpoint_spatialfuncs.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <utils/float.h>
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
#include <lwgeodetic.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_call.h"
#include "general/lifting.h"
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "general/tnumber_mathfuncs.h"
#include "point/pgis_call.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_spatialrels.h"

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
 * The first variant (e.g. datum_point2d) is slower than the second (e.g.
 * datum_point2d_p) since the point is passed by value and thus the bytes
 * are copied. The second version is declared const because you aren't allowed
 * to modify the values, only read them.
 */

#define LWFLAG_EXTFLAGS    0x20
#define LWFLAG_VERSBIT2    0x80

#define FLAGS_GET_EXTFLAGS(flags)     (((flags) & LWFLAG_EXTFLAGS)>>5)
#define FLAGS_GET_VERSBIT2(flags)     (((flags) & LWFLAG_VERSBIT2)>>7)

#define GS_POINT_PTR(gs)    ( (uint8_t *) gs->data + 8 + \
  FLAGS_GET_BBOX(gs->gflags) * FLAGS_NDIMS_BOX(gs->gflags) * 8 + \
  FLAGS_GET_VERSBIT2(gs->gflags) * 8 )

/**
 * Return a 2D point from the datum
 */
POINT2D
datum_point2d(Datum geom)
{
  const GSERIALIZED *gs = DatumGetGserializedP(geom);
  POINT2D *point = (POINT2D *) GS_POINT_PTR(gs);
  return *point;
}

/**
 * Return a 3DZ point from the datum
 */
POINT3DZ
datum_point3dz(Datum geom)
{
  const GSERIALIZED *gs = DatumGetGserializedP(geom);
  POINT3DZ *point = (POINT3DZ *) GS_POINT_PTR(gs);
  return *point;
}

/**
 * Return a 4D point from the datum
 * @note The M dimension is ignored
 */
void
datum_point4d(Datum geom, POINT4D *p)
{
  const GSERIALIZED *gs = DatumGetGserializedP(geom);
  memset(p, 0, sizeof(POINT4D));
  if (FLAGS_GET_Z(gs->gflags))
  {
    POINT3DZ *point = (POINT3DZ *) GS_POINT_PTR(gs);
    p->x = point->x;
    p->y = point->y;
    p->z = point->z;
  }
  else
  {
    POINT2D *point = (POINT2D *) GS_POINT_PTR(gs);
    p->x = point->x;
    p->y = point->y;
  }
  return;
}

/*****************************************************************************/

/**
 * Return a pointer to a 2D point from the datum
 */
const POINT2D *
datum_point2d_p(Datum geom)
{
  const GSERIALIZED *gs = DatumGetGserializedP(geom);
  return (POINT2D *) GS_POINT_PTR(gs);
}

/**
 * Return a 2D point from the serialized geometry
 */
const POINT2D *
gserialized_point2d_p(const GSERIALIZED *gs)
{
  return (POINT2D *) GS_POINT_PTR(gs);
}

/**
 * Return a pointer to a 3DZ point from the datum
 */
const POINT3DZ *
datum_point3dz_p(Datum geom)
{
  const GSERIALIZED *gs = DatumGetGserializedP(geom);
  return (POINT3DZ *) GS_POINT_PTR(gs);
}

/**
 * Return a 3DZ point from the serialized geometry
 */
const POINT3DZ *
gserialized_point3dz_p(const GSERIALIZED *gs)
{
  return (POINT3DZ *) GS_POINT_PTR(gs);
}

/**
 * Return true if the points are equal
 */
bool
datum_point_eq(Datum geopoint1, Datum geopoint2)
{
  const GSERIALIZED *gs1 = DatumGetGserializedP(geopoint1);
  const GSERIALIZED *gs2 = DatumGetGserializedP(geopoint2);
  if (gserialized_get_srid(gs1) != gserialized_get_srid(gs2) ||
    FLAGS_GET_Z(gs1->gflags) != FLAGS_GET_Z(gs2->gflags) ||
    FLAGS_GET_GEODETIC(gs1->gflags) != FLAGS_GET_GEODETIC(gs2->gflags))
    return false;
  if (FLAGS_GET_Z(gs1->gflags))
  {
    const POINT3DZ *point1 = gserialized_point3dz_p(gs1);
    const POINT3DZ *point2 = gserialized_point3dz_p(gs2);
    return FP_EQUALS(point1->x, point2->x) && FP_EQUALS(point1->y, point2->y) &&
      FP_EQUALS(point1->z, point2->z);
  }
  else
  {
    const POINT2D *point1 = gserialized_point2d_p(gs1);
    const POINT2D *point2 = gserialized_point2d_p(gs2);
    return FP_EQUALS(point1->x, point2->x) && FP_EQUALS(point1->y, point2->y);
  }
}

/**
 * Return true if the points are equal
 */
Datum
datum2_point_eq(Datum geopoint1, Datum geopoint2)
{
  return BoolGetDatum(datum_point_eq(geopoint1, geopoint2));
}

/**
 * Return true if the points are equal
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
geo_serialize(const LWGEOM *geom)
{
  size_t size;
  GSERIALIZED *result = gserialized_from_lwgeom((LWGEOM *) geom, &size);
  SET_VARSIZE(result, size);
  return result;
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * Select the appropriate distance function
 */
datum_func2
distance_fn(int16 flags)
{
  datum_func2 result;
  if (MOBDB_FLAGS_GET_GEODETIC(flags))
    result = &geog_distance;
  else
    result = MOBDB_FLAGS_GET_Z(flags) ?
      &geom_distance3d : &geom_distance2d;
  return result;
}

/**
 * Select the appropriate distance function
 */
datum_func2
pt_distance_fn(int16 flags)
{
  datum_func2 result;
  if (MOBDB_FLAGS_GET_GEODETIC(flags))
    result = &geog_distance;
  else
    result = MOBDB_FLAGS_GET_Z(flags) ?
      &pt_distance3d : &pt_distance2d;
  return result;
}

/**
 * Return the 2D distance between the two geometries
 * @pre For PostGIS version > 3 the geometries are NOT toasted
 */
Datum
geom_distance2d(Datum geom1, Datum geom2)
{
  return Float8GetDatum(gserialized_distance(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2)));
}

/**
 * Return the 3D distance between the two geometries
 */
Datum
geom_distance3d(Datum geom1, Datum geom2)
{
  return Float8GetDatum(gserialized_3Ddistance(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2)));
}

/**
 * Return the distance between the two geographies
 */
Datum
geog_distance(Datum geog1, Datum geog2)
{
  return Float8GetDatum(gserialized_geog_distance(DatumGetGserializedP(geog1),
    DatumGetGserializedP(geog2)));
}

/**
 * Return the 2D distance between the two geometric points
 */
Datum
pt_distance2d(Datum geom1, Datum geom2)
{
  const POINT2D *p1 = datum_point2d_p(geom1);
  const POINT2D *p2 = datum_point2d_p(geom2);
  return Float8GetDatum(distance2d_pt_pt(p1, p2));
}

/**
 * Return the 3D distance between the two geometric points
 */
Datum
pt_distance3d(Datum geom1, Datum geom2)
{
  const POINT3DZ *p1 = datum_point3dz_p(geom1);
  const POINT3DZ *p2 = datum_point3dz_p(geom2);
  return Float8GetDatum(distance3d_pt_pt((POINT3D *) p1, (POINT3D *) p2));
}

/**
 * Return the 2D intersection between the two geometries
 */
Datum
geom_intersection2d(Datum geom1, Datum geom2)
{
  return PointerGetDatum(gserialized_intersection(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2)));
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
  if (tgeo_type(temp1->temptype))
  {
    ensure_same_srid(tpoint_srid(temp1), tpoint_srid(temp2));
    ensure_same_dimensionality(temp1->flags, temp2->flags);
  }
  return;
}

/**
 * Ensure that the spatiotemporal argument has planar coordinates
 */
void
ensure_not_geodetic(int16 flags)
{
  if (MOBDB_FLAGS_GET_GEODETIC(flags))
    elog(ERROR, "Only planar coordinates supported");
  return;
}

/**
 * Ensure that the spatiotemporal argument have the same type of coordinates,
 * either planar or geodetic
 */
void
ensure_same_geodetic(int16 flags1, int16 flags2)
{
  if (MOBDB_FLAGS_GET_X(flags1) && MOBDB_FLAGS_GET_X(flags2) &&
    MOBDB_FLAGS_GET_GEODETIC(flags1) != MOBDB_FLAGS_GET_GEODETIC(flags2))
    elog(ERROR, "Operation on mixed planar and geodetic coordinates");
  return;
}

/**
 * Ensure that the two spatial "objects" have the same SRID
 */
void
ensure_same_srid(int32_t srid1, int32_t srid2)
{
  if (srid1 != srid2)
    elog(ERROR, "Operation on mixed SRID");
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
    elog(ERROR, "Operation on mixed SRID");
  return;
}

/**
 * Ensure that a temporal point and a spatiotemporal boxes have the same SRID
 */
void
ensure_same_srid_tpoint_stbox(const Temporal *temp, const STBOX *box)
{
  if (MOBDB_FLAGS_GET_X(box->flags) &&
    tpoint_srid(temp) != box->srid)
    elog(ERROR, "Operation on mixed SRID");
  return;
}

/**
 * Ensure that a temporal point and a geometry/geography have the same SRID
 */
void
ensure_same_srid_stbox_gs(const STBOX *box, const GSERIALIZED *gs)
{
  if (box->srid != gserialized_get_srid(gs))
    elog(ERROR, "Operation on mixed SRID");
  return;
}

/**
 * Ensure that two temporal points have the same dimensionality as given by
 * their flags
 */
void
ensure_same_dimensionality(int16 flags1, int16 flags2)
{
  if (MOBDB_FLAGS_GET_X(flags1) != MOBDB_FLAGS_GET_X(flags2) ||
    MOBDB_FLAGS_GET_Z(flags1) != MOBDB_FLAGS_GET_Z(flags2) ||
    MOBDB_FLAGS_GET_T(flags1) != MOBDB_FLAGS_GET_T(flags2))
    elog(ERROR, "The arguments must be of the same dimensionality");
  return;
}

/**
 * Ensure that two temporal points have the same spatial dimensionality as
 * given by their flags
 */
void
ensure_same_spatial_dimensionality(int16 flags1, int16 flags2)
{
  if (MOBDB_FLAGS_GET_X(flags1) != MOBDB_FLAGS_GET_X(flags2) ||
    MOBDB_FLAGS_GET_Z(flags1) != MOBDB_FLAGS_GET_Z(flags2))
    elog(ERROR, "Operation on mixed 2D/3D dimensions");
  return;
}

/**
 * Ensure that a temporal point and a spatiotemporal box have the same spatial
 * dimensionality as given by their flags
 */
void
ensure_same_spatial_dimensionality_temp_box(int16 flags1, int16 flags2)
{
  if (MOBDB_FLAGS_GET_X(flags1) != MOBDB_FLAGS_GET_X(flags2) ||
      /* Geodetic boxes are always in 3D */
      (! MOBDB_FLAGS_GET_GEODETIC(flags2) &&
      MOBDB_FLAGS_GET_Z(flags1) != MOBDB_FLAGS_GET_Z(flags2)))
    elog(ERROR, "Operation on mixed 2D/3D dimensions");
  return;
}

/**
 * Ensure that two geometries/geographies have the same dimensionality
 */
void
ensure_same_dimensionality_gs(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  if (FLAGS_GET_Z(gs1->gflags) != FLAGS_GET_Z(gs2->gflags))
    elog(ERROR, "Operation on mixed 2D/3D dimensions");
  return;
}

/**
 * Ensure that a temporal point and a geometry/geography have the same dimensionality
 */
void
ensure_same_dimensionality_tpoint_gs(const Temporal *temp, const GSERIALIZED *gs)
{
  if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->gflags))
    elog(ERROR, "Operation on mixed 2D/3D dimensions");
  return;
}

/**
 * Ensure that the spatiotemporal boxes have the same spatial dimensionality
 */
void
ensure_same_spatial_dimensionality_stbox_gs(const STBOX *box, const GSERIALIZED *gs)
{
  if (! MOBDB_FLAGS_GET_X(box->flags) ||
      /* Geodetic boxes are always in 3D */
     (! MOBDB_FLAGS_GET_GEODETIC(box->flags) &&
        MOBDB_FLAGS_GET_Z(box->flags) != FLAGS_GET_Z(gs->gflags)))
    elog(ERROR, "The spatiotemporal box and the geometry must be of the same dimensionality");
  return;
}

/**
 * Ensure that a temporal point has Z dimension
 */
void
ensure_has_Z(int16 flags)
{
  if (! MOBDB_FLAGS_GET_Z(flags))
    elog(ERROR, "The temporal point must have Z dimension");
  return;
}

/**
 * Ensure that a temporal point has not Z dimension
 */
void
ensure_has_not_Z(int16 flags)
{
  if (MOBDB_FLAGS_GET_Z(flags))
    elog(ERROR, "The temporal point cannot have Z dimension");
  return;
}

/**
 * Ensure that the geometry/geography has not Z dimension
 */
void
ensure_has_Z_gs(const GSERIALIZED *gs)
{
  if (! FLAGS_GET_Z(gs->gflags))
    elog(ERROR, "The geometry must have Z dimension");
  return;
}

/**
 * Ensure that the geometry/geography has not Z dimension
 */
void
ensure_has_not_Z_gs(const GSERIALIZED *gs)
{
  if (FLAGS_GET_Z(gs->gflags))
    elog(ERROR, "The geometry cannot have Z dimension");
  return;
}

/**
 * Ensure that the geometry/geography has M dimension
 */
void
ensure_has_M_gs(const GSERIALIZED *gs)
{
  if (! FLAGS_GET_M(gs->gflags))
    elog(ERROR, "Only geometries with M dimension accepted");
  return;
}

/**
 * Ensure that the geometry/geography has not M dimension
 */
void
ensure_has_not_M_gs(const GSERIALIZED *gs)
{
  if (FLAGS_GET_M(gs->gflags))
    elog(ERROR, "Only geometries without M dimension accepted");
  return;
}

/**
 * Ensure that the geometry/geography is a point
 */
void
ensure_point_type(const GSERIALIZED *gs)
{
  if (gserialized_get_type(gs) != POINTTYPE)
    elog(ERROR, "Only point geometries accepted");
  return;
}

/**
 * Ensure that the geometry/geography is not empty
 */
void
ensure_non_empty(const GSERIALIZED *gs)
{
  if (gserialized_is_empty(gs))
    elog(ERROR, "Only non-empty geometries accepted");
  return;
}

/*****************************************************************************
 * Return true if a point is in a segment (2D, 3D, or geodetic).
 * For 2D/3D points we proceed as follows.
 * If the cross product of (B-A) and (p-A) is 0, then the points A, B,
 * and p are aligned. To know if p is between A and B, we also have to
 * check that the dot product of (B-A) and (p-A) is positive and is less
 * than the square of the distance between A and B.
 * https://stackoverflow.com/questions/328107/how-can-you-determine-a-point-is-between-two-other-points-on-a-line-segment
 *****************************************************************************/
/*
 * Return true if point p is in the segment defined by A and B (2D)
 * @note The test of p = A or p = B MUST BE done in the calling function
 *   to take care of the inclusive/exclusive bounds for temporal sequences
 */
static bool
point2d_on_segment(const POINT2D *p, const POINT2D *A, const POINT2D *B)
{
  double crossproduct = (p->y - A->y) * (B->x - A->x) -
    (p->x - A->x) * (B->y - A->y);
  if (fabs(crossproduct) >= MOBDB_EPSILON)
    return false;
  double dotproduct = (p->x - A->x) * (B->x - A->x) +
    (p->y - A->y) * (B->y - A->y);
  return (dotproduct >= 0);
}

/*
 * Return true if point p is in the segment defined by A and B (3D)
 * @note The test of p = A or p = B MUST BE done in the calling function
 *   to take care of the inclusive/exclusive bounds for temporal sequences
 */
static bool
point3dz_on_segment(const POINT3DZ *p, const POINT3DZ *A, const POINT3DZ *B)
{
  /* Find the collinearity of the points using the cross product
   * http://www.ambrsoft.com/TrigoCalc/Line3D/LineColinear.htm */
  double i = (p->y - A->y) * (B->z - A->z) - (p->z - A->z) * (B->y - A->y);
  double j = (p->z - A->z) * (B->x - A->x) - (p->x - A->x) * (B->z - A->z);
  double k = (p->x - A->x) * (B->y - A->y) - (p->y - A->y) * (B->x - A->x);
  if (fabs(i) >= MOBDB_EPSILON || fabs(j) >= MOBDB_EPSILON ||
    fabs(k) >= MOBDB_EPSILON)
    return false;
  double dotproduct = (p->x - A->x) * (B->x - A->x) +
    (p->y - A->y) * (B->y - A->y) + (p->z - A->z) * (B->z - A->z);
  return (dotproduct >= 0);
}

/**
 * Return true if point p is in the segment defined by A and B (geodetic)
 */
static bool
point_on_segment_sphere(const POINT4D *p, const POINT4D *A, const POINT4D *B)
{
  POINT4D closest;
  double dist;
  closest_point_on_segment_sphere(p, A, B, &closest, &dist);
  return (dist > MOBDB_EPSILON) && (FP_EQUALS(p->z, closest.z));
}

/**
 * Determine if a point is in a segment.
 *
 * @param[in] start,end Points defining the segment
 * @param[in] point Point
 */
static bool
point_on_segment(Datum start, Datum end, Datum point)
{
  GSERIALIZED *gs = DatumGetGserializedP(start);
  if (FLAGS_GET_GEODETIC(gs->gflags))
  {
    POINT4D p1, p2, p;
    datum_point4d(start, &p1);
    datum_point4d(end, &p2);
    datum_point4d(point, &p);
    return point_on_segment_sphere(&p, &p1, &p2);
  }
  if (FLAGS_GET_Z(gs->gflags))
  {
    const POINT3DZ *p1 = datum_point3dz_p(start);
    const POINT3DZ *p2 = datum_point3dz_p(end);
    const POINT3DZ *p = datum_point3dz_p(point);
    return point3dz_on_segment(p, p1, p2);
  }
  /* 2D */
  const POINT2D *p1 = datum_point2d_p(start);
  const POINT2D *p2 = datum_point2d_p(end);
  const POINT2D *p = datum_point2d_p(point);
  return point2d_on_segment(p, p1, p2);
}

/*****************************************************************************
 * Ever/always equal comparison operators
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal instant point is ever equal to a point
 * @pre The validity of the parameters is verified in function @ref tpoint_ever_eq
 * @sqlop @p ?=
 */
bool
tpointinst_ever_eq(const TInstant *inst, Datum value)
{
  Datum value1 = tinstant_value(inst);
  return datum_point_eq(value1, value);
}

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal sequence point is ever equal to a point
 * @pre The validity of the parameters is verified in function @ref tpoint_ever_eq
 * @sqlop @p ?=
 */
bool
tpointseq_ever_eq(const TSequence *seq, Datum value)
{
  int i;
  Datum value1;

  /* Bounding box test */
  if (! temporal_bbox_ev_al_eq((Temporal *) seq, value, EVER))
    return false;

  /* Stepwise interpolation or instantaneous sequence */
  if (! MOBDB_FLAGS_GET_LINEAR(seq->flags) || seq->count == 1)
  {
    for (i = 0; i < seq->count; i++)
    {
      value1 = tinstant_value(tsequence_inst_n(seq, i));
      if (datum_point_eq(value1, value))
        return true;
    }
    return false;
  }

  /* Linear interpolation*/
  const TInstant *inst1 = tsequence_inst_n(seq, 0);
  value1 = tinstant_value(inst1);
  bool lower_inc = seq->period.lower_inc;
  for (i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = tsequence_inst_n(seq, i);
    Datum value2 = tinstant_value(inst2);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    /* Constant segment */
    if (datum_point_eq(value1, value2) && datum_point_eq(value1, value))
      return true;
    /* Test bounds */
    if (datum_point_eq(value1, value))
    {
      if (lower_inc) return true;
    }
    else if (datum_point_eq(value2, value))
    {
      if (upper_inc) return true;
    }
    /* Test point on segment */
    else if (point_on_segment(value1, value2, value))
      return true;
    value1 = value2;
    lower_inc = true;
  }
  return false;
}

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal sequence set point is ever equal to a point
 * @pre The validity of the parameters is verified in function @ref tpoint_ever_eq
 * @sqlop @p ?=
 */
bool
tpointseqset_ever_eq(const TSequenceSet *ss, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_eq((Temporal *) ss, value, EVER))
    return false;

  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    if (tpointseq_ever_eq(seq, value))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal point is ever equal to a point.
 * @see tpointinst_ever_eq
 * @see tpointseq_ever_eq
 * @see tpointseqset_ever_eq
 */
bool
tpoint_ever_eq(const Temporal *temp, Datum value)
{
  GSERIALIZED *gs = DatumGetGserializedP(value);
  if (gserialized_is_empty(gs))
    return false;
  ensure_point_type(gs);
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
  ensure_same_dimensionality_tpoint_gs(temp, gs);

  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tpointinst_ever_eq((TInstant *) temp, value);
  else if (temp->subtype == TSEQUENCE)
    result = tpointseq_ever_eq((TSequence *) temp, value);
  else /* temp->subtype == TSEQUENCESET */
    result = tpointseqset_ever_eq((TSequenceSet *) temp, value);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal geometric point is ever equal to a point.
 * @sqlop @p ?=
 */
bool tgeompoint_ever_eq(const Temporal *temp, GSERIALIZED *gs)
{
  return tpoint_ever_eq(temp, PointerGetDatum(gs));

}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal geographic point is ever equal to a point.
 * @sqlop @p ?=
 */
bool tgeogpoint_ever_eq(const Temporal *temp, GSERIALIZED *gs)
{
  return tpoint_ever_eq(temp, PointerGetDatum(gs));
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal instant point is always equal to a point.
 * @pre The validity of the parameters is verified in function @ref tpoint_always_eq
 * @sqlop @p %=
 */
bool
tpointinst_always_eq(const TInstant *inst, Datum value)
{
  return tpointinst_ever_eq(inst, value);
}

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal sequence point is always equal to a point.
 * @pre The validity of the parameters is verified in function @ref tpoint_always_eq
 * @sqlop @p %=
 */
bool
tpointseq_always_eq(const TSequence *seq, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_eq((Temporal *) seq, value, ALWAYS))
    return false;

  /* The bounding box test above is enough to compute the answer for
   * temporal numbers */
  return true;
}

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal sequence set point is always equal to a point.
 * @pre The validity of the parameters is verified in function @ref tpoint_always_eq
 * @sqlop @p %=
 */
bool
tpointseqset_always_eq(const TSequenceSet *ss, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_eq((Temporal *)ss, value, ALWAYS))
    return false;

  /* The bounding box test above is enough to compute the answer for
   * temporal numbers */
  return true;
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal point is always equal to a point.
 * @see tpointinst_always_eq
 * @see tpointseq_always_eq
 * @see tpointseqset_always_eq
 * @sqlop @p %=
 */
bool
tpoint_always_eq(const Temporal *temp, Datum value)
{
  GSERIALIZED *gs = DatumGetGserializedP(value);
  if (gserialized_is_empty(gs))
    return false;
  ensure_point_type(gs);
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
  ensure_same_dimensionality_tpoint_gs(temp, gs);

  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tpointinst_always_eq((TInstant *) temp, value);
  else if (temp->subtype == TSEQUENCE)
    result = tpointseq_always_eq((TSequence *) temp, value);
  else /* temp->subtype == TSEQUENCESET */
    result = tpointseqset_always_eq((TSequenceSet *) temp, value);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal geometric point is always equal to a point.
 * @sqlop @p %=
 */
bool tgeompoint_always_eq(const Temporal *temp, GSERIALIZED *gs)
{
  return tpoint_always_eq(temp, PointerGetDatum(gs));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal geographic point is always equal to a point.
 * @sqlop @p %=
 */
bool tgeogpoint_always_eq(const Temporal *temp, GSERIALIZED *gs)
{
  return tpoint_always_eq(temp, PointerGetDatum(gs));
}
#endif /* MEOS */

/*****************************************************************************
 * Functions derived from PostGIS to increase floating-point precision
 *****************************************************************************/

/**
 * Function derived from PostGIS file lwalgorithm.c since it is declared static
 */
static bool
lw_seg_interact(const POINT2D p1, const POINT2D p2, const POINT2D q1,
  const POINT2D q2)
{
  double minq = FP_MIN(q1.x, q2.x);
  double maxq = FP_MAX(q1.x, q2.x);
  double minp = FP_MIN(p1.x, p2.x);
  double maxp = FP_MAX(p1.x, p2.x);

  if (FP_GT(minp, maxq) || FP_LT(maxp, minq))
    return false;

  minq = FP_MIN(q1.y, q2.y);
  maxq = FP_MAX(q1.y, q2.y);
  minp = FP_MIN(p1.y, p2.y);
  maxp = FP_MAX(p1.y, p2.y);

  if (FP_GT(minp,maxq) || FP_LT(maxp,minq))
    return false;

  return true;
}

/**
 * Return a long double between 0 and 1 representing the location of the
 * closest point on the segment to the given point, as a fraction of total
 * segment length (2D version)
 *
 * @note Function derived from the PostGIS function closest_point_on_segment
 */
long double
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
  long double r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) ) /
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
 * Return a float between 0 and 1 representing the location of the closest
 * point on the segment to the given point, as a fraction of total segment
 * length (3D version)
 *
 * @note Function derived from the PostGIS function closest_point_on_segment
 */
long double
closest_point3dz_on_segment_ratio(const POINT3DZ *p, const POINT3DZ *A,
  const POINT3DZ *B, POINT3DZ *closest)
{
  if (FP_EQUALS(A->x, B->x) && FP_EQUALS(A->y, B->y) && FP_EQUALS(A->z, B->z))
  {
    *closest = *A;
    return 0.0;
  }

  /*
   * Function closest_point2d_on_segment_ratio above explains how r is computed
   */
  long double r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) +
      (p->z-A->z) * (B->z-A->z) ) /
    ( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) +
      (B->z-A->z) * (B->z-A->z) );

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
 * Return a float between 0 and 1 representing the location of the closest
 * point on the geography segment to the given point, as a fraction of total
 * segment length
 *
 *@param[in] p Reference point
 *@param[in] A,B Points defining the segment
 *@param[out] closest Closest point in the segment
 *@param[out] dist Distance between the closest point and the reference point
 */
long double
closest_point_on_segment_sphere(const POINT4D *p, const POINT4D *A,
  const POINT4D *B, POINT4D *closest, double *dist)
{
  GEOGRAPHIC_EDGE e;
  GEOGRAPHIC_POINT gp, proj;
  long double length, /* length from A to the closest point */
    seglength; /* length of the segment AB */
  long double result; /* ratio */

  /* Initialize target point */
  geographic_point_init(p->x, p->y, &gp);

  /* Initialize edge */
  geographic_point_init(A->x, A->y, &(e.start));
  geographic_point_init(B->x, B->y, &(e.end));

  /* Get the spherical distance between point and edge */
  *dist = edge_distance_to_point(&e, &gp, &proj);

  /* Compute distance from beginning of the segment to closest point */
  seglength = (long double) sphere_distance(&(e.start), &(e.end));
  length = (long double) sphere_distance(&(e.start), &proj);
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

/**
 * Find interpolation point p between geography points p1 and p2
 * so that the len(p1,p) == len(p1,p2)
 * f and p falls on p1,p2 segment
 *
 * @param[in] p1,p2 3D-space points we are interpolating between
 * @param[in] v1,v2 real values and z/m coordinates
 * @param[in] f Fraction
 * @param[out] p Result
 */
void
interpolate_point4d_sphere(const POINT3D *p1, const POINT3D *p2,
  const POINT4D *v1, const POINT4D *v2, double f, POINT4D *p)
{
  /* Calculate interpolated point */
  POINT3D mid;
  mid.x = p1->x + ((p2->x - p1->x) * f);
  mid.y = p1->y + ((p2->y - p1->y) * f);
  mid.z = p1->z + ((p2->z - p1->z) * f);
  normalize(&mid);

  /* Calculate z/m values */
  GEOGRAPHIC_POINT g;
  cart2geog(&mid, &g);
  p->x = rad2deg(g.lon);
  p->y = rad2deg(g.lat);
  p->z = v1->z + ((v2->z - v1->z) * f);
  p->m = v1->m + ((v2->m - v1->m) * f);
}

/*****************************************************************************
 * Functions specializing the PostGIS functions ST_LineInterpolatePoint and
 * ST_LineLocatePoint
 *****************************************************************************/

/**
 * Create a point
 */
GSERIALIZED *
gspoint_make(double x, double y, double z, bool hasz, bool geodetic,
  int32 srid)
{
  LWPOINT *lwpoint = hasz ?
    lwpoint_make3dz(srid, x, y, z) : lwpoint_make2d(srid, x, y);
  FLAGS_SET_GEODETIC(lwpoint->flags, geodetic);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwpoint);
  lwpoint_free(lwpoint);
  return result;
}

/**
 * Return a point interpolated from the geometry/geography segment with
 * respect to the fraction of its total length
 *
 * @param[in] start,end Points defining the segment
 * @param[in] ratio Float between 0 and 1 representing the fraction of the
 * total length of the segment where the point must be located
 */
Datum
geosegm_interpolate_point(Datum start, Datum end, long double ratio)
{
  GSERIALIZED *gs = DatumGetGserializedP(start);
  int srid = gserialized_get_srid(gs);
  POINT4D p1, p2, p;
  datum_point4d(start, &p1);
  datum_point4d(end, &p2);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool geodetic = (bool) FLAGS_GET_GEODETIC(gs->gflags);
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
  {
    /* We cannot call the PostGIS function
     * interpolate_point4d(&p1, &p2, &p, ratio);
     * since it uses a double and not a long double for the interpolation */
    p.x = p1.x + ((long double) (p2.x - p1.x) * ratio);
    p.y = p1.y + ((long double) (p2.y - p1.y) * ratio);
    p.z = p1.z + ((long double) (p2.z - p1.z) * ratio);
    p.m = 0.0;
  }

  Datum result = PointerGetDatum(gspoint_make(p.x, p.y, p.z, hasz, geodetic,
    srid));
  PG_FREE_IF_COPY_P(gs, DatumGetPointer(start));
  return result;
}

/**
 * Return a float between 0 and 1 representing the location of the closest
 * point on the geometry segment to the given point, as a fraction of total
 * segment length
 *
 *@param[in] start,end Points defining the segment
 *@param[in] point Reference point
 *@param[out] dist Distance
 */
long double
geosegm_locate_point(Datum start, Datum end, Datum point, double *dist)
{
  GSERIALIZED *gs = DatumGetGserializedP(start);
  long double result;
  if (FLAGS_GET_GEODETIC(gs->gflags))
  {
    POINT4D p1, p2, p, closest;
    datum_point4d(start, &p1);
    datum_point4d(end, &p2);
    datum_point4d(point, &p);
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
      if (FLAGS_GET_Z(gs->gflags))
        d = sqrt( (closest.z - p.z) * (closest.z - p.z) + d*d );
      *dist = d;
    }
  }
  else
  {
    if (FLAGS_GET_Z(gs->gflags))
    {
      const POINT3DZ *p1 = datum_point3dz_p(start);
      const POINT3DZ *p2 = datum_point3dz_p(end);
      const POINT3DZ *p = datum_point3dz_p(point);
      POINT3DZ proj;
      result = closest_point3dz_on_segment_ratio(p, p1, p2, &proj);
      /* For robustness, force 0/1 when closest point == start/endpoint */
      if (p3d_same((POINT3D *) p1, (POINT3D *) &proj))
        result = 0.0;
      else if (p3d_same((POINT3D *) p2, (POINT3D *) &proj))
        result = 1.0;
      if (dist)
        *dist = distance3d_pt_pt((POINT3D *) p, (POINT3D *) &proj);
    }
    else
    {
      const POINT2D *p1 = datum_point2d_p(start);
      const POINT2D *p2 = datum_point2d_p(end);
      const POINT2D *p = datum_point2d_p(point);
      POINT2D proj;
      result = closest_point2d_on_segment_ratio(p, p1, p2, &proj);
      if (p2d_same(p1, &proj))
        result = 0.0;
      else if (p2d_same(p2, &proj))
        result = 1.0;
      if (dist)
        *dist = distance2d_pt_pt((POINT2D *) p, &proj);
    }
  }
  return result;
}

/*****************************************************************************
 * Interpolation functions defining functionality required by tsequence.c
 * that must be implemented by each temporal type
 *****************************************************************************/

/**
 * Return true if a segment of a temporal point value intersects a point
 * at the timestamp
 *
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] value Base value
 * @param[out] t Timestamp
 * @pre The geometry is not empty
 */
bool
tpointsegm_intersection_value(const TInstant *inst1, const TInstant *inst2,
  Datum value, TimestampTz *t)
{
  GSERIALIZED *gs = DatumGetGserializedP(value);
  assert(! gserialized_is_empty(gs));

  /* We are sure that the trajectory is a line */
  Datum start = tinstant_value(inst1);
  Datum end = tinstant_value(inst2);
  double dist;
  double fraction = geosegm_locate_point(start, end, value, &dist);
  if (fabs(dist) >= MOBDB_EPSILON)
    return false;
  if (t != NULL)
  {
    double duration = (inst2->t - inst1->t);
    /* Note that due to roundoff errors it may be the case that the
     * resulting timestamp t may be equal to inst1->t or to inst2->t */
    *t = inst1->t + (TimestampTz) (duration * fraction);
  }
  return true;
}

/**
 * Return true if two segments of a temporal geometric points intersect at
 * a timestamp
 *
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[out] t Timestamp
 * @pre The instants are synchronized, i.e., start1->t = start2->t and
 * end1->t = end2->t
 */
bool
tgeompointsegm_intersection(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, TimestampTz *t)
{
  long double fraction, xfraction = 0, yfraction = 0, xdenum, ydenum;
  if (MOBDB_FLAGS_GET_Z(start1->flags)) /* 3D */
  {
    long double zfraction = 0, zdenum;
    const POINT3DZ *p1 = datum_point3dz_p(tinstant_value(start1));
    const POINT3DZ *p2 = datum_point3dz_p(tinstant_value(end1));
    const POINT3DZ *p3 = datum_point3dz_p(tinstant_value(start2));
    const POINT3DZ *p4 = datum_point3dz_p(tinstant_value(end2));
    xdenum = p2->x - p1->x - p4->x + p3->x;
    ydenum = p2->y - p1->y - p4->y + p3->y;
    zdenum = p2->z - p1->z - p4->z + p3->z;
    if (xdenum == 0 && ydenum == 0 && zdenum == 0)
      /* Parallel segments */
      return false;

    if (xdenum != 0)
    {
      xfraction = (p3->x - p1->x) / xdenum;
      if (xfraction < -1 * MOBDB_EPSILON || 1.0 + MOBDB_EPSILON < xfraction)
        /* Intersection occurs out of the period */
        return false;
    }
    if (ydenum != 0)
    {
      yfraction = (p3->y - p1->y) / ydenum;
      if (yfraction < -1 * MOBDB_EPSILON || 1.0 + MOBDB_EPSILON < yfraction)
        /* Intersection occurs out of the period */
        return false;
    }
    if (zdenum != 0)
    {
      zfraction = (p3->z - p1->z) / zdenum;
      if (zfraction < -1 * MOBDB_EPSILON || 1.0 + MOBDB_EPSILON < zfraction)
        /* Intersection occurs out of the period */
        return false;
    }
    /* If intersection occurs at different timestamps on each dimension */
    if ((xdenum != 0 && ydenum != 0 && zdenum != 0 &&
        fabsl(xfraction - yfraction) > MOBDB_EPSILON &&
        fabsl(xfraction - zfraction) > MOBDB_EPSILON) ||
      (xdenum == 0 && ydenum != 0 && zdenum != 0 &&
        fabsl(yfraction - zfraction) > MOBDB_EPSILON) ||
      (xdenum != 0 && ydenum == 0 && zdenum != 0 &&
        fabsl(xfraction - zfraction) > MOBDB_EPSILON) ||
      (xdenum != 0 && ydenum != 0 && zdenum == 0 &&
        fabsl(xfraction - yfraction) > MOBDB_EPSILON))
      return false;
    if (xdenum != 0)
      fraction = xfraction;
    else if (ydenum != 0)
      fraction = yfraction;
    else
      fraction = zfraction;
  }
  else /* 2D */
  {
    const POINT2D *p1 = datum_point2d_p(tinstant_value(start1));
    const POINT2D *p2 = datum_point2d_p(tinstant_value(end1));
    const POINT2D *p3 = datum_point2d_p(tinstant_value(start2));
    const POINT2D *p4 = datum_point2d_p(tinstant_value(end2));
    xdenum = p2->x - p1->x - p4->x + p3->x;
    ydenum = p2->y - p1->y - p4->y + p3->y;
    if (xdenum == 0 && ydenum == 0)
      /* Parallel segments */
      return false;

    if (xdenum != 0)
    {
      xfraction = (p3->x - p1->x) / xdenum;
      if (xfraction < -1 * MOBDB_EPSILON || 1.0 + MOBDB_EPSILON < xfraction)
        /* Intersection occurs out of the period */
        return false;
    }
    if (ydenum != 0)
    {
      yfraction = (p3->y - p1->y) / ydenum;
      if (yfraction < -1 * MOBDB_EPSILON || 1.0 + MOBDB_EPSILON < yfraction)
        /* Intersection occurs out of the period */
        return false;
    }
    /* If intersection occurs at different timestamps on each dimension */
    if (xdenum != 0 && ydenum != 0 &&
        fabsl(xfraction - yfraction) > MOBDB_EPSILON)
      return false;
    fraction = xdenum != 0 ? xfraction : yfraction;
  }
  double duration = (end1->t - start1->t);
  *t = start1->t + (TimestampTz) (duration * fraction);
  /* Note that due to roundoff errors it may be the case that the
   * resulting timestamp t may be equal to inst1->t or to inst2->t */
  if (*t <= start1->t || *t >= end1->t)
    return false;
  return true;
}

/**
 * Return true if two segments of twp temporal geographic points intersect at
 * a timestamp
 *
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[out] t Timestamp
 * @pre The instants are synchronized, i.e., start1->t = start2->t and
 * end1->t = end2->t
 */
bool
tgeogpointsegm_intersection(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, TimestampTz *t)
{
  GEOGRAPHIC_EDGE e1, e2;
  POINT3D A1, A2, B1, B2;
  POINT4D p1, p2, p3, p4;

  datum_point4d(tinstant_value(start1), &p1);
  geographic_point_init(p1.x, p1.y, &(e1.start));
  geog2cart(&(e1.start), &A1);

  datum_point4d(tinstant_value(end1), &p2);
  geographic_point_init(p2.x, p2.y, &(e1.end));
  geog2cart(&(e1.end), &A2);

  datum_point4d(tinstant_value(start2), &p3);
  geographic_point_init(p3.x, p3.y, &(e2.start));
  geog2cart(&(e2.start), &B1);

  datum_point4d(tinstant_value(end2), &p4);
  geographic_point_init(p4.x, p4.y, &(e2.end));
  geog2cart(&(e2.end), &B2);

  if (! edge_intersects(&A1, &A2, &B1, &B2))
    return false;

  /* Compute the intersection point */
  GEOGRAPHIC_POINT inter;
  edge_intersection(&e1, &e2, &inter);
  /* Compute distance from beginning of the segment to closest point */
  long double seglength = sphere_distance(&(e1.start), &(e1.end));
  long double length = sphere_distance(&(e1.start), &inter);
  long double fraction = length / seglength;

  long double duration = (end1->t - start1->t);
  *t = start1->t + (TimestampTz) (duration * fraction);
  /* Note that due to roundoff errors it may be the case that the
   * resulting timestamp t may be equal to inst1->t or to inst2->t */
  if (*t <= start1->t || *t >= end1->t)
    return false;
  return true;
}

/**
 * Return true if the three values are collinear
 *
 * @param[in] value1,value2,value3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the
 * timestamps associated to `value1` and `value2` divided by the duration
 * of the timestamps associated to `value1` and `value3`
 * @param[in] hasz True if the points have Z coordinates
 * @param[in] geodetic True for geography, false for geometry
 */
bool
geopoint_collinear(Datum value1, Datum value2, Datum value3,
  double ratio, bool hasz, bool geodetic)
{
  POINT4D p1, p2, p3, p;
  datum_point4d(value1, &p1);
  datum_point4d(value2, &p2);
  datum_point4d(value3, &p3);
  if (geodetic)
  {
    POINT3D q1, q3;
    GEOGRAPHIC_POINT g1, g3;
    geographic_point_init(p1.x, p1.y, &g1);
    geographic_point_init(p3.x, p3.y, &g3);
    geog2cart(&g1, &q1);
    geog2cart(&g3, &q3);
    interpolate_point4d_sphere(&q1, &q3, &p1, &p3, ratio, &p);
  }
  else
    interpolate_point4d(&p1, &p3, &p, ratio);

  bool result = hasz ?
    fabs(p2.x - p.x) <= MOBDB_EPSILON && fabs(p2.y - p.y) <= MOBDB_EPSILON &&
      fabs(p2.z - p.z) <= MOBDB_EPSILON :
    fabs(p2.x - p.x) <= MOBDB_EPSILON && fabs(p2.y - p.y) <= MOBDB_EPSILON;
  return result;
}

/*****************************************************************************
 * Trajectory functions
 *****************************************************************************/

/**
 * @brief Return -1, 0, or 1 depending on whether the first LWPOINT
 * is less than, equal, or greater than the second one.
 * @pre There is no empty point in the array, the points are of the same
 * dimensionality
 */
static int
lwpoint_cmp(const LWPOINT *p, const LWPOINT *q)
{
  assert(FLAGS_GET_ZM(p->flags) == FLAGS_GET_ZM(q->flags));
  POINT4D p4d, q4d;
  /* We are sure the points are not empty */
  lwpoint_getPoint4d_p(p, &p4d);
  lwpoint_getPoint4d_p(q, &q4d);
  int cmp = float8_cmp_internal(p4d.x, q4d.x);
  if (cmp != 0)
    return cmp;
  cmp = float8_cmp_internal(p4d.y, q4d.y);
  if (cmp != 0)
    return cmp;
  if (FLAGS_GET_Z(p->flags))
  {
    cmp = float8_cmp_internal(p4d.z, q4d.z);
    if (cmp != 0)
      return cmp;
  }
  if (FLAGS_GET_M(p->flags))
  {
    cmp = float8_cmp_internal(p4d.m, q4d.m);
    if (cmp != 0)
      return cmp;
  }
  return 0;
}

/**
 * @brief Comparator function for lwpoints
 */
static int
lwpoint_sort_cmp(const LWPOINT **l, const LWPOINT **r)
{
  return lwpoint_cmp(*l, *r);
}

/**
 * @brief Sort function for lwpoints
 */
void
lwpointarr_sort(LWPOINT **points, int count)
{
  qsort(points, (size_t) count, sizeof(LWPOINT *),
    (qsort_comparator) &lwpoint_sort_cmp);
}

/**
 * @brief Remove duplicates from an array of LWGEOM points
 */
LWGEOM **
lwpointarr_remove_duplicates(LWGEOM **points, int count, int *newcount)
{
  assert (count > 0);
  /* We bypass the call to function lwgeom_as_lwpoint through memcpy */
  LWGEOM **newpoints = palloc(sizeof(LWGEOM *) * count);
  memcpy(newpoints, points, sizeof(LWGEOM *) * count);
  lwpointarr_sort((LWPOINT **) newpoints, count);
  int count1 = 0;
  for (int i = 1; i < count; i++)
    if (! lwpoint_same((LWPOINT *) newpoints[count1], (LWPOINT *) newpoints[i]))
      newpoints[++ count1] = newpoints[i];
  *newcount = count1 + 1;
  return newpoints;
}

/**
 * @brief Compute a trajectory from a set of points. The result is either a
 * linestring or a multipoint depending on whether the interpolation is
 * stepwise/discrete or linear.
 * @note The function does not remove duplicate points, that is, repeated
 * points in a multipoint or consecutive equal points in a line string. This
 * should be done in the calling function.
 *
 * @param[in] lwpoints Array of points
 * @param[in] count Number of elements in the input array
 * @param[in] interp Interpolation
 */
LWGEOM *
lwpointarr_make_trajectory(LWGEOM **lwpoints, int count, interpType interp)
{
  if (count == 1)
    return lwpoint_as_lwgeom(lwpoint_clone(lwgeom_as_lwpoint(lwpoints[0])));

  LWGEOM *result = (interp == LINEAR) ?
    (LWGEOM *) lwline_from_lwgeom_array(lwpoints[0]->srid, (uint32_t) count,
      lwpoints) :
    (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE, lwpoints[0]->srid,
      NULL, (uint32_t) count, lwpoints);
  FLAGS_SET_Z(result->flags, FLAGS_GET_Z(lwpoints[0]->flags));
  FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(lwpoints[0]->flags));
  return result;
}

/**
 * Compute the trajectory from two geometry points
 *
 * @param[in] value1,value2 Points
 */
LWLINE *
lwline_make(Datum value1, Datum value2)
{
  /* Obtain the flags and the SRID from the first value */
  GSERIALIZED *gs = DatumGetGserializedP(value1);
  int srid = gserialized_get_srid(gs);
  int hasz = FLAGS_GET_Z(gs->gflags);
  int isgeodetic = FLAGS_GET_GEODETIC(gs->gflags);
  /* Since there is no M value a 0 value is passed */
  POINTARRAY *pa = ptarray_construct_empty(hasz, 0, 2);
  POINT4D pt;
  datum_point4d(value1, &pt);
  ptarray_append_point(pa, &pt, LW_TRUE);
  datum_point4d(value2, &pt);
  ptarray_append_point(pa, &pt, LW_TRUE);
  LWLINE *result = lwline_construct(srid, NULL, pa);
  FLAGS_SET_Z(result->flags, hasz);
  FLAGS_SET_GEODETIC(result->flags, isgeodetic);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Compute the trajectory of a temporal discrete sequence point
 *
 * @param[in] seq Temporal value
 * @note Notice that this function does not remove duplicate points
 * @sqlfunc trajectory()
 */
GSERIALIZED *
tpointdiscseq_trajectory(const TSequence *seq)
{
  /* Singleton discrete sequence */
  if (seq->count == 1)
    return DatumGetGserializedP(tinstant_value_copy(tsequence_inst_n(seq, 0)));

  LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    Datum value = tinstant_value(tsequence_inst_n(seq, i));
    GSERIALIZED *gsvalue = DatumGetGserializedP(value);
    points[i] = lwgeom_from_gserialized(gsvalue);
  }
  LWGEOM *lwgeom = lwpointarr_make_trajectory(points, seq->count, STEPWISE);
  GSERIALIZED *result = geo_serialize(lwgeom);
  pfree(lwgeom);
  for (int i = 0; i < seq->count; i++)
    lwpoint_free((LWPOINT *) points[i]);
  pfree(points);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return the trajectory of a temporal sequence point
 *
 * @param[in] seq Temporal sequence
 * @note Since the sequence has been already validated there is no
 * verification of the input in this function, in particular for geographies
 * it is supposed that the composing points are geodetic
 * @sqlfunc trajectory()
 */
GSERIALIZED *
tpointcontseq_trajectory(const TSequence *seq)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return DatumGetGserializedP(tinstant_value_copy(tsequence_inst_n(seq, 0)));

  LWPOINT **points = palloc(sizeof(LWPOINT *) * seq->count);
  /* Remove two consecutive points if they are equal */
  Datum value = tinstant_value(tsequence_inst_n(seq, 0));
  GSERIALIZED *gs = DatumGetGserializedP(value);
  points[0] = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
  int k = 1;
  for (int i = 1; i < seq->count; i++)
  {
    value = tinstant_value(tsequence_inst_n(seq, i));
    gs = DatumGetGserializedP(value);
    LWPOINT *lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
    if (! lwpoint_same(lwpoint, points[k - 1]))
      points[k++] = lwpoint;
  }
  LWGEOM *lwgeom = lwpointarr_make_trajectory((LWGEOM **) points, k,
    MOBDB_FLAGS_GET_INTERP(seq->flags));
  GSERIALIZED *result = geo_serialize(lwgeom);
  pfree(lwgeom);
  for (int i = 0; i < k; i++)
    lwpoint_free(points[i]);
  pfree(points);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return the trajectory of a temporal point with sequence set type
 * @note The function does not remove duplicates point/linestring components.
 * @sqlfunc trajectory()
 */
GSERIALIZED *
tpointseqset_trajectory(const TSequenceSet *ss)
{
  /* Singleton sequence set */
  if (ss->count == 1)
    return tpointcontseq_trajectory(tsequenceset_seq_n(ss, 0));

  bool geodetic = MOBDB_FLAGS_GET_GEODETIC(ss->flags);
  LWPOINT **points = palloc(sizeof(LWPOINT *) * ss->totalcount);
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ss->count);
  int k = 0, l = 0;
  for (int i = 0; i < ss->count; i++)
  {
    GSERIALIZED *traj = tpointcontseq_trajectory(tsequenceset_seq_n(ss, i));
    int geotype = gserialized_get_type(traj);
    if (geotype == POINTTYPE)
      points[l++] = lwgeom_as_lwpoint(lwgeom_from_gserialized(traj));
    else if (geotype == MULTIPOINTTYPE)
    {
      LWMPOINT *lwmpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(traj));
      int count = lwmpoint->ngeoms;
      for (int m = 0; m < count; m++)
        points[l++] = lwmpoint->geoms[m];
    }
    /* gserialized_get_type(traj) == LINETYPE */
    else
      geoms[k++] = lwgeom_from_gserialized(traj);
  }
  GSERIALIZED *result;
  if (k == 0)
  {
    /* Only points */
    LWGEOM *lwgeom = lwpointarr_make_trajectory((LWGEOM **) points, l, STEPWISE);
    result = geo_serialize(lwgeom);
    pfree(lwgeom);
  }
  else if (l == 0)
  {
    /* Only lines */
    /* k > 1 since otherwise it is a singleton sequence set and this case
     * was taken care at the begining of the function */
    // TODO add the bounding box instead of ask PostGIS to compute it again
    LWGEOM *coll = (LWGEOM *) lwcollection_construct(MULTILINETYPE,
      geoms[0]->srid, NULL, (uint32_t) k, geoms);
    FLAGS_SET_GEODETIC(coll->flags, geodetic);
    result = geo_serialize(coll);
    /* We cannot lwgeom_free(geoms[i] or lwgeom_free(coll) */
  }
  else
  {
    /* Both points and lines */
    if (l == 1)
      geoms[k++] = (LWGEOM *) points[0];
    else
    {
      geoms[k++] = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
        points[0]->srid, NULL, (uint32_t) l, (LWGEOM **) points);
      /* We cannot lwpoint_free(points[i]); */
    }
    // TODO add the bounding box instead of ask PostGIS to compute it again
    LWGEOM *coll = (LWGEOM *) lwcollection_construct(COLLECTIONTYPE,
      geoms[0]->srid, NULL, (uint32_t) k, geoms);
    FLAGS_SET_GEODETIC(coll->flags, geodetic);
    result = geo_serialize(coll);
  }
  pfree(points); pfree(geoms);
  return result;
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return the trajectory of a temporal point.
 * @sqlfunc trajectory()
 */
GSERIALIZED *
tpoint_trajectory(const Temporal *temp)
{
  GSERIALIZED *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = DatumGetGserializedP(tinstant_value_copy((TInstant *) temp));
  else if (temp->subtype == TSEQUENCE)
    result = MOBDB_FLAGS_GET_DISCRETE(temp->flags) ?
      tpointdiscseq_trajectory((TSequence *) temp) :
      tpointcontseq_trajectory((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tpointseqset_trajectory((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return the SRID of a temporal instant point.
 * @sqlfunc SRID()
 */
int
tpointinst_srid(const TInstant *inst)
{
  GSERIALIZED *gs = DatumGetGserializedP(&inst->value);
  return gserialized_get_srid(gs);
}

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return the SRID of a temporal sequence point.
 * @sqlfunc SRID()
 */
int
tpointseq_srid(const TSequence *seq)
{
  STBOX *box = TSEQUENCE_BBOX_PTR(seq);
  return box->srid;
}

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return the SRID of a temporal sequence set point.
 * @sqlfunc SRID()
 */
int
tpointseqset_srid(const TSequenceSet *ss)
{
  STBOX *box = TSEQUENCESET_BBOX_PTR(ss);
  return box->srid;
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return the SRID of a temporal point.
 * @sqlfunc SRID()
 */
int
tpoint_srid(const Temporal *temp)
{
  int result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = tpointinst_srid((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tpointseq_srid((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tpointseqset_srid((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_spatial_transf
 * @brief Set the SRID of a temporal instant point
 * @sqlfunc setSRID()
 */
TInstant *
tpointinst_set_srid(const TInstant *inst, int32 srid)
{
  TInstant *result = tinstant_copy(inst);
  GSERIALIZED *gs = DatumGetGserializedP(&result->value);
  gserialized_set_srid(gs, srid);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_spatial_transf
 * @brief Set the SRID of a temporal sequence point
 * @sqlfunc setSRID()
 */
TSequence *
tpointseq_set_srid(const TSequence *seq, int32 srid)
{
  TSequence *result = tsequence_copy(seq);
  /* Set the SRID of the composing points */
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(result, i);
    GSERIALIZED *gs = DatumGetGserializedP(&inst->value);
    gserialized_set_srid(gs, srid);
  }
  /* Set the SRID of the bounding box */
  STBOX *box = TSEQUENCE_BBOX_PTR(result);
  box->srid = srid;
  return result;
}

/**
 * @ingroup libmeos_int_temporal_spatial_transf
 * @brief Set the SRID of a temporal sequence set point
 * @sqlfunc setSRID()
 */
TSequenceSet *
tpointseqset_set_srid(const TSequenceSet *ss, int32 srid)
{
  STBOX *box;
  TSequenceSet *result = tsequenceset_copy(ss);
  /* Loop for every composing sequence */
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(result, i);
    for (int j = 0; j < seq->count; j++)
    {
      /* Set the SRID of the composing points */
      const TInstant *inst = tsequence_inst_n(seq, j);
      GSERIALIZED *gs = DatumGetGserializedP(&inst->value);
      gserialized_set_srid(gs, srid);
    }
    /* Set the SRID of the bounding box */
    box = TSEQUENCE_BBOX_PTR(seq);
    box->srid = srid;
  }
  /* Set the SRID of the bounding box */
  box = TSEQUENCESET_BBOX_PTR(result);
  box->srid = srid;
  return result;
}

/**
 * @ingroup libmeos_temporal_spatial_transf
 * @brief Set the SRID of a temporal point.
 * @see tpointinst_set_srid
 * @see tpointseq_set_srid
 * @see tpointseqset_set_srid
 * @sqlfunc setSRID()
 */
Temporal *
tpoint_set_srid(const Temporal *temp, int32 srid)
{
  Temporal *result;
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tpointinst_set_srid((TInstant *) temp, srid);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tpointseq_set_srid((TSequence *) temp, srid);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tpointseqset_set_srid((TSequenceSet *) temp, srid);

  assert(result != NULL);
  return result;
}

/*****************************************************************************
 * Cast functions
 * Notice that a geometry point and a geography point are of different size
 * since the geography point keeps a bounding box
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_spatial_transf
 * @brief Convert a temporal point from/to a geometry/geography point
 *
 * @param[in] inst Temporal instant point
 * @param[in] oper True when transforming from geometry to geography,
 * false otherwise
 * @sqlop ::
 */
TInstant *
tgeompointinst_tgeogpointinst(const TInstant *inst, bool oper)
{
  GSERIALIZED *gs = DatumGetGserializedP(tinstant_value(inst));
  Datum point = (oper == GEOM_TO_GEOG) ?
    PointerGetDatum(gserialized_geog_from_geom(gs)) :
    PointerGetDatum(gserialized_geom_from_geog(gs));
  TInstant *result = tinstant_make(point, (oper == GEOM_TO_GEOG) ?
    T_TGEOGPOINT : T_TGEOMPOINT, inst->t);
  pfree(DatumGetPointer(point));
  return result;
}

/**
 * @ingroup libmeos_int_temporal_spatial_transf
 * @brief Convert a temporal point from/to a geometry/geography point
 *
 * @param[in] seq Temporal sequence point
 * @param[in] oper True when transforming from geometry to geography,
 * false otherwise
 * @sqlop ::
 */
TSequence *
tgeompointseq_tgeogpointseq(const TSequence *seq, bool oper)
{
  /* Construct a multipoint with all the points */
  LWPOINT **points = palloc(sizeof(LWPOINT *) * seq->count);
  const TInstant *inst;
  GSERIALIZED *gs;
  for (int i = 0; i < seq->count; i++)
  {
    inst = tsequence_inst_n(seq, i);
    gs = DatumGetGserializedP(&inst->value);
    points[i] = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
  }
  LWGEOM *lwresult = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
      points[0]->srid, NULL, (uint32_t) seq->count, (LWGEOM **) points);
  GSERIALIZED *mpoint_orig = geo_serialize(lwresult);
  for (int i = 0; i < seq->count; i++)
    lwpoint_free(points[i]);
  pfree(points);
  /* Convert the multipoint geometry/geography */
  gs = (oper == GEOM_TO_GEOG) ?
    gserialized_geog_from_geom(mpoint_orig) :
    gserialized_geom_from_geog(mpoint_orig);
  /* Construct the resulting tpoint from the multipoint geometry/geography */
  LWMPOINT *lwmpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  mobdbType restype = (oper == GEOM_TO_GEOG) ?  T_TGEOGPOINT : T_TGEOMPOINT;
  for (int i = 0; i < seq->count; i++)
  {
    inst = tsequence_inst_n(seq, i);
    Datum point = PointerGetDatum(geo_serialize((LWGEOM *) (lwmpoint->geoms[i])));
    instants[i] = tinstant_make(point, restype, inst->t);
    pfree(DatumGetPointer(point));
  }
  lwmpoint_free(lwmpoint);
  return tsequence_make_free(instants, seq->count, seq->count,
    seq->period.lower_inc, seq->period.upper_inc,
    MOBDB_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @ingroup libmeos_int_temporal_spatial_transf
 * @brief Convert a temporal point from/to a geometry/geography point
 *
 * @param[in] ss Temporal sequence set point
 * @param[in] oper True when transforming from geometry to geography,
 * false otherwise
 * @sqlop ::
 */
TSequenceSet *
tgeompointseqset_tgeogpointseqset(const TSequenceSet *ss, bool oper)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    sequences[i] = tgeompointseq_tgeogpointseq(seq, oper);
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @ingroup libmeos_temporal_spatial_transf
 * @brief Convert a temporal point to a geometry/geography point.
 * @param[in] temp Temporal point
 * @param[in] oper True when transforming from geometry to geography,
 * false otherwise
 * @see tgeompointinst_tgeogpointinst
 * @see tgeompointseq_tgeogpointseq
 * @see tgeompointseqset_tgeogpointseqset
 * @sqlop ::
 */
Temporal *
tgeompoint_tgeogpoint(const Temporal *temp, bool oper)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tgeompointinst_tgeogpointinst((TInstant *) temp,
      oper);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tgeompointseq_tgeogpointseq((TSequence *) temp,
      oper);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tgeompointseqset_tgeogpointseqset(
      (TSequenceSet *) temp, oper);
  return result;
}

/*****************************************************************************
 * Functions for extracting coordinates
 *****************************************************************************/

/**
 * Get the X coordinates of a temporal point
 */
static Datum
point_get_x(Datum point)
{
  POINT4D p;
  datum_point4d(point, &p);
  return Float8GetDatum(p.x);
}

/**
 * Get the Y coordinates of a temporal point
 */
static Datum
point_get_y(Datum point)
{
  POINT4D p;
  datum_point4d(point, &p);
  return Float8GetDatum(p.y);
}

/**
 * Get the Z coordinates of a temporal point
 */
static Datum
point_get_z(Datum point)
{
  POINT4D p;
  datum_point4d(point, &p);
  return Float8GetDatum(p.z);
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Get one of the coordinates of a temporal point as a temporal float.
 *
 * @param[in] temp Temporal point
 * @param[in] coord Coordinate number where 0 = X, 1 = Y, 2 = Z
 * @sqlfunc getX(), getY(), getZ()
 */
Temporal *
tpoint_get_coord(const Temporal *temp, int coord)
{
  ensure_tgeo_type(temp->temptype);
  if (coord == 2)
    ensure_has_Z(temp->flags);
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  assert(coord >= 0 && coord <= 2);
  if (coord == 0)
    lfinfo.func = (varfunc) &point_get_x;
  else if (coord == 1)
    lfinfo.func = (varfunc) &point_get_y;
  else /* coord == 2 */
    lfinfo.func = (varfunc) &point_get_z;
  lfinfo.numparam = 0;
  lfinfo.restype = T_TFLOAT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  return result;
}

/*****************************************************************************
 * Length functions
 *****************************************************************************/

/**
 * Return the length traversed by a temporal sequence point with plannar
 * coordinates
 * @pre The temporal point has linear interpolation
 */
static double
tpointseq_length_2d(const TSequence *seq)
{
  double result = 0;
  Datum start = tinstant_value(tsequence_inst_n(seq, 0));
  const POINT2D *p1 = datum_point2d_p(start);
  for (int i = 1; i < seq->count; i++)
  {
    Datum end = tinstant_value(tsequence_inst_n(seq, i));
    const POINT2D *p2 = datum_point2d_p(end);
    result += sqrt( ((p1->x - p2->x) * (p1->x - p2->x)) +
      ((p1->y - p2->y) * (p1->y - p2->y)) );
    p1 = p2;
  }
  return result;
}

/**
 * Return the length traversed by a temporal sequence point with plannar
 * coordinates
 * @pre The temporal point has linear interpolation
 */
static double
tpointseq_length_3d(const TSequence *seq)
{
  double result = 0;
  Datum start = tinstant_value(tsequence_inst_n(seq, 0));
  const POINT3DZ *p1 = datum_point3dz_p(start);
  for (int i = 1; i < seq->count; i++)
  {
    Datum end = tinstant_value(tsequence_inst_n(seq, i));
    const POINT3DZ *p2 = datum_point3dz_p(end);
    result += sqrt( ((p1->x - p2->x)*(p1->x - p2->x)) +
      ((p1->y - p2->y)*(p1->y - p2->y)) +
      ((p1->z - p2->z)*(p1->z - p2->z)) );
    p1 = p2;
  }
  return result;
}

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return the length traversed by a temporal sequence point.
 * @sqlfunc length()
 */
double
tpointseq_length(const TSequence *seq)
{
  assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
  if (seq->count == 1)
    return 0;

  if (! MOBDB_FLAGS_GET_GEODETIC(seq->flags))
  {
    return MOBDB_FLAGS_GET_Z(seq->flags) ?
      tpointseq_length_3d(seq) : tpointseq_length_2d(seq);
  }
  else
  {
    /* We are sure that the trajectory is a line */
    GSERIALIZED *traj = tpointcontseq_trajectory(seq);
    double result = gserialized_geog_length(traj, true);
    pfree(traj);
    return result;
  }
}

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return the length traversed by a temporal sequence set point.
 * @sqlfunc length()
 */
double
tpointseqset_length(const TSequenceSet *ss)
{
  assert(MOBDB_FLAGS_GET_LINEAR(ss->flags));
  double result = 0;
  for (int i = 0; i < ss->count; i++)
    result += tpointseq_length(tsequenceset_seq_n(ss, i));
  return result;
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return the length traversed by a temporal sequence (set) point
 * @sqlfunc length()
 */
double
tpoint_length(const Temporal *temp)
{
  double result = 0.0;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT || ! MOBDB_FLAGS_GET_LINEAR(temp->flags))
    ;
  else if (temp->subtype == TSEQUENCE)
    result = tpointseq_length((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tpointseqset_length((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return the cumulative length traversed by a temporal point.
 * @pre The sequence has linear interpolation
 * @sqlfunc cumulativeLength()
 */
TSequence *
tpointseq_cumulative_length(const TSequence *seq, double prevlength)
{
  assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
  const TInstant *inst1;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = tsequence_inst_n(seq, 0);
    TInstant *inst = tinstant_make(Float8GetDatum(prevlength), T_TFLOAT,
      inst1->t);
    TSequence *result = tinstant_to_tsequence(inst, LINEAR);
    pfree(inst);
    return result;
  }

  /* General case */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  datum_func2 func = pt_distance_fn(seq->flags);
  inst1 = tsequence_inst_n(seq, 0);
  Datum value1 = tinstant_value(inst1);
  double length = prevlength;
  instants[0] = tinstant_make(Float8GetDatum(length), T_TFLOAT, inst1->t);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = tsequence_inst_n(seq, i);
    Datum value2 = tinstant_value(inst2);
    if (! datum_point_eq(value1, value2))
      length += DatumGetFloat8(func(value1, value2));
    instants[i] = tinstant_make(Float8GetDatum(length), T_TFLOAT, inst2->t);
    value1 = value2;
  }
  return tsequence_make_free(instants, seq->count, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, LINEAR, NORMALIZE);
}

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return the cumulative length traversed by a temporal point.
 * @sqlfunc cumulativeLength()
 */
TSequenceSet *
tpointseqset_cumulative_length(const TSequenceSet *ss)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  double length = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    sequences[i] = tpointseq_cumulative_length(seq, length);
    /* sequences[i] may have less sequences than seq->count due to normalization */
    const TInstant *end = tsequence_inst_n(sequences[i], sequences[i]->count - 1);
    length = DatumGetFloat8(tinstant_value(end));
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return the cumulative length traversed by a temporal point.
 * @sqlfunc cumulativeLength()
 */
Temporal *
tpoint_cumulative_length(const Temporal *temp)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT || ! MOBDB_FLAGS_GET_LINEAR(temp->flags))
    result = temporal_from_base(Float8GetDatum(0.0), T_TFLOAT, temp,
      MOBDB_FLAGS_GET_INTERP(temp->flags));
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tpointseq_cumulative_length((TSequence *) temp, 0);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tpointseqset_cumulative_length((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return the speed of a temporal point.
 * @pre The temporal point has linear interpolation
 * @sqlfunc speed()
 */
TSequence *
tpointseq_speed(const TSequence *seq)
{
  assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));

  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  /* General case */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  datum_func2 func = pt_distance_fn(seq->flags);
  const TInstant *inst1 = tsequence_inst_n(seq, 0);
  Datum value1 = tinstant_value(inst1);
  double speed;
  for (int i = 0; i < seq->count - 1; i++)
  {
    const TInstant *inst2 = tsequence_inst_n(seq, i + 1);
    Datum value2 = tinstant_value(inst2);
    speed = datum_point_eq(value1, value2) ? 0.0 :
      DatumGetFloat8(func(value1, value2)) /
        ((double)(inst2->t - inst1->t) / 1000000.0);
    instants[i] = tinstant_make(Float8GetDatum(speed), T_TFLOAT, inst1->t);
    inst1 = inst2;
    value1 = value2;
  }
  instants[seq->count - 1] = tinstant_make(Float8GetDatum(speed), T_TFLOAT,
    seq->period.upper);
  /* The resulting sequence has step interpolation */
  TSequence *result = tsequence_make((const TInstant **) instants, seq->count,
    seq->count, seq->period.lower_inc, seq->period.upper_inc, STEPWISE,
    NORMALIZE);
  pfree_array((void **) instants, seq->count - 1);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return the speed of a temporal point
 * @sqlfunc speed()
 */
TSequenceSet *
tpointseqset_speed(const TSequenceSet *ss)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  int k = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    if (seq->count > 1)
      sequences[k++] = tpointseq_speed(seq);
  }
  /* The resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return the speed of a temporal point
 * @sqlfunc speed()
 */
Temporal *
tpoint_speed(const Temporal *temp)
{
  Temporal *result = NULL;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT || ! MOBDB_FLAGS_GET_LINEAR(temp->flags))
    ;
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tpointseq_speed((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tpointseqset_speed((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Time-weighed centroid for temporal geometry points
 *****************************************************************************/

/**
 * @brief Split the temporal point sequence into temporal float sequences for
 * each of its coordinates
 */
void
tpointseq_twcentroid1(const TSequence *seq, bool hasz, interpType interp,
  TSequence **seqx, TSequence **seqy, TSequence **seqz)
{
  TInstant **instantsx = palloc(sizeof(TInstant *) * seq->count);
  TInstant **instantsy = palloc(sizeof(TInstant *) * seq->count);
  TInstant **instantsz = hasz ?
    palloc(sizeof(TInstant *) * seq->count) : NULL;

  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    POINT4D p;
    datum_point4d(tinstant_value(inst), &p);
    instantsx[i] = tinstant_make(Float8GetDatum(p.x), T_TFLOAT, inst->t);
    instantsy[i] = tinstant_make(Float8GetDatum(p.y), T_TFLOAT, inst->t);
    if (hasz)
      instantsz[i] = tinstant_make(Float8GetDatum(p.z), T_TFLOAT, inst->t);
  }
  *seqx = tsequence_make_free(instantsx, seq->count, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, interp, NORMALIZE);
  *seqy = tsequence_make_free(instantsy, seq->count, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, interp, NORMALIZE);
  if (hasz)
    *seqz = tsequence_make_free(instantsz, seq->count, seq->count,
      seq->period.lower_inc, seq->period.upper_inc, interp, NORMALIZE);
  return;
}

/**
 * @ingroup libmeos_int_temporal_agg
 * @brief Return the time-weighed centroid of a temporal geometry point.
 * @sqlfunc twCentroid()
 */
GSERIALIZED *
tpointseq_twcentroid(const TSequence *seq)
{
  int srid = tpointseq_srid(seq);
  bool hasz = MOBDB_FLAGS_GET_Z(seq->flags);
  interpType interp = MOBDB_FLAGS_GET_INTERP(seq->flags);
  TSequence *seqx, *seqy, *seqz;
  tpointseq_twcentroid1(seq, hasz, interp, &seqx, &seqy, &seqz);
  double twavgx = (interp == DISCRETE) ?
    tnumberdiscseq_twavg(seqx) : tnumbercontseq_twavg(seqx);
  double twavgy = (interp == DISCRETE) ?
    tnumberdiscseq_twavg(seqy) : tnumbercontseq_twavg(seqy);
  double twavgz = 0.0;
  if (hasz)
    twavgz = (interp == DISCRETE) ?
      tnumberdiscseq_twavg(seqz) : tnumbercontseq_twavg(seqz);
  GSERIALIZED *result = gspoint_make(twavgx, twavgy, twavgz, hasz, false, srid);
  pfree(seqx); pfree(seqy);
  if (hasz)
    pfree(seqz);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_agg
 * @brief Return the time-weighed centroid of a temporal geometry point.
 * @sqlfunc twCentroid()
 */
GSERIALIZED *
tpointseqset_twcentroid(const TSequenceSet *ss)
{
  int srid = tpointseqset_srid(ss);
  bool hasz = MOBDB_FLAGS_GET_Z(ss->flags);
  interpType interp = MOBDB_FLAGS_GET_INTERP(ss->flags);
  TSequence **sequencesx = palloc(sizeof(TSequence *) * ss->count);
  TSequence **sequencesy = palloc(sizeof(TSequence *) * ss->count);
  TSequence **sequencesz = hasz ?
    palloc(sizeof(TSequence *) * ss->count) : NULL;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    tpointseq_twcentroid1(seq, hasz, interp, &sequencesx[i], &sequencesy[i],
      &sequencesz[i]);
  }
  TSequenceSet *tsx = tsequenceset_make_free(sequencesx, ss->count, NORMALIZE);
  TSequenceSet *tsy = tsequenceset_make_free(sequencesy, ss->count, NORMALIZE);
  TSequenceSet *tsz = hasz ?
    tsequenceset_make_free(sequencesz, ss->count, NORMALIZE) : NULL;

  double twavgx = tnumberseqset_twavg(tsx);
  double twavgy = tnumberseqset_twavg(tsy);
  double twavgz = hasz ? tnumberseqset_twavg(tsz) : 0;
  GSERIALIZED *result = gspoint_make(twavgx, twavgy, twavgz, hasz, false, srid);
  pfree(tsx); pfree(tsy);
  if (hasz)
    pfree(tsz);
  return result;
}

/**
 * @ingroup libmeos_temporal_agg
 * @brief Return the time-weighed centroid of a temporal geometry point.
 * @sqlfunc twCentroid()
 */
GSERIALIZED *
tpoint_twcentroid(const Temporal *temp)
{
  GSERIALIZED *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = DatumGetGserializedP(tinstant_value_copy((TInstant *) temp));
  else if (temp->subtype == TSEQUENCE)
    result = tpointseq_twcentroid((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tpointseqset_twcentroid((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

/**
 * Return the azimuth of the two geometry points
 */
static Datum
geom_azimuth(Datum geom1, Datum geom2)
{
  const POINT2D *p1 = datum_point2d_p(geom1);
  const POINT2D *p2 = datum_point2d_p(geom2);
  double result;
  azimuth_pt_pt(p1, p2, &result);
  return Float8GetDatum(result);
}

/**
 * Return the azimuth the two geography points
 */
static Datum
geog_azimuth(Datum geog1, Datum geog2)
{
  const GSERIALIZED *g1 = DatumGetGserializedP(geog1);
  const GSERIALIZED *g2 = DatumGetGserializedP(geog2);
  const LWGEOM *lwgeom1 = lwgeom_from_gserialized(g1);
  const LWGEOM *lwgeom2 = lwgeom_from_gserialized(g2);

  SPHEROID s;
  spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);
  double result = lwgeom_azumith_spheroid(lwgeom_as_lwpoint(lwgeom1),
    lwgeom_as_lwpoint(lwgeom2), &s);
  return Float8GetDatum(result);
}

/**
 * Return the temporal azimuth of a temporal geometry point.
 *
 * @param[in] seq Temporal value
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 */
static int
tpointseq_azimuth1(const TSequence *seq, TSequence **result)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return 0;

  /* Determine the PostGIS function to call */
  datum_func2 func = MOBDB_FLAGS_GET_GEODETIC(seq->flags) ?
    &geog_azimuth : &geom_azimuth;

  /* We are sure that there are at least 2 instants */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  const TInstant *inst1 = tsequence_inst_n(seq, 0);
  Datum value1 = tinstant_value(inst1);
  int k = 0, l = 0;
  Datum azimuth = 0; /* Make the compiler quiet */
  bool lower_inc = seq->period.lower_inc, upper_inc;
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = tsequence_inst_n(seq, i);
    Datum value2 = tinstant_value(inst2);
    upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    if (! datum_point_eq(value1, value2))
    {
      azimuth = func(value1, value2);
      instants[k++] = tinstant_make(azimuth, T_TFLOAT, inst1->t);
    }
    else
    {
      if (k != 0)
      {
        instants[k++] = tinstant_make(azimuth, T_TFLOAT, inst1->t);
        upper_inc = true;
        /* Resulting sequence has step interpolation */
        result[l++] = tsequence_make((const TInstant **) instants, k, k,
          lower_inc, upper_inc, STEPWISE, NORMALIZE);
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
    instants[k++] = tinstant_make(azimuth, T_TFLOAT, inst1->t);
    /* Resulting sequence has step interpolation */
    result[l++] = tsequence_make((const TInstant **) instants, k, k,
      lower_inc, upper_inc, STEPWISE, NORMALIZE);
  }

  pfree(instants);
  return l;
}

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return the temporal azimuth of a temporal geometry point.
 * @sqlfunc azimuth()
 */
TSequenceSet *
tpointseq_azimuth(const TSequence *seq)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count);
  int count = tpointseq_azimuth1(seq, sequences);
  /* Resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return the temporal azimuth of a temporal geometry point.
 * @sqlfunc azimuth()
 */
TSequenceSet *
tpointseqset_azimuth(const TSequenceSet *ss)
{
  if (ss->count == 1)
    return tpointseq_azimuth(tsequenceset_seq_n(ss, 0));

  TSequence **sequences = palloc(sizeof(TSequence *) * ss->totalcount);
  int k = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    k += tpointseq_azimuth1(seq, &sequences[k]);
  }
  /* Resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return the temporal azimuth of a temporal geometry point.
 * @sqlfunc azimuth()
 */
Temporal *
tpoint_azimuth(const Temporal *temp)
{
  Temporal *result = NULL;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT || ! MOBDB_FLAGS_GET_LINEAR(temp->flags))
    ;
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tpointseq_azimuth((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tpointseqset_azimuth((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Temporal bearing
 *****************************************************************************/

/**
 * Normalize the bearing from -180° to + 180° (in radians) to
 * 0° to 360° (in radians)
 */
static double
alpha(const POINT2D *p1, const POINT2D *p2)
{
  if (p1->x <= p2->x && p1->y <= p2->y)
    return 0.0;
  if ((p1->x < p2->x && p1->y > p2->y) ||
      (p1->x >= p2->x && p1->y > p2->y))
    return M_PI;
  else /* p1->x > p2->x && p1->y <= p2->y */
    return M_PI * 2.0;
}

/**
 * Computes the bearing between two geometric points
 */
static Datum
geom_bearing(Datum point1, Datum point2)
{
  const POINT2D *p1 = datum_point2d_p(point1);
  const POINT2D *p2 = datum_point2d_p(point2);
  if ((fabs(p1->x - p2->x) <= MOBDB_EPSILON) &&
      (fabs(p1->y - p2->y) <= MOBDB_EPSILON))
    return 0.0;
  if (fabs(p1->y - p2->y) > MOBDB_EPSILON)
  {
    double bearing = pg_datan((p1->x - p2->x) / (p1->y - p2->y)) +
      alpha(p1, p2);
    if (fabs(bearing) <= MOBDB_EPSILON)
      bearing = 0.0;
    return Float8GetDatum(bearing);
  }
  if (p1->x < p2->x)
    return Float8GetDatum(M_PI / 2.0);
  else
    return Float8GetDatum(M_PI * 3.0 / 2.0);
}

/**
 * Compute the bearing between two geographic points
 * Derived from https://gist.github.com/jeromer/2005586
 *
 * N.B. In PostGIS, for geodetic coordinates, X is longitude and Y is latitude
 * The formulae used is the following:
 *   lat  = sin(Δlong).cos(lat2)
 *   long = cos(lat1).sin(lat2) - sin(lat1).cos(lat2).cos(Δlong)
 *   θ    = atan2(lat, long)
 */
static Datum
geog_bearing(Datum point1, Datum point2)
{
  const POINT2D *p1 = datum_point2d_p(point1);
  const POINT2D *p2 = datum_point2d_p(point2);
  if ((fabs(p1->x - p2->x) <= MOBDB_EPSILON) &&
      (fabs(p1->y - p2->y) <= MOBDB_EPSILON))
    return 0.0;

  double lat1 = float8_mul(p1->y, RADIANS_PER_DEGREE);
  double lat2 = float8_mul(p2->y, RADIANS_PER_DEGREE);
  double diffLong = float8_mul(p2->x - p1->x, RADIANS_PER_DEGREE);
  double lat = pg_dsin(diffLong) * pg_dcos(lat2);
  double lgt = ( pg_dcos(lat1) * pg_dsin(lat2) ) -
    ( pg_dsin(lat1) * pg_dcos(lat2) * pg_dcos(diffLong) );
  /* Notice that the arguments are inverted, e.g., wrt the atan2 in Python */
  double initial_bearing = pg_datan2(lat, lgt);
  /* Normalize the bearing from -180° to + 180° (in radians) to
   * 0° to 360° (in radians) */
  double bearing = fmod(initial_bearing + M_PI * 2.0, M_PI * 2.0);
  return Float8GetDatum(bearing);
}

/**
 * Select the appropriate bearing function
 */
static datum_func2
get_bearing_fn(int16 flags)
{
  datum_func2 result;
  if (MOBDB_FLAGS_GET_GEODETIC(flags))
    result = &geog_bearing;
  else
    result = &geom_bearing;
  return result;
}

/**
 * Return the value and timestamp at which the a temporal point segment
 * and a point are at the minimum bearing
 *
 * @param[in] start,end Instants defining the segment
 * @param[in] point Geometric/geographic point
 * @param[in] basetypid Base type
 * @param[out] value Value
 * @param[out] t Timestamp
 * @pre The segment is not constant and has linear interpolation
 * @note The parameter basetype is not needed for temporal points
 */
static bool
tpoint_geo_min_bearing_at_timestamp(const TInstant *start, const TInstant *end,
  Datum point, mobdbType basetypid __attribute__((unused)), Datum *value,
  TimestampTz *t)
{
  Datum dstart = tinstant_value(start);
  Datum dend = tinstant_value(end);
  const POINT2D *p1 = datum_point2d_p(dstart);
  const POINT2D *p2 = datum_point2d_p(dend);
  const POINT2D *p = datum_point2d_p(point);
  const POINT2D *q;
  long double fraction;
  Datum proj;
  bool geodetic = MOBDB_FLAGS_GET_GEODETIC(start->flags);
  if (geodetic)
  {
    GEOGRAPHIC_EDGE e, e1;
    GEOGRAPHIC_POINT gp, inter;
    geographic_point_init(p->x, p->y, &gp);
    geographic_point_init(p1->x, p1->y, &(e.start));
    geographic_point_init(p2->x, p2->y, &(e.end));
    if (! edge_contains_coplanar_point(&e, &gp))
      return false;
    /* Create an edge in the same meridian as p */
    geographic_point_init(p->x, 89.999999, &(e1.start));
    geographic_point_init(p->x, -89.999999, &(e1.end));
    edge_intersection(&e, &e1, &inter);
    proj = PointerGetDatum(gspoint_make(rad2deg(inter.lon), rad2deg(inter.lat),
      0, false, true, tpointinst_srid(start)));
    fraction = geosegm_locate_point(dstart, dend, proj, NULL);
  }
  else
  {
    bool ds = (p1->x - p->x) > 0;
    bool de = (p2->x - p->x) > 0;
    /* If there is not a North passage */
    if (ds == de)
      return false;
    fraction = (long double)(p->x - p1->x) / (long double)(p2->x - p1->x);
  }
  if (fraction <= MOBDB_EPSILON || fraction >= (1.0 - MOBDB_EPSILON))
    return false;
  long double duration = (end->t - start->t);
  *t = start->t + (TimestampTz) (duration * fraction);
  *value = (Datum) 0;
  /* Compute the projected value only for geometries */
  if (! geodetic)
    proj = tsegment_value_at_timestamp(start, end, true, *t);
  q = datum_point2d_p(proj);
  /* We add a turning point only if p is to the North of q */
  return FP_GTEQ(p->y, q->y) ? true : false;
}

/**
 * Return the value and timestamp at which the two temporal point segments
 * are at the minimum bearing
 *
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[out] value Value
 * @param[out] t Timestamp
 * @pre The segments are not both constants and are both linear
 * @note This function is currently not available for two temporal geographic
 * points
 */
static bool
tpointsegm_min_bearing_at_timestamp(const TInstant *start1,
  const TInstant *end1, const TInstant *start2,
  const TInstant *end2, Datum *value, TimestampTz *t)
{
  assert(!MOBDB_FLAGS_GET_GEODETIC(start1->flags));
  const POINT2D *sp1 = datum_point2d_p(tinstant_value(start1));
  const POINT2D *ep1 = datum_point2d_p(tinstant_value(end1));
  const POINT2D *sp2 = datum_point2d_p(tinstant_value(start2));
  const POINT2D *ep2 = datum_point2d_p(tinstant_value(end2));
  /* It there is a North passage we call the function
    tgeompoint_min_dist_at_timestamp */
  bool ds = (sp1->x - sp2->x) > 0;
  bool de = (ep1->x - ep2->x) > 0;
  if (ds == de)
    return false;

  /*
   * Compute the instants t1 and t2 at which the linear functions of the two
   * segments take the value 0: at1 + b = 0, ct2 + d = 0. There is a
   * minimum/maximum exactly at the middle between t1 and t2.
   * To reduce problems related to floating point arithmetic, t1 and t2
   * are shifted, respectively, to 0 and 1 before the computation
   * N.B. The code that follows is adapted from the function
   * tnumber_arithop_tp_at_timestamp1 in file tnumber_mathfuncs.c
   */
  if ((ep1->x - sp1->x) == 0.0 || (ep2->x - sp2->x) == 0.0)
    return false;

  long double d1 = (-1 * sp1->x) / (ep1->x - sp1->x);
  long double d2 = (-1 * sp2->x) / (ep2->x - sp2->x);
  long double min = Min(d1, d2);
  long double max = Max(d1, d2);
  long double fraction = min + (max - min)/2;
  long double duration = (long double) (end1->t - start1->t);
  if (fraction <= MOBDB_EPSILON || fraction >= (1.0 - MOBDB_EPSILON))
    /* Minimum/maximum occurs out of the period */
    return false;

  *t = start1->t + (TimestampTz) (duration * fraction);
  /* We need to verify that at timestamp t the first segment is to the
   * North of the second */
  Datum value1 = tsegment_value_at_timestamp(start1, end1, true, *t);
  Datum value2 = tsegment_value_at_timestamp(start2, end2, true, *t);
  sp1 = datum_point2d_p(value1);
  sp2 = datum_point2d_p(value2);
  if (sp1->y > sp2->y) // TODO Use MOBDB_EPSILON
    return false;
 /* We know that the bearing is 0 */
  if (value)
    *value = Float8GetDatum(0.0);
  return true;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return the temporal bearing between two geometry/geography points
 * @note The following function could be included in PostGIS one day
 * @sqlfunc bearing()
 */
bool
bearing_point_point(const GSERIALIZED *geo1, const GSERIALIZED *geo2,
  double *result)
{
  ensure_point_type(geo1); ensure_point_type(geo2);
  ensure_same_srid(gserialized_get_srid(geo1), gserialized_get_srid(geo2));
  ensure_same_dimensionality_gs(geo1, geo2);
  if (gserialized_is_empty(geo1) || gserialized_is_empty(geo2))
    return false;
  *result = FLAGS_GET_GEODETIC(geo1->gflags) ?
    DatumGetFloat8(geog_bearing(PointerGetDatum(geo1), PointerGetDatum(geo2))) :
    DatumGetFloat8(geom_bearing(PointerGetDatum(geo1), PointerGetDatum(geo2)));
  return true;
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return the temporal bearing between a temporal point and a
 * geometry/geography point.
 * @sqlfunc bearing()
 */
Temporal *
bearing_tpoint_point(const Temporal *temp, const GSERIALIZED *gs, bool invert)
{
  if (gserialized_is_empty(gs))
    return NULL;
  ensure_point_type(gs);
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
  ensure_same_dimensionality_tpoint_gs(temp, gs);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) get_bearing_fn(temp->flags);
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = lfinfo.argtype[1] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = MOBDB_FLAGS_GET_LINEAR(temp->flags);
  lfinfo.invert = invert;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = &tpoint_geo_min_bearing_at_timestamp;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal_base(temp, PointerGetDatum(gs), &lfinfo);
  return result;
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return the temporal bearing between two temporal points
 * @sqlfunc bearing()
 */
Temporal *
bearing_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  ensure_same_srid(tpoint_srid(temp1), tpoint_srid(temp2));
  ensure_same_dimensionality(temp1->flags, temp2->flags);
  datum_func2 func = get_bearing_fn(temp1->flags);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = temptype_basetype(temp1->temptype);
  lfinfo.argtype[1] = temptype_basetype(temp2->temptype);
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = MOBDB_FLAGS_GET_LINEAR(temp1->flags) ||
    MOBDB_FLAGS_GET_LINEAR(temp2->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = lfinfo.reslinear ?
    &tpointsegm_min_bearing_at_timestamp : NULL;
  Temporal *result = tfunc_temporal_temporal(temp1, temp2, &lfinfo);
  return result;
}

/*****************************************************************************
 * Functions computing the intersection of two segments derived from
 * http://www.science.smith.edu/~jorourke/books/ftp.html
 *****************************************************************************/

/*
 * The possible ways a pair of segments can interact.
 * Returned by the function seg2d_intersection
 */
enum
{
  MOBDB_SEG_NO_INTERSECTION,  /* Segments do not intersect */
  MOBDB_SEG_OVERLAP,          /* Segments overlap */
  MOBDB_SEG_CROSS,            /* Segments cross */
  MOBDB_SEG_TOUCH,            /* Segments touch in a vertex */
} MOBDB_SEG_INTER_TYPE;

/**
 * Finds the UNIQUE point of intersection p between two closed
 * collinear segments ab and cd. Return p and a MOBDB_SEG_INTER_TYPE value.
 * Notice that if the segments overlap no point is returned since they
 * can be an infinite number of them.
 * This function is called after verifying that the points
 * are collinear and that their bounding boxes intersect.
 */
static int
parseg2d_intersection(const POINT2D a, const POINT2D b, const POINT2D c,
  const POINT2D d, POINT2D *p)
{
  /* Compute the intersection of the bounding boxes */
  double xmin = Max(Min(a.x, b.x), Min(c.x, d.x));
  double xmax = Min(Max(a.x, b.x), Max(c.x, d.x));
  double ymin = Max(Min(a.y, b.y), Min(c.y, d.y));
  double ymax = Min(Max(a.y, b.y), Max(c.y, d.y));
  /* If the intersection of the bounding boxes is not a point */
  if (xmin < xmax || ymin < ymax )
    return MOBDB_SEG_OVERLAP;
  /* We are sure that the segments touch each other */
  if ((b.x == c.x && b.y == c.y) ||
      (b.x == d.x && b.y == d.y))
  {
    p->x = b.x;
    p->y = b.y;
    return MOBDB_SEG_TOUCH;
  }
  if ((a.x == c.x && a.y == c.y) ||
      (a.x == d.x && a.y == d.y))
  {
    p->x = a.x;
    p->y = a.y;
    return MOBDB_SEG_TOUCH;
  }
  /* We should never arrive here since this function is called after verifying
   * that the bounding boxes of the segments intersect */
  return MOBDB_SEG_NO_INTERSECTION;
}

/**
 * Finds the UNIQUE point of intersection p between two closed segments
 * ab and cd. Return p and a MOBDB_SEG_INTER_TYPE value. Notice that if the
 * segments overlap no point is returned since they can be an infinite
 * number of them
 */
static int
seg2d_intersection(const POINT2D a, const POINT2D b, const POINT2D c,
  const POINT2D d, POINT2D *p)
{
  double s, t;        /* The two parameters of the parametric equations */
  double num, denom;  /* Numerator and denominator of equations */
  int result;         /* Return value characterizing the intersection */

  denom = a.x * (d.y - c.y) + b.x * (c.y - d.y) +
          d.x * (b.y - a.y) + c.x * (a.y - b.y);

  /* If denom is zero, then segments are parallel: handle separately */
  if (fabs(denom) < MOBDB_EPSILON)
    return parseg2d_intersection(a, b, c, d, p);

  num = a.x * (d.y - c.y) + c.x * (a.y - d.y) + d.x * (c.y - a.y);
  s = num / denom;

  num = -(a.x * (c.y - b.y) + b.x * (a.y - c.y) + c.x * (b.y - a.y));
  t = num / denom;

  if ((0.0 == s || s == 1.0) && (0.0 == t || t == 1.0))
   result = MOBDB_SEG_TOUCH;
  else if (0.0 <= s && s <= 1.0 && 0.0 <= t && t <= 1.0)
   result = MOBDB_SEG_CROSS;
  else
   result = MOBDB_SEG_NO_INTERSECTION;

  p->x = a.x + s * (b.x - a.x);
  p->y = a.y + s * (b.y - a.y);

  return result;
}

/*****************************************************************************
 * Non self-intersecting (a.k.a. simple) functions
 *****************************************************************************/

/**
 * Split a temporal point sequence with discrete or stepwise interpolation
 * into an array of non self-intersecting pieces
 *
 * @param[in] seq Temporal point
 * @param[out] count Number of elements in the resulting array
 * @result Boolean array determining the instant numbers at which the
 * discrete sequence must be split
 * @pre The temporal point has at least 2 instants
 */
static bool *
tpointseq_discstep_find_splits(const TSequence *seq, int *count)
{
  assert(! MOBDB_FLAGS_GET_LINEAR(seq->flags));
  int count1 = seq->count;
  assert(count1 > 1);
  /* bitarr is an array of bool for collecting the splits */
  bool *bitarr = palloc0(sizeof(bool) * count1);
  int numsplits = 0;
  int start = 0, end = count1 - 1;
  while (start < count1 - 1)
  {
    const TInstant *inst1 = tsequence_inst_n(seq, start);
    Datum value1 = tinstant_value(inst1);
    /* Find intersections in the piece defined by start and end in a
     * breadth-first search */
    int j = start, k = start + 1;
    while (k <= end)
    {
      const TInstant *inst2 = tsequence_inst_n(seq, k);
      Datum value2 = tinstant_value(inst2);
      if (datum_point_eq(value1, value2))
      {
        /* Set the new end */
        end = k;
        bitarr[end] = true;
        numsplits++;
        break;
      }
      if (j < k - 1)
        j++;
      else
      {
        k++;
        j = start;
      }
    }
    /* Process the next split */
    start = end;
  }
  *count = numsplits;
  return bitarr;
}

/**
 * Split a temporal point sequence with linear interpolation into an array
 * of non self-intersecting pieces. The function works only on 2D even if
 * the input points are in 3D
 *
 * @param[in] seq Temporal point
 * @param[out] count Number of elements in the resulting array
 * @result Boolean array determining the instant numbers at which the
 * sequence must be split
 * @pre The input sequence has at least 3 instants
 */
static bool *
tpointseq_linear_find_splits(const TSequence *seq, int *count)
{
  assert(seq->count >= 2);
 /* points is an array of points in the sequence */
  POINT2D *points = palloc0(sizeof(POINT2D) * seq->count);
  /* bitarr is an array of bool for collecting the splits */
  bool *bitarr = palloc0(sizeof(bool) * seq->count);
  points[0] = datum_point2d(tinstant_value(tsequence_inst_n(seq, 0)));
  int numsplits = 0;
  for (int i = 1; i < seq->count; i++)
  {
    points[i] = datum_point2d(tinstant_value(tsequence_inst_n(seq, i)));
    /* If stationary segment we need to split the sequence */
    if (points[i - 1].x == points[i].x && points[i - 1].y == points[i].y)
    {
      if (i > 1)
      {
        bitarr[i - 1] = true;
        numsplits++;
      }
      if (i < seq->count - 1)
      {
        bitarr[i] = true;
        numsplits++;
      }
    }
  }

  /* Loop for every split due to stationary segments while adding
   * additional splits due to intersecting segments */
  int start = 0;
  while (start < seq->count - 2)
  {
    int end = start + 1;
    while (end < seq->count - 1 && ! bitarr[end])
      end++;
    if (end == start + 1)
    {
      start = end;
      continue;
    }
    /* Find intersections in the piece defined by start and end in a
     * breadth-first search */
    int i = start, j = start + 1;
    while (j < end)
    {
      /* If the bounding boxes of the segments intersect */
      if (lw_seg_interact(points[i], points[i + 1],
          points[j], points[j + 1]))
      {
        /* Candidate for intersection */
        POINT2D p;
        int intertype = seg2d_intersection(points[i], points[i + 1],
          points[j], points[j + 1], &p);
        if (intertype > 0 &&
          /* Exclude the case when two consecutive segments that
           * necessarily touch each other in their common point */
          (intertype != MOBDB_SEG_TOUCH || j != i + 1 ||
           p.x != points[j].x || p.y != points[j].y))
        {
          /* Set the new end */
          end = j;
          bitarr[end] = true;
          numsplits++;
          break;
        }
      }
      if (i < j - 1)
        i++;
      else
      {
        j++;
        i = start;
      }
    }
    /* Process the next split */
    start = end;
  }
  pfree(points);
  *count = numsplits;
  return bitarr;
}

/*****************************************************************************
 * Functions for testing whether a temporal point is simple and for spliting
 * a temporal point into an array of temporal points that are simple.
 * A temporal point is simple if all its components are non self-intersecting.
 * - a temporal instant point is simple
 * - a temporal discrete sequence point is simple if it is non self-intersecting
 * - a temporal sequence point is simple if it is non self-intersecting and
 *   do not have stationary segments
 * - a temporal sequence set point is simple if every composing sequence is
 *   simple even if two composing sequences intersect
 *****************************************************************************/

/**
 * Return true if a temporal point does not self-intersect.
 *
 * @param[in] seq Temporal point
 * @param[in] count Number of instants of the temporal point
 * @pre The temporal point sequence has discrete or stepwise interpolation
 */
static bool
tpointseq_discstep_is_simple(const TSequence *seq, int count)
{
  assert(count > 1);
  Datum *points = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    points[i] = tinstant_value(inst);
  }
  datumarr_sort(points, count, temptype_basetype(seq->temptype));
  bool found = false;
  for (int i = 1; i < count; i++)
  {
    if (datum_point_eq(points[i - 1], points[i]))
    {
      found = true;
      break;
    }
  }
  pfree(points);
  return ! found;
}

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return true if a temporal point does not self-intersect.
 *
 * @param[in] seq Temporal point
 * @sqlfunc isSimple();
 */
bool
tpointseq_is_simple(const TSequence *seq)
{
  if (seq->count == 1)
    return true;

  if (! MOBDB_FLAGS_GET_LINEAR(seq->flags))
    return tpointseq_discstep_is_simple(seq, seq->count);

  int numsplits;
  bool *splits = tpointseq_linear_find_splits(seq, &numsplits);
  pfree(splits);
  return numsplits == 0;
}

/**
 * @ingroup libmeos_int_temporal_spatial_accessor
 * @brief Return true if a temporal point does not self-intersect.
 *
 * @param[in] ss Temporal point
 * @sqlfunc isSimple()
 */
bool
tpointseqset_is_simple(const TSequenceSet *ss)
{
  bool result = true;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    result &= tpointseq_is_simple(seq);
    if (! result)
      break;
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return true if a temporal point does not self-intersect.
 * @sqlfunc isSimple()
 */
bool
tpoint_is_simple(const Temporal *temp)
{
  bool result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = true;
  else if (temp->subtype == TSEQUENCE)
    result = tpointseq_is_simple((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tpointseqset_is_simple((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************/

/**
 * Split a temporal discrete sequence point into an array of non self-intersecting
 * pieces.
 *
 * @param[in] seq Temporal point
 * @param[in] splits Bool array stating the splits
 * @param[in] count Number of elements in the resulting array
 * @pre The discrete sequence has at least two instants
 */
static TSequence **
tpointdiscseq_split(const TSequence *seq, bool *splits, int count)
{
  assert(seq->count > 1);
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  TSequence **result = palloc(sizeof(TSequence *) * count);
  /* Create the splits */
  int start = 0, k = 0;
  while (start < seq->count)
  {
    int end = start + 1;
    while (end < seq->count && ! splits[end])
      end++;
    /* Construct piece from start to end */
    for (int j = 0; j < end - start; j++)
      instants[j] = tsequence_inst_n(seq, j + start);
    result[k++] = tsequence_make(instants, end - start, end - start,
      true, true, DISCRETE, NORMALIZE_NO);
    /* Continue with the next split */
    start = end;
  }
  pfree(instants);
  return result;
}

/**
 * Split a temporal point into an array of non self-intersecting pieces
 *
 * @param[in] seq Temporal sequence point
 * @param[in] splits Bool array stating the splits
 * @param[in] count Number of elements in the resulting array
 * @note This function is called for each sequence of a sequence set
 */
static TSequence **
tpointcontseq_split(const TSequence *seq, bool *splits, int count)
{
  assert(seq->count > 2);
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  TSequence **result = palloc(sizeof(TSequence *) * count);
  /* Create the splits */
  int start = 0, k = 0;
  while (start < seq->count - 1)
  {
    int end = start + 1;
    while (end < seq->count - 1 && ! splits[end])
      end++;
    /* Construct piece from start to end inclusive */
    for (int j = 0; j <= end - start; j++)
      instants[j] = (TInstant *) tsequence_inst_n(seq, j + start);
    bool lower_inc1 = (start == 0) ? seq->period.lower_inc : true;
    bool upper_inc1 = (end == seq->count - 1) ?
      seq->period.upper_inc && ! splits[seq->count - 1] : false;
    /* The last two values of sequences with step interpolation and
     * exclusive upper bound must be equal */
    bool tofree = false;
    if (! linear &&
      ! datum_point_eq(tinstant_value(instants[end - start - 1]),
      tinstant_value(instants[end - start])))
    {
      Datum value = tinstant_value(instants[end - start - 1]);
      TimestampTz t = tsequence_inst_n(seq, end - start)->t;
      instants[end - start] = tinstant_make(value, seq->temptype, t);
      tofree = true;
      upper_inc1 = false;
    }
    result[k++] = tsequence_make((const TInstant **) instants, end - start + 1,
      end - start + 1, lower_inc1, upper_inc1, linear ? LINEAR :
      STEPWISE, NORMALIZE_NO);
    if (tofree)
    {
      /* Free the last instant created for the step interpolation */
      pfree(instants[end - start]);
      tofree = false;
    }
    /* Continue with the next split */
    start = end;
  }
  if (seq->period.upper_inc && splits[seq->count - 1])
  {
    /* Construct piece containing last instant of sequence */
    instants[0] = (TInstant *) tsequence_inst_n(seq, seq->count - 1);
    result[k++] = tsequence_make((const TInstant **) instants,
      seq->count - start, seq->count - start, true, seq->period.upper_inc,
      linear, NORMALIZE_NO);
  }
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_spatial_transf
 * @brief Split a temporal sequence point into an array of non
 * self-intersecting pieces.
 *
 * @param[in] seq Temporal sequence point
 * @param[out] count Number of elements in the resulting array
 * @note This function is called for each sequence of a sequence set
 * @sqlfunc makeSimple()
 */
TSequence **
tpointseq_make_simple(const TSequence *seq, int *count)
{
  interpType interp = MOBDB_FLAGS_GET_INTERP(seq->flags);
  TSequence **result;
  /* Special cases when the input sequence has 1 or 2 instants */
  if ((interp == DISCRETE && seq->count == 1) ||
      (interp != DISCRETE && seq->count <= 2))
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_copy(seq);
    *count = 1;
    return result;
  }

  int numsplits;
  bool *splits = (interp == LINEAR) ?
    tpointseq_linear_find_splits(seq, &numsplits) :
    tpointseq_discstep_find_splits(seq, &numsplits);
  if (numsplits == 0)
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_copy(seq);
    pfree(splits);
    *count = 1;
    return result;
  }

  result = (interp == DISCRETE) ?
    tpointdiscseq_split(seq, splits, numsplits + 1) :
    tpointcontseq_split(seq, splits, numsplits + 1);
  pfree(splits);
  *count = numsplits + 1;
  return result;
}

/**
 * @ingroup libmeos_int_temporal_spatial_transf
 * @brief Split a temporal sequence set point into an array of non
 * self-intersecting pieces.
 *
 * @param[in] ss Temporal sequence set point
 * @param[out] count Number of elements in the output array
 * @sqlfunc makeSimple()
 */
TSequence **
tpointseqset_make_simple(const TSequenceSet *ss, int *count)
{
  /* Singleton sequence set */
  if (ss->count == 1)
    return tpointseq_make_simple(tsequenceset_seq_n(ss, 0), count);

  /* General case */
  TSequence ***sequences = palloc0(sizeof(TSequence **) * ss->count);
  int *countseqs = palloc0(sizeof(int) * ss->count);
  int totalcount = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    sequences[i] = tpointseq_make_simple(seq, &countseqs[i]);
    totalcount += countseqs[i];
  }
  assert(totalcount > 0);
  *count = totalcount;
  return tseqarr2_to_tseqarr(sequences, countseqs, ss->count, totalcount);
}

/**
 * @ingroup libmeos_temporal_spatial_transf
 * @brief Split a temporal point into an array of non self-intersecting pieces.
 *
 * @param[in] temp Temporal point
 * @param[out] count Number of elements in the output array
 * @see tpointseq_make_simple
 * @see tpointseqset_make_simple
 * @sqlfunc makeSimple()
 */
Temporal **
tpoint_make_simple(const Temporal *temp, int *count)
{
  Temporal **result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
  {
    result = palloc0(sizeof(TInstant *));
    result[0] = (Temporal *) tinstant_copy((TInstant *) temp);
    *count = 1;
  }
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal **) tpointseq_make_simple((TSequence *) temp, count);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal **) tpointseqset_make_simple((TSequenceSet *) temp, count);
  return result;
}

/*****************************************************************************
 * Restriction functions
 * N.B. In the current PostGIS version there is no true ST_Intersection
 * function for geography, it is implemented as ST_DWithin with tolerance 0
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal point instant to (the complement of) a geometry.
 *
 * @param[in] inst Temporal instant point
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is at, false for minus
 * @pre The arguments are of the same dimensionality, have the same SRID,
 * and the geometry is not empty. This is verified in #tpoint_restrict_geometry
 * @sqlfunc atGeometry(), minusGeometry()
 */
TInstant *
tpointinst_restrict_geometry(const TInstant *inst, const GSERIALIZED *gs,
  bool atfunc)
{
  bool inter = DatumGetBool(geom_intersects2d(tinstant_value(inst),
    PointerGetDatum(gs)));
  if ((atfunc && !inter) || (!atfunc && inter))
    return NULL;
  return tinstant_copy(inst);
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal point discrete sequence to (the complement of) a
 * geometry.
 *
 * @param[in] seq Temporal discrete sequence point
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is at, false for minus
 * @pre The arguments are of the same dimensionality, have the same SRID,
 * and the geometry is not empty. This is verified in #tpoint_restrict_geometry
 * @sqlfunc atGeometry(), minusGeometry()
 */
TSequence *
tpointdiscseq_restrict_geometry(const TSequence *seq, const GSERIALIZED *gs,
  bool atfunc)
{
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int k = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    bool inter = DatumGetBool(geom_intersects2d(tinstant_value(inst),
      PointerGetDatum(gs)));
    if ((atfunc && inter) || (!atfunc && !inter))
      instants[k++] = inst;
  }
  TSequence *result = NULL;
  if (k != 0)
    result = tsequence_make(instants, k, k, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * Get the intersection points
 */
static int
gsinter_get_points(Datum *result, GSERIALIZED *gsinter)
{
  int gstype = gserialized_get_type(gsinter);
  /* The gserialized is of point or multipoint type */
  assert(gstype == POINTTYPE || gstype == MULTIPOINTTYPE);
  int k = 0;
  if (gstype == POINTTYPE)
  {
    result[k++] = PointerGetDatum(gserialized_copy(gsinter));
  }
  else
  /* It is a collection of type MULTIPOINTTYPE */
  {
    LWGEOM *lwinter = lwgeom_from_gserialized(gsinter);
    LWCOLLECTION *coll = lwgeom_as_lwcollection(lwinter);
    int countinter = coll->ngeoms;
    for (int i = 0; i < countinter; i++)
    {
      /* Find the i-th intersection */
      result[k++] = PointerGetDatum(geo_serialize(coll->geoms[i]));
    }
    lwgeom_free(lwinter);
  }
  return k;
}

/**
 * Restrict a temporal sequence point with step interpolation to a geometry
 *
 * @param[in] seq Temporal point
 * @param[in] gs Geometry
 * @param[out] count Number of elements in the resulting array
 */
static TSequence **
tpointseq_step_at_geometry(const TSequence *seq, const GSERIALIZED *gs,
  int *count)
{
  /* Split the temporal point in an array of non self-intersecting
   * temporal points */
  int countsimple;
  TSequence **simpleseqs = tpointseq_make_simple(seq, &countsimple);
  /* Allocate memory for the intersection points */
  Datum *points = palloc(sizeof(Datum) * countsimple * seq->count);
  int k = 0;
  /* Loop for every simple piece of the sequence */
  for (int i = 0; i < countsimple; i++)
  {
    Datum traj = PointerGetDatum(tpointcontseq_trajectory(simpleseqs[i]));
    Datum inter = geom_intersection2d(traj, PointerGetDatum(gs));
    GSERIALIZED *gsinter = DatumGetGserializedP(inter);
    if (! gserialized_is_empty(gsinter))
      k += gsinter_get_points(&points[k], gsinter);
    PG_FREE_IF_COPY_P(gsinter, DatumGetPointer(inter));
    pfree(DatumGetPointer(inter));
  }
  pfree_array((void **) simpleseqs, countsimple);
  if (k == 0)
  {
    *count = 0;
    return NULL;
  }
  TSequence **result = palloc(sizeof(TSequence *) * seq->count);
  int newcount = tsequence_at_values1(seq, points, k, result);
  pfree_datumarr(points, k);
  *count = newcount;
  return result;
}

/*****************************************************************************/

/**
 * Return the timestamp at which a segment of a temporal point takes a base
 * value
 *
 * This function must take into account the roundoff errors and thus it uses
 * the datum_point_eq_eps for comparing two values so the coordinates of the
 * values may differ by MOBDB_EPSILON. Furthermore, we must drop the Z values
 * since this function may be called for finding the intersection of a sequence
 * and a geometry and the Z values crrently given by PostGIS/GEOS are not
 * necessarily extact, they "are copied, averaged or interpolated" as stated in
 * https://postgis.net/docs/ST_Intersection.html
 *
 * @param[in] inst1,inst2 Temporal values
 * @param[in] value Base value
 * @param[out] t Timestamp
 * @result Return true if the point is found in the temporal point
 * @pre The segment is not constant and has linear interpolation
 * @note The resulting timestamp may be at an exclusive bound
 */
static bool
tpointsegm_timestamp_at_value1(const TInstant *inst1, const TInstant *inst2,
  Datum value, TimestampTz *t)
{
  bool hasz = MOBDB_FLAGS_GET_Z(inst1->flags);
  Datum value1, value2, val;
  GSERIALIZED *gs1, *gs2, *gs;
  if (hasz)
  {
    /* We need to do the comparison in 2D since the Z values returned by PostGIS
     * may not be correct */
    gs1 = DatumGetGserializedP(tinstant_value_copy(inst1));
    gs2 = DatumGetGserializedP(tinstant_value_copy(inst2));
    gs = gserialized_copy(DatumGetGserializedP(value));
    FLAGS_SET_Z(gs1->gflags, false);
    FLAGS_SET_Z(gs2->gflags, false);
    FLAGS_SET_Z(gs->gflags, false);
    value1 = PointerGetDatum(gs1);
    value2 = PointerGetDatum(gs2);
    val = PointerGetDatum(gs);
  }
  else
  {
    value1 = tinstant_value(inst1);
    value2 = tinstant_value(inst2);
    val = value;
  }
  /* Is the lower bound the answer? */
  bool result = true;
  if (datum_point_eq(value1, val))
    *t = inst1->t;
  /* Is the upper bound the answer? */
  else if (datum_point_eq(value2, value))
    *t = inst2->t;
  else
  {
    double dist;
    double fraction = geosegm_locate_point(value1, value2, val, &dist);
    if (fabs(dist) >= MOBDB_EPSILON)
      result = false;
    else
    {
      double duration = (inst2->t - inst1->t);
      *t = inst1->t + (TimestampTz) (duration * fraction);
    }
  }
  if (hasz)
  {
    pfree(gs1); pfree(gs2); pfree(gs);
  }
  return result;
}

/**
 * Return the timestamp at which a temporal point sequence is equal to a point
 *
 * This function is called by the atGeometry function to find the timestamp
 * at which an intersection point found by PostGIS is located. This function
 * differs from function tpointsegm_intersection_value in particular since the
 * latter is used for finding crossings during synchronization and thus it is
 * required that the timestamp in strictly between the timestamps of a segment.
 *
 * @param[in] seq Temporal point sequence
 * @param[in] value Base value
 * @param[out] t Timestamp
 * @result Return true if the point is found in the temporal point
 * @pre The point is known to belong to the temporal sequence (taking
 * into account roundoff errors), the temporal sequence has linear
 * interpolation, and it simple
 * @note The resulting timestamp may be at an exclusive bound
 */
static bool
tpointseq_timestamp_at_value(const TSequence *seq, Datum value,
  TimestampTz *t)
{
  assert(seq->count >= 1);
  const TInstant *inst1 = tsequence_inst_n(seq, 0);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = tsequence_inst_n(seq, i);
    /* We are sure that the segment is not constant since the
     * sequence is simple */
    if (tpointsegm_timestamp_at_value1(inst1, inst2, value, t))
      return true;
    inst1 = inst2;
  }
  /* We should never arrive here */
  elog(ERROR, "The value has not been found due to roundoff errors");
  return false;
}

/**
 * Get the periods at which a temporal sequence point with linear interpolation
 * intersects a geometry
 *
 * @param[in] seq Temporal point
 * @param[in] gsinter Intersection of the temporal point and the geometry
 * @param[out] count Number of elements in the resulting array
 * @pre The temporal sequence is simple, that is, non self-intersecting and
 * the intersection is non empty
 */
Period **
tpointseq_interperiods(const TSequence *seq, GSERIALIZED *gsinter, int *count)
{
  /* The temporal sequence has at least 2 instants since
   * (1) the test for instantaneous full sequence is done in the calling function
   * (2) the simple components of a non self-intersecting sequence have at least
   *     two instants */
  assert(seq->count > 1);
  const TInstant *start = tsequence_inst_n(seq, 0);
  const TInstant *end = tsequence_inst_n(seq, seq->count - 1);
  Period **result;

  /* If the sequence is stationary the whole sequence intersects with the
   * geometry since gsinter is not empty */
  if (seq->count == 2 &&
    datum_point_eq(tinstant_value(start), tinstant_value(end)))
  {
    result = palloc(sizeof(Span *));
    result[0] = span_copy(&seq->period);
    *count = 1;
    return result;
  }

  /* General case */
  LWGEOM *lwgeom_inter = lwgeom_from_gserialized(gsinter);
  int type = lwgeom_inter->type;
  int countinter;
  LWPOINT *lwpoint_inter = NULL; /* make compiler quiet */
  LWLINE *lwline_inter = NULL; /* make compiler quiet */
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
  Period **periods = palloc(sizeof(Period *) * countinter);
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
    GSERIALIZED *gspoint;
    /* Each intersection is either a point or a linestring */
    if (type == POINTTYPE)
    {
      gspoint = geo_serialize((LWGEOM *) lwpoint_inter);
      tpointseq_timestamp_at_value(seq, PointerGetDatum(gspoint), &t1);
      pfree(gspoint);
      /* If the intersection is not at an exclusive bound */
      if ((seq->period.lower_inc || t1 > start->t) &&
          (seq->period.upper_inc || t1 < end->t))
        periods[k++] = span_make(t1, t1, true, true, T_TIMESTAMPTZ);
    }
    else
    {
      /* Get the fraction of the start point of the intersecting line */
      LWPOINT *lwpoint = lwline_get_lwpoint(lwline_inter, 0);
      gspoint = geo_serialize((LWGEOM *) lwpoint);
      tpointseq_timestamp_at_value(seq, PointerGetDatum(gspoint), &t1);
      pfree(gspoint);
      /* Get the fraction of the end point of the intersecting line */
      lwpoint = lwline_get_lwpoint(lwline_inter, lwline_inter->points->npoints - 1);
      gspoint = geo_serialize((LWGEOM *) lwpoint);
      tpointseq_timestamp_at_value(seq, PointerGetDatum(gspoint), &t2);
      pfree(gspoint);
      /* If t1 == t2 and the intersection is not at an exclusive bound */
      if (t1 == t2)
      {
        if ((seq->period.lower_inc || t1 > start->t) &&
            (seq->period.upper_inc || t1 < end->t))
          periods[k++] = span_make(t1, t1, true, true, T_TIMESTAMPTZ);
      }
      else
      {
        TimestampTz lower1 = Min(t1, t2);
        TimestampTz upper1 = Max(t1, t2);
        bool lower_inc1 = (lower1 == start->t) ? seq->period.lower_inc : true;
        bool upper_inc1 = (upper1 == end->t) ? seq->period.upper_inc : true;
        periods[k++] = span_make(lower1, upper1, lower_inc1, upper_inc1,
          T_TIMESTAMPTZ);
      }
    }
  }
  lwgeom_free(lwgeom_inter);

  if (k == 0)
  {
    *count = k;
    pfree(periods);
    return NULL;
  }
  if (k == 1)
  {
    *count = k;
    return periods;
  }

  int newcount;
  result = spanarr_normalize(periods, k, SORT, &newcount);
  *count = newcount;
  return result;
}

/**
 * Restrict a temporal sequence point with linear interpolation to a geometry
 *
 * @param[in] seq Temporal point sequence
 * @param[in] gs Geometry
 * @param[out] count Number of elements in the resulting array
 */
static TSequence **
tpointseq_linear_at_geometry(const TSequence *seq, const GSERIALIZED *gs,
  int *count)
{
  /* Split the temporal point in an array of non self-intersecting
   * temporal points */
  int countsimple;
  TSequence **simpleseqs = tpointseq_make_simple(seq, &countsimple);
  Period **allperiods = NULL; /* make compiler quiet */
  int totalcount = 0;

  if (countsimple == 1)
  {
    /* Particular case when the input sequence is simple */
    pfree_array((void **) simpleseqs, countsimple);
    Datum traj = PointerGetDatum(tpointcontseq_trajectory(seq));
    Datum inter = geom_intersection2d(traj, PointerGetDatum(gs));
    GSERIALIZED *gsinter = DatumGetGserializedP(inter);
    if (! gserialized_is_empty(gsinter))
      allperiods = tpointseq_interperiods(seq, gsinter, &totalcount);
    PG_FREE_IF_COPY_P(gsinter, DatumGetPointer(inter));
    pfree(DatumGetPointer(inter));
    if (totalcount == 0)
    {
      *count = 0;
      return NULL;
    }
  }
  else
  {
    /* General case: Allocate memory for the result */
    Period ***periods = palloc(sizeof(Period *) * countsimple);
    int *countpers = palloc0(sizeof(int) * countsimple);
    /* Loop for every simple piece of the sequence */
    for (int i = 0; i < countsimple; i++)
    {
      Datum traj = PointerGetDatum(tpointcontseq_trajectory(simpleseqs[i]));
      Datum inter = geom_intersection2d(traj, PointerGetDatum(gs));
      GSERIALIZED *gsinter = DatumGetGserializedP(inter);
      if (! gserialized_is_empty(gsinter))
      {
        periods[i] = tpointseq_interperiods(simpleseqs[i], gsinter,
          &countpers[i]);
        totalcount += countpers[i];
      }
      PG_FREE_IF_COPY_P(gsinter, DatumGetPointer(inter));
      pfree(DatumGetPointer(inter));
    }
    pfree_array((void **) simpleseqs, countsimple);
    if (totalcount == 0)
    {
      *count = 0;
      return NULL;
    }
    /* Assemble the periods into a single array */
    allperiods = palloc(sizeof(Period *) * totalcount);
    int k = 0;
    for (int i = 0; i < countsimple; i++)
    {
      for (int j = 0; j < countpers[i]; j++)
        allperiods[k++] = periods[i][j];
      if (countpers[i] != 0)
        pfree(periods[i]);
    }
    /* It is necessary to sort the periods */
    spanarr_sort(allperiods, totalcount);
  }
  PeriodSet *ps = periodset_make_free(allperiods, totalcount, NORMALIZE);
  TSequence **result = palloc(sizeof(TSequence *) * totalcount);
  *count = tcontseq_at_periodset1(seq, ps, result);
  pfree(ps);
  return result;
}

/**
 * @brief Restrict a temporal point to (the complement of) a geometry.
 *
 * The function splits the temporal point in an array of temporal point
 * sequences that are simple (that is, not self-intersecting) and loops
 * for each piece
 * @param[in] seq Temporal sequence point
 * @param[in] gs Geometry
 * @param[out] count Number of elements in the resulting array
 * @pre The temporal sequence is simple, that is, does not self-intersect
 */
TSequence **
tpointseq_at_geometry(const TSequence *seq, const GSERIALIZED *gs, int *count)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TInstant *inst = tpointinst_restrict_geometry(tsequence_inst_n(seq, 0),
      gs, REST_AT);
    if (inst == NULL)
    {
      *count = 0;
      return NULL;
    }
    TSequence **result = palloc(sizeof(TSequence *));
    result[0] = tsequence_copy(seq);
    *count = 1;
    return result;
  }
  if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
    return tpointseq_linear_at_geometry(seq, gs, count);
  else
    return tpointseq_step_at_geometry(seq, gs, count);
}

/**
 * Restrict a temporal sequence point to the complement of a geometry
 *
 * It is not possible to use a similar approach as for tpointseq_at_geometry1
 * where instead of computing the intersections we compute the difference since
 * in PostGIS the following query
 * @code
 * select st_astext(st_difference(geometry 'Linestring(0 0,3 3)',
 *     geometry 'MultiPoint((1 1),(2 2),(3 3))'))
 * @endcode
 * returns `LINESTRING(0 0,3 3)`. Therefore we compute tpointseq_at_geometry1
 * and then compute the complement of the value obtained
 *
 * @param[in] seq Temporal point sequence
 * @param[in] gs Geometry
 * @param[out] count Number of elements in the resulting array
 */
static TSequence **
tpointseq_minus_geometry(const TSequence *seq, const GSERIALIZED *gs,
  int *count)
{
  int countinter;
  TSequence **sequences = tpointseq_at_geometry(seq, gs, &countinter);
  if (countinter == 0)
  {
    TSequence **result = palloc(sizeof(TSequence *));
    result[0] = tsequence_copy(seq);
    *count = 1;
    return result;
  }

  const Period **periods = palloc(sizeof(Period *) * countinter);
  for (int i = 0; i < countinter; i++)
    periods[i] = &sequences[i]->period;
  PeriodSet *ps1 = periodset_make(periods, countinter, NORMALIZE_NO);
  PeriodSet *ps2 = minus_period_periodset(&seq->period, ps1);
  pfree(ps1); pfree(periods);
  if (ps2 == NULL)
  {
    *count = 0;
    return NULL;
  }
  TSequence **result = palloc(sizeof(TSequence *) * ps2->count);
  *count = tcontseq_at_periodset1(seq, ps2, result);
  pfree(ps2);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal point sequence to (the complement of a) geometry.
 *
 * @param[in] seq Temporal sequence point
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is at, false for minus
 * @pre The arguments are of the same dimensionality, have the same SRID,
 * and the geometry is not empty. This is verified in #tpoint_restrict_geometry
 * @note The test for instantaneous sequences is done at the function
 * tpointseq_at_geometry since the latter function is called for each sequence
 * of a sequence set
 * @sqlfunc atGeometry(), minusGeometry()
 */
TSequenceSet *
tpointseq_restrict_geometry(const TSequence *seq, const GSERIALIZED *gs,
  bool atfunc)
{
  int count;
  TSequence **sequences = atfunc ? tpointseq_at_geometry(seq, gs, &count) :
    tpointseq_minus_geometry(seq, gs, &count);
  if (sequences == NULL)
    return NULL;

  /* It is necessary to sort the sequences */
  tseqarr_sort((TSequence **) sequences, count);
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal point sequence set to (the complement of) a
 * geometry.
 *
 * @param[in] ss Temporal sequence set point
 * @param[in] gs Geometry
 * @param[in] box Bounding box of the geometry
 * @param[in] atfunc True if the restriction is at, false for minus
 * @pre The arguments are of the same dimensionality, have the same SRID,
 * and the geometry is not empty. This is verified in #tpoint_restrict_geometry
 * @sqlfunc atGeometry(), minusGeometry()
 */
TSequenceSet *
tpointseqset_restrict_geometry(const TSequenceSet *ss, const GSERIALIZED *gs,
  const STBOX *box, bool atfunc)
{
  /* Singleton sequence set */
  if (ss->count == 1)
    return tpointseq_restrict_geometry(tsequenceset_seq_n(ss, 0), gs, atfunc);

  /* palloc0 used due to the bounding box test in the for loop below */
  TSequence ***sequences = palloc0(sizeof(TSequence *) * ss->count);
  int *countseqs = palloc0(sizeof(int) * ss->count);
  int totalcount = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    /* Bounding box test */
    STBOX *box1 = TSEQUENCE_BBOX_PTR(seq);
    bool overlaps = overlaps_stbox_stbox(box1, box);
    if (atfunc)
    {
      if (overlaps)
      {
        sequences[i] = tpointseq_at_geometry(seq, gs, &countseqs[i]);
        totalcount += countseqs[i];
      }
    }
    else
    {
      if (! overlaps)
      {
        sequences[i] = palloc(sizeof(TSequence *));
        sequences[i][0] = tsequence_copy(seq);
        countseqs[i] = 1;
        totalcount ++;
      }
      else
      {
        sequences[i] = tpointseq_minus_geometry(seq, gs, &countseqs[i]);
        totalcount += countseqs[i];
      }
    }
  }
  if (totalcount == 0)
  {
    pfree(sequences); pfree(countseqs);
    return NULL;
  }
  TSequence **allseqs = tseqarr2_to_tseqarr(sequences,
    countseqs, ss->count, totalcount);
  return tsequenceset_make_free(allseqs, totalcount, NORMALIZE);
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal point to (the complement of) a geometry.
 *
 *
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is at, false for minus
 * @sqlfunc atGeometry(), minusGeometry()
 */
Temporal *
tpoint_restrict_geometry(const Temporal *temp, const GSERIALIZED *gs,
  bool atfunc)
{
  if (gserialized_is_empty(gs))
  {
    if (atfunc)
      return NULL;
    else
      return temporal_copy(temp);
  }
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));

  /* Bounding box test */
  STBOX box1, box2;
  temporal_set_bbox(temp, &box1);
  /* Non-empty geometries have a bounding box */
  geo_set_stbox(gs, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
    return atfunc ? NULL : temporal_copy(temp);

  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tpointinst_restrict_geometry((TInstant *) temp,
      gs, atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = MOBDB_FLAGS_GET_DISCRETE(temp->flags) ?
      (Temporal *) tpointdiscseq_restrict_geometry((TSequence *) temp, gs, atfunc) :
      (Temporal *) tpointseq_restrict_geometry((TSequence *) temp, gs, atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tpointseqset_restrict_geometry((TSequenceSet *) temp,
      gs, &box2, atfunc);

  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict a temporal point to (the complement of) a geometry.
 * @sqlfunc atGeometry(), minusGeometry()
 */
Temporal *
tpoint_at_geometry(const Temporal *temp, const GSERIALIZED *gs)
{
  Temporal *result = tpoint_restrict_geometry(temp, gs, REST_AT);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict a temporal point to (the complement of) a geometry.
 * @sqlfunc atGeometry(), minusGeometry()
 */
Temporal *
tpoint_minus_geometry(const Temporal *temp, const GSERIALIZED *gs)
{
  Temporal *result = tpoint_restrict_geometry(temp, gs, REST_MINUS);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * Assemble a 2D point for its x and y coordinates, srid, and geodetic flag
 */
static Datum
point2D_assemble(Datum x, Datum y, Datum srid, Datum geodetic)
{
  double x1 = DatumGetFloat8(x);
  double y1 = DatumGetFloat8(y);
  int srid1 = DatumGetInt32(srid);
  LWPOINT *lwpoint = lwpoint_make2d(srid1, x1, y1);
  FLAGS_SET_GEODETIC(lwpoint->flags, DatumGetBool(geodetic));
  Datum result = PointerGetDatum(geo_serialize((LWGEOM *) lwpoint));
  lwpoint_free(lwpoint);
  return result;
}

/**
 * Assemble a 2D temporal point for two temporal floats, srid, and geodetic
 * flag
 */
static Temporal *
tpoint_assemble_coords_xy(Temporal *temp_x, Temporal *temp_y, int srid,
  bool geodetic)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &point2D_assemble;
  lfinfo.numparam = 2;
  lfinfo.param[0] = Int32GetDatum(srid);
  lfinfo.param[1] = BoolGetDatum(geodetic);
  lfinfo.restype = T_TGEOMPOINT; // TODO Geography ???
  lfinfo.reslinear = MOBDB_FLAGS_GET_LINEAR(temp_x->flags) ||
    MOBDB_FLAGS_GET_LINEAR(temp_y->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal_temporal(temp_x, temp_y, &lfinfo);
}

/**
 * Add a z value to a 2D point
 */
static Datum
point2D_add_z(Datum point, Datum z, Datum srid)
{
  GSERIALIZED *gs = DatumGetGserializedP(point);
  bool geodetic = FLAGS_GET_GEODETIC(gs->gflags);
  const POINT2D *pt = datum_point2d_p(point);
  double z1 = DatumGetFloat8(z);
  int srid1 = DatumGetInt32(srid);
  LWPOINT *lwpoint = lwpoint_make3dz(srid1, pt->x, pt->y, z1);
  FLAGS_SET_GEODETIC(lwpoint->flags, geodetic);
  Datum result = PointerGetDatum(geo_serialize((LWGEOM *) lwpoint));
  lwpoint_free(lwpoint);
  return result;
}

/**
 * Assemble a 2D temporal point for two temporal floats
 */
static Temporal *
tpoint_add_z(Temporal *temp, Temporal *temp_z, int srid)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &point2D_add_z;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Int32GetDatum(srid);
  lfinfo.restype = T_TGEOMPOINT; // TODO Geography ???
  lfinfo.reslinear = MOBDB_FLAGS_GET_LINEAR(temp->flags) ||
    MOBDB_FLAGS_GET_LINEAR(temp_z->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal_temporal(temp, temp_z, &lfinfo);
}

/**
 * @brief Restrict a temporal point to a spatiotemporal box.
 * @pre The arguments are of the same dimensionality and have the same SRID
 */
Temporal *
tpoint_at_stbox1(const Temporal *temp, const STBOX *box, bool upper_inc)
{
  /* At least one of MOBDB_FLAGS_GET_X and MOBDB_FLAGS_GET_T is true */
  bool hasx = MOBDB_FLAGS_GET_X(box->flags);
  bool hasz = MOBDB_FLAGS_GET_Z(box->flags);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  assert(hasx || hast);

  /* Bounding box test */
  STBOX box1;
  temporal_set_bbox(temp, &box1);
  if (! overlaps_stbox_stbox(box, &box1))
    return NULL;

  Temporal *temp1;
  if (hast)
  {
    temp1 = temporal_restrict_period(temp, &box->period, REST_AT);
    /* Despite the bounding box test above, temp1 may be NULL due to
     * exclusive bounds */
    if (temp1 == NULL)
      return NULL;
  }
  else
    temp1 = (Temporal *) temp;

  Temporal *result = NULL;
  if (hasx)
  {
    /* Split the temporal point into temporal floats for each coordinate */
    Temporal *temp_x = tpoint_get_coord(temp1, 0);
    Temporal *temp_y = tpoint_get_coord(temp1, 1);
    Temporal *temp_z = NULL;
    if (hasz)
      temp_z = tpoint_get_coord(temp1, 2);
    Span *span_x = span_make(Float8GetDatum(box->xmin),
      Float8GetDatum(box->xmax), true, upper_inc, T_FLOAT8);
    Span *span_y = span_make(Float8GetDatum(box->ymin),
      Float8GetDatum(box->ymax), true, upper_inc, T_FLOAT8);
    Span *span_z = NULL;
    if (hasz)
      span_z = span_make(Float8GetDatum(box->zmin), Float8GetDatum(box->zmax),
        true, upper_inc, T_FLOAT8);
    Temporal *at_temp_x = tnumber_restrict_span(temp_x, span_x, REST_AT);
    Temporal *at_temp_y = tnumber_restrict_span(temp_y, span_y, REST_AT);
    Temporal *at_temp_z = NULL;
    if (hasz)
      at_temp_z = tnumber_restrict_span(temp_z, span_z, REST_AT);
    Temporal *result2D = NULL;
    if (at_temp_x != NULL && at_temp_y != NULL && (! hasz || at_temp_z != NULL))
    {
      /* Combine the temporal floats for each coordinate into a temporal point */
      int srid = tpoint_srid(temp1);
      bool geodetic = MOBDB_FLAGS_GET_GEODETIC(temp1->flags);
      result2D = tpoint_assemble_coords_xy(at_temp_x, at_temp_y, srid,
        geodetic);
      result = (result2D != NULL && hasz) ?
        tpoint_add_z(result2D, at_temp_z, srid) : result2D;
    }
    pfree(temp_x); pfree(span_x); pfree(temp_y); pfree(span_y);
    if (at_temp_x != NULL) pfree(at_temp_x);
    if (at_temp_y != NULL) pfree(at_temp_y);
    if (hasz)
    {
      pfree(temp_z); pfree(span_z);
      if (at_temp_z != NULL) pfree(at_temp_z);
      if (result2D != NULL) pfree(result2D);
    }
  }
  else
    result = temp1;
  if (hasx && hast)
    pfree(temp1);
  return result;
}

/**
 * @brief Restrict a temporal point to the complement of a spatiotemporal box.
 *
 * We cannot make the difference from each dimension separately, i.e.,
 * restrict at the period and then restrict to the polygon. Therefore, we
 * compute the atStbox and then compute the complement of the value obtained.
 *
 * @pre The arguments are of the same dimensionality and have the same SRID
 */
static Temporal *
tpoint_minus_stbox1(const Temporal *temp, const STBOX *box)
{
  /* Bounding box test */
  STBOX box1;
  temporal_set_bbox(temp, &box1);
  if (! overlaps_stbox_stbox(box, &box1))
    return temporal_copy(temp);

  Temporal *result = NULL;
  Temporal *temp1 = tpoint_at_stbox1(temp, box, UPPER_INC);
  if (temp1 != NULL)
  {
    PeriodSet *ps1 = temporal_time(temp);
    PeriodSet *ps2 = temporal_time(temp1);
    PeriodSet *ps = minus_periodset_periodset(ps1, ps2);
    if (ps != NULL)
    {
      result = temporal_restrict_periodset(temp, ps, REST_MINUS);
      pfree(ps);
    }
    pfree(temp1); pfree(ps1); pfree(ps2);
  }
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal point to (the complement of) a spatiotemporal
 * box.
 *
 * @param[in] temp Temporal point
 * @param[in] box Box
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note Mixing 2D/3D is enabled to compute, for example, 2.5D operations.
 * @sqlfunc atStbox(), minusStbox()
 */
Temporal *
tpoint_restrict_stbox(const Temporal *temp, const STBOX *box, bool atfunc)
{
  ensure_common_dimension(temp->flags, box->flags);
  if (MOBDB_FLAGS_GET_X(box->flags))
  {
    ensure_same_geodetic(temp->flags, box->flags);
    ensure_same_srid_tpoint_stbox(temp, box);
    ensure_same_spatial_dimensionality_temp_box(temp->flags, box->flags);
  }
  Temporal *result = atfunc ? tpoint_at_stbox1(temp, box, UPPER_INC) :
    tpoint_minus_stbox1(temp, box);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict a temporal point to a spatiotemporal box.
 * @sqlfunc atStbox(), minusGeometry()
 */
Temporal *
tpoint_at_stbox(const Temporal *temp, const STBOX *box)
{
  Temporal *result = tpoint_restrict_stbox(temp, box, REST_AT);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict a temporal point to the complement of a spatiotemporal box.
 * @sqlfunc minusStbox()
 */
Temporal *
tpoint_minus_stbox(const Temporal *temp, const STBOX *box)
{
  Temporal *result = tpoint_restrict_stbox(temp, box, REST_MINUS);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/
