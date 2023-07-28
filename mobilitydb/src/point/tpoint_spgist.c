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
 * @brief SP-GiST implementation of 8-dimensional quad-tree over temporal
 * points.
 *
 * This module provides SP-GiST implementation for boxes using an oct-tree
 * analogy in 8-dimensional space. SP-GiST doesn't allow indexing of
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
#include <access/spgist.h>
#include <access/spgist_private.h>
#include <utils/float.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "point/tpoint_boxops.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/temporal.h"
#include "pg_general/tnumber_spgist.h"
#include "pg_point/tpoint_gist.h"

/*****************************************************************************
 * Data structures
 *****************************************************************************/

/**
 * @brief Structure to represent the bounding box of a temporal point as a 6-
 * or 8-dimensional point depending on whether the temporal point is in 2D+T or
 * 3D+T.
 */
typedef struct
{
  STBox left;
  STBox right;
} STboxNode;

/**
 * @brief Structure to sort the temporal boxes of an inner node
 */
typedef struct SortedSTbox
{
  STBox box;
  int i;
} SortedSTbox;

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Copy a STboxNode
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
 *
 * The quadrant is 8 bit unsigned integer with all bits in use.
 * This function accepts 2 STBox as input.  All 8 bits are set by comparing a
 * corner of the box. This makes 256 quadrants in total.
 */
static uint8
getOctant8D(const STBox *centroid, const STBox *inBox)
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
 *
 * In the beginning, we don't have any restrictions.  We have to
 * initialize the struct to cover the whole 8D space.
 */
static void
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
 * @brief Calculate the next traversal value
 *
 * All centroids are bounded by STboxNode, but SP-GiST only keeps
 * boxes. When we are traversing the tree, we must calculate STboxNode,
 * using centroid and quadrant.
 */
static void
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
 * box and the centroid of the current node, the half number (0 or 1) and the
 * level.
 */
static void
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
 * @brief Can any box from nodebox overlap with query?
 */
static bool
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
 * @brief Can any box from nodebox overlap with query?
 */
static bool
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
 * @brief Can any box from nodebox contain query?
 */
static bool
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
 * @brief Can any box from nodebox overlap with query?
 */
static bool
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
 * @brief Can any box from nodebox be left of query?
 */
static bool
left8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->right.xmax < query->xmin);
}

/**
 * @brief Can any box from nodebox does not extend the right of query?
 */
static bool
overLeft8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->right.xmax <= query->xmax);
}

/**
 * @brief Can any box from nodebox be right of query?
 */
static bool
right8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->left.xmin > query->xmax);
}

/**
 * @brief Can any box from nodebox does not extend the left of query?
 */
static bool
overRight8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->left.xmin >= query->xmin);
}

/**
 * @brief Can any box from nodebox be below of query?
 */
static bool
below8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->right.ymax < query->ymin);
}

/**
 * @brief Can any box from nodebox does not extend above query?
 */
static bool
overBelow8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->right.ymax <= query->ymax);
}

/**
 * @brief Can any box from nodebox be above of query?
 */
static bool
above8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->left.ymin > query->ymax);
}

/**
 * @brief Can any box from nodebox does not extend below of query?
 */
static bool
overAbove8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->left.ymin >= query->ymin);
}

/**
 * @brief Can any box from nodebox be in front of query?
 */
static bool
front8D(STboxNode *nodebox, STBox *query)
{
  return (nodebox->right.zmax < query->zmin);
}

/**
 * @brief Can any box from nodebox does not extend the back of query?
 */
static bool
overFront8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->right.zmax <= query->zmax);
}

/**
 * @brief Can any box from nodebox be back to query?
 */
static bool
back8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->left.zmin > query->zmax);
}

/**
 * @brief Can any box from nodebox does not extend the front of query?
 */
static bool
overBack8D(const STboxNode *nodebox, const STBox *query)
{
  return (nodebox->left.zmin >= query->zmin);
}

