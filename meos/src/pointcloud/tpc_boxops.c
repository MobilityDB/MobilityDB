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
 * @brief TPCBox bounding-box helpers for temporal pointcloud types (tpcpoint,
 * tpcpatch)
 * @details Mirrors the tspatial_boxops.c implementation
 * for the STBox case but reads its spatial dimensions differently:
 * pcpoint via the schema-aware libpc.a dimension readers, pcpatch
 * via the embedded @c PCBOUNDS (2D only — per-point Z is inside the
 * compressed data block and cannot be summarised without decoding).
 */

#include "pointcloud/tpc_boxops.h"

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
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
#include <meos_pointcloud.h>
#include "temporal/span.h"
#include "temporal/temporal.h"
#include "temporal/temporal_boxops.h"  /* tsequence_set_bbox_slice */
#include "temporal/bbox_index.h"  /* bbox_merge_n */
#include "geo/stbox_index.h"  /* stbox_adjust, stbox_size */
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
 * @brief Populate the spatial bounds of @p box from a pcpoint, using the schema
 * cache to read X / Y / Z dimensions
 */
static void
pcpoint_fill_tpcbox_spatial(const Pcpoint *pt, TPCBox *box)
{
  assert(pt); assert(box);
  const PCSCHEMA *schema = meos_pc_schema(pt->pcid);
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
  const PCSCHEMA *schema = meos_pc_schema(pa->pcid);
  /* The extent is in the patch's own header, but the reference system it is
   * expressed in is the schema's, so an unresolved schema leaves the box
   * without the spatial part rather than reading one through it */
  if (! schema)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "No schema registered for pcid %u", pa->pcid);
    return;
  }
  TPCBox *spatial = pcpatch_srid_to_tpcbox(pa, (int32_t) schema->srid);
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
    const TPCBox *box1 = (const TPCBox *) TSEQUENCE_BBOX_PTR(sequences[i]);
    tpcbox_expand(box1, box);
  }
}

/*****************************************************************************
 * Extent aggregation
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_agg
 * @brief Transition function for temporal extent aggregate of temporal point
 * clouds
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal point cloud to aggregate @nullable
 * @csqlfn #Tpc_extent_transfn()
 * @csqlaggfn #extentTransition()
 */
TPCBox *
tpc_extent_transfn(TPCBox *state, const Temporal *temp)
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

/**
 * @ingroup meos_pointcloud_agg
 * @brief Transition function for temporal extent aggregate of temporal point
 * cloud boxes
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] box Temporal point cloud box to aggregate @nullable
 * @note The function is also the combine function of the extent aggregates
 * of temporal point cloud boxes and temporal point clouds, the box to
 * aggregate being the state of another partial aggregation
 * @csqlfn #Tpcbox_extent_transfn(), #Tpcbox_extent_combinefn()
 * @csqlaggfn #extentTransition()
 */
TPCBox *
tpcbox_extent_transfn(TPCBox *state, const TPCBox *box)
{
  /* Can't do anything with null inputs */
  if (! state && ! box)
    return NULL;
  /* Null state and non-null box, return a copy of the box */
  if (! state)
    return tpcbox_copy(box);
  /* Non-null state and null box, return the state */
  if (! box)
    return state;

  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(state, box) ||
      ! ensure_same_dimensionality(state->flags, box->flags))
    return NULL;

  /* Both state and box are not null */
  tpcbox_expand(box, state);
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
 * @brief Ensure the validity of a temporal point cloud value and a point
 * cloud box: the box of the value and the box share their schema, their
 * reference system and their geodetic flag, as #ensure_valid_tpcbox_tpcbox
 * states it for two boxes
 * @param[in] temp Temporal point cloud value
 * @param[in] box Point cloud box
 */
bool
ensure_valid_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  VALIDATE_TPOINTCLOUD(temp, false); VALIDATE_TPCBOX(box, false);
  TPCBox box1;
  temporal_set_bbox(temp, &box1);
  return ensure_valid_tpcbox_tpcbox(&box1, box);
}

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
  /* Ensure the validity of the arguments */
  VALIDATE_TPOINTCLOUD(temp, false);
  VALIDATE_NOT_NULL(func, false);

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
  /* Ensure the validity of the arguments */
  VALIDATE_TPOINTCLOUD(temp1, false); VALIDATE_TPOINTCLOUD(temp2, false);
  VALIDATE_NOT_NULL(func, false);

  TPCBox box1, box2;
  temporal_set_bbox(temp1, &box1);
  temporal_set_bbox(temp2, &box2);
  return func(&box1, &box2);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if a point cloud box is adjacent to the point cloud box of a
 * temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Adjacent_tpcbox_tpointcloud()
 */
