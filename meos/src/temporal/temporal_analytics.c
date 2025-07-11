/***********************************************************************
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
 * @brief Analytics function for temporal types
 */

#include "temporal/temporal_analytics.h"

/* C */
#include <assert.h>
#include <math.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/postgres_types.h"
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/temporal_tile.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h"
#include "geo/tgeo_distance.h"
#include "geo/tgeo_spatialfuncs.h"

/*****************************************************************************
 * Time precision functions for time values
 *****************************************************************************/

/**
 * @ingroup meos_setspan_transf
 * @brief Return a timestamptz with the precision set to a time bin
 * @param[in] t Time value
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 */
TimestampTz
timestamptz_tprecision(TimestampTz t, const Interval *duration,
  TimestampTz torigin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(duration, DT_NOEND);
  if (! ensure_positive_duration(duration))
    return DT_NOEND;
  return timestamptz_get_bin(t, duration, torigin);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a timestamptz set with the precision set to a time bin
 * @param[in] s Timestamptz set
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 */
Set *
tstzset_tprecision(const Set *s, const Interval *duration, TimestampTz torigin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_duration(duration))
    return NULL;

  Datum *values = palloc(sizeof(Datum) * s->count);
  /* Loop for each value */
  for (int i = 0; i < s->count; i++)
    values[i] = timestamptz_get_bin(SET_VAL_N(s, i), duration, torigin);
  return set_make_free(values, s->count, T_TIMESTAMPTZ, ORDER);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a timestamptz span with the precision set to a time bin
 * @param[in] s Time value
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 */
Span *
tstzspan_tprecision(const Span *s, const Interval *duration,
  TimestampTz torigin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, NULL); VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_duration(duration))
    return NULL;

  int64 tunits = interval_units(duration);
  TimestampTz lower = DatumGetTimestampTz(s->lower);
  TimestampTz upper = DatumGetTimestampTz(s->upper);
  TimestampTz lower_bin = timestamptz_get_bin(lower, duration, torigin);
  /* We need to add tunits to obtain the end timestamptz of the last bin */
  TimestampTz upper_bin = timestamptz_get_bin(upper, duration, torigin) +
    tunits;
  return span_make(TimestampTzGetDatum(lower_bin),
    TimestampTzGetDatum(upper_bin), true, false, T_TIMESTAMPTZ);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a timestamptz span set with the precision set to a time bin
 * @param[in] ss Time value
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 */
SpanSet *
tstzspanset_tprecision(const SpanSet *ss, const Interval *duration,
  TimestampTz torigin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL); VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_duration(duration))
    return NULL;

  int64 tunits = interval_units(duration);
  TimestampTz lower = DatumGetTimestampTz(ss->span.lower);
  TimestampTz upper = DatumGetTimestampTz(ss->span.upper);
  TimestampTz lower_bin = timestamptz_get_bin(lower, duration, torigin);
  /* We need to add tunits to obtain the end timestamptz of the last bin */
  TimestampTz upper_bin = timestamptz_get_bin(upper, duration, torigin) +
    tunits;
  /* Number of bins */
  int count = (int) (((int64) upper_bin - (int64) lower_bin) / tunits);
  Span *spans = palloc(sizeof(Span) * count);
  lower = lower_bin;
  upper = lower_bin + tunits;
  int nspans = 0;
  /* Loop for each bin */
  for (int i = 0; i < count; i++)
  {
    Span s;
    span_set(TimestampTzGetDatum(lower),TimestampTzGetDatum(upper),
      true, false, T_TIMESTAMPTZ, T_TSTZSPAN, &s);
    if (overlaps_spanset_span(ss, &s))
      spans[nspans++] = s;
    lower += tunits;
    upper += tunits;
  }
  return spanset_make_free(spans, nspans, NORMALIZE, ORDER_NO);
}

/*****************************************************************************
 * Time precision functions for temporal values
 *****************************************************************************/

/**
 * @brief Return a temporal instant with the precision set to a time bin
 * @param[in] inst Temporal value
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 */
TInstant *
tinstant_tprecision(const TInstant *inst, const Interval *duration,
  TimestampTz torigin)
{
  assert(inst); assert(duration); assert(positive_duration(duration));
  TimestampTz lower = timestamptz_get_bin(inst->t, duration, torigin);
  return tinstant_make(tinstant_value_p(inst), inst->temptype, lower);
}

/**
 * @brief Return a temporal sequence with the precision set to a time bin
 * @param[in] seq Temporal value
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 */
TSequence *
tsequence_tprecision(const TSequence *seq, const Interval *duration,
  TimestampTz torigin)
{
  assert(seq); assert(duration); assert(positive_duration(duration));
  assert(seq->temptype == T_TINT || seq->temptype == T_TFLOAT ||
    seq->temptype == T_TGEOMPOINT || seq->temptype == T_TGEOGPOINT ||
    seq->temptype == T_TGEOMETRY || seq->temptype == T_TGEOGRAPHY );

  int64 tunits = interval_units(duration);
  TimestampTz lower = DatumGetTimestampTz(seq->period.lower);
  TimestampTz upper = DatumGetTimestampTz(seq->period.upper);
  TimestampTz lower_bin = timestamptz_get_bin(lower, duration, torigin);
  /* We need to add tunits to obtain the end timestamp of the last bin */
  TimestampTz upper_bin = timestamptz_get_bin(upper, duration, torigin) +
    tunits;
  /* Number of bins */
  int count = (int) (((int64) upper_bin - (int64) lower_bin) / tunits);
  TInstant **ininsts = palloc(sizeof(TInstant *) * seq->count);
  TInstant **outinsts = palloc(sizeof(TInstant *) * count);
  lower = lower_bin;
  upper = lower_bin + tunits;
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  meosType temptype_out = (seq->temptype == T_TINT) ? T_TFLOAT : seq->temptype;
  /* Determine whether we are computing the twAvg or the twCentroid */
  bool twavg = tnumber_type(seq->temptype);
  /* New instants computing the value at the beginning/end of the bin */
  TInstant *start = NULL, *end = NULL;
  /* Sequence for computing the twAvg/twCentroid of each bin */
  TSequence *seq1;
  Datum value;
  int i = 0;   /* Instant of the input sequence being processed */
  int k = 0;   /* Number of instants for computing the twAvg/twCentroid */
  int l = 0;   /* Number of instants of the output sequence */
  /* Loop for each instant of the sequence */
  while (i < seq->count)
  {
    /* Get the next instant */
    TInstant *inst = (TInstant *) TSEQUENCE_INST_N(seq, i);
    int cmp = timestamptz_cmp_internal(inst->t, upper);
    if (cmp < 0)
    {
      /* ACCUMULATE the instant since it is WITHIN the current bin */
      ininsts[k++] = inst;
      i++;
    }
    else
    {
      /* We have reached the end of the bin or beyond: Add or compute the value
       * the end of the bin ONLY for continuous interpolation */
      if (k > 0)
      {
        if (interp == STEP)
        {
          /* For STEP interpolation ALWAYS generate the end of bin instant */
          value = ininsts[k - 1]->value;
          ininsts[k++] = end = tinstant_make(value, seq->temptype, upper);
        }
        else if (interp == LINEAR)
        {
          /* For LINEAR interpolation generate the end of bin instant ONLY if 
           * the last instant read is beyond the end of the bin */
          if (cmp == 0)
            ininsts[k++] = inst;
          else
          {
            tsequence_value_at_timestamptz(seq, upper, false, &value);
            ininsts[k++] = end = tinstant_make_free(value, seq->temptype, upper);
          }
        }
        /* Construct the sequence with the accumulated values */
        seq1 = tsequence_make((const TInstant **) ininsts, k, true,
          (k == 1) ? true : false, interp, NORMALIZE);
        /* Compute the twAvg/twCentroid for the bin */
        value = twavg ? Float8GetDatum(tnumberseq_twavg(seq1)) :
          PointerGetDatum(tpointseq_twcentroid(seq1));
        outinsts[l++] = tinstant_make(value, temptype_out, lower);
        pfree(seq1);
        if (! twavg)
          pfree(DatumGetPointer(value));
      }
      /* Free the instant at the beginning of the bin if it was generated */
      if (start)
      {
        pfree(start); start = NULL;
      }
      if (end)
      {
        /* For STEP interpolation, remove the instant generated for the end of
           the bin when the last instant READ is equal to the end of the bin */
        if (interp == STEP && cmp == 0)
          pfree(end);
        else 
          start = end;
        end = NULL;
      }
      /* Reinitilize the accumulation by default */
      k = 0;
      /* If the last instant READ is the start of the bin */
      if (cmp == 0)
      {
        ininsts[0] = inst; i++; k = 1;
      }
      /* If the last instant READ is the after the bin and the interpolation
       * is continuous and thus the start of the bin has been generated */
      else if (start)
      {
        k = 0;
        ininsts[k++] = start;
        /* If the next instant is within the next time bin */
        if (timestamptz_cmp_internal(inst->t, upper) <= 0)
        {
          ininsts[k++] = inst; 
          i++;
        }
      }
      lower = upper;
      upper += tunits;
    }
  }
  /* Compute the twAvg/twCentroid of the last bin */
  if (k > 0)
  {
    seq1 = tsequence_make((const TInstant **) ininsts, k, true,
      (k == 1) ? true : seq->period.upper_inc, interp, NORMALIZE);
    value = twavg ? Float8GetDatum(tnumberseq_twavg(seq1)) :
      PointerGetDatum(tpointseq_twcentroid(seq1));
    outinsts[l++] = tinstant_make(value, temptype_out, lower);
    if (! twavg)
      pfree(DatumGetPointer(value));
    pfree(seq1);
  }
  /* The lower and upper bounds of the result are both true since the 
   * tprecision operation amounts to a granularity change */
   TSequence *result = tsequence_make_free(outinsts, l, true, true, interp,
    NORMALIZE);
  pfree(ininsts);
  if (start)
    pfree(start);
  return result;
}

