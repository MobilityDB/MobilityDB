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
 * @brief R-tree GiST index for temporal integers and temporal floats
 * @details These functions are based on those in the file `gistproc.c`.
 */

#include "pg_temporal/tnumber_gist.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#include <access/gist.h>
#include <utils/float.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <pgtypes.h>
#include "temporal/span.h"
#include "temporal/bbox_index.h"
#include "temporal/span_index.h"
#include "temporal/stratnum.h"
#include "temporal/tbox.h"
#include "temporal/tbox_index.h"
#include "temporal/temporal_boxops.h"
#include "temporal/type_util.h"
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"
#include "pg_temporal/temporal.h"
#include "pg_temporal/index_sortsupport.h"
#if POINTCLOUD
  #include <meos_pointcloud.h>
#endif

/*****************************************************************************
 * GiST consistent methods
 *****************************************************************************/

/**
 * @brief Transform the query argument into a box initializing the dimensions
 * that must not be taken into account by the operators to infinity
 */
static bool
tnumber_gist_get_tbox(FunctionCallInfo fcinfo, TBox *result, MeosType type)
{
  Span *s;
  if (tnumber_spantype(type))
  {
    s = PG_GETARG_SPAN_P(1);
    if (! s)
      return false;
    numspan_set_tbox(s, result);
  }
  else if (type == T_TSTZSPAN)
  {
    s = PG_GETARG_SPAN_P(1);
    tstzspan_set_tbox(s, result);
  }
  else if (type == T_TBOX)
  {
    TBox *box = PG_GETARG_TBOX_P(1);
    if (! box)
      return false;
    memcpy(result, box, sizeof(TBox));
  }
  else if (tnumber_type(type))
  {
    if (PG_ARGISNULL(1))
      return false;
    Datum tempdatum = PG_GETARG_DATUM(1);
    Temporal *temp = temporal_slice(tempdatum);
    tnumber_set_tbox(temp, result);
  }
  else
    elog(ERROR, "Unsupported type for indexing: %d", type);
  return true;
}

/*****************************************************************************
 * GiST compress method
 *****************************************************************************/

PGDLLEXPORT Datum Tnumber_gist_compress(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_gist_compress);
/**
 * @brief GiST compress method for temporal numbers
 */
Datum
Tnumber_gist_compress(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  if (entry->leafkey)
  {
    GISTENTRY *retval = palloc(sizeof(GISTENTRY));
    TBox *box = palloc(sizeof(TBox));
    Temporal *temp = temporal_slice(entry->key);
    tnumber_set_tbox(temp, box);
    gistentryinit(*retval, PointerGetDatum(box), entry->rel, entry->page,
      entry->offset, false);
    PG_RETURN_POINTER(retval);
  }
  PG_RETURN_POINTER(entry);
}

/*****************************************************************************
 * GiST picksplit method
 *****************************************************************************/

/**
 * @brief Return a pointer to the flags of a bounding box
 * @param[in] bboxtype Type of the box: T_TBOX, T_STBOX or T_TPCBOX
 * @param[in] box Bounding box
 */
static int16 *
bbox_gist_flags(MeosType bboxtype, const void *box)
{
  /* A point cloud box begins with a whole spatiotemporal box */
  return (bboxtype == T_TBOX) ? &((TBox *) box)->flags :
    &((STBox *) box)->flags;
}

/**
 * @brief Grow a key of a GiST index over bounding boxes to include a box
 * @details Boxes of one structure, see #bbox_same_structure, grow the key as
 * the function given does. A key over boxes of several structures bounds none
 * of them: it carries the flag @p MEOS_FLAG_PARTS, and the consistent and
 * distance methods descend it for every query
 * @param[in] bboxtype Type of the boxes: T_TBOX, T_STBOX or T_TPCBOX
 * @param[in,out] key Key grown
 * @param[in] box Box or key added
 * @param[in] bbox_adjust Function growing its first box to include the second
 */
