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
 * @file tpoint_spatialfuncs.h
 * Spatial functions for temporal points.
 */

#ifndef __TPOINT_SPATIALFUNCS_H__
#define __TPOINT_SPATIALFUNCS_H__

/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MobilityDB */
#include "general/temporal.h"
#include "point/tpoint.h"

/* Get the flags byte of a GSERIALIZED depending on the version */
#if POSTGIS_VERSION_NUMBER < 30000
#define GS_FLAGS(gs) (gs->flags)
#else
#define GS_FLAGS(gs) (gs->gflags)
#endif

/** Symbolic constants for transforming tgeompoint <-> tgeogpoint */
#define GEOM_TO_GEOG        true
#define GEOG_TO_GEOM        false

/*****************************************************************************/

/* Fetch from and store in the cache the fcinfo of the external function */

extern FunctionCallInfo fetch_fcinfo();
extern void store_fcinfo(FunctionCallInfo fcinfo);

/* Utility functions */

extern POINT2D datum_point2d(Datum value);
extern POINT3DZ datum_point3dz(Datum value);
extern void datum_point4d(Datum value, POINT4D *p);

extern const POINT2D *datum_point2d_p(Datum value);
extern const POINT2D *gserialized_point2d_p(const GSERIALIZED *gs);
extern const POINT3DZ *datum_point3dz_p(Datum value);
extern const POINT3DZ *gserialized_point3dz_p(const GSERIALIZED *gs);

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
// extern void ensure_not_geodetic_gs(const GSERIALIZED *gs);
extern void ensure_not_geodetic(int16 flags);
extern void ensure_same_geodetic(int16 flags1, int16 flags2);
extern void ensure_same_geodetic_gs(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2);
extern void ensure_same_srid(int32_t srid1, int32_t srid2);
extern void ensure_same_srid_stbox(const STBOX *box1, const STBOX *box2);
extern void ensure_same_srid_tpoint_stbox(const Temporal *temp,
  const STBOX *box);
extern void ensure_same_srid_stbox_gs(const STBOX *box, const GSERIALIZED *gs);
extern void ensure_same_dimensionality(int16 flags1, int16 flags2);
extern void ensure_same_spatial_dimensionality(int16 flags1, int16 flags2);
extern void ensure_same_dimensionality_gs(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2);
extern void ensure_same_dimensionality_tpoint_gs(const Temporal *temp,
  const GSERIALIZED *gs);
extern void ensure_same_spatial_dimensionality_stbox_gs(const STBOX *box1,
  const GSERIALIZED *gs);
extern void ensure_has_Z(int16 flags);
extern void ensure_has_not_Z(int16 flags);
extern void ensure_has_Z_gs(const GSERIALIZED *gs);
extern void ensure_has_not_Z_gs(const GSERIALIZED *gs);
extern void ensure_has_M_gs(const GSERIALIZED *gs);
extern void ensure_has_not_M_gs(const GSERIALIZED *gs);
extern void ensure_point_type(const GSERIALIZED *gs);
extern void ensure_non_empty(const GSERIALIZED *gs);

/* Ever equal comparison operator */

extern bool tpointinst_ever_eq(const TInstant *inst, Datum value);
extern bool tpointinstset_ever_eq(const TInstantSet *ti, Datum value);
extern bool tpointseq_ever_eq(const TSequence *seq, Datum value);
extern bool tpointseqset_ever_eq(const TSequenceSet *ts, Datum value);
extern bool tpoint_ever_eq(const Temporal *temp, Datum value);

extern bool tpointinst_always_eq(const TInstant *inst, Datum value);
extern bool tpointinstset_always_eq(const TInstantSet *ti, Datum value);
extern bool tpointseq_always_eq(const TSequence *seq, Datum value);
extern bool tpointseqset_always_eq(const TSequenceSet *ts, Datum value);
extern bool tpoint_always_eq(const Temporal *temp, Datum value);

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

