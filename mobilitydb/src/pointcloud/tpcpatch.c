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
 * @brief PG wrappers for the tpcpatch lifted temporal type.
 *
 * Structural mirror of mobilitydb/src/pointcloud/tpcpoint.c. The generic
 * Temporal_* dispatch tables already handle @c T_TPCPATCH via the
 * base-type wiring; this file holds only the per-type pcid accessor
 * (and will grow a per-instant points-count accessor later).
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <funcapi.h>
#include <access/htup_details.h>
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
 * Per-type accessors
 *****************************************************************************/

PGDLLEXPORT Datum Tpcpatch_pcid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_pcid);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the pgpointcloud schema id (pcid) of a tpcpatch.
 * @details All instants share the same pcid — enforced at construction
 *   time by set_make_exp's same-pcid check. This is just a first-
 *   instant read.
 * @sqlfn pcid()
 */
Datum
Tpcpatch_pcid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum first = temporal_start_value(temp);
  const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(first);
  uint32_t pcid = pcpatch_get_pcid(pa);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32((int32) pcid);
}

PGDLLEXPORT Datum Tpcpatch_start_npoints(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_start_npoints);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the number of points in the first instant's pcpatch.
 * @sqlfn startNumPoints()
 */
Datum
Tpcpatch_start_npoints(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum first = temporal_start_value(temp);
  const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(first);
  uint32_t n = pcpatch_npoints(pa);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32((int32) n);
}

PGDLLEXPORT Datum Tpcpatch_end_npoints(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_end_npoints);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the number of points in the last instant's pcpatch.
 * @sqlfn endNumPoints()
 */
Datum
Tpcpatch_end_npoints(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum last = temporal_end_value(temp);
  const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(last);
  uint32_t n = pcpatch_npoints(pa);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32((int32) n);
}

PGDLLEXPORT Datum Tpcpatch_npoints(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_npoints);
/**
 * @ingroup mobilitydb_pointcloud_accessor
 * @brief Return the total number of points across every instant's
 *   pcpatch in a tpcpatch value.
 * @details Per-instant @c pcpatch_npoints summed without decompressing
 *   any payload (the count is in the patch header). bigint return type
 *   because a long sequence of dense patches can blow past int32.
 * @sqlfn numPoints()
 */
Datum
Tpcpatch_npoints(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int64 total = 0;
  if (temp->subtype == TINSTANT)
  {
    const TInstant *inst = (const TInstant *) temp;
    const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(tinstant_value_p(inst));
    total = (int64) pcpatch_npoints(pa);
  }
  else if (temp->subtype == TSEQUENCE)
  {
    const TSequence *seq = (const TSequence *) temp;
    for (int i = 0; i < seq->count; i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(tinstant_value_p(inst));
      total += (int64) pcpatch_npoints(pa);
    }
  }
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    for (int i = 0; i < ss->count; i++)
    {
      const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
      for (int j = 0; j < seq->count; j++)
      {
        const TInstant *inst = TSEQUENCE_INST_N(seq, j);
        const Pcpatch *pa =
          (const Pcpatch *) DatumGetPointer(tinstant_value_p(inst));
        total += (int64) pcpatch_npoints(pa);
      }
    }
  }
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT64(total);
}

/*****************************************************************************
 * TPCBox-based restriction (patch-level / coarse)
 *
 * Each pcpatch instant is kept-or-dropped based on whether its 2D
 * PCBOUNDS overlaps the box's xy extent and its timestamp falls in
 * the box's period. No per-point decompression is performed, and the
 * patch payload is preserved verbatim. The Z dimension is ignored at
 * this granularity because pgpointcloud's PCBOUNDS is 2D-only.
 *
 * The @p border_inc flag toggles inclusive (touching counts) vs.
 * strict overlap; the default is inclusive.
 *****************************************************************************/

/**
 * @brief Per-instant predicate: pcpatch's pcid + 2D @c PCBOUNDS +
 *   timestamp all overlap the supplied @c TPCBox.
 * @param pa          Source pcpatch.
 * @param t           Instant timestamp.
 * @param box         Filtering @c TPCBox.
 * @param border_inc  Inclusive (@c true) or strict (@c false) overlap.
 * @return @c true if the instant should be kept under at-semantics.
 */
static bool
pcpatch_passes_tpcbox(const Pcpatch *pa, TimestampTz t, const TPCBox *box,
  bool border_inc)
{
  if (pa->pcid != box->pcid)
    return false;
  if (! contains_span_timestamptz(&box->period, t))
    return false;
  /* PCBOUNDS layout: {xmin, xmax, ymin, ymax} — see pc_api.h. */
  double pxmin = pa->bounds[0], pxmax = pa->bounds[1];
  double pymin = pa->bounds[2], pymax = pa->bounds[3];
  if (border_inc)
    return ! (pxmin > box->xmax || pxmax < box->xmin ||
              pymin > box->ymax || pymax < box->ymin);
  return ! (pxmin >= box->xmax || pxmax <= box->xmin ||
            pymin >= box->ymax || pymax <= box->ymin);
}

