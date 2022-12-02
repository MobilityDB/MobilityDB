/*-------------------------------------------------------------------------
 *
 * ttext_gin_proc.c
 *    support functions for GIN's indexing of any array
 *
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *    src/backend/access/gin/ttext_gin_proc.c
 *-------------------------------------------------------------------------
 */

/* PostgreSQL */
#include "postgres.h"
#include "access/gin.h"
#include "access/stratnum.h"
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h"
/* MobilityDB */

#define GinOverlapStrategy     1
#define GinContainsStrategy    2
#define GinContainedStrategy   3
#define GinEqualStrategy       4

PG_FUNCTION_INFO_V1(Ttext_gin_extract_value);
/*
 * extractValue support function
 */
Datum
Ttext_gin_extract_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  bool **nullFlags = (bool **) PG_GETARG_POINTER(2);
  int count;
  Datum *elems = temporal_values(temp, &count);
  *nkeys = count;
  *nullFlags = NULL;
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(elems);
}

PG_FUNCTION_INFO_V1(Ttext_gin_extract_query);
/*
 * extractQuery support function
 */
Datum
Ttext_gin_extract_query(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 *nkeys = (int32 *) PG_GETARG_POINTER(1);
  StrategyNumber strategy = PG_GETARG_UINT16(2);
  bool **nullFlags = (bool **) PG_GETARG_POINTER(5);
  int32 *searchMode = (int32 *) PG_GETARG_POINTER(6);
  int count;
  Datum *elems = temporal_values(temp, &count);
  *nkeys = count;
  *nullFlags = NULL;

  switch (strategy)
  {
    case GinOverlapStrategy:
    case GinContainsStrategy:
    case GinContainedStrategy:
    case GinEqualStrategy:
      *searchMode = GIN_SEARCH_MODE_DEFAULT;
      break;
    default:
      elog(ERROR, "Ttext_gin_extract_query: unknown strategy number: %d",
         strategy);
  }

  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(elems);
}

PG_FUNCTION_INFO_V1(Ttext_gin_consistent);
/*
 * consistent support function
 */
Datum
Ttext_gin_consistent(PG_FUNCTION_ARGS)
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
    case GinOverlapStrategy:
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
    case GinContainsStrategy:
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
    case GinContainedStrategy:
      /* we will need recheck */
      *recheck = true;
      /* can't do anything else useful here */
      res = true;
      break;
    case GinEqualStrategy:
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
        if (!check[i])
        {
          res = false;
          break;
        }
      }
      break;
    default:
      elog(ERROR, "ttext_gin_consistent: unknown strategy number: %d",
         strategy);
      res = false;
  }

  PG_RETURN_BOOL(res);
}

PG_FUNCTION_INFO_V1(Ttext_gin_triconsistent);
/*
 * triconsistent support function
 */
Datum
Ttext_gin_triconsistent(PG_FUNCTION_ARGS)
{
  GinTernaryValue *check = (GinTernaryValue *) PG_GETARG_POINTER(0);
  StrategyNumber strategy = PG_GETARG_UINT16(1);
  int32 nkeys = PG_GETARG_INT32(3);
  bool *nullFlags = (bool *) PG_GETARG_POINTER(6);
  GinTernaryValue res;
  int32 i;

  switch (strategy)
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
      elog(ERROR, "Ttext_gin_consistent: unknown strategy number: %d",
         strategy);
      res = false;
  }

  PG_RETURN_GIN_TERNARY_VALUE(res);
}