bool
adjacent_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &adjacent_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud is adjacent to
 * a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Adjacent_tpointcloud_tpcbox()
 */
bool
adjacent_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &adjacent_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud is adjacent to
 * the point cloud box of another one
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Adjacent_tpointcloud_tpointcloud()
 */
bool
adjacent_tpointcloud_tpointcloud(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &adjacent_tpcbox_tpcbox);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if a point cloud box is equal in the common dimensions to the point cloud box of a
 * temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Same_tpcbox_tpointcloud()
 */
bool
same_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &same_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud is equal in the common dimensions to
 * a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Same_tpointcloud_tpcbox()
 */
bool
same_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &same_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud is equal in the common dimensions to
 * the point cloud box of another one
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Same_tpointcloud_tpointcloud()
 */
bool
same_tpointcloud_tpointcloud(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &same_tpcbox_tpcbox);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if a point cloud box is contained in the point cloud box of a
 * temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Contained_tpcbox_tpointcloud()
 */
bool
contained_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &contained_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud is contained in
 * a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Contained_tpointcloud_tpcbox()
 */
bool
contained_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &contained_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud is contained in
 * the point cloud box of another one
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Contained_tpointcloud_tpointcloud()
 */
bool
contained_tpointcloud_tpointcloud(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &contained_tpcbox_tpcbox);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if a point cloud box contains the point cloud box of a
 * temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Contains_tpcbox_tpointcloud()
 */
bool
contains_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &contains_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud contains
 * a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Contains_tpointcloud_tpcbox()
 */
bool
contains_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &contains_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud contains
 * the point cloud box of another one
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Contains_tpointcloud_tpointcloud()
 */
bool
contains_tpointcloud_tpointcloud(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &contains_tpcbox_tpcbox);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if a point cloud box overlaps the point cloud box of a
 * temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Overlaps_tpcbox_tpointcloud()
 */
bool
overlaps_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overlaps_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud overlaps
 * a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Overlaps_tpointcloud_tpcbox()
 */
bool
overlaps_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overlaps_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud overlaps
 * the point cloud box of another one
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Overlaps_tpointcloud_tpointcloud()
 */
bool
overlaps_tpointcloud_tpointcloud(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &overlaps_tpcbox_tpcbox);
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
 * @note A nearest approach is measured over the time the operands share, so
 * operands whose time frames do not intersect have none and the answer is
 * DBL_MAX
 * @errval -1.0
 * @csqlfn #NAD_tpcbox_tpcbox()
 */
double
nad_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpcbox_tpcbox(box1, box2))
    return -1.0;
  STBox sbox1, sbox2;
  tpcbox_set_stbox(box1, &sbox1);
  tpcbox_set_stbox(box2, &sbox2);
  return nad_stbox_stbox(&sbox1, &sbox2, true);
}

/**
 * @ingroup meos_pointcloud_box_dist
 * @brief Return the nearest-approach distance between a temporal pointcloud
 *   value and a TPCBox
 * @param[in] temp Temporal pointcloud value
 * @param[in] box Bounding box
 * @note A nearest approach is measured over the time the operands share, so
 * operands whose time frames do not intersect have none and the answer is
 * DBL_MAX
 * @errval -1.0
 * @csqlfn #NAD_tpointcloud_tpcbox() #NAD_tpcbox_tpointcloud()
 */
double
nad_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOINTCLOUD(temp, -1.0); VALIDATE_TPCBOX(box, -1.0);
  TPCBox tmp;
  temporal_set_bbox(temp, &tmp);
  return nad_tpcbox_tpcbox(&tmp, box);
}

/**
 * @ingroup meos_pointcloud_box_dist
 * @brief Return the nearest-approach distance between two temporal
 *   pointcloud values
 * @param[in] temp1,temp2 Temporal pointcloud values
 * @note A nearest approach is measured over the time the operands share, so
 * operands whose time frames do not intersect have none and the answer is
 * DBL_MAX
 * @errval -1.0
 * @csqlfn #NAD_tpointcloud_tpointcloud()
 */
