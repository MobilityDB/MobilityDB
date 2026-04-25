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
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"
#include "temporal/tsequence.h"
#include "temporal/tsequenceset.h"
#include "pointcloud/pcpatch.h"
#include "pointcloud/tpcbox.h"

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

static bool
pcpatch_passes_tpcbox(const Pcpatch *pa, TimestampTz t, const TPCBox *box,
  bool border_inc)
{
  if (pa->pcid != box->pcid)
    return false;
  if (! contains_span_timestamptz(&box->period, t))
    return false;
  /* PCBOUNDS layout: bounds[0..3] = xmin, ymin, xmax, ymax. */
  double pxmin = pa->bounds[0], pymin = pa->bounds[1];
  double pxmax = pa->bounds[2], pymax = pa->bounds[3];
  if (border_inc)
    return ! (pxmin > box->xmax || pxmax < box->xmin ||
              pymin > box->ymax || pymax < box->ymin);
  return ! (pxmin >= box->xmax || pxmax <= box->xmin ||
            pymin >= box->ymax || pymax <= box->ymin);
}

/*
 * Walk every instant of @p temp, collect the timestamps where the
 * patch passes the predicate, then defer to temporal_restrict_tstzset
 * for the actual at/minus restriction.
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

/*****************************************************************************/
