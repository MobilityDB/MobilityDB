/*****************************************************************************
 *
 * TemporalSelFuncs.h
 * 		Selectivity functions for the temporal types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	include/TemporalSelFuncs.h
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
 * Internal selectivity functions for Temporal types.
 *****************************************************************************/
extern Selectivity temporal_bbox_sel(PlannerInfo *root, Oid operator, List *args, int varRelid, CachedOp cachedOp);
extern Selectivity estimate_temporal_bbox_sel(PlannerInfo *root, VariableStatData vardata, ConstantData constantData,
								 CachedOp cachedOp);
extern Selectivity estimate_temporal_position_sel(PlannerInfo *root, VariableStatData vardata,
												  Node *other, bool isgt, bool iseq, CachedOp operator);
extern Selectivity period_sel_internal(PlannerInfo *root, VariableStatData *vardata, Period *constval,
									   Oid operator, StatisticsStrategy strategy);
extern Selectivity scalarineq_sel(PlannerInfo *root, Oid operator, bool isgt, bool iseq,
								  VariableStatData *vardata, Datum constval, Oid consttype,
								  StatisticsStrategy strategy);
extern Selectivity mcv_selectivity_internal(VariableStatData *vardata, FmgrInfo *opproc,
											Datum constval, Oid atttype, bool varonleft, double *sumcommonp,
											StatisticsStrategy strategy);
extern double ineq_histogram_selectivity(PlannerInfo *root, VariableStatData *vardata,
										 FmgrInfo *opproc, bool isgt, bool iseq, Datum constval, Oid consttype,
										 StatisticsStrategy strategy);
extern CachedOp get_temporal_cacheOp(Oid operator);
/*****************************************************************************
 * Internal selectivity functions for Tnumber types.
 *****************************************************************************/
extern Selectivity tnumber_bbox_sel(PlannerInfo *root, Oid operator, List *args, int varRelid, CachedOp cachedOp);
extern Selectivity estimate_tnumber_bbox_sel(PlannerInfo *root, VariableStatData vardata, ConstantData constantData,
											 CachedOp cachedOp);
extern Selectivity estimate_tnumber_position_sel(PlannerInfo *root, VariableStatData vardata,
												 Node *other, bool isgt, bool iseq, CachedOp operator);
extern Selectivity range_sel_internal(PlannerInfo *root, VariableStatData *vardata, Datum constval,
                                      bool isgt, bool iseq, TypeCacheEntry *typcache, StatisticsStrategy strategy);
extern Selectivity calc_range_hist_selectivity(VariableStatData *vardata, Datum constval,
											   TypeCacheEntry *typcache, bool isgt, bool iseq,
											   StatisticsStrategy strategy);
extern Selectivity calc_hist_selectivity_scalar(TypeCacheEntry *typcache, Datum constbound,
												RangeBound *hist, int hist_nvalues, bool equal);
extern int rbound_bsearch(TypeCacheEntry *typcache, Datum value, RangeBound *hist,
						  int hist_length, bool equal);
extern CachedOp get_tnumber_cacheOp(Oid operator);
/*****************************************************************************
 * Helper functions for calculating selectivity of time types.
 *****************************************************************************/

extern double default_period_selectivity(Oid operator);
extern int period_rbound_bsearch(PeriodBound *value, PeriodBound *hist,
                                 int hist_length, bool equal);
extern float8 get_period_position(PeriodBound *value, PeriodBound *hist1,
                                  PeriodBound *hist2);
extern float8 get_len_position(double value, double hist1, double hist2);
extern float8 get_period_distance(PeriodBound *bound1, PeriodBound *bound2);
extern int length_hist_bsearch(Datum *length_hist_values,
                               int length_hist_nvalues, double value, bool equal);
extern double calc_period_hist_selectivity(VariableStatData *vardata,
                                           Period *constval, Oid operator, StatisticsStrategy strategy);
extern double calc_period_hist_selectivity_scalar(PeriodBound *constbound,
                                                  PeriodBound *hist, int hist_nvalues, bool equal);
extern double calc_length_hist_frac(Datum *length_hist_values,
                                    int length_hist_nvalues, double length1, double length2, bool equal);
extern double calc_period_hist_selectivity_contained(PeriodBound *lower,
                                                     PeriodBound *upper, PeriodBound *hist_lower, int hist_nvalues,
                                                     Datum *length_hist_values, int length_hist_nvalues);
extern double calc_period_hist_selectivity_contains(PeriodBound *lower,
                                                    PeriodBound *upper,	PeriodBound *hist_lower, int hist_nvalues,
                                                    Datum *length_hist_values, int length_hist_nvalues);
extern double calc_period_hist_selectivity_adjacent(PeriodBound *lower,
                                                    PeriodBound *upper, PeriodBound *hist_lower, PeriodBound *hist_upper,
                                                    int hist_nvalues);
/*****************************************************************************
 * Helper functions for calculating selectivity.
 *****************************************************************************/
extern double lower_or_higher_value_bound(Node *other, bool higher);
extern PeriodBound *lower_or_higher_temporal_bound(Node *other, bool higher);
extern bool get_attstatsslot_internal(AttStatsSlot *sslot, HeapTuple statstuple,
									  int reqkind, Oid reqop, int flags, StatisticsStrategy strategy);
extern double default_temporaltypes_selectivity(Oid operator);
extern void get_const_bounds(Node *other, BBoxBounds *bBoxBounds, bool *numeric,
							 double *lower, double *upper, bool *temporal, Period **period);
extern double var_eq_const(VariableStatData *vardata, Oid operator, Datum constval,
						   bool negate, StatisticsStrategy strategy);
extern bool convert_to_scalar(Oid valuetypid, Datum value, double *scaledvalue,
							  Datum lobound, Datum hibound, Oid boundstypid, double *scaledlobound,
							  double *scaledhibound);
extern double convert_numeric_to_scalar(Oid typid, Datum value);
extern double convert_timevalue_to_scalar(Oid typid, Datum value);
extern bool get_actual_variable_range(PlannerInfo *root, VariableStatData *vardata,
									  Oid sortop, Datum *min, Datum *max);
#define BTREE_AM_OID   403
#endif