/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Bounding box operators for temporal types.
 *
 * The bounding box of temporal values are
 * - a `Period` for temporal Booleans
 * - a `TBox` for temporal integers and floats, where the *x* coordinate is for
 *   the value dimension and the *t* coordinate is for the time dimension.
 * The following operators are defined: `overlaps`, `contains`, `contained`,
 * `same`, and `adjacent`.
 *
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the value dimension, only the time dimension, or both
 * the value and the time dimensions.
 */

#include "general/temporal_boxops.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporaltypes.h"
#include "point/tpoint_boxops.h"
#if NPOINT
  #include "npoint/tnpoint_boxops.h"
#endif

/*****************************************************************************
 * Functions on generic bounding boxes of temporal types
 *****************************************************************************/

/**
 * @brief Return true if the type is a bounding box type
 */
bool
bbox_type(meosType bboxtype)
{
  if (bboxtype == T_TSTZSPAN || bboxtype == T_TBOX || bboxtype == T_STBOX)
    return true;
  return false;
}

/**
 * @brief Ensure that the type corresponds to a bounding box type
 */
void
ensure_bbox_type(meosType bboxtype)
{
  if (! bbox_type(bboxtype))
    elog(ERROR, "unknown bounding box type: %d", bboxtype);
  return;
}

/**
 * @brief Return the size of a bounding box type
 */
size_t
bbox_get_size(meosType bboxtype)
{
  ensure_bbox_type(bboxtype);
  if (bboxtype == T_TSTZSPAN)
    return sizeof(Span);
  if (bboxtype == T_TBOX)
    return sizeof(TBox);
  else /* bboxtype == T_STBOX */
    return sizeof(STBox);
}

/**
 * @brief Return the maximum number of dimensions of a bounding box type
 */