double
nad_tpointcloud_tpointcloud(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOINTCLOUD(temp1, -1.0); VALIDATE_TPOINTCLOUD(temp2, -1.0);

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
  const Pcpoint *first1 = (const Pcpoint *) DatumGetPointer(
    tinstant_value_p(temporal_start_inst(temp1)));
  const Pcpoint *first2 = (const Pcpoint *) DatumGetPointer(
    tinstant_value_p(temporal_start_inst(temp2)));
  if (! ensure_same_pcid_pcpoint(first1, first2))
    return -1.0;

  /* Project both values onto their point trajectories and return the
   * minimum of their synchronized distance, as the sibling spatiotemporal
   * types do. This is finer than the bounding-box distance computed by
   * nad_tpcbox_tpcbox: it can only be equal to or larger. */
  Temporal *proj1 = tpcpoint_to_tgeompoint(temp1);
  Temporal *proj2 = tpcpoint_to_tgeompoint(temp2);
  if (! proj1 || ! proj2)
  {
    if (proj1) pfree(proj1);
    if (proj2) pfree(proj2);
    return -1.0;
  }
  double result = nad_tgeo_tgeo(proj1, proj2, true);
  pfree(proj1); pfree(proj2);
  return result;
}

/* GENERATED-BOXSPLIT-BEGIN tpointcloud — tools/codegen/inherited/generate.py from templates/boxsplit.c.tmpl; DO NOT EDIT BY HAND;
 * edit the template + manifest.d/boxsplit.yaml and re-run. */
/**
 * @ingroup meos_internal_pointcloud_bbox
 * @brief Return a singleton array of point cloud boxes from a
 * temporal point cloud value instant
 * @param[in] inst Temporal instant
 */
TPCBox *
tpointcloudinst_tpcboxes(const TInstant *inst)
{
  assert(inst); assert(tpointcloud_temptype(inst->temptype));
  /* One bounding box per instant */
  TPCBox *result = palloc(sizeof(TPCBox));
  tpointcloudinst_set_tpcbox(inst, &result[0]);
  return result;
}

/**
 * @brief Return an array of point cloud boxes from the instants
 * of a temporal point cloud value sequence with discrete interpolation
 * @param[in] seq Temporal sequence
 */
