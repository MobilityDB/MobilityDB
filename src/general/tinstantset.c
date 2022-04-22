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
 * @file tinstantset.c
 * @brief General functions for temporal instant sets.
 */

#include "general/tinstantset.h"

/* PostgreSQL */
#include <assert.h>
#include <libpq/pqformat.h>
#include <utils/builtins.h>
#include <utils/lsyscache.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/timetypes.h"
#include "general/timestampset.h"
#include "general/period.h"
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/temporaltypes.h"
#include "general/tempcache.h"
#include "general/temporal_util.h"
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
tinstantset_bbox_ptr(const TInstantSet *ti)
{
  return (void *)(((char *) ti) + double_pad(sizeof(TInstantSet)));
}

/**
 * Copy in the second argument the bounding box of the temporal value
 */
void
tinstantset_bbox(const TInstantSet *ti, void *box)
{
  memset(box, 0, ti->bboxsize);
  memcpy(box, tinstantset_bbox_ptr(ti), ti->bboxsize);
  return;
}

/**
 * Return a pointer to the offsets array of the temporal value
 */
static size_t *
tinstantset_offsets_ptr(const TInstantSet *ti)
{
  return (size_t *)(((char *) ti) + double_pad(sizeof(TInstantSet)) +
    double_pad(ti->bboxsize));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the n-th instant of the temporal value.
 */
const TInstant *
tinstantset_inst_n(const TInstantSet *ti, int index)
{
  return (TInstant *)(
    /* start of data */
    ((char *) ti) + double_pad(sizeof(TInstantSet)) + ti->bboxsize +
      ti->count * sizeof(size_t) +
      /* offset */
      (tinstantset_offsets_ptr(ti))[index]);
}

/**
 * Ensure the validity of the arguments when creating a temporal value
 */
static void
tinstantset_make_valid(const TInstant **instants, int count, bool merge)
{
  /* Test the validity of the instants */
  assert(count > 0);
  ensure_tinstarr(instants, count);
  ensure_valid_tinstarr(instants, count, merge, INSTANTSET);
  return;
}

/**
 * Creating a temporal value from its arguments
 * @pre The validity of the arguments has been tested before
 */
TInstantSet *
tinstantset_make1(const TInstant **instants, int count)
{
  /* Get the bounding box size */
  size_t bboxsize = temporal_bbox_size(instants[0]->temptype);

  /* Compute the size of the temporal instant set */
  /* Bounding box size */
  size_t memsize = bboxsize;
  /* Size of composing instants */
  for (int i = 0; i < count; i++)
    memsize += double_pad(VARSIZE(instants[i]));
  /* Size of the struct and the offset array */
  memsize +=  double_pad(sizeof(TInstantSet)) + count * sizeof(size_t);
  /* Create the TInstantSet */
  TInstantSet *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  result->count = count;
  result->temptype = instants[0]->temptype;
  result->subtype = INSTANTSET;
  result->bboxsize = bboxsize;
  bool continuous = MOBDB_FLAGS_GET_CONTINUOUS(instants[0]->flags);
  MOBDB_FLAGS_SET_CONTINUOUS(result->flags, continuous);
  MOBDB_FLAGS_SET_LINEAR(result->flags, continuous);
  MOBDB_FLAGS_SET_X(result->flags, true);
  MOBDB_FLAGS_SET_T(result->flags, true);
  if (tgeo_type(instants[0]->temptype))
  {
    MOBDB_FLAGS_SET_Z(result->flags, MOBDB_FLAGS_GET_Z(instants[0]->flags));
    MOBDB_FLAGS_SET_GEODETIC(result->flags, MOBDB_FLAGS_GET_GEODETIC(instants[0]->flags));
  }
  /* Initialization of the variable-length part */
  /*
   * Compute the bounding box
   * Only external types have bounding box, internal types such
   * as double2, double3, or double4 do not have bounding box
   */
  if (bboxsize != 0)
  {
    tinstantset_make_bbox(instants, count, tinstantset_bbox_ptr(result));
  }
  /* Store the composing instants */
  size_t pdata = double_pad(sizeof(TInstantSet)) + double_pad(bboxsize) +
    count * sizeof(size_t);
  size_t pos = 0;
  for (int i = 0; i < count; i++)
  {
    memcpy(((char *)result) + pdata + pos, instants[i], VARSIZE(instants[i]));
    (tinstantset_offsets_ptr(result))[i] = pos;
    pos += double_pad(VARSIZE(instants[i]));
  }

  return result;
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal instant set value from the array of temporal
 * instant values.
 *
 * For example, the memory structure of a temporal instant set value
 * with two instants is as follows
 * @code
 *  -----------------------------------------------------------
 *  ( TInstantSet )_X | ( bbox )_X | offset_0 | offset_1 | ...
 *  -----------------------------------------------------------
 *  -------------------------------------
 *  ( TInstant_0 )_X | ( TInstant_1 )_X |
 *  -------------------------------------
 * @endcode
 * where the `_X` are unused bytes added for double padding, `offset_0` and
 * `offset_1` are offsets for the corresponding instants
 *
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] merge True when overlapping instants are allowed as required in
 * merge operations
 */
TInstantSet *
tinstantset_make(const TInstant **instants, int count, bool merge)
{
  tinstantset_make_valid(instants, count, merge);
  return tinstantset_make1(instants, count);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal instant set value from the array of temporal
 * instant values and free the array and the instants after the creation.
 *
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] merge True when overlapping instants are allowed as required in
 * merge operations
 */
TInstantSet *
tinstantset_make_free(TInstant **instants, int count, bool merge)
{
  if (count == 0)
  {
    pfree(instants);
    return NULL;
  }
  TInstantSet *result = tinstantset_make((const TInstant **) instants,
    count, merge);
  pfree_array((void **) instants, count);
  return result;
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Return a copy of the temporal value.
 */
TInstantSet *
tinstantset_copy(const TInstantSet *ti)
{
  TInstantSet *result = palloc0(VARSIZE(ti));
  memcpy(result, ti, VARSIZE(ti));
  return result;
}

/**
 * Return the location of the timestamp in the temporal instant set
 * value using binary search
 *
 * If the timestamp is contained in the temporal value, the index
 * of the sequence is returned in the output parameter. Otherwise,
 * returns a number encoding whether the timestamp is before, between
 * two sequences, or after the temporal value.
 * For example, given a value composed of 3 instants and a timestamp,
 * the value returned in the output parameter is as follows:
 * @code
 *            0        1        2
 *            |        |        |
 * 1)    t^                            => result = 0
 * 2)        t^                        => result = 0
 * 3)            t^                    => result = 1
 * 4)                    t^            => result = 2
 * 5)                            t^    => result = 3
 * @endcode
 *
 * @param[in] ti Temporal instant set value
 * @param[in] t Timestamp
 * @param[out] loc Location
 * @result Return true if the timestamp is contained in the temporal value
 */
bool
tinstantset_find_timestamp(const TInstantSet *ti, TimestampTz t, int *loc)
{
  int first = 0;
  int last = ti->count - 1;
  int middle = 0; /* make compiler quiet */
  const TInstant *inst = NULL; /* make compiler quiet */
  while (first <= last)
  {
    middle = (first + last)/2;
    inst = tinstantset_inst_n(ti, middle);
    int cmp = timestamp_cmp_internal(inst->t, t);
    if (cmp == 0)
    {
      *loc = middle;
      return true;
    }
    if (cmp > 0)
      last = middle - 1;
    else
      first = middle + 1;
  }
  if (t > inst->t)
    middle++;
  *loc = middle;
  return false;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the string representation of the temporal value.
 *
 * @param[in] ti Temporal value
 * @param[in] value_out Function called to output the base value depending on
 * its Oid
 */
char *
tinstantset_to_string(const TInstantSet *ti, char *(*value_out)(Oid, Datum))
{
  char **strings = palloc(sizeof(char *) * ti->count);
  size_t outlen = 0;

  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    strings[i] = tinstant_to_string(inst, value_out);
    outlen += strlen(strings[i]) + 2;
  }
  return stringarr_to_string(strings, ti->count, outlen, "", '{', '}');
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Write the binary representation of the temporal value
 * into the buffer.
 *
 * @param[in] ti Temporal value
 * @param[in] buf Buffer
 */
void
tinstantset_write(const TInstantSet *ti, StringInfo buf)
{
  pq_sendint32(buf, ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    tinstant_write(inst, buf);
  }
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a new temporal value from its binary representation
 * read from the buffer.
 *
 * @param[in] buf Buffer
 * @param[in] temptype Temporal type
 */
TInstantSet *
tinstantset_read(StringInfo buf, CachedType temptype)
{
  int count = (int) pq_getmsgint(buf, 4);
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
    instants[i] = tinstant_read(buf, temptype);
  return tinstantset_make_free(instants, count, MERGE_NO);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal instant set value from a base value and a
 * timestamp set.
 */
TInstantSet *
tinstantset_from_base(Datum value, CachedType temptype, const TimestampSet *ts)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ts->count);
  for (int i = 0; i < ts->count; i++)
    instants[i] = tinstant_make(value, timestampset_time_n(ts, i), temptype);
  return tinstantset_make_free(instants, ts->count, MERGE_NO);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * Return the array of base values of the temporal value
 *
 * @param[in] ti Temporal value
 * @param[out] result Array of base values
 * @result Number of elements in the output array
 */
int
tinstantset_values1(const TInstantSet *ti, Datum *result)
{
  for (int i = 0; i < ti->count; i++)
    result[i] = tinstant_value(tinstantset_inst_n(ti, i));
  if (ti->count == 1)
    return 1;
  CachedType basetype = temptype_basetype(ti->temptype);
  datumarr_sort(result, ti->count, basetype);
  return datumarr_remove_duplicates(result, ti->count, basetype);
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of base values of the temporal value.
 */
Datum *
tinstantset_values(const TInstantSet *ti, int *count)
{
  Datum *result = palloc(sizeof(Datum *) * ti->count);
  *count = tinstantset_values1(ti, result);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of ranges of the temporal float value.
 */
RangeType **
tfloatinstset_ranges(const TInstantSet *ti, int *count)
{
  int newcount;
  Datum *values = tinstantset_values(ti, &newcount);
  RangeType **result = palloc(sizeof(RangeType *) * newcount);
  for (int i = 0; i < newcount; i++)
    result[i] = range_make(values[i], values[i], true, true, T_FLOAT8);
  pfree(values);
  *count = newcount;
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the time on which the temporal value is defined as a
 * period set.
 */
PeriodSet *
tinstantset_time(const TInstantSet *ti)
{
  Period **periods = palloc(sizeof(Period *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    periods[i] = period_make(inst->t, inst->t, true, true);
  }
  PeriodSet *result = periodset_make_free(periods, ti->count, NORMALIZE_NO);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the minimum base value of the temporal value.
 */
Datum
tinstantset_min_value(const TInstantSet *ti)
{
  if (ti->temptype == T_TINT)
  {
    TBOX *box = tinstantset_bbox_ptr(ti);
    return Int32GetDatum((int)(box->xmin));
  }
  else if (ti->temptype == T_TFLOAT)
  {
    TBOX *box = tinstantset_bbox_ptr(ti);
    return Float8GetDatum(box->xmin);
  }
  else
  {
    CachedType basetype = temptype_basetype(ti->temptype);
    Datum min = tinstant_value(tinstantset_inst_n(ti, 0));
    int idx = 0;
    for (int i = 1; i < ti->count; i++)
    {
      Datum value = tinstant_value(tinstantset_inst_n(ti, i));
      if (datum_lt(value, min, basetype))
      {
        min = value;
        idx = i;
      }
    }
    return tinstant_value(tinstantset_inst_n(ti, idx));
  }
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the maximum base value of the temporal value.
 */
Datum
tinstantset_max_value(const TInstantSet *ti)
{
  if (ti->temptype == T_TINT)
  {
    TBOX *box = tinstantset_bbox_ptr(ti);
    return Int32GetDatum((int)(box->xmax));
  }
  else if (ti->temptype == T_TFLOAT)
  {
    TBOX *box = tinstantset_bbox_ptr(ti);
    return Float8GetDatum(box->xmax);
  }
  else
  {
    CachedType basetype = temptype_basetype(ti->temptype);
    Datum max = tinstant_value(tinstantset_inst_n(ti, 0));
    int idx = 0;
    for (int i = 1; i < ti->count; i++)
    {
      Datum value = tinstant_value(tinstantset_inst_n(ti, i));
      if (datum_gt(value, max, basetype))
      {
        max = value;
        idx = i;
      }
    }
    return tinstant_value(tinstantset_inst_n(ti, idx));
  }
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the bounding period on which the temporal value is defined.
 */
void
tinstantset_period(const TInstantSet *ti, Period *p)
{
  TimestampTz lower = tinstantset_start_timestamp(ti);
  TimestampTz upper = tinstantset_end_timestamp(ti);
  return period_set(lower, upper, true, true, p);
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the timespan of the timestamp set value.
 */
Interval *
tinstantset_timespan(const TInstantSet *ti)
{
  TimestampTz lower = tinstantset_start_timestamp(ti);
  TimestampTz upper = tinstantset_end_timestamp(ti);
  Interval *result = (Interval *) DatumGetPointer(call_function2(timestamp_mi,
    TimestampTzGetDatum(upper), TimestampTzGetDatum(lower)));
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of sequences of the temporal value.
 */
TSequence **
tinstantset_sequences(const TInstantSet *ti)
{
  TSequence **result = palloc(sizeof(TSequence *) * ti->count);
  bool linear = MOBDB_FLAGS_GET_CONTINUOUS(ti->flags);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    result[i] = tinstant_tsequence(inst, linear);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of instants of the temporal value.
 */
const TInstant **
tinstantset_instants(const TInstantSet *ti)
{
  const TInstant **result = palloc(sizeof(TInstant *) * ti->count);
  for (int i = 0; i < ti->count; i++)
    result[i] = tinstantset_inst_n(ti, i);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start timestamp of the temporal value.
 */
TimestampTz
tinstantset_start_timestamp(const TInstantSet *ti)
{
  return (tinstantset_inst_n(ti, 0))->t;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end timestamp of the temporal value.
 */
TimestampTz
tinstantset_end_timestamp(const TInstantSet *ti)
{
  return (tinstantset_inst_n(ti, ti->count - 1))->t;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the distinct timestamps of the temporal value.
 */
TimestampTz *
tinstantset_timestamps(const TInstantSet *ti)
{
  TimestampTz *result = palloc(sizeof(TimestampTz) * ti->count);
  for (int i = 0; i < ti->count; i++)
    result[i] = (tinstantset_inst_n(ti, i))->t;
  return result;
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast the temporal integer value as a temporal float value.
 */
TInstantSet *
tintinstset_tfloatinstset(const TInstantSet *ti)
{
  TInstantSet *result = tinstantset_copy(ti);
  result->temptype = T_TFLOAT;
  for (int i = 0; i < ti->count; i++)
  {
    TInstant *inst = (TInstant *) tinstantset_inst_n(result, i);
    inst->temptype = T_TFLOAT;
    Datum *value_ptr = tinstant_value_ptr(inst);
    *value_ptr = Float8GetDatum((double)DatumGetInt32(tinstant_value(inst)));
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast the temporal float value as a temporal integer value.
 */
TInstantSet *
tfloatinstset_tintinstset(const TInstantSet *ti)
{
  TInstantSet *result = tinstantset_copy(ti);
  result->temptype = T_TINT;
  for (int i = 0; i < ti->count; i++)
  {
    TInstant *inst = (TInstant *) tinstantset_inst_n(result, i);
    inst->temptype = T_TINT;
    Datum *value_ptr = tinstant_value_ptr(inst);
    *value_ptr = Int32GetDatum((double)DatumGetFloat8(tinstant_value(inst)));
  }
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal instant value into a temporal instant set value.
 */
TInstantSet *
tinstant_tinstantset(const TInstant *inst)
{
  return tinstantset_make(&inst, 1, MERGE_NO);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transforms the temporal sequence value into a temporal instant value.
 *
 * @return Return an error if the temporal sequence has more than one instant
 */
TInstantSet *
tsequence_tinstantset(const TSequence *seq)
{
  if (seq->count != 1)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Cannot transform input to a temporal instant set")));

  const TInstant *inst = tsequence_inst_n(seq, 0);
  return tinstant_tinstantset(inst);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal sequence set value into a temporal instant
 * set value.
 *
 * @return Return an error if any of the composing temporal sequences has
 * more than one instant
*/
TInstantSet *
tsequenceset_tinstantset(const TSequenceSet *ts)
{
  const TSequence *seq;
  for (int i = 0; i < ts->count; i++)
  {
    seq = tsequenceset_seq_n(ts, i);
    if (seq->count != 1)
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Cannot transform input to a temporal instant set")));
  }

  const TInstant **instants = palloc(sizeof(TInstant *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    seq = tsequenceset_seq_n(ts, i);
    instants[i] = tsequence_inst_n(seq, 0);
  }
  TInstantSet *result = tinstantset_make(instants, ts->count, MERGE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Shift and/or scale the time span of the temporal value by the two
 * intervals
 *
 * @pre The duration is greater than 0 if it is not NULL
 */
TInstantSet *
tinstantset_shift_tscale(const TInstantSet *ti, const Interval *start,
  const Interval *duration)
{
  assert(start != NULL || duration != NULL);

  /* Copy the input instant set to the result */
  TInstantSet *result = tinstantset_copy(ti);

  /* Shift and/or scale the period */
  Period p1, p2;
  const TInstant *inst1 = tinstantset_inst_n(ti, 0);
  const TInstant *inst2 = tinstantset_inst_n(ti, ti->count - 1);
  period_set(inst1->t, inst2->t, true, true, &p1);
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

  /* Set the first instant */
  TInstant *inst = (TInstant *) tinstantset_inst_n(result, 0);
  inst->t = p2.lower;
  if (ti->count > 1)
  {
    /* Shift and/or scale from the second to the penultimate instant */
    for (int i = 1; i < ti->count - 1; i++)
    {
      inst = (TInstant *) tinstantset_inst_n(result, i);
      if (start != NULL && (duration == NULL || instant))
        inst->t += shift;
      if (duration != NULL && ! instant)
        inst->t = p2.lower + (inst->t - p1.lower) * scale;
    }
    /* Set the last instant */
    inst = (TInstant *) tinstantset_inst_n(result, ti->count - 1);
    inst->t = p2.upper;
  }
  /* Shift and/or scale bounding box */
  void *bbox = tinstantset_bbox_ptr(result);
  temporal_bbox_shift_tscale(bbox, start, duration, ti->temptype);
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
tinstantset_ever_eq(const TInstantSet *ti, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_eq((Temporal *) ti, value, EVER))
    return false;

  CachedType basetype = temptype_basetype(ti->temptype);
  for (int i = 0; i < ti->count; i++)
  {
    Datum valueinst = tinstant_value(tinstantset_inst_n(ti, i));
    if (datum_eq(valueinst, value, basetype))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is always equal to the base value.
 */
bool
tinstantset_always_eq(const TInstantSet *ti, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_eq((Temporal *) ti, value, ALWAYS))
    return false;

  /* The bounding box test above is enough to compute the answer for
   * temporal numbers */
  if (tnumber_type(ti->temptype))
    return true;

  CachedType basetype = temptype_basetype(ti->temptype);
  for (int i = 0; i < ti->count; i++)
  {
    Datum valueinst = tinstant_value(tinstantset_inst_n(ti, i));
    if (datum_ne(valueinst, value, basetype))
      return false;
  }
  return true;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is ever less than the base value.
 */
bool
tinstantset_ever_lt(const TInstantSet *ti, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *) ti, value, EVER))
    return false;

  CachedType basetype = temptype_basetype(ti->temptype);
  for (int i = 0; i < ti->count; i++)
  {
    Datum valueinst = tinstant_value(tinstantset_inst_n(ti, i));
    if (datum_lt(valueinst, value, basetype))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is ever less than or equal to the
 * base value
 */
bool
tinstantset_ever_le(const TInstantSet *ti, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *) ti, value, EVER))
    return false;

  CachedType basetype = temptype_basetype(ti->temptype);
  for (int i = 0; i < ti->count; i++)
  {
    Datum valueinst = tinstant_value(tinstantset_inst_n(ti, i));
    if (datum_le(valueinst, value, basetype))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is always less than the base value.
 */
bool
tinstantset_always_lt(const TInstantSet *ti, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *) ti, value, ALWAYS))
    return false;

  CachedType basetype = temptype_basetype(ti->temptype);
  for (int i = 0; i < ti->count; i++)
  {
    Datum valueinst = tinstant_value(tinstantset_inst_n(ti, i));
    if (! datum_lt(valueinst, value, basetype))
      return false;
  }
  return true;
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is always less than or equal to the
 * base value
 */
bool
tinstantset_always_le(const TInstantSet *ti, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *) ti, value, ALWAYS))
    return false;

  /* The bounding box test above is enough to compute
   * the answer for temporal numbers */
  if (tnumber_type(ti->temptype))
    return true;

  CachedType basetype = temptype_basetype(ti->temptype);
  for (int i = 0; i < ti->count; i++)
  {
    Datum valueinst = tinstant_value(tinstantset_inst_n(ti, i));
    if (! datum_le(valueinst, value, basetype))
      return false;
  }
  return true;
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the (complement of the) base value.
 *
 * @param[in] ti Temporal value
 * @param[in] value Base values
 * @param[in] atfunc True when the restriction is at, false for minus
 * @note There is no bounding box test in this function, it is done in the
 * dispatch function for all temporal types.
 */
TInstantSet *
tinstantset_restrict_value(const TInstantSet *ti, Datum value, bool atfunc)
{
  CachedType basetype = temptype_basetype(ti->temptype);

  /* Singleton instant set */
  if (ti->count == 1)
  {
    Datum value1 = tinstant_value(tinstantset_inst_n(ti, 0));
    bool equal = datum_eq(value, value1, basetype);
    if ((atfunc && ! equal) || (! atfunc && equal))
      return NULL;
    return tinstantset_copy(ti);
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int count = 0;
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    bool equal = datum_eq(value, tinstant_value(inst), basetype);
    if ((atfunc && equal) || (! atfunc && ! equal))
      instants[count++] = inst;
  }
  TInstantSet *result = (count == 0) ? NULL :
    tinstantset_make(instants, count, MERGE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the (complement of the) array of base
 * values.
 *
 * @param[in] ti Temporal value
 * @param[in] values Array of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True when the restriction is at, false for minus
 * @pre There are no duplicates values in the array
 */
TInstantSet *
tinstantset_restrict_values(const TInstantSet *ti, const Datum *values,
  int count, bool atfunc)
{
  const TInstant *inst;

  /* Singleton instant set */
  if (ti->count == 1)
  {
    inst = tinstantset_inst_n(ti, 0);
    if (tinstant_restrict_values_test(inst, values, count, atfunc))
      return tinstantset_copy(ti);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int newcount = 0;
  for (int i = 0; i < ti->count; i++)
  {
    inst = tinstantset_inst_n(ti, i);
    if (tinstant_restrict_values_test(inst, values, count, atfunc))
      instants[newcount++] = inst;
  }
  TInstantSet *result = (newcount == 0) ? NULL :
    tinstantset_make(instants, newcount, MERGE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal number to the (complement of the) range of base
 * values.
 *
 * @param[in] ti Temporal number
 * @param[in] range Range of base values
 * @param[in] atfunc True when the restriction is at, false for minus
 * @return Resulting temporal number
 * @note A bounding box test has been done in the dispatch function.
 */
TInstantSet *
tnumberinstset_restrict_range(const TInstantSet *ti, const RangeType *range,
  bool atfunc)
{
  /* Singleton instant set */
  if (ti->count == 1)
    return atfunc ? tinstantset_copy(ti) : NULL;

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int count = 0;
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    if (tnumberinst_restrict_range_test(inst, range, atfunc))
      instants[count++] = inst;
  }
  TInstantSet *result = (count == 0) ? NULL :
    tinstantset_make(instants, count, MERGE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the (complement of the) array of ranges
 * of base values.
 *
 * @param[in] ti Temporal number
 * @param[in] normranges Array of ranges of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True when the restriction is at, false for minus
 * @return Resulting temporal number
 * @pre The array of ranges is normalized
 * @note A bounding box test has been done in the dispatch function.
 */
TInstantSet *
tnumberinstset_restrict_ranges(const TInstantSet *ti, RangeType **normranges,
  int count, bool atfunc)
{
  const TInstant *inst;

  /* Singleton instant set */
  if (ti->count == 1)
  {
    inst = tinstantset_inst_n(ti, 0);
    if (tnumberinst_restrict_ranges_test(inst, normranges, count, atfunc))
      return tinstantset_copy(ti);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int newcount = 0;
  for (int i = 0; i < ti->count; i++)
  {
    inst = tinstantset_inst_n(ti, i);
    if (tnumberinst_restrict_ranges_test(inst, normranges, count, atfunc))
      instants[newcount++] = inst;
  }
  TInstantSet *result = (newcount == 0) ? NULL :
    tinstantset_make(instants, newcount, MERGE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return a pointer to the instant with minimum base value of the
 * temporal value
 *
 * @note Function used, e.g., for computing the shortest line between two
 * temporal points from their temporal distance
 */
const TInstant *
tinstantset_min_instant(const TInstantSet *ti)
{
  Datum min = tinstant_value(tinstantset_inst_n(ti, 0));
  int k = 0;
  CachedType basetype = temptype_basetype(ti->temptype);
  for (int i = 1; i < ti->count; i++)
  {
    Datum value = tinstant_value(tinstantset_inst_n(ti, i));
    if (datum_lt(value, min, basetype))
    {
      min = value;
      k = i;
    }
  }
  return tinstantset_inst_n(ti, k);
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return a pointer to the instant with minimum base value of the
 * temporal value
 */
const TInstant *
tinstantset_max_instant(const TInstantSet *ti)
{
  Datum max = tinstant_value(tinstantset_inst_n(ti, 0));
  int k = 0;
  CachedType basetype = temptype_basetype(ti->temptype);
  for (int i = 1; i < ti->count; i++)
  {
    Datum value = tinstant_value(tinstantset_inst_n(ti, i));
    if (datum_gt(value, max, basetype))
    {
      max = value;
      k = i;
    }
  }
  return tinstantset_inst_n(ti, k);
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to (the complement of) the minimum/maximum
 * base value
 */
TInstantSet *
tinstantset_restrict_minmax(const TInstantSet *ti, bool min, bool atfunc)
{
  Datum minmax = min ? tinstantset_min_value(ti) : tinstantset_max_value(ti);
  return tinstantset_restrict_value(ti, minmax, atfunc);
}

/**
 * Return the base value of the temporal value at the timestamp
 *
 * @note In order to be compatible with the corresponding functions for temporal
 * sequences that need to interpolate the value, it is necessary to return
 * a copy of the value
 */
bool
tinstantset_value_at_timestamp(const TInstantSet *ti, TimestampTz t, Datum *result)
{
  int loc;
  if (! tinstantset_find_timestamp(ti, t, &loc))
    return false;

  const TInstant *inst = tinstantset_inst_n(ti, loc);
  *result = tinstant_value_copy(inst);
  return true;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to (the complement of) the timestamp.
 *
 * @note In order to be compatible with the corresponding functions for temporal
 * sequences that need to interpolate the value, it is necessary to return
 * a copy of the value
 */
Temporal *
tinstantset_restrict_timestamp(const TInstantSet *ti, TimestampTz t, bool atfunc)
{
  /* Bounding box test */
  Period p;
  tinstantset_period(ti, &p);
  if (!contains_period_timestamp(&p, t))
    return atfunc ? NULL : (Temporal *) tinstantset_copy(ti);

  /* Singleton instant set */
  if (ti->count == 1)
    return atfunc ? (Temporal *) tinstant_copy(tinstantset_inst_n(ti, 0)) : NULL;

  /* General case */
  const TInstant *inst;
  if (atfunc)
  {
    int loc;
    if (! tinstantset_find_timestamp(ti, t, &loc))
      return NULL;
    inst = tinstantset_inst_n(ti, loc);
    return (Temporal *) tinstant_copy(inst);
  }
  else
  {
    const TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
    int count = 0;
    for (int i = 0; i < ti->count; i++)
    {
      inst= tinstantset_inst_n(ti, i);
      if (inst->t != t)
        instants[count++] = inst;
    }
    TInstantSet *result = (count == 0) ? NULL :
      tinstantset_make(instants, count, MERGE_NO);
    pfree(instants);
    return (Temporal *) result;
  }
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the (complement of the) timestamp set.
 */
TInstantSet *
tinstantset_restrict_timestampset(const TInstantSet *ti, const TimestampSet *ts,
  bool atfunc)
{
  TInstantSet *result;
  const TInstant *inst;

  /* Singleton timestamp set */
  if (ts->count == 1)
  {
    Temporal *temp = tinstantset_restrict_timestamp(ti,
      timestampset_time_n(ts, 0), atfunc);
    if (temp == NULL || temp->subtype == INSTANTSET)
      return (TInstantSet *) temp;
    TInstant *inst1 = (TInstant *) temp;
    result = tinstantset_make((const TInstant **) &inst1, 1, MERGE_NO);
    pfree(inst1);
    return result;
  }

  /* Bounding box test */
  Period p1;
  tinstantset_period(ti, &p1);
  const Period *p2 = timestampset_bbox_ptr(ts);
  if (!overlaps_period_period(&p1, p2))
    return atfunc ? NULL : tinstantset_copy(ti);


  /* Singleton instant set */
  if (ti->count == 1)
  {
    inst = tinstantset_inst_n(ti, 0);
    if (tinstant_restrict_timestampset_test(inst, ts, atfunc))
      return tinstantset_copy(ti);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int i = 0, j = 0, k = 0;
  while (i < ti->count && j < ts->count)
  {
    inst = tinstantset_inst_n(ti, i);
    TimestampTz t = timestampset_time_n(ts, j);
    int cmp = timestamp_cmp_internal(inst->t, t);
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
    while (i < ti->count)
      instants[k++] = tinstantset_inst_n(ti, i++);
  }
  result = (k == 0) ? NULL : tinstantset_make(instants, k, MERGE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to (the complement of) the period.
 */
TInstantSet *
tinstantset_restrict_period(const TInstantSet *ti, const Period *period,
  bool atfunc)
{
  /* Bounding box test */
  Period p;
  tinstantset_period(ti, &p);
  if (!overlaps_period_period(&p, period))
    return atfunc ? NULL : tinstantset_copy(ti);

  /* Singleton instant set */
  if (ti->count == 1)
    return atfunc ? tinstantset_copy(ti) : NULL;

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int count = 0;
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    bool contains = contains_period_timestamp(period, inst->t);
    if ((atfunc && contains) || (! atfunc && ! contains))
      instants[count++] = inst;
  }
  TInstantSet *result = (count == 0) ? NULL :
    tinstantset_make(instants, count, MERGE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to (the complement of) the period set.
 */
TInstantSet *
tinstantset_restrict_periodset(const TInstantSet *ti, const PeriodSet *ps,
  bool atfunc)
{
  const TInstant *inst;

  /* Singleton period set */
  if (ps->count == 1)
    return tinstantset_restrict_period(ti, periodset_per_n(ps, 0), atfunc);

  /* Bounding box test */
  Period p1;
  tinstantset_period(ti, &p1);
  const Period *p2 = periodset_bbox_ptr(ps);
  if (!overlaps_period_period(&p1, p2))
    return atfunc ? NULL : tinstantset_copy(ti);

  /* Singleton instant set */
  if (ti->count == 1)
  {
    inst = tinstantset_inst_n(ti, 0);
    if (tinstant_restrict_periodset_test(inst, ps, atfunc))
      return tinstantset_copy(ti);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int count = 0;
  for (int i = 0; i < ti->count; i++)
  {
    inst = tinstantset_inst_n(ti, i);
    bool contains = contains_periodset_timestamp(ps, inst->t);
    if ((atfunc && contains) || (! atfunc && ! contains))
      instants[count++] = inst;
  }
  TInstantSet *result = (count == 0) ? NULL :
    tinstantset_make(instants, count, MERGE_NO);
  pfree(instants);
  return result;
}

/*****************************************************************************
 * Append and merge functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_transf
 * @brief Append an instant to the temporal value.
 */
TInstantSet *
tinstantset_append_tinstant(const TInstantSet *ti, const TInstant *inst)
{
  /* Ensure validity of the arguments */
  assert(ti->temptype == inst->temptype);
  const TInstant *inst1 = tinstantset_inst_n(ti, ti->count - 1);
  ensure_increasing_timestamps(inst1, inst, MERGE);
  if (inst1->t == inst->t)
    return tinstantset_copy(ti);

  /* Create the result */
  const TInstant **instants = palloc(sizeof(TInstant *) * ti->count + 1);
  for (int i = 0; i < ti->count; i++)
    instants[i] = tinstantset_inst_n(ti, i);
  instants[ti->count] = (TInstant *) inst;
  TInstantSet *result = tinstantset_make1(instants, ti->count + 1);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Merge the two temporal values.
 */
Temporal *
tinstantset_merge(const TInstantSet *ti1, const TInstantSet *ti2)
{
  const TInstantSet *instsets[] = {ti1, ti2};
  return tinstantset_merge_array(instsets, 2);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Merge the array of temporal instant values.
 *
 * @note The function does not assume that the values in the array are strictly
 * ordered on time, i.e., the intersection of the bounding boxes of two values
 * may be a period. For this reason two passes are necessary.
 *
 * @param[in] instsets Array of values
 * @param[in] count Number of elements in the array
 * @result Merged value that can be either a temporal instant or a
 * temporal instant set
 */
Temporal *
tinstantset_merge_array(const TInstantSet **instsets, int count)
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
      instants[k++] = tinstantset_inst_n(instsets[i], j);
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
 * Temporally intersect the two temporal values
 *
 * @param[in] ti,inst Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tinstantset_tinstant(const TInstantSet *ti, const TInstant *inst,
  TInstant **inter1, TInstant **inter2)
{
  TInstant *inst1 = (TInstant *) tinstantset_restrict_timestamp(ti, inst->t, REST_AT);
  if (inst1 == NULL)
    return false;

  *inter1 = inst1;
  *inter2 = tinstant_copy(inst);
  return true;
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] inst,ti Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tinstant_tinstantset(const TInstant *inst, const TInstantSet *ti,
  TInstant **inter1, TInstant **inter2)
{
  return intersection_tinstantset_tinstant(ti, inst, inter2, inter1);
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] ti1,ti2 Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tinstantset_tinstantset(const TInstantSet *ti1, const TInstantSet *ti2,
  TInstantSet **inter1, TInstantSet **inter2)
{
  /* Test whether the bounding period of the two temporal values overlap */
  Period p1, p2;
  tinstantset_period(ti1, &p1);
  tinstantset_period(ti2, &p2);
  if (!overlaps_period_period(&p1, &p2))
    return false;

  int count = Min(ti1->count, ti2->count);
  const TInstant **instants1 = palloc(sizeof(TInstant *) * count);
  const TInstant **instants2 = palloc(sizeof(TInstant *) * count);
  int i = 0, j = 0, k = 0;
  const TInstant *inst1 = tinstantset_inst_n(ti1, i);
  const TInstant *inst2 = tinstantset_inst_n(ti2, j);
  while (i < ti1->count && j < ti2->count)
  {
    int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      instants1[k] = inst1;
      instants2[k++] = inst2;
      inst1 = tinstantset_inst_n(ti1, ++i);
      inst2 = tinstantset_inst_n(ti2, ++j);
    }
    else if (cmp < 0)
      inst1 = tinstantset_inst_n(ti1, ++i);
    else
      inst2 = tinstantset_inst_n(ti2, ++j);
  }
  if (k != 0)
  {
    *inter1 = tinstantset_make(instants1, k, MERGE_NO);
    *inter2 = tinstantset_make(instants2, k, MERGE_NO);
  }

  pfree(instants1); pfree(instants2);
  return k != 0;
}

/*****************************************************************************
 * Intersects functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersects the timestamp.
 */
bool
tinstantset_intersects_timestamp(const TInstantSet *ti, TimestampTz t)
{
  int loc;
  return tinstantset_find_timestamp(ti, t, &loc);
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersects the timestamp set.
 */
bool
tinstantset_intersects_timestampset(const TInstantSet *ti,
  const TimestampSet *ts)
{
  for (int i = 0; i < ts->count; i++)
    if (tinstantset_intersects_timestamp(ti, timestampset_time_n(ts, i)))
      return true;
  return false;
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersects the period.
 */
bool
tinstantset_intersects_period(const TInstantSet *ti, const Period *period)
{
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    if (contains_period_timestamp(period, inst->t))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersects the period set.
 */
bool
tinstantset_intersects_periodset(const TInstantSet *ti, const PeriodSet *ps)
{
  for (int i = 0; i < ps->count; i++)
    if (tinstantset_intersects_period(ti, periodset_per_n(ps, i)))
      return true;
  return false;
}

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_agg
 * @brief Return the time-weighted average of the temporal number
 */
double
tnumberinstset_twavg(const TInstantSet *ti)
{
  CachedType basetype = temptype_basetype(ti->temptype);
  double result = 0.0;
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    result += datum_double(tinstant_value(inst), basetype);
  }
  return result / ti->count;
}

/*****************************************************************************
 * Functions for defining B-tree indexes
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the two temporal instant set values are equal.
 *
 * @pre The arguments are of the same base type
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
tinstantset_eq(const TInstantSet *ti1, const TInstantSet *ti2)
{
  assert(ti1->temptype == ti2->temptype);
  /* If number of sequences or flags are not equal */
  if (ti1->count != ti2->count || ti1->flags != ti2->flags)
    return false;

  /* If bounding boxes are not equal */
  void *box1 = tinstantset_bbox_ptr(ti1);
  void *box2 = tinstantset_bbox_ptr(ti2);
  if (! temporal_bbox_eq(box1, box2, ti1->temptype))
    return false;

  /* Compare the composing instants */
  for (int i = 0; i < ti1->count; i++)
  {
    const TInstant *inst1 = tinstantset_inst_n(ti1, i);
    const TInstant *inst2 = tinstantset_inst_n(ti2, i);
    if (! tinstant_eq(inst1, inst2))
      return false;
  }
  return true;
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal value is
 * less than, equal, or greater than the second one.
 *
 * @pre The arguments are of the same base type
 * @note Period and bounding box comparison have been done by the calling
 * function temporal_cmp
 */
int
tinstantset_cmp(const TInstantSet *ti1, const TInstantSet *ti2)
{
  assert(ti1->temptype == ti2->temptype);

  /* Compare composing instants */
  int count = Min(ti1->count, ti2->count);
  for (int i = 0; i < count; i++)
  {
    const TInstant *inst1 = tinstantset_inst_n(ti1, i);
    const TInstant *inst2 = tinstantset_inst_n(ti2, i);
    int result = tinstant_cmp(inst1, inst2);
    if (result)
      return result;
  }

  /* ti1->count == ti2->count because of the bounding box and the
   * composing instant tests above */

  /* ti1->flags == ti2->flags since the equality of flags were
   * tested for each of the composing sequences */

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
 * @brief Return the hash value of the temporal value
 */
uint32
tinstantset_hash(const TInstantSet *ti)
{
  uint32 result = 1;
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    uint32 inst_hash = tinstant_hash(inst);
    result = (result << 5) - result + inst_hash;
  }
  return result;
}

/*****************************************************************************/
