/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written 
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO 
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

/**
 * @file temporal_selfuncs.h
 * Selectivity functions for temporal types
 */

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
#include "tempcache.h"

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

#if POSTGRESQL_VERSION_NUMBER < 120000
extern double var_eq_const(VariableStatData *vardata, Oid operator,
  Datum constval, bool constisnull, bool varonleft, bool negate);
#endif

/*****************************************************************************/

extern Datum temporal_sel(PG_FUNCTION_ARGS);
extern Datum temporal_joinsel(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
