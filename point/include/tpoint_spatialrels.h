/*****************************************************************************
 *
 * tpoint_spatialrels.h
 *    Spatial relationships for temporal points.
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB contributors
 *
 * Permission to use, copy, modify, and distribute this software and its documentation for any purpose, without fee, and without a written agreement is hereby
 * granted, provided that the above copyright notice and this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
 * PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

#ifndef __TPOINT_SPATIALRELS_H__
#define __TPOINT_SPATIALRELS_H__

#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>
#include "lifting.h"

/*****************************************************************************/

extern Datum spatialrel(Datum value1, Datum value2, Datum param,
  LiftedFunctionInfo lfinfo);

extern Datum geom_contains(Datum geom1, Datum geom2);
extern Datum geom_containsproperly(Datum geom1, Datum geom2);
extern Datum geom_covers(Datum geom1, Datum geom2);
extern Datum geom_coveredby(Datum geom1, Datum geom2);
extern Datum geom_crosses(Datum geom1, Datum geom2);
extern Datum geom_disjoint(Datum geom1, Datum geom2);
extern Datum geom_equals(Datum geom1, Datum geom2);
extern Datum geom_intersects2d(Datum geom1, Datum geom2);
extern Datum geom_intersects3d(Datum geom1, Datum geom2);
extern Datum geom_overlaps(Datum geom1, Datum geom2);
extern Datum geom_touches(Datum geom1, Datum geom2);
extern Datum geom_within(Datum geom1, Datum geom2);
extern Datum geom_dwithin2d(Datum geom1, Datum geom2, Datum dist);
extern Datum geom_dwithin3d(Datum geom1, Datum geom2, Datum dist);
extern Datum geom_relate(Datum geom1, Datum geom2);
extern Datum geom_relate_pattern(Datum geom1, Datum geom2, Datum pattern);

extern Datum geog_covers(Datum geog1, Datum geog2);
extern Datum geog_coveredby(Datum geog1, Datum geog2);
extern Datum geog_intersects(Datum geog1, Datum geog2);
extern Datum geog_dwithin(Datum geog1, Datum geog2, Datum dist);

extern Datum contains_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum contains_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum contains_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum containsproperly_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum containsproperly_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum containsproperly_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum covers_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum covers_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum covers_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum coveredby_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum coveredby_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum coveredby_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum crosses_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum crosses_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum crosses_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum disjoint_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum disjoint_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum disjoint_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum equals_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum equals_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum equals_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum intersects_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum intersects_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum intersects_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum overlaps_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum overlaps_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum overlaps_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum touches_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum touches_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum touches_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum within_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum within_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum within_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum dwithin_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum dwithin_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum dwithin_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum relate_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum relate_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum relate_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum relate_pattern_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum relate_pattern_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum relate_pattern_tpoint_tpoint(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