/**
 * @brief Can any box from nodebox be before of query?
 */
static bool
before8D(const STboxNode *nodebox, const STBox *query)
{
  return datum_lt(nodebox->right.period.upper, query->period.lower, T_TIMESTAMPTZ);
}

/**
 * @brief Can any box from nodebox does not extend the after of query?
 */
static bool
overBefore8D(const STboxNode *nodebox, const STBox *query)
{
  return datum_le(nodebox->right.period.upper, query->period.upper, T_TIMESTAMPTZ);
}

/**
 * @brief Can any box from nodebox be after of query?
 */
static bool
after8D(const STboxNode *nodebox, const STBox *query)
{
  return datum_gt(nodebox->left.period.lower, query->period.upper, T_TIMESTAMPTZ);
}

/**
 * @brief Can any box from nodebox does not extend the before of query?
 */
static bool
overAfter8D(const STboxNode *nodebox, const STBox *query)
{
  return datum_ge(nodebox->left.period.lower, query->period.lower, T_TIMESTAMPTZ);
}

/**
 * @brief Lower bound for the distance between query and nodebox.
 * @note The temporal dimension is only taken into account for returning
 * +infinity (which will be translated into NULL) if the boxes do not
 * intersect in time. Besides that, it is not possible to mix different
 * units in the computation. As a consequence, the filtering is not very
 * restrictive.
 */
static double
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
 * @brief Transform a query argument into an STBox.
 */
static bool
tpoint_spgist_get_stbox(const ScanKeyData *scankey, STBox *result)
{
  meosType type = oid_type(scankey->sk_subtype);
  if (type == T_TSTZSPAN)
  {
    Span *p = DatumGetSpanP(scankey->sk_argument);
    period_set_stbox(p, result);
  }
  else if (type == T_STBOX)
  {
    memcpy(result, DatumGetSTboxP(scankey->sk_argument), sizeof(STBox));
  }
  else if (tspatial_type(type))
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

PGDLLEXPORT Datum Stbox_spgist_config(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_spgist_config);
/**
 * @brief SP-GiST config function for temporal points
 */
Datum
Stbox_spgist_config(PG_FUNCTION_ARGS)
{
  spgConfigOut *cfg = (spgConfigOut *) PG_GETARG_POINTER(1);
  Oid stbox_oid = type_oid(T_STBOX);
  cfg->prefixType = stbox_oid;  /* A type represented by its bounding box */
  cfg->labelType = VOIDOID;     /* We don't need node labels. */
  cfg->leafType = stbox_oid;
  cfg->canReturnData = false;
  cfg->longValuesOK = false;
  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST choose function
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_quadtree_choose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_quadtree_choose);
/**
 * @brief SP-GiST choose function for temporal points
 */
Datum
Stbox_quadtree_choose(PG_FUNCTION_ARGS)
{
  spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
  spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
  STBox *centroid = DatumGetSTboxP(in->prefixDatum),
    *box = DatumGetSTboxP(in->leafDatum);

  out->resultType = spgMatchNode;
  out->result.matchNode.restDatum = PointerGetDatum(box);

  /* nodeN will be set by core, when allTheSame. */
  if (!in->allTheSame)
    out->result.matchNode.nodeN = getOctant8D(centroid, box);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * K-d tree choose function
 *****************************************************************************/

/**
 * @brief Determine which half a 4D-mapped temporal box falls into, relative to
 * the centroid and the level number.
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

/**
 * @brief Comparator of temporal boxes based on their xmin value
 */
static int
stbox_xmin_cmp(const STBox *box1, const STBox *box2)
{
  assert(MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags));
  if (box1->xmin == box2->xmin)
    return 0;
  return (box1->xmin > box2->xmin) ? 1 : -1;
}

/**
 * @brief Comparator of temporal boxes based on their xmax value
 */
static int
stbox_xmax_cmp(const STBox *box1, const STBox *box2)
{
  assert(MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags));
  if (box1->xmax == box2->xmax)
    return 0;
  return (box1->xmax > box2->xmax) ? 1 : -1;
}

