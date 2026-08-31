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
 * @brief R-tree GiST index for the TPCBox bounding-box type.
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
#include "temporal/span.h"              /* PG_GETARG_SPAN_P */
#include "temporal/stratnum.h"
#include "temporal/type_util.h"
#include "pointcloud/tpcbox.h"
#include "pointcloud/tpcbox_index.h"
#include "pointcloud/tpc_boxops.h"  /* tpcbox_set_stbox */
#include "geo/stbox.h"              /* PG_RETURN_STBOX_P */
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

PGDLLEXPORT Datum Tpcbox_gist_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_gist_consistent);
/**
 * @ingroup mobilitydb_pointcloud_index
 * @brief GiST consistent method for TPCBox
 * @sqlfn tpcbox_gist_consistent()
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
  if (GIST_LEAF(entry))
    result = tpcbox_index_leaf_consistent(key, &query, strategy);
  else
    result = tpcbox_gist_inner_consistent(key, &query, strategy);

  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * GiST union (and shared adjust helper)
 *****************************************************************************/

/**
 * @brief Increase @p box1 to include @p box2.
 * @details Same as stbox_adjust on the binary-compatible prefix.  The
 * pcid field is left untouched: the GiST opclass is per-type, not
 * per-schema, so all entries in one index already share a pcid.
 */
void
tpcbox_adjust(void *bbox1, void *bbox2)
{
  TPCBox *box1 = (TPCBox *) bbox1;
  TPCBox *box2 = (TPCBox *) bbox2;
  box1->xmin = FLOAT8_MIN(box1->xmin, box2->xmin);
  box1->xmax = FLOAT8_MAX(box1->xmax, box2->xmax);
  box1->ymin = FLOAT8_MIN(box1->ymin, box2->ymin);
  box1->ymax = FLOAT8_MAX(box1->ymax, box2->ymax);
  box1->zmin = FLOAT8_MIN(box1->zmin, box2->zmin);
  box1->zmax = FLOAT8_MAX(box1->zmax, box2->zmax);
  if (MEOS_FLAGS_GET_T(box1->flags))
    span_expand(&box2->period, &box1->period);
}

PGDLLEXPORT Datum Tpcbox_gist_union(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_gist_union);
/**
 * @ingroup mobilitydb_pointcloud_index
 * @brief GiST union method — minimum bounding TPCBox over an entry vector
 * @sqlfn tpcbox_gist_union()
 */
Datum
Tpcbox_gist_union(PG_FUNCTION_ARGS)
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GISTENTRY *ent = entryvec->vector;
  TPCBox *result = tpcbox_copy(DatumGetTpcboxP(ent[0].key));
  for (int i = 1; i < entryvec->n; i++)
    tpcbox_adjust(result, DatumGetTpcboxP(ent[i].key));
  PG_RETURN_TPCBOX_P(result);
}

/*****************************************************************************
 * GiST penalty
 *****************************************************************************/

/**
 * @brief Volume of a TPCBox for penalty calculation.
 */
static double
tpcbox_size(const TPCBox *box)
{
  double result_size = 1;
  bool hasx = MEOS_FLAGS_GET_X(box->flags),
       hasz = MEOS_FLAGS_GET_Z(box->flags),
       hast = MEOS_FLAGS_GET_T(box->flags);

  /* Zero-width / inverted cases — same handling as stbox_size */
  if ((hasx && (FLOAT8_LE(box->xmax, box->xmin) ||
                FLOAT8_LE(box->ymax, box->ymin) ||
                (hasz && FLOAT8_LE(box->zmax, box->zmin)))) ||
      (hast && datum_le(box->period.upper, box->period.lower, T_TIMESTAMPTZ)))
    return 0.0;

  if (hasx && (isnan(box->xmax) || isnan(box->ymax) ||
               (hasz && isnan(box->zmax))))
    return get_float8_infinity();

  if (hasx)
  {
    result_size *= (box->xmax - box->xmin) * (box->ymax - box->ymin);
    if (hasz)
      result_size *= (box->zmax - box->zmin);
  }
  if (hast)
    result_size *= (DatumGetTimestampTz(box->period.upper) -
      DatumGetTimestampTz(box->period.lower)) / USECS_PER_SEC;
  return result_size;
}

/**
 * @brief Increase in TPCBox volume from inserting @p bbox2 into @p bbox1.
 */
double
tpcbox_penalty(void *bbox1, void *bbox2)
{
  const TPCBox *original = (TPCBox *) bbox1;
  const TPCBox *new = (TPCBox *) bbox2;
  TPCBox unionbox;
  memcpy(&unionbox, original, sizeof(TPCBox));
  tpcbox_adjust(&unionbox, (void *) new);
  return tpcbox_size(&unionbox) - tpcbox_size(original);
}

PGDLLEXPORT Datum Tpcbox_gist_penalty(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_gist_penalty);
/**
 * @ingroup mobilitydb_pointcloud_index
 * @brief GiST penalty method
 * @sqlfn tpcbox_gist_penalty()
 */
