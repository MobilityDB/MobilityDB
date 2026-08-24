/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief In-memory RTree index for MEOS bounding boxes, i.e., for Span, TBox,
 * and STBox
 */

/* C */
#include <stdlib.h>
#include <limits.h>
#include <math.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#if POINTCLOUD
  #include <meos_pointcloud.h>
  #include "pointcloud/tpc_boxops.h"
#endif
#include "temporal/span.h"
#include "temporal/tbox.h"
#include "temporal/temporal.h"
#include "temporal/type_util.h"
#include "geo/geo_funcs.h"
#include "temporal/temporal_rtree.h"

/*****************************************************************************
 * Functions passed as parameters in the creation of an RTree
 *****************************************************************************/

/**
 * @brief Return the lower or upper bound from a span as a double
 * @param[in] box Span
 * @param[in] axis Axis to retrieve, it is always 0 for spans since there is
 * only one dimension
 * @param[in] upper True when retrieving the upper bound, False for the lower
 * bound
 */
static double
get_axis_span(const void *box, int axis UNUSED, bool upper)
{
  assert(box);
  Span *span = (Span *) box;
  return upper ? (double) span->upper : (double) span->lower;
}

/**
 * @brief Return the lower or upper bound of a given axis from a temporal box
 * as a double
 * @details The function supports the X value axis and the temporal axis
 * @param[in] box Temporal box from which the axis value is to be retrieved
 * @param[in] axis Axis to retrieve (0 = X, 1 = time)
 * @param[in] upper True when retrieving the upper bound, False for the lower
 * bound
 */
static double
get_axis_tbox(const void *box, int axis, bool upper)
{
  assert(box); assert(axis == 0 || axis == 1);
  TBox *tbox = (TBox *) box;
  if (axis == 0)
    return upper ? (double) tbox->span.upper : (double) tbox->span.lower;
  else /* axis == 1 */
    return upper ? (double)((int64) tbox->period.upper) :
      (double)((int64) tbox->period.lower);
}

/**
 * @brief Return the lower or upper bound of a given axis from a
 * spatiotemporal box as a double
 * @details The function supports the X, Y, and Z spatial axes and the temporal
 * axis
 * @param[in] box Spatiotemporal box
 * @param[in] axis The axis to retrieve (0 = X, 1 = Y, 2 = time, 3 = Z)
 * @param[in] upper True when retrieving the upper bound, False for the lower
 * bound
 */
static double
get_axis_stbox(const void *box, int axis, bool upper)
{
  assert(box); assert(axis >= 0 && axis <= 3);
  STBox *stbox = (STBox *) box;
  if (axis == 0)
    return upper ? stbox->xmax : stbox->xmin;
  else if (axis == 1)
    return upper ? stbox->ymax : stbox->ymin;
  else if (axis == 2)
    return upper ? (double)((int64) stbox->period.upper) :
      (double)((int64) stbox->period.lower);
  else /* axis == 3 */
    return upper ? stbox->zmax : stbox->zmin;
}

#if POINTCLOUD
/**
 * @brief Return the lower or upper bound of a given axis from a TPCBox
 *   as a double.
 * @details Same axis convention as @ref get_axis_stbox (TPCBox shares
 *   the STBox prefix layout) — the cast is binary-compatible.
 */
static double
get_axis_tpcbox(const void *box, int axis, bool upper)
{
  /* TPCBox struct is binary-compatible with STBox in the prefix
   * (Span period + xyz min/max + srid + flags); axis dispatch is
   * identical. */
  return get_axis_stbox(box, axis, upper);
}
#endif

/*****************************************************************************/

/**
 * @brief Expand the second span with the first one
 * @param[in] box1,box2 Spans
 */
static inline void
bbox_expand_span(const void *box1, void *box2)
{
  return span_expand((Span *) box1, (Span *) box2);
}

/**
 * @brief Expand the second temporal box with the first one
 * @param[in] box1,box2 Temporal boxes
 */
static inline void
bbox_expand_tbox(const void *box1, void *box2)
{
  return tbox_expand((TBox *) box1, (TBox *) box2);
}

/**
 * @brief Expand the second spatiotemporal box with the first one
 * @param[in] box1,box2 Spatiotemporal boxes
 */
static inline void
bbox_expand_stbox(const void *box1, void *box2)
{
  return stbox_expand((STBox *) box1, (STBox *) box2);
}

#if POINTCLOUD
/**
 * @brief Expand the second TPCBox with the first one.
 */
static inline void
bbox_expand_tpcbox(const void *box1, void *box2)
{
  return tpcbox_expand((TPCBox *) box1, (TPCBox *) box2);
}
#endif

/*****************************************************************************/

/**
 * @brief Return `true` if the first span contains the second one, `false`
 * otherwise
 * @param[in] box1,box2 Spans
 */
static inline bool
bbox_contains_span(const void *box1, const void *box2)
{
  return contains_span_span((Span *) box1, (Span *) box2);
}

/**
 * @brief Return `true` if the first temporal box contains the second one,
 * `false` otherwise
 * @param[in] box1,box2 Temporal boxes
 */
static inline bool
bbox_contains_tbox(const void *box1, const void *box2)
{
  return contains_tbox_tbox((TBox *) box1, (TBox *) box2);
}

/**
 * @brief Return `true` if the first spatiotemporal box contains the second
 * one, `false` otherwise.
 * @param[in] box1,box2 Spatiotemporal boxes
 */
static inline bool
bbox_contains_stbox(const void *box1, const void *box2)
{
  return contains_stbox_stbox((STBox *) box1, (STBox *) box2);
}

#if POINTCLOUD
/**
 * @brief Return `true` if the first TPCBox contains the second.
 */
static inline bool
bbox_contains_tpcbox(const void *box1, const void *box2)
{
  return contains_tpcbox_tpcbox((TPCBox *) box1, (TPCBox *) box2);
}
#endif

/*****************************************************************************/

/**
 * @brief Return `true` if the two spans overlap, `false` otherwise
 * @details An R-tree stores homogeneous boxes -- same span type by
 * construction -- so the validation of #overlaps_span_span is unnecessary
 * here and the bounds are compared directly. This callback also serves as the
 * span-overlap building block of the temporal-box and TPCBox overlap tests
 * @param[in] box1,box2 Spans
 */
static inline bool
bbox_overlaps_span(const void *box1, const void *box2)
{
  const Span *s1 = (const Span *) box1;
  const Span *s2 = (const Span *) box2;
  int cmp1 = datum_cmp(s1->lower, s2->upper, s1->basetype);
  int cmp2 = datum_cmp(s2->lower, s1->upper, s1->basetype);
  return
    (cmp1 < 0 || (cmp1 == 0 && s1->lower_inc && s2->upper_inc)) &&
    (cmp2 < 0 || (cmp2 == 0 && s2->lower_inc && s1->upper_inc));
}

/**
 * @brief Return `true` if the two boxes share a boundary, `false` otherwise
 * @details Each delegates to the predicate of its box type, adjacency holding
 * across every dimension the two boxes share and being a different relation
 * from overlap: two boxes meeting at an excluded bound share a boundary and no
 * point, and two boxes meeting at an included one share a point and are not
 * adjacent.
 */
static bool
bbox_adjacent_span(const void *box1, const void *box2)
{
  return adjacent_span_span((const Span *) box1, (const Span *) box2);
}

static bool
bbox_adjacent_tbox(const void *box1, const void *box2)
{
  return adjacent_tbox_tbox((const TBox *) box1, (const TBox *) box2);
}

static bool
bbox_adjacent_stbox(const void *box1, const void *box2)
{
  return adjacent_stbox_stbox((const STBox *) box1, (const STBox *) box2);
}

#if POINTCLOUD
static bool
bbox_adjacent_tpcbox(const void *box1, const void *box2)
{
  return adjacent_tpcbox_tpcbox((const TPCBox *) box1, (const TPCBox *) box2);
}
#endif /* POINTCLOUD */

/**
 * @brief Return `true` if the two temporal boxes overlap, `false` otherwise
 * @details The homogeneous boxes stored in an R-tree share the same value span
 * type by construction, so the dimension and type validation of
 * #overlaps_tbox_tbox is unnecessary here and the present axes are compared
 * directly
 * @param[in] box1,box2 Temporal boxes
 */
static inline bool
bbox_overlaps_tbox(const void *box1, const void *box2)
{
  const TBox *b1 = (const TBox *) box1;
  const TBox *b2 = (const TBox *) box2;
  if (MEOS_FLAGS_GET_X(b1->flags) && MEOS_FLAGS_GET_X(b2->flags) &&
      ! bbox_overlaps_span(&b1->span, &b2->span))
    return false;
  if (MEOS_FLAGS_GET_T(b1->flags) && MEOS_FLAGS_GET_T(b2->flags) &&
      ! bbox_overlaps_span(&b1->period, &b2->period))
    return false;
  return true;
}

/**
 * @brief Return `true` if the two spatiotemporal boxes overlap, `false`
 * otherwise
 * @details An R-tree stores homogeneous boxes -- same SRID, dimensionality and
 * geodetic flag by construction -- so the SRID and dimension validation of
 * #overlaps_stbox_stbox is unnecessary here and the axes the boxes carry are
 * compared directly
 * @param[in] box1,box2 Spatiotemporal boxes
 */
static inline bool
bbox_overlaps_stbox(const void *box1, const void *box2)
{
  const STBox *b1 = (const STBox *) box1;
  const STBox *b2 = (const STBox *) box2;
  if (MEOS_FLAGS_GET_X(b1->flags) && MEOS_FLAGS_GET_X(b2->flags) && (
      b1->xmax < b2->xmin || b1->xmin > b2->xmax ||
      b1->ymax < b2->ymin || b1->ymin > b2->ymax))
    return false;
  if (MEOS_FLAGS_GET_Z(b1->flags) && MEOS_FLAGS_GET_Z(b2->flags) &&
      (b1->zmax < b2->zmin || b1->zmin > b2->zmax))
    return false;
  if (MEOS_FLAGS_GET_T(b1->flags) && MEOS_FLAGS_GET_T(b2->flags) && (
      datum_lt(b1->period.upper, b2->period.lower, T_TIMESTAMPTZ) ||
      datum_gt(b1->period.lower, b2->period.upper, T_TIMESTAMPTZ)))
    return false;
  return true;
}