void
bbox_gist_adjust(MeosType bboxtype, void *key, const void *box,
  void (*bbox_adjust)(void *, void *))
{
  int16 *flags = bbox_gist_flags(bboxtype, key);
  if (MEOS_FLAGS_GET_PARTS(*flags))
    return;
  if (MEOS_FLAGS_GET_PARTS(*bbox_gist_flags(bboxtype, box)) ||
      ! bbox_same_structure(bboxtype, key, box))
    MEOS_FLAGS_SET_PARTS(*flags, true);
  else
    bbox_adjust(key, (void *) box);
  return;
}

/**
 * @brief Return the penalty of adding a box to a key of a GiST index over
 * bounding boxes
 * @details A box goes under a key of its structure when there is one, else
 * under a key over several structures, so that the boxes of each structure
 * gather under keys of their own
 * @param[in] bboxtype Type of the boxes: T_TBOX, T_STBOX or T_TPCBOX
 * @param[in] key Key
 * @param[in] box Box or key added
 * @param[in] bbox_penalty Function returning the growth of its first box when
 * the second is added
 */
double
bbox_gist_penalty(MeosType bboxtype, void *key, void *box,
  double (*bbox_penalty)(void *, void *))
{
  if (MEOS_FLAGS_GET_PARTS(*bbox_gist_flags(bboxtype, key)))
    return FLT_MAX / 2;
  if (MEOS_FLAGS_GET_PARTS(*bbox_gist_flags(bboxtype, box)) ||
      ! bbox_same_structure(bboxtype, key, box))
    return FLT_MAX;
  return bbox_penalty(key, box);
}

/**
 * @brief Split by structure the entries of a GiST index over bounding boxes
 * @details The entries of the structure of the first box go to the left and
 * the others to the right; keys over several structures go to the right, and
 * when every entry is one, the entries are halved
 * @param[in] bboxtype Type of the boxes: T_TBOX, T_STBOX or T_TPCBOX
 * @param[in] boxes Boxes or keys of the entries
 * @param[in] count Number of entries
 * @param[out] left Whether each entry goes to the left
 * @return False when the entries are boxes of one structure, which the
 * function leaves unsplit
 */
static bool
bbox_gist_split_structures(MeosType bboxtype, void **boxes, int count,
  bool *left)
{
  int first = -1;
  bool several = false;
  for (int i = 0; i < count && ! several; i++)
  {
    if (MEOS_FLAGS_GET_PARTS(*bbox_gist_flags(bboxtype, boxes[i])))
      several = true;
    else if (first < 0)
      first = i;
    else if (! bbox_same_structure(bboxtype, boxes[first], boxes[i]))
      several = true;
  }
  if (! several)
    return false;
  for (int i = 0; i < count; i++)
    left[i] = (first < 0) ? (i < count / 2) :
      (! MEOS_FLAGS_GET_PARTS(*bbox_gist_flags(bboxtype, boxes[i])) &&
       bbox_same_structure(bboxtype, boxes[first], boxes[i]));
  return true;
}

/**
 * @brief GiST picksplit method for bounding boxes
 * @details Entries of several structures are split by structure
 * (#bbox_gist_split_structures). The split of the entries of one structure is
 * the double sorting split of Korotkov that MEOS
 * computes (#bbox_split) on the boxes of the entries, shared with the
 * in-memory RTree; this function only reads the entries and writes the two
 * groups and their bounding boxes in the form GiST expects.
 * @param[in] fcinfo Arguments of the GiST picksplit method
 * @param[in] bboxtype Type of the boxes: T_TBOX, T_STBOX or T_TPCBOX
 * @param[in] bbox_adjust Function growing its first box to include the second
 * @param[in] bbox_penalty Function returning the growth of its first box when
 * the second is added
 */
