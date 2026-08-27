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
 * @brief Bounding box operators for temporal types
 *
 * The bounding box of temporal values are
 * - a @p Span for temporal Booleans
 * - a @p TBox for temporal integers and floats, where the @p x coordinate is
 *   for the value dimension and the @p t coordinate is for the time dimension.
 * The following operators are defined: @p overlaps, @p contains, @p contained,
 * @p same, and @p adjacent.
 *
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the value dimension, only the time dimension, or both
 * the value and the time dimensions.
 */

#include "temporal/temporal_boxops.h"

/* C */
#include <assert.h>
#include <limits.h>
#include <math.h>
/* PostgreSQL */
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/span.h"
#include "temporal/type_util.h"
#include "temporal/type_util.h"
#include "geo/tspatial_boxops.h"
#if POINTCLOUD
  #include <meos_pointcloud.h>
  #include "pointcloud/tpc_boxops.h"
#endif

/*****************************************************************************
 * Functions on generic bounding boxes of temporal types
 *****************************************************************************/

/**
 * @brief Return true if the type is a bounding box type
 */
bool
bbox_type(MeosType bboxtype)
{
  if (bboxtype == T_TSTZSPAN || bboxtype == T_TBOX || bboxtype == T_STBOX)
    return true;
#if POINTCLOUD
  if (bboxtype == T_TPCBOX)
    return true;
#endif
  return false;
}

/**
 * @brief Return the size of a bounding box type
 */
size_t
bbox_get_size(MeosType bboxtype)
{
  assert(bbox_type(bboxtype) || span_type(bboxtype));
  if (span_type(bboxtype))
    return sizeof(Span);
  if (bboxtype == T_TBOX)
    return sizeof(TBox);
#if POINTCLOUD
  if (bboxtype == T_TPCBOX)
    return sizeof(TPCBox);
#endif
  else /* bboxtype == T_STBOX */
    return sizeof(STBox);
}

/**
 * @brief Return the maximum number of dimensions of a bounding box type
 */
int
bbox_max_dims(MeosType bboxtype)
{
  assert(bbox_type(bboxtype));
  if (bboxtype == T_TSTZSPAN)
    return 1;
  if (bboxtype == T_TBOX)
    return 2;
  else /* bboxtype == T_STBOX */
    return 4;
}

/**
 * @brief Return true if two bounding boxes are equal
 * @param[in] box1,box2 Bounding boxes
 * @param[in] temptype Temporal type
 */
bool
temporal_bbox_eq(const void *box1, const void *box2, MeosType temptype)
{
  MeosType bboxtype = type_bboxtype(temptype);
  /* The box a temporal type carries is the one its catalog row
   * prescribes, so the dispatch below reads that and not the class */
  assert(bboxtype != T_UNKNOWN);
  if (bboxtype == T_TSTZSPAN)
    return span_eq((Span *) box1, (Span *) box2);
  else if (bboxtype == T_TBOX)
    return tbox_eq((TBox *) box1, (TBox *) box2);
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
    return tpcbox_eq((TPCBox *) box1, (TPCBox *) box2);
#endif
  else /* T_STBOX */
    // TODO Due to floating point precision the current statement
    // is not equal to the next one.
    // return stbox_eq((STBox *) box1, (STBox *) box2);
    // Problem raised in the test file 51_tpoint_tbl.test.out
    // Look for temp != merge in that file for 2 other cases where
    // a problem still remains (result != 0) even with the _cmp function
    return stbox_cmp((STBox *) box1, (STBox *) box2) == 0;
}

/**
 * @brief Return -1, 0, or 1 depending on whether the first bounding box
 * is less than, equal to, or greater than the second one
 * @param[in] box1,box2 Bounding boxes
 * @param[in] temptype Temporal type
 */
int
temporal_bbox_cmp(const void *box1, const void *box2, MeosType temptype)
{
  MeosType bboxtype = type_bboxtype(temptype);
  /* The box a temporal type carries is the one its catalog row
   * prescribes, so the dispatch below reads that and not the class */
  assert(bboxtype != T_UNKNOWN);
  if (bboxtype == T_TSTZSPAN)
    return span_cmp((Span *) box1, (Span *) box2);
  else if (bboxtype == T_TBOX)
    return tbox_cmp((TBox *) box1, (TBox *) box2);
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
    return tpcbox_cmp((TPCBox *) box1, (TPCBox *) box2);
#endif
  else /* T_STBOX */
    return stbox_cmp((STBox *) box1, (STBox *) box2);
}

/**
 * @brief Return true if the bounding box type is compatible with the
 * temporal type, report an error otherwise
 * @details Shared by the in-memory RTree and SPTree indexes: temporal alphas
 * (tbool, ttext) require a span bounding box, temporal numbers (tint,
 * tfloat) require a TBox, spatiotemporal types (tgeompoint, tgeogpoint,
 * etc.) require an STBox, and, when POINTCLOUD is enabled, temporal point
 * clouds (tpcpoint, tpcpatch) require a TPCBox.
 * @param[in] bboxtype Bounding box type of the index
 * @param[in] temp Temporal value
 */
bool
ensure_bbox_temporal_compatible(MeosType bboxtype, const Temporal *temp)
{
  assert(temp);
  MeosType temptype = temp->temptype;
  /* The catalog prescribes exactly one box per temporal type */
  bool compatible = (bboxtype == type_bboxtype(temptype));
  if (! compatible)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Bounding box type (%s) is not compatible with temporal type (%s)",
      meostype_name(bboxtype), meostype_name(temptype));
    return false;
  }
  return true;
}

/**
 * @brief Return true if two indexes hold the same bounding box type, report an
 * error otherwise
 * @details Shared by the in-memory RTree and SPTree indexes: a join reads the
 * entries of one index against the entries of the other, so the two hold boxes
 * of one type
 * @param[in] bboxtype1,bboxtype2 Bounding box types of the two indexes
 */
bool
ensure_same_index_bboxtype(MeosType bboxtype1, MeosType bboxtype2)
{
  if (bboxtype1 == bboxtype2)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "Join of indexes on mixed bounding box types: %s and %s",
    meostype_name(bboxtype1), meostype_name(bboxtype2));
  return false;
}

/**
 * @brief Return true if an operation pairs the entries of two indexes, report
 * an error otherwise
 * @details Shared by the in-memory RTree and SPTree indexes. A join prunes a
 * pair of subtrees on the overlap of what they cover, which holds only for the
 * operations an overlap implies: one entry contains, or is contained by,
 * another only where the two overlap. The operations that order a dimension
 * pair entries that lie apart, which is what the pruning discards, so a join
 * refuses them rather than reporting a silently short answer.
 * @param[in] op Search operation
 */
bool
ensure_index_join_op(IndexSearchOp op)
{
  if (op == INDEX_OVERLAPS || op == INDEX_CONTAINS || op == INDEX_CONTAINED_BY ||
      op == INDEX_SAME || op == INDEX_ADJACENT)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "A join answers the operations that compare extents, not the ones that "
    "order a dimension");
  return false;
}

