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
 * @brief SP-GiST implementation of 4-dimensional quad-tree and kd-tree over
 * temporal integers and temporal floats
 * @note These functions are based on those in the file ``geo_spgist.c`.
 * @details This module provides an SP-GiST implementation for temporal number
 * types using a quad tree analogy in 4-dimensional space. Since SP-GiST
 * does not allow indexing of overlapping objects, we transform 2D objects
 * in a 4D space to make them not overlap. This technique has some benefits
 * compared to traditional R-Tree which is implemented as GiST. The performance
 * tests reveal that this technique especially beneficial with too much
 * overlapping objects, so called "spaghetti data".
 *
 * Unlike the original quadtree, we are splitting the tree into 16 quadrants
 * in 4D space. It is easier to imagine it as splitting space two times into 4:
 * @code
 *              |      |
 *              |      |
 *              | -----+-----
 *              |      | c2
 *              |      |
 * -------------+-------------
 *              | c1
 *              |
 *              |
 *              |
 *              |
 * @endcode
 * where `c1` and `c2` are the centroids of the node and the child quadrant.
 *
 * We use centroids represented as a temporal box as the prefix, but we treat 
 * them as points in 4-dimensional space. Notice that 2D boxes are not enough 
 * to represent the quadrant boundaries in 4D space. However, they are 
 * sufficient to point out the additional boundaries of the next quadrant.
 *
 * We use node boxes (see below) composed by a left and a right temporal boxes
 * as traversal values to calculate and to store the bounds of the quadrants
 * while traversing the tree. A traversal value has all the boundaries in 
 * 4D space, and is is capable of transferring the required boundaries to the
 * following traversal values.  In conclusion, three things are necessary
 * to calculate the next traversal value:
 *
 *  1. the traversal value of the parent
 *  2. the quadrant of the current node
 *  3. the prefix of the current node
 *
 * If we visualize them on the above drawing, transferred boundaries of
 * (1) would be the relevant part of the outer axis, 
 * (2) would be the up right part of the other axis, and 
 * (3) would be the inner axis.
 *
 * For example, consider the case of overlapping.  When recursion descends
 * deeper and deeper down the tree, all quadrants in the current node will be
 * checked for overlapping.  The boundaries will be re-calculated for all
 * quadrants. Overlap check answers  the question: can any box from this
 * quadrant overlap with the given box? If yes, then this quadrant will be
 * walked. If no, then this quadrant will be skipped.
 *
 * This method provides restrictions for minimum and maximum values of
 * every dimension of every corner of the box on every level of the tree
 * except the root.  For the root node, we are setting the boundaries
 * that we don't yet have as infinity.
 */

#include "pg_temporal/tnumber_spgist.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#include <access/spgist.h>
#include <utils/float.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/span.h"
#include "temporal/stratnum.h"
#include "temporal/tbox.h"
#include "temporal/tbox_index.h"
#include "temporal/type_util.h"
#include "cbuffer/cbuffer.h"
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"
#include "pg_temporal/temporal.h"
#include "pg_temporal/tnumber_gist.h"

/*****************************************************************************
 * Data structures
 *****************************************************************************/

/**
 * @brief Structure to represent the bounding box of an inner node containing a
 * set of temporal boxes
 * @details The left box keeps, for the X and T dimensions, the ranges of the
 * lower bounds of the boxes in the quadrant, while the right box keeps the
 * ranges of the upper boxes.
 *
 * As an example, suppose that a quadrant contains two boxes 
 * @code
 * b1 = TBOXFLOAT XT([3, 5],[2001-01-03, 2001-01-05])
 * b3 = TBOXFLOAT XT([7, 9],[2001-01-07, 2001-01-09])
 * @endcode
 * The corresponding `TboxNode` will be 
 * @code
 * left = TBOXFLOAT XT([3, 7],[2001-01-03, 2001-01-07])
 * right = TBOXFLOAT XT([5, 9],[2001-01-05, 2001-01-09])
 * @endcode
 */
typedef struct
{
  TBox left;
  TBox right;
} TboxNode;

/**
 * @brief Structure to sort the temporal boxes of an inner node
 */
typedef struct SortedTbox
{
  TBox box;
  int i;
} SortedTbox;

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Initialize the traversal value
 * @details In the beginning, we don't have any restrictions. We initialize the
 * node box to cover the whole 4D space as follows
 * @code
 * left = TBOXFLOAT XT((-inf, +inf),(-inf, +inf))
 * right = TBOXFLOAT XT((-inf, +inf),(-inf, +inf))
 * @endcode
 */
static void
tboxnode_init(TBox *centroid, TboxNode *nodebox)
{
  Datum neginf, posinf;
  if (centroid->span.basetype == T_INT4)
  {
    posinf = Int32GetDatum(INT_MAX);
    neginf = Int32GetDatum(INT_MIN);
  }
  else
  {
    double d = get_float8_infinity();
    posinf = Float8GetDatum(d);
    neginf = Float8GetDatum(d * -1.0);
  }
  nodebox->left.span.lower = nodebox->right.span.lower = neginf;
  nodebox->left.span.upper = nodebox->right.span.upper = posinf;
  nodebox->left.span.spantype = nodebox->right.span.spantype =
    centroid->span.spantype;
  nodebox->left.span.basetype = nodebox->right.span.basetype =
    centroid->span.basetype;

  nodebox->left.period.lower = nodebox->right.period.lower =
    TimestampTzGetDatum(DT_NOBEGIN);
  nodebox->left.period.upper = nodebox->right.period.upper =
    TimestampTzGetDatum(DT_NOEND);
  nodebox->left.period.spantype = nodebox->right.period.spantype =
    centroid->period.spantype;
  nodebox->left.period.basetype = nodebox->right.period.basetype =
    centroid->period.basetype;
  nodebox->left.flags = nodebox->right.flags = centroid->flags;
  return;
}

/**
 * @brief Copy a traversal value
 */
TboxNode *
tboxnode_copy(const TboxNode *box)
{
  TboxNode *result = palloc(sizeof(TboxNode));
  memcpy(result, box, sizeof(TboxNode));
  return result;
}

/**
 * @brief Calculate the quadrant
 * @details The quadrant is 8 bit unsigned integer with 4 least bits in use.
 * This function accepts temporal boxes as input. All 4 bits are set by
 * comparing a corner of the box. This makes 16 quadrants in total.
 *
 * Continuing with the example at the top of this file 
 * @code
 *              |      |
 *              |      |
 *              | -----+-----
 *              |      | inbox
 *              |      |
 * -------------+-------------
 *              | centroid
 *              |
 *              |
 *              |
 *              |
 * @endcode
 * where `centroid` and `inbox` are as follows
 * @code
 * centroid = TBOXFLOAT XT([3, 5],[2001-01-03, 2001-01-05])
 * inbox = TBOXFLOAT XT([7, 9],[2001-01-07, 2001-01-09])
 * @endcode
 * Then
  - `quadrant |= 0x8` since 7 > 3
  - `quadrant |= 0x4` since 9 > 5
  - `quadrant |= 0x2` since 2001-01-07 > 2001-01-03
  - `quadrant |= 0x1` since 2001-01-09 > 2001-01-05
 */
static uint8
getQuadrant4D(const TBox *centroid, const TBox *inBox)
{
  uint8 quadrant = 0;

  if (datum_gt(inBox->span.lower, centroid->span.lower, inBox->span.basetype))
    quadrant |= 0x8;

  if (datum_gt(inBox->span.upper, centroid->span.upper, inBox->span.basetype))
    quadrant |= 0x4;

  if (datum_gt(inBox->period.lower, centroid->period.lower, T_TIMESTAMPTZ))
    quadrant |= 0x2;

  if (datum_gt(inBox->period.upper, centroid->period.upper, T_TIMESTAMPTZ))
    quadrant |= 0x1;

  return quadrant;
}

/**
 * @brief Calculate the next traversal value
 * @details All centroids are bounded by TboxNode, but SP-GiST only keeps
 * boxes. When we are traversing the tree, we must calculate TboxNode,
 * using centroid and quadrant.
 *
 * Continuing with the example at the top of this file 
 * @code
 *              |      |
 *              | 1110 | 1111
 *              | -----+-----
 *              | 1100 | 1101
 *              |      |
 * -------------+-------------
 *              | centroid
 *              |
 *              |
 *              |
 *              |
 * @endcode
 * where `centroid` is as follows
 * @code
 * c1 = TBOXFLOAT XT([3, 5],[2001-01-03, 2001-01-05])
 * centroid = TBOXFLOAT XT([3, 5],[2001-01-03, 2001-01-05])
 * @endcode
 * The next traversal value of quadrant ´1111´ is
 * @code
 * left = TBOXFLOAT XT([3, +inf),[2001-01-03, +inf))
 * right = TBOXFLOAT XT([5, +inf),[2001-01-05, +inf))
 * @endcode
 * Also, the traversal value of quadrant ´1100´ is
 * @code
 * left = TBOXFLOAT XT([3, +inf),(-inf, 2001-01-03))
 * right = TBOXFLOAT XT([5, +inf),(-inf, 2001-01-05))
 * @endcode
 */
static void
tboxnode_quadtree_next(const TboxNode *nodebox, const TBox *centroid,
  uint8 quadrant, TboxNode *next_nodebox)
{
  memcpy(next_nodebox, nodebox, sizeof(TboxNode));

  if (quadrant & 0x8)
    next_nodebox->left.span.lower = centroid->span.lower;
  else
    next_nodebox->left.span.upper = centroid->span.lower;

  if (quadrant & 0x4)
    next_nodebox->right.span.lower = centroid->span.upper;
  else
    next_nodebox->right.span.upper = centroid->span.upper;

  if (quadrant & 0x2)
    next_nodebox->left.period.lower = centroid->period.lower;
  else
    next_nodebox->left.period.upper = centroid->period.lower;

  if (quadrant & 0x1)
    next_nodebox->right.period.lower = centroid->period.upper;
  else
    next_nodebox->right.period.upper = centroid->period.upper;

  return;
}

/**
 * @brief Compute the next traversal value for a k-d tree given the bounding
 * box and the centroid of the current node, the half number (0 or 1) and the
 * level.
 */
static void
tboxnode_kdtree_next(const TboxNode *nodebox, const TBox *centroid,
  uint8 node, int level, TboxNode *next_nodebox)
{
  memcpy(next_nodebox, nodebox, sizeof(TboxNode));
  int mod = level % 4;
  if (mod == 0)
  {
    /* Split the bounding box by lower bound  */
    if (node == 0)
      next_nodebox->right.span.lower = centroid->span.lower;
    else
      next_nodebox->left.span.lower = centroid->span.lower;
  }
  else if (mod == 1)
  {
    /* Split the bounding box by upper bound */
    if (node == 0)
      next_nodebox->right.span.upper = centroid->span.upper;
    else
      next_nodebox->left.span.upper = centroid->span.upper;
  }
  else if (mod == 2)
  {
    /* Split the bounding box by lower bound  */
    if (node == 0)
      next_nodebox->right.period.lower = centroid->period.lower;
    else
      next_nodebox->left.period.lower = centroid->period.lower;
  }
  else /* mod == 3 */
  {
    /* Split the bounding box by upper bound */
    if (node == 0)
      next_nodebox->right.period.upper = centroid->period.upper;
    else
      next_nodebox->left.period.upper = centroid->period.upper;
  }
  return;
}

/**
 * @brief Can any box from nodebox overlap with the query?
 * @details A span `s1` contains a span `s2` if `s1.lower <= s2.lower`
 * and `s1.upper >= s2.upper`. Therefore, given a query box and a node box, a
 * @details Two spans overlap if the maximum of their lower bound is greater
 * than or equal to the minimum of their upper bound. Therefore, given a 
 * query box and a node box, the query box may overlap a box in the
 * node box if for each dimension X and T
 * - `nodebox->left.span.lower` (the minimum of the lower bounds in the node
 *   box) <= `query->span.upper` (the upper bound of the query).
 * - `nodebox->right.span.upper` (the maximum of the upper bounds in the node
 *   box) <= `query->span.lower` (the lower bound of the query).
 * 
 * Continuing with the example at the top of this file, if `TboxNode` is 
 * @code
 * left = TBOXFLOAT XT([3, 7],[2001-01-03, 2001-01-07])
 * right = TBOXFLOAT XT([5, 9],[2001-01-05, 2001-01-09])
 * @endcode
 * a query `TBOXFLOAT XT([2, 4],[2001-01-02, 2001-01-04]) satisfies the above
 * condition.
 */
