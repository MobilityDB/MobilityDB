/*****************************************************************************
 *
 * TemporalSelFuncs.h
 * 		Selectivity functions for the temporal types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Anas Al Bassit
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORAL_SELFUNCS_H__
#define __TEMPORAL_SELFUNCS_H__

#include "TemporalTypes.h"

typedef enum {
 	VALUE_STATISTICS,
 	TEMPORAL_STATISTICS,
 	DEFAULT_STATISTICS
} StatisticsStrategy;

typedef enum {
 	SNCONST, /* Single Numeric Constant */
 	DNCONST, /* Double Numeric Constant */
 	STCONST, /* Single Temporal Constant */
 	DTCONST, /* Double Temporal Constant */
 	SNCONST_STCONST, /* Single Numeric Constant and Single Temporal Constant*/
 	SNCONST_DTCONST, /* Single Numeric Constant and Double Temporal Constant*/
 	DNCONST_STCONST, /* Double Numeric Constant and Single Temporal Constant*/
 	DNCONST_DTCONST, /* Double Numeric Constant and Double Temporal Constant*/
} BBoxBounds;

/* Temporal Unit Instant */
typedef struct
{
 	BBoxBounds bBoxBounds;
 	double lower, upper;
 	Period *period;
	Oid oid;
} ConstantData;

/*****************************************************************************
 * Internal selectivity functions for the operators.
 *****************************************************************************/

extern double bbox_overlaps_sel_internal(PlannerInfo *root, VariableStatData vardata, ConstantData constantData);
extern double bbox_contains_sel_internal(PlannerInfo *root, VariableStatData vardata, ConstantData constantData);
extern double bbox_contained_sel_internal(PlannerInfo *root, VariableStatData vardata, ConstantData constantData);
extern double bbox_same_sel_internal(PlannerInfo *root, VariableStatData vardata, ConstantData constantData);

/*****************************************************************************
 * Helper functions for calculating selectivity of period type.
 *****************************************************************************/
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

/*****************************************************************************
 * Some other helper functions.
 *****************************************************************************/
extern bool get_attstatsslot_internal(AttStatsSlot *sslot, HeapTuple statstuple,
									  int reqkind, Oid reqop, int flags, StatisticsStrategy strategy);
extern void get_const_bounds(Node *other, BBoxBounds *bBoxBounds, bool *numeric,
							 double *lower, double *upper, bool *temporal, Period **period);
extern double ineq_histogram_selectivity(PlannerInfo *root, VariableStatData *vardata,
										 FmgrInfo *opproc, bool isgt, bool iseq, Datum constval, Oid consttype,
										 StatisticsStrategy strategy);

#define STATISTIC_KIND_BOUNDS_HISTOGRAM_FIRST_DIM  500;
#define STATISTIC_KIND_BOUNDS_HISTOGRAM_SECOND_DIM 501;
#define STATISTIC_KIND_BOUNDS_HISTOGRAM_THIRD_DIM  502;
#endif