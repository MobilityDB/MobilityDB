/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Spatial functions for temporal points.
 */

#include "point/tpoint_spatialfuncs.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <utils/float.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
#include <lwgeodetic.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/lifting.h"
#include "general/temporaltypes.h"
#include "general/type_util.h"
#include "point/pgis_call.h"

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

/**
 * @brief Return a 4D point from the datum
 * @note The M dimension is ignored
 */
void
datum_point4d(Datum value, POINT4D *p)
{
  const GSERIALIZED *gs = DatumGetGserializedP(value);
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
 * @brief Return true if the points are equal
 * @note This function is called in the iterations over sequences where we
 * are sure that their SRID, Z, and GEODETIC are equal
 */
bool
gspoint_eq(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  if (FLAGS_GET_Z(gs1->gflags))
  {
    const POINT3DZ *point1 = GSERIALIZED_POINT3DZ_P(gs1);
    const POINT3DZ *point2 = GSERIALIZED_POINT3DZ_P(gs2);
    return float8_eq(point1->x, point2->x) &&
      float8_eq(point1->y, point2->y) && float8_eq(point1->z, point2->z);
  }
  else
  {
    const POINT2D *point1 = GSERIALIZED_POINT2D_P(gs1);
    const POINT2D *point2 = GSERIALIZED_POINT2D_P(gs2);
    return float8_eq(point1->x, point2->x) && float8_eq(point1->y, point2->y);
  }
}

/**
 * @brief Return true if the points are equal up to the floating point tolerance
 * @note This function is called in the iterations over sequences where we
 * are sure that their SRID, Z, and GEODETIC are equal
 */
bool
gspoint_same(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  if (FLAGS_GET_Z(gs1->gflags))
  {
    const POINT3DZ *point1 = GSERIALIZED_POINT3DZ_P(gs1);
    const POINT3DZ *point2 = GSERIALIZED_POINT3DZ_P(gs2);
    return MEOS_FP_EQ(point1->x, point2->x) &&
      MEOS_FP_EQ(point1->y, point2->y) && MEOS_FP_EQ(point1->z, point2->z);
  }
  else
  {
    const POINT2D *point1 = GSERIALIZED_POINT2D_P(gs1);
    const POINT2D *point2 = GSERIALIZED_POINT2D_P(gs2);
    return MEOS_FP_EQ(point1->x, point2->x) &&
      MEOS_FP_EQ(point1->y, point2->y);
  }
}

/**
 * @brief Return true if the points are equal
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
  return gspoint_eq(gs1, gs2);
}

/**
 * @brief Return true if the points are equal
 */
bool
datum_point_same(Datum geopoint1, Datum geopoint2)
{
  const GSERIALIZED *gs1 = DatumGetGserializedP(geopoint1);
  const GSERIALIZED *gs2 = DatumGetGserializedP(geopoint2);
  if (gserialized_get_srid(gs1) != gserialized_get_srid(gs2) ||
      FLAGS_GET_Z(gs1->gflags) != FLAGS_GET_Z(gs2->gflags) ||
      FLAGS_GET_GEODETIC(gs1->gflags) != FLAGS_GET_GEODETIC(gs2->gflags))
    return false;
  return gspoint_same(gs1, gs2);
}

/**
 * @brief Return true if the points are equal
 */
Datum
datum2_point_eq(Datum geopoint1, Datum geopoint2)
{
  return BoolGetDatum(datum_point_eq(geopoint1, geopoint2));
}

/**
 * @brief Return true if the points are equal
 */
Datum
datum2_point_ne(Datum geopoint1, Datum geopoint2)
{
  return BoolGetDatum(! datum_point_eq(geopoint1, geopoint2));
}

/**
 * @brief Return true if the points are equal
 */
Datum
datum2_point_same(Datum geopoint1, Datum geopoint2)
{
  return BoolGetDatum(datum_point_same(geopoint1, geopoint2));
}

/**
 * @brief Return true if the points are equal
 */
Datum
datum2_point_nsame(Datum geopoint1, Datum geopoint2)
{
  return BoolGetDatum(! datum_point_same(geopoint1, geopoint2));
}

/**
 * @brief Serialize a geometry/geography
 * @pre It is supposed that the flags such as Z and geodetic have been
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
 * @brief Select the appropriate distance function
 */
datum_func2
distance_fn(int16 flags)
{
  datum_func2 result;
  if (MEOS_FLAGS_GET_GEODETIC(flags))
    result = &geog_distance;
  else
    result = MEOS_FLAGS_GET_Z(flags) ?
      &geom_distance3d : &geom_distance2d;
  return result;
}

/**
 * @brief Select the appropriate distance function
 */
datum_func2
pt_distance_fn(int16 flags)
{
  datum_func2 result;
  if (MEOS_FLAGS_GET_GEODETIC(flags))
    result = &geog_distance;
  else
    result = MEOS_FLAGS_GET_Z(flags) ?
      &pt_distance3d : &pt_distance2d;
  return result;
}

/**
 * @brief Return the 2D distance between the two geometries
 * @pre For PostGIS version > 3 the geometries are NOT toasted
 */
Datum
geom_distance2d(Datum geom1, Datum geom2)
{
  return Float8GetDatum(gserialized_distance(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2)));
}

/**
 * @brief Return the 3D distance between the two geometries
 */
Datum
geom_distance3d(Datum geom1, Datum geom2)
{
  return Float8GetDatum(gserialized_3Ddistance(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2)));
}

/**
 * @brief Return the distance between the two geographies
 */
Datum
geog_distance(Datum geog1, Datum geog2)
{
  return Float8GetDatum(gserialized_geog_distance(DatumGetGserializedP(geog1),
    DatumGetGserializedP(geog2)));
}

/**
 * @brief Return the 2D distance between the two geometric points
 */
Datum
pt_distance2d(Datum geom1, Datum geom2)
{
  const POINT2D *p1 = DATUM_POINT2D_P(geom1);
  const POINT2D *p2 = DATUM_POINT2D_P(geom2);
  return Float8GetDatum(distance2d_pt_pt(p1, p2));
}

/**
 * @brief Return the 3D distance between the two geometric points
 */
Datum
pt_distance3d(Datum geom1, Datum geom2)
{
  const POINT3DZ *p1 = DATUM_POINT3DZ_P(geom1);
  const POINT3DZ *p2 = DATUM_POINT3DZ_P(geom2);
  return Float8GetDatum(distance3d_pt_pt((POINT3D *) p1, (POINT3D *) p2));
}

/**
 * @brief Return the 2D intersection between the two geometries
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
 * @brief Ensure that the spatial constraints required for operating on two
 * temporal geometries are satisfied
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
 * @brief Ensure that the spatiotemporal argument has planar coordinates
 */
void
ensure_not_geodetic(int16 flags)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags))
    elog(ERROR, "Only planar coordinates supported");
  return;
}

/**
 * @brief Ensure that the spatiotemporal argument have the same type of
 * coordinates, either planar or geodetic
 */
void
ensure_same_geodetic(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_X(flags1) && MEOS_FLAGS_GET_X(flags2) &&
    MEOS_FLAGS_GET_GEODETIC(flags1) != MEOS_FLAGS_GET_GEODETIC(flags2))
    elog(ERROR, "Operation on mixed planar and geodetic coordinates");
  return;
}

/**
 * @brief Ensure that the two spatial "objects" have the same SRID
 */
void
ensure_same_srid(int32_t srid1, int32_t srid2)
{
  if (srid1 != srid2)
    elog(ERROR, "Operation on mixed SRID");
  return;
}

/**
 * @brief Ensure that a temporal point and a geometry/geography have the same
 * SRID
 */
void
ensure_same_srid_stbox_gs(const STBox *box, const GSERIALIZED *gs)
{
  if (box->srid != gserialized_get_srid(gs))
    elog(ERROR, "Operation on mixed SRID");
  return;
}

