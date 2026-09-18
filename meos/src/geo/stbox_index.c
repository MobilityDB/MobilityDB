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
 * @brief R-tree GiST index for temporal points
 */

#include "geo/stbox_index.h"

/* C */
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/bbox_index.h"
#include "temporal/span.h"
#include "temporal/type_util.h"
#include "geo/stbox.h"
#include "geo/stbox_index.h"

/*****************************************************************************
 * GiST consistent methods
 *****************************************************************************/

/**
 * @brief Leaf consistency for temporal points
 * @details Since spatiotemporal boxes do not distinguish between inclusive
 * and exclusive bounds it is necessary to generalize the tests, e.g.,
 * - before : (box1->tmax < box2->tmin) => (box1->tmax <= box2->tmin)
 *   e.g., to take into account before([a,b],(b,c])
 * - after : (box1->tmin > box2->tmax) => (box1->tmin >= box2->tmax)
 *   e.g., to take into account after((b,c],[a,b])
 * @param[in] key Element in the index
 * @param[in] query Value being looked up in the index
 * @param[in] strategy Operator of the operator class being applied
 * @note This function is used for both GiST and SP-GiST indexes
 */
bool
stbox_index_leaf_consistent(const STBox *key, const STBox *query,
  StrategyNumber strategy)
{
  bool retval;

  switch (strategy)
  {
    case RTOverlapStrategyNumber:
      retval = overlaps_stbox_stbox(key, query);
      break;
    case RTContainsStrategyNumber:
      retval = contains_stbox_stbox(key, query);
      break;
    case RTContainedByStrategyNumber:
      retval = contained_stbox_stbox(key, query);
      break;
    case RTSameStrategyNumber:
      retval = same_stbox_stbox(key, query);
      break;
    case RTAdjacentStrategyNumber:
      /* A spatiotemporal box does not preserve, in its overlap/adjacency
       * tests, whether a shared boundary belongs to the value or merely
       * touches it. A leaf whose box overlaps the query may still be
       * adjacent to it once the inclusive/exclusive bounds are taken into
       * account, so the candidate set is broadened to overlapping boxes and
       * filtered by the recheck against the actual operator. */
      retval = adjacent_stbox_stbox(key, query) ||
        overlaps_stbox_stbox(key, query);
      break;
    case RTLeftStrategyNumber:
      retval = left_stbox_stbox(key, query);
      break;
    case RTOverLeftStrategyNumber:
      retval = overleft_stbox_stbox(key, query);
      break;
    case RTRightStrategyNumber:
      retval = right_stbox_stbox(key, query);
      break;
    case RTOverRightStrategyNumber:
      retval = overright_stbox_stbox(key, query);
      break;
    case RTBelowStrategyNumber:
      retval = below_stbox_stbox(key, query);
      break;
    case RTOverBelowStrategyNumber:
      retval = overbelow_stbox_stbox(key, query);
      break;
    case RTAboveStrategyNumber:
      retval = above_stbox_stbox(key, query);
      break;
    case RTOverAboveStrategyNumber:
      retval = overabove_stbox_stbox(key, query);
      break;
    case RTFrontStrategyNumber:
      retval = front_stbox_stbox(key, query);
      break;
    case RTOverFrontStrategyNumber:
      retval = overfront_stbox_stbox(key, query);
      break;
    case RTBackStrategyNumber:
      retval = back_stbox_stbox(key, query);
      break;
    case RTOverBackStrategyNumber:
      retval = overback_stbox_stbox(key, query);
      break;
    case RTBeforeStrategyNumber:
      retval = before_stbox_stbox(key, query);
      break;
    case RTOverBeforeStrategyNumber:
      retval = overbefore_stbox_stbox(key, query);
      break;
    case RTAfterStrategyNumber:
      retval = after_stbox_stbox(key, query);
      break;
    case RTOverAfterStrategyNumber:
      retval = overafter_stbox_stbox(key, query);
      break;
    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unrecognized stbox strategy number: %d", strategy);
      retval = false;    /* keep compiler quiet */
      break;
  }
  return retval;
}

/**
 * @brief Inner consistent method for temporal points
 * @details Return false if for all data items x below entry, the predicate
 * x op query must be false, where op is the oper corresponding to strategy
 * in the pg_amop table.
 * @param[in] key Element in the index
 * @param[in] query Value being looked up in the index
 * @param[in] strategy Operator of the operator class being applied
 */
bool
stbox_gist_inner_consistent(const STBox *key, const STBox *query,
  StrategyNumber strategy)
{
  bool retval;

  switch (strategy)
  {
    case RTOverlapStrategyNumber:
    case RTContainedByStrategyNumber:
      retval = overlaps_stbox_stbox(key, query);
      break;
    case RTContainsStrategyNumber:
    case RTSameStrategyNumber:
      retval = contains_stbox_stbox(key, query);
      break;
    case RTAdjacentStrategyNumber:
      if (adjacent_stbox_stbox(key, query))
        return true;
      return overlaps_stbox_stbox(key, query);
    case RTLeftStrategyNumber:
      retval = ! overright_stbox_stbox(key, query);
      break;
    case RTOverLeftStrategyNumber:
      retval = ! right_stbox_stbox(key, query);
      break;
    case RTRightStrategyNumber:
      retval = ! overleft_stbox_stbox(key, query);
      break;
    case RTOverRightStrategyNumber:
      retval = ! left_stbox_stbox(key, query);
      break;
    case RTBelowStrategyNumber:
      retval = ! overabove_stbox_stbox(key, query);
      break;
    case RTOverBelowStrategyNumber:
      retval = ! above_stbox_stbox(key, query);
      break;
    case RTAboveStrategyNumber:
      retval = ! overbelow_stbox_stbox(key, query);
      break;
    case RTOverAboveStrategyNumber:
      retval = ! below_stbox_stbox(key, query);
      break;
    case RTFrontStrategyNumber:
      retval = ! overback_stbox_stbox(key, query);
      break;
    case RTOverFrontStrategyNumber:
      retval = ! back_stbox_stbox(key, query);
      break;
    case RTBackStrategyNumber:
      retval = ! overfront_stbox_stbox(key, query);
      break;
    case RTOverBackStrategyNumber:
      retval = ! front_stbox_stbox(key, query);
      break;
    case RTBeforeStrategyNumber:
      retval = ! overafter_stbox_stbox(key, query);
      break;
    case RTOverBeforeStrategyNumber:
      retval = ! after_stbox_stbox(key, query);
      break;
    case RTAfterStrategyNumber:
      retval = ! overbefore_stbox_stbox(key, query);
      break;
    case RTOverAfterStrategyNumber:
      retval = ! before_stbox_stbox(key, query);
      break;
    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unrecognized strategy number: %d", strategy);
      retval = false;    /* keep compiler quiet */
      break;
  }
  return retval;
}

