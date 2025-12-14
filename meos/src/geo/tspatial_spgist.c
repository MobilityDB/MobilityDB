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
 * @brief SP-GiST implementation of 8-dimensional quad-tree over temporal
 * spatial values
 * @details  This module provides SP-GiST implementation for boxes using an
 * oct-tree analogy in 8-dimensional space. SP-GiST does not allow indexing of
 * overlapping objects. We are making 4D objects never-overlapping in
 * 8D space.  This technique has some benefits compared to traditional
 * R-Tree which is implemented as GiST. The performance tests reveal
 * that this technique especially beneficial with too much overlapping
 * objects, so called "spaghetti data".
 *
 * Unlike the original oct-tree, we are splitting the tree into 256
 * quadrants in 8D space.  It is easier to imagine it as splitting space
 * four times into four:
 * @code
 *              |      |                        |      |
 *              |      |                        |      |
 *              | -----+-----                   | -----+-----
 *              |      |                        |      |
 *              |      |                        |      |
 * -------------+------------- -+- -------------+-------------
 *              |                               |
 *              |                               |
 *              |                               |
 *              |                               |
 *              |                               |
 *            FRONT                           BACK
 * @endcode
 * We are using STBox data type as the prefix, but we are treating them
 * as points in 8-dimensional space, because 4D boxes are not enough
 * to represent the quadrant boundaries in 8D space.  They however are
 * sufficient to point out the additional boundaries of the next
 * quadrant.
 *
 * We are using traversal values provided by SP-GiST to calculate and
 * to store the bounds of the quadrants, while traversing into the tree.
 * Traversal value has all the boundaries in the 8D space, and is is
 * capable of transferring the required boundaries to the following
 * traversal values.  In conclusion, three things are necessary
 * to calculate the next traversal value:
 *
 *  1. the traversal value of the parent
 *  2. the quadrant of the current node
 *  3. the prefix of the current node
 *
 * If we visualize them on our simplified drawing (see the drawing above);
 * transferred boundaries of (1) would be the outer axis, relevant part
 * of (2) would be the up range_y part of the other axis, and (3) would be
 * the inner axis.
 *
 * For example, consider the case of overlapping.  When recursion
 * descends deeper and deeper down the tree, all quadrants in
 * the current node will be checked for overlapping.  The boundaries
 * will be re-calculated for all quadrants.  Overlap check answers
 * the question: can any box from this quadrant overlap with the given
 * box?  If yes, then this quadrant will be walked.  If no, then this
 * quadrant will be skipped.
 *
 * This method provides restrictions for minimum and maximum values of
 * every dimension of every corner of the box on every level of the tree
 * except the root.  For the root node, we are setting the boundaries
 * that we don't yet have as infinity.
 */

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/span.h"
#include "temporal/stratnum.h"
#include "temporal/temporal.h"
#include "temporal/type_util.h"
#include "geo/stbox.h"
#include "geo/stbox_index.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Copy an @p STboxNode
 */
STboxNode *
stboxnode_copy(const STboxNode *box)
{
  STboxNode *result = palloc(sizeof(STboxNode));
  memcpy(result, box, sizeof(STboxNode));
  return result;
}

/**
 * @brief Calculate the quadrant
 * @details The quadrant is an 8-bit unsigned integer with all bits in use.
 * This function accepts 2 STBox as input.  All 8 bits are set by comparing a
 * corner of the box. This makes 256 quadrants in total.
 */
uint8
getQuadrant8D(const STBox *centroid, const STBox *inBox)
{
  uint8 quadrant = 0;

  if (MEOS_FLAGS_GET_Z(centroid->flags))
  {
    if (inBox->zmin > centroid->zmin)
      quadrant |= 0x80;

    if (inBox->zmax > centroid->zmax)
      quadrant |= 0x40;
  }

  if (inBox->ymin > centroid->ymin)
    quadrant |= 0x20;

  if (inBox->ymax > centroid->ymax)
    quadrant |= 0x10;

  if (inBox->xmin > centroid->xmin)
    quadrant |= 0x08;

  if (inBox->xmax > centroid->xmax)
    quadrant |= 0x04;

  if (datum_gt(inBox->period.lower, centroid->period.lower, T_TIMESTAMPTZ))
    quadrant |= 0x02;

  if (datum_gt(inBox->period.upper, centroid->period.upper, T_TIMESTAMPTZ))
    quadrant |= 0x01;

  return quadrant;
}