/**
 * @brief Comparator of temporal boxes based on their ymin value
 */
static int
stbox_ymin_cmp(const STBox *box1, const STBox *box2)
{
  assert(MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags));
  if (box1->ymin == box2->ymin)
    return 0;
  return (box1->ymin > box2->ymin) ? 1 : -1;
}

/**
 * @brief Comparator of temporal boxes based on their ymax value
 */
static int
stbox_ymax_cmp(const STBox *box1, const STBox *box2)
{
  assert(MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags));
  if (box1->ymax == box2->ymax)
    return 0;
  return (box1->ymax > box2->ymax) ? 1 : -1;
}

/**
 * @brief Comparator of temporal boxes based on their zmin value
 */
static int
stbox_zmin_cmp(const STBox *box1, const STBox *box2)
{
  assert(MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags));
  if (box1->zmin == box2->zmin)
    return 0;
  return (box1->zmin > box2->zmin) ? 1 : -1;
}

/**
 * @brief Comparator of temporal boxes based on their zmax value
 */
static int
stbox_zmax_cmp(const STBox *box1, const STBox *box2)
{
  assert(MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags));
  if (box1->zmax == box2->zmax)
    return 0;
  return (box1->zmax > box2->zmax) ? 1 : -1;
}

/**
 * @brief Comparator of temporal boxes based on their tmin value
 */
static int
stbox_tmin_cmp(const STBox *box1, const STBox *box2)
{
  assert(MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags));
  if (datum_eq2(box1->period.lower, box2->period.lower, box1->period.basetype,
        box2->period.basetype))
    return 0;
  return datum_gt2(box1->period.lower, box2->period.lower, box1->period.basetype,
        box2->period.basetype) ? 1 : -1;
}

/**
 * @brief Comparator of temporal boxes based on their tmax value
 */
static int
stbox_tmax_cmp(const STBox *box1, const STBox *box2)
{
  assert(MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags));
  if (datum_eq2(box1->period.upper, box2->period.upper, box1->period.basetype,
        box2->period.basetype))
    return 0;
  return datum_gt2(box1->period.upper, box2->period.upper, box1->period.basetype,
        box2->period.basetype) ? 1 : -1;
}

/*****************************************************************************/

static int
stbox_level_cmp(STBox *centroid, STBox *query, int level)
{
  bool hasz = MEOS_FLAGS_GET_Z(centroid->flags);
  int mod = hasz ? level % 8 : level % 6;
  if (mod == 0)
    return stbox_xmin_cmp(query, centroid);
  else if (mod == 1)
    return stbox_xmax_cmp(query, centroid);
  else if (mod == 2)
    return stbox_ymin_cmp(query, centroid);
  else if (mod == 3)
    return stbox_ymax_cmp(query, centroid);
  else if (hasz && mod == 4)
    return stbox_zmin_cmp(query, centroid);
  else if (hasz && mod == 5)
    return stbox_zmax_cmp(query, centroid);
  else if ((hasz && mod == 6) || (! hasz && mod == 4))
    return stbox_tmin_cmp(query, centroid);
  else /* (hasz && mod == 7) || (! hasz && mod == 5) */
    return stbox_tmax_cmp(query, centroid);
}

PGDLLEXPORT Datum Stbox_kdtree_choose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_kdtree_choose);
/**
 * @brief K-d tree choose function for time types
 */
