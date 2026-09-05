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
 * @brief In-memory space-partitioning index (quad-tree and k-d tree) for MEOS
 * bounding boxes, i.e., for Span and TBox
 * @details Each node is a data box (its centroid) that partitions the space
 * into `2^dims` quadrants (quad-tree) or two halves alternating one dimension
 * per level (k-d tree). Insertion descends to an empty child slot and stores
 * the box there; a search descends every child whose region is consistent with
 * the query, collecting the ids of the stored boxes that match. The family
 * geometry — quadrant assignment, child region, and the consistency tests — is
 * the same as the SP-GiST operator classes, reused through function pointers.
 */

/* C */
#include <stdlib.h>
#include <limits.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#if POINTCLOUD
  #include <meos_pointcloud.h>
  #include "pointcloud/tpc_boxops.h"
#endif
#include "temporal/temporal.h"
#include "temporal/span_index.h"
#include "temporal/tbox_index.h"
#include "geo/stbox_index.h"
#include "geo/geo_funcs.h"
#include "temporal/temporal_sptree.h"

/*****************************************************************************
 * k-d split dimensions
 *
 * A k-d level stores a box under one bit of its quadrant code, and the search
 * then narrows the node region on one dimension. The two must be the same
 * dimension: the tables below give, for each family, the quadrant bit carrying
 * the dimension that `*node_kdtree_next` narrows at that level.
 *
 * The bits are those the matching getQuadrant assigns, and the order is the one
 * the matching *node_kdtree_next walks, so the two orders being different — as
 * they are for the spatiotemporal box, whose bits run Z, Y, X, period while its
 * search narrows X, Y, Z, period — is carried by the table rather than by a
 * rule. `*node_kdtree_next` is shared with the SP-GiST operator classes, whose
 * descent is PostgreSQL's, so the order it walks is fixed and this side adapts.
 *****************************************************************************/

/* lower, upper */
static const uint8 SPAN_KD_BITS[2] = {1, 0};
/* span.lower, span.upper, period.lower, period.upper */
static const uint8 TBOX_KD_BITS[4] = {3, 2, 1, 0};
/* xmin, xmax, ymin, ymax, zmin, zmax, period.lower, period.upper */
static const uint8 STBOX_KD_BITS_Z[8] = {3, 2, 5, 4, 7, 6, 1, 0};
/* xmin, xmax, ymin, ymax, period.lower, period.upper */
static const uint8 STBOX_KD_BITS[6] = {3, 2, 5, 4, 1, 0};

/*****************************************************************************
 * Family-specific adapters (Span)
 *****************************************************************************/

static uint8
span_get_quadrant(const void *centroid, const void *key)
{
  return getQuadrant2D((const Span *) centroid, (const Span *) key);
}

static void
span_nodebox_init(void *nodebox, const void *centroid UNUSED,
  const struct SPTree *sptree)
{
  spannode_init((SpanNode *) nodebox, sptree->spantype, sptree->basetype);
}

static void
span_quadtree_next(const void *nodebox, const void *centroid, uint8 quadrant,
  void *next)
{
  spannode_quadtree_next((const SpanNode *) nodebox, (const Span *) centroid,
    quadrant, (SpanNode *) next);
}

static void
span_kdtree_next(const void *nodebox, const void *centroid, uint8 node,
  int level, void *next)
{
  spannode_kdtree_next((const SpanNode *) nodebox, (const Span *) centroid,
    node, level, (SpanNode *) next);
}

static bool
span_inner_consistent(const void *nodebox, const void *query, IndexSearchOp op)
{
  const SpanNode *n = (const SpanNode *) nodebox;
  const Span *q = (const Span *) query;
  switch (op)
  {
    case INDEX_CONTAINS:
    case INDEX_SAME:
      return contain2D(n, q);
    case INDEX_ADJACENT:
      /* A span meeting another at an excluded bound shares a boundary with it
       * and no point of it, so overlap alone would prune the very entries an
       * adjacency search looks for */
      return adjacent2D(n, q) || overlap2D(n, q);
    case INDEX_LEFT:      return ! overRight2D(n, q);
    case INDEX_OVERLEFT:  return ! right2D(n, q);
    case INDEX_RIGHT:     return ! overLeft2D(n, q);
    case INDEX_OVERRIGHT: return ! left2D(n, q);
    default:
      /* INDEX_OVERLAPS and INDEX_CONTAINED_BY prune on overlap */
      return overlap2D(n, q);
  }
}

/**
 * @brief Return true if a leaf entry satisfies the search operation
 * @details A tree stores homogeneous spans -- same span type by construction,
 * which #ensure_valid_sptree_box establishes at the entry point -- so the
 * internal predicates, which assert that contract, are what the descent calls
 * @param[in] key,query Spans
 * @param[in] op Search operation
 */
static bool
span_leaf_consistent(const void *key, const void *query, IndexSearchOp op)
{
  const Span *k = (const Span *) key;
  const Span *q = (const Span *) query;
  switch (op)
  {
    case INDEX_CONTAINS:      return span_contains(k, q);
    case INDEX_CONTAINED_BY:  return span_contains(q, k);
    case INDEX_SAME:          return span_contains(k, q) && span_contains(q, k);
    case INDEX_ADJACENT:      return span_adjacent(k, q);
    case INDEX_OVERLAPS:      return span_overlaps(k, q);
    case INDEX_LEFT:          return span_left(k, q);
    case INDEX_OVERLEFT:      return span_overleft(k, q);
    case INDEX_RIGHT:         return span_right(k, q);
    case INDEX_OVERRIGHT:     return span_overright(k, q);
    default:
      /* A span has one dimension, which the value operations order */
      return false;
  }
}

/*****************************************************************************
 * Family-specific adapters (TBox)
 *****************************************************************************/

static uint8
tbox_get_quadrant(const void *centroid, const void *key)
{
  return getQuadrant4D((const TBox *) centroid, (const TBox *) key);
}

static void
tbox_nodebox_init(void *nodebox, const void *centroid,
  const struct SPTree *sptree UNUSED)
{
  tboxnode_init((TBox *) centroid, (TboxNode *) nodebox);
}

static void
tbox_quadtree_next(const void *nodebox, const void *centroid, uint8 quadrant,
  void *next)
{
  tboxnode_quadtree_next((const TboxNode *) nodebox, (const TBox *) centroid,
    quadrant, (TboxNode *) next);
}

static void
tbox_kdtree_next(const void *nodebox, const void *centroid, uint8 node,
  int level, void *next)
{
  tboxnode_kdtree_next((const TboxNode *) nodebox, (const TBox *) centroid,
    node, level, (TboxNode *) next);
}

static bool
tbox_inner_consistent(const void *nodebox, const void *query, IndexSearchOp op)
{
  const TboxNode *n = (const TboxNode *) nodebox;
  const TBox *q = (const TBox *) query;
  switch (op)
  {
    case INDEX_CONTAINS:
    case INDEX_SAME:
      return contain4D(n, q);
    case INDEX_LEFT:       return ! overRight4D(n, q);
    case INDEX_OVERLEFT:   return ! right4D(n, q);
    case INDEX_RIGHT:      return ! overLeft4D(n, q);
    case INDEX_OVERRIGHT:  return ! left4D(n, q);
    case INDEX_BEFORE:     return ! overAfter4D(n, q);
    case INDEX_OVERBEFORE: return ! after4D(n, q);
    case INDEX_AFTER:      return ! overBefore4D(n, q);
    case INDEX_OVERAFTER:  return ! before4D(n, q);
    default:
      /* INDEX_OVERLAPS, INDEX_CONTAINED_BY and INDEX_ADJACENT prune on
       * overlap, the region being compared on its bounds rather than on the
       * spans holding them, so a region meeting the query overlaps it here */
      return overlap4D(n, q);
  }
}

