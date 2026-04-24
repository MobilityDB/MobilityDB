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
 * @brief R-tree GiST strategies for the TPCBox bounding-box type.
 * @details Mirrors meos/src/geo/stbox_index.c — same dispatch surface,
 * but tpcbox_*_tpcbox predicates additionally enforce same-pcid.
 */

#include "pointcloud/tpcbox_index.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/stratnum.h"

/*****************************************************************************
 * GiST / SP-GiST consistent methods
 *****************************************************************************/

bool
tpcbox_index_leaf_consistent(const TPCBox *key, const TPCBox *query,
  StrategyNumber strategy)
{
  switch (strategy)
  {
    case RTOverlapStrategyNumber:
      return overlaps_tpcbox_tpcbox(key, query);
    case RTContainsStrategyNumber:
      return contains_tpcbox_tpcbox(key, query);
    case RTContainedByStrategyNumber:
      return contained_tpcbox_tpcbox(key, query);
    case RTSameStrategyNumber:
      return same_tpcbox_tpcbox(key, query);
    case RTAdjacentStrategyNumber:
      return adjacent_tpcbox_tpcbox(key, query);
    case RTLeftStrategyNumber:
      return left_tpcbox_tpcbox(key, query);
    case RTOverLeftStrategyNumber:
      return overleft_tpcbox_tpcbox(key, query);
    case RTRightStrategyNumber:
      return right_tpcbox_tpcbox(key, query);
    case RTOverRightStrategyNumber:
      return overright_tpcbox_tpcbox(key, query);
    case RTBelowStrategyNumber:
      return below_tpcbox_tpcbox(key, query);
    case RTOverBelowStrategyNumber:
      return overbelow_tpcbox_tpcbox(key, query);
    case RTAboveStrategyNumber:
      return above_tpcbox_tpcbox(key, query);
    case RTOverAboveStrategyNumber:
      return overabove_tpcbox_tpcbox(key, query);
    case RTFrontStrategyNumber:
      return front_tpcbox_tpcbox(key, query);
    case RTOverFrontStrategyNumber:
      return overfront_tpcbox_tpcbox(key, query);
    case RTBackStrategyNumber:
      return back_tpcbox_tpcbox(key, query);
    case RTOverBackStrategyNumber:
      return overback_tpcbox_tpcbox(key, query);
    case RTBeforeStrategyNumber:
      return before_tpcbox_tpcbox(key, query);
    case RTOverBeforeStrategyNumber:
      return overbefore_tpcbox_tpcbox(key, query);
    case RTAfterStrategyNumber:
      return after_tpcbox_tpcbox(key, query);
    case RTOverAfterStrategyNumber:
      return overafter_tpcbox_tpcbox(key, query);
    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unrecognized tpcbox strategy number: %d", strategy);
      return false;
  }
}

bool
tpcbox_gist_inner_consistent(const TPCBox *key, const TPCBox *query,
  StrategyNumber strategy)
{
  switch (strategy)
  {
    case RTOverlapStrategyNumber:
    case RTContainedByStrategyNumber:
      return overlaps_tpcbox_tpcbox(key, query);
    case RTContainsStrategyNumber:
    case RTSameStrategyNumber:
      return contains_tpcbox_tpcbox(key, query);
    case RTAdjacentStrategyNumber:
      if (adjacent_tpcbox_tpcbox(key, query))
        return true;
      return overlaps_tpcbox_tpcbox(key, query);
    case RTLeftStrategyNumber:
      return ! overright_tpcbox_tpcbox(key, query);
    case RTOverLeftStrategyNumber:
      return ! right_tpcbox_tpcbox(key, query);
    case RTRightStrategyNumber:
      return ! overleft_tpcbox_tpcbox(key, query);
    case RTOverRightStrategyNumber:
      return ! left_tpcbox_tpcbox(key, query);
    case RTBelowStrategyNumber:
      return ! overabove_tpcbox_tpcbox(key, query);
    case RTOverBelowStrategyNumber:
      return ! above_tpcbox_tpcbox(key, query);
    case RTAboveStrategyNumber:
      return ! overbelow_tpcbox_tpcbox(key, query);
    case RTOverAboveStrategyNumber:
      return ! below_tpcbox_tpcbox(key, query);
    case RTFrontStrategyNumber:
      return ! overback_tpcbox_tpcbox(key, query);
    case RTOverFrontStrategyNumber:
      return ! back_tpcbox_tpcbox(key, query);
    case RTBackStrategyNumber:
      return ! overfront_tpcbox_tpcbox(key, query);
    case RTOverBackStrategyNumber:
      return ! front_tpcbox_tpcbox(key, query);
    case RTBeforeStrategyNumber:
      return ! overafter_tpcbox_tpcbox(key, query);
    case RTOverBeforeStrategyNumber:
      return ! after_tpcbox_tpcbox(key, query);
    case RTAfterStrategyNumber:
      return ! overbefore_tpcbox_tpcbox(key, query);
    case RTOverAfterStrategyNumber:
      return ! before_tpcbox_tpcbox(key, query);
    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unrecognized tpcbox strategy number: %d", strategy);
      return false;
  }
}

/**
 * @brief Determine whether a recheck is necessary depending on the strategy.
 * @details Same conventions as stbox_index_recheck — bbox-only predicates
 * (position ops on closed boxes) are exact; predicates that depend on
 * inclusive/exclusive bounds (overlap, contains, same, before/after) are
 * lossy and require a recheck against the underlying tpcpoint / tpcpatch.
 */
bool
tpcbox_index_recheck(StrategyNumber strategy)
{
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
