/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Selectivity functions for temporal types
 */

#ifndef __TEMPORAL_SELFUNCS_H__
#define __TEMPORAL_SELFUNCS_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_operator.h>
#include <commands/vacuum.h>
#include <utils/lsyscache.h>
#include <utils/selfuncs.h>
#include <utils/typcache.h>
/* MEOS */
#include <meos.h>
#include "general/meos_catalog.h"
#include "general/temporal.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"

#define BTREE_AM_OID   403

/**
* Default temporal selectivity factor
*/
#define DEFAULT_TEMP_SEL 0.0001
#define DEFAULT_TEMP_JOINSEL 0.001

/*****************************************************************************
 * Internal selectivity functions for Temporal types.
 *****************************************************************************/

extern Selectivity scalarineqsel(PlannerInfo *root, Oid operid, bool isgt,
  bool iseq, VariableStatData *vardata, Datum constval, Oid consttypid);
extern Selectivity temporal_sel_period(VariableStatData *vardata, Span *period,
  meosOper oper);

/*****************************************************************************
 * Some other helper functions.
 *****************************************************************************/

extern float8 temporal_sel(PlannerInfo *root, Oid operid, List *args,
  int varRelid, TemporalFamily tempfamily);
extern double temporal_sel_ext(FunctionCallInfo fcinfo,
  TemporalFamily tempfamily);

extern double temporal_joinsel(PlannerInfo *root, Oid operid,
  List *args, JoinType jointype, SpecialJoinInfo *sjinfo,
  TemporalFamily tempfamily);

extern double temporal_joinsel_ext(FunctionCallInfo fcinfo,
  TemporalFamily tempfamily);

/*****************************************************************************/

#endif