/**
 * @brief Ensure that two temporal points have the same dimensionality as given
 * by their flags
 */
void
ensure_same_dimensionality(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_X(flags1) != MEOS_FLAGS_GET_X(flags2) ||
    MEOS_FLAGS_GET_Z(flags1) != MEOS_FLAGS_GET_Z(flags2) ||
    MEOS_FLAGS_GET_T(flags1) != MEOS_FLAGS_GET_T(flags2))
    elog(ERROR, "The arguments must be of the same dimensionality");
  return;
}

/**
 * @brief Ensure that two temporal points have the same spatial dimensionality
 * as given by their flags
 */
void
ensure_same_spatial_dimensionality(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_X(flags1) != MEOS_FLAGS_GET_X(flags2) ||
    MEOS_FLAGS_GET_Z(flags1) != MEOS_FLAGS_GET_Z(flags2))
    elog(ERROR, "Operation on mixed 2D/3D dimensions");
  return;
}

/**
 * @brief Ensure that a temporal point and a spatiotemporal box have the same
 * spatial dimensionality as given by their flags
 */
void
ensure_same_spatial_dimensionality_temp_box(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_X(flags1) != MEOS_FLAGS_GET_X(flags2) ||
      /* Geodetic boxes are always in 3D */
      (! MEOS_FLAGS_GET_GEODETIC(flags2) &&
      MEOS_FLAGS_GET_Z(flags1) != MEOS_FLAGS_GET_Z(flags2)))
    elog(ERROR, "Operation on mixed 2D/3D dimensions");
  return;
}

/**
 * @brief Ensure that two geometries/geographies have the same dimensionality
 */
void
ensure_same_dimensionality_gs(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  if (FLAGS_GET_Z(gs1->gflags) != FLAGS_GET_Z(gs2->gflags))
    elog(ERROR, "Operation on mixed 2D/3D dimensions");
  return;
}

/**
 * @brief Ensure that a temporal point and a geometry/geography have the same
 * dimensionality
 */
void
ensure_same_dimensionality_tpoint_gs(const Temporal *temp, const GSERIALIZED *gs)
{
  if (MEOS_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->gflags))
    elog(ERROR, "Operation on mixed 2D/3D dimensions");
  return;
}

/**
 * @brief Ensure that the spatiotemporal boxes have the same spatial
 * dimensionality
 */
void
ensure_same_spatial_dimensionality_stbox_gs(const STBox *box, const GSERIALIZED *gs)
{
  if (! MEOS_FLAGS_GET_X(box->flags) ||
      /* Geodetic boxes are always in 3D */
     (! MEOS_FLAGS_GET_GEODETIC(box->flags) &&
        MEOS_FLAGS_GET_Z(box->flags) != FLAGS_GET_Z(gs->gflags)))
    elog(ERROR, "The spatiotemporal box and the geometry must be of the same dimensionality");
  return;
}

/**
 * @brief Ensure that a temporal point has Z dimension
 */
void
ensure_has_Z(int16 flags)
{
  if (! MEOS_FLAGS_GET_Z(flags))
    elog(ERROR, "The temporal point must have Z dimension");
  return;
}

/**
 * @brief Ensure that a temporal point has not Z dimension
 */
void
ensure_has_not_Z(int16 flags)
{
  if (MEOS_FLAGS_GET_Z(flags))
    elog(ERROR, "The temporal point cannot have Z dimension");
  return;
}

/**
 * @brief Ensure that the geometry/geography has not Z dimension
 */
void
ensure_has_Z_gs(const GSERIALIZED *gs)
{
  if (! FLAGS_GET_Z(gs->gflags))
    elog(ERROR, "The geometry must have Z dimension");
  return;
}

/**
 * @brief Ensure that the geometry/geography has not Z dimension
 */
void
ensure_has_not_Z_gs(const GSERIALIZED *gs)
{
  if (FLAGS_GET_Z(gs->gflags))
    elog(ERROR, "The geometry cannot have Z dimension");
  return;
}

/**
 * @brief Ensure that the geometry/geography has M dimension
 */
void
ensure_has_M_gs(const GSERIALIZED *gs)
{
  if (! FLAGS_GET_M(gs->gflags))
    elog(ERROR, "Only geometries with M dimension accepted");
  return;
}

/**
 * @brief Ensure that the geometry/geography has not M dimension
 */
void
ensure_has_not_M_gs(const GSERIALIZED *gs)
{
  if (FLAGS_GET_M(gs->gflags))
    elog(ERROR, "Only geometries without M dimension accepted");
  return;
}

/**
 * @brief Ensure that the geometry/geography is a point
 */
void
ensure_point_type(const GSERIALIZED *gs)
{
  if (gserialized_get_type(gs) != POINTTYPE)
    elog(ERROR, "Only point geometries accepted");
  return;
}

/**
 * @brief Ensure that the geometry/geography is not empty
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
/**
 * @brief Return true if point p is in the segment defined by A and B (2D)
 * @note The test of p = A or p = B MUST BE done in the calling function
 *   to take care of the inclusive/exclusive bounds for temporal sequences
 */
static bool
point2d_on_segment(const POINT2D *p, const POINT2D *A, const POINT2D *B)
{
  double crossproduct = (p->y - A->y) * (B->x - A->x) -
    (p->x - A->x) * (B->y - A->y);
  if (fabs(crossproduct) >= MEOS_EPSILON)
    return false;
  double dotproduct = (p->x - A->x) * (B->x - A->x) +
    (p->y - A->y) * (B->y - A->y);
  return (dotproduct >= 0);
}

/**
 * @brief Return true if point p is in the segment defined by A and B (3D)
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
  if (fabs(i) >= MEOS_EPSILON || fabs(j) >= MEOS_EPSILON ||
      fabs(k) >= MEOS_EPSILON)
    return false;
  double dotproduct = (p->x - A->x) * (B->x - A->x) +
    (p->y - A->y) * (B->y - A->y) + (p->z - A->z) * (B->z - A->z);
  return (dotproduct >= 0);
}

/**
 * @brief Return true if point p is in the segment defined by A and B (geodetic)
 */
static bool
point_on_segment_sphere(const POINT4D *p, const POINT4D *A, const POINT4D *B)
{
  POINT4D closest;
  double dist;
  closest_point_on_segment_sphere(p, A, B, &closest, &dist);
  return (dist > MEOS_EPSILON) && (FP_EQUALS(p->z, closest.z));
}

/**
 * @brief Determine if a point is in a segment.
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
    const POINT3DZ *p1 = DATUM_POINT3DZ_P(start);
    const POINT3DZ *p2 = DATUM_POINT3DZ_P(end);
    const POINT3DZ *p = DATUM_POINT3DZ_P(point);
    return point3dz_on_segment(p, p1, p2);
  }
  /* 2D */
  const POINT2D *p1 = DATUM_POINT2D_P(start);
  const POINT2D *p2 = DATUM_POINT2D_P(end);
  const POINT2D *p = DATUM_POINT2D_P(point);
  return point2d_on_segment(p, p1, p2);
}

