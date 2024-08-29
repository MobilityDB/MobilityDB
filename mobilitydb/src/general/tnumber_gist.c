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
 * @brief R-tree GiST index for temporal integers and temporal floats
 *
 * These functions are based on those in the file `gistproc.c`.
 */

#include "pg_general/tnumber_gist.h"


#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/span.h"
#include "general/tbox.h"
#include "general/tbox_index.h"
#include "general/temporal_boxops.h"
#include "general/type_util.h"
#include "general/tnumber_gist.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/temporal.h"
#include "pg_general/span_gist.h"

/*****************************************************************************
 * GiST consistent methods
 *****************************************************************************/

/**
 * @brief Transform the query argument into a box initializing the dimensions
 * that must not be taken into account by the operators to infinity
 */
static bool
tnumber_gist_get_tbox(FunctionCallInfo fcinfo, TBox *result, Oid typid)
{
  meosType type = oid_type(typid);
  Span *s;
  if (tnumber_spantype(type))
  {
    s = PG_GETARG_SPAN_P(1);
    if (s == NULL)
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
    if (box == NULL)
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
  bool *recheck = (bool *) PG_GETARG_POINTER(4), result;
  const TBox *key = DatumGetTboxP(entry->key);
  TBox query;

  /*
   * All tests are lossy since boxes do not distinghish between inclusive
   * and exclusive bounds.
   */
  *recheck = true;

  if (key == NULL)
    PG_RETURN_BOOL(false);

  /* Transform the query into a box */
  if (! tnumber_gist_get_tbox(fcinfo, &query, typid))
    PG_RETURN_BOOL(false);

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
 *
 * Return the minimal bounding box that encloses all the entries in entryvec
 */
Datum
Tbox_gist_union(PG_FUNCTION_ARGS)
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GISTENTRY *ent = entryvec->vector;
  TBox *result = tbox_cp(DatumGetTboxP(ent[0].key));
  for (int i = 1; i < entryvec->n; i++)
    tbox_adjust((void *)result, DatumGetPointer(ent[i].key));
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

/* Helper macros to place an entry in the left or right group */
#define PLACE_LEFT(box, off) \
  do {  \
    if (v->spl_nleft > 0)  \
      bbox_adjust(leftBox, box);  \
    else  \
      memcpy(leftBox, box, bbox_size);  \
    v->spl_left[v->spl_nleft++] = off;  \
  } while(0)

#define PLACE_RIGHT(box, off)  \
  do {  \
    if (v->spl_nright > 0)  \
      bbox_adjust(rightBox, box);  \
    else  \
      memcpy(rightBox, box, bbox_size);  \
    v->spl_right[v->spl_nright++] = off;  \
  } while(0)



/**
 * @brief Double sorting split algorithm
 *
 * The algorithm finds split of boxes by considering splits along each axis.
 * Each entry is first projected as an interval on the X-axis, and different
 * ways to split the intervals into two groups are considered, trying to
 * minimize the overlap of the groups. Then the same is repeated for the
 * Y-axis, and the overall best split is chosen. The quality of a split is
 * determined by overlap along that axis and some other criteria (see
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
 * @param[in] fcinfo Function call Info. Used to get the PG_GETARG_POINTER for 
 * * entryvec and *v.
 * @param[in] bboxtype Meostype of the type of box. Supports T_BOX and T_STBOX
 * @param[in] bbox_adjust Increase the first box to include the second one
 * @param[in] bbox_penalty Return the amount by which the union of the 
 * two boxes is larger than the original STBox's volume.

 */
Datum
bbox_gist_picksplit(FunctionCallInfo fcinfo, meosType bboxtype,
  void (*bbox_adjust)(void *, void *), double (*bbox_penalty)(void *, void *))
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);
  bbox_picksplit(bboxtype, bbox_adjust, bbox_penalty, entryvec, v);
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
 * Return true only when boxes are exactly the same.  We can't use fuzzy
 * comparisons here without breaking index consistency; therefore, this isn't
 * equivalent to box_same().
 */
Datum
Tbox_gist_same(PG_FUNCTION_ARGS)
{
  TBox *b1 = PG_GETARG_TBOX_P(0);
  TBox *b2 = PG_GETARG_TBOX_P(1);
  bool *result = (bool *) PG_GETARG_POINTER(2);
  if (b1 && b2)
    *result = FLOAT8_EQ(DatumGetFloat8(b1->span.lower),
        DatumGetFloat8(b2->span.lower)) &&
      FLOAT8_EQ(DatumGetFloat8(b1->span.upper),
        DatumGetFloat8(b2->span.upper)) &&
      /* Equality test does not require to use DatumGetTimestampTz */
      (b1->period.lower == b2->period.lower) &&
      (b1->period.upper == b2->period.upper);
  else
    *result = (b1 == NULL && b2 == NULL);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * GiST distance method
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_gist_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_gist_distance);
/**
 * @brief GiST support distance function that takes in a query and an entry and
 * returns the "distance" between them
*/
Datum
Tbox_gist_distance(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  TBox *key = (TBox *) DatumGetPointer(entry->key);
  TBox query;
  double distance;

  /* The index is lossy for leaf levels */
  if (GIST_LEAF(entry))
    *recheck = true;

  if (key == NULL)
    PG_RETURN_FLOAT8(DBL_MAX);

  /* Transform the query into a box */
  if (! tnumber_gist_get_tbox(fcinfo, &query, typid))
    PG_RETURN_FLOAT8(DBL_MAX);

  /* Since we only have boxes we'll return the minimum possible distance,
   * and let the recheck sort things out in the case of leaves */
  distance = nad_tbox_tbox(key, &query);

  PG_RETURN_FLOAT8(distance);
}

/*****************************************************************************/
