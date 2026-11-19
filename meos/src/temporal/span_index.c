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
 * @brief R-tree GiST index for span and span set types
 * @note These functions are based on those in the file `rangetypes_gist.c`
 */

#include "temporal/span_index.h"

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/temporal.h"

/*****************************************************************************
 * GiST consistent methods
 *****************************************************************************/

/**
 * @brief Leaf consistency for span types
 * @details An index holds spans of one type -- the column fixes it, and the
 * query span is built from the operand type the operator class declares -- so
 * the two spans are comparable by construction. A descent therefore calls the
 * internal predicates, which assert that contract; the external ones test it,
 * and testing here would pay for it again at every entry the scan visits
 * @param[in] key Element in the index
 * @param[in] query Value being looked up in the index
 * @param[in] strategy Operator of the operator class being applied
 * @note This function is used for both GiST and SP-GiST indexes
 */
bool
span_index_leaf_consistent(const Span *key, const Span *query,
  StrategyNumber strategy)
{
  switch (strategy)
  {
    case RTOverlapStrategyNumber:
    case RTEverOverlapStrategyNumber:
    case RTAlwaysOverlapStrategyNumber:
      return span_overlaps(key, query);
    case RTContainsStrategyNumber:
      return span_contains(key, query);
    case RTContainedByStrategyNumber:
      return span_contains(query, key);
    case RTEqualStrategyNumber:
    case RTSameStrategyNumber:
      return span_eq(key, query);
    case RTAdjacentStrategyNumber:
      return span_adjacent(key, query);
    case RTLeftStrategyNumber:
    case RTBeforeStrategyNumber:
      return span_left(key, query);
    case RTOverLeftStrategyNumber:
    case RTOverBeforeStrategyNumber:
      return span_overleft(key, query);
    case RTRightStrategyNumber:
    case RTAfterStrategyNumber:
      return span_right(key, query);
    case RTOverRightStrategyNumber:
    case RTOverAfterStrategyNumber:
      return span_overright(key, query);
    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unrecognized span strategy: %d", strategy);
      return false;    /* keep compiler quiet */
  }
}

/**
 * @brief GiST internal-page consistency for span types
 * @param[in] key Element in the index
 * @param[in] query Value being looked up in the index
 * @param[in] strategy Operator of the operator class being applied
 */
bool
span_gist_inner_consistent(const Span *key, const Span *query,
  StrategyNumber strategy)
{
  switch (strategy)
  {
    case RTOverlapStrategyNumber:
    case RTEverOverlapStrategyNumber:
    case RTAlwaysOverlapStrategyNumber:
    case RTContainedByStrategyNumber:
      return span_overlaps(key, query);
    case RTContainsStrategyNumber:
    case RTEqualStrategyNumber:
    case RTSameStrategyNumber:
      return span_contains(key, query);
    case RTAdjacentStrategyNumber:
      return span_adjacent(key, query) || span_overlaps(key, query);
    case RTLeftStrategyNumber:
    case RTBeforeStrategyNumber:
      return ! span_overright(key, query);
    case RTOverLeftStrategyNumber:
    case RTOverBeforeStrategyNumber:
      return ! span_right(key, query);
    case RTRightStrategyNumber:
    case RTAfterStrategyNumber:
      return ! span_overleft(key, query);
    case RTOverRightStrategyNumber:
    case RTOverAfterStrategyNumber:
      return ! span_left(key, query);
    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unrecognized span strategy: %d", strategy);
      return false;
  }
}

/**
 * @brief Return true if a recheck is necessary depending on the strategy
 */
bool
span_index_recheck(StrategyNumber strategy)
{
  /* These operators are based on bounding boxes */
  switch (strategy)
  {
    case RTLeftStrategyNumber:
    case RTBeforeStrategyNumber:
    case RTOverLeftStrategyNumber:
    case RTOverBeforeStrategyNumber:
    case RTRightStrategyNumber:
    case RTAfterStrategyNumber:
    case RTOverRightStrategyNumber:
    case RTOverAfterStrategyNumber:
    case RTKNNSearchStrategyNumber:
      return false;
    default:
      return true;
  }
}

/**
 * @brief Increase the first span to include the second one
 * @details The span twin of #tbox_adjust and #stbox_adjust
 */
void
span_adjust(void *bbox1, void *bbox2)
{
  assert(bbox1); assert(bbox2);
  span_expand((const Span *) bbox2, (Span *) bbox1);
  return;
}

/**
 * @brief Return the size of a span, the distance between its bounds read in
 * its base type, as the penalty of its GiST index reads the growth of a span
 * @details The span twin of #tbox_size and #stbox_size, which measure the
 * boxes of the index the same way
 * @note The result can be +Infinity, but not NaN
 */
double
span_size(const void *bbox)
{
  assert(bbox);
  const Span *s = (const Span *) bbox;
  return distance_value_value_double(s->lower, s->upper, s->basetype);
}

/**
 * @brief Return the size of a span for a query span: its width plus the
 * width of the query span, to which a query of that width placed at random
 * reads it in proportion
 * @details The query twin of #span_size, the widths read as it reads them
 * @param[in] bbox Span
 * @param[in] query Query span, of the span type of the span
 */
double
span_query_size(const void *bbox, const void *query)
{
  assert(bbox); assert(query);
  const Span *q = (const Span *) query;
  assert(((const Span *) bbox)->spantype == q->spantype);
  return span_size(bbox) + span_size(q);
}

/*****************************************************************************/