extern Datum point_make(double x, double y, double z, bool hasz,
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

extern LWGEOM *lwpointarr_make_trajectory(LWGEOM **lwpoints, int count,
  bool linear);
extern LWLINE *lwline_make(Datum value1, Datum value2);
extern Datum line_make(Datum value1, Datum value2);

extern Datum tpointinstset_trajectory(const TInstantSet *ti);
extern Datum tpointseq_trajectory(const TSequence *seq);
extern Datum tpointseqset_trajectory(const TSequenceSet *ts);
extern Datum tpoint_trajectory(const Temporal *temp);

/* Functions for spatial reference systems */

extern int tpointinst_srid(const TInstant *inst);
extern int tpointinstset_srid(const TInstantSet *ti);
extern int tpointseq_srid(const TSequence *seq);
extern int tpointseqset_srid(const TSequenceSet *ts);
extern int tpoint_srid(const Temporal *t);

extern TInstant *tpointinst_set_srid(const TInstant *inst, int32 srid);
extern TInstantSet *tpointinstset_set_srid(const TInstantSet *ti, int32 srid);
extern TSequence *tpointseq_set_srid(const TSequence *seq, int32 srid);
extern TSequenceSet *tpointseqset_set_srid(const TSequenceSet *ts, int32 srid);
extern Temporal *tpoint_set_srid(const Temporal *temp, int32 srid);

extern TInstant *tpointinst_transform(const TInstant *inst, Datum srid);
extern TInstantSet *tpointinstset_transform(const TInstantSet *ti, Datum srid);
extern TSequence *tpointseq_transform(const TSequence *seq, Datum srid);
extern TSequenceSet *tpointseqset_transform(const TSequenceSet *ts, Datum srid);
extern Temporal *tpoint_transform(const Temporal *temp, Datum srid);

/* Cast functions */

extern TInstant *tgeompointinst_tgeogpointinst(const TInstant *inst, bool oper);
extern TInstantSet *tgeompointinstset_tgeogpointinstset(const TInstantSet *ti,
  bool oper);
extern TSequence *tgeompointseq_tgeogpointseq(const TSequence *seq, bool oper);
extern TSequenceSet *tgeompointseqset_tgeogpointseqset(const TSequenceSet *ts,
  bool oper);
extern Temporal *tgeompoint_tgeogpoint(const Temporal *temp, bool oper);

/* Set precision of the coordinates */

extern Datum datum_round_geo(Datum value, Datum prec);
extern Temporal *tpoint_round(const Temporal *temp, Datum prec);

/* Functions for extracting coordinates */

extern Temporal *tpoint_get_coord(const Temporal *temp, int coord);

/* Length, speed, time-weighted centroid, temporal azimuth, and
 * temporal bearing functions */

extern double tpointseq_length(const TSequence *seq);
extern double tpointseqset_length(const TSequenceSet *ts);
extern double tpoint_length(const Temporal *temp);

extern TInstant *tpointinst_cumulative_length(const TInstant *inst);
extern TInstantSet *tpointinstset_cumulative_length(const TInstantSet *ti);
extern TSequence *tpointseq_cumulative_length(const TSequence *seq,
  double prevlength);
extern TSequenceSet *tpointseqset_cumulative_length(const TSequenceSet *ts);
extern Temporal *tpoint_cumulative_length(const Temporal *temp);

extern TSequence *tpointseq_speed(const TSequence *seq);
extern TSequenceSet *tpointseqset_speed(const TSequenceSet *ts);
extern Temporal *tpoint_speed(const Temporal *temp);

extern Datum tpointinstset_twcentroid(const TInstantSet *ti);
extern Datum tpointseq_twcentroid(const TSequence *seq);
extern Datum tpointseqset_twcentroid(const TSequenceSet *ts);
extern Datum tpoint_twcentroid(const Temporal *temp);

extern TSequenceSet *tpointseq_azimuth(const TSequence *seq);
extern TSequenceSet *tpointseqset_azimuth(const TSequenceSet *ts);
extern Temporal *tpoint_azimuth(const Temporal *temp);

extern bool bearing_geo_geo(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  Datum *result);
extern Temporal *bearing_tpoint_geo(const Temporal *temp,
  const GSERIALIZED *gs, bool invert);
extern Temporal *bearing_tpoint_tpoint(const Temporal *temp1,
  const Temporal *temp2);

/* Non self-intersecting (a.k.a. simple) functions */

extern bool tpointinstset_is_simple(const TInstantSet *ti);
extern bool tpointseq_is_simple(const TSequence *seq);
extern bool tpointseqset_is_simple(const TSequenceSet *ts);
extern bool tpoint_is_simple(const Temporal *temp);
extern TInstantSet **tpointinstset_make_simple(const TInstantSet *ti,
  int *count);
extern TSequence **tpointseq_make_simple(const TSequence *seq, int *count);
extern TSequence **tpointseqset_make_simple(const TSequenceSet *ts,
  int *count);
extern Temporal **tpoint_make_simple(const Temporal *temp, int *count);

/* Restriction functions */

extern TInstant *tpointinst_restrict_geometry(const TInstant *inst,
  Datum geom, bool atfunc);
extern TInstantSet *tpointinstset_restrict_geometry(const TInstantSet *ti,
  Datum geom, bool atfunc);
extern TSequenceSet *tpointseq_restrict_geometry(const TSequence *seq,
  Datum geom, bool atfunc);
extern TSequenceSet *tpointseqset_restrict_geometry(const TSequenceSet *ts,
  Datum geom, const STBOX *box, bool atfunc);
extern TSequence **tpointseq_at_geometry(const TSequence *seq, Datum geo,
  int *count);
extern Temporal *tpoint_restrict_geometry(const Temporal *temp,
  const GSERIALIZED *gs, bool atfunc);

extern Temporal *tpoint_at_stbox(const Temporal *temp, const STBOX *box,
  bool upper_inc);
extern Temporal *tpoint_minus_stbox(const Temporal *temp, const STBOX *box);
extern Temporal *tpoint_restrict_stbox(const Temporal *temp, const STBOX *box,
  bool atfunc);

extern Period **tpointseq_interperiods(const TSequence *seq,
  GSERIALIZED *gsinter, int *count);

/*****************************************************************************/

#endif