static bool
tbox_leaf_consistent(const void *key, const void *query, IndexSearchOp op)
{
  const TBox *k = (const TBox *) key;
  const TBox *q = (const TBox *) query;
  switch (op)
  {
    case INDEX_CONTAINS:      return contains_tbox_tbox(k, q);
    case INDEX_CONTAINED_BY:  return contains_tbox_tbox(q, k);
    case INDEX_SAME:          return contains_tbox_tbox(k, q) && contains_tbox_tbox(q, k);
    case INDEX_ADJACENT:      return adjacent_tbox_tbox(k, q);
    case INDEX_OVERLAPS:      return overlaps_tbox_tbox(k, q);
    case INDEX_LEFT:          return left_tbox_tbox(k, q);
    case INDEX_OVERLEFT:      return overleft_tbox_tbox(k, q);
    case INDEX_RIGHT:         return right_tbox_tbox(k, q);
    case INDEX_OVERRIGHT:     return overright_tbox_tbox(k, q);
    case INDEX_BEFORE:        return before_tbox_tbox(k, q);
    case INDEX_OVERBEFORE:    return overbefore_tbox_tbox(k, q);
    case INDEX_AFTER:         return after_tbox_tbox(k, q);
    case INDEX_OVERAFTER:     return overafter_tbox_tbox(k, q);
    default:
      /* A temporal box has no spatial dimension to be below or in front of */
      return false;
  }
}

/*****************************************************************************
 * Family-specific adapters (STBox)
 *****************************************************************************/

static int
stbox_box_dims(const void *box)
{
  return MEOS_FLAGS_GET_Z(((const STBox *) box)->flags) ? 8 : 6;
}

static uint8
stbox_get_quadrant(const void *centroid, const void *key)
{
  return getQuadrant8D((const STBox *) centroid, (const STBox *) key);
}

static void
stbox_nodebox_init(void *nodebox, const void *centroid,
  const struct SPTree *sptree UNUSED)
{
  stboxnode_init((const STBox *) centroid, (STboxNode *) nodebox);
}

static void
stbox_quadtree_next(const void *nodebox, const void *centroid, uint8 quadrant,
  void *next)
{
  stboxnode_quadtree_next((const STboxNode *) nodebox, (const STBox *) centroid,
    quadrant, (STboxNode *) next);
}

static void
stbox_kdtree_next(const void *nodebox, const void *centroid, uint8 node,
  int level, void *next)
{
  stboxnode_kdtree_next((const STboxNode *) nodebox, (const STBox *) centroid,
    node, level, (STboxNode *) next);
}

static bool
stbox_inner_consistent(const void *nodebox, const void *query, IndexSearchOp op)
{
  const STboxNode *n = (const STboxNode *) nodebox;
  const STBox *q = (const STBox *) query;
  switch (op)
  {
    case INDEX_CONTAINS:
    case INDEX_SAME:
      return contain8D(n, q);
    case INDEX_OVERLAPS:
    case INDEX_CONTAINED_BY:
    case INDEX_ADJACENT:
      /* The region is compared on its bounds rather than on the spans holding
       * them, so a region meeting the query is a region overlapping it here */
      return overlap8D(n, q);
    /* A region can hold a box on one side of the query only when the region
     * itself reaches that side, which is the negation of the opposite
     * relation. The same descent the SP-GiST operator classes make */
    case INDEX_LEFT:       return ! overRight8D(n, q);
    case INDEX_OVERLEFT:   return ! right8D(n, q);
    case INDEX_RIGHT:      return ! overLeft8D(n, q);
    case INDEX_OVERRIGHT:  return ! left8D(n, q);
    case INDEX_BELOW:      return ! overAbove8D(n, q);
    case INDEX_OVERBELOW:  return ! above8D(n, q);
    case INDEX_ABOVE:      return ! overBelow8D(n, q);
    case INDEX_OVERABOVE:  return ! below8D(n, q);
    case INDEX_FRONT:      return ! overBack8D(n, q);
    case INDEX_OVERFRONT:  return ! back8D(n, q);
    case INDEX_BACK:       return ! overFront8D(n, q);
    case INDEX_OVERBACK:   return ! front8D(n, q);
    case INDEX_BEFORE:     return ! overAfter8D(n, q);
    case INDEX_OVERBEFORE: return ! after8D(n, q);
    case INDEX_AFTER:      return ! overBefore8D(n, q);
    case INDEX_OVERAFTER:  return ! before8D(n, q);
  }
  return false;
}

static bool
stbox_leaf_consistent(const void *key, const void *query, IndexSearchOp op)
{
  const STBox *k = (const STBox *) key;
  const STBox *q = (const STBox *) query;
  switch (op)
  {
    case INDEX_CONTAINS:      return contains_stbox_stbox(k, q);
    case INDEX_CONTAINED_BY:  return contains_stbox_stbox(q, k);
    case INDEX_SAME:          return contains_stbox_stbox(k, q) && contains_stbox_stbox(q, k);
    case INDEX_ADJACENT:      return adjacent_stbox_stbox(k, q);
    case INDEX_OVERLAPS:      return overlaps_stbox_stbox(k, q);
    case INDEX_LEFT:          return left_stbox_stbox(k, q);
    case INDEX_OVERLEFT:      return overleft_stbox_stbox(k, q);
    case INDEX_RIGHT:         return right_stbox_stbox(k, q);
    case INDEX_OVERRIGHT:     return overright_stbox_stbox(k, q);
    case INDEX_BELOW:         return below_stbox_stbox(k, q);
    case INDEX_OVERBELOW:     return overbelow_stbox_stbox(k, q);
    case INDEX_ABOVE:         return above_stbox_stbox(k, q);
    case INDEX_OVERABOVE:     return overabove_stbox_stbox(k, q);
    case INDEX_FRONT:         return front_stbox_stbox(k, q);
    case INDEX_OVERFRONT:     return overfront_stbox_stbox(k, q);
    case INDEX_BACK:          return back_stbox_stbox(k, q);
    case INDEX_OVERBACK:      return overback_stbox_stbox(k, q);
    case INDEX_BEFORE:        return before_stbox_stbox(k, q);
    case INDEX_OVERBEFORE:    return overbefore_stbox_stbox(k, q);
    case INDEX_AFTER:         return after_stbox_stbox(k, q);
    case INDEX_OVERAFTER:     return overafter_stbox_stbox(k, q);
  }
  return false;
}

#if POINTCLOUD
/*****************************************************************************
 * Family-specific adapters (TPCBox)
 *
 * A TPCBox carries the same spatial and temporal bounds as an STBox plus a
 * trailing pcid, so a tpcbox tree is internally an STBox tree: every incoming
 * box and query is projected to an STBox on entry through #tpcbox_to_stbox
 * and the STBox adapters above do the rest. Only the projection is specific
 * to the type.
 *****************************************************************************/

/**
 * @brief Project a TPCBox onto an STBox held in caller-provided memory
 */
static void
tpcbox_project(const void *in, void *out)
{
  STBox *box = tpcbox_to_stbox((const TPCBox *) in);
  memcpy(out, box, sizeof(STBox));
  pfree(box);
  return;
}
#endif /* POINTCLOUD */

/*****************************************************************************
 * Creation
 *****************************************************************************/

/**
 * @brief Create an in-memory space-partitioning index for a bounding box type
 * @param[in] bboxtype Type of the bounding box (a span type or @p T_TBOX)
 * @param[in] kind Quad-tree or k-d tree
 * @return SPTree initialized
 */