/**
 * @brief Return a temporal sequence set with the precision set to a time bin
 * @param[in] ss Temporal value
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 */
TSequenceSet *
tsequenceset_tprecision(const TSequenceSet *ss, const Interval *duration,
  TimestampTz torigin)
{
  assert(ss); assert(duration); assert(positive_duration(duration));
  assert(ss->temptype == T_TINT || ss->temptype == T_TFLOAT ||
    ss->temptype == T_TGEOMPOINT || ss->temptype == T_TGEOGPOINT );

  int64 tunits = interval_units(duration);
  TimestampTz lower = DatumGetTimestampTz(ss->period.lower);
  TimestampTz upper = DatumGetTimestampTz(ss->period.upper);
  TimestampTz lower_bin = timestamptz_get_bin(lower, duration, torigin);
  /* We need to add tunits to obtain the end timestamp of the last bin */
  TimestampTz upper_bin = timestamptz_get_bin(upper, duration, torigin) +
    tunits;
  /* Number of bins */
  int count = (int) (((int64) upper_bin - (int64) lower_bin) / tunits);
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  lower = lower_bin;
  upper = lower_bin + tunits;
  interpType interp = MEOS_FLAGS_GET_INTERP(ss->flags);
  meosType temptype_out = (ss->temptype == T_TINT) ? T_TFLOAT : ss->temptype;
  meosType basetype_out = temptype_basetype(temptype_out);
  /* Determine whether we are computing the twAvg or the twCentroid */
  bool twavg = tnumber_type(ss->temptype);
  int ninsts = 0;
  int nseqs = 0;
  /* Loop for each bin */
  for (int i = 0; i < count; i++)
  {
    Span p;
    span_set(TimestampTzGetDatum(lower), TimestampTzGetDatum(upper),
      true, false, T_TIMESTAMPTZ, T_TSTZSPAN, &p);
    TSequenceSet *proj = tsequenceset_restrict_tstzspan(ss, &p, REST_AT);
    if (proj)
    {
      Datum value = twavg ? Float8GetDatum(tnumber_twavg((Temporal *) proj)) :
        PointerGetDatum(tpoint_twcentroid((Temporal *) proj));
      /* We keep only the first instant since the tprecision operation amounts
       * to a granularity change */
      instants[ninsts++] = tinstant_make(value, temptype_out, lower);
      DATUM_FREE(value, basetype_out);
      pfree(proj);
    }
    else
    {
      /* Close the previous sequence if any and start a new one */
      if (ninsts > 0)
      {
        /* The lower and upper bounds are both true since the tprecision
         * operation amounts to a granularity change */
        sequences[nseqs++] = tsequence_make((const TInstant **) instants,
          ninsts, true, true, interp, NORMALIZE);
        for (int j = 0; j < ninsts; j++)
          pfree(instants[j]);
        ninsts = 0;
      }
    }
    lower += tunits;
    upper += tunits;
  }
  /* Close the last sequence if any */
  if (ninsts > 0)
  {
    /* The lower and upper bounds are both true since the tprecision
     * operation amounts to a granularity change */
    sequences[nseqs++] = tsequence_make((const TInstant **) instants, ninsts,
      true, true, interp, NORMALIZE);
    for (int j = 0; j < ninsts; j++)
      pfree(instants[j]);
  }
  pfree(instants);
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @ingroup meos_temporal_analytics_reduction
 * @brief Return a temporal value with the precision set to time bins
 * @param[in] temp Temporal value
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 * @csqlfn #Temporal_tprecision()
 */
Temporal *
temporal_tprecision(const Temporal *temp, const Interval *duration,
  TimestampTz torigin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_duration(duration))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tinstant_tprecision((TInstant *) temp, duration,
        torigin);
    case TSEQUENCE:
      return (Temporal *) tsequence_tprecision((TSequence *) temp, duration,
        torigin);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_tprecision((TSequenceSet *) temp,
        duration, torigin);
  }
}

/*****************************************************************************
 * Temporal sample
 *****************************************************************************/

/**
 * @brief Return a temporal value sampled at time bins
 * @param[in] inst Temporal value
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 */
TInstant *
tinstant_tsample(const TInstant *inst, const Interval *duration,
  TimestampTz torigin)
{
  assert(inst); assert(duration); assert(positive_duration(duration));
  TimestampTz lower = timestamptz_get_bin(inst->t, duration, torigin);
  if (timestamp_cmp_internal(lower, inst->t) == 0)
    return tinstant_copy(inst);
  return NULL;
}


/**
 * @brief Return a temporal value sampled according to time bins
 * @param[in] seq Temporal value
 * @param[in] lower_bin,upper_bin First and last bins
 * @param[in] tunits Time size of the tiles in PostgreSQL time units
 * @param[out] result Output array of temporal instants
 * @note The result is an temporal sequence with discrete interpolation
 */
