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
 * @brief Spatial functions for temporal points.
 */

#ifndef __TGEO_SPATIALFUNCS_H__
#define __TGEO_SPATIALFUNCS_H__

/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include "geo/geo_funcs.h"
#include "temporal/temporal.h"

/** Symbolic constants for transforming tgeompoint <-> tgeogpoint */
#define TGEOMP_TO_TGEOGP    true
#define TGEOGP_TO_TGEOMP    false

/** Symbolic constants for transforming tgeompoint <-> tgeogpoint */
#define TGEOM_TO_TGEOG      true
#define TGEOG_TO_TGEOM      false

/** Symbolic constants for transforming tgeo <-> tpoint */
#define TGEO_TO_TPOINT      true
#define TPOINT_TO_TGEO      false

/*****************************************************************************/

/* Utility functions */

extern GSERIALIZED *geo_values_collect(const Temporal *temp, bool unary_union);

extern void datum_point4d(Datum value, POINT4D *p);
extern bool datum_point_eq(Datum point1, Datum point2);
extern bool datum_point_same(Datum point1, Datum point2);
extern Datum datum2_point_eq(Datum point1, Datum point2);
extern Datum datum2_point_ne(Datum point1, Datum point2);
extern Datum datum2_point_same(Datum point1, Datum point2);
extern Datum datum2_point_nsame(Datum point1, Datum point2);
extern Datum datum2_geom_centroid(Datum geo);
extern Datum datum2_geog_centroid(Datum geo);

/* Generic functions */

extern datum_func2 geo_distance_fn(int16 flags);
extern datum_func2 pt_distance_fn(int16 flags);
extern Datum datum_geom_distance2d(Datum geom1, Datum geom2);
extern Datum datum_geom_distance3d(Datum geom1, Datum geom2);
extern Datum datum_geog_distance(Datum geog1, Datum geog2);
extern Datum datum_pt_distance2d(Datum geom1, Datum geom2);
extern Datum datum_pt_distance3d(Datum geom1, Datum geom2);
extern int16 spatial_flags(Datum d, MeosType basetype);

/* Validity functions */

extern bool ensure_spatial_validity(const Temporal *temp1,
  const Temporal *temp2);
extern int spheroid_init_from_srid(int32_t srid, SPHEROID *s);
extern bool ensure_same_geodetic_tspatial_geo(const Temporal *temp,
  const GSERIALIZED *gs);
extern bool ensure_same_geodetic_set_geo(const Set *s,
  const GSERIALIZED *gs);
extern bool same_dimensionality_tspatial_geo(const Temporal *temp,
  const GSERIALIZED *gs);
extern bool ensure_same_dimensionality_tspatial_geo(const Temporal *temp,
  const GSERIALIZED *gs);
extern bool ensure_same_spatial_dimensionality_stbox_geo(const STBox *box,
  const GSERIALIZED *gs);
extern bool ensure_same_geodetic_stbox_geo(const STBox *box,
  const GSERIALIZED *gs);
extern bool ensure_valid_stbox_geo(const STBox *box, const GSERIALIZED *gs);
extern bool ensure_valid_tspatial_geo(const Temporal *temp,
  const GSERIALIZED *gs);
extern bool ensure_valid_tspatial_tspatial(const Temporal *temp1,
  const Temporal *temp2);
extern bool ensure_valid_tgeo_stbox(const Temporal *temp, const STBox *box);
extern bool ensure_valid_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern bool ensure_valid_tgeo_tgeo(const Temporal *temp1,
  const Temporal *temp2);
extern bool ensure_valid_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern bool ensure_valid_tpoint_tpoint(const Temporal *temp1,
  const Temporal *temp2);


/* Functions for extracting coordinates */

extern Temporal *tpoint_get_coord(const Temporal *temp, int coord);

/* Ever/always comparisons */

extern int eacomp_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, MeosType), bool ever);

/* Functions derived from PostGIS to increase floating-point precision */


/* Functions specializing the PostGIS functions ST_LineInterpolatePoint and
 * ST_LineLocatePoint */

extern Datum pointsegm_interpolate(Datum start, Datum end,
  long double ratio);
extern long double pointsegm_locate(Datum start, Datum end, Datum point,
  double *dist);

/* Intersection functions */

extern int tgeompointsegm_intersection(Datum start1, Datum end1, Datum start2,
  Datum end2, TimestampTz lower, TimestampTz upper, TimestampTz *t1,
  TimestampTz *t2);
extern int tgeogpointsegm_intersection(Datum start1, Datum end1, Datum start2,
  Datum end2, TimestampTz lower, TimestampTz upper, TimestampTz *t1,
  TimestampTz *t2);

extern bool geopoint_collinear(Datum value1, Datum value2, Datum value3,
  double ratio, bool hasz, bool geodetic);

/* Trajectory functions */

extern LWLINE *lwline_make(Datum value1, Datum value2);

/* Stop function */

int tpointseq_stops_iter(const TSequence *seq, double maxdist, int64 mintunits,
  TSequence **result);

/*****************************************************************************/

#endif