static bool
overlap4D(const TboxNode *nodebox, const TBox *query)
{
  bool result = true;
  /* If the dimension is not missing */
  if (MEOS_FLAGS_GET_X(query->flags))
    result &=
      datum_le(nodebox->left.span.lower, query->span.upper, query->span.basetype) &&
      datum_ge(nodebox->right.span.upper, query->span.lower, query->span.basetype);
  /* If the dimension is not missing */
  if (MEOS_FLAGS_GET_T(query->flags))
    result &=
      datum_le(nodebox->left.period.lower, query->period.upper, T_TIMESTAMPTZ) &&
      datum_ge(nodebox->right.period.upper, query->period.lower, T_TIMESTAMPTZ);
  return result;
}

/**
 * @brief Can any box from nodebox contain the query?
 * @details A span `s1` contains a span `s2` if `s1.lower <= s2.lower`
 * and `s1.upper >= s2.upper`. Therefore, given a query box and a node box, a
 * box in the node box may contain the query box if for each
 * dimension X and T
 * - `nodebox->left.span.lower` (the minimum of the lower bounds in the node
 *   box) <= `query->span.lower` (the lower bound of the query).
 * - `nodebox->right.span.upper` (the maximum of the upper bounds in the node
 *   box) >= `query->span.upper` (the upper bound of the query).
 * 
 * Continuing with the example at the top of this file, if `TboxNode` is 
 * @code
 * left = TBOXFLOAT XT([3, 7],[2001-01-03, 2001-01-07])
 * right = TBOXFLOAT XT([5, 9],[2001-01-05, 2001-01-09])
 * @endcode
 * a query `TBOXFLOAT XT([3, 4],[2001-01-02, 2001-01-04]) satisfies the above
 * condition.
 */
