/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Bounding box operators for temporal types
 * @details The bounding box of temporal values are
 * - a @p Span for temporal Booleans
 * - a @p TBox for temporal integers and floats, where the @p x coordinate is
 * for the value dimension and the @p t coordinate is for the time dimension.
 * The following operators are defined: @p overlaps, @p contains, @p contained,
 * @p same, and @p adjacent.
 *
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the value dimension, only the time dimension, or both
 * the value and the time dimensions.
 */

#include "temporal/temporal_boxops.h"

/* C */
#include <assert.h>
#include <limits.h>
#include <math.h>
/* PostgreSQL */
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/bbox_index.h"
#include "temporal/span.h"
#include "temporal/tbox.h"
#include "temporal/temporal.h"
#include "temporal/type_util.h"
#include "geo/stbox.h"
#include "geo/geo_funcs.h"
#include "geo/tspatial_boxops.h"
#include "temporal/span_index.h"
#include "temporal/tbox_index.h"
#if POINTCLOUD
  #include <meos_pointcloud.h>
  #include "pointcloud/tpc_boxops.h"
  #include "pointcloud/tpcbox.h"
#endif

/*****************************************************************************
 * Functions on generic bounding boxes of temporal types
 *****************************************************************************/

/**
 * @brief Return true if the type is a bounding box type
 */
bool
bbox_type(MeosType bboxtype)
{
  if (bboxtype == T_TSTZSPAN || bboxtype == T_TBOX || bboxtype == T_STBOX)
    return true;
#if POINTCLOUD
  if (bboxtype == T_TPCBOX)
    return true;
#endif
  return false;
}

/**
 * @brief Return the size of a bounding box type
 */
size_t
bbox_get_size(MeosType bboxtype)
{
  assert(bbox_type(bboxtype) || span_type(bboxtype));
  if (span_type(bboxtype))
    return sizeof(Span);
  if (bboxtype == T_TBOX)
    return sizeof(TBox);
#if POINTCLOUD
  if (bboxtype == T_TPCBOX)
    return sizeof(TPCBox);
#endif
  else /* bboxtype == T_STBOX */
    return sizeof(STBox);
}

/**
 * @brief Return the maximum number of dimensions of a bounding box type
 */
int
bbox_max_dims(MeosType bboxtype)
{
  assert(bbox_type(bboxtype));
  if (bboxtype == T_TSTZSPAN)
    return 1;
  if (bboxtype == T_TBOX)
    return 2;
  else /* bboxtype == T_STBOX */
    return 4;
}

/**
 * @brief Return true if two bounding boxes are equal
 * @param[in] box1,box2 Bounding boxes
 * @param[in] temptype Temporal type
 */
bool
temporal_bbox_eq(const void *box1, const void *box2, MeosType temptype)
{
  MeosType bboxtype = type_bboxtype(temptype);
  /* The box a temporal type carries is the one its catalog row
   * prescribes, so the dispatch below reads that and not the class */
  assert(bboxtype != T_UNKNOWN);
  if (bboxtype == T_TSTZSPAN)
    return span_eq((Span *) box1, (Span *) box2);
  else if (bboxtype == T_TBOX)
    return tbox_eq((TBox *) box1, (TBox *) box2);
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
    return tpcbox_eq((TPCBox *) box1, (TPCBox *) box2);
#endif
  else if (bboxtype == T_STBOX)
    return stbox_eq((STBox *) box1, (STBox *) box2);
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Unknown bounding box type: %s", meostype_name(bboxtype));
    return false;
  }
}

/**
 * @brief Return -1, 0, or 1 depending on whether the first bounding box
 * is less than, equal to, or greater than the second one
 * @param[in] box1,box2 Bounding boxes
 * @param[in] temptype Temporal type
 */
int
temporal_bbox_cmp(const void *box1, const void *box2, MeosType temptype)
{
  MeosType bboxtype = type_bboxtype(temptype);
  /* The box a temporal type carries is the one its catalog row
   * prescribes, so the dispatch below reads that and not the class */
  assert(bboxtype != T_UNKNOWN);
  if (bboxtype == T_TSTZSPAN)
    return span_cmp((Span *) box1, (Span *) box2);
  else if (bboxtype == T_TBOX)
    return tbox_cmp((TBox *) box1, (TBox *) box2);
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
    return tpcbox_cmp((TPCBox *) box1, (TPCBox *) box2);
#endif
  else if (bboxtype == T_STBOX)
    return stbox_cmp((STBox *) box1, (STBox *) box2);
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Unknown bounding box type: %s", meostype_name(bboxtype));
    return INT_MAX;
  }
}

/**
 * @brief Return true if the bounding box type is compatible with the
 * temporal type, report an error otherwise
 * @details Shared by the in-memory RTree and SPTree indexes: temporal alphas
 * (tbool, ttext) require a span bounding box, temporal numbers (tint,
 * tfloat) require a TBox, spatiotemporal types (tgeompoint, tgeogpoint,
 * etc.) require an STBox, and, when POINTCLOUD is enabled, temporal point
 * clouds (tpcpoint, tpcpatch) require a TPCBox.
 * @param[in] bboxtype Bounding box type of the index
 * @param[in] temp Temporal value
 */
bool
ensure_bbox_temporal_compatible(MeosType bboxtype, const Temporal *temp)
{
  assert(temp);
  MeosType temptype = temp->temptype;
  /* The catalog prescribes exactly one box per temporal type */
  bool compatible = (bboxtype == type_bboxtype(temptype));
  if (! compatible)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Bounding box type (%s) is not compatible with temporal type (%s)",
      meostype_name(bboxtype), meostype_name(temptype));
    return false;
  }
  return true;
}

/**
 * @brief Return true if two indexes hold the same bounding box type, report an
 * error otherwise
 * @details Shared by the in-memory RTree and SPTree indexes: a join reads the
 * entries of one index against the entries of the other, so the two hold boxes
 * of one type
 * @param[in] bboxtype1,bboxtype2 Bounding box types of the two indexes
 */
bool
ensure_same_index_bboxtype(MeosType bboxtype1, MeosType bboxtype2)
{
  if (bboxtype1 == bboxtype2)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "Join of indexes on mixed bounding box types: %s and %s",
    meostype_name(bboxtype1), meostype_name(bboxtype2));
  return false;
}

/**
 * @brief Return true if an operation pairs the entries of two indexes, report
 * an error otherwise
 * @details Shared by the in-memory RTree and SPTree indexes. A join prunes a
 * pair of subtrees on the overlap of what they cover, which holds only for the
 * operations an overlap implies: one entry contains, or is contained by,
 * another only where the two overlap. The operations that order a dimension
 * pair entries that lie apart, which is what the pruning discards, so a join
 * refuses them rather than reporting a silently short answer.
 * @param[in] op Search operation
 */
bool
ensure_index_join_op(IndexSearchOp op)
{
  if (op == INDEX_OVERLAPS || op == INDEX_CONTAINS || op == INDEX_CONTAINED_BY ||
      op == INDEX_SAME || op == INDEX_ADJACENT)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "A join answers the operations that compare extents, not the ones that "
    "order a dimension");
  return false;
}

/**
 * @brief Return true if two bounding boxes have the same structure
 * @details Shared by the in-memory RTree and SPTree indexes and the
 * PostgreSQL GiST and SP-GiST indexes. Two boxes have the same structure when
 * they carry the same axes and, where they carry a value or a spatial axis,
 * the same span type and the same SRID and geodetic flag. A point cloud box is
 * indexed as the spatiotemporal box it begins with, its SRID read from its
 * schema, so its structure is the one of that box. Whether an operation raises
 * an error on a pair of boxes depends on their structures, and a tree
 * partitions its boxes on the axes they carry, so an index keeps the boxes of
 * each structure in a tree of their own
 * @param[in] bboxtype Type of the bounding boxes
 * @param[in] box1,box2 Bounding boxes
 */
bool
bbox_same_structure(MeosType bboxtype, const void *box1, const void *box2)
{
  assert(box1); assert(box2);
  if (bboxtype == T_TBOX)
  {
    const TBox *b1 = (const TBox *) box1, *b2 = (const TBox *) box2;
    if (MEOS_FLAGS_GET_X(b1->flags) != MEOS_FLAGS_GET_X(b2->flags) ||
        MEOS_FLAGS_GET_T(b1->flags) != MEOS_FLAGS_GET_T(b2->flags) ||
        MEOS_FLAGS_GET_WHOLE(b1->flags) != MEOS_FLAGS_GET_WHOLE(b2->flags))
      return false;
    return ! MEOS_FLAGS_GET_X(b1->flags) ||
      b1->span.spantype == b2->span.spantype;
  }
  if (bboxtype == T_STBOX
#if POINTCLOUD
      || bboxtype == T_TPCBOX
#endif
     )
  {
    /* A point cloud box begins with a whole spatiotemporal box */
    const STBox *b1 = (const STBox *) box1, *b2 = (const STBox *) box2;
    int16 f1 = b1->flags, f2 = b2->flags;
    if (MEOS_FLAGS_GET_X(f1) != MEOS_FLAGS_GET_X(f2) ||
        MEOS_FLAGS_GET_Z(f1) != MEOS_FLAGS_GET_Z(f2) ||
        MEOS_FLAGS_GET_T(f1) != MEOS_FLAGS_GET_T(f2) ||
        MEOS_FLAGS_GET_GEODETIC(f1) != MEOS_FLAGS_GET_GEODETIC(f2) ||
        MEOS_FLAGS_GET_WHOLE(f1) != MEOS_FLAGS_GET_WHOLE(f2))
      return false;
    return ! MEOS_FLAGS_GET_X(f1) || b1->srid == b2->srid;
  }
  /* The spans of an index share their span type */
  return true;
}

/* A predicate of two boxes answering a search operation */
typedef bool (*index_pred_fn)(const void *, const void *);

#define INDEX_PRED(fn, T) \
  static bool ip_##fn(const void *box, const void *query) \
  { return fn((const T *) box, (const T *) query); }