#if POINTCLOUD
/**
 * @brief Return `true` if two TPCBox values overlap, `false` otherwise
 * @details The homogeneous boxes stored in an R-tree share the same point
 * cloud schema by construction, so the identifier and dimension validation of
 * #overlaps_tpcbox_tpcbox is unnecessary here and the present axes are compared
 * directly
 * @param[in] box1,box2 TPCBox values
 */
static inline bool
bbox_overlaps_tpcbox(const void *box1, const void *box2)
{
  const TPCBox *b1 = (const TPCBox *) box1;
  const TPCBox *b2 = (const TPCBox *) box2;
  if (MEOS_FLAGS_GET_X(b1->flags) && MEOS_FLAGS_GET_X(b2->flags))
  {
    if (b1->xmax < b2->xmin || b2->xmax < b1->xmin) return false;
    if (b1->ymax < b2->ymin || b2->ymax < b1->ymin) return false;
    if (MEOS_FLAGS_GET_Z(b1->flags) && MEOS_FLAGS_GET_Z(b2->flags) &&
        (b1->zmax < b2->zmin || b2->zmax < b1->zmin))
      return false;
  }
  if (MEOS_FLAGS_GET_T(b1->flags) && MEOS_FLAGS_GET_T(b2->flags) &&
      ! bbox_overlaps_span(&b1->period, &b2->period))
    return false;
  return true;
}
#endif

/*****************************************************************************
 * Position of one box with respect to another
 *
 * An operation that orders a dimension is answered at a leaf by the operator
 * of the box type, and at an inner node by the same operator read the other
 * way round. A node holds every entry of its subtree, so an entry can lie on
 * one side of the query only when the node itself reaches that side: an entry
 * left of the query needs a node that is not entirely to its right, and so on
 * for each pair. The pairing is an involution, which #index_op_dual gives.
 *****************************************************************************/

/**
 * @brief Return the operation whose negation, read against a node, tells
 * whether the subtree can hold an entry satisfying @p op
 */
static IndexSearchOp
index_op_dual(IndexSearchOp op)
{
  switch (op)
  {
    case INDEX_LEFT:       return INDEX_OVERRIGHT;
    case INDEX_OVERRIGHT:  return INDEX_LEFT;
    case INDEX_OVERLEFT:   return INDEX_RIGHT;
    case INDEX_RIGHT:      return INDEX_OVERLEFT;
    case INDEX_BELOW:      return INDEX_OVERABOVE;
    case INDEX_OVERABOVE:  return INDEX_BELOW;
    case INDEX_OVERBELOW:  return INDEX_ABOVE;
    case INDEX_ABOVE:      return INDEX_OVERBELOW;
    case INDEX_FRONT:      return INDEX_OVERBACK;
    case INDEX_OVERBACK:   return INDEX_FRONT;
    case INDEX_OVERFRONT:  return INDEX_BACK;
    case INDEX_BACK:       return INDEX_OVERFRONT;
    case INDEX_BEFORE:     return INDEX_OVERAFTER;
    case INDEX_OVERAFTER:  return INDEX_BEFORE;
    case INDEX_OVERBEFORE: return INDEX_AFTER;
    case INDEX_AFTER:      return INDEX_OVERBEFORE;
    default:               return op;
  }
}

/**
 * @brief Return `true` if a span is positioned with respect to another one as
 * @p op asks, `false` otherwise
 * @details A span orders one dimension, so only the value operations apply
 * @param[in] box1,box2 Span values
 * @param[in] op Position operation
 */
static bool
bbox_position_span(const void *box1, const void *box2, IndexSearchOp op)
{
  const Span *s1 = (const Span *) box1;
  const Span *s2 = (const Span *) box2;
  switch (op)
  {
    case INDEX_LEFT:      return left_span_span(s1, s2);
    case INDEX_OVERLEFT:  return overleft_span_span(s1, s2);
    case INDEX_RIGHT:     return right_span_span(s1, s2);
    case INDEX_OVERRIGHT: return overright_span_span(s1, s2);
    default:              return false;
  }
}

/**
 * @brief Return `true` if a temporal box is positioned with respect to another
 * one as @p op asks, `false` otherwise
 * @details A temporal box orders a value dimension and a time dimension
 * @param[in] box1,box2 TBox values
 * @param[in] op Position operation
 */
static bool
bbox_position_tbox(const void *box1, const void *box2, IndexSearchOp op)
{
  const TBox *b1 = (const TBox *) box1;
  const TBox *b2 = (const TBox *) box2;
  switch (op)
  {
    case INDEX_LEFT:       return left_tbox_tbox(b1, b2);
    case INDEX_OVERLEFT:   return overleft_tbox_tbox(b1, b2);
    case INDEX_RIGHT:      return right_tbox_tbox(b1, b2);
    case INDEX_OVERRIGHT:  return overright_tbox_tbox(b1, b2);
    case INDEX_BEFORE:     return before_tbox_tbox(b1, b2);
    case INDEX_OVERBEFORE: return overbefore_tbox_tbox(b1, b2);
    case INDEX_AFTER:      return after_tbox_tbox(b1, b2);
    case INDEX_OVERAFTER:  return overafter_tbox_tbox(b1, b2);
    default:               return false;
  }
}

/**
 * @brief Return `true` if a spatiotemporal box is positioned with respect to
 * another one as @p op asks, `false` otherwise
 * @param[in] box1,box2 STBox values
 * @param[in] op Position operation
 */
static bool
bbox_position_stbox(const void *box1, const void *box2, IndexSearchOp op)
{
  const STBox *b1 = (const STBox *) box1;
  const STBox *b2 = (const STBox *) box2;
  switch (op)
  {
    case INDEX_LEFT:       return left_stbox_stbox(b1, b2);
    case INDEX_OVERLEFT:   return overleft_stbox_stbox(b1, b2);
    case INDEX_RIGHT:      return right_stbox_stbox(b1, b2);
    case INDEX_OVERRIGHT:  return overright_stbox_stbox(b1, b2);
    case INDEX_BELOW:      return below_stbox_stbox(b1, b2);
    case INDEX_OVERBELOW:  return overbelow_stbox_stbox(b1, b2);
    case INDEX_ABOVE:      return above_stbox_stbox(b1, b2);
    case INDEX_OVERABOVE:  return overabove_stbox_stbox(b1, b2);
    case INDEX_FRONT:      return front_stbox_stbox(b1, b2);
    case INDEX_OVERFRONT:  return overfront_stbox_stbox(b1, b2);
    case INDEX_BACK:       return back_stbox_stbox(b1, b2);
    case INDEX_OVERBACK:   return overback_stbox_stbox(b1, b2);
    case INDEX_BEFORE:     return before_stbox_stbox(b1, b2);
    case INDEX_OVERBEFORE: return overbefore_stbox_stbox(b1, b2);
    case INDEX_AFTER:      return after_stbox_stbox(b1, b2);
    case INDEX_OVERAFTER:  return overafter_stbox_stbox(b1, b2);
    default:               return false;
  }
}

#if POINTCLOUD
/**
 * @brief Return `true` if a TPCBox is positioned with respect to another one
 * as @p op asks, `false` otherwise
 * @details A TPCBox carries the same spatial and temporal bounds as an STBox
 * plus a trailing point cloud identifier, which orders no dimension, so the
 * position is the one the projected boxes hold
 * @param[in] box1,box2 TPCBox values
 * @param[in] op Position operation
 */
static bool
bbox_position_tpcbox(const void *box1, const void *box2, IndexSearchOp op)
{
  STBox sbox1, sbox2;
  tpcbox_set_stbox((const TPCBox *) box1, &sbox1);
  tpcbox_set_stbox((const TPCBox *) box2, &sbox2);
  return bbox_position_stbox(&sbox1, &sbox2, op);
}
#endif /* POINTCLOUD */

/*****************************************************************************
 * Rtree functions
 *****************************************************************************/

/**
 * @brief Creates a new RTree node
 * @param[in] node_type Type of the node
 * @param[in] bboxsize Bounding box size 
 * @return Pointer to the newly created node
 */
static RTreeNode *
node_make(RTreeNodeType node_type, size_t bboxsize)
{
  size_t bboxes_size = bboxsize * MAXITEMS;
  RTreeNode *node = palloc0(sizeof(RTreeNode) + bboxes_size);
  node->node_type = node_type;
  node->bboxsize = bboxsize;
  node->count = 0;
  return node;
}

/**
 * @brief Return the length of a bounding box along a given axis as a double
 * @param[in] rtree Pointer to the RTree structure containing the function to
 * retrieve axis values
 * @param[in] box Pointer to the bounding box for which the axis length is to
 * be calculated
 * @param[in] axis The given axis (e.g., 0, 1, 2) along which to compute the
 * length
 */
static inline double
get_axis_length(const RTree *rtree, const void *box, int axis)
{
  return rtree->get_axis(box, axis, true) - rtree->get_axis(box, axis, false);
}

/**
 * @brief Return the length, area, or volume of a bounding box
 * @details The function iterates over all dimensions defined in the RTree
 * multiplying the lengths of the box along each axis
 * @param[in] rtree Pointer to the RTree structure, which provides the number
 * of dimensions and the method for retrieving axis lengths
 * @param[in] box Pointer to the bounding box
 */
static double
box_area(const RTree *rtree, const void *box)
{
  double result = 1.0;
  for (int i = 0; i < rtree->dims; ++i)
    result *= get_axis_length(rtree, box, i);
  return result;
}