/**
 * @brief Patch-level @c at / @c minus restriction by @c TPCBox.
 *
 * Walks every instant of @p temp, collects the timestamps whose
 * patch passes the @c PCBOUNDS-overlap + timestamp-in-period
 * predicate, then defers to @c temporal_restrict_tstzset for the
 * actual restriction. Granularity is patch-level — surviving
 * instants keep their pcpatch payload verbatim.
 *
 * @param temp        Source tpcpatch.
 * @param box         Filtering @c TPCBox.
 * @param border_inc  Inclusive (@c true) or strict (@c false) overlap.
 * @param atfunc      @c REST_AT keeps passing instants; @c REST_MINUS
 *                    keeps the complement.
 * @return Newly allocated @c Temporal of the same subtype as @p temp,
 *   or @c NULL when every instant is dropped (or every instant
 *   survives in @c REST_MINUS).
 */
static Temporal *
tpcpatch_restrict_tpcbox(const Temporal *temp, const TPCBox *box,
  bool border_inc, bool atfunc)
{
  /* PCID mismatch on the first instant: short-circuit to the identity
   * branch without scanning the rest (all instants share pcid). */
  const Pcpatch *first =
    (const Pcpatch *) DatumGetPointer(temporal_start_value(temp));
  if (first->pcid != box->pcid)
    return atfunc ? NULL : temporal_copy(temp);

  /* Upper bound on the number of surviving timestamps. */
  int ninst = 0;
  switch (temp->subtype)
  {
    case TINSTANT:
      ninst = 1;
      break;
    case TSEQUENCE:
      ninst = ((const TSequence *) temp)->count;
      break;
    default: /* TSEQUENCESET */
    {
      const TSequenceSet *ss = (const TSequenceSet *) temp;
      for (int i = 0; i < ss->count; i++)
        ninst += TSEQUENCESET_SEQ_N(ss, i)->count;
    }
  }

  Datum *kept = palloc(sizeof(Datum) * ninst);
  int nkept = 0;

  if (temp->subtype == TINSTANT)
  {
    const TInstant *inst = (const TInstant *) temp;
    const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(tinstant_value_p(inst));
    if (pcpatch_passes_tpcbox(pa, inst->t, box, border_inc))
      kept[nkept++] = TimestampTzGetDatum(inst->t);
  }
  else if (temp->subtype == TSEQUENCE)
  {
    const TSequence *seq = (const TSequence *) temp;
    for (int i = 0; i < seq->count; i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(tinstant_value_p(inst));
      if (pcpatch_passes_tpcbox(pa, inst->t, box, border_inc))
        kept[nkept++] = TimestampTzGetDatum(inst->t);
    }
  }
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    for (int i = 0; i < ss->count; i++)
    {
      const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
      for (int j = 0; j < seq->count; j++)
      {
        const TInstant *inst = TSEQUENCE_INST_N(seq, j);
        const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(tinstant_value_p(inst));
        if (pcpatch_passes_tpcbox(pa, inst->t, box, border_inc))
          kept[nkept++] = TimestampTzGetDatum(inst->t);
      }
    }
  }

  /* Edge cases: no survivors / all survivors. */
  if (nkept == 0)
  {
    pfree(kept);
    return atfunc ? NULL : temporal_copy(temp);
  }
  if (nkept == ninst)
  {
    pfree(kept);
    return atfunc ? temporal_copy(temp) : NULL;
  }

  Set *tset = set_make(kept, nkept, T_TIMESTAMPTZ, ORDER);
  pfree(kept);
  Temporal *result = temporal_restrict_tstzset(temp, tset, atfunc);
  pfree(tset);
  return result;
}

/*****************************************************************************
 * Per-point restrict — filter every point of every patch through a
 * predicate, rebuild instants, repackage. Used by atTpcboxFine,
 * minusTpcboxFine, atGeometry, minusGeometry on tpcpatch.
 *****************************************************************************/

/**
 * @brief Filter one instant of a tpcpatch through @p pred.
 * @param inst             Source instant.
 * @param pred             Per-point predicate.
 * @param extra            Caller-supplied state for @p pred.
 * @param keep_when_true   @c true for @c at semantics, @c false for
 *                         @c minus.
 * @return Newly allocated @c TInstant whose value is the filtered
 *   pcpatch, or @c NULL when no points survive.
 */
