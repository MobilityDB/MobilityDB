/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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

/* MobilityDB */
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
 * Return true if the type is a bounding box type
 */
bool
bbox_type(mobdbType bboxtype)
{
  if (bboxtype == T_PERIOD || bboxtype == T_TBOX || bboxtype == T_STBOX)
    return true;
  return false;
}

/**
 * Ensure that the type corresponds to a bounding box type
 */
void
ensure_bbox_type(mobdbType bboxtype)
{
  if (! bbox_type(bboxtype))
    elog(ERROR, "unknown bounding box type: %d", bboxtype);
  return;
}

/**
 * Return the size of a bounding box type
 */
size_t
bbox_get_size(mobdbType bboxtype)
{
  ensure_bbox_type(bboxtype);
  if (bboxtype == T_PERIOD)
    return sizeof(Period);
  if (bboxtype == T_TBOX)
    return sizeof(TBox);
  else /* bboxtype == T_STBOX */
    return sizeof(STBox);
}

/**
 * Return the maximum number of dimensions of a bounding box type
 */
int
bbox_max_dims(mobdbType bboxtype)
{
  ensure_bbox_type(bboxtype);
  if (bboxtype == T_PERIOD)
    return 1;
  if (bboxtype == T_TBOX)
    return 2;
  else /* bboxtype == T_STBOX */
    return 4;
}

/**
 * Return true if two bounding boxes are equal
 *
 * @param[in] box1,box2 Bounding boxes
 * @param[in] temptype Temporal type
 */
bool
temporal_bbox_eq(const void *box1, const void *box2, mobdbType temptype)
{
  /* Only external types have bounding box */
  ensure_temporal_type(temptype);
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
  elog(ERROR, "unknown bounding box function for temporal type: %d", temptype);
}

/**
 * Return -1, 0, or 1 depending on whether the first bounding box
 * is less than, equal, or greater than the second one
 *
 * @param[in] box1,box2 Bounding boxes
 * @param[in] temptype Temporal type
 */
int
temporal_bbox_cmp(const void *box1, const void *box2, mobdbType temptype)
{
  /* Only external types have bounding box */
  ensure_temporal_type(temptype);
  if (talpha_type(temptype))
    return span_cmp((Span *) box1, (Span *) box2);
  if (tnumber_type(temptype))
    return tbox_cmp((TBox *) box1, (TBox *) box2);
  if (tspatial_type(temptype))
    return stbox_cmp((STBox *) box1, (STBox *) box2);
  elog(ERROR, "unknown bounding box function for temporal type: %d", temptype);
}

/**
 * Shift and/or scale the time span of the bounding box by the intervals
 *
 * @param[in] box Bounding box
 * @param[in] shift Interval to shift
 * @param[in] duration Interval to scale
 * @param[in] temptype Temporal type
 */
void
temporal_bbox_shift_tscale(void *box, mobdbType temptype, const Interval *shift,
  const Interval *duration)
{
  ensure_temporal_type(temptype);
  if (talpha_type(temptype))
    period_shift_tscale((Period *) box, shift, duration);
  else if (tnumber_type(temptype))
    tbox_shift_tscale((TBox *) box, shift, duration);
  else if (tspatial_type(temptype))
    stbox_shift_tscale((STBox *) box, shift, duration);
  else
    elog(ERROR, "unknown bounding box function for temporal type: %d",
      temptype);
  return;
}

/*****************************************************************************
 * Compute the bounding box at the creation of temporal values
 * Only external types have precomputed bbox, internal types such as double2,
 * double3, or double4 do not have precomputed bounding box.
 *****************************************************************************/

/**
 * Return true if a temporal type does not have bounding box
 */
static bool
temptype_without_bbox(mobdbType temptype)
{
  if (temptype == T_TDOUBLE2 || temptype == T_TDOUBLE3 ||
      temptype == T_TDOUBLE4)
    return true;
  return false;
}

/**
 * Return the size of a bounding box of a temporal type
 */
