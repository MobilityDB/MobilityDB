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

PGDLLEXPORT Datum Tnumber_gist_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_gist_consistent);
/**
 * @brief GiST consistent method for temporal numbers
 */
Datum
Tnumber_gist_consistent(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  const TBox *key = DatumGetTboxP(entry->key);
  if (! key)
    PG_RETURN_BOOL(false);

  /* Determine whether the index is lossy depending on the strategy */
  *recheck = tbox_index_recheck(strategy);

  /* Transform the query into a box */
  TBox query;
  if (! tnumber_gist_get_tbox(fcinfo, &query, oid_meostype(typid)))
    PG_RETURN_BOOL(false);

  bool result;
  if (GIST_LEAF(entry))
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
 * @brief GiST union method for temporal numbers
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
    tbox_adjust((void *) result, DatumGetPointer(ent[i].key));
  PG_RETURN_TBOX_P(result);
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
  *result = (float) tbox_penalty(origbox, newbox);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * GiST picksplit method
 *****************************************************************************/

/**
 * @brief GiST picksplit method for bounding boxes
 * @details The split is the double sorting split of Korotkov that MEOS
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
        bbox_adjust(leftBox, boxes[i]);
      else
        memcpy(leftBox, boxes[i], bbox_size);
      v->spl_left[v->spl_nleft++] = off;
    }
    else
    {
      if (v->spl_nright > 0)
        bbox_adjust(rightBox, boxes[i]);
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

PGDLLEXPORT Datum Tbox_gist_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_gist_picksplit);
/**
 * @brief GiST picksplit method for temporal numbers
 */
Datum
Tbox_gist_picksplit(PG_FUNCTION_ARGS)
{
  return bbox_gist_picksplit(fcinfo, T_TBOX, &tbox_adjust, &tbox_penalty);
}

/*****************************************************************************
 * GiST same method
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_gist_same(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_gist_same);
/**
 * @brief GiST same method for temporal numbers
 * @details Return true only when boxes are exactly the same. Fuzzy comparisons
 * cannot be used here without breaking index consistency; therefore, this is
 * not equivalent to box_same().
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
      (! MEOS_FLAGS_GET_X(b1->flags) || span_eq(&b1->span, &b2->span)) &&
      (! MEOS_FLAGS_GET_T(b1->flags) || span_eq(&b1->period, &b2->period));
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
 * @param[in] boxcolumn True when the index is built over a column of temporal
 *   boxes, so that a leaf key is the value itself
 */
static Datum
tbox_gist_distance(FunctionCallInfo fcinfo, bool boxcolumn)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  TBox *key = (TBox *) DatumGetPointer(entry->key);
  MeosType type = oid_meostype(typid);

  /* A leaf key of a column of boxes is the value itself, so its distance to a
   * query box is the one the operator computes. Every other pairing measures a
   * bounding box against a value and bounds the distance from below, which the
   * recheck of the leaf entries settles */
  if (key && GIST_LEAF(entry))
    *recheck = ! (boxcolumn && type == T_TBOX);

  /* Transform the query into a box. The distance is unknown when there is no
   * key or the query is not a box, and the maximum orders those entries after
   * every candidate whose distance is known. A null cannot be returned here,
   * as the GiST framework reads the result of its distance method as a
   * double and rejects a null one */
  TBox query;
  if (! key || ! tnumber_gist_get_tbox(fcinfo, &query, type))
    PG_RETURN_FLOAT8(DBL_MAX);

  /* Since we only have boxes we'll return the minimum possible distance,
   * and let the recheck sort things out in the case of leaves. Since the
   * GiST framework expects a double for the distance method, we need to
   * convert the integer distance for temporal integer boxes into a double */
  /* The kernel states its preconditions as assertions, and this scan reaches
   * it with no MEOS function in between. An entry the query cannot be measured
   * against takes the same maximum as one carrying no key at all */
  if (! ensure_valid_tbox_tbox(key, &query) ||
      ! ensure_has_X(T_TBOX, key->flags) ||
      ! ensure_has_X(T_TBOX, query.flags))
    PG_RETURN_FLOAT8(DBL_MAX);
  double distance = distance_double(nad_tbox_tbox(key, &query),
    key->span.basetype);
  PG_RETURN_FLOAT8(distance);
}

PGDLLEXPORT Datum Tbox_gist_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_gist_distance);
/**
 * @brief GiST distance for temporal boxes
 * @note Take in a query and an entry and return the "distance" between them
 */
Datum
Tbox_gist_distance(PG_FUNCTION_ARGS)
{
  return tbox_gist_distance(fcinfo, true);
}

PGDLLEXPORT Datum Tnumber_gist_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_gist_distance);
/**
 * @brief GiST distance for temporal numbers
 * @note Take in a query and an entry and return the "distance" between them
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
 * @brief Convert a temporal box into its abbreviated key
 */
static Datum
Tbox_abbrev_convert(Datum original, SortSupport ssup)
{
  (void) ssup;
  return UInt64GetDatum(tbox_sort_hash(DatumGetTboxP(original)));
}

/**
 * @brief Compare two temporal boxes for the sorted index build
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
 * @brief GiST sort support method for temporal numbers
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

/*****************************************************************************/