int
tsequence_tsample_iter(const TSequence *seq, TimestampTz lower_bin,
  TimestampTz upper_bin, int64 tunits, TInstant **result)
{
  meosType basetype = temptype_basetype(seq->temptype);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  const TInstant *start = TSEQUENCE_INST_N(seq, 0);
  TimestampTz lower = lower_bin;
  int i; /* Current segment of the sequence */
  int ninsts = 0; /* Number of instants of the result */
  int cmp1;
  if (interp == DISCRETE)
  {
    i = 0; /* Current instant of the sequence */
    while (i < seq->count && lower < upper_bin)
    {
      cmp1 = timestamptz_cmp_internal(start->t, lower);
      /* If the instant is equal to the lower bound of the bin */
      if (cmp1 == 0)
      {
        result[ninsts++] = tinstant_copy(start);
        lower += tunits;
      }
      /* Advance the bin if it is after the instant */
      else if (cmp1 > 0)
      {
        int times = ceil((double) (start->t - lower) / tunits);
        lower +=  times * tunits;
        continue;
      }
      if (cmp1 <= 0)
      {
        /* If there are no more segments */
        if (i == seq->count - 1)
          break;
        start = TSEQUENCE_INST_N(seq, ++i);
      }
      /* If there are no more segments */
    }
  }
  else
  {
    /* Loop for each segment */
    const TInstant *end = TSEQUENCE_INST_N(seq, 1);
    bool lower_inc = seq->period.lower_inc;
    i = 1; /* Current segment of the sequence */
    while (i < seq->count && lower < upper_bin)
    {
      bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
      cmp1 = timestamptz_cmp_internal(start->t, lower);
      int cmp2 = timestamptz_cmp_internal(lower, end->t);
      /* If the segment contains the lower bound of the bin */
      if ((cmp1 < 0 || (cmp1 == 0 && lower_inc)) &&
          (cmp2 < 0 || (cmp2 == 0 && upper_inc)))
      {
        Datum startvalue = tinstant_value_p(start);
        Datum endvalue = (interp == LINEAR) ?
          tinstant_value_p(end) : startvalue;
        Datum value = tsegment_value_at_timestamptz(startvalue, endvalue,
          start->temptype, start->t, end->t, lower);
        result[ninsts++] = tinstant_make(value, seq->temptype, lower);
        DATUM_FREE(value, basetype);
        /* Advance the bin */
        lower += tunits;
      }
      /* Advance the bin if it is after the start of the segment */
      else if (cmp1 >= 0)
        lower += tunits;
      /* Advance the segment if it is after the lower bound of the bin */
      else if (cmp2 >= 0)
      {
        /* If there are no more segments */
        if (i == seq->count - 1)
          break;
        start = end;
        end = TSEQUENCE_INST_N(seq, ++i);
      }
    }
  }
  return ninsts;
}

/**
 * @brief Return a temporal value sampled according to time bins
 * @param[in] seq Temporal value
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 * @param[in] interp Interpolation
 */
TSequence *
tsequence_tsample(const TSequence *seq, const Interval *duration,
  TimestampTz torigin, interpType interp)
{
  assert(seq); assert(duration); assert(positive_duration(duration));

  int64 tunits = interval_units(duration);
  TimestampTz lower = DatumGetTimestampTz(seq->period.lower);
  TimestampTz upper = DatumGetTimestampTz(seq->period.upper);
  TimestampTz lower_bin = timestamptz_get_bin(lower, duration, torigin);
  /* We need to add tunits to obtain the end timestamp of the last bin */
  TimestampTz upper_bin = timestamptz_get_bin(upper, duration, torigin) +
    tunits;
  /* Number of bins */
  int count = (int) (((int64) upper_bin - (int64) lower_bin) / tunits) + 1;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  int ninsts = tsequence_tsample_iter(seq, lower_bin, upper_bin, tunits,
    &instants[0]);
  return tsequence_make_free(instants, ninsts, true, true, interp, NORMALIZE);
}

/**
 * @brief Return a temporal value sampled according to time bins
 * @param[in] ss Temporal value
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 */
TSequence *
tsequenceset_disc_tsample(const TSequenceSet *ss, const Interval *duration,
  TimestampTz torigin)
{
  assert(ss); assert(duration); assert(positive_duration(duration));

  int64 tunits = interval_units(duration);
  TimestampTz lower = tsequenceset_start_timestamptz(ss);
  TimestampTz upper = tsequenceset_end_timestamptz(ss);
  TimestampTz lower_bin = timestamptz_get_bin(lower, duration, torigin);
  /* We need to add tunits to obtain the end timestamp of the last bin */
  TimestampTz upper_bin = timestamptz_get_bin(upper, duration, torigin) +
    tunits;
  /* Number of bins */
  int count = (int) (((int64) upper_bin - (int64) lower_bin) / tunits) + 1;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  /* Loop for each segment */
  int ninsts = 0;
  for (int i = 0; i < ss->count; i++)
  {
    ninsts += tsequence_tsample_iter(TSEQUENCESET_SEQ_N(ss, i), lower_bin,
      upper_bin, tunits, &instants[ninsts]);
  }
  return tsequence_make_free(instants, ninsts, true, true, DISCRETE,
    NORMALIZE);
}

/**
 * @brief Return a temporal value sampled according to time bins
 * @param[in] ss Temporal value
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 * @param[in] interp Interpolation
 */
TSequenceSet *
tsequenceset_cont_tsample(const TSequenceSet *ss, const Interval *duration,
  TimestampTz torigin, interpType interp)
{
  assert(ss); assert(duration); assert(positive_duration(duration));
  assert(interp != DISCRETE);

  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *seq = tsequence_tsample(TSEQUENCESET_SEQ_N(ss, i), duration,
      torigin, interp);
    if (seq)
      sequences[nseqs++] = seq;
  }
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @brief Return a temporal value sampled according to time bins
 * @param[in] ss Temporal value
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 * @param[in] interp Interpolation
 */
Temporal *
tsequenceset_tsample(const TSequenceSet *ss, const Interval *duration,
  TimestampTz torigin, interpType interp)
{
  return (interp == DISCRETE) ?
    (Temporal *) tsequenceset_disc_tsample(ss, duration, torigin) :
    (Temporal *) tsequenceset_cont_tsample(ss, duration, torigin, interp);
}

/**
 * @ingroup meos_temporal_analytics_reduction
 * @brief Return a temporal value sampled according to time bins
 * @param[in] temp Temporal value
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_tsample()
 */
Temporal *
temporal_tsample(const Temporal *temp, const Interval *duration,
  TimestampTz torigin, interpType interp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_duration(duration))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tinstant_tsample((TInstant *) temp, duration,
        torigin);
    case TSEQUENCE:
      return (Temporal *) tsequence_tsample((TSequence *) temp, duration,
        torigin, interp);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_tsample((TSequenceSet *) temp,
        duration, torigin, interp);
  }
}

/*****************************************************************************
 * Linear space computation of the similarity distance
 *****************************************************************************/

/**
 * @brief Return the distance between two temporal instants
 * @param[in] inst1,inst2 Temporal instants
 * @param[in] func Distance function
 */
double
tinstant_distance(const TInstant *inst1, const TInstant *inst2,
  datum_func2 func)
{
  assert(tnumber_type(inst1->temptype) || tgeo_type_all(inst1->temptype));
  if (tnumber_type(inst1->temptype))
    return tnumberinst_distance(inst1, inst2);
  else if (tgeo_type_all(inst1->temptype))
    return DatumGetFloat8(func(tinstant_value_p(inst1),
      tinstant_value_p(inst2)));
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Unknown distance function for type: %s", 
      meostype_name(inst1->temptype));
    return DBL_MAX;
  }
}

/**
 * @brief Linear space computation of the similarity distance between two
 * temporal values
 * @param[in] instants1,instants2 Arrays of temporal instants
 * @param[in] count1,count2 Number of instants in the arrays
 * @param[in] simfunc Similarity function, i.e., Frechet or DTW
 * @param[out] dist Array keeping the distances
 * @note Only two rows of the full matrix are used
 */
