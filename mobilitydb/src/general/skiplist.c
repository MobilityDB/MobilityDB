/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/span.h"
#include "general/temporal_aggfuncs.h"
/* MobilityDB */
#include "pg_general/span.h"
#include "pg_general/temporal.h"
#include "pg_general/temporal_util.h"

/*****************************************************************************
 * Functions manipulating skip lists
 *****************************************************************************/

/**
 * Switch to the memory context for aggregation
 */
MemoryContext
set_aggregation_context(FunctionCallInfo fcinfo)
{
  MemoryContext ctx;
  if (! AggCheckCallContext(fcinfo, &ctx))
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Cannot switch to aggregation context")));
  return MemoryContextSwitchTo(ctx);
}

/**
 * Switch to the given memory context
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
 * Reads the state value from the buffer
 *
 * @param[in] state State
 * @param[in] data Structure containing the data
 * @param[in] size Size of the structure
 */
void
aggstate_set_extra(SkipList *state, void *data, size_t size)
{
#if ! MEOS
  MemoryContext ctx;
  assert(AggCheckCallContext(fetch_fcinfo(), &ctx));
  MemoryContext oldctx = MemoryContextSwitchTo(ctx);
#endif /* ! MEOS */
  state->extra = palloc(size);
  state->extrasize = size;
  memcpy(state->extra, data, size);
#if ! MEOS
  MemoryContextSwitchTo(oldctx);
#endif /* ! MEOS */
}

/**
 * Writes the state value into the buffer
 *
 * @param[in] state State
 * @param[in] buf Buffer
 */
static void
aggstate_write(SkipList *state, StringInfo buf)
{
  int i;
  void **values = skiplist_values(state);
  pq_sendint32(buf, (uint32) state->elemtype);
  pq_sendint32(buf, (uint32) state->length);
  if (state->elemtype == TIMESTAMPTZ)
  {
    for (i = 0; i < state->length; i ++)
    {
      bytea *time = call_send(T_TIMESTAMPTZ,
        TimestampTzGetDatum((TimestampTz) values[i]));
      pq_sendbytes(buf, VARDATA(time), VARSIZE(time) - VARHDRSZ);
      pfree(time);
    }
  }
  else if (state->elemtype == PERIOD)
  {
    for (i = 0; i < state->length; i ++)
      span_write((const Span *) values[i], buf);
  }
  else /* state->elemtype == TEMPORAL */
  {
    for (i = 0; i < state->length; i ++)
    {
      SPI_connect();
      temporal_write((Temporal *) values[i], buf);
      SPI_finish();
    }
    pq_sendint64(buf, state->extrasize);
    if (state->extra)
      pq_sendbytes(buf, state->extra, (int) state->extrasize);
  }
  pfree(values);
  return;
}

/**
 * Reads the state value from the buffer
 *
 * @param[in] buf Buffer
 */
static SkipList *
aggstate_read(StringInfo buf)
{
  SkipListElemType elemtype = (SkipListElemType) pq_getmsgint(buf, 4);
  int length = pq_getmsgint(buf, 4);
  void **values = palloc0(sizeof(void *) * length);
  SkipList *result = NULL; /* make compiler quiet */
  if (elemtype == TIMESTAMPTZ)
  {
    for (int i = 0; i < length; i ++)
      values[i] = (void **) DatumGetTimestampTz(call_recv(T_TIMESTAMPTZ, buf));
    result = skiplist_make(values, length, TIMESTAMPTZ);
    pfree(values);
  }
  else if (elemtype == PERIOD)
  {
    for (int i = 0; i < length; i ++)
      values[i] = span_recv(buf);
    result = skiplist_make(values, length, PERIOD);
    pfree_array(values, length);
  }
  else /* elemtype == TEMPORAL */
  {
    for (int i = 0; i < length; i ++)
      values[i] = temporal_recv(buf);
    size_t extrasize = (size_t) pq_getmsgint64(buf);
    result = skiplist_make(values, length, TEMPORAL);
    if (extrasize)
    {
      const char *extra = pq_getmsgbytes(buf, (int) extrasize);
      aggstate_set_extra(result, (void *) extra, extrasize);
    }
    pfree_array(values, length);
  }
  return result;
}

PG_FUNCTION_INFO_V1(Tagg_serialize);
/**
 * Serialize the state value
 */
PGDLLEXPORT Datum
Tagg_serialize(PG_FUNCTION_ARGS)
{
  SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  aggstate_write(state, &buf);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(Tagg_deserialize);
/**
 * Deserialize the state value
 */
PGDLLEXPORT Datum
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
