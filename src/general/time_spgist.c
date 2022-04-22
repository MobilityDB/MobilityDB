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

/**
 * @file time_spgist.c
 * @brief Quad-tree SP-GiST index for time types.
 *
 * The functions in this file are based on those in the file
 * `rangetypes_spgist.c`.
 */

#include "general/time_spgist.h"

/* PostgreSQL */
#include <assert.h>
#include <access/spgist.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/timetypes.h"
#include "general/time_ops.h"
#include "general/timestampset.h"
#include "general/period.h"
#include "general/periodset.h"
#include "general/time_gist.h"
#include "general/temporal_util.h"
#include "general/tempcache.h"

/*****************************************************************************
 * Data structures
 *****************************************************************************/

/**
 * Structure to represent the bounding box of an inner node containing a set
 * of periods
 */
typedef struct
{
  Period left;
  Period right;
} PeriodNode;

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * Initialize the traversal value
 *
 * In the beginning, we don't have any restrictions.  We have to
 * initialize the struct to cover the whole 2D space.
 */
static void
periodnode_init(PeriodNode *nodebox)
{
  memset(nodebox, 0, sizeof(PeriodNode));
  nodebox->left.lower = nodebox->left.upper = DT_NOBEGIN;
  nodebox->right.lower = nodebox->right.upper = DT_NOEND;
  return;
}

/**
 * Copy a traversal value
 */
PeriodNode *
periodnode_copy(const PeriodNode *orig)
{
  PeriodNode *result = palloc(sizeof(PeriodNode));
  memcpy(result, orig, sizeof(PeriodNode));
  return result;
}

/**
 * Compute the next traversal value for a quadtree given the bounding box
 * and the centroid of the current node and the quadrant number (0 to 3).
 *
 * For example, given the bounding box of the root node (level 0) and
 * the centroid as follows
 *     nodebox = (-infinity, -infinity)(infinity, infinity)
 *     centroid = (2001-06-13 18:10:00+02, 2001-06-13 18:11:00+02)
 * the quadrants are as follows
 *     0 = (-infinity, 2001-06-13 18:10:00)(infinity, 2001-06-13 18:11:00)
 *     1 = (-infinity, 2001-06-13 18:10:00)(2001-06-13 18:11:00, infinity)
 *     2 = (2001-06-13 18:10:00, -infinity)(infinity, 2001-06-13 18:11:00)
 *     3 = (2001-06-13 18:10:00, -infinity)(2001-06-13 18:11:00, infinity)
 */
static void
periodnode_quadtree_next(const PeriodNode *nodebox, const Period *centroid,
  uint8 quadrant, PeriodNode *next_nodeperiod)
{
  memcpy(next_nodeperiod, nodebox, sizeof(PeriodNode));
  if (quadrant & 0x2)
  {
    next_nodeperiod->left.lower = centroid->lower;
    next_nodeperiod->left.lower_inc = centroid->lower_inc;
  }
  else
  {
    next_nodeperiod->left.upper = centroid->lower;
    next_nodeperiod->left.upper_inc = centroid->lower_inc;
  }
  if (quadrant & 0x1)
  {
    next_nodeperiod->right.lower = centroid->upper;
    next_nodeperiod->right.lower_inc = centroid->upper_inc;
  }
  else
  {
    next_nodeperiod->right.upper = centroid->upper;
    next_nodeperiod->right.upper_inc = centroid->upper_inc;
  }
  return;
}

/**
 * Calculate the quadrant
 *
 * The quadrant is 8 bit unsigned integer with 2 least bits in use.
 * This function accepts Periods as input. The 2 bits are set by comparing
 * a corner of the box. This makes 4 quadrants in total.
 */
static uint8
get_quadrant2D(const Period *centroid, const Period *query)
{
  uint8 quadrant = 0;
  if (period_lower_cmp(query, centroid) > 0)
    quadrant |= 0x2;
  if (period_upper_cmp(query, centroid) > 0)
    quadrant |= 0x1;
  return quadrant;
}

