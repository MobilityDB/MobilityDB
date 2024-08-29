/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief R-tree GiST index for temporal points
 */

#include "pg_point/tpoint_gist.h"

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
#include "general/span.h"
#include "general/type_util.h"
#include "point/tpoint_gist.h"
#include "point/stbox.h"
#include "point/stbox_index.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/temporal.h"
#include "pg_general/tnumber_gist.h"

/*****************************************************************************
 * GiST consistent methods
 *****************************************************************************/

/**
 * @brief Transform the query argument into a box initializing the dimensions
 * that must not be taken into account by the operators to infinity.
 */
static bool
tpoint_gist_get_stbox(FunctionCallInfo fcinfo, STBox *result, meosType type)
{
  if (type == T_TSTZSPAN)
  {
    Span *s = PG_GETARG_SPAN_P(1);
    tstzspan_set_stbox(s, result);
  }
  else if (type == T_STBOX)
  {
    STBox *box = PG_GETARG_STBOX_P(1);
    if (box == NULL)
      return false;
    memcpy(result, box, sizeof(STBox));
  }
  else if (tspatial_type(type))
  {
    if (PG_ARGISNULL(1))
      return false;
    Datum tempdatum = PG_GETARG_DATUM(1);
    Temporal *temp = temporal_slice(tempdatum);
    tspatial_set_stbox(temp, result);
  }
  else
    elog(ERROR, "Unsupported type for indexing: %d", type);
  return true;
}

PGDLLEXPORT Datum Stbox_gist_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_gist_consistent);
/**
 * @brief GiST consistent method for temporal points
 */
Datum
Stbox_gist_consistent(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4), result;
  STBox *key = DatumGetSTboxP(entry->key), query;

  /* Determine whether the index is lossy depending on the strategy */
  *recheck = stbox_index_recheck(strategy);

  if (key == NULL)
    PG_RETURN_BOOL(false);

  /* Transform the query into a box */
  if (! tpoint_gist_get_stbox(fcinfo, &query, oid_type(typid)))
    PG_RETURN_BOOL(false);

  if (GIST_LEAF(entry))
    result = stbox_index_leaf_consistent(key, &query, strategy);
  else
    result = stbox_gist_inner_consistent(key, &query, strategy);

  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * GiST union method
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_gist_union(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_gist_union);
/**
 * @brief GiST union method for temporal points
 *
 * Return the minimal bounding box that encloses all the entries in entryvec
 */
Datum
Stbox_gist_union(PG_FUNCTION_ARGS)
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GISTENTRY *ent = entryvec->vector;
  STBox *result = stbox_cp(DatumGetSTboxP(ent[0].key));
  for (int i = 1; i < entryvec->n; i++)
    stbox_adjust(result, DatumGetSTboxP(ent[i].key));
  PG_RETURN_SPAN_P(result);
}

/*****************************************************************************
 * GiST compress methods
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_gist_compress(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_gist_compress);
/**
 * @brief GiST compress methods for temporal points
 */
Datum
Tpoint_gist_compress(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  if (entry->leafkey)
  {
    GISTENTRY *retval = palloc(sizeof(GISTENTRY));
    STBox *box = palloc(sizeof(STBox));
    Temporal *temp = temporal_slice(entry->key);
    tspatial_set_stbox(temp, box);
    gistentryinit(*retval, PointerGetDatum(box), entry->rel, entry->page,
      entry->offset, false);
    PG_RETURN_STBOX_P(retval);
  }
  PG_RETURN_POINTER(entry);
}

/*****************************************************************************
 * GiST penalty method
 *****************************************************************************/

/**
 * @brief Calculate the union of two spatiotemporal boxes
 * @param[in] a,b Input boxes
 * @param[out] new Resulting box
 */
static void
stbox_union_rt(const STBox *a, const STBox *b, STBox *new)
{
  memset(new, 0, sizeof(STBox));
  new->xmin = FLOAT8_MIN(a->xmin, b->xmin);
  new->xmax = FLOAT8_MAX(a->xmax, b->xmax);
  new->ymin = FLOAT8_MIN(a->ymin, b->ymin);
  new->ymax = FLOAT8_MAX(a->ymax, b->ymax);
  new->zmin = FLOAT8_MIN(a->zmin, b->zmin);
  new->zmax = FLOAT8_MAX(a->zmax, b->zmax);
  TimestampTz tmin = Min(DatumGetTimestampTz(a->period.lower),
    DatumGetTimestampTz(b->period.lower));
  TimestampTz tmax = Max(DatumGetTimestampTz(a->period.upper),
    DatumGetTimestampTz(b->period.upper));
  new->period.lower = TimestampTzGetDatum(tmin);
  new->period.upper = TimestampTzGetDatum(tmax);
  return;
}

/**
 * @brief Return the size of a spatiotemporal box for penalty-calculation
 * purposes
 * @note The result can be +Infinity, but not NaN
 */
