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
 * @brief PG V1 wrappers for th3index lat/lng conversions
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <funcapi.h>
/* MEOS */
#include <meos.h>
#include <meos_h3.h>
#include "temporal/temporal.h"
#include "temporal/tcellindex.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"

/*****************************************************************************
 * Temporal point to temporal H3 cell index
 *****************************************************************************/

PGDLLEXPORT Datum Tgeompoint_to_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeompoint_to_th3index);
/**
 * @ingroup mobilitydb_h3_latlng
 * @brief Return the temporal H3 cell index of a temporal planar point at
 * the given resolution (SRID must be 4326)
 * @sqlfn th3index()
 */
Datum
Tgeompoint_to_th3index(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 resolution = PG_GETARG_INT32(1);
  Temporal *result = tgeompoint_to_th3index(temp, resolution);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tgeogpoint_to_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeogpoint_to_th3index);
/**
 * @ingroup mobilitydb_h3_latlng
 * @brief Return the temporal H3 cell index of a temporal geodetic point at
 * the given resolution
 * @sqlfn th3index()
 */
Datum
Tgeogpoint_to_th3index(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 resolution = PG_GETARG_INT32(1);
  Temporal *result = tgeogpoint_to_th3index(temp, resolution);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Split
 *****************************************************************************/

/**
 * @brief State of an H3 split, holding the cell and the fragment of every row
 * it has left to return
 */
typedef struct
{
  bool done;          /**< True when every row is returned */
  int i;              /**< Index of the next row */
  int count;          /**< Number of rows */
  Datum *cells;       /**< Cell of each fragment */
  Temporal **frags;   /**< Fragment of each cell */
} H3indexSplitState;

/**
 * @brief Return the fragments of a temporal point split by the H3 cells it
 * crosses, and the cell of each
 */
static Datum
Tpoint_h3index_split_ext(FunctionCallInfo fcinfo,
  Temporal **(*split)(const Temporal *, int32, Datum **, int *))
{
  FuncCallContext *funcctx;

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Get input parameters */
    Temporal *temp = PG_GETARG_TEMPORAL_P(0);
    int32 resolution = PG_GETARG_INT32(1);
    /* Create function state */
    H3indexSplitState *state = palloc0(sizeof(H3indexSplitState));
    state->frags = split(temp, resolution, &state->cells, &state->count);
    state->done = (state->count == 0);
    funcctx->user_fctx = state;
    /* Build a tuple description for the function output */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  H3indexSplitState *state = funcctx->user_fctx;
  /* Stop when every row is returned */
  if (state->done)
  {
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    if (state->frags)
      pfree(state->frags);
    if (state->cells)
      pfree(state->cells);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }

  /* Get the cell and its fragment */
  Datum values[2]; /* used to construct the composite return value */
  values[0] = state->cells[state->i];
  values[1] = PointerGetDatum(state->frags[state->i]);
  /* Advance state */
  if (++state->i == state->count)
    state->done = true;
  /* Form tuple and return */
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  Datum result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

PGDLLEXPORT Datum Tgeompoint_h3index_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeompoint_h3index_split);
/**
 * @ingroup mobilitydb_h3_latlng
 * @brief Return the fragments of a temporal planar point split by the H3
 * cells it crosses at the given resolution, and the cell of each
 * @sqlfn h3Split()
 */
Datum
Tgeompoint_h3index_split(PG_FUNCTION_ARGS)
{
  return Tpoint_h3index_split_ext(fcinfo, &tgeompoint_h3index_split);
}

PGDLLEXPORT Datum Tgeogpoint_h3index_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeogpoint_h3index_split);
/**
 * @ingroup mobilitydb_h3_latlng
 * @brief Return the fragments of a temporal geodetic point split by the H3
 * cells it crosses at the given resolution, and the cell of each
 * @sqlfn h3Split()
 */
Datum
Tgeogpoint_h3index_split(PG_FUNCTION_ARGS)
{
  return Tpoint_h3index_split_ext(fcinfo, &tgeogpoint_h3index_split);
}

/*****************************************************************************
 * cellToPoint
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_cell_to_tgeogpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_cell_to_tgeogpoint);
/**
 * @ingroup mobilitydb_h3_latlng
 * @brief Return the geodetic centroid trajectory of a temporal H3 cell
 * @sqlfn cellToPoint()
 */
Datum
Th3index_cell_to_tgeogpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_to_tgeogpoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Th3index_cell_to_tgeompoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_cell_to_tgeompoint);
/**
 * @ingroup mobilitydb_h3_latlng
 * @brief Return the planar centroid trajectory (SRID 4326) of a temporal
 * H3 cell
 * @sqlfn tgeompoint()
 */
Datum
Th3index_cell_to_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_to_tgeompoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * cellToBoundary
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_cell_to_boundary(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_cell_to_boundary);
/**
 * @ingroup mobilitydb_h3_latlng
 * @brief Return the per-instant polygon boundary of a temporal H3 cell as
 * a temporal geography
 * @sqlfn cellToBoundary()
 */
Datum
Th3index_cell_to_boundary(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcellindex_cell_to_boundary(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
