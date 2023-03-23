/*****************************************************************************
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
 * @brief Spatial functions for temporal points.
 */

#ifndef __TPOINT_SPATIALFUNCS_H__
#define __TPOINT_SPATIALFUNCS_H__

/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include "general/temporal.h"
#include "point/tpoint.h"

/** Symbolic constants for transforming tgeompoint <-> tgeogpoint */
#define GEOM_TO_GEOG        true
#define GEOG_TO_GEOM        false

/*****************************************************************************
 * Direct access to a single point in the GSERIALIZED struct
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

/**
 * @brief Definition for the internal aspects of  the GSERIALIZED struct
 */
// #define LWFLAG_EXTFLAGS    0x20
#define LWFLAG_VERSBIT2    0x80

// #define FLAGS_GET_EXTFLAGS(flags)     (((flags) & LWFLAG_EXTFLAGS)>>5)
#define FLAGS_GET_VERSBIT2(flags)     (((flags) & LWFLAG_VERSBIT2)>>7)

#define GS_POINT_PTR(gs)    ( (uint8_t *) gs->data + 8 + \
  FLAGS_GET_BBOX(gs->gflags) * FLAGS_NDIMS_BOX(gs->gflags) * 8 + \
  FLAGS_GET_VERSBIT2(gs->gflags) * 8 )

/**
 * @brief Return a pointer to a 2D/3DZ point from the datum/GSERIALIZED
 */
#define DATUM_POINT2D_P(gs)  ( (POINT2D *) GS_POINT_PTR(DatumGetGserializedP(gs)) )
#define DATUM_POINT3DZ_P(gs) ( (POINT3DZ *) GS_POINT_PTR(DatumGetGserializedP(gs)) )

#define GSERIALIZED_POINT2D_P(gs)  ( (POINT2D *) GS_POINT_PTR((gs)) )
#define GSERIALIZED_POINT3DZ_P(gs) ( (POINT3DZ *) GS_POINT_PTR((gs)) )

/*****************************************************************************/

/* Utility functions */

extern POINT2D datum_point2d(Datum value);
extern POINT3DZ datum_point3dz(Datum value);
extern void datum_point4d(Datum value, POINT4D *p);

extern bool datum_point_eq(Datum geopoint1, Datum geopoint2);
extern Datum datum2_point_eq(Datum geopoint1, Datum geopoint2);
extern Datum datum2_point_ne(Datum geopoint1, Datum geopoint2);

extern GSERIALIZED *geo_serialize(const LWGEOM *geom);
extern Datum datum_transform(Datum value, Datum srid);

/* Generic functions */

extern datum_func2 distance_fn(int16 flags);
extern datum_func2 pt_distance_fn(int16 flags);
extern Datum geom_distance2d(Datum geom1, Datum geom2);
extern Datum geom_distance3d(Datum geom1, Datum geom2);
extern Datum geog_distance(Datum geog1, Datum geog2);
extern Datum pt_distance2d(Datum geom1, Datum geom2);
extern Datum pt_distance3d(Datum geom1, Datum geom2);
extern Datum geom_intersection2d(Datum geom1, Datum geom2);

/* Parameter tests */

extern void ensure_spatial_validity(const Temporal *temp1,
  const Temporal *temp2);
extern void ensure_not_geodetic(int16 flags);
extern void ensure_same_geodetic(int16 flags1, int16 flags2);
extern void ensure_same_srid(int32_t srid1, int32_t srid2);
extern void ensure_same_srid_stbox(const STBox *box1, const STBox *box2);
extern void ensure_same_srid_tpoint_stbox(const Temporal *temp,
  const STBox *box);
extern void ensure_same_srid_stbox_gs(const STBox *box, const GSERIALIZED *gs);
extern void ensure_same_dimensionality(int16 flags1, int16 flags2);
extern void ensure_same_spatial_dimensionality(int16 flags1, int16 flags2);
extern void ensure_same_spatial_dimensionality_temp_box(int16 flags1, int16 flags2);
extern void ensure_same_dimensionality_gs(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2);
extern void ensure_same_dimensionality_tpoint_gs(const Temporal *temp,
  const GSERIALIZED *gs);
