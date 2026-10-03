/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @file
 * @brief Functions for manipulating skiplists
 */

#include "temporal/skiplist.h"

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/type_util.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"

/*****************************************************************************
 * Functions manipulating skip lists
 *****************************************************************************/

/**
 * @brief Switch to the memory context for aggregation
 */
MemoryContext
set_aggregation_context(FunctionCallInfo fcinfo)
{
  MemoryContext ctx = NULL;
  if (! AggCheckCallContext(fcinfo, &ctx))
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Cannot switch to aggregation context")));
  return MemoryContextSwitchTo(ctx);
}

/**
 * @brief Switch to the given memory context
 */
void
unset_aggregation_context(MemoryContext ctx)
{
  MemoryContextSwitchTo(ctx);
  return;
}

/*****************************************************************************
 * Generic binary aggregate functions needed for parallelization
 *****************************************************************************/

Datum Taggstate_serialize(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Taggstate_serialize);
/**
 * @brief Serialize the state value
 */
Datum
Taggstate_serialize(PG_FUNCTION_ARGS)
{
  SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
  /* #skiplist_values takes the aggregation context from #fetch_fcinfo */
  store_fcinfo(fcinfo);
  size_t size;
  uint8_t *bytes = taggstate_serialize(state, &size);
  bytea *result = palloc(VARHDRSZ + size);
  SET_VARSIZE(result, VARHDRSZ + size);
  memcpy(VARDATA(result), bytes, size);
  pfree(bytes);
  PG_RETURN_BYTEA_P(result);
}

Datum Taggstate_deserialize(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Taggstate_deserialize);
/**
 * @brief Deserialize the state value
 */
Datum
Taggstate_deserialize(PG_FUNCTION_ARGS)
{
  bytea *data = PG_GETARG_BYTEA_P(0);
  /* The skiplist this reads is built by #temporal_skiplist_make and
   * #temporal_skiplist_splice, which take the aggregation context from
   * #fetch_fcinfo. Every other entry point of the aggregate stores its own
   * fcinfo for them to find; without it they read the one a previous call
   * left and build the state in a context that is reset before the final
   * function runs. */
  store_fcinfo(fcinfo);
  SkipList *result = taggstate_deserialize((uint8_t *) VARDATA(data),
    VARSIZE(data) - VARHDRSZ);
  PG_RETURN_SKIPLIST_P(result);
}

/*****************************************************************************/
