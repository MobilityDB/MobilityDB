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

extern Datum point_to_geo_measure(PG_FUNCTION_ARGS);
extern Datum tfloat_simplify(PG_FUNCTION_ARGS);
extern Datum tpoint_simplify(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