static bool
contain4D(const TboxNode *nodebox, const TBox *query)
{
  bool result = true;
  /* If the dimension is not missing */
  if (MEOS_FLAGS_GET_X(query->flags))
    result &=
      datum_le(nodebox->left.span.lower, query->span.lower, query->span.basetype) &&
      datum_ge(nodebox->right.span.upper, query->span.upper, query->span.basetype);
  /* If the dimension is not missing */
  if (MEOS_FLAGS_GET_T(query->flags))
    result &=
      datum_ge(nodebox->right.period.upper, query->period.upper, T_TIMESTAMPTZ) &&
      datum_le(nodebox->left.period.lower, query->period.lower, T_TIMESTAMPTZ);
  return result;
}

/**
 * @brief Can any box from nodebox be to the left of the query?
 * @details A span `s1` is to the left of a span `s2` if `s1.upper < s2.lower`.
 * Therefore, given a query box and a node box, a box in the node box
 * may may be to the left of the query box if for dimension X
 * - `nodebox->right.span.upper` (the maximum of the upper bounds in the node
 *   box) < `query->span.lower` (the lower bound of the query).
 * 
 * Continuing with the example at the top of this file, if `TboxNode` is 
 * @code
 * left = TBOXFLOAT XT([3, 7],[2001-01-03, 2001-01-07])
 * right = TBOXFLOAT XT([5, 9],[2001-01-05, 2001-01-09])
 * @endcode
 * a query `TBOXFLOAT XT([10, 12],[2001-01-10, 2001-01-12]) satisfies the above
 * condition.
 */
static bool
left4D(const TboxNode *nodebox, const TBox *query)
{
  return datum_lt(nodebox->right.span.upper, query->span.lower,
    query->span.basetype);
}

