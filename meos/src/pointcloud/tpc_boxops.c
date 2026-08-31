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
 * @brief TPCBox bounding-box helpers for temporal pointcloud types
 *   (tpcpoint, tpcpatch). Mirrors the tspatial_boxops.c implementation
 *   for the STBox case but reads its spatial dimensions differently:
 *   pcpoint via the schema-aware libpc.a dimension readers, pcpatch
 *   via the embedded @c PCBOUNDS (2D only — per-point Z is inside the
 *   compressed data block and cannot be summarised without decoding).
 */

#include "pointcloud/tpc_boxops.h"

/* C */
#include <assert.h>
#include <float.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <varatt.h>
/* pgPointCloud */
#include <pc_api.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>           /* nad_stbox_stbox */
#include <meos_internal.h>
#include "temporal/span.h"
#include "temporal/temporal.h"
#include "temporal/meos_catalog.h"  /* T_TPCPOINT */
#include <utils/timestamp.h>   /* TimestampTzGetDatum */
#include "pointcloud/meos_schema_hook.h"
#include "pointcloud/pcpoint.h"
#include "pointcloud/pcpatch.h"
#include "pointcloud/tpcbox.h"
#include "geo/geo_funcs.h"    /* ensure_same_dimensionality */

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
  assert(pt); assert(box);
  PCSCHEMA *schema = meos_pc_schema(pt->pcid);
  /* Which dimensions the point holds, and in what reference system, is what
   * the schema states, so an unresolved one leaves the box with no spatial
   * extent rather than one read through a schema that is not there. The
   * handler a binding installs returns from an error instead of ending the
   * process, and what it returns to is this reader */
  if (! schema)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "No schema registered for pcid %u", pt->pcid);
    return;
  }
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
 * @brief Populate the spatial bounds of @p box from a pcpatch
 * @details The spatial extent of a patch has one reader, @ref
 *   pcpatch_to_tpcbox, so the box a temporal patch carries states the same
 *   dimensions as the box the patch converts to, Z included where its schema
 *   holds one. The period @p box keeps is the caller's, which states it after
 *   this fills the spatial part.
 */
static void
pcpatch_fill_tpcbox_spatial(const Pcpatch *pa, TPCBox *box)
{
  assert(pa); assert(box);
  PCSCHEMA *schema = meos_pc_schema(pa->pcid);
  /* The extent is in the patch's own header, but the reference system it is
   * expressed in is the schema's, so an unresolved schema leaves the box
   * without the spatial part rather than reading one through it */
  if (! schema)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "No schema registered for pcid %u", pa->pcid);
    return;
  }
  TPCBox *spatial = pcpatch_to_tpcbox(pa, (int32_t) schema->srid);
  if (! spatial)
    return;
  Span period = box->period;
  memcpy(box, spatial, sizeof(TPCBox));
  box->period = period;
  pfree(spatial);
}

/**
 * @brief Set a temporal pointcloud bounding box from a timestamptz span
 * @param[in] s Timestamptz span
 * @param[out] box Bounding box carrying the span and no spatial dimension
 * @note Mirrors @p tstzspan_set_stbox. The span names no schema, so @p pcid
 *   stays 0. A box carrying no coordinates has nothing for a schema to give a
 *   meaning to, so @p ensure_valid_tpcbox_tpcbox leaves its schema unread,
 *   which is what lets a time-only query meet an indexed box.
 */
void
tstzspan_set_tpcbox(const Span *s, TPCBox *box)
{
  assert(s); assert(box); assert(s->spantype == T_TSTZSPAN);
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(TPCBox));
  memcpy(&box->period, s, sizeof(Span));
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @brief Set the bounding box of a temporal pointcloud instant
 * @param[in] inst Temporal instant of type @p T_TPCPOINT or @p T_TPCPATCH
 * @param[out] box Bounding box
 */
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

/**
 * @brief Set the bounding box from an array of temporal pointcloud instants
 * @param[in] instants Array of temporal instants
 * @param[in] count Number of instants
 * @param[in] lower_inc,upper_inc Period bound inclusivity
 * @param[in] interp Interpolation (unused — does not affect the union)
 * @param[out] box Bounding box
 */
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