/*****************************************************************************
 * Ever/always equal comparison operators
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_ever
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
 * @ingroup libmeos_internal_temporal_ever
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

  /* Step interpolation or instantaneous sequence */
  if (! MEOS_FLAGS_GET_LINEAR(seq->flags) || seq->count == 1)
  {
    for (i = 0; i < seq->count; i++)
    {
      value1 = tinstant_value(TSEQUENCE_INST_N(seq, i));
      if (datum_point_eq(value1, value))
        return true;
    }
    return false;
  }

  /* Linear interpolation*/
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  value1 = tinstant_value(inst1);
  bool lower_inc = seq->period.lower_inc;
  for (i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
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
 * @ingroup libmeos_internal_temporal_ever
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
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (tpointseq_ever_eq(seq, value))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_temporal_ever
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
  assert(temptype_subtype(temp->subtype));
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
 * @ingroup libmeos_internal_temporal_ever
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
 * @ingroup libmeos_internal_temporal_ever
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
 * @ingroup libmeos_internal_temporal_ever
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
  assert(temptype_subtype(temp->subtype));
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
 * @brief Return a long double between 0 and 1 representing the location of the
 * closest point on the segment to the given point, as a fraction of total
 * segment length (2D version)
 * @note Function derived from the PostGIS function closest_point_on_segment
 */
long double
closest_point2d_on_segment_ratio(const POINT2D *p, const POINT2D *A,
  const POINT2D *B, POINT2D *closest)
{
  if (FP_EQUALS(A->x, B->x) && FP_EQUALS(A->y, B->y))
  {
    if (closest)
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
    if (closest)
      *closest = *A;
    return 0.0;
  }
  if (r > 1)
  {
    if (closest)
      *closest = *B;
    return 1.0;
  }

  if (closest)
  {
    closest->x = (double) (A->x + ( (B->x - A->x) * r ));
    closest->y = (double) (A->y + ( (B->y - A->y) * r ));
  }
  return r;
}

/**
 * @brief Return a float between 0 and 1 representing the location of the
 * closest point on the segment to the given point, as a fraction of total
 * segment length (3D version)
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

  closest->x = (double) (A->x + ( (B->x - A->x) * r ));
  closest->y = (double) (A->y + ( (B->y - A->y) * r ));
  closest->z = (double) (A->z + ( (B->z - A->z) * r ));
  return r;
}

/**
 * @brief Return a float between 0 and 1 representing the location of the
 * closest point on the geography segment to the given point, as a fraction of
 * total segment length
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
    closest->z = (double) (A->z + ((B->z - A->z) * result));
    closest->m = (double) (A->m + ((B->m - A->m) * result));
  }
  return result;
}

/**
 * @brief Find interpolation point p between geography points p1 and p2
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
 * @brief Create a point
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
 * @brief Return a point interpolated from the geometry/geography segment with
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
    interpolate_point4d_sphere(&q1, &q2, &p1, &p2, (double) ratio, &p);
  }
  else
  {
    /* We cannot call the PostGIS function
     * interpolate_point4d(&p1, &p2, &p, ratio);
     * since it uses a double and not a long double for the interpolation */
    p.x = p1.x + (double) ((long double) (p2.x - p1.x) * ratio);
    p.y = p1.y + (double) ((long double) (p2.y - p1.y) * ratio);
    p.z = p1.z + (double) ((long double) (p2.z - p1.z) * ratio);
    p.m = 0.0;
  }

  Datum result = PointerGetDatum(gspoint_make(p.x, p.y, p.z, hasz, geodetic,
    srid));
  PG_FREE_IF_COPY_P(gs, DatumGetPointer(start));
  return result;
}

/**
 * @brief Return a float between 0 and 1 representing the location of the
 * closest point on the geometry segment to the given point, as a fraction of
 * total segment length
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
      const POINT3DZ *p1 = DATUM_POINT3DZ_P(start);
      const POINT3DZ *p2 = DATUM_POINT3DZ_P(end);
      const POINT3DZ *p = DATUM_POINT3DZ_P(point);
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
      const POINT2D *p1 = DATUM_POINT2D_P(start);
      const POINT2D *p2 = DATUM_POINT2D_P(end);
      const POINT2D *p = DATUM_POINT2D_P(point);
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
 * @brief Return true if a segment of a temporal point value intersects a point
 * at the timestamp
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] value Base value
 * @param[out] t Timestamp
 * @pre The geometry is not empty
 */
bool
tpointsegm_intersection_value(const TInstant *inst1, const TInstant *inst2,
  Datum value, TimestampTz *t)
{
  assert(! gserialized_is_empty(DatumGetGserializedP(value)));

  /* We are sure that the trajectory is a line */
  Datum start = tinstant_value(inst1);
  Datum end = tinstant_value(inst2);
  double dist;
  double fraction = (double) geosegm_locate_point(start, end, value, &dist);
  if (fabs(dist) >= MEOS_EPSILON)
    return false;
  if (t != NULL)
  {
    double duration = (double) (inst2->t - inst1->t);
    /* Note that due to roundoff errors it may be the case that the
     * resulting timestamp t may be equal to inst1->t or to inst2->t */
    *t = inst1->t + (TimestampTz) (duration * fraction);
  }
  return true;
}

/**
 * @brief Return true if two segments of a temporal geometric points intersect
 * at a timestamp
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
  double x1, y1, z1 = 0.0, x2, y2, z2 = 0.0, x3, y3, z3 = 0.0, x4, y4, z4 = 0.0;
  bool hasz = MEOS_FLAGS_GET_Z(start1->flags);
  if (hasz)
  {
    const POINT3DZ *p1 = DATUM_POINT3DZ_P(tinstant_value(start1));
    const POINT3DZ *p2 = DATUM_POINT3DZ_P(tinstant_value(end1));
    const POINT3DZ *p3 = DATUM_POINT3DZ_P(tinstant_value(start2));
    const POINT3DZ *p4 = DATUM_POINT3DZ_P(tinstant_value(end2));
    x1 = p1->x; y1 = p1->y; z1 = p1->z;
    x2 = p2->x; y2 = p2->y; z2 = p2->z;
    x3 = p3->x; y3 = p3->y; z3 = p3->z;
    x4 = p4->x; y4 = p4->y; z4 = p4->z;
    /* Segments intersecting in the boundaries */
    if ((float8_eq(x1, x3) && float8_eq(y1, y3) && float8_eq(z1, z3)) ||
        (float8_eq(x2, x4) && float8_eq(y2, y4) && float8_eq(z2, z4)))
      return false;
  }
  else
  {
    const POINT2D *p1 = DATUM_POINT2D_P(tinstant_value(start1));
    const POINT2D *p2 = DATUM_POINT2D_P(tinstant_value(end1));
    const POINT2D *p3 = DATUM_POINT2D_P(tinstant_value(start2));
    const POINT2D *p4 = DATUM_POINT2D_P(tinstant_value(end2));
    x1 = p1->x; y1 = p1->y;
    x2 = p2->x; y2 = p2->y;
    x3 = p3->x; y3 = p3->y;
    x4 = p4->x; y4 = p4->y;
    /* Segments intersecting in the boundaries */
    if ((float8_eq(x1, x3) && float8_eq(y1, y3)) ||
        (float8_eq(x2, x4) && float8_eq(y2, y4)))
      return false;
  }

  long double xdenom = x2 - x1 - x4 + x3;
  long double ydenom = y2 - y1 - y4 + y3;
  long double zdenom = 0.0;
  if (hasz)
    zdenom = z2 - z1 - z4 + z3;
  if (xdenom == 0 && ydenom == 0 && zdenom == 0)
    /* Parallel segments */
    return false;

  /* Potentially avoid the division based on
   * Franklin Antonio, Faster Line Segment Intersection, Graphic Gems III
   * https://github.com/erich666/GraphicsGems/blob/master/gemsiii/insectc.c */
  long double fraction, xfraction = 0, yfraction = 0, zfraction = 0;
  if (xdenom != 0)
  {
    long double xnum = x3 - x1;
    if ((xdenom > 0 && (xnum < 0 || xnum > xdenom)) ||
        (xdenom < 0 && (xnum > 0 || xnum < xdenom)))
      return false;
    xfraction = xnum / xdenom;
    if (xfraction < -1 * MEOS_EPSILON || 1.0 + MEOS_EPSILON < xfraction)
      /* Intersection occurs out of the period */
      return false;
  }
  if (ydenom != 0)
  {
    long double ynum = y3 - y1;
    if ((ydenom > 0 && (ynum < 0 || ynum > ydenom)) ||
        (ydenom < 0 && (ynum > 0 || ynum < ydenom)))
      return false;
    yfraction = ynum / ydenom;
    if (yfraction < -1 * MEOS_EPSILON || 1.0 + MEOS_EPSILON < yfraction)
      /* Intersection occurs out of the period */
      return false;
  }
  if (hasz && zdenom != 0)
  {
    long double znum = z3 - z1;
    if ((zdenom > 0 && (znum < 0 || znum > zdenom)) ||
        (zdenom < 0 && (znum > 0 || znum < zdenom)))
      return false;
    zfraction = znum / zdenom;
    if (zfraction < -1 * MEOS_EPSILON || 1.0 + MEOS_EPSILON < zfraction)
      /* Intersection occurs out of the period */
      return false;
  }
  if (hasz)
  {
    /* If intersection occurs at different timestamps on each dimension */
    if ((xdenom != 0 && ydenom != 0 && zdenom != 0 &&
        fabsl(xfraction - yfraction) > MEOS_EPSILON &&
        fabsl(xfraction - zfraction) > MEOS_EPSILON) ||
      (xdenom == 0 && ydenom != 0 && zdenom != 0 &&
        fabsl(yfraction - zfraction) > MEOS_EPSILON) ||
      (xdenom != 0 && ydenom == 0 && zdenom != 0 &&
        fabsl(xfraction - zfraction) > MEOS_EPSILON) ||
      (xdenom != 0 && ydenom != 0 && zdenom == 0 &&
        fabsl(xfraction - yfraction) > MEOS_EPSILON))
      return false;
    if (xdenom != 0)
      fraction = xfraction;
    else if (ydenom != 0)
      fraction = yfraction;
    else
      fraction = zfraction;
  }
  else /* 2D */
  {
    /* If intersection occurs at different timestamps on each dimension */
    if (xdenom != 0 && ydenom != 0 &&
        fabsl(xfraction - yfraction) > MEOS_EPSILON)
      return false;
    fraction = xdenom != 0 ? xfraction : yfraction;
  }
  double duration = (double) (end1->t - start1->t);
  *t = start1->t + (TimestampTz) (duration * fraction);
  /* Note that due to roundoff errors it may be the case that the
   * resulting timestamp t may be equal to inst1->t or to inst2->t */
  if (*t <= start1->t || *t >= end1->t)
    return false;
  return true;
}