/**
 * @brief Return the length, area, or volume of the union of two bounding
 * boxes
 * @details The function first creates a new bounding box that is the union of
 * the two input boxes, then calculates the length, area, or volume of this
 * union box using the given RTree's dimensions
 * @param[in] box1,box2 Pointers to the bounding boxes
 * @param[in] rtree Pointer to the RTree structure, which provides the number
 * of dimensions and the method for the computation
 */
static double
unioned_area(const RTree *rtree, const void *box1, const void *box2)
{
  /* Use a stack buffer large enough for any MEOS bounding box type */
  char union_buf[sizeof(STBox)];
  memcpy(union_buf, box1, rtree->bboxsize);
  rtree->bbox_expand(box2, union_buf);
  return box_area(rtree, union_buf);
}

/**
 * @brief Return the child node that requires the least enlargement to
 * accommodate a new bounding box
 * @details The function iterates through the child nodes of a given node to
 * determine which child node's bounding box would need the least enlargement
 * to include a new bounding box. The function calculates the area of each
 * node's bounding box before and after union with the new bounding box and
 * selects the one with the smallest area increase.
 * @param[in] node Pointer to the node containing the child nodes
 * @param[in] box Pointer to the bounding box that is being inserted
 * or considered
 * @param[in] rtree Pointer to the RTree structure, which provides the method
 * for calculating areas
 * @return The index of the child node that requires the least enlargement
 */
static int
node_choose_least_enlargement(const RTree *rtree, const RTreeNode *node,
  const void *box)
{
  int result = 0;
  double previous_enlargement = INFINITY;
  for (int i = 0; i < node->count; ++i)
  {
    double union_area = unioned_area(rtree, RTREE_NODE_BBOX_N(node, i), box);
    double area = box_area(rtree, RTREE_NODE_BBOX_N(node, i));
    double enlarge_area = union_area - area;
    if (enlarge_area < previous_enlargement)
    {
      result = i;
      previous_enlargement = enlarge_area;
    }
  }
  return result;
}

/**
 * @brief Returns the best child node for inserting a new bounding box in an
 * RTree
 * @details The function determines the most suitable child node within a node
 * for inserting a new bounding box. It first checks if the new box can be
 * added to any child node without requiring the expansion of its bounding box.
 * If none of the child nodes can accommodate the new bounding box without
 * expansion, the function falls back to selecting the node that requires the
 * least enlargement.
 * @param[in] rtree Pointer to the RTree structure, providing access to the
 * overall RTree configuration
 * @param[in] box Pointer to the bounding box that is being inserted
 * @param[in] node Pointer to the node containing the child nodes
 * @return The index of the chosen child node for insertion
 */
static int
node_choose(const RTree *rtree, const void *box, const RTreeNode *node)
{
  /* Check if the bounding box can be added without expanding any rectangle */
  for (int i = 0; i < node->count; ++i)
  {
    if (rtree->bbox_contains(RTREE_NODE_BBOX_N(node, i), box))
      return i;
  }
  /* Fallback to "least enlargement" */
  return node_choose_least_enlargement(rtree, node, box);
}

/**
 * @brief Return the bounding box that encloses all bounding boxes in an RTree
 * node
 * @details The function takes the destination bounding box and change it to the
 * minimal bounding box enclosing all bounding boxes within a given RTree node
 * @param[in] rtree Pointer to the RTree structure
 * @param[in] node Pointer to the node containing the bounding boxes
 * @param[out] box bounding box that will be expanded
 */
static void
node_box_calculate(const RTree *rtree, const RTreeNode *node, void *box)
{
  memcpy(box, RTREE_NODE_BBOX_N(node, 0), rtree->bboxsize);
  for (int i = 1; i < node->count; ++i)
    rtree->bbox_expand(RTREE_NODE_BBOX_N(node, i), box);
  return;
}

/**
 * @brief Return the axis with the largest length in a bounding box
 * @details The function determines the axis of a bounding box that has the
 * largest length by comparing the lengths of the bounding box along each
 * dimension defined in the RTree
 * @param[in] box Pointer to the bounding box whose largest axis is to be
 * determined.
 * @param[in] rtree Pointer to the RTree structure, which provides the method
 * for calculating axis lengths and defines the number of dimensions.
 * @return The index of the axis with the largest length.
 */
static int
box_largest_axis(const RTree *rtree, const void *box)
{
  int largest_axis = 0;
  double previous_largest = get_axis_length(rtree, box, 0);
  for (int i = 1; i < rtree->dims; ++i)
  {
    if (previous_largest < get_axis_length(rtree, box, i))
    {
      previous_largest = get_axis_length(rtree, box, i);
      largest_axis = i;
    }
  }
  return largest_axis;
}

/**
 * @brief Moves a bounding box from one RTree node to another.
 * @details Changes the information from one node into another.
 * @param[in] from Pointer to the node from which the bounding box is
 * being moved.
 * @param[in] index The index of the bounding box in the `from` node that is to
 * be moved.
 * @param[in] into Pointer to the node where the bounding box is being moved
 * to.
 */
static void
node_move_box_at_index_into(RTreeNode *from, int index, RTreeNode *into)
{
  memcpy(RTREE_NODE_BBOX_N(into, into->count), RTREE_NODE_BBOX_N(from, index),
    from->bboxsize);
  memcpy(RTREE_NODE_BBOX_N(from, index),
    RTREE_NODE_BBOX_N(from, from->count - 1), from->bboxsize);
  if (from->node_type == RTREE_LEAF)
  {
    into->ids[into->count] = from->ids[index];
    from->ids[index] = from->ids[from->count - 1];
  }
  else
  {
    into->nodes[into->count] = from->nodes[index];
    from->nodes[index] = from->nodes[from->count - 1];
  }
  from->count--;
  into->count++;
  return;
}

/**
 * @brief Swaps two bounding boxes and their associated data within an RTree node.
 * @details The function exchanges the positions of two bounding boxes within a
 * single RTree node. If the node is a leaf, it also swaps the associated IDs.
 * For internal nodes, it swaps the pointers to child nodes. The function is
 * useful for reordering elements within a node.
 * @param[in] rtree Pointer to the RTree structure
 * @param[in,out] node Pointer to the node containing the
 * bounding boxes and associated data.
 * @param[in] i The index of the first bounding box to be swapped.
 * @param[in] j The index of the second bounding box to be swapped.
 */
static void
node_swap(const RTree *rtree, RTreeNode *node, int i, int j)
{
  /* Use a stack buffer large enough for any MEOS bounding box type */
  char buf[sizeof(STBox)];
  memcpy(buf, RTREE_NODE_BBOX_N(node, i), rtree->bboxsize);
  memcpy(RTREE_NODE_BBOX_N(node, i), RTREE_NODE_BBOX_N(node, j),
    rtree->bboxsize);
  memcpy(RTREE_NODE_BBOX_N(node, j), buf, rtree->bboxsize);
  if (node->node_type == RTREE_LEAF)
  {
    int64 tmp = node->ids[i];
    node->ids[i] = node->ids[j];
    node->ids[j] = tmp;
  }
  else
  {
    RTreeNode *tree = node->nodes[i];
    node->nodes[i] = node->nodes[j];
    node->nodes[j] = tree;
  }
  return;
}

/**
 * @brief Sort the bounding boxes within an RTree node using the QuickSort
 * algorithm
 * @details The function recursively sorts the bounding boxes within a given
 * range in an RTree node along a particular axis. It uses the QuickSort
 * algorithm to order the bounding boxes based on their axis values, either
 * upper or lower, as provided by the `get_axis` function in the RTree
 * structure.
 * @param[in] rtree Pointer to the RTree structure which provides the function
 * for retrieving axis values
 * @param[in,out] node Pointer to the node containing the bounding boxes to be
 * sorted
 * @param[in] index The axis index along which to sort the bounding boxes
 * @param[in] upper A Boolean indicating whether to sort by upper or lower
 * axis value
 * @param[in] s The starting index of the range to be sorted in the
 * `node->boxes` array
 * @param[in] e The ending index (exclusive) of the range to be sorted in the
 * `node->boxes` array
 */
static void
node_qsort(const RTree *rtree, RTreeNode *node, int index, bool upper, int s,
  int e)
{
  int num_boxes = e - s;
  if (num_boxes < 2)
    return;

  int left = 0;
  int right = num_boxes - 1;
  int pivot = num_boxes / 2;
  node_swap(rtree, node, s + pivot, s + right);
  for (int i = 0; i < num_boxes; ++i)
  {
    if (rtree->get_axis(RTREE_NODE_BBOX_N(node, right + s), index, upper) >
        rtree->get_axis(RTREE_NODE_BBOX_N(node, s + i), index, upper))
    {
      node_swap(rtree, node, s + i, s + left);
      left++;
    }
  }
  node_swap(rtree, node, s + left, s + right);
  node_qsort(rtree, node, index, upper, s, s + left);
  node_qsort(rtree, node, index, upper, s + left + 1, e);
  return;
}

/**
 * @brief Sort the bounding boxes in an RTree node along a given axis using
 * QuickSort
 * @param[in] rtree Pointer to the RTree structure, which provides the function
 * for retrieving axis values
 * @param[in,out] node Pointer to the node containing the bounding boxes to be
 * sorted
 * @param[in] index The axis index along which to sort the bounding boxes
 * @param[in] upper A boolean that indicates whether to sort by the upper or
 * lower axis value
 */
static void
node_sort_axis(const RTree *rtree, RTreeNode *node, int index, bool upper)
{
  node_qsort(rtree, node, index, upper, 0, node->count);
  return;
}