Datum
Stbox_kdtree_choose(PG_FUNCTION_ARGS)
{
  spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
  spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
  STBox *query = DatumGetSTboxP(in->leafDatum), *centroid;
  assert(in->hasPrefix);
  centroid = DatumGetSTboxP(in->prefixDatum);
  assert(in->nNodes == 2);
  out->resultType = spgMatchNode;
  out->result.matchNode.nodeN =
    (stbox_level_cmp(centroid, query, in->level) < 0) ? 0 : 1;
  out->result.matchNode.levelAdd = 1;
  out->result.matchNode.restDatum = STboxPGetDatum(query);
  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST pick-split function
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_quadtree_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_quadtree_picksplit);
/**
 * @brief SP-GiST pick-split function for temporal points
 *
 * It splits a list of boxes into quadrants by choosing a central 8D
 * point as the median of the coordinates of the boxes.
 */
Datum
Stbox_quadtree_picksplit(PG_FUNCTION_ARGS)
{
  spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
  spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
  STBox *box = DatumGetSTboxP(in->datums[0]);
  bool hasz = MEOS_FLAGS_GET_Z(box->flags);
  STBox *centroid = palloc0(sizeof(STBox));
  centroid->srid = box->srid;
  centroid->flags = box->flags;
  int  median, i;
  double *lowXs = palloc(sizeof(double) * in->nTuples);
  double *highXs = palloc(sizeof(double) * in->nTuples);
  double *lowYs = palloc(sizeof(double) * in->nTuples);
  double *highYs = palloc(sizeof(double) * in->nTuples);
  double *lowZs = NULL, *highZs = NULL; /* make compiler quiet */
  if (hasz)
  {
    lowZs = palloc(sizeof(double) * in->nTuples);
    highZs = palloc(sizeof(double) * in->nTuples);
  }
  double *lowTs = palloc(sizeof(double) * in->nTuples);
  double *highTs = palloc(sizeof(double) * in->nTuples);

  /* Calculate median of all 8D coordinates */
  for (i = 0; i < in->nTuples; i++)
  {
    box = DatumGetSTboxP(in->datums[i]);
    lowXs[i] = box->xmin;
    highXs[i] = box->xmax;
    lowYs[i] = box->ymin;
    highYs[i] = box->ymax;
    if (hasz)
    {
      lowZs[i] = box->zmin;
      highZs[i] = box->zmax;
    }
    lowTs[i] = (double) DatumGetTimestampTz(box->period.lower);
    highTs[i] = (double) DatumGetTimestampTz(box->period.upper);
  }

  qsort(lowXs, (size_t) in->nTuples, sizeof(double), compareDoubles);
  qsort(highXs, (size_t) in->nTuples, sizeof(double), compareDoubles);
  qsort(lowYs, (size_t) in->nTuples, sizeof(double), compareDoubles);
  qsort(highYs, (size_t) in->nTuples, sizeof(double), compareDoubles);
  if (hasz)
  {
    qsort(lowZs, (size_t) in->nTuples, sizeof(double), compareDoubles);
    qsort(highZs, (size_t) in->nTuples, sizeof(double), compareDoubles);
  }
  qsort(lowTs, (size_t) in->nTuples, sizeof(double), compareDoubles);
  qsort(highTs, (size_t) in->nTuples, sizeof(double), compareDoubles);

  median = in->nTuples / 2;

  centroid->xmin = lowXs[median];
  centroid->xmax = highXs[median];
  centroid->ymin = lowYs[median];
  centroid->ymax = highYs[median];
  if (hasz)
  {
    centroid->zmin = lowZs[median];
    centroid->zmax = highZs[median];
  }
  centroid->period.lower = TimestampTzGetDatum((TimestampTz) lowTs[median]);
  centroid->period.upper = TimestampTzGetDatum((TimestampTz) highTs[median]);

  /* Fill the output */
  out->hasPrefix = true;
  out->prefixDatum = STboxPGetDatum(centroid);
  out->nNodes = hasz ? 256 : 128;
  out->nodeLabels = NULL;    /* We don't need node labels. */
  out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
  out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);

  /*
   * Assign ranges to corresponding nodes according to quadrants relative to
   * the "centroid" range
   */
  for (i = 0; i < in->nTuples; i++)
  {
    box = DatumGetSTboxP(in->datums[i]);
    uint8 quadrant = getOctant8D(centroid, box);
    out->leafTupleDatums[i] = STboxPGetDatum(box);
    out->mapTuplesToNodes[i] = quadrant;
  }

  pfree(lowXs); pfree(highXs);
  pfree(lowYs); pfree(highYs);
  if (hasz)
  {
    pfree(lowZs); pfree(highZs);
  }
  pfree(lowTs); pfree(highTs);

  PG_RETURN_VOID();
}

