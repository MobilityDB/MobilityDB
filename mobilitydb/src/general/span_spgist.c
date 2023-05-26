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
 * @brief Quad-tree SP-GiST index for span types.
 *
 * The functions in this file are based on those in the file
 * `rangetypes_spgist.c`.
 */

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include <access/spgist.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/spanset.h"
#include "general/temporal.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/span_gist.h"
#include "pg_general/temporal.h"

/*****************************************************************************
 * Data structures
 *****************************************************************************/

/**
 * @brief Structure to represent the bounding box of an inner node containing a
 * set of spans
 */
typedef struct
{
  Span left;
  Span right;
} SpanNode;

/**
 * @brief Structure to sort a set of spans of an inner node
 */
typedef struct SortedSpan
{
  Span s;
  int i;
} SortedSpan;

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Comparator to sort spans on their lower bound
 */
static int
span_lower_qsort_cmp(const void *a, const void *b)
{
  Span *pa = (Span *) a;
  Span *pb = (Span *) b;
  return span_lower_cmp(pa, pb);
}

/**
 * @brief Comparator to sort spans on their upper bound
 */
static int
span_upper_qsort_cmp(const void *a, const void *b)
{
  Span *pa = (Span *) a;
  Span *pb = (Span *) b;
  return span_upper_cmp(pa, pb);
}

/**
 * @brief Initialize the traversal value
 *
 * In the beginning, we don't have any restrictions.  We have to
 * initialize the struct to cover the whole 2D space.
 */
static void
spannode_init(SpanNode *nodebox, meosType spantype, meosType basetype)
{
  memset(nodebox, 0, sizeof(SpanNode));
  Datum min, max;
  assert(span_type(spantype));
  if (spantype == T_TSTZSPAN)
  {
    min = TimestampTzGetDatum(DT_NOBEGIN);
    max = TimestampTzGetDatum(DT_NOEND);
  }
  else if (spantype == T_INTSPAN)
  {
    min = Int32GetDatum(PG_INT32_MIN);
    max = Int32GetDatum(PG_INT32_MAX);
  }
  else if (spantype == T_BIGINTSPAN)
  {
    min = Int64GetDatum(PG_INT64_MIN);
    max = Int64GetDatum(PG_INT64_MAX);
  }
  else /* spantype == T_FLOATSPAN */
  {
    min = Float8GetDatum(-1 * DBL_MAX);
    max = Float8GetDatum(DBL_MAX);
  }
  nodebox->left.lower = nodebox->left.upper = min;
  nodebox->right.lower = nodebox->right.upper = max;
  nodebox->left.spantype = nodebox->right.spantype = spantype;
  nodebox->left.basetype = nodebox->right.basetype = basetype;
  return;
}

/**
 * @brief Copy a traversal value
 */
SpanNode *
spannode_copy(const SpanNode *orig)
{
  SpanNode *result = palloc(sizeof(SpanNode));
  memcpy(result, orig, sizeof(SpanNode));
  return result;
}

/**
 * @brief Compute the next traversal value for a quadtree given the bounding
 * box and the centroid of the current node and the quadrant number (0 to 3).
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
 * @brief Compute the next traversal value for a k-d tree given the bounding
 * box and the centroid of the current node, the half number (0 or 1), and the
 * level.
 *
 * For example, given the bounding box of the root node (level 0) and
 * the centroid as follows
 *     nodebox = (-infinity, -infinity)(infinity, infinity)
 *     centroid = (2001-06-19 09:07:00, 2001-06-19 09:13:00]
 * the halves are as follows
 *     0 = (-infinity, -infinity)(2001-06-19 09:07:00+02, infinity)
 *     1 = [2001-06-19 09:07:00+02, -infinity)(infinity, infinity)
 */