/**
 * @brief Splits an RTree node and redistributes its bounding boxes between two
 * nodes
 * @details The function splits an RTree node into two nodes by distributing
 * the bounding boxes based on the axis with the largest length. The bounding
 * boxes are moved to either the original node or a new right node, depending
 * on their position relative to the splitting axis. After the initial split,
 * the function ensures that both nodes have at least a minimum number of
 * bounding boxes by redistributing the bounding boxes if necessary. If the
 * node is a branch node, it also sorts both nodes by the first axis.
 * @param[in] rtree Pointer to the RTree structure, which provides methods for
 * retrieving axis values and determining dimensions
 * @param[in] node Pointer to the node to be split
 * @param[in] box Pointer to the bounding box used to guide the split
 * @param[out] right_out Pointer to a pointer where the new RTreeNode (right
 * node) will be stored
 */
static void
node_split(RTree *rtree, RTreeNode *node, void *box, RTreeNode **right_out)
{
  /* Split through the largest axis */
  int largest_axis = box_largest_axis(rtree, box);
  RTreeNode *right = node_make(node->node_type, rtree->bboxsize);
  for (int i = 0; i < node->count; ++i)
  {
    double min_dist =
      rtree->get_axis(RTREE_NODE_BBOX_N(node, i), largest_axis, false) -
      rtree->get_axis(box, largest_axis, false);
    double max_dist =
      rtree->get_axis(box, largest_axis, true) -
      rtree->get_axis(RTREE_NODE_BBOX_N(node, i), largest_axis, true);
    /* Move to the right */
    if (max_dist < min_dist)
      node_move_box_at_index_into(node, i--, right);
  }

  /* Make sure that both left and right nodes have at least MINITEMS by moving
   * data into underflowed nodes */
  if (node->count < MINITEMS)
  {
    /* Reverse sort by min axis */
    node_sort_axis(rtree, right, largest_axis, false);
    do
    {
      node_move_box_at_index_into(right, right->count - 1, node);
    } while (node->count < MINITEMS);
  }
  else if (right->count < MINITEMS)
  {
    /* Reverse sort by max axis */
    node_sort_axis(rtree, node, largest_axis, true);
    do
    {
      node_move_box_at_index_into(node, node->count - 1, right);
    } while (right->count < MINITEMS);
  }
  if (node->node_type == RTREE_INNER)
  {
    node_sort_axis(rtree, node, 0, false);
    node_sort_axis(rtree, right, 0, false);
  }
  *right_out = right;
  return;
}

/**
 * @brief Inserts a new bounding box into an RTree node and handles node
 * splitting if necessary
 * @details If the node is a leaf and already contains the maximum number of
 * items (`MAXITEMS`), the function sets the `split` flag to `true` to indicate
 * that the node needs to be split. For non-leaf nodes, the function determines
 * the appropriate child node for insertion and recursively inserts the
 * bounding box. If splitting occurs, the function handles the split and
 * updates the parent node's bounding boxes.
 * @param[in] rtree Pointer to the RTree structure that provides axis value
 * retrieval and node splitting functions
 * @param[in] node_bounding_box Pointer to the bounding bounding box of all the
 * bounding boxes in `node`
 * @param[in] node Pointer to the node where the bounding box is being
 * inserted
 * @param[in] new_box Pointer to the bounding box to be inserted
 * @param[in] id Identifier associated with the new bounding box (used only for
 * leaf nodes)
 * @param[out] split Pointer to a boolean flag that indicates if the node was
 * split during insertion
 */
static void
node_insert(RTree *rtree, void *node_bounding_box, RTreeNode *node,
  void *new_box, int64 id, bool *split)
{
  if (node->node_type == RTREE_LEAF)
  {
    if (node->count == MAXITEMS)
    {
      *split = true;
      return;
    }
    int index = node->count;
    memcpy(RTREE_NODE_BBOX_N(node, index), new_box, rtree->bboxsize);
    node->ids[index] = id;
    node->count++;
    *split = false;
    return;
  }
  int insertion_node = node_choose(rtree, new_box, node);
  node_insert(rtree, RTREE_NODE_BBOX_N(node, insertion_node),
    (RTreeNode *) node->nodes[insertion_node], new_box, id, split);
  if (! *split)
  {
    rtree->bbox_expand(new_box, RTREE_NODE_BBOX_N(node, insertion_node));
    *split = false;
    return;
  }
  if (node->count == MAXITEMS)
  {
    *split = true;
    return;
  }
  RTreeNode *right;
  node_split(rtree, node->nodes[insertion_node],
    RTREE_NODE_BBOX_N(node, insertion_node), &right);
  node_box_calculate(rtree, node->nodes[insertion_node],
    RTREE_NODE_BBOX_N(node, insertion_node));
  node_box_calculate(rtree, right, RTREE_NODE_BBOX_N(node, node->count));
  node->nodes[node->count] = right;
  node->count++;
  node_insert(rtree, node_bounding_box, node, new_box, id, split);
  return;
}


/**
 * @brief Return true if a leaf entry is consistent with the search query
 * @details This is the exact predicate applied at leaf level.
 * @param[in] rtree Pointer to the RTree structure
 * @param[in] key The bounding box of the leaf entry
 * @param[in] query The bounding box that serves as query
 * @param[in] op The search operation
 */
static bool
leaf_consistent(const RTree *rtree, const void *key, const void *query,
  IndexSearchOp op)
{
  switch (op)
  {
    case INDEX_OVERLAPS:
      return rtree->bbox_overlaps(key, query);
    case INDEX_CONTAINS:
      return rtree->bbox_contains(key, query);
    case INDEX_CONTAINED_BY:
      return rtree->bbox_contains(query, key);
    case INDEX_SAME:
      /* Two extents are equal exactly when each contains the other */
      return rtree->bbox_contains(key, query) && rtree->bbox_contains(query, key);
    case INDEX_ADJACENT:
      return rtree->bbox_adjacent(key, query);
    default:
      /* An entry satisfies a position operation as the box type answers it */
      return rtree->bbox_position ?
        rtree->bbox_position(key, query, op) : false;
  }
}

/**
 * @brief Return true if an inner node is consistent with the search query
 * @details This is a looser check used for pruning subtrees during traversal.
 * @param[in] rtree Pointer to the RTree structure
 * @param[in] key The bounding box of the inner node
 * @param[in] query The bounding box that serves as query
 * @param[in] op The search operation
 */
static bool
inner_consistent(const RTree *rtree, const void *key, const void *query,
  IndexSearchOp op)
{
  switch (op)
  {
    case INDEX_OVERLAPS:
    case INDEX_CONTAINED_BY:
      return rtree->bbox_overlaps(key, query);
    case INDEX_ADJACENT:
      /* An entry meeting the query lies in a box that meets it or overlaps it,
       * the entry being contained by that box */
      return rtree->bbox_overlaps(key, query) ||
        rtree->bbox_adjacent(key, query);
    case INDEX_CONTAINS:
    case INDEX_SAME:
      /* A subtree holds an entry equal to the query only when the box bounding
       * the subtree contains the query, an entry being contained by it */
      return rtree->bbox_contains(key, query);
    default:
      /* The subtree can hold an entry on one side of the query only when the
       * node itself reaches that side, which the dual operation denies */
      return rtree->bbox_position ?
        ! rtree->bbox_position(key, query, index_op_dual(op)) : false;
  }
}

/**
 * @brief Searches recursively a node looking for hits with a query
 * @param[in] rtree Pointer to the RTree structure
 * @param[in] node The node to be searched
 * @param[in] op The search operation (overlaps, contains, or contained by)
 * @param[in] query The bounding box that serves as query
 * @param[out] result MeosArray to collect matching IDs
 */
static void
node_search(const RTree *rtree, const RTreeNode *node, IndexSearchOp op,
  const void *query, MeosArray *result)
{
  for (int i = 0; i < node->count; ++i)
  {
    if (node->node_type == RTREE_LEAF)
    {
      if (leaf_consistent(rtree, RTREE_NODE_BBOX_N(node, i), query, op))
      {
        int64 id = node->ids[i];
        meos_array_add(result, &id);
      }
    }
    else
    {
      if (inner_consistent(rtree, RTREE_NODE_BBOX_N(node, i), query, op))
        node_search(rtree, node->nodes[i], op, query, result);
    }
  }
  return;
}

/**
 * @brief Report the qualifying entry pairs of two nodes, descending both trees
 * @details A node does not store its own bounding box, so each node is visited
 * together with the box its parent holds for it; the roots are visited with a
 * null box, which prunes nothing. Only one side descends at a time when the
 * other is a leaf: iterating a leaf's boxes and recursing with that same leaf
 * would visit its entries once per box.
 *
 * Subtrees are pruned by overlap, since one entry contains or is contained by
 * another only if their boxes overlap, so a node pair whose boxes are disjoint
 * holds no qualifying pair below it. Pruning on overlap is what restricts a
 * join to the operations an overlap implies, which #ensure_index_join_op holds
 * the caller to.
 * @param[in] rtree1,rtree2 The RTrees being joined
 * @param[in] node1,node2 The nodes to be joined
 * @param[in] box1,box2 The boxes the parents hold for @p node1 and @p node2,
 * `NULL` for a root
 * @param[in] op The join operation
 * @param[out] result MeosArray collecting the two ids of each pair
 */