/*****************************************************************************/

PGDLLEXPORT Datum Stbox_kdtree_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_kdtree_picksplit);
/**
 * @brief K-d tree pick-split function for spatiotemporal boxes
 */
Datum
Stbox_kdtree_picksplit(PG_FUNCTION_ARGS)
{
  spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
  spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
  int i;

  /* Sort the boxes and determine the centroid */
  SortedSTbox *sorted = palloc(sizeof(SortedSTbox) * in->nTuples);
  for (i = 0; i < in->nTuples; i++)
  {
    memcpy(&sorted[i].box, DatumGetSTboxP(in->datums[i]), sizeof(STBox));
    sorted[i].i = i;
  }
  bool hasz = MEOS_FLAGS_GET_Z(sorted[0].box.flags);
  int mod = hasz ? in->level % 8 : in->level % 6;
  qsort_comparator qsortfn;
  if (mod == 0)
    qsortfn = (qsort_comparator) &stbox_xmin_cmp;
  else if (mod == 1)
    qsortfn = (qsort_comparator) &stbox_xmax_cmp;
  else if (mod == 2)
    qsortfn = (qsort_comparator) &stbox_ymin_cmp;
  else if (mod == 3)
    qsortfn = (qsort_comparator) &stbox_ymax_cmp;
  else if (hasz && mod == 4)
    qsortfn = (qsort_comparator) &stbox_zmin_cmp;
  else if (hasz && mod == 5)
    qsortfn = (qsort_comparator) &stbox_zmax_cmp;
  else if ((hasz && mod == 6) || (! hasz && mod == 4))
    qsortfn = (qsort_comparator) &stbox_tmin_cmp;
  else /* (hasz && mod == 7) || (! hasz && mod == 5) */
    qsortfn = (qsort_comparator) &stbox_tmax_cmp;
  qsort(sorted, in->nTuples, sizeof(SortedSTbox), qsortfn);
  int median = in->nTuples >> 1;
  STBox *centroid = stbox_copy(&sorted[median].box);

  /* Fill the output data structure */
  out->hasPrefix = true;
  out->prefixDatum = STboxPGetDatum(centroid);
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
    STBox *box = stbox_copy(&sorted[i].box);
    int n = sorted[i].i;
    out->mapTuplesToNodes[n] = (i < median) ? 0 : 1;
    out->leafTupleDatums[n] = STboxPGetDatum(box);
  }
  pfree(sorted);
  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST inner consistent functions
 *****************************************************************************/