Datum
bbox_gist_picksplit(FunctionCallInfo fcinfo, MeosType bboxtype,
  void (*bbox_adjust)(void *, void *), double (*bbox_penalty)(void *, void *))
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);
  OffsetNumber maxoff = (OffsetNumber) (entryvec->n - 1);
  int nentries = maxoff - FirstOffsetNumber + 1;

  void **boxes = palloc(sizeof(void *) * nentries);
  bool *left = palloc(sizeof(bool) * nentries);
  for (int i = 0; i < nentries; i++)
    boxes[i] = DatumGetPointer(entryvec->vector[i + FirstOffsetNumber].key);
  if (! bbox_gist_split_structures(bboxtype, boxes, nentries, left))
    bbox_split(bboxtype, boxes, nentries, bbox_adjust, bbox_penalty, left);

  size_t bbox_size = bbox_get_size(bboxtype);
  void *leftBox = palloc0(bbox_size);
  void *rightBox = palloc0(bbox_size);
  v->spl_left = palloc(sizeof(OffsetNumber) * nentries);
  v->spl_right = palloc(sizeof(OffsetNumber) * nentries);
  v->spl_nleft = v->spl_nright = 0;
  for (int i = 0; i < nentries; i++)
  {
    OffsetNumber off = (OffsetNumber) (i + FirstOffsetNumber);
    if (left[i])
    {
      if (v->spl_nleft > 0)
        bbox_gist_adjust(bboxtype, leftBox, boxes[i], bbox_adjust);
      else
        memcpy(leftBox, boxes[i], bbox_size);
      v->spl_left[v->spl_nleft++] = off;
    }
    else
    {
      if (v->spl_nright > 0)
        bbox_gist_adjust(bboxtype, rightBox, boxes[i], bbox_adjust);
      else
        memcpy(rightBox, boxes[i], bbox_size);
      v->spl_right[v->spl_nright++] = off;
    }
  }
  v->spl_ldatum = PointerGetDatum(leftBox);
  v->spl_rdatum = PointerGetDatum(rightBox);
  pfree(boxes); pfree(left);
  PG_RETURN_POINTER(v);
}

/*****************************************************************************
 * GiST sort support method
 *****************************************************************************/


/* GENERATED-BOXGIST-BEGIN tbox — tools/codegen/inherited/generate.py from templates/boxgist.c.tmpl; DO NOT EDIT BY HAND;
 * edit the template + manifest.d/boxgist.yaml and re-run. */
/*****************************************************************************
 * GiST consistent method
 *****************************************************************************/

PGDLLEXPORT Datum Tnumber_gist_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_gist_consistent);
/**
 * @brief GiST consistent method for temporal boxes
 */
Datum
Tnumber_gist_consistent(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  TBox *key = DatumGetTboxP(entry->key), query;
  if (! key)
    PG_RETURN_BOOL(false);

  /* Determine whether the index is lossy depending on the strategy */
  *recheck = tbox_index_recheck(strategy);

  /* Transform the query into a box */
  if (! tnumber_gist_get_tbox(fcinfo, &query, oid_meostype(typid)))
    PG_RETURN_BOOL(false);

  bool result;
  if (MEOS_FLAGS_GET_PARTS(key->flags))
    /* A key over boxes of several structures bounds none of them */
    result = true;
  else if (! index_strategy_is_valid(T_TBOX, strategy, key, &query))
  {
    /* The operator raises its error on the boxes under the key, which the
     * recheck applies it to */
    *recheck = true;
    result = true;
  }
  else if (GIST_LEAF(entry))
    result = tbox_index_leaf_consistent(key, &query, strategy);
  else
    result = tbox_gist_inner_consistent(key, &query, strategy);

  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * GiST union method
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_gist_union(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_gist_union);
/**
 * @brief GiST union method for temporal boxes
 * @details Return the minimal bounding box that encloses all the entries in
 * entryvec
 */
Datum
Tbox_gist_union(PG_FUNCTION_ARGS)
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GISTENTRY *ent = entryvec->vector;
  TBox *result = tbox_copy(DatumGetTboxP(ent[0].key));
  for (int i = 1; i < entryvec->n; i++)
    bbox_gist_adjust(T_TBOX, result, DatumGetTboxP(ent[i].key),
      &tbox_adjust);
  PG_RETURN_TBOX_P(result);
}

/*****************************************************************************
 * GiST penalty method
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_gist_penalty(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_gist_penalty);
/**
 * @brief GiST penalty method for temporal boxes
 * @note As in the R-tree paper, we use change in area as our penalty metric
 */