static void
node_join(const RTree *rtree1, const RTreeNode *node1, const void *box1,
  const RTree *rtree2, const RTreeNode *node2, const void *box2,
  IndexSearchOp op, MeosArray *result)
{
  /* A pair of subtrees holds a qualifying pair only where the two boxes
   * bounding them meet, which for every operation but adjacency is an overlap.
   * Two boxes meeting at an excluded bound share a boundary and no point, so
   * pruning that pair on overlap alone drops the pairs an adjacency join is
   * looking for -- and it drops them at the node boundaries, where nothing
   * else in the answer is missing. */
  if (box1 && box2 && ! rtree1->bbox_overlaps(box1, box2) &&
      ! (op == INDEX_ADJACENT && rtree1->bbox_adjacent(box1, box2)))
    return;

  bool leaf1 = (node1->node_type == RTREE_LEAF);
  bool leaf2 = (node2->node_type == RTREE_LEAF);
  if (leaf1 && leaf2)
  {
    for (int i = 0; i < node1->count; ++i)
    {
      const void *key = RTREE_NODE_BBOX_N(node1, i);
      for (int j = 0; j < node2->count; ++j)
      {
        if (leaf_consistent(rtree1, key, RTREE_NODE_BBOX_N(node2, j), op))
        {
          int64 id1 = node1->ids[i];
          int64 id2 = node2->ids[j];
          meos_array_add(result, &id1);
          meos_array_add(result, &id2);
        }
      }
    }
  }
  else if (! leaf1 && leaf2)
  {
    for (int i = 0; i < node1->count; ++i)
      node_join(rtree1, node1->nodes[i], RTREE_NODE_BBOX_N(node1, i), rtree2,
        node2, box2, op, result);
  }
  else if (leaf1 && ! leaf2)
  {
    for (int j = 0; j < node2->count; ++j)
      node_join(rtree1, node1, box1, rtree2, node2->nodes[j],
        RTREE_NODE_BBOX_N(node2, j), op, result);
  }
  else
  {
    for (int i = 0; i < node1->count; ++i)
      for (int j = 0; j < node2->count; ++j)
        node_join(rtree1, node1->nodes[i], RTREE_NODE_BBOX_N(node1, i), rtree2,
          node2->nodes[j], RTREE_NODE_BBOX_N(node2, j), op, result);
  }
  return;
}

/**
 * @brief Creates an RTree index.
 * @param[in] bboxtype The MeosType of the elements to index.
 * @return RTree initialized.
 */
RTree *
rtree_create(MeosType bboxtype)
{
  assert(span_type(bboxtype) || bboxtype == T_TBOX || bboxtype == T_STBOX
#if POINTCLOUD
    || bboxtype == T_TPCBOX
#endif
    );
  size_t bboxsize = bbox_get_size(bboxtype);
  RTree *rtree = palloc0(sizeof(RTree) + bboxsize);
  if (span_type(bboxtype))
  {
    rtree->dims = 1;
    rtree->get_axis = &get_axis_span;
    rtree->bbox_expand = &bbox_expand_span;
    rtree->bbox_contains = &bbox_contains_span;
    rtree->bbox_overlaps = &bbox_overlaps_span;
    rtree->bbox_adjacent = &bbox_adjacent_span;
    rtree->bbox_position = &bbox_position_span;
  }
  else if (bboxtype == T_TBOX)
  {
    rtree->dims = 2;
    rtree->get_axis = &get_axis_tbox;
    rtree->bbox_expand = &bbox_expand_tbox;
    rtree->bbox_contains = &bbox_contains_tbox;
    rtree->bbox_overlaps = &bbox_overlaps_tbox;
    rtree->bbox_adjacent = &bbox_adjacent_tbox;
    rtree->bbox_position = &bbox_position_tbox;
  }
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
  {
    /* dims set at first-insert time, like STBox (Z presence
     * determined dynamically) */
    rtree->dims = -1;
    rtree->get_axis = &get_axis_tpcbox;
    rtree->bbox_expand = &bbox_expand_tpcbox;
    rtree->bbox_contains = &bbox_contains_tpcbox;
    rtree->bbox_overlaps = &bbox_overlaps_tpcbox;
    rtree->bbox_adjacent = &bbox_adjacent_tpcbox;
    rtree->bbox_position = &bbox_position_tpcbox;
  }
#endif
  else /* bboxtype == T_STBOX */
  {
    /* To be set when the first node is created since it is not known yet
     * whether there is a Z dimension or not */
    rtree->dims = -1;
    rtree->get_axis = &get_axis_stbox;
    rtree->bbox_expand = &bbox_expand_stbox;
    rtree->bbox_contains = &bbox_contains_stbox;
    rtree->bbox_overlaps = &bbox_overlaps_stbox;
    rtree->bbox_adjacent = &bbox_adjacent_stbox;
    rtree->bbox_position = &bbox_position_stbox;
  }
  rtree->bboxtype = bboxtype;
  rtree->bboxsize = bboxsize;
  return rtree;
}

/**
 * @ingroup meos_misc
 * @brief Creates an RTree index for integer spans
 * @return RTree initialized
 */
RTree *
rtree_create_intspan()
{
  return rtree_create(T_INTSPAN);
}

/**
 * @ingroup meos_misc
 * @brief Creates an RTree index for big integer spans
 * @return RTree initialized
 */
RTree *
rtree_create_bigintspan()
{
  return rtree_create(T_BIGINTSPAN);
}

/**
 * @ingroup meos_misc
 * @brief Creates an RTree index for float spans
 * @return RTree initialized
 */
RTree *
rtree_create_floatspan()
{
  return rtree_create(T_FLOATSPAN);
}

/**
 * @ingroup meos_misc
 * @brief Creates an RTree index for temporal boxes
 * @return RTree initialized
 */
RTree *
rtree_create_datespan()
{
  return rtree_create(T_DATESPAN);
}

/**
 * @ingroup meos_misc
 * @brief Creates an RTree index for temporal boxes
 * @return RTree initialized
 */
RTree *
rtree_create_tstzspan()
{
  return rtree_create(T_TSTZSPAN);
}

/**
 * @ingroup meos_misc
 * @brief Creates an RTree index for temporal boxes
 * @return RTree initialized
 */
RTree *
rtree_create_tbox()
{
  return rtree_create(T_TBOX);
}

/**
 * @ingroup meos_misc
 * @brief Creates an RTree index for spatiotemporal boxes
 * @return RTree initialized
 */
RTree *
rtree_create_stbox()
{
  return rtree_create(T_STBOX);
}

#if POINTCLOUD
/**
 * @ingroup meos_pointcloud_box
 * @brief Create an in-memory RTree index for the @c tpcbox bounding-box
 *   type.  Pair with @ref rtree_insert / @ref rtree_insert_temporal to
 *   populate, @ref rtree_search / @ref rtree_search_temporal to query.
 * @return RTree initialized.
 */
RTree *
rtree_create_tpcbox()
{
  return rtree_create(T_TPCBOX);
}
#endif

/*****************************************************************************
 * Build from a whole entry set
 *****************************************************************************/

/* The build releases the nodes the tree holds, which the recursive release
 * below it does */
static void node_free(RTreeNode *node);

typedef struct
{
  void *box;          /**< bbox of the item (leaf: caller's; inner: owned MBR) */
  int64 id;           /**< leaf payload */
  RTreeNode *child;   /**< inner payload */
} STRItem;

typedef struct { const RTree *tree; int axis; } STRCtx;

static int
str_cmp(const void *a, const void *b, void *arg)
{
  const STRCtx *c = (const STRCtx *) arg;
  const STRItem *ia = (const STRItem *) a;
  const STRItem *ib = (const STRItem *) b;
  double ca = (c->tree->get_axis(ia->box, c->axis, false) +
               c->tree->get_axis(ia->box, c->axis, true)) / 2.0;
  double cb = (c->tree->get_axis(ib->box, c->axis, false) +
               c->tree->get_axis(ib->box, c->axis, true)) / 2.0;
  return (ca > cb) - (ca < cb);
}

/**
 * @brief Pack one level of items into nodes, Sort-Tile-Recursive
 * @details Sorts on the centre of axis 0, cuts into ceil(sqrt(pages)) slices,
 * sorts each slice on axis 1, then fills nodes to capacity. The packing order
 * affects tree QUALITY only; validity comes from each parent box being the
 * union of its children, computed here whatever the ordering.
 */
static RTreeNode **
str_pack_level(RTree *rtree, STRItem *items, int count, bool leaf, int *nout)
{
  int pages = (count + MAXITEMS - 1) / MAXITEMS;
  int slices = (int) ceil(sqrt((double) pages));
  if (slices < 1) slices = 1;
  int per_slice = slices * MAXITEMS;

  STRCtx ctx; ctx.tree = rtree; ctx.axis = 0;
  qsort_arg(items, (size_t) count, sizeof(STRItem), str_cmp, &ctx);

  RTreeNode **out = palloc(sizeof(RTreeNode *) * (size_t) pages);
  int nnodes = 0;
  for (int s = 0; s < count; s += per_slice)
  {
    int slen = (count - s < per_slice) ? count - s : per_slice;
    if (rtree->dims > 1)
    {
      ctx.axis = 1;
      qsort_arg(items + s, (size_t) slen, sizeof(STRItem), str_cmp, &ctx);
    }
    for (int p = 0; p < slen; p += MAXITEMS)
    {
      int plen = (slen - p < MAXITEMS) ? slen - p : MAXITEMS;
      RTreeNode *node = node_make(leaf ? RTREE_LEAF : RTREE_INNER, rtree->bboxsize);
      for (int k = 0; k < plen; k++)
      {
        STRItem *it = &items[s + p + k];
        memcpy(RTREE_NODE_BBOX_N(node, k), it->box, rtree->bboxsize);
        if (leaf)
          node->ids[k] = it->id;
        else
          node->nodes[k] = it->child;
      }
      node->count = plen;
      out[nnodes++] = node;
    }
  }
  *nout = nnodes;
  return out;
}

/**
 * @brief Return true if a box may enter or query an RTree, report an error
 * otherwise
 * @details A tree holds boxes of one SRID: the first entry fixes it, and a box
 * of another one is an error rather than a coercion. The check belongs here,
 * at the entry point, so that every comparison the descent makes can assume it
 */
