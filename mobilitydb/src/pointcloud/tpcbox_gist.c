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
#include "temporal/stratnum.h"
#include "temporal/type_util.h"
#include "pointcloud/tpcbox.h"
#include "pointcloud/tpcbox_index.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_temporal/tnumber_gist.h"

/*****************************************************************************
 * GiST consistent
 *****************************************************************************/

PGDLLEXPORT Datum Tpcbox_gist_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_gist_consistent);
/**
 * @ingroup mobilitydb_pointcloud_index
 * @brief GiST consistent method for TPCBox
 */
Datum
Tpcbox_gist_consistent(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  TPCBox *query = PG_GETARG_TPCBOX_P(1);
  StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
  /* Oid typid = PG_GETARG_OID(3); -- always TPCBox in this opclass */
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  TPCBox *key = DatumGetTpcboxP(entry->key);
  if (! key || ! query)
    PG_RETURN_BOOL(false);

  *recheck = tpcbox_index_recheck(strategy);

  bool result;
  if (GIST_LEAF(entry))
    result = tpcbox_index_leaf_consistent(key, query, strategy);
  else
    result = tpcbox_gist_inner_consistent(key, query, strategy);

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
  TimestampTz tmin = Min(DatumGetTimestampTz(box1->period.lower),
    DatumGetTimestampTz(box2->period.lower));
  TimestampTz tmax = Max(DatumGetTimestampTz(box1->period.upper),
    DatumGetTimestampTz(box2->period.upper));
  box1->period.lower = TimestampTzGetDatum(tmin);
  box1->period.upper = TimestampTzGetDatum(tmax);
}

PGDLLEXPORT Datum Tpcbox_gist_union(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_gist_union);
/**
 * @ingroup mobilitydb_pointcloud_index
 * @brief GiST union method — minimum bounding TPCBox over an entry vector.
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
 * @brief GiST picksplit method — defers to the generic
 * double-sorting helper, which knows the TPCBox layout via T_TPCBOX.
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
 * @c same_tpcbox_tpcbox (which is fuzzy on the same-pcid front).
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
      (b1->period.lower == b2->period.lower) &&
      (b1->period.upper == b2->period.upper));
  else
    *result = (b1 == NULL && b2 == NULL);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