/**
 * @brief Can any box from nodebox does not extend to the right of this query?
 * @details A span `s1` does not extend to the right of a span `s2` if
 * `s1.upper < s2.upper`. Therefore, given a query box and a node box, a box
 * in the node box may may not extend to the right of the query box
 * if for dimension X
 * - `nodebox->right.span.upper` (the maximum of the upper bounds in the node
 *   box) <= `query->span.upper` (the upper bound of the query).
 * 
 * Continuing with the example at the top of this file, if `TboxNode` is 
 * @code
 * left = TBOXFLOAT XT([3, 7],[2001-01-03, 2001-01-07])
 * right = TBOXFLOAT XT([5, 9],[2001-01-05, 2001-01-09])
 * @endcode
 * a query `TBOXFLOAT XT([10, 12],[2001-01-10, 2001-01-12]) satisfies the above
 * condition.
 */
static bool
overLeft4D(const TboxNode *nodebox, const TBox *query)
{
  return datum_le(nodebox->right.span.upper, query->span.upper,
    query->span.basetype);
}

/**
 * @brief Can any box from nodebox be right of the query?
 * @details A span `s1` is to the right of a span `s2` if `s1.lower > s2.upper`.
 * Therefore, given a query box and a node box, a box in the node box
 * may may be to the left of the query box if for dimension X
 * - `nodebox->left.span.lower` (the minimum of the lower bounds in the node
 *   box) > `query->span.upper` (the upper bound of the query).
 * 
 * Continuing with the example at the top of this file, if `TboxNode` is 
 * @code
 * left = TBOXFLOAT XT([3, 7],[2001-01-03, 2001-01-07])
 * right = TBOXFLOAT XT([5, 9],[2001-01-05, 2001-01-09])
 * @endcode
 * a query `TBOXFLOAT XT([10, 12],[2001-01-10, 2001-01-12]) satisfies the above
 * condition.
 */
static bool
right4D(const TboxNode *nodebox, const TBox *query)
{
  return datum_gt(nodebox->left.span.lower, query->span.upper, query->span.basetype);
}

/**
 * @brief Can any box from nodebox does not extend to the left of the query?
 * @details A span `s1` does not extend to the left of a span `s2` if
 * `s1.lower > s2.lower`. Therefore, given a query box and a node box, a box in
 * the node box may may not extend to the right of the query box
 * if for dimension X
 * - `nodebox->left.span.lower` (the minimum of the lower bounds in the node
 *   box) <= `query->span.lower` (the lower bound of the query).
 * 
 * Continuing with the example at the top of this file, if `TboxNode` is 
 * @code
 * left = TBOXFLOAT XT([3, 7],[2001-01-03, 2001-01-07])
 * right = TBOXFLOAT XT([5, 9],[2001-01-05, 2001-01-09])
 * @endcode
 * a query `TBOXFLOAT XT([10, 12],[2001-01-10, 2001-01-12]) satisfies the above
 * condition.
 */
static bool
overRight4D(const TboxNode *nodebox, const TBox *query)
{
  return datum_ge(nodebox->left.span.lower, query->span.lower,
    query->span.basetype);
}

/**
 * @brief Can any box from nodebox be before the query?
 * @note See above the explanations for #left4D
 */
static bool
before4D(const TboxNode *nodebox, const TBox *query)
{
  return datum_lt(nodebox->right.period.upper, query->period.lower,
    T_TIMESTAMPTZ);
}

/**
 * @brief Can any box from nodebox be not after the query?
 * @note See above the explanations for #overLeft4D
 */
static bool
overBefore4D(const TboxNode *nodebox, const TBox *query)
{
  return datum_le(nodebox->right.period.upper, query->period.upper,
    T_TIMESTAMPTZ);
}

/**
 * @brief Can any box from nodebox be after the query?
 * @note See above the explanations for #right4D
 */
static bool
after4D(const TboxNode *nodebox, const TBox *query)
{
  return datum_gt(nodebox->left.period.lower, query->period.upper,
    T_TIMESTAMPTZ);
}

/**
 * @brief Can any box from nodebox be not before the query?
 * @note See above the explanations for #overRight4D
 */
static bool
overAfter4D(const TboxNode *nodebox, const TBox *query)
{
  return datum_ge(nodebox->left.period.lower, query->period.lower,
    T_TIMESTAMPTZ);
}

/**
 * @brief Return the lower bound for the distance between query and nodebox
 * @details The distance between two spans `s1` and `s2` is 
 * - `s1.upper - s2.lower` if `s1` is to the left of `s2`: `s1.upper > s2.lower`
 * - `s1.lower - s2.upper` if `s1` is to the right of `s2`: `s1.lower > s2.upper`
 * - 0 if the `s1` and `s2` overlap
 * Therefore, given a query box and a node box, the minimum distance between a
 * box in the node box and the query will be 
 * - `nodebox->left.span.lower - query->span.upper` (the distance between the
 *   upper bound of the query and the minimum of the lower bounds in the node
 *   box).
 * - `query->span.lower - nodebox->right.span.upper` (the distance between the 
 *   lower bound of the query and the maximum of the upper bounds in the node
 *   box).
 * - 0 otherwise.
 * 
 * Continuing with the example at the top of this file, if `TboxNode` is 
 * @code
 * left = TBOXFLOAT XT([3, 7],[2001-01-03, 2001-01-07])
 * right = TBOXFLOAT XT([5, 9],[2001-01-05, 2001-01-09])
 * @endcode
 * the distance with a query `TBOXFLOAT XT([10, 12],[2001-01-10, 2001-01-12])
 * is 10 - 9 = 1 since the query is to the right of the node box
 * condition.
 * @note The distance is computed only on the value dimension. The temporal
 * dimension is taken into account for setting the distance to infinity when
 * the time spans of the node box and the query do not overlap.
 */
