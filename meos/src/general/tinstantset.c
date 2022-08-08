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
 * General functions
 *****************************************************************************/

/**
 * Return a pointer to the bounding box of a temporal instant set
 */
void *
tinstantset_bbox_ptr(const TInstantSet *is)
{
  return (void *)(&is->period);
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Set the second argument to the bounding box of a temporal instant set
 * @sqlfunc period(), tbox(), stbox()
 * @sqlop @p ::
 */
void
tinstantset_set_bbox(const TInstantSet *is, void *box)
{
  memset(box, 0, is->bboxsize);
  memcpy(box, tinstantset_bbox_ptr(is), is->bboxsize);
  return;
}

/**
 * Return a pointer to the offsets array of a temporal instant set
 */
static size_t *
tinstantset_offsets_ptr(const TInstantSet *is)
{
  return (size_t *)(((char *) is) + double_pad(sizeof(TInstantSet)) +
    /* The period component of the bbox is already declared in the struct */
    double_pad(is->bboxsize - sizeof(Period)));
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the n-th instant of a temporal instant set.
 * @pre The argument @p index is less than the number of instants in the
 * instant set
 */
const TInstant *
tinstantset_inst_n(const TInstantSet *is, int index)
{
  return (TInstant *)(
    /* start of data */
    ((char *) is) + double_pad(sizeof(TInstantSet)) +
      /* The period component of the bbox is already declared in the struct */
      (is->bboxsize - sizeof(Period)) + is->count * sizeof(size_t) +
      /* offset */
      (tinstantset_offsets_ptr(is))[index]);
}

/**
 * Ensure the validity of the arguments when creating a temporal instant set
 */
static void
tinstantset_make_valid(const TInstant **instants, int count, bool merge)
{
  /* Test the validity of the instants */
  assert(count > 0);
  ensure_tinstarr(instants, count);
  ensure_valid_tinstarr(instants, count, merge, TINSTANTSET);
  return;
}

/**
 * Create a temporal instant set from its arguments
 * @pre The validity of the arguments has been tested before
 *
 * For example, the memory structure of a temporal instant set with two
 * instants is as follows
 * @code
 *  -----------------------------------------------------------------
 *  ( TInstantSet )_X | (end of bbox )_X | offset_0 | offset_1 | ...
 *  -----------------------------------------------------------------
 *  -------------------------------------
 *  ( TInstant_0 )_X | ( TInstant_1 )_X |
 *  -------------------------------------
 * @endcode
 * where the `_X` are unused bytes added for double padding, `offset_0` and
 * `offset_1` are offsets for the corresponding instants
 */
TInstantSet *
tinstantset_make1(const TInstant **instants, int count)
{
  /* Get the bounding box size */
  size_t bboxsize = temporal_bbox_size(instants[0]->temptype);

  /* Compute the size of the temporal instant set */
  /* The period component of the bbox is already declared in the struct */
  size_t memsize = bboxsize - sizeof(Period);
  /* Size of composing instants */
  for (int i = 0; i < count; i++)
    memsize += double_pad(VARSIZE(instants[i]));
  /* Size of the struct and the offset array */
  memsize += double_pad(sizeof(TInstantSet)) + count * sizeof(size_t);
  /* Create the TInstantSet */
  TInstantSet *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  result->count = count;
  result->temptype = instants[0]->temptype;
  result->subtype = TINSTANTSET;
  result->bboxsize = bboxsize;
  bool continuous = MOBDB_FLAGS_GET_CONTINUOUS(instants[0]->flags);
  MOBDB_FLAGS_SET_CONTINUOUS(result->flags, continuous);
  MOBDB_FLAGS_SET_LINEAR(result->flags, continuous);
  MOBDB_FLAGS_SET_X(result->flags, true);
  MOBDB_FLAGS_SET_T(result->flags, true);
  if (tgeo_type(instants[0]->temptype))
  {
    MOBDB_FLAGS_SET_Z(result->flags, MOBDB_FLAGS_GET_Z(instants[0]->flags));
    MOBDB_FLAGS_SET_GEODETIC(result->flags,
      MOBDB_FLAGS_GET_GEODETIC(instants[0]->flags));
  }
  /* Initialization of the variable-length part */
  /*
   * Compute the bounding box
   * Only external types have bounding box, internal types such
   * as double2, double3, or double4 do not have bounding box
   */
  if (bboxsize != 0)
  {
    tinstantset_compute_bbox(instants, count, tinstantset_bbox_ptr(result));
  }
  /* Store the composing instants */
  size_t pdata = double_pad(sizeof(TInstantSet)) +
    double_pad(bboxsize - sizeof(Period)) + count * sizeof(size_t);
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
 * Return the location of the timestamp in a temporal instant set
 * value using binary search
 *
 * If the timestamp is contained in the temporal instant set, the index
 * of the sequence is returned in the output parameter. Otherwise,
 * returns a number encoding whether the timestamp is before, between
 * two sequences, or after the temporal instant set.
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
 * @param[in] is Temporal instant set
 * @param[in] t Timestamp
 * @param[out] loc Location
 * @result Return true if the timestamp is contained in the temporal instant set
 */
bool
tinstantset_find_timestamp(const TInstantSet *is, TimestampTz t, int *loc)
{
  int first = 0;
  int last = is->count - 1;
  int middle = 0; /* make compiler quiet */
  const TInstant *inst = NULL; /* make compiler quiet */
  while (first <= last)
  {
    middle = (first + last)/2;
    inst = tinstantset_inst_n(is, middle);
    int cmp = timestamptz_cmp_internal(inst->t, t);
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

#if MEOS
/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set from its Well-Known Text (WKT)
 * representation.
 *
 * @param[in] str String
 * @param[in] temptype Temporal type
 */
TInstantSet *
tinstantset_in(char *str, mobdbType temptype)
{
  return tinstantset_parse(&str, temptype);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set boolean from its Well-Known Text (WKT)
 * representation.
 */
TInstantSet *
tboolinstset_in(char *str)
{
  return tinstantset_parse(&str, T_TBOOL);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set integer from its Well-Known Text (WKT)
 * representation.
 */
TInstantSet *
tintinstset_in(char *str)
{
  return tinstantset_parse(&str, T_TINT);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set float from its Well-Known Text (WKT)
 * representation.
 */
TInstantSet *
tfloatinstset_in(char *str)
{
  return tinstantset_parse(&str, T_TFLOAT);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set text from its Well-Known Text (WKT)
 * representation.
 */
TInstantSet *
ttextinstset_in(char *str)
{
  return tinstantset_parse(&str, T_TTEXT);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set geometric point from its Well-Known Text
 * (WKT) representation.
 */
TInstantSet *
tgeompointinstset_in(char *str)
{
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tpoint_parse(&str, T_TGEOMPOINT);
  assert (temp->subtype == TINSTANT);
  return (TInstantSet *) temp;
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant set geographic point from its Well-Known
 * Text (WKT) representation.
 */
TInstantSet *
tgeogpointinstset_in(char *str)
{
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tpoint_parse(&str, T_TGEOGPOINT);
  assert (temp->subtype == TINSTANTSET);
  return (TInstantSet *) temp;
}
#endif

/**
 * @brief Return the Well-Known Text (WKT) representation of a temporal instant
 * set.
 *
 * @param[in] is Temporal instant set
 * @param[in] arg Maximum number of decimal digits to output for floating point
 * values
 * @param[in] value_out Function called to output the base value depending on
 * its type
 */
char *
tinstantset_to_string(const TInstantSet *is, Datum arg,
  char *(*value_out)(mobdbType, Datum, Datum))
{
  char **strings = palloc(sizeof(char *) * is->count);
  size_t outlen = 0;

  for (int i = 0; i < is->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(is, i);
    strings[i] = tinstant_to_string(inst, arg, value_out);
    outlen += strlen(strings[i]) + 2;
  }
  return stringarr_to_string(strings, is->count, outlen, "", '{', '}');
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return the Well-Known Text (WKT) representation of a temporal instant
 * set.
 */
char *
tinstantset_out(const TInstantSet *is, Datum arg)
{
  return tinstantset_to_string(is, arg, &basetype_output);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal instant set from an array of temporal instants.
 *
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] merge True when overlapping instants are allowed as required in
 * merge operations
 * @sqlfunc tbool_instset(), tint_instset(), tfloat_instset(), ttext_instset(),
 * etc.
 */
TInstantSet *
tinstantset_make(const TInstant **instants, int count, bool merge)
{
  tinstantset_make_valid(instants, count, merge);
  return tinstantset_make1(instants, count);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal instant set from an array of temporal instants
 * and free the array and the instants after the creation.
 *
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] merge True when overlapping instants are allowed as required in
 * merge operations
 * @see tinstantset_make
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
 * @ingroup libmeos_int_temporal_constructor
 * @brief Return a copy of a temporal instant set.
 */
TInstantSet *
tinstantset_copy(const TInstantSet *is)
{
  TInstantSet *result = palloc0(VARSIZE(is));
  memcpy(result, is, VARSIZE(is));
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_constructor
 * @brief Construct a temporal instant set from a base value and the time frame
 * of another temporal instant set.
 * @sqlfunc tbool_instset(), tint_instset(), tfloat_instset(), ttext_instset(),
 * etc.
 */
TInstantSet *
tinstantset_from_base(Datum value, mobdbType temptype, const TInstantSet *is)
{
  TInstant **instants = palloc(sizeof(TInstant *) * is->count);
  for (int i = 0; i < is->count; i++)
    instants[i] = tinstant_make(value, temptype,
      tinstantset_inst_n(is, i)->t);
  return tinstantset_make_free(instants, is->count, MERGE_NO);
}

#if MEOS
/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal boolean instant set from a boolean and a
 * timestamp set.
 */
TInstantSet *
tboolinstset_from_base(bool b, const TInstantSet *is)
{
  return tinstantset_from_base(BoolGetDatum(b), T_TBOOL, is);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal integer instant set from an integer and a
 * timestamp set.
 */
TInstantSet *
tintinstset_from_base(int i, const TInstantSet *is)
{
  return tinstantset_from_base(Int32GetDatum(i), T_TINT, is);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal float instant set from a float and a
 * timestamp set.
 */
TInstantSet *
tfloatinstset_from_base(bool b, const TInstantSet *is)
{
  return tinstantset_from_base(BoolGetDatum(b), T_TFLOAT, is);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal text instant set from a text and a timestamp set.
 */
TInstantSet *
ttextinstset_from_base(const text *txt, const TInstantSet *is)
{
  return tinstantset_from_base(PointerGetDatum(txt), T_TTEXT, is);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geometric point instant set from a point and a
 * timestamp set.
 */
TInstantSet *
tgeompointinstset_from_base(const GSERIALIZED *gs, const TInstantSet *is)
{
  return tinstantset_from_base(PointerGetDatum(gs), T_TGEOMPOINT, is);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geographic point instant set from a point and a
 * timestamp set.
 */
TInstantSet *
tgeogpointinstset_from_base(const GSERIALIZED *gs, const TInstantSet *is)
{
  return tinstantset_from_base(PointerGetDatum(gs), T_TGEOGPOINT, is);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_constructor
 * @brief Construct a temporal instant set from a base value and a timestamp set.
 * @sqlfunc tbool_instset(), tint_instset(), tfloat_instset(), ttext_instset(),
 * etc.
 */
TInstantSet *
tinstantset_from_base_time(Datum value, mobdbType temptype,
  const TimestampSet *ts)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ts->count);
  for (int i = 0; i < ts->count; i++)
    instants[i] = tinstant_make(value, temptype, timestampset_time_n(ts, i));
  return tinstantset_make_free(instants, ts->count, MERGE_NO);
}

#if MEOS
/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal boolean instant set from a boolean and a
 * timestamp set.
 */
TInstantSet *
tboolinstset_from_base_time(bool b, const TimestampSet *ts)
{
  return tinstantset_from_base_time(BoolGetDatum(b), T_TBOOL, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal integer instant set from an integer and a
 * timestamp set.
 */
TInstantSet *
tintinstset_from_base_time(int i, const TimestampSet *ts)
{
  return tinstantset_from_base_time(Int32GetDatum(i), T_TINT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal float instant set from a float and a
 * timestamp set.
 */
TInstantSet *
tfloatinstset_from_base_time(bool b, const TimestampSet *ts)
{
  return tinstantset_from_base_time(BoolGetDatum(b), T_TFLOAT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal text instant set from a text and a timestamp set.
 */
TInstantSet *
ttextinstset_from_base_time(const text *txt, const TimestampSet *ts)
{
  return tinstantset_from_base_time(PointerGetDatum(txt), T_TTEXT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geometric point instant set from a point and a
 * timestamp set.
 */
TInstantSet *
tgeompointinstset_from_base_time(const GSERIALIZED *gs, const TimestampSet *ts)
{
  return tinstantset_from_base_time(PointerGetDatum(gs), T_TGEOMPOINT, ts);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geographic point instant set from a point and a
 * timestamp set.
 */
TInstantSet *
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
 * @param[in] is Temporal instant set
 * @param[out] result Array of base values
 * @result Number of elements in the output array
 */
int
tinstantset_values1(const TInstantSet *is, Datum *result)
{
  for (int i = 0; i < is->count; i++)
    result[i] = tinstant_value(tinstantset_inst_n(is, i));
  if (is->count == 1)
    return 1;
  mobdbType basetype = temptype_basetype(is->temptype);
  datumarr_sort(result, is->count, basetype);
  return datumarr_remove_duplicates(result, is->count, basetype);
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the array of base values of a temporal instant set.
 * @sqlfunc getValues()
 */
Datum *
tinstantset_values(const TInstantSet *is, int *count)
{
  Datum *result = palloc(sizeof(Datum *) * is->count);
  *count = tinstantset_values1(is, result);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the array of spans of a temporal instant set float.
 * @sqlfunc getValues()
 */
Span **
tfloatinstset_spans(const TInstantSet *is, int *count)
{
  int newcount;
  Datum *values = tinstantset_values(is, &newcount);
  Span **result = palloc(sizeof(Span *) * newcount);
  for (int i = 0; i < newcount; i++)
    result[i] = span_make(values[i], values[i], true, true, T_FLOAT8);
  pfree(values);
  *count = newcount;
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the time frame of a temporal instant set as a period set.
 * @sqlfunc getTime()
 */
PeriodSet *
tinstantset_time(const TInstantSet *is)
{
  Period **periods = palloc(sizeof(Period *) * is->count);
  for (int i = 0; i < is->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(is, i);
    periods[i] = span_make(inst->t, inst->t, true, true, T_TIMESTAMPTZ);
  }
  PeriodSet *result = periodset_make_free(periods, is->count, NORMALIZE_NO);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the minimum base value of a temporal instant set.
 * @sqlfunc minValue()
 */
Datum
tinstantset_min_value(const TInstantSet *is)
{
  if (is->temptype == T_TINT || is->temptype == T_TFLOAT)
  {
    TBOX *box = tinstantset_bbox_ptr(is);
    return box->span.lower;
  }

  mobdbType basetype = temptype_basetype(is->temptype);
  Datum result = tinstant_value(tinstantset_inst_n(is, 0));
  for (int i = 1; i < is->count; i++)
  {
    Datum value = tinstant_value(tinstantset_inst_n(is, i));
    if (datum_lt(value, result, basetype))
      result = value;
  }
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the maximum base value of a temporal instant set.
 * @sqlfunc maxValue()
 */
Datum
tinstantset_max_value(const TInstantSet *is)
{
  if (is->temptype == T_TINT || is->temptype == T_TFLOAT)
  {
    TBOX *box = tinstantset_bbox_ptr(is);
    return box->span.upper;
  }

  mobdbType basetype = temptype_basetype(is->temptype);
  Datum result = tinstant_value(tinstantset_inst_n(is, 0));
  for (int i = 1; i < is->count; i++)
  {
    Datum value = tinstant_value(tinstantset_inst_n(is, i));
    if (datum_gt(value, result, basetype))
      result = value;
  }
  return result;
}

/**
 * @ingroup libmeos_int_temporal_cast
 * @brief Return the bounding period of a temporal instant set
 * @sqlfunc period()
 * @sqlop @p ::
 */
void
tinstantset_set_period(const TInstantSet *is, Period *p)
{
  TimestampTz lower = tinstantset_start_timestamp(is);
  TimestampTz upper = tinstantset_end_timestamp(is);
  return span_set(TimestampTzGetDatum(lower), TimestampTzGetDatum(upper),
    true, true, T_TIMESTAMPTZ, p);
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the timespan of a temporal instant set.
 * @sqlfunc timespan()
 */
Interval *
tinstantset_timespan(const TInstantSet *is)
{
  TimestampTz lower = tinstantset_start_timestamp(is);
  TimestampTz upper = tinstantset_end_timestamp(is);
  Interval *result = pg_timestamp_mi(upper, lower);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the array of sequences of a temporal instant set.
 * @post The output parameter @p count is equal to the number of instants of
 * the input temporal instant set
 * @sqlfunc sequences()
 */
TSequence **
tinstantset_sequences(const TInstantSet *is, int *count)
{
  TSequence **result = palloc(sizeof(TSequence *) * is->count);
  bool linear = MOBDB_FLAGS_GET_CONTINUOUS(is->flags);
  for (int i = 0; i < is->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(is, i);
    result[i] = tinstant_to_tsequence(inst, linear);
  }
  *count = is->count;
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the array of instants of a temporal instant set.
 * @post The output parameter @p count is equal to the number of instants of
 * the input temporal instant set
 * @sqlfunc instants()
 */
const TInstant **
tinstantset_instants(const TInstantSet *is, int *count)
{
  const TInstant **result = palloc(sizeof(TInstant *) * is->count);
  for (int i = 0; i < is->count; i++)
    result[i] = tinstantset_inst_n(is, i);
  *count = is->count;
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the start timestamp of a temporal instant set.
 * @sqlfunc startTimestamp()
 */
TimestampTz
tinstantset_start_timestamp(const TInstantSet *is)
{
  return (tinstantset_inst_n(is, 0))->t;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the end timestamp of a temporal instant set.
 * @sqlfunc endTimestamp()
 */
TimestampTz
tinstantset_end_timestamp(const TInstantSet *is)
{
  return (tinstantset_inst_n(is, is->count - 1))->t;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the distinct timestamps of a temporal instant set.
 * @post The output parameter @p count is equal to the number of instants of
 * the input temporal instant set
 * @sqlfunc timestamps()
 */
TimestampTz *
tinstantset_timestamps(const TInstantSet *is, int *count)
{
  TimestampTz *result = palloc(sizeof(TimestampTz) * is->count);
  for (int i = 0; i < is->count; i++)
    result[i] = (tinstantset_inst_n(is, i))->t;
  *count = is->count;
  return result;
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_cast
 * @brief Cast a temporal instant set integer to a temporal instant set float.
 * @sqlop @p ::
 */
TInstantSet *
tintinstset_to_tfloatinstset(const TInstantSet *is)
{
  TInstantSet *result = tinstantset_copy(is);
  result->temptype = T_TFLOAT;
  for (int i = 0; i < is->count; i++)
  {
    TInstant *inst = (TInstant *) tinstantset_inst_n(result, i);
    inst->temptype = T_TFLOAT;
    inst->value = Float8GetDatum((double)DatumGetInt32(tinstant_value(inst)));
  }
  return result;
}

/**
 * @ingroup libmeos_int_temporal_cast
 * @brief Cast a temporal instant set float to a temporal instant set integer.
 * @sqlop @p ::
 */
TInstantSet *
tfloatinstset_to_tintinstset(const TInstantSet *is)
{
  TInstantSet *result = tinstantset_copy(is);
  result->temptype = T_TINT;
  for (int i = 0; i < is->count; i++)
  {
    TInstant *inst = (TInstant *) tinstantset_inst_n(result, i);
    inst->temptype = T_TINT;
    inst->value = Int32GetDatum((double)DatumGetFloat8(tinstant_value(inst)));
  }
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Return a temporal instant transformed into a temporal instant set.
 * @sqlfunc tbool_instset(), tint_instset(), tfloat_instset(), ttext_instset(),
 * etc.
 */
TInstantSet *
tinstant_to_tinstantset(const TInstant *inst)
{
  return tinstantset_make(&inst, 1, MERGE_NO);
}

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Return a temporal sequence transformed into a temporal instant.
 * @return Return an error if a temporal sequence has more than one instant
 * @sqlfunc tbool_instset(), tint_instset(), tfloat_instset(), ttext_instset(),
 * etc.
 */
TInstantSet *
tsequence_to_tinstantset(const TSequence *seq)
{
  if (seq->count != 1)
    elog(ERROR, "Cannot transform input to a temporal instant set");

  const TInstant *inst = tsequence_inst_n(seq, 0);
  return tinstant_to_tinstantset(inst);
}

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Return a temporal sequence set transformed into a temporal instant set.
 * @return Return an error if any of the composing temporal sequences has
 * more than one instant
 * @sqlfunc tbool_instset(), tint_instset(), tfloat_instset(), ttext_instset(),
 * etc.
 */
TInstantSet *
tsequenceset_to_tinstantset(const TSequenceSet *ts)
{
  const TSequence *seq;
  for (int i = 0; i < ts->count; i++)
  {
    seq = tsequenceset_seq_n(ts, i);
    if (seq->count != 1)
      elog(ERROR, "Cannot transform input to a temporal instant set");
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
 * @ingroup libmeos_int_temporal_transf
 * @brief Return a temporal instant set shifted and/or scaled by the intervals
 * @pre The duration is greater than 0 if it is not NULL
 * @sqlfunc shift(), tscale(), shiftTscale().
 */
TInstantSet *
tinstantset_shift_tscale(const TInstantSet *is, const Interval *shift,
  const Interval *duration)
{
  assert(shift != NULL || duration != NULL);

  /* Copy the input instant set to the result */
  TInstantSet *result = tinstantset_copy(is);

  /* Shift and/or scale the period */
  Period p1, p2;
  const TInstant *inst1 = tinstantset_inst_n(is, 0);
  const TInstant *inst2 = tinstantset_inst_n(is, is->count - 1);
  span_set(TimestampTzGetDatum(inst1->t), TimestampTzGetDatum(inst2->t),
    true, true, T_TIMESTAMPTZ, &p1);
  span_set(p1.lower, p1.upper, p1.lower_inc, p1.upper_inc, T_TIMESTAMPTZ, &p2);
  period_shift_tscale(shift, duration, &p2);
  TimestampTz delta;
  if (shift != NULL)
    delta = p2.lower - p1.lower;
  double scale;
  bool instant = (p2.lower == p2.upper);
  /* If the sequence set is instantaneous we cannot scale */
  if (duration != NULL && ! instant)
    scale = (double) (p2.upper - p2.lower) / (double) (p1.upper - p1.lower);

  /* Set the first instant */
  TInstant *inst = (TInstant *) tinstantset_inst_n(result, 0);
  inst->t = p2.lower;
  if (is->count > 1)
  {
    /* Shift and/or scale from the second to the penultimate instant */
    for (int i = 1; i < is->count - 1; i++)
    {
      inst = (TInstant *) tinstantset_inst_n(result, i);
      if (shift != NULL && (duration == NULL || instant))
        inst->t += delta;
      if (duration != NULL && ! instant)
        inst->t = p2.lower + (inst->t - p1.lower) * scale;
    }
    /* Set the last instant */
    inst = (TInstant *) tinstantset_inst_n(result, is->count - 1);
    inst->t = p2.upper;
  }
  /* Shift and/or scale bounding box */
  void *bbox = tinstantset_bbox_ptr(result);
  temporal_bbox_shift_tscale(shift, duration, is->temptype, bbox);
  return result;
}

/*****************************************************************************
 * Ever/always functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal instant set is ever equal to a base value.
 * @sqlop @p ?=
 */
bool
tinstantset_ever_eq(const TInstantSet *is, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_eq((Temporal *) is, value, EVER))
    return false;

  mobdbType basetype = temptype_basetype(is->temptype);
  for (int i = 0; i < is->count; i++)
  {
    Datum valueinst = tinstant_value(tinstantset_inst_n(is, i));
    if (datum_eq(valueinst, value, basetype))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal instant set is always equal to a base value.
 * @sqlop @p %=
 */
bool
tinstantset_always_eq(const TInstantSet *is, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_eq((Temporal *) is, value, ALWAYS))
    return false;

  /* The bounding box test above is enough to compute the answer for temporal
   * numbers */
  if (tnumber_type(is->temptype))
    return true;

  mobdbType basetype = temptype_basetype(is->temptype);
  for (int i = 0; i < is->count; i++)
  {
    Datum valueinst = tinstant_value(tinstantset_inst_n(is, i));
    if (datum_ne(valueinst, value, basetype))
      return false;
  }
  return true;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal instant set is ever less than a base value.
 * @sqlop @p ?<
 */
bool
tinstantset_ever_lt(const TInstantSet *is, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *) is, value, EVER))
    return false;

  mobdbType basetype = temptype_basetype(is->temptype);
  for (int i = 0; i < is->count; i++)
  {
    Datum valueinst = tinstant_value(tinstantset_inst_n(is, i));
    if (datum_lt(valueinst, value, basetype))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal instant set is ever less than or equal to a
 * base value
 * @sqlop @p ?<=
 */
bool
tinstantset_ever_le(const TInstantSet *is, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *) is, value, EVER))
    return false;

  mobdbType basetype = temptype_basetype(is->temptype);
  for (int i = 0; i < is->count; i++)
  {
    Datum valueinst = tinstant_value(tinstantset_inst_n(is, i));
    if (datum_le(valueinst, value, basetype))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal instant set is always less than a base value.
 * @sqlop @p %<
 */
bool
tinstantset_always_lt(const TInstantSet *is, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *) is, value, ALWAYS))
    return false;

  mobdbType basetype = temptype_basetype(is->temptype);
  for (int i = 0; i < is->count; i++)
  {
    Datum valueinst = tinstant_value(tinstantset_inst_n(is, i));
    if (! datum_lt(valueinst, value, basetype))
      return false;
  }
  return true;
}

/**
 * @ingroup libmeos_int_temporal_ever
 * @brief Return true if a temporal instant set is always less than or equal to
 * a base value
 * @sqlop @p %<=
 */
bool
tinstantset_always_le(const TInstantSet *is, Datum value)
{
  /* Bounding box test */
  if (! temporal_bbox_ev_al_lt_le((Temporal *) is, value, ALWAYS))
    return false;

  /* The bounding box test above is enough to compute the answer for temporal
   * numbers */
  if (tnumber_type(is->temptype))
    return true;

  mobdbType basetype = temptype_basetype(is->temptype);
  for (int i = 0; i < is->count; i++)
  {
    Datum valueinst = tinstant_value(tinstantset_inst_n(is, i));
    if (! datum_le(valueinst, value, basetype))
      return false;
  }
  return true;
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set to (the complement of) a base value.
 *
 * @param[in] is Temporal instant set
 * @param[in] value Base values
 * @param[in] atfunc True when the restriction is at, false for minus
 * @note There is no bounding box test in this function, it is done in the
 * dispatch function for all temporal types.
 * @sqlfunc atValue(), minusValue()
 */
TInstantSet *
tinstantset_restrict_value(const TInstantSet *is, Datum value, bool atfunc)
{
  mobdbType basetype = temptype_basetype(is->temptype);

  /* Singleton instant set */
  if (is->count == 1)
  {
    Datum value1 = tinstant_value(tinstantset_inst_n(is, 0));
    bool equal = datum_eq(value, value1, basetype);
    if ((atfunc && ! equal) || (! atfunc && equal))
      return NULL;
    return tinstantset_copy(is);
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * is->count);
  int count = 0;
  for (int i = 0; i < is->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(is, i);
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
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set to (the complement of) an array of
 * base values.
 *
 * @param[in] is Temporal instant set
 * @param[in] values Array of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True when the restriction is at, false for minus
 * @pre There are no duplicates values in the array
 * @sqlfunc atValues(), minusValues()
 */
TInstantSet *
tinstantset_restrict_values(const TInstantSet *is, const Datum *values,
  int count, bool atfunc)
{
  const TInstant *inst;

  /* Singleton instant set */
  if (is->count == 1)
  {
    inst = tinstantset_inst_n(is, 0);
    if (tinstant_restrict_values_test(inst, values, count, atfunc))
      return tinstantset_copy(is);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * is->count);
  int newcount = 0;
  for (int i = 0; i < is->count; i++)
  {
    inst = tinstantset_inst_n(is, i);
    if (tinstant_restrict_values_test(inst, values, count, atfunc))
      instants[newcount++] = inst;
  }
  TInstantSet *result = (newcount == 0) ? NULL :
    tinstantset_make(instants, newcount, MERGE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set number to (the complement of) a
 * span of base values.
 *
 * @param[in] is Temporal number
 * @param[in] span Span of base values
 * @param[in] atfunc True when the restriction is at, false for minus
 * @return Resulting temporal number
 * @note A bounding box test has been done in the dispatch function.
 * @sqlfunc atSpan(), minusSpan()
 */
TInstantSet *
tnumberinstset_restrict_span(const TInstantSet *is, const Span *span,
  bool atfunc)
{
  /* Singleton instant set */
  if (is->count == 1)
    return atfunc ? tinstantset_copy(is) : NULL;

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * is->count);
  int count = 0;
  for (int i = 0; i < is->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(is, i);
    if (tnumberinst_restrict_span_test(inst, span, atfunc))
      instants[count++] = inst;
  }
  TInstantSet *result = (count == 0) ? NULL :
    tinstantset_make(instants, count, MERGE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set number to (the complement of) an
 * array of spans of base values.
 *
 * @param[in] is Temporal number
 * @param[in] normspans Array of spans of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True when the restriction is at, false for minus
 * @return Resulting temporal number
 * @pre The array of spans is normalized
 * @note A bounding box test has been done in the dispatch function.
 * @sqlfunc atSpans(), minusSpans()
 */
TInstantSet *
tnumberinstset_restrict_spans(const TInstantSet *is, Span **normspans,
  int count, bool atfunc)
{
  const TInstant *inst;

  /* Singleton instant set */
  if (is->count == 1)
  {
    inst = tinstantset_inst_n(is, 0);
    if (tnumberinst_restrict_spans_test(inst, normspans, count, atfunc))
      return tinstantset_copy(is);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * is->count);
  int newcount = 0;
  for (int i = 0; i < is->count; i++)
  {
    inst = tinstantset_inst_n(is, i);
    if (tnumberinst_restrict_spans_test(inst, normspans, count, atfunc))
      instants[newcount++] = inst;
  }
  TInstantSet *result = (newcount == 0) ? NULL :
    tinstantset_make(instants, newcount, MERGE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return a pointer to the instant with minimum base value of a
 * temporal instant set
 *
 * @note Function used, e.g., for computing the shortest line between two
 * temporal points from their temporal distance
 * @sqlfunc minInstant()
 */
const TInstant *
tinstantset_min_instant(const TInstantSet *is)
{
  Datum min = tinstant_value(tinstantset_inst_n(is, 0));
  int k = 0;
  mobdbType basetype = temptype_basetype(is->temptype);
  for (int i = 1; i < is->count; i++)
  {
    Datum value = tinstant_value(tinstantset_inst_n(is, i));
    if (datum_lt(value, min, basetype))
    {
      min = value;
      k = i;
    }
  }
  return tinstantset_inst_n(is, k);
}

/**
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return a pointer to the instant with minimum base value of a
 * temporal instant set
 * @sqlfunc maxInstant()
 */
const TInstant *
tinstantset_max_instant(const TInstantSet *is)
{
  Datum max = tinstant_value(tinstantset_inst_n(is, 0));
  int k = 0;
  mobdbType basetype = temptype_basetype(is->temptype);
  for (int i = 1; i < is->count; i++)
  {
    Datum value = tinstant_value(tinstantset_inst_n(is, i));
    if (datum_gt(value, max, basetype))
    {
      max = value;
      k = i;
    }
  }
  return tinstantset_inst_n(is, k);
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set to (the complement of) its
 * minimum/maximum base value
 *
 * @param[in] is Temporal instant set
 * @param[in] min True when restricted to the minumum value, false for the
 * maximum value
 * @param[in] atfunc True when the restriction is at, false for minus
 * @sqlfunc atMin(), atMax(), minusMin(), minusMax()
 */
TInstantSet *
tinstantset_restrict_minmax(const TInstantSet *is, bool min, bool atfunc)
{
  Datum minmax = min ? tinstantset_min_value(is) : tinstantset_max_value(is);
  return tinstantset_restrict_value(is, minmax, atfunc);
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
tinstantset_value_at_timestamp(const TInstantSet *is, TimestampTz t, Datum *result)
{
  int loc;
  if (! tinstantset_find_timestamp(is, t, &loc))
    return false;

  const TInstant *inst = tinstantset_inst_n(is, loc);
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
tinstantset_restrict_timestamp(const TInstantSet *is, TimestampTz t, bool atfunc)
{
  /* Bounding box test */
  Period p;
  tinstantset_set_period(is, &p);
  if (!contains_period_timestamp(&p, t))
    return atfunc ? NULL : (Temporal *) tinstantset_copy(is);

  /* Singleton instant set */
  if (is->count == 1)
    return atfunc ? (Temporal *) tinstant_copy(tinstantset_inst_n(is, 0)) : NULL;

  /* General case */
  const TInstant *inst;
  if (atfunc)
  {
    int loc;
    if (! tinstantset_find_timestamp(is, t, &loc))
      return NULL;
    inst = tinstantset_inst_n(is, loc);
    return (Temporal *) tinstant_copy(inst);
  }
  else
  {
    const TInstant **instants = palloc(sizeof(TInstant *) * is->count);
    int count = 0;
    for (int i = 0; i < is->count; i++)
    {
      inst= tinstantset_inst_n(is, i);
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
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set to (the complement of) a timestamp set.
 * @sqlfunc atTimestampSet(), minusTimestampSet()
 */
TInstantSet *
tinstantset_restrict_timestampset(const TInstantSet *is, const TimestampSet *ts,
  bool atfunc)
{
  TInstantSet *result;
  const TInstant *inst;

  /* Singleton timestamp set */
  if (ts->count == 1)
  {
    Temporal *temp = tinstantset_restrict_timestamp(is,
      timestampset_time_n(ts, 0), atfunc);
    if (temp == NULL || temp->subtype == TINSTANTSET)
      return (TInstantSet *) temp;
    TInstant *inst1 = (TInstant *) temp;
    result = tinstantset_make((const TInstant **) &inst1, 1, MERGE_NO);
    pfree(inst1);
    return result;
  }

  /* Bounding box test */
  Period p1;
  tinstantset_set_period(is, &p1);
  const Period *p2 = timestampset_period_ptr(ts);
  if (!overlaps_span_span(&p1, p2))
    return atfunc ? NULL : tinstantset_copy(is);


  /* Singleton instant set */
  if (is->count == 1)
  {
    inst = tinstantset_inst_n(is, 0);
    if (tinstant_restrict_timestampset_test(inst, ts, atfunc))
      return tinstantset_copy(is);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * is->count);
  int i = 0, j = 0, k = 0;
  while (i < is->count && j < ts->count)
  {
    inst = tinstantset_inst_n(is, i);
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
    while (i < is->count)
      instants[k++] = tinstantset_inst_n(is, i++);
  }
  result = (k == 0) ? NULL : tinstantset_make(instants, k, MERGE_NO);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set to (the complement of) a period.
 * @sqlfunc atPeriod(), minusPeriod()
 */
TInstantSet *
tinstantset_restrict_period(const TInstantSet *is, const Period *period,
  bool atfunc)
{
  /* Bounding box test */
  Period p;
  tinstantset_set_period(is, &p);
  if (!overlaps_span_span(&p, period))
    return atfunc ? NULL : tinstantset_copy(is);

  /* Singleton instant set */
  if (is->count == 1)
    return atfunc ? tinstantset_copy(is) : NULL;

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * is->count);
  int count = 0;
  for (int i = 0; i < is->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(is, i);
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
 * @ingroup libmeos_int_temporal_restrict
 * @brief Restrict a temporal instant set to (the complement of) a period set.
 * @sqlfunc atPeriodSet(), minusPeriodSet()
 */
TInstantSet *
tinstantset_restrict_periodset(const TInstantSet *is, const PeriodSet *ps,
  bool atfunc)
{
  const TInstant *inst;

  /* Singleton period set */
  if (ps->count == 1)
    return tinstantset_restrict_period(is, periodset_per_n(ps, 0), atfunc);

  /* Bounding box test */
  Period p1;
  tinstantset_set_period(is, &p1);
  const Period *p2 = periodset_period_ptr(ps);
  if (!overlaps_span_span(&p1, p2))
    return atfunc ? NULL : tinstantset_copy(is);

  /* Singleton instant set */
  if (is->count == 1)
  {
    inst = tinstantset_inst_n(is, 0);
    if (tinstant_restrict_periodset_test(inst, ps, atfunc))
      return tinstantset_copy(is);
    return NULL;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * is->count);
  int count = 0;
  for (int i = 0; i < is->count; i++)
  {
    inst = tinstantset_inst_n(is, i);
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
 * @ingroup libmeos_int_temporal_transf
 * @brief Append an instant to a temporal instant set.
 * @sqlfunc appendInstant()
 */
TInstantSet *
tinstantset_append_tinstant(const TInstantSet *is, const TInstant *inst)
{
  /* Ensure validity of the arguments */
  assert(is->temptype == inst->temptype);
  const TInstant *inst1 = tinstantset_inst_n(is, is->count - 1);
  ensure_increasing_timestamps(inst1, inst, MERGE);
  if (inst1->t == inst->t)
    return tinstantset_copy(is);

  /* Create the result */
  const TInstant **instants = palloc(sizeof(TInstant *) * is->count + 1);
  for (int i = 0; i < is->count; i++)
    instants[i] = tinstantset_inst_n(is, i);
  instants[is->count] = (TInstant *) inst;
  TInstantSet *result = tinstantset_make1(instants, is->count + 1);
  pfree(instants);
  return result;
}

/**
 * @ingroup libmeos_int_temporal_transf
 * @brief Merge two temporal instant sets.
 * @sqlfunc merge()
 */
Temporal *
tinstantset_merge(const TInstantSet *is1, const TInstantSet *is2)
{
  const TInstantSet *instsets[] = {is1, is2};
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
 * Temporally intersect a temporal instant set and a temporal instant
 *
 * @param[in] is,inst Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tinstantset_tinstant(const TInstantSet *is, const TInstant *inst,
  TInstant **inter1, TInstant **inter2)
{
  TInstant *inst1 = (TInstant *) tinstantset_restrict_timestamp(is, inst->t, REST_AT);
  if (inst1 == NULL)
    return false;

  *inter1 = inst1;
  *inter2 = tinstant_copy(inst);
  return true;
}

/**
 * Temporally intersect two temporal instant sets
 *
 * @param[in] inst,is Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tinstant_tinstantset(const TInstant *inst, const TInstantSet *is,
  TInstant **inter1, TInstant **inter2)
{
  return intersection_tinstantset_tinstant(is, inst, inter2, inter1);
}

/**
 * Temporally intersect two temporal instant sets
 *
 * @param[in] is1,is2 Input values
 * @param[out] inter1, inter2 Output values
 * @result Return false if the input values do not overlap on time
 */
bool
intersection_tinstantset_tinstantset(const TInstantSet *is1, const TInstantSet *is2,
  TInstantSet **inter1, TInstantSet **inter2)
{
  /* Test whether the bounding period of the two temporal values overlap */
  Period p1, p2;
  tinstantset_set_period(is1, &p1);
  tinstantset_set_period(is2, &p2);
  if (!overlaps_span_span(&p1, &p2))
    return false;

  int count = Min(is1->count, is2->count);
  const TInstant **instants1 = palloc(sizeof(TInstant *) * count);
  const TInstant **instants2 = palloc(sizeof(TInstant *) * count);
  int i = 0, j = 0, k = 0;
  const TInstant *inst1 = tinstantset_inst_n(is1, i);
  const TInstant *inst2 = tinstantset_inst_n(is2, j);
  while (i < is1->count && j < is2->count)
  {
    int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      instants1[k] = inst1;
      instants2[k++] = inst2;
      inst1 = tinstantset_inst_n(is1, ++i);
      inst2 = tinstantset_inst_n(is2, ++j);
    }
    else if (cmp < 0)
      inst1 = tinstantset_inst_n(is1, ++i);
    else
      inst2 = tinstantset_inst_n(is2, ++j);
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
 * @ingroup libmeos_int_temporal_time
 * @brief Return true if a temporal instant set intersects a timestamp.
 * @sqlfunc intersectsTimestamp()
 */
bool
tinstantset_intersects_timestamp(const TInstantSet *is, TimestampTz t)
{
  int loc;
  return tinstantset_find_timestamp(is, t, &loc);
}

/**
 * @ingroup libmeos_int_temporal_time
 * @brief Return true if a temporal instant set intersects a timestamp set.
 * @sqlfunc intersectsTimestampSet()
 */
bool
tinstantset_intersects_timestampset(const TInstantSet *is,
  const TimestampSet *ts)
{
  for (int i = 0; i < ts->count; i++)
    if (tinstantset_intersects_timestamp(is, timestampset_time_n(ts, i)))
      return true;
  return false;
}

/**
 * @ingroup libmeos_int_temporal_time
 * @brief Return true if a temporal instant set intersects a period.
 * @sqlfunc intersectsPeriod()
 */
bool
tinstantset_intersects_period(const TInstantSet *is, const Period *period)
{
  for (int i = 0; i < is->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(is, i);
    if (contains_period_timestamp(period, inst->t))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_int_temporal_time
 * @brief Return true if a temporal instant set intersects a period set.
 * @sqlfunc intersectsPeriodSet()
 */
bool
tinstantset_intersects_periodset(const TInstantSet *is, const PeriodSet *ps)
{
  for (int i = 0; i < ps->count; i++)
    if (tinstantset_intersects_period(is, periodset_per_n(ps, i)))
      return true;
  return false;
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
tnumberinstset_twavg(const TInstantSet *is)
{
  mobdbType basetype = temptype_basetype(is->temptype);
  double result = 0.0;
  for (int i = 0; i < is->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(is, i);
    result += datum_double(tinstant_value(inst), basetype);
  }
  return result / is->count;
}

/*****************************************************************************
 * Functions for defining B-tree indexes
 *****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_comp
 * @brief Return true if two temporal instant sets are equal.
 *
 * @pre The arguments are of the same base type
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
 */
bool
tinstantset_eq(const TInstantSet *is1, const TInstantSet *is2)
{
  assert(is1->temptype == is2->temptype);
  /* If number of sequences or flags are not equal */
  if (is1->count != is2->count || is1->flags != is2->flags)
    return false;

  /* If bounding boxes are not equal */
  void *box1 = tinstantset_bbox_ptr(is1);
  void *box2 = tinstantset_bbox_ptr(is2);
  if (! temporal_bbox_eq(box1, box2, is1->temptype))
    return false;

  /* Compare the composing instants */
  for (int i = 0; i < is1->count; i++)
  {
    const TInstant *inst1 = tinstantset_inst_n(is1, i);
    const TInstant *inst2 = tinstantset_inst_n(is2, i);
    if (! tinstant_eq(inst1, inst2))
      return false;
  }
  return true;
}

/**
 * @ingroup libmeos_int_temporal_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal instant
 * set is less than, equal, or greater than the second one.
 *
 * @pre The arguments are of the same base type
 * @note Period and bounding box comparison have been done by the calling
 * function temporal_cmp
 * @sqlfunc tbool_cmp(), tint_cmp(), tfloat_cmp(), ttext_cmp(), etc.
 */
int
tinstantset_cmp(const TInstantSet *is1, const TInstantSet *is2)
{
  assert(is1->temptype == is2->temptype);

  /* Compare composing instants */
  int count = Min(is1->count, is2->count);
  for (int i = 0; i < count; i++)
  {
    const TInstant *inst1 = tinstantset_inst_n(is1, i);
    const TInstant *inst2 = tinstantset_inst_n(is2, i);
    int result = tinstant_cmp(inst1, inst2);
    if (result)
      return result;
  }

  /* is1->count == is2->count because of the bounding box and the
   * composing instant tests above */

  /* is1->flags == is2->flags since the equality of flags were
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
 * @ingroup libmeos_int_temporal_accessor
 * @brief Return the 32-bit hash value of a temporal instant set
 * @sqlfunc tbool_hash(), tint_hash(), tfloat_hash(), ttext_hash(), etc.
 */
uint32
tinstantset_hash(const TInstantSet *is)
{
  uint32 result = 1;
  for (int i = 0; i < is->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(is, i);
    uint32 inst_hash = tinstant_hash(inst);
    result = (result << 5) - result + inst_hash;
  }
  return result;
}

/*****************************************************************************/