int
bbox_max_dims(meosType bboxtype)
{
  ensure_bbox_type(bboxtype);
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
temporal_bbox_eq(const void *box1, const void *box2, meosType temptype)
{
  assert(temporal_type(temptype));
  if (talpha_type(temptype))
    return span_eq((Span *) box1, (Span *) box2);
  if (tnumber_type(temptype))
    return tbox_eq((TBox *) box1, (TBox *) box2);
  if (tspatial_type(temptype))
    // TODO Due to floating point precision the current statement
    // is not equal to the next one.
    // return stbox_eq((STBox *) box1, (STBox *) box2);
    // Problem raised in the test file 51_tpoint_tbl.test.out
    // Look for temp != merge in that file for 2 other cases where
    // a problem still remains (result != 0) even with the _cmp function
    return stbox_cmp((STBox *) box1, (STBox *) box2) == 0;
  elog(ERROR, "unknown temporal type for bounding box function: %d", temptype);
  return false; /* make compiler quiet */
}

/**
 * @brief Return -1, 0, or 1 depending on whether the first bounding box
 * is less than, equal, or greater than the second one
 * @param[in] box1,box2 Bounding boxes
 * @param[in] temptype Temporal type
 */
int
temporal_bbox_cmp(const void *box1, const void *box2, meosType temptype)
{
  assert(temporal_type(temptype));
  if (talpha_type(temptype))
    return span_cmp((Span *) box1, (Span *) box2);
  if (tnumber_type(temptype))
    return tbox_cmp((TBox *) box1, (TBox *) box2);
  if (tspatial_type(temptype))
    return stbox_cmp((STBox *) box1, (STBox *) box2);
  elog(ERROR, "unknown temporal type for bounding box function: %d", temptype);
  return 0; /* make compiler quiet */
}

/*****************************************************************************
 * Compute the bounding box at the creation of temporal values
 *****************************************************************************/

/**
 * @brief Return the size of a bounding box of a temporal type
 */
size_t
temporal_bbox_size(meosType temptype)
{
  if (talpha_type(temptype))
    return sizeof(Span);
  if (tnumber_type(temptype))
    return sizeof(TBox);
  if (tspatial_type(temptype))
    return sizeof(STBox);
  elog(ERROR, "unknown temporal type for bounding box function: %d", temptype);
  return 0; /* make compiler quiet */
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Compute the bounding box of a temporal instant
 * @param[in] box Bounding box
 * @param[in] inst Temporal value
 * @sqlfunc period(), tbox(), stbox()
 * @sqlop @p ::
 */
void
tinstant_set_bbox(const TInstant *inst, void *box)
{
  assert(temporal_type(inst->temptype));
  if (talpha_type(inst->temptype))
    span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
      true, true, T_TIMESTAMPTZ, (Span *) box);
  else if (tnumber_type(inst->temptype))
  {
    meosType basetype = temptype_basetype(inst->temptype);
    Datum value = Float8GetDatum(datum_double(tinstant_value(inst), basetype));
    TBox *tbox = (TBox *) box;
    memset(tbox, 0, sizeof(TBox));
    span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
      true, true, T_TIMESTAMPTZ, &tbox->period);
    /* TBox always has a float span */
    span_set(value, value, true, true, T_FLOAT8, &tbox->span);
    MEOS_FLAGS_SET_X(tbox->flags, true);
    MEOS_FLAGS_SET_T(tbox->flags, true);
  }
  else if (tgeo_type(inst->temptype))
    tpointinst_set_stbox(inst, (STBox *) box);
#if NPOINT
  else if (inst->temptype == T_TNPOINT)
    tnpointinst_set_stbox(inst, (STBox *) box);
#endif
  else
    elog(ERROR, "unknown temporal type for bounding box function: %d",
      inst->temptype);
  return;
}

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
tnumberinstarr_set_tbox(const TInstant **instants, int count, bool lower_inc,
  bool upper_inc, interpType interp, TBox *box)
{
  assert(tnumber_type(instants[0]->temptype));
  meosType basetype = temptype_basetype(instants[0]->temptype);
  /* For discrete or step interpolation the bounds are always inclusive */
  bool lower_inc1 = lower_inc;
  bool upper_inc1 = upper_inc;
  if (interp != LINEAR)
  {
    lower_inc1 = upper_inc1 = true;
  }
  /* Compute the value span */
  Datum min = tinstant_value(instants[0]);
  Datum max = min;
  bool min_inc = lower_inc1, max_inc = lower_inc1;
  for (int i = 1; i < count; i++)
  {
    Datum value = tinstant_value(instants[i]);
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
  /* TBox always has a float span */
  if (basetype == T_INT4)
  {
    min = Float8GetDatum((double) DatumGetInt32(min));
    max = Float8GetDatum((double) DatumGetInt32(max));
  }
  if (datum_eq(min, max, T_FLOAT8))
  {
    min_inc = max_inc = true;
  }
  span_set(min, max, min_inc, max_inc, T_FLOAT8, &box->span);
  /* Compute the time span */
  span_set(TimestampTzGetDatum(instants[0]->t),
    TimestampTzGetDatum(instants[count - 1]->t), lower_inc, upper_inc,
    T_TIMESTAMPTZ, &box->period);
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
tinstarr_compute_bbox(const TInstant **instants, int count, bool lower_inc,
  bool upper_inc, interpType interp, void *box)
{
  meosType temptype = instants[0]->temptype;
  assert(temporal_type(temptype));
  if (talpha_type(temptype))
    span_set(TimestampTzGetDatum(instants[0]->t),
      TimestampTzGetDatum(instants[count - 1]->t), lower_inc, upper_inc,
      T_TIMESTAMPTZ, (Span *) box);
  else if (tnumber_type(temptype))
    tnumberinstarr_set_tbox(instants, count, lower_inc, upper_inc,
      interp, (TBox *) box);
  else if (tgeo_type(temptype))
    tpointinstarr_set_stbox(instants, count, (STBox *) box);
#if NPOINT
  else if (temptype == T_TNPOINT)
    tnpointinstarr_set_stbox(instants, count, interp, (STBox *) box);
#endif
  else
    elog(ERROR, "unknown temporal type for bounding box function: %d",
      temptype);
  /* Set the lower_inc and upper_inc bounds of the period at the beginning
   * of the bounding box */
  Span *p = (Span *) box;
  p->lower_inc = lower_inc;
  p->upper_inc = upper_inc;
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
  tinstant_set_bbox(inst, &box);
  tbox_expand(&box, (TBox *) TSEQUENCE_BBOX_PTR(seq));
  return;
}

/**
 * @brief Expand the bounding box of a temporal sequence with an additional instant
 * @param[inout] seq Temporal sequence
 * @param[in] inst Temporal instant
 */
void
tsequence_expand_bbox(TSequence *seq, const TInstant *inst)
{
  assert(temporal_type(seq->temptype));
  if (talpha_type(seq->temptype))
    span_set(TimestampTzGetDatum((TSEQUENCE_INST_N(seq, 0))->t),
      TimestampTzGetDatum(inst->t), seq->period.lower_inc, true, T_TIMESTAMPTZ,
      (Span *) TSEQUENCE_BBOX_PTR(seq));
  else if (tnumber_type(seq->temptype))
    tnumberseq_expand_tbox(seq, inst);
  else if (tgeo_type(seq->temptype))
    tpointseq_expand_stbox(seq, inst);
#if NPOINT
  else if (seq->temptype == T_TNPOINT)
    tnpointseq_expand_stbox(seq, inst);
#endif
  else
    elog(ERROR, "unknown temporal type for bounding box function: %d",
      seq->temptype);
  return;
}

/**
 * @brief Expand the bounding box of a temporal sequence set with an additional
 * sequence
 * @param[inout] ss Temporal sequence set
 * @param[in] seq Temporal sequence
 */
void
tsequenceset_expand_bbox(TSequenceSet *ss, const TSequence *seq)
{
  assert(temporal_type(ss->temptype));
  if (talpha_type(ss->temptype))
    span_expand(&seq->period, &ss->period);
  else if (tnumber_type(ss->temptype))
    tbox_expand((TBox *) TSEQUENCE_BBOX_PTR(seq),
      (TBox *) TSEQUENCE_BBOX_PTR(ss));
  // TODO Generalize as for tgeogpointseq_expand_stbox
  else if (tspatial_type(ss->temptype))
    stbox_expand((STBox *) TSEQUENCE_BBOX_PTR(seq),
      (STBox *) TSEQUENCE_BBOX_PTR(ss));
  else
    elog(ERROR, "unknown temporal type for bounding box function: %d",
      ss->temptype);
  return;
}

/**
 * @brief Set the period from the array of temporal sequence values
 * @param[in] period Period
 * @param[in] sequences Temporal instants
 * @param[in] count Number of elements in the array
 */
static void
tseqarr_set_period(const TSequence **sequences, int count, Span *period)
{
  const Span *first = &sequences[0]->period;
  const Span *last = &sequences[count - 1]->period;
  span_set(first->lower, last->upper, first->lower_inc, last->upper_inc,
    T_TIMESTAMPTZ, period);
  return;
}

/**
 * @brief Set the temporal box from the array of temporal number sequence values
 * @param[in] box Box
 * @param[in] sequences Temporal instants
 * @param[in] count Number of elements in the array
 */
static void
tnumberseqarr_set_tbox(const TSequence **sequences, int count, TBox *box)
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
 * @brief Set the bounding box from the array of temporal sequence values
 */
void
tseqarr_compute_bbox(const TSequence **sequences, int count, void *box)
{
  assert(temporal_type(sequences[0]->temptype));
  if (talpha_type(sequences[0]->temptype))
    tseqarr_set_period(sequences, count, (Span *) box);
  else if (tnumber_type(sequences[0]->temptype))
    tnumberseqarr_set_tbox(sequences, count, (TBox *) box);
  else if (tspatial_type(sequences[0]->temptype))
    tpointseqarr_set_stbox(sequences, count, (STBox *) box);
  else
    elog(ERROR, "unknown temporal type for bounding box function: %d",
      sequences[0]->temptype);
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
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = TSEQUENCE_INST_N(seq, i);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  tinstarr_compute_bbox(instants, seq->count, seq->period.lower_inc,
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
  const TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = TSEQUENCESET_SEQ_N(ss, i);
  tseqarr_compute_bbox(sequences, ss->count, TSEQUENCESET_BBOX_PTR(ss));
  pfree(sequences);
  return;
}
#endif /* MEOS */

/*****************************************************************************/