/**
 * Can any period from nodebox overlap with this argument?
 */
static bool
overlap2D(const PeriodNode *nodebox, const Period *query)
{
  Period p;
  period_set(nodebox->left.lower, nodebox->right.upper,
    nodebox->left.lower_inc, nodebox->right.upper_inc, &p);
  return overlaps_period_period(&p, query);
}

/**
 * Can any period from nodebox contain the query?
 */
static bool
contain2D(const PeriodNode *nodebox, const Period *query)
{
  Period p;
  period_set(nodebox->left.lower, nodebox->right.upper,
    nodebox->left.lower_inc, nodebox->right.upper_inc, &p);
  return contains_period_period(&p, query);
}

/**
 * Can any period from nodebox be before the query?
 */
static bool
before2D(const PeriodNode *nodebox, const Period *query)
{
  return before_period_period(&nodebox->right, query);
}

/**
 * Can any period from nodebox does not extend after the query?
 */
static bool
overBefore2D(const PeriodNode *nodebox, const Period *query)
{
  return overbefore_period_period(&nodebox->right, query);
}

/**
 * Can any period from nodebox be after the query?
 */
static bool
after2D(const PeriodNode *nodebox, const Period *query)
{
  return after_period_period(&nodebox->left, query);
}

/**
 * Can any period from nodebox does not extend before the query?
 */
static bool
overAfter2D(const PeriodNode *nodebox, const Period *query)
{
  return overafter_period_period(&nodebox->left, query);
}

#if POSTGRESQL_VERSION_NUMBER >= 120000
/**
 * Distance between a query period and a box of periods
 */
static double
distance_period_nodeperiod(Period *query, PeriodNode *nodebox)
{
  /* If the the period intersects the nodebox return 0 */
  Period p;
  period_set(nodebox->left.lower, nodebox->right.upper,
    nodebox->left.lower_inc, nodebox->right.upper_inc, &p);
  if (overlaps_period_period(query, &p))
    return 0;

  /* If the query is to the left of the nodebox return the distance between
   * the upper bound of the query and lower bound of the nodebox */
  if (nodebox->left.lower >= query->upper)
    return nodebox->left.lower - query->upper;

  /* If the query is to the right of the nodebox return the distance between
   * the upper bound of the nodebox and lower bound of the query */
  return query->lower - nodebox->right.upper;
}
#endif /* POSTGRESQL_VERSION_NUMBER >= 120000 */

/**
 * Transform a query argument into a period.
 */
static bool
time_spgist_get_period(const ScanKeyData *scankey, Period *result)
{
  CachedType type = oid_type(scankey->sk_subtype);
  if (type == T_TIMESTAMPTZ)
  {
    TimestampTz t = DatumGetTimestampTz(scankey->sk_argument);
    period_set(t, t, true, true, result);
  }
  else if (type == T_TIMESTAMPSET)
  {
    timestampset_bbox_slice(scankey->sk_argument, result);
  }
  else if (type == T_PERIOD)
  {
    Period *p = DatumGetPeriodP(scankey->sk_argument);
    memcpy(result, p, sizeof(Period));
  }
  else if (type == T_PERIODSET)
  {
    periodset_bbox_slice(scankey->sk_argument, result);
  }
  /* For temporal types whose bounding box is a period */
  else if (temporal_type(type))
  {
    temporal_bbox_slice(scankey->sk_argument, result);
  }
  else
    elog(ERROR, "Unsupported type for indexing: %d", type);
  return true;
}

/*****************************************************************************
 * SP-GiST config function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_spgist_config);
/**
 * SP-GiST config function for time types
 */
PGDLLEXPORT Datum
Period_spgist_config(PG_FUNCTION_ARGS)
{
  spgConfigOut *cfg = (spgConfigOut *) PG_GETARG_POINTER(1);
  cfg->prefixType = type_oid(T_PERIOD);  /* A type represented by its bounding box */
  cfg->labelType = VOIDOID;  /* We don't need node labels. */
  cfg->leafType = type_oid(T_PERIOD);
  cfg->canReturnData = false;
  cfg->longValuesOK = false;
  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST choose functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_quadtree_choose);
