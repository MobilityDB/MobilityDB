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
 * @brief In memory index for STBox based on RTree
 */

/* C */
#include <stdlib.h>

#include <math.h>

/* MEOS */ #include <meos.h>

#include "point/tpoint_rtree.h"

#include "point/stbox.h"


/**
 * @brief Creates a new RTree node.
 * @details This function initializes a new RTree node, allocating memory for 
 * storing bounding boxes (STBox) and child nodes.
 * @param[in] kind Boolean flag indicating the type of node (e.g., leaf or branch).
 * @return Pointer to the newly created RTreeNode structure.
 */
static RTreeNode *
  node_new(bool kind) {
    RTreeNode * node = malloc(sizeof(RTreeNode));
    memset(node, 0, sizeof(RTreeNode));
    node -> kind = kind;
    node -> boxes = malloc(sizeof(STBox * ) * MAXITEMS);
    node -> nodes = malloc(sizeof(RTreeNode * ) * MAXITEMS);
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
get_axis_length(const RTree * rtree,
  const STBox * box, int axis) {
  return rtree -> get_axis(box, axis, true) - rtree -> get_axis(box, axis, false);
}

/**
 * @brief Calculates the area (or volume) of an STBox.
 * @details This function computes the area or volume of an STBox by multiplying
 * the lengths of the box along each axis. The calculation iterates over all
 * dimensions defined in the RTree.
 * @param[in] box Pointer to the STBox structure for which the area or volume
 * is to be calculated.
 * @param[in] rtree Pointer to the RTree structure, which provides the number
 * of dimensions and the method for retrieving axis lengths.
 * @return The computed area or volume of the STBox.
 */
static double
box_area(const STBox * box, RTree * rtree) {
  double result = 1.0;
  for (int i = 0; i < rtree -> dims; ++i) {
    result *= get_axis_length(rtree, box, i);
  }
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
box_unioned_area(const STBox * box,
  const STBox * other_box, RTree * rtree) {
  STBox * union_box = malloc(sizeof(STBox));
  memcpy(union_box, box, sizeof(STBox));
  stbox_expand(other_box, union_box);

  double answer = box_area(union_box, rtree);
  free(union_box);
  return answer;
}

/**
 * @brief Selects the child node that requires the least enlargement to accommodate a new STBox.
 * @details This function iterates through the child nodes of a given RTree node to determine 
 * which child node's bounding box would need the least enlargement to include a new STBox. 
 * The function calculates the area of each node's bounding box before and after union with 
 * the new STBox and selects the one with the smallest area increase.
 * @param[in] node Pointer to the RTreeNode structure containing the child nodes.
 * @param[in] box Pointer to the STBox structure that is being inserted or considered.
 * @param[in] rtree Pointer to the RTree structure, which provides the method for calculating areas.
 * @return The index of the child node that requires the least enlargement.
 */
static int
node_choose_least_enlargement(const RTreeNode * node,
  const STBox * box, RTree * rtree) {
  int result = 0;
  double previous_enlargement = INFINITY;
  for (int i = 0; i < node -> count; ++i) {
    double unioned_area = box_unioned_area(node -> boxes[i], box, rtree);
    double area = box_area(node -> boxes[i], rtree);
    double enlarge_area = unioned_area - area;
    if (enlarge_area < previous_enlargement) {
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
 * @param[in] node Pointer to the RTreeNode structure containing the child nodes.
 * @return The index of the chosen child node for insertion.
 */
static int
node_choose(RTree * rtree,
  const STBox * box,
    const RTreeNode * node) {
  // Check if you can add without expanding any rectangle.
  for (int i = 0; i < node -> count; ++i) {
    if (contains_stbox_stbox(rtree -> box, box)) {
      return i;
    }
  }

  // Fallback to "least enlargement"
  return node_choose_least_enlargement(node, box, rtree);
}

/**
 * @brief Calculates the bounding box that encloses all STBoxes in an RTree node.
 * @details This function computes a new STBox that represents the minimal bounding box 
 * enclosing all STBoxes within a given RTree node.
 * @param[in] node Pointer to the RTreeNode structure containing the STBoxes.
 * @return A pointer to the newly allocated STBox structure that represents the bounding 
 * box enclosing all STBoxes.
 */
static STBox *
  node_box_calculate(const RTreeNode * node) {
    STBox * result = malloc(sizeof(STBox));
    memcpy(result, node -> boxes[0], sizeof(STBox));
    for (int i = 0; i < node -> count; ++i) {
      stbox_expand(node -> boxes[i], result);
    }
    return result;
  }

/**
 * @brief Identifies the axis with the largest length in an STBox.
 * @details This function determines which axis of an STBox 
 * has the largest length by comparing the lengths of the STBox along each 
 * dimension defined in the RTree. The axis with the greatest length is identified 
 * and returned.
 * @param[in] box Pointer to the STBox structure whose largest axis is to be determined.
 * @param[in] rtree Pointer to the RTree structure, which provides the method for calculating axis lengths and defines the number of dimensions.
 * @return The index of the axis with the largest length.
 */
static int
stbox_largest_axis(STBox * box, RTree * rtree) {
  int largest_axis = 0;
  double previous_largest = get_axis_length(rtree, box, 0);
  for (int i = 1; i < rtree -> dims; ++i) {
    if (previous_largest < get_axis_length(rtree, box, 0)) {
      previous_largest = get_axis_length(rtree, box, 0);
      largest_axis = i;
    }
  }
  return largest_axis;
}

/**
 * @brief Moves an STBox from one RTree node to another.
 * @details Changes the information from one node into another.
 * @param[in] from Pointer to the RTreeNode structure from which the STBox is being moved.
 * @param[in] index The index of the STBox in the `from` node that is to be moved.
 * @param[in] into Pointer to the RTreeNode structure where the STBox is being moved to.
 */
static void
node_move_box_at_index_into(RTreeNode * from, int index, RTreeNode * into) {
  into -> boxes[into -> count] = from -> boxes[index];
  from -> boxes[index] = from -> boxes[from -> count - 1];
  if (from -> kind == LEAF) {
    into -> ids[into -> count] = from -> ids[index];
    from -> ids[index] = from -> ids[from -> count - 1];
  } else {
    into -> nodes[into -> count] = from -> nodes[index];
    from -> nodes[index] = from -> nodes[from -> count - 1];
  }
  from -> count--;
  into -> count++;
}

/**
 * @brief Swaps two STBoxes and their associated data within an RTree node.
 * @details This function exchanges the positions of two STBoxes within a single RTree node. 
 * If the node is a leaf, it also swaps the associated IDs. For internal nodes, it swaps the 
 * pointers to child nodes. This function is useful for reordering elements within a node.
 * @param[in] node Pointer to the RTreeNode structure containing the STBoxes and associated data.
 * @param[in] i The index of the first STBox to be swapped.
 * @param[in] j The index of the second STBox to be swapped.
 */
static void
node_swap(RTreeNode * node, int i, int j) {
  STBox * tmp = node -> boxes[i];
  node -> boxes[i] = node -> boxes[j];
  node -> boxes[j] = tmp;
  if (node -> kind == LEAF) {
    int tmp = node -> ids[i];
    node -> ids[i] = node -> ids[j];
    node -> ids[j] = tmp;
  } else {
    RTreeNode * tmp = node -> nodes[i];
    node -> nodes[i] = node -> nodes[j];
    node -> nodes[j] = tmp;
  }
}

/**
 * @brief Sorts STBoxes within an RTree node using the QuickSort algorithm.
 * @details This function recursively sorts the STBoxes within a specified range in an RTree node 
 * along a particular axis. It uses the QuickSort algorithm to order the STBoxes based on their 
 * axis values, either upper or lower, as provided by the `get_axis` function in the RTree structure.
 * @param[in] rtree Pointer to the RTree structure which provides the function for retrieving axis values.
 * @param[in] node Pointer to the RTreeNode structure containing the STBoxes to be sorted.
 * @param[in] index The axis index along which to sort the STBoxes.
 * @param[in] upper Boolean flag indicating whether to sort by upper or lower axis value.
 * @param[in] s The starting index of the range to be sorted in the `node->boxes` array.
 * @param[in] e The ending index (exclusive) of the range to be sorted in the `node->boxes` array.
 */
static void
node_qsort(const RTree * rtree, RTreeNode * node, int index, bool upper, int s, int e) {
  int no_box = e - s;
  if (no_box < 2) {
    return;
  }
  int left = 0;
  int right = no_box - 1;
  int pivot = no_box / 2;
  node_swap(node, s + pivot, s + right);
  for (int i = 0; i < no_box; ++i) {
    if (rtree -> get_axis(node -> boxes[right + s], index, upper) > rtree -> get_axis(node -> boxes[s + i], index, upper)) {
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
 * @details This function initiates the sorting of STBoxes within a given RTree node using QuickSort.
 * @param[in] rtree Pointer to the RTree structure, which provides the function for retrieving axis values.
 * @param[in,out] node Pointer to the RTreeNode structure containing the STBoxes to be sorted.
 * @param[in] index The axis index along which to sort the STBoxes.
 * @param[in] upper Boolean flag indicating whether to sort by the upper or lower axis value.
 */
static void
node_sort_axis(const RTree * rtree, RTreeNode * node, int index, bool upper) {
  node_qsort(rtree, node, index, upper, 0, node -> count);
}

/**
 * @brief Splits an RTree node and redistributes its STBoxes between two nodes.
 * @details This function splits an RTree node into two nodes by distributing the STBoxes based on 
 * the axis with the largest length. The STBoxes are moved to either the original node or a new 
 * right node, depending on their position relative to the splitting axis. After the initial split, 
 * the function ensures that both nodes have at least a minimum number of STBoxes by redistributing 
 * the STBoxes if necessary. If the node is a branch node, it also sorts both nodes by the first axis.
 * @param[in] rtree Pointer to the RTree structure, which provides methods for retrieving axis values and 
 * determining dimensions.
 * @param[in] node Pointer to the RTreeNode structure to be split.
 * @param[in] box Pointer to the STBox structure used to guide the split.
 * @param[out] right_out Pointer to a pointer where the new RTreeNode (right node) will be stored.
 */
static void
node_split(RTree * rtree, RTreeNode * node, STBox * box, RTreeNode ** right_out) {
  // Split through the largest axis. 
  int largest_axis = stbox_largest_axis(box, rtree);
  RTreeNode * right = node_new(node -> kind);
  for (int i = 0; i < node -> count; ++i) {
    double min_dist = rtree -> get_axis(node -> boxes[i], largest_axis, false) - rtree -> get_axis(box, largest_axis, false);
    double max_dist = rtree -> get_axis(box, largest_axis, true) - rtree -> get_axis(node -> boxes[i], largest_axis, true);
    // move to right
    if (max_dist < min_dist) {
      node_move_box_at_index_into(node, i, right);
      i--;
    }
  }

  // Make sure that both left and right nodes have at least
  // MINITEMS by moving data into underflowed nodes.
  if (node -> count < MINITEMS) {
    // reverse sort by min axis
    node_sort_axis(rtree, right, largest_axis, false);
    do {
      node_move_box_at_index_into(right, right -> count, node);
    } while (node -> count < MINITEMS);
  } else if (right -> count < MINITEMS) {
    // reverse sort by max axis
    node_sort_axis(rtree, node, largest_axis, true);
    do {
      node_move_box_at_index_into(node, node -> count - 1, right);
    } while (right -> count < MINITEMS);
  }
  if (node -> kind == BRANCH) {
    node_sort_axis(rtree, node, 0, false);
    node_sort_axis(rtree, right, 0, false);
  }
  * right_out = right;
}

/**
 * @brief Inserts an STBox into an RTree node and handles node splitting if necessary.
 * @details This function inserts a new STBox into an RTree node. If the node is a leaf and 
 * already contains the maximum number of items (`MAXITEMS`), it sets the `split` flag to `true`
 * to indicate that the node needs to be split. For non-leaf nodes, the function determines 
 * the appropriate child node for insertion and recursively inserts the STBox. If splitting occurs, 
 * the function handles the split and updates the parent node's bounding boxes.
 * @param[in] rtree Pointer to the RTree structure that provides axis value retrieval and node splitting functions.
 * @param[in] old_box Pointer to the STBox that is being replaced or added.
 * @param[in] node Pointer to the RTreeNode structure where the STBox is being inserted.
 * @param[in] new_box Pointer to the STBox to be inserted.
 * @param[in] id Identifier associated with the new STBox (used only for leaf nodes).
 * @param[out] split Pointer to a boolean flag that indicates if the node was split during insertion.
 */
static void
node_insert(RTree * rtree, STBox * old_box, RTreeNode * node,
  STBox * new_box, int id, bool * split) {
  if (node -> kind == LEAF) {
    if (node -> count == MAXITEMS) {
      * split = true;
      return;
    }
    int index = node -> count;
    node -> boxes[index] = new_box;
    node -> ids[index] = id;
    node -> count++;
    * split = false;
    return;
  }
  int insertion_node = node_choose(rtree, new_box, node);
  node_insert(rtree, node -> boxes[insertion_node], (RTreeNode * ) node -> nodes[insertion_node], new_box, id, split);
  if (! * split) {
    stbox_expand(new_box, node -> boxes[insertion_node]);
    * split = false;
    return;
  }
  if (node -> count == MAXITEMS) {
    * split = true;
    return;
  }
  RTreeNode * right;
  node_split(rtree, node -> nodes[insertion_node], node -> boxes[insertion_node], & right);
  node -> boxes[insertion_node] = node_box_calculate(node -> nodes[insertion_node]);
  node -> boxes[node -> count] = node_box_calculate(right);
  node -> nodes[node -> count] = right;;
  node -> count++;
  return node_insert(rtree, old_box, node, new_box, id, split);

}

/**
 * @brief Adds an ID to the dynamically allocated array with the answer of a query.
 * @param[in] id The integer ID to be added to the array.
 * @param[in] ids Pointer to a pointer to the dynamically allocated array of integers.
 * @param[in] count Pointer to an integer representing the current number of elements in the array.
 */
void add_answer(int id, int ** ids, int * count) {
  if ( * count >= 63 && (( * count & ( * count + 1)) == 0)) {
    * ids = realloc( * ids, sizeof(int) * ( * count + 1) * 2);
  }
  ( * ids)[ * count] = id;
  ( * count) ++;
}

/**
 * @brief Frees the memory allocated for an RTree node and its associated resources.
 * @details This function recursively frees the memory of an RTree node. If the node is a branch node,
 * it first recursively frees all child nodes. After handling the child nodes, it frees the memory allocated
 * for the STBoxes and the arrays of boxes and child nodes within the current node. Finally, it frees the memory
 * allocated for the node itself.
 * @param[in] node Pointer to the RTreeNode structure to be freed.
 */
static void
node_free(RTreeNode * node) {
  if (node -> kind == BRANCH) {
    for (int i = 0; i < node -> count; ++i) {
      node_free(node -> nodes[i]);
    }
  }
  for (int i = 0; i < node -> count; ++i) {
    free(node -> boxes[i]);
  }
  free(node -> boxes);
  free(node -> nodes);
  free(node);
}

/**
 * @brief Searches recursively a node looking for hits with a query.
 * @param[in] node The node to be searched
 * @param[in] query The STBox that serves as query
 * @param[in] ids The array with the list of answers.
 * @param[in] count Total of elements found
 */
void node_search(const RTreeNode * node,
  const STBox * query, int ** ids, int * count) {
  if (node -> kind == LEAF) {
    for (int i = 0; i < node -> count; ++i) {
      if (overlaps_stbox_stbox(query, node -> boxes[i])) {
        add_answer(node -> ids[i], ids, count);
      }
    }
    return;
  }
  for (int i = 0; i < node -> count; ++i) {
    if (overlaps_stbox_stbox(query, node -> boxes[i])) {
      node_search(node -> nodes[i], query, ids, count);
    }
  }
}

/**
 * @ingroup meos_stbox_rtree_index
 * @brief Insert an STBox into the RTree index. 
 * @note the parameter @id is used for the search function, when a match is found
 * the id will be returned. The STBox will be copied into the RTRee.
 * @param[in] rtree The RTree previously initialized
 * @param[in] box The stbox to be inserted
 * @param[in] id The id of the box being inserted
 */
void
rtree_insert(RTree * rtree, STBox * box, int64 id) {
  /** Copy STBox */
  STBox * new_box = malloc(sizeof(STBox));
  memcpy(new_box, box, sizeof(STBox));

  while (1) {
    if (!rtree -> root) {
      RTreeNode * new_root = node_new(LEAF);
      rtree -> root = new_root;
      rtree -> box = new_box;
    }
    bool split = false;
    node_insert(rtree, rtree -> box, rtree -> root, new_box, id, & split);
    if (!split) {
      stbox_expand(new_box, rtree -> box);
      rtree -> count++;
      return;
    }
    RTreeNode * new_root = node_new(BRANCH);
    RTreeNode * right;
    node_split(rtree, rtree -> root, rtree -> box, & right);

    new_root -> boxes[0] = node_box_calculate(rtree -> root);
    new_root -> boxes[1] = node_box_calculate(right);
    new_root -> nodes[0] = rtree -> root;
    new_root -> nodes[1] = right;
    rtree -> root = new_root;
    rtree -> root -> count = 2;
  }

}

/**
 * @ingroup meos_stbox_rtree_index
 * @brief Creates an RTree index. 
 * @note the get axis function is to facilitate STBox having only 
 * some data available i.e. z is optional.
 * @param[in] get_axis function that given an stbox, a dimension and whether
 * upper or lower, returns the STBox value at that dimension in the lower or 
 * uppper value.
 * @param[in] dims The number of axis.@
 * @return RTree initialized.
 */
RTree *
  rtree_create(double( * get_axis)(const STBox * , int, bool), int dims) {
    RTree * rtree = malloc(sizeof(RTree));
    memset(rtree, 0, sizeof(RTree));
    rtree -> dims = dims;
    rtree -> get_axis = get_axis;
    return rtree;
  }

/**
 * @ingroup meos_stbox_rtree_index
 * @brief Queries an RTree with an STBox. Returns an array of ids of STBoxes.
 * @note The @p count will be the output size of the array given.
 * @param[in] rtree The RTree to query.
 * @param[in] query The STBox that serves as query.
 * @param[in] count An int that corresponds to the number of hits the RTree found.
 * @return array of ids that have a hit.
 */
int *
  rtree_search(const RTree * rtree,
    const STBox * query, int * count) {
    int ** ids = malloc(sizeof(int * ));
    * ids = malloc(sizeof(int) * 64);
    if (rtree -> root) {
      node_search(rtree -> root, query, ids, count);
    }
    return * ids;
  }

/**
 * @ingroup meos_stbox_rtree_index
 * @brief Frees the RTree
 * @param[in] rtree The RTree to free.
 */
void
rtree_free(RTree * rtree) {
  if (rtree -> root) {
    node_free(rtree -> root);
  }
  free(rtree);
}
