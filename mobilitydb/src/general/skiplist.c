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
 * @file
 * @brief Functions manipulating skiplists.
 */

#include "general/skiplist.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <executor/spi.h>
#include <libpq/pqformat.h>
#include <utils/memutils.h>
#include <utils/timestamp.h>
/* GSL */
#include <gsl/gsl_rng.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/span.h"
#include "general/temporal_aggfuncs.h"
/* MobilityDB */
#include "pg_general/span.h"
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"

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

/**
 * @brief Write the state value into the buffer
 * @param[in] state State
 * @param[in] buf Buffer
 */
static void
aggstate_write(SkipList *state, StringInfo buf)
{
  int i;
  void **values = skiplist_values(state);
  pq_sendint32(buf, (uint32) state->length);
  for (i = 0; i < state->length; i ++)
  {
    SPI_connect();
    temporal_write((Temporal *) values[i], buf);
    SPI_finish();
  }
  pq_sendint64(buf, state->extrasize);
  if (state->extra)
    pq_sendbytes(buf, state->extra, (int) state->extrasize);
  pfree(values);
  return;
}

/**
 * @brief Read the state value from the buffer
 * @param[in] buf Buffer
 */
static SkipList *
aggstate_read(StringInfo buf)
{
  int length = pq_getmsgint(buf, 4);
  void **values = palloc0(sizeof(void *) * length);
  SkipList *result = NULL; /* make compiler quiet */
  for (int i = 0; i < length; i ++)
    values[i] = temporal_recv(buf);
  size_t extrasize = (size_t) pq_getmsgint64(buf);
  result = skiplist_make(values, length);
  if (extrasize)
  {
    const char *extra = pq_getmsgbytes(buf, (int) extrasize);
    aggstate_set_extra(result, (void *) extra, extrasize);
  }
  pfree_array(values, length);
  return result;
}

Datum Tagg_serialize(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tagg_serialize);
/**
 * @brief Serialize the state value
 */
Datum
Tagg_serialize(PG_FUNCTION_ARGS)
{
  SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  aggstate_write(state, &buf);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

Datum Tagg_deserialize(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tagg_deserialize);
/**
 * @brief Deserialize the state value
 */
Datum
Tagg_deserialize(PG_FUNCTION_ARGS)
{
  bytea *data = PG_GETARG_BYTEA_P(0);
  StringInfoData buf =
  {
    .cursor = 0,
    .data = VARDATA(data),
    .len = VARSIZE(data),
    .maxlen = VARSIZE(data)
  };
  SkipList *result = aggstate_read(&buf);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