static double
tinstarr_similarity1(double *dist, const TInstant **instants1, int count1,
  const TInstant **instants2, int count2, SimFunc simfunc)
{
  datum_func2 func = pt_distance_fn(instants1[0]->flags);
  for (int i = 0; i < count1; i++)
  {
    for (int j = 0; j < count2; j++)
    {
      const TInstant *inst1 = instants1[i];
      const TInstant *inst2 = instants2[j];
      double d = tinstant_distance(inst1, inst2, func);
      if (i > 0 && j > 0)
      {
        if (simfunc == FRECHET)
        {
          dist[i%2 * count2 + j] = Max(d,
            Min(dist[(i - 1)%2 * count2 + j - 1],
              Min(dist[(i - 1)%2 * count2 + j], dist[i%2 * count2 + j - 1])));
        }
        else /* simfunc == DYNTIMEWARP */
        {
          dist[i%2 * count2 + j] = d +
            Min(dist[(i - 1)%2 * count2 + j - 1],
              Min(dist[(i - 1)%2 * count2 + j], dist[i%2 * count2 + j - 1]));
        }
      }
      else if (i > 0 && j == 0)
      {
        if (simfunc == FRECHET)
        {
          dist[i%2 * count2] = Max(d, dist[(i - 1)%2 * count2]);
        }
        else /* simfunc == DYNTIMEWARP */
        {
          dist[i%2 * count2] = d + dist[(i - 1)%2 * count2];
        }
      }
      else if (i == 0 && j > 0)
      {
        if (simfunc == FRECHET)
        {
          dist[j] = Max(d, dist[j - 1]);
        }
        else /* simfunc == DYNTIMEWARP */
        {
          dist[j] = d + dist[j - 1];
        }
      }
      else /* i == 0 && j == 0 */
      {
        dist[0] = d;
      }
    }
  }
  return dist[(count1 - 1)%2 * count2 + count2 - 1];
}

/**
 * @brief Linear space computation of the similarity distance between two
 * temporal values
 * @param[in] instants1,instants2 Arrays of temporal instants
 * @param[in] count1,count2 Number of instants in the arrays
 * @param[in] simfunc Similarity function, i.e., Frechet or DTW
 * @note Only two rows of the full matrix are used
 */
static double
tinstarr_similarity(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2, SimFunc simfunc)
{
  /* Allocate memory for two rows of the distance matrix */
  double *dist = palloc(sizeof(double) * 2 * count2);
  /* Initialise it with -1.0 */
  for (int i = 0; i < 2 * count2; i++)
    *(dist + i) = -1.0;
  /* Call the linear_space computation of the similarity distance */
  double result = tinstarr_similarity1(dist, instants1, count1, instants2,
    count2, simfunc);
  /* Free memory */
  pfree(dist);
  return result;
}

/**
 * @brief Return the similarity distance between two temporal values
 * @param[in] temp1,temp2 Temporal values
 * @param[in] simfunc Similarity function, i.e., Frechet or DTW
 */
double
temporal_similarity(const Temporal *temp1, const Temporal *temp2,
  SimFunc simfunc)
{
  assert(temp1); assert(temp2);
  assert(temp1->temptype == temp2->temptype);
  double result;
  int count1, count2;
  const TInstant **instants1 = temporal_instants_p(temp1, &count1);
  const TInstant **instants2 = temporal_instants_p(temp2, &count2);
  result = count1 > count2 ?
    tinstarr_similarity(instants1, count1, instants2, count2, simfunc) :
    tinstarr_similarity(instants2, count2, instants1, count1, simfunc);
  /* Free memory */
  pfree(instants1); pfree(instants2);
  return result;
}

#if MEOS
/**
 * @ingroup meos_temporal_analytics_similarity
 * @brief Return the Frechet distance between two temporal values
 * @param[in] temp1,temp2 Temporal values
 * @return On error return @p DBL_MAX
 * @csqlfn #Temporal_frechet_distance()
 */
double
temporal_frechet_distance(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return DBL_MAX;
  return temporal_similarity(temp1, temp2, FRECHET);
}

/**
 * @ingroup meos_temporal_analytics_similarity
 * @brief Return the Dynamic Time Warp distance between two temporal values
 * @param[in] temp1,temp2 Temporal values
 * @return On error return @p DBL_MAX
 * @csqlfn #Temporal_dyntimewarp_distance()
 */
double
temporal_dyntimewarp_distance(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return DBL_MAX;
  return temporal_similarity(temp1, temp2, DYNTIMEWARP);
}
#endif

/*****************************************************************************
 * Iterative implementation of the similarity distance with a full matrix
 *****************************************************************************/

/* Maximum length of the typmod string */
#define MAX_MATRIX_LEN 65536

#if DEBUG_BUILD
/**
 * @brief Print a distance matrix in tabular form
 */
void
matrix_print(double *dist, int count1, int count2)
{
  char buf[MAX_MATRIX_LEN];
  int i, j;
  size_t len = snprintf(buf, MAX_MATRIX_LEN - 1, "\n      ");
  for (j = 0; j < count2; j++)
    len += snprintf(buf + len, MAX_MATRIX_LEN - len - 1, "    %2d    ", j);
  len += snprintf(buf + len, MAX_MATRIX_LEN - len - 1, "\n");
  for (j = 0; j < count2; j++)
    len += snprintf(buf + len, MAX_MATRIX_LEN - len - 1, "------------");
  len += snprintf(buf + len, MAX_MATRIX_LEN - len - 1, "\n");
  for (i = 0; i < count1; i++)
  {
    len += snprintf(buf + len, MAX_MATRIX_LEN - len - 1, "%2d | ", i);
    for (j = 0; j < count2; j++)
      len += snprintf(buf + len, MAX_MATRIX_LEN - len - 1, " %9.3f",
        dist[i * count2 + j]);
    len += snprintf(buf + len, MAX_MATRIX_LEN - len - 1, "\n");
  }
  for (j = 0; j < count2; j++)
    len += snprintf(buf + len, MAX_MATRIX_LEN - len - 1, "------------");
  len += snprintf(buf + len, MAX_MATRIX_LEN - len - 1, "\n      ");
  for (j = 0; j < count2; j++)
    len += snprintf(buf + len, MAX_MATRIX_LEN - len - 1, "    %2d    ", j);
  /* make Codacy quiet by removing last assignment */
  snprintf(buf + len, MAX_MATRIX_LEN - len - 1, "\n"); 
  meos_error(WARNING, 0, "MATRIX:\n%s", buf);
  return;
}

/* Maximum length of the path string */
#define MAX_PATH_LEN 65536

/**
 * @brief Print a distant path found from the distance matrix
 */
void
path_print(Match *path, int count)
{
  size_t len = 0;
  char buf[MAX_PATH_LEN];
  int k = 0;
  for (int i = count - 1; i >= 0; i--)
    len += snprintf(buf + len, MAX_PATH_LEN - len - 1,
      "%d: (%2d,%2d)\n", k++, path[i].i, path[i].j);
  meos_error(WARNING, 0, "PATH:\n%s", buf);
  return;
}
#endif

/**
 * @brief Return the similarity path between two temporal values based on the
 * distance matrix
 * @param[in] dist Matrix keeping the distances
 * @param[in] count1,count2 Number of rows and columns of the matrix
 * @param[out] count Number of elements of the similarity path
 */