static TInstant *
tpcpatch_inst_filter(const TInstant *inst, pcpatch_pointpred_fn pred,
  void *extra, bool keep_when_true)
{
  const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(tinstant_value_p(inst));
  Pcpatch *filtered = pcpatch_filter_per_point(pa, pred, extra, keep_when_true);
  if (! filtered)
    return NULL;
  TInstant *result = tinstant_make(PointerGetDatum(filtered), T_TPCPATCH,
    inst->t);
  pfree(filtered);
  return result;
}

/**
 * @brief Filter a @c TSequence's instants, emit one output TSequence
 *   per contiguous run of survivors.
 * @param seq              Source sequence.
 * @param pred             Per-point predicate.
 * @param extra            Caller-supplied state for @p pred.
 * @param keep_when_true   @c true for @c at semantics, @c false for
 *                         @c minus.
 * @param out_seqs         Output buffer (caller-owned, capacity at
 *                         least @c seq->count).
 * @return Number of @c TSequences appended to @p out_seqs.
 */
static int
tpcpatch_seq_filter(const TSequence *seq, pcpatch_pointpred_fn pred,
  void *extra, bool keep_when_true, TSequence **out_seqs)
{
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  TInstant **run = palloc(sizeof(TInstant *) * seq->count);
  int run_n = 0, n_emitted = 0;
  bool run_starts_at_seq_lower = true;

  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    TInstant *filt = tpcpatch_inst_filter(inst, pred, extra, keep_when_true);
    if (filt)
    {
      run[run_n++] = filt;
    }
    else if (run_n > 0)
    {
      bool low = run_starts_at_seq_lower ? seq->period.lower_inc : true;
      out_seqs[n_emitted++] = tsequence_make((TInstant **) run, run_n,
        low, true, interp, NORMALIZE);
      for (int k = 0; k < run_n; k++) pfree(run[k]);
      run_n = 0;
      run_starts_at_seq_lower = false;
    }
  }
  if (run_n > 0)
  {
    bool low = run_starts_at_seq_lower ? seq->period.lower_inc : true;
    out_seqs[n_emitted++] = tsequence_make((TInstant **) run, run_n,
      low, seq->period.upper_inc, interp, NORMALIZE);
    for (int k = 0; k < run_n; k++) pfree(run[k]);
  }
  pfree(run);
  return n_emitted;
}

/**
 * @brief Per-point @c at / @c minus restriction driver.
 *
 * Shared backend for atTpcboxFine, minusTpcboxFine, atGeometry, and
 * minusGeometry on tpcpatch. Switches on subtype, calls
 * @c tpcpatch_seq_filter where applicable, repackages survivors into
 * a result of the same subtype shape (or a TSequenceSet when a
 * TSequence input contains gaps).
 *
 * @param temp     Source tpcpatch.
 * @param pred     Per-point predicate.
 * @param extra    Caller-supplied state for @p pred.
 * @param atfunc   @c REST_AT keeps points where @p pred is true,
 *                 @c REST_MINUS keeps the complement.
 * @return Newly allocated @c Temporal, or @c NULL when every instant
 *   filters to empty.
 */
static Temporal *
tpcpatch_filter_temporal(const Temporal *temp, pcpatch_pointpred_fn pred,
  void *extra, bool atfunc)
{
  /* keep_when_true mirrors atfunc: at keeps points where pred is true,
   * minus keeps the complement. */
  bool keep = atfunc;

  if (temp->subtype == TINSTANT)
  {
    TInstant *result = tpcpatch_inst_filter((const TInstant *) temp, pred,
      extra, keep);
    return (Temporal *) result;
  }

  if (temp->subtype == TSEQUENCE)
  {
    const TSequence *seq = (const TSequence *) temp;
    TSequence **seqs = palloc(sizeof(TSequence *) * seq->count);
    int n = tpcpatch_seq_filter(seq, pred, extra, keep, seqs);
    if (n == 0)
    {
      pfree(seqs);
      return NULL;
    }
    Temporal *result;
    if (n == 1)
    {
      result = (Temporal *) seqs[0];
    }
    else
    {
      result = (Temporal *) tsequenceset_make(seqs, n, NORMALIZE);
      for (int i = 0; i < n; i++) pfree(seqs[i]);
    }
    pfree(seqs);
    return result;
  }

  /* TSEQUENCESET */
  const TSequenceSet *ss = (const TSequenceSet *) temp;
  /* Upper bound: at most one output TSequence per input instant. */
  int cap = 0;
  for (int i = 0; i < ss->count; i++)
    cap += TSEQUENCESET_SEQ_N(ss, i)->count;
  TSequence **seqs = palloc(sizeof(TSequence *) * cap);
  int n = 0;
  for (int i = 0; i < ss->count; i++)
    n += tpcpatch_seq_filter(TSEQUENCESET_SEQ_N(ss, i), pred, extra, keep,
      seqs + n);
  if (n == 0)
  {
    pfree(seqs);
    return NULL;
  }
  Temporal *result = (Temporal *) tsequenceset_make(seqs, n, NORMALIZE);
  for (int i = 0; i < n; i++) pfree(seqs[i]);
  pfree(seqs);
  return result;
}

