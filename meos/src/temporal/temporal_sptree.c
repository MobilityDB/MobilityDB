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
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#if POINTCLOUD
  #include <meos_pointcloud.h>
#endif
#include "temporal/temporal.h"
#include "temporal/span_index.h"
#include "temporal/tbox_index.h"
#include "geo/stbox_index.h"
#include "temporal/temporal_sptree.h"

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
span_inner_consistent(const void *nodebox, const void *query, RTreeSearchOp op)
{
  const SpanNode *n = (const SpanNode *) nodebox;
  const Span *q = (const Span *) query;
  if (op == RTREE_CONTAINS)
    return contain2D(n, q);
  /* RTREE_OVERLAPS and RTREE_CONTAINED_BY prune on overlap */
  return overlap2D(n, q);
}

static bool
span_leaf_consistent(const void *key, const void *query, RTreeSearchOp op)
{
  const Span *k = (const Span *) key;
  const Span *q = (const Span *) query;
  if (op == RTREE_CONTAINS)
    return contains_span_span(k, q);
  if (op == RTREE_CONTAINED_BY)
    return contains_span_span(q, k);
  return overlaps_span_span(k, q);
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
tbox_inner_consistent(const void *nodebox, const void *query, RTreeSearchOp op)
{
  const TboxNode *n = (const TboxNode *) nodebox;
  const TBox *q = (const TBox *) query;
  if (op == RTREE_CONTAINS)
    return contain4D(n, q);
  return overlap4D(n, q);
}

static bool
tbox_leaf_consistent(const void *key, const void *query, RTreeSearchOp op)
{
  const TBox *k = (const TBox *) key;
  const TBox *q = (const TBox *) query;
  if (op == RTREE_CONTAINS)
    return contains_tbox_tbox(k, q);
  if (op == RTREE_CONTAINED_BY)
    return contains_tbox_tbox(q, k);
  return overlaps_tbox_tbox(k, q);
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
stbox_inner_consistent(const void *nodebox, const void *query, RTreeSearchOp op)
{
  const STboxNode *n = (const STboxNode *) nodebox;
  const STBox *q = (const STBox *) query;
  if (op == RTREE_CONTAINS)
    return contain8D(n, q);
  return overlap8D(n, q);
}

static bool
stbox_leaf_consistent(const void *key, const void *query, RTreeSearchOp op)
{
  const STBox *k = (const STBox *) key;
  const STBox *q = (const STBox *) query;
  if (op == RTREE_CONTAINS)
    return contains_stbox_stbox(k, q);
  if (op == RTREE_CONTAINED_BY)
    return contains_stbox_stbox(q, k);
  return overlaps_stbox_stbox(k, q);
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
 */
static SPNode *
spnode_make(const SPTree *sptree, const void *box, int id)
{
  SPNode *node = palloc0(sizeof(SPNode) + sptree->boxsize);
  node->id = id;
  node->children = palloc0((size_t) sptree->nchild * sizeof(SPNode *));
  memcpy(node->centroid, box, sptree->boxsize);
  return node;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Insert a bounding box into an in-memory space-partitioning index
 * @details The box is stored at the first empty child slot reached while
 * descending from the root by quadrant. Equal boxes chain through the first
 * child, so the search still finds every id.
 * @param[in] sptree The SPTree previously initialized
 * @param[in] box The bounding box to insert
 * @param[in] id The id associated with the box
 */
void
sptree_insert(SPTree *sptree, void *box, int id)
{
  /* Project the incoming box into the internal box type (TPCBox: STBox) */
  bboxunion proj;
  if (sptree->project)
  {
    sptree->project(box, &proj);
    box = &proj;
  }
  /* Determine the deferred dimensions from the first box (STBox: 6 for 2D+T,
   * 8 for 3D+T) and hence the number of children per node */
  if (sptree->dims < 0)
  {
    sptree->dims = sptree->box_dims(box);
    sptree->nchild = (sptree->kind == SPTREE_QUADTREE) ?
      (1 << sptree->dims) : 2;
  }
  SPNode **slot = &sptree->root;
  int level = 0;
  while (*slot != NULL)
  {
    uint8 quadrant = sptree->get_quadrant((*slot)->centroid, box);
    int child = (sptree->kind == SPTREE_QUADTREE) ? (int) quadrant :
      (int) ((quadrant >> (level % sptree->dims)) & 1);
    slot = &(*slot)->children[child];
    level++;
  }
  *slot = spnode_make(sptree, box, id);
  return;
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
  RTreeSearchOp op, const void *query, int level, MeosArray *result)
{
  if (sptree->leaf_consistent(node->centroid, query, op))
  {
    int id = node->id;
    meos_array_add(result, &id);
  }
  for (int quadrant = 0; quadrant < sptree->nchild; quadrant++)
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
 * @param[in] op The search operation: @p RTREE_OVERLAPS finds boxes that
 * overlap the query, @p RTREE_CONTAINS finds boxes that contain the query,
 * @p RTREE_CONTAINED_BY finds boxes contained by the query
 * @param[in] query The bounding box that serves as query
 * @param[out] result MeosArray of int to collect matching ids
 * @return Number of matching ids
 */
int
sptree_search(const SPTree *sptree, RTreeSearchOp op, const void *query,
  MeosArray *result)
{
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
 * Temporal and multi-entry (MEST) functions
 *
 * The bounding box of a temporal value is extracted (single-box path) or the
 * value is decomposed into several tight per-segment boxes sharing one id
 * (multi-entry path), reusing the same per-type splitters as the R-tree index.
 * The tree and search algorithms are unchanged; only the build-side
 * decomposition and a search-time deduplication of repeated ids are added.
 *****************************************************************************/

/**
 * @brief Return true if the SPTree's bounding box type is compatible with the
 * temporal type, report an error otherwise
 * @param[in] sptree Pointer to the SPTree structure
 * @param[in] temp Temporal value
 */
static bool
ensure_sptree_temporal_compatible(const SPTree *sptree, const Temporal *temp)
{
  MeosType temptype = temp->temptype;
  bool compatible =
    (talpha_type(temptype) && sptree->bboxtype == T_TSTZSPAN) ||
    (tnumber_type(temptype) && sptree->bboxtype == T_TBOX) ||
    (tspatial_type(temptype) && sptree->bboxtype == T_STBOX)
#if POINTCLOUD
    || (tpointcloud_temptype(temptype) && sptree->bboxtype == T_TPCBOX)
#endif
    ;
  if (! compatible)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "SPTree bounding box type (%s) is not compatible with temporal type (%s)",
      meostype_name(sptree->bboxtype), meostype_name(temptype));
    return false;
  }
  return true;
}

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
void
sptree_insert_temporal(SPTree *sptree, const Temporal *temp, int id)
{
  if (! ensure_sptree_temporal_compatible(sptree, temp))
    return;
  /* Use a stack buffer large enough for any MEOS bounding box type */
  bboxunion buf;
  memset(&buf, 0, sizeof(buf));
  temporal_set_bbox(temp, &buf);
  sptree_insert(sptree, &buf, id);
  return;
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
sptree_search_temporal(const SPTree *sptree, RTreeSearchOp op,
  const Temporal *temp, MeosArray *result)
{
  if (! ensure_sptree_temporal_compatible(sptree, temp))
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
 * @brief Decompose a temporal value into an array of tight per-segment
 * bounding boxes whose element type matches the SPTree's bounding box type
 * @details The decomposition reuses the same per-segment machinery as the
 * single-box path: temporal alphas are split with #temporal_split_n_spans and
 * temporal numbers with #tnumber_split_n_tboxes. When `maxboxes <= 1` or the
 * temporal value is an instant, the function degenerates to the single minimum
 * bounding box, byte-identical to the result of #temporal_set_bbox used by
 * #sptree_insert_temporal.
 * @param[in] sptree The SPTree whose bounding box type drives the element type
 * @param[in] temp The temporal value to decompose
 * @param[in] maxboxes Maximum number of boxes produced for `temp`
 * @param[out] count Number of boxes in the returned array
 * @return Allocated array of `*count` bounding boxes, each of `sptree->boxsize`
 * bytes
 * @pre `temp` is compatible with `sptree` (verified by the callers)
 */
static void *
sptree_temporal_split_boxes(const SPTree *sptree, const Temporal *temp,
  int maxboxes, int *count)
{
  assert(sptree); assert(temp); assert(count);

  /* Degenerate single minimum bounding box. The allocation is sized for any
   * MEOS bounding box type because the temporal type's box may be larger than
   * the internal one (a TPCBox is projected to an STBox at insertion) */
  if (maxboxes <= 1 || temp->subtype == TINSTANT)
  {
    void *result = palloc0(sizeof(bboxunion));
    temporal_set_bbox(temp, result);
    *count = 1;
    return result;
  }

  /* Multi-box decomposition reusing the existing per-type splitters */
  MeosType temptype = temp->temptype;
  if (talpha_type(temptype))
    return temporal_split_n_spans(temp, maxboxes, count);
  if (tnumber_type(temptype))
    return tnumber_split_n_tboxes(temp, maxboxes, count);
  if (tgeo_type_all(temptype))
    return tgeo_split_n_stboxes(temp, maxboxes, count);

  /* No per-segment splitter (e.g. tcbuffer, tnpoint, tpose, tpcpoint,
   * tpcpatch): fall back to the single minimum bounding box */
  void *result = palloc0(sizeof(bboxunion));
  temporal_set_bbox(temp, result);
  *count = 1;
  return result;
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
void
sptree_insert_temporal_split(SPTree *sptree, const Temporal *temp, int id,
  int maxboxes)
{
  if (! ensure_sptree_temporal_compatible(sptree, temp))
    return;
  int count;
  void *boxes = sptree_temporal_split_boxes(sptree, temp, maxboxes, &count);
  if (! boxes)
    return;
  for (int i = 0; i < count; i++)
    sptree_insert(sptree, (char *) boxes + (size_t) i * sptree->boxsize, id);
  pfree(boxes);
  return;
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
int
sptree_search_temporal_dedup(const SPTree *sptree, RTreeSearchOp op,
  const Temporal *temp, int maxboxes, MeosArray *result)
{
  meos_array_reset(result);
  if (! ensure_sptree_temporal_compatible(sptree, temp))
    return 0;

  int count;
  void *boxes = sptree_temporal_split_boxes(sptree, temp, maxboxes, &count);
  if (! boxes)
    return 0;

  /* Accumulate the raw (possibly duplicated) candidate ids of every query
   * box, tracking the maximum id seen to size the dedup bitset */
  MeosArray *raw = meos_array_create(sizeof(int));
  MeosArray *hits = meos_array_create(sizeof(int));
  int maxid = -1;
  for (int i = 0; i < count; i++)
  {
    int nhits = sptree_search(sptree, op,
      (char *) boxes + (size_t) i * sptree->boxsize, hits);
    for (int j = 0; j < nhits; j++)
    {
      int id = *(int *) meos_array_get(hits, j);
      if (id > maxid)
        maxid = id;
      meos_array_add(raw, &id);
    }
  }
  pfree(boxes);
  meos_array_destroy(hits);

  /* Collapse duplicates with a seen-set sized to the maximum id */
  int nraw = meos_array_count(raw);
  if (maxid >= 0 && nraw > 0)
  {
    bool *seen = palloc0((size_t) (maxid + 1) * sizeof(bool));
    for (int i = 0; i < nraw; i++)
    {
      int id = *(int *) meos_array_get(raw, i);
      if (! seen[id])
      {
        seen[id] = true;
        meos_array_add(result, &id);
      }
    }
    pfree(seen);
  }
  meos_array_destroy(raw);
  return meos_array_count(result);
}

/*****************************************************************************
 * Free
 *****************************************************************************/

/**
 * @brief Recursively free a node and its children
 */
static void
spnode_free(const SPTree *sptree, SPNode *node)
{
  if (! node)
    return;
  for (int i = 0; i < sptree->nchild; i++)
    spnode_free(sptree, node->children[i]);
  pfree(node->children);
  pfree(node);
  return;
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
  int id;                      /**< Stored id (when @p is_emit) */
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
 * @param[in] sptree The SPTree to query
 * @param[in] query The query bounding box of type @p sptree->bboxtype
 * @return A cursor to be freed with #sptree_nn_cursor_close
 */
SPNNCursor *
sptree_nn_cursor_open(const SPTree *sptree, const void *query)
{
  assert(sptree); assert(query);
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
 * @return @p true if a neighbour was produced, @p false once exhausted
 */
bool
sptree_nn_cursor_next(SPNNCursor *cursor, int *id_out, double *dist_out)
{
  assert(cursor);
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
    for (int quadrant = 0; quadrant < sptree->nchild; quadrant++)
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
