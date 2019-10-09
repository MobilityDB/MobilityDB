/*****************************************************************************
 *
 * time_analyze.h
 *	  Functions for gathering statistics from time type columns
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TIME_ANALYZE_H__
#define __TIME_ANALYZE_H__

#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>

/* 
 * It is not possible to differentiate bound histogram of ranges and of periods
 * with the combinination stakind/staop values, since the staop is not set by
 * the compute_range_stats function and thus it is necessary to define a new stakind
 */
#define STATISTIC_KIND_PERIOD_BOUNDS_HISTOGRAM  8
/* 
 * It is not possible to differentiate lengths of ranges and lengths of periods
 * with the combinination stakind/staop values, since the lenghts are expressed
 * with float8 values and thus it is necessary to define a new stakind
 */
#define STATISTIC_KIND_PERIOD_LENGTH_HISTOGRAM  9

/*****************************************************************************/

extern int period_bound_qsort_cmp(const void *a1, const void *a2);
extern int float8_qsort_cmp(const void *a1, const void *a2);

extern Datum period_analyze(PG_FUNCTION_ARGS);
extern Datum timestampset_analyze(PG_FUNCTION_ARGS);
extern Datum periodset_analyze(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
