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
 * @brief General functions for temporal instant sets.
 */

#include "general/tinstantset.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_call.h"
#include "general/timestampset.h"
#include "general/periodset.h"
#include "general/temporaltypes.h"
#include "general/temporal_parser.h"
#include "general/temporal_util.h"
#include "general/temporal_boxops.h"
#include "point/tpoint_parser.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set from its Well-Known Text (WKT)
 * representation.
 *
 * @param[in] str String
 * @param[in] temptype Temporal type
 */
TSequence *
tinstantset_in(char *str, mobdbType temptype)
{
  return tinstantset_parse(&str, temptype);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set boolean from its Well-Known Text (WKT)
 * representation.
 */
TSequence *
tboolinstset_in(char *str)
{
  return tinstantset_parse(&str, T_TBOOL);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set integer from its Well-Known Text (WKT)
 * representation.
 */
TSequence *
tintinstset_in(char *str)
{
  return tinstantset_parse(&str, T_TINT);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set float from its Well-Known Text (WKT)
 * representation.
 */
TSequence *
tfloatinstset_in(char *str)
{
  return tinstantset_parse(&str, T_TFLOAT);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set text from its Well-Known Text (WKT)
 * representation.
 */
TSequence *
ttextinstset_in(char *str)
{
  return tinstantset_parse(&str, T_TTEXT);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set geometric point from its Well-Known Text
 * (WKT) representation.
 */
TSequence *
tgeompointinstset_in(char *str)
{
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tpoint_parse(&str, T_TGEOMPOINT);
  assert (temp->subtype == TINSTANT);
  return (TSequence *) temp;
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set geographic point from its Well-Known
 * Text (WKT) representation.
 */
TSequence *
tgeogpointinstset_in(char *str)
{
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tpoint_parse(&str, T_TGEOGPOINT);
  assert (temp->subtype == TINSTANTSET);
  return (TSequence *) temp;
}
#endif

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_constructor
 * @brief Construct a temporal instant set from a base value and the time frame
 * of another temporal instant set.
 * @sqlfunc tbool_instset(), tint_instset(), tfloat_instset(), ttext_instset(),
 * etc.
 */
TSequence *
tinstantset_from_base(Datum value, mobdbType temptype, const TSequence *seq)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tinstant_make(value, temptype,
      tsequence_inst_n(seq, i)->t);
  return tsequence_make_free(instants, seq->count, true, true, DISCRETE,
    NORMALIZE_NO);
}

#if MEOS
/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal boolean instant set from a boolean and a
 * timestamp set.
 */
TSequence *
tboolinstset_from_base(bool b, const TSequence *seq)
{
  return tinstantset_from_base(BoolGetDatum(b), T_TBOOL, seq);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal integer instant set from an integer and a
 * timestamp set.
 */
TSequence *
tintinstset_from_base(int i, const TSequence *seq)
{
  return tinstantset_from_base(Int32GetDatum(i), T_TINT, seq);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal float instant set from a float and a
 * timestamp set.
 */
TSequence *
tfloatinstset_from_base(bool b, const TSequence *seq)
{
  return tinstantset_from_base(BoolGetDatum(b), T_TFLOAT, seq);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal text instant set from a text and a timestamp set.
 */
TSequence *
ttextinstset_from_base(const text *txt, const TSequence *seq)
{
  return tinstantset_from_base(PointerGetDatum(txt), T_TTEXT, seq);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geometric point instant set from a point and a
 * timestamp set.
 */
TSequence *
tgeompointinstset_from_base(const GSERIALIZED *gs, const TSequence *seq)
{
  return tinstantset_from_base(PointerGetDatum(gs), T_TGEOMPOINT, seq);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geographic point instant set from a point and a
 * timestamp set.
 */
TSequence *
tgeogpointinstset_from_base(const GSERIALIZED *gs, const TSequence *seq)
{
  return tinstantset_from_base(PointerGetDatum(gs), T_TGEOGPOINT, seq);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_constructor
 * @brief Construct a temporal instant set from a base value and a timestamp set.
 * @sqlfunc tbool_instset(), tint_instset(), tfloat_instset(), ttext_instset(),
 * etc.
 */
TSequence *
tinstantset_from_base_time(Datum value, mobdbType temptype,
  const TimestampSet *ts)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ts->count);
  for (int i = 0; i < ts->count; i++)
    instants[i] = tinstant_make(value, temptype, timestampset_time_n(ts, i));
  return tsequence_make_free(instants, ts->count, true, true, DISCRETE,
    NORMALIZE_NO);
}

#if MEOS
/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal boolean instant set from a boolean and a
 * timestamp set.
 */
TSequence *
tboolinstset_from_base_time(bool b, const TimestampSet *ts)
{
  return tinstantset_from_base_time(BoolGetDatum(b), T_TBOOL, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal integer instant set from an integer and a
 * timestamp set.
 */
TSequence *
tintinstset_from_base_time(int i, const TimestampSet *ts)
{
  return tinstantset_from_base_time(Int32GetDatum(i), T_TINT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal float instant set from a float and a
 * timestamp set.
 */
TSequence *
tfloatinstset_from_base_time(bool b, const TimestampSet *ts)
{
  return tinstantset_from_base_time(BoolGetDatum(b), T_TFLOAT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal text instant set from a text and a timestamp set.
 */
TSequence *
ttextinstset_from_base_time(const text *txt, const TimestampSet *ts)
{
  return tinstantset_from_base_time(PointerGetDatum(txt), T_TTEXT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geometric point instant set from a point and a
 * timestamp set.
 */
TSequence *
tgeompointinstset_from_base_time(const GSERIALIZED *gs, const TimestampSet *ts)
{
  return tinstantset_from_base_time(PointerGetDatum(gs), T_TGEOMPOINT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geographic point instant set from a point and a
 * timestamp set.
 */
TSequence *
tgeogpointinstset_from_base_time(const GSERIALIZED *gs, const TimestampSet *ts)
{
  return tinstantset_from_base_time(PointerGetDatum(gs), T_TGEOGPOINT, ts);
}
#endif /* MEOS */

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * Return the array of base values of a temporal instant set
 *
 * @param[in] seq Temporal instant set
 * @param[out] result Array of base values
 * @result Number of elements in the output array
 */
int
tinstantset_values1(const TSequence *seq, Datum *result)
{
  for (int i = 0; i < seq->count; i++)
    result[i] = tinstant_value(tsequence_inst_n(seq, i));
  if (seq->count == 1)
    return 1;
  mobdbType basetype = temptype_basetype(seq->temptype);
  datumarr_sort(result, seq->count, basetype);
  return datumarr_remove_duplicates(result, seq->count, basetype);
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the array of base values of a temporal instant set.
 * @sqlfunc getValues()
 */
Datum *
tinstantset_values(const TSequence *seq, int *count)
{
  Datum *result = palloc(sizeof(Datum *) * seq->count);
  *count = tinstantset_values1(seq, result);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the array of spans of a temporal instant set float.
 * @sqlfunc getValues()
 */
Span **
tfloatinstset_spans(const TSequence *seq, int *count)
{
  int newcount;
  Datum *values = tinstantset_values(seq, &newcount);
  Span **result = palloc(sizeof(Span *) * newcount);
  for (int i = 0; i < newcount; i++)
    result[i] = span_make(values[i], values[i], true, true, T_FLOAT8);
  pfree(values);
  *count = newcount;
  return result;
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set to (the complement of) a base value.
 *
 * @param[in] seq Temporal instant set
 * @param[in] value Base values
 * @param[in] atfunc True when the restriction is at, false for minus
 * @note There is no bounding box test in this function, it is done in the
 * dispatch function for all temporal types.
 * @sqlfunc atValue(), minusValue()
 */
TSequence *
tinstantset_restrict_value(const TSequence *seq, Datum value, bool atfunc)
{
  mobdbType basetype = temptype_basetype(seq->temptype);

  /* Singleton instant set */
  if (seq->count == 1)
  {
    Datum value1 = tinstant_value(tsequence_inst_n(seq, 0));
    bool equal = datum_eq(value, value1, basetype);
    if ((atfunc && ! equal) || (! atfunc && equal))
      return NULL;
    return tsequence_copy(seq);
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    bool equal = datum_eq(value, tinstant_value(inst), basetype);
    if ((atfunc && equal) || (! atfunc && ! equal))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set to (the complement of) an array of
 * base values.
 *
 * @param[in] seq Temporal instant set
 * @param[in] values Array of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True when the restriction is at, false for minus
 * @pre There are no duplicates values in the array
 * @sqlfunc atValues(), minusValues()
 */
TSequence *
tinstantset_restrict_values(const TSequence *seq, const Datum *values,
  int count, bool atfunc)
{
  const TInstant *inst;

  /* Singleton instant set */
  if (seq->count == 1)
  {
    inst = tsequence_inst_n(seq, 0);
    if (tinstant_restrict_values_test(inst, values, count, atfunc))
      return tsequence_copy(seq);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int newcount = 0;
  for (int i = 0; i < seq->count; i++)
  {
    inst = tsequence_inst_n(seq, i);
    if (tinstant_restrict_values_test(inst, values, count, atfunc))
      instants[newcount++] = inst;
  }
  TSequence *result = (newcount == 0) ? NULL :
    tsequence_make(instants, newcount, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set number to (the complement of) a
 * span of base values.
 *
 * @param[in] seq Temporal number
 * @param[in] span Span of base values
 * @param[in] atfunc True when the restriction is at, false for minus
 * @return Resulting temporal number
 * @note A bounding box test has been done in the dispatch function.
 * @sqlfunc atSpan(), minusSpan()
 */
TSequence *
tnumberinstset_restrict_span(const TSequence *seq, const Span *span,
  bool atfunc)
{
  /* Singleton instant set */
  if (seq->count == 1)
    return atfunc ? tsequence_copy(seq) : NULL;

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    if (tnumberinst_restrict_span_test(inst, span, atfunc))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set number to (the complement of) an
 * array of spans of base values.
 *
 * @param[in] seq Temporal number
 * @param[in] normspans Array of spans of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True when the restriction is at, false for minus
 * @return Resulting temporal number
 * @pre The array of spans is normalized
 * @note A bounding box test has been done in the dispatch function.
 * @sqlfunc atSpans(), minusSpans()
 */
TSequence *
tnumberinstset_restrict_spans(const TSequence *seq, Span **normspans,
  int count, bool atfunc)
{
  const TInstant *inst;

  /* Singleton instant set */
  if (seq->count == 1)
  {
    inst = tsequence_inst_n(seq, 0);
    if (tnumberinst_restrict_spans_test(inst, normspans, count, atfunc))
      return tsequence_copy(seq);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int newcount = 0;
  for (int i = 0; i < seq->count; i++)
  {
    inst = tsequence_inst_n(seq, i);
    if (tnumberinst_restrict_spans_test(inst, normspans, count, atfunc))
      instants[newcount++] = inst;
  }
  TSequence *result = (newcount == 0) ? NULL :
    tsequence_make(instants, newcount, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set to (the complement of) its
 * minimum/maximum base value
 *
 * @param[in] seq Temporal instant set
 * @param[in] min True when restricted to the minumum value, false for the
 * maximum value
 * @param[in] atfunc True when the restriction is at, false for minus
 * @sqlfunc atMin(), atMax(), minusMin(), minusMax()
 */
TSequence *
tinstantset_restrict_minmax(const TSequence *seq, bool min, bool atfunc)
{
  Datum minmax = min ? tsequence_min_value(seq) : tsequence_max_value(seq);
  return tinstantset_restrict_value(seq, minmax, atfunc);
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the base value of a temporal instant set at a timestamp
 *
 * @note In order to be compatible with the corresponding functions for temporal
 * sequences that need to interpolate the value, it is necessary to return
 * a copy of the value
 * @sqlfunc valueAtTimestamp()
 */
bool
tinstantset_value_at_timestamp(const TSequence *seq, TimestampTz t, Datum *result)
{
  int loc = tdiscseq_find_timestamp(seq, t);
  if (loc < 0)
    return false;

  const TInstant *inst = tsequence_inst_n(seq, loc);
  *result = tinstant_value_copy(inst);
  return true;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set to (the complement of) a timestamp.
 *
 * @note In order to be compatible with the corresponding functions for temporal
 * sequences that need to interpolate the value, it is necessary to return
 * a copy of the value
 * @sqlfunc atTimestamp(), minusTimestamp()
 */
Temporal *
tinstantset_restrict_timestamp(const TSequence *seq, TimestampTz t, bool atfunc)
{
  /* Bounding box test */
  if (!contains_period_timestamp(&seq->period, t))
    return atfunc ? NULL : (Temporal *) tsequence_copy(seq);

  /* Singleton instant set */
  if (seq->count == 1)
    return atfunc ? (Temporal *) tinstant_copy(tsequence_inst_n(seq, 0)) : NULL;

  /* General case */
  const TInstant *inst;
  if (atfunc)
  {
    int loc = tdiscseq_find_timestamp(seq, t);
    if (loc < 0)
      return NULL;
    inst = tsequence_inst_n(seq, loc);
    return (Temporal *) tinstant_copy(inst);
  }
  else
  {
    const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
    int count = 0;
    for (int i = 0; i < seq->count; i++)
    {
      inst= tsequence_inst_n(seq, i);
      if (inst->t != t)
        instants[count++] = inst;
    }
    TSequence *result = (count == 0) ? NULL :
      tsequence_make(instants, count, true, true, DISCRETE, NORMALIZE_NO);
    pfree(instants);
    return (Temporal *) result;
  }
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set to (the complement of) a timestamp set.
 * @sqlfunc atTimestampSet(), minusTimestampSet()
 */
TSequence *
tinstantset_restrict_timestampset(const TSequence *seq, const TimestampSet *ts,
  bool atfunc)
{
  TSequence *result;
  const TInstant *inst;

  /* Singleton timestamp set */
  if (ts->count == 1)
  {
    Temporal *temp = tinstantset_restrict_timestamp(seq,
      timestampset_time_n(ts, 0), atfunc);
    if (temp == NULL)
      return (TSequence *) temp;
    TInstant *inst1 = (TInstant *) temp;
    result = tsequence_make((const TInstant **) &inst1, 1, true, true,
      DISCRETE, NORMALIZE_NO);
    pfree(inst1);
    return result;
  }

  /* Bounding box test */
  if (!overlaps_span_span(&seq->period, &ts->period))
    return atfunc ? NULL : tsequence_copy(seq);


  /* Singleton instant set */
  if (seq->count == 1)
  {
    inst = tsequence_inst_n(seq, 0);
    if (tinstant_restrict_timestampset_test(inst, ts, atfunc))
      return tsequence_copy(seq);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int i = 0, j = 0, k = 0;
  while (i < seq->count && j < ts->count)
  {
    inst = tsequence_inst_n(seq, i);
    TimestampTz t = timestampset_time_n(ts, j);
    int cmp = timestamptz_cmp_internal(inst->t, t);
    if (cmp == 0)
    {
      if (atfunc)
        instants[k++] = inst;
      i++;
      j++;
    }
    else if (cmp < 0)
    {
      if (! atfunc)
        instants[k++] = inst;
      i++;
    }
    else
      j++;
  }
  /* For minus copy the instants after the instant set */
  if (! atfunc)
  {
    while (i < seq->count)
      instants[k++] = tsequence_inst_n(seq, i++);
  }
  result = (k == 0) ? NULL : tsequence_make(instants, k, true, true, DISCRETE,
    NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set to (the complement of) a period.
 * @sqlfunc atPeriod(), minusPeriod()
 */
TSequence *
tinstantset_restrict_period(const TSequence *seq, const Period *period,
  bool atfunc)
{
  /* Bounding box test */
  if (!overlaps_span_span(&seq->period, period))
    return atfunc ? NULL : tsequence_copy(seq);

  /* Singleton instant set */
  if (seq->count == 1)
    return atfunc ? tsequence_copy(seq) : NULL;

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    bool contains = contains_period_timestamp(period, inst->t);
    if ((atfunc && contains) || (! atfunc && ! contains))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set to (the complement of) a period set.
 * @sqlfunc atPeriodSet(), minusPeriodSet()
 */
TSequence *
tinstantset_restrict_periodset(const TSequence *seq, const PeriodSet *ps,
  bool atfunc)
{
  const TInstant *inst;

  /* Singleton period set */
  if (ps->count == 1)
    return tinstantset_restrict_period(seq, periodset_per_n(ps, 0), atfunc);

  /* Bounding box test */
  if (!overlaps_span_span(&seq->period, &ps->period))
    return atfunc ? NULL : tsequence_copy(seq);

  /* Singleton instant set */
  if (seq->count == 1)
  {
    inst = tsequence_inst_n(seq, 0);
    if (tinstant_restrict_periodset_test(inst, ps, atfunc))
      return tsequence_copy(seq);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int count = 0;
  for (int i = 0; i < seq->count; i++)
  {
    inst = tsequence_inst_n(seq, i);
    bool contains = contains_periodset_timestamp(ps, inst->t);
    if ((atfunc && contains) || (! atfunc && ! contains))
      instants[count++] = inst;
  }
  TSequence *result = (count == 0) ? NULL :
    tsequence_make(instants, count, true, true, DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/*****************************************************************************
 * Append and merge functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Append an instant to a temporal instant set.
 * @sqlfunc appendInstant()
 */
TSequence *
tinstantset_append_tinstant(const TSequence *seq, const TInstant *inst)
{
  /* Ensure validity of the arguments */
  assert(seq->temptype == inst->temptype);
  const TInstant *inst1 = tsequence_inst_n(seq, seq->count - 1);
  ensure_increasing_timestamps(inst1, inst, MERGE);
  if (inst1->t == inst->t)
    return tsequence_copy(seq);

  /* Create the result */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count + 1);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tsequence_inst_n(seq, i);
  instants[seq->count] = (TInstant *) inst;
  TSequence *result = tsequence_make1(instants, seq->count + 1, true, true,
    DISCRETE, NORMALIZE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Merge two temporal instant sets.
 * @sqlfunc merge()
 */
Temporal *
tinstantset_merge(const TSequence *seq1, const TSequence *seq2)
{
  const TSequence *instsets[] = {seq1, seq2};
  return tinstantset_merge_array(instsets, 2);
}

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Merge an array of temporal instants.
 *
 * @note The function does not assume that the values in the array are strictly
 * ordered on time, i.e., the intersection of the bounding boxes of two values
 * may be a period. For this reason two passes are necessary.
 *
 * @param[in] instsets Array of values
 * @param[in] count Number of elements in the array
 * @result Result value that can be either a temporal instant or a
 * temporal instant set
 * @sqlfunc merge()
 */
Temporal *
tinstantset_merge_array(const TSequence **instsets, int count)
{
  /* Validity test will be done in tinstant_merge_array */
  /* Collect the composing instants */
  int totalcount = 0;
  for (int i = 0; i < count; i++)
    totalcount += instsets[i]->count;
  const TInstant **instants = palloc0(sizeof(TInstant *) * totalcount);
  int k = 0;
  for (int i = 0; i < count; i++)
  {
    for (int j = 0; j < instsets[i]->count; j++)
      instants[k++] = tsequence_inst_n(instsets[i], j);
  }
  /* Create the result */
  Temporal *result = tinstant_merge_array(instants, totalcount);
  pfree(instants);
  return result;
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

/**
 * Temporally intersect a temporal instant set and a temporal instant
 *
 * @param[in] seq,inst Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tinstantset_tinstant(const TSequence *seq, const TInstant *inst,
  TInstant **inter1, TInstant **inter2)
{
  TInstant *inst1 = (TInstant *) tinstantset_restrict_timestamp(seq, inst->t, REST_AT);
  if (inst1 == NULL)
    return false;

  *inter1 = inst1;
  *inter2 = tinstant_copy(inst);
  return true;
}

/**
 * Temporally intersect two temporal instant sets
 *
 * @param[in] inst,seq Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tinstant_tinstantset(const TInstant *inst, const TSequence *seq,
  TInstant **inter1, TInstant **inter2)
{
  return intersection_tinstantset_tinstant(seq, inst, inter2, inter1);
}

/**
 * Temporally intersect two temporal instant sets
 *
 * @param[in] seq1,seq2 Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tinstantset_tinstantset(const TSequence *seq1, const TSequence *seq2,
  TSequence **inter1, TSequence **inter2)
{
  /* Bounding period test */
  if (!overlaps_span_span(&seq1->period, &seq2->period))
    return false;

  int count = Min(seq1->count, seq2->count);
  const TInstant **instants1 = palloc(sizeof(TInstant *) * count);
  const TInstant **instants2 = palloc(sizeof(TInstant *) * count);
  int i = 0, j = 0, k = 0;
  const TInstant *inst1 = tsequence_inst_n(seq1, i);
  const TInstant *inst2 = tsequence_inst_n(seq2, j);
  while (i < seq1->count && j < seq2->count)
  {
    int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      instants1[k] = inst1;
      instants2[k++] = inst2;
      inst1 = tsequence_inst_n(seq1, ++i);
      inst2 = tsequence_inst_n(seq2, ++j);
    }
    else if (cmp < 0)
      inst1 = tsequence_inst_n(seq1, ++i);
    else
      inst2 = tsequence_inst_n(seq2, ++j);
  }
  if (k != 0)
  {
    *inter1 = tsequence_make(instants1, k, true, true, DISCRETE, NORMALIZE_NO);
    *inter2 = tsequence_make(instants2, k, true, true, DISCRETE, NORMALIZE_NO);
  }

  pfree(instants1); pfree(instants2);
  return k != 0;
}

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_agg
 * @brief Return the time-weighted average of a temporal instant set number
 * @note Since an instant set does not have duration, the function returns the
 * traditional average of the values
 * @sqlfunc twAvg()
 */
double
tnumberinstset_twavg(const TSequence *seq)
{
  mobdbType basetype = temptype_basetype(seq->temptype);
  double result = 0.0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    result += datum_double(tinstant_value(inst), basetype);
  }
  return result / seq->count;
}

/*****************************************************************************/
