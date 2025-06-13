/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief Spatial relationships for temporal points.
 */

#ifndef __TGEO_SPATIALRELS_H__
#define __TGEO_SPATIALRELS_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"

#define INVERT_RESULT(result) (result < 0 ? -1 : (result > 0) ? 0 : 1)

/*****************************************************************************/

extern Datum datum_geom_contains(Datum geom1, Datum geom2);
extern Datum datum_geom_covers(Datum geom1, Datum geom2);
extern Datum datum_geom_disjoint2d(Datum geom1, Datum geom2);
extern Datum datum_geom_disjoint3d(Datum geom1, Datum geom2);
extern Datum datum_geog_disjoint(Datum geog1, Datum geog2);
extern Datum datum_geom_intersects2d(Datum geom1, Datum geom2);
extern Datum datum_geom_intersects3d(Datum geom1, Datum geom2);
extern Datum datum_geog_intersects(Datum geog1, Datum geog2);
extern Datum datum_geom_touches(Datum geom1, Datum geom2);
extern Datum datum_geom_dwithin2d(Datum geom1, Datum geom2, Datum dist);
extern Datum datum_geom_dwithin3d(Datum geom1, Datum geom2, Datum dist);
extern Datum datum_geog_dwithin(Datum geog1, Datum geog2, Datum dist);
extern Datum datum_geom_relate_pattern(Datum geog1, Datum geog2, Datum p);
extern Datum datum_geom_touches(Datum geom1, Datum geom2);

extern datum_func2 geo_disjoint_fn(int16 flags1, int16 flags2);
extern datum_func2 geo_disjoint_fn_geo(int16 flags1, uint8_t flags2);
extern datum_func2 geo_intersects_fn(int16 flags1, int16 flags2);
extern datum_func2 geo_intersects_fn_geo(int16 flags1, uint8_t flags2);
extern datum_func3 geo_dwithin_fn(int16 flags1, int16 flags2);
extern datum_func3 geo_dwithin_fn_geo(int16 flags1, uint8_t flags2);

/*****************************************************************************/

extern int tpointsegm_tdwithin_turnpt(Datum start1, Datum end1, Datum start2,
  Datum end2, Datum value, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2);

extern int spatialrel_geo_geo(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  Datum param, varfunc func, int numparam, bool invert);
extern int spatialrel_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2,
  Datum param, varfunc func, int numparam);

extern int ea_contains_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp, 
  bool ever);
extern int ea_contains_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool ever);
extern int ea_contains_tgeo_tgeo(const Temporal *temp, const Temporal *temp2,
  bool ever);
  
extern int ea_covers_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp,
  bool ever);
extern int ea_covers_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool ever);
extern int ea_covers_tgeo_tgeo(const Temporal *temp, const Temporal *temp2,
  bool ever);

extern int ea_disjoint_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp, 
  bool ever);
extern int ea_disjoint_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool ever);
extern int ea_disjoint_tgeo_tgeo(const Temporal *temp, const Temporal *temp2,
  bool ever);

extern int ea_intersects_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp, 
  bool ever);
extern int ea_intersects_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool ever);
extern int ea_intersects_tgeo_tgeo(const Temporal *temp, const Temporal *temp2,
  bool ever);

extern int ea_touches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool ever);
extern int ea_touches_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool ever);
extern int ea_touches_tgeo_tgeo(const Temporal *temp, const Temporal *temp2,
  bool ever);

extern int ea_dwithin_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  double dist, bool ever);
extern int ea_dwithin_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2,
  double dist, bool ever);

extern int ea_spatialrel_tspatial_geo(const Temporal *temp,
  const GSERIALIZED *gs, datum_func2 func, bool ever, bool invert);
extern int ea_spatialrel_tspatial_tspatial(const Temporal *temp1,
  const Temporal *temp2, datum_func2 func, bool ever);
  
/*****************************************************************************/

#endif 
