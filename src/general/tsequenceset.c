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
 * @file tsequenceset.c
 * @brief General functions for temporal sequence sets.
 */

#include "general/tsequenceset.h"

/* PostgreSQL */
#include <assert.h>
#include <libpq/pqformat.h>
#include <utils/lsyscache.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/timestampset.h"
#include "general/period.h"
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "general/tempcache.h"
#include "general/temporal_boxops.h"
#include "general/rangetypes_ext.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * Return a pointer to the bounding box of the temporal value
 */
void *
tsequenceset_bbox_ptr(const TSequenceSet *ts)
{
  return (void *)(((char *)ts) + double_pad(sizeof(TSequenceSet)));
}

/**
 * Copy in the second argument the bounding box of the temporal value
 */
void
tsequenceset_bbox(const TSequenceSet *ts, void *box)
{
  memset(box, 0, ts->bboxsize);
  memcpy(box, tsequenceset_bbox_ptr(ts), ts->bboxsize);
  return;
}

/**
 * Return a pointer to the offsets array of the temporal value
 */
static size_t *
tsequenceset_offsets_ptr(const TSequenceSet *ts)
{
  return (size_t *)(((char *)ts) + double_pad(sizeof(TSequenceSet)) +
    double_pad(ts->bboxsize));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the n-th sequence of the temporal value.
 */
const TSequence *
tsequenceset_seq_n(const TSequenceSet *ts, int index)
{
  return (TSequence *)(
    /* start of data */
    ((char *)ts) + double_pad(sizeof(TSequenceSet)) + ts->bboxsize +
      ts->count * sizeof(size_t) +
      /* offset */
      (tsequenceset_offsets_ptr(ts))[index]);
}

/**
 * Ensure the validity of the arguments when creating a temporal value
 */
static void
tsequenceset_make_valid(const TSequence **sequences, int count)
{
  bool linear = MOBDB_FLAGS_GET_LINEAR(sequences[0]->flags);
  /* Ensure that all values are of sequence subtype and of the same interpolation */
  for (int i = 0; i < count; i++)
  {
    if (sequences[i]->subtype != SEQUENCE)
    {
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Input values must be temporal sequences")));
    }
    if (MOBDB_FLAGS_GET_LINEAR(sequences[i]->flags) != linear)
    {
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Input sequences must have the same interpolation")));
    }
  }
  return;
}

/**
 * Construct a temporal sequence set value from the array of temporal
 * sequence values
 *
 * For example, the memory structure of a temporal sequence set value
 * with two sequences is as follows
 * @code
 * ------------------------------------------------------------
 * ( TSequenceSet )_X | ( bbox )_X | offset_0 | offset_1 | ...
 * ------------------------------------------------------------
 * ---------------------------------------
 * ( TSequence_0 )_X | ( TSequence_1 )_X |
 * ---------------------------------------
 * @endcode
 * where the `_X` are unused bytes added for double padding, `offset_0` and
 * `offset_1` are offsets for the corresponding sequences.
 *
 * @param[in] sequences Array of sequences
 * @param[in] count Number of elements in the array
 * @param[in] normalize True when the resulting value should be normalized.
 * In particular, normalize is false when synchronizing two
 * temporal sequence set values before applying an operation to them.
 */
TSequenceSet *
tsequenceset_make1(const TSequence **sequences, int count, bool normalize)
{
  /* Test the validity of the sequences */
  assert(count > 0);
  ensure_valid_tseqarr(sequences, count);
  /* Normalize the array of sequences */
  TSequence **normseqs = (TSequence **) sequences;
  int newcount = count;
  if (normalize && count > 1)
    normseqs = tseqarr_normalize(sequences, count, &newcount);

  /* Get the bounding box size */
  size_t bboxsize = temporal_bbox_size(sequences[0]->temptype);

  /* Compute the size of the temporal sequence */
  /* Bounding box size */
  size_t memsize = bboxsize;
  /* Size of composing sequences */
  int totalcount = 0;
  for (int i = 0; i < newcount; i++)
  {
    totalcount += normseqs[i]->count;
    memsize += double_pad(VARSIZE(normseqs[i]));
  }
  /* Size of the struct and the offset array */
  memsize += double_pad(sizeof(TSequenceSet)) + newcount * sizeof(size_t);
  /* Create the temporal sequence set */
  TSequenceSet *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  result->count = newcount;
  result->totalcount = totalcount;
  result->temptype = sequences[0]->temptype;
  result->subtype = SEQUENCESET;
  result->bboxsize = bboxsize;
  MOBDB_FLAGS_SET_CONTINUOUS(result->flags,
    MOBDB_FLAGS_GET_CONTINUOUS(sequences[0]->flags));
  MOBDB_FLAGS_SET_LINEAR(result->flags,
    MOBDB_FLAGS_GET_LINEAR(sequences[0]->flags));
  MOBDB_FLAGS_SET_X(result->flags, true);
  MOBDB_FLAGS_SET_T(result->flags, true);
  if (tgeo_type(sequences[0]->temptype))
  {
    MOBDB_FLAGS_SET_Z(result->flags,
      MOBDB_FLAGS_GET_Z(sequences[0]->flags));
    MOBDB_FLAGS_SET_GEODETIC(result->flags,
      MOBDB_FLAGS_GET_GEODETIC(sequences[0]->flags));
  }
  /* Initialization of the variable-length part */
  /*
   * Compute the bounding box
   * Only external types have bounding box, internal types such
   * as double2, double3, or double4 do not have bounding box
   */
  if (bboxsize != 0)
  {
    tsequenceset_make_bbox((const TSequence **) normseqs, newcount,
      tsequenceset_bbox_ptr(result));
  }
  /* Store the composing instants */
  size_t pdata = double_pad(sizeof(TSequenceSet)) + double_pad(bboxsize) +
    newcount * sizeof(size_t);
  size_t pos = 0;
  for (int i = 0; i < newcount; i++)
  {
    memcpy(((char *) result) + pdata + pos, normseqs[i],
      VARSIZE(normseqs[i]));
    (tsequenceset_offsets_ptr(result))[i] = pos;
    pos += double_pad(VARSIZE(normseqs[i]));
  }
  if (normalize && count > 1)
    pfree_array((void **) normseqs, newcount);
  return result;
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal sequence set value from the array of temporal
 * sequence values.
 *
 * @param[in] sequences Array of sequences
 * @param[in] count Number of elements in the array
 * @param[in] normalize True when the resulting value should be normalized.
 * In particular, normalize is false when synchronizing two
 * temporal sequence set values before applying an operation to them.
 */
TSequenceSet *
tsequenceset_make(const TSequence **sequences, int count, bool normalize)
{
  tsequenceset_make_valid(sequences, count);
  return tsequenceset_make1(sequences, count, normalize);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal sequence set value from the array of temporal
 * sequence values and free the array and the sequences after the creation.
 *
 * @param[in] sequences Array of sequences
 * @param[in] count Number of elements in the array
 * @param[in] normalize True when the resulting value should be normalized.
 */
TSequenceSet *
tsequenceset_make_free(TSequence **sequences, int count, bool normalize)
{
  if (count == 0)
  {
    pfree(sequences);
    return NULL;
  }
  TSequenceSet *result = tsequenceset_make((const TSequence **) sequences,
    count, normalize);
  pfree_array((void **) sequences, count);
  return result;
}

/**
 * Ensure the validity of the arguments when creating a temporal value
 * This function extends function tsequence_make_valid by spliting the
 * sequences according the maximum distance or interval between instants.
 */
int *
tsequenceset_make_valid_gaps(const TInstant **instants, int count, bool lower_inc,
  bool upper_inc, bool linear, double maxdist, Interval *maxt, int *countsplits)
{
  tsequence_make_valid1(instants, count, lower_inc, upper_inc, linear);
  return ensure_valid_tinstarr_gaps(instants, count, MERGE_NO,
    SEQUENCE, maxdist, maxt, countsplits);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal sequence set value from the array of temporal
 * sequence values.
 *
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] linear True when the resulting value has linear interpolation.
 * @param[in] maxdist Maximum distance for defining a gap.
 * @param[in] maxt Maximum time interval for defining a gap.
 */
TSequenceSet *
tsequenceset_make_gaps(const TInstant **instants, int count, bool linear,
  float maxdist, Interval *maxt)
{
  /* Set the interval to NULL if it is negative or zero */
  Interval intervalzero;
  memset(&intervalzero, 0, sizeof(Interval));
  int cmp = call_function2(interval_cmp, PointerGetDatum(maxt),
    PointerGetDatum(&intervalzero));
  if (cmp <= 0)
    maxt = NULL;

  TSequence *seq;
  TSequenceSet *result;

  /* If no gaps are given construt call the standard sequence constructor */
  if (maxdist <= 0.0 && maxt == NULL)
  {
    seq = tsequence_make((const TInstant **) instants, count, true, true,
      linear, NORMALIZE);
    result = tsequenceset_make((const TSequence **) &seq, 1, NORMALIZE_NO);
    return result;
  }

  /* Ensure that the array of instants is valid and determine the splits */
  int countsplits;
  int *splits = tsequenceset_make_valid_gaps((const TInstant **) instants,
    count, true, true, linear, maxdist, maxt, &countsplits);
  if (countsplits == 0)
  {
    /* There are no gaps  */
    pfree(splits);
    seq = tsequence_make1((const TInstant **) instants, count,
      true, true, linear, NORMALIZE);
    result = tsequenceset_make((const TSequence **) &seq, 1, NORMALIZE_NO);
  }
  else
  {
    int newcount = 0;
    /* Split according to gaps  */
    const TInstant **newinsts = palloc(sizeof(TInstant *) * count);
    TSequence **sequences = palloc(sizeof(TSequence *) * (countsplits + 1));
    int j = 0, k = 0;
    for (int i = 0; i < count; i++)
    {
      if (splits[j] == i)
      {
        /* Finalize the current sequence and start a new one */
        assert(k > 0);
        sequences[newcount++] = tsequence_make1((const TInstant **) newinsts, k,
          true, true, linear, NORMALIZE);
        j++; k = 0;
      }
      /* Continue with the current sequence */
      newinsts[k++] = instants[i];
    }
    /* Construct last sequence */
    if (k > 0)
      sequences[newcount++] = tsequence_make1((const TInstant **) newinsts, k,
        true, true, linear, NORMALIZE);
    result = tsequenceset_make((const TSequence **) sequences, newcount,
      NORMALIZE);
    pfree(newinsts); pfree(sequences);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Return a copy of the temporal value.
 */
TSequenceSet *
tsequenceset_copy(const TSequenceSet *ts)
{
  TSequenceSet *result = palloc0(VARSIZE(ts));
  memcpy(result, ts, VARSIZE(ts));
  return result;
}

/**
 * Return the location of the timestamp in the temporal sequence set value
 * using binary search
 *
 * If the timestamp is contained in the temporal value, the index of the
 * sequence is returned in the output parameter. Otherwise, returns a number
 * encoding whether the timestamp is before, between two sequences, or after
 * the temporal value. For example, given a value composed of 3 sequences
 * and a timestamp, the value returned in the output parameter is as follows:
 * @code
 *               0          1          2
 *            |-----|    |-----|    |-----|
 * 1)    t^                                         => loc = 0
 * 2)                 t^                            => loc = 1
 * 3)                       t^                      => loc = 1
 * 4)                             t^                => loc = 2
 * 5)                                         t^    => loc = 3
 * @endcode
 *
 * @param[in] ts Temporal sequence set value
 * @param[in] t Timestamp
 * @param[out] loc Location
 * @result Return true if the timestamp is contained in the temporal value
 */
bool
tsequenceset_find_timestamp(const TSequenceSet *ts, TimestampTz t, int *loc)
{
  int first = 0, last = ts->count - 1;
  int middle = 0; /* make compiler quiet */
  const TSequence *seq = NULL; /* make compiler quiet */
  while (first <= last)
  {
    middle = (first + last)/2;
    seq = tsequenceset_seq_n(ts, middle);
    if (contains_period_timestamp(&seq->period, t))
    {
      *loc = middle;
      return true;
    }
    if (t <= seq->period.lower)
      last = middle - 1;
    else
      first = middle + 1;
  }
  if (t >= seq->period.upper)
    middle++;
  *loc = middle;
  return false;
}

/*****************************************************************************
 * Append and merge functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_transf
 * @brief Append an instant to the temporal value.
 */
TSequenceSet *
tsequenceset_append_tinstant(const TSequenceSet *ts, const TInstant *inst)
{
  assert(ts->temptype == inst->temptype);
  const TSequence *seq = tsequenceset_seq_n(ts, ts->count - 1);
  Temporal *temp = tsequence_append_tinstant(seq, inst);
  const TSequence **sequences = palloc(sizeof(TSequence *) * ts->count + 1);
  int k = 0;
  for (int i = 0; i < ts->count - 1; i++)
    sequences[k++] = tsequenceset_seq_n(ts, i);
  assert(temp->subtype == SEQUENCE || temp->subtype == SEQUENCESET);
  if (temp->subtype == SEQUENCE)
    sequences[k++] = (const TSequence *) temp;
  else /* temp->subtype == SEQUENCESET */
  {
    TSequenceSet *ts1 = (TSequenceSet *) temp;
    sequences[k++] = tsequenceset_seq_n(ts1, 0);
    sequences[k++] = tsequenceset_seq_n(ts1, 1);
  }
  return tsequenceset_make(sequences, k, NORMALIZE_NO);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Merge the two temporal values
 */
TSequenceSet *
tsequenceset_merge(const TSequenceSet *ts1, const TSequenceSet *ts2)
{
  const TSequenceSet *seqsets[] = {ts1, ts2};
  return tsequenceset_merge_array(seqsets, 2);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Merge the array of temporal sequence set values.
 * The values in the array may overlap in a single instant.
 *
 * @param[in] seqsets Array of values
 * @param[in] count Number of elements in the array
 * @result Merged value
 */
TSequenceSet *
tsequenceset_merge_array(const TSequenceSet **seqsets, int count)
{
  /* Validity test will be done in tsequence_merge_array */
  /* Collect the composing sequences */
  int totalcount = 0;
  for (int i = 0; i < count; i++)
    totalcount += seqsets[i]->count;
  const TSequence **sequences = palloc0(sizeof(TSequence *) * totalcount);
  int k = 0;
  for (int i = 0; i < count; i++)
  {
    for (int j = 0; j < seqsets[i]->count; j++)
      sequences[k++] = tsequenceset_seq_n(seqsets[i], j);
  }
  /* We cannot call directly tsequence_merge_array since the result must always
   * be of subtype TSEQUENCESET */
  int newcount;
  TSequence **newseqs = tsequence_merge_array1(sequences, totalcount, &newcount);
  return tsequenceset_make_free(newseqs, newcount, NORMALIZE);
}

/*****************************************************************************
 * Synchronize functions
 *****************************************************************************/

/**
 * Temporally intersect or synchronize the two temporal values
 *
 * The resulting values are composed of denormalized sequences
 * covering the intersection of their time spans
 *
 * @param[in] ts,seq Input values
 * @param[in] mode Enumeration for either intersect or synchronize
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
synchronize_tsequenceset_tsequence(const TSequenceSet *ts, const TSequence *seq,
  SyncMode mode, TSequenceSet **inter1, TSequenceSet **inter2)
{
  /* Test whether the bounding period of the two temporal values overlap */
  Period p;
  tsequenceset_period(ts, &p);
  if (! overlaps_period_period(&seq->period, &p))
    return false;

  int loc;
  tsequenceset_find_timestamp(ts, seq->period.lower, &loc);
  /* We are sure that loc < ts->count due to the bounding period test above */
  TSequence **sequences1 = palloc(sizeof(TSequence *) * ts->count - loc);
  TSequence **sequences2 = palloc(sizeof(TSequence *) * ts->count - loc);
  int k = 0;
  for (int i = loc; i < ts->count; i++)
  {
    const TSequence *seq1 = tsequenceset_seq_n(ts, i);
    TSequence *interseq1, *interseq2;
    bool hasinter;
    /* mode == SYNCHRONIZE or SYNCHRONIZE_CROSS */
    hasinter = synchronize_tsequence_tsequence(seq, seq1,
      &interseq1, &interseq2, mode == SYNCHRONIZE_CROSS);
    if (hasinter)
    {
      sequences1[k] = interseq1;
      sequences2[k++] = interseq2;
    }
    int cmp = timestamp_cmp_internal(seq->period.upper, seq1->period.upper);
    if (cmp < 0 ||
      (cmp == 0 && (!seq->period.upper_inc || seq1->period.upper_inc)))
      break;
  }
  if (k == 0)
  {
    pfree(sequences1); pfree(sequences2);
    return false;
  }

  *inter1 = tsequenceset_make_free(sequences1, k, NORMALIZE_NO);
  *inter2 = tsequenceset_make_free(sequences2, k, NORMALIZE_NO);
  return true;
}

/**
 * Temporally intersect or synchronize the two temporal values
 *
 * @param[in] ts1,ts2 Input values
 * @param[in] mode Intersection or synchronization (with or without adding crossings)
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
synchronize_tsequenceset_tsequenceset(const TSequenceSet *ts1,
  const TSequenceSet *ts2, SyncMode mode, TSequenceSet **inter1,
  TSequenceSet **inter2)
{
  /* Test whether the bounding period of the two temporal values overlap */
  Period p1, p2;
  tsequenceset_period(ts1, &p1);
  tsequenceset_period(ts2, &p2);
  if (! overlaps_period_period(&p1, &p2))
    return false;

  int count = ts1->count + ts2->count;
  TSequence **sequences1 = palloc(sizeof(TSequence *) * count);
  TSequence **sequences2 = palloc(sizeof(TSequence *) * count);
  int i = 0, j = 0, k = 0;
  while (i < ts1->count && j < ts2->count)
  {
    const TSequence *seq1 = tsequenceset_seq_n(ts1, i);
    const TSequence *seq2 = tsequenceset_seq_n(ts2, j);
    TSequence *interseq1, *interseq2;
    bool hasinter;
    /* mode == SYNCHRONIZE or SYNCHRONIZE_CROSS */
    hasinter = synchronize_tsequence_tsequence(seq1, seq2,
      &interseq1, &interseq2, mode == SYNCHRONIZE_CROSS);
    if (hasinter)
    {
      sequences1[k] = interseq1;
      sequences2[k++] = interseq2;
    }
    int cmp = timestamp_cmp_internal(seq1->period.upper, seq2->period.upper);
    if (cmp == 0 && seq1->period.upper_inc == seq2->period.upper_inc)
    {
      i++; j++;
    }
    else if (cmp < 0 ||
      (cmp == 0 && ! seq1->period.upper_inc && seq2->period.upper_inc))
      i++;
    else
      j++;
  }
  if (k == 0)
  {
    pfree(sequences1); pfree(sequences2);
    return false;
  }

  *inter1 = tsequenceset_make_free(sequences1, k, NORMALIZE_NO);
  *inter2 = tsequenceset_make_free(sequences2, k, NORMALIZE_NO);
  return true;
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] ts,inst Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tsequenceset_tinstant(const TSequenceSet *ts, const TInstant *inst,
  TInstant **inter1, TInstant **inter2)
{
  TInstant *inst1 = (TInstant *)
    tsequenceset_restrict_timestamp(ts, inst->t, REST_AT);
  if (inst1 == NULL)
    return false;

  *inter1 = inst1;
  *inter2 = tinstant_copy(inst);
  return true;
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] inst,ts Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tinstant_tsequenceset(const TInstant *inst, const TSequenceSet *ts,
  TInstant **inter1, TInstant **inter2)
{
  return intersection_tsequenceset_tinstant(ts, inst, inter2, inter1);
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] ts,ti Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tsequenceset_tinstantset(const TSequenceSet *ts,
  const TInstantSet *ti, TInstantSet **inter1, TInstantSet **inter2)
{
  /* Test whether the bounding period of the two temporal values overlap */
  Period p1, p2;
  tsequenceset_period(ts, &p1);
  tinstantset_period(ti, &p2);
  if (! overlaps_period_period(&p1, &p2))
    return false;

  TInstant **instants1 = palloc(sizeof(TInstant *) * ti->count);
  const TInstant **instants2 = palloc(sizeof(TInstant *) * ti->count);
  int i = 0, j = 0, k = 0;
  while (i < ts->count && j < ti->count)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    const TInstant *inst = tinstantset_inst_n(ti, j);
    if (contains_period_timestamp(&seq->period, inst->t))
    {
      instants1[k] = tsequence_at_timestamp(seq, inst->t);
      instants2[k++] = inst;
    }
    int cmp = timestamp_cmp_internal(seq->period.upper, inst->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
      i++;
    else
      j++;
  }
  if (k == 0)
  {
    pfree(instants1); pfree(instants2);
    return false;
  }

  *inter1 = tinstantset_make_free(instants1, k, MERGE_NO);
  *inter2 = tinstantset_make(instants2, k, MERGE_NO);
  pfree(instants2);
  return true;
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] ti,ts Input values
 * @param[out] inter1,inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tinstantset_tsequenceset(const TInstantSet *ti,
  const TSequenceSet *ts, TInstantSet **inter1, TInstantSet **inter2)
{
  return intersection_tsequenceset_tinstantset(ts, ti, inter2, inter1);
}

/**
 * Temporally intersect or synchronize the two temporal values
 *
 * @param[in] seq,ts Input values
 * @param[in] mode Enumeration for either intersect or synchronize
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on times
 */
bool
intersection_tsequence_tsequenceset(const TSequence *seq, const TSequenceSet *ts,
  SyncMode mode, TSequenceSet **inter1, TSequenceSet **inter2)
{
  return synchronize_tsequenceset_tsequence(ts, seq, mode, inter2, inter1);
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the string representation of the temporal value.
 *
 * @param[in] ts Temporal value
 * @param[in] value_out Function called to output the base value
 * depending on its Oid
 */
char *
tsequenceset_to_string(const TSequenceSet *ts, char *(*value_out)(Oid, Datum))
{
  char **strings = palloc(sizeof(char *) * ts->count);
  size_t outlen = 0;
  char prefix[20];
  if (MOBDB_FLAGS_GET_CONTINUOUS(ts->flags) &&
      ! MOBDB_FLAGS_GET_LINEAR(ts->flags))
    sprintf(prefix, "Interp=Stepwise;");
  else
    prefix[0] = '\0';
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    strings[i] = tsequence_to_string(seq, true, value_out);
    outlen += strlen(strings[i]) + 2;
  }
  return stringarr_to_string(strings, ts->count, outlen, prefix, '{', '}');
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Write the binary representation of the temporal value into the
 * buffer.
 *
 * @param[in] ts Temporal value
 * @param[in] buf Buffer
 */
void
tsequenceset_write(const TSequenceSet *ts, StringInfo buf)
{
  pq_sendint32(buf, ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    tsequence_write(seq, buf);
  }
  return;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a new temporal value from its binary representation read from
 * the buffer.
 *
 * @param[in] buf Buffer
 * @param[in] temptype Temporal type
 */
TSequenceSet *
tsequenceset_read(StringInfo buf, CachedType temptype)
{
  int count = (int) pq_getmsgint(buf, 4);
  assert(count > 0);
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  for (int i = 0; i < count; i++)
    sequences[i] = tsequence_read(buf, temptype);
  return tsequenceset_make_free(sequences, count, NORMALIZE_NO);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal sequence set value from from a base value and
 * a timestamp set.
 *
 * @param[in] value Base value
 * @param[in] temptype Temporal type
 * @param[in] ps Period set
 * @param[in] linear True when the resulting value has linear interpolation
*/
TSequenceSet *
tsequenceset_from_base(Datum value, CachedType temptype, const PeriodSet *ps,
  bool linear)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ps->count);
  for (int i = 0; i < ps->count; i++)
  {
    const Period *p = periodset_per_n(ps, i);
    sequences[i] = tsequence_from_base(value, temptype, p, linear);
  }
  return tsequenceset_make_free(sequences, ps->count, NORMALIZE_NO);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of distinct base values of the temporal value with
 * stepwise interpolation
 *
 * @param[in] ts Temporal value
 * @param[out] count Number of elements in the output array
 * @result Array of Datums
 */
Datum *
tsequenceset_values(const TSequenceSet *ts, int *count)
{
  Datum *result = palloc(sizeof(Datum *) * ts->totalcount);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    for (int j = 0; j < seq->count; j++)
      result[k++] = tinstant_value(tsequence_inst_n(seq, j));
  }
  if (k > 1)
  {
    CachedType basetype = temptype_basetype(ts->temptype);
    datumarr_sort(result, k, basetype);
    k = datumarr_remove_duplicates(result, k, basetype);
  }
  *count = k;
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of ranges of base values of the temporal float
 * value.
 */
RangeType **
tfloatseqset_ranges(const TSequenceSet *ts, int *count)
{
  int count1 = MOBDB_FLAGS_GET_LINEAR(ts->flags) ?
    ts->count : ts->totalcount;
  RangeType **ranges = palloc(sizeof(RangeType *) * count1);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tfloatseq_ranges1(seq, &ranges[k]);
  }
  RangeType **result = rangearr_normalize(ranges, k, count);
  if ((*count) > 1)
    rangearr_sort(result, *count);
  pfree_array((void **) ranges, k);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return a pointer to the instant with minimum base value of the
 * temporal value.
 *
 * @note The function does not take into account whether the instant is at an
 * exclusive bound or not
 * @note Function used, e.g., for computing the shortest line between two
 * temporal points from their temporal distance.
 */
const TInstant *
tsequenceset_min_instant(const TSequenceSet *ts)
{
  const TSequence *seq = tsequenceset_seq_n(ts, 0);
  const TInstant *result = tsequence_inst_n(seq, 0);
  Datum min = tinstant_value(result);
  CachedType basetype = temptype_basetype(seq->temptype);
  for (int i = 0; i < ts->count; i++)
  {
    seq = tsequenceset_seq_n(ts, i);
    for (int j = 0; j < seq->count; j++)
    {
      const TInstant *inst = tsequence_inst_n(seq, j);
      Datum value = tinstant_value(inst);
      if (datum_lt(value, min, basetype))
      {
        min = value;
        result = inst;
      }
    }
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return a pointer to the instant with maximum base value of the
 * temporal value.
 *
 * @note The function does not take into account whether the instant is at an
 * exclusive bound or not
 */
const TInstant *
tsequenceset_max_instant(const TSequenceSet *ts)
{
  const TSequence *seq = tsequenceset_seq_n(ts, 0);
  const TInstant *result = tsequence_inst_n(seq, 0);
  Datum max = tinstant_value(result);
  CachedType basetype = temptype_basetype(seq->temptype);
  for (int i = 0; i < ts->count; i++)
  {
    seq = tsequenceset_seq_n(ts, i);
    for (int j = 0; j < seq->count; j++)
    {
      const TInstant *inst = tsequence_inst_n(seq, j);
      Datum value = tinstant_value(inst);
      if (datum_gt(value, max, basetype))
      {
        max = value;
        result = inst;
      }
    }
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the minimum base value of the temporal value.
 */
Datum
tsequenceset_min_value(const TSequenceSet *ts)
{
  CachedType basetype = temptype_basetype(ts->temptype);
  if (basetype == T_INT4)
  {
    TBOX *box = tsequenceset_bbox_ptr(ts);
    return Int32GetDatum((int)(box->xmin));
  }
  if (basetype == T_FLOAT8)
  {
    TBOX *box = tsequenceset_bbox_ptr(ts);
    return Float8GetDatum(box->xmin);
  }
  Datum result = tsequence_min_value(tsequenceset_seq_n(ts, 0));
  for (int i = 1; i < ts->count; i++)
  {
    Datum value = tsequence_min_value(tsequenceset_seq_n(ts, i));
    if (datum_lt(value, result, basetype))
      result = value;
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the maximum base value of the temporal value.
 */
Datum
tsequenceset_max_value(const TSequenceSet *ts)
{
  CachedType basetype = temptype_basetype(ts->temptype);
  if (basetype == T_INT4)
  {
    TBOX *box = tsequenceset_bbox_ptr(ts);
    return Int32GetDatum((int)(box->xmax));
  }
  if (basetype == T_FLOAT8)
  {
    TBOX *box = tsequenceset_bbox_ptr(ts);
    return Float8GetDatum(box->xmax);
  }
  Datum result = tsequence_max_value(tsequenceset_seq_n(ts, 0));
  for (int i = 1; i < ts->count; i++)
  {
    Datum value = tsequence_max_value(tsequenceset_seq_n(ts, i));
    if (datum_gt(value, result, basetype))
      result = value;
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the time on which the temporal value is defined as a
 * period set.
 */
PeriodSet *
tsequenceset_time(const TSequenceSet *ts)
{
  const Period **periods = palloc(sizeof(Period *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    periods[i] = &seq->period;
  }
  PeriodSet *result = periodset_make(periods, ts->count, NORMALIZE_NO);
  pfree(periods);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the timespan of the temporal value.
 */
Interval *
tsequenceset_timespan(const TSequenceSet *ts)
{
  const TSequence *seq1 = tsequenceset_seq_n(ts, 0);
  const TSequence *seq2 = tsequenceset_seq_n(ts, ts->count - 1);
  Interval *result = (Interval *) DatumGetPointer(call_function2(timestamp_mi,
    TimestampTzGetDatum(seq2->period.upper),
    TimestampTzGetDatum(seq1->period.lower)));
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the duration of the temporal value.
 */
Interval *
tsequenceset_duration(const TSequenceSet *ts)
{
  const TSequence *seq = tsequenceset_seq_n(ts, 0);
  Datum result = call_function2(timestamp_mi,
    TimestampTzGetDatum(seq->period.upper), TimestampTzGetDatum(seq->period.lower));
  for (int i = 1; i < ts->count; i++)
  {
    seq = tsequenceset_seq_n(ts, i);
    Datum interval1 = call_function2(timestamp_mi,
      TimestampTzGetDatum(seq->period.upper), TimestampTzGetDatum(seq->period.lower));
    Datum interval2 = call_function2(interval_pl, result, interval1);
    pfree(DatumGetPointer(result)); pfree(DatumGetPointer(interval1));
    result = interval2;
  }
  return (Interval *) DatumGetPointer(result);
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the bounding period on which the temporal value is defined.
 */
void
tsequenceset_period(const TSequenceSet *ts, Period *p)
{
  const TSequence *start = tsequenceset_seq_n(ts, 0);
  const TSequence *end = tsequenceset_seq_n(ts, ts->count - 1);
  period_set(start->period.lower, end->period.upper,
    start->period.lower_inc, end->period.upper_inc, p);
  return;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of pointers to the sequences of the temporal value.
 */
const TSequence **
tsequenceset_sequences_p(const TSequenceSet *ts)
{
  const TSequence **result = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
    result[i] = tsequenceset_seq_n(ts, i);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of sequences of the temporal value.
 */
TSequence **
tsequenceset_sequences(const TSequenceSet *ts)
{
  TSequence **result = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
    result[i] = tsequence_copy(tsequenceset_seq_n(ts, i));
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of segments of the temporal value.
 */
TSequence **
tsequenceset_segments(const TSequenceSet *ts, int *count)
{
  TSequence **result = palloc(sizeof(TSequence *) * ts->totalcount);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tsequence_segments1(seq, &result[k]);
  }
  *count = k;
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the number of distinct instants of the temporal value.
 */
int
tsequenceset_num_instants(const TSequenceSet *ts)
{
  const TInstant *lastinst;
  bool first = true;
  int result = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    result += seq->count;
    if (! first)
    {
      if (tinstant_eq(lastinst, tsequence_inst_n(seq, 0)))
        result --;
    }
    lastinst = tsequence_inst_n(seq, seq->count - 1);
    first = false;
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the n-th distinct instant of the temporal value.
 */
const TInstant *
tsequenceset_inst_n(const TSequenceSet *ts, int n)
{
  assert (n >= 1 && n <= ts->totalcount);
  if (n == 1)
    return tsequence_inst_n(tsequenceset_seq_n(ts, 0), 0);

  /* Continue the search 0-based */
  n--;
  const TInstant *prev, *next;
  bool first = true, found = false;
  int i = 0, count = 0, prevcount = 0;
  while (i < ts->count)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    count += seq->count;
    if (! first && tinstant_eq(prev, tsequence_inst_n(seq, 0)))
    {
        prevcount --;
        count --;
    }
    if (prevcount <= n && n < count)
    {
      next = tsequence_inst_n(seq, n - prevcount);
      found = true;
      break;
    }
    prevcount = count;
    prev = tsequence_inst_n(seq, seq->count - 1);
    first = false;
    i++;
  }
  if (! found)
    return NULL;
  return next;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the distinct instants of the temporal value.
 */
const TInstant **
tsequenceset_instants(const TSequenceSet *ts)
{
  const TInstant **result = palloc(sizeof(TInstant *) * ts->totalcount);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    for (int j = 0; j < seq->count; j++)
      result[k++] = tsequence_inst_n(seq, j);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start timestamp of the temporal value.
 */
TimestampTz
tsequenceset_start_timestamp(const TSequenceSet *ts)
{
  const TSequence *seq = tsequenceset_seq_n(ts, 0);
  return seq->period.lower;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end timestamp of the temporal value.
 */
TimestampTz
tsequenceset_end_timestamp(const TSequenceSet *ts)
{
  const TSequence *seq = tsequenceset_seq_n(ts, ts->count - 1);
  return seq->period.upper;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the number of distinct timestamps of the temporal value.
 */
int
tsequenceset_num_timestamps(const TSequenceSet *ts)
{
  TimestampTz lasttime;
  bool first = true;
  int result = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    result += seq->count;
    if (! first)
    {
      if (lasttime == tsequence_inst_n(seq, 0)->t)
        result --;
    }
    lasttime = tsequence_inst_n(seq, seq->count - 1)->t;
    first = false;
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the n-th distinct timestamp of the temporal value.
 */
bool
tsequenceset_timestamp_n(const TSequenceSet *ts, int n, TimestampTz *result)
{
  bool found = false;
  if (n < 1)
    return false;
  if (n == 1)
  {
    *result = tsequence_inst_n(tsequenceset_seq_n(ts, 0), 0)->t;
    return true ;
  }

  /* Continue the search 0-based */
  n--;
  TimestampTz prev, next;
  bool first = true;
  int i = 0, count = 0, prevcount = 0;
  while (i < ts->count)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    count += seq->count;
    if (! first && prev == tsequence_inst_n(seq, 0)->t)
    {
        prevcount --;
        count --;
    }
    if (prevcount <= n && n < count)
    {
      next = tsequence_inst_n(seq, n - prevcount)->t;
      found = true;
      break;
    }
    prevcount = count;
    prev = tsequence_inst_n(seq, seq->count - 1)->t;
    first = false;
    i++;
  }
  if (! found)
    return false;
  *result = next;
  return true;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of distinct timestamps of the temporal value.
 */
TimestampTz *
tsequenceset_timestamps(const TSequenceSet *ts, int *count)
{
  TimestampTz *result = palloc(sizeof(TimestampTz) * ts->totalcount);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tsequence_timestamps1(seq, &result[k]);
  }
  if (k > 1)
  {
    timestamparr_sort(result, k);
    k = timestamparr_remove_duplicates(result, k);
  }
  *count = k;
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the base value of the temporal value at the timestamp.
 *
 * @param[in] ts Temporal value
 * @param[in] t Timestamp
 * @param[out] result Base value
 * @result Return true if the timestamp is contained in the temporal value
 * @pre A bounding box test has been done before by the calling function
 */
bool
tsequenceset_value_at_timestamp(const TSequenceSet *ts, TimestampTz t,
  Datum *result)
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tsequence_value_at_timestamp(tsequenceset_seq_n(ts, 0), t, result);

  /* General case */
  int loc;
  if (! tsequenceset_find_timestamp(ts, t, &loc))
    return false;
  return tsequence_value_at_timestamp(tsequenceset_seq_n(ts, loc), t, result);
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the base value of the temporal value at the timestamp when the
 * timestamp may be at an exclusive bound.
 *
 * @param[in] ts Temporal value
 * @param[in] t Timestamp
 * @param[out] result Base value
 * @result Return true if the timestamp is found in the temporal value
 */
bool
tsequenceset_value_at_timestamp_inc(const TSequenceSet *ts, TimestampTz t,
  Datum *result)
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tsequence_value_at_timestamp_inc(tsequenceset_seq_n(ts, 0),
      t, result);

  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    if (contains_period_timestamp(&seq->period, t))
      return tsequence_value_at_timestamp(seq, t, result);
    /* Test whether the timestamp is at one of the bounds */
    const TInstant *inst = tsequence_inst_n(seq, 0);
    if (inst->t == t)
      return tinstant_value_at_timestamp(inst, t, result);
    inst = tsequence_inst_n(seq, seq->count - 1);
    if (inst->t == t)
      return tinstant_value_at_timestamp(inst, t, result);
  }
  /* Since this function is always called with a timestamp that appears
   * in the sequence set value the next statement is never reached */
  return false;
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast a temporal float value as a floatrange.
 */
RangeType *
tfloatseqset_range(const TSequenceSet *ts)
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tfloatseq_range(tsequenceset_seq_n(ts, 0));

  /* General case */
  TBOX *box = tsequenceset_bbox_ptr(ts);
  Datum min = Float8GetDatum(box->xmin);
  Datum max = Float8GetDatum(box->xmax);
  /* It step interpolation */
  if(! MOBDB_FLAGS_GET_LINEAR(ts->flags))
    return range_make(min, max, true, true, T_FLOAT8);

  /* Linear interpolation */
  RangeType **ranges = palloc(sizeof(RangeType *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    ranges[i] = tfloatseq_range(seq);
  }
  int newcount;
  RangeType **normranges = rangearr_normalize(ranges, ts->count, &newcount);
  RangeType *result;
  if (newcount == 1)
  {
    result = normranges[0];
    pfree_array((void **) ranges, ts->count);
    pfree(normranges);
    return result;
  }

  RangeType *start = normranges[0];
  RangeType *end = normranges[newcount - 1];
  result = range_make(lower_datum(start), upper_datum(end),
    lower_inc(start), upper_inc(end), T_FLOAT8);
  pfree_array((void **) normranges, newcount);
  pfree_array((void **) ranges, ts->count);
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast the temporal integer value as a temporal float value.
 */
TSequenceSet *
tintseqset_tfloatseqset(const TSequenceSet *ts)
{
  TSequenceSet *result = tsequenceset_copy(ts);
  result->temptype = T_TFLOAT;
  MOBDB_FLAGS_SET_CONTINUOUS(result->flags, true);
  MOBDB_FLAGS_SET_LINEAR(result->flags, false);
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = (TSequence *) tsequenceset_seq_n(result, i);
    seq->temptype = T_TFLOAT;
    for (int j = 0; j < seq->count; j++)
    {
      TInstant *inst = (TInstant *) tsequence_inst_n(seq, j);
      inst->temptype = T_TFLOAT;
      Datum *value_ptr = tinstant_value_ptr(inst);
      *value_ptr = Float8GetDatum((double)DatumGetInt32(tinstant_value(inst)));
    }
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast the temporal float value as a temporal integer value.
 */
TSequenceSet *
tfloatseqset_tintseqset(const TSequenceSet *ts)
{
  if (MOBDB_FLAGS_GET_LINEAR(ts->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Cannot cast temporal float with linear interpolation to temporal integer")));
  TSequenceSet *result = tsequenceset_copy(ts);
  result->temptype = T_TINT;
  MOBDB_FLAGS_SET_CONTINUOUS(result->flags, false);
  MOBDB_FLAGS_SET_LINEAR(result->flags, false);
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = (TSequence *) tsequenceset_seq_n(result, i);
    seq->temptype = T_TINT;
    for (int j = 0; j < seq->count; j++)
    {
      TInstant *inst = (TInstant *) tsequence_inst_n(seq, j);
      inst->temptype = T_TINT;
      Datum *value_ptr = tinstant_value_ptr(inst);
      *value_ptr = Int32GetDatum((double)DatumGetFloat8(tinstant_value(inst)));
    }
  }
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal instant value into a temporal sequence set.
 * value.
 */
TSequenceSet *
tinstant_tsequenceset(const TInstant *inst, bool linear)
{
  TSequence *seq = tinstant_tsequence(inst, linear);
  TSequenceSet *result = tsequence_tsequenceset(seq);
  pfree(seq);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal instant set value into a temporal sequence set
 * value.
 */
TSequenceSet *
tinstantset_tsequenceset(const TInstantSet *ti, bool linear)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    sequences[i] = tinstant_tsequence(inst, linear);
  }
  TSequenceSet *result = tsequenceset_make((const TSequence **) sequences,
    ti->count, NORMALIZE_NO);
  pfree(sequences);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal sequence set value from the temporal sequence.
 */
TSequenceSet *
tsequence_tsequenceset(const TSequence *seq)
{
  return tsequenceset_make(&seq, 1, NORMALIZE_NO);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal value with continuous base type from stepwise
 * to linear interpolation.
 */
TSequenceSet *
tstepseqset_tlinearseqset(const TSequenceSet *ts)
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tstepseq_tlinearseq(tsequenceset_seq_n(ts, 0));

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->totalcount);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tstepseq_tlinearseq1(seq, &sequences[k]);
  }
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Shift and/or scale the time span of the temporal value by the two
 * intervals.
 *
 * @pre The duration is greater than 0 if it is not NULL
 */
TSequenceSet *
tsequenceset_shift_tscale(const TSequenceSet *ts, const Interval *start,
  const Interval *duration)
{
  assert(start != NULL || duration != NULL);

  /* Copy the input sequence set to the result */
  TSequenceSet *result = tsequenceset_copy(ts);

  /* Determine the shift and/or the scale values */
  Period p1, p2;
  const TSequence *seq1 = tsequenceset_seq_n(ts, 0);
  const TSequence *seq2 = tsequenceset_seq_n(ts, ts->count - 1);
  const TInstant *inst1 = tsequence_inst_n(seq1, 0);
  const TInstant *inst2 = tsequence_inst_n(seq2, seq2->count - 1);
  period_set(inst1->t, inst2->t, seq1->period.lower_inc,
    seq2->period.upper_inc, &p1);
  period_set(p1.lower, p1.upper, p1.lower_inc, p1.upper_inc, &p2);
  period_shift_tscale(start, duration, &p2);
  TimestampTz shift;
  if (start != NULL)
    shift = p2.lower - p1.lower;
  double scale;
  bool instant = (p2.lower == p2.upper);
  /* If the sequence set is instantaneous we cannot scale */
  if (duration != NULL && ! instant)
    scale = (double) (p2.upper - p2.lower) / (double) (p1.upper - p1.lower);

  /* Shift and/or scale each composing sequence */
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = (TSequence *) tsequenceset_seq_n(result, i);
    /* Shift and/or scale the bounding period of the sequence */
    if (start != NULL && (duration == NULL || seq->count == 1))
    {
      seq->period.lower += shift;
      seq->period.upper += shift;
    }
    /* If the sequence is instantaneous we cannot scale */
    if (duration != NULL && seq->count > 1)
    {
      seq->period.lower = p2.lower + (seq->period.lower - p1.lower) * scale;
      seq->period.upper = p2.lower + (seq->period.upper - p1.lower) * scale;
    }
    /* Shift and/or scale each composing instant */
    for (int j = 0; j < seq->count; j++)
    {
      TInstant *inst = (TInstant *) tsequence_inst_n(seq, j);
      /* Shift and/or scale the bounding period of the sequence */
      if (start != NULL)
        inst->t += shift;
      /* If the sequence is instantaneous we cannot scale */
      if (duration != NULL && seq->count > 1)
        inst->t = p2.lower + (inst->t - p2.lower) * scale;
    }
  }
  return result;
}

/*****************************************************************************
 * Ever/always functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is ever equal to the base value.
 */
bool
tsequenceset_ever_eq(const TSequenceSet *ts, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_eq((Temporal *)ts, value, EVER))
    return false;

  for (int i = 0; i < ts->count; i++)
    if (tsequence_ever_eq(tsequenceset_seq_n(ts, i), value))
      return true;
  return false;
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is always equal to the base value.
 */
bool
tsequenceset_always_eq(const TSequenceSet *ts, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_eq((Temporal *)ts, value, ALWAYS))
    return false;

  /* The bounding box test above is enough to compute the answer for
   * temporal numbers */
  if (tnumber_type(ts->temptype))
    return true;

  for (int i = 0; i < ts->count; i++)
    if (! tsequence_always_eq(tsequenceset_seq_n(ts, i), value))
      return false;
  return true;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is ever less than the base value.
 */
bool
tsequenceset_ever_lt(const TSequenceSet *ts, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *)ts, value, EVER))
    return false;

  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    if (tsequence_ever_lt(seq, value))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is ever less than or equal
 * to the base value.
 */
bool
tsequenceset_ever_le(const TSequenceSet *ts, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *)ts, value, EVER))
    return false;

  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    if (tsequence_ever_le(seq, value))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is always less than the base value.
 */
bool
tsequenceset_always_lt(const TSequenceSet *ts, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *)ts, value, ALWAYS))
    return false;

  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    if (! tsequence_always_lt(seq, value))
      return false;
  }
  return true;
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is always less than or equal
 * to the base value.
 */
bool
tsequenceset_always_le(const TSequenceSet *ts, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *)ts, value, ALWAYS))
    return false;

  /* The bounding box test above is enough to compute
   * the answer for temporal numbers */
  if (tnumber_type(ts->temptype))
    return true;

  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    if (! tsequence_always_le(seq, value))
      return false;
  }
  return true;
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the base value.
 *
 * @note There is no bounding box test in this function, it is done in the
 * dispatch function for all temporal types.
 */
TSequenceSet *
tsequenceset_restrict_value(const TSequenceSet *ts, Datum value, bool atfunc)
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tsequence_restrict_value(tsequenceset_seq_n(ts, 0), value, atfunc);

  /* General case */
  int count = ts->totalcount;
  /* For minus and linear interpolation we need the double of the count */
  if (! atfunc && MOBDB_FLAGS_GET_LINEAR(ts->flags))
    count *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tsequence_restrict_value1(seq, value, atfunc, &sequences[k]);
  }
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the (complement of the) array of
 * base values.
 *
 * @param[in] ts Temporal value
 * @param[in] values Array of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True when the restriction is at, false for minus
 * @pre There are no duplicates values in the array
 */
TSequenceSet *
tsequenceset_restrict_values(const TSequenceSet *ts, const Datum *values,
  int count, bool atfunc)
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tsequence_restrict_values(tsequenceset_seq_n(ts, 0), values,
      count, atfunc);

  /* General case
   * Compute the AT function */
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->totalcount * count);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tsequence_at_values1(seq, values, count, &sequences[k]);
  }
  TSequenceSet *atresult = tsequenceset_make_free(sequences, k, NORMALIZE);
  if (atfunc)
    return atresult;

  /*
   * MINUS function
   * Compute the complement of the previous value.
   */
  if (k == 0)
    return tsequenceset_copy(ts);

  PeriodSet *ps1 = tsequenceset_time(ts);
  PeriodSet *ps2 = tsequenceset_time(atresult);
  PeriodSet *ps = minus_periodset_periodset(ps1, ps2);
  TSequenceSet *result = NULL;
  if (ps != NULL)
  {
    result = tsequenceset_restrict_periodset(ts, ps, REST_AT);
    pfree(ps);
  }
  pfree(atresult); pfree(ps1); pfree(ps2);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal number to the range of base values.
 *
 * @note It is supposed that a bounding box test has been done in the dispatch
 * function.
 */
TSequenceSet *
tnumberseqset_restrict_range(const TSequenceSet *ts, const RangeType *range,
  bool atfunc)
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tnumberseq_restrict_range(tsequenceset_seq_n(ts, 0), range,
      atfunc);

  /* General case */
  int count = ts->totalcount;
  /* For minus and linear interpolation we need the double of the count */
  if (! atfunc && MOBDB_FLAGS_GET_LINEAR(ts->flags))
    count *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tnumberseq_restrict_range2(seq, range, atfunc, &sequences[k]);
  }
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal number to the (complement of the) array of
 * ranges of base values
 *
 * @param[in] ts Temporal number
 * @param[in] normranges Array of ranges of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True when the restriction is at, false for minus
 * @return Resulting temporal number value
 * @pre The array of ranges is normalized
 * @note A bounding box test has been done in the dispatch function.
 */
TSequenceSet *
tnumberseqset_restrict_ranges(const TSequenceSet *ts, RangeType **normranges,
  int count, bool atfunc)
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tnumberseq_restrict_ranges(tsequenceset_seq_n(ts, 0),
      normranges, count, atfunc, BBOX_TEST_NO);

  /* General case */
  int maxcount = ts->totalcount * count;
  /* For minus and linear interpolation we need the double of the count */
  if (! atfunc && MOBDB_FLAGS_GET_LINEAR(ts->flags))
    maxcount *= 2;
  TSequence **sequences = palloc(sizeof(TSequence *) * maxcount);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tnumberseq_restrict_ranges1(seq, normranges, count, atfunc,
      BBOX_TEST, &sequences[k]);
  }
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to (the complement of) the
 * minimum/maximum base value
 */
TSequenceSet *
tsequenceset_restrict_minmax(const TSequenceSet *ts, bool min, bool atfunc)
{
  Datum minmax = min ? tsequenceset_min_value(ts) : tsequenceset_max_value(ts);
  return tsequenceset_restrict_value(ts, minmax, atfunc);
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the (the complement of) timestamp.
 */
Temporal *
tsequenceset_restrict_timestamp(const TSequenceSet *ts, TimestampTz t,
  bool atfunc)
{
  /* Bounding box test */
  Period p;
  tsequenceset_period(ts, &p);
  if (! contains_period_timestamp(&p, t))
    return atfunc ? NULL : (Temporal *) tsequenceset_copy(ts);

  /* Singleton sequence set */
  if (ts->count == 1)
    return atfunc ?
      (Temporal *) tsequence_at_timestamp(tsequenceset_seq_n(ts, 0), t) :
      (Temporal *) tsequence_minus_timestamp(tsequenceset_seq_n(ts, 0), t);

  /* General case */
  const TSequence *seq;
  if (atfunc)
  {
    int loc;
    if (! tsequenceset_find_timestamp(ts, t, &loc))
      return NULL;
    seq = tsequenceset_seq_n(ts, loc);
    return (Temporal *) tsequence_at_timestamp(seq, t);
  }
  else
  {
    /* At most one composing sequence can be split into two */
    TSequence **sequences = palloc(sizeof(TSequence *) * (ts->count + 1));
    int k = 0;
    int i;
    for (i = 0; i < ts->count; i++)
    {
      seq = tsequenceset_seq_n(ts, i);
      k += tsequence_minus_timestamp1(seq, t, &sequences[k]);
      if (t < seq->period.upper)
      {
        i++;
        break;
      }
    }
    /* Copy the remaining sequences if went out of the for loop with the break */
    for (int j = i; j < ts->count; j++)
      sequences[k++] = tsequence_copy(tsequenceset_seq_n(ts, j));
    /* k is never equal to 0 since in that case it is a singleton sequence set
       and it has been dealt by tsequence_minus_timestamp above */
    return (Temporal *) tsequenceset_make_free(sequences, k, NORMALIZE_NO);
  }
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the (complement of the) timestamp set.
 */
Temporal *
tsequenceset_restrict_timestampset(const TSequenceSet *ts1,
  const TimestampSet *ts2, bool atfunc)
{
  /* Singleton timestamp set */
  if (ts2->count == 1)
  {
    Temporal *temp = tsequenceset_restrict_timestamp(ts1,
      timestampset_time_n(ts2, 0), atfunc);
    if (atfunc && temp != NULL)
    {
      TInstant *inst = (TInstant *) temp;
      Temporal *result = (Temporal *) tinstantset_make((const TInstant **) &inst,
        1, MERGE_NO);
      pfree(inst);
      return result;
    }
    return temp;
  }

  /* Bounding box test */
  Period p1;
  tsequenceset_period(ts1, &p1);
  const Period *p2 = timestampset_bbox_ptr(ts2);
  if (! overlaps_period_period(&p1, p2))
    return atfunc ? NULL : (Temporal *) tsequenceset_copy(ts1);

  /* Singleton sequence set */
  if (ts1->count == 1)
    return atfunc ?
      (Temporal *) tsequence_at_timestampset(tsequenceset_seq_n(ts1, 0), ts2) :
      (Temporal *) tsequence_minus_timestampset(tsequenceset_seq_n(ts1, 0), ts2);

  /* General case */
  const TSequence *seq;
  if (atfunc)
  {
    TInstant **instants = palloc(sizeof(TInstant *) * ts2->count);
    int count = 0;
    int i = 0, j = 0;
    while (i < ts2->count && j < ts1->count)
    {
      seq = tsequenceset_seq_n(ts1, j);
      TimestampTz t = timestampset_time_n(ts2, i);
      if (contains_period_timestamp(&seq->period, t))
      {
        instants[count++] = tsequence_at_timestamp(seq, t);
        i++;
      }
      else
      {
        if (t <= seq->period.lower)
          i++;
        if (t >= seq->period.upper)
          j++;
      }
    }
    return (Temporal *) tinstantset_make_free(instants, count, MERGE_NO);
  }
  else
  {
    /* For the minus case each timestamp will split at most one
     * composing sequence into two */
    TSequence **sequences = palloc(sizeof(TSequence *) *
      (ts1->count + ts2->count + 1));
    int k = 0;
    for (int i = 0; i < ts1->count; i++)
    {
      seq = tsequenceset_seq_n(ts1, i);
      k += tsequence_minus_timestampset1(seq, ts2, &sequences[k]);

    }
    return (Temporal *) tsequenceset_make_free(sequences, k, NORMALIZE);
  }
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the (complement of the) period.
 */
TSequenceSet *
tsequenceset_restrict_period(const TSequenceSet *ts, const Period *p,
  bool atfunc)
{
  /* Bounding box test */
  Period p1;
  tsequenceset_period(ts, &p1);
  if (! overlaps_period_period(&p1, p))
    return atfunc ? NULL : tsequenceset_copy(ts);

  TSequence *seq;
  TSequenceSet *result;

  /* Singleton sequence set */
  if (ts->count == 1)
  {
    if (atfunc)
    {
      seq = tsequence_at_period(tsequenceset_seq_n(ts, 0), p);
      result = tsequence_tsequenceset(seq);
      pfree(seq);
      return result;
    }
    else
      tsequence_minus_period(tsequenceset_seq_n(ts, 0), p);
  }

  /* General case */
  if (atfunc)
  {
    /* AT */
    int loc;
    tsequenceset_find_timestamp(ts, p->lower, &loc);
    /* We are sure that loc < ts->count due to the bounding period test above */
    TSequence **sequences = palloc(sizeof(TSequence *) * (ts->count - loc));
    TSequence *tofree[2];
    int k = 0, l = 0;
    for (int i = loc; i < ts->count; i++)
    {
      seq = (TSequence *) tsequenceset_seq_n(ts, i);
      if (contains_period_period(p, &seq->period))
        sequences[k++] = seq;
      else if (overlaps_period_period(p, &seq->period))
      {
        TSequence *newseq = tsequence_at_period(seq, p);
        sequences[k++] = tofree[l++] = newseq;
      }
      int cmp = timestamp_cmp_internal(p->upper, seq->period.upper);
      if (cmp < 0 || (cmp == 0 && seq->period.upper_inc))
        break;
    }
    if (k == 0)
    {
      pfree(sequences);
      return NULL;
    }
    /* Since both the tsequenceset and the period are normalized it is not
     * necessary to normalize the result of the projection */
    result = tsequenceset_make((const TSequence **) sequences, k, NORMALIZE_NO);
    for (int i = 0; i < l; i++)
      pfree(tofree[i]);
    pfree(sequences);
    return result;
  }
  else
  {
    /* MINUS */
    PeriodSet *ps = tsequenceset_time(ts);
    PeriodSet *resultps = minus_periodset_period(ps, p);
    result = NULL;
    if (resultps != NULL)
    {
      result = tsequenceset_restrict_periodset(ts, resultps, REST_AT);
      pfree(resultps);
    }
    pfree(ps);
    return result;
  }
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the (complement of the) period set.
 */
TSequenceSet *
tsequenceset_restrict_periodset(const TSequenceSet *ts, const PeriodSet *ps,
  bool atfunc)
{
  /* Singleton period set */
  if (ps->count == 1)
    return tsequenceset_restrict_period(ts, periodset_per_n(ps, 0), atfunc);

  /* Bounding box test */
  Period p1;
  tsequenceset_period(ts, &p1);
  const Period *p2 = periodset_bbox_ptr(ps);
  if (! overlaps_period_period(&p1, p2))
    return atfunc ? NULL : tsequenceset_copy(ts);

  /* Singleton sequence set */
  if (ts->count == 1)
    return tsequence_restrict_periodset(tsequenceset_seq_n(ts, 0), ps, atfunc);

  /* General case */
  TSequence **sequences;
  int i = 0, j = 0, k = 0;
  if (atfunc)
  {
    TimestampTz t = Max(p1.lower, p2->lower);
    tsequenceset_find_timestamp(ts, t, &i);
    periodset_find_timestamp(ps, t, &j);
    sequences = palloc(sizeof(TSequence *) * (ts->count + ps->count - i - j));
  }
  else
    sequences = palloc(sizeof(TSequence *) * (ts->count + ps->count));
  while (i < ts->count && j < ps->count)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    p2 = periodset_per_n(ps, j);
    /* The sequence and the period do not overlap */
    if (before_period_period(&seq->period, p2))
    {
      if (! atfunc)
        /* Copy the sequence */
        sequences[k++] = tsequence_copy(seq);
      i++;
    }
    else if (overlaps_period_period(&seq->period, p2))
    {
      if (atfunc)
      {
        /* Compute the restriction of the sequence and the period */
        TSequence *seq1 = tsequence_at_period(seq, p2);
        if (seq1 != NULL)
          sequences[k++] = seq1;
        int cmp = timestamp_cmp_internal(seq->period.upper, p2->upper);
        if (cmp == 0 && seq->period.upper_inc == p2->upper_inc)
        {
          i++; j++;
        }
        else if (cmp < 0 ||
          (cmp == 0 && ! seq->period.upper_inc && p2->upper_inc))
          i++;
        else
          j++;
      }
      else
      {
        /* Compute the difference of the sequence and the FULL periodset.
         * Notice that we cannot compute the difference with the
         * current period without replicating the functionality in
         * tsequence_minus_periodset */
        k += tsequence_minus_periodset(seq, ps, j, &sequences[k]);
        i++;
      }
    }
    else
      j++;
  }
  if (! atfunc)
  {
    /* For minus copy the sequences after the period set */
    while (i < ts->count)
      sequences[k++] = tsequence_copy(tsequenceset_seq_n(ts, i++));
  }
  /* It is necessary to normalize despite the fact that both the tsequenceset
  * and the periodset are normalized */
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/*****************************************************************************
 * Intersects functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersect the timestamp.
 */
bool
tsequenceset_intersects_timestamp(const TSequenceSet *ts, TimestampTz t)
{
  int loc;
  if (tsequenceset_find_timestamp(ts, t, &loc))
    return true;
  return false;
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersect the timestamp set.
 */
bool
tsequenceset_intersects_timestampset(const TSequenceSet *ts,
  const TimestampSet *ts1)
{
  for (int i = 0; i < ts1->count; i++)
    if (tsequenceset_intersects_timestamp(ts, timestampset_time_n(ts1, i)))
      return true;
  return false;
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersect the period.
 */
bool
tsequenceset_intersects_period(const TSequenceSet *ts, const Period *p)
{
  /* Binary search of lower and upper bounds of period */
  int loc1, loc2;
  if (tsequenceset_find_timestamp(ts, p->lower, &loc1) ||
    tsequenceset_find_timestamp(ts, p->upper, &loc2))
    return true;

  for (int i = loc1; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    if (overlaps_period_period(&seq->period, p))
      return true;
    if (p->upper < seq->period.upper)
      break;
  }
  return false;
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersect the period set.
 */
bool
tsequenceset_intersects_periodset(const TSequenceSet *ts, const PeriodSet *ps)
{
  for (int i = 0; i < ps->count; i++)
    if (tsequenceset_intersects_period(ts, periodset_per_n(ps, i)))
      return true;
  return false;
}

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_agg
 * @brief Return the integral (area under the curve) of the temporal number
 */
double
tnumberseqset_integral(const TSequenceSet *ts)
{
  double result = 0;
  for (int i = 0; i < ts->count; i++)
    result += tnumberseq_integral(tsequenceset_seq_n(ts, i));
  return result;
}

/**
 * Return the duration of the temporal value as a double
 */
static double
tsequenceset_interval_double(const TSequenceSet *ts)
{
  double result = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    result += (double) (seq->period.upper - seq->period.lower);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_agg
 * @brief Return the time-weighted average of the temporal number
 */
double
tnumberseqset_twavg(const TSequenceSet *ts)
{
  double duration = tsequenceset_interval_double(ts);
  double result;
  if (duration == 0.0)
  {
    result = 0;
    for (int i = 0; i < ts->count; i++)
      result += tnumberseq_twavg(tsequenceset_seq_n(ts, i));
    return result / ts->count;
  }
  else
    result = tnumberseqset_integral(ts) / duration;
  return result;
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the two temporal sequence set values are equal.
 *
 * @pre The arguments are of the same base type
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
tsequenceset_eq(const TSequenceSet *ts1, const TSequenceSet *ts2)
{
  assert(ts1->temptype == ts2->temptype);
  /* If number of sequences or flags are not equal */
  if (ts1->count != ts2->count || ts1->flags != ts2->flags)
    return false;

  /* If bounding boxes are not equal */
  void *box1 = tsequenceset_bbox_ptr(ts1);
  void *box2 = tsequenceset_bbox_ptr(ts2);
  if (! temporal_bbox_eq(box1, box2, ts1->temptype))
    return false;

  /* Compare the composing sequences */
  for (int i = 0; i < ts1->count; i++)
  {
    const TSequence *seq1 = tsequenceset_seq_n(ts1, i);
    const TSequence *seq2 = tsequenceset_seq_n(ts2, i);
    if (! tsequence_eq(seq1, seq2))
      return false;
  }
  return true;
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal value
 * is less than, equal, or greater than the second one.
 *
 * @pre The arguments are of the same base type
 * @note Period and bounding box comparison have been done by the calling
 * function temporal_cmp
 */
int
tsequenceset_cmp(const TSequenceSet *ts1, const TSequenceSet *ts2)
{
  assert(ts1->temptype == ts2->temptype);

  /* Compare composing instants */
  int count = Min(ts1->count, ts2->count);
  for (int i = 0; i < count; i++)
  {
    const TSequence *seq1 = tsequenceset_seq_n(ts1, i);
    const TSequence *seq2 = tsequenceset_seq_n(ts2, i);
    int result = tsequence_cmp(seq1, seq2);
    if (result)
      return result;
  }

  /* ts1->count == ts2->count because of the bounding box and the
   * composing sequence tests above */

  /* ts1->flags == ts2->flags since all the composing sequences are equal */

  /* The two values are equal */
  return 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of
 * the elements.
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the 32-bit hash value of the temporal value.
 */
uint32
tsequenceset_hash(const TSequenceSet *ts)
{
  uint32 result = 1;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    uint32 seq_hash = tsequence_hash(seq);
    result = (result << 5) - result + seq_hash;
  }
  return result;
}

/*****************************************************************************/