static void
spannode_kdtree_next(const SpanNode *nodebox, const Span *centroid,
  uint8 node, int level, SpanNode *next_nodespan)
{
  memcpy(next_nodespan, nodebox, sizeof(SpanNode));
  if (level % 2)
  {
    /* Split the bounding box by lower bound  */
    if (node == 0)
    {
      next_nodespan->right.lower = centroid->lower;
      next_nodespan->right.lower_inc = true;
    }
    else
    {
      /* The inclusive flag must be set to true so that the bounds with the
       * same timestamp are in one of the two children */
      next_nodespan->left.lower = centroid->lower;
      next_nodespan->left.lower_inc = true;
    }
  }
  else
  {
    /* Split the bounding box by upper bound */
    if (node == 0)
    {
      next_nodespan->right.upper = centroid->upper;
      next_nodespan->right.upper_inc = true;
    }
    else
    {
      /* The inclusive flag must be negated so that the bounds with the
       * same timestamp are in one of the two childs */
      next_nodespan->left.upper = centroid->upper;
      next_nodespan->left.upper_inc = true;
    }
  }
  return;
}

/**
 * @brief Calculate the quadrant
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
 * @brief Can any span from nodebox overlap with this argument?
 */
static bool
overlap2D(const SpanNode *nodebox, const Span *query)
{
  Span s;
  span_set(nodebox->left.lower, nodebox->right.upper, nodebox->left.lower_inc,
    nodebox->right.upper_inc, nodebox->left.basetype, &s);
  return overlaps_span_span(&s, query);
}

/**
 * @brief Can any span from nodebox contain the query?
 */
static bool
contain2D(const SpanNode *nodebox, const Span *query)
{
  Span s;
  span_set(nodebox->left.lower, nodebox->right.upper, nodebox->left.lower_inc,
    nodebox->right.upper_inc, nodebox->left.basetype, &s);
  return contains_span_span(&s, query);
}

/**
 * @brief Can any span from nodebox be left the query?
 */
static bool
left2D(const SpanNode *nodebox, const Span *query)
{
  return left_span_span(&nodebox->right, query);
}

/**
 * @brief Can any span from nodebox does not extend right the query?
 */
static bool
overLeft2D(const SpanNode *nodebox, const Span *query)
{
  return overleft_span_span(&nodebox->right, query);
}

/**
 * @brief Can any span from nodebox be right the query?
 */
static bool
right2D(const SpanNode *nodebox, const Span *query)
{
  return right_span_span(&nodebox->left, query);
}

/**
 * @brief Can any span from nodebox does not extend left the query?
 */
static bool
overRight2D(const SpanNode *nodebox, const Span *query)
{
  return overright_span_span(&nodebox->left, query);
}

/**
 * @brief Can any period from nodebox be before the query?
 */
static bool
before2D(const SpanNode *nodebox, const Span *query)
{
  return left_span_span(&nodebox->right, query);
}

/**
 * @brief Can any period from nodebox does not extend after the query?
 */
static bool
overBefore2D(const SpanNode *nodebox, const Span *query)
{
  return overleft_span_span(&nodebox->right, query);
}

/**
 * @brief Can any period from nodebox be after the query?
 */
static bool
after2D(const SpanNode *nodebox, const Span *query)
{
  return right_span_span(&nodebox->left, query);
}

/**
 * @brief Can any period from nodebox does not extend before the query?
 */
static bool
overAfter2D(const SpanNode *nodebox, const Span *query)
{
  return overright_span_span(&nodebox->left, query);
}

/**
 * @brief Distance between a query span and a box of spans
 */
static double
distance_span_nodespan(Span *query, SpanNode *nodebox)
{
  /* Determine the maximum span of the nodebox */
  Span s;
  span_set(nodebox->left.lower, nodebox->right.upper, nodebox->left.lower_inc,
    nodebox->right.upper_inc, nodebox->left.basetype, &s);

  /* Compute the distance between the query span and the nodebox span */
  return distance_span_span(query, &s);
}

/**
 * @brief Transform a query argument into a span.
 */
