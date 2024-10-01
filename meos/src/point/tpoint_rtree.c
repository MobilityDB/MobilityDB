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
 * @brief In-memory index for STBox based on RTree
 */

/* C */
#include <stdlib.h>
#include <math.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "point/tpoint_rtree.h"

/**
 * @brief Retrieves the value of a specific axis from an STBox.
 * @details Returns the coordinate or temporal value of a specified axis
 * from the stbox. The axis is determined by the `axis` parameter,
 * and whether the value is from the lower or upper bound of that axis is
 * specified by the `upper` parameter. The function supports the X, Y, Z 
 * spatial axes and the temporal axis.
 * @param[in] box The STBox structure from which the axis value is to be
 * retrieved.
 * @param[in] axis The axis to retrieve (0 = X, 1 = Y, 2 = time, 3 = Z).
 * @param[in] upper A boolean indicating whether to retrieve the upper (`true`) 
 * or lower (`false`) bound of the specified axis.
 * @return The value of the specified axis and bound as a double. Returns 0.0
 * if the axis is invalid.
 */
static double
get_axis_stbox(const STBox *box, int axis, bool upper)
{
  if (axis == 0 && upper)
    return box->xmax;
  if (axis == 0 && ! upper)
    return box->xmin;
  if (axis == 1 && upper)
    return box->ymax;
  if (axis == 1 && ! upper)
    return box->ymin;
  if (axis == 2 && upper)
    return (double)((int64) box->period.upper);
  if (axis == 2 && ! upper)
    return (double)((int64) box->period.lower);
  if (axis == 3 && upper)
    return box->zmax;
  if (axis == 3 && ! upper)
    return box->zmin;
  return 0.0;
}

/**
 * @brief Creates a new RTree node.
 * @details This function initializes a new RTree node.
 * @param[in] kind Boolean flag indicating the type of node (e.g.,inner node
 * or not inner node).
 * @return Pointer to the newly created RTreeNode structure.
 */
static RTreeNode *
node_new(bool kind)
{
  RTreeNode *node = palloc(sizeof(RTreeNode));
  node->kind = kind;
  return node;
}

/**
 * @brief Calculates the length of an STBox along a specified axis.
 * @details This function computes the difference between the upper and lower 
 * bounds of an STBox along a given axis, effectively calculating the length of 
 * the box along that axis.
 * @param[in] rtree Pointer to the RTree structure containing the function to 
 * retrieve axis values.
 * @param[in] box Pointer to the STBox structure for which the axis length is 
 * to be calculated.
 * @param[in] axis The specific axis (e.g. 0, 1, 2) along which to compute 
 * the length.
 * @return The length of the STBox as a double along the specified axis.
 */
static double
get_axis_length(const RTree *rtree, const STBox *box, int axis)
{
  return rtree->get_axis(box, axis, true) - rtree->get_axis(box, axis, false);
}

/**
 * @brief Calculates the area (or volume) of an STBox.
 * @details This function computes the area or volume of an STBox b
 * multiplying the lengths of the box along each axis. The calculation
 * iterates over all dimensions defined in the RTree.
 * @param[in] box Pointer to the STBox structure for which the area or volume
 * is to be calculated.
 * @param[in] rtree Pointer to the RTree structure, which provides the number
 * of dimensions and the method for retrieving axis lengths.
 * @return The computed area or volume of the STBox.
 */
static double
box_area(const STBox *box, const RTree *rtree)
{
  double result = 1.0;
  for (int i = 0; i < rtree->dims; ++i)
    result *= get_axis_length(rtree, box, i);
  return result;
}

/**
 * @brief Calculates the area (or volume) of the union of two STBoxes.
 * @details This function computes the area or volume of the resulting STBox 
 * that represents the union of two given STBoxes. It first creates a new 
 * STBox that is the union of the two input boxes, then calculates the area 
 * or volume of this union box using the specified RTree's dimensions.
 * @param[in] box Pointer to the first STBox structure.
 * @param[in] other_box Pointer to the second STBox structure to be unioned 
 * with the first.
 * @param[in] rtree Pointer to the RTree structure, which provides the number 
 * of dimensions and the method for calculating areas.
 * @return The area or volume of the unioned STBox.
 */
static double
box_unioned_area(const STBox *box, const STBox *other_box, const RTree *rtree)
{
  STBox union_box;
  memcpy(&union_box, box, sizeof(STBox));
  stbox_expand(other_box, &union_box);
  return box_area(&union_box, rtree);
}