Datum
Tpcbox_gist_penalty(PG_FUNCTION_ARGS)
{
  GISTENTRY *origentry = (GISTENTRY *) PG_GETARG_POINTER(0);
  GISTENTRY *newentry = (GISTENTRY *) PG_GETARG_POINTER(1);
  float *result = (float *) PG_GETARG_POINTER(2);
  void *origbox = (TPCBox *) DatumGetPointer(origentry->key);
  void *newbox = (TPCBox *) DatumGetPointer(newentry->key);
  *result = (float) tpcbox_penalty(origbox, newbox);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * GiST picksplit
 *****************************************************************************/

PGDLLEXPORT Datum Tpcbox_gist_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_gist_picksplit);
/**
 * @ingroup mobilitydb_pointcloud_index
 * @brief GiST picksplit method — defers to the generic double-sorting
 *   helper, which knows the TPCBox layout via T_TPCBOX
 * @sqlfn tpcbox_gist_picksplit()
 */
Datum
Tpcbox_gist_picksplit(PG_FUNCTION_ARGS)
{
  return bbox_gist_picksplit(fcinfo, T_TPCBOX, &tpcbox_adjust,
    &tpcbox_penalty);
}

/*****************************************************************************
 * GiST same
 *****************************************************************************/

PGDLLEXPORT Datum Tpcbox_gist_same(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_gist_same);
/**
 * @ingroup mobilitydb_pointcloud_index
 * @brief GiST same method — exact equality, not the user-facing
 *   @c same_tpcbox_tpcbox (which is fuzzy on the same-pcid front)
 * @sqlfn tpcbox_gist_same()
 */
Datum
Tpcbox_gist_same(PG_FUNCTION_ARGS)
{
  TPCBox *b1 = PG_GETARG_TPCBOX_P(0);
  TPCBox *b2 = PG_GETARG_TPCBOX_P(1);
  bool *result = (bool *) PG_GETARG_POINTER(2);
  if (b1 && b2)
    *result = (b1->pcid == b2->pcid &&
      FLOAT8_EQ(b1->xmin, b2->xmin) && FLOAT8_EQ(b1->ymin, b2->ymin) &&
      FLOAT8_EQ(b1->zmin, b2->zmin) && FLOAT8_EQ(b1->xmax, b2->xmax) &&
      FLOAT8_EQ(b1->ymax, b2->ymax) && FLOAT8_EQ(b1->zmax, b2->zmax) &&
      /* The period is compared as a span, so that two keys ending on the same
       * instant with a different inclusivity are NOT reported as the same */
      MEOS_FLAGS_GET_T(b1->flags) == MEOS_FLAGS_GET_T(b2->flags) &&
      (! MEOS_FLAGS_GET_T(b1->flags) ||
        span_eq(&b1->period, &b2->period)));
  else
    *result = (b1 == NULL && b2 == NULL);
  PG_RETURN_POINTER(result);
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
 * GiST distance — KNN ordering (strategy 25)
 *****************************************************************************/

PGDLLEXPORT Datum Tpcbox_gist_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_gist_distance);
/**
 * @ingroup mobilitydb_pointcloud_index
 * @brief GiST distance method — minimum nearest-approach distance from
 *   the index key (TPCBox) to the query (tpcbox / tpcpoint / tpcpatch)
 * @details Returns DBL_MAX for null keys or pcid mismatches; the
 *   recheck on leaf entries refines the distance to the actual leaf
 *   values.
 * @sqlfn tpcbox_gist_distance()
 */
Datum
Tpcbox_gist_distance(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  TPCBox *key = (TPCBox *) DatumGetPointer(entry->key);
  if (! key)
    PG_RETURN_FLOAT8(DBL_MAX);

  /* Index keys are bboxes; recheck on leaf levels refines against the
   * actual leaf entry. */
  if (GIST_LEAF(entry))
    *recheck = true;

  /* Derive a TPCBox from whatever query-type the user supplied. */
  TPCBox query;
  MeosType qtype = oid_meostype(typid);
  if (qtype == T_TPCBOX)
  {
    memcpy(&query, DatumGetTpcboxP(PG_GETARG_DATUM(1)), sizeof(TPCBox));
  }
  else if (qtype == T_TPCPOINT || qtype == T_TPCPATCH)
  {
    Temporal *temp = temporal_slice(PG_GETARG_DATUM(1));
    temporal_set_bbox(temp, &query);
  }
  else
    PG_RETURN_FLOAT8(DBL_MAX);

  double distance = nad_tpcbox_tpcbox(key, &query);
  if (distance < 0)
    PG_RETURN_FLOAT8(DBL_MAX);
  PG_RETURN_FLOAT8(distance);
}

/*****************************************************************************
 * GiST sort support method
 *****************************************************************************/


/**
 * @brief Convert a point cloud box into its abbreviated key
 * @details A `TPCBox` begins with a whole `STBox`, so it takes the
 * spatiotemporal key; the schema identifier it carries beyond that is not a
 * dimension and only settles the boxes the curve ties.
 */
static Datum
Tpcbox_abbrev_convert(Datum original, SortSupport ssup)
{
  (void) ssup;
  return UInt64GetDatum(stbox_sort_hash((const STBox *)
    DatumGetTpcboxP(original)));
}

/**
 * @brief Compare two point cloud boxes for the sorted index build
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
 * @ingroup mobilitydb_pointcloud_index
 * @brief GiST sort support method for point cloud values
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

/*****************************************************************************/