static Match *
tinstarr_similarity_path(double *dist, int count1, int count2, int *count)
{
  Match *result = palloc(sizeof(Match) * (count1 + count2));
  int i = count1 - 1;
  int j = count2 - 1;
  int k = 0;
  while (true)
  {
    result[k].i = i;
    result[k++].j = j;
    if (i == 0 && j == 0)
      break;
    if (i > 0 && j > 0)
    {
      /* Compute the minimum distance of the 3 neighboring cells */
      double d = Min(dist[(i - 1) * count2 + j - 1],
        Min(dist[(i - 1) * count2 + j], dist[i * count2 + j - 1]));
      /* We prioritize the diagonal in case of ties */
      if (dist[(i - 1) * count2 + j - 1] == d)
      {
        i--; j--;
      }
      else if (dist[(i - 1) * count2 + j] == d)
        i--;
      else /* (dist[(i) * count2 + j - 1] == d) */
        j--;
    }
    else if (i > 0)
      i--;
    else /* j > 0 */
      j--;
  }
  *count = k;
  return result;
}

/**
 * @brief Return the similarity distance between two temporal values using a
 * full matrix
 * @param[in] instants1,instants2 Instants of the temporal values
 * @param[in] count1,count2 Number of instants of the temporal values
 * @param[in] simfunc Similarity function, i.e., Frechet or DTW
 * @param[out] dist Matrix keeping the distances
 */
static void
tinstarr_similarity_matrix1(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2, SimFunc simfunc, double *dist)
{
  datum_func2 func = pt_distance_fn(instants1[0]->flags);
  for (int i = 0; i < count1; i++)
  {
    for (int j = 0; j < count2; j++)
    {
      const TInstant *inst1 = instants1[i];
      const TInstant *inst2 = instants2[j];
      double d = tinstant_distance(inst1, inst2, func);
      if (i > 0 && j > 0)
      {
        if (simfunc == FRECHET)
        {
          dist[i * count2 + j] = Max(d,
            Min(dist[(i - 1) * count2 + j - 1],
              Min(dist[(i - 1) * count2 + j], dist[i * count2 + j - 1])));
        }
        else /* simfunc == DYNTIMEWARP */
        {
          dist[i * count2 + j] = d +
            Min(dist[(i - 1) * count2 + j - 1],
              Min(dist[(i - 1) * count2 + j], dist[i * count2 + j - 1]));
        }
      }
      else if (i > 0 && j == 0)
      {
        if (simfunc == FRECHET)
        {
          dist[i * count2] = Max(d, dist[(i - 1) * count2]);
        }
        else /* simfunc == DYNTIMEWARP */
        {
          dist[i * count2] = d + dist[(i - 1) * count2];
        }
      }
      else if (i == 0 && j > 0)
      {
        if (simfunc == FRECHET)
        {
          dist[j] = Max(dist[j - 1], d);
        }
        else /* simfunc == DYNTIMEWARP */
        {
          dist[j] = d + dist[j - 1];
        }
      }
      else /* i == 0 && j == 0 */
      {
        dist[0] = d;
      }
    }
  }
  return;
}

/**
 * @brief Return the similarity distance between two temporal values
 * @param[in] instants1,instants2 Arrays of temporal instants
 * @param[in] count1,count2 Number of instants in the arrays
 * @param[in] simfunc Similarity function, i.e., Frechet or DTW
 * @param[out] count Number of elements in the resulting array
 */
static Match *
tinstarr_similarity_matrix(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2, SimFunc simfunc, int *count)
{
  /* Allocate memory for dist */
  double *dist = palloc(sizeof(double) * count1 * count2);
  /* Initialise it with -1.0 */
  for (int i = 0; i < count1 * count2; i++)
    *(dist + i) = -1.0;
  /* Call the iterative computation of the similarity distance */
  tinstarr_similarity_matrix1(instants1, count1, instants2, count2, simfunc,
    dist);
  /* Compute the path */
  Match *result = tinstarr_similarity_path(dist, count1, count2, count);
  /* Free memory */
  pfree(dist);
  return result;
}

/*****************************************************************************
 * Quadratic space computation of the similarity path
 *****************************************************************************/

/**
 * @brief Return the similarity path between two temporal values
 */
Match *
temporal_similarity_path(const Temporal *temp1, const Temporal *temp2,
  int *count, SimFunc simfunc)
{
  assert(temp1); assert(temp2); assert(count);
  assert(temp1->temptype == temp2->temptype);
  int count1, count2;
  const TInstant **instants1 = temporal_instants_p(temp1, &count1);
  const TInstant **instants2 = temporal_instants_p(temp2, &count2);
  Match *result = count1 > count2 ?
    tinstarr_similarity_matrix(instants1, count1, instants2, count2,
      simfunc, count) :
    tinstarr_similarity_matrix(instants2, count2, instants1, count1,
      simfunc, count);
  /* Free memory */
  pfree(instants1); pfree(instants2);
  return result;
}

#if MEOS
/**
 * @ingroup meos_temporal_analytics_similarity
 * @brief Return the Frechet distance between two temporal values
 * @param[in] temp1,temp2 Temporal values
 * @param[out] count Number of elements of the output array
 * @csqlfn #Temporal_frechet_path()
 */
Match *
temporal_frechet_path(const Temporal *temp1, const Temporal *temp2, int *count)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return NULL;
  return temporal_similarity_path(temp1, temp2, count, FRECHET);
}

/**
 * @ingroup meos_temporal_analytics_similarity
 * @brief Return the Dynamic Time Warp distance between two temporal values
 * @param[in] temp1,temp2 Temporal values
 * @param[out] count Number of elements of the output array
 * @csqlfn #Temporal_dyntimewarp_path()
 */
Match *
temporal_dyntimewarp_path(const Temporal *temp1, const Temporal *temp2,
  int *count)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return NULL;
  return temporal_similarity_path(temp1, temp2, count, DYNTIMEWARP);
}
#endif

/*****************************************************************************
 * Hausdorff distance
 *****************************************************************************/

/**
 * @brief Return the discrete Hausdorff distance between two temporal values
 * @param[in] instants1,instants2 Arrays of temporal instants
 * @param[in] count1,count2 Number of instants in the arrays
 */
static double
tinstarr_hausdorff_distance(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2)
{
  datum_func2 func = pt_distance_fn(instants1[0]->flags);
  const TInstant *inst1, *inst2;
  double cmax = 0.0, cmin;
  double d;
  int i, j;
  for (i = 0; i < count1; i++)
  {
    inst1 = instants1[i];
    cmin = DBL_MAX;
    for (j = 0; j < count2; j++)
    {
      inst2 = instants2[j];
      d = tinstant_distance(inst1, inst2, func);
      if (d < cmin)
        cmin = d;
      if (cmin < cmax)
        break;
    }
    if (cmax < cmin && cmin < DBL_MAX)
      cmax = cmin;
  }
  for (j = 0; j < count2; j++)
  {
    cmin = DBL_MAX;
    inst2 = instants2[j];
    for (i = 0; i < count1; i++)
    {
      inst1 = instants1[i];
      d = tinstant_distance(inst1, inst2, func);
      if (d < cmin)
        cmin = d;
      if (cmin < cmax)
        break;
    }
    if (cmax < cmin && cmin < DBL_MAX)
      cmax = cmin;
  }
  return cmax;
}

/**
 * @ingroup meos_temporal_analytics_similarity
 * @brief Return the Hausdorf distance between two temporal values
 * @param[in] temp1,temp2 Temporal values
 * @return On error return `DBL_MAX`
 */
double
temporal_hausdorff_distance(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return DBL_MAX;

  double result;
  int count1, count2;
  const TInstant **instants1 = temporal_instants_p(temp1, &count1);
  const TInstant **instants2 = temporal_instants_p(temp2, &count2);
  result = tinstarr_hausdorff_distance(instants1, count1, instants2, count2);
  /* Free memory */
  pfree(instants1); pfree(instants2);
  return result;
}