/**
 * @brief Selects the child node that requires the least enlargement to
 * accommodate a new STBox.
 * @details This function iterates through the child nodes of a given RTree
 * node to determine which child node's bounding box would need the least
 * enlargement to include a new STBox. The function calculates the area of
 * each node's bounding box before and after union with the new STBox and
 * selects the one with the smallest area increase.
 * @param[in] node Pointer to the RTreeNode structure containing the child
 * nodes.
 * @param[in] box Pointer to the STBox structure that is being inserted or
 * considered.
 * @param[in] rtree Pointer to the RTree structure, which provides the method
 * for calculating areas.
 * @return The index of the child node that requires the least enlargement.
 */
static int
node_choose_least_enlargement(const RTreeNode *node, const STBox *box,
  const RTree *rtree)
{
  int result = 0;
  double previous_enlargement = INFINITY;
  for (int i = 0; i < node->count; ++i)
  {
    double unioned_area = box_unioned_area(&node->boxes[i], box, rtree);
    double area = box_area(&node->boxes[i], rtree);
    double enlarge_area = unioned_area - area;
    if (enlarge_area < previous_enlargement)
    {
      result = i;
      previous_enlargement = enlarge_area;
    }
  }
  return result;
}

/**
 * @brief Chooses the best child node for inserting a new STBox in an RTree.
 * @details This function determines the most suitable child node within an 
 * RTree node for inserting a new STBox. It first checks if the new box can 
 * be added to any child node without requiring the expansion of its bounding 
 * box. If none of the child nodes can accommodate the new STBox without 
 * expansion, the function falls back to selecting the node that requires the 
 * least enlargement.
 * @param[in] rtree Pointer to the RTree structure, providing access to the 
 * overall RTree configuration.
 * @param[in] box Pointer to the STBox structure that is being inserted.
 * @param[in] node Pointer to the RTreeNode structure containing the child
 * nodes.
 * @return The index of the chosen child node for insertion.
 */
static int
node_choose(const RTree *rtree, const STBox *box, const RTreeNode *node)
{
  /* Check if you can add without expanding any rectangle */
  for (int i = 0; i < node->count; ++i)
  {
    if (contains_stbox_stbox(&rtree->box, box))
      return i;
  }
  /* Fallback to "least enlargement" */
  return node_choose_least_enlargement(node, box, rtree);
}

/**
 * @brief Calculates the bounding box that encloses all STBoxes in an RTree
 * node.
 * @details This function takes the destination STBox and change it to the 
 * minimal bounding box enclosing all STBoxes within a given RTree node.
 * @param[in] node Pointer to the RTreeNode structure containing the STBoxes.
 * @param[out] box STBox that will be expanded
 */
static void
node_box_calculate(const RTreeNode *node, STBox *box)
{
  memcpy(box, & node->boxes[0], sizeof(STBox));
  for (int i = 1; i < node->count; ++i)
    stbox_expand(&node->boxes[i], box);
}

/**
 * @brief Identifies the axis with the largest length in an STBox.
 * @details This function determines which axis of an STBox 
 * has the largest length by comparing the lengths of the STBox along each 
 * dimension defined in the RTree. The axis with the greatest length is
 * identified and returned.
 * @param[in] box Pointer to the STBox structure whose largest axis is to be
 * determined.
 * @param[in] rtree Pointer to the RTree structure, which provides the method
 * for calculating axis lengths and defines the number of dimensions.
 * @return The index of the axis with the largest length.
 */
static int
stbox_largest_axis(const STBox *box, const RTree *rtree)
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
 * @brief Moves an STBox from one RTree node to another.
 * @details Changes the information from one node into another.
 * @param[in] from Pointer to the RTreeNode structure from which the STBox is
 * being moved.
 * @param[in] index The index of the STBox in the `from` node that is to be
 * moved.
 * @param[in] into Pointer to the RTreeNode structure where the STBox is being
 * moved to.
 */
static void
node_move_box_at_index_into(RTreeNode *from, int index, RTreeNode *into)
{
  into->boxes[into->count] = from->boxes[index];
  from->boxes[index] = from->boxes[from->count - 1];
  if (from->kind == RTREE_INNER_NODE_NO)
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
}

/**
 * @brief Swaps two STBoxes and their associated data within an RTree node.
 * @details This function exchanges the positions of two STBoxes within a
 * single RTree node. If the node is a leaf, it also swaps the associated IDs.
 * For internal nodes, it swaps the pointers to child nodes. This function is
 * useful for reordering elements within a node.
 * @param[in,out] node Pointer to the RTreeNode structure containing the
 * STBoxes and associated data.
 * @param[in] i The index of the first STBox to be swapped.
 * @param[in] j The index of the second STBox to be swapped.
 */
