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
 * @brief In memory index for STBox based on RTree
 */

#ifndef __TEMPORAL_RTREE__
#define __TEMPORAL_RTREE__

/* MEOS */
#include <meos.h>

#include "temporal/meos_catalog.h"

/*****************************************************************************
 * RTree
 *****************************************************************************/

#define MAXITEMS 64
#define SEARCH_ARRAY_STARTING_SIZE 64
#define MINITEMS_PERCENTAGE 10
#define MINITEMS ((MAXITEMS) * (MINITEMS_PERCENTAGE) / 100 + 1)

/**
 * @brief Enumeration that defines the node types for an RTree.
 */
typedef enum
{
  RTREE_NODE_ROOT =     0,
  RTREE_NODE_INNER =    1,
  RTREE_NODE_LEAF =     2,
} RTREE_NODE_TYPE;

/**
 * @brief Internal representation of an RTree node.
 */
typedef struct RTreeNode
{
  size_t bboxsize;            /**< Size of the bouding box */
  int count;                  /**< Number of bouding boxes */
  RTREE_NODE_TYPE node_type;
  union 
  {
    struct RTreeNode *nodes[MAXITEMS];
    int64 ids[MAXITEMS];
  };
  /* The bounding boxes can be of type Span, TBox, or STBox */
  char boxes[];
} RTreeNode;

/**
 * @brief Rtree in-memory index basic structure.
 * @details It works based on Span, TBox and STBox. 
 * - The spliting criteria is based on the largest axis. 
 * - The inserting criteria is based on least enlarging square.
 * - The get axis function makes it ease to implement with X,Y,Z and time or any
 *   combination that you may want.
 */
struct RTree
{
  size_t bboxsize;       /**< Size of the bouding box */
  meosType bboxtype;     /**< Type of the bouding box */
  int dims;
  RTreeNode *root;
  double (*get_axis)(const void *, int, bool);
  void (*bbox_expand)(const void *, void *);
  bool (*bbox_contains)(const void *, const void *);
  bool (*bbox_overlaps)(const void *, const void *);
  char box[];
};

/**
 * @brief Return a pointer to the n-th bounding box of a node
 * @details The bouding boxes of a node can be of type Span, TBox, or STBox
 */
#define RTREE_NODE_BBOX_N(node, n) ( (void *)( \
  ((char *) &((node)->boxes)) + (n) * (node)->bboxsize ) )

/*****************************************************************************/

#endif /* __TEMPORAL_RTREE__ */