/**
 * SP-GiST choose function for time types
 */
PGDLLEXPORT Datum
Period_quadtree_choose(PG_FUNCTION_ARGS)
{
  spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
  spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
  Period *centroid = DatumGetPeriodP(in->prefixDatum),
    *period = DatumGetPeriodP(in->leafDatum);

  out->resultType = spgMatchNode;
  out->result.matchNode.restDatum = PointerGetDatum(period);

  /* nodeN will be set by core, when allTheSame. */
  if (!in->allTheSame)
    out->result.matchNode.nodeN = get_quadrant2D(centroid, period);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST pick-split function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_quadtree_picksplit);
/**
 * SP-GiST pick-split function for time types
 *
 * It splits a list of time types into quadrants by choosing a central 4D
 * point as the median of the coordinates of the time types.
 */
PGDLLEXPORT Datum
Period_quadtree_picksplit(PG_FUNCTION_ARGS)
{
  spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
  spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
  Period *centroid;
  int median, i;
  /* Use the median values of lower and upper bounds as the centroid period */
  PeriodBound *lowerBounds = palloc(sizeof(PeriodBound) * in->nTuples);
  PeriodBound *upperBounds = palloc(sizeof(PeriodBound) * in->nTuples);

  /* Construct "centroid" period from medians of lower and upper bounds */
  for (i = 0; i < in->nTuples; i++)
    period_deserialize(DatumGetPeriodP(in->datums[i]),
      &lowerBounds[i], &upperBounds[i]);

  qsort(lowerBounds, (size_t) in->nTuples, sizeof(PeriodBound), period_bound_qsort_cmp);
  qsort(upperBounds, (size_t) in->nTuples, sizeof(PeriodBound), period_bound_qsort_cmp);

  median = in->nTuples / 2;

  centroid = period_make(lowerBounds[median].t, upperBounds[median].t,
    lowerBounds[median].inclusive, upperBounds[median].inclusive);

  /* Fill the output */
  out->hasPrefix = true;
  out->prefixDatum = PeriodPGetDatum(centroid);
  out->nNodes = 4;
  out->nodeLabels = NULL;    /* we don't need node labels */
  out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
  out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);

  /*
   * Assign periods to corresponding nodes according to quadrants relative to
   * "centroid" period.
   */
  for (i = 0; i < in->nTuples; i++)
  {
    Period *period = DatumGetPeriodP(in->datums[i]);
    int16 quadrant = get_quadrant2D(centroid, period);
    out->leafTupleDatums[i] = PeriodPGetDatum(period);
    out->mapTuplesToNodes[i] = quadrant;
  }

  pfree(lowerBounds); pfree(upperBounds);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST inner consistent functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_quadtree_inner_consistent);
/**
 * SP-GiST inner consistent function function for time types
 */