/**
 * @brief Decompose a temporal value into an array of tight per-segment
 * bounding boxes whose element type matches the given bounding box type
 * @details Shared by the in-memory RTree and SPTree indexes. The
 * decomposition reuses the same per-segment bounding box machinery as the
 * single-box path: temporal alphas are split with #temporal_split_n_spans,
 * temporal numbers with #tnumber_split_n_tboxes, and temporal geos with
 * #tgeo_split_n_stboxes. These already handle the TINSTANT, TSEQUENCE and
 * TSEQUENCESET subtypes, the discrete, step and linear interpolations, and
 * the Z, geodetic and SRID flags, and coarsen by merging adjacent segment
 * boxes (deterministic chunking) down to at most `maxboxes` boxes. When
 * `maxboxes <= 1`, the temporal value is an instant, or the temporal type
 * has no per-segment splitter (e.g. tcbuffer, tnpoint, tpose, tpcpoint,
 * tpcpatch), the function degenerates to the single minimum bounding box,
 * byte-identical to the result of #temporal_set_bbox.
 * @param[in] bboxtype Bounding box type of the index (unused for the
 * multi-box decomposition, kept for symmetry with the compatibility check
 * and for the assertion below)
 * @param[in] boxsize Size in bytes of a single bounding box allocated for
 * the degenerate/fallback single-box result; the caller passes a size large
 * enough to hold #temporal_set_bbox's output for `temp`, which may differ
 * from the index's own per-entry box size (e.g. an index that internally
 * projects a larger box type onto a smaller one)
 * @param[in] temp Temporal value to decompose
 * @param[in] maxboxes Maximum number of boxes produced for `temp`
 * @param[out] count Number of boxes in the returned array
 * @return Allocated array of `*count` bounding boxes, or @p NULL on error
 * @pre `temp` is compatible with `bboxtype` (verified by the callers)
 */
void *
bbox_temporal_split_boxes(MeosType bboxtype UNUSED, size_t boxsize,
  const Temporal *temp, int maxboxes, int *count)
{
  assert(bbox_type(bboxtype)); assert(temp); assert(count);

  /* Degenerate single minimum bounding box */
  if (maxboxes <= 1 || temp->subtype == TINSTANT)
  {
    void *result = palloc0(boxsize);
    temporal_set_bbox(temp, result);
    *count = 1;
    return result;
  }

  /* Multi-box decomposition reusing the existing per-type splitters */
  MeosType temptype = temp->temptype;
  if (talpha_type(temptype))
    return temporal_split_n_spans(temp, maxboxes, count);
  if (tnumber_type(temptype))
    return tnumber_split_n_tboxes(temp, maxboxes, count);
  if (tgeo_type_all(temptype))
    return tgeo_split_n_stboxes(temp, maxboxes, count);

  /* No per-segment splitter: fall back to the single minimum bounding box */
  void *result = palloc0(boxsize);
  temporal_set_bbox(temp, result);
  *count = 1;
  return result;
}

/*****************************************************************************
 * Compute the bounding box at the creation of temporal values
 *****************************************************************************/

/**
 * @brief Return the size of a bounding box of a temporal type
 */