static bool
span_spgist_get_span(const ScanKeyData *scankey, Span *result)
{
  meosType type = oid_type(scankey->sk_subtype);
  if (span_basetype(type))
  {
    Datum d = scankey->sk_argument;
    span_set(d, d, true, true, type, result);
  }
  else if (set_type(type))
  {
    Set *s = DatumGetSetP(scankey->sk_argument);
    set_set_span(s, result);
  }
  else if (span_type(type))
  {
    Span *s = DatumGetSpanP(scankey->sk_argument);
    memcpy(result, s, sizeof(Span));
  }
  else if (spanset_type(type))
  {
    spanset_span_slice(scankey->sk_argument, result);
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

PGDLLEXPORT Datum Intspan_spgist_config(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intspan_spgist_config);
/**
 * @brief SP-GiST config function for span types
 */
Datum
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

PGDLLEXPORT Datum Bigintspan_spgist_config(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Bigintspan_spgist_config);
/**
 * @brief SP-GiST config function for span types
 */
Datum
Bigintspan_spgist_config(PG_FUNCTION_ARGS)
{
  spgConfigOut *cfg = (spgConfigOut *) PG_GETARG_POINTER(1);
  cfg->prefixType = type_oid(T_BIGINTSPAN);  /* A type represented by its bounding box */
  cfg->labelType = VOIDOID;  /* We don't need node labels. */
  cfg->leafType = type_oid(T_BIGINTSPAN);
  cfg->canReturnData = false;
  cfg->longValuesOK = false;
  PG_RETURN_VOID();
}

PGDLLEXPORT Datum Floatspan_spgist_config(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Floatspan_spgist_config);
/**
 * @brief SP-GiST config function for span types
 */
Datum
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

PGDLLEXPORT Datum Period_spgist_config(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Period_spgist_config);
/**
 * @brief SP-GiST config function for span types
 */
Datum
Period_spgist_config(PG_FUNCTION_ARGS)
{
  spgConfigOut *cfg = (spgConfigOut *) PG_GETARG_POINTER(1);
  cfg->prefixType = type_oid(T_TSTZSPAN);  /* A type represented by its bounding box */
  cfg->labelType = VOIDOID;  /* We don't need node labels. */
  cfg->leafType = type_oid(T_TSTZSPAN);
  cfg->canReturnData = false;
  cfg->longValuesOK = false;
  PG_RETURN_VOID();
}

/*****************************************************************************
 * Quad-tree choose functions
 *****************************************************************************/

/**
 * @brief Determine which quadrant a 2D-mapped span falls into, relative to the
 * centroid.
 *
 * Quadrants are numbered as follows:
 * @code
 *  3  |  0
 * ----+----
 *  2  |  1
 * @endcode
 * where the lower bound of span is the horizontal axis and upper bound the
 * vertical axis.
 *
 * Periods on one of the axes are taken to lie in the quadrant with higher value
 * along perpendicular axis. That is, a value on the horizontal axis is taken
 * to belong to quadrant 0 or 3, and a value on the vertical axis is taken to
 * belong to quadrant 0 or 1. A span equal to centroid is taken to lie in
 * quadrant 0.
 */

PGDLLEXPORT Datum Span_quadtree_choose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_quadtree_choose);
/**
 * @brief SP-GiST choose function for span types
 */
Datum
Span_quadtree_choose(PG_FUNCTION_ARGS)
{
  spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
  spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
  Span *centroid = DatumGetSpanP(in->prefixDatum),
    *span = DatumGetSpanP(in->leafDatum);

  if (in->allTheSame)
  {
    out->resultType = spgMatchNode;
    /* nodeN will be set by core */
    out->result.matchNode.levelAdd = 0;
    out->result.matchNode.restDatum = SpanPGetDatum(span);
    PG_RETURN_VOID();
  }

  /* Get quadrant number */
  assert(in->hasPrefix);
  int8 quadrant = get_quadrant2D(centroid, span);
  assert(quadrant < in->nNodes);

  /* Select node matching to quadrant number */
  out->resultType = spgMatchNode;
  out->result.matchNode.nodeN = quadrant;
  out->result.matchNode.levelAdd = 1;
  out->result.matchNode.restDatum = SpanPGetDatum(span);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * K-d tree choose function
 *****************************************************************************/

/**
 * @brief Determine which half a 2D-mapped span falls into, relative to the
 * centroid and the level number.
 *
 * Halves are numbered 0 and 1, and depending on whether the level number is
 * even or odd, respectively, they will be as follows:
 * @code
 * ----+----
 *  0  |  1
 * ----+----
 * @endcode
 * or
 * @code
 * ---------
 *    1
 * ---------
 *    0
 * ---------
 * @endcode
 * where the lower bound of the span is the horizontal axis and the upper
 * bound is the vertical axis.
 *
 * Periods whose lower/upper bound is equal to the centroid bound (including
 * their inclusive flag) may get classified into either node depending on
 * where they happen to fall in the sorted list. This is okay as long as the
 * inner_consistent function descends into both sides for such cases. This is
 * better than the alternative of trying to have an exact boundary, because
 * it keeps the tree balanced even when we have many instances of the same
 * span value. In this way, we should never trigger the allTheSame logic.
 */
static int
span_level_cmp(Span *centroid, Span *query, int level)
{
  if (level % 2)
    return span_lower_cmp(query, centroid);
  else
    return span_upper_cmp(query, centroid);
}

PGDLLEXPORT Datum Span_kdtree_choose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_kdtree_choose);
/**
 * @brief K-d tree choose function for span types
 */
Datum
Span_kdtree_choose(PG_FUNCTION_ARGS)
{
  spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
  spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
  Span *query = DatumGetSpanP(in->leafDatum), *centroid;
  assert(in->hasPrefix);
  centroid = DatumGetSpanP(in->prefixDatum);
  assert(in->nNodes == 2);
  out->resultType = spgMatchNode;
  out->result.matchNode.nodeN =
    (span_level_cmp(centroid, query, in->level) < 0) ? 0 : 1;
  out->result.matchNode.levelAdd = 1;
  out->result.matchNode.restDatum = SpanPGetDatum(query);
  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST pick-split function
 *****************************************************************************/

PGDLLEXPORT Datum Span_quadtree_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_quadtree_picksplit);
/**
 * @brief SP-GiST pick-split function for span types
 *
 * It splits a list of span types into quadrants by choosing a central 4D
 * point as the median of the coordinates of the span types.
 */
Datum
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

  median = in->nTuples >> 1;

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
 * K-d tree pick-split function
 *****************************************************************************/

PGDLLEXPORT Datum Span_kdtree_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_kdtree_picksplit);
/**
 * @brief K-d tree pick-split function for span types
 */