/**
 * @brief Return true if two segments of twp temporal geographic points
 * intersect at a timestamp
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

  long double duration = (long double) (end1->t - start1->t);
  *t = start1->t + (TimestampTz) (duration * fraction);
  /* Note that due to roundoff errors it may be the case that the
   * resulting timestamp t may be equal to inst1->t or to inst2->t */
  if (*t <= start1->t || *t >= end1->t)
    return false;
  return true;
}

/**
 * @brief Return true if the three values are collinear
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
    fabs(p2.x - p.x) <= MEOS_EPSILON && fabs(p2.y - p.y) <= MEOS_EPSILON &&
      fabs(p2.z - p.z) <= MEOS_EPSILON :
    fabs(p2.x - p.x) <= MEOS_EPSILON && fabs(p2.y - p.y) <= MEOS_EPSILON;
  return result;
}

/*****************************************************************************
 * Trajectory functions
 *****************************************************************************/

/**
 * @brief Return -1, 0, or 1 depending on whether the first LWPOINT
 * is less than, equal, or greater than the second one.
 * @pre The points are not empty and are of the same dimensionality
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
 * step/discrete or linear.
 * @note The function does not remove duplicate points, that is, repeated
 * points in a multipoint or consecutive equal points in a line string. This
 * should be done in the calling function.
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
 * @brief Compute the trajectory from two geometry points
 * @param[in] value1,value2 Points
 */
LWLINE *
lwline_make(Datum value1, Datum value2)
{
  /* Obtain the flags and the SRID from the first value */
  GSERIALIZED *gs = DatumGetGserializedP(value1);
  int srid = gserialized_get_srid(gs);
  int hasz = FLAGS_GET_Z(gs->gflags);
  int geodetic = FLAGS_GET_GEODETIC(gs->gflags);
  /* Since there is no M value a 0 value is passed */
  POINTARRAY *pa = ptarray_construct_empty((char) hasz, 0, 2);
  POINT4D pt;
  datum_point4d(value1, &pt);
  ptarray_append_point(pa, &pt, LW_TRUE);
  datum_point4d(value2, &pt);
  ptarray_append_point(pa, &pt, LW_TRUE);
  LWLINE *result = lwline_construct(srid, NULL, pa);
  FLAGS_SET_Z(result->flags, hasz);
  FLAGS_SET_GEODETIC(result->flags, geodetic);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Compute the trajectory of a temporal discrete sequence point
 * @param[in] seq Temporal value
 * @note Notice that this function does not remove duplicate points
 * @sqlfunc trajectory()
 */
GSERIALIZED *
tpointseq_disc_trajectory(const TSequence *seq)
{
  /* Singleton discrete sequence */
  if (seq->count == 1)
    return DatumGetGserializedP(tinstant_value_copy(TSEQUENCE_INST_N(seq, 0)));

  LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    Datum value = tinstant_value(TSEQUENCE_INST_N(seq, i));
    GSERIALIZED *gsvalue = DatumGetGserializedP(value);
    points[i] = lwgeom_from_gserialized(gsvalue);
  }
  LWGEOM *lwresult = lwpointarr_make_trajectory(points, seq->count, STEP);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return the trajectory of a temporal sequence point
 * @param[in] seq Temporal sequence
 * @note Since the sequence has been already validated there is no verification
 * of the input in this function, in particular for geographies it is supposed
 * that the composing points are geodetic
 * @sqlfunc trajectory()
 */
GSERIALIZED *
tpointseq_cont_trajectory(const TSequence *seq)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return DatumGetGserializedP(tinstant_value_copy(TSEQUENCE_INST_N(seq, 0)));

  LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
  /* Remove two consecutive points if they are equal */
  Datum value = tinstant_value(TSEQUENCE_INST_N(seq, 0));
  GSERIALIZED *gs = DatumGetGserializedP(value);
  points[0] = lwgeom_from_gserialized(gs);
  int npoints = 1;
  for (int i = 1; i < seq->count; i++)
  {
    value = tinstant_value(TSEQUENCE_INST_N(seq, i));
    gs = DatumGetGserializedP(value);
    LWPOINT *lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
    /* Remove two consecutive points if they are equal */
    if (! lwpoint_same(lwpoint, (LWPOINT *) points[npoints - 1]))
      points[npoints++] = (LWGEOM *) lwpoint;
  }
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  LWGEOM *lwresult = lwpointarr_make_trajectory(points, npoints, interp);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult);
  if (interp == LINEAR)
  {
    for (int i = 0; i < npoints; i++)
      lwgeom_free(points[i]);
    pfree(points);
  }
  return result;
}

/**
 * @brief Construct a geometry from an array of points and lines
 * @pre There is at least one geometry in both arrays
 */
