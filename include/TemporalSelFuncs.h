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
 * Internal selectivity functions for the operators.
 *****************************************************************************/
extern Selectivity bbox_sel(PlannerInfo *root, Oid operator, List *args, int varRelid, CachedOp cachedOp);
extern Selectivity calc_bbox_sel(PlannerInfo *root, VariableStatData vardata, ConstantData constantData,
								 CachedOp cachedOp);
extern Selectivity period_sel_internal(PlannerInfo *root, VariableStatData *vardata, Period *constval,
									   Oid operator, StatisticsStrategy strategy);
extern Selectivity scalarineq_sel(PlannerInfo *root, Oid operator, bool isgt, bool iseq,
								  VariableStatData *vardata, Datum constval, Oid consttype,
								  StatisticsStrategy strategy);
extern Selectivity mcv_selectivity_internal(VariableStatData *vardata, FmgrInfo *opproc,
											Datum constval, Oid atttype, bool varonleft, double *sumcommonp,
											StatisticsStrategy strategy);
extern double bbox_overlaps_sel_internal(PlannerInfo *root, VariableStatData vardata, ConstantData constantData);
extern double bbox_contains_sel_internal(PlannerInfo *root, VariableStatData vardata, ConstantData constantData);
extern double bbox_contained_sel_internal(PlannerInfo *root, VariableStatData vardata, ConstantData constantData);
extern double bbox_same_sel_internal(PlannerInfo *root, VariableStatData vardata, ConstantData constantData);
/*****************************************************************************
 * Helper functions for calculating selectivity.
 *****************************************************************************/
extern double default_temporaltypes_selectivity(Oid operator);
extern void get_const_bounds(Node *other, BBoxBounds *bBoxBounds, bool *numeric,
							 double *lower, double *upper, bool *temporal, Period **period);
extern double ineq_histogram_selectivity(PlannerInfo *root, VariableStatData *vardata,
										 FmgrInfo *opproc, bool isgt, bool iseq, Datum constval, Oid consttype,
										 StatisticsStrategy strategy);
extern double var_eq_const(VariableStatData *vardata, Oid operator, Datum constval,
						   bool negate, StatisticsStrategy strategy);

#define STATISTIC_KIND_BOUNDS_HISTOGRAM_FIRST_DIM  500;
#define STATISTIC_KIND_BOUNDS_HISTOGRAM_SECOND_DIM 501;
#define STATISTIC_KIND_BOUNDS_HISTOGRAM_THIRD_DIM  502;
#endif