Datum
Span_kdtree_picksplit(PG_FUNCTION_ARGS)
{
  spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
  spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
  int median = in->nTuples >> 1, i;

  /* Sort the spans and determine the centroid */
  SortedSpan *sorted = palloc(sizeof(SortedSpan) * in->nTuples);
  for (i = 0; i < in->nTuples; i++)
  {
    memcpy(&sorted[i].s, DatumGetSpanP(in->datums[i]), sizeof(Span));
    sorted[i].i = i;
  }
  qsort(sorted, (size_t) in->nTuples, sizeof(SortedSpan),
    (in->level % 2) ? span_lower_qsort_cmp : span_upper_qsort_cmp);
  Span *centroid = span_copy(&sorted[median].s);

  /* Fill the output data structure */
  out->hasPrefix = true;
  out->prefixDatum = SpanPGetDatum(centroid);
  out->nNodes = 2;
  out->nodeLabels = NULL;    /* we don't need node labels */
  out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
  out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);
  /*
   * Note: points that have coordinates exactly equal to centroid may get
   * classified into either node, depending on where they happen to fall in
   * the sorted list.  This is okay as long as the inner_consistent function
   * descends into both sides for such cases.  This is better than the
   * alternative of trying to have an exact boundary, because it keeps the
   * tree balanced even when we have many instances of the same point value.
   * So we should never trigger the allTheSame logic.
   */
  for (i = 0; i < in->nTuples; i++)
  {
    Span *s = span_copy(&sorted[i].s);
    int n = sorted[i].i;
    out->mapTuplesToNodes[n] = (i < median) ? 0 : 1;
    out->leafTupleDatums[n] = SpanPGetDatum(s);
  }
  pfree(sorted);
  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST inner consistent functions
 *****************************************************************************/