Datum
stbox_spgist_inner_consistent(FunctionCallInfo fcinfo, SPGistIndexType idxtype)
{
  spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
  spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
  int i;
  uint16 node;
  MemoryContext old_ctx;
  STBox *centroid, *queries = NULL, *orderbys = NULL; /* make compiler quiet */
  STboxNode *nodebox, infbox, next_nodebox;

  /* Fetch the centroid of this node. */
  assert(in->hasPrefix);
  centroid = DatumGetSTboxP(in->prefixDatum);

  /*
   * We are saving the traversal value or initialize it an unbounded one, if
   * we have just begun to walk the tree.
   */
  if (in->traversalValue)
    nodebox = in->traversalValue;
  else
  {
    stboxnode_init(centroid, &infbox);
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
    orderbys = palloc0(sizeof(STBox) * in->norderbys);
    for (i = 0; i < in->norderbys; i++)
      /* If the argument is an empty geometry the following call will do nothing */
      tpoint_spgist_get_stbox(&in->orderbys[i], &orderbys[i]);
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
          out->traversalValues[i] = stboxnode_copy(nodebox);
          MemoryContextSwitchTo(old_ctx);

          /* Compute the distances */
          double *distances = palloc0(sizeof(double) * in->norderbys);
          out->distances[i] = distances;
          for (int j = 0; j < in->norderbys; j++)
            distances[j] = distance_stbox_nodebox(&orderbys[i], nodebox);

          pfree(orderbys);
        }
      }

      PG_RETURN_VOID();
    }
    else
      elog(ERROR, "allTheSame should not occur for k-d trees");
  }

  /*
   * Transform the queries into bounding boxes initializing the dimensions
   * that must not be taken into account for the operators to infinity.
   * This transformation is done here to avoid doing it for all nodes
   * in the loop below.
   */
  if (in->nkeys > 0)
  {
    queries = palloc0(sizeof(STBox) * in->nkeys);
    for (i = 0; i < in->nkeys; i++)
      /* If the argument is an empty geometry the following call will do nothing */
      tpoint_spgist_get_stbox(&in->scankeys[i], &queries[i]);
  }

  /* Allocate enough memory for nodes */
  out->nNodes = 0;
  out->nodeNumbers = palloc(sizeof(int) * in->nNodes);
  out->traversalValues = palloc(sizeof(void *) * in->nNodes);
  if (in->norderbys > 0)
    out->distances = palloc(sizeof(double *) * in->nNodes);

  /*
   * Switch memory context to allocate memory for new traversal values
   * (next_nodebox) and pass these pieces of memory to further calls of
   * this function.
   */
  old_ctx = MemoryContextSwitchTo(in->traversalMemoryContext);

  /* Loop for each child */
  for (node = 0; node < in->nNodes; node++)
  {
    /* Compute the bounding box of the child */
    if (idxtype == SPGIST_QUADTREE)
      stboxnode_quadtree_next(nodebox, centroid, (uint8) node, &next_nodebox);
    else
      stboxnode_kdtree_next(nodebox, centroid, (uint8) node, (in->level) + 1,
        &next_nodebox);
    bool flag = true;
    for (i = 0; i < in->nkeys; i++)
    {
      StrategyNumber strategy = in->scankeys[i].sk_strategy;
      switch (strategy)
      {
        case RTOverlapStrategyNumber:
        case RTContainedByStrategyNumber:
        case RTAdjacentStrategyNumber:
          flag = (idxtype == SPGIST_QUADTREE) ?
            overlap8D(&next_nodebox, &queries[i]) :
            overlapKD(&next_nodebox, &queries[i], in->level);
          break;
        case RTContainsStrategyNumber:
        case RTSameStrategyNumber:
          flag = (idxtype == SPGIST_QUADTREE) ?
            contain8D(&next_nodebox, &queries[i]) :
            containKD(&next_nodebox, &queries[i], in->level);
          break;
        case RTLeftStrategyNumber:
          flag = ! overRight8D(&next_nodebox, &queries[i]);
          break;
        case RTOverLeftStrategyNumber:
          flag = ! right8D(&next_nodebox, &queries[i]);
          break;
        case RTRightStrategyNumber:
          flag = ! overLeft8D(&next_nodebox, &queries[i]);
          break;
        case RTOverRightStrategyNumber:
          flag = ! left8D(&next_nodebox, &queries[i]);
          break;
        case RTFrontStrategyNumber:
          flag = ! overBack8D(&next_nodebox, &queries[i]);
          break;
        case RTOverFrontStrategyNumber:
          flag = ! back8D(&next_nodebox, &queries[i]);
          break;
        case RTBackStrategyNumber:
          flag = ! overFront8D(&next_nodebox, &queries[i]);
          break;
        case RTOverBackStrategyNumber:
          flag = ! front8D(&next_nodebox, &queries[i]);
          break;
        case RTAboveStrategyNumber:
          flag = ! overBelow8D(&next_nodebox, &queries[i]);
          break;
        case RTOverAboveStrategyNumber:
          flag = ! below8D(&next_nodebox, &queries[i]);
          break;
        case RTBelowStrategyNumber:
          flag = ! overAbove8D(&next_nodebox, &queries[i]);
          break;
        case RTOverBelowStrategyNumber:
          flag = ! above8D(&next_nodebox, &queries[i]);
          break;
        case RTAfterStrategyNumber:
          flag = ! overBefore8D(&next_nodebox, &queries[i]);
          break;
        case RTOverAfterStrategyNumber:
          flag = ! before8D(&next_nodebox, &queries[i]);
          break;
        case RTBeforeStrategyNumber:
          flag = ! overAfter8D(&next_nodebox, &queries[i]);
          break;
        case RTOverBeforeStrategyNumber:
          flag = ! after8D(&next_nodebox, &queries[i]);
          break;
        default:
          elog(ERROR, "unrecognized strategy: %d", strategy);
      }
      /* If any check is failed, we have found our answer. */
      if (!flag)
        break;
    }

    if (flag)
    {
      /* Pass traversalValue and node */
      out->traversalValues[out->nNodes] = stboxnode_copy(&next_nodebox);
      out->nodeNumbers[out->nNodes] = node;
      /* Pass distances */
      if (in->norderbys > 0)
      {
        double *distances = palloc(sizeof(double) * in->norderbys);
        out->distances[out->nNodes] = distances;
        for (int j = 0; j < in->norderbys; j++)
          distances[j] = distance_stbox_nodebox(&orderbys[j], &next_nodebox);
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

PGDLLEXPORT Datum Stbox_quadtree_inner_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_quadtree_inner_consistent);
/**
 * @brief Quad-tree inner consistent function for temporal numbers
 */
Datum
Stbox_quadtree_inner_consistent(PG_FUNCTION_ARGS)
{
  return stbox_spgist_inner_consistent(fcinfo, SPGIST_QUADTREE);
}

PGDLLEXPORT Datum Stbox_kdtree_inner_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_kdtree_inner_consistent);
/**
 * @brief Kd-tree inner consistent function for temporal numbers
 */
Datum
Stbox_kdtree_inner_consistent(PG_FUNCTION_ARGS)
{
  return stbox_spgist_inner_consistent(fcinfo, SPGIST_KDTREE);
}

/*****************************************************************************
 * SP-GiST leaf-level consistency function
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_spgist_leaf_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_spgist_leaf_consistent);
/**
 * @brief SP-GiST leaf-level consistency function for temporal points
 */
Datum
Stbox_spgist_leaf_consistent(PG_FUNCTION_ARGS)
{
  spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
  spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
  STBox *key = DatumGetSTboxP(in->leafDatum), box;
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
    out->recheck |= tpoint_index_recheck(strategy);

    if (tpoint_spgist_get_stbox(&in->scankeys[i], &box))
      result = stbox_index_consistent_leaf(key, &box, strategy);
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
      /* Cast the order by argument to a box and perform the test */
      if (tpoint_spgist_get_stbox(&in->orderbys[i], &box))
        distances[i] = nad_stbox_stbox(&box, key);
      else
        /* If empty geometry */
        distances[i] = DBL_MAX;
    }
  }

  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * SP-GiST compress functions
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_spgist_compress(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_spgist_compress);
/**
 * @brief SP-GiST compress functions for temporal points
 */
Datum
Tpoint_spgist_compress(PG_FUNCTION_ARGS)
{
  Datum tempdatum = PG_GETARG_DATUM(0);
  STBox *result = palloc(sizeof(STBox));
  temporal_bbox_slice(tempdatum, result);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************/
