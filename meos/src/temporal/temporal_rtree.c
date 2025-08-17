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
 * @brief In-memory RTree index for MEOS bounding boxes, i.e., for Span, TBox,
 * and STBox
 */

/* C */
#include <stdlib.h>
#include <math.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
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
get_axis_span(const void *box, int axis, bool upper)
{
  assert(box); assert(axis == 0);
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
  assert(box); assert(axis >= 0 || axis <= 3);
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

/*****************************************************************************/

/**
 * @brief Return `true` if the two spans overlap, `false` otherwise
 * @param[in] box1,box2 Spans
 */
static inline bool
bbox_overlaps_span(const void *box1, const void *box2)
{
  return overlaps_span_span((Span *) box1, (Span *) box2);
}

/**
 * @brief Return `true` if the two temporal boxes overlap, `false` otherwise
 * @param[in] box1,box2 Temporal boxes
 */
static inline bool
bbox_overlaps_tbox(const void *box1, const void *box2)
{
  return overlaps_tbox_tbox((TBox *) box1, (TBox *) box2);
}

/**
 * @brief Return `true` if the two spatiotemporal boxes overlap, `false`
 * otherwise
 * @param[in] box1,box2 Spatiotemporal boxes
 */
static inline bool
bbox_overlaps_stbox(const void *box1, const void *box2)
{
  return overlaps_stbox_stbox((STBox *) box1, (STBox *) box2);
}

/*****************************************************************************
 * Rtree functions
 *****************************************************************************/

/**
 * @brief Creates a new RTree node
 * @param[in] isRoot True when the node is a root node, False otherwise
 * @param[in] bboxsize Bounding box size 
 * @return Pointer to the newly created node
 */
static RTreeNode *
node_make(bool isRoot, size_t bboxsize)
{
  size_t bboxes_size = bboxsize * MAXITEMS;
  RTreeNode *node = palloc0(sizeof(RTreeNode) + bboxes_size);
  node->isRoot = isRoot;
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
  /* STBox is the largest MEOS bounding boxes */
  STBox union_box;
  memcpy(&union_box, box1, rtree->bboxsize);
  rtree->bbox_expand(box2, &union_box);
  return box_area(rtree, &union_box);
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
    if (rtree->bbox_contains(&rtree->box, box))
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
  if (from->isRoot == RTREE_ROOT_NODE)
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
 * @param[in,out] node Pointer to the node containing the
 * bounding boxes and associated data.
 * @param[in] i The index of the first bounding box to be swapped.
 * @param[in] j The index of the second bounding box to be swapped.
 */
static void
node_swap(const RTree *rtree, RTreeNode *node, int i, int j)
{
  /* STBox is the largest MEOS bounding boxes */
  STBox box;
  memcpy(&box, RTREE_NODE_BBOX_N(node, i), rtree->bboxsize);
  memcpy(RTREE_NODE_BBOX_N(node, i), RTREE_NODE_BBOX_N(node, j),
    rtree->bboxsize);
  memcpy(RTREE_NODE_BBOX_N(node, j), &box, rtree->bboxsize);
  if (node->isRoot == RTREE_ROOT_NODE)
  {
    int tmp = node->ids[i];
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
  int no_box = e - s;
  if (no_box < 2)
    return;

  int left = 0;
  int right = no_box - 1;
  int pivot = no_box / 2;
  node_swap(rtree, node, s + pivot, s + right);
  for (int i = 0; i < no_box; ++i)
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
  RTreeNode *right = node_make(node->isRoot, rtree->bboxsize);
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
  if (node->isRoot == RTREE_INNER_NODE)
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
  void *new_box, int id, bool *split)
{
  if (node->isRoot == RTREE_ROOT_NODE)
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
 * @brief Returns `true` if a number greater than 0 is a power of two, `false`
 * otherwise
 * @param[in] n Number to check
 */
static inline bool
is_power_of_two(const int n)
{
  return (n & (n - 1)) == 0;
}

/**
 * @brief Adds an ID to the dynamically allocated array with the answer of a
 * query
 * @param[in] id The integer ID to be added to the array
 * @param[in] ids Pointer to a pointer to the dynamically allocated array of
 * integers
 * @param[in] count Pointer to an integer representing the current number of
 * elements in the array.
 */
static void
add_answer(const int id, int **ids, int *count)
{
  /* Every power of two that exceeds the size of the array must be resized to
   * double the current size */
  if (*count >= SEARCH_ARRAY_STARTING_SIZE && is_power_of_two(*count))
    *ids = repalloc(*ids, sizeof(int) * (*count) * 2);
  (*ids)[*count] = id;
  (*count)++;
  return;
}

/**
 * @brief Searches recursively a node looking for hits with a query
 * @param[in] node The node to be searched
 * @param[in] query The bounding box that serves as query
 * @param[in] ids The array with the list of answers
 * @param[in] count Total of elements found
 */
void
node_search(const RTree *rtree, const RTreeNode *node, const void *query,
  int **ids, int *count)
{
  for (int i = 0; i < node->count; ++i)
  {
    if (rtree->bbox_overlaps(query, RTREE_NODE_BBOX_N(node, i)))
    {
      if (node->isRoot == RTREE_ROOT_NODE)
        add_answer(node->ids[i], ids, count);
      else
        node_search(rtree, node->nodes[i], query, ids, count);
    }
  }
  return;
}

/**
 * @brief Creates an RTree index.
 * @param[in] bboxtype The meosType of the elements to index.
 * @return RTree initialized.
 */
RTree *
rtree_create(meosType bboxtype)
{
  assert(span_type(bboxtype) || bboxtype == T_TBOX || bboxtype == T_STBOX);
  size_t bboxsize = bbox_get_size(bboxtype);
  RTree *rtree = palloc0(sizeof(RTree) + bboxsize);
  if (span_type(bboxtype))
  {
    rtree->dims = 1;
    rtree->get_axis = &get_axis_span;
    rtree->bbox_expand = &bbox_expand_span;
    rtree->bbox_contains = &bbox_contains_span;
    rtree->bbox_overlaps = &bbox_overlaps_span;
  }
  else if (bboxtype == T_TBOX)
  {
    rtree->dims = 2;
    rtree->get_axis = &get_axis_tbox;
    rtree->bbox_expand = &bbox_expand_tbox;
    rtree->bbox_contains = &bbox_contains_tbox;
    rtree->bbox_overlaps = &bbox_overlaps_tbox;
  }
  else /* bboxtype == T_STBOX */
  {
    /* To be set when the first node is created since it is not known yet
     * whether there is a Z dimension or not */
    rtree->dims = -1;
    rtree->get_axis = &get_axis_stbox;
    rtree->bbox_expand = &bbox_expand_stbox;
    rtree->bbox_contains = &bbox_contains_stbox;
    rtree->bbox_overlaps = &bbox_overlaps_stbox;
  }
  rtree->bboxtype = bboxtype;
  rtree->bboxsize = bboxsize;
  return rtree;
}

/**
 * @brief Creates an RTree index for integer spans
 * @return RTree initialized
 */
RTree *
rtree_create_intspan()
{
  return rtree_create(T_INTSPAN);
}

/**
 * @brief Creates an RTree index for big integer spans
 * @return RTree initialized
 */
RTree *
rtree_create_bigintspan()
{
  return rtree_create(T_BIGINTSPAN);
}

/**
 * @brief Creates an RTree index for float spans
 * @return RTree initialized
 */
RTree *
rtree_create_floatspan()
{
  return rtree_create(T_FLOATSPAN);
}

/**
 * @brief Creates an RTree index for temporal boxes
 * @return RTree initialized
 */
RTree *
rtree_create_datespan()
{
  return rtree_create(T_DATESPAN);
}

/**
 * @brief Creates an RTree index for temporal boxes
 * @return RTree initialized
 */
RTree *
rtree_create_tstzspan()
{
  return rtree_create(T_TSTZSPAN);
}

/**
 * @brief Creates an RTree index for temporal boxes
 * @return RTree initialized
 */
RTree *
rtree_create_tbox()
{
  return rtree_create(T_TBOX);
}

/**
 * @brief Creates an RTree index for spatiotemporal boxes
 * @return RTree initialized
 */
RTree *
rtree_create_stbox()
{
  return rtree_create(T_STBOX);
}

/**
 * @ingroup meos_geo_box_index
 * @brief Insert a bounding box into the RTree index.
 * @note The parameter `id` is used for the search function, when a match
 * is found the id will be returned. The bounding box will be copied into the
 * RTRee.
 * @param[in] rtree The RTree previously initialized
 * @param[in] box The bounding box to be inserted
 * @param[in] id The id of the box being inserted
 */
void
rtree_insert(RTree *rtree, void *box, int64 id)
{
  while (1)
  {
    if (! rtree->root)
    {
      RTreeNode *new_root = node_make(RTREE_ROOT_NODE, rtree->bboxsize);
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
      return;
    }
    RTreeNode *new_root = node_make(RTREE_INNER_NODE, rtree->bboxsize);
    RTreeNode *right;
    node_split(rtree, rtree->root, &rtree->box, &right);

    node_box_calculate(rtree, rtree->root, RTREE_NODE_BBOX_N(new_root, 0));
    node_box_calculate(rtree, right, RTREE_NODE_BBOX_N(new_root, 1));
    new_root->nodes[0] = rtree->root;
    new_root->nodes[1] = right;
    rtree->root = new_root;
    rtree->root->count = 2;
  }
  return;
}

/**
 * @ingroup meos_geo_box_index
 * @brief Queries an RTree with a bounding box. Returns an array of ids of
 * bounding boxes.
 * @param[in] rtree The RTree to query
 * @param[in] query The bounding box that serves as query
 * @param[out] count The number of hits the RTree found
 * @return Array of ids that have a hit.
 * @note The `count` will be the output size of the array given.
 */
int *
rtree_search(const RTree *rtree, const void *query, int *count)
{
  int *ids = palloc(sizeof(int) * SEARCH_ARRAY_STARTING_SIZE);
  *count = 0;
  if (rtree->root)
    node_search(rtree, rtree->root, query, &ids, count);
  return ids;
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
  if (node->isRoot == RTREE_INNER_NODE)
  {
    for (int i = 0; i < node->count; ++i)
      node_free(node->nodes[i]);
  }
  free(node);
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
  free(rtree);
  return;
}

/*****************************************************************************/