/***********************************************************************
 * Minimum distance simplification for temporal floats and points.
 * Inspired from Moving Pandas function MinDistanceGeneralizer
 * https://github.com/movingpandas/movingpandas/blob/main/movingpandas/trajectory_generalizer.py
 ***********************************************************************/

/**
 * @brief Return a temporal float/point sequence simplified ensuring that
 * consecutive values are at least a given distance apart
 * @param[in] seq Temporal value
 * @param[in] dist Minimum distance
 */
TSequence *
tsequence_simplify_min_dist(const TSequence *seq, double dist)
{
  datum_func2 func = pt_distance_fn(seq->flags);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  /* Add first instant to the output sequence */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  instants[0] = inst1;
  int ninsts = 1;
  bool last = false;
  /* Loop for every instant */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    double d = tinstant_distance(inst1, inst2, func);
    if (d > dist)
    {
      /* Add instant to output sequence */
      instants[ninsts++] = inst2;
      inst1 = inst2;
      if (i == seq->count - 1)
        last = true;
    }
  }
  if (seq->count > 1 && ! last)
    instants[ninsts++] = TSEQUENCE_INST_N(seq, seq->count - 1);
  TSequence *result = tsequence_make(instants, ninsts,
    (ninsts == 1) ? true : seq->period.lower_inc,
    (ninsts == 1) ? true : seq->period.upper_inc, LINEAR, NORMALIZE);
  pfree(instants);
  return result;
}

/**
 * @brief Return a temporal float/point sequence simplified ensuring that
 * consecutive values are at least a given distance apart
 * @param[in] ss Temporal value
 * @param[in] dist Distance
 */
TSequenceSet *
tsequenceset_simplify_min_dist(const TSequenceSet *ss, double dist)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tsequence_simplify_min_dist(TSEQUENCESET_SEQ_N(ss, i), dist);
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @ingroup meos_temporal_analytics_simplify
 * @brief Return a temporal float/point sequence simplified ensuring that
 * consecutive values are at least a given distance apart
 * @details This function is inspired from the Moving Pandas function
 * MinDistanceGeneralizer
 * https://github.com/movingpandas/movingpandas/blob/main/movingpandas/trajectory_generalizer.py
 * @param[in] temp Temporal value
 * @param[in] dist Distance in the units of the values for temporal floats or
 * the units of the coordinate system for temporal points.
 * @note The funcion applies only for temporal sequences or sequence sets with
 * linear interpolation. In all other cases, it returns a copy of the temporal
 * value.
 * @csqlfn #Temporal_simplify_min_dist()
 */
Temporal *
temporal_simplify_min_dist(const Temporal *temp, double dist)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  if (! ensure_tnumber_tpoint_type(temp->temptype) ||
      ! ensure_positive_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return temporal_copy(temp);
    case TSEQUENCE:
      return (Temporal *) tsequence_simplify_min_dist((TSequence *) temp, dist);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_simplify_min_dist((TSequenceSet *) temp,
        dist);
  }
}

/***********************************************************************
 * Minimum time delta simplification for temporal floats and points.
 * Inspired from Moving Pandas function MinTimeDeltaGeneralizer
 * https://github.com/movingpandas/movingpandas/blob/main/movingpandas/trajectory_generalizer.py
 ***********************************************************************/

/**
 * @brief Return a temporal float/point sequence simplified ensuring that
 * consecutive values are at least a certain time interval apart
 * @param[in] seq Temporal value
 * @param[in] mint Minimum time interval
 */
TSequence *
tsequence_simplify_min_tdelta(const TSequence *seq, const Interval *mint)
{
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  /* Add first instant to the output sequence */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  instants[0] = inst1;
  int ninsts = 1;
  bool last = false;
  /* Loop for every instant */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Interval *duration = minus_timestamptz_timestamptz(inst2->t, inst1->t);
    if (pg_interval_cmp(duration, mint) > 0)
    {
      /* Add instant to output sequence */
      instants[ninsts++] = inst2;
      inst1 = inst2;
      if (i == seq->count - 1)
        last = true;
    }
    pfree(duration);
  }
  if (seq->count > 1 && ! last)
    instants[ninsts++] = TSEQUENCE_INST_N(seq, seq->count - 1);
  TSequence *result = tsequence_make(instants, ninsts,
    (ninsts == 1) ? true : seq->period.lower_inc,
    (ninsts == 1) ? true : seq->period.upper_inc, LINEAR, NORMALIZE);
  pfree(instants);
  return result;
}

/**
 * @brief Return a temporal float/point sequence simplified ensuring that
 * consecutive values are at least a certain time interval apart
 * @param[in] ss Temporal value
 * @param[in] mint Minimum time interval
 */
TSequenceSet *
tsequenceset_simplify_min_tdelta(const TSequenceSet *ss, const Interval *mint)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tsequence_simplify_min_tdelta(TSEQUENCESET_SEQ_N(ss, i), mint);
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @ingroup meos_temporal_analytics_simplify
 * @brief Return a temporal float/point sequence simplified ensuring that
 * consecutive values are at least a certain time interval apart
 * @details This function is inspired from the Moving Pandas function
 * MinTimeDeltaGeneralizer
 * https://github.com/movingpandas/movingpandas/blob/main/movingpandas/trajectory_generalizer.py
 * @param[in] temp Temporal value
 * @param[in] mint Minimum time interval
 * @note The funcion applies only for temporal sequences or sequence sets with
 * linear interpolation. In all other cases, it returns a copy of the temporal
 * value.
 * @csqlfn #Temporal_simplify_min_tdelta()
 */
Temporal *
temporal_simplify_min_tdelta(const Temporal *temp, const Interval *mint)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(mint, NULL);
  if (! ensure_tnumber_tpoint_type(temp->temptype) ||
      ! ensure_positive_duration(mint))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return temporal_copy(temp);
    case TSEQUENCE:
      return ! MEOS_FLAGS_LINEAR_INTERP(temp->flags) ? temporal_copy(temp) :
        (Temporal *) tsequence_simplify_min_tdelta((TSequence *) temp, mint);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_simplify_min_tdelta((TSequenceSet *) temp,
        mint);
  }
}

/***********************************************************************
 * Simple Douglas-Peucker-like value simplification for temporal floats.
 ***********************************************************************/

/**
 * @brief Find a split when simplifying the temporal float sequence using the
 * Douglas-Peucker line simplification algorithm
 * @param[in] seq Temporal sequence
 * @param[in] i1,i2 Indexes of the reference instants
 * @param[out] split Location of the split
 * @param[out] dist Distance at the split
 * @note For temporal floats only the Synchronized Distance is used
 */
static void
tfloatseq_findsplit(const TSequence *seq, int i1, int i2, int *split,
  double *dist)
{
  *split = i1;
  *dist = -1;
  if (i1 + 1 >= i2)
    return;

  const TInstant *start = TSEQUENCE_INST_N(seq, i1);
  const TInstant *end = TSEQUENCE_INST_N(seq, i2);
  double startval = DatumGetFloat8(tinstant_value_p(start));
  double endval = DatumGetFloat8(tinstant_value_p(end));
  double duration2 = (double) (end->t - start->t);
  /* Loop for every instant between i1 and i2 */
  for (int idx = i1 + 1; idx < i2; idx++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, idx);
    double value = DatumGetFloat8(tinstant_value_p(inst));
    /*
     * The following is equivalent to
     * #tsegment_value_at_timestamptz(start, end, LINEAR, inst->t);
     */
    double duration1 = (double) (inst->t - start->t);
    double ratio = duration1 / duration2;
    double value_interp = startval + (endval - startval) * ratio;
    double d = fabs(value - value_interp);
    if (d > *dist)
    {
      /* Record the maximum */
      *split = idx;
      *dist = d;
    }
  }
  return;
}