/**
 * @brief Determine whether a recheck is necessary depending on the strategy
 * @param[in] strategy Operator of the operator class being applied
 */
bool
stbox_index_recheck(StrategyNumber strategy)
{
  /* These operators are based on bounding boxes and do not consider
   * inclusive or exclusive bounds.
   * The adjacent strategy is excluded: bounding-box adjacency cannot
   * distinguish a value whose extent lies strictly inside the query from
   * one that merely touches its boundary (the bounds are not preserved in
   * the box), so the leaf-consistent result is a superset that must be
   * rechecked against the actual operator. */
  switch (strategy)
  {
    case RTLeftStrategyNumber:
    case RTOverLeftStrategyNumber:
    case RTRightStrategyNumber:
    case RTOverRightStrategyNumber:
    case RTBelowStrategyNumber:
    case RTOverBelowStrategyNumber:
    case RTAboveStrategyNumber:
    case RTOverAboveStrategyNumber:
    case RTFrontStrategyNumber:
    case RTOverFrontStrategyNumber:
    case RTBackStrategyNumber:
    case RTOverBackStrategyNumber:
      return false;
    default:
      return true;
  }
}

/*****************************************************************************/

/*****************************************************************************
 * Growing and measuring a spatiotemporal box, for choosing where an entry
 * goes in an index node and for splitting one
 *****************************************************************************/

/**
 * @brief Increase the first box to include the second one
 */
void
stbox_adjust(void *bbox1, void *bbox2)
{
  STBox *box1 = (STBox *) bbox1;
  STBox *box2 = (STBox *) bbox2;
  box1->xmin = FLOAT8_MIN(box1->xmin, box2->xmin);
  box1->xmax = FLOAT8_MAX(box1->xmax, box2->xmax);
  box1->ymin = FLOAT8_MIN(box1->ymin, box2->ymin);
  box1->ymax = FLOAT8_MAX(box1->ymax, box2->ymax);
  box1->zmin = FLOAT8_MIN(box1->zmin, box2->zmin);
  box1->zmax = FLOAT8_MAX(box1->zmax, box2->zmax);
  if (MEOS_FLAGS_GET_T(box1->flags))
    span_expand(&box2->period, &box1->period);
  return;
}

/**
 * @brief Return the size of a spatiotemporal box for penalty calculation
 * @note The result can be +Infinity, but not NaN
 */
static double
stbox_size(const STBox *box)
{
  double result_size = 1;
  bool hasx = MEOS_FLAGS_GET_X(box->flags),
       hasz = MEOS_FLAGS_GET_Z(box->flags),
       hast = MEOS_FLAGS_GET_T(box->flags);
  /*
   * Check for zero-width cases.  Note that we define the size of a zero-
   * by-infinity box as zero.  It's important to special-case this somehow,
   * as naively multiplying infinity by zero will produce NaN.
   *
   * The less-than cases should not happen, but if they do, say "zero".
   */
  if ((hasx && (FLOAT8_LE(box->xmax, box->xmin) ||
                FLOAT8_LE(box->ymax, box->ymin) ||
                (hasz && FLOAT8_LE(box->zmax, box->zmin)))) ||
      (hast && datum_le(box->period.upper, box->period.lower, T_TIMESTAMPTZ)))
    return 0.0;

  /*
   * We treat NaN as larger than +Infinity, so any distance involving a NaN
   * and a non-NaN is infinite.  Note the previous check eliminated the
   * possibility that the low fields are NaNs.
   */
  if (hasx && (isnan(box->xmax) || isnan(box->ymax) || (hasz && isnan(box->zmax))))
    return get_float8_infinity();

  if (hasx)
  {
    result_size *= (box->xmax - box->xmin) * (box->ymax - box->ymin);
    if (hasz)
      result_size *= (box->zmax - box->zmin);
  }
  if (hast)
    /* Expressed in seconds */
    result_size *= (DatumGetTimestampTz(box->period.upper) -
      DatumGetTimestampTz(box->period.lower)) / USECS_PER_SEC;
  return result_size;
}

/**
 * @brief Return the amount by which the union of the two boxes is larger than
 * the first one
 * @details A point cloud box shares the layout of a spatiotemporal box on the
 * fields read here, so it is measured by the same function
 * @note The result can be +Infinity, but not NaN
 */
double
stbox_penalty(void *bbox1, void *bbox2)
{
  const STBox *original = (STBox *) bbox1;
  const STBox *new = (STBox *) bbox2;
  STBox unionbox;
  memcpy(&unionbox, original, sizeof(STBox));
  stbox_adjust(&unionbox, (void *) new);
  return stbox_size(&unionbox) - stbox_size(original);
}

/*****************************************************************************/
