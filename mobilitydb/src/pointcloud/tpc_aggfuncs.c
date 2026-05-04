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
 * @brief PG wrappers for TPCBox-based aggregate functions over the
 *   pgPointCloud temporal types — currently @c extent for tpcpoint /
 *   tpcpatch / tpcbox. Mirrors the stbox / tspatial extent surface in
 *   @c mobilitydb/src/geo/tgeo_aggfuncs.c.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "pointcloud/tpc_boxops.h"
#include "pointcloud/tpcbox.h"          /* PG_GETARG_TPCBOX_P, etc. */
#include "pointcloud/pcpatch.h"
#include "temporal/temporal.h"
#include "temporal/skiplist.h"          /* PG_RETURN_SKIPLIST_P macro */
#include "pg_temporal/skiplist.h"       /* INPUT_AGG_TRANS_STATE etc. */

/* Internal MEOS combine functions reused by the tnpoints / tdensity
 * aggregates. */
extern Datum datum_sum_int32(Datum l, Datum r);
extern Datum datum_sum_float8(Datum l, Datum r);

/*****************************************************************************
 * Extent aggregation
 *
 * One transfn covers tpcpoint and tpcpatch (the second arg is a generic
 * Temporal, dispatched by temporal_set_bbox). A separate transfn handles
 * tpcbox-with-tpcbox aggregation, plus the parallel-combinefn.
 *****************************************************************************/

PGDLLEXPORT Datum Tpc_extent_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpc_extent_transfn);
/**
 * @ingroup mobilitydb_pointcloud_agg
 * @brief Transition function for the extent aggregate over tpcpoint /
 *   tpcpatch values.
 * @sqlfn extent()
 */
Datum
Tpc_extent_transfn(PG_FUNCTION_ARGS)
{
  TPCBox *state = PG_ARGISNULL(0) ? NULL : PG_GETARG_TPCBOX_P(0);
  Temporal *temp = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL_P(1);
  TPCBox *result = tpcbox_extent_transfn(state, temp);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TPCBOX_P(result);
}

PGDLLEXPORT Datum Tpcbox_extent_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_extent_transfn);
/**
 * @ingroup mobilitydb_pointcloud_agg
 * @brief Transition function for the extent aggregate over tpcbox values.
 *   Doubles as the parallel combine function for the temporal variants.
 * @sqlfn extent()
 */
Datum
Tpcbox_extent_transfn(PG_FUNCTION_ARGS)
{
  TPCBox *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_TPCBOX_P(0);
  TPCBox *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_TPCBOX_P(1);
  if (! box1 && ! box2) PG_RETURN_NULL();
  if (! box1) PG_RETURN_TPCBOX_P(tpcbox_copy(box2));
  if (! box2) PG_RETURN_TPCBOX_P(tpcbox_copy(box1));
  if (box1->pcid != box2->pcid)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Extent aggregation across distinct pcids: %u vs %u",
        box1->pcid, box2->pcid)));
  TPCBox *result = tpcbox_copy(box1);
  tpcbox_expand(box2, result);
  PG_RETURN_TPCBOX_P(result);
}

/*****************************************************************************
 * tnpoints — temporal running point-count aggregate
 *
 * Distinct from tcount(tpcpatch), which counts the *number of patches*
 * present at each instant. tnpoints accumulates the per-instant
 * pcpatch's npoints, so the result tint reads as "how many points
 * total are in the cloud at time t".
 *****************************************************************************/

/**
 * @brief Walk a Temporal of tpcpatch and return an array of TInstants
 * over T_TINT where each instant carries the pcpatch's npoints.
 */
