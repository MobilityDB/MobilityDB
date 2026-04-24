/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief TPCBox bounding-box helpers for temporal pointcloud types
 *   (tpcpoint, tpcpatch). Mirrors the tspatial_boxops.c implementation
 *   for the STBox case but reads its spatial dimensions differently:
 *   pcpoint via the schema-aware libpc.a dimension readers, pcpatch
 *   via the embedded @c PCBOUNDS (2D only — per-point Z is inside the
 *   compressed data block and cannot be summarised without decoding).
 */

#include "pointcloud/tpc_boxops.h"

#include <assert.h>
#include <float.h>
#include <string.h>
#include <postgres.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
#include <pc_api.h>

#include <meos.h>
#include <meos_internal.h>
#include "temporal/span.h"
#include "temporal/temporal.h"
#include "pointcloud/meos_schema_hook.h"
#include "pointcloud/pcpoint.h"
#include "pointcloud/pcpatch.h"

/*****************************************************************************
 * Single-instant → TPCBox
 *****************************************************************************/

/**
 * @brief Populate the spatial bounds of @p box from a pcpoint, using
 *   the schema cache to read X / Y / Z dimensions.
 */
static void
pcpoint_fill_tpcbox_spatial(const Pcpoint *pt, TPCBox *box)
{
  PCSCHEMA *schema = meos_pc_schema(pt->pcid);
  PCPOINT pcpt;
  pcpt.readonly = 1;
  pcpt.schema = schema;
  pcpt.data = (uint8_t *) pt->data;

  double x = 0.0, y = 0.0, z = 0.0;
  bool hasz = (schema->zdim != NULL);
  if (schema->xdim) pc_point_get_x(&pcpt, &x);
  if (schema->ydim) pc_point_get_y(&pcpt, &y);
  if (hasz) pc_point_get_z(&pcpt, &z);

  box->xmin = box->xmax = x;
  box->ymin = box->ymax = y;
  if (hasz) { box->zmin = box->zmax = z; }
  box->srid = (int32_t) schema->srid;
  box->pcid = pt->pcid;
  MEOS_FLAGS_SET_X(box->flags, true);
  MEOS_FLAGS_SET_Z(box->flags, hasz);
}

/**
 * @brief Populate the spatial bounds of @p box from a pcpatch, using
 *   the embedded PCBOUNDS (2D). Z is not populated — the per-point Z
 *   values are inside the compressed @c data block and cannot be
 *   summarised at the bbox layer.
 */
static void
pcpatch_fill_tpcbox_spatial(const Pcpatch *pa, TPCBox *box)
{
  PCSCHEMA *schema = meos_pc_schema(pa->pcid);
  box->xmin = pa->bounds[0];
  box->ymin = pa->bounds[1];
  box->xmax = pa->bounds[2];
  box->ymax = pa->bounds[3];
  box->srid = (int32_t) schema->srid;
  box->pcid = pa->pcid;
  MEOS_FLAGS_SET_X(box->flags, true);
  MEOS_FLAGS_SET_Z(box->flags, false);
}

void
tpointcloudinst_set_tpcbox(const TInstant *inst, TPCBox *box)
{
  assert(inst); assert(box);
  assert(inst->temptype == T_TPCPOINT || inst->temptype == T_TPCPATCH);
  memset(box, 0, sizeof(TPCBox));

  if (inst->temptype == T_TPCPOINT)
  {
    const Pcpoint *pt =
      (const Pcpoint *) DatumGetPointer(tinstant_value_p(inst));
    pcpoint_fill_tpcbox_spatial(pt, box);
  }
  else  /* T_TPCPATCH */
  {
    const Pcpatch *pa =
      (const Pcpatch *) DatumGetPointer(tinstant_value_p(inst));
    pcpatch_fill_tpcbox_spatial(pa, box);
  }

  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
}

/*****************************************************************************
 * Array of instants → TPCBox (for TSequence construction)
 *****************************************************************************/

void
tpointcloudinstarr_set_tpcbox(TInstant **instants, int count, bool lower_inc,
  bool upper_inc, interpType interp, TPCBox *box)
{
  (void) interp;  /* Step vs linear does not affect the TPCBox union */
  assert(instants); assert(count > 0); assert(box);
  assert(instants[0]->temptype == T_TPCPOINT ||
         instants[0]->temptype == T_TPCPATCH);
  tpointcloudinst_set_tpcbox(instants[0], box);
  for (int i = 1; i < count; i++)
  {
    TPCBox tmp;
    tpointcloudinst_set_tpcbox(instants[i], &tmp);
    tpcbox_expand(&tmp, box);
  }
  /* Override the period bounds with the caller-provided inclusivity. */
  span_set(TimestampTzGetDatum(instants[0]->t),
    TimestampTzGetDatum(instants[count - 1]->t),
    lower_inc, upper_inc, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
}

/*****************************************************************************
 * Incremental bbox growth when appending an instant to a sequence
 *****************************************************************************/

void
tpointcloudseq_expand_tpcbox(TSequence *seq, const TInstant *inst)
{
  assert(seq); assert(inst);
  TPCBox tmp;
  tpointcloudinst_set_tpcbox(inst, &tmp);
  tpcbox_expand(&tmp, (TPCBox *) TSEQUENCE_BBOX_PTR(seq));
  /* Extend the period's upper bound to the new instant's timestamp. */
  TPCBox *seq_box = (TPCBox *) TSEQUENCE_BBOX_PTR(seq);
  seq_box->period.upper = TimestampTzGetDatum(inst->t);
  seq_box->period.upper_inc = true;
}

/*****************************************************************************
 * Array of sequences → TPCBox (for TSequenceSet construction)
 *****************************************************************************/

void
tpointcloudseqarr_set_tpcbox(TSequence **sequences, int count, TPCBox *box)
{
  assert(sequences); assert(count > 0); assert(box);
  memcpy(box, TSEQUENCE_BBOX_PTR(sequences[0]), sizeof(TPCBox));
  for (int i = 1; i < count; i++)
  {
    TPCBox *box1 = (TPCBox *) TSEQUENCE_BBOX_PTR(sequences[i]);
    tpcbox_expand(box1, box);
  }
}

/*****************************************************************************/
