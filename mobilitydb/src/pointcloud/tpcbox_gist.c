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
 * @brief R-tree GiST index for the TPCBox bounding-box type
 * @details Mirrors mobilitydb/src/geo/tspatial_gist.c — same five
 * support functions (consistent / union / penalty / picksplit / same),
 * specialized to TPCBox.  picksplit reuses the generic
 * bbox_gist_picksplit helper, which is layout-aware of T_TPCBOX
 * (binary-compatible STBox prefix).
 */

/* C */
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include <access/gist.h>
#include <utils/float.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "temporal/bbox_index.h"
#include "temporal/span.h"              /* PG_GETARG_SPAN_P */
#include "temporal/stratnum.h"
#include "temporal/type_util.h"
#include "pointcloud/tpcbox.h"
#include "pointcloud/tpcbox_index.h"
#include "pointcloud/tpc_boxops.h"  /* tpcbox_set_stbox */
#include "geo/stbox.h"              /* PG_RETURN_STBOX_P */
#include "geo/stbox_index.h"        /* stbox_adjust, stbox_penalty */
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_temporal/meos_catalog.h"  /* oid_meostype */
#include "pg_temporal/tnumber_gist.h"
#include "pg_temporal/index_sortsupport.h"

/*****************************************************************************
 * GiST consistent
 *****************************************************************************/

/**
 * @brief Transform a query argument into a box, initializing the dimensions
 * that must not be taken into account by the operators
 * @note Mirrors @p tspatial_gist_get_stbox. The opclass declares members
 *   against @p tstzspan and against the temporal type itself as well as
 *   against @p tpcbox, so the query datum carries whichever of those types
 *   the operator names.
 */
static bool
tpc_gist_get_tpcbox(FunctionCallInfo fcinfo, TPCBox *result, MeosType type)
{
  if (type == T_TSTZSPAN)
  {
    Span *s = PG_GETARG_SPAN_P(1);
    tstzspan_set_tpcbox(s, result);
  }
  else if (type == T_TPCBOX)
  {
    TPCBox *box = PG_GETARG_TPCBOX_P(1);
    if (! box)
      return false;
    memcpy(result, box, sizeof(TPCBox));
  }
  else if (tpointcloud_temptype(type))
  {
    if (PG_ARGISNULL(1))
      return false;
    Datum tempdatum = PG_GETARG_DATUM(1);
    Temporal *temp = temporal_slice(tempdatum);
    temporal_set_bbox(temp, result);
  }
  else
    elog(ERROR, "Unsupported type for indexing: %d", type);
  return true;
}

/*****************************************************************************
 * GiST compress — for opclasses keyed on tpcpoint / tpcpatch
 *
 * The leaf key is a Temporal* (tpcpoint or tpcpatch); the GiST
 * storage type is TPCBox. Compute the bbox via the generic
 * temporal_set_bbox dispatcher (works for both temporal types) and
 * emit it as the GiST entry key. Mirrors Tspatial_gist_compress.
 *****************************************************************************/

PGDLLEXPORT Datum Tpc_gist_compress(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpc_gist_compress);
/**
 * @ingroup mobilitydb_pointcloud_index
 * @brief GiST compress method for tpcpoint / tpcpatch — derives the
 *   TPCBox bbox of each leaf entry as its index key
 * @sqlfn tpc_gist_compress()
 */
Datum
Tpc_gist_compress(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  if (entry->leafkey)
  {
    GISTENTRY *retval = palloc(sizeof(GISTENTRY));
    TPCBox *box = palloc(sizeof(TPCBox));
    Temporal *temp = temporal_slice(entry->key);
    temporal_set_bbox(temp, box);
    gistentryinit(*retval, PointerGetDatum(box), entry->rel, entry->page,
      entry->offset, false);
    PG_RETURN_POINTER(retval);
  }
  PG_RETURN_POINTER(entry);
}

