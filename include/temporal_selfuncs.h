/*****************************************************************************
 *
 * temporal_selfuncs.h
 *   Selectivity functions for the temporal types
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORAL_SELFUNCS_H__
#define __TEMPORAL_SELFUNCS_H__

#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_operator.h>
#include <commands/vacuum.h>
#include <utils/lsyscache.h>
#include <utils/rangetypes.h>
#include <utils/selfuncs.h>
#include <utils/typcache.h>

#include "temporal.h"
#include "oidcache.h"

#define BTREE_AM_OID   403

#define DEFAULT_TEMP_SELECTIVITY 0.001

/*****************************************************************************
 * Internal selectivity functions for Temporal types.
 *****************************************************************************/

extern Selectivity scalarineqsel(PlannerInfo *root, Oid operator,
  bool isgt, bool iseq, VariableStatData *vardata, Datum constval,
  Oid consttype);
extern Selectivity temporal_sel_internal(PlannerInfo *root, VariableStatData *vardata,
  Period *period, CachedOp cachedOp);


/*****************************************************************************
 * Some other helper functions.
 *****************************************************************************/

#if MOBDB_PGSQL_VERSION < 120000
extern double var_eq_const(VariableStatData *vardata, Oid operator,
  Datum constval, bool constisnull, bool varonleft, bool negate);
#endif

/*****************************************************************************/

extern Datum temporal_sel(PG_FUNCTION_ARGS);
extern Datum temporal_joinsel(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