size_t
temporal_bbox_size(mobdbType temptype)
{
  if (talpha_type(temptype))
    return sizeof(Period);
  if (tnumber_type(temptype))
    return sizeof(TBox);
  if (tspatial_type(temptype))
    return sizeof(STBox);
  /* Types without bounding box, such as tdoubleN, must be explicity stated */
  if (temptype_without_bbox(temptype))
    return 0;
  elog(ERROR, "unknown temporal_bbox_size function for temporal type: %d",
    temptype);
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Set the second argument to the bounding box of a temporal instant
 *
 * @param[in] box Bounding box
 * @param[in] inst Temporal value
 * @sqlfunc period(), tbox(), stbox()
 * @sqlop @p ::
 */
void
tinstant_set_bbox(const TInstant *inst, void *box)
{
  /* Only external types have bounding box */
  ensure_temporal_type(inst->temptype);
  if (talpha_type(inst->temptype))
    span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
      true, true, T_TIMESTAMPTZ, (Span *) box);
  else if (tnumber_type(inst->temptype))
  {
    Datum value;
    if (inst->temptype == T_TINT)
      value = Float8GetDatum(tnumberinst_double(inst));
    else /* inst->temptype == T_TFLOAT */
      value = tinstant_value(inst);
    TBox *tbox = (TBox *) box;
    memset(tbox, 0, sizeof(TBox));
    span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
      true, true, T_TIMESTAMPTZ, &tbox->period);
    /* TBox always has a float span */
    span_set(value, value, true, true, T_FLOAT8, &tbox->span);
    MOBDB_FLAGS_SET_X(tbox->flags, true);
    MOBDB_FLAGS_SET_T(tbox->flags, true);
  }
  else if (tgeo_type(inst->temptype))
    tpointinst_set_stbox(inst, (STBox *) box);
#if NPOINT
  else if (inst->temptype == T_TNPOINT)
    tnpointinst_set_stbox(inst, (STBox *) box);
#endif
  else
    elog(ERROR, "unknown bounding box function for temporal type: %d",
      inst->temptype);
  return;
}

/**
 * Set a temporal box from an array of temporal number instants
 *
 * @param[in] instants Temporal instants
 * @param[in] count Number of elements in the array
 * @param[in] box Box
 */
static void
tnumberinstarr_set_tbox(const TInstant **instants, int count, TBox *box)
{
  tinstant_set_bbox(instants[0], box);
  for (int i = 1; i < count; i++)
  {
    TBox box1;
    tinstant_set_bbox(instants[i], &box1);
    tbox_expand(&box1, box);
  }
  return;
}

/**
 * Set a bounding box from an array of temporal instant values
 *
 * @param[in] instants Temporal instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc,upper_inc Period bounds
 * @param[in] interp Interpolation
 * @param[out] box Bounding box
 */
void
tinstarr_compute_bbox(const TInstant **instants, int count, bool lower_inc,
#if NPOINT
  bool upper_inc, interpType interp, void *box)
#else
  bool upper_inc, interpType interp __attribute__((unused)), void *box)
#endif
{
  /* Only external types have bounding box */
  ensure_temporal_type(instants[0]->temptype);
  if (talpha_type(instants[0]->temptype))
    span_set(TimestampTzGetDatum(instants[0]->t),
      TimestampTzGetDatum(instants[count - 1]->t), lower_inc, upper_inc,
      T_TIMESTAMPTZ, (Span *) box);
  else if (tnumber_type(instants[0]->temptype))
    tnumberinstarr_set_tbox(instants, count, (TBox *) box);
  else if (instants[0]->temptype == T_TGEOMPOINT)
    tgeompointinstarr_set_stbox(instants, count, (STBox *) box);
  else if (instants[0]->temptype == T_TGEOGPOINT)
    tgeogpointinstarr_set_stbox(instants, count, interp, (STBox *) box);
#if NPOINT
  else if (instants[0]->temptype == T_TNPOINT)
    tnpointinstarr_set_stbox(instants, count, interp, (STBox *) box);
#endif
  else
    elog(ERROR, "unknown bounding box function for temporal type: %d",
      instants[0]->temptype);
  /* Set the lower_inc and upper_inc bounds of the period at the beginning
   * of the bounding box */
  Period *p = (Period *) box;
  p->lower_inc = lower_inc;
  p->upper_inc = upper_inc;
  return;
}

#if MEOS
/**
 * Expand the bounding box of a temporal number sequence with an instant
 *
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
 * Expand the bounding box of a temporal sequence with an additional instant
 *
 * @param[inout] seq Temporal sequence
 * @param[in] inst Temporal instant
 */