PGDLLEXPORT Datum
Period_quadtree_inner_consistent(PG_FUNCTION_ARGS)
{
  spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
  spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
  int i;
  uint8 node;
  MemoryContext old_ctx;
  PeriodNode *nodebox, infbox, next_nodeperiod;
#if POSTGRESQL_VERSION_NUMBER >= 120000
  Period *centroid, *queries, *orderbys;
#else
  Period *centroid, *queries;
#endif

  /* Fetch the centroid of this node. */
  assert(in->hasPrefix);
  centroid = DatumGetPeriodP(in->prefixDatum);

  /*
   * We are saving the traversal value or initialize it an unbounded one, if
   * we have just begun to walk the tree.
   */
  if (in->traversalValue)
    nodebox = in->traversalValue;
  else
  {
    periodnode_init(&infbox);
    nodebox = &infbox;
  }

#if POSTGRESQL_VERSION_NUMBER >= 120000
  /*
   * Transform the orderbys into bounding boxes initializing the dimensions
   * that must not be taken into account for the operators to infinity.
   * This transformation is done here to avoid doing it for all quadrants
   * in the loop below.
   */
  if (in->norderbys > 0)
  {
    orderbys = palloc0(sizeof(Period) * in->norderbys);
    for (i = 0; i < in->norderbys; i++)
      time_spgist_get_period(&in->orderbys[i], &orderbys[i]);
  }
#endif

  if (in->allTheSame)
  {
    /* Report that all nodes should be visited */
    out->nNodes = in->nNodes;
    out->nodeNumbers = (int *) palloc(sizeof(int) * in->nNodes);
    for (i = 0; i < in->nNodes; i++)
    {
      out->nodeNumbers[i] = i;

#if POSTGRESQL_VERSION_NUMBER >= 120000
      if (in->norderbys > 0)
      {
        /* Use parent quadrant nodebox as traversalValue */
        old_ctx = MemoryContextSwitchTo(in->traversalMemoryContext);
        out->traversalValues[i] = periodnode_copy(nodebox);
        MemoryContextSwitchTo(old_ctx);

        /* Compute the distances */
        double *distances = palloc(sizeof(double) * in->norderbys);
        out->distances[i] = distances;
        for (int j = 0; j < in->norderbys; j++)
          distances[j] = distance_period_nodeperiod(&orderbys[j], nodebox);

        pfree(orderbys);
      }
#endif /* POSTGRESQL_VERSION_NUMBER >= 120000 */
    }

    PG_RETURN_VOID();
  }

  /* Transform the queries into periods */
  if (in->nkeys > 0)
  {
    queries = (Period *) palloc0(sizeof(Period) * in->nkeys);
    for (i = 0; i < in->nkeys; i++)
      time_spgist_get_period(&in->scankeys[i], &queries[i]);
  }

  /* Allocate enough memory for nodes */
  out->nNodes = 0;
  out->nodeNumbers = (int *) palloc(sizeof(int) * in->nNodes);
  out->traversalValues = (void **) palloc(sizeof(void *) * in->nNodes);
#if POSTGRESQL_VERSION_NUMBER >= 120000
  if (in->norderbys > 0)
    out->distances = (double **) palloc(sizeof(double *) * in->nNodes);
#endif /* POSTGRESQL_VERSION_NUMBER >= 120000 */

  /* Loop for each child */
  for (node = 0; node < in->nNodes; node++)
  {
    /* Compute the bounding box of the child node */
    periodnode_quadtree_next(nodebox, centroid, node, &next_nodeperiod);
    bool flag = true;
    for (i = 0; i < in->nkeys; i++)
    {
      StrategyNumber strategy = in->scankeys[i].sk_strategy;
      switch (strategy)
      {
        case RTOverlapStrategyNumber:
        case RTContainedByStrategyNumber:
        case RTAdjacentStrategyNumber:
          flag = overlap2D(&next_nodeperiod, &queries[i]);
          break;
        case RTContainsStrategyNumber:
        case RTSameStrategyNumber:
          flag = contain2D(&next_nodeperiod, &queries[i]);
          break;
        case RTBeforeStrategyNumber:
          flag = !overAfter2D(&next_nodeperiod, &queries[i]);
          break;
        case RTOverBeforeStrategyNumber:
          flag = !after2D(&next_nodeperiod, &queries[i]);
          break;
        case RTAfterStrategyNumber:
          flag = !overBefore2D(&next_nodeperiod, &queries[i]);
          break;
        case RTOverAfterStrategyNumber:
          flag = !before2D(&next_nodeperiod, &queries[i]);
          break;
        default:
          elog(ERROR, "unrecognized strategy: %d", strategy);
      }
      /* If any check is failed, we have found our answer. */
      if (! flag)
        break;
    }

    if (flag)
    {
      /* Pass traversalValue and node */
      old_ctx = MemoryContextSwitchTo(in->traversalMemoryContext);
      out->traversalValues[out->nNodes] = periodnode_copy(&next_nodeperiod);
      MemoryContextSwitchTo(old_ctx);
      out->nodeNumbers[out->nNodes] = node;
#if POSTGRESQL_VERSION_NUMBER >= 120000
      /* Pass distances */
      if (in->norderbys > 0)
      {
        double *distances = palloc(sizeof(double) * in->norderbys);
        out->distances[out->nNodes] = distances;
        for (i = 0; i < in->norderbys; i++)
          distances[i] = distance_period_nodeperiod(&orderbys[i], &next_nodeperiod);
      }
#endif /* POSTGRESQL_VERSION_NUMBER >= 120000 */
      out->nNodes++;
    }
  } /* Loop for every child */

  if (in->nkeys > 0)
    pfree(queries);
#if POSTGRESQL_VERSION_NUMBER >= 120000
  if (in->norderbys > 0)
    pfree(orderbys);
#endif /* POSTGRESQL_VERSION_NUMBER >= 120000 */

  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST leaf-level consistency function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_spgist_leaf_consistent);