SPTree *
sptree_create(MeosType bboxtype, SPTreeKind kind)
{
  assert(span_type(bboxtype) || bboxtype == T_TBOX || bboxtype == T_STBOX
#if POINTCLOUD
    || bboxtype == T_TPCBOX
#endif
    );
  SPTree *sptree = palloc0(sizeof(SPTree));
  sptree->bboxtype = bboxtype;
  sptree->boxsize = bbox_get_size(bboxtype);
  sptree->kind = kind;
  sptree->root = NULL;
  if (span_type(bboxtype))
  {
    sptree->spantype = bboxtype;
    sptree->basetype = spantype_basetype(bboxtype);
    sptree->dims = 2;
    sptree->kd_bits = SPAN_KD_BITS;
    sptree->nodeboxsize = sizeof(SpanNode);
    sptree->get_quadrant = &span_get_quadrant;
    sptree->nodebox_init = &span_nodebox_init;
    sptree->quadtree_next = &span_quadtree_next;
    sptree->kdtree_next = &span_kdtree_next;
    sptree->inner_consistent = &span_inner_consistent;
    sptree->leaf_consistent = &span_leaf_consistent;
  }
  else if (bboxtype == T_TBOX)
  {
    sptree->dims = 4;
    sptree->kd_bits = TBOX_KD_BITS;
    sptree->nodeboxsize = sizeof(TboxNode);
    sptree->get_quadrant = &tbox_get_quadrant;
    sptree->nodebox_init = &tbox_nodebox_init;
    sptree->quadtree_next = &tbox_quadtree_next;
    sptree->kdtree_next = &tbox_kdtree_next;
    sptree->inner_consistent = &tbox_inner_consistent;
    sptree->leaf_consistent = &tbox_leaf_consistent;
  }
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
  {
    /* A tpcbox tree is internally an STBox tree: boxes and queries are
     * projected through #tpcbox_project on entry, so the box storage, the
     * node regions and every adapter are the STBox ones */
    sptree->boxsize = sizeof(STBox);
    sptree->dims = -1;
    sptree->nodeboxsize = sizeof(STboxNode);
    sptree->project = &tpcbox_project;
    sptree->box_dims = &stbox_box_dims;
    sptree->get_quadrant = &stbox_get_quadrant;
    sptree->nodebox_init = &stbox_nodebox_init;
    sptree->quadtree_next = &stbox_quadtree_next;
    sptree->kdtree_next = &stbox_kdtree_next;
    sptree->inner_consistent = &stbox_inner_consistent;
    sptree->leaf_consistent = &stbox_leaf_consistent;
  }
#endif /* POINTCLOUD */
  else /* bboxtype == T_STBOX */
  {
    /* The dimensions (6 for 2D+T, 8 for 3D+T) and hence the number of children
     * are determined at the first insertion from the Z flag of the box */
    sptree->dims = -1;
    sptree->nodeboxsize = sizeof(STboxNode);
    sptree->box_dims = &stbox_box_dims;
    sptree->get_quadrant = &stbox_get_quadrant;
    sptree->nodebox_init = &stbox_nodebox_init;
    sptree->quadtree_next = &stbox_quadtree_next;
    sptree->kdtree_next = &stbox_kdtree_next;
    sptree->inner_consistent = &stbox_inner_consistent;
    sptree->leaf_consistent = &stbox_leaf_consistent;
  }
  sptree->nchild = (sptree->dims < 0) ? -1 :
    ((kind == SPTREE_QUADTREE) ? (1 << sptree->dims) : 2);
  assert(sptree->nodeboxsize <= SPTREE_NODEBOX_MAXSIZE);
  return sptree;
}

/**
 * @ingroup meos_misc
 * @brief Create an in-memory space-partitioning index for integer spans
 * @param[in] kind Quad-tree or k-d tree
 * @return SPTree initialized
 */
SPTree *
sptree_create_intspan(SPTreeKind kind)
{
  return sptree_create(T_INTSPAN, kind);
}

/**
 * @ingroup meos_misc
 * @brief Create an in-memory space-partitioning index for big integer spans
 * @param[in] kind Quad-tree or k-d tree
 * @return SPTree initialized
 */
SPTree *
sptree_create_bigintspan(SPTreeKind kind)
{
  return sptree_create(T_BIGINTSPAN, kind);
}

/**
 * @ingroup meos_misc
 * @brief Create an in-memory space-partitioning index for float spans
 * @param[in] kind Quad-tree or k-d tree
 * @return SPTree initialized
 */
SPTree *
sptree_create_floatspan(SPTreeKind kind)
{
  return sptree_create(T_FLOATSPAN, kind);
}

/**
 * @ingroup meos_misc
 * @brief Create an in-memory space-partitioning index for date spans
 * @param[in] kind Quad-tree or k-d tree
 * @return SPTree initialized
 */
SPTree *
sptree_create_datespan(SPTreeKind kind)
{
  return sptree_create(T_DATESPAN, kind);
}

/**
 * @ingroup meos_misc
 * @brief Create an in-memory space-partitioning index for timestamptz spans
 * @param[in] kind Quad-tree or k-d tree
 * @return SPTree initialized
 */
SPTree *
sptree_create_tstzspan(SPTreeKind kind)
{
  return sptree_create(T_TSTZSPAN, kind);
}

/**
 * @ingroup meos_misc
 * @brief Create an in-memory space-partitioning index for temporal boxes
 * @param[in] kind Quad-tree or k-d tree
 * @return SPTree initialized
 */
SPTree *
sptree_create_tbox(SPTreeKind kind)
{
  return sptree_create(T_TBOX, kind);
}

/**
 * @ingroup meos_misc
 * @brief Create an in-memory space-partitioning index for spatiotemporal boxes
 * @param[in] kind Quad-tree or k-d tree
 * @return SPTree initialized
 */
SPTree *
sptree_create_stbox(SPTreeKind kind)
{
  return sptree_create(T_STBOX, kind);
}

#if POINTCLOUD
/**
 * @ingroup meos_pointcloud_box
 * @brief Create an in-memory space-partitioning index for the @c tpcbox
 * bounding-box type
 * @details The boxes are projected to spatiotemporal boxes on entry, so the
 * tree reuses the spatiotemporal box partitioning unchanged. Pair with
 * @ref sptree_insert / @ref sptree_insert_temporal to populate and
 * @ref sptree_search / @ref sptree_search_temporal to query.
 * @param[in] kind Quad-tree or k-d tree
 * @return SPTree initialized
 */
SPTree *
sptree_create_tpcbox(SPTreeKind kind)
{
  return sptree_create(T_TPCBOX, kind);
}
#endif /* POINTCLOUD */

/*****************************************************************************
 * Insertion
 *****************************************************************************/

/**
 * @brief Return a new node holding a bounding box
 * @details The node holds no child slots. A node acquires them when a descent
 * first passes through it, so a node that never gains a child never holds one.
 */
static SPNode *
spnode_make(const SPTree *sptree, const void *box, int64 id)
{
  SPNode *node = palloc0(sizeof(SPNode) + sptree->boxsize);
  node->id = id;
  memcpy(node->centroid, box, sptree->boxsize);
  return node;
}

/**
 * @brief Return the child slots of a node, creating them on first use
 * @details The slots a node holds are as many as the quadrants of the space it
 * partitions, which for an STBox is 64 at 6 dimensions and 256 at 8, while the
 * nodes a tree ends in hold none of them. Reading the slots of a node that has
 * no child therefore spends the whole array on nothing, so the array is
 * created by the descent that places the first child rather than by the node.
 */
static SPNode **
spnode_children(const SPTree *sptree, SPNode *node)
{
  if (! node->children)
    node->children = palloc0((size_t) sptree->nchild * sizeof(SPNode *));
  return node->children;
}

/**
 * @brief Return the number of dimensions a box type carries, for a type that
 * determines it from the data rather than from the type
 * @details An STBox tree partitions on 6 dimensions for 2D+T and on 8 for
 * 3D+T, so the dimensions, the bit each level narrows and the number of
 * children per node are all read from the first box the tree receives.
 */
static void
sptree_set_dims(SPTree *sptree, const void *box)
{
  sptree->dims = sptree->box_dims(box);
  sptree->kd_bits = (sptree->dims == 8) ? STBOX_KD_BITS_Z : STBOX_KD_BITS;
  sptree->nchild = (sptree->kind == SPTREE_QUADTREE) ? (1 << sptree->dims) : 2;
  return;
}

/**
 * @brief Return the child slot a box occupies under a node at a given level
 * @details A quad-tree slot is the whole quadrant of the box with respect to
 * the centroid. A k-d tree slot is the single bit of that quadrant carrying
 * the dimension the search narrows at this level, so that the region a search
 * descends into is the region the box was partitioned by.
 * @param[in] sptree The SPTree
 * @param[in] centroid The bounding box held by the node
 * @param[in] box The bounding box being placed or sought
 * @param[in] level The depth of the node
 */
static int
spnode_child(const SPTree *sptree, const void *centroid, const void *box,
  int level)
{
  uint8 quadrant = sptree->get_quadrant(centroid, box);
  return (sptree->kind == SPTREE_QUADTREE) ? (int) quadrant :
    (int) ((quadrant >> sptree->kd_bits[level % sptree->dims]) & 1);
}

/**
 * @brief Return true if a box may enter or query an SPTree, report an error
 * otherwise
 * @details A tree holds boxes of one SRID: the first entry fixes it, and a box
 * of another one is an error rather than a coercion. The check belongs here,
 * at the entry point, so that every comparison the descent makes can assume it
 */
static bool
ensure_valid_sptree_box(const SPTree *sptree, const void *box)
{
  if (! sptree->root)
    return true;
  if (sptree->bboxtype == T_STBOX)
    return ensure_same_srid(((const STBox *) sptree->root->centroid)->srid,
      ((const STBox *) box)->srid);
#if POINTCLOUD
  if (sptree->bboxtype == T_TPCBOX)
  {
    /* A tpcbox tree holds its centroids already projected to STBox, so only
     * the incoming box is projected here */
    STBox s;
    tpcbox_set_stbox((const TPCBox *) box, &s);
    return ensure_same_srid(((const STBox *) sptree->root->centroid)->srid,
      s.srid);
  }
#endif /* POINTCLOUD */
  return true;
}

