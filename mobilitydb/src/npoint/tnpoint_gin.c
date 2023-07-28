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
 * @brief GIN index for the rid of temporal network points.
*/

/* PostgreSQL */
#include "postgres.h"
#include "access/gin.h"
#include "access/stratnum.h"
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/temporal.h"
#include "npoint/tnpoint.h"
/* MobilityDB */
#include "pg_general/temporal.h"

/*****************************************************************************
 * Operator strategy numbers used in the GIN set and tnpoint opclasses
 *****************************************************************************/

#define GinOverlapStrategyTnpointSet             10    /* for @@ */
#define GinOverlapStrategyTnpointTnpoint         11    /* for @@ */
#define GinContainsStrategyTnpointValue          20    /* for @? */
#define GinContainsStrategyTnpointSet            21    /* for @? */
#define GinContainsStrategyTnpointTnpoint        22    /* for @? */
#define GinContainedStrategyTnpointSet           30    /* for ?@ */
#define GinContainedStrategyTnpointTnpoint       31    /* for ?@ */
#define GinEqualStrategyTnpointSet               40    /* for @=*/
#define GinEqualStrategyTnpointTnpoint           41    /* for @=*/

/*****************************************************************************/

PGDLLEXPORT Datum Tnpoint_gin_extract_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_gin_extract_value);
/**
 * @brief extractValue support function
 */
Datum
Tnpoint_gin_extract_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  bool **nullFlags = (bool **) PG_GETARG_POINTER(2);
  Set *routes = tnpoint_routes(temp);
  /* Transform the routes into Datums */
  Datum *elems = palloc(sizeof(Datum) * routes->count);
  for (int i = 0; i < routes->count; i++)
    elems[i] = Int64GetDatum(SET_VAL_N(routes, i));
  pfree(routes);
  *nkeys = routes->count;
  *nullFlags = NULL;
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(elems);
}

PGDLLEXPORT Datum Tnpoint_gin_extract_query(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_gin_extract_query);
/**
 * @brief extractQuery support function
 */
Datum
Tnpoint_gin_extract_query(PG_FUNCTION_ARGS)
{
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  StrategyNumber strategy = PG_GETARG_UINT16(2);
  bool **nullFlags = (bool **) PG_GETARG_POINTER(5);
  int32 *searchMode = (int32 *) PG_GETARG_POINTER(6);
  Temporal *temp;
  Datum *elems = NULL; /* make compiler quiet */
  Set *s, *routes;
  *nullFlags = NULL;
  *searchMode = GIN_SEARCH_MODE_DEFAULT;

  switch (strategy)
  {
    case GinContainsStrategyTnpointValue:
      elems = palloc(sizeof(Datum));
      elems[0] = PG_GETARG_DATUM(0);
      *nkeys = 1;
      break;
    case GinOverlapStrategyTnpointSet:
    case GinContainsStrategyTnpointSet:
    case GinContainedStrategyTnpointSet:
    case GinEqualStrategyTnpointSet:
      s = PG_GETARG_SET_P(0);
      elems = set_values(s);
      *nkeys = s->count;
      *searchMode = GIN_SEARCH_MODE_DEFAULT;
      PG_FREE_IF_COPY(s, 0);
      break;
    case GinOverlapStrategyTnpointTnpoint:
    case GinContainsStrategyTnpointTnpoint:
    case GinContainedStrategyTnpointTnpoint:
    case GinEqualStrategyTnpointTnpoint:
      temp = PG_GETARG_TEMPORAL_P(0);
      routes = tnpoint_routes(temp);
      /* Transform the routes into Datums */
      elems = palloc(sizeof(Datum) * routes->count);
      for (int i = 0; i < routes->count; i++)
        elems[i] = Int64GetDatum(SET_VAL_N(routes, i));
      pfree(routes);
      *nkeys = routes->count;
      *searchMode = GIN_SEARCH_MODE_DEFAULT;
      PG_FREE_IF_COPY(temp, 0);
      break;
    default:
      elog(ERROR, "Tnpoint_gin_extract_query: unknown strategy number: %d",
         strategy);
  }

  PG_RETURN_POINTER(elems);
}

/*****************************************************************************/