void
tsequence_expand_bbox(TSequence *seq, const TInstant *inst)
{
  /* Only external types have bounding box */
  ensure_temporal_type(seq->temptype);
  if (talpha_type(seq->temptype))
    span_set(TimestampTzGetDatum(tsequence_inst_n(seq, 0)->t),
      TimestampTzGetDatum(inst->t), seq->period.lower_inc, true, T_TIMESTAMPTZ,
      (Span *) TSEQUENCE_BBOX_PTR(seq));
  else if (tnumber_type(seq->temptype))
    tnumberseq_expand_tbox(seq, inst);
  else if (seq->temptype == T_TGEOMPOINT)
    tgeompointseq_expand_stbox(seq, inst);
  else if (seq->temptype == T_TGEOGPOINT)
    tgeogpointseq_expand_stbox(seq, inst);
#if NPOINT
  else if (seq->temptype == T_TNPOINT)
    tnpointseq_expand_stbox(seq, (STBox *) TSEQUENCE_BBOX_PTR(seq));
#endif
  else
    elog(ERROR, "unknown bounding box function for temporal type: %d",
      seq->temptype);
  return;
}

/**
 * Expand the bounding box of a temporal sequence set with an additional sequence
 *
 * @param[inout] ss Temporal sequence set
 * @param[in] seq Temporal sequence
 */
void
tsequenceset_expand_bbox(TSequenceSet *ss, const TSequence *seq)
{
  /* Only external types have bounding box */
  ensure_temporal_type(ss->temptype);
  if (talpha_type(ss->temptype))
    span_expand(&seq->period, &ss->period);
  else if (tnumber_type(ss->temptype))
    tbox_expand((TBox *) TSEQUENCE_BBOX_PTR(seq),
      (TBox *) TSEQUENCE_BBOX_PTR(ss));
  // TODO Generalize as for tgeogpointseq_expand_stbox
  else if (ss->temptype == T_TGEOMPOINT || ss->temptype == T_TGEOGPOINT)
    stbox_expand((STBox *) TSEQUENCE_BBOX_PTR(seq),
      (STBox *) TSEQUENCE_BBOX_PTR(ss));
#if NPOINT
  else if (ss->temptype == T_TNPOINT)
    tnpointseqset_expand_stbox(ss, (STBox *) TSEQUENCE_BBOX_PTR(seq));
#endif
  else
    elog(ERROR, "unknown bounding box function for temporal type: %d",
      ss->temptype);
  return;
}
#endif /* MEOS */

/**
 * Set the period from the array of temporal sequence values
 *
 * @param[in] period Period
 * @param[in] sequences Temporal instants
 * @param[in] count Number of elements in the array
 */
static void
tseqarr_set_period(const TSequence **sequences, int count, Period *period)
{
  const Period *first = &sequences[0]->period;
  const Period *last = &sequences[count - 1]->period;
  span_set(first->lower, last->upper, first->lower_inc, last->upper_inc,
    T_TIMESTAMPTZ, period);
  return;
}

/**
 * Set the temporal box from the array of temporal number sequence values
 *
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
 * Set the bounding box from the array of temporal sequence values
 */
void
tsequenceset_compute_bbox(const TSequence **sequences, int count, void *box)
{
  /* Only external types have bounding box */ // TODO
  ensure_temporal_type(sequences[0]->temptype);
  if (talpha_type(sequences[0]->temptype))
    tseqarr_set_period(sequences, count, (Period *) box);
  else if (tnumber_type(sequences[0]->temptype))
    tnumberseqarr_set_tbox(sequences, count, (TBox *) box);
  else if (tspatial_type(sequences[0]->temptype))
    tpointseqarr_set_stbox(sequences, count, (STBox *) box);
  else
    elog(ERROR, "unknown bounding box function for temporal type: %d",
      sequences[0]->temptype);
  return;
}

/*****************************************************************************
 * Bounding box operators for temporal types: Generic functions
 * The inclusive/exclusive bounds are taken into account for the comparisons
 *****************************************************************************/

/**
 * Generic bounding box operator for a temporal value and a timestamp.
 *
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] func Bounding box function
 * @param[in] invert True if the timestamp is the first argument of the
 * function
 */
Datum
boxop_temporal_timestamp(const Temporal *temp, TimestampTz t,
  bool (*func)(const Period *, const Period *), bool invert)
{
  Period p1, p2;
  temporal_set_period(temp, &p1);
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, &p2);
  bool result = invert ? func(&p2, &p1) : func(&p1, &p2);
  return result;
}

/**
 * Generic bounding box operator for a period and a temporal value.
 *
 * @param[in] temp Temporal value
 * @param[in] ts Timestamp set
 * @param[in] func Bounding box function
 * @param[in] invert True if the timestamp set is the first argument of the
 * function
 */
Datum
boxop_temporal_timestampset(const Temporal *temp, const TimestampSet *ts,
  bool (*func)(const Period *, const Period *), bool invert)
{
  Period p1, p2;
  temporal_set_period(temp, &p1);
  set_set_span(ts, &p2);
  bool result = invert ? func(&p2, &p1) : func(&p1, &p2);
  return result;
}

