/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
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

/* PostgreSQL */
#include "postgres.h"
#include "access/gin.h"
#include "access/stratnum.h"
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h"
#include "npoint/tnpoint.h"
/* MobilityDB */
#include "pg_general/temporal.h"

PG_FUNCTION_INFO_V1(Tnpoint_gin_extract_value);
/*
 * extractValue support function
 */
Datum
Tnpoint_gin_extract_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  bool **nullFlags = (bool **) PG_GETARG_POINTER(2);
  OrderedSet *routes = tnpoint_routes(temp);
  /* Transform the routes into Datums */
  Datum *elems = palloc(sizeof(Datum) * routes->count);
  for (int i = 0; i < routes->count; i++)
    elems[i] = Int64GetDatum(orderedset_val_n(routes, i));
  pfree(routes);
  *nkeys = routes->count;
  *nullFlags = NULL;
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(elems);
}

PG_FUNCTION_INFO_V1(Tnpoint_gin_extract_query);
/*
 * extractQuery support function
 */
Datum
Tnpoint_gin_extract_query(PG_FUNCTION_ARGS)
{
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  StrategyNumber strategy = PG_GETARG_UINT16(2);
  bool **nullFlags = (bool **) PG_GETARG_POINTER(5);
  int32 *searchMode = (int32 *) PG_GETARG_POINTER(6);
  Temporal *temp;
  Datum *elems;
  OrderedSet *routes;
  *nullFlags = NULL;
  *searchMode = GIN_SEARCH_MODE_DEFAULT;

  switch (strategy)
  {
    case GinContainsStrategyValue:
      elems = palloc(sizeof(Datum));
      elems[0] = PG_GETARG_DATUM(0);
      *nkeys = 1;
      break;
    case GinOverlapStrategy:
    case GinContainsStrategySet:
    case GinContainedStrategy:
    case GinEqualStrategy:
      temp = PG_GETARG_TEMPORAL_P(0);
      routes = tnpoint_routes(temp);
      /* Transform the routes into Datums */
      elems = palloc(sizeof(Datum) * routes->count);
      for (int i = 0; i < routes->count; i++)
        elems[i] = Int64GetDatum(orderedset_val_n(routes, i));
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