/**
 * @brief Generic SP-GiST inner consistent function for span types
 */
Datum
Span_spgist_inner_consistent(FunctionCallInfo fcinfo, SPGistIndexType idxtype)
{
  spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
  spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
  int i;
  uint8 node;
  MemoryContext old_ctx;
  SpanNode *nodebox, infbox, next_nodespan;
  Span *centroid, *queries = NULL, *orderbys = NULL; /* make compiler quiet */

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
    spannode_init(&infbox, centroid->spantype, centroid->basetype);
    nodebox = &infbox;
  }

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

  if (in->allTheSame)
  {
    if (idxtype == SPGIST_QUADTREE)
    {
      /* Report that all nodes should be visited */
      out->nNodes = in->nNodes;
      out->nodeNumbers = palloc(sizeof(int) * in->nNodes);
      for (i = 0; i < in->nNodes; i++)
      {
        out->nodeNumbers[i] = i;
        if (in->norderbys > 0 && in->nNodes > 0)
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

          out->distances = palloc(sizeof(double *) * in->nNodes);
          out->distances[0] = distances;

          for (i = 1; i < in->nNodes; i++)
          {
            out->distances[i] = palloc(sizeof(double) * in->norderbys);
            memcpy(out->distances[i], distances, sizeof(double) * in->norderbys);
          }
        }
      }

      // pfree(orderbys);

      PG_RETURN_VOID();
    }
    else
      elog(ERROR, "allTheSame should not occur for k-d trees");
  }

  /* Transform the queries into spans */
  if (in->nkeys > 0)
  {
    queries = palloc0(sizeof(Span) * in->nkeys);
    for (i = 0; i < in->nkeys; i++)
      span_spgist_get_span(&in->scankeys[i], &queries[i]);
  }

  /* Allocate enough memory for nodes */
  out->nNodes = 0;
  out->nodeNumbers = palloc(sizeof(int) * in->nNodes);
  out->traversalValues = palloc(sizeof(void *) * in->nNodes);
  if (in->norderbys > 0)
    out->distances = palloc(sizeof(double *) * in->nNodes);

  /*
   * Switch memory context to allocate memory for new traversal values
   * (next_nodespan) and pass these pieces of memory to further calls
   * of this function.
   */
  old_ctx = MemoryContextSwitchTo(in->traversalMemoryContext);

  /* Loop for each child */
  for (node = 0; node < in->nNodes; node++)
  {
    /* Compute the bounding box of the child */
    if (idxtype == SPGIST_QUADTREE)
      spannode_quadtree_next(nodebox, centroid, node, &next_nodespan);
    else
      spannode_kdtree_next(nodebox, centroid, node, (in->level) + 1,
        &next_nodespan);
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
        case RTEqualStrategyNumber:
        case RTSameStrategyNumber:
          flag = contain2D(&next_nodespan, &queries[i]);
          break;
        case RTLeftStrategyNumber:
          flag = ! overRight2D(&next_nodespan, &queries[i]);
          break;
        case RTOverLeftStrategyNumber:
          flag = ! right2D(&next_nodespan, &queries[i]);
          break;
        case RTRightStrategyNumber:
          flag = ! overLeft2D(&next_nodespan, &queries[i]);
          break;
        case RTOverRightStrategyNumber:
          flag = ! left2D(&next_nodespan, &queries[i]);
          break;
        case RTBeforeStrategyNumber:
          flag = ! overAfter2D(&next_nodespan, &queries[i]);
          break;
        case RTOverBeforeStrategyNumber:
          flag = ! after2D(&next_nodespan, &queries[i]);
          break;
        case RTAfterStrategyNumber:
          flag = ! overBefore2D(&next_nodespan, &queries[i]);
          break;
        case RTOverAfterStrategyNumber:
          flag = ! before2D(&next_nodespan, &queries[i]);
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
      out->traversalValues[out->nNodes] = spannode_copy(&next_nodespan);
      out->nodeNumbers[out->nNodes] = node;
      /* Pass distances */
      if (in->norderbys > 0)
      {
        double *distances = palloc(sizeof(double) * in->norderbys);
        out->distances[out->nNodes] = distances;
        for (i = 0; i < in->norderbys; i++)
          distances[i] = distance_span_nodespan(&orderbys[i], &next_nodespan);
      }
      out->nNodes++;
    }
  } /* Loop for every child */

  /* Switch back to initial memory context */
  MemoryContextSwitchTo(old_ctx);

  if (in->nkeys > 0)
    pfree(queries);
  if (in->norderbys > 0)
    pfree(orderbys);

  PG_RETURN_VOID();
}