static double
distance_tbox_nodebox(const TBox *query, const TboxNode *nodebox)
{
  /* If the boxes do not intersect in the time dimension return infinity */
  bool hast = MEOS_FLAGS_GET_T(query->flags);
  if (hast && (
      datum_gt(query->period.lower, nodebox->right.period.upper, T_TIMESTAMPTZ) ||
      datum_gt(nodebox->left.period.lower, query->period.upper, T_TIMESTAMPTZ)))
    return DBL_MAX;

  double result;
  if (datum_lt(query->span.upper, nodebox->left.span.lower, query->span.basetype))
    result = (query->span.basetype == T_INT4) ?
      (double) (DatumGetInt32(nodebox->left.span.lower) - DatumGetInt32(query->span.upper)) :
      DatumGetFloat8(nodebox->left.span.lower) - DatumGetFloat8(query->span.upper);
  else if (datum_gt(query->span.lower, nodebox->right.span.upper, query->span.basetype))
    result = (query->span.basetype == T_INT4) ?
      (double) (DatumGetInt32(query->span.lower) - DatumGetInt32(nodebox->right.span.upper)) :
      DatumGetFloat8(query->span.lower) - DatumGetFloat8(nodebox->right.span.upper);
  else
    result = 0.0;
  return result;
}

/**
 * @brief Transform a query argument into a temporal box
 */
static bool
tnumber_spgist_get_tbox(const ScanKeyData *scankey, TBox *result)
{
  Span *s;
  meosType type = oid_meostype(scankey->sk_subtype);
  if (tnumber_spantype(type))
  {
    s = DatumGetSpanP(scankey->sk_argument);
    numspan_set_tbox(s, result);
  }
  else if (type == T_TSTZSPAN)
  {
    s = DatumGetSpanP(scankey->sk_argument);
    tstzspan_set_tbox(s, result);
  }
  else if (type == T_TBOX)
  {
    memcpy(result, DatumGetTboxP(scankey->sk_argument), sizeof(TBox));
  }
  else if (tnumber_type(type))
  {
    Temporal *temp = temporal_slice(scankey->sk_argument);
    tnumber_set_tbox(temp, result);
  }
  else
    elog(ERROR, "Unsupported type for indexing: %d", type);
  return true;
}

/**
 * @brief Comparator of temporal boxes based on their xmin value
 */
static int
tbox_xmin_cmp(const TBox *box1, const TBox *box2)
{
  assert(MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags));
  if (datum_eq(box1->span.lower, box2->span.lower, box1->span.basetype))
    return 0;
  return datum_gt(box1->span.lower, box2->span.lower, box1->span.basetype) ?
    1 : -1;
}

/**
 * @brief Comparator of temporal boxes based on their xmax value
 */
static int
tbox_xmax_cmp(const TBox *box1, const TBox *box2)
{
  assert(MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags));
  if (datum_eq(box1->span.upper, box2->span.upper, box1->span.basetype))
    return 0;
  return datum_gt(box1->span.upper, box2->span.upper, box1->span.basetype) ?
    1 : -1;
}

/**
 * @brief Comparator of temporal boxes based on their tmin value
 */
static int
tbox_tmin_cmp(const TBox *box1, const TBox *box2)
{
  assert(MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags));
  if (datum_eq(box1->period.lower, box2->period.lower, box1->period.basetype))
    return 0;
  return datum_gt(box1->period.lower, box2->period.lower,
    box1->period.basetype) ? 1 : -1;
}

/**
 * @brief Comparator of temporal boxes based on their tmax value
 */
static int
tbox_tmax_cmp(const TBox *box1, const TBox *box2)
{
  assert(MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags));
  if (datum_eq(box1->period.upper, box2->period.upper, box1->period.basetype))
    return 0;
  return datum_gt(box1->period.upper, box2->period.upper,
    box1->period.basetype) ? 1 : -1;
}

static int
tbox_level_cmp(TBox *centroid, TBox *query, int level)
{
  int mod = level % 4;
  if (mod == 0)
    return tbox_xmin_cmp(query, centroid);
  else if (mod == 1)
    return tbox_xmax_cmp(query, centroid);
  else if (mod == 2)
    return tbox_tmin_cmp(query, centroid);
  else
    return tbox_tmax_cmp(query, centroid);
}

/*****************************************************************************
 * SP-GiST config function
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_spgist_config(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_spgist_config);
/**
 * @brief SP-GiST config function for temporal numbers
 */
Datum
Tbox_spgist_config(PG_FUNCTION_ARGS)
{
  Oid tbox_oid = meostype_oid(T_TBOX);
  spgConfigOut *cfg = (spgConfigOut *) PG_GETARG_POINTER(1);
  cfg->prefixType = tbox_oid; /* A type represented by its bounding box */
  cfg->labelType = VOIDOID;  /* We don't need node labels */
  cfg->leafType = tbox_oid;
  cfg->canReturnData = false;
  cfg->longValuesOK = false;
  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST choose function
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_quadtree_choose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_quadtree_choose);
/**
 * @brief SP-GiST choose function for temporal numbers
 */