#define INDEX_PREDS_BASE(B, T) \
  INDEX_PRED(overlaps_##B##_##B, T) INDEX_PRED(contains_##B##_##B, T) \
  INDEX_PRED(contained_##B##_##B, T) INDEX_PRED(same_##B##_##B, T) \
  INDEX_PRED(adjacent_##B##_##B, T) INDEX_PRED(left_##B##_##B, T) \
  INDEX_PRED(overleft_##B##_##B, T) INDEX_PRED(right_##B##_##B, T) \
  INDEX_PRED(overright_##B##_##B, T)
#define INDEX_PREDS_T(B, T) \
  INDEX_PRED(before_##B##_##B, T) INDEX_PRED(overbefore_##B##_##B, T) \
  INDEX_PRED(after_##B##_##B, T) INDEX_PRED(overafter_##B##_##B, T)
#define INDEX_PREDS_YZ(B, T) \
  INDEX_PRED(below_##B##_##B, T) INDEX_PRED(overbelow_##B##_##B, T) \
  INDEX_PRED(above_##B##_##B, T) INDEX_PRED(overabove_##B##_##B, T) \
  INDEX_PRED(front_##B##_##B, T) INDEX_PRED(overfront_##B##_##B, T) \
  INDEX_PRED(back_##B##_##B, T) INDEX_PRED(overback_##B##_##B, T)

INDEX_PREDS_BASE(span, Span)
INDEX_PREDS_BASE(tbox, TBox) INDEX_PREDS_T(tbox, TBox)
INDEX_PREDS_BASE(stbox, STBox) INDEX_PREDS_T(stbox, STBox)
INDEX_PREDS_YZ(stbox, STBox)
#if POINTCLOUD
INDEX_PREDS_BASE(tpcbox, TPCBox) INDEX_PREDS_T(tpcbox, TPCBox)
INDEX_PREDS_YZ(tpcbox, TPCBox)
#endif /* POINTCLOUD */

#define INDEX_OPS_BASE(B) \
  [INDEX_OVERLAPS] = ip_overlaps_##B##_##B, \
  [INDEX_CONTAINS] = ip_contains_##B##_##B, \
  [INDEX_CONTAINED_BY] = ip_contained_##B##_##B, \
  [INDEX_SAME] = ip_same_##B##_##B, \
  [INDEX_ADJACENT] = ip_adjacent_##B##_##B, \
  [INDEX_LEFT] = ip_left_##B##_##B, \
  [INDEX_OVERLEFT] = ip_overleft_##B##_##B, \
  [INDEX_RIGHT] = ip_right_##B##_##B, \
  [INDEX_OVERRIGHT] = ip_overright_##B##_##B
#define INDEX_OPS_T(B) \
  [INDEX_BEFORE] = ip_before_##B##_##B, \
  [INDEX_OVERBEFORE] = ip_overbefore_##B##_##B, \
  [INDEX_AFTER] = ip_after_##B##_##B, \
  [INDEX_OVERAFTER] = ip_overafter_##B##_##B
#define INDEX_OPS_YZ(B) \
  [INDEX_BELOW] = ip_below_##B##_##B, \
  [INDEX_OVERBELOW] = ip_overbelow_##B##_##B, \
  [INDEX_ABOVE] = ip_above_##B##_##B, \
  [INDEX_OVERABOVE] = ip_overabove_##B##_##B, \
  [INDEX_FRONT] = ip_front_##B##_##B, \
  [INDEX_OVERFRONT] = ip_overfront_##B##_##B, \
  [INDEX_BACK] = ip_back_##B##_##B, \
  [INDEX_OVERBACK] = ip_overback_##B##_##B

/* Number of search operations */
#define INDEX_NUM_OPS (INDEX_ADJACENT + 1)

static const index_pred_fn INDEX_SPAN_PREDS[INDEX_NUM_OPS] =
  { INDEX_OPS_BASE(span) };
static const index_pred_fn INDEX_TBOX_PREDS[INDEX_NUM_OPS] =
  { INDEX_OPS_BASE(tbox), INDEX_OPS_T(tbox) };
static const index_pred_fn INDEX_STBOX_PREDS[INDEX_NUM_OPS] =
  { INDEX_OPS_BASE(stbox), INDEX_OPS_T(stbox), INDEX_OPS_YZ(stbox) };
#if POINTCLOUD
static const index_pred_fn INDEX_TPCBOX_PREDS[INDEX_NUM_OPS] =
  { INDEX_OPS_BASE(tpcbox), INDEX_OPS_T(tpcbox), INDEX_OPS_YZ(tpcbox) };
#endif /* POINTCLOUD */

/**
 * @brief Return the predicate answering a search operation on a bounding box
 * type, NULL when the box type answers no such operation
 * @details A span has one axis, which the operations to the left and to the
 * right order whether it holds values or times, a temporal box orders its
 * value to the left and to the right and its time before and after, and a
 * spatiotemporal or point cloud box orders also its other spatial axes
 * @param[in] bboxtype Type of the bounding boxes
 * @param[in] op Search operation
 */
static index_pred_fn
index_op_pred(MeosType bboxtype, IndexSearchOp op)
{
  if ((int) op < 0 || (int) op >= INDEX_NUM_OPS)
    return NULL;
  if (span_type(bboxtype))
    return INDEX_SPAN_PREDS[op];
  if (bboxtype == T_TBOX)
    return INDEX_TBOX_PREDS[op];
  if (bboxtype == T_STBOX)
    return INDEX_STBOX_PREDS[op];
#if POINTCLOUD
  if (bboxtype == T_TPCBOX)
    return INDEX_TPCBOX_PREDS[op];
#endif /* POINTCLOUD */
  return NULL;
}

/**
 * @brief Return true if an index of a bounding box type answers a search
 * operation, report an error otherwise
 * @details Shared by the in-memory RTree and SPTree indexes, as
 * #ensure_index_join_op
 * @param[in] bboxtype Type of the bounding boxes
 * @param[in] op Search operation
 */
bool
ensure_index_op(MeosType bboxtype, IndexSearchOp op)
{
  if (index_op_pred(bboxtype, op))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "An index of type %s answers no such search operation",
    meostype_name(bboxtype));
  return false;
}

/**
 * @brief Return true if the validator of a pair of boxes raises no error on a
 * stored box and a query, and in the last arguments their flags
 * @details The validator of the pair is #tbox_tbox_is_valid,
 * #stbox_stbox_is_valid or #tpcbox_tpcbox_is_valid by the type of the boxes
 * @param[in] bboxtype Type of the bounding boxes, no span type
 * @param[in] box Stored bounding box
 * @param[in] query Query bounding box
 * @param[out] f1,f2 Flags of the stored box and of the query
 */
static bool
index_pair_is_valid(MeosType bboxtype, const void *box, const void *query,
  int16 *f1, int16 *f2)
{
  if (bboxtype == T_TBOX)
  {
    *f1 = ((const TBox *) box)->flags; *f2 = ((const TBox *) query)->flags;
    return tbox_tbox_is_valid((const TBox *) box, (const TBox *) query);
  }
#if POINTCLOUD
  if (bboxtype == T_TPCBOX)
  {
    *f1 = ((const TPCBox *) box)->flags; *f2 = ((const TPCBox *) query)->flags;
    return tpcbox_tpcbox_is_valid((const TPCBox *) box,
      (const TPCBox *) query);
  }
#endif /* POINTCLOUD */
  *f1 = ((const STBox *) box)->flags; *f2 = ((const STBox *) query)->flags;
  return stbox_stbox_is_valid((const STBox *) box, (const STBox *) query);
}

/**
 * @brief Return true if a search operation applied to a stored box and a
 * query raises no error
 * @details Shared by the in-memory RTree and SPTree indexes and the
 * PostgreSQL GiST and SP-GiST indexes. The test is the conjunction of the
 * tests of the validators the operation calls, so that it holds exactly when
 * the operation raises no error: a span operation tests the span types
 * (#span_type_is_same); a topological operation on boxes tests the pair
 * (#tbox_tbox_is_valid, #stbox_stbox_is_valid, #tpcbox_tpcbox_is_valid) and a
 * common dimension (#dimension_is_common); an operation ordering the value or
 * a spatial axis tests the pair and that both boxes carry the axis; an
 * operation ordering the time tests that both boxes carry it. It depends on
 * the structures of the boxes alone, see #bbox_same_structure
 * @param[in] bboxtype Type of the bounding boxes
 * @param[in] op Search operation, one #ensure_index_op accepts
 * @param[in] box Stored bounding box
 * @param[in] query Query bounding box
 */
bool
index_op_is_valid(MeosType bboxtype, IndexSearchOp op, const void *box,
  const void *query)
{
  assert(box); assert(query); assert(index_op_pred(bboxtype, op));
  if (span_type(bboxtype))
    return span_type_is_same((const Span *) box, (const Span *) query);
  int16 f1, f2;
  bool pair = index_pair_is_valid(bboxtype, box, query, &f1, &f2);
  switch (op)
  {
    case INDEX_OVERLAPS:
    case INDEX_CONTAINS:
    case INDEX_CONTAINED_BY:
    case INDEX_SAME:
    case INDEX_ADJACENT:
      return pair && dimension_is_common(f1, f2);
    case INDEX_LEFT:
    case INDEX_OVERLEFT:
    case INDEX_RIGHT:
    case INDEX_OVERRIGHT:
    case INDEX_BELOW:
    case INDEX_OVERBELOW:
    case INDEX_ABOVE:
    case INDEX_OVERABOVE:
      return pair && MEOS_FLAGS_GET_X(f1) && MEOS_FLAGS_GET_X(f2);
    case INDEX_FRONT:
    case INDEX_OVERFRONT:
    case INDEX_BACK:
    case INDEX_OVERBACK:
      return pair && MEOS_FLAGS_GET_Z(f1) && MEOS_FLAGS_GET_Z(f2);
    default: /* INDEX_BEFORE, INDEX_OVERBEFORE, INDEX_AFTER, INDEX_OVERAFTER */
      return MEOS_FLAGS_GET_T(f1) && MEOS_FLAGS_GET_T(f2);
  }
}

/**
 * @brief Return the result of a search operation applied to a stored box and
 * a query
 * @details Shared by the in-memory RTree and SPTree indexes. Where
 * #index_op_is_valid states that the operation raises an error on the boxes
 * of a tree, the search applies it to a stored box, which raises the error a
 * scan of the boxes raises
 * @param[in] bboxtype Type of the bounding boxes
 * @param[in] op Search operation, one #ensure_index_op accepts
 * @param[in] box Stored bounding box
 * @param[in] query Query bounding box
 */
bool
index_op_apply(MeosType bboxtype, IndexSearchOp op, const void *box,
  const void *query)
{
  index_pred_fn pred = index_op_pred(bboxtype, op);
  assert(pred);
  return pred(box, query);
}

/**
 * @brief Return in the last argument a query box projected onto the axes a
 * stored box carries
 * @details Shared by the in-memory RTree and SPTree indexes and the
 * PostgreSQL SP-GiST indexes. An operation raising no error, see
 * #index_op_is_valid, compares two boxes on the axes both carry, so an axis
 * of the query that the boxes of a tree lack does not restrict a search of
 * the tree, and the node predicates of the tree read the axes of the
 * projected query. A span has a single axis, which the query keeps
 * @param[in] bboxtype Type of the bounding boxes
 * @param[in] query Query bounding box
 * @param[in] box Bounding box of the structure of the tree
 * @param[out] result Projected query, of the size of the box type
 */
void
index_query_project(MeosType bboxtype, const void *query, const void *box,
  void *result)
{
  assert(query); assert(box); assert(result);
  memcpy(result, query, bbox_get_size(bboxtype));
  if (span_type(bboxtype))
    return;
  if (bboxtype == T_TBOX)
  {
    int16 flags = ((const TBox *) box)->flags;
    TBox *res = (TBox *) result;
    if (! MEOS_FLAGS_GET_X(flags))
      MEOS_FLAGS_SET_X(res->flags, false);
    if (! MEOS_FLAGS_GET_T(flags))
      MEOS_FLAGS_SET_T(res->flags, false);
    return;
  }
  /* A point cloud box begins with a whole spatiotemporal box */
  int16 flags = ((const STBox *) box)->flags;
  STBox *res = (STBox *) result;
  if (! MEOS_FLAGS_GET_X(flags))
    MEOS_FLAGS_SET_X(res->flags, false);
  if (! MEOS_FLAGS_GET_Z(flags))
    MEOS_FLAGS_SET_Z(res->flags, false);
  if (! MEOS_FLAGS_GET_T(flags))
    MEOS_FLAGS_SET_T(res->flags, false);
  return;
}

/**
 * @brief Return true if the nearest approach distance of a stored box and a
 * query raises no error
 * @details Shared by the in-memory RTree and SPTree indexes and the
 * PostgreSQL GiST and SP-GiST indexes, as #index_op_is_valid is for the
 * search operations. The test is the conjunction of the tests of the
 * validators the distance calls: a span distance tests the span types
 * (#span_type_is_same); a box distance tests the pair (#tbox_tbox_is_valid,
 * #stbox_stbox_is_valid, #tpcbox_tpcbox_is_valid) and that both boxes carry
 * the value or the space. Boxes of different dimensions read the dimensions
 * both have (#stbox_nad). It depends on the structures of the boxes alone, see
 * #bbox_same_structure
 * @param[in] bboxtype Type of the bounding boxes
 * @param[in] box Stored bounding box
 * @param[in] query Query bounding box
 */
bool
index_distance_is_valid(MeosType bboxtype, const void *box, const void *query)
{
  assert(box); assert(query);
  if (span_type(bboxtype))
    return span_type_is_same((const Span *) box, (const Span *) query);
  int16 f1, f2;
  bool pair = index_pair_is_valid(bboxtype, box, query, &f1, &f2);
  return pair && MEOS_FLAGS_GET_X(f1) && MEOS_FLAGS_GET_X(f2);
}

/**
 * @brief Return the nearest approach distance of a stored box and a query
 * @details Shared by the in-memory RTree and SPTree indexes. Where
 * #index_distance_is_valid states that the distance raises an error on the
 * boxes of a tree, a nearest-neighbour search measures a box stored in it,
 * which raises the error the operator @p |=| raises over the stored boxes:
 * the one of the validators of #NAD_tbox_tbox for temporal boxes, of
 * #nad_stbox_stbox for spatiotemporal boxes and of #nad_tpcbox_tpcbox for
 * point cloud boxes
 * @param[in] bboxtype Type of the bounding boxes
 * @param[in] box Stored bounding box
 * @param[in] query Query bounding box
 * @errval DBL_MAX
 */
double
index_distance_apply(MeosType bboxtype, const void *box, const void *query)
{
  assert(box); assert(query);
  if (span_type(bboxtype))
  {
    if (! ensure_same_span_type((const Span *) box, (const Span *) query))
      return DBL_MAX;
    return distance_span_span_double((const Span *) box, (const Span *) query);
  }
  if (bboxtype == T_TBOX)
  {
    const TBox *b = (const TBox *) box, *q = (const TBox *) query;
    if (! ensure_valid_tbox_tbox(b, q) || ! ensure_has_X(T_TBOX, b->flags) ||
        ! ensure_has_X(T_TBOX, q->flags))
      return DBL_MAX;
    return distance_double(nad_tbox_tbox_intl(b, q), b->span.basetype);
  }
#if POINTCLOUD
  if (bboxtype == T_TPCBOX)
    return nad_tpcbox_tpcbox((const TPCBox *) box, (const TPCBox *) query);
#endif /* POINTCLOUD */
  return nad_stbox_stbox((const STBox *) box, (const STBox *) query, true);
}

/**
 * @brief Return in the last argument the search operation of a strategy of
 * a PostgreSQL index over bounding boxes
 * @param[in] strategy Strategy number
 * @param[out] op Search operation
 * @return False when the strategy is no search operation
 */
static bool
index_strategy_op(StrategyNumber strategy, IndexSearchOp *op)
{
  switch (strategy)
  {
    case RTOverlapStrategyNumber: *op = INDEX_OVERLAPS; return true;
    case RTEverOverlapStrategyNumber: *op = INDEX_OVERLAPS; return true;
    case RTAlwaysOverlapStrategyNumber: *op = INDEX_OVERLAPS; return true;
    case RTContainsStrategyNumber: *op = INDEX_CONTAINS; return true;
    case RTContainedByStrategyNumber: *op = INDEX_CONTAINED_BY; return true;
    case RTSameStrategyNumber: *op = INDEX_SAME; return true;
    case RTAdjacentStrategyNumber: *op = INDEX_ADJACENT; return true;
    case RTLeftStrategyNumber: *op = INDEX_LEFT; return true;
    case RTOverLeftStrategyNumber: *op = INDEX_OVERLEFT; return true;
    case RTRightStrategyNumber: *op = INDEX_RIGHT; return true;
    case RTOverRightStrategyNumber: *op = INDEX_OVERRIGHT; return true;
    case RTBelowStrategyNumber: *op = INDEX_BELOW; return true;
    case RTOverBelowStrategyNumber: *op = INDEX_OVERBELOW; return true;
    case RTAboveStrategyNumber: *op = INDEX_ABOVE; return true;
    case RTOverAboveStrategyNumber: *op = INDEX_OVERABOVE; return true;
    case RTFrontStrategyNumber: *op = INDEX_FRONT; return true;
    case RTOverFrontStrategyNumber: *op = INDEX_OVERFRONT; return true;
    case RTBackStrategyNumber: *op = INDEX_BACK; return true;
    case RTOverBackStrategyNumber: *op = INDEX_OVERBACK; return true;
    case RTBeforeStrategyNumber: *op = INDEX_BEFORE; return true;
    case RTOverBeforeStrategyNumber: *op = INDEX_OVERBEFORE; return true;
    case RTAfterStrategyNumber: *op = INDEX_AFTER; return true;
    case RTOverAfterStrategyNumber: *op = INDEX_OVERAFTER; return true;
    default: return false;
  }
}

/**
 * @brief Return true if the operator of a strategy of a PostgreSQL index over
 * bounding boxes applied to a stored box and a query raises no error
 * @details The test #index_op_is_valid, for the consistent functions of the
 * GiST and SP-GiST indexes, which never raise: where the operator raises, they
 * return the entry as a candidate, and the recheck applies the operator to
 * the row, which raises its error
 * @param[in] bboxtype Type of the bounding boxes
 * @param[in] strategy Strategy number
 * @param[in] box Stored bounding box, or the key of a GiST index over boxes
 * of one structure
 * @param[in] query Query bounding box
 */
bool
index_strategy_is_valid(MeosType bboxtype, StrategyNumber strategy,
  const void *box, const void *query)
{
  IndexSearchOp op;
  return ! index_strategy_op(strategy, &op) ||
    index_op_is_valid(bboxtype, op, box, query);
}

/**
 * @brief Return true if an array holds the ids an index answers, report an
 * error otherwise
 * @details Shared by the in-memory RTree and SPTree indexes. A search copies
 * each id into the array at the width of its elements, so an array of
 * narrower elements keeps part of every id and answers another one
 * @param[in] result Array
 */
bool
ensure_index_result(const MeosArray *result)
{
  if (! result->varlength && result->elem_size == sizeof(int64))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The array cannot hold the ids an index answers, create it with "
    "index_result_create");
  return false;
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Return an array collecting the ids an in-memory index answers
 * @details A search or a join of an RTree or an SPTree fills the array, and
 * #index_result_id reads an id back. The array can be reused across searches,
 * each of which resets it
 * @return New array
 * @errval NULL
 */
MeosArray *
index_result_create(void)
{
  return meos_array_create(sizeof(int64));
}

/**
 * @ingroup meos_temporal_box_index
 * @brief Return in the last argument the n-th id, 0-based, an in-memory index
 * answers
 * @details A join answers two ids per pair, so pair `k` is read at positions
 * `2 * k` and `2 * k + 1`
 * @param[in] result Array filled by a search or a join
 * @param[in] n Position of the id
 * @param[out] id Id
 * @return Return true if the id is found
 */
bool
index_result_id(const MeosArray *result, int n, int64 *id)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(result, false); VALIDATE_NOT_NULL(id, false);
  if (! ensure_index_result(result))
    return false;
  const int64 *slot = meos_array_get(result, n);
  if (! slot)
    return false;
  *id = *slot;
  return true;
}

/**
 * @brief Decompose a temporal value into an array of tight per-segment
 * bounding boxes whose element type matches the given bounding box type
 * @details Shared by the in-memory RTree and SPTree indexes. The
 * decomposition reuses the same per-segment bounding box machinery as the
 * single-box path: temporal alphas are split with #temporal_split_n_spans,
 * temporal numbers with #tnumber_split_n_tboxes, and temporal geos with
 * #tgeo_split_n_stboxes. These already handle the TINSTANT, TSEQUENCE and
 * TSEQUENCESET subtypes, the discrete, step and linear interpolations, and
 * the Z, geodetic and SRID flags, and coarsen by merging adjacent segment
 * boxes (deterministic chunking) down to at most `maxboxes` boxes. When
 * `maxboxes <= 1`, the temporal value is an instant, or the temporal type
 * has no per-segment splitter (e.g. tcbuffer, tnpoint, tpose, tpcpoint,
 * tpcpatch), the function degenerates to the single minimum bounding box,
 * byte-identical to the result of #temporal_set_bbox.
 * @param[in] bboxtype Bounding box type of the index (unused for the
 * multi-box decomposition, kept for symmetry with the compatibility check
 * and for the assertion below)
 * @param[in] boxsize Size in bytes of a single bounding box allocated for
 * the degenerate/fallback single-box result; the caller passes a size large
 * enough to hold #temporal_set_bbox's output for `temp`, which may differ
 * from the index's own per-entry box size (e.g. an index that internally
 * projects a larger box type onto a smaller one)
 * @param[in] temp Temporal value to decompose
 * @param[in] maxboxes Maximum number of boxes produced for `temp`
 * @param[out] count Number of boxes in the returned array
 * @return Allocated array of `*count` bounding boxes
 * @errval NULL
 * @pre `temp` is compatible with `bboxtype` (verified by the callers)
 */
void *
bbox_temporal_split_boxes(MeosType bboxtype UNUSED, size_t boxsize,
  const Temporal *temp, int maxboxes, int *count)
{
  assert(bbox_type(bboxtype)); assert(temp); assert(count);

  /* Degenerate single minimum bounding box */
  if (maxboxes <= 1 || temp->subtype == TINSTANT)
  {
    void *result = palloc0(boxsize);
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

  /* No per-segment splitter: fall back to the single minimum bounding box */
  void *result = palloc0(boxsize);
  temporal_set_bbox(temp, result);
  *count = 1;
  return result;
}

/*****************************************************************************
 * Compute the bounding box at the creation of temporal values
 *****************************************************************************/

/**
 * @brief Return the size of a bounding box of a temporal type
 */
size_t
temporal_bbox_size(MeosType temptype)
{
  MeosType bboxtype = type_bboxtype(temptype);
  assert(bboxtype != T_UNKNOWN);
  return bbox_get_size(bboxtype);
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return in the last argument the temporal box of a temporal number
 * instant
 * @param[in] inst Temporal value
 * @param[out] box Result
 */
void
tnumberinst_set_tbox(const TInstant *inst, TBox *box)
{
  assert(inst); assert(temporal_type(inst->temptype)); assert(box);
  assert(tnumber_type(inst->temptype));
  MeosType basetype = temptype_basetype(inst->temptype);
  MeosType spantype = basetype_spantype(basetype);
  Datum value = tinstant_value_p(inst);
  Datum time = TimestampTzGetDatum(inst->t);
  TBox *tbox = (TBox *) box;
  memset(tbox, 0, sizeof(TBox));
  span_set(value, value, true, true, basetype, spantype, &tbox->span);
  span_set(time, time, true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &tbox->period);
  MEOS_FLAGS_SET_X(tbox->flags, true);
  MEOS_FLAGS_SET_T(tbox->flags, true);
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return in the last argument the bounding box of a temporal instant
 * @param[in] inst Temporal value
 * @param[out] box Result
 */
void
tinstant_set_bbox(const TInstant *inst, void *box)
{
  assert(inst); assert(box);
  MeosType bboxtype = type_bboxtype(inst->temptype);
  /* The box a temporal type carries is the one its catalog row
   * prescribes, so the dispatch below reads that and not the class */
  assert(bboxtype != T_UNKNOWN);
  if (bboxtype == T_TSTZSPAN)
    span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
      true, true, T_TIMESTAMPTZ, T_TSTZSPAN, (Span *) box);
  else if (bboxtype == T_TBOX)
    tnumberinst_set_tbox(inst, (TBox *) box);
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
    tpointcloudinst_set_tpcbox(inst, (TPCBox *) box);
#endif
  else if (bboxtype == T_STBOX)
    tspatialinst_set_stbox(inst, (STBox *) box);
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Unknown bounding box type: %s", meostype_name(bboxtype));
    return;
  }
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return int the last argument the temporal box of a temporal number
 * sequence
 * @param[in] seq Temporal sequence
 * @param[out] box Temporal box
 */
void
tnumberseq_set_tbox(const TSequence *seq, TBox *box)
{
  assert(seq); assert(box); assert(tnumber_type(seq->temptype));
  memcpy(box, TSEQUENCE_BBOX_PTR(seq), sizeof(TBox));
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return in the last argument the bounding box of a temporal sequence
 * @param[in] seq Temporal sequence
 * @param[out] box Bounding box
 */
void
tsequence_set_bbox(const TSequence *seq, void *box)
{
  assert(seq); assert(box);
  memset(box, 0, seq->bboxsize);
  memcpy(box, TSEQUENCE_BBOX_PTR(seq), seq->bboxsize);
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return in the last argument the temporal box of a temporal number
 * sequence
 * @param[in] ss Temporal sequence set
 * @param[out] box Temporal box
 */
void
tnumberseqset_set_tbox(const TSequenceSet *ss, TBox *box)
{
  assert(ss); assert(box); assert(tnumber_type(ss->temptype));
  memcpy(box, TSEQUENCESET_BBOX_PTR(ss), sizeof(TBox));
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return in the last argument the bounding box of a temporal sequence
 * set
 * @param[in] ss Temporal sequence set
 * @param[out] box Bounding box
 */
void
tsequenceset_set_bbox(const TSequenceSet *ss, void *box)
{
  assert(ss); assert(box);
  memset(box, 0, ss->bboxsize);
  memcpy(box, TSEQUENCESET_BBOX_PTR(ss), ss->bboxsize);
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return in the last argument the temporal box of a temporal number
 * @param[in] temp Temporal number
 * @param[out] box Temporal box
 */
void
tnumber_set_tbox(const Temporal *temp, TBox *box)
{
  assert(temp); assert(box); assert(tnumber_type(temp->temptype));
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      tnumberinst_set_tbox((TInstant *) temp, box);
      break;
    case TSEQUENCE:
      tnumberseq_set_tbox((TSequence *) temp, box);
      break;
    default: /* TSEQUENCESET */
      tnumberseqset_set_tbox((TSequenceSet *) temp, box);
  }
  return;
}

/*****************************************************************************/

/**
 * @brief Set a temporal box from an array of temporal number instants
 * @param[in] instants Temporal instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc, upper_inc True when the corresponding bound is
 * inclusive, false otherwise
 * @param[in] interp Interpolation
 * @param[out] box Box
 */
static void
tnumberinstarr_set_tbox(TInstant **instants, int count, bool lower_inc,
  bool upper_inc, interpType interp, TBox *box)
{
  assert(tnumber_type(instants[0]->temptype));
  MeosType basetype = temptype_basetype(instants[0]->temptype);
  MeosType spantype = basetype_spantype(basetype);
  /* Zero-fill the box, whose flags are set bit by bit below, as
   * #tnumberinst_set_tbox does */
  memset(box, 0, sizeof(TBox));
  /* For discrete or step interpolation the bounds are always inclusive */
  bool lower_inc1 = lower_inc;
  bool upper_inc1 = upper_inc;
  if (interp != LINEAR)
  {
    lower_inc1 = upper_inc1 = true;
  }
  /* Compute the value span */
  Datum min = tinstant_value_p(instants[0]);
  Datum max = min;
  bool min_inc = lower_inc1, max_inc = lower_inc1;
  for (int i = 1; i < count; i++)
  {
    Datum value = tinstant_value_p(instants[i]);
    int min_cmp = datum_cmp(value, min, basetype);
    int max_cmp = datum_cmp(value, max, basetype);
    if (min_cmp <= 0)
    {
      min = value;
      if (min_cmp == 0)
        min_inc |= (i < count - 1) ? true : upper_inc1;
      else
        min_inc = (i < count - 1) ? true : upper_inc1;
    }
    if (max_cmp >= 0)
    {
      max = value;
      if (max_cmp == 0)
        max_inc |= (i < count - 1) ? true : upper_inc1;
      else
        max_inc = (i < count - 1) ? true : upper_inc1;
    }
  }
  if (datum_eq(min, max, basetype))
  {
    min_inc = max_inc = true;
  }
  span_set(min, max, min_inc, max_inc, basetype, spantype, &box->span);
  /* Compute the time span */
  span_set(TimestampTzGetDatum(instants[0]->t),
    TimestampTzGetDatum(instants[count - 1]->t), lower_inc, upper_inc,
    T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  /* Set the flags */
  MEOS_FLAGS_SET_X(box->flags, true);
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @brief Set a bounding box from an array of temporal instant values
 * @param[in] instants Temporal instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc,upper_inc Period bounds
 * @param[in] interp Interpolation
 * @param[out] box Bounding box
 */
void
tinstarr_set_bbox(TInstant **instants, int count, bool lower_inc,
  bool upper_inc, interpType interp, void *box)
{
  assert(instants); assert(box);
  MeosType bboxtype = type_bboxtype(instants[0]->temptype);
  /* The box a temporal type carries is the one its catalog row
   * prescribes, so the dispatch below reads that and not the class */
  assert(bboxtype != T_UNKNOWN);
  if (bboxtype == T_TSTZSPAN)
    span_set(TimestampTzGetDatum(instants[0]->t),
      TimestampTzGetDatum(instants[count - 1]->t), lower_inc, upper_inc,
      T_TIMESTAMPTZ, T_TSTZSPAN, (Span *) box);
  else if (bboxtype == T_TBOX)
    tnumberinstarr_set_tbox(instants, count, lower_inc, upper_inc,
      interp, (TBox *) box);
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
    tpointcloudinstarr_set_tpcbox(instants, count, lower_inc, upper_inc,
      interp, (TPCBox *) box);
#endif
  else if (bboxtype == T_STBOX)
    tspatialinstarr_set_stbox(instants, count, lower_inc, upper_inc,
      interp, (STBox *) box);
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Unknown bounding box type: %s", meostype_name(bboxtype));
    return;
  }
  /* Set the lower_inc and upper_inc bounds of the period at the beginning
   * of the bounding box */
  Span *s = (Span *) box;
  s->lower_inc = lower_inc;
  s->upper_inc = upper_inc;
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return in the last argument the bounding box of the instants of a
 * temporal sequence between two positions
 * @param[in] seq Temporal sequence
 * @param[in] first,last Positions of the first and of the last instant
 * @param[out] box Bounding box
 * @note A bound interior to the sequence is inclusive, since the instant it
 * names belongs to both of the segments meeting there, while a bound at an
 * end of the sequence carries the inclusivity the sequence gives that end.
 * The box of a slice is thus contained in the box of the sequence, which is
 * what an index keyed on the slices of a value relies on
 */
void
tsequence_set_bbox_slice(const TSequence *seq, int first, int last, void *box)
{
  assert(seq); assert(box);
  assert(first >= 0); assert(last < seq->count); assert(first <= last);
  int count = last - first + 1;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
    instants[i] = (TInstant *) TSEQUENCE_INST_N(seq, first + i);
  tinstarr_set_bbox(instants, count,
    (first == 0) ? seq->period.lower_inc : true,
    (last == seq->count - 1) ? seq->period.upper_inc : true,
    MEOS_FLAGS_GET_INTERP(seq->flags), box);
  pfree(instants);
  return;
}

/**
 * @brief Set a time span to the span of the instants first to last of a
 * temporal sequence
 * @details The span twin of #tsequence_set_bbox_slice: a bound interior to the
 * sequence is inclusive, since the instant it names belongs to both segments
 * meeting there, and a bound at an end of the sequence carries the
 * sequence's own inclusivity, so the span lies in the period of the sequence
 * @param[in] seq Temporal sequence
 * @param[in] first,last Positions of the first and last instants
 * @param[out] s Time span
 */
void
tsequence_set_tstzspan_slice(const TSequence *seq, int first, int last,
  Span *s)
{
  assert(seq); assert(s);
  assert(first >= 0); assert(last < seq->count); assert(first <= last);
  span_set(TimestampTzGetDatum(TSEQUENCE_INST_N(seq, first)->t),
    TimestampTzGetDatum(TSEQUENCE_INST_N(seq, last)->t),
    (first == 0) ? seq->period.lower_inc : true,
    (last == seq->count - 1) ? seq->period.upper_inc : true,
    T_TIMESTAMPTZ, T_TSTZSPAN, s);
  return;
}

/**
 * @brief Expand the bounding box of a temporal number sequence with an instant
 * @param[inout] seq Temporal sequence
 * @param[in] inst Temporal instant
 */
static void
tnumberseq_expand_tbox(TSequence *seq, const TInstant *inst)
{
  TBox box;
  tnumberinst_set_tbox(inst, &box);
  tbox_expand(&box, (TBox *) TSEQUENCE_BBOX_PTR(seq));
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Expand the bounding box of a temporal sequence with an additional
 * instant
 * @param[inout] seq Temporal sequence
 * @param[in] inst Temporal instant
 */
void
tsequence_expand_bbox(TSequence *seq, const TInstant *inst)
{
  MeosType bboxtype = type_bboxtype(seq->temptype);
  /* The box a temporal type carries is the one its catalog row
   * prescribes, so the dispatch below reads that and not the class */
  assert(bboxtype != T_UNKNOWN);
  if (bboxtype == T_TSTZSPAN)
    span_set(TimestampTzGetDatum(TSEQUENCE_INST_N(seq, 0)->t),
      TimestampTzGetDatum(inst->t), seq->period.lower_inc, true, T_TIMESTAMPTZ,
      T_TSTZSPAN, (Span *) TSEQUENCE_BBOX_PTR(seq));
  else if (bboxtype == T_TBOX)
    tnumberseq_expand_tbox(seq, inst);
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
    tpointcloudseq_expand_tpcbox(seq, inst);
#endif
  else if (bboxtype == T_STBOX)
    tspatialseq_expand_stbox(seq, inst);
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Unknown bounding box type: %s", meostype_name(bboxtype));
    return;
  }
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Expand the bounding box of a temporal sequence set with an additional
 * sequence
 * @param[inout] ss Temporal sequence set
 * @param[in] seq Temporal sequence
 */
void
tsequenceset_expand_bbox(TSequenceSet *ss, const TSequence *seq)
{
  assert(ss); assert(seq);
  MeosType bboxtype = type_bboxtype(ss->temptype);
  /* The box a temporal type carries is the one its catalog row
   * prescribes, so the dispatch below reads that and not the class */
  assert(bboxtype != T_UNKNOWN);
  if (bboxtype == T_TSTZSPAN)
    span_expand(&seq->period, &ss->period);
  else if (bboxtype == T_TBOX)
    tbox_expand((TBox *) TSEQUENCE_BBOX_PTR(seq),
      (TBox *) TSEQUENCE_BBOX_PTR(ss));
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
    tpcbox_expand((TPCBox *) TSEQUENCE_BBOX_PTR(seq),
      (TPCBox *) TSEQUENCESET_BBOX_PTR(ss));
#endif
  // TODO Generalize as for tgeogpointseq_expand_stbox
  else if (bboxtype == T_STBOX)
    stbox_expand((STBox *) TSEQUENCE_BBOX_PTR(seq),
      (STBox *) TSEQUENCE_BBOX_PTR(ss));
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Unknown bounding box type: %s", meostype_name(bboxtype));
    return;
  }
  return;
}

/**
 * @brief Return in the last argument a timestamptz span constructed from an
 * array of temporal sequences
 * @param[in] sequences Temporal sequences
 * @param[in] count Number of elements in the array
 * @param[out] s Result
 */
static void
tseqarr_set_tstzspan(TSequence **sequences, int count, Span *s)
{
  const Span *first = &sequences[0]->period;
  const Span *last = &sequences[count - 1]->period;
  span_set(first->lower, last->upper, first->lower_inc, last->upper_inc,
    T_TIMESTAMPTZ, T_TSTZSPAN, s);
  return;
}

/**
 * @brief Return in the last argument a temporal box constructed from an
 * array of temporal number sequences
 * @param[in] box Box
 * @param[in] sequences Temporal instants
 * @param[in] count Number of elements in the array
 */
static void
tnumberseqarr_set_tbox(TSequence **sequences, int count, TBox *box)
{
  memcpy(box, TSEQUENCE_BBOX_PTR(sequences[0]), sizeof(TBox));
  for (int i = 1; i < count; i++)
  {
    const TBox *box1 = TSEQUENCE_BBOX_PTR(sequences[i]);
    tbox_expand(box1, box);
  }
  return;
}

/**
 * @brief Return in the last argument a bounding box constructed from an
 * array of temporal sequences
 */
void
tseqarr_compute_bbox(TSequence **sequences, int count, void *box)
{
  MeosType bboxtype = type_bboxtype(sequences[0]->temptype);
  /* The box a temporal type carries is the one its catalog row
   * prescribes, so the dispatch below reads that and not the class */
  assert(bboxtype != T_UNKNOWN);
  if (bboxtype == T_TSTZSPAN)
    tseqarr_set_tstzspan(sequences, count, (Span *) box);
  else if (bboxtype == T_TBOX)
    tnumberseqarr_set_tbox(sequences, count, (TBox *) box);
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
    tpointcloudseqarr_set_tpcbox(sequences, count, (TPCBox *) box);
#endif
  else if (bboxtype == T_STBOX)
    tspatialseqarr_set_stbox(sequences, count, (STBox *) box);
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Unknown bounding box type: %s", meostype_name(bboxtype));
    return;
  }
  return;
}

/*****************************************************************************/

#if MEOS
/**
 * @brief Recompute the bounding box of a temporal sequence
 * @param[inout] seq Temporal sequence
 * @note This function is applied upon a restart
 */
void
tsequence_compute_bbox(TSequence *seq)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = (TInstant *) TSEQUENCE_INST_N(seq, i);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  tinstarr_set_bbox(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, interp, TSEQUENCESET_BBOX_PTR(seq));
  pfree(instants);
  return;
}

/**
 * @brief (Re)compute the bounding box of a temporal sequence set
 * @param[inout] ss Temporal sequence set
 * @note This function is applied upon a restart
 */
void
tsequenceset_compute_bbox(TSequenceSet *ss)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = (TSequence *) TSEQUENCESET_SEQ_N(ss, i);
  tseqarr_compute_bbox(sequences, ss->count, TSEQUENCESET_BBOX_PTR(ss));
  pfree(sequences);
  return;
}
#endif /* MEOS */

/*****************************************************************************
 * Spans functions for temporal values
 * These functions can be used for defining Multi-Entry Search Trees indexes
 * https://www.pgcon.org/2014/schedule/events/696.en.html
 * https://github.com/MobilityDB/mest
 *****************************************************************************/

/**
 * @brief Set the span in the last argument from a temporal instant
 * @param[in] inst Temporal value
 * @param[out] result Span
 */
static void
tinstant_set_span(const TInstant *inst, Span *result)
{
  assert(inst); assert(result);
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, result);
  return;
}

/* GENERATED-BOXSPLIT-BEGIN temporal — tools/codegen/inherited/generate.py from templates/boxsplit.c.tmpl; DO NOT EDIT BY HAND;
 * edit the template + manifest.d/boxsplit.yaml and re-run. */
/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return a singleton array of spans from a
 * temporal value instant
 * @param[in] inst Temporal instant
 */
Span *
tinstant_spans(const TInstant *inst)
{
  assert(inst); assert(temporal_type(inst->temptype));
  /* One bounding box per instant */
  Span *result = palloc(sizeof(Span));
  tinstant_set_span(inst, &result[0]);
  return result;
}

/**
 * @brief Return an array of spans from the instants
 * of a temporal value sequence with discrete interpolation
 * @param[in] seq Temporal sequence
 */
static Span *
tsequence_disc_spans(const TSequence *seq)
{
  assert(seq); assert(temporal_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  /* One bounding box per instant */
  Span *result = palloc(sizeof(Span) * seq->count);
  for (int i = 0; i < seq->count; i++)
    tinstant_set_span(TSEQUENCE_INST_N(seq, i), &result[i]);
  return result;
}

/**
 * @brief Return an array of spans from the segments of a
 * temporal value sequence with continuous interpolation (iterator function)
 * @param[in] seq Temporal sequence
 * @param[out] result Array of boxes
 * @return Number of elements in the ouput array
 */
static int
tsequence_cont_spans_iter(const TSequence *seq, Span *result)
{
  assert(seq); assert(result); assert(temporal_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tsequence_set_tstzspan_slice(seq, 0, 0, &result[0]);
    return 1;
  }

  /* One bounding box per segment */
  for (int i = 0; i < seq->count - 1; i++)
    tsequence_set_tstzspan_slice(seq, i, i + 1, &result[i]);
  return seq->count - 1;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spans from the instants or segments
 * of a temporal value sequence
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] seq Temporal sequence
 * @param[out] count Number of elements in the output array
 */
Span *
tsequence_spans(const TSequence *seq, int *count)
{
  assert(seq); assert(count); assert(temporal_type(seq->temptype));

  /* Discrete case */
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
  {
    *count = seq->count;
    return tsequence_disc_spans(seq);
  }

  /* Continuous case */
  int nboxes = (seq->count == 1) ? 1 : seq->count - 1;
  Span *result = palloc(sizeof(Span) * nboxes);
  *count = tsequence_cont_spans_iter(seq, result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spans from the segments of a
 * temporal value sequence set
 * @param[in] ss Temporal sequence set
 * @param[out] count Number of elements in the output array
 */
Span *
tsequenceset_spans(const TSequenceSet *ss, int *count)
{
  assert(ss); assert(count); assert(temporal_type(ss->temptype));

  /* One bounding box per segment */
  Span *result = palloc(sizeof(Span) * ss->totalcount);
  int nboxes = 0;
  for (int i = 0; i < ss->count; i++)
    nboxes += tsequence_cont_spans_iter(TSEQUENCESET_SEQ_N(ss, i),
      &result[nboxes]);
  assert(nboxes <= ss->totalcount);
  *count = nboxes;
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spans from the instants or segments
 * of a temporal value, its arguments valid
 * @details The internal twin of #temporal_spans, which an index calls
 * @param[in] temp Temporal value
 * @param[out] count Number of values of the output array
 */
Span *
temporal_spans_intl(const Temporal *temp, int *count)
{
  assert(temp); assert(count); assert(temporal_type(temp->temptype));
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tinstant_spans((TInstant *) temp);
    case TSEQUENCE:
      return tsequence_spans((TSequence *) temp, count);
    default: /* TSEQUENCESET */
      return tsequenceset_spans((TSequenceSet *) temp, count);
  }
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of spans from the instants or segments
 * of a temporal value
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] temp Temporal value
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Temporal_spans()
 */
Span *
temporal_spans(const Temporal *temp, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  return temporal_spans_intl(temp, count);
}

/*****************************************************************************/

/**
 * @brief Return an array of N spans from the instants
 * of a temporal value sequence with discrete interpolation
 * @param[in] seq Temporal sequence
 * @param[in] span_count Number of boxes
 * @param[out] count Number of elements in the output array
 */
static Span *
tsequence_disc_split_n_spans(const TSequence *seq, int span_count, int *count)
{
  assert(seq); assert(count); assert(temporal_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  assert(span_count > 0);

  /* One bounding box per instant */
  if (seq->count <= span_count)
  {
    *count = seq->count;
    return tsequence_disc_spans(seq);
  }

  /* One bounding box per several consecutive instants */
  Span *result = palloc(sizeof(Span) * span_count);
  /* Minimum number of input instants merged together in an output box */
  int size = seq->count / span_count;
  /* Number of output boxes that result from merging (size + 1) instants */
  int remainder = seq->count % span_count;
  int i = 0; /* Loop variable for input instants */
  for (int k = 0; k < span_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    tinstant_set_span(TSEQUENCE_INST_N(seq, i), &result[k]);
    for (int l = i + 1; l < j; l++)
    {
      Span box;
      tinstant_set_span(TSEQUENCE_INST_N(seq, l), &box);
      span_expand(&box, &result[k]);
    }
    i = j;
  }
  assert(i == seq->count);
  *count = span_count;
  return result;
}

/**
 * @brief Return an array of N spans from the segments of a 
 * temporal value sequence with continuous interpolation (iterator function)
 * @param[in] seq Temporal value
 * @param[in] span_count Number of boxes
 * @param[out] result Array of boxes
 */
static int
tsequence_cont_split_n_spans_iter(const TSequence *seq, int span_count,
  Span *result)
{
  assert(seq); assert(result); assert(temporal_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);
  assert(span_count > 0);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tsequence_set_tstzspan_slice(seq, 0, 0, &result[0]);
    return 1;
  }

  /* One bounding box per segment */
  int nsegs = seq->count - 1;
  if (nsegs <= span_count)
    return tsequence_cont_spans_iter(seq, result);

  /* One bounding box per several consecutive segments */
  /* Minimum number of input segments merged together in an output box */
  int size = nsegs / span_count;
  /* Number of output boxes that result from merging (size + 1) segments */
  int remainder = nsegs % span_count;
  int i = 0; /* Loop variable for input segments */
  for (int k = 0; k < span_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    tsequence_set_tstzspan_slice(seq, i, j, &result[k]);
    i = j;
  }
  assert(i == nsegs);
  return span_count;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N spans from the instants or
 * segments of a temporal value sequence
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] seq Temporal sequence
 * @param[in] span_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @return If `seq->count <= span_count`, the result contains one box
 * per instant or segment. Otherwise, consecutive instants or segments are
 * combined into a single box in the result to reach the given number of
 * boxes.
 */
Span *
tsequence_split_n_spans(const TSequence *seq, int span_count, int *count)
{
  assert(seq); assert(count); assert(temporal_type(seq->temptype));
  assert(span_count > 0);

  /* Discrete case */
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return tsequence_disc_split_n_spans(seq, span_count, count);

  /* Continuous case */
  int nboxes = (seq->count <= span_count) ?
    (seq->count == 1 ? 1 : seq->count - 1) : span_count;
  Span *result = palloc(sizeof(Span) * nboxes);
  *count = tsequence_cont_split_n_spans_iter(seq, span_count, result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N spans from the segments of a
 * temporal value sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] span_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @return If `ss->totalcount <= span_count`, the result contains one box per
 * segment. Otherwise, consecutive input segments are combined into a single box
 * in
 * the result to reach the given number of boxes.
 */
Span *
tsequenceset_split_n_spans(const TSequenceSet *ss, int span_count, int *count)
{
  assert(ss); assert(count); assert(temporal_type(ss->temptype));
  assert(span_count > 0);

  /* One bounding box per segment */
  if (ss->totalcount <= span_count)
    return tsequenceset_spans(ss, count);

  /* At most span_count boxes, the segments being more */
  Span *result = palloc(sizeof(Span) * span_count);

  /* Number of bounding boxes per composing sequence determined from the
   * proportion of seq->count and ss->totalcount */
  if (ss->count <= span_count)
  {
    int nboxes1 = 0;
    for (int i = 0; i < ss->count; i++)
    {
      bool end = false;
      const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
      int nboxes_seq = (int) (span_count * seq->count * 1.0 / ss->totalcount);
      if (! nboxes_seq)
        nboxes_seq = 1;
      if (nboxes_seq + nboxes1 >= span_count)
      {
        end = true;
        nboxes_seq = span_count - nboxes1;
      }
      nboxes1 += tsequence_cont_split_n_spans_iter(seq, nboxes_seq,
        &result[nboxes1]);
      if (end)
        break;
    }
    assert(nboxes1 <= span_count);
    *count = nboxes1;
    return result;
  }

  /* Merge consecutive sequences to reach the maximum number of boxes */
  /* Minimum number of sequences merged together in an output box */
  int size = ss->count / span_count;
  /* Number of output boxes that result from merging (size + 1) sequences */
  int remainder = ss->count % span_count;
  int i = 0; /* Loop variable for input sequences */
  for (int k = 0; k < span_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    tsequence_cont_split_n_spans_iter(TSEQUENCESET_SEQ_N(ss, i), 1,
      &result[k]);
    for (int l = i + 1; l < j; l++)
    {
      Span box;
      tsequence_cont_split_n_spans_iter(TSEQUENCESET_SEQ_N(ss, l), 1, &box);
      span_expand(&box, &result[k]);
    }
    i = j;
  }
  assert(i == ss->count);
  *count = span_count;
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N spans obtained by merging
 * consecutive instants or segments of a temporal value, its arguments valid
 * @details The internal twin of #temporal_split_n_spans, which an index calls
 * @param[in] temp Temporal value
 * @param[in] span_count Number of boxes
 * @param[out] count Number of elements in the output array
 */
Span *
temporal_split_n_spans_intl(const Temporal *temp, int span_count, int *count)
{
  assert(temp); assert(count); assert(temporal_type(temp->temptype));
  assert(span_count > 0);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tinstant_spans((TInstant *) temp);
    case TSEQUENCE:
      return tsequence_split_n_spans((TSequence *) temp, span_count, count);
    default: /* TSEQUENCESET */
      return tsequenceset_split_n_spans((TSequenceSet *) temp, span_count,
        count);
  }
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of N spans obtained by merging
 * consecutive instants or segments of a temporal value
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] temp Temporal value
 * @param[in] span_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @errval NULL
 * @csqlfn #Temporal_split_n_spans()
 */
Span *
temporal_split_n_spans(const Temporal *temp, int span_count, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  if (! ensure_positive(span_count))
    return NULL;
  return temporal_split_n_spans_intl(temp, span_count, count);
}

/*****************************************************************************
 * split_each_n_spans function
 *****************************************************************************/

/**
 * @brief Return an array of spans obtained by merging
 * consecutive instants of a temporal value sequence with discrete interpolation
 * @param[in] seq Temporal sequence
 * @param[in] elem_count Number of input instants merged into an output box
 * @param[out] count Number of elements in the output array
 */
static Span *
tsequence_disc_split_each_n_spans(const TSequence *seq, int elem_count,
  int *count)
{
  assert(seq); assert(count); assert(temporal_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  assert(elem_count > 0);

  int nboxes = ceil((double) seq->count / (double) elem_count);
  Span *result = palloc(sizeof(Span) * nboxes);
  int k = 0;
  for (int i = 0; i < seq->count; ++i)
  {
    if (i % elem_count == 0)
      tinstant_set_span(TSEQUENCE_INST_N(seq, i), &result[k++]);
    else
    {
      Span box;
      tinstant_set_span(TSEQUENCE_INST_N(seq, i), &box);
      span_expand(&box, &result[k - 1]);
    }
  }
  assert(k == nboxes);
  *count = k;
  return result;
}

/**
 * @brief Return an array of spans obtained by merging
 * consecutive segments of a temporal value sequence (iterator function)
 * @details The sequence has continuous interpolation.
 * @param[in] seq Temporal sequence
 * @param[in] elem_count Number of input segments merged into an output box
 * @param[out] result Array of boxes
 * @return Number of elements in the output array
 */
static int
tsequence_cont_split_each_n_spans_iter(const TSequence *seq,
  int elem_count, Span *result)
{
  assert(seq); assert(result); assert(temporal_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);
  assert(elem_count > 0);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    *(&result[0]) = (seq)->period;
    return 1;
  }

  /* General case */
  int nboxes = ceil((double) (seq->count - 1) / (double) elem_count);
  for (int k = 0; k < nboxes; k++)
  {
    int first = k * elem_count;
    int last = Min(first + elem_count, seq->count - 1);
    tsequence_set_tstzspan_slice(seq, first, last, &result[k]);
  }
  return nboxes;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spans obtained by merging
 * consecutive instants or segments of a temporal value sequence
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] seq Temporal sequence
 * @param[in] elem_count Number of input segments merged into an output box
 * @param[out] count Number of elements in the output array
 */
Span *
tsequence_split_each_n_spans(const TSequence *seq, int elem_count,
  int *count)
{
  assert(seq); assert(count); assert(temporal_type(seq->temptype));
  assert(elem_count > 0);

  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return tsequence_disc_split_each_n_spans(seq, elem_count, count);

  /* Number of instants or segments */
  int nelems = (seq->count == 1) ? 1 : seq->count - 1;
  int nboxes = ceil((double) nelems / (double) elem_count);
  Span *result = palloc(sizeof(Span) * nboxes);
  *count = tsequence_cont_split_each_n_spans_iter(seq, elem_count,
    result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spans obtained by merging
 * consecutive segments of a temporal value sequence set 
 * @param[in] ss Temporal sequence set
 * @param[in] elem_count Number of input segments merged into an output box
 * @param[out] count Number of elements in the output array
 */
Span *
tsequenceset_split_each_n_spans(const TSequenceSet *ss, int elem_count,
  int *count)
{
  assert(ss); assert(count); assert(temporal_type(ss->temptype));
  assert(elem_count > 0);

  /* Singleton sequence set */
  if (ss->count == 1)
    return tsequence_split_each_n_spans(TSEQUENCESET_SEQ_N(ss, 0),
      elem_count, count);

  /* Iterate for every composing sequence */
  int nboxes = 0;
  Span *result = palloc(sizeof(Span) * ss->totalcount);
  for (int i = 0; i < ss->count; ++i)
    nboxes += tsequence_cont_split_each_n_spans_iter(
      TSEQUENCESET_SEQ_N(ss, i), elem_count, &result[nboxes]);
  *count = nboxes;
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spans obtained by merging
 * consecutive instants or segments of a temporal value, its arguments valid
 * @details The internal twin of #temporal_split_each_n_spans, which an index
 * calls
 * @param[in] temp Temporal value
 * @param[in] elem_count Number of input instants or segments merged into an
 * output box
 * @param[out] count Number of values of the output array
 */
Span *
temporal_split_each_n_spans_intl(const Temporal *temp, int elem_count,
  int *count)
{
  assert(temp); assert(count); assert(temporal_type(temp->temptype));
  assert(elem_count > 0);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tinstant_spans((TInstant *) temp);
    case TSEQUENCE:
      return tsequence_split_each_n_spans((TSequence *) temp, elem_count,
        count);
    default: /* TSEQUENCESET */
      return tsequenceset_split_each_n_spans((TSequenceSet *) temp,
        elem_count, count);
  }
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of spans obtained by merging
 * consecutive instants or segments of a temporal value
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] temp Temporal value
 * @param[in] elem_count Number of input instants or segments merged into an
 * output box
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Temporal_split_each_n_spans()
 */
Span *
temporal_split_each_n_spans(const Temporal *temp, int elem_count, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  if (! ensure_positive(elem_count))
    return NULL;
  return temporal_split_each_n_spans_intl(temp, elem_count, count);
}

/*****************************************************************************
 * merge_n_spans and merge_each_n_spans functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N spans obtained by merging the boxes of
 * the consecutive instants or segments of a temporal value, each time the
 * consecutive pair whose union adds the least size to theirs, its arguments
 * valid
 * @details The internal twin of #temporal_merge_n_spans, which an index calls
 * @param[in] temp Temporal value
 * @param[in] span_count Number of boxes
 * @param[out] count Number of values of the output array
 */
Span *
temporal_merge_n_spans_intl(const Temporal *temp, int span_count, int *count)
{
  assert(temp); assert(count); assert(temporal_type(temp->temptype));
  assert(span_count > 0);
  Span *result = temporal_spans_intl(temp, count);
  if (result)
    *count = bbox_merge_n(T_TSTZSPAN, result, *count, span_count, &span_adjust,
      &span_size);
  return result;
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of N spans obtained by merging the boxes of
 * the consecutive instants or segments of a temporal value, each time the
 * consecutive pair whose union adds the least size to theirs
 * @details The merge twin of #temporal_split_n_spans, by #bbox_merge_n
 * @param[in] temp Temporal value
 * @param[in] span_count Number of boxes
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Temporal_merge_n_spans()
 */
Span *
temporal_merge_n_spans(const Temporal *temp, int span_count, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  if (! ensure_positive(span_count))
    return NULL;
  return temporal_merge_n_spans_intl(temp, span_count, count);
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spans obtained by merging the boxes of the
 * consecutive instants or segments of a temporal value, each time the
 * consecutive
 * pair whose union adds the least size to theirs, until a box holds on
 * average a number of them, its arguments valid
 * @details The internal twin of #temporal_merge_each_n_spans, which an index
 * calls
 * @param[in] temp Temporal value
 * @param[in] elem_count Number of input instants or segments merged into an
 * output box on average
 * @param[out] count Number of values of the output array
 */
Span *
temporal_merge_each_n_spans_intl(const Temporal *temp, int elem_count,
  int *count)
{
  assert(temp); assert(count); assert(temporal_type(temp->temptype));
  assert(elem_count > 0);
  Span *result = temporal_spans_intl(temp, count);
  if (result)
    *count = bbox_merge_each_n(T_TSTZSPAN, result, *count, elem_count,
      &span_adjust, &span_size);
  return result;
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of spans obtained by merging the boxes of the
 * consecutive instants or segments of a temporal value, each time the
 * consecutive
 * pair whose union adds the least size to theirs, until a box holds on
 * average a number of them
 * @details The merge twin of #temporal_split_each_n_spans, by
 * #bbox_merge_each_n
 * @param[in] temp Temporal value
 * @param[in] elem_count Number of input instants or segments merged into an
 * output box on average
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Temporal_merge_each_n_spans()
 */
Span *
temporal_merge_each_n_spans(const Temporal *temp, int elem_count, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  if (! ensure_positive(elem_count))
    return NULL;
  return temporal_merge_each_n_spans_intl(temp, elem_count, count);
}

/*****************************************************************************
 * merge_query_spans function
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spans obtained by merging
 * the boxes of the consecutive instants or segments of a
 * temporal value into those a query of the extents of a query box
 * is expected to read least, its arguments valid
 * @details The internal twin of #temporal_merge_query_spans, which an index
 * calls
 * @param[in] temp Temporal value
 * @param[in] query Query box
 * @param[out] count Number of values of the output array
 */
Span *
temporal_merge_query_spans_intl(const Temporal *temp, const Span *query,
  int *count)
{
  assert(temp); assert(query); assert(count);
  assert(temporal_type(temp->temptype));
  assert(ensure_span_isof_type(query, T_TSTZSPAN));
  Span *result = temporal_spans_intl(temp, count);
  if (result)
    *count = bbox_merge_query(T_TSTZSPAN, result, *count, query, &span_adjust,
      &span_query_size);
  return result;
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of spans obtained by merging
 * the boxes of the consecutive instants or segments of a
 * temporal value into those a query of the extents of a query box
 * is expected to read least
 * @details The query twin of #temporal_merge_n_spans, by #bbox_merge_query:
 * the widths of the query box are the extents of the query, its position is
 * not read, and a dimension it lacks restricts no query
 * @param[in] temp Temporal value
 * @param[in] query Query box
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Temporal_merge_query_spans()
 */
Span *
temporal_merge_query_spans(const Temporal *temp, const Span *query,
  int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(query, NULL);
  if (! ensure_span_isof_type(query, T_TSTZSPAN))
    return NULL;
  return temporal_merge_query_spans_intl(temp, query, count);
}
/* GENERATED-BOXSPLIT-END temporal */

/*****************************************************************************
 * Boxes functions
 * These functions can be used for defining Multi-Entry Search Trees indexes
 * https://www.pgcon.org/2014/schedule/events/696.en.html
 * https://github.com/MobilityDB/mest
 *****************************************************************************/

/* GENERATED-BOXSPLIT-BEGIN tnumber — tools/codegen/inherited/generate.py from templates/boxsplit.c.tmpl; DO NOT EDIT BY HAND;
 * edit the template + manifest.d/boxsplit.yaml and re-run. */
/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return a singleton array of temporal boxes from a
 * temporal number instant
 * @param[in] inst Temporal instant
 */
TBox *
tnumberinst_tboxes(const TInstant *inst)
{
  assert(inst); assert(tnumber_type(inst->temptype));
  /* One bounding box per instant */
  TBox *result = palloc(sizeof(TBox));
  tnumberinst_set_tbox(inst, &result[0]);
  return result;
}

/**
 * @brief Return an array of temporal boxes from the instants
 * of a temporal number sequence with discrete interpolation
 * @param[in] seq Temporal sequence
 */
static TBox *
tnumberseq_disc_tboxes(const TSequence *seq)
{
  assert(seq); assert(tnumber_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  /* One bounding box per instant */
  TBox *result = palloc(sizeof(TBox) * seq->count);
  for (int i = 0; i < seq->count; i++)
    tnumberinst_set_tbox(TSEQUENCE_INST_N(seq, i), &result[i]);
  return result;
}

/**
 * @brief Return an array of temporal boxes from the segments of a
 * temporal number sequence with continuous interpolation (iterator function)
 * @param[in] seq Temporal sequence
 * @param[out] result Array of boxes
 * @return Number of elements in the ouput array
 */
static int
tnumberseq_cont_tboxes_iter(const TSequence *seq, TBox *result)
{
  assert(seq); assert(result); assert(tnumber_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tsequence_set_bbox_slice(seq, 0, 0, &result[0]);
    return 1;
  }

  /* One bounding box per segment */
  for (int i = 0; i < seq->count - 1; i++)
    tsequence_set_bbox_slice(seq, i, i + 1, &result[i]);
  return seq->count - 1;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of temporal boxes from the instants or segments
 * of a temporal number sequence
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] seq Temporal sequence
 * @param[out] count Number of elements in the output array
 */
TBox *
tnumberseq_tboxes(const TSequence *seq, int *count)
{
  assert(seq); assert(count); assert(tnumber_type(seq->temptype));

  /* Discrete case */
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
  {
    *count = seq->count;
    return tnumberseq_disc_tboxes(seq);
  }

  /* Continuous case */
  int nboxes = (seq->count == 1) ? 1 : seq->count - 1;
  TBox *result = palloc(sizeof(TBox) * nboxes);
  *count = tnumberseq_cont_tboxes_iter(seq, result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of temporal boxes from the segments of a
 * temporal number sequence set
 * @param[in] ss Temporal sequence set
 * @param[out] count Number of elements in the output array
 */
TBox *
tnumberseqset_tboxes(const TSequenceSet *ss, int *count)
{
  assert(ss); assert(count); assert(tnumber_type(ss->temptype));

  /* One bounding box per segment */
  TBox *result = palloc(sizeof(TBox) * ss->totalcount);
  int nboxes = 0;
  for (int i = 0; i < ss->count; i++)
    nboxes += tnumberseq_cont_tboxes_iter(TSEQUENCESET_SEQ_N(ss, i),
      &result[nboxes]);
  assert(nboxes <= ss->totalcount);
  *count = nboxes;
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of temporal boxes from the instants or segments
 * of a temporal number, its arguments valid
 * @details The internal twin of #tnumber_tboxes, which an index calls
 * @param[in] temp Temporal number
 * @param[out] count Number of values of the output array
 */
TBox *
tnumber_tboxes_intl(const Temporal *temp, int *count)
{
  assert(temp); assert(count); assert(tnumber_type(temp->temptype));
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tnumberinst_tboxes((TInstant *) temp);
    case TSEQUENCE:
      return tnumberseq_tboxes((TSequence *) temp, count);
    default: /* TSEQUENCESET */
      return tnumberseqset_tboxes((TSequenceSet *) temp, count);
  }
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of temporal boxes from the instants or segments
 * of a temporal number
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] temp Temporal number
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Tnumber_tboxes()
 */
TBox *
tnumber_tboxes(const Temporal *temp, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL);
  return tnumber_tboxes_intl(temp, count);
}

/*****************************************************************************/

/**
 * @brief Return an array of N temporal boxes from the instants
 * of a temporal number sequence with discrete interpolation
 * @param[in] seq Temporal sequence
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 */
static TBox *
tnumberseq_disc_split_n_tboxes(const TSequence *seq, int box_count, int *count)
{
  assert(seq); assert(count); assert(tnumber_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  assert(box_count > 0);

  /* One bounding box per instant */
  if (seq->count <= box_count)
  {
    *count = seq->count;
    return tnumberseq_disc_tboxes(seq);
  }

  /* One bounding box per several consecutive instants */
  TBox *result = palloc(sizeof(TBox) * box_count);
  /* Minimum number of input instants merged together in an output box */
  int size = seq->count / box_count;
  /* Number of output boxes that result from merging (size + 1) instants */
  int remainder = seq->count % box_count;
  int i = 0; /* Loop variable for input instants */
  for (int k = 0; k < box_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    tnumberinst_set_tbox(TSEQUENCE_INST_N(seq, i), &result[k]);
    for (int l = i + 1; l < j; l++)
    {
      TBox box;
      tnumberinst_set_tbox(TSEQUENCE_INST_N(seq, l), &box);
      tbox_expand(&box, &result[k]);
    }
    i = j;
  }
  assert(i == seq->count);
  *count = box_count;
  return result;
}

/**
 * @brief Return an array of N temporal boxes from the segments of a 
 * temporal number sequence with continuous interpolation (iterator function)
 * @param[in] seq Temporal number
 * @param[in] box_count Number of boxes
 * @param[out] result Array of boxes
 */
static int
tnumberseq_cont_split_n_tboxes_iter(const TSequence *seq, int box_count,
  TBox *result)
{
  assert(seq); assert(result); assert(tnumber_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);
  assert(box_count > 0);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tsequence_set_bbox_slice(seq, 0, 0, &result[0]);
    return 1;
  }

  /* One bounding box per segment */
  int nsegs = seq->count - 1;
  if (nsegs <= box_count)
    return tnumberseq_cont_tboxes_iter(seq, result);

  /* One bounding box per several consecutive segments */
  /* Minimum number of input segments merged together in an output box */
  int size = nsegs / box_count;
  /* Number of output boxes that result from merging (size + 1) segments */
  int remainder = nsegs % box_count;
  int i = 0; /* Loop variable for input segments */
  for (int k = 0; k < box_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    tsequence_set_bbox_slice(seq, i, j, &result[k]);
    i = j;
  }
  assert(i == nsegs);
  return box_count;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N temporal boxes from the instants or
 * segments of a temporal number sequence
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] seq Temporal sequence
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @return If `seq->count <= box_count`, the result contains one box
 * per instant or segment. Otherwise, consecutive instants or segments are
 * combined into a single box in the result to reach the given number of
 * boxes.
 */
TBox *
tnumberseq_split_n_tboxes(const TSequence *seq, int box_count, int *count)
{
  assert(seq); assert(count); assert(tnumber_type(seq->temptype));
  assert(box_count > 0);

  /* Discrete case */
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return tnumberseq_disc_split_n_tboxes(seq, box_count, count);

  /* Continuous case */
  int nboxes = (seq->count <= box_count) ?
    (seq->count == 1 ? 1 : seq->count - 1) : box_count;
  TBox *result = palloc(sizeof(TBox) * nboxes);
  *count = tnumberseq_cont_split_n_tboxes_iter(seq, box_count, result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N temporal boxes from the segments of a
 * temporal number sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @return If `ss->totalcount <= box_count`, the result contains one box per
 * segment. Otherwise, consecutive input segments are combined into a single box
 * in
 * the result to reach the given number of boxes.
 */
TBox *
tnumberseqset_split_n_tboxes(const TSequenceSet *ss, int box_count, int *count)
{
  assert(ss); assert(count); assert(tnumber_type(ss->temptype));
  assert(box_count > 0);

  /* One bounding box per segment */
  if (ss->totalcount <= box_count)
    return tnumberseqset_tboxes(ss, count);

  /* At most box_count boxes, the segments being more */
  TBox *result = palloc(sizeof(TBox) * box_count);

  /* Number of bounding boxes per composing sequence determined from the
   * proportion of seq->count and ss->totalcount */
  if (ss->count <= box_count)
  {
    int nboxes1 = 0;
    for (int i = 0; i < ss->count; i++)
    {
      bool end = false;
      const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
      int nboxes_seq = (int) (box_count * seq->count * 1.0 / ss->totalcount);
      if (! nboxes_seq)
        nboxes_seq = 1;
      if (nboxes_seq + nboxes1 >= box_count)
      {
        end = true;
        nboxes_seq = box_count - nboxes1;
      }
      nboxes1 += tnumberseq_cont_split_n_tboxes_iter(seq, nboxes_seq,
        &result[nboxes1]);
      if (end)
        break;
    }
    assert(nboxes1 <= box_count);
    *count = nboxes1;
    return result;
  }

  /* Merge consecutive sequences to reach the maximum number of boxes */
  /* Minimum number of sequences merged together in an output box */
  int size = ss->count / box_count;
  /* Number of output boxes that result from merging (size + 1) sequences */
  int remainder = ss->count % box_count;
  int i = 0; /* Loop variable for input sequences */
  for (int k = 0; k < box_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    tnumberseq_cont_split_n_tboxes_iter(TSEQUENCESET_SEQ_N(ss, i), 1,
      &result[k]);
    for (int l = i + 1; l < j; l++)
    {
      TBox box;
      tnumberseq_cont_split_n_tboxes_iter(TSEQUENCESET_SEQ_N(ss, l), 1, &box);
      tbox_expand(&box, &result[k]);
    }
    i = j;
  }
  assert(i == ss->count);
  *count = box_count;
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N temporal boxes obtained by merging
 * consecutive instants or segments of a temporal number, its arguments valid
 * @details The internal twin of #tnumber_split_n_tboxes, which an index calls
 * @param[in] temp Temporal number
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 */
TBox *
tnumber_split_n_tboxes_intl(const Temporal *temp, int box_count, int *count)
{
  assert(temp); assert(count); assert(tnumber_type(temp->temptype));
  assert(box_count > 0);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tnumberinst_tboxes((TInstant *) temp);
    case TSEQUENCE:
      return tnumberseq_split_n_tboxes((TSequence *) temp, box_count, count);
    default: /* TSEQUENCESET */
      return tnumberseqset_split_n_tboxes((TSequenceSet *) temp, box_count,
        count);
  }
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of N temporal boxes obtained by merging
 * consecutive instants or segments of a temporal number
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] temp Temporal number
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @errval NULL
 * @csqlfn #Tnumber_split_n_tboxes()
 */
TBox *
tnumber_split_n_tboxes(const Temporal *temp, int box_count, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL);
  if (! ensure_positive(box_count))
    return NULL;
  return tnumber_split_n_tboxes_intl(temp, box_count, count);
}

/*****************************************************************************
 * split_each_n_tboxes function
 *****************************************************************************/

/**
 * @brief Return an array of temporal boxes obtained by merging
 * consecutive instants of a temporal number sequence with discrete
 * interpolation
 * @param[in] seq Temporal sequence
 * @param[in] elem_count Number of input instants merged into an output box
 * @param[out] count Number of elements in the output array
 */
static TBox *
tnumberseq_disc_split_each_n_tboxes(const TSequence *seq, int elem_count,
  int *count)
{
  assert(seq); assert(count); assert(tnumber_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  assert(elem_count > 0);

  int nboxes = ceil((double) seq->count / (double) elem_count);
  TBox *result = palloc(sizeof(TBox) * nboxes);
  int k = 0;
  for (int i = 0; i < seq->count; ++i)
  {
    if (i % elem_count == 0)
      tnumberinst_set_tbox(TSEQUENCE_INST_N(seq, i), &result[k++]);
    else
    {
      TBox box;
      tnumberinst_set_tbox(TSEQUENCE_INST_N(seq, i), &box);
      tbox_expand(&box, &result[k - 1]);
    }
  }
  assert(k == nboxes);
  *count = k;
  return result;
}

/**
 * @brief Return an array of temporal boxes obtained by merging
 * consecutive segments of a temporal number sequence (iterator function)
 * @details The sequence has continuous interpolation.
 * @param[in] seq Temporal sequence
 * @param[in] elem_count Number of input segments merged into an output box
 * @param[out] result Array of boxes
 * @return Number of elements in the output array
 */
static int
tnumberseq_cont_split_each_n_tboxes_iter(const TSequence *seq,
  int elem_count, TBox *result)
{
  assert(seq); assert(result); assert(tnumber_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);
  assert(elem_count > 0);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tnumberseq_set_tbox(seq, &result[0]);
    return 1;
  }

  /* General case */
  int nboxes = ceil((double) (seq->count - 1) / (double) elem_count);
  for (int k = 0; k < nboxes; k++)
  {
    int first = k * elem_count;
    int last = Min(first + elem_count, seq->count - 1);
    tsequence_set_bbox_slice(seq, first, last, &result[k]);
  }
  return nboxes;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of temporal boxes obtained by merging
 * consecutive instants or segments of a temporal number sequence
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] seq Temporal sequence
 * @param[in] elem_count Number of input segments merged into an output box
 * @param[out] count Number of elements in the output array
 */
TBox *
tnumberseq_split_each_n_tboxes(const TSequence *seq, int elem_count,
  int *count)
{
  assert(seq); assert(count); assert(tnumber_type(seq->temptype));
  assert(elem_count > 0);

  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return tnumberseq_disc_split_each_n_tboxes(seq, elem_count, count);

  /* Number of instants or segments */
  int nelems = (seq->count == 1) ? 1 : seq->count - 1;
  int nboxes = ceil((double) nelems / (double) elem_count);
  TBox *result = palloc(sizeof(TBox) * nboxes);
  *count = tnumberseq_cont_split_each_n_tboxes_iter(seq, elem_count,
    result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of temporal boxes obtained by merging
 * consecutive segments of a temporal number sequence set 
 * @param[in] ss Temporal sequence set
 * @param[in] elem_count Number of input segments merged into an output box
 * @param[out] count Number of elements in the output array
 */
TBox *
tnumberseqset_split_each_n_tboxes(const TSequenceSet *ss, int elem_count,
  int *count)
{
  assert(ss); assert(count); assert(tnumber_type(ss->temptype));
  assert(elem_count > 0);

  /* Singleton sequence set */
  if (ss->count == 1)
    return tnumberseq_split_each_n_tboxes(TSEQUENCESET_SEQ_N(ss, 0),
      elem_count, count);

  /* Iterate for every composing sequence */
  int nboxes = 0;
  TBox *result = palloc(sizeof(TBox) * ss->totalcount);
  for (int i = 0; i < ss->count; ++i)
    nboxes += tnumberseq_cont_split_each_n_tboxes_iter(
      TSEQUENCESET_SEQ_N(ss, i), elem_count, &result[nboxes]);
  *count = nboxes;
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of temporal boxes obtained by merging
 * consecutive instants or segments of a temporal number, its arguments valid
 * @details The internal twin of #tnumber_split_each_n_tboxes, which an index
 * calls
 * @param[in] temp Temporal value
 * @param[in] elem_count Number of input instants or segments merged into an
 * output box
 * @param[out] count Number of values of the output array
 */
TBox *
tnumber_split_each_n_tboxes_intl(const Temporal *temp, int elem_count,
  int *count)
{
  assert(temp); assert(count); assert(tnumber_type(temp->temptype));
  assert(elem_count > 0);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tnumberinst_tboxes((TInstant *) temp);
    case TSEQUENCE:
      return tnumberseq_split_each_n_tboxes((TSequence *) temp, elem_count,
        count);
    default: /* TSEQUENCESET */
      return tnumberseqset_split_each_n_tboxes((TSequenceSet *) temp,
        elem_count, count);
  }
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of temporal boxes obtained by merging
 * consecutive instants or segments of a temporal number
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] temp Temporal value
 * @param[in] elem_count Number of input instants or segments merged into an
 * output box
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Tnumber_split_each_n_tboxes()
 */
TBox *
tnumber_split_each_n_tboxes(const Temporal *temp, int elem_count, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL);
  if (! ensure_positive(elem_count))
    return NULL;
  return tnumber_split_each_n_tboxes_intl(temp, elem_count, count);
}

/*****************************************************************************
 * merge_n_tboxes and merge_each_n_tboxes functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N temporal boxes obtained by merging the boxes of
 * the consecutive instants or segments of a temporal number, each time the
 * consecutive pair whose union adds the least size to theirs, its arguments
 * valid
 * @details The internal twin of #tnumber_merge_n_tboxes, which an index calls
 * @param[in] temp Temporal value
 * @param[in] box_count Number of boxes
 * @param[out] count Number of values of the output array
 */
TBox *
tnumber_merge_n_tboxes_intl(const Temporal *temp, int box_count, int *count)
{
  assert(temp); assert(count); assert(tnumber_type(temp->temptype));
  assert(box_count > 0);
  TBox *result = tnumber_tboxes_intl(temp, count);
  if (result)
    *count = bbox_merge_n(T_TBOX, result, *count, box_count, &tbox_adjust,
      &tbox_size);
  return result;
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of N temporal boxes obtained by merging the boxes of
 * the consecutive instants or segments of a temporal number, each time the
 * consecutive pair whose union adds the least size to theirs
 * @details The merge twin of #tnumber_split_n_tboxes, by #bbox_merge_n
 * @param[in] temp Temporal value
 * @param[in] box_count Number of boxes
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Tnumber_merge_n_tboxes()
 */
TBox *
tnumber_merge_n_tboxes(const Temporal *temp, int box_count, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL);
  if (! ensure_positive(box_count))
    return NULL;
  return tnumber_merge_n_tboxes_intl(temp, box_count, count);
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of temporal boxes obtained by merging the boxes of the
 * consecutive instants or segments of a temporal number, each time the
 * consecutive
 * pair whose union adds the least size to theirs, until a box holds on
 * average a number of them, its arguments valid
 * @details The internal twin of #tnumber_merge_each_n_tboxes, which an index
 * calls
 * @param[in] temp Temporal value
 * @param[in] elem_count Number of input instants or segments merged into an
 * output box on average
 * @param[out] count Number of values of the output array
 */
TBox *
tnumber_merge_each_n_tboxes_intl(const Temporal *temp, int elem_count,
  int *count)
{
  assert(temp); assert(count); assert(tnumber_type(temp->temptype));
  assert(elem_count > 0);
  TBox *result = tnumber_tboxes_intl(temp, count);
  if (result)
    *count = bbox_merge_each_n(T_TBOX, result, *count, elem_count,
      &tbox_adjust, &tbox_size);
  return result;
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of temporal boxes obtained by merging the boxes of the
 * consecutive instants or segments of a temporal number, each time the
 * consecutive
 * pair whose union adds the least size to theirs, until a box holds on
 * average a number of them
 * @details The merge twin of #tnumber_split_each_n_tboxes, by
 * #bbox_merge_each_n
 * @param[in] temp Temporal value
 * @param[in] elem_count Number of input instants or segments merged into an
 * output box on average
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Tnumber_merge_each_n_tboxes()
 */
TBox *
tnumber_merge_each_n_tboxes(const Temporal *temp, int elem_count, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL);
  if (! ensure_positive(elem_count))
    return NULL;
  return tnumber_merge_each_n_tboxes_intl(temp, elem_count, count);
}

/*****************************************************************************
 * merge_query_tboxes function
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of temporal boxes obtained by merging
 * the boxes of the consecutive instants or segments of a
 * temporal number into those a query of the extents of a query box
 * is expected to read least, its arguments valid
 * @details The internal twin of #tnumber_merge_query_tboxes, which an index
 * calls
 * @param[in] temp Temporal value
 * @param[in] query Query box
 * @param[out] count Number of values of the output array
 */
TBox *
tnumber_merge_query_tboxes_intl(const Temporal *temp, const TBox *query,
  int *count)
{
  assert(temp); assert(query); assert(count);
  assert(tnumber_type(temp->temptype));
  assert(ensure_valid_tnumber_tbox(temp, query));
  TBox *result = tnumber_tboxes_intl(temp, count);
  if (result)
    *count = bbox_merge_query(T_TBOX, result, *count, query, &tbox_adjust,
      &tbox_query_size);
  return result;
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of temporal boxes obtained by merging
 * the boxes of the consecutive instants or segments of a
 * temporal number into those a query of the extents of a query box
 * is expected to read least
 * @details The query twin of #tnumber_merge_n_tboxes, by #bbox_merge_query:
 * the widths of the query box are the extents of the query, its position is
 * not read, and a dimension it lacks restricts no query
 * @param[in] temp Temporal value
 * @param[in] query Query box
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Tnumber_merge_query_tboxes()
 */
TBox *
tnumber_merge_query_tboxes(const Temporal *temp, const TBox *query,
  int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL); VALIDATE_NOT_NULL(query, NULL);
  if (! ensure_valid_tnumber_tbox(temp, query))
    return NULL;
  return tnumber_merge_query_tboxes_intl(temp, query, count);
}
/* GENERATED-BOXSPLIT-END tnumber */

/*****************************************************************************
 * Generic bounding box functions for temporal types
 * The inclusive/exclusive bounds are taken into account for the comparisons
 *****************************************************************************/

/**
 * @brief Generic bounding box function for a temporal value and a timestamptz
 * span
 */
bool
boxop_temporal_tstzspan(const Temporal *temp, const Span *s,
  bool (*func)(const Span *, const Span *), bool invert)
{
  assert(temp); assert(s); assert(func);
  Span s1;
  temporal_set_tstzspan(temp, &s1);
  return invert ? func(s, &s1) : func(&s1, s);

}

/**
 * @brief Generic bounding box function for two temporal values
 */
bool
boxop_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const Span *, const Span *))
{
  assert(temp1); assert(temp2); assert(func);
  Span s1, s2;
  temporal_set_tstzspan(temp1, &s1);
  temporal_set_tstzspan(temp2, &s2);
  return func(&s1, &s2);
}

/**
 * @brief Return the number of parts a temporal value is defined on: the sequences of a
 * sequence set, the instants of a discrete sequence, and one part for anything else
 */
static int
temporal_time_part_count(const Temporal *temp)
{
  if (temp->subtype == TSEQUENCESET)
    return ((const TSequenceSet *) temp)->count;
  if (temp->subtype == TSEQUENCE && MEOS_FLAGS_DISCRETE_INTERP(temp->flags))
    return ((const TSequence *) temp)->count;
  return 1;
}

/**
 * @brief Return in the last argument the @p i-th part a temporal value is defined on,
 * @p whole being its bounding span, which is the part itself where it has only one
 */
static void
temporal_time_part_n(const Temporal *temp, int i, const Span *whole, Span *result)
{
  if (i >= temporal_time_part_count(temp))
    return;
  if (temp->subtype == TSEQUENCESET)
    *result = TSEQUENCESET_SEQ_N((const TSequenceSet *) temp, i)->period;
  else if (temp->subtype == TSEQUENCE && MEOS_FLAGS_DISCRETE_INTERP(temp->flags))
  {
    TimestampTz t = TSEQUENCE_INST_N((const TSequence *) temp, i)->t;
    span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
      T_TIMESTAMPTZ, T_TSTZSPAN, result);
  }
  else
    *result = *whole;
  return;
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time of two temporal values actually overlaps
 * @details Unlike @ref overlaps_temporal_temporal, which tests the bounding
 * spans, this walks the parts each value is defined on, so two values whose
 * active parts fall in each other's gaps do not count as overlapping, and
 * neither do two sets of instants that interleave without sharing one. The
 * periods are read in place from the sequences, so there is no allocation.
 * @param[in] temp1,temp2 Temporal values
 */
bool
temporal_time_overlaps(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp1, false); VALIDATE_NOT_NULL(temp2, false);
  /* Cheap bounding-span reject first (no allocation). The bounding span is also
   * the exact time of an instant or a single sequence, which have no gaps. */
  Span s1, s2;
  temporal_set_tstzspan(temp1, &s1);
  temporal_set_tstzspan(temp2, &s2);
  if (! overlaps_span_span(&s1, &s2))
    return false;
  /* Walk the parts each value is actually defined on, in the order it holds them: the
   * periods of a sequence set, and the instants of a discrete sequence, which is a set of
   * instants and holds nothing between them. Both lists ascend and neither overlaps
   * itself, so one pass over the two answers the question */
  int n1 = temporal_time_part_count(temp1), n2 = temporal_time_part_count(temp2);
  int i = 0, j = 0;
  Span p1, p2;
  temporal_time_part_n(temp1, i, &s1, &p1);
  temporal_time_part_n(temp2, j, &s2, &p2);
  while (i < n1 && j < n2)
  {
    if (overlaps_span_span(&p1, &p2))
      return true;
    /* The part ending first can meet nothing further in the other value */
    if (datum_le(p1.upper, p2.upper, p1.basetype))
      temporal_time_part_n(temp1, ++i, &s1, &p1);
    else
      temporal_time_part_n(temp2, ++j, &s2, &p2);
  }
  return false;
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a timestamptz span contains the time span of a
 * temporal value
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Contains_tstzspan_temporal()
 */
bool
contains_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, false); VALIDATE_TSTZSPAN(s, false);
  return boxop_temporal_tstzspan(temp, s, &contains_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time span of a temporal value contains a
 * timestamptz span
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Contains_temporal_tstzspan()
 */
bool
contains_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, false); VALIDATE_TSTZSPAN(s, false);
  return boxop_temporal_tstzspan(temp, s, &contains_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time span of the first temporal value contains the
 * one of the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Contains_temporal_temporal()
 */
bool
contains_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp1, false); VALIDATE_NOT_NULL(temp2, false);
  return boxop_temporal_temporal(temp1, temp2, &contains_span_span);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a timestamptz span is contained the time span of
 * a temporal value
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Contained_tstzspan_temporal()
 */
bool
contained_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, false); VALIDATE_TSTZSPAN(s, false);
  return boxop_temporal_tstzspan(temp, s, &contained_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time span of a temporal value is contained
 * in a timestamptz span
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Contained_temporal_tstzspan()
 */
bool
contained_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, false); VALIDATE_TSTZSPAN(s, false);
  return boxop_temporal_tstzspan(temp, s, &contained_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time span of the first temporal value is
 * contained in the one of the second temporal value
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Contained_temporal_temporal()
 */
bool
contained_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp1, false); VALIDATE_NOT_NULL(temp2, false);
  return boxop_temporal_temporal(temp1, temp2, &contained_span_span);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a timestamptz span and the time span of a
 * temporal value overlap
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Overlaps_tstzspan_temporal()
 */
bool
overlaps_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, false); VALIDATE_TSTZSPAN(s, false);
  return boxop_temporal_tstzspan(temp, s, &overlaps_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time span of a temporal value and a
 * timestamptz span overlap
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Overlaps_temporal_tstzspan()
 */
bool
overlaps_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, false); VALIDATE_TSTZSPAN(s, false);
  return boxop_temporal_tstzspan(temp, s, &overlaps_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time spans of two temporal values overlap
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Overlaps_temporal_temporal()
 */
bool
overlaps_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp1, false); VALIDATE_NOT_NULL(temp2, false);
  return boxop_temporal_temporal(temp1, temp2, &overlaps_span_span);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a timestamptz span and the time span of a
 * temporal value are equal
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Same_tstzspan_temporal()
 */
bool
same_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, false); VALIDATE_TSTZSPAN(s, false);
  return boxop_temporal_tstzspan(temp, s, &span_eq, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time span of a temporal value and a timestamptz
 * span are equal
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Same_temporal_tstzspan()
 */
bool
same_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, false); VALIDATE_TSTZSPAN(s, false);
  return boxop_temporal_tstzspan(temp, s, &span_eq, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time spans of two temporal values are equal
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Same_temporal_temporal()
 */
bool
same_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp1, false); VALIDATE_NOT_NULL(temp2, false);
  return boxop_temporal_temporal(temp1, temp2, &span_eq);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a timestamptz span and the time span of a temporal
 * value are adjacent
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Adjacent_tstzspan_temporal()
 */
bool
adjacent_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, false); VALIDATE_TSTZSPAN(s, false);
  return boxop_temporal_tstzspan(temp, s, &adjacent_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time span of a temporal value and a timestamptz
 * span are adjacent
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Adjacent_temporal_tstzspan()
 */
bool
adjacent_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, false); VALIDATE_TSTZSPAN(s, false);
  return boxop_temporal_tstzspan(temp, s, &adjacent_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time spans of two temporal values are adjacent
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Adjacent_temporal_temporal()
 */
bool
adjacent_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp1, false); VALIDATE_NOT_NULL(temp2, false);
  return boxop_temporal_temporal(temp1, temp2, &adjacent_span_span);
}

/*****************************************************************************
 * Generic bounding box functions for temporal number types
 *****************************************************************************/

/**
 * @brief Generic bounding box function for a temporal number and a span
 */
bool
boxop_tnumber_numspan(const Temporal *temp, const Span *s,
  bool (*func)(const Span *, const Span *), bool invert)
{
  assert(temp); assert(s); assert(func);
  Span s1;
  tnumber_set_span(temp, &s1);
  return invert ? func(s, &s1) : func(&s1, s);
}

/**
 * @brief Generic bounding box function for a temporal number and a temporal
 * box
 */
bool
boxop_tnumber_tbox(const Temporal *temp, const TBox *box,
  bool (*func)(const TBox *, const TBox *), bool invert)
{
  assert(temp); assert(box); assert(func);
  TBox box1;
  tnumber_set_tbox(temp, &box1);
  return invert ? func(box, &box1) : func(&box1, box);
}

/**
 * @brief Generic bounding box function for two temporal numbers
 */
bool
boxop_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const TBox *, const TBox *))
{
  assert(temp1); assert(temp2); assert(func);
  TBox box1, box2;
  tnumber_set_tbox(temp1, &box1);
  tnumber_set_tbox(temp2, &box2);
  return func(&box1, &box2);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a number span contains the value span of a temporal
 * number
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Contains_numspan_tnumber()
 */
bool
contains_numspan_tnumber(const Span *s, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_numspan(temp, s))
    return false;
  return boxop_tnumber_numspan(temp, s, &contains_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the value span of a temporal number contains a number
 * span
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Contains_tnumber_numspan()
 */
bool
contains_tnumber_numspan(const Temporal *temp, const Span *s)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_numspan(temp, s))
    return false;
  return boxop_tnumber_numspan(temp, s, &contains_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a temporal box contains the bounding box of a
 * temporal number
 * @param[in] box Bounding box
 * @param[in] temp Temporal value
 * @csqlfn #Contains_tbox_tnumber()
 */
bool
contains_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tbox(temp, box))
    return false;
  return boxop_tnumber_tbox(temp, box, &contains_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding box of a temporal number contains a
 * temporal box
 * @param[in] temp Temporal value
 * @param[in] box Bounding box
 * @csqlfn #Contains_tnumber_tbox()
 */
bool
contains_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tbox(temp, box))
    return false;
  return boxop_tnumber_tbox(temp, box, &contains_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding box of the first temporal number contains
 * the one of the second temporal number
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Contains_tnumber_tnumber()
 */
bool
contains_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tnumber(temp1, temp2))
    return false;
  return boxop_tnumber_tnumber(temp1, temp2, &contains_tbox_tbox);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a number span is contained in the value span of a
 * temporal number
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Contained_numspan_tnumber()
 */
bool
contained_numspan_tnumber(const Span *s, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_numspan(temp, s))
    return false;
  return boxop_tnumber_numspan(temp, s, &contained_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the value span of a temporal number is contained in a
 * number span
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Contained_tnumber_numspan()
 */
bool
contained_tnumber_numspan(const Temporal *temp, const Span *s)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_numspan(temp, s))
    return false;
  return boxop_tnumber_numspan(temp, s, &contained_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a temporal box is contained in the bounding box of a
 * temporal number
 * @param[in] box Bounding box
 * @param[in] temp Temporal value
 * @csqlfn #Contained_tbox_tnumber()
 */
bool
contained_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tbox(temp, box))
    return false;
  return boxop_tnumber_tbox(temp, box, &contained_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding box of a temporal number is contained in
 * a temporal box
 * @param[in] temp Temporal value
 * @param[in] box Bounding box
 * @csqlfn #Contained_tnumber_tbox()
 */
bool
contained_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tbox(temp, box))
    return false;
  return boxop_tnumber_tbox(temp, box, &contained_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding box of the first temporal number is
 * contained in the one of the second temporal number
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Contained_tnumber_tnumber()
 */
bool
contained_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tnumber(temp1, temp2))
    return false;
  return boxop_tnumber_tnumber(temp1, temp2, &contained_tbox_tbox);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a number span and the value span of a temporal number
 * overlap
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Overlaps_numspan_tnumber()
 */
bool
overlaps_numspan_tnumber(const Span *s, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_numspan(temp, s))
    return false;
  return boxop_tnumber_numspan(temp, s, &overlaps_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the value span of a temporal number and the number
 * span overlap
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Overlaps_tnumber_numspan()
 */
bool
overlaps_tnumber_numspan(const Temporal *temp, const Span *s)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_numspan(temp, s))
    return false;
  return boxop_tnumber_numspan(temp, s, &overlaps_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a temporal box and the bounding box of a temporal
 * number overlap
 * @param[in] box Bounding box
 * @param[in] temp Temporal value
 * @csqlfn #Overlaps_tbox_tnumber()
 */
bool
overlaps_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tbox(temp, box))
    return false;
  return boxop_tnumber_tbox(temp, box, &overlaps_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding box of a temporal number and a temporal
 * box overlap
 * @param[in] temp Temporal value
 * @param[in] box Bounding box
 * @csqlfn #Overlaps_tnumber_tbox()
 */
bool
overlaps_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tbox(temp, box))
    return false;
  return boxop_tnumber_tbox(temp, box, &overlaps_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding boxes of two temporal numbers overlap
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Overlaps_tnumber_tnumber()
 */
bool
overlaps_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tnumber(temp1, temp2))
    return false;
  return boxop_tnumber_tnumber(temp1, temp2, &overlaps_tbox_tbox);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a number span and the value span of a temporal number
 * are equal
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Same_numspan_tnumber()
 */
bool
same_numspan_tnumber(const Span *s, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_numspan(temp, s))
    return false;
  return boxop_tnumber_numspan(temp, s, &span_eq, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the value span of a temporal number and a number span
 * are equal
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Same_tnumber_numspan()
 */
bool
same_tnumber_numspan(const Temporal *temp, const Span *s)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_numspan(temp, s))
    return false;
  return boxop_tnumber_numspan(temp, s, &span_eq, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a temporal box and the bounding box of a temporal
 * number are equal in the common dimensions
 * @param[in] box Bounding box
 * @param[in] temp Temporal value
 * @csqlfn #Same_tbox_tnumber()
 */
bool
same_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tbox(temp, box))
    return false;
  return boxop_tnumber_tbox(temp, box, &same_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding box of a temporal number and a temporal
 * box are equal in the common dimensions
 * @param[in] temp Temporal value
 * @param[in] box Bounding box
 * @csqlfn #Same_tnumber_tbox()
 */
bool
same_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tbox(temp, box))
    return false;
  return boxop_tnumber_tbox(temp, box, &same_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding boxes of two temporal numbers are equal
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Same_tnumber_tnumber()
 */
bool
same_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tnumber(temp1, temp2))
    return false;
  return boxop_tnumber_tnumber(temp1, temp2, &same_tbox_tbox);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a number span and the value span of a temporal number
 * are adjacent
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Adjacent_numspan_tnumber()
 */
bool
adjacent_numspan_tnumber(const Span *s, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_numspan(temp, s))
    return false;
  return boxop_tnumber_numspan(temp, s, &adjacent_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the value span of a temporal number and a number span
 * are adjacent
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Adjacent_tnumber_numspan()
 */
bool
adjacent_tnumber_numspan(const Temporal *temp, const Span *s)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_numspan(temp, s))
    return false;
  return boxop_tnumber_numspan(temp, s, &adjacent_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a temporal box and the bounding box of a temporal
 * number are adjacent
 * @param[in] box Bounding box
 * @param[in] temp Temporal value
 * @csqlfn #Adjacent_tbox_tnumber()
 */
bool
adjacent_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tbox(temp, box))
    return false;
  return boxop_tnumber_tbox(temp, box, &adjacent_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding box of a temporal number and a temporal
 * box are adjacent
 * @param[in] temp Temporal value
 * @param[in] box Bounding box
 * @csqlfn #Adjacent_tnumber_tbox()
 */
bool
adjacent_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tbox(temp, box))
    return false;
  return boxop_tnumber_tbox(temp, box, &adjacent_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding boxes of two temporal numbers are
 * adjacent
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Adjacent_tnumber_tnumber()
 */
bool
adjacent_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tnumber(temp1, temp2))
    return false;
  return boxop_tnumber_tnumber(temp1, temp2, &adjacent_tbox_tbox);
}

/*****************************************************************************
 * Double sorting split of a set of bounding boxes
 *
 * The split of Korotkov, "A new double sorting-based node splitting algorithm
 * for R-tree", http://syrcose.ispras.ru/2011/files/SYRCoSE2011_Proceedings.pdf
 * #page=36, which the GiST operator classes of PostgreSQL use for their boxes.
 * It is shared by the GiST operator classes of the extension and the in-memory
 * RTree of MEOS.
 *****************************************************************************/

/**
 * @brief Compare common entries by their deltas
 * @note We assume the deltas can't be NaN
 */
int
common_entry_cmp(const void *i1, const void *i2)
{
  double delta1 = ((const CommonEntry *) i1)->delta;
  double delta2 = ((const CommonEntry *) i2)->delta;
  if (delta1 < delta2)
    return -1;
  else if (delta1 > delta2)
    return 1;
  else
    return 0;
}

/**
 * @brief Interval comparison function by lower bound of the intervals
 */
static int
interval_cmp_lower(const void *i1, const void *i2)
{
  return pg_float8_cmp(((const SplitInterval *) i1)->lower,
    ((const SplitInterval *) i2)->lower);
}

/**
 * @brief Interval comparison function by upper bound of the intervals
 */
static int
interval_cmp_upper(const void *i1, const void *i2)
{
  return pg_float8_cmp(((const SplitInterval *) i1)->upper,
    ((const SplitInterval *) i2)->upper);
}

/**
 * @brief Replace a negative (or NaN) value with zero
 */
static inline float
non_negative(float val)
{
  return (val >= 0.0f) ? val : 0.0f;
}

/* Axes a box can be split on: the value span or X, Y, Z and time */
#define SPLIT_AXIS_X 0
#define SPLIT_AXIS_Y 1
#define SPLIT_AXIS_Z 2
#define SPLIT_AXIS_T 3

/**
 * @brief Project a bounding box on a split axis
 * @details A temporal box has a value span on #SPLIT_AXIS_X and a period on
 * #SPLIT_AXIS_T; a spatiotemporal box, and a point cloud box sharing its
 * layout, have X, Y, Z and a period
 */
static void
bbox_split_project(MeosType bboxtype, const void *box, int axis,
  double *lower, double *upper)
{
  if (bboxtype == T_TBOX)
  {
    const TBox *b = (const TBox *) box;
    if (axis == SPLIT_AXIS_X)
    {
      *lower = DatumGetFloat8(b->span.lower);
      *upper = DatumGetFloat8(b->span.upper);
    }
    else
    {
      *lower = (double) DatumGetTimestampTz(b->period.lower);
      *upper = (double) DatumGetTimestampTz(b->period.upper);
    }
    return;
  }
  /* T_STBOX or T_TPCBOX, which share the STBox prefix */
  const STBox *b = (const STBox *) box;
  switch (axis)
  {
    case SPLIT_AXIS_X: *lower = b->xmin; *upper = b->xmax; return;
    case SPLIT_AXIS_Y: *lower = b->ymin; *upper = b->ymax; return;
    case SPLIT_AXIS_Z: *lower = b->zmin; *upper = b->zmax; return;
    default:
      *lower = (double) DatumGetTimestampTz(b->period.lower);
      *upper = (double) DatumGetTimestampTz(b->period.upper);
      return;
  }
}

/**
 * @brief Return the axes a set of boxes is split on: those its boxes carry
 * @details A box lacking an axis projects every entry on the same empty
 * interval there, whose range is zero, so a split read on it has no overlap
 * to compare but NaN and must never be considered
 */
static int
bbox_split_axes(MeosType bboxtype, const void *box, int axes[4])
{
  int n = 0;
  if (bboxtype == T_TBOX)
  {
    int16 flags = ((const TBox *) box)->flags;
    if (MEOS_FLAGS_GET_X(flags))
      axes[n++] = SPLIT_AXIS_X;
    if (MEOS_FLAGS_GET_T(flags))
      axes[n++] = SPLIT_AXIS_T;
    return n;
  }
  int16 flags = ((const STBox *) box)->flags;
  if (MEOS_FLAGS_GET_X(flags))
  {
    axes[n++] = SPLIT_AXIS_X;
    axes[n++] = SPLIT_AXIS_Y;
  }
  if (MEOS_FLAGS_GET_Z(flags))
    axes[n++] = SPLIT_AXIS_Z;
  if (MEOS_FLAGS_GET_T(flags))
    axes[n++] = SPLIT_AXIS_T;
  return n;
}

/**
 * @brief Information about the split selected so far
 */
typedef struct
{
  int entriesCount;   /**< Total number of entries being split */
  double range[4];    /**< Width of the projection of all the entries on
                           each axis */
  bool first;         /**< True if no split was selected yet */
  double leftUpper;   /**< Upper bound of the left interval */
  double rightLower;  /**< Lower bound of the right interval */
  float4 ratio;
  float4 overlap;
  int axis;           /**< Axis of the split */
} BboxSplitContext;

/**
 * @brief Consider replacing the split selected so far with a better one
 */
static void
bbox_split_consider(BboxSplitContext *context, int axis, double rightLower,
  int minLeftCount, double leftUpper, int maxLeftCount)
{
  /* Distribution ratio assuming the most uniform distribution of the common
   * entries */
  int leftCount;
  if (minLeftCount >= (context->entriesCount + 1) / 2)
    leftCount = minLeftCount;
  else if (maxLeftCount <= context->entriesCount / 2)
    leftCount = maxLeftCount;
  else
    leftCount = context->entriesCount / 2;
  int rightCount = context->entriesCount - leftCount;

  /* Ratio of the split: the smaller group over the number of entries */
  float4 ratio = ((float4) Min(leftCount, rightCount)) /
    ((float4) context->entriesCount);
  if (ratio <= LIMIT_RATIO)
    return;

  double range = context->range[axis];
  float4 overlap = (float4) ((leftUpper - rightLower) / range);
  bool selectthis = false;
  if (context->first)
    selectthis = true;
  else if (context->axis == axis)
  {
    /* Within the same axis, a smaller overlap, or the same overlap and a
     * better ratio */
    if (overlap < context->overlap ||
        (overlap == context->overlap && ratio > context->ratio))
      selectthis = true;
  }
  else
  {
    /* Across axes, a smaller non-negative overlap, or the same one and a
     * larger range. Where leaf boxes do not overlap, non-overlapping splits
     * are frequent and tend to be along one axis, which makes the boxes very
     * long on the other; the range as second criterion makes them squarer */
    if (non_negative(overlap) < non_negative(context->overlap) ||
        (range > context->range[context->axis] &&
         non_negative(overlap) <= non_negative(context->overlap)))
      selectthis = true;
  }
  if (selectthis)
  {
    context->first = false;
    context->ratio = ratio;
    context->overlap = overlap;
    context->rightLower = rightLower;
    context->leftUpper = leftUpper;
    context->axis = axis;
  }
  return;
}

/**
 * @brief Place a box in a group, growing the group's bounding box
 */
static void
bbox_split_place(void *groupbox, int *groupcount, const void *box,
  size_t bboxsize, void (*bbox_adjust)(void *, void *))
{
  if (*groupcount > 0)
    bbox_adjust(groupbox, (void *) box);
  else
    memcpy(groupbox, box, bboxsize);
  (*groupcount)++;
  return;
}

/**
 * @ingroup meos_internal_box_index
 * @brief Split a set of bounding boxes in two groups
 * @details The double sorting split of Korotkov. Each box is projected as an
 * interval on each axis the boxes carry, the ways to split the intervals of
 * an axis into two groups are considered, and the split of least overlap
 * whose smaller group holds more than #LIMIT_RATIO of the boxes is selected.
 * The boxes that fit only one group of that split go to it; those that fit
 * both ("common entries") are distributed by least penalty, the most
 * ambiguous first. When no split reaches the ratio, the first half of the
 * boxes goes left and the rest right.
 * @param[in] bboxtype Type of the boxes: T_TBOX, T_STBOX or T_TPCBOX
 * @param[in] boxes Array of pointers to the boxes
 * @param[in] count Number of boxes, at least two
 * @param[in] bbox_adjust Function growing its first box to include the second
 * @param[in] bbox_penalty Function returning the growth of its first box when
 * the second is added
 * @param[out] left For each box, true if it goes to the left group
 */
void
bbox_split(MeosType bboxtype, void **boxes, int count,
  void (*bbox_adjust)(void *, void *), double (*bbox_penalty)(void *, void *),
  bool *left)
{
  assert(boxes); assert(left); assert(count >= 2);
  assert(bboxtype == T_TBOX || bboxtype == T_STBOX
#if POINTCLOUD
    || bboxtype == T_TPCBOX
#endif
    );
  size_t bboxsize = bbox_get_size(bboxtype);

  /* The bounding box of all the entries gives the range of every axis */
  bboxunion all;
  memcpy(&all, boxes[0], bboxsize);
  for (int i = 1; i < count; i++)
    bbox_adjust(&all, boxes[i]);
  BboxSplitContext context;
  memset(&context, 0, sizeof(BboxSplitContext));
  context.entriesCount = count;
  context.first = true;
  int axes[4];
  int naxes = bbox_split_axes(bboxtype, boxes[0], axes);
  for (int a = 0; a < naxes; a++)
  {
    double lower, upper;
    bbox_split_project(bboxtype, &all, axes[a], &lower, &upper);
    context.range[axes[a]] = upper - lower;
    /* MEOS refuses a non-finite number where one enters it, so no bound is
     * NaN */
    assert(! isnan(context.range[axes[a]]));
  }

  SplitInterval *intervalsLower = palloc(sizeof(SplitInterval) * count);
  SplitInterval *intervalsUpper = palloc(sizeof(SplitInterval) * count);
  for (int a = 0; a < naxes; a++)
  {
    int axis = axes[a];
    /* An axis on which every box projects on the same point separates
     * nothing */
    if (context.range[axis] <= 0.0)
      continue;
    for (int i = 0; i < count; i++)
      bbox_split_project(bboxtype, boxes[i], axis, &intervalsLower[i].lower,
        &intervalsLower[i].upper);
    memcpy(intervalsUpper, intervalsLower, sizeof(SplitInterval) * count);
    qsort(intervalsLower, (size_t) count, sizeof(SplitInterval),
      interval_cmp_lower);
    qsort(intervalsUpper, (size_t) count, sizeof(SplitInterval),
      interval_cmp_upper);

    /* The left and right intervals are of the form (min, a) and (b, max).
     * First the splits where b is the lower bound of an entry, each with the
     * smallest possible a */
    int i1 = 0, i2 = 0;
    double rightLower = intervalsLower[i1].lower;
    double leftUpper = intervalsUpper[i2].lower;
    while (true)
    {
      while (i1 < count && FLOAT8_EQ(rightLower, intervalsLower[i1].lower))
      {
        if (FLOAT8_LT(leftUpper, intervalsLower[i1].upper))
          leftUpper = intervalsLower[i1].upper;
        i1++;
      }
      if (i1 >= count)
        break;
      rightLower = intervalsLower[i1].lower;
      while (i2 < count && FLOAT8_LE(intervalsUpper[i2].upper, leftUpper))
        i2++;
      bbox_split_consider(&context, axis, rightLower, i1, leftUpper, i2);
    }

    /* Then the splits where a is the upper bound of an entry, each with the
     * greatest possible b */
    i1 = count - 1;
    i2 = count - 1;
    rightLower = intervalsLower[i1].upper;
    leftUpper = intervalsUpper[i2].upper;
    while (true)
    {
      while (i2 >= 0 && FLOAT8_EQ(leftUpper, intervalsUpper[i2].upper))
      {
        if (FLOAT8_GT(rightLower, intervalsUpper[i2].lower))
          rightLower = intervalsUpper[i2].lower;
        i2--;
      }
      if (i2 < 0)
        break;
      leftUpper = intervalsUpper[i2].upper;
      while (i1 >= 0 && FLOAT8_GE(intervalsLower[i1].lower, rightLower))
        i1--;
      bbox_split_consider(&context, axis, rightLower, i1 + 1, leftUpper,
        i2 + 1);
    }
  }
  pfree(intervalsLower); pfree(intervalsUpper);

  /* No acceptable split: the first half left, the rest right */
  if (context.first)
  {
    for (int i = 0; i < count; i++)
      left[i] = (i < count / 2);
    return;
  }

  /* The entries that fit one group only go to it, the others are collected */
  bboxunion leftbox, rightbox;
  int nleft = 0, nright = 0, ncommon = 0;
  CommonEntry *common = palloc(sizeof(CommonEntry) * count);
  for (int i = 0; i < count; i++)
  {
    double lower, upper;
    bbox_split_project(bboxtype, boxes[i], context.axis, &lower, &upper);
    if (FLOAT8_LE(upper, context.leftUpper))
    {
      if (FLOAT8_GE(lower, context.rightLower))
        common[ncommon++].index = i;
      else
      {
        left[i] = true;
        bbox_split_place(&leftbox, &nleft, boxes[i], bboxsize, bbox_adjust);
      }
    }
    else
    {
      /* An entry not fitting the left group fits the right one */
      assert(FLOAT8_GE(lower, context.rightLower));
      left[i] = false;
      bbox_split_place(&rightbox, &nright, boxes[i], bboxsize, bbox_adjust);
    }
  }

  /* The common entries, the most ambiguous first, each to the group it
   * enlarges least unless a group needs it to reach the ratio */
  if (ncommon > 0)
  {
    int m = (int) ceil(LIMIT_RATIO * (double) count);
    for (int j = 0; j < ncommon; j++)
    {
      void *box = boxes[common[j].index];
      common[j].delta = (nleft == 0 || nright == 0) ? 0.0 :
        fabs(bbox_penalty(&leftbox, box) - bbox_penalty(&rightbox, box));
    }
    qsort(common, (size_t) ncommon, sizeof(CommonEntry), common_entry_cmp);
    for (int j = 0; j < ncommon; j++)
    {
      int idx = common[j].index;
      void *box = boxes[idx];
      bool goleft;
      if (nleft + (ncommon - j) <= m)
        goleft = true;
      else if (nright + (ncommon - j) <= m)
        goleft = false;
      else if (nleft == 0 || nright == 0)
        goleft = (nleft == 0);
      else
        goleft = bbox_penalty(&leftbox, box) < bbox_penalty(&rightbox, box);
      left[idx] = goleft;
      if (goleft)
        bbox_split_place(&leftbox, &nleft, box, bboxsize, bbox_adjust);
      else
        bbox_split_place(&rightbox, &nright, box, bboxsize, bbox_adjust);
    }
  }
  pfree(common);
  return;
}


/*****************************************************************************
 * Merge of consecutive bounding boxes
 *****************************************************************************/

/**
 * @brief Pair of consecutive boxes of a merge, ordered by the size their
 * union adds
 */
typedef struct
{
  double cost;    /**< Size the union of the two boxes adds to theirs */
  int left;       /**< Position of the left box of the pair */
  int right;      /**< Position of the right box of the pair */
  int lversion;   /**< Version of the left box when the cost was read */
  int rversion;   /**< Version of the right box when the cost was read */
} BoxPair;

/**
 * @brief Return true if a pair of boxes is merged before another one: the one
 * whose union adds less, or of equal cost the one further left, a total order
 * so that the merges do not depend on the order the heap holds equal pairs in
 */
static bool
boxpair_lt(const BoxPair *pair1, const BoxPair *pair2)
{
  return pair1->cost < pair2->cost ||
    (pair1->cost == pair2->cost && pair1->left < pair2->left);
}

/**
 * @brief Restore the order of a heap of box pairs from a position upwards
 */
static void
boxpair_heap_up(BoxPair *heap, int i)
{
  while (i > 0)
  {
    int parent = (i - 1) / 2;
    if (! boxpair_lt(&heap[i], &heap[parent]))
      break;
    BoxPair tmp = heap[parent]; heap[parent] = heap[i]; heap[i] = tmp;
    i = parent;
  }
  return;
}

/**
 * @brief Restore the order of a heap of box pairs from a position downwards
 */
static void
boxpair_heap_down(BoxPair *heap, int count, int i)
{
  while (true)
  {
    int least = i, l = 2 * i + 1, r = 2 * i + 2;
    if (l < count && boxpair_lt(&heap[l], &heap[least]))
      least = l;
    if (r < count && boxpair_lt(&heap[r], &heap[least]))
      least = r;
    if (least == i)
      break;
    BoxPair tmp = heap[least]; heap[least] = heap[i]; heap[i] = tmp;
    i = least;
  }
  return;
}

/**
 * @brief Return the size the union of two boxes adds to the sizes of both,
 * measured for a query box when one is given
 */
static double
bbox_merge_cost(const char *box1, const char *box2, char *unionbox,
  size_t bboxsize, const void *query, void (*bbox_adjust)(void *, void *),
  double (*bbox_size)(const void *),
  double (*bbox_query_size)(const void *, const void *))
{
  memcpy(unionbox, box1, bboxsize);
  bbox_adjust(unionbox, (void *) box2);
  if (query)
    return bbox_query_size(unionbox, query) - bbox_query_size(box1, query) -
      bbox_query_size(box2, query);
  return bbox_size(unionbox) - bbox_size(box1) - bbox_size(box2);
}

/**
 * @brief Merge consecutive bounding boxes of an array in place, each time the
 * consecutive pair whose union adds the least size to theirs, until a number
 * of them remain
 * @details The boxes are those of the consecutive instants or segments of a
 * value, so two consecutive boxes of a temporal value meet at an instant at
 * most, and the size of their union less theirs is the empty space the merge
 * adds. A heap of the consecutive pairs gives the next merge in logarithmic
 * time; a pair whose box changed since its cost was read is discarded when
 * reached. The boxes keep their order.
 * @param[in] bboxtype Type of the boxes
 * @param[in,out] boxes Array of boxes, the merged boxes at its start
 * @param[in] count Number of boxes
 * @param[in] box_count Number of boxes to keep, at least one
 * @param[in] query Query box, or NULL
 * @param[in] bbox_adjust Function growing its first box to include the second
 * @param[in] bbox_size Function returning the size of a box, read when
 * @p query is NULL
 * @param[in] bbox_query_size Function returning the size of a box for a query
 * box, read when @p query is not NULL
 * @param[out] best When not NULL, the number of boxes after the merge at
 * which the sum of the sizes is least, the boxes before any merge included
 * @return Number of boxes kept
 */
static int
bbox_merge(MeosType bboxtype, void *boxes, int count, int box_count,
  const void *query, void (*bbox_adjust)(void *, void *),
  double (*bbox_size)(const void *),
  double (*bbox_query_size)(const void *, const void *), int *best)
{
  assert(boxes); assert(count >= 0); assert(box_count > 0);
  assert(bbox_adjust); assert(query ? bbox_query_size != NULL : bbox_size != NULL);
  if (best)
    *best = count;
  if (count <= box_count)
    return count;

  size_t bboxsize = bbox_get_size(bboxtype);
  char *box = (char *) boxes;
  int *prev = palloc(sizeof(int) * count);
  int *next = palloc(sizeof(int) * count);
  int *version = palloc0(sizeof(int) * count);
  bool *alive = palloc(sizeof(bool) * count);
  /* Every merge adds at most two pairs to the count - 1 initial ones */
  BoxPair *heap = palloc(sizeof(BoxPair) * (3 * count));
  char *unionbox = palloc(bboxsize);
  int nheap = 0;
  for (int i = 0; i < count; i++)
  {
    prev[i] = i - 1;
    next[i] = (i < count - 1) ? i + 1 : -1;
    alive[i] = true;
  }
  for (int i = 0; i < count - 1; i++)
  {
    heap[nheap] = (BoxPair) {
      bbox_merge_cost(box + i * bboxsize, box + (i + 1) * bboxsize, unionbox,
        bboxsize, query, bbox_adjust, bbox_size, bbox_query_size),
      i, i + 1, 0, 0 };
    boxpair_heap_up(heap, nheap++);
  }

  int nboxes = count;
  /* The sum of the sizes changes by the cost of each merge, so its least
   * value is found without measuring the boxes again */
  double total = 0.0, least = 0.0;
  while (nboxes > box_count && nheap > 0)
  {
    BoxPair pair = heap[0];
    heap[0] = heap[--nheap];
    boxpair_heap_down(heap, nheap, 0);
    if (! alive[pair.left] || ! alive[pair.right] ||
        next[pair.left] != pair.right ||
        version[pair.left] != pair.lversion ||
        version[pair.right] != pair.rversion)
      continue;
    /* Merge the right box into the left one */
    bbox_adjust(box + pair.left * bboxsize, box + pair.right * bboxsize);
    alive[pair.right] = false;
    next[pair.left] = next[pair.right];
    if (next[pair.right] >= 0)
      prev[next[pair.right]] = pair.left;
    version[pair.left]++;
    nboxes--;
    total += pair.cost;
    if (best && total < least)
    {
      least = total;
      *best = nboxes;
    }
    /* The pairs the merged box forms with its neighbours */
    int p = prev[pair.left], n = next[pair.left];
    if (p >= 0)
    {
      heap[nheap] = (BoxPair) {
        bbox_merge_cost(box + p * bboxsize, box + pair.left * bboxsize,
          unionbox, bboxsize, query, bbox_adjust, bbox_size, bbox_query_size),
        p, pair.left, version[p], version[pair.left] };
      boxpair_heap_up(heap, nheap++);
    }
    if (n >= 0)
    {
      heap[nheap] = (BoxPair) {
        bbox_merge_cost(box + pair.left * bboxsize, box + n * bboxsize,
          unionbox, bboxsize, query, bbox_adjust, bbox_size, bbox_query_size),
        pair.left, n, version[pair.left], version[n] };
      boxpair_heap_up(heap, nheap++);
    }
  }

  /* Move the boxes kept to the start of the array, in their order */
  int k = 0;
  for (int i = 0; i >= 0; i = next[i])
  {
    if (i != k)
      memcpy(box + k * bboxsize, box + i * bboxsize, bboxsize);
    k++;
  }
  pfree(prev); pfree(next); pfree(version); pfree(alive); pfree(heap);
  pfree(unionbox);
  return k;
}

/**
 * @ingroup meos_internal_box_index
 * @brief Merge consecutive bounding boxes of an array in place until a
 * number of them remain, each time the consecutive pair whose union adds the
 * least size to theirs
 * @details The merges are those of #bbox_merge, the size of a box read by
 * @p bbox_size.
 * @param[in] bboxtype Type of the boxes, a span type, T_TBOX, T_STBOX or
 * T_TPCBOX
 * @param[in,out] boxes Array of boxes, the merged boxes at its start
 * @param[in] count Number of boxes
 * @param[in] box_count Number of boxes to keep, at least one
 * @param[in] bbox_adjust Function growing its first box to include the second
 * @param[in] bbox_size Function returning the size of a box
 * @return Number of boxes kept, the least of @p count and @p box_count
 * @note The merge twin of #bbox_split, which splits a set of boxes by the
 * same functions
 */
int
bbox_merge_n(MeosType bboxtype, void *boxes, int count, int box_count,
  void (*bbox_adjust)(void *, void *), double (*bbox_size)(const void *))
{
  return bbox_merge(bboxtype, boxes, count, box_count, NULL, bbox_adjust,
    bbox_size, NULL, NULL);
}

/**
 * @ingroup meos_internal_box_index
 * @brief Merge consecutive bounding boxes of an array in place into the
 * boxes a query of the extents of a query box is expected to read least
 * along the merges, each time the consecutive pair whose union lowers that
 * number most
 * @details A query box of widths q placed at random reads a box of widths w
 * with a likelihood proportional to the product over the dimensions of
 * w + q, so the boxes are measured by that product, @p bbox_query_size, and
 * the merges are those of #bbox_merge, the boxes kept being those of the
 * merge after which the sum of the products is least along the merges down
 * to one box, the boxes before any merge included. The answer thus reads no
 * more than the boxes unmerged or the box of all of them. The position of the
 * query box is not read.
 * @param[in] bboxtype Type of the boxes, a span type, T_TBOX, T_STBOX or
 * T_TPCBOX
 * @param[in,out] boxes Array of boxes, the merged boxes at its start
 * @param[in] count Number of boxes
 * @param[in] query Query box, of the type of the boxes
 * @param[in] bbox_adjust Function growing its first box to include the second
 * @param[in] bbox_query_size Function returning the size of a box for the
 * query box
 * @return Number of boxes kept
 */
int
bbox_merge_query(MeosType bboxtype, void *boxes, int count, const void *query,
  void (*bbox_adjust)(void *, void *),
  double (*bbox_query_size)(const void *, const void *))
{
  assert(query); assert(bbox_query_size);
  if (count == 0)
    return 0;
  /* Merge a copy down to one box to find the number of boxes whose sum of
   * sizes is least, then merge the boxes to that number: the merges are the
   * same, the heap ordering the pairs alike in both runs */
  size_t size = bbox_get_size(bboxtype) * count;
  void *copy = palloc(size);
  memcpy(copy, boxes, size);
  int best;
  bbox_merge(bboxtype, copy, count, 1, query, bbox_adjust, NULL,
    bbox_query_size, &best);
  pfree(copy);
  return bbox_merge(bboxtype, boxes, count, best, query, bbox_adjust, NULL,
    bbox_query_size, NULL);
}

/**
 * @ingroup meos_internal_box_index
 * @brief Merge consecutive bounding boxes of an array in place until a box
 * holds on average a number of them
 * @details The number of boxes kept is @p count divided by
 * @p elems_per_box, rounded up, as the functions splitting each N elements
 * count them; the merges are those of #bbox_merge_n.
 * @param[in] bboxtype Type of the boxes
 * @param[in,out] boxes Array of boxes, the merged boxes at its start
 * @param[in] count Number of boxes
 * @param[in] elems_per_box Number of boxes merged into a box on average
 * @param[in] bbox_adjust Function growing its first box to include the second
 * @param[in] bbox_size Function returning the size of a box
 * @return Number of boxes kept
 */
int
bbox_merge_each_n(MeosType bboxtype, void *boxes, int count,
  int elems_per_box, void (*bbox_adjust)(void *, void *),
  double (*bbox_size)(const void *))
{
  assert(elems_per_box > 0);
  if (count == 0)
    return 0;
  return bbox_merge_n(bboxtype, boxes, count,
    (count + elems_per_box - 1) / elems_per_box, bbox_adjust, bbox_size);
}

/*****************************************************************************/
