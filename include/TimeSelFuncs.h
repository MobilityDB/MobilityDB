/*****************************************************************************
 *
 * TimeSelFuncs.h
 *	  Functions for selectivity estimation of time types operators
 *
 * These functions are based on those of the file rangetypes_selfuncs.c.
 * Estimates are based on histograms of lower and upper bounds.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	include/TimeSelFuncs.h
 *****************************************************************************/

#ifndef MOBILITYDB_TIMESELFUNCS_H
#define MOBILITYDB_TIMESELFUNCS_H

#include "TemporalTypes.h"

extern double default_period_selectivity(Oid operator);
extern int period_rbound_bsearch(PeriodBound *value, PeriodBound *hist, int hist_length, bool equal);
extern float8 get_period_position(PeriodBound *value, PeriodBound *hist1, PeriodBound *hist2);
extern float8 get_len_position(double value, double hist1, double hist2);
extern float8 get_period_distance(PeriodBound *bound1, PeriodBound *bound2);
extern int length_hist_bsearch(Datum *length_hist_values, int length_hist_nvalues, double value, bool equal);
extern double calc_period_hist_selectivity(VariableStatData *vardata,
                                           Period *constval, Oid operator, StatisticsStrategy strategy);
extern double calc_period_hist_selectivity_scalar(PeriodBound *constbound,
                                                  PeriodBound *hist, int hist_nvalues, bool equal);
extern double calc_length_hist_frac(Datum *length_hist_values,
                                    int length_hist_nvalues, double length1, double length2, bool equal);
extern double calc_period_hist_selectivity_contained(PeriodBound *lower,
                                                     PeriodBound *upper,	PeriodBound *hist_lower, int hist_nvalues,
                                                     Datum *length_hist_values, int length_hist_nvalues);
extern double calc_period_hist_selectivity_contains(PeriodBound *lower,
                                                    PeriodBound *upper,	PeriodBound *hist_lower, int hist_nvalues,
                                                    Datum *length_hist_values, int length_hist_nvalues);
extern double calc_period_hist_selectivity_adjacent(PeriodBound *lower, PeriodBound *upper,
                                                    PeriodBound *hist_lower, PeriodBound *hist_upper, int hist_nvalues);
extern bool get_attstatsslot_internal(AttStatsSlot *sslot, HeapTuple statstuple,
                                      int reqkind, Oid reqop, int flags, StatisticsStrategy strategy);

#endif //MOBILITYDB_TIMESELFUNCS_H