extern void ensure_same_spatial_dimensionality_stbox_gs(const STBox *box1,
  const GSERIALIZED *gs);
extern void ensure_has_Z(int16 flags);
extern void ensure_has_not_Z(int16 flags);
extern void ensure_has_Z_gs(const GSERIALIZED *gs);
extern void ensure_has_not_Z_gs(const GSERIALIZED *gs);
extern void ensure_has_M_gs(const GSERIALIZED *gs);
extern void ensure_has_not_M_gs(const GSERIALIZED *gs);
extern void ensure_point_type(const GSERIALIZED *gs);
extern void ensure_non_empty(const GSERIALIZED *gs);

/* Functions derived from PostGIS to increase floating-point precision */

extern long double closest_point2d_on_segment_ratio(const POINT2D *p,
  const POINT2D *A, const POINT2D *B, POINT2D *closest);
extern long double closest_point3dz_on_segment_ratio(const POINT3DZ *p,
  const POINT3DZ *A, const POINT3DZ *B, POINT3DZ *closest);
extern long double closest_point_on_segment_sphere(const POINT4D *p,
  const POINT4D *A, const POINT4D *B, POINT4D *closest, double *dist);
extern void interpolate_point4d_sphere(const POINT3D *p1, const POINT3D *p2,
  const POINT4D *v1, const POINT4D *v2, double f, POINT4D *p);

/* Functions specializing the PostGIS functions ST_LineInterpolatePoint and
 * ST_LineLocatePoint */

extern GSERIALIZED *gspoint_make(double x, double y, double z, bool hasz,
  bool geodetic, int32 srid);
extern Datum geosegm_interpolate_point(Datum start, Datum end,
  long double ratio);
extern long double geosegm_locate_point(Datum start, Datum end, Datum point,
  double *dist);

/* Intersection functions */

extern bool tpointsegm_intersection_value(const TInstant *inst1,
  const TInstant *inst2, Datum value, TimestampTz *t);
extern bool tgeompointsegm_intersection(const TInstant *start1,
  const TInstant *end1, const TInstant *start2, const TInstant *end2,
  TimestampTz *t);
extern bool tgeogpointsegm_intersection(const TInstant *start1,
  const TInstant *end1, const TInstant *start2, const TInstant *end2,
  TimestampTz *t);

extern bool geopoint_collinear(Datum value1, Datum value2, Datum value3,
  double ratio, bool hasz, bool geodetic);

/* Trajectory functions */

extern LWGEOM **lwpointarr_remove_duplicates(LWGEOM **points, int count,
  int *newcount);
extern LWGEOM *lwpointarr_make_trajectory(LWGEOM **lwpoints, int count,
  interpType interp);
extern LWLINE *lwline_make(Datum value1, Datum value2);

/* Functions for spatial reference systems */

extern TInstant *tpointinst_transform(const TInstant *inst, int srid);
extern TSequence *tpointdiscseq_transform(const TSequence *is, int srid);
extern TSequence *tpointcontseq_transform(const TSequence *seq, int srid);
extern TSequenceSet *tpointseqset_transform(const TSequenceSet *ss, int srid);
extern Temporal *tpoint_transform(const Temporal *temp, int srid);

/* Set precision of the coordinates */

extern Datum datum_round_geo(Datum value, Datum prec);
extern Temporal *tpoint_round(const Temporal *temp, int prec);

/* Functions for extracting coordinates */

extern Temporal *tpoint_get_coord(const Temporal *temp, int coord);

/* Restriction functions */

extern TSequence **tpointseq_at_geometry(const TSequence *seq,
  const GSERIALIZED *gs, int *count);
extern Temporal *tpoint_at_stbox1(const Temporal *temp, const STBox *box);
extern Temporal *tpoint_minus_stbox(const Temporal *temp, const STBox *box);
extern Span *tpointseq_interperiods(const TSequence *seq,
  GSERIALIZED *gsinter, int *count);

/*****************************************************************************/

#endif