Datum
Tbox_quadtree_choose(PG_FUNCTION_ARGS)
{
  spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
  spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
  TBox *centroid = DatumGetTboxP(in->prefixDatum),
    *box = DatumGetTboxP(in->leafDatum);

  out->resultType = spgMatchNode;
  out->result.matchNode.restDatum = PointerGetDatum(box);

  /* nodeN will be set by core, when allTheSame. */
  if (!in->allTheSame)
    out->result.matchNode.nodeN = (int) getQuadrant4D(centroid, box);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * K-d tree choose function
 *****************************************************************************/

/**
 * @brief Determine which half a 4D-mapped temporal box falls into, relative to
 * the centroid and the level number
 *
 * Halves are numbered 0 and 1, and depending on the value of level number
 * modulo 4 is even or odd, the halves will be as follows:
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
 * where the lower bound of the splitting dimension is the horizontal axis and
 * the upper bound is the vertical axis.
 *
 * Boxes whose lower/upper bound of the splitting dimension is equal to the
 * centroid bound (including their inclusive flag) may get classified into
 * either node depending on where they happen to fall in the sorted list.
 * This is okay as long as the inner_consistent function descends into both
 * sides for such cases. This is better than the alternative of trying to
 * have an exact boundary, because it keeps the tree balanced even when we
 * have many instances of the same box value. In this way, we should never
 * trigger the allTheSame logic.
 */

PGDLLEXPORT Datum Tbox_kdtree_choose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_kdtree_choose);
/**
 * @brief K-d tree choose function for time types
 */
Datum
Tbox_kdtree_choose(PG_FUNCTION_ARGS)
{
  spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
  spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
  TBox *query = DatumGetTboxP(in->leafDatum), *centroid;
  assert(in->hasPrefix);
  centroid = DatumGetTboxP(in->prefixDatum);
  assert(in->nNodes == 2);
  out->resultType = spgMatchNode;
  out->result.matchNode.nodeN =
    (tbox_level_cmp(centroid, query, in->level) < 0) ? 0 : 1;
  out->result.matchNode.levelAdd = 1;
  out->result.matchNode.restDatum = TboxPGetDatum(query);
  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST pick-split function
 *****************************************************************************/

/**
 * @brief Comparator for qsort for integer values
 */
int
compareInt4(const void *a, const void *b)
{
  int x = DatumGetInt32(*(Datum *) a);
  int y = DatumGetInt32(*(Datum *) b);
  if (x == y)
    return 0;
  return (x > y) ? 1 : -1;
}

/**
 * @brief Comparator for qsort for double values
 * @note We don't need to use the floating point macros in here, because this
 * is only going to be used in a place to affect the performance of the index,
 * not the correctness.
 */
int
compareFloat8(const void *a, const void *b)
{
  double x = DatumGetFloat8(*(Datum *) a);
  double y = DatumGetFloat8(*(Datum *) b);
  if (x == y)
    return 0;
  return (x > y) ? 1 : -1;
}

/**
 * @brief Comparator for qsort for timestamp values
 */
int
compareTimestampTz(const void *a, const void *b)
{
  TimestampTz x = *(TimestampTz *) a;
  TimestampTz y = *(TimestampTz *) b;
  return timestamptz_cmp_internal(x,y);
}

PGDLLEXPORT Datum Tbox_quadtree_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_quadtree_picksplit);
/**
 * @brief SP-GiST pick-split function for temporal numbers
 *
 * It splits a list of boxes into quadrants by choosing a central 4D
 * point as the median of the coordinates of the boxes.
 */
Datum
Tbox_quadtree_picksplit(PG_FUNCTION_ARGS)
{
  spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
  spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
  TBox *centroid;
  int median, i;
  Datum *lowXs = palloc(sizeof(Datum) * in->nTuples);
  Datum *highXs = palloc(sizeof(Datum) * in->nTuples);
  TimestampTz *lowTs = palloc(sizeof(TimestampTz) * in->nTuples);
  TimestampTz *highTs = palloc(sizeof(TimestampTz) * in->nTuples);

  /* Get basetype of span in the datums */
  TBox *box = DatumGetTboxP(in->datums[0]);
  meosType basetype = box->span.basetype;
  /* Calculate median of all 4D coordinates */
  for (i = 0; i < in->nTuples; i++)
  {
    box = DatumGetTboxP(in->datums[i]);
    lowXs[i] = box->span.lower;
    highXs[i] = box->span.upper;
    lowTs[i] = DatumGetTimestampTz(box->period.lower);
    highTs[i] = DatumGetTimestampTz(box->period.upper);
  }

  qsort(lowXs, (size_t) in->nTuples, sizeof(Datum),
    (basetype == T_INT4) ? compareInt4 : compareFloat8);
  qsort(highXs, (size_t) in->nTuples, sizeof(Datum),
    (basetype == T_INT4) ? compareInt4 : compareFloat8);
  qsort(lowTs, (size_t) in->nTuples, sizeof(TimestampTz), compareTimestampTz);
  qsort(highTs, (size_t) in->nTuples, sizeof(TimestampTz), compareTimestampTz);

  median = in->nTuples >> 1;

  centroid = palloc0(sizeof(TBox));
  Span s, p;
  meosType spantype = basetype_spantype(basetype);
  span_set(lowXs[median], highXs[median], true, true, basetype, spantype, &s);
  span_set(lowTs[median], highTs[median], true, true, T_TIMESTAMPTZ,
    T_TSTZSPAN, &p);
  tbox_set(&s, &p, centroid);

  /* Fill the output */
  out->hasPrefix = true;
  out->prefixDatum = PointerGetDatum(centroid);
  out->nNodes = 16;
  out->nodeLabels = NULL;    /* We don't need node labels */
  out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
  out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);

  /*
   * Assign spans to corresponding nodes according to quadrants relative to
   * the "centroid" span
   */
  for (i = 0; i < in->nTuples; i++)
  {
    box = DatumGetTboxP(in->datums[i]);
    uint8 quadrant = getQuadrant4D(centroid, box);
    out->leafTupleDatums[i] = PointerGetDatum(box);
    out->mapTuplesToNodes[i] = quadrant;
  }

  pfree(lowXs); pfree(highXs);
  pfree(lowTs); pfree(highTs);

  PG_RETURN_VOID();
}