/**
 * @brief Initialize the traversal value
 * @details In the beginning, we don't have any restrictions.  We have to
 * initialize the struct to cover the whole 8D space.
 */
void
stboxnode_init(const STBox *centroid, STboxNode *nodebox)
{
  memset(nodebox, 0, sizeof(STboxNode));
  double infinity = get_float8_infinity();

  nodebox->left.xmin = nodebox->right.xmin = -infinity;
  nodebox->left.xmax = nodebox->right.xmax = infinity;

  nodebox->left.ymin = nodebox->right.ymin = -infinity;
  nodebox->left.ymax = nodebox->right.ymax = infinity;

  nodebox->left.zmin = nodebox->right.zmin = -infinity;
  nodebox->left.zmax = nodebox->right.zmax = infinity;

  nodebox->left.period.lower = nodebox->right.period.lower =
    TimestampTzGetDatum(DT_NOBEGIN);
  nodebox->left.period.upper = nodebox->right.period.upper =
    TimestampTzGetDatum(DT_NOEND);

  nodebox->left.srid = nodebox->right.srid = centroid->srid;
  nodebox->left.flags = nodebox->right.flags = centroid->flags;

  return;
}

/**
 * @brief Calculate the next traversal value for an oct-tree
 * @details All centroids are bounded by STboxNode, but SP-GiST only keeps
 * boxes. When we are traversing the tree, we must calculate STboxNode,
 * using centroid and quadrant.
 */
void
stboxnode_quadtree_next(const STboxNode *nodebox, const STBox *centroid,
  uint8 quadrant, STboxNode *next_nodebox)
{
  memcpy(next_nodebox, nodebox, sizeof(STboxNode));

  if (MEOS_FLAGS_GET_Z(centroid->flags))
  {
    if (quadrant & 0x80)
      next_nodebox->left.zmin = centroid->zmin;
    else
      next_nodebox->left.zmax = centroid->zmin;

    if (quadrant & 0x40)
      next_nodebox->right.zmin = centroid->zmax;
    else
      next_nodebox->right.zmax = centroid->zmax;
  }

  if (quadrant & 0x20)
    next_nodebox->left.ymin = centroid->ymin;
  else
    next_nodebox->left.ymax = centroid->ymin;

  if (quadrant & 0x10)
    next_nodebox->right.ymin = centroid->ymax;
  else
    next_nodebox->right.ymax = centroid->ymax;

  if (quadrant & 0x08)
    next_nodebox->left.xmin = centroid->xmin;
  else
    next_nodebox->left.xmax = centroid->xmin;

  if (quadrant & 0x04)
    next_nodebox->right.xmin = centroid->xmax;
  else
    next_nodebox->right.xmax = centroid->xmax;

  if (quadrant & 0x02)
    next_nodebox->left.period.lower = centroid->period.lower;
  else
    next_nodebox->left.period.upper = centroid->period.lower;

  if (quadrant & 0x01)
    next_nodebox->right.period.lower = centroid->period.upper;
  else
    next_nodebox->right.period.upper = centroid->period.upper;

  return;
}

/**
 * @brief Compute the next traversal value for a k-d tree given the bounding
 * box and the centroid of the current node, and the level.
 */
