/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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

#include "general/span_index.h"

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/span.h"
#include "general/temporal.h"

/*****************************************************************************
 * GiST consistent methods
 *****************************************************************************/

/**
 * @brief Leaf consistency for span types
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
      return over_span_span(key, query);
    case RTContainsStrategyNumber:
      return cont_span_span(key, query);
    case RTContainedByStrategyNumber:
      return cont_span_span(query, key);
    case RTEqualStrategyNumber:
    case RTSameStrategyNumber:
      return span_eq(key, query);
    case RTAdjacentStrategyNumber:
      return adj_span_span(key, query);
    case RTLeftStrategyNumber:
    case RTBeforeStrategyNumber:
      return lf_span_span(key, query);
    case RTOverLeftStrategyNumber:
    case RTOverBeforeStrategyNumber:
      return ovlf_span_span(key, query);
    case RTRightStrategyNumber:
    case RTAfterStrategyNumber:
      return ri_span_span(key, query);
    case RTOverRightStrategyNumber:
    case RTOverAfterStrategyNumber:
      return ovri_span_span(key, query);
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
    case RTContainedByStrategyNumber:
      return over_span_span(key, query);
    case RTContainsStrategyNumber:
    case RTEqualStrategyNumber:
    case RTSameStrategyNumber:
      return cont_span_span(key, query);
    case RTAdjacentStrategyNumber:
      return adj_span_span(key, query) || overlaps_span_span(key, query);
    case RTLeftStrategyNumber:
    case RTBeforeStrategyNumber:
      return ! ovri_span_span(key, query);
    case RTOverLeftStrategyNumber:
    case RTOverBeforeStrategyNumber:
      return ! ri_span_span(key, query);
    case RTRightStrategyNumber:
    case RTAfterStrategyNumber:
      return ! ovlf_span_span(key, query);
    case RTOverRightStrategyNumber:
    case RTOverAfterStrategyNumber:
      return ! lf_span_span(key, query);
    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unrecognized span strategy: %d", strategy);
      return false;    /* keep compiler quiet */
  }
}

/**
 * @brief Return true if a recheck is necessary depending on the strategy
 */
bool
span_index_recheck(StrategyNumber strategy)
{
  /* These operators are based on bounding boxes */
  if (strategy == RTLeftStrategyNumber ||
      strategy == RTBeforeStrategyNumber ||
      strategy == RTOverLeftStrategyNumber ||
      strategy == RTOverBeforeStrategyNumber ||
      strategy == RTRightStrategyNumber ||
      strategy == RTAfterStrategyNumber ||
      strategy == RTOverRightStrategyNumber ||
      strategy == RTOverAfterStrategyNumber ||
      strategy == RTKNNSearchStrategyNumber)
    return false;
  return true;
}

/*****************************************************************************/