static void
node_swap(RTreeNode *node, int i, int j)
{
  STBox box = node->boxes[i];
  node->boxes[i] = node->boxes[j];
  node->boxes[j] = box;
  if (node->kind == RTREE_INNER_NODE_NO)
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
}

/**
 * @brief Sorts STBoxes within an RTree node using the QuickSort algorithm.
 * @details This function recursively sorts the STBoxes within a specified
 * range in an RTree node along a particular axis. It uses the QuickSort
 * algorithm to order the STBoxes based on their axis values, either upper or
 * lower, as provided by the `get_axis` function in the RTree structure.
 * @param[in] rtree Pointer to the RTree structure which provides the function
 * for retrieving axis values.
 * @param[in,out] node Pointer to the RTreeNode structure containing the
 * STBoxes to be sorted.
 * @param[in] index The axis index along which to sort the STBoxes.
 * @param[in] upper Boolean flag indicating whether to sort by upper or lower
 * axis value.
 * @param[in] s The starting index of the range to be sorted in the 
 * `node->boxes` array.
 * @param[in] e The ending index (exclusive) of the range to be sorted in the 
 * `node->boxes` array.
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
  node_swap(node, s + pivot, s + right);
  for (int i = 0; i < no_box; ++i)
  {
    if (rtree->get_axis(&node->boxes[right + s], index, upper) > 
        rtree->get_axis(&node->boxes[s + i], index, upper))
    {
      node_swap(node, s + i, s + left);
      left++;
    }
  }
  node_swap(node, s + left, s + right);
  node_qsort(rtree, node, index, upper, s, s + left);
  node_qsort(rtree, node, index, upper, s + left + 1, e);
}

/**
 * @brief Sorts STBoxes within an RTree node along a specified axis.
 * @details This function initiates the sorting of STBoxes within a given RTree
 * node using QuickSort.
 * @param[in] rtree Pointer to the RTree structure, which provides the function
 * for retrieving axis values.
 * @param[in,out] node Pointer to the RTreeNode structure containing the
 * STBoxes to be sorted.
 * @param[in] index The axis index along which to sort the STBoxes.
 * @param[in] upper Boolean flag indicating whether to sort by the upper or
 * lower axis value.
 */
static void
node_sort_axis(const RTree *rtree, RTreeNode *node, int index, bool upper)
{
  node_qsort(rtree, node, index, upper, 0, node->count);
}

/**
 * @brief Splits an RTree node and redistributes its STBoxes between two nodes.
 * @details This function splits an RTree node into two nodes by distributing
 * the STBoxes based on the axis with the largest length. The STBoxes are moved
 * to either the original node or a new right node, depending on their position
 * relative to the splitting axis. After the initial split, the function
 * ensures that both nodes have at least a minimum number of STBoxes by
 * redistributing the STBoxes if necessary. If the node is a branch node, it
 * also sorts both nodes by the first axis.
 * @param[in] rtree Pointer to the RTree structure, which provides methods for
 * retrieving axis values and determining dimensions.
 * @param[in] node Pointer to the RTreeNode structure to be split.
 * @param[in] box Pointer to the STBox structure used to guide the split.
 * @param[out] right_out Pointer to a pointer where the new RTreeNode (right
 * node) will be stored.
 */
static void
node_split(RTree *rtree, RTreeNode *node, STBox *box, RTreeNode ** right_out)
{
  /* Split through the largest axis */
  int largest_axis = stbox_largest_axis(box, rtree);
  RTreeNode *right = node_new(node->kind);
  for (int i = 0; i < node->count; ++i)
  {
    double min_dist = rtree->get_axis(&node->boxes[i], largest_axis, false) -
      rtree->get_axis(box, largest_axis, false);
    double max_dist = rtree->get_axis(box, largest_axis, true) -
      rtree->get_axis(&node->boxes[i], largest_axis, true);
    /* Move to the right */
    if (max_dist < min_dist)
    {
      node_move_box_at_index_into(node, i, right);
      i--;
    }
  }

  /* Make sure that both left and right nodes have at least
   * MINITEMS by moving data into underflowed nodes */
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
  if (node->kind == RTREE_INNER_NODE)
  {
    node_sort_axis(rtree, node, 0, false);
    node_sort_axis(rtree, right, 0, false);
  }
  *right_out = right;
}