static TInstant **
tpcpatch_transform_tnpoints(const Temporal *temp, int *count_out)
{
  /* Easiest correct path: enumerate all instants regardless of subtype.
   * temporal_num_instants returns total count across instant / sequence
   * / sequenceset; temporal_instant_n returns the i-th. */
  int n = temporal_num_instants(temp);
  TInstant **result = palloc(sizeof(TInstant *) * n);
  for (int i = 0; i < n; i++)
  {
    const TInstant *inst = temporal_instant_n(temp, i + 1);
    Pcpatch *pa = (Pcpatch *) DatumGetPointer(tinstant_value_p(inst));
    int32 npts = (int32) pcpatch_npoints(pa);
    result[i] = tinstant_make(Int32GetDatum(npts), T_TINT, inst->t);
  }
  *count_out = n;
  return result;
}

PGDLLEXPORT Datum Tpcpatch_tnpoints_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_tnpoints_transfn);
/**
 * @ingroup mobilitydb_pointcloud_agg
 * @brief Transition function for tnpoints(tpcpatch) — temporal running
 *   sum of per-instant pcpatch->npoints.
 * @sqlfn tnpoints()
 */
Datum
Tpcpatch_tnpoints_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  if (PG_ARGISNULL(1))
  {
    if (state) PG_RETURN_POINTER(state);
    PG_RETURN_NULL();
  }
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  store_fcinfo(fcinfo);
  int n;
  TInstant **insts = tpcpatch_transform_tnpoints(temp, &n);
  if (! state) state = temporal_skiplist_make();
  temporal_skiplist_splice(state, (void **) insts, n, &datum_sum_int32, false);
  for (int i = 0; i < n; i++)
    pfree(insts[i]);
  pfree(insts);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_SKIPLIST_P(state);
}

/*****************************************************************************
 * tdensity — temporal running per-instant density aggregate
 *
 * Per-instant density is npoints / xy-bbox area. xy-bbox comes from the
 * pcpatch's inline PCBOUNDS (xmin/xmax/ymin/ymax). Composes with sum to
 * give a running per-timestamp density across multiple tpcpatch rows.
 *
 * Edge cases: a 1-point or co-linear patch has zero-area bbox; we emit
 * +Infinity for that instant (IEEE 1.0/0.0). Callers can filter with
 * isfinite() / IS NOT NAN if they want to drop those.
 *****************************************************************************/

/**
 * @brief Walk a Temporal of tpcpatch and return an array of TInstants
 * over T_TFLOAT each carrying npoints / (xrange * yrange).
 */
static TInstant **
tpcpatch_transform_tdensity(const Temporal *temp, int *count_out)
{
  int n = temporal_num_instants(temp);
  TInstant **result = palloc(sizeof(TInstant *) * n);
  for (int i = 0; i < n; i++)
  {
    const TInstant *inst = temporal_instant_n(temp, i + 1);
    Pcpatch *pa = (Pcpatch *) DatumGetPointer(tinstant_value_p(inst));
    double xrange = pa->bounds[1] - pa->bounds[0];
    double yrange = pa->bounds[3] - pa->bounds[2];
    double area = xrange * yrange;
    double density = (area > 0.0) ? (double) pa->npoints / area
                                   : (double) pa->npoints / 0.0;
    result[i] = tinstant_make(Float8GetDatum(density), T_TFLOAT, inst->t);
  }
  *count_out = n;
  return result;
}

PGDLLEXPORT Datum Tpcpatch_tdensity_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_tdensity_transfn);
/**
 * @ingroup mobilitydb_pointcloud_agg
 * @brief Transition function for tdensity(tpcpatch) — temporal running
 *   sum of per-instant density (npoints / xy-bbox-area).
 * @sqlfn tdensity()
 */
Datum
Tpcpatch_tdensity_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  if (PG_ARGISNULL(1))
  {
    if (state) PG_RETURN_POINTER(state);
    PG_RETURN_NULL();
  }
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  store_fcinfo(fcinfo);
  int n;
  TInstant **insts = tpcpatch_transform_tdensity(temp, &n);
  if (! state) state = temporal_skiplist_make();
  temporal_skiplist_splice(state, (void **) insts, n, &datum_sum_float8, false);
  for (int i = 0; i < n; i++)
    pfree(insts[i]);
  pfree(insts);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_SKIPLIST_P(state);
}

/*****************************************************************************/
