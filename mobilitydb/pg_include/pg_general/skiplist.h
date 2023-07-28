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
 * @brief Skiplist data structure used for performing aggregates
 */

#ifndef __PG_SKIPLIST_H__
#define __PG_SKIPLIST_H__

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <utils/palloc.h>
/* MEOS */
#include "general/temporal.h"
/* MobilityDB */
#include "pg_general/temporal.h"

/*****************************************************************************/

extern FunctionCallInfo fetch_fcinfo(void);
extern void store_fcinfo(FunctionCallInfo fcinfo);
extern MemoryContext set_aggregation_context(FunctionCallInfo fcinfo);
extern void unset_aggregation_context(MemoryContext ctx);

/*****************************************************************************/

/**
 * Helper macros to input the current aggregate state
 */
#define INPUT_AGG_TRANS_STATE(fcinfo, state)  \
  do {  \
    MemoryContext ctx = set_aggregation_context(fcinfo); \
    state = PG_ARGISNULL(0) ? NULL : (SkipList *) PG_GETARG_POINTER(0);  \
    if (PG_ARGISNULL(1))  \
    {  \
      if (state)  \
        PG_RETURN_POINTER(state);  \
      else  \
        PG_RETURN_NULL();  \
    }  \
    unset_aggregation_context(ctx); \
  } while (0)

#define INPUT_AGG_COMB_STATE(fcinfo, state1, state2)  \
  do {  \
    MemoryContext ctx = set_aggregation_context(fcinfo); \
    state1 = PG_ARGISNULL(0) ? NULL : (SkipList *) PG_GETARG_POINTER(0);  \
    state2 = PG_ARGISNULL(1) ? NULL : (SkipList *) PG_GETARG_POINTER(1);  \
    if (state1 == NULL && state2 == NULL)  \
      PG_RETURN_NULL();  \
    unset_aggregation_context(ctx); \
  } while (0)

/*****************************************************************************/

#endif