/*****************************************************************************
 * SP-GiST compress — for opclasses keyed on tpcpoint / tpcpatch with
 * STBox storage (lossy: pcid is dropped, recovered by recheck on the
 * actual operator). Mirrors Tspatial_spgist_compress.
 *****************************************************************************/

PGDLLEXPORT Datum Tpc_spgist_compress(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpc_spgist_compress);
/**
 * @ingroup mobilitydb_pointcloud_index
 * @brief SP-GiST compress method for tpcpoint / tpcpatch — derives a
 *   STBox by computing the leaf entry's TPCBox bbox and dropping pcid
 * @sqlfn tpc_spgist_compress()
 */
Datum
Tpc_spgist_compress(PG_FUNCTION_ARGS)
{
  Datum tempdatum = PG_GETARG_DATUM(0);
  Temporal *temp = temporal_slice(tempdatum);
  TPCBox tpcbox;
  temporal_set_bbox(temp, &tpcbox);
  STBox *result = palloc(sizeof(STBox));
  tpcbox_set_stbox(&tpcbox, result);
  PG_RETURN_STBOX_P(result);
}

PGDLLEXPORT Datum Tpcbox_spgist_compress(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_spgist_compress);
/**
 * @ingroup mobilitydb_pointcloud_index
 * @brief SP-GiST compress method for tpcbox — drops pcid, returns
 *   an STBox with the same bounds
 * @sqlfn tpcbox_spgist_compress()
 */
Datum
Tpcbox_spgist_compress(PG_FUNCTION_ARGS)
{
  TPCBox *box = PG_GETARG_TPCBOX_P(0);
  STBox *result = palloc(sizeof(STBox));
  tpcbox_set_stbox(box, result);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************
 * GiST sort support method
 *****************************************************************************/


/* GENERATED-BOXGIST-BEGIN tpcbox — tools/codegen/inherited/generate.py from templates/boxgist.c.tmpl; DO NOT EDIT BY HAND;
 * edit the template + manifest.d/boxgist.yaml and re-run. */
/*****************************************************************************
 * GiST consistent method
 *****************************************************************************/

PGDLLEXPORT Datum Tpcbox_gist_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_gist_consistent);
/**
 * @brief GiST consistent method for point cloud boxes
 */
Datum
Tpcbox_gist_consistent(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  TPCBox *key = DatumGetTpcboxP(entry->key), query;
  if (! key)
    PG_RETURN_BOOL(false);

  /* Determine whether the index is lossy depending on the strategy */
  *recheck = tpcbox_index_recheck(strategy);

  /* Transform the query into a box */
  if (! tpc_gist_get_tpcbox(fcinfo, &query, oid_meostype(typid)))
    PG_RETURN_BOOL(false);

  bool result;
  if (MEOS_FLAGS_GET_PARTS(key->flags))
    /* A key over boxes of several structures bounds none of them */
    result = true;
  else if (! index_strategy_is_valid(T_TPCBOX, strategy, key, &query))
  {
    /* The operator raises its error on the boxes under the key, which the
     * recheck applies it to */
    *recheck = true;
    result = true;
  }
  else if (GIST_LEAF(entry))
    result = tpcbox_index_leaf_consistent(key, &query, strategy);
  else
    result = tpcbox_gist_inner_consistent(key, &query, strategy);

  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * GiST union method
 *****************************************************************************/

PGDLLEXPORT Datum Tpcbox_gist_union(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_gist_union);
/**
 * @brief GiST union method for point cloud boxes
 * @details Return the minimal bounding box that encloses all the entries in
 * entryvec
 */
Datum
Tpcbox_gist_union(PG_FUNCTION_ARGS)
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GISTENTRY *ent = entryvec->vector;
  TPCBox *result = tpcbox_copy(DatumGetTpcboxP(ent[0].key));
  for (int i = 1; i < entryvec->n; i++)
    bbox_gist_adjust(T_TPCBOX, result, DatumGetTpcboxP(ent[i].key),
      &stbox_adjust);
  PG_RETURN_TPCBOX_P(result);
}

