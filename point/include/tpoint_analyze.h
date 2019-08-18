/*****************************************************************************
 *
 * tpoint_analyze.h
 *	  Functions for gathering statistics from temporal point columns
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TPOINT_ANALYZE_H__
#define __TPOINT_ANALYZE_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <commands/vacuum.h>

/*****************************************************************************/

extern Datum tpoint_analyze(PG_FUNCTION_ARGS);
extern Datum tpoint_analyze_internal(VacAttrStats *stats, int durationType);
extern void tpoint_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
                                 int samplerows, double totalrows);

/*****************************************************************************/

#endif
