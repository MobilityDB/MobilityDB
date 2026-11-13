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
 * @brief PG wrappers for the tpcpatch lifted temporal type
 * @details Structural mirror of mobilitydb/src/pointcloud/tpcpoint.c. The
 * generic Temporal_* dispatch tables handle @c T_TPCPATCH via the base-type
 * wiring; this file holds the wrappers that need to know they are looking at
 * pcpatch data, among them the pcid and point count accessors.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <funcapi.h>
#include <access/htup_details.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"
#include "temporal/tsequence.h"
#include "temporal/tsequenceset.h"
#include "pointcloud/pcpatch.h"
#include "pointcloud/pcpatch_decompose.h"
#include "pointcloud/pgsql_compat.h"
#include "pointcloud/meos_schema_hook.h"
#include "pointcloud/tpcbox.h"
#include "pg_geo/postgis.h"
/* pgPointCloud */
#include "pc_api.h"

/*****************************************************************************
 * Conversion to tgeometry
 *
 * The value-level answer — the multipoint of the positions a patch's points
 * occupy — lives in the MEOS function @c pcpatch_to_geom, which @c
 * tpcpatch_to_tgeometry lifts over time. The temporal spatial relationships
 * of the type reach the geometry engine through the cast this wrapper backs.
 *****************************************************************************/

PGDLLEXPORT Datum Tpcpatch_to_tgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_to_tgeometry);
/**
 * @ingroup mobilitydb_pointcloud_conversion
 * @brief Convert a tpcpatch into the temporal geometry of the multipoints its
 * patches occupy
 * @sqlfn tgeometry()
 */