/*****************************************************************************
 * GiST penalty method
 *****************************************************************************/

PGDLLEXPORT Datum Tpcbox_gist_penalty(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_gist_penalty);
/**
 * @brief GiST penalty method for point cloud boxes
 * @note As in the R-tree paper, we use change in area as our penalty metric
 */
Datum
Tpcbox_gist_penalty(PG_FUNCTION_ARGS)
{
  GISTENTRY *origentry = (GISTENTRY *) PG_GETARG_POINTER(0);
  GISTENTRY *newentry = (GISTENTRY *) PG_GETARG_POINTER(1);
  float *result = (float *) PG_GETARG_POINTER(2);
  void *origbox = DatumGetPointer(origentry->key);
  void *newbox = DatumGetPointer(newentry->key);
  *result = (float) bbox_gist_penalty(T_TPCBOX, origbox, newbox,
    &stbox_penalty);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * GiST picksplit method
 *****************************************************************************/

PGDLLEXPORT Datum Tpcbox_gist_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_gist_picksplit);
/**
 * @brief GiST picksplit method for point cloud boxes
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
Tpcbox_gist_picksplit(PG_FUNCTION_ARGS)
{
  return bbox_gist_picksplit(fcinfo, T_TPCBOX, &stbox_adjust,
    &stbox_penalty);
}

/*****************************************************************************
 * GiST same method
 *****************************************************************************/

PGDLLEXPORT Datum Tpcbox_gist_same(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_gist_same);
/**
 * @brief GiST same method for point cloud boxes
 * @details Return true only when the boxes are exactly the same. Fuzzy
 * comparisons cannot be used here without breaking index consistency, and an
 * index method answers where the operator @p ~= refuses its arguments, so
 * this is not that operator
 */
Datum
Tpcbox_gist_same(PG_FUNCTION_ARGS)
{
  TPCBox *b1 = PG_GETARG_TPCBOX_P(0);
  TPCBox *b2 = PG_GETARG_TPCBOX_P(1);
  bool *result = (bool *) PG_GETARG_POINTER(2);
  if (b1 && b2)
    /* A key over boxes of several structures bounds none of them */
    *result = b1->flags == b2->flags && (MEOS_FLAGS_GET_PARTS(b1->flags) ||
     (b1->pcid == b2->pcid && b1->srid == b2->srid &&
      FLOAT8_EQ(b1->xmin, b2->xmin) && FLOAT8_EQ(b1->ymin, b2->ymin) &&
      FLOAT8_EQ(b1->zmin, b2->zmin) && FLOAT8_EQ(b1->xmax, b2->xmax) &&
      FLOAT8_EQ(b1->ymax, b2->ymax) && FLOAT8_EQ(b1->zmax, b2->zmax) &&
      /* The period is compared as a span, so that two keys ending on the same
       * instant with a different inclusivity are NOT reported as the same */
      MEOS_FLAGS_GET_T(b1->flags) == MEOS_FLAGS_GET_T(b2->flags) &&
      (! MEOS_FLAGS_GET_T(b1->flags) ||
        span_eq(&b1->period, &b2->period))));
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
 *   point cloud boxes, so that a leaf key is the value itself
 */
static Datum
tpcbox_gist_distance(FunctionCallInfo fcinfo, bool boxcolumn)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  TPCBox *key = (TPCBox *) DatumGetPointer(entry->key);
  if (! key)
    PG_RETURN_FLOAT8(DBL_MAX);
  MeosType type = oid_meostype(typid);

  /* A leaf key of a column of boxes is the value itself, so its distance to a
   * query box is the one the operator computes. Every other pairing measures
   * a bounding box against a value and bounds the distance from below, which
   * the recheck of the leaf entries settles */
  if (GIST_LEAF(entry))
    *recheck = ! (boxcolumn && type == T_TPCBOX);

  /* Transform the query into a box. The distance is unknown when the query is
   * not a box, and the maximum orders those entries after every candidate
   * whose distance is known */
  TPCBox query;
  if (! tpc_gist_get_tpcbox(fcinfo, &query, type))
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
  if (! index_distance_is_valid(T_TPCBOX, key, &query))
  {
    if (GIST_LEAF(entry) && ! *recheck)
      PG_RETURN_FLOAT8(index_distance_apply(T_TPCBOX, key, &query));
    PG_RETURN_FLOAT8(-get_float8_infinity());
  }

  double distance = nad_tpcbox_tpcbox(key, &query);

  /* An inner entry, and a leaf entry the executor rechecks, carry a bounding
   * box rather than the value, so what they report is lowered to stay under
   * the distance the ordering operator computes. The exact leaf of a column of
   * boxes reports the operator's own answer and keeps it */
  if (! GIST_LEAF(entry) || *recheck)
    distance = stbox_index_distance_bound(distance, (const STBox *) key,
      (const STBox *) &query);

  PG_RETURN_FLOAT8(distance);
}