/***********************************************************************
 * Simple spatio-temporal Douglas-Peucker line simplification.
 * No checks are done to avoid introduction of self-intersections.
 * No topology relations are considered.
 ***********************************************************************/

/**
 * @brief Return the 2D distance between the points
 */
static inline double
dist2d_pt_pt(POINT2D *p1, POINT2D *p2)
{
  return hypot(p2->x - p1->x, p2->y - p1->y);
}

/**
 * @brief Return the 3D distance between the points
 */
static inline double
dist3d_pt_pt(POINT3DZ *p1, POINT3DZ *p2)
{
  return hypot3d(p2->x - p1->x, p2->y - p1->y, p2->z - p1->z);
}

/**
 * @brief Return the 2D distance between the point the segment
 * @param[in] p Point
 * @param[in] A,B Points defining the segment
 * @see http://geomalgorithms.com/a02-_lines.html
 * @note Derived from the PostGIS function lw_dist2d_pt_seg in
 * file measures.c
 */
static double
dist2d_pt_seg(POINT2D *p, POINT2D *A, POINT2D *B)
{
  POINT2D c;
  double r;
  /* If start==end, then use pt distance */
  if (A->x == B->x && A->y == B->y)
    return dist2d_pt_pt(p, A);

  r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) ) /
      ( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) );

  if (r < 0) /* If the first vertex A is closest to the point p */
    return dist2d_pt_pt(p, A);
  if (r > 1)  /* If the second vertex B is closest to the point p */
    return dist2d_pt_pt(p, B);

  /* else if the point p is closer to some point between a and b
  then we find that point and send it to dist2d_pt_pt */
  c.x = A->x + r * (B->x - A->x);
  c.y = A->y + r * (B->y - A->y);

  return dist2d_pt_pt(p, &c);
}

/**
 * @brief Return the 3D distance between the point the segment
 * @param[in] p Point
 * @param[in] A,B Points defining the segment
 * @note Derived from the PostGIS function lw_dist3d_pt_seg in file
 * measures3d.c
 * @see http://geomalgorithms.com/a02-_lines.html
 */
static double
dist3d_pt_seg(POINT3DZ *p, POINT3DZ *A, POINT3DZ *B)
{
  POINT3DZ c;
  double r;
  /* If start==end, then use pt distance */
  if (FP_EQUALS(A->x, B->x) && FP_EQUALS(A->y, B->y) && FP_EQUALS(A->z, B->z))
    return dist3d_pt_pt(p, A);

  r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) +
        (p->z-A->z) * (B->z-A->z) ) /
      ( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) +
        (B->z-A->z) * (B->z-A->z) );

  if (r < 0) /* If the first vertex A is closest to the point p */
    return dist3d_pt_pt(p, A);
  if (r > 1) /* If the second vertex B is closest to the point p */
    return dist3d_pt_pt(p, B);

  /* If the point p is closer to some point between a and b, then we find that
     point and send it to dist3d_pt_pt */
  c.x = A->x + r * (B->x - A->x);
  c.y = A->y + r * (B->y - A->y);
  c.z = A->z + r * (B->z - A->z);

  return dist3d_pt_pt(p, &c);
}

/**
 * @brief Find a split when simplifying the temporal point sequence using the
 * Douglas-Peucker line simplification algorithm
 * @param[in] seq Temporal sequence
 * @param[in] i1,i2 Indexes of the reference instants
 * @param[in] syncdist True when using the Synchronized Euclidean Distance
 * @param[out] split Location of the split
 * @param[out] dist Distance at the split
 */
static void
tpointseq_findsplit(const TSequence *seq, int i1, int i2, bool syncdist,
  int *split, double *dist)
{
  POINT2D *p2k, *p2_sync, *p2a, *p2b;
  POINT3DZ *p3k, *p3_sync, *p3a, *p3b;
  Datum value;
  // interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
  double d = -1;
  *split = i1;
  *dist = -1;
  if (i1 + 1 >= i2)
    return;

  /* Initialization of values wrt instants i1 and i2 */
  const TInstant *start = TSEQUENCE_INST_N(seq, i1);
  const TInstant *end = TSEQUENCE_INST_N(seq, i2);
  Datum startvalue = tinstant_value_p(start);
  Datum endvalue = tinstant_value_p(end);
  if (hasz)
  {
    p3a = (POINT3DZ *) DATUM_POINT3DZ_P(tinstant_value_p(start));
    p3b = (POINT3DZ *) DATUM_POINT3DZ_P(tinstant_value_p(end));
  }
  else
  {
    p2a = (POINT2D *) DATUM_POINT2D_P(tinstant_value_p(start));
    p2b = (POINT2D *) DATUM_POINT2D_P(tinstant_value_p(end));
  }

  /* Loop for every instant between i1 and i2 */
  for (int idx = i1 + 1; idx < i2; idx++)
  {
    double d_tmp;
    const TInstant *inst = TSEQUENCE_INST_N(seq, idx);
    if (hasz)
    {
      p3k = (POINT3DZ *) DATUM_POINT3DZ_P(tinstant_value_p(inst));
      if (syncdist)
      {
        // TODO Should we take into account the interpolation ?
        value = tsegment_value_at_timestamptz(startvalue, endvalue,
          start->temptype, start->t, end->t, inst->t);
        p3_sync = (POINT3DZ *) DATUM_POINT3DZ_P(value);
        d_tmp = dist3d_pt_pt(p3k, p3_sync);
        pfree(DatumGetPointer(value));
      }
      else
        d_tmp = dist3d_pt_seg(p3k, p3a, p3b);
    }
    else
    {
      p2k = (POINT2D *) DATUM_POINT2D_P(tinstant_value_p(inst));
      if (syncdist)
      {
        // TODO Should we take into account the interpolation ?
        value = tsegment_value_at_timestamptz(startvalue, endvalue,
          start->temptype, start->t, end->t, inst->t);
        p2_sync = (POINT2D *) DATUM_POINT2D_P(value);
        d_tmp = dist2d_pt_pt(p2k, p2_sync);
        pfree(DatumGetPointer(value));
      }
      else
        d_tmp = dist2d_pt_seg(p2k, p2a, p2b);
    }
    if (d_tmp > d)
    {
      /* record the maximum */
      d = d_tmp;
      *split = idx;
    }
  }
  *dist = d;
  return;
}

/*****************************************************************************/

/**
 * @brief Return a temporal float/point sequence simplified using a single-pass
 * implementation of the Douglas-Peucker line simplification algorithm that
 * checks whether the provided distance threshold is exceeded
 * @param[in] seq Temporal value
 * @param[in] dist Minimum distance
 * @param[in] syncdist True when computing the Synchronized Euclidean
 * Distance (SED), false when computing the spatial only distance.
 * @param[in] minpts Minimum number of points
 */