Datum
Tpcpatch_to_tgeometry(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpcpatch_to_tgeometry(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpcpatch_to_tgeography(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_to_tgeography);
/**
 * @ingroup mobilitydb_pointcloud_conversion
 * @brief Convert a temporal point cloud patch into a temporal geography
 * @sqlfn tgeography()
 * @sqlop @p ::
 */
Datum
Tpcpatch_to_tgeography(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpcpatch_to_tgeography(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Per-type accessors
 *****************************************************************************/

PGDLLEXPORT Datum Tpcpatch_pcid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_pcid);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the pgpointcloud schema id (pcid) of a tpcpatch
 * @sqlfn pcid()
 */
Datum
Tpcpatch_pcid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  uint32_t result = tpcpatch_pcid(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32((int32) result);
}

PGDLLEXPORT Datum Tpcpatch_start_npoints(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_start_npoints);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the number of points in the first instant's pcpatch
 * @sqlfn startNumPoints()
 */
Datum
Tpcpatch_start_npoints(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  uint32_t result = tpcpatch_start_npoints(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32((int32) result);
}

PGDLLEXPORT Datum Tpcpatch_end_npoints(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_end_npoints);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the number of points in the last instant's pcpatch
 * @sqlfn endNumPoints()
 */
Datum
Tpcpatch_end_npoints(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  uint32_t result = tpcpatch_end_npoints(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32((int32) result);
}

PGDLLEXPORT Datum Tpcpatch_npoints(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_npoints);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the total number of points across every instant's pcpatch in a
 * tpcpatch value
 * @sqlfn numPoints()
 */
Datum
Tpcpatch_npoints(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int64 result = tpcpatch_npoints(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT64(result);
}

PGDLLEXPORT Datum Tpcpatch_swath(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_swath);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the swath of a tpcpatch, the region of the plane its patches
 * sweep
 * @sqlfn swath()
 * @note The function is not strict, since the largest gap joined is optional
 */
Datum
Tpcpatch_swath(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *maxt = PG_ARGISNULL(1) ? NULL : PG_GETARG_INTERVAL_P(1);
  GSERIALIZED *result = tpcpatch_swath(temp, maxt);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Per-point restrict by TPCBox (atTpcboxFine / minusTpcboxFine)
 *****************************************************************************/

PGDLLEXPORT Datum Tpcpatch_at_tpcbox_fine(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_at_tpcbox_fine);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Restrict a tpcpatch to only the points inside the supplied TPCBox
 * @details Per-point granularity: each surviving instant carries a
 *   freshly-built pcpatch containing only the points inside the box.
 *   Instants whose patches have zero points inside are dropped, an instant
 *   outside the period of the box having none.
 * @sqlfn atTpcboxFine()
 */
Datum
Tpcpatch_at_tpcbox_fine(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TPCBox *box = PG_GETARG_TPCBOX_P(1);
  bool border_inc = PG_GETARG_BOOL(2);
  Temporal *result = tpcpatch_at_tpcbox_fine(temp, box, border_inc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpcpatch_minus_tpcbox_fine(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_minus_tpcbox_fine);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Restrict a tpcpatch to the points outside the supplied TPCBox
 * @details Mirror of atTpcboxFine; instants whose every point is inside
 *   the box are dropped.
 * @sqlfn minusTpcboxFine()
 */
Datum
Tpcpatch_minus_tpcbox_fine(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TPCBox *box = PG_GETARG_TPCBOX_P(1);
  bool border_inc = PG_GETARG_BOOL(2);
  Temporal *result = tpcpatch_minus_tpcbox_fine(temp, box, border_inc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Per-point restrict by geometry (atGeometry / minusGeometry on tpcpatch)
 *****************************************************************************/

PGDLLEXPORT Datum Tpcpatch_at_geometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_at_geometry);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Restrict a tpcpatch to only the points whose XY projection intersects
 * the supplied geometry
 * @sqlfn atGeometry()
 */
Datum
Tpcpatch_at_geometry(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = tpcpatch_restrict_points(temp, NULL,
    pcpoint_intersects_geometry, (void *) gs, REST_AT);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpcpatch_minus_geometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_minus_geometry);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Restrict a tpcpatch to only the points whose XY projection does NOT
 * intersect the supplied geometry
 * @sqlfn minusGeometry()
 */
Datum
Tpcpatch_minus_geometry(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = tpcpatch_restrict_points(temp, NULL,
    pcpoint_intersects_geometry, (void *) gs, REST_MINUS);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Spatial relationships — eIntersects / aIntersects(tpcpatch, geometry)
 *****************************************************************************/

/**
 * @brief Test whether any instant of @p temp has at least one point matching
 * @p pred
 * @details The function walks instants in subtype order and short-circuits on
 * the first hit; it never rebuilds a patch. It backs the @c eIntersects PG
 * wrapper.
 * @param temp   Source tpcpatch.
 * @param pred   Per-point predicate.
 * @param extra  Caller-supplied state for @p pred.
 * @return @c true on the first matching point, @c false if none of
 *   the instants' points matched.
 */
static bool
tpcpatch_any_point_matches(const Temporal *temp,
  pcpatch_pointpred_fn pred, void *extra)
{
  int ninst = 0;
  const TInstant **instants = temporal_insts_p(temp, &ninst);
  bool found = false;
  for (int i = 0; i < ninst && ! found; i++)
  {
    const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(
      tinstant_value_p(instants[i]));
    if (pcpatch_any_point_matches(pa, pred, extra))
      found = true;
  }
  pfree(instants);
  return found;
}

PGDLLEXPORT Datum Eintersects_tpcpatch_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tpcpatch_geo);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true iff at least one point in any of the tpcpatch's instants
 * intersects the geometry
 * @details Walks instants in order, short-circuiting on the first
 *   hit. No patch rebuild, no allocations beyond the temporary
 *   per-instant decompression.
 * @sqlfn eIntersects()
 */
Datum
Eintersects_tpcpatch_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  bool result = tpcpatch_any_point_matches(temp,
    pcpoint_intersects_geometry, (void *) gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Tpcpatch_at_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_at_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Restrict a tpcpatch to the patches whose extent overlaps the box
 * and whose timestamp falls in its period
 * @details Patch-level granularity: each surviving instant keeps its
 *   pcpatch payload verbatim. The X and Y extent is the PCBOUNDS header of
 *   the patch, and the Z extent, tested when the box has Z, the one its
 *   points state.
 * @sqlfn atTpcbox()
 */
Datum
Tpcpatch_at_tpcbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TPCBox *box = PG_GETARG_TPCBOX_P(1);
  bool border_inc = PG_GETARG_BOOL(2);
  Temporal *result = tpcpatch_at_tpcbox(temp, box, border_inc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpcpatch_minus_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_minus_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Remove from a tpcpatch the instants whose patch passes the
 * extent-overlap + timestamp-in-period predicate
 * @sqlfn minusTpcbox()
 */
Datum
Tpcpatch_minus_tpcbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TPCBox *box = PG_GETARG_TPCBOX_P(1);
  bool border_inc = PG_GETARG_BOOL(2);
  Temporal *result = tpcpatch_minus_tpcbox(temp, box, border_inc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * points(tpcpatch) — set-returning function emitting one row per
 * (timestamp, pcpoint) pair. Native C path replacing the
 * PC_Explode-based SQL wrapper used elsewhere in the codebase.
 *****************************************************************************/

typedef struct
{
  int n;             /* total emissions */
  int i;             /* next emission index */
  TimestampTz *ts;   /* parallel arrays */
  Datum *pts;        /* serialized Pcpoint datums (Pointer) */
} TpcpatchPointsState;

/**
 * @brief Build the materialized state backing the @c points(tpcpatch) SRF
 * @details Walks every instant of @p temp, decompresses each patch via
 * @c MEOS_PC_PATCH_DESERIALIZE, materializes a flat array of
 * (timestamp, serialized-Pcpoint) pairs in the SRF's
 * @c multi_call_memory_ctx so subsequent @c SRF_PERCALL invocations
 * just emit one entry at a time.
 *
 * @param temp Source tpcpatch.
 * @return Newly allocated state (in the current memory context;
 *   caller must invoke from inside the SRF's
 *   @c multi_call_memory_ctx). Raises @c meos_error and returns
 *   @c NULL when the schema for @c temp's pcid cannot be resolved.
 */
static TpcpatchPointsState *
tpcpatch_points_state_make(const Temporal *temp)
{
  /* Resolve schema once — every instant shares pcid by construction. */
  const Pcpatch *first = (const Pcpatch *) DatumGetPointer(
    temporal_start_value(temp));
  PCSCHEMA *schema = meos_pc_schema(first->pcid);
  if (! schema)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "points(tpcpatch): no schema registered for pcid %u", first->pcid);
    return NULL;
  }

  int ninst = 0;
  const TInstant **instants = temporal_insts_p(temp, &ninst);

  /* First pass: count points. */
  uint64_t total = 0;
  uint32_t *per_inst_n = palloc(sizeof(uint32_t) * ninst);
  PCPATCH **patches = palloc(sizeof(PCPATCH *) * ninst);
  PCPOINTLIST **lists = palloc(sizeof(PCPOINTLIST *) * ninst);
  for (int i = 0; i < ninst; i++)
  {
    const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(
      tinstant_value_p(instants[i]));
    patches[i] = MEOS_PC_PATCH_DESERIALIZE(
      (const SERIALIZED_PATCH *) pa, schema);
    lists[i] = patches[i] ? pc_pointlist_from_patch(patches[i]) : NULL;
    per_inst_n[i] = lists[i] ? lists[i]->npoints : 0;
    total += per_inst_n[i];
  }

  TpcpatchPointsState *s = palloc(sizeof(TpcpatchPointsState));
  s->n = (int) total;
  s->i = 0;
  s->ts  = palloc(sizeof(TimestampTz) * (total ? total : 1));
  s->pts = palloc(sizeof(Datum) * (total ? total : 1));

  /* Second pass: serialize each point. */
  uint64_t k = 0;
  for (int i = 0; i < ninst; i++)
  {
    if (! lists[i]) continue;
    TimestampTz t = instants[i]->t;
    for (uint32_t j = 0; j < per_inst_n[i]; j++)
    {
      PCPOINT *pt = pc_pointlist_get_point(lists[i], j);
      SERIALIZED_POINT *ser = MEOS_PC_POINT_SERIALIZE(pt);
      s->ts[k]  = t;
      s->pts[k] = PointerGetDatum(ser);
      k++;
    }
    pc_pointlist_free(lists[i]);
    pc_patch_free(patches[i]);
  }
  pfree(lists); pfree(patches); pfree(per_inst_n); pfree(instants);
  return s;
}

PGDLLEXPORT Datum Tpcpatch_points(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_points);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Set-returning function emitting one (timestamptz, pcpoint) row per
 * point per instant of a tpcpatch
 * @sqlfn points()
 */
Datum
Tpcpatch_points(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;

  if (SRF_IS_FIRSTCALL())
  {
    funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldctx = MemoryContextSwitchTo(
      funcctx->multi_call_memory_ctx);
    Temporal *temp = PG_GETARG_TEMPORAL_P(0);
    funcctx->user_fctx = tpcpatch_points_state_make(temp);
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldctx);
  }

  funcctx = SRF_PERCALL_SETUP();
  TpcpatchPointsState *s = funcctx->user_fctx;
  if (s->i >= s->n)
  {
    SRF_RETURN_DONE(funcctx);
  }

  Datum values[2] = { TimestampTzGetDatum(s->ts[s->i]), s->pts[s->i] };
  bool isnull[2] = { false, false };
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  Datum result = HeapTupleGetDatum(tuple);
  s->i++;
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/
