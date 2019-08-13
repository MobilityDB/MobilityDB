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

#ifndef __TEMPORALSELFUNCS_H__
#define __TEMPORALSELFUNCS_H__

#include <postgres.h>
#include <catalog/pg_operator.h>
#include <commands/vacuum.h>
#include <utils/selfuncs.h>
#include <utils/typcache.h>
#include <utils/lsyscache.h>
#include <utils/timestamp.h>
#include <access/htup_details.h>
#include <catalog/pg_collation_d.h>
#include <Period.h>
#include <Range.h>
#include "Temporal.h"
#include "OidCache.h"

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
extern Selectivity estimate_tnumber_position_sel(VariableStatData vardata, Node *other, bool isgt, bool iseq);
extern Selectivity range_sel_internal(VariableStatData *vardata, Datum constval,
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