void
stboxnode_kdtree_next(const STboxNode *nodebox, const STBox *centroid,
  uint8 node, int level, STboxNode *next_nodebox)
{
  bool hasz = MEOS_FLAGS_GET_Z(centroid->flags);
  memcpy(next_nodebox, nodebox, sizeof(STboxNode));
  int mod = hasz ? level % 8 : level % 6 ;
  if (mod == 0)
  {
    /* Split the bounding box by lower bound  */
    if (node == 0)
      next_nodebox->right.xmin = centroid->xmin;
    else
      next_nodebox->left.xmin = centroid->xmin;
  }
  else if (mod == 1)
  {
    /* Split the bounding box by upper bound */
    if (node == 0)
      next_nodebox->right.xmax = centroid->xmax;
    else
      next_nodebox->left.xmax = centroid->xmax;
  }
  else if (mod == 2)
  {
    /* Split the bounding box by lower bound  */
    if (node == 0)
      next_nodebox->right.ymin = centroid->ymin;
    else
      next_nodebox->left.ymin = centroid->ymin;
  }
  else if (mod == 3)
  {
    /* Split the bounding box by upper bound */
    if (node == 0)
      next_nodebox->right.ymax = centroid->ymax;
    else
      next_nodebox->left.ymax = centroid->ymax;
  }
  else if (hasz && mod == 4)
  {
    /* Split the bounding box by lower bound  */
    if (node == 0)
      next_nodebox->right.zmin = centroid->zmin;
    else
      next_nodebox->left.zmin = centroid->zmin;
  }
  else if (hasz && mod == 5)
  {
    /* Split the bounding box by upper bound */
    if (node == 0)
      next_nodebox->right.zmax = centroid->zmax;
    else
      next_nodebox->left.zmax = centroid->zmax;
  }
  else if ((hasz && mod == 6) || (! hasz && mod == 4))
  {
    /* Split the bounding box by lower bound  */
    if (node == 0)
      next_nodebox->right.period.lower = centroid->period.lower;
    else
      next_nodebox->left.period.lower = centroid->period.lower;
  }
  else /* (hasz && mod == 7) || (! hasz && mod == 5) */
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
 * @brief Can any box from a nodebox overlap with a query?
 */
bool
overlap8D(const STboxNode *nodebox, const STBox *query)
{
  bool result = true;
  /* Result value is computed only for the dimensions of the query */
  if (MEOS_FLAGS_GET_X(query->flags))
    result &= nodebox->left.xmin <= query->xmax &&
      nodebox->right.xmax >= query->xmin &&
      nodebox->left.ymin <= query->ymax &&
      nodebox->right.ymax >= query->ymin;
  if (MEOS_FLAGS_GET_Z(query->flags))
    result &= nodebox->left.zmin <= query->zmax &&
      nodebox->right.zmax >= query->zmin;
  if (MEOS_FLAGS_GET_T(query->flags))
    result &=
      datum_le(nodebox->left.period.lower, query->period.upper, T_TIMESTAMPTZ) &&
      datum_ge(nodebox->right.period.upper, query->period.lower, T_TIMESTAMPTZ);
  return result;
}

/**
 * @brief Can any box from a nodebox overlap with a query?
 */
bool
overlapKD(const STboxNode *nodebox, const STBox *query, int level)
{
  bool hasz = MEOS_FLAGS_GET_Z(nodebox->left.flags);
  int mod = hasz ? level % 8 : level % 6;
  bool result = true;
  /* Result value is computed only for the dimensions of the query */
  if (MEOS_FLAGS_GET_X(query->flags))
  {
    if (mod == 0)
      result &= nodebox->left.xmin <= query->xmax;
    else if (mod == 1)
      result &= nodebox->right.xmax >= query->xmin;
    else if (mod == 2)
      result &= nodebox->left.ymin <= query->ymax;
    else if (mod == 3)
      result &= nodebox->right.ymax >= query->ymin;
  }
  if (MEOS_FLAGS_GET_Z(query->flags))
  {
    if (hasz && mod == 4)
      result &= nodebox->left.zmin <= query->zmax;
    else if (hasz && mod == 5)
      result &= nodebox->right.zmax >= query->zmin;
  }
  if (MEOS_FLAGS_GET_T(query->flags))
  {
    if ((hasz && mod == 6) || (! hasz && mod == 4))
      result &= datum_le(nodebox->left.period.lower, query->period.upper,
        T_TIMESTAMPTZ);
    else /* (hasz && mod == 7) || (! hasz && mod == 5) */
      result &= datum_ge(nodebox->right.period.upper, query->period.lower,
        T_TIMESTAMPTZ);
  }
  return result;
}

/**
 * @brief Can any box from a nodebox contain a query?
 */
bool
contain8D(const STboxNode *nodebox, const STBox *query)
{
  bool result = true;
  /* Result value is computed only for the dimensions of the query */
  if (MEOS_FLAGS_GET_X(query->flags))
    result &= nodebox->right.xmax >= query->xmax &&
      nodebox->left.xmin <= query->xmin &&
      nodebox->right.ymax >= query->ymax &&
      nodebox->left.ymin <= query->ymin;
  if (MEOS_FLAGS_GET_Z(query->flags))
    result &= nodebox->right.zmax >= query->zmax &&
      nodebox->left.zmin <= query->zmin;
  if (MEOS_FLAGS_GET_T(query->flags))
    result &=
      datum_ge(nodebox->right.period.upper, query->period.upper, T_TIMESTAMPTZ) &&
      datum_le(nodebox->left.period.lower, query->period.lower, T_TIMESTAMPTZ);
  return result;
}

/**
 * @brief Can any box from a nodebox contain a query?
 */
bool
containKD(const STboxNode *nodebox, const STBox *query, int level)
{
  bool hasz = MEOS_FLAGS_GET_Z(nodebox->left.flags);
  int mod = hasz ? level % 8 : level % 6;
  bool result = true;
  /* Result value is computed only for the dimensions of the query */
  if (MEOS_FLAGS_GET_X(query->flags))
  {
    if (mod == 0)
      result &= nodebox->left.xmin <= query->xmin;
    else if (mod == 1)
      result &= nodebox->right.xmax >= query->xmax;
    else if (mod == 2)
      result &= nodebox->left.ymin <= query->ymin;
    else if (mod == 3)
      result &= nodebox->right.ymax >= query->ymax;
  }
  if (MEOS_FLAGS_GET_Z(query->flags))
  {
    if (hasz && mod == 4)
      result &= nodebox->left.zmin <= query->zmin;
    else if (hasz && mod == 5)
      result &= nodebox->right.zmax >= query->zmax;
  }
  if (MEOS_FLAGS_GET_T(query->flags))
  {
    if ((hasz && mod == 6) || (! hasz && mod == 4))
      result &= datum_le(nodebox->left.period.lower, query->period.lower,
        T_TIMESTAMPTZ);
    else /* (hasz && mod == 7) || (! hasz && mod == 5) */
      result &= datum_ge(nodebox->right.period.upper, query->period.upper,
        T_TIMESTAMPTZ);
  }
  return result;
}

/**
 * @brief Can any box from a nodebox be to the left of a query?
 */
bool
left8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->right.xmax < query->xmin);
}

