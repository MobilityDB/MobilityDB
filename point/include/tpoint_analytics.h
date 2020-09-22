/*****************************************************************************
 *
 * tpoint_analytics.h
 *    Analytics functions for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TPOINT_ANALYTICS_H__
#define __TPOINT_ANALYTICS_H__

#include <postgres.h>
#include <catalog/pg_type.h>

/*****************************************************************************/

/* Convert a temporal point into a PostGIS trajectory geometry/geography */

extern Datum tpoint_to_geo(PG_FUNCTION_ARGS);
extern Datum geo_to_tpoint(PG_FUNCTION_ARGS);

/* Convert a temporal point and a temporal float into a PostGIS geometry/geography */

extern Datum point_to_geo_measure(PG_FUNCTION_ARGS);

/* Simple Douglas-Peucker-like value simplification for temporal floats and 
 * temporal points. */

extern Datum tfloat_simplify(PG_FUNCTION_ARGS);
extern Datum tpoint_simplify(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
