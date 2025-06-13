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
 * @brief Bin and tile functions for temporal types
 * @note The time bin functions are inspired from TimescaleDB
 * https://docs.timescale.com/latest/api#time_bucket
 */

#include "temporal/temporal_tile.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/date.h>
#include <utils/datetime.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/temporal_restrict.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h"

/*****************************************************************************
 * Multidimensional tile functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_analytics_tile
 * @brief Generate a multidimensional grid for temporal numbers
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the tiles on the time
 * dimension, may be `NULL`
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[out] count Number of elements in the output array
 */
TBox *
tbox_value_time_tiles(const TBox *box, Datum vsize, const Interval *duration,
  Datum vorigin, TimestampTz torigin, int *count)
{
  assert(box); assert(count);
  assert(not_negative_datum(vsize, box->span.basetype));
  assert(! duration || positive_duration(duration));

  TboxGridState *state = tbox_tile_state_make(NULL, box, vsize, duration,
    vorigin, torigin);
  int nrows = 1, ncols = 1;
  Datum start_bin, end_bin;
  /* Determine the number of value bins */
  if (datum_double(vsize, box->span.basetype))
    nrows = span_num_bins(&box->span, vsize, vorigin, &start_bin, &end_bin);
  /* Determine the number of time bins */
  int64 tunits = duration ? interval_units(duration) : 0;
  if (tunits)
    ncols = span_num_bins(&box->period, Int64GetDatum(tunits),
      TimestampTzGetDatum(torigin), &start_bin, &end_bin);
  /* Total number of tiles */
  int count1 = nrows * ncols;

  /* Compute the tiles */
  TBox *result = palloc0(sizeof(TBox) * count1);
  for (int i = 0; i < count1; i++)
  {
    tbox_tile_state_set(state->value, state->t, state->vsize, state->tunits,
      state->box.span.basetype, state->box.span.spantype, &result[i]);
    tbox_tile_state_next(state);
  }
  *count = count1;
  pfree(state);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tiles of a temporal integer box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the bins
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_time_tiles()
 */
TBox *
tintbox_value_time_tiles(const TBox *box, int vsize, const Interval *duration,
  int vorigin, TimestampTz torigin, int *count)
{
  /* Ensure validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  return tbox_value_time_tiles(box, Int32GetDatum(vsize), duration,
    Int32GetDatum(vorigin), torigin, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tiles of a temporal float box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the bins
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_time_tiles()
 */
TBox *
tfloatbox_value_time_tiles(const TBox *box, double vsize,
  const Interval *duration, double vorigin, TimestampTz torigin, int *count)
{
  /* Ensure validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  return tbox_value_time_tiles(box, Float8GetDatum(vsize), duration,
    Float8GetDatum(vorigin), torigin, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the value tiles of a temporal integer box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_tiles()
 */
TBox *
tintbox_value_tiles(const TBox *box, int vsize, int vorigin, int *count)
{
  return tbox_value_time_tiles(box, Int32GetDatum(vsize), NULL,
    Int32GetDatum(vorigin), 0, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the value tiles of a temporal float box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_tiles()
 */
TBox *
tfloatbox_value_tiles(const TBox *box, double vsize, double vorigin,
  int *count)
{
  return tbox_value_time_tiles(box, Float8GetDatum(vsize), NULL,
    Float8GetDatum(vorigin), 0, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the time tiles of a temporal float box
 * @param[in] box Input box to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_time_tiles()
 */
TBox *
tintbox_time_tiles(const TBox *box, const Interval *duration, 
  TimestampTz torigin, int *count)
{
  return tbox_value_time_tiles(box, Int32GetDatum(0), duration,
    Int32GetDatum(0), torigin, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the time tiles of a temporal float box
 * @param[in] box Input box to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_time_tiles()
 */
TBox *
tfloatbox_time_tiles(const TBox *box, const Interval *duration, 
  TimestampTz torigin, int *count)
{
  return tbox_value_time_tiles(box, Float8GetDatum(0.0), duration,
    Float8GetDatum(0.0), torigin, count);
}

/*****************************************************************************
 * Boxes functions for temporal numbers
 *****************************************************************************/

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value grid
 */
TBox *
tint_value_boxes(const Temporal *temp, int vsize, int vorigin, int *count)
{
  return tnumber_value_time_boxes(temp, Int32GetDatum(vsize), NULL,
    Int32GetDatum(vorigin), Int64GetDatum(0), count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a time grid
 */
TBox *
tint_time_boxes(const Temporal *temp, const Interval *duration,
  TimestampTz torigin, int *count)
{
  return tnumber_value_time_boxes(temp, Int32GetDatum(0), duration,
    Int32GetDatum(0), torigin, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value and possibly a time grid
 */
TBox *
tint_value_time_boxes(const Temporal *temp, int vsize,
  const Interval *duration, int vorigin, TimestampTz torigin, int *count)
{
  return tnumber_value_time_boxes(temp, Int32GetDatum(vsize), duration,
    Int32GetDatum(vorigin), torigin, count);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value grid
 */
TBox *
tfloat_value_boxes(const Temporal *temp, double vsize, double vorigin,
  int *count)
{
  return tnumber_value_time_boxes(temp, Float8GetDatum(vsize), NULL,
    Float8GetDatum(vorigin), Float8GetDatum(0.0), count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value and possibly a time grid
 */
TBox *
tfloat_time_boxes(const Temporal *temp, const Interval *duration, 
  TimestampTz torigin, int *count)
{
  return tnumber_value_time_boxes(temp, Float8GetDatum(0.0), duration,
    Float8GetDatum(0.0), torigin, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value and possibly a time grid
 */
TBox *
tfloat_value_time_boxes(const Temporal *temp, double vsize,
  const Interval *duration, double vorigin, TimestampTz torigin, int *count)
{
  return tnumber_value_time_boxes(temp, Float8GetDatum(vsize), duration,
    Float8GetDatum(vorigin), torigin, count);
}

/*****************************************************************************
 * Time split functions for temporal values
 *****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to time
 * bins
 * @param[in] inst Temporal value
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] torigin Time origin of the tiles
 * @param[out] bins Start timestamp of the bins containing a fragment
 * @param[out] count Number of values in the output array
 */
static TInstant **
tinstant_time_split(const TInstant *inst, int64 tunits, TimestampTz torigin,
  TimestampTz **bins, int *count)
{
  assert(inst);
  TInstant **result = palloc(sizeof(TInstant *));
  TimestampTz *times = palloc(sizeof(TimestampTz));
  result[0] = tinstant_copy(inst);
  times[0] = timestamptz_bin_start(inst->t, tunits, torigin);
  *bins = times;
  *count = 1;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to time
 * bins
 * @param[in] seq Temporal value
 * @param[in] start Start timestamp of the bins
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] count Number of bins
 * @param[out] bins Start timestamp of the bins containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static TSequence **
tdiscseq_time_split(const TSequence *seq, TimestampTz start, int64 tunits,
  int count, TimestampTz **bins, int *newcount)
{
  assert(seq); assert(bins); assert(newcount);

  TSequence **result = palloc(sizeof(TSequence *) * count);
  TimestampTz *times = palloc(sizeof(TimestampTz) * count);
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int i = 0,       /* counter for instants of temporal value */
      ninsts = 0,  /* counter for instants of next split */
      nfrags = 0;  /* counter for resulting fragments */
  TimestampTz lower = start;
  TimestampTz upper = start + tunits;
  while (i < seq->count)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if (lower <= inst->t && inst->t < upper)
    {
      instants[ninsts++] = inst;
      i++;
    }
    else
    {
      if (ninsts > 0)
      {
        times[nfrags] = lower;
        result[nfrags++] = tsequence_make(instants, ninsts, true, true,
          DISCRETE, NORMALIZE_NO);
        ninsts = 0;
      }
      lower = upper;
      upper += tunits;
    }
  }
  if (ninsts > 0)
  {
    times[nfrags] = lower;
    result[nfrags++] = tsequence_make(instants, ninsts, true, true, DISCRETE,
      NORMALIZE_NO);
  }
  pfree(instants);
  *bins = times;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to time
 * bins
 * @param[in] seq Temporal value
 * @param[in] start,end Start and end timestamps of the bins
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] count Number of bins
 * @param[out] result Output array of fragments of the temporal value
 * @param[out] times Output array of bin lower bounds
 * @note This function is called for each sequence of a temporal sequence set
 */
static int
tcontseq_time_split_iter(const TSequence *seq, TimestampTz start,
  TimestampTz end, int64 tunits, int count, TSequence **result,
  TimestampTz *times)
{
  assert(seq); assert(result); assert(times);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  TimestampTz lower = start;
  TimestampTz upper = lower + tunits;
  /* This loop is needed for filtering unnecesary time bins that are in the
   * time gaps between sequences composing a sequence set.
   * The upper bound for the bin is exclusive => the test below is >= */
  while (lower < end &&
    (DatumGetTimestampTz(seq->period.lower) >= upper ||
     lower > DatumGetTimestampTz(seq->period.upper) ||
     (lower == DatumGetTimestampTz(seq->period.upper) &&
        ! seq->period.upper_inc)))
  {
    lower = upper;
    upper += tunits;
  }

  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count * count);
  TInstant **tofree = palloc(sizeof(TInstant *) * seq->count * count);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  int i = 0,      /* counter for instants of temporal value */
      ninsts = 0, /* counter for instants of next split */
      nfree = 0,  /* counter for instants to free */
      nfrags = 0; /* counter for resulting fragments */
  bool lower_inc1;
  while (i < seq->count)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    /* If the instant is in the bin */
    if ((lower <= inst->t && inst->t < upper) ||
      (inst->t == upper && (interp == LINEAR || i == seq->count - 1)))
    {
      instants[ninsts++] = inst;
      i++;
    }
    else
    {
      assert(ninsts > 0);
      /* Compute the value at the end of the bin */
      if (instants[ninsts - 1]->t < upper)
      {
        if (interp == LINEAR)
          tofree[nfree] = tsegment_at_timestamptz(instants[ninsts - 1], inst,
            interp, upper);
        else
        {
          /* The last two values of sequences with step interpolation and
           * exclusive upper bound must be equal */
          Datum value = tinstant_value_p(instants[ninsts - 1]);
          tofree[nfree] = tinstant_make(value, seq->temptype, upper);
        }
        instants[ninsts++] = tofree[nfree++];
      }

      /* Compute the fragment */
      lower_inc1 = (nfrags == 0) ? seq->period.lower_inc : true;
      times[nfrags] = lower;
      result[nfrags++] = tsequence_make(instants, ninsts, lower_inc1,
         (ninsts > 1) ? false : true, interp, NORMALIZE);

      /* Set up for the next bin */
      ninsts = 0;
      lower = upper;
      upper += tunits;
      /* The second condition is needed for filtering unnecesary time bins
       * that are in the gaps between sequences composing a sequence set */
      if (lower >= end || ! contains_span_timestamptz(&seq->period, lower))
        break;
      /* The end value of the previous bin is the start of the new bin */
      if (lower < inst->t)
        instants[ninsts++] = TSEQUENCE_INST_N(result[nfrags - 1],
          result[nfrags - 1]->count - 1);
     }
  }
  if (ninsts > 0)
  {
    lower_inc1 = (nfrags == 0) ? seq->period.lower_inc : true;
    times[nfrags] = lower;
    result[nfrags++] = tsequence_make(instants, ninsts, lower_inc1,
      seq->period.upper_inc, interp, NORMALIZE);
  }
  pfree_array((void **) tofree, nfree);
  pfree(instants);
  return nfrags;
}

/**
 * @brief Split a temporal value into an array of fragments according to time
 * bins
 * @param[in] seq Temporal value
 * @param[in] start,end Start and end timestamps of the bins
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] count Number of bins
 * @param[out] bins Start timestamp of the bins containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static TSequence **
tcontseq_time_split(const TSequence *seq, TimestampTz start, TimestampTz end,
  int64 tunits, int count, TimestampTz **bins, int *newcount)
{
  assert(seq); assert(bins); assert(newcount);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  TSequence **result = palloc(sizeof(TSequence *) * count);
  TimestampTz *times = palloc(sizeof(TimestampTz) * count);
  *newcount = tcontseq_time_split_iter(seq, start, end, tunits, count, result,
    times);
  *bins = times;
  return result;
}

/**
 * @brief Split a temporal value into an array of disjoint fragments
 * @param[in] ss Temporal value
 * @param[in] start,end Start and end timestamps of the bins
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] count Number of bins
 * @param[out] bins Start timestamp of the bins containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static TSequenceSet **
tsequenceset_time_split(const TSequenceSet *ss, TimestampTz start,
  TimestampTz end, int64 tunits, int count, TimestampTz **bins,
  int *newcount)
{
  assert(ss); assert(bins); assert(newcount);

  /* Singleton sequence set */
  if (ss->count == 1)
  {
    TSequence **sequences = tcontseq_time_split(TSEQUENCESET_SEQ_N(ss, 0),
      start, end, tunits, count, bins, newcount);
    TSequenceSet **result = palloc(sizeof(TSequenceSet *) * *newcount);
    for (int i = 0; i < *newcount; i++)
      result[i] = tsequence_to_tsequenceset_free(sequences[i]);
    pfree(sequences);
    return result;
  }

  /* General case */
  /* Sequences obtained by spliting one composing sequence */
  TSequence **sequences = palloc(sizeof(TSequence *) * (ss->count * count));
  /* Start timestamp of bins obtained by spliting one composing sequence */
  TimestampTz *times = palloc(sizeof(TimestampTz) * (ss->count + count));
  /* Sequences composing the currently constructed bin of the sequence set */
  TSequence **fragments = palloc(sizeof(TSequence *) * (ss->count * count));
  /* Sequences for the bins of the sequence set */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  /* Variable used to adjust the start timestamp passed to the
   * tcontseq_time_split1 function in the loop */
  TimestampTz lower = start;
  int nfrags = 0, /* Number of accumulated fragments of the current time bin */
      nbucks = 0; /* Number of time bins already processed */
  for (int i = 0; i < ss->count; i++)
  {
    TimestampTz upper = lower + tunits;
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    /* Output the accumulated fragments of the current time bin (if any)
     * if the current sequence starts on the next time bin */
    if (nfrags > 0 && DatumGetTimestampTz(seq->period.lower) >= upper)
    {
      result[nbucks++] = tsequenceset_make((const TSequence **) fragments, nfrags,
        NORMALIZE);
      for (int j = 0; j < nfrags; j++)
        pfree(fragments[j]);
      nfrags = 0;
      lower += tunits;
      upper += tunits;
    }
    /* Number of time bins of the current sequence */
    int l = tcontseq_time_split_iter(seq, lower, end, tunits, count,
      sequences, &times[nbucks]);
    /* If the current sequence has produced more than two time bins */
    if (l > 1)
    {
      /* Assemble the accumulated fragments of the first time bin (if any)  */
      if (nfrags == 0)
        result[nbucks++] = tsequence_to_tsequenceset_free(sequences[0]);
      else
      {
        fragments[nfrags++] = sequences[0];
        result[nbucks++] = tsequenceset_make((const TSequence **) fragments,
          nfrags, NORMALIZE);
        for (int j = 0; j < nfrags; j++)
          pfree(fragments[j]);
        nfrags = 0;
      }
      for (int j = 1; j < l - 1; j++)
        result[nbucks++] = tsequence_to_tsequenceset_free(sequences[j]);
    }
    /* Save the last fragment in case it is necessary to assemble with the
     * first one of the next sequence */
    fragments[nfrags++] = sequences[l - 1];
    lower = times[nbucks];
  }
  /* Process the accumulated fragments of the last time bin */
  if (nfrags > 0)
  {
    result[nbucks++] = tsequenceset_make((const TSequence **) fragments, nfrags,
      NORMALIZE);
    for (int j = 0; j < nfrags; j++)
      pfree(fragments[j]);
  }
  pfree(sequences); pfree(fragments);
  *bins = times;
  *newcount = nbucks;
  return result;
}

/*****************************************************************************/

/**
 * @brief Return the fragments of a temporal value split according to
 * time bins
 * @param[in] temp Temporal value
 * @param[in] start,end Start and end timestamps of the bins
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] torigin Time origin of the tiles
 * @param[in] count Number of bins
 * @param[out] bins Start timestamp of the bins containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static Temporal **
temporal_time_split_int(const Temporal *temp, TimestampTz start,
  TimestampTz end, int64 tunits, TimestampTz torigin, int count,
  TimestampTz **bins, int *newcount)
{
  assert(temp); assert(bins); assert(newcount); assert(start < end);
  assert(count > 0);

  /* Split the temporal value */
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal **) tinstant_time_split((const TInstant *) temp,
        tunits, torigin, bins, newcount);
    case TSEQUENCE:
      return MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        (Temporal **) tdiscseq_time_split((const TSequence *) temp,
          start, tunits, count, bins, newcount) :
        (Temporal **) tcontseq_time_split((const TSequence *) temp,
          start, end, tunits, count, bins, newcount);
    default: /* TSEQUENCESET */
      return (Temporal **) tsequenceset_time_split((const TSequenceSet *) temp,
        start, end, tunits, count, bins, newcount);
  }
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal value split according to
 * time bins
 * @param[in] temp Temporal value
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 * @param[out] bins Array of bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_time_split()
 */
Temporal **
temporal_time_split(const Temporal *temp, const Interval *duration,
  TimestampTz torigin, TimestampTz **bins, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_positive_duration(duration))
    return NULL;

  Span s;
  temporal_set_tstzspan(temp, &s);
  Datum start_bin, end_bin;
  int64 tunits = interval_units(duration);
  int nbins = span_num_bins(&s, Int64GetDatum(tunits),
    TimestampTzGetDatum(torigin), &start_bin, &end_bin);
  return temporal_time_split_int(temp, DatumGetTimestampTz(start_bin),
    DatumGetTimestampTz(end_bin), tunits, torigin, nbins, bins, count);
}

/*****************************************************************************
 * Value split functions for temporal numbers
 *****************************************************************************/

/**
 * @brief Get the bin number in the bin space that contains the value
 * @param[in] value Input value
 * @param[in] size Size of the bins
 * @param[in] origin Origin of the bins
 * @param[in] type Type of the arguments
 */
static int
bin_position(Datum value, Datum size, Datum origin, meosType type)
{
  assert(tnumber_basetype(type));
  if (type == T_INT4)
    return (DatumGetInt32(value) - DatumGetInt32(origin)) /
      DatumGetInt32(size);
  else /* type == T_FLOAT8 */
    return (int) floor( (DatumGetFloat8(value) - DatumGetFloat8(origin)) /
      DatumGetFloat8(size) );
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] inst Temporal value
 * @param[in] size Size of the value bins
 * @param[in] start_bin Value of the start bin
 * @param[out] bins Start value of the bins containing a fragment
 * @param[out] newcount Number of values in the output arrays
 */
static TInstant **
tnumberinst_value_split(const TInstant *inst, Datum start_bin, Datum size,
  Datum **bins, int *newcount)
{
  assert(inst); assert(bins); assert(newcount);

  Datum value = tinstant_value_p(inst);
  meosType basetype = temptype_basetype(inst->temptype);
  TInstant **result = palloc(sizeof(TInstant *));
  Datum *values = palloc(sizeof(Datum));
  result[0] = tinstant_copy(inst);
  values[0] = datum_bin(value, size, start_bin, basetype);
  *bins = values;
  *newcount = 1;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] seq Temporal value
 * @param[in] size Size of the value bins
 * @param[in] start_bin Value of the start bin
 * @param[in] count Number of bins
 * @param[out] bins Start value of the bins containing a fragment
 * @param[out] newcount Number of values in the output arrays
 */
static TSequence **
tnumberseq_disc_value_split(const TSequence *seq, Datum start_bin,
  Datum size, int count, Datum **bins, int *newcount)
{
  assert(seq); assert(bins); assert(newcount);

  meosType basetype = temptype_basetype(seq->temptype);
  TSequence **result;
  Datum *values, value, bin_value;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    result = palloc(sizeof(TSequence *));
    values = palloc(sizeof(Datum));
    result[0] = tsequence_copy(seq);
    value = tinstant_value_p(TSEQUENCE_INST_N(seq, 0));
    values[0] = datum_bin(value, size, start_bin, basetype);
    *bins = values;
    *newcount = 1;
    return result;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count * count);
  /* palloc0 to initialize the counters to 0 */
  int *ninsts = palloc0(sizeof(int) * count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    value = tinstant_value_p(inst);
    bin_value = datum_bin(value, size, start_bin, basetype);
    int bin_no = bin_position(bin_value, size, start_bin, basetype);
    int inst_no = ninsts[bin_no]++;
    instants[bin_no * seq->count + inst_no] = inst;
  }
  /* Assemble the result for each value bin */
  result = palloc(sizeof(TSequence *) * count);
  values = palloc(sizeof(Datum) * count);
  int nfrags = 0;
  bin_value = start_bin;
  for (int i = 0; i < count; i++)
  {
    if (ninsts[i] > 0)
    {
      result[nfrags] = tsequence_make(&instants[i * seq->count], ninsts[i],
        true, true, DISCRETE, NORMALIZE_NO);
      values[nfrags++] = bin_value;
    }
    bin_value = datum_add(bin_value, size, basetype);
  }
  pfree(instants);
  pfree(ninsts);
  *bins = values;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] seq Temporal value
 * @param[in] start_bin Value of the start bin
 * @param[in] size Size of the value bins
 * @param[in] count Number of bins
 * @param[in,out] result Array containing the fragments of each bin
 * @param[in,out] nseqs Number of fragments for each bin
 * @param[in] numcols Number of columns in the 2D pointer array. It can be
 * @p seq->count for sequences or @p ss->totalcount for sequence sets
 */
static void
tnumberseq_step_value_split(const TSequence *seq, Datum start_bin,
  Datum size, int count, TSequence **result, int *nseqs, int numcols)
{
  assert(seq); assert(result); assert(nseqs);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == STEP);

  meosType basetype = temptype_basetype(seq->temptype);
  Datum value, bin_value;
  int bin_no, seq_no;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    value = tinstant_value_p(TSEQUENCE_INST_N(seq, 0));
    bin_value = datum_bin(value, size, start_bin, basetype);
    bin_no = bin_position(bin_value, size, start_bin, basetype);
    seq_no = nseqs[bin_no]++;
    result[bin_no * numcols + seq_no] = tsequence_copy(seq);
    return;
  }

  /* General case */
  TInstant **tofree = palloc(sizeof(TInstant *) * seq->count * count);
  int nfree = 0;   /* counter for the instants to free */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count; i++)
  {
    value = tinstant_value_p(inst1);
    bin_value = datum_bin(value, size, start_bin, basetype);
    bin_no = bin_position(bin_value, size, start_bin, basetype);
    seq_no = nseqs[bin_no]++;
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    bool lower_inc1 = (i == 1) ? seq->period.lower_inc : true;
    TInstant *bounds[2];
    bounds[0] = (TInstant *) inst1;
    int nfrags = 1;
    if (i < seq->count)
    {
      tofree[nfree++] = bounds[1] = tinstant_make(value, seq->temptype,
        inst2->t);
      nfrags++;
    }
    result[bin_no * numcols + seq_no] = tsequence_make(
      (const TInstant **) bounds, nfrags, lower_inc1, false, STEP, NORMALIZE);
    bounds[0] = bounds[1];
    inst1 = inst2;
    lower_inc1 = true;
  }
  /* Last value if upper inclusive */
  if (seq->period.upper_inc)
  {
    inst1 = TSEQUENCE_INST_N(seq, seq->count - 1);
    value = tinstant_value_p(inst1);
    bin_value = datum_bin(value, size, start_bin, basetype);
    bin_no = bin_position(bin_value, size, start_bin, basetype);
    seq_no = nseqs[bin_no]++;
    result[bin_no * numcols + seq_no] = tinstant_to_tsequence(inst1, STEP);
  }
  pfree_array((void **) tofree, nfree);
  return;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] seq Temporal value
 * @param[in] start_bin Value of the start bin
 * @param[in] size Size of the value bins
 * @param[in] count Number of bins
 * @param[in,out] result Array containing the fragments of each bin
 * @param[in,out] nseqs Number of fragments for each bin
 * @param[in] numcols Number of columns in the 2D pointer array. It can be
 * @p seq->count for sequences or @p ss->totalcount for sequence sets
 */
static void
tnumberseq_linear_value_split(const TSequence *seq, Datum start_bin,
  Datum size, int count, TSequence **result, int *nseqs, int numcols)
{
  assert(seq); assert(result); assert(nseqs);
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  meosType basetype = temptype_basetype(seq->temptype);
  meosType spantype = basetype_spantype(basetype);
  Datum value1, bin_value1;
  int bin_no1, seq_no;
  Span segspan;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    value1 = tinstant_value_p(TSEQUENCE_INST_N(seq, 0));
    bin_value1 = datum_bin(value1, size, start_bin, basetype);
    bin_no1 = bin_position(bin_value1, size, start_bin, basetype);
    seq_no = nseqs[bin_no1]++;
    result[bin_no1 * numcols + seq_no] = tsequence_copy(seq);
    return;
  }

  /* General case */
  TInstant **tofree = palloc(sizeof(TInstant *) * seq->count * count);
  int nfree = 0;   /* counter for the instants to free */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  value1 = tinstant_value_p(inst1);
  bin_value1 = datum_bin(value1, size, start_bin, basetype);
  bin_no1 = bin_position(bin_value1, size, start_bin, basetype);
  /* For each segment */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_value_p(inst2);
    Datum bin_value2 = datum_bin(value2, size, start_bin, basetype);
    int bin_no2 = bin_position(bin_value2, size, start_bin, basetype);

    /* Set variables depending on whether the segment is constant, increasing,
     * or decreasing */
    Datum min_value, max_value;
    int first_bin, last_bin, first, last;
    bool lower_inc1, upper_inc1; /* Lower/upper bound inclusion of the segment */
    bool lower_inc_def, upper_inc_def; /* Default lower/upper bound inclusion */
    int cmp = datum_cmp(value1, value2, basetype);
    if (cmp <= 0)
    {
      /* Both for constant and increasing segments */
      min_value = value1;
      max_value = value2;
      first_bin = bin_no1;
      last_bin = bin_no2;
      first = 0;
      last = 1;
      lower_inc_def = true;
      upper_inc_def = false;
      lower_inc1 = (i == 1) ? seq->period.lower_inc : true;
      upper_inc1 = (i == seq->count - 1) ? seq->period.upper_inc : false;
    }
    else
    {
      min_value = value2;
      max_value = value1;
      first_bin = bin_no2;
      last_bin = bin_no1;
      first = 1;
      last = 0;
      lower_inc_def = false;
      upper_inc_def = true;
      lower_inc1 = (i == seq->count - 1) ? seq->period.upper_inc : false;
      upper_inc1 = (i == 1) ? seq->period.lower_inc : true;
    }

    /* Split the segment into bins */
    span_set(min_value, max_value, lower_inc1, (cmp != 0) ? upper_inc1 : true,
      basetype, spantype, &segspan);
    TInstant *bounds[2];
    bounds[first] = (cmp <= 0) ? (TInstant *) inst1 : (TInstant *) inst2;
    Datum bin_lower = (cmp <= 0) ? bin_value1 : bin_value2;
    Datum bin_upper = datum_add(bin_lower, size, basetype);
    for (int j = first_bin; j <= last_bin; j++)
    {
      /* Choose between interpolate or take one of the segment ends */
      if (datum_lt(min_value, bin_upper, basetype) &&
          datum_lt(bin_upper, max_value, basetype))
      {
        TimestampTz t1, t2;
        /* We are sure there is a unique turning point */
        tsegment_intersection_value(value1, value2, bin_upper, inst1->temptype,
          inst1->t, inst2->t, &t1, &t2);
        /* To reduce the roundoff errors we take the value projected to the
         * timestamp instead of the bound value */
        Datum projvalue1 = tsegment_value_at_timestamptz(value1, value2,
          inst1->temptype, inst1->t, inst2->t, t1);
        tofree[nfree++] = bounds[last] = 
          tinstant_make(projvalue1, seq->temptype, t1);
      }
      else
        bounds[last] = (cmp <= 0) ? (TInstant *) inst2 : (TInstant *) inst1;
      /* Determine the bounds of the resulting sequence */
      if (j == first_bin || j == last_bin)
      {
        Span binspan;
        span_set(bin_lower, bin_upper, true, false, basetype, spantype,
          &binspan);
        Span inter;
        bool found = inter_span_span(&segspan, &binspan, &inter);
        if (found)
        {
          /* Do nothing for constant segments */
          if (cmp < 0)
          {
            lower_inc1 = inter.lower_inc;
            upper_inc1 = inter.upper_inc;
          }
          else if (cmp > 0)
          {
            lower_inc1 = inter.upper_inc;
            upper_inc1 = inter.lower_inc;
          }
        }
        else
        {
          lower_inc1 = upper_inc1 = false;
        }
      }
      else
      {
        /* Sequence bounds are the bin bounds */
        lower_inc1 = lower_inc_def;
        upper_inc1 = upper_inc_def;
      }
      /* If last bin contains a single instant */
      int nfrags = (bounds[0]->t == bounds[1]->t) ? 1 : 2;
      /* We cannot add to last bin if last instant has exclusive bound */
      if (nfrags == 1 && ! upper_inc1)
        break;
      seq_no = nseqs[j]++;
      result[j * numcols + seq_no] = tsequence_make((const TInstant **) bounds,
        nfrags, (nfrags > 1) ? lower_inc1 : true, (nfrags > 1) ? upper_inc1 : true,
        LINEAR, NORMALIZE_NO);
      bounds[first] = bounds[last];
      bin_lower = bin_upper;
      bin_upper = datum_add(bin_upper, size, basetype);
    }
    inst1 = inst2;
    value1 = value2;
    bin_value1 = bin_value2;
    bin_no1 = bin_no2;
  }
  pfree_array((void **) tofree, nfree);
  return;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] seq Temporal value
 * @param[in] start_bin Value of the start bin
 * @param[in] size Size of the value bins
 * @param[in] count Number of bins
 * @param[out] bins Start value of the bins containing the fragments
 * @param[out] newcount Number of elements in output arrays
 */
static TSequenceSet **
tnumberseq_cont_value_split(const TSequence *seq, Datum start_bin, Datum size,
  int count, Datum **bins, int *newcount)
{
  assert(seq); assert(bins); assert(newcount);

  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  assert(interp != DISCRETE);
  meosType basetype = temptype_basetype(seq->temptype);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TSequenceSet **result = palloc(sizeof(TSequenceSet *));
    Datum *values = palloc(sizeof(Datum));
    result[0] = tsequence_to_tsequenceset(seq);
    Datum value = tinstant_value_p(TSEQUENCE_INST_N(seq, 0));
    values[0] = datum_bin(value, size, start_bin, basetype);
    *bins = values;
    *newcount = 1;
    return result;
  }

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count * count);
  /* palloc0 to initialize the counters to 0 */
  int *nseqs = palloc0(sizeof(int) * count);
  if (interp == LINEAR)
    tnumberseq_linear_value_split(seq, start_bin, size, count, sequences,
      nseqs, seq->count);
  else
    tnumberseq_step_value_split(seq, start_bin, size, count, sequences,
      nseqs, seq->count);
  /* Assemble the result for each value bin */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  Datum *values = palloc(sizeof(Datum) * count);
  Datum bin_value = start_bin;
  int nfrags = 0;
  for (int i = 0; i < count; i++)
  {
    if (nseqs[i] > 0)
    {
      result[nfrags] = tsequenceset_make(
        (const TSequence **)(&sequences[seq->count * i]), nseqs[i], NORMALIZE);
      values[nfrags++] = bin_value;
    }
    bin_value = datum_add(bin_value, size, basetype);
  }
  pfree(sequences);
  pfree(nseqs);
  *bins = values;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] ss Temporal value
 * @param[in] start_bin Start value of the first bin
 * @param[in] size Size of the value bins
 * @param[in] count Number of bins
 * @param[out] bins Array of start values of the bins containing the
 * fragments
 * @param[out] newcount Number of values in the output arrays
 */
static TSequenceSet **
tnumberseqset_value_split(const TSequenceSet *ss, Datum start_bin, Datum size,
  int count, Datum **bins, int *newcount)
{
  assert(ss); assert(bins); assert(newcount);

  /* Singleton sequence set */
  if (ss->count == 1)
    return tnumberseq_cont_value_split(TSEQUENCESET_SEQ_N(ss, 0), start_bin,
      size, count, bins, newcount);

  /* General case */
  meosType basetype = temptype_basetype(ss->temptype);
  TSequence **binseqs = palloc(sizeof(TSequence *) * ss->totalcount * count);
  /* palloc0 to initialize the counters to 0 */
  int *nseqs = palloc0(sizeof(int) * count);
  bool linear = MEOS_FLAGS_LINEAR_INTERP(ss->flags);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (linear)
      tnumberseq_linear_value_split(seq, start_bin, size, count, binseqs,
        nseqs, ss->totalcount);
    else
      tnumberseq_step_value_split(seq, start_bin, size, count, binseqs,
        nseqs, ss->totalcount);
  }
  /* Assemble the result for each value bin */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  Datum *values = palloc(sizeof(Datum) * count);
  Datum bin_value = start_bin;
  int nfrags = 0;
  for (int i = 0; i < count; i++)
  {
    if (nseqs[i] > 0)
    {
      result[nfrags] = tsequenceset_make((const TSequence **)
        (&binseqs[i * ss->totalcount]), nseqs[i], NORMALIZE);
      values[nfrags++] = bin_value;
    }
    bin_value = datum_add(bin_value, size, basetype);
  }
  pfree(binseqs);
  pfree(nseqs);
  *bins = values;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_tile
 * @brief Split a temporal number into an array of fragments according to value
 * bins
 * @param[in] temp Temporal value
 * @param[in] size Size of the value bins
 * @param[in] vorigin Origin of the value bins
 * @param[out] bins Array of start values of the bins containing the fragments
 * @param[out] count Number of values in the output arrays
 */
Temporal **
tnumber_value_split(const Temporal *temp, Datum size, Datum vorigin,
  Datum **bins, int *count)
{
  assert(temp); assert(bins); assert(count);
  assert(tnumber_type(temp->temptype));

  /* Compute the value bounds */
  Span s;
  tnumber_set_span(temp, &s);
  Datum start_bin, end_bin;
  int nbins = span_num_bins(&s, size, vorigin, &start_bin, &end_bin);

  /* Split the temporal value */
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal **) tnumberinst_value_split((const TInstant *) temp,
        start_bin, size, bins, count);
    case TSEQUENCE:
      return MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        (Temporal **) tnumberseq_disc_value_split((const TSequence *) temp,
          start_bin, size, nbins, bins, count) :
        (Temporal **) tnumberseq_cont_value_split((const TSequence *) temp,
          start_bin, size, nbins, bins, count);
    default: /* TSEQUENCESET */
      return (Temporal **) tnumberseqset_value_split(
        (const TSequenceSet *) temp, start_bin, size, nbins, bins, count);
  }
}

/*****************************************************************************/

/**
 * @brief Return a temporal value split according to a base value and possibly
 * a temporal grid
 */
Temporal **
tnumber_value_time_split(const Temporal *temp, Datum size,
  const Interval *duration, Datum vorigin, TimestampTz torigin,
  Datum **value_bins, TimestampTz **time_bins, int *count)
{
  meosType basetype = temptype_basetype(temp->temptype);
  ensure_positive_datum(size, basetype);
  ensure_positive_duration(duration);

  Datum start_bin, end_bin, start_time_bin, end_time_bin;
  /* Compute the value bounds */
  Span s;
  tnumber_set_span(temp, &s);
  int value_count = span_num_bins(&s, size, vorigin, &start_bin,
    &end_bin);
  /* Compute the time bounds */
  temporal_set_tstzspan(temp, &s);
  int64 tunits = interval_units(duration);
  int time_count = span_num_bins(&s, Int64GetDatum(tunits),
    TimestampTzGetDatum(torigin), &start_time_bin, &end_time_bin);
  TimestampTz start_time = DatumGetTimestampTz(start_time_bin);
  TimestampTz end_time = DatumGetTimestampTz(end_time_bin);
  /* Total number of tiles */
  int ntiles = value_count * time_count;

  /* Split the temporal value */
  Datum *v_bins = NULL;
  TimestampTz *t_bins = NULL;
  Temporal **fragments;
  v_bins = palloc(sizeof(Datum) * ntiles);
  t_bins = palloc(sizeof(TimestampTz) * ntiles);
  fragments = palloc(sizeof(Temporal *) * ntiles);
  int nfrags = 0;
  Datum lower_value = start_bin;
  meosType spantype = basetype_spantype(basetype);
  while (datum_lt(lower_value, end_bin, basetype))
  {
    Datum upper_value = datum_add(lower_value, size, basetype);
    span_set(lower_value, upper_value, true, false, basetype, spantype, &s);
    Temporal *atspan = tnumber_restrict_span(temp, &s, REST_AT);
    if (atspan != NULL)
    {
      int num_time_splits;
      TimestampTz *times;
      Temporal **time_splits = temporal_time_split_int(atspan, start_time,
        end_time, tunits, torigin, time_count, &times, &num_time_splits);
      for (int i = 0; i < num_time_splits; i++)
      {
        v_bins[i + nfrags] = lower_value;
        t_bins[i + nfrags] = times[i];
        fragments[i + nfrags] = time_splits[i];
      }
      nfrags += num_time_splits;
      pfree(time_splits);
      pfree(times);
      pfree(atspan);
    }
    lower_value = upper_value;
  }
  *count = nfrags;
  if (value_bins)
    *value_bins = v_bins;
  else
    pfree(v_bins);
  if (time_bins)
    *time_bins = t_bins;
  else
    pfree(t_bins);
  return fragments;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal integer split according to value
 * bins
 * @param[in] temp Temporal value
 * @param[in] size Size of the value bins
 * @param[in] origin Time origin of the bins
 * @param[out] bins Array of bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_split()
 */
Temporal **
tint_value_split(const Temporal *temp, int size, int origin, int **bins,
  int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TINT) || ! ensure_positive(size))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_split(temp, Int32GetDatum(size),
    Int32GetDatum(origin), &datum_bins, count);
  /* Transform the datum bins into float bins and return */
  int *values = palloc(sizeof(int) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetInt32(datum_bins[i]);
  if (bins)
    *bins = values;
  pfree(datum_bins);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal float split according to value
 * bins
 * @param[in] temp Temporal value
 * @param[in] size Size of the value bins
 * @param[in] origin Time origin of the bins
 * @param[out] bins Array of bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_split()
 */
Temporal **
tfloat_value_split(const Temporal *temp, double size, double origin,
  double **bins, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT) ||
      ! ensure_positive_datum(Float8GetDatum(size), T_FLOAT8))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_split(temp, Float8GetDatum(size),
    Float8GetDatum(origin), &datum_bins, count);
  /* Transform the datum bins into float bins and return */
  double *values = palloc(sizeof(double) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetFloat8(datum_bins[i]);
  if (bins)
    *bins = values;
  pfree(datum_bins);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal integer split according to value
 * and time bins
 * @param[in] temp Temporal value
 * @param[in] size Size of the value bins
 * @param[in] duration Size of the time bins
 * @param[in] vorigin Time origin of the bins
 * @param[in] torigin Time origin of the bins
 * @param[out] value_bins Array of value bins
 * @param[out] time_bins Array of time bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_time_split()
 */
Temporal **
tint_value_time_split(const Temporal *temp, int size, const Interval *duration,
  int vorigin, TimestampTz torigin, int **value_bins,
  TimestampTz **time_bins, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TINT) || ! ensure_positive(size) ||
      ! ensure_positive_duration(duration))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_time_split(temp, Int32GetDatum(size),
    duration, Int32GetDatum(vorigin), torigin, &datum_bins, time_bins,
    count);

  /* Transform the datum bins into float bins and return */
  int *values = palloc(sizeof(double) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetInt32(datum_bins[i]);
  if (value_bins)
    *value_bins = values;
  else
    pfree(values);
  pfree(datum_bins);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal integer split according to value
 * and time bins
 * @param[in] temp Temporal value
 * @param[in] size Size of the value bins
 * @param[in] duration Size of the time bins
 * @param[in] vorigin Time origin of the bins
 * @param[in] torigin Time origin of the bins
 * @param[out] value_bins Array of value bins
 * @param[out] time_bins Array of time bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_time_split()
 */
Temporal **
tfloat_value_time_split(const Temporal *temp, double size,
  const Interval *duration, double vorigin, TimestampTz torigin,
  double **value_bins, TimestampTz **time_bins, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT) ||
      ! ensure_positive_datum(Float8GetDatum(size), T_FLOAT8) ||
      ! ensure_positive_duration(duration))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_time_split(temp, Float8GetDatum(size),
    duration, Float8GetDatum(vorigin), torigin, &datum_bins, time_bins,
    count);

  /* Transform the datum bins into float bins and return */
  double *values = palloc(sizeof(double) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetFloat8(datum_bins[i]);
  if (value_bins)
    *value_bins = values;
  else
    pfree(values);
  pfree(datum_bins);
  return result;
}

/*****************************************************************************/
