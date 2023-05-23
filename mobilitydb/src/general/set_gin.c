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
 * @brief GIN index functions for set types.
 */

/* PostgreSQL */
#include "postgres.h"
#include "access/gin.h"
#include "access/stratnum.h"
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
/* MobilityDB */
#include "pg_general/temporal.h"

/*****************************************************************************
 * Operator strategy numbers used in the GIN set and tnpoint opclasses
 *****************************************************************************/

#define GinOverlapStrategySetSet             10    /* for && */
#define GinContainsStrategySetValue          20    /* for @> */
#define GinContainsStrategySetSet            21    /* for @> */
#define GinContainedStrategySetSet           30    /* for <@ */
#define GinEqualStrategySetSet               40    /* for =  */

/*****************************************************************************/

PGDLLEXPORT Datum Set_gin_extract_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_gin_extract_value);
/**
 * @brief extractValue support function
 */
Datum
Set_gin_extract_value(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  bool **nullFlags = (bool **) PG_GETARG_POINTER(2);
  Datum *elems = set_values(s);
  *nkeys = s->count;
  *nullFlags = NULL;
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_POINTER(elems);
}

PGDLLEXPORT Datum Set_gin_extract_query(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_gin_extract_query);
/**
 * @brief extractQuery support function
 */
Datum
Set_gin_extract_query(PG_FUNCTION_ARGS)
{
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  StrategyNumber strategy = PG_GETARG_UINT16(2);
  bool **nullFlags = (bool **) PG_GETARG_POINTER(5);
  int32 *searchMode = (int32 *) PG_GETARG_POINTER(6);
  Set *s;
  Datum *elems = NULL;
  *nullFlags = NULL;
  *searchMode = GIN_SEARCH_MODE_DEFAULT;

  switch (strategy)
  {
    case GinContainsStrategySetValue:
      elems = palloc(sizeof(Datum));
      elems[0] = PG_GETARG_DATUM(0);
      *nkeys = 1;
      break;
    case GinOverlapStrategySetSet:
    case GinContainsStrategySetSet:
    case GinContainedStrategySetSet:
    case GinEqualStrategySetSet:
      s = PG_GETARG_SET_P(0);
      elems = set_values(s);
      *nkeys = s->count;
      PG_FREE_IF_COPY(s, 0);
      break;
    default:
      elog(ERROR, "Set_gin_extract_query: unknown strategy number: %d",
         strategy);
  }
  PG_RETURN_POINTER(elems);
}

PGDLLEXPORT Datum Set_gin_triconsistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_gin_triconsistent);
/**
 * @brief triconsistent support function
 */
Datum
Set_gin_triconsistent(PG_FUNCTION_ARGS)
{
  GinTernaryValue *check = (GinTernaryValue *) PG_GETARG_POINTER(0);
  StrategyNumber strategy = PG_GETARG_UINT16(1);
  int32 nkeys = PG_GETARG_INT32(3);
  bool *nullFlags = (bool *) PG_GETARG_POINTER(6);
  GinTernaryValue res;
  int32 i;
  /* Use the generic strategy numbers independent of the argument types */
  StrategyNumber ginstrategy = strategy / 10;

  switch (ginstrategy)
  {
    case GinOverlapStrategy:
      /* must have a match for at least one non-null element */
      res = GIN_FALSE;
      for (i = 0; i < nkeys; i++)
      {
        if (!nullFlags[i])
        {
          if (check[i] == GIN_TRUE)
          {
            res = GIN_TRUE;
            break;
          }
          else if (check[i] == GIN_MAYBE && res == GIN_FALSE)
          {
            res = GIN_MAYBE;
          }
        }
      }
      break;
    case GinContainsStrategy:
      /* must have all elements in check[] true, and no nulls */
      res = GIN_TRUE;
      for (i = 0; i < nkeys; i++)
      {
        if (check[i] == GIN_FALSE || nullFlags[i])
        {
          res = GIN_FALSE;
          break;
        }
        if (check[i] == GIN_MAYBE)
        {
          res = GIN_MAYBE;
        }
      }
      break;
    case GinContainedStrategy:
      /* can't do anything else useful here */
      res = GIN_MAYBE;
      break;
    case GinEqualStrategy:

      /*
       * Must have all elements in check[] true; no discrimination
       * against nulls here.  This is because array_contain_compare and
       * array_eq handle nulls differently ...
       */
      res = GIN_MAYBE;
      for (i = 0; i < nkeys; i++)
      {
        if (check[i] == GIN_FALSE)
        {
          res = GIN_FALSE;
          break;
        }
      }
      break;
    default:
      elog(ERROR, "Set_gin_consistent: unknown strategy number: %d",
         strategy);
      res = false;
  }

  PG_RETURN_GIN_TERNARY_VALUE(res);
}