/**
 * @brief Expand the bounding box of a temporal pointcloud sequence with a
 *   newly-appended instant
 * @param[in,out] seq Temporal sequence whose stored bbox is grown in place
 * @param[in] inst Instant being appended
 */
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

/**
 * @brief Set the bounding box from an array of temporal pointcloud sequences
 * @param[in] sequences Array of sequences whose bboxes are unioned
 * @param[in] count Number of sequences
 * @param[out] box Bounding box
 */
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

/*****************************************************************************
 * Extent aggregation
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_box_constructor
 * @brief Transition function for the extent aggregate over tpcpoint /
 *   tpcpatch values. Folds @p temp's bounding box into @p state.
 * @return @p state (mutated) when both inputs are non-NULL and comparable;
 *   a freshly-palloc'd TPCBox when @p state is NULL and @p temp is non-NULL;
 *   @p NULL when both are NULL, or when the two boxes name different schemas
 *   or hold different dimensions (which raises an error).
 * @csqlfn #Tpc_extent_transfn()
 */
TPCBox *
tpcbox_extent_transfn(TPCBox *state, const Temporal *temp)
{
  if (! state && ! temp)
    return NULL;
  if (! temp)
    return state;
  if (! state)
  {
    TPCBox *result = palloc0(sizeof(TPCBox));
    temporal_set_bbox(temp, result);
    return result;
  }
  TPCBox tmp;
  temporal_set_bbox(temp, &tmp);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(state, &tmp) ||
      ! ensure_same_dimensionality(state->flags, tmp.flags))
    return NULL;
  tpcbox_expand(&tmp, state);
  return state;
}

/*****************************************************************************
 * Generic bbox dispatchers
 *
 * Mirror boxop_tspatial_{stbox,tspatial} from tgeo_boxops.c.  Each
 * computes the TPCBox of its temporal arg(s) and applies the supplied
 * tpcbox-vs-tpcbox predicate (overlaps / contains / contained / same /
 * adjacent — i.e. the MEOS primitives in meos_pointcloud.h).
 *****************************************************************************/

/**
 * @brief Generic bbox dispatcher between a temporal pointcloud value and a
 *   TPCBox
 * @param[in] temp Temporal pointcloud value
 * @param[in] box Bounding box
 * @param[in] func TPCBox-vs-TPCBox predicate to apply
 * @param[in] inverted Swap argument order when @p true
 */
bool
boxop_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box,
  bool (*func)(const TPCBox *, const TPCBox *), bool inverted)
{
  TPCBox box1;
  temporal_set_bbox(temp, &box1);
  return inverted ? func(box, &box1) : func(&box1, box);
}

/**
 * @brief Generic bbox dispatcher between two temporal pointcloud values
 * @param[in] temp1,temp2 Temporal pointcloud values
 * @param[in] func TPCBox-vs-TPCBox predicate to apply
 */
bool
boxop_tpointcloud_tpointcloud(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const TPCBox *, const TPCBox *))
{
  TPCBox box1, box2;
  temporal_set_bbox(temp1, &box1);
  temporal_set_bbox(temp2, &box2);
  return func(&box1, &box2);
}

/*****************************************************************************
 * Lossy TPCBox → STBox conversion
 *
 * Used by the SP-GiST opclasses where the index storage is STBox.
 * Drops pcid; the operator's recheck restores pcid filtering on the
 * actual leaf entry. Period, srid and the X/Z/T dimension flags are
 * copied; GEODETIC is always cleared (tpointcloud values are
 * cartesian).
 *****************************************************************************/

/**
 * @brief Lossy conversion from a TPCBox to an STBox
 * @details Copies period, srid, X/Z/T flags and spatial bounds; drops the
 *   pcid (the operator's recheck restores pcid filtering on the leaf
 *   entry) and clears the GEODETIC flag since tpointcloud values are
 *   cartesian.
 */
