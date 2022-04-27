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
 * @file span_spgist.c
 * @brief Quad-tree SP-GiST index for span types.
 *
 * The functions in this file are based on those in the file
 * `rangetypes_spgist.c`.
 */

#include "general/span_spgist.h"

/* PostgreSQL */
#include <postgres.h>
#include <assert.h>
#include <access/spgist.h>
/* MobilityDB */
#include "general/span_ops.h"
#include "general/span_gist.h"
#include "general/temporal_util.h"
#include "general/tempcache.h"

/*****************************************************************************
 * Data structures
 *****************************************************************************/

/**
 * Structure to represent the bounding box of an inner node containing a set
 * of spans
 */
typedef struct
{
  Span left;
  Span right;
} SpanNode;

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
spannode_init(SpanNode *nodebox)
{
  memset(nodebox, 0, sizeof(SpanNode));
  nodebox->left.lower = nodebox->left.upper = DT_NOBEGIN;
  nodebox->right.lower = nodebox->right.upper = DT_NOEND;
  return;
}

/**
 * Copy a traversal value
 */
SpanNode *
spannode_copy(const SpanNode *orig)
{
  SpanNode *result = palloc(sizeof(SpanNode));
  memcpy(result, orig, sizeof(SpanNode));
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
spannode_quadtree_next(const SpanNode *nodebox, const Span *centroid,
  uint8 quadrant, SpanNode *next_nodespan)
{
  memcpy(next_nodespan, nodebox, sizeof(SpanNode));
  if (quadrant & 0x2)
  {
    next_nodespan->left.lower = centroid->lower;
    next_nodespan->left.lower_inc = centroid->lower_inc;
  }
  else
  {
    next_nodespan->left.upper = centroid->lower;
    next_nodespan->left.upper_inc = centroid->lower_inc;
  }
  if (quadrant & 0x1)
  {
    next_nodespan->right.lower = centroid->upper;
    next_nodespan->right.lower_inc = centroid->upper_inc;
  }
  else
  {
    next_nodespan->right.upper = centroid->upper;
    next_nodespan->right.upper_inc = centroid->upper_inc;
  }
  return;
}

/**
 * Calculate the quadrant
 *
 * The quadrant is 8 bit unsigned integer with 2 least bits in use.
 * This function accepts Spans as input. The 2 bits are set by comparing
 * a corner of the box. This makes 4 quadrants in total.
 */
static uint8
get_quadrant2D(const Span *centroid, const Span *query)
{
  uint8 quadrant = 0;
  if (span_lower_cmp(query, centroid) > 0)
    quadrant |= 0x2;
  if (span_upper_cmp(query, centroid) > 0)
    quadrant |= 0x1;
  return quadrant;
}

/**
 * Can any span from nodebox overlap with this argument?
 */
static bool
overlap2D(const SpanNode *nodebox, const Span *query)
{
  Span s;
  span_set(nodebox->left.lower, nodebox->right.upper, nodebox->left.lower_inc,
    nodebox->right.upper_inc, nodebox->left.spantype, &s);
  return overlaps_span_span(&s, query);
}

/**
 * Can any span from nodebox contain the query?
 */
static bool
contain2D(const SpanNode *nodebox, const Span *query)
{
  Span s;
  span_set(nodebox->left.lower, nodebox->right.upper, nodebox->left.lower_inc,
    nodebox->right.upper_inc, nodebox->left.spantype,&s);
  return contains_span_span(&s, query);
}

/**
 * Can any span from nodebox be left the query?
 */
static bool
left2D(const SpanNode *nodebox, const Span *query)
{
  return left_span_span(&nodebox->right, query);
}

/**
 * Can any span from nodebox does not extend right the query?
 */
static bool
overLeft2D(const SpanNode *nodebox, const Span *query)
{
  return overleft_span_span(&nodebox->right, query);
}

/**
 * Can any span from nodebox be right the query?
 */
static bool
right2D(const SpanNode *nodebox, const Span *query)
{
  return right_span_span(&nodebox->left, query);
}

/**
 * Can any span from nodebox does not extend left the query?
 */
static bool
overRight2D(const SpanNode *nodebox, const Span *query)
{
  return overright_span_span(&nodebox->left, query);
}

#if POSTGRESQL_VERSION_NUMBER >= 120000
/**
 * Distance between a query span and a box of spans
 */
static double
distance_span_nodespan(Span *query, SpanNode *nodebox)
{
  /* Determine the maximum span of the nodebox */
  Span s;
  span_set(nodebox->left.lower, nodebox->right.upper, nodebox->left.lower_inc,
    nodebox->right.upper_inc, nodebox->left.spantype, &s);

  /* Compute the distance between the query span and the nodebox span */
  return distance_span_span(query, &s);
}
#endif /* POSTGRESQL_VERSION_NUMBER >= 120000 */

/**
 * Transform a query argument into a span.
 */
static bool
span_spgist_get_span(const ScanKeyData *scankey, Span *result)
{
  CachedType type = oid_type(scankey->sk_subtype);
  if (type == T_INT4 || type == T_FLOAT8)
  {
    Datum d = scankey->sk_argument;
    span_set(d, d, true, true, type, result);
  }
  else if (type == T_INTSPAN || type == T_FLOATSPAN)
  {
    Span *s = DatumGetSpanP(scankey->sk_argument);
    memcpy(result, s, sizeof(Span));
  }
  /* For temporal types whose bounding box is a span */
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

PG_FUNCTION_INFO_V1(Intspan_spgist_config);
/**
 * SP-GiST config function for span types
 */
PGDLLEXPORT Datum
Intspan_spgist_config(PG_FUNCTION_ARGS)
{
  spgConfigOut *cfg = (spgConfigOut *) PG_GETARG_POINTER(1);
  cfg->prefixType = type_oid(T_INTSPAN);  /* A type represented by its bounding box */
  cfg->labelType = VOIDOID;  /* We don't need node labels. */
  cfg->leafType = type_oid(T_INTSPAN);
  cfg->canReturnData = false;
  cfg->longValuesOK = false;
  PG_RETURN_VOID();
}

PG_FUNCTION_INFO_V1(Floatspan_spgist_config);
/**
 * SP-GiST config function for span types
 */
PGDLLEXPORT Datum
Floatspan_spgist_config(PG_FUNCTION_ARGS)
{
  spgConfigOut *cfg = (spgConfigOut *) PG_GETARG_POINTER(1);
  cfg->prefixType = type_oid(T_FLOATSPAN);  /* A type represented by its bounding box */
  cfg->labelType = VOIDOID;  /* We don't need node labels. */
  cfg->leafType = type_oid(T_FLOATSPAN);
  cfg->canReturnData = false;
  cfg->longValuesOK = false;
  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST choose functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_quadtree_choose);
/**
 * SP-GiST choose function for span types
 */
PGDLLEXPORT Datum
Span_quadtree_choose(PG_FUNCTION_ARGS)
{
  spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
  spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
  Span *centroid = DatumGetSpanP(in->prefixDatum),
    *span = DatumGetSpanP(in->leafDatum);

  out->resultType = spgMatchNode;
  out->result.matchNode.restDatum = PointerGetDatum(span);

  /* nodeN will be set by core, when allTheSame. */
  if (!in->allTheSame)
    out->result.matchNode.nodeN = get_quadrant2D(centroid, span);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST pick-split function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_quadtree_picksplit);
/**
 * SP-GiST pick-split function for span types
 *
 * It splits a list of span types into quadrants by choosing a central 4D
 * point as the median of the coordinates of the span types.
 */
PGDLLEXPORT Datum
Span_quadtree_picksplit(PG_FUNCTION_ARGS)
{
  spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
  spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
  Span *centroid;
  int median, i;
  /* Use the median values of lower and upper bounds as the centroid span */
  SpanBound *lowerBounds = palloc(sizeof(SpanBound) * in->nTuples);
  SpanBound *upperBounds = palloc(sizeof(SpanBound) * in->nTuples);

  /* Construct "centroid" span from medians of lower and upper bounds */
  for (i = 0; i < in->nTuples; i++)
    span_deserialize(DatumGetSpanP(in->datums[i]), &lowerBounds[i],
      &upperBounds[i]);

  qsort(lowerBounds, (size_t) in->nTuples, sizeof(SpanBound),
    span_bound_qsort_cmp);
  qsort(upperBounds, (size_t) in->nTuples, sizeof(SpanBound),
    span_bound_qsort_cmp);

  median = in->nTuples / 2;

  centroid = span_make(lowerBounds[median].val, upperBounds[median].val,
    lowerBounds[median].inclusive, upperBounds[median].inclusive,
    lowerBounds[median].basetype);

  /* Fill the output */
  out->hasPrefix = true;
  out->prefixDatum = SpanPGetDatum(centroid);
  out->nNodes = 4;
  out->nodeLabels = NULL;    /* we don't need node labels */
  out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
  out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);

  /*
   * Assign spans to corresponding nodes according to quadrants relative to
   * "centroid" span.
   */
  for (i = 0; i < in->nTuples; i++)
  {
    Span *span = DatumGetSpanP(in->datums[i]);
    int16 quadrant = get_quadrant2D(centroid, span);
    out->leafTupleDatums[i] = SpanPGetDatum(span);
    out->mapTuplesToNodes[i] = quadrant;
  }

  pfree(lowerBounds); pfree(upperBounds);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST inner consistent functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_quadtree_inner_consistent);
/**
 * SP-GiST inner consistent function function for span types
 */
PGDLLEXPORT Datum
Span_quadtree_inner_consistent(PG_FUNCTION_ARGS)
{
  spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
  spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
  int i;
  uint8 node;
  MemoryContext old_ctx;
  SpanNode *nodebox, infbox, next_nodespan;
#if POSTGRESQL_VERSION_NUMBER >= 120000
  Span *centroid, *queries, *orderbys;
#else
  Span *centroid, *queries;
#endif

  /* Fetch the centroid of this node. */
  assert(in->hasPrefix);
  centroid = DatumGetSpanP(in->prefixDatum);

  /*
   * We are saving the traversal value or initialize it an unbounded one, if
   * we have just begun to walk the tree.
   */
  if (in->traversalValue)
    nodebox = in->traversalValue;
  else
  {
    spannode_init(&infbox);
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
    orderbys = palloc0(sizeof(Span) * in->norderbys);
    for (i = 0; i < in->norderbys; i++)
      span_spgist_get_span(&in->orderbys[i], &orderbys[i]);
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
        out->traversalValues[i] = spannode_copy(nodebox);
        MemoryContextSwitchTo(old_ctx);

        /* Compute the distances */
        double *distances = palloc(sizeof(double) * in->norderbys);
        out->distances[i] = distances;
        for (int j = 0; j < in->norderbys; j++)
          distances[j] = distance_span_nodespan(&orderbys[j], nodebox);

        pfree(orderbys);
      }
#endif /* POSTGRESQL_VERSION_NUMBER >= 120000 */
    }

    PG_RETURN_VOID();
  }

  /* Transform the queries into spans */
  if (in->nkeys > 0)
  {
    queries = (Span *) palloc0(sizeof(Span) * in->nkeys);
    for (i = 0; i < in->nkeys; i++)
      span_spgist_get_span(&in->scankeys[i], &queries[i]);
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
    spannode_quadtree_next(nodebox, centroid, node, &next_nodespan);
    bool flag = true;
    for (i = 0; i < in->nkeys; i++)
    {
      StrategyNumber strategy = in->scankeys[i].sk_strategy;
      switch (strategy)
      {
        case RTOverlapStrategyNumber:
        case RTContainedByStrategyNumber:
        case RTAdjacentStrategyNumber:
          flag = overlap2D(&next_nodespan, &queries[i]);
          break;
        case RTContainsStrategyNumber:
        case RTSameStrategyNumber:
          flag = contain2D(&next_nodespan, &queries[i]);
          break;
        case RTLeftStrategyNumber:
          flag = !overRight2D(&next_nodespan, &queries[i]);
          break;
        case RTOverLeftStrategyNumber:
          flag = !right2D(&next_nodespan, &queries[i]);
          break;
        case RTRightStrategyNumber:
          flag = !overLeft2D(&next_nodespan, &queries[i]);
          break;
        case RTOverRightStrategyNumber:
          flag = !left2D(&next_nodespan, &queries[i]);
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
      out->traversalValues[out->nNodes] = spannode_copy(&next_nodespan);
      MemoryContextSwitchTo(old_ctx);
      out->nodeNumbers[out->nNodes] = node;
#if POSTGRESQL_VERSION_NUMBER >= 120000
      /* Pass distances */
      if (in->norderbys > 0)
      {
        double *distances = palloc(sizeof(double) * in->norderbys);
        out->distances[out->nNodes] = distances;
        for (i = 0; i < in->norderbys; i++)
          distances[i] = distance_span_nodespan(&orderbys[i], &next_nodespan);
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

PG_FUNCTION_INFO_V1(Span_spgist_leaf_consistent);
/**
 * SP-GiST leaf-level consistency function for span types
 */
PGDLLEXPORT Datum
Span_spgist_leaf_consistent(PG_FUNCTION_ARGS)
{
  spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
  spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
  Span *key = DatumGetSpanP(in->leafDatum), span;
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
    out->recheck |= span_index_recheck(strategy);

    /* Cast the query to a span and perform the test */
    span_spgist_get_span(&in->scankeys[i], &span);
    result = span_index_consistent_leaf(key, &span, strategy);
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
      /* Cast the order by argument to a span and perform the test */
      span_spgist_get_span(&in->orderbys[i], &span);
      distances[i] = distance_span_span(&span, key);
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

/*****************************************************************************/
