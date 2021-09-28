/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
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
 * @file tpoint_distance.h
 * Distance functions for temporal points.
 */

#ifndef __TPOINT_DISTANCE_H__
#define __TPOINT_DISTANCE_H__

#include <postgres.h>
#include <catalog/pg_type.h>

#include "general/temporal.h"
#include <liblwgeom.h>
#include "tpoint.h"

/*****************************************************************************/

extern double lw_dist_sphere_point_dist(const LWGEOM *lw1, const LWGEOM *lw2,
  int mode, long double *fraction);

/* Distance functions */

extern Datum distance_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum distance_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum distance_tpoint_tpoint(PG_FUNCTION_ARGS);

extern bool tpoint_min_dist_at_timestamp(const TInstant *start1,
  const TInstant *end1, bool linear1, const TInstant *start2,
  const TInstant *end2, bool linear2, TimestampTz *t);

extern Temporal *distance_tpoint_geo_internal(const Temporal *temp, Datum geo);
extern Temporal *distance_tpoint_tpoint_internal(const Temporal *temp1,
  const Temporal *temp2);

/* Nearest approach distance/instance and shortest line functions */

extern Datum NAI_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum NAI_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum NAI_tpoint_tpoint(PG_FUNCTION_ARGS);

extern TInstant *NAI_tpoint_geo_internal(FunctionCallInfo fcinfo,
  const Temporal *temp, GSERIALIZED *gs);

extern Datum NAD_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum NAD_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum NAD_geo_stbox(PG_FUNCTION_ARGS);
extern Datum NAD_stbox_geo(PG_FUNCTION_ARGS);
extern Datum NAD_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum NAD_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum NAD_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum NAD_tpoint_tpoint(PG_FUNCTION_ARGS);

extern double NAD_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);

extern Datum shortestline_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum shortestline_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum shortestline_tpoint_tpoint(PG_FUNCTION_ARGS);

extern bool shortestline_tpoint_tpoint_internal(const Temporal *temp1,
  const Temporal *temp2, Datum *line);

/*****************************************************************************/

#endif
