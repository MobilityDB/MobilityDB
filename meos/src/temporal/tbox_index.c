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
 * @brief R-tree GiST index for temporal integers and temporal floats
 *
 * These functions are based on those in the file `gistproc.c`.
 */

#include "temporal/tbox_index.h"

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/span.h"
#include "temporal/tbox.h"
#include "temporal/temporal_boxops.h"
#include "temporal/type_util.h"

/*****************************************************************************
 * GiST consistent methods
 *****************************************************************************/

/**
 * @brief Leaf consistency for temporal numbers
 * @details Since temporal boxes do not distinguish between inclusive and
 * exclusive bounds, it is necessary to generalize the tests, e.g.,
 * - left : (box1->xmax < box2->xmin) => (box1->xmax <= box2->xmin)
 *   e.g., to take into account left([a,b],(b,c])
 * - right : (box1->xmin > box2->xmax) => (box1->xmin >= box2->xmax)
 *   e.g., to take into account right((b,c],[a,b])
 * and similarly for before and after
 * @param[in] key Element in the index
 * @param[in] query Value being looked up in the index
 * @param[in] strategy Operator of the operator class being applied
 * @note This function is used for both GiST and SP-GiST indexes
 */
bool
tbox_index_leaf_consistent(const TBox *key, const TBox *query,
  StrategyNumber strategy)
{
  bool retval;

  switch (strategy)
  {
    case RTOverlapStrategyNumber:
      retval = overlaps_tbox_tbox(key, query);
      break;
    case RTContainsStrategyNumber:
      retval = contains_tbox_tbox(key, query);
      break;
    case RTContainedByStrategyNumber:
      retval = contained_tbox_tbox(key, query);
      break;
    case RTSameStrategyNumber:
      retval = same_tbox_tbox(key, query);
      break;
    case RTAdjacentStrategyNumber:
      retval = adjacent_tbox_tbox(key, query);
      break;
    case RTLeftStrategyNumber:
      retval = left_tbox_tbox(key, query);
      break;
    case RTOverLeftStrategyNumber:
      retval = overleft_tbox_tbox(key, query);
      break;
    case RTRightStrategyNumber:
      retval = right_tbox_tbox(key, query);
      break;
    case RTOverRightStrategyNumber:
      retval = overright_tbox_tbox(key, query);
      break;
    case RTBeforeStrategyNumber:
      retval = before_tbox_tbox(key, query);
      break;
    case RTOverBeforeStrategyNumber:
      retval = overbefore_tbox_tbox(key, query);
      break;
    case RTAfterStrategyNumber:
      retval = after_tbox_tbox(key, query);
      break;
    case RTOverAfterStrategyNumber:
      retval = overafter_tbox_tbox(key, query);
      break;
    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unrecognized tbox strategy number: %d", strategy);
      retval = false;    /* keep compiler quiet */
      break;
  }
  return retval;
}

/**
 * @brief GiST inner consistent method for temporal numbers
 *
 * Return false if for all data items x below entry, the predicate
 * x op query must be false, where op is the oper corresponding to
 * strategy in the pg_amop table.
 *
 * @param[in] key Element in the index
 * @param[in] query Value being looked up in the index
 * @param[in] strategy Operator of the operator class being applied
 */
bool
tbox_gist_inner_consistent(const TBox *key, const TBox *query,
  StrategyNumber strategy)
{
  bool retval;

  switch (strategy)
  {
    case RTOverlapStrategyNumber:
    case RTContainedByStrategyNumber:
      retval = overlaps_tbox_tbox(key, query);
      break;
    case RTContainsStrategyNumber:
    case RTSameStrategyNumber:
      retval = contains_tbox_tbox(key, query);
      break;
    case RTAdjacentStrategyNumber:
      retval = adjacent_tbox_tbox(key, query) ||
        overlaps_tbox_tbox(key, query);
      break;
    case RTLeftStrategyNumber:
      retval = ! overright_tbox_tbox(key, query);
      break;
    case RTOverLeftStrategyNumber:
      retval = ! right_tbox_tbox(key, query);
      break;
    case RTRightStrategyNumber:
      retval = ! overleft_tbox_tbox(key, query);
      break;
    case RTOverRightStrategyNumber:
      retval = ! left_tbox_tbox(key, query);
      break;
    case RTBeforeStrategyNumber:
      retval = ! overafter_tbox_tbox(key, query);
      break;
    case RTOverBeforeStrategyNumber:
      retval = ! after_tbox_tbox(key, query);
      break;
    case RTAfterStrategyNumber:
      retval = ! overbefore_tbox_tbox(key, query);
      break;
    case RTOverAfterStrategyNumber:
      retval = ! before_tbox_tbox(key, query);
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
tbox_index_recheck(StrategyNumber strategy)
{
  /* These operators are based on bounding boxes */
  switch (strategy)
  {
    case RTAdjacentStrategyNumber:
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