/**
 * @brief Inserts an STBox into an RTree node and handles node splitting if 
 * necessary.
 * @details This function inserts a new STBox into an RTree node. If the node
 * is a leaf and already contains the maximum number of items (`MAXITEMS`), 
 * it sets the `split` flag to `true` to indicate that the node needs to be
 * split. For non-leaf nodes, the function determines the appropriate child
 * node for insertion and recursively inserts the STBox. If splitting occurs, 
 * the function handles the split and updates the parent node's bounding boxes.
 * @param[in] rtree Pointer to the RTree structure that provides axis value
 * retrieval and node splitting functions.
 * @param[in] node_bounding_box Pointer to the bounding STBox of all the 
 * STBoxes in `node`
 * @param[in] node Pointer to the RTreeNode structure where the STBox is being
 * inserted.
 * @param[in] new_box Pointer to the STBox to be inserted.
 * @param[in] id Identifier associated with the new STBox (used only for leaf
 * nodes).
 * @param[out] split Pointer to a boolean flag that indicates if the node was
 * split during insertion.
 */
static void
node_insert(RTree *rtree, STBox *node_bounding_box, RTreeNode *node,
  STBox *new_box, int id, bool * split)
{
  if (node->kind == RTREE_INNER_NODE_NO)
  {
    if (node->count == MAXITEMS)
    {
      * split = true;
      return;
    }
    int index = node->count;
    node->boxes[index] = * new_box;
    node->ids[index] = id;
    node->count++;
    * split = false;
    return;
  }
  int insertion_node = node_choose(rtree, new_box, node);
  node_insert(rtree, & node->boxes[insertion_node],
    (RTreeNode *) node->nodes[insertion_node], new_box, id, split);
  if (! *split)
  {
    stbox_expand(new_box, & node->boxes[insertion_node]);
    *split = false;
    return;
  }
  if (node->count == MAXITEMS)
  {
    *split = true;
    return;
  }
  RTreeNode *right;
  node_split(rtree, node->nodes[insertion_node], &node->boxes[insertion_node],
    &right);
  node_box_calculate(node->nodes[insertion_node], &node->boxes[insertion_node]);
  node_box_calculate(right, &node->boxes[node->count]);
  node->nodes[node->count] = right;;
  node->count++;
  return node_insert(rtree, node_bounding_box, node, new_box, id, split);
}

/**
 * @brief Checks if a number bigger than 0 is a power of two or not. 
 * @param[in] n Number to check
 * @returns True if `n` is a power of two, False otherwise.
 */
static bool
is_power_of_two(const int n)
{
  return (n & (n - 1)) == 0;
}

/**
 * @brief Adds an ID to the dynamically allocated array with the answer of a 
 * query.
 * @param[in] id The integer ID to be added to the array.
 * @param[in] ids Pointer to a pointer to the dynamically allocated array of
 * integers.
 * @param[in] count Pointer to an integer representing the current number of
 * elements in the array.
 */
static void
add_answer(const int id, int **ids, int *count)
{
  /* Every power of two that exceeds the size of the array must  
   * be resized to double the current size */
  if (*count >= SEARCH_ARRAY_STARTING_SIZE && is_power_of_two(*count))
    *ids = repalloc(*ids, sizeof(int) * (*count) * 2);
  (*ids)[*count] = id;
  (*count)++;
}

/**
 * @brief Frees the memory allocated for an RTree node and its associated
 * resources.
 * @details This function recursively frees the memory of an RTree node.
 * If the node is a branch node, it first recursively frees all child nodes. 
 * After handling the child nodes, it frees the memory allocated for the
 * STBoxes and the arrays of boxes and child nodes within the current node.
 * Finally, it frees the memory allocated for the node itself.
 * @param[in] node Pointer to the RTreeNode structure to be freed.
 */
static void
node_free(RTreeNode *node)
{
  if (node->kind == RTREE_INNER_NODE)
  {
    for (int i = 0; i < node->count; ++i)
      node_free(node->nodes[i]);
  }
  free(node);
}

/**
 * @brief Searches recursively a node looking for hits with a query.
 * @param[in] node The node to be searched
 * @param[in] query The STBox that serves as query
 * @param[in] ids The array with the list of answers.
 * @param[in] count Total of elements found
 */
void node_search(const RTreeNode *node, const STBox *query, int **ids,
  int *count)
{
  for (int i = 0; i < node->count; ++i)
  {
    if (overlaps_stbox_stbox(query, &node->boxes[i]))
    {
      if (node->kind == RTREE_INNER_NODE_NO)
        add_answer(node->ids[i], ids, count);
      else
        node_search(node->nodes[i], query, ids, count);
    }
  }
}