LWGEOM *
lwcoll_from_points_lines(LWGEOM **points, LWGEOM **lines, int npoints,
  int nlines)
{
  assert(npoints > 0 || nlines > 0);
  LWGEOM *result, *respoints = NULL, *reslines = NULL;
  if (npoints > 0)
  {
    if (npoints == 1)
      respoints = points[0];
    else
    {
      /* There may be less points than the size of the array */
      LWGEOM **points1 = palloc(sizeof(LWGEOM *) * npoints);
      memcpy(points1, points, sizeof(LWGEOM *) * npoints);
      // TODO add the bounding box instead of ask PostGIS to compute it again
      respoints = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
        points[0]->srid, NULL, (uint32_t) npoints, points1);
      FLAGS_SET_Z(respoints->flags, FLAGS_GET_Z(points[0]->flags));
      FLAGS_SET_GEODETIC(respoints->flags, FLAGS_GET_GEODETIC(points[0]->flags));
    }
  }
  if (nlines > 0)
  {
    if (nlines == 1)
      reslines = (LWGEOM *) lines[0];
    else
    {
      /* There may be less lines than the size of the array */
      LWGEOM **lines1 = palloc(sizeof(LWGEOM *) * nlines);
      memcpy(lines1, lines, sizeof(LWGEOM *) * nlines);
      // TODO add the bounding box instead of ask PostGIS to compute it again
      reslines = (LWGEOM *) lwcollection_construct(MULTILINETYPE,
        lines[0]->srid, NULL, (uint32_t) nlines, lines1);
      FLAGS_SET_Z(reslines->flags, FLAGS_GET_Z(lines[0]->flags));
      FLAGS_SET_GEODETIC(reslines->flags, FLAGS_GET_GEODETIC(lines[0]->flags));
    }
  }
  /* If both points and lines */
  if (npoints > 0 && nlines > 0)
  {
    LWGEOM **geoms = palloc(sizeof(LWGEOM *) * 2);
    geoms[0] = respoints;
    geoms[1] = reslines;
    // TODO add the bounding box instead of ask PostGIS to compute it again
    result = (LWGEOM *) lwcollection_construct(COLLECTIONTYPE, respoints->srid,
      NULL, (uint32_t) 2, geoms);
    FLAGS_SET_Z(result->flags, FLAGS_GET_Z(respoints->flags));
    FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(respoints->flags));
  }
  /* If only points */
  else if (nlines == 0)
    result = respoints;
  /* If only lines */
  else /* npoints == 0 */
    result = reslines;
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return the trajectory of a temporal point with sequence set type
 * @sqlfunc trajectory()
 */