/**
 * @brief Can any box from a nodebox does not extend to the right of a query?
 */
bool
overLeft8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->right.xmax <= query->xmax);
}

/**
 * @brief Can any box from a nodebox be to the right of a query?
 */
bool
right8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->left.xmin > query->xmax);
}

/**
 * @brief Can any box from a nodebox does not extend to the left of a query?
 */
bool
overRight8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->left.xmin >= query->xmin);
}

/**
 * @brief Can any box from a nodebox be below a query?
 */
bool
below8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->right.ymax < query->ymin);
}

/**
 * @brief Can any box from a nodebox does not extend above a query?
 */
bool
overBelow8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->right.ymax <= query->ymax);
}

/**
 * @brief Can any box from a nodebox be above a query?
 */
bool
above8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->left.ymin > query->ymax);
}

/**
 * @brief Can any box from a nodebox does not extend below a query?
 */
bool
overAbove8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->left.ymin >= query->ymin);
}

/**
 * @brief Can any box from a nodebox be in front of a query?
 */
bool
front8D(STboxNode *nodebox, STBox *query)
{
  return (nodebox->right.zmax < query->zmin);
}

/**
 * @brief Can any box from a nodebox does not extend to the back of a query?
 */
bool
overFront8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->right.zmax <= query->zmax);
}