size_t
temporal_bbox_size(MeosType temptype)
{
  MeosType bboxtype = type_bboxtype(temptype);
  assert(bboxtype != T_UNKNOWN);
  return bbox_get_size(bboxtype);
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return in the last argument the temporal box of a temporal number
 * instant
 * @param[in] inst Temporal value
 * @param[out] box Result
 */
void
tnumberinst_set_tbox(const TInstant *inst, TBox *box)
{
  assert(inst); assert(temporal_type(inst->temptype)); assert(box);
  assert(tnumber_type(inst->temptype));
  MeosType basetype = temptype_basetype(inst->temptype);
  MeosType spantype = basetype_spantype(basetype);
  Datum value = tinstant_value_p(inst);
  Datum time = TimestampTzGetDatum(inst->t);
  TBox *tbox = (TBox *) box;
  memset(tbox, 0, sizeof(TBox));
  span_set(value, value, true, true, basetype, spantype, &tbox->span);
  span_set(time, time, true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &tbox->period);
  MEOS_FLAGS_SET_X(tbox->flags, true);
  MEOS_FLAGS_SET_T(tbox->flags, true);
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return in the last argument the bounding box of a temporal instant
 * @param[in] inst Temporal value
 * @param[out] box Result
 */
void
tinstant_set_bbox(const TInstant *inst, void *box)
{
  assert(inst); assert(box);
  MeosType bboxtype = type_bboxtype(inst->temptype);
  /* The box a temporal type carries is the one its catalog row
   * prescribes, so the dispatch below reads that and not the class */
  assert(bboxtype != T_UNKNOWN);
  if (bboxtype == T_TSTZSPAN)
    span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
      true, true, T_TIMESTAMPTZ, T_TSTZSPAN, (Span *) box);
  else if (bboxtype == T_TBOX)
    tnumberinst_set_tbox(inst, (TBox *) box);
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
    tpointcloudinst_set_tpcbox(inst, (TPCBox *) box);
#endif
  else /* T_STBOX */
    tspatialinst_set_stbox(inst, (STBox *) box);
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return int the last argument the temporal box of a temporal number
 * sequence
 * @param[in] seq Temporal sequence
 * @param[out] box Temporal box
 */
void
tnumberseq_set_tbox(const TSequence *seq, TBox *box)
{
  assert(seq); assert(box); assert(tnumber_type(seq->temptype));
  memcpy(box, TSEQUENCE_BBOX_PTR(seq), sizeof(TBox));
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return in the last argument the bounding box of a temporal sequence
 * @param[in] seq Temporal sequence
 * @param[out] box Bounding box
 */
void
tsequence_set_bbox(const TSequence *seq, void *box)
{
  assert(seq); assert(box);
  memset(box, 0, seq->bboxsize);
  memcpy(box, TSEQUENCE_BBOX_PTR(seq), seq->bboxsize);
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return in the last argument the temporal box of a temporal number
 * sequence
 * @param[in] ss Temporal sequence set
 * @param[out] box Temporal box
 */
void
tnumberseqset_set_tbox(const TSequenceSet *ss, TBox *box)
{
  assert(ss); assert(box); assert(tnumber_type(ss->temptype));
  memcpy(box, TSEQUENCESET_BBOX_PTR(ss), sizeof(TBox));
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return in the last argument the bounding box of a temporal sequence
 * set
 * @param[in] ss Temporal sequence set
 * @param[out] box Bounding box
 */
void
tsequenceset_set_bbox(const TSequenceSet *ss, void *box)
{
  assert(ss); assert(box);
  memset(box, 0, ss->bboxsize);
  memcpy(box, TSEQUENCESET_BBOX_PTR(ss), ss->bboxsize);
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return in the last argument the temporal box of a temporal number
 * @param[in] temp Temporal number
 * @param[out] box Temporal box
 */
void
tnumber_set_tbox(const Temporal *temp, TBox *box)
{
  assert(temp); assert(box); assert(tnumber_type(temp->temptype));
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      tnumberinst_set_tbox((TInstant *) temp, box);
      break;
    case TSEQUENCE:
      tnumberseq_set_tbox((TSequence *) temp, box);
      break;
    default: /* TSEQUENCESET */
      tnumberseqset_set_tbox((TSequenceSet *) temp, box);
  }
  return;
}

/*****************************************************************************/

/**
 * @brief Set a temporal box from an array of temporal number instants
 * @param[in] instants Temporal instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc, upper_inc True when the corresponding bound is
 * inclusive, false otherwise
 * @param[in] interp Interpolation
 * @param[in] box Box
 */
static void
tnumberinstarr_set_tbox(TInstant **instants, int count, bool lower_inc,
  bool upper_inc, interpType interp, TBox *box)
{
  assert(tnumber_type(instants[0]->temptype));
  MeosType basetype = temptype_basetype(instants[0]->temptype);
  MeosType spantype = basetype_spantype(basetype);
  /* For discrete or step interpolation the bounds are always inclusive */
  bool lower_inc1 = lower_inc;
  bool upper_inc1 = upper_inc;
  if (interp != LINEAR)
  {
    lower_inc1 = upper_inc1 = true;
  }
  /* Compute the value span */
  Datum min = tinstant_value_p(instants[0]);
  Datum max = min;
  bool min_inc = lower_inc1, max_inc = lower_inc1;
  for (int i = 1; i < count; i++)
  {
    Datum value = tinstant_value_p(instants[i]);
    int min_cmp = datum_cmp(value, min, basetype);
    int max_cmp = datum_cmp(value, max, basetype);
    if (min_cmp <= 0)
    {
      min = value;
      if (min_cmp == 0)
        min_inc |= (i < count - 1) ? true : upper_inc1;
      else
        min_inc = (i < count - 1) ? true : upper_inc1;
    }
    if (max_cmp >= 0)
    {
      max = value;
      if (max_cmp == 0)
        max_inc |= (i < count - 1) ? true : upper_inc1;
      else
        max_inc = (i < count - 1) ? true : upper_inc1;
    }
  }
  if (datum_eq(min, max, basetype))
  {
    min_inc = max_inc = true;
  }
  span_set(min, max, min_inc, max_inc, basetype, spantype, &box->span);
  /* Compute the time span */
  span_set(TimestampTzGetDatum(instants[0]->t),
    TimestampTzGetDatum(instants[count - 1]->t), lower_inc, upper_inc,
    T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  /* Set the flags */
  MEOS_FLAGS_SET_X(box->flags, true);
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @brief Set a bounding box from an array of temporal instant values
 * @param[in] instants Temporal instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc,upper_inc Period bounds
 * @param[in] interp Interpolation
 * @param[out] box Bounding box
 */
void
tinstarr_set_bbox(TInstant **instants, int count, bool lower_inc,
  bool upper_inc, interpType interp, void *box)
{
  assert(instants); assert(box);
  MeosType bboxtype = type_bboxtype(instants[0]->temptype);
  /* The box a temporal type carries is the one its catalog row
   * prescribes, so the dispatch below reads that and not the class */
  assert(bboxtype != T_UNKNOWN);
  if (bboxtype == T_TSTZSPAN)
    span_set(TimestampTzGetDatum(instants[0]->t),
      TimestampTzGetDatum(instants[count - 1]->t), lower_inc, upper_inc,
      T_TIMESTAMPTZ, T_TSTZSPAN, (Span *) box);
  else if (bboxtype == T_TBOX)
    tnumberinstarr_set_tbox(instants, count, lower_inc, upper_inc,
      interp, (TBox *) box);
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
    tpointcloudinstarr_set_tpcbox(instants, count, lower_inc, upper_inc,
      interp, (TPCBox *) box);
#endif
  else /* T_STBOX */
    tspatialinstarr_set_stbox(instants, count, lower_inc, upper_inc,
      interp, (STBox *) box);
  /* Set the lower_inc and upper_inc bounds of the period at the beginning
   * of the bounding box */
  Span *s = (Span *) box;
  s->lower_inc = lower_inc;
  s->upper_inc = upper_inc;
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return in the last argument the bounding box of the instants of a
 * temporal sequence between two positions
 * @param[in] seq Temporal sequence
 * @param[in] first,last Positions of the first and of the last instant
 * @param[out] box Bounding box
 * @note A bound interior to the sequence is inclusive, since the instant it
 * names belongs to both of the segments meeting there, while a bound at an
 * end of the sequence carries the inclusivity the sequence gives that end.
 * The box of a slice is thus contained in the box of the sequence, which is
 * what an index keyed on the slices of a value relies on
 */
void
tsequence_set_bbox_slice(const TSequence *seq, int first, int last, void *box)
{
  assert(seq); assert(box);
  assert(first >= 0); assert(last < seq->count); assert(first <= last);
  int count = last - first + 1;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
    instants[i] = (TInstant *) TSEQUENCE_INST_N(seq, first + i);
  tinstarr_set_bbox(instants, count,
    (first == 0) ? seq->period.lower_inc : true,
    (last == seq->count - 1) ? seq->period.upper_inc : true,
    MEOS_FLAGS_GET_INTERP(seq->flags), box);
  pfree(instants);
  return;
}

/**
 * @brief Expand the bounding box of a temporal number sequence with an instant
 * @param[inout] seq Temporal sequence
 * @param[in] inst Temporal instant
 */
static void
tnumberseq_expand_tbox(TSequence *seq, const TInstant *inst)
{
  TBox box;
  tnumberinst_set_tbox(inst, &box);
  tbox_expand(&box, (TBox *) TSEQUENCE_BBOX_PTR(seq));
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Expand the bounding box of a temporal sequence with an additional
 * instant
 * @param[inout] seq Temporal sequence
 * @param[in] inst Temporal instant
 */
void
tsequence_expand_bbox(TSequence *seq, const TInstant *inst)
{
  MeosType bboxtype = type_bboxtype(seq->temptype);
  /* The box a temporal type carries is the one its catalog row
   * prescribes, so the dispatch below reads that and not the class */
  assert(bboxtype != T_UNKNOWN);
  if (bboxtype == T_TSTZSPAN)
    span_set(TimestampTzGetDatum(TSEQUENCE_INST_N(seq, 0)->t),
      TimestampTzGetDatum(inst->t), seq->period.lower_inc, true, T_TIMESTAMPTZ,
      T_TSTZSPAN, (Span *) TSEQUENCE_BBOX_PTR(seq));
  else if (bboxtype == T_TBOX)
    tnumberseq_expand_tbox(seq, inst);
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
    tpointcloudseq_expand_tpcbox(seq, inst);
#endif
  else /* T_STBOX */
    tspatialseq_expand_stbox(seq, inst);
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Expand the bounding box of a temporal sequence set with an additional
 * sequence
 * @param[inout] ss Temporal sequence set
 * @param[in] seq Temporal sequence
 */
void
tsequenceset_expand_bbox(TSequenceSet *ss, const TSequence *seq)
{
  assert(ss); assert(seq);
  MeosType bboxtype = type_bboxtype(ss->temptype);
  /* The box a temporal type carries is the one its catalog row
   * prescribes, so the dispatch below reads that and not the class */
  assert(bboxtype != T_UNKNOWN);
  if (bboxtype == T_TSTZSPAN)
    span_expand(&seq->period, &ss->period);
  else if (bboxtype == T_TBOX)
    tbox_expand((TBox *) TSEQUENCE_BBOX_PTR(seq),
      (TBox *) TSEQUENCE_BBOX_PTR(ss));
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
    tpcbox_expand((TPCBox *) TSEQUENCE_BBOX_PTR(seq),
      (TPCBox *) TSEQUENCESET_BBOX_PTR(ss));
#endif
  // TODO Generalize as for tgeogpointseq_expand_stbox
  else /* T_STBOX */
    stbox_expand((STBox *) TSEQUENCE_BBOX_PTR(seq),
      (STBox *) TSEQUENCE_BBOX_PTR(ss));
  return;
}

/**
 * @brief Return in the last argument a timestamptz span constructed from an
 * array of temporal sequences
 * @param[in] sequences Temporal sequences
 * @param[in] count Number of elements in the array
 * @param[out] s Result
 */
static void
tseqarr_set_tstzspan(TSequence **sequences, int count, Span *s)
{
  const Span *first = &sequences[0]->period;
  const Span *last = &sequences[count - 1]->period;
  span_set(first->lower, last->upper, first->lower_inc, last->upper_inc,
    T_TIMESTAMPTZ, T_TSTZSPAN, s);
  return;
}

/**
 * @brief Return in the last argument a temporal box constructed from an
 * array of temporal number sequences
 * @param[in] box Box
 * @param[in] sequences Temporal instants
 * @param[in] count Number of elements in the array
 */
static void
tnumberseqarr_set_tbox(TSequence **sequences, int count, TBox *box)
{
  memcpy(box, TSEQUENCE_BBOX_PTR(sequences[0]), sizeof(TBox));
  for (int i = 1; i < count; i++)
  {
    const TBox *box1 = TSEQUENCE_BBOX_PTR(sequences[i]);
    tbox_expand(box1, box);
  }
  return;
}

/**
 * @brief Return in the last argument a bounding box constructed from an
 * array of temporal sequences
 */
void
tseqarr_compute_bbox(TSequence **sequences, int count, void *box)
{
  MeosType bboxtype = type_bboxtype(sequences[0]->temptype);
  /* The box a temporal type carries is the one its catalog row
   * prescribes, so the dispatch below reads that and not the class */
  assert(bboxtype != T_UNKNOWN);
  if (bboxtype == T_TSTZSPAN)
    tseqarr_set_tstzspan(sequences, count, (Span *) box);
  else if (bboxtype == T_TBOX)
    tnumberseqarr_set_tbox(sequences, count, (TBox *) box);
#if POINTCLOUD
  else if (bboxtype == T_TPCBOX)
    tpointcloudseqarr_set_tpcbox(sequences, count, (TPCBox *) box);
#endif
  else /* T_STBOX */
    tspatialseqarr_set_stbox(sequences, count, (STBox *) box);
  return;
}

/*****************************************************************************/

#if MEOS
/**
 * @brief Recompute the bounding box of a temporal sequence
 * @param[inout] seq Temporal sequence
 * @note This function is applied upon a restart
 */
void
tsequence_compute_bbox(TSequence *seq)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = (TInstant *) TSEQUENCE_INST_N(seq, i);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  tinstarr_set_bbox(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, interp, TSEQUENCESET_BBOX_PTR(seq));
  pfree(instants);
  return;
}

/**
 * @brief (Re)compute the bounding box of a temporal sequence set
 * @param[inout] ss Temporal sequence set
 * @note This function is applied upon a restart
 */
void
tsequenceset_compute_bbox(TSequenceSet *ss)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = (TSequence *) TSEQUENCESET_SEQ_N(ss, i);
  tseqarr_compute_bbox(sequences, ss->count, TSEQUENCESET_BBOX_PTR(ss));
  pfree(sequences);
  return;
}
#endif /* MEOS */

/*****************************************************************************
 * Spans functions for temporal values
 * These functions can be used for defining Multi-Entry Search Trees indexes
 * https://www.pgcon.org/2014/schedule/events/696.en.html
 * https://github.com/MobilityDB/mest
 *****************************************************************************/

/**
 * @brief Set the span in the last argument from a temporal instant
 * @param[in] inst Temporal value
 * @param[out] result Span
 */
static void
tinstant_set_span(const TInstant *inst, Span *result)
{
  assert(inst); assert(result);
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, result);
  return;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return a singleton array of spans from a temporal instant
 * @param[in] inst Temporal instant
 */
Span *
tinstant_spans(const TInstant *inst)
{
  assert(inst);
  Span *result = palloc(sizeof(Span));
  tinstant_set_span(inst, result);
  return result;
}

/**
 * @brief Return an array of spans obtained from the instants of a temporal
 * sequence with discrete interpolation
 * @param[in] seq Temporal value
 */
static Span *
tdiscseq_spans(const TSequence *seq)
{
  assert(seq); assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  /* One bounding span per instant */
  Span *result = palloc(sizeof(Span) * seq->count);
  for (int i = 0; i < seq->count; i++)
    tinstant_set_span(TSEQUENCE_INST_N(seq, i), &result[i]);
  return result;
}

/**
 * @brief Return an array of spans obtained from the segments of a temporal
 * sequence with continuous interpolation (iterator function)
 * @param[in] seq Temporal value
 * @param[out] result Temporal span
 * @return Number of elements in the array
 */
static int
tcontseq_spans_iter(const TSequence *seq, Span *result)
{
  assert(seq); assert(result);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tinstant_set_span(TSEQUENCE_INST_N(seq, 0), &result[0]);
    return 1;
  }

  /* One bounding span per segment */
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  for (int i = 0; i < seq->count - 1; i++)
  {
    tinstant_set_span(inst, &result[i]);
    inst = TSEQUENCE_INST_N(seq, i + 1);
    Span span;
    tinstant_set_span(inst, &span);
    span_expand(&span, &result[i]);
  }
  return seq->count - 1;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spans obtained from the instants or segments of a
 * temporal sequence, where the choice between instants or segments depends,
 * respectively, on whether the interpolation is discrete or continuous
 * @param[in] seq Temporal sequence
 * @param[out] count Number of elements in the output array
 */
Span *
tsequence_spans(const TSequence *seq, int *count)
{
  assert(seq); assert(count);

  /* Discrete case */
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
  {
    *count = seq->count;
    return tdiscseq_spans(seq);
  }

  /* Continuous case */
  int nspans = (seq->count == 1) ? 1 : seq->count - 1;
  Span *result = palloc(sizeof(Span) * nspans);
  *count = tcontseq_spans_iter(seq, result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spans obtained from the segments of a temporal
 * sequence set
 * @param[in] ss Temporal sequence set
 * @param[out] count Number of elements in the output array
 */
Span *
tsequenceset_spans(const TSequenceSet *ss, int *count)
{
  assert(ss); assert(count);
  /* One bounding span per segment */
  Span *result = palloc(sizeof(Span) * ss->totalcount);
  int nboxes = 0;
  for (int i = 0; i < ss->count; i++)
    nboxes += tcontseq_spans_iter(TSEQUENCESET_SEQ_N(ss, i), &result[nboxes]);
  assert(nboxes <= ss->totalcount);
  *count = nboxes;
  return result;
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of spans obtained from the instants or segments of a
 * temporal value, where the choice between instants or segments depends,
 * respectively, on whether the interpolation is discrete or continuous
 * @param[in] temp Temporal value
 * @param[out] count Number of values of the output array
 * @return On error return @p NULL
 * @csqlfn #Temporal_spans()
 */
Span *
temporal_spans(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(count, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tinstant_spans((TInstant *) temp);
    case TSEQUENCE:
      return tsequence_spans((TSequence *) temp, count);
    default: /* TSEQUENCESET */
      return tsequenceset_spans((TSequenceSet *) temp, count);
  }
}

/*****************************************************************************/

/**
 * @brief Return an array of N spans from the instants of a temporal sequence
 * with discrete interpolation
 * @param[in] seq Temporal sequence
 * @param[in] span_count Number of spans
 * @param[out] count Number of elements in the output array
 * @return If the number of instants in the sequence is <= `span_count`, the
 * result contains one span per instant. Otherwise, consecutive instants are
 * combined into a single span in the result to reach the number of spans.
 */
static Span *
tdiscseq_split_n_spans(const TSequence *seq, int span_count, int *count)
{
  assert(seq); assert(count); assert(span_count > 0);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);

  /* One bounding span per instant */
  if (seq->count <= span_count)
  {
    *count = seq->count;
    return tdiscseq_spans(seq);
  }

  /* One bounding span per several consecutive instants */
  Span *result = palloc(sizeof(Span) * seq->count);
  /* Minimum number of input instants merged together in an output span */
  int size = seq->count / span_count;
  /* Number of output boxes that result from merging (size + 1) instants */
  int remainder = seq->count % span_count;
  int i = 0; /* Loop variable for input instants */
  for (int k = 0; k < span_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    tinstant_set_span(TSEQUENCE_INST_N(seq, i), &result[k]);
    if (i < j - 1)
    {
      Span span;
      tinstant_set_span(TSEQUENCE_INST_N(seq, j - 1), &span);
      span_expand(&span, &result[k]);
    }
    i = j;
  }
  assert(i == seq->count);
  *count = span_count;
  return result;
}

/**
 * @brief Return an array of N spans from the segments of a temporal sequence
 * with continuous interpolation (iterator function)
 * @param[in] seq Temporal sequence
 * @param[in] span_count Number of spans
 * @param[in] result Array of spans. If the number of segments in the sequence
 * is <= `span_count`, the result contains one span per segment. Otherwise,
 * consecutive segments are combined into a single span in the result to reach
 * the number of spans.
 * @return Number of elements in the output array
 */
static int
tcontseq_split_n_spans_iter(const TSequence *seq, int span_count, Span *result)
{
  assert(seq); assert(result); assert(span_count > 0);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tinstant_set_span(TSEQUENCE_INST_N(seq, 0), &result[0]);
    return 1;
  }

  /* One bounding span per segment */
  int nsegs = seq->count - 1;
  if (nsegs <= span_count)
    return tcontseq_spans_iter(seq, result);

  /* One bounding span per several consecutive segments */
  /* Minimum number of input segments merged together in an output span */
  int size = nsegs / span_count;
  /* Number of output boxes that result from merging (size + 1) segments */
  int remainder = nsegs % span_count;
  int i = 0; /* Loop variable for input segments */
  for (int k = 0; k < span_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    tinstant_set_span(TSEQUENCE_INST_N(seq, i), &result[k]);
    Span span;
    tinstant_set_span(TSEQUENCE_INST_N(seq, j), &span);
    span_expand(&span, &result[k]);
    i = j;
  }
  assert(i == nsegs);
  return span_count;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N spans obtained from the instants or segments of
 * a temporal sequence, where the choice between instants or segments depends,
 * respectively, on whether the interpolation is discrete or continuous
 * @param[in] seq Temporal sequence
 * @param[in] span_count Number of spans
 * @param[out] count Number of elements in the output array
 * @return If the number of instants or segments in the sequence is <=
 * `span_count`, the result contains one span per instant or segment.
 * Otherwise, consecutive instants or segments are combined into a single span
 * in the result to reach the number of spans.
 */
Span *
tsequence_split_n_spans(const TSequence *seq, int span_count, int *count)
{
  assert(seq); assert(count); assert(span_count > 0);

  /* Discrete case */
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return tdiscseq_split_n_spans(seq, span_count, count);

  /* Continuous case */
  int nspans = (seq->count - 1 <= span_count) ?
    (seq->count == 1 ? 1 : seq->count - 1) : span_count;
  Span *result = palloc(sizeof(Span) * nspans);
  *count = tcontseq_split_n_spans_iter(seq, span_count, result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N spans from the segments of a temporal sequence
 * set
 * @param[in] ss Temporal sequence set
 * @param[in] span_count Number of spans
 * @param[out] count Number of elements in the output array
 * @return If the number of segments in the temporal sequence set is <=
 * `span_count`, the result contains one span per segment. Otherwise,
 * consecutive segments are combined into a single span in the result to reach
 * the number of spans.
 */
Span *
tsequenceset_split_n_spans(const TSequenceSet *ss, int span_count, int *count)
{
  assert(ss); assert(count); assert(span_count > 0);

  /* One bounding span per segment */
  int nspans = (ss->totalcount <= span_count) ? ss->totalcount : span_count;
  Span *result = palloc(sizeof(Span) * nspans);
  if (ss->totalcount <= span_count)
    return tsequenceset_spans(ss, count);

  /* Number of spans per composing sequence determined from the proportion of
   * seq->count and ss->totalcount */
  if (ss->count <= span_count)
  {
    int nspans1 = 0;
    for (int i = 0; i < ss->count; i++)
    {
      const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
      int nspans_seq = (int) (span_count * seq->count * 1.0 / ss->totalcount);
      if (! nspans_seq)
        nspans_seq = 1;
      nspans1 += tcontseq_split_n_spans_iter(seq, nspans_seq,
        &result[nspans1]);
    }
    assert(nspans1 <= span_count);
    *count = nspans1;
    return result;
  }

  /* Merge consecutive sequences to reach the maximum number of spans */
  /* Minimum number of sequences merged together in an output span */
  int size = ss->count / span_count;
  /* Number of output spans that result from merging (size + 1) sequences */
  int remainder = ss->count % span_count;
  int i = 0; /* Loop variable for input sequences */
  for (int k = 0; k < span_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    tcontseq_split_n_spans_iter(TSEQUENCESET_SEQ_N(ss, i), 1, &result[k]);
    if (i < j - 1)
    {
      Span span;
      tcontseq_split_n_spans_iter(TSEQUENCESET_SEQ_N(ss, j - 1), 1, &span);
      span_expand(&span, &result[k]);
    }
    i = j;
  }
  assert(i == ss->count);
  *count = span_count;
  return result;
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of N spans obtained from the instants or segments of
 * a temporal value, where the choice between instants or segments depends,
 * respectively, on whether the interpolation is discrete or continuous
 * @param[in] temp Temporal value
 * @param[in] span_count Number of spans
 * @param[out] count Number of values of the output array
 * @return If the number of instants or segments is <= `span_count`, the result
 * contains one span per instant or segment. Otherwise, consecutive instants or
 * segments are combined into a single span in the result to reach the number
 * of spans. On error return @p NULL
 * @csqlfn #Temporal_split_n_spans()
 */
Span *
temporal_split_n_spans(const Temporal *temp, int span_count, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  if (! ensure_positive(span_count))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tinstant_spans((TInstant *) temp);
    case TSEQUENCE:
      return tsequence_split_n_spans((TSequence *) temp, span_count, count);
    default: /* TSEQUENCESET */
      return tsequenceset_split_n_spans((TSequenceSet *) temp, span_count,
        count);
  }
}

/*****************************************************************************
 * Split_each_n_spans functions
 *****************************************************************************/

/**
 * @brief Return an array of spans obtained by merging consecutive instants
 * from a temporal number sequence with discrete interpolation
 * @param[in] seq Temporal sequence
 * @param[in] elems_per_span Number of instants merged into an output span
 * @param[out] count Number of elements in the output array
 */
static Span *
tdiscseq_split_each_n_spans(const TSequence *seq, int elems_per_span,
  int *count)
{
  assert(seq); assert(count); assert(elems_per_span > 0);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);

  int nspans = ceil((double) seq->count / (double) elems_per_span);
  Span *result = palloc(sizeof(Span) * nspans);
  int k = 0;
  for (int i = 0; i < seq->count; ++i)
  {
    if (i % elems_per_span == 0)
      tinstant_set_span(TSEQUENCE_INST_N(seq, i), &result[k++]);
    else
    {
      Span span;
      tinstant_set_span(TSEQUENCE_INST_N(seq, i), &span);
      span_expand(&span, &result[k - 1]);
    }
  }
  assert(k == nspans);
  *count = k;
  return result;
}

/**
 * @brief Return an array of spans of a temporal number sequence with
 * continuous interpolation obtained by merging consecutive segments
 * (iterator function)
 * @param[in] seq Temporal value
 * @param[in] elems_per_span Number of segments merged into an output span
 * @param[out] result Array of spans
 * @return Number of elements in the output array
 */
static int
tcontseq_split_each_n_spans_iter(const TSequence *seq, int elems_per_span,
  Span *result)
{
  assert(seq); assert(result); assert(elems_per_span > 0);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tsequence_set_tstzspan(seq, &result[0]);
    return 1;
  }

  /* General case */
  int k = 0;
  tinstant_set_span(TSEQUENCE_INST_N(seq, 0), &result[k]);
  for (int i = 1; i < seq->count; ++i)
  {
    Span span;
    tinstant_set_span(TSEQUENCE_INST_N(seq, i), &span);
    span_expand(&span, &result[k]);
    if ((i % elems_per_span == 0) && (i < seq->count - 1))
      result[++k] = span;
  }
  int nspans = ceil((double) (seq->count - 1) / (double) elems_per_span);
  assert(k + 1 == nspans);
  return nspans;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spans of a temporal number sequence obtained
 * by merging consecutive instants or segments, where the choice between
 * instants or segments depends, respectively, on whether the interpolation
 * is discrete or continuous
 * @param[in] seq Temporal sequence
 * @param[in] elems_per_span Number of segments merged into an output span
 * @param[out] count Number of elements in the output array
 */
static Span *
tsequence_split_each_n_spans(const TSequence *seq, int elems_per_span,
  int *count)
{
  assert(seq); assert(count); assert(elems_per_span > 0);

  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return tdiscseq_split_each_n_spans(seq, elems_per_span, count);

  /* Number of instants or segments */
  int nelems = (seq->count == 1) ? 1 : seq->count - 1;
  int nspans = ceil((double) nelems / (double) elems_per_span);
  Span *result = palloc(sizeof(Span) * nspans);
  *count = tcontseq_split_each_n_spans_iter(seq, elems_per_span, result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spans of a temporal number sequence set
 * obtained by merging consecutive segments
 * @param[in] ss Temporal sequence set
 * @param[in] elems_per_span Number of segments merged into an output span
 * @param[out] count Number of elements in the output array
 */
static Span *
tsequenceset_split_each_n_spans(const TSequenceSet *ss, int elems_per_span,
  int *count)
{
  assert(ss); assert(count); assert(elems_per_span > 0);

  /* Singleton sequence set */
  if (ss->count == 1)
    return tsequence_split_each_n_spans(TSEQUENCESET_SEQ_N(ss, 0),
      elems_per_span, count);

  /* Iterate for every composing sequence */
  int nspans = 0;
  Span *result = palloc(sizeof(Span) * ss->totalcount);
  for (int i = 0; i < ss->count; ++i)
    nspans += tcontseq_split_each_n_spans_iter(TSEQUENCESET_SEQ_N(ss, i),
      elems_per_span, &result[nspans]);
  *count = nspans;
  return result;
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of spans obtained from the instants or segments of a
 * temporal value, where the choice between instants or segments depends,
 * respectively, on whether the interpolation is discrete or continuous
 * @param[in] temp Temporal value
 * @param[in] elems_per_span Number of input instants or segments merged into an
 * output span
 * @param[out] count Number of spans of the output array
 * @return If the number of instants or segments is <= `elems_per_span`, the
 * result contains a single span. Otherwise, the number consecutive input
 * instants or segments are combined into a single output span in the result.
 * On error return @p NULL
 * @csqlfn #Temporal_split_each_n_spans()
 */
Span *
temporal_split_each_n_spans(const Temporal *temp, int elems_per_span,
  int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  if (! ensure_positive(elems_per_span))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tinstant_spans((TInstant *) temp);
    case TSEQUENCE:
      return tsequence_split_each_n_spans((TSequence *) temp, elems_per_span,
        count);
    default: /* TSEQUENCESET */
      return tsequenceset_split_each_n_spans((TSequenceSet *) temp,
        elems_per_span, count);
  }
}

/*****************************************************************************
 * Boxes functions
 * These functions can be used for defining Multi-Entry Search Trees indexes
 * https://www.pgcon.org/2014/schedule/events/696.en.html
 * https://github.com/MobilityDB/mest
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return a singleton array of temporal boxes from a temporal number
 * instant
 * @param[in] inst Temporal value
 */
TBox *
tnumberinst_tboxes(const TInstant *inst)
{
  assert(inst); assert(tnumber_type(inst->temptype));
  TBox *result = palloc(sizeof(TBox));
  tnumberinst_set_tbox(inst, &result[0]);
  return result;
}

/**
 * @brief Return an array of temporal boxes obtained from the instants of a
 * temporal number sequence with discrete interpolation
 * @param[in] seq Temporal sequence
 */
static TBox *
tnumberseq_disc_tboxes(const TSequence *seq)
{
  assert(seq); assert(tnumber_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  /* One bounding box per instant */
  TBox *result = palloc(sizeof(TBox) * seq->count);
  for (int i = 0; i < seq->count; i++)
    tnumberinst_set_tbox(TSEQUENCE_INST_N(seq, i), &result[i]);
  return result;
}

/**
 * @brief Return an array of temporal boxes from the segments of a temporal
 * number sequence with continuous interpolation (iterator function)
 * @param[in] seq Temporal value
 * @param[out] result Temporal box
 * @return Number of elements in the array
 */
static int
tnumberseq_cont_tboxes_iter(const TSequence *seq, TBox *result)
{
  assert(seq); assert(result); assert(tnumber_type(seq->temptype));
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
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of temporal boxes obtained from the instants or
 * segments of a temporal number sequence, where the choice between instants or
 * segments depends, respectively, on whether the interpolation is discrete or
 * continuous
 * @param[in] seq Temporal sequence
 * @param[out] count Number of elements in the output array
 */
TBox *
tnumberseq_tboxes(const TSequence *seq, int *count)
{
  assert(seq); assert(count); assert(tnumber_type(seq->temptype));

  /* Discrete case */
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
  {
    *count = seq->count;
    return tnumberseq_disc_tboxes(seq);
  }

  /* Continuous case */
  int nboxes = (seq->count == 1) ? 1 : seq->count - 1;
  TBox *result = palloc(sizeof(TBox) * nboxes);
  *count = tnumberseq_cont_tboxes_iter(seq, result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of temporal boxes obtained from the segments of a
 * temporal number sequence set
 * @param[in] ss Temporal sequence set
 * @param[out] count Number of elements in the output array
 */
TBox *
tnumberseqset_tboxes(const TSequenceSet *ss, int *count)
{
  assert(ss); assert(count); assert(tnumber_type(ss->temptype));
  assert(MEOS_FLAGS_LINEAR_INTERP(ss->flags));
  /* One bounding box per segment */
  TBox *result = palloc(sizeof(TBox) * ss->totalcount);
  int nboxes = 0;
  for (int i = 0; i < ss->count; i++)
    nboxes += tnumberseq_cont_tboxes_iter(TSEQUENCESET_SEQ_N(ss, i),
      &result[nboxes]);
  assert(nboxes <= ss->totalcount);
  *count = nboxes;
  return result;
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of temporal boxes obtained from the instants or
 * segments of a temporal number, where the choice between instants or segments
 * depends on whether the interpolation is discrete or continuous
 * @param[in] temp Temporal value
 * @param[out] count Number of elements in the output array
 * @return On error return @p NULL
 * @csqlfn #Tnumber_tboxes()
 */
TBox *
tnumber_tboxes(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(count, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tnumberinst_tboxes((TInstant *) temp);
    case TSEQUENCE:
      return tnumberseq_tboxes((TSequence *) temp, count);
    default: /* TSEQUENCESET */
      return tnumberseqset_tboxes((TSequenceSet *) temp, count);
  }
}

/*****************************************************************************/

/**
 * @brief Return an array of N temporal boxes from the instants of a temporal
 * number sequence with discrete interpolation
 * @param[in] seq Temporal sequence
 * @param[in] box_count Number of elements in the output array
 * @param[out] count Number of elements in the array
 * @return If the number of instants is <= `box_count`, the result contains one
 * box per instant
 */
static TBox *
tnumberseq_disc_split_n_tboxes(const TSequence *seq, int box_count, int *count)
{
  assert(seq); assert(count); assert(tnumber_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  assert(box_count > 0);

  /* One bounding box per instant */
  if (seq->count <= box_count)
  {
    *count = seq->count;
    return tnumberseq_disc_tboxes(seq);
  }

  /* One bounding box per several consecutive instants */
  TBox *result = palloc(sizeof(TBox) * seq->count);
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
    tnumberinst_set_tbox(TSEQUENCE_INST_N(seq, i), &result[k]);
    for (int l = i + 1; l < j; l++)
    {
      TBox box;
      tnumberinst_set_tbox(TSEQUENCE_INST_N(seq, l), &box);
      tbox_expand(&box, &result[k]);
    }
    i = j;
  }
  assert(i == seq->count);
  *count = box_count;
  return result;
}

/**
 * @brief Return an array of N temporal boxes from the segments of a temporal
 * number sequence with continuous interpolation (iterator function)
 * @param[in] seq Temporal value
 * @param[in] box_count Number of elements in the output array
 * @param[out] result If the number of segments is <= `box_count`, the result
 * contains one box per segment. Otherwise, consecutive segments are combined
 * into a single box in the result to reach the given number of boxes.
 * @return Number of elements in the output array
 */
static int
tnumberseq_cont_split_n_tboxes_iter(const TSequence *seq, int box_count,
  TBox *result)
{
  assert(seq); assert(result); assert(tnumber_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);;
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
    return tnumberseq_cont_tboxes_iter(seq, result);

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
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N temporal boxes from the instants or segments of
 * a temporal number sequence, where the choice between instants or segments
 * depends, respectively, on whether the interpolation is discrete or
 * continuous
 * @param[in] seq Temporal sequence
 * @param[in] box_count Number of elements in the output array
 * @param[out] count Number of elements in the output array
 * @return If the number of instants or segments is <= `box_count`, the result
 * contains one box per instant or segment. Otherwise, consecutive instants or
 * segments are combined into a single box in the result to reach the given
 * number of boxes.
 */
TBox *
tnumberseq_split_n_tboxes(const TSequence *seq, int box_count, int *count)
{
  assert(seq); assert(count); assert(tnumber_type(seq->temptype));
  assert(box_count > 0);

  /* Discrete case */
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return tnumberseq_disc_split_n_tboxes(seq, box_count, count);

  /* Continuous case */
  int nboxes = (seq->count <= box_count) ?
    (seq->count == 1 ? 1 : seq->count - 1) : box_count;
  TBox *result = palloc(sizeof(TBox) * nboxes);
  *count = tnumberseq_cont_split_n_tboxes_iter(seq, box_count, result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N temporal boxes from the segments of a temporal
 * number sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] box_count Number of elements in the output array
 * @param[out] count Number of elements in the output array
 * @return If the number of segments is <= `box_count`, the result contains one
 * box per segment. Otherwise, consecutive segments are combined into a single
 * box in the result to reach the given number of boxes.
 */
TBox *
tnumberseqset_split_n_tboxes(const TSequenceSet *ss, int box_count, int *count)
{
  assert(ss); assert(count); assert(tnumber_type(ss->temptype));
  assert(box_count > 0);

  /* One bounding box per segment */
  int nboxes = (ss->totalcount <= box_count) ? ss->totalcount : box_count;
  TBox *result = palloc(sizeof(TBox) * nboxes);
  if (ss->totalcount <= box_count)
    return tnumberseqset_tboxes(ss, count);

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
      nboxes1 += tnumberseq_cont_split_n_tboxes_iter(seq, nboxes_seq,
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
    tnumberseq_cont_split_n_tboxes_iter(TSEQUENCESET_SEQ_N(ss, i), 1,
      &result[k]);
    for (int l = i + 1; l < j; l++)
    {
      TBox box;
      tnumberseq_cont_split_n_tboxes_iter(TSEQUENCESET_SEQ_N(ss, l), 1, &box);
      tbox_expand(&box, &result[k]);
    }
    i = j;
  }
  assert(i == ss->count);
  *count = box_count;
  return result;
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of N temporal boxes obtained from the instants or
 * segments of a temporal number, where the choice between instants or segments
 * depends, respectively, on whether the interpolation is discrete or continuous
 * @param[in] temp Temporal number
 * @param[in] box_count Number of boxes
 * @param[out] count Number of values of the output array
 * @return On error return @p NULL
 * @csqlfn #Tnumber_split_n_tboxes()
 */
TBox *
tnumber_split_n_tboxes(const Temporal *temp, int box_count, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL);
  if (! ensure_positive(box_count))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tnumberinst_tboxes((TInstant *) temp);
    case TSEQUENCE:
      return tnumberseq_split_n_tboxes((TSequence *) temp, box_count, count);
    default: /* TSEQUENCESET */
      return tnumberseqset_split_n_tboxes((TSequenceSet *) temp, box_count,
        count);
  }
}

/*****************************************************************************
 * split_each_n_tboxes function
 *****************************************************************************/

/**
 * @brief Return an array of temporal boxes obtained from the instants of a
 * temporal number sequence with discrete interpolation
 * @param[in] seq Temporal sequence
 * @param[in] elems_per_box Number of instants merged into an output box
 * @param[out] count Number of elements in the output array
 */
static TBox *
tnumberseq_disc_split_each_n_tboxes(const TSequence *seq, int elems_per_box,
  int *count)
{
  assert(seq); assert(count); assert(tnumber_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  assert(elems_per_box > 0);

  int nboxes = ceil((double) seq->count / (double) elems_per_box);
  TBox *result = palloc(sizeof(TBox) * nboxes);
  int k = 0;
  for (int i = 0; i < seq->count; ++i)
  {
    if (i % elems_per_box == 0)
      tnumberinst_set_tbox(TSEQUENCE_INST_N(seq, i), &result[k++]);
    else
    {
      TBox box;
      tnumberinst_set_tbox(TSEQUENCE_INST_N(seq, i), &box);
      tbox_expand(&box, &result[k - 1]);
    }
  }
  assert(k == nboxes);
  *count = k;
  return result;
}

/**
 * @brief Return an array of temporal boxes obtained from the segments of a
 * temporal number sequence with continuous interpolation (iterator function)
 * @param[in] seq Temporal sequence
 * @param[in] elems_per_box Number of segments merged into an output box
 * @param[out] result Array of temporal boxes
 * @return Number of elements in the output array
 */
static int
tnumberseq_cont_split_each_n_tboxes_iter(const TSequence *seq,
  int elems_per_box, TBox *result)
{
  assert(seq); assert(result); assert(tnumber_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);
  assert(elems_per_box > 0);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tnumberseq_set_tbox(seq, &result[0]);
    return 1;
  }

  /* General case */
  int nboxes = ceil((double) (seq->count - 1) / (double) elems_per_box);
  for (int k = 0; k < nboxes; k++)
  {
    int first = k * elems_per_box;
    int last = Min(first + elems_per_box, seq->count - 1);
    tsequence_set_bbox_slice(seq, first, last, &result[k]);
  }
  return nboxes;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of temporal boxes obtained from the instants or
 * segments of a temporal number sequence, where the choice between instants
 * or segments depends, respectively, on whether the interpolation is discrete
 * or continuous
 * @param[in] seq Temporal sequence
 * @param[in] elems_per_box Number of segments merged into an output box
 * @param[out] count Number of elements in the output array
 */

static TBox *
tnumberseq_split_each_n_tboxes(const TSequence *seq, int elems_per_box,
  int *count)
{
  assert(seq); assert(count); assert(tnumber_type(seq->temptype));
  assert(elems_per_box > 0);

  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return tnumberseq_disc_split_each_n_tboxes(seq, elems_per_box, count);

  /* The number of instants or segments */
  int nelems = (seq->count == 1) ? 1 : seq->count - 1;
  int nboxes = ceil((double) nelems / (double) elems_per_box);
  TBox *result = palloc(sizeof(TBox) * nboxes);
  *count = tnumberseq_cont_split_each_n_tboxes_iter(seq, elems_per_box,
    result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of temporal boxes obtained from the segments of a
 * temporal number sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] elems_per_box Number of segments merged into an output box
 * @param[out] count Number of elements in the output array
 */
static TBox *
tnumberseqset_split_each_n_tboxes(const TSequenceSet *ss, int elems_per_box,
  int *count)
{
  assert(ss); assert(count); assert(tnumber_type(ss->temptype));
  assert(elems_per_box > 0);

  /* Singleton sequence set */
  if (ss->count == 1)
    return tnumberseq_split_each_n_tboxes(TSEQUENCESET_SEQ_N(ss, 0),
      elems_per_box, count);

  /* Iterate for every composing sequence */
  int nboxes = 0;
  TBox *result = palloc(sizeof(TBox) * ss->totalcount);
  for (int i = 0; i < ss->count; ++i)
    nboxes += tnumberseq_cont_split_each_n_tboxes_iter(
      TSEQUENCESET_SEQ_N(ss, i), elems_per_box, &result[nboxes]);
  *count = nboxes;
  return result;
}

/**
 * @ingroup meos_temporal_bbox_split
 * @brief Return an array of temporal boxes obtained from the instants or
 * segments of a temporal number, where the choice between instants or segments
 * depends, respectively, on whether the interpolation is discrete or continuous
 * @param[in] temp Temporal number
 * @param[in] elems_per_box Number of input elements merged in an output box
 * @param[out] count Number of values of the output array
 * @return On error return @p NULL
 * @csqlfn #Tnumber_split_each_n_tboxes()
 */
TBox *
tnumber_split_each_n_tboxes(const Temporal *temp, int elems_per_box, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL);
  if (! ensure_positive(elems_per_box))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tnumberinst_tboxes((TInstant *) temp);
    case TSEQUENCE:
      return tnumberseq_split_each_n_tboxes((TSequence *) temp, elems_per_box,
        count);
    default: /* TSEQUENCESET */
      return tnumberseqset_split_each_n_tboxes((TSequenceSet *) temp,
        elems_per_box, count);
  }
}

/*****************************************************************************
 * Generic bounding box functions for temporal types
 * The inclusive/exclusive bounds are taken into account for the comparisons
 *****************************************************************************/

/**
 * @brief Generic bounding box function for a temporal value and a timestamptz
 * span
 */
bool
boxop_temporal_tstzspan(const Temporal *temp, const Span *s,
  bool (*func)(const Span *, const Span *), bool invert)
{
  assert(temp); assert(s); assert(func);
  Span s1;
  temporal_set_tstzspan(temp, &s1);
  return invert ? func(s, &s1) : func(&s1, s);

}

/**
 * @brief Generic bounding box function for two temporal values
 */
bool
boxop_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const Span *, const Span *))
{
  assert(temp1); assert(temp2); assert(func);
  Span s1, s2;
  temporal_set_tstzspan(temp1, &s1);
  temporal_set_tstzspan(temp2, &s2);
  return func(&s1, &s2);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time of two temporal values actually overlaps
 * @details Unlike @ref overlaps_temporal_temporal, which tests the bounding
 * spans, this walks the actual periods, so two values whose active parts fall
 * in each other's gaps do not count as overlapping. The periods are read in
 * place from the sequences, so there is no allocation.
 * @param[in] temp1,temp2 Temporal values
 */
bool
temporal_time_overlaps(const Temporal *temp1, const Temporal *temp2)
{
  assert(temp1); assert(temp2);
  /* Cheap bounding-span reject first (no allocation). The bounding span is also
   * the exact time of an instant or a single sequence, which have no gaps. */
  Span s1, s2;
  temporal_set_tstzspan(temp1, &s1);
  temporal_set_tstzspan(temp2, &s2);
  if (! overlaps_span_span(&s1, &s2))
    return false;
  if (temp1->subtype != TSEQUENCESET && temp2->subtype != TSEQUENCESET)
    return true;
  /* Walk the actual periods of the sequence set(s) in place. */
  int n1 = (temp1->subtype == TSEQUENCESET) ?
    ((const TSequenceSet *) temp1)->count : 1;
  int n2 = (temp2->subtype == TSEQUENCESET) ?
    ((const TSequenceSet *) temp2)->count : 1;
  for (int i = 0; i < n1; i++)
  {
    const Span *p1 = (temp1->subtype == TSEQUENCESET) ?
      &TSEQUENCESET_SEQ_N((const TSequenceSet *) temp1, i)->period : &s1;
    for (int j = 0; j < n2; j++)
    {
      const Span *p2 = (temp2->subtype == TSEQUENCESET) ?
        &TSEQUENCESET_SEQ_N((const TSequenceSet *) temp2, j)->period : &s2;
      if (overlaps_span_span(p1, p2))
        return true;
    }
  }
  return false;
}

/*****************************************************************************
 * Generic bounding box functions for temporal number types
 *****************************************************************************/

/**
 * @brief Generic bounding box function for a temporal number and a span
 */
bool
boxop_tnumber_numspan(const Temporal *temp, const Span *s,
  bool (*func)(const Span *, const Span *), bool invert)
{
  assert(temp); assert(s); assert(func);
  Span s1;
  tnumber_set_span(temp, &s1);
  return invert ? func(s, &s1) : func(&s1, s);
}

/**
 * @brief Generic bounding box function for a temporal number and a temporal
 * box
 */
bool
boxop_tnumber_tbox(const Temporal *temp, const TBox *box,
  bool (*func)(const TBox *, const TBox *), bool invert)
{
  assert(temp); assert(box); assert(func);
  TBox box1;
  tnumber_set_tbox(temp, &box1);
  return invert ? func(box, &box1) : func(&box1, box);
}

/**
 * @brief Generic bounding box function for two temporal numbers
 */
bool
boxop_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const TBox *, const TBox *))
{
  assert(temp1); assert(temp2); assert(func);
  TBox box1, box2;
  tnumber_set_tbox(temp1, &box1);
  tnumber_set_tbox(temp2, &box2);
  return func(&box1, &box2);
}

/*****************************************************************************/