/**
 * @brief Sets the dimensions of an R-tree based on the meostype of the RTree
 * @param[in,out] rtree The R-tree structure whose dimensions are to be set.
 * @param[in] box The spatial bounding box (STBox) from which to derive the
 * dimensions.
 * @return `true` if the dimensions were successfully set; `false` otherwise.
 */
static bool
rtree_set_dims(RTree *rtree, const STBox *box)
{
  switch (rtree->basetype)
  {
    case T_STBOX:
      rtree->dims = 3 + MEOS_FLAGS_GET_Z(box->flags);
      return true;
    default:
      break;
  }
  return false;
}

/**
 * @brief Sets the appropriate get axis function for the R-tree based on its
 * meosType.
 * @param[in,out] rtree The R-tree structure for which the function pointer is
 * to be set.
 * @return `true` if the function pointer was successfully set; `false`
 * otherwise.
 */
static bool
rtree_set_functions(RTree *rtree)
{
  switch (rtree->basetype)
  {
    case T_STBOX:
      /* TODO: This should be deprecated when we start using picksplit since
       * this function is only used in the splitting phase. */
      rtree->get_axis = get_axis_stbox;
      return true;
    default:
      break;
  }
  return false;
}

/**
 * @brief Creates an RTree index.
 * @param[in] basetype The meosType of the elements to index.
 * Currently the only basetype supported is T_STBOX.
 * @return RTree initialized.
 */
RTree *
rtree_create(meosType basetype)
{
  RTree *rtree = palloc0(sizeof(RTree));
  rtree->basetype = basetype;
  if (!rtree_set_functions(rtree))
  {
    pfree(rtree);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, 
      "Unsupported base type for RTree %d", basetype);
    return NULL;
  }
  return rtree;
}

/**
 * @ingroup meos_stbox_rtree_index
 * @brief Insert an STBox into the RTree index. 
 * @note the parameter `id` is used for the search function, when a match
 * is found
 * the id will be returned. The STBox will be copied into the RTRee.
 * @param[in] rtree The RTree previously initialized
 * @param[in] box The stbox to be inserted
 * @param[in] id The id of the box being inserted
 */
void
rtree_insert(RTree *rtree, STBox *box, int64 id)
{
  while (1)
  {
    if (! rtree->root)
    {
      RTreeNode *new_root = node_new(RTREE_INNER_NODE_NO);
      rtree_set_dims(rtree, box);
      rtree->root = new_root;
      rtree->box = *box;
    }
    bool split = false;
    node_insert(rtree, &rtree->box, rtree->root, box, id, &split);
    if (! split)
    {
      stbox_expand(box, &rtree->box);
      return;
    }
    RTreeNode *new_root = node_new(RTREE_INNER_NODE);
    RTreeNode *right;
    node_split(rtree, rtree->root, &rtree->box, &right);

    node_box_calculate(rtree->root, &new_root->boxes[0]);
    node_box_calculate(right, &new_root->boxes[1]);
    new_root->nodes[0] = rtree->root;
    new_root->nodes[1] = right;
    rtree->root = new_root;
    rtree->root->count = 2;
  }
}

/**
 * @ingroup meos_stbox_rtree_index
 * @brief Creates an RTree index for STBoxes.
 * @return RTree initialized for STBoxes.
 */
RTree *
rtree_create_stbox()
{
  return rtree_create(T_STBOX);
}

/**
 * @ingroup meos_stbox_rtree_index
 * @brief Queries an RTree with an STBox. Returns an array of ids of STBoxes.
 * @note The @p count will be the output size of the array given.
 * @param[in] rtree The RTree to query.
 * @param[in] query The STBox that serves as query.
 * @param[out] count An int that corresponds to the number of hits the RTree
 * found.
 * @return array of ids that have a hit.
 */
int *
rtree_search(const RTree *rtree, const STBox *query, int *count)
{
  int *ids = palloc(sizeof(int) * SEARCH_ARRAY_STARTING_SIZE);
  *count = 0;
  if (rtree->root)
    node_search(rtree->root, query, &ids, count);
  return ids;
}

/**
 * @ingroup meos_stbox_rtree_index
 * @brief Frees the RTree
 * @param[in] rtree The RTree to free.
 */
void
rtree_free(RTree *rtree)
{
  if (rtree->root)
    node_free(rtree->root);
  free(rtree);
}
