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
#include "general/set.h"

PG_FUNCTION_INFO_V1(Set_gin_extract_value);
/*
 * extractValue support function
 */
Datum
Set_gin_extract_value(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  bool **nullFlags = (bool **) PG_GETARG_POINTER(2);
  Datum *elems = orderedset_values(os);
  *nkeys = os->count;
  *nullFlags = NULL;
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_POINTER(elems);
}

PG_FUNCTION_INFO_V1(Set_gin_extract_query);
/*
 * extractQuery support function
 */
Datum
Set_gin_extract_query(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  StrategyNumber strategy = PG_GETARG_UINT16(2);
  bool **nullFlags = (bool **) PG_GETARG_POINTER(5);
  int32 *searchMode = (int32 *) PG_GETARG_POINTER(6);
  Datum *elems = orderedset_values(os);
  *nkeys = os->count;
  *nullFlags = NULL;

  switch (strategy)
  {
    case RTOverlapStrategyNumber:
    case RTContainsStrategyNumber:
    case RTContainedByStrategyNumber:
    case RTSameStrategyNumber:
      *searchMode = GIN_SEARCH_MODE_DEFAULT;
      break;
    default:
      elog(ERROR, "Set_gin_extract_query: unknown strategy number: %d",
         strategy);
  }

  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_POINTER(elems);
}

PG_FUNCTION_INFO_V1(Set_gin_consistent);
/*
 * consistent support function
 */
Datum
Set_gin_consistent(PG_FUNCTION_ARGS)
{
  bool *check = (bool *) PG_GETARG_POINTER(0);
  StrategyNumber strategy = PG_GETARG_UINT16(1);
  int32 nkeys = PG_GETARG_INT32(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(5);
  bool *nullFlags = (bool *) PG_GETARG_POINTER(7);
  bool res;
  int32 i;

  switch (strategy)
  {
    case RTOverlapStrategyNumber:
      /* result is not lossy */
      *recheck = false;
      /* must have a match for at least one non-null element */
      res = false;
      for (i = 0; i < nkeys; i++)
      {
        if (check[i] && !nullFlags[i])
        {
          res = true;
          break;
        }
      }
      break;
    case RTContainsStrategyNumber:
      /* result is not lossy */
      *recheck = false;
      /* must have all elements in check[] true, and no nulls */
      res = true;
      for (i = 0; i < nkeys; i++)
      {
        if (!check[i] || nullFlags[i])
        {
          res = false;
          break;
        }
      }
      break;
    case RTContainedByStrategyNumber:
      /* we will need recheck */
      *recheck = true;
      /* can't do anything else useful here */
      res = true;
      break;
    case RTSameStrategyNumber:
      /* we will need recheck */
      *recheck = true;

      /*
       * Must have all elements in check[] true; no discrimination
       * against nulls here.  This is because array_contain_compare and
       * array_eq handle nulls differently ...
       */
      res = true;
      for (i = 0; i < nkeys; i++)
      {
        if (! check[i])
        {
          res = false;
          break;
        }
      }
      break;
    default:
      elog(ERROR, "Set_gin_consistent: unknown strategy number: %d",
         strategy);
      res = false;
  }

  PG_RETURN_BOOL(res);
}

PG_FUNCTION_INFO_V1(Set_gin_triconsistent);
/*
 * triconsistent support function
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

  switch (strategy)
  {
    case RTOverlapStrategyNumber:
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
    case RTContainsStrategyNumber:
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
    case RTContainedByStrategyNumber:
      /* can't do anything else useful here */
      res = GIN_MAYBE;
      break;
    case RTSameStrategyNumber:

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
