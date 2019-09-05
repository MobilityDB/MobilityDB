/*****************************************************************************
 *
 * time_selfuncs.c
 *	  Functions for selectivity estimation of time types operators
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TIME_SELFUNCS_H__
#define __TIME_SELFUNCS_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/selfuncs.h>
#include "temporal_selfuncs.h"
#include "timetypes.h"

/*****************************************************************************/

extern double calc_period_hist_selectivity(VariableStatData *vardata,
	Period *constval, CachedOp cachedOp);
extern double calc_period_hist_selectivity_scalar(PeriodBound *constbound,
	PeriodBound *hist, int hist_nvalues, bool equal);
extern double calc_period_hist_selectivity_contained(PeriodBound *lower,
	PeriodBound *upper, PeriodBound *hist_lower, int hist_nvalues,
	Datum *length_hist_values, int length_hist_nvalues);
extern double calc_period_hist_selectivity_contains(PeriodBound *lower,
	PeriodBound *upper,	PeriodBound *hist_lower, int hist_nvalues,
	Datum *length_hist_values, int length_hist_nvalues);
extern double calc_period_hist_selectivity_adjacent(PeriodBound *lower,
	PeriodBound *upper, PeriodBound *hist_lower,
	PeriodBound *hist_upper, int hist_nvalues);

extern int length_hist_bsearch(Datum *length_hist_values,
	int length_hist_nvalues, double value, bool equal);
extern double get_len_position(double value, double hist1, double hist2);
extern double calc_length_hist_frac(Datum *length_hist_values, int length_hist_nvalues,
	double length1, double length2, bool equal);

extern Datum period_analyze(PG_FUNCTION_ARGS);
extern Datum timestampset_analyze(PG_FUNCTION_ARGS);
extern Datum periodset_analyze(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
