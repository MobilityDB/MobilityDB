/*****************************************************************************
 *
 * tpoint_distance.h
 *    Distance functions for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TPOINT_DISTANCE_H__
#define __TPOINT_DISTANCE_H__

#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H 1

#include <postgres.h>
#include <catalog/pg_type.h>
#include <float.h>

#include "temporal.h"
#include <liblwgeom.h>
#include "tpoint.h"

/*****************************************************************************/

extern double lw_dist_sphere_point_dist(const LWGEOM *lw1, const LWGEOM *lw2,
  int mode, double *fraction);

/* Distance functions */

extern Datum distance_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum distance_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum distance_tpoint_tpoint(PG_FUNCTION_ARGS);

extern bool tpointseq_min_dist_at_timestamp(const TInstant *start1,
  const TInstant *end1, bool linear1, const TInstant *start2, 
  const TInstant *end2, bool linear2, TimestampTz *t);

extern Temporal *distance_tpoint_geo_internal(const Temporal *temp, Datum geo);
extern Temporal *distance_tpoint_tpoint_internal(const Temporal *temp1, const Temporal *temp2);

/* Nearest approach distance/instance and shortest line functions */

extern Datum NAI_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum NAI_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum NAI_tpoint_tpoint(PG_FUNCTION_ARGS);

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

/*****************************************************************************/

#endif