PGDLLEXPORT Datum Span_quadtree_inner_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_quadtree_inner_consistent);
/**
 * @brief Quad-tree inner consistent function for span types
 */
Datum
Span_quadtree_inner_consistent(PG_FUNCTION_ARGS)
{
  return Span_spgist_inner_consistent(fcinfo, SPGIST_QUADTREE);
}

PGDLLEXPORT Datum Span_kdtree_inner_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_kdtree_inner_consistent);
/**
 * @brief K-d tree inner consistent function for span types
 */
Datum
Span_kdtree_inner_consistent(PG_FUNCTION_ARGS)
{
  return Span_spgist_inner_consistent(fcinfo, SPGIST_KDTREE);
}

/*****************************************************************************
 * SP-GiST leaf-level consistency function
 *****************************************************************************/

PGDLLEXPORT Datum Span_spgist_leaf_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_spgist_leaf_consistent);
/**
 * @brief SP-GiST leaf-level consistency function for span types
 */
Datum
Span_spgist_leaf_consistent(PG_FUNCTION_ARGS)
{
  spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
  spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
  Span *key = DatumGetSpanP(in->leafDatum), span;
  bool result = true;
  int i;

  /* Initialize the value to do not recheck, will be updated below */
  out->recheck = false;

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
    /* All tests are lossy for temporal types */
    if (temporal_type(in->scankeys[i].sk_subtype))
      out->recheck = true;

    result = span_index_consistent_leaf(key, &span, strategy);

    /* If any check is failed, we have found our answer. */
    if (! result)
      break;
  }

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

  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * SP-GiST compress functions
 *****************************************************************************/

PGDLLEXPORT Datum Set_spgist_compress(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_spgist_compress);
/**
 * @brief SP-GiST compress function for timestamp sets
 */
Datum
Set_spgist_compress(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Span *result = palloc(sizeof(Span));
  set_set_span(s, result);
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Spanset_spgist_compress(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_spgist_compress);
/**
 * @brief SP-GiST compress function for period sets
 */
Datum
Spanset_spgist_compress(PG_FUNCTION_ARGS)
{
  Datum psdatum = PG_GETARG_DATUM(0);
  Span *result = palloc(sizeof(Span));
  spanset_span_slice(psdatum, result);
  PG_RETURN_SPAN_P(result);
}

/*****************************************************************************/