/**
 * SP-GiST leaf-level consistency function for time types
 */
PGDLLEXPORT Datum
Period_spgist_leaf_consistent(PG_FUNCTION_ARGS)
{
  spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
  spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
  Period *key = DatumGetPeriodP(in->leafDatum), period;
  bool result = true;
  int i;

  /**
   * Initialization so that all the tests are lossy.
   * This will be changed below for some tests.
   */
  out->recheck = true;

  /* leafDatum is what it is... */
  out->leafValue = in->leafDatum;

  /* Perform the required comparison(s) */
  for (i = 0; i < in->nkeys; i++)
  {
    StrategyNumber strategy = in->scankeys[i].sk_strategy;

    /* Update the recheck flag according to the strategy */
    out->recheck |= period_index_recheck(strategy);

    /* Cast the query to a period and perform the test */
    time_spgist_get_period(&in->scankeys[i], &period);
    result = period_index_consistent_leaf(key, &period, strategy);
    /* All tests are lossy for temporal types */
    if (temporal_type(in->scankeys[i].sk_subtype))
      out->recheck = true;

    /* If any check is failed, we have found our answer. */
    if (! result)
      break;
  }

#if POSTGRESQL_VERSION_NUMBER >= 120000
  if (result && in->norderbys > 0)
  {
    /* Recheck is necessary when computing distance with bounding boxes */
    out->recheckDistances = true;
    double *distances = palloc(sizeof(double) * in->norderbys);
    out->distances = distances;
    for (i = 0; i < in->norderbys; i++)
    {
      /* Cast the order by argument to a period and perform the test */
      time_spgist_get_period(&in->orderbys[i], &period);
      distances[i] = distance_secs_period_period(&period, key);
    }
    /* Recheck is necessary when computing distance with bounding boxes */
    out->recheckDistances = true;
  }
#endif /* POSTGRESQL_VERSION_NUMBER >= 120000 */

  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * SP-GiST compress functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_spgist_compress);
/**
 * SP-GiST compress function for timestamp sets
 */
PGDLLEXPORT Datum
Timestampset_spgist_compress(PG_FUNCTION_ARGS)
{
  Datum tsdatum = PG_GETARG_DATUM(0);
  Period *result = (Period *) palloc(sizeof(Period));
  timestampset_bbox_slice(tsdatum, result);
  PG_RETURN_PERIOD_P(result);
}

PG_FUNCTION_INFO_V1(Periodset_spgist_compress);
/**
 * SP-GiST compress function for period sets
 */
PGDLLEXPORT Datum
Periodset_spgist_compress(PG_FUNCTION_ARGS)
{
  Datum psdatum = PG_GETARG_DATUM(0);
  Period *result = (Period *) palloc(sizeof(Period));
  periodset_bbox_slice(psdatum, result);
  PG_RETURN_PERIOD_P(result);
}

/*****************************************************************************/