static double
stbox_size(const STBox *box)
{
  double result_size = 1;
  bool  hasx = MEOS_FLAGS_GET_X(box->flags),
        hasz = MEOS_FLAGS_GET_Z(box->flags),
        hast = MEOS_FLAGS_GET_T(box->flags);
  /*
   * Check for zero-width cases.  Note that we define the size of a zero-
   * by-infinity box as zero.  It's important to special-case this somehow,
   * as naively multiplying infinity by zero will produce NaN.
   *
   * The less-than cases should not happen, but if they do, say "zero".
   */
  if ((hasx && (FLOAT8_LE(box->xmax, box->xmin)
                || FLOAT8_LE(box->ymax, box->ymin)
                || (hasz && FLOAT8_LE(box->zmax, box->zmin))))
      || (hast && datum_le(box->period.upper, box->period.lower, T_TIMESTAMPTZ)))
    return 0.0;

  /*
   * We treat NaN as larger than +Infinity, so any distance involving a NaN
   * and a non-NaN is infinite.  Note the previous check eliminated the
   * possibility that the low fields are NaNs.
   */
  if (hasx && (isnan(box->xmax) || isnan(box->ymax) || (hasz && isnan(box->zmax))))
    return get_float8_infinity();

  /*
   * Compute the box size
   */
  if (hasx)
  {
    result_size *= (box->xmax - box->xmin) * (box->ymax - box->ymin);
    if (hasz)
      result_size *= (box->xmax - box->xmin) * (box->ymax - box->ymin);
  }
  if (hast)
    /* Expressed in seconds */
    result_size *= (DatumGetTimestampTz(box->period.upper) -
      DatumGetTimestampTz(box->period.lower)) / USECS_PER_SEC;
  return result_size;
}

/**
 * @brief Return the amount by which the union of the two boxes is larger than
 * the original STBox's volume
 * @note The result can be +Infinity, but not NaN
 */
double
stbox_penalty(void *bbox1, void *bbox2)
{
  const STBox *original = (STBox *) bbox1;
  const STBox *new = (STBox *) bbox2;
  STBox unionbox;
  stbox_union_rt(original, new, &unionbox);
  return stbox_size(&unionbox) - stbox_size(original);
}

PGDLLEXPORT Datum Stbox_gist_penalty(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_gist_penalty);
/**
 * @brief GiST penalty method for temporal points
 * @note As in the R-tree paper, we use change in area as our penalty metric
 */
Datum
Stbox_gist_penalty(PG_FUNCTION_ARGS)
{
  GISTENTRY *origentry = (GISTENTRY *) PG_GETARG_POINTER(0);
  GISTENTRY *newentry = (GISTENTRY *) PG_GETARG_POINTER(1);
  float *result = (float *) PG_GETARG_POINTER(2);
  void *origstbox = (STBox *) DatumGetPointer(origentry->key);
  void *newbox = (STBox *) DatumGetPointer(newentry->key);
  *result = (float) stbox_penalty(origstbox, newbox);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * GiST picksplit method
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_gist_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_gist_picksplit);
/**
 * @brief GiST picksplit method for temporal points
 *
 * The algorithm finds split of boxes by considering splits along each axis.
 * Each entry is first projected as an interval on the X-axis, and different
 * ways to split the intervals into two groups are considered, trying to
 * minimize the overlap of the groups. Then the same is repeated for the
 * Y-axis and the Z-axis, and the overall best split is chosen.
 * The quality of a split is determined by overlap along that axis and some
 * other criteria (see bbox_gist_consider_split).
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
Stbox_gist_picksplit(PG_FUNCTION_ARGS)
{
  return bbox_gist_picksplit(fcinfo, T_STBOX, &stbox_adjust, &stbox_penalty);
}
/*****************************************************************************
 * GiST same method
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_gist_same(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_gist_same);
/**
 * @brief GiST same method for temporal points
 *
 * Return true only when boxes are exactly the same.  We can't use fuzzy
 * comparisons here without breaking index consistency; therefore, this isn't
 * equivalent to stbox_same().
 */
Datum
Stbox_gist_same(PG_FUNCTION_ARGS)
{
  STBox *b1 = PG_GETARG_STBOX_P(0);
  STBox *b2 = PG_GETARG_STBOX_P(1);
  bool *result = (bool *) PG_GETARG_POINTER(2);
  if (b1 && b2)
    *result = (FLOAT8_EQ(b1->xmin, b2->xmin) && FLOAT8_EQ(b1->ymin, b2->ymin) &&
      FLOAT8_EQ(b1->zmin, b2->zmin) && FLOAT8_EQ(b1->xmax, b2->xmax) &&
      FLOAT8_EQ(b1->ymax, b2->ymax) && FLOAT8_EQ(b1->zmax, b2->zmax) &&
      /* Equality test does not require to use DatumGetTimestampTz */
      (b1->period.lower == b2->period.lower) &&
      (b1->period.upper == b2->period.upper));
  else
    *result = (b1 == NULL && b2 == NULL);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * GiST distance method
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_gist_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_gist_distance);
/**
 * @brief GiST distance for temporal points
 * @note Take in a query and an entry and return the "distance" between them
*/
Datum
Stbox_gist_distance(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  STBox *key = (STBox *) DatumGetPointer(entry->key);
  STBox query;
  double distance;

  /* The index is lossy for leaf levels */
  if (GIST_LEAF(entry))
    *recheck = true;

  if (key == NULL)
    PG_RETURN_FLOAT8(DBL_MAX);

  /* Transform the query into a box */
  if (! tpoint_gist_get_stbox(fcinfo, &query, oid_type(typid)))
    PG_RETURN_FLOAT8(DBL_MAX);

  /* Since we only have boxes we'll return the minimum possible distance,
   * and let the recheck sort things out in the case of leaves */
  distance = nad_stbox_stbox(key, &query);

  PG_RETURN_FLOAT8(distance);
}

/*****************************************************************************/