static bool
ensure_valid_rtree_box(const RTree *rtree, const void *box)
{
  if (! rtree->root)
    return true;
  if (rtree->bboxtype == T_STBOX)
    return ensure_same_srid(((const STBox *) rtree->box)->srid,
      ((const STBox *) box)->srid);
#if POINTCLOUD
  if (rtree->bboxtype == T_TPCBOX)
  {
    STBox s1, s2;
    tpcbox_set_stbox((const TPCBox *) rtree->box, &s1);
    tpcbox_set_stbox((const TPCBox *) box, &s2);
    return ensure_same_srid(s1.srid, s2.srid);
  }
#endif /* POINTCLOUD */
  return true;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Build an RTree from all of its entries at once
 * @details Bottom-up Sort-Tile-Recursive packing. The result answers the same
 * queries as inserting every entry one by one, but the whole set is known in
 * advance, so nodes are filled to capacity and no node is ever split.
 *
 * The tree ends up holding exactly the entries given, whatever it holds on
 * entry, so this is both the fast first build and the only way to make an
 * index smaller: an index has no removal entry point, and a caller that has to
 * drop entries rebuilds from the ones it keeps. Loading no entries leaves an
 * empty tree.
 * @param[in] rtree An RTree of the appropriate bounding box type
 * @param[in] boxes Contiguous array of @p count boxes of the tree bbox size
 * @param[in] ids The id of each box
 * @param[in] count Number of entries
 */
bool
rtree_load(RTree *rtree, const void *boxes, const int64 *ids, int count)
{
  /* Ensure the validity of the arguments */
  if (! ensure_not_null((void *) rtree) || ! ensure_not_null((void *) boxes) ||
      ! ensure_not_null((void *) ids) || ! ensure_valid_rtree_box(rtree, boxes))
    return false;

  /* The packing assigns the root, so the nodes the tree holds are released
   * before it does, and are released whatever the number of entries given: an
   * empty entry set leaves an empty tree rather than the previous one */
  if (rtree->root)
  {
    node_free(rtree->root);
    rtree->root = NULL;
  }

  if (count <= 0)
    return true;

  /* A box type whose dimension count depends on the data carries -1 until the
   * first box arrives, which for a tree grown by insertion is the first insert */
  if (rtree->dims < 0)
    rtree->dims = 3 + MEOS_FLAGS_GET_Z(((const STBox *) boxes)->flags);

  STRItem *items = palloc(sizeof(STRItem) * (size_t) count);
  for (int i = 0; i < count; i++)
  {
    items[i].box = (void *) ((const char *) boxes + (size_t) i * rtree->bboxsize);
    items[i].id = ids[i];
    items[i].child = NULL;
  }

  int nnodes;
  RTreeNode **level = str_pack_level(rtree, items, count, true, &nnodes);
  pfree(items);

  while (nnodes > 1)
  {
    STRItem *up = palloc(sizeof(STRItem) * (size_t) nnodes);
    for (int i = 0; i < nnodes; i++)
    {
      void *mbr = palloc0(rtree->bboxsize);
      memcpy(mbr, RTREE_NODE_BBOX_N(level[i], 0), rtree->bboxsize);
      for (int k = 1; k < level[i]->count; k++)
        rtree->bbox_expand(RTREE_NODE_BBOX_N(level[i], k), mbr);
      up[i].box = mbr; up[i].id = 0; up[i].child = level[i];
    }
    int prev = nnodes;
    RTreeNode **parents = str_pack_level(rtree, up, prev, false, &nnodes);
    for (int i = 0; i < prev; i++)
      pfree(up[i].box);
    pfree(up); pfree(level);
    level = parents;
  }

  rtree->root = level[0];
  memcpy(&rtree->box, RTREE_NODE_BBOX_N(level[0], 0), rtree->bboxsize);
  for (int k = 1; k < level[0]->count; k++)
    rtree->bbox_expand(RTREE_NODE_BBOX_N(level[0], k), &rtree->box);
  pfree(level);
  return true;
}


/**
 * @ingroup meos_temporal_box_index
 * @brief Insert a bounding box into the RTree index.
 * @note The parameter `id` is used for the search function, when a match
 * is found the id will be returned. The bounding box will be copied into the
 * RTRee.
 * @param[in] rtree The RTree previously initialized
 * @param[in] box The bounding box to be inserted
 * @param[in] id The id of the box being inserted
 */
bool
rtree_insert(RTree *rtree, void *box, int64 id)
{
  /* Ensure the validity of the arguments */
  if (! ensure_not_null((void *) rtree) || ! ensure_not_null((void *) box) ||
      ! ensure_valid_rtree_box(rtree, box))
    return false;

  while (1)
  {
    if (! rtree->root)
    {
      RTreeNode *new_root = node_make(RTREE_LEAF, rtree->bboxsize);
      if (rtree->dims < 0)
        rtree->dims = 3 + MEOS_FLAGS_GET_Z(((STBox *) box)->flags);
      rtree->root = new_root;
      memcpy(rtree->box, box, rtree->bboxsize);
    }
    bool split = false;
    node_insert(rtree, &rtree->box, rtree->root, box, id, &split);
    if (! split)
    {
      rtree->bbox_expand(box, &rtree->box);
      return true;
    }
    RTreeNode *new_root = node_make(RTREE_INNER, rtree->bboxsize);
    RTreeNode *right;
    node_split(rtree, rtree->root, &rtree->box, &right);

    node_box_calculate(rtree, rtree->root, RTREE_NODE_BBOX_N(new_root, 0));
    node_box_calculate(rtree, right, RTREE_NODE_BBOX_N(new_root, 1));
    new_root->nodes[0] = rtree->root;
    new_root->nodes[1] = right;
    rtree->root = new_root;
    rtree->root->count = 2;
  }
  return true;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Search an RTree with a bounding box, collecting matching IDs into
 * a MeosArray
 * @details The result array is reset before the search. After the call,
 * use the returned count and #meos_array_get to read the matching IDs.
 * The same array can be reused across multiple searches without reallocating.
 * @param[in] rtree The RTree to query
 * @param[in] op The search operation: @p INDEX_OVERLAPS finds boxes that
 * overlap the query, @p INDEX_CONTAINS finds boxes that contain the query,
 * @p INDEX_CONTAINED_BY finds boxes contained by the query
 * @param[in] query The bounding box that serves as query
 * @param[out] result MeosArray of int to collect matching IDs (created by the
 * caller with `meos_array_create(sizeof(int64))`)
 * @return Number of matching IDs
 */
int
rtree_search(const RTree *rtree, IndexSearchOp op, const void *query,
  MeosArray *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rtree, INT_MAX); VALIDATE_NOT_NULL(query, INT_MAX);
  VALIDATE_NOT_NULL(result, INT_MAX);
  if (! ensure_valid_rtree_box(rtree, query))
    return INT_MAX;

  meos_array_reset(result);
  if (rtree->root)
    node_search(rtree, rtree->root, op, query, result);
  return meos_array_count(result);
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Join two RTrees, collecting the ids of every qualifying pair into a
 * MeosArray
 * @details Descends both trees at once, skipping the entries of a subtree pair
 * whose boxes are disjoint, so a join reads far fewer pairs than querying one
 * tree once per entry of the other.
 *
 * The result array is reset before the join. It receives two ids per pair, the
 * entry of @p rtree1 followed by the entry of @p rtree2, so pair `k` is read
 * with #meos_array_get at positions `2 * k` and `2 * k + 1`.
 * @param[in] rtree1,rtree2 The RTrees to join, of the same bounding box type
 * @param[in] op The join operation: @p INDEX_OVERLAPS pairs entries that
 * overlap, @p INDEX_CONTAINS pairs entries of @p rtree1 that contain an entry
 * of @p rtree2, @p INDEX_CONTAINED_BY pairs entries of @p rtree1 contained by
 * an entry of @p rtree2
 * @param[out] result MeosArray of int to collect the ids (created by the caller
 * with `meos_array_create(sizeof(int64))`)
 * @return Number of qualifying pairs, half the number of collected ids, on
 * error @p INT_MAX
 */
int
rtree_join(const RTree *rtree1, const RTree *rtree2, IndexSearchOp op,
  MeosArray *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rtree1, INT_MAX); VALIDATE_NOT_NULL(rtree2, INT_MAX);
  VALIDATE_NOT_NULL(result, INT_MAX);
  if (! ensure_same_index_bboxtype(rtree1->bboxtype, rtree2->bboxtype) ||
      ! ensure_index_join_op(op))
    return INT_MAX;

  meos_array_reset(result);
  if (rtree1->root && rtree2->root)
    node_join(rtree1, rtree1->root, NULL, rtree2, rtree2->root, NULL, op,
      result);
  return meos_array_count(result) / 2;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Insert a temporal value into the RTree index
 * @details The bounding box is automatically extracted from the temporal value.
 * The temporal type must be compatible with the RTree's bounding box type:
 * temporal alphas (tbool, ttext) require a span-based RTree, temporal numbers
 * (tint, tfloat) require a TBox-based RTree, and spatiotemporal types
 * (tgeompoint, tgeogpoint) require an STBox-based RTree.
 * @param[in] rtree The RTree previously initialized
 * @param[in] temp The temporal value to be inserted
 * @param[in] id The id of the temporal value being inserted
 */
bool
rtree_insert_temporal(RTree *rtree, const Temporal *temp, int64 id)
{
  if (! ensure_bbox_temporal_compatible(rtree->bboxtype, temp))
    return false;
  /* Use a stack buffer large enough for any MEOS bounding box type */
  bboxunion buf;
  memset(&buf, 0, sizeof(buf));
  temporal_set_bbox(temp, &buf);
  rtree_insert(rtree, &buf, id);
  return true;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Search an RTree using a temporal value's bounding box, collecting
 * matching IDs into a MeosArray
 * @details The bounding box is automatically extracted from the temporal value
 * and used as the search query. The result array is reset before the search.
 * @param[in] rtree The RTree to query
 * @param[in] op The search operation
 * @param[in] temp The temporal value whose bounding box serves as query
 * @param[out] result MeosArray of int to collect matching IDs
 * @return Number of matching IDs
 */
int
rtree_search_temporal(const RTree *rtree, IndexSearchOp op,
  const Temporal *temp, MeosArray *result)
{
  if (! ensure_bbox_temporal_compatible(rtree->bboxtype, temp))
  {
    meos_array_reset(result);
    return 0;
  }
  /* Use a stack buffer large enough for any MEOS bounding box type */
  bboxunion buf;
  memset(&buf, 0, sizeof(buf));
  temporal_set_bbox(temp, &buf);
  return rtree_search(rtree, op, &buf, result);
}

/*****************************************************************************
 * Multi-entry (MEST) temporal functions
 *
 * These functions implement a multi-entry-per-id indexing pattern: a single
 * temporal value is decomposed into several tight per-segment bounding boxes
 * that are all inserted under the same id. The tree, split and search
 * algorithms are unchanged; only the build-side decomposition and a
 * search-time deduplication of repeated ids are added. They are strictly
 * additive: #rtree_insert, #rtree_insert_temporal, #rtree_search and
 * #rtree_search_temporal keep their exact semantics.
 *****************************************************************************/

/**
 * @ingroup meos_temporal_box_index
 * @brief Insert a temporal value into the RTree index as several tight
 * per-segment bounding boxes sharing the same id (multi-entry indexing)
 * @details The temporal value is decomposed into at most `maxboxes` tight
 * per-segment bounding boxes, all inserted under `id`. This yields a more
 * selective index than #rtree_insert_temporal for wiggly or high-extent
 * temporal values, at the cost of more leaf entries. The temporal type must
 * be compatible with the RTree's bounding box type: temporal alphas (tbool,
 * ttext) require a span-based RTree, temporal numbers (tint, tfloat) require a
 * TBox-based RTree, and spatiotemporal types (tgeompoint, tgeogpoint) require
 * an STBox-based RTree. When `maxboxes <= 1`, the temporal value is an
 * instant, or the spatial type has no per-segment STBox decomposition, the
 * behaviour is identical to #rtree_insert_temporal (a single minimum bounding
 * box). The tree, split and insertion algorithms are unchanged; this only
 * inserts several boxes under the same id, which the parallel ids/boxes leaf
 * layout already supports. Search results may contain the same id several
 * times; use #rtree_search_temporal_dedup to collapse them.
 * @param[in] rtree The RTree previously initialized
 * @param[in] temp The temporal value to be inserted
 * @param[in] id The id of the temporal value being inserted, shared by every
 * box produced for `temp`
 * @param[in] maxboxes Maximum number of bounding boxes produced for `temp`;
 * values `<= 1` degenerate to the single minimum bounding box
 * @see rtree_insert_temporal
 */
bool
rtree_insert_temporal_split(RTree *rtree, const Temporal *temp, int64 id,
  int maxboxes)
{
  if (! ensure_bbox_temporal_compatible(rtree->bboxtype, temp))
    return false;
  int count;
  void *boxes = bbox_temporal_split_boxes(rtree->bboxtype, rtree->bboxsize, temp, maxboxes, &count);
  if (! boxes)
    return true;
  for (int i = 0; i < count; i++)
    rtree_insert(rtree, (char *) boxes + (size_t) i * rtree->bboxsize, id);
  pfree(boxes);
  return true;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Search an RTree built with #rtree_insert_temporal_split using a
 * temporal value, returning each matching id exactly once
 * @details The query temporal value is decomposed into the same tight
 * per-segment bounding boxes as #rtree_insert_temporal_split, the existing
 * #rtree_search is run for every query box, and the union of matching ids is
 * deduplicated so that each surviving id appears exactly once. This is the
 * search counterpart of #rtree_insert_temporal_split; it never produces false
 * negatives with respect to a single-box search and is typically more
 * selective. The result array is reset before the search. The underlying
 * #rtree_search and #rtree_search_temporal semantics are unchanged.
 * @param[in] rtree The RTree to query
 * @param[in] op The search operation
 * @param[in] temp The temporal value whose per-segment bounding boxes serve as
 * queries
 * @param[in] maxboxes Maximum number of query boxes derived from `temp`;
 * values `<= 1` degenerate to a single minimum bounding box query
 * @param[out] result MeosArray of int to collect the deduplicated matching ids
 * @return Number of distinct matching ids
 * @see rtree_search_temporal
 */
/**
 * @brief Order two indexed ids, in the form @p qsort takes
 * @note `int64_cmp` in type_util.c compares two `int64` values rather than two
 * pointers to them, so it is not a `qsort` comparator
 */
static int
rtree_id_cmp(const void *a, const void *b)
{
  int64 l = *(const int64 *) a, r = *(const int64 *) b;
  return (l < r) ? -1 : ((l > r) ? 1 : 0);
}

int
rtree_search_temporal_dedup(const RTree *rtree, IndexSearchOp op,
  const Temporal *temp, int maxboxes, MeosArray *result)
{
  meos_array_reset(result);
  if (! ensure_bbox_temporal_compatible(rtree->bboxtype, temp))
    return 0;

  int count;
  void *boxes = bbox_temporal_split_boxes(rtree->bboxtype, rtree->bboxsize, temp, maxboxes, &count);
  if (! boxes)
    return 0;

  /* Accumulate the raw (possibly duplicated) candidate ids of every query box */
  MeosArray *raw = meos_array_create(sizeof(int64));
  MeosArray *hits = meos_array_create(sizeof(int64));
  for (int i = 0; i < count; i++)
  {
    int nhits = rtree_search(rtree, op,
      (char *) boxes + (size_t) i * rtree->bboxsize, hits);
    for (int j = 0; j < nhits; j++)
    {
      int64 id = *(int64 *) meos_array_get(hits, j);
      meos_array_add(raw, &id);
    }
  }
  pfree(boxes);
  meos_array_destroy(hits);

  /* Collapse duplicates by sorting. The memory this costs is proportional to
   * the NUMBER of candidate ids, never to their magnitude: an id is the
   * caller's own row identifier, so a set sized to the largest id allocates in
   * proportion to a value the caller chooses and reaches gigabytes on an
   * ordinary sparse id space. */
  int nraw = meos_array_count(raw);
  if (nraw > 0)
  {
    int64 *ids = palloc((size_t) nraw * sizeof(int64));
    for (int i = 0; i < nraw; i++)
      ids[i] = *(int64 *) meos_array_get(raw, i);
    qsort(ids, (size_t) nraw, sizeof(int64), rtree_id_cmp);
    for (int i = 0; i < nraw; i++)
      if (i == 0 || ids[i] != ids[i - 1])
        meos_array_add(result, &ids[i]);
    pfree(ids);
  }
  meos_array_destroy(raw);
  return meos_array_count(result);
}

/*****************************************************************************
 * Nearest-neighbour (kNN) cursor
 *
 * An incremental best-first traversal (Hjaltason and Samet) that yields the
 * indexed ids in order of increasing distance to a query bounding box. A
 * binary min-heap holds two kinds of entries: tree nodes still to be expanded
 * and leaf ids ready to be emitted, both keyed by the minimum distance between
 * the query box and the entry's bounding box. Because a node's box distance is
 * a lower bound on the distance of every entry it contains, when a leaf id
 * reaches the top of the heap no unexpanded node can be closer, so ids pop in
 * exact distance order. The caller controls how many ids are consumed (e.g. to
 * honour a `LIMIT k`), so the traversal never materialises more of the tree
 * than the caller reads. The distance is the same box-to-box nearest approach
 * distance used by the `|=|` operator; an entry whose box does not share the
 * query's time extent (for temporal box types with a time dimension on both
 * sides) is reported at infinity and ordered last.
 *****************************************************************************/

/**
 * @brief Return the nearest approach distance between two bounding boxes of
 * the RTree's box type
 * @details Dispatches on the RTree box type to the canonical box-to-box
 * distance so the cursor orders entries exactly as the `|=|` operator. For
 * spatiotemporal boxes the distance is spatial and, when both boxes carry a
 * time dimension, infinite if their time extents are disjoint (mirroring
 * #nad_stbox_stbox); for temporal boxes the same holds through
 * #nad_tbox_tbox; for spans it is the one-dimensional gap.
 * @param[in] rtree The RTree providing the box type
 * @param[in] query,box Bounding boxes of type @p rtree->bboxtype
 */
static double
rtree_bbox_distance(const RTree *rtree, const void *query, const void *box)
{
  if (rtree->bboxtype == T_TBOX)
    return distance_double(nad_tbox_tbox((const TBox *) query,
      (const TBox *) box), ((const TBox *) query)->span.basetype);
  if (rtree->bboxtype == T_STBOX)
    return nad_stbox_stbox((const STBox *) query, (const STBox *) box);
#if POINTCLOUD
  if (rtree->bboxtype == T_TPCBOX)
    /* TPCBox shares the STBox prefix layout (see get_axis_tpcbox) */
    return nad_stbox_stbox((const STBox *) query, (const STBox *) box);
#endif
  /* Span types: the one-dimensional gap between the two spans, zero when they
   * overlap, read through the box's axis accessor */
  double qlo = rtree->get_axis(query, 0, false);
  double qup = rtree->get_axis(query, 0, true);
  double blo = rtree->get_axis(box, 0, false);
  double bup = rtree->get_axis(box, 0, true);
  if (bup < qlo)
    return qlo - bup;
  if (qup < blo)
    return blo - qup;
  return 0.0;
}

/**
 * @brief A single entry of the kNN cursor's priority queue
 * @details An entry is either a tree node still to be expanded
 * (@p is_leaf_entry false, @p node set) or a leaf id ready to be emitted
 * (@p is_leaf_entry true, @p id set), keyed by @p dist, the distance from the
 * query box to the entry's bounding box.
 */
typedef struct RTreeNNEntry
{
  double dist;             /**< Distance from the query to the entry's box */
  bool is_leaf_entry;      /**< True for an emittable id, false for a node */
  int64 id;                /**< Leaf id (when @p is_leaf_entry) */
  const RTreeNode *node;   /**< Tree node to expand (when not @p is_leaf_entry) */
} RTreeNNEntry;

/**
 * @brief Incremental nearest-neighbour cursor over an RTree
 */
struct RTreeNNCursor
{
  const RTree *rtree;      /**< Indexed RTree (borrowed, not owned) */
  void *query;            /**< Private copy of the query bounding box */
  RTreeNNEntry *heap;     /**< Binary min-heap keyed by distance */
  int count;              /**< Number of entries currently in the heap */
  int capacity;           /**< Allocated capacity of the heap array */
};

/**
 * @brief Push an entry onto the cursor's min-heap, growing it if needed
 * @param[in] cursor The cursor whose heap receives the entry
 * @param[in] entry The entry to insert
 */
static void
nn_heap_push(RTreeNNCursor *cursor, RTreeNNEntry entry)
{
  if (cursor->count == cursor->capacity)
  {
    cursor->capacity *= 2;
    cursor->heap = repalloc(cursor->heap,
      (size_t) cursor->capacity * sizeof(RTreeNNEntry));
  }
  int i = cursor->count++;
  cursor->heap[i] = entry;
  /* Sift up while the child is closer than its parent */
  while (i > 0)
  {
    int parent = (i - 1) / 2;
    if (cursor->heap[parent].dist <= cursor->heap[i].dist)
      break;
    RTreeNNEntry tmp = cursor->heap[parent];
    cursor->heap[parent] = cursor->heap[i];
    cursor->heap[i] = tmp;
    i = parent;
  }
  return;
}

/**
 * @brief Pop and return the minimum-distance entry of the cursor's heap
 * @param[in] cursor The cursor whose heap is non-empty
 * @pre `cursor->count > 0`
 */
static RTreeNNEntry
nn_heap_pop(RTreeNNCursor *cursor)
{
  assert(cursor->count > 0);
  RTreeNNEntry top = cursor->heap[0];
  cursor->heap[0] = cursor->heap[--cursor->count];
  /* Sift down towards the closer child */
  int i = 0;
  while (true)
  {
    int left = 2 * i + 1, right = 2 * i + 2, smallest = i;
    if (left < cursor->count &&
        cursor->heap[left].dist < cursor->heap[smallest].dist)
      smallest = left;
    if (right < cursor->count &&
        cursor->heap[right].dist < cursor->heap[smallest].dist)
      smallest = right;
    if (smallest == i)
      break;
    RTreeNNEntry tmp = cursor->heap[i];
    cursor->heap[i] = cursor->heap[smallest];
    cursor->heap[smallest] = tmp;
    i = smallest;
  }
  return top;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Open a nearest-neighbour cursor that yields the ids stored in an
 * RTree in order of increasing distance to a query bounding box
 * @details The cursor performs an incremental best-first traversal: repeated
 * calls to #rtree_nn_cursor_next return the indexed ids from nearest to
 * farthest, using the same box-to-box distance as the `|=|` operator. The
 * caller stops consuming when it has enough neighbours (e.g. after `k`
 * results), so no more of the tree is visited than required. The query box is
 * copied into the cursor, so the caller may free or reuse it immediately. The
 * query box must have the same box type as the RTree; a spatial-only query on
 * an STBox index yields a purely spatial ordering, while a query carrying a
 * time dimension additionally requires overlapping time extents (as for
 * `|=|`). Close the cursor with #rtree_nn_cursor_close.
 * @param[in] rtree The RTree to query
 * @param[in] query The query bounding box of type @p rtree->bboxtype
 * @return A cursor to be freed with #rtree_nn_cursor_close
 */
RTreeNNCursor *
rtree_nn_cursor_open(const RTree *rtree, const void *query)
{
  assert(rtree); assert(query);
  RTreeNNCursor *cursor = palloc0(sizeof(RTreeNNCursor));
  cursor->rtree = rtree;
  cursor->query = palloc(rtree->bboxsize);
  memcpy(cursor->query, query, rtree->bboxsize);
  cursor->capacity = MAXITEMS;
  cursor->heap = palloc((size_t) cursor->capacity * sizeof(RTreeNNEntry));
  cursor->count = 0;
  /* Seed the heap with the root, which is always expanded first */
  if (rtree->root)
  {
    RTreeNNEntry root_entry;
    root_entry.dist = 0.0;
    root_entry.is_leaf_entry = false;
    root_entry.id = 0;
    root_entry.node = rtree->root;
    nn_heap_push(cursor, root_entry);
  }
  return cursor;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Advance a nearest-neighbour cursor to the next closest id
 * @details Returns the next id in order of increasing distance to the query
 * box. When @p id_out or @p dist_out is not @p NULL it receives the id and its
 * distance. An id whose box has no valid distance to the query (e.g. disjoint
 * time extents for temporal box types) is reported last with an infinite
 * distance.
 * @param[in] cursor The cursor previously opened with #rtree_nn_cursor_open
 * @param[out] id_out Receives the id of the next neighbour, or @p NULL
 * @param[out] dist_out Receives the distance of the next neighbour, or @p NULL
 * @return @p true if a neighbour was produced, @p false once exhausted
 */
bool
rtree_nn_cursor_next(RTreeNNCursor *cursor, int64 *id_out, double *dist_out)
{
  assert(cursor);
  while (cursor->count > 0)
  {
    RTreeNNEntry entry = nn_heap_pop(cursor);
    if (entry.is_leaf_entry)
    {
      /* A leaf id reached the top of the heap: nothing unexpanded is closer */
      if (id_out)
        *id_out = entry.id;
      if (dist_out)
        *dist_out = entry.dist;
      return true;
    }
    /* Expand the node: push every child keyed by its box distance */
    const RTreeNode *node = entry.node;
    for (int i = 0; i < node->count; i++)
    {
      RTreeNNEntry child;
      child.dist = rtree_bbox_distance(cursor->rtree, cursor->query,
        RTREE_NODE_BBOX_N(node, i));
      if (node->node_type == RTREE_LEAF)
      {
        child.is_leaf_entry = true;
        child.id = node->ids[i];
        child.node = NULL;
      }
      else
      {
        child.is_leaf_entry = false;
        child.id = 0;
        child.node = node->nodes[i];
      }
      nn_heap_push(cursor, child);
    }
  }
  return false;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Close a nearest-neighbour cursor and free its resources
 * @param[in] cursor The cursor to close; @p NULL is ignored
 */
void
rtree_nn_cursor_close(RTreeNNCursor *cursor)
{
  if (! cursor)
    return;
  pfree(cursor->heap);
  pfree(cursor->query);
  pfree(cursor);
  return;
}

/**
 * @brief Frees the memory allocated for an RTree node
 * @details The function recursively frees the memory of an RTree node.
 * If the node is a branch node, it first recursively frees all child nodes.
 * After handling the child nodes, it frees the memory allocated for the
 * bounding boxes and the arrays of boxes and child nodes within the current
 * node. Finally, it frees the memory allocated for the node itself.
 * @param[in] node Pointer to the node to be freed
 */
static void
node_free(RTreeNode *node)
{
  if (node->node_type == RTREE_INNER)
  {
    for (int i = 0; i < node->count; ++i)
      node_free(node->nodes[i]);
  }
  pfree(node);
}

/*****************************************************************************
 * What an index holds
 *
 * An RTree is an opaque handle, so the size and the shape of the tree are
 * reported by the index itself rather than read from its layout. A consumer
 * accounting for the memory an index holds, or a test asserting that a build
 * chooses the depth rather than following the order the entries arrive in,
 * asks here.
 *****************************************************************************/

/**
 * @brief Accumulate the entries, the bytes and the depth of a subtree
 */
static void
node_stats(const RTreeNode *node, size_t bboxsize, int level, int *entries,
  int64 *bytes, int *height)
{
  *bytes += (int64) (sizeof(RTreeNode) + bboxsize * MAXITEMS);
  if (level > *height)
    *height = level;
  if (node->node_type == RTREE_LEAF)
  {
    *entries += node->count;
    return;
  }
  for (int i = 0; i < node->count; i++)
    node_stats(node->nodes[i], bboxsize, level + 1, entries, bytes, height);
  return;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Return the number of entries an RTree holds
 * @param[in] rtree The RTree
 * @return On error return @p INT_MAX
 */
int
rtree_num_entries(const RTree *rtree)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rtree, INT_MAX);
  if (! rtree->root)
    return 0;
  int entries = 0, height = 0;
  int64 bytes = 0;
  node_stats(rtree->root, rtree->bboxsize, 1, &entries, &bytes, &height);
  return entries;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Return the number of bytes an RTree holds
 * @details The nodes of the tree and the tree itself, which is what a caller
 * accounting for the memory of an index reports
 * @param[in] rtree The RTree
 * @return On error return @p INT64_MAX
 */
int64
rtree_mem_size(const RTree *rtree)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rtree, INT64_MAX);
  int64 bytes = (int64) (sizeof(RTree) + rtree->bboxsize);
  if (rtree->root)
  {
    int entries = 0, height = 0;
    node_stats(rtree->root, rtree->bboxsize, 1, &entries, &bytes, &height);
  }
  return bytes;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Return the number of levels an RTree holds
 * @details An empty tree has no levels and a tree whose root is a leaf has one
 * @param[in] rtree The RTree
 * @return On error return @p INT_MAX
 */
int
rtree_height(const RTree *rtree)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(rtree, INT_MAX);
  if (! rtree->root)
    return 0;
  int entries = 0, height = 0;
  int64 bytes = 0;
  node_stats(rtree->root, rtree->bboxsize, 1, &entries, &bytes, &height);
  return height;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Frees an RTree
 * @param[in] rtree The RTree to free
 */
void
rtree_free(RTree *rtree)
{
  if (rtree->root)
    node_free(rtree->root);
  pfree(rtree);
  return;
}

/*****************************************************************************/