/*****************************************************************************/

PGDLLEXPORT Datum Tbox_kdtree_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_kdtree_picksplit);
/**
 * @brief K-d tree pick-split function for temporal number
 */
Datum
Tbox_kdtree_picksplit(PG_FUNCTION_ARGS)
{
  spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
  spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
  int i;

  /* Sort the boxes and determine the centroid */
  SortedTbox *sorted = palloc(sizeof(SortedTbox) * in->nTuples);
  for (i = 0; i < in->nTuples; i++)
  {
    memcpy(&sorted[i].box, DatumGetTboxP(in->datums[i]), sizeof(TBox));
    sorted[i].i = i;
  }
  qsort_comparator qsortfn;
  int mod = in->level % 4;
  if (mod == 0)
    qsortfn = (qsort_comparator) &tbox_xmin_cmp;
  else if (mod == 1)
    qsortfn = (qsort_comparator) &tbox_xmax_cmp;
  else if (mod == 2)
    qsortfn = (qsort_comparator) &tbox_tmin_cmp;
  else
    qsortfn = (qsort_comparator) &tbox_tmax_cmp;
  qsort(sorted, in->nTuples, sizeof(SortedTbox), qsortfn);
  int median = in->nTuples >> 1;
  TBox *centroid = tbox_copy(&sorted[median].box);

  /* Fill the output data structure */
  out->hasPrefix = true;
  out->prefixDatum = TboxPGetDatum(centroid);
  out->nNodes = 2;
  out->nodeLabels = NULL;    /* we don't need node labels */
  out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
  out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);
  /*
   * Note: points that have coordinates exactly equal to centroid may get
   * classified into either node, depending on where they happen to fall in
   * the sorted list. This is okay as long as the inner_consistent function
   * descends into both sides for such cases. This is better than the
   * alternative of trying to have an exact boundary, because it keeps the
   * tree balanced even when we have many instances of the same point value.
   * So we should never trigger the allTheSame logic.
   */
  for (i = 0; i < in->nTuples; i++)
  {
    TBox *box = tbox_copy(&sorted[i].box);
    int n = sorted[i].i;
    out->mapTuplesToNodes[n] = (i < median) ? 0 : 1;
    out->leafTupleDatums[n] = TboxPGetDatum(box);
  }
  pfree(sorted);
  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST inner consistent function
 *****************************************************************************/

/**
 * @brief Generic SP-GiST inner consistent function for temporal numbers
 */