void
tpcbox_set_stbox(const TPCBox *src, STBox *dst)
{
  assert(src); assert(dst);
  memset(dst, 0, sizeof(STBox));
  dst->period = src->period;
  dst->xmin = src->xmin; dst->ymin = src->ymin; dst->zmin = src->zmin;
  dst->xmax = src->xmax; dst->ymax = src->ymax; dst->zmax = src->zmax;
  dst->srid = src->srid;
  MEOS_FLAGS_SET_X(dst->flags, MEOS_FLAGS_GET_X(src->flags));
  MEOS_FLAGS_SET_Z(dst->flags, MEOS_FLAGS_GET_Z(src->flags));
  MEOS_FLAGS_SET_T(dst->flags, MEOS_FLAGS_GET_T(src->flags));
  MEOS_FLAGS_SET_GEODETIC(dst->flags, false);
}

/*****************************************************************************
 * Nearest-approach distance
 *
 * Reuses the stbox NAD machinery via the lossy tpcbox→stbox conversion.
 * Boxes of different schemas are rejected by the validity check, since the
 * schemas would have to be compatible for the dimensions to mean the same
 * thing, exactly as mixed SRIDs are rejected for the spatial types.
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_box_dist
 * @brief Return the nearest-approach distance between two TPCBox values
 * @param[in] box1,box2 Bounding boxes
 * @return On error or if the time frames do not intersect return infinity
 * @csqlfn #NAD_tpcbox_tpcbox()
 */
double
nad_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2))
    return DBL_MAX;
  STBox sbox1, sbox2;
  tpcbox_set_stbox(box1, &sbox1);
  tpcbox_set_stbox(box2, &sbox2);
  return nad_stbox_stbox(&sbox1, &sbox2);
}

/**
 * @ingroup meos_pointcloud_box_dist
 * @brief Return the nearest-approach distance between a temporal pointcloud
 *   value and a TPCBox
 * @param[in] temp Temporal pointcloud value
 * @param[in] box Bounding box
 * @return On error or if the time frames do not intersect return infinity
 * @csqlfn #NAD_tpointcloud_tpcbox() #NAD_tpcbox_tpointcloud()
 */
double
nad_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOINTCLOUD(temp, DBL_MAX); VALIDATE_TPCBOX(box, DBL_MAX);
  TPCBox tmp;
  temporal_set_bbox(temp, &tmp);
  return nad_tpcbox_tpcbox(&tmp, box);
}

/**
 * @ingroup meos_pointcloud_box_dist
 * @brief Return the nearest-approach distance between two temporal
 *   pointcloud values
 * @param[in] temp1,temp2 Temporal pointcloud values
 * @return On error or if the time frames do not intersect return infinity
 * @csqlfn #NAD_tpointcloud_tpointcloud()
 */
double
nad_tpointcloud_tpointcloud(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOINTCLOUD(temp1, DBL_MAX); VALIDATE_TPOINTCLOUD(temp2, DBL_MAX);

  /* Patches are not point trajectories: they have no single-point
   * projection to a tgeompoint, so their nearest-approach distance stays
   * the distance between their bounding boxes, exactly as before. */
  if (temp1->temptype != T_TPCPOINT || temp2->temptype != T_TPCPOINT)
  {
    TPCBox tmp1, tmp2;
    temporal_set_bbox(temp1, &tmp1);
    temporal_set_bbox(temp2, &tmp2);
    return nad_tpcbox_tpcbox(&tmp1, &tmp2);
  }

  /* Values of different schemas live in unrelated coordinate systems and are
   * rejected, exactly as the TPCBox path above rejects them */
  const Pcpoint *first1 =
    (const Pcpoint *) DatumGetPointer(temporal_start_value(temp1));
  const Pcpoint *first2 =
    (const Pcpoint *) DatumGetPointer(temporal_start_value(temp2));
  if (! ensure_same_pcid_pcpoint(first1, first2))
    return DBL_MAX;

  /* Project both values onto their point trajectories and return the
   * minimum of their synchronized distance, as the sibling spatiotemporal
   * types do. This is finer than the bounding-box distance computed by
   * nad_tpcbox_tpcbox: it can only be equal to or larger. */
  Temporal *proj1 = tpointcloud_to_tgeompoint(temp1);
  Temporal *proj2 = tpointcloud_to_tgeompoint(temp2);
  if (! proj1 || ! proj2)
  {
    if (proj1) pfree(proj1);
    if (proj2) pfree(proj2);
    return DBL_MAX;
  }
  double result = nad_tgeo_tgeo(proj1, proj2);
  pfree(proj1); pfree(proj2);
  return result;
}

/*****************************************************************************/