PGDLLEXPORT Datum Tpcbox_gist_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_gist_distance);
/**
 * @brief GiST distance method of a column of point cloud boxes
 */
Datum
Tpcbox_gist_distance(PG_FUNCTION_ARGS)
{
  return tpcbox_gist_distance(fcinfo, true);
}

PGDLLEXPORT Datum Tpointcloud_gist_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpointcloud_gist_distance);
/**
 * @brief GiST distance method of a column of temporal point clouds
 */
Datum
Tpointcloud_gist_distance(PG_FUNCTION_ARGS)
{
  return tpcbox_gist_distance(fcinfo, false);
}

/*****************************************************************************
 * GiST sort support method
 *****************************************************************************/

/**
 * @brief Convert a box of the point cloud boxes into its abbreviated key
 */
static Datum
Tpcbox_abbrev_convert(Datum original, SortSupport ssup)
{
  (void) ssup;
  return UInt64GetDatum(stbox_sort_hash((const STBox *) DatumGetTpcboxP(original)));
}

/**
 * @brief Compare two boxes of the point cloud boxes for the sorted index build
 */
static int
Tpcbox_cmp_full(Datum x, Datum y, SortSupport ssup)
{
  const TPCBox *box1 = DatumGetTpcboxP(x);
  const TPCBox *box2 = DatumGetTpcboxP(y);
  uint64 hash1 = stbox_sort_hash((const STBox *) box1);
  uint64 hash2 = stbox_sort_hash((const STBox *) box2);
  (void) ssup;
  if (hash1 > hash2)
    return 1;
  if (hash1 < hash2)
    return -1;
  /* Boxes on the same point of the curve are ordered by their own comparison,
   * so that the sort is deterministic */
  return tpcbox_cmp(box1, box2);
}

PGDLLEXPORT Datum Tpcbox_gist_sortsupport(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_gist_sortsupport);
/**
 * @brief GiST sort support method for point cloud boxes
 */
Datum
Tpcbox_gist_sortsupport(PG_FUNCTION_ARGS)
{
  SortSupport ssup = (SortSupport) PG_GETARG_POINTER(0);
  ssup->comparator = Tpcbox_cmp_full;
  ssup->ssup_extra = NULL;
  /* An abbreviated key is a whole Datum, so it is only available where a
   * Datum is 64 bits wide */
  if (ssup->abbreviate && sizeof(Datum) == 8)
  {
    ssup->comparator = sortsupport_abbrev_cmp;
    ssup->abbrev_converter = Tpcbox_abbrev_convert;
    ssup->abbrev_abort = sortsupport_abbrev_abort;
    ssup->abbrev_full_comparator = Tpcbox_cmp_full;
  }
  PG_RETURN_VOID();
}
/* GENERATED-BOXGIST-END tpcbox */

/*****************************************************************************/