static Datum
tbox_spgist_inner_consistent(FunctionCallInfo fcinfo, SPGistIndexType idxtype)
{
  spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
  spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
  int i;
  MemoryContext old_ctx;
  TBox *centroid, *queries = NULL, *orderbys = NULL; /* make compiler quiet */
  TboxNode *nodebox, infbox, next_nodebox;

  /* Fetch the centroid of this node. */
  assert(in->hasPrefix);
  centroid = DatumGetTboxP(in->prefixDatum);

  /*
   * We are saving the traversal value or initialize it an unbounded one, if
   * we have just begun to walk the tree.
   */
  if (in->traversalValue)
    nodebox = in->traversalValue;
  else
  {
    tboxnode_init(centroid, &infbox);
    nodebox = &infbox;
  }

  /*
   * Transform the orderbys into bounding boxes initializing the dimensions
   * that must not be taken into account for the operators to infinity.
   * This transformation is done here to avoid doing it for all nodes
   * in the loop below.
   */
  if (in->norderbys > 0)
  {
    orderbys = palloc0(sizeof(TBox) * in->norderbys);
    for (i = 0; i < in->norderbys; i++)
      tnumber_spgist_get_tbox(&in->orderbys[i], &orderbys[i]);
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
          /* Use parent node box as traversalValue */
          old_ctx = MemoryContextSwitchTo(in->traversalMemoryContext);
          out->traversalValues[i] = tboxnode_copy(nodebox);
          MemoryContextSwitchTo(old_ctx);

          /* Compute the distances */
          double *distances = palloc0(sizeof(double) * in->norderbys);
          out->distances[i] = distances;
          for (int j = 0; j < in->norderbys; j++)
            distances[j] = distance_tbox_nodebox(&orderbys[j], nodebox);

          pfree(orderbys);
        }
      }

      PG_RETURN_VOID();
    }
    else
      elog(ERROR, "allTheSame should not occur for k-d trees");
  }

  /* Transform the queries into bounding boxes. */
  if (in->nkeys > 0)
  {
    queries = palloc0(sizeof(TBox) * in->nkeys);
    for (i = 0; i < in->nkeys; i++)
      tnumber_spgist_get_tbox(&in->scankeys[i], &queries[i]);
  }

  /* Allocate enough memory for nodes */
  out->nNodes = 0;
  out->nodeNumbers = palloc(sizeof(int) * in->nNodes);
  out->levelAdds = palloc(sizeof(int) * in->nNodes);
  out->traversalValues = palloc(sizeof(void *) * in->nNodes);
  if (in->norderbys > 0)
    out->distances = palloc(sizeof(double *) * in->nNodes);

  /*
   * Switch memory context to allocate memory for new traversal values
   * (next_nodebox) and pass these pieces of memory to further calls of
   * this function
   */
  old_ctx = MemoryContextSwitchTo(in->traversalMemoryContext);

  /* Loop for each child */
  for (uint8 node = 0; node < (uint8) in->nNodes; node++)
  {
    /* Compute the bounding box of the child */
    if (idxtype == SPGIST_QUADTREE)
      tboxnode_quadtree_next(nodebox, centroid, node, &next_nodebox);
    else
      tboxnode_kdtree_next(nodebox, centroid, node, in->level, &next_nodebox);
    bool flag = true;
    for (i = 0; i < in->nkeys; i++)
    {
      StrategyNumber strategy = in->scankeys[i].sk_strategy;
      switch (strategy)
      {
        case RTOverlapStrategyNumber:
        case RTContainedByStrategyNumber:
        case RTAdjacentStrategyNumber:
          flag = overlap4D(&next_nodebox, &queries[i]);
          break;
        case RTContainsStrategyNumber:
        case RTSameStrategyNumber:
          flag = contain4D(&next_nodebox, &queries[i]);
          break;
        case RTLeftStrategyNumber:
          flag = ! overRight4D(&next_nodebox, &queries[i]);
          break;
        case RTOverLeftStrategyNumber:
          flag = ! right4D(&next_nodebox, &queries[i]);
          break;
        case RTRightStrategyNumber:
          flag = ! overLeft4D(&next_nodebox, &queries[i]);
          break;
        case RTOverRightStrategyNumber:
          flag = ! left4D(&next_nodebox, &queries[i]);
          break;
        case RTBeforeStrategyNumber:
          flag = ! overAfter4D(&next_nodebox, &queries[i]);
          break;
        case RTOverBeforeStrategyNumber:
          flag = ! after4D(&next_nodebox, &queries[i]);
          break;
        case RTAfterStrategyNumber:
          flag = ! overBefore4D(&next_nodebox, &queries[i]);
          break;
        case RTOverAfterStrategyNumber:
          flag = ! before4D(&next_nodebox, &queries[i]);
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
      out->traversalValues[out->nNodes] = tboxnode_copy(&next_nodebox);
      out->nodeNumbers[out->nNodes] = node;
      /* Increase level */
      out->levelAdds[out->nNodes] = 1;
      /* Pass distances */
      if (in->norderbys > 0)
      {
        double *distances = palloc(sizeof(double) * in->norderbys);
        out->distances[out->nNodes] = distances;
        for (i = 0; i < in->norderbys; i++)
          distances[i] = distance_tbox_nodebox(&orderbys[i], &next_nodebox);
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

PGDLLEXPORT Datum Tbox_quadtree_inner_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_quadtree_inner_consistent);
/**
 * @brief Quad-tree inner consistent function for temporal numbers
 */
Datum
Tbox_quadtree_inner_consistent(PG_FUNCTION_ARGS)
{
  return tbox_spgist_inner_consistent(fcinfo, SPGIST_QUADTREE);
}

PGDLLEXPORT Datum Tbox_kdtree_inner_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_kdtree_inner_consistent);
/**
 * @brief Kd-tree inner consistent function for temporal numbers
 */
Datum
Tbox_kdtree_inner_consistent(PG_FUNCTION_ARGS)
{
  return tbox_spgist_inner_consistent(fcinfo, SPGIST_KDTREE);
}

/*****************************************************************************
 * SP-GiST leaf consistency function
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_spgist_leaf_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_spgist_leaf_consistent);
/**
 * @brief SP-GiST leaf consistency function for temporal numbers
 */
Datum
Tbox_spgist_leaf_consistent(PG_FUNCTION_ARGS)
{
  spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
  spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
  TBox *key = DatumGetTboxP(in->leafDatum), box;
  bool result = true;
  int i;

  /*
   * All tests are lossy since boxes do not distinghish between inclusive
   * and exclusive bounds.
   */
  out->recheck = true;

  /* leafDatum is what it is... */
  out->leafValue = in->leafDatum;

  /* Perform the required comparison(s) */
  for (i = 0; i < in->nkeys; i++)
  {
    StrategyNumber strategy = in->scankeys[i].sk_strategy;
    /* Convert the query to a box and perform the test */
    if (tnumber_spgist_get_tbox(&in->scankeys[i], &box))
      result = tbox_index_leaf_consistent(key, &box, strategy);
    else
      result = false;
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
      /* Convert the order by argument to a box and perform the test */
      tnumber_spgist_get_tbox(&in->orderbys[i], &box);
      distances[i] = nad_tbox_tbox(&box, key);
    }
    /* Recheck is necessary when computing distance with bounding boxes */
    out->recheckDistances = true;
  }

  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * SP-GiST compress function
 *****************************************************************************/

PGDLLEXPORT Datum Tnumber_spgist_compress(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_spgist_compress);
/**
 * @brief SP-GiST compress function for temporal numbers
 */
Datum
Tnumber_spgist_compress(PG_FUNCTION_ARGS)
{
  Datum tempdatum = PG_GETARG_DATUM(0);
  Temporal *temp = temporal_slice(tempdatum);
  TBox *result = palloc(sizeof(TBox));
  tnumber_set_tbox(temp, result);
  PG_RETURN_TBOX_P(result);
}

/*****************************************************************************/