TSequence *
tsequence_simplify_max_dist(const TSequence *seq, double dist, bool syncdist,
  uint32_t minpts)
{
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  const TInstant *prev = NULL;
  const TInstant *cur = NULL;
  uint32_t start = 0,   /* Lower index for finding the split */
           ninsts = 0;  /* Number of instants in the result */
  int split;            /* Index of the split */
  double d;             /* Distance */
  for (int i = 0; i < seq->count; i++)
  {
    cur = TSEQUENCE_INST_N(seq, i);
    if (prev == NULL)
    {
      instants[ninsts++] = cur;
      prev = cur;
      continue;
    }
    /* For temporal floats only Synchronized Distance is used */
    if (seq->temptype == T_TFLOAT)
      tfloatseq_findsplit(seq, start, i, &split, &d);
    else /* tpoint_type(seq->temptype) */
      tpointseq_findsplit(seq, start, i, syncdist, &split, &d);
    bool dosplit = (d >= 0 && (d > dist || start + i + 1 < minpts));
    if (dosplit)
    {
      prev = cur;
      instants[ninsts++] = TSEQUENCE_INST_N(seq, split);
      start = split;
      continue;
    }
  }
  if (ninsts > 0 && instants[ninsts - 1] != cur)
    instants[ninsts++] = cur;
  TSequence *result = tsequence_make(instants, ninsts,
    (ninsts == 1) ? true : seq->period.lower_inc,
    (ninsts == 1) ? true : seq->period.upper_inc, LINEAR, NORMALIZE);
  pfree(instants);
  return result;
}

/**
 * @brief Return a temporal float sequence set/point simplified using a
 * single-pass Douglas-Peucker line simplification algorithm
 * @param[in] ss Temporal point
 * @param[in] dist Distance
 * @param[in] syncdist True when computing the Synchronized Euclidean
 * Distance (SED), false when computing the spatial only distance.
 * @param[in] minpts Minimum number of points
 */
static TSequenceSet *
tsequenceset_simplify_max_dist(const TSequenceSet *ss, double dist,
  bool syncdist, uint32_t minpts)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tsequence_simplify_max_dist(TSEQUENCESET_SEQ_N(ss, i), dist,
      syncdist, minpts);
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @ingroup meos_temporal_analytics_simplify
 * @brief Return a temporal float/point simplified using a single-pass
 * Douglas-Peucker line simplification algorithm
 * @param[in] temp Temporal value
 * @param[in] dist Distance in the units of the values for temporal floats or
 * the units of the coordinate system for temporal points.
 * @param[in] syncdist True when the Synchronized Distance is used, false when
 * the spatial-only distance is used. Only used for temporal points.
 * @note The funcion applies only for temporal sequences or sequence sets with
 * linear interpolation. In all other cases, it returns a copy of the temporal
 * value.
 * @csqlfn #Temporal_simplify_max_dist()
 */
Temporal *
temporal_simplify_max_dist(const Temporal *temp, double dist, bool syncdist)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  if (! ensure_tnumber_tpoint_type(temp->temptype) ||
      ! ensure_positive_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return temporal_copy(temp);
    case TSEQUENCE:
      return ! MEOS_FLAGS_LINEAR_INTERP(temp->flags) ? temporal_copy(temp) :
        (Temporal *) tsequence_simplify_max_dist((TSequence *) temp, dist,
          syncdist, 2);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_simplify_max_dist((TSequenceSet *) temp,
        dist, syncdist, 2);
  }
}

/*****************************************************************************/

/**
 * @brief Return a negative or a positive value depending on whether the first
 * number is less than or greater than the second one
 */
static int
int_cmp(const void *a, const void *b)
{
  /* casting pointer types */
  const int *ia = (const int *) a;
  const int *ib = (const int *) b;
  /* returns negative if b > a and positive if a > b */
  return *ia - *ib;
}

#define DP_STACK_SIZE 256

/**
 * @brief Return a temporal float sequence set/point simplified using the
 * Douglas-Peucker line simplification algorithm
 */
static TSequence *
tsequence_simplify_dp(const TSequence *seq, double dist, bool syncdist,
  uint32_t minpts)
{
  int *stack, *outlist; /* recursion stack */
  int stack_static[DP_STACK_SIZE];
  int outlist_static[DP_STACK_SIZE];
  int sp = -1; /* recursion stack pointer */
  int i1, split;
  uint32_t outn = 0;
  uint32_t i;
  double d;

  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));
  assert(seq->temptype == T_TFLOAT || tpoint_type(seq->temptype));
  /* Do not try to simplify really short things */
  if (seq->count < 3)
    return tsequence_copy(seq);

  /* Only heap allocate book-keeping arrays if necessary */
  if ((unsigned int) seq->count > DP_STACK_SIZE)
  {
    stack = palloc(sizeof(int) * seq->count);
    outlist = palloc(sizeof(int) * seq->count);
  }
  else
  {
    stack = stack_static;
    outlist = outlist_static;
  }

  i1 = 0;
  stack[++sp] = seq->count - 1;
  /* Add first point to output list */
  outlist[outn++] = 0;
  do
  {
    /* For temporal floats only Synchronized Distance is used */
    if (seq->temptype == T_TFLOAT)
      tfloatseq_findsplit(seq, i1, stack[sp], &split, &d);
    else /* tpoint_type(seq->temptype) */
      tpointseq_findsplit(seq, i1, stack[sp], syncdist, &split, &d);
    bool dosplit = (d >= 0 && (d > dist || outn + sp + 1 < minpts));
    if (dosplit)
      stack[++sp] = split;
    else
    {
      outlist[outn++] = stack[sp];
      i1 = stack[sp--];
    }
  }
  while (sp >= 0);

  /* Order the list of points kept */
  qsort(outlist, outn, sizeof(int), int_cmp);
  /* Create a new temporal sequence */
  const TInstant **instants = palloc(sizeof(TInstant *) * outn);
  for (i = 0; i < outn; i++)
    instants[i] = TSEQUENCE_INST_N(seq, outlist[i]);
  TSequence *result = tsequence_make(instants, outn, seq->period.lower_inc,
    seq->period.upper_inc, LINEAR, NORMALIZE);
  pfree(instants);

  /* Free memory only if arrays are on the heap */
  if (stack != stack_static)
    pfree(stack);
  if (outlist != outlist_static)
    pfree(outlist);

  return result;
}

/**
 * @brief Return a temporal float sequence set/point simplified using the
 * Douglas-Peucker line simplification algorithm
 * @param[in] ss Temporal point
 * @param[in] dist Distance
 * @param[in] syncdist True when computing the Synchronized Euclidean
 * Distance (SED), false when computing the spatial only distance.
 * @param[in] minpts Minimum number of points
 */
static TSequenceSet *
tsequenceset_simplify_dp(const TSequenceSet *ss, double dist, bool syncdist,
  uint32_t minpts)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tsequence_simplify_dp(TSEQUENCESET_SEQ_N(ss, i), dist,
      syncdist, minpts);
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @ingroup meos_temporal_analytics_simplify
 * @brief Return a temporal float/point simplified using the Douglas-Peucker
 * line simplification algorithm
 * @param[in] temp Temporal value
 * @param[in] dist Distance in the units of the values for temporal floats or
 * the units of the coordinate system for temporal points.
 * @param[in] syncdist True when the Synchronized Distance is used, false when
 * the spatial-only distance is used. Only used for temporal points.
 * @note The funcion applies only for temporal sequences or sequence sets with
 * linear interpolation. In all other cases, it returns a copy of the temporal
 * value.
 * @csqlfn #Temporal_simplify_dp()
 */
Temporal *
temporal_simplify_dp(const Temporal *temp, double dist, bool syncdist)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  if (! ensure_tnumber_tpoint_type(temp->temptype) ||
      ! ensure_positive_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return temporal_copy(temp);
    case TSEQUENCE:
      return ! MEOS_FLAGS_LINEAR_INTERP(temp->flags) ? temporal_copy(temp) :
        (Temporal *) tsequence_simplify_dp((TSequence *) temp, dist, syncdist, 2);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_simplify_dp((TSequenceSet *) temp, dist,
        syncdist, 2);
  }
}

/*****************************************************************************/
