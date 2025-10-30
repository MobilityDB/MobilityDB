/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Quad-tree SP-GiST index for span types
 *
 * The functions in this file are based on those in the file
 * `rangetypes_spgist.c`.
 */

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/span_index.h"
#include "temporal/stratnum.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Comparator to sort spans on their lower bound
 */
int
span_lower_qsort_cmp(const void *a, const void *b)
{
  Span *sa = (Span *) a;
  Span *sb = (Span *) b;
  return span_lower_cmp(sa, sb);
}

/**
 * @brief Comparator to sort spans on their upper bound
 */
int
span_upper_qsort_cmp(const void *a, const void *b)
{
  Span *sa = (Span *) a;
  Span *sb = (Span *) b;
  return span_upper_cmp(sa, sb);
}

/**
 * @brief Initialize the traversal value
 *
 * In the beginning, we don't have any restrictions.  We have to
 * initialize the struct to cover the whole 2D space.
 */
void
spannode_init(SpanNode *nodebox, meosType spantype, meosType basetype)
{
  memset(nodebox, 0, sizeof(SpanNode));
  Datum min, max;
  assert(span_type(spantype));
  switch (spantype)
  {
    case T_TSTZSPAN:
      min = TimestampTzGetDatum(DT_NOBEGIN);
      max = TimestampTzGetDatum(DT_NOEND);
      break;
    case T_DATESPAN:
      min = DateADTGetDatum(DATEVAL_NOBEGIN);
      max = DateADTGetDatum(DATEVAL_NOEND);
      break;
    case T_INTSPAN:
      min = Int32GetDatum(PG_INT32_MIN);
      max = Int32GetDatum(PG_INT32_MAX);
      break;
    case T_BIGINTSPAN:
      min = Int64GetDatum(PG_INT64_MIN);
      max = Int64GetDatum(PG_INT64_MAX);
      break;
    case T_FLOATSPAN:
      min = Float8GetDatum(-1 * DBL_MAX);
      max = Float8GetDatum(DBL_MAX);
      break;
    default: /* Error */
      elog(ERROR, "Unsupported span type for indexing: %d", spantype);
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
 * box and the centroid of the current node and the quadrant number (0 to 3)
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
void
spannode_quadtree_next(const SpanNode *nodebox, const Span *centroid,
  uint8 quadrant, SpanNode *next_nodespan)
{
  memcpy(next_nodespan, nodebox, sizeof(SpanNode));
  if (quadrant & 0x2)
  {
    next_nodespan->left.lower = centroid->lower;
    next_nodespan->left.lower_inc = true;
  }
  else
  {
    next_nodespan->left.upper = centroid->lower;
    next_nodespan->left.upper_inc = true;
  }
  if (quadrant & 0x1)
  {
    next_nodespan->right.lower = centroid->upper;
    next_nodespan->right.lower_inc = true;
  }
  else
  {
    next_nodespan->right.upper = centroid->upper;
    next_nodespan->right.upper_inc = true;
  }
  return;
}

/**
 * @brief Compute the next traversal value for a k-d tree given the bounding
 * box and the centroid of the current node, the half number (0 or 1), and the
 * level
 *
 * For example, given the bounding box of the root node (level 0) and
 * the centroid as follows
 *     nodebox = (-infinity, -infinity)(infinity, infinity)
 *     centroid = (2001-06-19 09:07:00, 2001-06-19 09:13:00]
 * the halves are as follows
 *     0 = (-infinity, -infinity)(2001-06-19 09:07:00+02, infinity)
 *     1 = [2001-06-19 09:07:00+02, -infinity)(infinity, infinity)
 */
void
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
      /* The inclusive flag must set to true so that the bounds with the
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
uint8
getQuadrant2D(const Span *centroid, const Span *query)
{
  uint8 quadrant = 0;
  if (span_lower_cmp(query, centroid) > 0)
    quadrant |= 0x2;
  if (span_upper_cmp(query, centroid) > 0)
    quadrant |= 0x1;
  return quadrant;
}

/**
 * @brief Can any span from nodebox overlap with the query?
 */
bool
overlap2D(const SpanNode *nodebox, const Span *query)
{
  Span s;
  span_set(nodebox->left.lower, nodebox->right.upper, nodebox->left.lower_inc,
    nodebox->right.upper_inc, nodebox->left.basetype, nodebox->left.spantype, &s);
  return overlaps_span_span(&s, query);
}

/**
 * @brief Can any span from nodebox contain the query?
 */
bool
contain2D(const SpanNode *nodebox, const Span *query)
{
  Span s;
  span_set(nodebox->left.lower, nodebox->right.upper, nodebox->left.lower_inc,
    nodebox->right.upper_inc, nodebox->left.basetype, nodebox->left.spantype, &s);
  return contains_span_span(&s, query);
}

/**
 * @brief Can any span from nodebox be to the left of the query?
 */
bool
left2D(const SpanNode *nodebox, const Span *query)
{
  return left_span_span(&nodebox->right, query);
}

/**
 * @brief Can any span from nodebox does not extend to the right of the query?
 */
bool
overLeft2D(const SpanNode *nodebox, const Span *query)
{
  return overleft_span_span(&nodebox->right, query);
}

/**
 * @brief Can any span from nodebox be right the query?
 */
bool
right2D(const SpanNode *nodebox, const Span *query)
{
  return right_span_span(&nodebox->left, query);
}

/**
 * @brief Can any span from nodebox does not extend to the left of the query?
 */
bool
overRight2D(const SpanNode *nodebox, const Span *query)
{
  return overright_span_span(&nodebox->left, query);
}

/**
 * @brief Can any span from nodebox be to the left of the query?
 */
bool
adjacent2D(const SpanNode *nodebox, const Span *query)
{
  return adjacent_span_span(&nodebox->left, query) ||
    adjacent_span_span(&nodebox->right, query);
}

/**
 * @brief Distance between a query span and a box of spans
 */
double
distance_span_nodespan(Span *query, SpanNode *nodebox)
{
  /* Determine the maximum span of the nodebox */
  Span s;
  span_set(nodebox->left.lower, nodebox->right.upper, nodebox->left.lower_inc,
    nodebox->right.upper_inc, nodebox->left.basetype, nodebox->left.spantype, &s);

  /* Compute the distance between the query span and the nodebox span */
  return distance_span_span(query, &s);
}

/**
 * @brief Transform a query argument into a span
 */
bool
span_spgist_get_span(Datum value, meosType type, Span *result)
{
  if (span_basetype(type))
  {
    meosType spantype = basetype_spantype(type);
    span_set(value, value, true, true, type, spantype, result);
  }
  else if (set_type(type))
  {
    Set *s = DatumGetSetP(value);
    set_set_span(s, result);
  }
  else if (span_type(type))
  {
    Span *s = DatumGetSpanP(value);
    memcpy(result, s, sizeof(Span));
  }
  else if (spanset_type(type))
  {
    SpanSet *ss = DatumGetSpanSetP(value);
    memcpy(result, &ss->span, sizeof(Span));
  }
  /* For temporal types whose bounding box is a timestamptz span */
  else if (temporal_type(type))
  {
    Temporal *temp = DatumGetTemporalP(value);
    temporal_set_tstzspan(temp, result);
  }
  else
    elog(ERROR, "Unsupported type for indexing: %d", type);
  return true;
}

/*****************************************************************************/