Datum
Tbox_gist_penalty(PG_FUNCTION_ARGS)
{
  GISTENTRY *origentry = (GISTENTRY *) PG_GETARG_POINTER(0);
  GISTENTRY *newentry = (GISTENTRY *) PG_GETARG_POINTER(1);
  float *result = (float *) PG_GETARG_POINTER(2);
  void *origbox = DatumGetPointer(origentry->key);
  void *newbox = DatumGetPointer(newentry->key);
  *result = (float) bbox_gist_penalty(T_TBOX, origbox, newbox,
    &tbox_penalty);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * GiST picksplit method
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_gist_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_gist_picksplit);
/**
 * @brief GiST picksplit method for temporal boxes
 * @details
 * The algorithm finds split of boxes by considering splits along each axis.
 * Each entry is first projected as an interval on the first axis, and
 * different ways to split the intervals into two groups are considered,
 * trying to minimize the overlap of the groups. Then the same is repeated for
 * the other axes, and the overall best split is chosen. The quality of a split
 * is determined by overlap along that axis and some other criteria (see
 * bbox_gist_consider_split).
 *
 * After that, all the entries are divided into three groups:
 *
 * 1. Entries which should be placed to the left group
 * 2. Entries which should be placed to the right group
 * 3. "Common entries" which can be placed to any of groups without affecting
 *    of overlap along selected axis.
 *
 * The common entries are distributed by minimizing penalty.
 *
 * For details see:
 * "A new double sorting-based node splitting algorithm for R-tree", A. Korotkov
 * http://syrcose.ispras.ru/2011/files/SYRCoSE2011_Proceedings.pdf#page=36
 */
Datum
Tbox_gist_picksplit(PG_FUNCTION_ARGS)
{
  return bbox_gist_picksplit(fcinfo, T_TBOX, &tbox_adjust,
    &tbox_penalty);
}

/*****************************************************************************
 * GiST same method
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_gist_same(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_gist_same);
/**
 * @brief GiST same method for temporal boxes
 * @details Return true only when the boxes are exactly the same. Fuzzy
 * comparisons cannot be used here without breaking index consistency, and an
 * index method answers where the operator @p ~= refuses its arguments, so
 * this is not that operator
 */
Datum
Tbox_gist_same(PG_FUNCTION_ARGS)
{
  TBox *b1 = PG_GETARG_TBOX_P(0);
  TBox *b2 = PG_GETARG_TBOX_P(1);
  bool *result = (bool *) PG_GETARG_POINTER(2);
  if (b1 && b2)
    /* Each span is compared as a span, so that two keys ending on the same
     * bound with a different inclusivity are NOT reported as the same */
    *result = MEOS_FLAGS_GET_X(b1->flags) == MEOS_FLAGS_GET_X(b2->flags) &&
      MEOS_FLAGS_GET_T(b1->flags) == MEOS_FLAGS_GET_T(b2->flags) &&
      MEOS_FLAGS_GET_PARTS(b1->flags) == MEOS_FLAGS_GET_PARTS(b2->flags) &&
      (MEOS_FLAGS_GET_PARTS(b1->flags) || ((
      (! MEOS_FLAGS_GET_X(b1->flags) || span_eq(&b1->span, &b2->span)) &&
      (! MEOS_FLAGS_GET_T(b1->flags) || span_eq(&b1->period, &b2->period)))));
  else
    *result = (! b1 && ! b2);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * GiST distance method
 *****************************************************************************/

/**
 * @brief Return the distance between the bounding box of an index entry and a
 * query, in the last argument whether the leaf entries must be rechecked
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] boxcolumn True when the index is built over a column of
 *   temporal boxes, so that a leaf key is the value itself
 */
static Datum
tbox_gist_distance(FunctionCallInfo fcinfo, bool boxcolumn)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  TBox *key = (TBox *) DatumGetPointer(entry->key);
  if (! key)
    PG_RETURN_FLOAT8(DBL_MAX);
  MeosType type = oid_meostype(typid);

  /* A leaf key of a column of boxes is the value itself, so its distance to a
   * query box is the one the operator computes. Every other pairing measures
   * a bounding box against a value and bounds the distance from below, which
   * the recheck of the leaf entries settles */
  if (GIST_LEAF(entry))
    *recheck = ! (boxcolumn && type == T_TBOX);

  /* Transform the query into a box. The distance is unknown when the query is
   * not a box, and the maximum orders those entries after every candidate
   * whose distance is known */
  TBox query;
  if (! tnumber_gist_get_tbox(fcinfo, &query, type))
    PG_RETURN_FLOAT8(DBL_MAX);
  /* A key over boxes of several structures bounds none of them, and is
   * visited before every other one, since it may hold boxes on which the
   * distance raises its error */
  if (MEOS_FLAGS_GET_PARTS(key->flags))
    PG_RETURN_FLOAT8(-get_float8_infinity());
  /* Where the distance raises its error on the boxes under the key, the
   * entry is visited before every other one. A leaf the executor rechecks
   * leaves the error to the operator applied to its row; the exact leaf of a
   * column of boxes is the value itself, which an index-only scan returns
   * without a recheck, so the distance applied to it raises the error */
  if (! index_distance_is_valid(T_TBOX, key, &query))
  {
    if (GIST_LEAF(entry) && ! *recheck)
      PG_RETURN_FLOAT8(index_distance_apply(T_TBOX, key, &query));
    PG_RETURN_FLOAT8(-get_float8_infinity());
  }

  double distance = distance_double(nad_tbox_tbox(key, &query),
    key->span.basetype);

  PG_RETURN_FLOAT8(distance);
}