/**
 * Generic bounding box operator for a period and a temporal value.
 *
 * @param[in] temp Temporal value
 * @param[in] p Period
 * @param[in] func Bounding box function
 * @param[in] invert True if the period is the first argument of the
 * function
 */
Datum
boxop_temporal_period(const Temporal *temp, const Period *p,
  bool (*func)(const Period *, const Period *), bool invert)
{
  Period p1;
  temporal_set_period(temp, &p1);
  bool result = invert ? func(p, &p1) : func(&p1, p);
  return result;
}

/**
 * Generic bounding box operator for a temporal value and a periodset
 *
 * @param[in] temp Temporal value
 * @param[in] ps Period set
 * @param[in] func Bounding box function
 * @param[in] invert True if the period set is the first argument of the
 * function
 */
bool
boxop_temporal_periodset(const Temporal *temp, const PeriodSet *ps,
  bool (*func)(const Period *, const Period *), bool invert)
{
  Period p;
  temporal_set_period(temp, &p);
  bool result = invert ? func(&ps->span, &p) : func(&p, &ps->span);
  return result;
}

/**
 * Generic bounding box operator for two temporal values
 *
 * @param[in] temp1,temp2 Temporal values
 * @param[in] func Bounding box function
 */
bool
boxop_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const Period *, const Period *))
{
  Period p1, p2;
  temporal_set_period(temp1, &p1);
  temporal_set_period(temp2, &p2);
  bool result = func(&p1, &p2);
  return result;
}

/*****************************************************************************
 * Bounding box operators for temporal number types: Generic functions
 *****************************************************************************/

/**
 * Generic bounding box operator for a temporal number and a number
 *
 * @param[in] temp Temporal number
 * @param[in] number Type
 * @param[in] basetype Base type value
 * @param[in] func Bounding box function
 * @param[in] invert True if the base value is the first argument of the
 * function
 */
bool
boxop_tnumber_number(const Temporal *temp, Datum number, mobdbType basetype,
  bool (*func)(const TBox *, const TBox *), bool invert)
{
  TBox box1, box2;
  temporal_set_bbox(temp, &box1);
  number_set_tbox(number, basetype, &box2);
  bool result = invert ? func(&box2, &box1) : func(&box1, &box2);
  return result;
}

/**
 * Generic bounding box operator for a temporal number and a span
 *
 * @param[in] temp Temporal number
 * @param[in] span Span
 * @param[in] func Bounding box function
 * @param[in] invert True if the span is the first argument of the function.
 */
bool
boxop_tnumber_numspan(const Temporal *temp, const Span *span,
  bool (*func)(const TBox *, const TBox *), bool invert)
{
  TBox box1, box2;
  temporal_set_bbox(temp, &box1);
  numspan_set_tbox(span, &box2);
  bool result = invert ? func(&box2, &box1) : func(&box1, &box2);
  return (result);
}

/**
 * Generic bounding box operator for a temporal number and a span
 *
 * @param[in] temp Temporal number
 * @param[in] ss Span set
 * @param[in] func Bounding box function
 * @param[in] invert True if the span is the first argument of the function.
 */
bool
boxop_tnumber_numspanset(const Temporal *temp, const SpanSet *ss,
  bool (*func)(const TBox *, const TBox *), bool invert)
{
  TBox box1, box2;
  temporal_set_bbox(temp, &box1);
  numspanset_set_tbox(ss, &box2);
  bool result = invert ? func(&box2, &box1) : func(&box1, &box2);
  return (result);
}

/**
 * Generic bounding box operator for a temporal number and a temporal box
 *
 * @param[in] temp Temporal number
 * @param[in] box Bounding box
 * @param[in] invert True if the bounding box is the first argument of the
 * function
 * @param[in] func Bounding box function
 */
bool
boxop_tnumber_tbox(const Temporal *temp, const TBox *box,
  bool (*func)(const TBox *, const TBox *), bool invert)
{
  TBox box1;
  temporal_set_bbox(temp, &box1);
  bool result = invert ? func(box, &box1) : func(&box1, box);
  return result;
}

/**
 * Generic bounding box operator for two temporal numbers
 *
 * @param[in] temp1,temp2 Temporal numbers
 * @param[in] func Bounding box function
 */
bool
boxop_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const TBox *, const TBox *))
{
  TBox box1, box2;
  temporal_set_bbox(temp1, &box1);
  temporal_set_bbox(temp2, &box2);
  bool result = func(&box1, &box2);
  return result;
}

/*****************************************************************************/