GSERIALIZED *
tpointseqset_trajectory(const TSequenceSet *ss)
{
  /* Singleton sequence set */
  if (ss->count == 1)
    return tpointseq_cont_trajectory(TSEQUENCESET_SEQ_N(ss, 0));

  int32 srid = tpointseqset_srid(ss);
  bool linear = MEOS_FLAGS_GET_LINEAR(ss->flags);
  bool hasz = MEOS_FLAGS_GET_Z(ss->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(ss->flags);
  LWGEOM **points = palloc(sizeof(LWGEOM *) * ss->totalcount);
  LWGEOM **lines = palloc(sizeof(LWGEOM *) * ss->count);
  int npoints = 0, nlines = 0;
  /* Iterate as in #tpointseq_cont_trajectory accumulating the results */
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    Datum value = tinstant_value(TSEQUENCE_INST_N(seq, 0));
    GSERIALIZED *gs = DatumGetGserializedP(value);
    /* npoints is the current number of points so far, k is the number of
     * additional points from the current sequence */
    LWGEOM *lwpoint1 = lwgeom_from_gserialized(gs);
    points[npoints] = lwpoint1;
    int k = 1;
    for (int j = 1; j < seq->count; j++)
    {
      value = tinstant_value(TSEQUENCE_INST_N(seq, j));
      gs = DatumGetGserializedP(value);
      /* Do not add the point if it is equal to the previous ones */
      LWGEOM *lwpoint2 = lwgeom_from_gserialized(gs);
      if (! lwpoint_same((LWPOINT *) lwpoint1, (LWPOINT *) lwpoint2))
      {
        points[npoints + k++] = lwpoint2;
        lwpoint1 = lwpoint2;
      }
      else
        lwgeom_free(lwpoint2);
    }
    if (linear && k > 1)
    {
      lines[nlines] = (LWGEOM *) lwline_from_lwgeom_array(srid, (uint32_t) k,
        &points[npoints]);
      FLAGS_SET_Z(lines[nlines]->flags, hasz);
      FLAGS_SET_GEODETIC(lines[nlines]->flags, geodetic);
      nlines++;
      for (int j = 0; j < k; j++)
        lwgeom_free(points[npoints + j]);
    }
    else
      npoints += k;
  }
  LWGEOM *lwresult = lwcoll_from_points_lines(points, lines, npoints, nlines);
  FLAGS_SET_Z(lwresult->flags, hasz);
  FLAGS_SET_GEODETIC(lwresult->flags, geodetic);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult);
  pfree(points); pfree(lines);
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
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = DatumGetGserializedP(tinstant_value_copy((TInstant *) temp));
  else if (temp->subtype == TSEQUENCE)
    result = MEOS_FLAGS_GET_DISCRETE(temp->flags) ?
      tpointseq_disc_trajectory((TSequence *) temp) :
      tpointseq_cont_trajectory((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tpointseqset_trajectory((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return the SRID of a temporal instant point.
 * @sqlfunc SRID()
 */
int
tpointinst_srid(const TInstant *inst)
{
  GSERIALIZED *gs = DatumGetGserializedP(tinstant_value(inst));
  return gserialized_get_srid(gs);
}

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return the SRID of a temporal sequence point.
 * @sqlfunc SRID()
 */
int
tpointseq_srid(const TSequence *seq)
{
  STBox *box = TSEQUENCE_BBOX_PTR(seq);
  return box->srid;
}

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return the SRID of a temporal sequence set point.
 * @sqlfunc SRID()
 */
int
tpointseqset_srid(const TSequenceSet *ss)
{
  STBox *box = TSEQUENCESET_BBOX_PTR(ss);
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
  assert(temptype_subtype(temp->subtype));
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
 * @ingroup libmeos_internal_temporal_spatial_transf
 * @brief Set the SRID of a temporal instant point
 * @sqlfunc setSRID()
 */
TInstant *
tpointinst_set_srid(const TInstant *inst, int32 srid)
{
  TInstant *result = tinstant_copy(inst);
  GSERIALIZED *gs = DatumGetGserializedP(tinstant_value(result));
  gserialized_set_srid(gs, srid);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_spatial_transf
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
    const TInstant *inst = TSEQUENCE_INST_N(result, i);
    GSERIALIZED *gs = DatumGetGserializedP(tinstant_value(inst));
    gserialized_set_srid(gs, srid);
  }
  /* Set the SRID of the bounding box */
  STBox *box = TSEQUENCE_BBOX_PTR(result);
  box->srid = srid;
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_spatial_transf
 * @brief Set the SRID of a temporal sequence set point
 * @sqlfunc setSRID()
 */
TSequenceSet *
tpointseqset_set_srid(const TSequenceSet *ss, int32 srid)
{
  STBox *box;
  TSequenceSet *result = tsequenceset_copy(ss);
  /* Loop for every composing sequence */
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(result, i);
    for (int j = 0; j < seq->count; j++)
    {
      /* Set the SRID of the composing points */
      const TInstant *inst = TSEQUENCE_INST_N(seq, j);
      GSERIALIZED *gs = DatumGetGserializedP(tinstant_value(inst));
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
 * @brief Coerce coordinate values into range [-180 -90, 180 90] for GEOGRAPHY
 * @note Transposed from PostGIS function ptarray_force_geodetic in file
 * lwgeodetic.c. We do not issue a warning.
 */
void
pt_force_geodetic(LWPOINT *point)
{
  POINT4D pt;
  getPoint4d_p(point->point, 0, &pt);
  if ( pt.x < -180.0 || pt.x > 180.0 || pt.y < -90.0 || pt.y > 90.0 )
  {
    pt.x = longitude_degrees_normalize(pt.x);
    pt.y = latitude_degrees_normalize(pt.y);
    ptarray_set_point4d(point->point, 0, &pt);
  }
  FLAGS_SET_GEODETIC(point->flags, true);
  return;
}

/**
 * @ingroup libmeos_internal_temporal_spatial_transf
 * @brief Convert a temporal point from/to a geometry/geography point
 * @param[in] inst Temporal instant point
 * @param[in] oper True when transforming from geometry to geography,
 * false otherwise
 * @sqlop ::
 */
TInstant *
tgeompointinst_tgeogpointinst(const TInstant *inst, bool oper)
{
  GSERIALIZED *gs = DatumGetGserializedP(tinstant_value(inst));
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  /* Short circuit functions gserialized_geog_from_geom and
     gserialized_geom_from_geog since we know it is a point */
  if ((int) lwgeom->srid <= 0)
    lwgeom->srid = SRID_DEFAULT;
  if (oper == GEOM_TO_GEOG)
  {
    /* We cannot test the following without access to PROJ */
    // srid_check_latlong(lwgeom->srid);
    /* Coerce the coordinate values into [-180 -90, 180 90] for GEOGRAPHY */
    pt_force_geodetic((LWPOINT *) lwgeom);
    lwgeom_set_geodetic(lwgeom, true);
  }
  else
  {
    lwgeom_set_geodetic(lwgeom, false);
  }
  GSERIALIZED *newgs = geo_serialize(lwgeom);
  TInstant *result = tinstant_make(PointerGetDatum(newgs),
    (oper == GEOM_TO_GEOG) ? T_TGEOGPOINT : T_TGEOMPOINT, inst->t);
  pfree(newgs);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_spatial_transf
 * @brief Convert a temporal point from/to a geometry/geography point
 * @param[in] seq Temporal sequence point
 * @param[in] oper True when transforming from geometry to geography,
 * false otherwise
 * @sqlop ::
 */
TSequence *
tgeompointseq_tgeogpointseq(const TSequence *seq, bool oper)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    instants[i] = tgeompointinst_tgeogpointinst(inst, oper);
  }
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @ingroup libmeos_internal_temporal_spatial_transf
 * @brief Convert a temporal point from/to a geometry/geography point
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
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
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
  assert(temptype_subtype(temp->subtype));
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
 * Length functions
 *****************************************************************************/

/**
 * @brief Return the length traversed by a temporal sequence point with plannar
 * coordinates
 * @pre The temporal point has linear interpolation
 */
static double
tpointseq_length_2d(const TSequence *seq)
{
  double result = 0;
  Datum start = tinstant_value(TSEQUENCE_INST_N(seq, 0));
  const POINT2D *p1 = DATUM_POINT2D_P(start);
  for (int i = 1; i < seq->count; i++)
  {
    Datum end = tinstant_value(TSEQUENCE_INST_N(seq, i));
    const POINT2D *p2 = DATUM_POINT2D_P(end);
    result += sqrt( ((p1->x - p2->x) * (p1->x - p2->x)) +
      ((p1->y - p2->y) * (p1->y - p2->y)) );
    p1 = p2;
  }
  return result;
}

/**
 * @brief Return the length traversed by a temporal sequence point with plannar
 * coordinates
 * @pre The temporal point has linear interpolation
 */
static double
tpointseq_length_3d(const TSequence *seq)
{
  double result = 0;
  Datum start = tinstant_value(TSEQUENCE_INST_N(seq, 0));
  const POINT3DZ *p1 = DATUM_POINT3DZ_P(start);
  for (int i = 1; i < seq->count; i++)
  {
    Datum end = tinstant_value(TSEQUENCE_INST_N(seq, i));
    const POINT3DZ *p2 = DATUM_POINT3DZ_P(end);
    result += sqrt( ((p1->x - p2->x)*(p1->x - p2->x)) +
      ((p1->y - p2->y)*(p1->y - p2->y)) +
      ((p1->z - p2->z)*(p1->z - p2->z)) );
    p1 = p2;
  }
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return the length traversed by a temporal sequence point.
 * @sqlfunc length()
 */
double
tpointseq_length(const TSequence *seq)
{
  assert(MEOS_FLAGS_GET_LINEAR(seq->flags));
  if (seq->count == 1)
    return 0;

  if (! MEOS_FLAGS_GET_GEODETIC(seq->flags))
  {
    return MEOS_FLAGS_GET_Z(seq->flags) ?
      tpointseq_length_3d(seq) : tpointseq_length_2d(seq);
  }
  else
  {
    /* We are sure that the trajectory is a line */
    GSERIALIZED *traj = tpointseq_cont_trajectory(seq);
    double result = gserialized_geog_length(traj, true);
    pfree(traj);
    return result;
  }
}

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return the length traversed by a temporal sequence set point.
 * @sqlfunc length()
 */
double
tpointseqset_length(const TSequenceSet *ss)
{
  assert(MEOS_FLAGS_GET_LINEAR(ss->flags));
  double result = 0;
  for (int i = 0; i < ss->count; i++)
    result += tpointseq_length(TSEQUENCESET_SEQ_N(ss, i));
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
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || ! MEOS_FLAGS_GET_LINEAR(temp->flags))
    ;
  else if (temp->subtype == TSEQUENCE)
    result = tpointseq_length((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tpointseqset_length((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return the cumulative length traversed by a temporal point.
 * @pre The sequence has linear interpolation
 * @sqlfunc cumulativeLength()
 */
TSequence *
tpointseq_cumulative_length(const TSequence *seq, double prevlength)
{
  assert(MEOS_FLAGS_GET_LINEAR(seq->flags));
  const TInstant *inst1;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = TSEQUENCE_INST_N(seq, 0);
    TInstant *inst = tinstant_make(Float8GetDatum(prevlength), T_TFLOAT,
      inst1->t);
    TSequence *result = tinstant_to_tsequence(inst, LINEAR);
    pfree(inst);
    return result;
  }

  /* General case */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  datum_func2 func = pt_distance_fn(seq->flags);
  inst1 = TSEQUENCE_INST_N(seq, 0);
  Datum value1 = tinstant_value(inst1);
  double length = prevlength;
  instants[0] = tinstant_make(Float8GetDatum(length), T_TFLOAT, inst1->t);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_value(inst2);
    if (! datum_point_eq(value1, value2))
      length += DatumGetFloat8(func(value1, value2));
    instants[i] = tinstant_make(Float8GetDatum(length), T_TFLOAT, inst2->t);
    value1 = value2;
  }
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, LINEAR, NORMALIZE);
}

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
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
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    sequences[i] = tpointseq_cumulative_length(seq, length);
    /* sequences[i] may have less sequences than seq->count due to normalization */
    const TInstant *end = TSEQUENCE_INST_N(sequences[i], sequences[i]->count - 1);
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
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || ! MEOS_FLAGS_GET_LINEAR(temp->flags))
    result = temporal_from_base_temp(Float8GetDatum(0.0), T_TFLOAT, temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tpointseq_cumulative_length((TSequence *) temp, 0);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tpointseqset_cumulative_length((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return the convex hull of a temporal point.
 * @sqlfunc convexHull()
 */
GSERIALIZED *
tpoint_convex_hull(const Temporal *temp)
{
  GSERIALIZED *traj = tpoint_trajectory(temp);
  GSERIALIZED *result = gserialized_convex_hull(traj);
  pfree(traj);
  return result;
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return the speed of a temporal point.
 * @pre The temporal point has linear interpolation
 * @sqlfunc speed()
 */
TSequence *
tpointseq_speed(const TSequence *seq)
{
  assert(MEOS_FLAGS_GET_LINEAR(seq->flags));

  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  /* General case */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  datum_func2 func = pt_distance_fn(seq->flags);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  Datum value1 = tinstant_value(inst1);
  double speed = 0.0; /* make compiler quiet */
  for (int i = 0; i < seq->count - 1; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
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
    seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE);
  pfree_array((void **) instants, seq->count - 1);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return the speed of a temporal point
 * @sqlfunc speed()
 */
TSequenceSet *
tpointseqset_speed(const TSequenceSet *ss)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (seq->count > 1)
      sequences[nseqs++] = tpointseq_speed(seq);
  }
  /* The resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
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
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || ! MEOS_FLAGS_GET_LINEAR(temp->flags))
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
 * each of its coordinates (iterator function).
 */
void
tpointseq_twcentroid_iter(const TSequence *seq, bool hasz, interpType interp,
  TSequence **seqx, TSequence **seqy, TSequence **seqz)
{
  TInstant **instantsx = palloc(sizeof(TInstant *) * seq->count);
  TInstant **instantsy = palloc(sizeof(TInstant *) * seq->count);
  TInstant **instantsz = hasz ?
    palloc(sizeof(TInstant *) * seq->count) : NULL;

  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    POINT4D p;
    datum_point4d(tinstant_value(inst), &p);
    instantsx[i] = tinstant_make(Float8GetDatum(p.x), T_TFLOAT, inst->t);
    instantsy[i] = tinstant_make(Float8GetDatum(p.y), T_TFLOAT, inst->t);
    if (hasz)
      instantsz[i] = tinstant_make(Float8GetDatum(p.z), T_TFLOAT, inst->t);
  }
  *seqx = tsequence_make_free(instantsx, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, interp, NORMALIZE);
  *seqy = tsequence_make_free(instantsy, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, interp, NORMALIZE);
  if (hasz)
    *seqz = tsequence_make_free(instantsz, seq->count, seq->period.lower_inc,
      seq->period.upper_inc, interp, NORMALIZE);
  return;
}

/**
 * @ingroup libmeos_internal_temporal_agg
 * @brief Return the time-weighed centroid of a temporal geometry point.
 * @sqlfunc twCentroid()
 */
GSERIALIZED *
tpointseq_twcentroid(const TSequence *seq)
{
  int srid = tpointseq_srid(seq);
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  TSequence *seqx, *seqy, *seqz;
  tpointseq_twcentroid_iter(seq, hasz, interp, &seqx, &seqy, &seqz);
  double twavgx = tnumberseq_twavg(seqx);
  double twavgy = tnumberseq_twavg(seqy);
  double twavgz = (hasz) ? tnumberseq_twavg(seqz) : 0.0;
  GSERIALIZED *result = gspoint_make(twavgx, twavgy, twavgz, hasz, false, srid);
  pfree(seqx); pfree(seqy);
  if (hasz)
    pfree(seqz);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_agg
 * @brief Return the time-weighed centroid of a temporal geometry point.
 * @sqlfunc twCentroid()
 */
GSERIALIZED *
tpointseqset_twcentroid(const TSequenceSet *ss)
{
  int srid = tpointseqset_srid(ss);
  bool hasz = MEOS_FLAGS_GET_Z(ss->flags);
  interpType interp = MEOS_FLAGS_GET_INTERP(ss->flags);
  TSequence **sequencesx = palloc(sizeof(TSequence *) * ss->count);
  TSequence **sequencesy = palloc(sizeof(TSequence *) * ss->count);
  TSequence **sequencesz = hasz ?
    palloc(sizeof(TSequence *) * ss->count) : NULL;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    tpointseq_twcentroid_iter(seq, hasz, interp, &sequencesx[i], &sequencesy[i],
      &sequencesz[i]);
  }
  TSequenceSet *ssx = tsequenceset_make_free(sequencesx, ss->count, NORMALIZE);
  TSequenceSet *ssy = tsequenceset_make_free(sequencesy, ss->count, NORMALIZE);
  TSequenceSet *ssz = hasz ?
    tsequenceset_make_free(sequencesz, ss->count, NORMALIZE) : NULL;

  double twavgx = tnumberseqset_twavg(ssx);
  double twavgy = tnumberseqset_twavg(ssy);
  double twavgz = hasz ? tnumberseqset_twavg(ssz) : 0;
  GSERIALIZED *result = gspoint_make(twavgx, twavgy, twavgz, hasz, false, srid);
  pfree(ssx); pfree(ssy);
  if (hasz)
    pfree(ssz);
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
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = DatumGetGserializedP(tinstant_value_copy((TInstant *) temp));
  else if (temp->subtype == TSEQUENCE)
    result = tpointseq_twcentroid((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tpointseqset_twcentroid((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Direction
 *****************************************************************************/

/**
 * @brief Return the azimuth of the two geometry points
 */
static Datum
geom_azimuth(Datum geom1, Datum geom2)
{
  const POINT2D *p1 = DATUM_POINT2D_P(geom1);
  const POINT2D *p2 = DATUM_POINT2D_P(geom2);
  double result;
  azimuth_pt_pt(p1, p2, &result);
  return Float8GetDatum(result);
}

/**
 * @brief Return the azimuth the two geography points
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
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return the direction of a temporal point.
 * @param[in] seq Temporal value
 * @param[out] result Azimuth between the first and last point
 * @result True when it is possible to determine the azimuth, i.e., when there
 * are at least two points that are not equal; false, otherwise.
 * @sqlfunc direction()
 */
bool
tpointseq_direction(const TSequence *seq, double *result)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return false;

  /* Determine the PostGIS function to call */
  datum_func2 func = MEOS_FLAGS_GET_GEODETIC(seq->flags) ?
    &geog_azimuth : &geom_azimuth;

  /* We are sure that there are at least 2 instants */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq, seq->count - 1);
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  if (datum_point_eq(value1, value2))
    return false;

  *result = DatumGetFloat8(func(value1, value2));
  return true;
}

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return the direction of a temporal point.
 * @param[in] ss Temporal value
 * @param[out] result Azimuth between the first and last point
 * @result True when it is possible to determine the azimuth, i.e., when there
 * are at least two points that are not equal; false, otherwise.
 * @sqlfunc direction()
 */
bool
tpointseqset_direction(const TSequenceSet *ss, double *result)
{
  /* Singleton sequence set */
  if (ss->count == 1)
    return tpointseq_direction(TSEQUENCESET_SEQ_N(ss, 0), result);

  /* Determine the PostGIS function to call */
  datum_func2 func = MEOS_FLAGS_GET_GEODETIC(ss->flags) ?
    &geog_azimuth : &geom_azimuth;

  /* We are sure that there are at least 2 instants */
  const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss, 0);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq1, 0);
  const TSequence *seq2 = TSEQUENCESET_SEQ_N(ss, ss->count - 1);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq2, seq2->count - 1);
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  if (datum_point_eq(value1, value2))
    return false;

  *result = DatumGetFloat8(func(value1, value2));
  return true;
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return the direction of a temporal point.
 * @sqlfunc direction()
 */
bool
tpoint_direction(const Temporal *temp, double *result)
{
  bool found = false;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    ;
  else if (temp->subtype == TSEQUENCE)
    found = tpointseq_direction((TSequence *) temp, result);
  else /* temp->subtype == TSEQUENCESET */
    found = tpointseqset_direction((TSequenceSet *) temp, result);
  return found;
}

/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

/**
 * @brief Return the temporal azimuth of a temporal geometry point
 * (iterator function).
 * @param[in] seq Temporal value
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 */
static int
tpointseq_azimuth_iter(const TSequence *seq, TSequence **result)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return 0;

  /* Determine the PostGIS function to call */
  datum_func2 func = MEOS_FLAGS_GET_GEODETIC(seq->flags) ?
    &geog_azimuth : &geom_azimuth;

  /* We are sure that there are at least 2 instants */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  Datum value1 = tinstant_value(inst1);
  int ninsts = 0, nseqs = 0;
  Datum azimuth = 0; /* Make the compiler quiet */
  bool lower_inc = seq->period.lower_inc;
  bool upper_inc = false; /* make compiler quiet */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_value(inst2);
    upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    if (! datum_point_eq(value1, value2))
    {
      azimuth = func(value1, value2);
      instants[ninsts++] = tinstant_make(azimuth, T_TFLOAT, inst1->t);
    }
    else
    {
      if (ninsts != 0)
      {
        instants[ninsts++] = tinstant_make(azimuth, T_TFLOAT, inst1->t);
        upper_inc = true;
        /* Resulting sequence has step interpolation */
        result[nseqs++] = tsequence_make((const TInstant **) instants, ninsts,
          lower_inc, upper_inc, STEP, NORMALIZE);
        for (int j = 0; j < ninsts; j++)
          pfree(instants[j]);
        ninsts = 0;
      }
      lower_inc = true;
    }
    inst1 = inst2;
    value1 = value2;
  }
  if (ninsts != 0)
  {
    instants[ninsts++] = tinstant_make(azimuth, T_TFLOAT, inst1->t);
    /* Resulting sequence has step interpolation */
    result[nseqs++] = tsequence_make((const TInstant **) instants, ninsts,
      lower_inc, upper_inc, STEP, NORMALIZE);
  }

  pfree(instants);
  return nseqs;
}

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return the temporal azimuth of a temporal geometry point.
 * @sqlfunc azimuth()
 */
TSequenceSet *
tpointseq_azimuth(const TSequence *seq)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count);
  int count = tpointseq_azimuth_iter(seq, sequences);
  /* Resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return the temporal azimuth of a temporal geometry point.
 * @sqlfunc azimuth()
 */
TSequenceSet *
tpointseqset_azimuth(const TSequenceSet *ss)
{
  if (ss->count == 1)
    return tpointseq_azimuth(TSEQUENCESET_SEQ_N(ss, 0));

  TSequence **sequences = palloc(sizeof(TSequence *) * ss->totalcount);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    nseqs += tpointseq_azimuth_iter(seq, &sequences[nseqs]);
  }
  /* Resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
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
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || ! MEOS_FLAGS_GET_LINEAR(temp->flags))
    ;
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tpointseq_azimuth((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tpointseqset_azimuth((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return the temporal angular difference of a temporal geometry point.
 * @sqlfunc azimuth()
 */
Temporal *
tpoint_angular_difference(const Temporal *temp)
{
  Temporal *tazimuth = tpoint_azimuth(temp);
  Temporal *result = NULL;
  if (tazimuth)
  {
    Temporal *tazimuth_deg = tfloat_degrees(tazimuth, false);
    result = tnumber_angular_difference(tazimuth_deg);
    pfree(tazimuth_deg);
  }
  return result;
}

/*****************************************************************************
 * Temporal bearing
 *****************************************************************************/

/**
 * @brief Normalize the bearing from -180° to + 180° (in radians) to
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
 * @brief Compute the bearing between two geometric points
 */
static Datum
geom_bearing(Datum point1, Datum point2)
{
  const POINT2D *p1 = DATUM_POINT2D_P(point1);
  const POINT2D *p2 = DATUM_POINT2D_P(point2);
  if ((fabs(p1->x - p2->x) <= MEOS_EPSILON) &&
      (fabs(p1->y - p2->y) <= MEOS_EPSILON))
    return Float8GetDatum(0.0);
  if (fabs(p1->y - p2->y) > MEOS_EPSILON)
  {
    double bearing = pg_datan((p1->x - p2->x) / (p1->y - p2->y)) +
      alpha(p1, p2);
    if (fabs(bearing) <= MEOS_EPSILON)
      bearing = 0.0;
    return Float8GetDatum(bearing);
  }
  if (p1->x < p2->x)
    return Float8GetDatum(M_PI / 2.0);
  else
    return Float8GetDatum(M_PI * 3.0 / 2.0);
}

/**
 * @brief Compute the bearing between two geographic points
 * @note Derived from https://gist.github.com/jeromer/2005586
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
  const POINT2D *p1 = DATUM_POINT2D_P(point1);
  const POINT2D *p2 = DATUM_POINT2D_P(point2);
  if ((fabs(p1->x - p2->x) <= MEOS_EPSILON) &&
      (fabs(p1->y - p2->y) <= MEOS_EPSILON))
    return Float8GetDatum(0.0);

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
 * @brief Select the appropriate bearing function
 */
static datum_func2
get_bearing_fn(int16 flags)
{
  datum_func2 result;
  if (MEOS_FLAGS_GET_GEODETIC(flags))
    result = &geog_bearing;
  else
    result = &geom_bearing;
  return result;
}

/**
 * @brief Return the value and timestamp at which the a temporal point segment
 * and a point are at the minimum bearing
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
  Datum point, meosType basetypid __attribute__((unused)), Datum *value,
  TimestampTz *t)
{
  Datum dstart = tinstant_value(start);
  Datum dend = tinstant_value(end);
  const POINT2D *p1 = DATUM_POINT2D_P(dstart);
  const POINT2D *p2 = DATUM_POINT2D_P(dend);
  const POINT2D *p = DATUM_POINT2D_P(point);
  const POINT2D *q;
  long double fraction;
  Datum proj = 0; /* make compiler quiet */
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(start->flags);
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
  if (fraction <= MEOS_EPSILON || fraction >= (1.0 - MEOS_EPSILON))
    return false;
  long double duration = (long double) (end->t - start->t);
  *t = start->t + (TimestampTz) (duration * fraction);
  *value = (Datum) 0;
  /* Compute the projected value only for geometries */
  if (! geodetic)
    proj = tsegment_value_at_timestamp(start, end, true, *t);
  q = DATUM_POINT2D_P(proj);
  /* We add a turning point only if p is to the North of q */
  return FP_GTEQ(p->y, q->y) ? true : false;
}

/**
 * @brief Return the value and timestamp at which the two temporal point
 * segments are at the minimum bearing
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
  assert(! MEOS_FLAGS_GET_GEODETIC(start1->flags));
  const POINT2D *sp1 = DATUM_POINT2D_P(tinstant_value(start1));
  const POINT2D *ep1 = DATUM_POINT2D_P(tinstant_value(end1));
  const POINT2D *sp2 = DATUM_POINT2D_P(tinstant_value(start2));
  const POINT2D *ep2 = DATUM_POINT2D_P(tinstant_value(end2));
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
   * #tnumber_arithop_tp_at_timestamp1 in file tnumber_mathfuncs.c
   */
  if ((ep1->x - sp1->x) == 0.0 || (ep2->x - sp2->x) == 0.0)
    return false;

  long double d1 = (-1 * sp1->x) / (ep1->x - sp1->x);
  long double d2 = (-1 * sp2->x) / (ep2->x - sp2->x);
  long double min = Min(d1, d2);
  long double max = Max(d1, d2);
  long double fraction = min + (max - min)/2;
  long double duration = (long double) (end1->t - start1->t);
  if (fraction <= MEOS_EPSILON || fraction >= (1.0 - MEOS_EPSILON))
    /* Minimum/maximum occurs out of the period */
    return false;

  *t = start1->t + (TimestampTz) (duration * fraction);
  /* We need to verify that at timestamp t the first segment is to the
   * North of the second */
  Datum value1 = tsegment_value_at_timestamp(start1, end1, true, *t);
  Datum value2 = tsegment_value_at_timestamp(start2, end2, true, *t);
  sp1 = DATUM_POINT2D_P(value1);
  sp2 = DATUM_POINT2D_P(value2);
  if (sp1->y > sp2->y) // TODO Use MEOS_EPSILON
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
  lfinfo.reslinear = MEOS_FLAGS_GET_LINEAR(temp->flags);
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
  lfinfo.reslinear = MEOS_FLAGS_GET_LINEAR(temp1->flags) ||
    MEOS_FLAGS_GET_LINEAR(temp2->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = lfinfo.reslinear ?
    &tpointsegm_min_bearing_at_timestamp : NULL;
  Temporal *result = tfunc_temporal_temporal(temp1, temp2, &lfinfo);
  return result;
}

/*****************************************************************************/
