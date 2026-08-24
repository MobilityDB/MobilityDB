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
 * @brief In-memory space-partitioning index (quad-tree and k-d tree) for MEOS
 * bounding boxes, i.e., for Span and TBox
 */

#ifndef __TEMPORAL_SPTREE__
#define __TEMPORAL_SPTREE__

/* MEOS */
#include <meos.h>

#include "temporal/meos_catalog.h"

/*****************************************************************************
 * SPTree
 *****************************************************************************/

/**
 * @brief Maximum size in bytes of an inner node region (SpanNode, TboxNode)
 */
#define SPTREE_NODEBOX_MAXSIZE 512

/**
 * @brief Node of an in-memory space-partitioning tree
 * @details A node is a stored bounding box (the @p centroid) that splits the
 * space into @p nchild children; the box lives in the flexible tail. An empty
 * child slot is a @p NULL pointer.
 */
typedef struct SPNode
{
  int64 id;                   /**< Identifier of the box stored at this node */
  struct SPNode **children;   /**< Array of @p nchild child pointers, held
                                   only once the node gains a child */
  char centroid[];            /**< The bounding box of this node */
} SPNode;

/**
 * @brief In-memory space-partitioning index (quad-tree or k-d tree)
 * @details Works on Span and TBox. Each node is a data box that partitions the
 * space; the family-specific geometry (quadrant assignment, child region, and
 * consistency tests) is provided through function pointers, reusing the same
 * routines as the SP-GiST operator classes.
 */
struct SPTree
{
  MeosType bboxtype;    /**< Type of the bounding box */
  MeosType spantype;    /**< Span type (span box types only) */
  MeosType basetype;    /**< Base type (span box types only) */
  size_t boxsize;       /**< Size of a bounding box */
  size_t nodeboxsize;   /**< Size of an inner node region */
  int dims;             /**< Number of dimensions of the box */
  int nchild;           /**< Number of children per node */
  const uint8 *kd_bits; /**< For a k-d tree, the quadrant bit carrying each
                             dimension, in the order @p kdtree_next narrows
                             them. A level must store a box under the bit of
                             the dimension its region is then narrowed on, or
                             the search prunes the subtrees holding the
                             matches. */
  SPTreeKind kind;      /**< Quad-tree or k-d tree */
  SPNode *root;         /**< Root node, or @p NULL when empty */
  int (*box_dims)(const void *box);  /**< Dimensions of a box, or @p NULL when
                                          fixed at creation */
  void (*project)(const void *in, void *out);  /**< Project an incoming box
                                          into the internal box type, or
                                          @p NULL when boxes are stored as
                                          received */
  uint8 (*get_quadrant)(const void *centroid, const void *key);
  void (*nodebox_init)(void *nodebox, const void *centroid,
    const struct SPTree *sptree);
  void (*quadtree_next)(const void *nodebox, const void *centroid,
    uint8 quadrant, void *next);
  void (*kdtree_next)(const void *nodebox, const void *centroid, uint8 node,
    int level, void *next);
  bool (*inner_consistent)(const void *nodebox, const void *query,
    IndexSearchOp op);
  bool (*leaf_consistent)(const void *key, const void *query, IndexSearchOp op);
};

/*****************************************************************************/

#endif /* __TEMPORAL_SPTREE__ */