/**
 * @brief Return true if two SPTrees may be joined, report an error otherwise
 * @details A join compares a box of one tree with a box of the other, so the
 * two trees answer the same question #ensure_valid_sptree_box asks of a query.
 * Both trees hold their centroids in the internal box type, a tpcbox tree
 * already projected to an @p STBox, so the two SRIDs compare directly rather
 * than through that function's projection of an incoming box
 */
static bool
ensure_valid_sptree_sptree(const SPTree *sptree1, const SPTree *sptree2)
{
  if (! sptree1->root || ! sptree2->root)
    return true;
  if (sptree1->bboxtype == T_STBOX
#if POINTCLOUD
      || sptree1->bboxtype == T_TPCBOX
#endif /* POINTCLOUD */
     )
    return ensure_same_srid(((const STBox *) sptree1->root->centroid)->srid,
      ((const STBox *) sptree2->root->centroid)->srid);
  return true;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Insert a bounding box into an in-memory space-partitioning index
 * @details The box is stored at the first empty child slot reached while
 * descending from the root by quadrant. Equal boxes chain through the first
 * child, so the search still finds every id.
 *
 * The depth a box reaches depends on the order the boxes arrive in, so a tree
 * grown from an ordered set is deep and prunes little. #sptree_load builds the
 * same index from a whole entry set at once and chooses the depth itself.
 * @param[in] sptree The SPTree previously initialized
 * @param[in] box The bounding box to insert
 * @param[in] id The id associated with the box
 */
bool
sptree_insert(SPTree *sptree, void *box, int64 id)
{
  /* Ensure the validity of the arguments */
  if (! ensure_not_null((void *) sptree) || ! ensure_not_null((void *) box) ||
      ! ensure_valid_sptree_box(sptree, box))
    return false;

  /* Project the incoming box into the internal box type (TPCBox: STBox) */
  bboxunion proj;
  if (sptree->project)
  {
    sptree->project(box, &proj);
    box = &proj;
  }
  if (sptree->dims < 0)
    sptree_set_dims(sptree, box);
  SPNode **slot = &sptree->root;
  int level = 0;
  while (*slot != NULL)
  {
    SPNode *cur = *slot;
    slot = &spnode_children(sptree, cur)[spnode_child(sptree, cur->centroid,
      box, level)];
    level++;
  }
  *slot = spnode_make(sptree, box, id);
  return true;
}

/*****************************************************************************
 * Build from a whole entry set
 *****************************************************************************/

/* The build releases the nodes the tree holds, which the recursive release
 * below it does */
static void spnode_free(const SPTree *sptree, SPNode *node);

/**
 * @brief The entries a build is arranging, and the scratch it arranges them in
 * @details The boxes are held in the internal box type and are permuted in
 * place, each permutation carrying its id along, so that the entries of a
 * subtree occupy a contiguous range and a node is described by a position and
 * a count.
 */
typedef struct
{
  SPTree *sptree;
  char *boxes;        /**< The entries, in the internal box type */
  int64 *ids;         /**< The id of each entry, permuted with the boxes */
  void *tmpbox;       /**< Holds one box while two entries are exchanged */
  void *pivotbox;     /**< Holds the box a selection compares against */
  char *scratchboxes; /**< Receives one range while it is being bucketed */
  int64 *scratchids;
} SPBuild;

/**
 * @brief Return the address of the n-th box of an array of internal boxes
 */
static void *
spbox_n(const SPTree *sptree, char *boxes, int n)
{
  return (void *) (boxes + (size_t) n * sptree->boxsize);
}

/**
 * @brief Exchange two entries
 */
static void
spitems_swap(SPBuild *build, int i, int j)
{
  const SPTree *sptree = build->sptree;
  if (i == j)
    return;
  void *boxi = spbox_n(sptree, build->boxes, i);
  void *boxj = spbox_n(sptree, build->boxes, j);
  memcpy(build->tmpbox, boxi, sptree->boxsize);
  memcpy(boxi, boxj, sptree->boxsize);
  memcpy(boxj, build->tmpbox, sptree->boxsize);
  int64 id = build->ids[i];
  build->ids[i] = build->ids[j];
  build->ids[j] = id;
  return;
}

/**
 * @brief Return true if @p box is above @p base on the dimension a quadrant
 * bit carries
 * @details The order is the one the tree itself partitions by: the quadrant of
 * @p box with respect to @p base carries the bit exactly when @p box is above
 * @p base on that dimension. Reading it in both directions distinguishes the
 * boxes that tie on the dimension from those that are below.
 */
static bool
spbox_above(const SPTree *sptree, const void *box, const void *base, uint8 bit)
{
  return ((sptree->get_quadrant(base, box) >> bit) & 1) != 0;
}

/**
 * @brief Reorder a range so that the entry at position @p k is the one the
 * dimension of @p bit orders there
 * @details Every entry before @p k is then no higher on that dimension and
 * every entry after it is no lower, which is all a build needs to choose a
 * centroid that halves the range. The entries that tie with the one compared
 * against are gathered in one pass, so a range that ties throughout is
 * recognised at once rather than peeled one entry at a time.
 * @param[in] build The entries being arranged
 * @param[in] from,count The range
 * @param[in] k Position, relative to @p from, to settle
 * @param[in] bit Quadrant bit carrying the dimension to order on
 */
static void
spitems_select(SPBuild *build, int from, int count, int k, uint8 bit)
{
  const SPTree *sptree = build->sptree;
  int lo = from, hi = from + count - 1;
  k += from;
  while (lo < hi)
  {
    /* Compare against the middle entry of the range, held aside since its
     * position takes part in the exchanges below */
    spitems_swap(build, lo + (hi - lo) / 2, lo);
    memcpy(build->pivotbox, spbox_n(sptree, build->boxes, lo),
      sptree->boxsize);
    /* Gather the range into the entries below the one compared against, those
     * tying with it, and those above it */
    int lt = lo, i = lo, gt = hi;
    while (i <= gt)
    {
      const void *box = spbox_n(sptree, build->boxes, i);
      if (spbox_above(sptree, build->pivotbox, box, bit))
        spitems_swap(build, lt++, i++);
      else if (spbox_above(sptree, box, build->pivotbox, bit))
        spitems_swap(build, i, gt--);
      else
        i++;
    }
    if (k < lt)
      hi = lt - 1;
    else if (k > gt)
      lo = gt + 1;
    else
      return;
  }
  return;
}

/**
 * @brief Gather a range into contiguous buckets, one per child slot of a node
 * @param[in] build The entries being arranged
 * @param[in] from,count The range
 * @param[in] centroid The bounding box held by the node
 * @param[in] level The depth of the node
 * @param[out] counts Number of entries of each child slot, in slot order
 */
static void
spitems_bucket(SPBuild *build, int from, int count, const void *centroid,
  int level, int *counts)
{
  const SPTree *sptree = build->sptree;
  int nchild = sptree->nchild;
  memset(counts, 0, (size_t) nchild * sizeof(int));
  for (int i = 0; i < count; i++)
    counts[spnode_child(sptree, centroid,
      spbox_n(sptree, build->boxes, from + i), level)]++;
  /* The first position of each bucket, which filling it then advances */
  int *cursor = palloc((size_t) nchild * sizeof(int));
  int offset = 0;
  for (int c = 0; c < nchild; c++)
  {
    cursor[c] = offset;
    offset += counts[c];
  }
  for (int i = 0; i < count; i++)
  {
    const void *box = spbox_n(sptree, build->boxes, from + i);
    int pos = cursor[spnode_child(sptree, centroid, box, level)]++;
    memcpy(spbox_n(sptree, build->scratchboxes, pos), box, sptree->boxsize);
    build->scratchids[pos] = build->ids[from + i];
  }
  memcpy(spbox_n(sptree, build->boxes, from), build->scratchboxes,
    (size_t) count * sptree->boxsize);
  memcpy(build->ids + from, build->scratchids,
    (size_t) count * sizeof(int64));
  pfree(cursor);
  return;
}

/**
 * @brief Build the subtree holding a range of entries
 * @details The centroid is the entry the level's dimension orders in the
 * middle, so a k-d node hands half the range to each of its two children and
 * the depth is logarithmic in the number of entries whatever order they
 * arrive in. The remaining entries go to the child slot an insertion would
 * descend into at this level, which is what makes the result a tree the search
 * reads like any other.
 *
 * Entries that a slot cannot separate, equal boxes above all, descend as a
 * chain, exactly as inserting them one by one produces.
 * @param[in] build The entries being arranged
 * @param[in] from,count The range
 * @param[in] level The depth of the subtree root
 */
static SPNode *
spnode_build(SPBuild *build, int from, int count, int level)
{
  SPTree *sptree = build->sptree;
  if (count <= 0)
    return NULL;

  int mid = count / 2;
  spitems_select(build, from, count, mid,
    sptree->kd_bits[level % sptree->dims]);
  SPNode *node = spnode_make(sptree,
    spbox_n(sptree, build->boxes, from + mid), build->ids[from + mid]);

  /* The node holds a copy of its centroid, so the entry it was taken from
   * leaves the range through its end */
  spitems_swap(build, from + mid, from + count - 1);
  int rest = count - 1;
  if (rest > 0)
  {
    int *counts = palloc((size_t) sptree->nchild * sizeof(int));
    spitems_bucket(build, from, rest, node->centroid, level, counts);
    SPNode **children = spnode_children(sptree, node);
    int offset = 0;
    for (int c = 0; c < sptree->nchild; c++)
    {
      if (counts[c] > 0)
        children[c] = spnode_build(build, from + offset, counts[c],
          level + 1);
      offset += counts[c];
    }
    pfree(counts);
  }
  return node;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Build an in-memory space-partitioning index from all of its entries
 * at once
 * @details Top-down median partitioning. The result answers the same queries
 * as inserting every entry one by one, but the whole set is known in advance,
 * so each node halves what is left of its range on the dimension its level
 * narrows and the depth no longer depends on the order the entries arrive in.
 * The kind of tree given at creation is the kind that is built: a quad-tree
 * node hands the entries to one of its @p 2^dims quadrant slots, a k-d tree
 * node to one of its two.
 *
 * The tree ends up holding exactly the entries given, whatever it holds on
 * entry, so this is both the fast first build and the only way to make an
 * index smaller: an index has no removal entry point, and a caller that has to
 * drop entries rebuilds from the ones it keeps. Loading no entries leaves an
 * empty tree.
 * @param[in] sptree An SPTree of the appropriate bounding box type
 * @param[in] boxes Contiguous array of @p count boxes of the tree bbox size
 * @param[in] ids The id of each box
 * @param[in] count Number of entries
 */
bool
sptree_load(SPTree *sptree, const void *boxes, const int64 *ids, int count)
{
  /* Ensure the validity of the arguments */
  if (! ensure_not_null((void *) sptree) || ! ensure_not_null((void *) boxes) ||
      ! ensure_not_null((void *) ids) || ! ensure_valid_sptree_box(sptree, boxes))
    return false;

  /* The build assigns the root, so the nodes the tree holds are released
   * before it does, and are released whatever the number of entries given: an
   * empty entry set leaves an empty tree rather than the previous one */
  if (sptree->root)
  {
    spnode_free(sptree, sptree->root);
    sptree->root = NULL;
  }

  if (count <= 0)
    return true;

  /* A type indexed through a projection is given boxes of its own type and
   * holds them as the type it partitions (TPCBox: STBox), so the entries are
   * read at the stride of the type and held at the stride of the tree */
  size_t insize = bbox_get_size(sptree->bboxtype);
  SPBuild build;
  build.sptree = sptree;
  build.boxes = palloc((size_t) count * sptree->boxsize);
  for (int i = 0; i < count; i++)
  {
    const void *in = (const char *) boxes + (size_t) i * insize;
    void *out = spbox_n(sptree, build.boxes, i);
    if (sptree->project)
      sptree->project(in, out);
    else
      memcpy(out, in, sptree->boxsize);
  }

  if (sptree->dims < 0)
    sptree_set_dims(sptree, build.boxes);

  build.ids = palloc((size_t) count * sizeof(int64));
  memcpy(build.ids, ids, (size_t) count * sizeof(int64));
  build.tmpbox = palloc(sptree->boxsize);
  build.pivotbox = palloc(sptree->boxsize);
  build.scratchboxes = palloc((size_t) count * sptree->boxsize);
  build.scratchids = palloc((size_t) count * sizeof(int64));

  sptree->root = spnode_build(&build, 0, count, 0);

  pfree(build.scratchids);
  pfree(build.scratchboxes);
  pfree(build.pivotbox);
  pfree(build.tmpbox);
  pfree(build.ids);
  pfree(build.boxes);
  return true;
}

/*****************************************************************************
 * Search
 *****************************************************************************/

/**
 * @brief Recursively collect the ids of the boxes consistent with the query
 * @param[in] sptree The SPTree
 * @param[in] node The node being visited
 * @param[in] nodebox The region covered by @p node
 * @param[in] op The search operation
 * @param[in] query The query bounding box
 * @param[in] level The depth of @p node (drives the k-d tree dimension)
 * @param[out] result MeosArray collecting the matching ids
 */
static void
spnode_search(const SPTree *sptree, const SPNode *node, const void *nodebox,
  IndexSearchOp op, const void *query, int level, MeosArray *result)
{
  if (sptree->leaf_consistent(node->centroid, query, op))
  {
    int64 id = node->id;
    meos_array_add(result, &id);
  }
  for (int quadrant = 0; node->children && quadrant < sptree->nchild;
    quadrant++)
  {
    const SPNode *child = node->children[quadrant];
    if (! child)
      continue;
    char next[SPTREE_NODEBOX_MAXSIZE];
    if (sptree->kind == SPTREE_QUADTREE)
      sptree->quadtree_next(nodebox, node->centroid, (uint8) quadrant, next);
    else
      sptree->kdtree_next(nodebox, node->centroid, (uint8) quadrant, level,
        next);
    if (sptree->inner_consistent(next, query, op))
      spnode_search(sptree, child, next, op, query, level + 1, result);
  }
  return;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Search an in-memory space-partitioning index with a bounding box,
 * collecting matching ids into a MeosArray
 * @details The result array is reset before the search.
 * @param[in] sptree The SPTree to query
 * @param[in] op The search operation: @p INDEX_OVERLAPS finds boxes that
 * overlap the query, @p INDEX_CONTAINS finds boxes that contain the query,
 * @p INDEX_CONTAINED_BY finds boxes contained by the query
 * @param[in] query The bounding box that serves as query
 * @param[out] result MeosArray of int to collect matching ids
 * @return Number of matching ids
 */
int
sptree_search(const SPTree *sptree, IndexSearchOp op, const void *query,
  MeosArray *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(sptree, INT_MAX); VALIDATE_NOT_NULL(query, INT_MAX);
  VALIDATE_NOT_NULL(result, INT_MAX);
  if (! ensure_valid_sptree_box(sptree, query))
    return INT_MAX;

  /* Project the query box into the internal box type (TPCBox: STBox) */
  bboxunion proj;
  if (sptree->project)
  {
    sptree->project(query, &proj);
    query = &proj;
  }
  meos_array_reset(result);
  if (sptree->root)
  {
    char rootbox[SPTREE_NODEBOX_MAXSIZE];
    sptree->nodebox_init(rootbox, sptree->root->centroid, sptree);
    spnode_search(sptree, sptree->root, rootbox, op, query, 0, result);
  }
  return meos_array_count(result);
}

/*****************************************************************************
 * Join
 *****************************************************************************/

/**
 * @brief Return the operation read with its arguments exchanged
 * @details One index is searched for the entries an entry of the other pairs
 * with, and a search reads its own entry first, so the operation is turned
 * round
 */
static IndexSearchOp
index_op_converse(IndexSearchOp op)
{
  switch (op)
  {
    case INDEX_CONTAINS:     return INDEX_CONTAINED_BY;
    case INDEX_CONTAINED_BY: return INDEX_CONTAINS;
    default:                 return INDEX_OVERLAPS;
  }
}

/**
 * @brief Report the pairs every entry of a subtree makes with the entries of
 * another index
 * @details The entries of the subtree are walked, and each is used as the query
 * of a search of the other index, which prunes the subtrees its region excludes
 * exactly as any search does. The operation is read from the side the search
 * stands on, so the pairs it collects are turned round: the entry of the first
 * index opens each pair.
 *
 * A space-partitioning index stores a box at every node and not only at its
 * leaves, so the two indexes are not descended together. A pair descent visits
 * the node pairs of equal depth, which leaves the pairs of two entries of
 * unequal depth to a probe made at every node pair it reaches, and the node
 * pairs are the product of the two levels where the entries are their sum.
 * Measured on 10000 by 10000 spatiotemporal boxes, that costs 2.6 to 3.0 times
 * what probing the entries costs, for the same answer.
 * @param[in] sptree1 The SPTree whose entries are walked
 * @param[in] node1 The node being visited
 * @param[in] sptree2 The SPTree being searched
 * @param[in] rootbox2 The region covered by the root of @p sptree2
 * @param[in] opconv The join operation, read from the side of @p sptree2
 * @param[in,out] found MeosArray the searches collect their ids into, reused
 * across the entries
 * @param[out] result MeosArray collecting the two ids of each pair
 */
static void
spnode_join(const SPTree *sptree1, const SPNode *node1, const SPTree *sptree2,
  const void *rootbox2, IndexSearchOp opconv, MeosArray *found,
  MeosArray *result)
{
  meos_array_reset(found);
  spnode_search(sptree2, sptree2->root, rootbox2, opconv, node1->centroid, 0,
    found);
  int count = meos_array_count(found);
  for (int k = 0; k < count; k++)
  {
    int64 id1 = node1->id;
    int64 id2 = *(int64 *) meos_array_get(found, k);
    meos_array_add(result, &id1);
    meos_array_add(result, &id2);
  }
  for (int quadrant = 0; node1->children && quadrant < sptree1->nchild;
    quadrant++)
  {
    const SPNode *child = node1->children[quadrant];
    if (child)
      spnode_join(sptree1, child, sptree2, rootbox2, opconv, found, result);
  }
  return;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Join two in-memory space-partitioning indexes, collecting the ids of
 * every qualifying pair into a MeosArray
 * @details Every entry of the first index is read against the second one,
 * which skips the subtrees its regions exclude, so a join reads far fewer pairs
 * than comparing every entry of one index with every entry of the other. An
 * index holds the boxes it was given, so a join needs nothing of the caller
 * beyond the two indexes.
 *
 * The result array is reset before the join. It receives two ids per pair, the
 * entry of @p sptree1 followed by the entry of @p sptree2, so pair `k` is read
 * with #meos_array_get at positions `2 * k` and `2 * k + 1`.
 * @param[in] sptree1,sptree2 The SPTrees to join, of the same bounding box
 * type. They need not be of the same kind
 * @param[in] op The join operation: @p INDEX_OVERLAPS pairs entries that
 * overlap, @p INDEX_CONTAINS pairs entries of @p sptree1 that contain an entry
 * of @p sptree2, @p INDEX_CONTAINED_BY pairs entries of @p sptree1 contained by
 * an entry of @p sptree2
 * @param[out] result MeosArray of int64 to collect the ids (created by the
 * caller with `meos_array_create(sizeof(int64))`)
 * @return Number of qualifying pairs, half the number of collected ids, on
 * error @p INT_MAX
 */
int
sptree_join(const SPTree *sptree1, const SPTree *sptree2, IndexSearchOp op,
  MeosArray *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(sptree1, INT_MAX); VALIDATE_NOT_NULL(sptree2, INT_MAX);
  VALIDATE_NOT_NULL(result, INT_MAX);
  if (! ensure_same_index_bboxtype(sptree1->bboxtype, sptree2->bboxtype) ||
      ! ensure_valid_sptree_sptree(sptree1, sptree2) ||
      ! ensure_index_join_op(op))
    return INT_MAX;

  meos_array_reset(result);
  if (sptree1->root && sptree2->root)
  {
    char rootbox2[SPTREE_NODEBOX_MAXSIZE];
    sptree2->nodebox_init(rootbox2, sptree2->root->centroid, sptree2);
    MeosArray *found = meos_array_create(sizeof(int64));
    spnode_join(sptree1, sptree1->root, sptree2, rootbox2,
      index_op_converse(op), found, result);
    meos_array_destroy(found);
  }
  return meos_array_count(result) / 2;
}

/*****************************************************************************
 * Temporal and multi-entry (MEST) functions
 *
 * The bounding box of a temporal value is extracted (single-box path) or the
 * value is decomposed into several tight per-segment boxes sharing one id
 * (multi-entry path), reusing the same per-type splitters as the R-tree index.
 * The tree and search algorithms are unchanged; only the build-side
 * decomposition and a search-time deduplication of repeated ids are added.
 *****************************************************************************/

/**
 * @ingroup meos_temporal_box_index
 * @brief Insert a temporal value into an in-memory space-partitioning index
 * @details The bounding box is automatically extracted from the temporal value.
 * The temporal type must be compatible with the SPTree's bounding box type:
 * temporal alphas (tbool, ttext) require a timestamptz-span-based SPTree and
 * temporal numbers (tint, tfloat) require a TBox-based SPTree.
 * @param[in] sptree The SPTree previously initialized
 * @param[in] temp The temporal value to be inserted
 * @param[in] id The id of the temporal value being inserted
 */
bool
sptree_insert_temporal(SPTree *sptree, const Temporal *temp, int64 id)
{
  if (! ensure_bbox_temporal_compatible(sptree->bboxtype, temp))
    return false;
  /* Use a stack buffer large enough for any MEOS bounding box type */
  bboxunion buf;
  memset(&buf, 0, sizeof(buf));
  temporal_set_bbox(temp, &buf);
  sptree_insert(sptree, &buf, id);
  return true;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Search an in-memory space-partitioning index using a temporal value's
 * bounding box, collecting matching ids into a MeosArray
 * @details The bounding box is automatically extracted from the temporal value
 * and used as the search query. The result array is reset before the search.
 * @param[in] sptree The SPTree to query
 * @param[in] op The search operation
 * @param[in] temp The temporal value whose bounding box serves as query
 * @param[out] result MeosArray of int to collect matching ids
 * @return Number of matching ids
 */
int
sptree_search_temporal(const SPTree *sptree, IndexSearchOp op,
  const Temporal *temp, MeosArray *result)
{
  if (! ensure_bbox_temporal_compatible(sptree->bboxtype, temp))
  {
    meos_array_reset(result);
    return 0;
  }
  /* Use a stack buffer large enough for any MEOS bounding box type */
  bboxunion buf;
  memset(&buf, 0, sizeof(buf));
  temporal_set_bbox(temp, &buf);
  return sptree_search(sptree, op, &buf, result);
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Insert a temporal value into an in-memory space-partitioning index as
 * several tight per-segment bounding boxes sharing the same id (multi-entry)
 * @details The temporal value is decomposed into at most `maxboxes` tight
 * per-segment bounding boxes, all inserted under `id`. This yields a more
 * selective index than #sptree_insert_temporal for wiggly or high-extent
 * temporal values. When `maxboxes <= 1` or the temporal value is an instant,
 * the behaviour is identical to #sptree_insert_temporal. Search results may
 * contain the same id several times; use #sptree_search_temporal_dedup to
 * collapse them.
 * @param[in] sptree The SPTree previously initialized
 * @param[in] temp The temporal value to be inserted
 * @param[in] id The id shared by every box produced for `temp`
 * @param[in] maxboxes Maximum number of bounding boxes produced for `temp`
 * @see sptree_insert_temporal
 */
bool
sptree_insert_temporal_split(SPTree *sptree, const Temporal *temp, int64 id,
  int maxboxes)
{
  if (! ensure_bbox_temporal_compatible(sptree->bboxtype, temp))
    return false;
  int count;
  void *boxes = bbox_temporal_split_boxes(sptree->bboxtype, sizeof(bboxunion), temp, maxboxes, &count);
  if (! boxes)
    return true;
  for (int i = 0; i < count; i++)
    sptree_insert(sptree, (char *) boxes + (size_t) i * sptree->boxsize, id);
  pfree(boxes);
  return true;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Search an in-memory space-partitioning index built with
 * #sptree_insert_temporal_split using a temporal value, returning each matching
 * id exactly once
 * @details The query temporal value is decomposed into the same tight
 * per-segment bounding boxes as #sptree_insert_temporal_split, the existing
 * #sptree_search is run for every query box, and the union of matching ids is
 * deduplicated so that each surviving id appears exactly once. The result array
 * is reset before the search.
 * @param[in] sptree The SPTree to query
 * @param[in] op The search operation
 * @param[in] temp The temporal value whose per-segment bounding boxes serve as
 * queries
 * @param[in] maxboxes Maximum number of query boxes derived from `temp`
 * @param[out] result MeosArray of int to collect the deduplicated matching ids
 * @return Number of distinct matching ids
 * @see sptree_search_temporal
 */
/**
 * @brief Order two indexed ids, in the form @p qsort takes
 */
static int
sptree_id_cmp(const void *a, const void *b)
{
  int64 l = *(const int64 *) a, r = *(const int64 *) b;
  return (l < r) ? -1 : ((l > r) ? 1 : 0);
}

int
sptree_search_temporal_dedup(const SPTree *sptree, IndexSearchOp op,
  const Temporal *temp, int maxboxes, MeosArray *result)
{
  meos_array_reset(result);
  if (! ensure_bbox_temporal_compatible(sptree->bboxtype, temp))
    return 0;

  int count;
  void *boxes = bbox_temporal_split_boxes(sptree->bboxtype, sizeof(bboxunion), temp, maxboxes, &count);
  if (! boxes)
    return 0;

  /* Accumulate the raw (possibly duplicated) candidate ids of every query box */
  MeosArray *raw = meos_array_create(sizeof(int64));
  MeosArray *hits = meos_array_create(sizeof(int64));
  for (int i = 0; i < count; i++)
  {
    int nhits = sptree_search(sptree, op,
      (char *) boxes + (size_t) i * sptree->boxsize, hits);
    for (int j = 0; j < nhits; j++)
    {
      int64 id = *(int64 *) meos_array_get(hits, j);
      meos_array_add(raw, &id);
    }
  }
  pfree(boxes);
  meos_array_destroy(hits);

  /* Collapse duplicates by sorting, so the memory is proportional to the
   * NUMBER of candidate ids rather than to their magnitude */
  int nraw = meos_array_count(raw);
  if (nraw > 0)
  {
    int64 *ids = palloc((size_t) nraw * sizeof(int64));
    for (int i = 0; i < nraw; i++)
      ids[i] = *(int64 *) meos_array_get(raw, i);
    qsort(ids, (size_t) nraw, sizeof(int64), sptree_id_cmp);
    for (int i = 0; i < nraw; i++)
      if (i == 0 || ids[i] != ids[i - 1])
        meos_array_add(result, &ids[i]);
    pfree(ids);
  }
  meos_array_destroy(raw);
  return meos_array_count(result);
}

/*****************************************************************************
 * Free
 *****************************************************************************/

/**
 * @brief Free a node and its children
 * @details The walk carries its own stack. The depth a node reaches is the
 * order the entries arrived in, so a tree grown from an ordered entry set is a
 * chain of one node per entry, and a walk that recurses once per level ends the
 * process on a tree the library itself builds. The stack holds the children
 * still to visit, whose pointers are read before the node holding them is
 * released.
 */
static void
spnode_free(const SPTree *sptree, SPNode *node)
{
  if (! node)
    return;

  size_t cap = 64, top = 0;
  SPNode **stack = palloc(cap * sizeof(SPNode *));
  stack[top++] = node;
  while (top > 0)
  {
    SPNode *cur = stack[--top];
    if (cur->children)
    {
      for (int i = 0; i < sptree->nchild; i++)
      {
        SPNode *child = cur->children[i];
        if (! child)
          continue;
        if (top == cap)
        {
          cap *= 2;
          stack = repalloc(stack, cap * sizeof(SPNode *));
        }
        stack[top++] = child;
      }
      pfree(cur->children);
    }
    pfree(cur);
  }
  pfree(stack);
  return;
}

/*****************************************************************************
 * What an index holds
 *
 * An SPTree is an opaque handle, so the size and the shape of the tree are
 * reported by the index itself rather than read from its layout, as the RTree
 * reports them.
 *****************************************************************************/

/**
 * @brief Accumulate the entries, the bytes and the depth of a subtree
 * @details Every node of a space-partitioning index holds one entry, its
 * centroid, so the entries are the nodes
 */
static void
spnode_stats(const SPTree *sptree, const SPNode *node, int level, int *entries,
  int64 *bytes, int *height)
{
  if (! node)
    return;

  /* The walk carries its own stack for the reason #spnode_free does: the
   * height of a tree grown from an ordered entry set is the number of entries,
   * and a walk that recurses once per level ends the process on it. */
  typedef struct
  {
    const SPNode *node;
    int level;
  } SPNodeLevel;

  size_t cap = 64, top = 0;
  SPNodeLevel *stack = palloc(cap * sizeof(SPNodeLevel));
  stack[top].node = node;
  stack[top++].level = level;
  while (top > 0)
  {
    SPNodeLevel cur = stack[--top];
    (*entries)++;
    *bytes += (int64) (sizeof(SPNode) + sptree->boxsize);
    if (cur.node->children)
      *bytes += (int64) ((size_t) sptree->nchild * sizeof(SPNode *));
    if (cur.level > *height)
      *height = cur.level;
    for (int i = 0; cur.node->children && i < sptree->nchild; i++)
    {
      const SPNode *child = cur.node->children[i];
      if (! child)
        continue;
      if (top == cap)
      {
        cap *= 2;
        stack = repalloc(stack, cap * sizeof(SPNodeLevel));
      }
      stack[top].node = child;
      stack[top++].level = cur.level + 1;
    }
  }
  pfree(stack);
  return;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Return the number of entries an SPTree holds
 * @param[in] sptree The SPTree
 * @return On error return @p INT_MAX
 */
int
sptree_num_entries(const SPTree *sptree)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(sptree, INT_MAX);
  int entries = 0, height = 0;
  int64 bytes = 0;
  spnode_stats(sptree, sptree->root, 1, &entries, &bytes, &height);
  return entries;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Return the number of bytes an SPTree holds
 * @details The nodes of the tree and the tree itself, which is what a caller
 * accounting for the memory of an index reports
 * @param[in] sptree The SPTree
 * @return On error return @p INT64_MAX
 */
int64
sptree_mem_size(const SPTree *sptree)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(sptree, INT64_MAX);
  int entries = 0, height = 0;
  int64 bytes = (int64) sizeof(SPTree);
  spnode_stats(sptree, sptree->root, 1, &entries, &bytes, &height);
  return bytes;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Return the number of levels an SPTree holds
 * @details An empty tree has no levels and a tree of one entry has one
 * @param[in] sptree The SPTree
 * @return On error return @p INT_MAX
 */
int
sptree_height(const SPTree *sptree)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(sptree, INT_MAX);
  int entries = 0, height = 0;
  int64 bytes = 0;
  spnode_stats(sptree, sptree->root, 1, &entries, &bytes, &height);
  return height;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Free an in-memory space-partitioning index
 * @param[in] sptree The SPTree to free
 */
void
sptree_free(SPTree *sptree)
{
  spnode_free(sptree, sptree->root);
  pfree(sptree);
  return;
}

/*****************************************************************************
 * Nearest-neighbour (kNN) cursor
 *
 * An incremental best-first traversal (Hjaltason and Samet) that yields the
 * stored ids in order of increasing distance to a query bounding box. A binary
 * min-heap holds two kinds of entries: tree nodes still to be expanded, keyed
 * by the distance from the query to the node's region, and stored ids ready to
 * be emitted, keyed by the distance from the query to the box stored at the
 * node. Because a node's region contains every box in its subtree, the region
 * distance is a lower bound on those box distances, so when an id reaches the
 * top of the heap nothing unexpanded is closer and ids pop in exact distance
 * order. The caller controls how many are consumed (e.g. to honour a `LIMIT
 * k`), so the traversal never materialises more of the tree than required. The
 * distance is the same one the SP-GiST operator class uses for its priority
 * queue: the region distance through #distance_span_nodespan,
 * #distance_tbox_nodebox and #distance_stbox_nodebox, and the distance to a
 * stored box through the same routines on the degenerate region equal to that
 * box.
 *****************************************************************************/

/**
 * @brief Return the distance from a query box to an inner node region of the
 * SPTree's box type
 * @param[in] sptree The SPTree providing the box type
 * @param[in] query The query bounding box
 * @param[in] nodebox The inner node region
 */
static double
sptree_nodebox_distance(const SPTree *sptree, const void *query,
  const void *nodebox)
{
  if (sptree->bboxtype == T_TBOX)
    return distance_tbox_nodebox((const TBox *) query,
      (const TboxNode *) nodebox);
  if (sptree->bboxtype == T_STBOX)
    return distance_stbox_nodebox((const STBox *) query,
      (const STboxNode *) nodebox);
#if POINTCLOUD
  if (sptree->bboxtype == T_TPCBOX)
    /* The boxes are stored internally as STBox (see tpcbox_project) */
    return distance_stbox_nodebox((const STBox *) query,
      (const STboxNode *) nodebox);
#endif
  /* Span types */
  return distance_span_nodespan((const Span *) query,
    (const SpanNode *) nodebox);
}

/**
 * @brief Fill @p nodebox with the degenerate node region equal to a single box
 * @details Every inner node region (SpanNode, TboxNode, STboxNode) is a pair of
 * boxes `{left, right}`; the region reduces to the single box @p box when both
 * halves equal it, so #sptree_nodebox_distance then returns the distance to
 * @p box exactly.
 * @param[in] sptree The SPTree providing the box size
 * @param[in] box The bounding box
 * @param[out] nodebox The degenerate node region
 */
static void
sptree_box_nodebox(const SPTree *sptree, const void *box, void *nodebox)
{
  memcpy(nodebox, box, sptree->boxsize);
  memcpy((char *) nodebox + sptree->boxsize, box, sptree->boxsize);
  return;
}

/**
 * @brief A single entry of the kNN cursor's priority queue
 * @details An entry is either a stored id ready to be emitted (@p is_emit true,
 * @p id set) or a tree node still to be expanded (@p is_emit false, @p node,
 * @p region and @p level set), keyed by @p dist.
 */
typedef struct SPNNEntry
{
  double dist;                 /**< Distance from the query to the entry */
  bool is_emit;                /**< True for an emittable id, false for a node */
  int64 id;                    /**< Stored id (when @p is_emit) */
  const SPNode *node;          /**< Tree node to expand (when not @p is_emit) */
  int level;                   /**< Depth of @p node (drives the k-d dimension) */
  char region[SPTREE_NODEBOX_MAXSIZE];  /**< Region covered by @p node */
} SPNNEntry;

/**
 * @brief Incremental nearest-neighbour cursor over an SPTree
 */
struct SPNNCursor
{
  const SPTree *sptree;   /**< Indexed SPTree (borrowed, not owned) */
  void *query;            /**< Private copy of the query bounding box */
  SPNNEntry *heap;        /**< Binary min-heap keyed by distance */
  int count;              /**< Number of entries currently in the heap */
  int capacity;           /**< Allocated capacity of the heap array */
};

/**
 * @brief Push an entry onto the cursor's min-heap, growing it if needed
 */
static void
spnn_heap_push(SPNNCursor *cursor, const SPNNEntry *entry)
{
  if (cursor->count == cursor->capacity)
  {
    cursor->capacity *= 2;
    cursor->heap = repalloc(cursor->heap,
      (size_t) cursor->capacity * sizeof(SPNNEntry));
  }
  int i = cursor->count++;
  cursor->heap[i] = *entry;
  /* Sift up while the child is closer than its parent */
  while (i > 0)
  {
    int parent = (i - 1) / 2;
    if (cursor->heap[parent].dist <= cursor->heap[i].dist)
      break;
    SPNNEntry tmp = cursor->heap[parent];
    cursor->heap[parent] = cursor->heap[i];
    cursor->heap[i] = tmp;
    i = parent;
  }
  return;
}

/**
 * @brief Pop and return the minimum-distance entry of the cursor's heap
 * @pre `cursor->count > 0`
 */
static SPNNEntry
spnn_heap_pop(SPNNCursor *cursor)
{
  assert(cursor->count > 0);
  SPNNEntry top = cursor->heap[0];
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
    SPNNEntry tmp = cursor->heap[i];
    cursor->heap[i] = cursor->heap[smallest];
    cursor->heap[smallest] = tmp;
    i = smallest;
  }
  return top;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Open a nearest-neighbour cursor that yields the ids stored in an
 * SPTree in order of increasing distance to a query bounding box
 * @details The cursor performs an incremental best-first traversal: repeated
 * calls to #sptree_nn_cursor_next return the indexed ids from nearest to
 * farthest. The caller stops consuming when it has enough neighbours (e.g.
 * after `k` results), so no more of the tree is visited than required. The
 * query box is copied into the cursor, so the caller may free or reuse it
 * immediately. Close the cursor with #sptree_nn_cursor_close.
 *
 * As for #sptree_search, the query box is validated here, at the entry point,
 * so that every distance the traversal computes can assume it.
 * @param[in] sptree The SPTree to query
 * @param[in] query The query bounding box of type @p sptree->bboxtype and of
 * the SRID the tree holds
 * @return A cursor to be freed with #sptree_nn_cursor_close, on error @p NULL
 */
SPNNCursor *
sptree_nn_cursor_open(const SPTree *sptree, const void *query)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(sptree, NULL); VALIDATE_NOT_NULL(query, NULL);
  if (! ensure_valid_sptree_box(sptree, query))
    return NULL;

  SPNNCursor *cursor = palloc0(sizeof(SPNNCursor));
  cursor->sptree = sptree;
  cursor->query = palloc(sptree->boxsize);
  /* Project the query box into the internal box type (TPCBox: STBox) */
  if (sptree->project)
    sptree->project(query, cursor->query);
  else
    memcpy(cursor->query, query, sptree->boxsize);
  cursor->capacity = 64;
  cursor->heap = palloc((size_t) cursor->capacity * sizeof(SPNNEntry));
  cursor->count = 0;
  /* Seed the heap with the root node covering the infinite region */
  if (sptree->root)
  {
    SPNNEntry entry;
    memset(&entry, 0, sizeof(entry));
    entry.is_emit = false;
    entry.node = sptree->root;
    entry.level = 0;
    sptree->nodebox_init(entry.region, sptree->root->centroid, sptree);
    entry.dist = sptree_nodebox_distance(sptree, cursor->query, entry.region);
    spnn_heap_push(cursor, &entry);
  }
  return cursor;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Advance a nearest-neighbour cursor to the next closest id
 * @details Returns the next id in order of increasing distance to the query
 * box. When @p id_out or @p dist_out is not @p NULL it receives the id and its
 * distance.
 * @param[in] cursor The cursor previously opened with #sptree_nn_cursor_open
 * @param[out] id_out Receives the id of the next neighbour, or @p NULL
 * @param[out] dist_out Receives the distance of the next neighbour, or @p NULL
 * @return @p true if a neighbour was produced, @p false once exhausted or on
 * error
 */
bool
sptree_nn_cursor_next(SPNNCursor *cursor, int64 *id_out, double *dist_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cursor, false);

  const SPTree *sptree = cursor->sptree;
  while (cursor->count > 0)
  {
    SPNNEntry entry = spnn_heap_pop(cursor);
    if (entry.is_emit)
    {
      /* A stored id reached the top of the heap: nothing unexpanded is closer */
      if (id_out)
        *id_out = entry.id;
      if (dist_out)
        *dist_out = entry.dist;
      return true;
    }
    /* Expand the node: emit its stored box and push its children */
    const SPNode *node = entry.node;
    SPNNEntry emit;
    memset(&emit, 0, sizeof(emit));
    emit.is_emit = true;
    emit.id = node->id;
    char centroidbox[SPTREE_NODEBOX_MAXSIZE];
    sptree_box_nodebox(sptree, node->centroid, centroidbox);
    emit.dist = sptree_nodebox_distance(sptree, cursor->query, centroidbox);
    spnn_heap_push(cursor, &emit);
    for (int quadrant = 0; node->children && quadrant < sptree->nchild;
      quadrant++)
    {
      const SPNode *child = node->children[quadrant];
      if (! child)
        continue;
      SPNNEntry childentry;
      memset(&childentry, 0, sizeof(childentry));
      childentry.is_emit = false;
      childentry.node = child;
      childentry.level = entry.level + 1;
      if (sptree->kind == SPTREE_QUADTREE)
        sptree->quadtree_next(entry.region, node->centroid, (uint8) quadrant,
          childentry.region);
      else
        sptree->kdtree_next(entry.region, node->centroid, (uint8) quadrant,
          entry.level, childentry.region);
      childentry.dist = sptree_nodebox_distance(sptree, cursor->query,
        childentry.region);
      spnn_heap_push(cursor, &childentry);
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
sptree_nn_cursor_close(SPNNCursor *cursor)
{
  if (! cursor)
    return;
  pfree(cursor->heap);
  pfree(cursor->query);
  pfree(cursor);
  return;
}

/*****************************************************************************/
