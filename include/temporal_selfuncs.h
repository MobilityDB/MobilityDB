/*****************************************************************************
 *
 * temporal_selfuncs.h
 * 	Selectivity functions for the temporal types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORAL_SELFUNCS_H__
#define __TEMPORAL_SELFUNCS_H__

#include <postgres.h>
#include <catalog/pg_operator.h>
#include <commands/vacuum.h>
#include <utils/lsyscache.h>
#include <utils/rangetypes.h>
#include <utils/selfuncs.h>
#include <utils/typcache.h>

#include "temporal.h"
#include "oidcache.h"

typedef enum {
	VALUE_STATISTICS,
	TEMPORAL_STATISTICS,
	DEFAULT_STATISTICS
} StatisticsStrategy;

typedef enum 
{
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

#define BTREE_AM_OID   403

/*****************************************************************************
 * Internal selectivity functions for Temporal types.
 *****************************************************************************/

extern Selectivity estimate_temporal_bbox_sel(PlannerInfo *root, VariableStatData vardata, ConstantData constantData,
											  CachedOp cachedOp);
extern Selectivity estimate_temporal_position_sel(PlannerInfo *root, VariableStatData vardata,
												  Node *other, bool isgt, bool iseq, CachedOp operator);

extern Selectivity period_sel_internal(PlannerInfo *root, VariableStatData *vardata, Period *constval,
									   Oid operator, StatisticsStrategy strategy);
extern Selectivity scalarineq_sel(PlannerInfo *root, Oid operator, bool isgt, bool iseq,
								  VariableStatData *vardata, Datum constval, Oid consttype,
								  StatisticsStrategy strategy);

/*****************************************************************************
 * Some other helper functions.
 *****************************************************************************/

extern bool get_attstatsslot_internal(AttStatsSlot *sslot, HeapTuple statstuple,
									  int reqkind, Oid reqop, int flags, StatisticsStrategy strategy);
extern double default_temporaltypes_selectivity(Oid operator);
extern void get_const_bounds(Node *other, BBoxBounds *bBoxBounds, bool *numeric,
							 double *lower, double *upper, bool *temporal, Period **period);
extern double var_eq_const(VariableStatData *vardata, Oid operator, Datum constval,
						   bool negate, StatisticsStrategy strategy);

/*****************************************************************************/

extern Datum temporal_overlaps_sel(PG_FUNCTION_ARGS);
extern Datum temporal_overlaps_joinsel(PG_FUNCTION_ARGS);
extern Datum temporal_contains_sel(PG_FUNCTION_ARGS);
extern Datum temporal_contains_joinsel(PG_FUNCTION_ARGS);
extern Datum temporal_same_sel(PG_FUNCTION_ARGS);
extern Datum temporal_same_joinsel(PG_FUNCTION_ARGS);
extern Datum temporal_position_sel(PG_FUNCTION_ARGS);
extern Datum temporal_position_joinsel(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif