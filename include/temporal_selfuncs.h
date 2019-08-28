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

typedef enum 
{
	VALUE_STATISTICS,
	TEMPORAL_STATISTICS,
	DEFAULT_STATISTICS
} StatStrategy;

#define BTREE_AM_OID   403

#define DEFAULT_SELECTIVITY 0.001

/*****************************************************************************
 * Internal selectivity functions for Temporal types.
 *****************************************************************************/

extern Selectivity estimate_temporal_bbox_sel(PlannerInfo *root, VariableStatData vardata, Period box,
											  CachedOp cachedOp);
extern Selectivity estimate_temporal_position_sel(PlannerInfo *root, VariableStatData vardata,
												  Node *other, bool isgt, bool iseq, CachedOp operator);

extern Selectivity scalarineqsel_mobdb(PlannerInfo *root, Oid operator, bool isgt, bool iseq,
								  VariableStatData *vardata, Datum constval, Oid consttype,
								  StatStrategy strategy);

/*****************************************************************************
 * Some other helper functions.
 *****************************************************************************/

extern bool get_attstatsslot_mobdb(AttStatsSlot *sslot, HeapTuple statstuple,
							 int reqkind, Oid reqop, int flags, StatStrategy strategy);
extern double default_temporal_selectivity(Oid operator);
extern double var_eq_const_mobdb(VariableStatData *vardata, Oid operator, Datum constval,
						   bool negate, StatStrategy strategy);

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