PGDLLEXPORT Datum Tbox_gist_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_gist_distance);
/**
 * @brief GiST distance method of a column of temporal boxes
 */
Datum
Tbox_gist_distance(PG_FUNCTION_ARGS)
{
  return tbox_gist_distance(fcinfo, true);
}

PGDLLEXPORT Datum Tnumber_gist_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_gist_distance);
/**
 * @brief GiST distance method of a column of temporal numbers
 */
Datum
Tnumber_gist_distance(PG_FUNCTION_ARGS)
{
  return tbox_gist_distance(fcinfo, false);
}

/*****************************************************************************
 * GiST sort support method
 *****************************************************************************/

/**
 * @brief Convert a box of the temporal boxes into its abbreviated key
 */
static Datum
Tbox_abbrev_convert(Datum original, SortSupport ssup)
{
  (void) ssup;
  return UInt64GetDatum(tbox_sort_hash(DatumGetTboxP(original)));
}

/**
 * @brief Compare two boxes of the temporal boxes for the sorted index build
 */
static int
Tbox_cmp_full(Datum x, Datum y, SortSupport ssup)
{
  const TBox *box1 = DatumGetTboxP(x);
  const TBox *box2 = DatumGetTboxP(y);
  uint64 hash1 = tbox_sort_hash(box1);
  uint64 hash2 = tbox_sort_hash(box2);
  (void) ssup;
  if (hash1 > hash2)
    return 1;
  if (hash1 < hash2)
    return -1;
  /* Boxes on the same point of the curve are ordered by their own comparison,
   * so that the sort is deterministic */
  return tbox_cmp(box1, box2);
}

PGDLLEXPORT Datum Tbox_gist_sortsupport(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_gist_sortsupport);
/**
 * @brief GiST sort support method for temporal boxes
 */
Datum
Tbox_gist_sortsupport(PG_FUNCTION_ARGS)
{
  SortSupport ssup = (SortSupport) PG_GETARG_POINTER(0);
  ssup->comparator = Tbox_cmp_full;
  ssup->ssup_extra = NULL;
  /* An abbreviated key is a whole Datum, so it is only available where a
   * Datum is 64 bits wide */
  if (ssup->abbreviate && sizeof(Datum) == 8)
  {
    ssup->comparator = sortsupport_abbrev_cmp;
    ssup->abbrev_converter = Tbox_abbrev_convert;
    ssup->abbrev_abort = sortsupport_abbrev_abort;
    ssup->abbrev_full_comparator = Tbox_cmp_full;
  }
  PG_RETURN_VOID();
}
/* GENERATED-BOXGIST-END tbox */

/*****************************************************************************/