/*****************************************************************************
 * Per-point restrict by TPCBox (atTpcboxFine / minusTpcboxFine)
 *****************************************************************************/

PGDLLEXPORT Datum Tpcpatch_at_tpcbox_fine(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_at_tpcbox_fine);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Restrict a tpcpatch to only the points inside the supplied
 *   TPCBox.
 * @details Per-point granularity: each surviving instant carries a
 *   freshly-built pcpatch containing only the points inside the box.
 *   Instants whose patches have zero points inside are dropped. The
 *   timestamp must also fall in the box's period.
 * @sqlfn atTpcboxFine()
 */
Datum
Tpcpatch_at_tpcbox_fine(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TPCBox *box = PG_GETARG_TPCBOX_P(1);
  bool border_inc = PG_GETARG_BOOL(2);
  PcpointInTpcboxArgs args = { .box = box, .border_inc = border_inc };
  Temporal *result = tpcpatch_filter_temporal(temp, pcpoint_in_tpcbox, &args,
    REST_AT);
  PG_FREE_IF_COPY(temp, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpcpatch_minus_tpcbox_fine(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_minus_tpcbox_fine);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Restrict a tpcpatch to the points outside the supplied TPCBox.
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
  PcpointInTpcboxArgs args = { .box = box, .border_inc = border_inc };
  Temporal *result = tpcpatch_filter_temporal(temp, pcpoint_in_tpcbox, &args,
    REST_MINUS);
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
 * @brief Restrict a tpcpatch to only the points whose XY projection
 *   intersects the supplied geometry.
 * @sqlfn atGeometry()
 */
Datum
Tpcpatch_at_geometry(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = tpcpatch_filter_temporal(temp, pcpoint_intersects_geometry,
    (void *) gs, REST_AT);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpcpatch_minus_geometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_minus_geometry);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Restrict a tpcpatch to only the points whose XY projection
 *   does NOT intersect the supplied geometry.
 * @sqlfn minusGeometry()
 */
Datum
Tpcpatch_minus_geometry(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = tpcpatch_filter_temporal(temp, pcpoint_intersects_geometry,
    (void *) gs, REST_MINUS);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Spatial relationships — eIntersects / aIntersects(tpcpatch, geometry)
 *****************************************************************************/

/**
 * @brief Test whether any instant of @p temp has at least one point
 *   matching @p pred.
 *
 * Walks instants in subtype order, short-circuits on the first hit —
 * never rebuilds a patch. Backs the @c eIntersects PG wrapper.
 *
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

PGDLLEXPORT Datum Tpcpatch_eIntersects_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_eIntersects_geo);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Return true iff at least one point in any of the tpcpatch's
 *   instants intersects the geometry.
 * @details Walks instants in order, short-circuiting on the first
 *   hit. No patch rebuild, no allocations beyond the temporary
 *   per-instant decompression.
 * @sqlfn eIntersects()
 */
Datum
Tpcpatch_eIntersects_geo(PG_FUNCTION_ARGS)
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
 * @brief Restrict a tpcpatch to the patches whose 2D PCBOUNDS overlap
 *   the box and whose timestamp falls in its period.
 * @details Patch-level granularity: each surviving instant keeps its
 *   pcpatch payload verbatim. No per-point decompression.
 * @sqlfn atTpcbox()
 */
Datum
Tpcpatch_at_tpcbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TPCBox *box = PG_GETARG_TPCBOX_P(1);
  bool border_inc = PG_GETARG_BOOL(2);
  Temporal *result = tpcpatch_restrict_tpcbox(temp, box, border_inc, REST_AT);
  PG_FREE_IF_COPY(temp, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpcpatch_minus_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcpatch_minus_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Remove from a tpcpatch the instants whose patch passes the
 *   PCBOUNDS-overlap + timestamp-in-period predicate.
 * @sqlfn minusTpcbox()
 */
Datum
Tpcpatch_minus_tpcbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TPCBox *box = PG_GETARG_TPCBOX_P(1);
  bool border_inc = PG_GETARG_BOOL(2);
  Temporal *result = tpcpatch_restrict_tpcbox(temp, box, border_inc, REST_MINUS);
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
 * @brief Build the materialized state backing the @c points(tpcpatch) SRF.
 *
 * Walks every instant of @p temp, decompresses each patch via
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
 * @brief Set-returning function emitting one (timestamptz, pcpoint) row
 *   per point per instant of a tpcpatch.
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