static TPCBox *
tpointcloudseq_disc_tpcboxes(const TSequence *seq)
{
  assert(seq); assert(tpointcloud_temptype(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  /* One bounding box per instant */
  TPCBox *result = palloc(sizeof(TPCBox) * seq->count);
  for (int i = 0; i < seq->count; i++)
    tpointcloudinst_set_tpcbox(TSEQUENCE_INST_N(seq, i), &result[i]);
  return result;
}

/**
 * @brief Return an array of point cloud boxes from the segments of a
 * temporal point cloud value sequence with continuous interpolation (iterator
 * function)
 * @param[in] seq Temporal sequence
 * @param[out] result Array of boxes
 * @return Number of elements in the ouput array
 */
static int
tpointcloudseq_cont_tpcboxes_iter(const TSequence *seq, TPCBox *result)
{
  assert(seq); assert(result); assert(tpointcloud_temptype(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tsequence_set_bbox_slice(seq, 0, 0, &result[0]);
    return 1;
  }

  /* One bounding box per segment */
  for (int i = 0; i < seq->count - 1; i++)
    tsequence_set_bbox_slice(seq, i, i + 1, &result[i]);
  return seq->count - 1;
}

/**
 * @ingroup meos_internal_pointcloud_bbox
 * @brief Return an array of point cloud boxes from the instants or segments
 * of a temporal point cloud value sequence
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] seq Temporal sequence
 * @param[out] count Number of elements in the output array
 */
TPCBox *
tpointcloudseq_tpcboxes(const TSequence *seq, int *count)
{
  assert(seq); assert(count); assert(tpointcloud_temptype(seq->temptype));

  /* Discrete case */
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
  {
    *count = seq->count;
    return tpointcloudseq_disc_tpcboxes(seq);
  }

  /* Continuous case */
  int nboxes = (seq->count == 1) ? 1 : seq->count - 1;
  TPCBox *result = palloc(sizeof(TPCBox) * nboxes);
  *count = tpointcloudseq_cont_tpcboxes_iter(seq, result);
  return result;
}

/**
 * @ingroup meos_internal_pointcloud_bbox
 * @brief Return an array of point cloud boxes from the segments of a
 * temporal point cloud value sequence set
 * @param[in] ss Temporal sequence set
 * @param[out] count Number of elements in the output array
 */
TPCBox *
tpointcloudseqset_tpcboxes(const TSequenceSet *ss, int *count)
{
  assert(ss); assert(count); assert(tpointcloud_temptype(ss->temptype));

  /* One bounding box per segment */
  TPCBox *result = palloc(sizeof(TPCBox) * ss->totalcount);
  int nboxes = 0;
  for (int i = 0; i < ss->count; i++)
    nboxes += tpointcloudseq_cont_tpcboxes_iter(TSEQUENCESET_SEQ_N(ss, i),
      &result[nboxes]);
  assert(nboxes <= ss->totalcount);
  *count = nboxes;
  return result;
}

/**
 * @ingroup meos_internal_pointcloud_bbox
 * @brief Return an array of point cloud boxes from the instants or segments
 * of a temporal point cloud value, its arguments valid
 * @details The internal twin of #tpointcloud_tpcboxes, which an index calls
 * @param[in] temp Temporal point cloud value
 * @param[out] count Number of values of the output array
 */
TPCBox *
tpointcloud_tpcboxes_intl(const Temporal *temp, int *count)
{
  assert(temp); assert(count); assert(tpointcloud_temptype(temp->temptype));
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tpointcloudinst_tpcboxes((TInstant *) temp);
    case TSEQUENCE:
      return tpointcloudseq_tpcboxes((TSequence *) temp, count);
    default: /* TSEQUENCESET */
      return tpointcloudseqset_tpcboxes((TSequenceSet *) temp, count);
  }
}

/**
 * @ingroup meos_pointcloud_bbox_split
 * @brief Return an array of point cloud boxes from the instants or segments
 * of a temporal point cloud value
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] temp Temporal point cloud value
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Tpointcloud_tpcboxes()
 */
TPCBox *
tpointcloud_tpcboxes(const Temporal *temp, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TPOINTCLOUD(temp, NULL);
  return tpointcloud_tpcboxes_intl(temp, count);
}

/*****************************************************************************/

/**
 * @brief Return an array of N point cloud boxes from the instants
 * of a temporal point cloud value sequence with discrete interpolation
 * @param[in] seq Temporal sequence
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 */
static TPCBox *
tpointcloudseq_disc_split_n_tpcboxes(const TSequence *seq, int box_count,
  int *count)
{
  assert(seq); assert(count); assert(tpointcloud_temptype(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  assert(box_count > 0);

  /* One bounding box per instant */
  if (seq->count <= box_count)
  {
    *count = seq->count;
    return tpointcloudseq_disc_tpcboxes(seq);
  }

  /* One bounding box per several consecutive instants */
  TPCBox *result = palloc(sizeof(TPCBox) * box_count);
  /* Minimum number of input instants merged together in an output box */
  int size = seq->count / box_count;
  /* Number of output boxes that result from merging (size + 1) instants */
  int remainder = seq->count % box_count;
  int i = 0; /* Loop variable for input instants */
  for (int k = 0; k < box_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    tpointcloudinst_set_tpcbox(TSEQUENCE_INST_N(seq, i), &result[k]);
    for (int l = i + 1; l < j; l++)
    {
      TPCBox box;
      tpointcloudinst_set_tpcbox(TSEQUENCE_INST_N(seq, l), &box);
      tpcbox_expand(&box, &result[k]);
    }
    i = j;
  }
  assert(i == seq->count);
  *count = box_count;
  return result;
}

/**
 * @brief Return an array of N point cloud boxes from the segments of a 
 * temporal point cloud value sequence with continuous interpolation (iterator
 * function)
 * @param[in] seq Temporal point cloud value
 * @param[in] box_count Number of boxes
 * @param[out] result Array of boxes
 */
static int
tpointcloudseq_cont_split_n_tpcboxes_iter(const TSequence *seq, int box_count,
  TPCBox *result)
{
  assert(seq); assert(result); assert(tpointcloud_temptype(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);
  assert(box_count > 0);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tsequence_set_bbox_slice(seq, 0, 0, &result[0]);
    return 1;
  }

  /* One bounding box per segment */
  int nsegs = seq->count - 1;
  if (nsegs <= box_count)
    return tpointcloudseq_cont_tpcboxes_iter(seq, result);

  /* One bounding box per several consecutive segments */
  /* Minimum number of input segments merged together in an output box */
  int size = nsegs / box_count;
  /* Number of output boxes that result from merging (size + 1) segments */
  int remainder = nsegs % box_count;
  int i = 0; /* Loop variable for input segments */
  for (int k = 0; k < box_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    tsequence_set_bbox_slice(seq, i, j, &result[k]);
    i = j;
  }
  assert(i == nsegs);
  return box_count;
}

/**
 * @ingroup meos_internal_pointcloud_bbox
 * @brief Return an array of N point cloud boxes from the instants or
 * segments of a temporal point cloud value sequence
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] seq Temporal sequence
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @return If `seq->count <= box_count`, the result contains one box
 * per instant or segment. Otherwise, consecutive instants or segments are
 * combined into a single box in the result to reach the given number of
 * boxes.
 */
TPCBox *
tpointcloudseq_split_n_tpcboxes(const TSequence *seq, int box_count, int *count)
{
  assert(seq); assert(count); assert(tpointcloud_temptype(seq->temptype));
  assert(box_count > 0);

  /* Discrete case */
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return tpointcloudseq_disc_split_n_tpcboxes(seq, box_count, count);

  /* Continuous case */
  int nboxes = (seq->count <= box_count) ?
    (seq->count == 1 ? 1 : seq->count - 1) : box_count;
  TPCBox *result = palloc(sizeof(TPCBox) * nboxes);
  *count = tpointcloudseq_cont_split_n_tpcboxes_iter(seq, box_count, result);
  return result;
}

/**
 * @ingroup meos_internal_pointcloud_bbox
 * @brief Return an array of N point cloud boxes from the segments of a
 * temporal point cloud value sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @return If `ss->totalcount <= box_count`, the result contains one box per
 * segment. Otherwise, consecutive input segments are combined into a single box
 * in
 * the result to reach the given number of boxes.
 */
TPCBox *
tpointcloudseqset_split_n_tpcboxes(const TSequenceSet *ss, int box_count,
  int *count)
{
  assert(ss); assert(count); assert(tpointcloud_temptype(ss->temptype));
  assert(box_count > 0);

  /* One bounding box per segment */
  if (ss->totalcount <= box_count)
    return tpointcloudseqset_tpcboxes(ss, count);

  /* At most box_count boxes, the segments being more */
  TPCBox *result = palloc(sizeof(TPCBox) * box_count);

  /* Number of bounding boxes per composing sequence determined from the
   * proportion of seq->count and ss->totalcount */
  if (ss->count <= box_count)
  {
    int nboxes1 = 0;
    for (int i = 0; i < ss->count; i++)
    {
      bool end = false;
      const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
      int nboxes_seq = (int) (box_count * seq->count * 1.0 / ss->totalcount);
      if (! nboxes_seq)
        nboxes_seq = 1;
      if (nboxes_seq + nboxes1 >= box_count)
      {
        end = true;
        nboxes_seq = box_count - nboxes1;
      }
      nboxes1 += tpointcloudseq_cont_split_n_tpcboxes_iter(seq, nboxes_seq,
        &result[nboxes1]);
      if (end)
        break;
    }
    assert(nboxes1 <= box_count);
    *count = nboxes1;
    return result;
  }

  /* Merge consecutive sequences to reach the maximum number of boxes */
  /* Minimum number of sequences merged together in an output box */
  int size = ss->count / box_count;
  /* Number of output boxes that result from merging (size + 1) sequences */
  int remainder = ss->count % box_count;
  int i = 0; /* Loop variable for input sequences */
  for (int k = 0; k < box_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    tpointcloudseq_cont_split_n_tpcboxes_iter(TSEQUENCESET_SEQ_N(ss, i), 1,
      &result[k]);
    for (int l = i + 1; l < j; l++)
    {
      TPCBox box;
      tpointcloudseq_cont_split_n_tpcboxes_iter(TSEQUENCESET_SEQ_N(ss, l), 1,
        &box);
      tpcbox_expand(&box, &result[k]);
    }
    i = j;
  }
  assert(i == ss->count);
  *count = box_count;
  return result;
}

/**
 * @ingroup meos_internal_pointcloud_bbox
 * @brief Return an array of N point cloud boxes obtained by merging
 * consecutive instants or segments of a temporal point cloud value, its
 * arguments valid
 * @details The internal twin of #tpointcloud_split_n_tpcboxes, which an index calls
 * @param[in] temp Temporal point cloud value
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 */
TPCBox *
tpointcloud_split_n_tpcboxes_intl(const Temporal *temp, int box_count,
  int *count)
{
  assert(temp); assert(count); assert(tpointcloud_temptype(temp->temptype));
  assert(box_count > 0);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tpointcloudinst_tpcboxes((TInstant *) temp);
    case TSEQUENCE:
      return tpointcloudseq_split_n_tpcboxes((TSequence *) temp, box_count,
        count);
    default: /* TSEQUENCESET */
      return tpointcloudseqset_split_n_tpcboxes((TSequenceSet *) temp,
        box_count, count);
  }
}

/**
 * @ingroup meos_pointcloud_bbox_split
 * @brief Return an array of N point cloud boxes obtained by merging
 * consecutive instants or segments of a temporal point cloud value
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] temp Temporal point cloud value
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @errval NULL
 * @csqlfn #Tpointcloud_split_n_tpcboxes()
 */
TPCBox *
tpointcloud_split_n_tpcboxes(const Temporal *temp, int box_count, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TPOINTCLOUD(temp, NULL);
  if (! ensure_positive(box_count))
    return NULL;
  return tpointcloud_split_n_tpcboxes_intl(temp, box_count, count);
}

/*****************************************************************************
 * split_each_n_tpcboxes function
 *****************************************************************************/

/**
 * @brief Return an array of point cloud boxes obtained by merging
 * consecutive instants of a temporal point cloud value sequence with discrete
 * interpolation
 * @param[in] seq Temporal sequence
 * @param[in] elem_count Number of input instants merged into an output box
 * @param[out] count Number of elements in the output array
 */
static TPCBox *
tpointcloudseq_disc_split_each_n_tpcboxes(const TSequence *seq, int elem_count,
  int *count)
{
  assert(seq); assert(count); assert(tpointcloud_temptype(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  assert(elem_count > 0);

  int nboxes = ceil((double) seq->count / (double) elem_count);
  TPCBox *result = palloc(sizeof(TPCBox) * nboxes);
  int k = 0;
  for (int i = 0; i < seq->count; ++i)
  {
    if (i % elem_count == 0)
      tpointcloudinst_set_tpcbox(TSEQUENCE_INST_N(seq, i), &result[k++]);
    else
    {
      TPCBox box;
      tpointcloudinst_set_tpcbox(TSEQUENCE_INST_N(seq, i), &box);
      tpcbox_expand(&box, &result[k - 1]);
    }
  }
  assert(k == nboxes);
  *count = k;
  return result;
}

/**
 * @brief Return an array of point cloud boxes obtained by merging
 * consecutive segments of a temporal point cloud value sequence (iterator
 * function)
 * @details The sequence has continuous interpolation.
 * @param[in] seq Temporal sequence
 * @param[in] elem_count Number of input segments merged into an output box
 * @param[out] result Array of boxes
 * @return Number of elements in the output array
 */
static int
tpointcloudseq_cont_split_each_n_tpcboxes_iter(const TSequence *seq,
  int elem_count, TPCBox *result)
{
  assert(seq); assert(result); assert(tpointcloud_temptype(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);
  assert(elem_count > 0);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tsequence_set_bbox_slice(seq, 0, 0, &result[0]);
    return 1;
  }

  /* General case */
  int nboxes = ceil((double) (seq->count - 1) / (double) elem_count);
  for (int k = 0; k < nboxes; k++)
  {
    int first = k * elem_count;
    int last = Min(first + elem_count, seq->count - 1);
    tsequence_set_bbox_slice(seq, first, last, &result[k]);
  }
  return nboxes;
}

/**
 * @ingroup meos_internal_pointcloud_bbox
 * @brief Return an array of point cloud boxes obtained by merging
 * consecutive instants or segments of a temporal point cloud value sequence
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] seq Temporal sequence
 * @param[in] elem_count Number of input segments merged into an output box
 * @param[out] count Number of elements in the output array
 */
TPCBox *
tpointcloudseq_split_each_n_tpcboxes(const TSequence *seq, int elem_count,
  int *count)
{
  assert(seq); assert(count); assert(tpointcloud_temptype(seq->temptype));
  assert(elem_count > 0);

  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return tpointcloudseq_disc_split_each_n_tpcboxes(seq, elem_count, count);

  /* Number of instants or segments */
  int nelems = (seq->count == 1) ? 1 : seq->count - 1;
  int nboxes = ceil((double) nelems / (double) elem_count);
  TPCBox *result = palloc(sizeof(TPCBox) * nboxes);
  *count = tpointcloudseq_cont_split_each_n_tpcboxes_iter(seq, elem_count,
    result);
  return result;
}

/**
 * @ingroup meos_internal_pointcloud_bbox
 * @brief Return an array of point cloud boxes obtained by merging
 * consecutive segments of a temporal point cloud value sequence set 
 * @param[in] ss Temporal sequence set
 * @param[in] elem_count Number of input segments merged into an output box
 * @param[out] count Number of elements in the output array
 */
TPCBox *
tpointcloudseqset_split_each_n_tpcboxes(const TSequenceSet *ss, int elem_count,
  int *count)
{
  assert(ss); assert(count); assert(tpointcloud_temptype(ss->temptype));
  assert(elem_count > 0);

  /* Singleton sequence set */
  if (ss->count == 1)
    return tpointcloudseq_split_each_n_tpcboxes(TSEQUENCESET_SEQ_N(ss, 0),
      elem_count, count);

  /* Iterate for every composing sequence */
  int nboxes = 0;
  TPCBox *result = palloc(sizeof(TPCBox) * ss->totalcount);
  for (int i = 0; i < ss->count; ++i)
    nboxes += tpointcloudseq_cont_split_each_n_tpcboxes_iter(
      TSEQUENCESET_SEQ_N(ss, i), elem_count, &result[nboxes]);
  *count = nboxes;
  return result;
}

/**
 * @ingroup meos_internal_pointcloud_bbox
 * @brief Return an array of point cloud boxes obtained by merging
 * consecutive instants or segments of a temporal point cloud value, its
 * arguments valid
 * @details The internal twin of #tpointcloud_split_each_n_tpcboxes, which an index
 * calls
 * @param[in] temp Temporal value
 * @param[in] elem_count Number of input instants or segments merged into an
 * output box
 * @param[out] count Number of values of the output array
 */
TPCBox *
tpointcloud_split_each_n_tpcboxes_intl(const Temporal *temp, int elem_count,
  int *count)
{
  assert(temp); assert(count); assert(tpointcloud_temptype(temp->temptype));
  assert(elem_count > 0);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tpointcloudinst_tpcboxes((TInstant *) temp);
    case TSEQUENCE:
      return tpointcloudseq_split_each_n_tpcboxes((TSequence *) temp,
        elem_count, count);
    default: /* TSEQUENCESET */
      return tpointcloudseqset_split_each_n_tpcboxes((TSequenceSet *) temp,
        elem_count, count);
  }
}

/**
 * @ingroup meos_pointcloud_bbox_split
 * @brief Return an array of point cloud boxes obtained by merging
 * consecutive instants or segments of a temporal point cloud value
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] temp Temporal value
 * @param[in] elem_count Number of input instants or segments merged into an
 * output box
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Tpointcloud_split_each_n_tpcboxes()
 */
TPCBox *
tpointcloud_split_each_n_tpcboxes(const Temporal *temp, int elem_count,
  int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TPOINTCLOUD(temp, NULL);
  if (! ensure_positive(elem_count))
    return NULL;
  return tpointcloud_split_each_n_tpcboxes_intl(temp, elem_count, count);
}

/*****************************************************************************
 * merge_n_tpcboxes and merge_each_n_tpcboxes functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_pointcloud_bbox
 * @brief Return an array of N point cloud boxes obtained by merging the boxes
 * of
 * the consecutive instants or segments of a temporal point cloud value, each
 * time the
 * consecutive pair whose union adds the least size to theirs, its arguments
 * valid
 * @details The internal twin of #tpointcloud_merge_n_tpcboxes, which an index calls
 * @param[in] temp Temporal value
 * @param[in] box_count Number of boxes
 * @param[out] count Number of values of the output array
 */
TPCBox *
tpointcloud_merge_n_tpcboxes_intl(const Temporal *temp, int box_count,
  int *count)
{
  assert(temp); assert(count); assert(tpointcloud_temptype(temp->temptype));
  assert(box_count > 0);
  TPCBox *result = tpointcloud_tpcboxes_intl(temp, count);
  if (result)
    *count = bbox_merge_n(T_TPCBOX, result, *count, box_count, &stbox_adjust,
      &stbox_size);
  return result;
}

/**
 * @ingroup meos_pointcloud_bbox_split
 * @brief Return an array of N point cloud boxes obtained by merging the boxes
 * of
 * the consecutive instants or segments of a temporal point cloud value, each
 * time the
 * consecutive pair whose union adds the least size to theirs
 * @details The merge twin of #tpointcloud_split_n_tpcboxes, by #bbox_merge_n
 * @param[in] temp Temporal value
 * @param[in] box_count Number of boxes
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Tpointcloud_merge_n_tpcboxes()
 */
TPCBox *
tpointcloud_merge_n_tpcboxes(const Temporal *temp, int box_count, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TPOINTCLOUD(temp, NULL);
  if (! ensure_positive(box_count))
    return NULL;
  return tpointcloud_merge_n_tpcboxes_intl(temp, box_count, count);
}

/**
 * @ingroup meos_internal_pointcloud_bbox
 * @brief Return an array of point cloud boxes obtained by merging the boxes of
 * the
 * consecutive instants or segments of a temporal point cloud value, each time
 * the consecutive
 * pair whose union adds the least size to theirs, until a box holds on
 * average a number of them, its arguments valid
 * @details The internal twin of #tpointcloud_merge_each_n_tpcboxes, which an index
 * calls
 * @param[in] temp Temporal value
 * @param[in] elem_count Number of input instants or segments merged into an
 * output box on average
 * @param[out] count Number of values of the output array
 */
TPCBox *
tpointcloud_merge_each_n_tpcboxes_intl(const Temporal *temp, int elem_count,
  int *count)
{
  assert(temp); assert(count); assert(tpointcloud_temptype(temp->temptype));
  assert(elem_count > 0);
  TPCBox *result = tpointcloud_tpcboxes_intl(temp, count);
  if (result)
    *count = bbox_merge_each_n(T_TPCBOX, result, *count, elem_count,
      &stbox_adjust, &stbox_size);
  return result;
}

/**
 * @ingroup meos_pointcloud_bbox_split
 * @brief Return an array of point cloud boxes obtained by merging the boxes of
 * the
 * consecutive instants or segments of a temporal point cloud value, each time
 * the consecutive
 * pair whose union adds the least size to theirs, until a box holds on
 * average a number of them
 * @details The merge twin of #tpointcloud_split_each_n_tpcboxes, by
 * #bbox_merge_each_n
 * @param[in] temp Temporal value
 * @param[in] elem_count Number of input instants or segments merged into an
 * output box on average
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Tpointcloud_merge_each_n_tpcboxes()
 */
TPCBox *
tpointcloud_merge_each_n_tpcboxes(const Temporal *temp, int elem_count,
  int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TPOINTCLOUD(temp, NULL);
  if (! ensure_positive(elem_count))
    return NULL;
  return tpointcloud_merge_each_n_tpcboxes_intl(temp, elem_count, count);
}

/*****************************************************************************
 * merge_query_tpcboxes function
 *****************************************************************************/

/**
 * @ingroup meos_internal_pointcloud_bbox
 * @brief Return an array of point cloud boxes obtained by merging
 * the boxes of the consecutive instants or segments of a
 * temporal point cloud value into those a query of the extents of a query box
 * is expected to read least, its arguments valid
 * @details The internal twin of #tpointcloud_merge_query_tpcboxes, which an index
 * calls
 * @param[in] temp Temporal value
 * @param[in] query Query box
 * @param[out] count Number of values of the output array
 */
TPCBox *
tpointcloud_merge_query_tpcboxes_intl(const Temporal *temp, const TPCBox *query,
  int *count)
{
  assert(temp); assert(query); assert(count);
  assert(tpointcloud_temptype(temp->temptype));
  assert(ensure_valid_tpointcloud_tpcbox(temp, query));
  TPCBox *result = tpointcloud_tpcboxes_intl(temp, count);
  if (result)
    *count = bbox_merge_query(T_TPCBOX, result, *count, query, &stbox_adjust,
      &stbox_query_size);
  return result;
}

/**
 * @ingroup meos_pointcloud_bbox_split
 * @brief Return an array of point cloud boxes obtained by merging
 * the boxes of the consecutive instants or segments of a
 * temporal point cloud value into those a query of the extents of a query box
 * is expected to read least
 * @details The query twin of #tpointcloud_merge_n_tpcboxes, by #bbox_merge_query:
 * the widths of the query box are the extents of the query, its position is
 * not read, and a dimension it lacks restricts no query
 * @param[in] temp Temporal value
 * @param[in] query Query box
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Tpointcloud_merge_query_tpcboxes()
 */
TPCBox *
tpointcloud_merge_query_tpcboxes(const Temporal *temp, const TPCBox *query,
  int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TPOINTCLOUD(temp, NULL); VALIDATE_NOT_NULL(query, NULL);
  if (! ensure_valid_tpointcloud_tpcbox(temp, query))
    return NULL;
  return tpointcloud_merge_query_tpcboxes_intl(temp, query, count);
}
/* GENERATED-BOXSPLIT-END tpointcloud */

/*****************************************************************************/