/**
 * @brief Can any box from a nodebox be back to a query?
 */
bool
back8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->left.zmin > query->zmax);
}

/**
 * @brief Can any box from a nodebox does not extend to the front of a query?
 */
bool
overBack8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->left.zmin >= query->zmin);
}

/**
 * @brief Can any box from a nodebox be before a query?
 */
bool
before8D(const STboxNode *nodebox, const STBox *query)
{
  return datum_lt(nodebox->right.period.upper, query->period.lower,
    T_TIMESTAMPTZ);
}

/**
 * @brief Can any box from a nodebox be after a query?
 */
bool
overBefore8D(const STboxNode *nodebox, const STBox *query)
{
  return datum_le(nodebox->right.period.upper, query->period.upper,
    T_TIMESTAMPTZ);
}

/**
 * @brief Can any box from a nodebox be after a query?
 */
bool
after8D(const STboxNode *nodebox, const STBox *query)
{
  return datum_gt(nodebox->left.period.lower, query->period.upper,
    T_TIMESTAMPTZ);
}

/**
 * @brief Can any box from a nodebox be before a query?
 */
bool
overAfter8D(const STboxNode *nodebox, const STBox *query)
{
  return datum_ge(nodebox->left.period.lower, query->period.lower,
    T_TIMESTAMPTZ);
}

/**
 * @brief Return the lower bound for the distance between a query and a nodebox
 * @note The temporal dimension is only taken into account for returning
 * +infinity (which will be translated into NULL) if the boxes do not
 * intersect in time. Besides that, it is not possible to mix different
 * units in the computation. As a consequence, the filtering is not very
 * restrictive.
 */
double
distance_stbox_nodebox(const STBox *query, const STboxNode *nodebox)
{
  /* The query argument can be an empty geometry */
  if (! MEOS_FLAGS_GET_X(query->flags))
      return DBL_MAX;

  /* If the boxes do not intersect in the time dimension return infinity */
  bool hast = MEOS_FLAGS_GET_T(query->flags);
  if (hast && (
      datum_gt(query->period.lower, nodebox->right.period.upper, T_TIMESTAMPTZ) ||
      datum_gt(nodebox->left.period.lower, query->period.upper, T_TIMESTAMPTZ)))
    return DBL_MAX;

  double dx, dy, dz = 0; /* make compiler quiet */
  if (query->xmax < nodebox->left.xmin)
    dx = nodebox->left.xmin - query->xmax;
  else if (query->xmin > nodebox->right.xmax)
    dx = query->xmin - nodebox->right.xmax;
  else
    dx = 0;

  if (query->ymax < nodebox->left.ymin)
    dy = nodebox->left.ymin - query->ymax;
  else if (query->ymin > nodebox->right.ymax)
    dy = query->ymin - nodebox->right.ymax;
  else
    dy = 0;

  bool hasz = MEOS_FLAGS_GET_Z(nodebox->left.flags);
  if (hasz)
  {
    if (query->zmax < nodebox->left.zmin)
      dz = nodebox->left.zmin - query->zmax;
    else if (query->zmin > nodebox->right.zmax)
      dz = query->zmin - nodebox->right.zmax;
    else
      dz = 0;
  }

  return hasz ? hypot3d(dx, dy, dz) : hypot(dx, dy);
}

/**
 * @brief Return in the last argument a spatiotemporal box obtained from a query 
 */
bool
tspatial_spgist_get_stbox(Datum value, meosType type, STBox *result)
{
  if (type == T_TSTZSPAN)
  {
    Span *s = DatumGetSpanP(value);
    tstzspan_set_stbox(s, result);
  }
  else if (type == T_STBOX)
  {
    memcpy(result, DatumGetSTboxP(value), sizeof(STBox));
  }
  else if (tspatial_type(type))
  {
    Temporal *temp = DatumGetTemporalP(value);
    tspatial_set_stbox(temp, result);
  }
  else
    elog(ERROR, "Unsupported type for indexing: %d", type);
  return true;
}

/*****************************************************************************/
