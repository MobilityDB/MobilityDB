
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
 * @file tinstant.c
 * @brief General functions for temporal instants.
 */

#include "general/tinstant.h"

/* PostgreSQL */
#include <assert.h>
#include <libpq/pqformat.h>
#include <utils/builtins.h>
#include <utils/lsyscache.h>  /* for get_typlenbyval */
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
 * Return a pointer to the base value of the temporal instant value
 */
Datum *
tinstant_value_ptr(const TInstant *inst)
{
  return (Datum *)((char *)inst + double_pad(sizeof(TInstant)));
}

/**
 * Return the base value of the temporal value
 */
Datum
tinstant_value(const TInstant *inst)
{
  Datum *value = tinstant_value_ptr(inst);
  /* For base types passed by value */
  if (MOBDB_FLAGS_GET_BYVAL(inst->flags))
    return *value;
  /* For base types passed by reference */
  return PointerGetDatum(value);
}

/**
 * Return a copy of the base value of the temporal instant value
 */
Datum
tinstant_value_copy(const TInstant *inst)
{
  Datum *value = tinstant_value_ptr(inst);
  /* For base types passed by value */
  if (MOBDB_FLAGS_GET_BYVAL(inst->flags))
    return *value;
  /* For base types passed by reference */
  int16 typlen =  basetype_length(temptype_basetype(inst->temptype));
  size_t value_size = (typlen != -1) ? (unsigned int) typlen : VARSIZE(value);
  void *result = palloc0(value_size);
  memcpy(result, value, value_size);
  return PointerGetDatum(result);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal instant value from the arguments.
 *
 * The memory structure of a temporal instant value is as follows
 * @code
 * ----------------------------------
 * ( TInstant )_X | ( Value )_X |
 * ----------------------------------
 * @endcode
 * where the `_X` are unused bytes added for double padding.
 *
 * @param value Base value
 * @param t Timestamp
 * @param temptype Base type
 */
TInstant *
tinstant_make(Datum value, TimestampTz t, CachedType temptype)
{
  size_t value_offset = double_pad(sizeof(TInstant));
  size_t size = value_offset;
  /* Create the temporal value */
  TInstant *result;
  size_t value_size;
  void *value_from;
  CachedType basetype = temptype_basetype(temptype);
  /* Copy value */
  bool typbyval = basetype_byvalue(basetype);
  if (typbyval)
  {
    /* For base types passed by value */
    value_size = double_pad(sizeof(Datum));
    value_from = &value;
  }
  else
  {
    /* For base types passed by reference */
    value_from = DatumGetPointer(value);
    int16 typlen = basetype_length(basetype);
    value_size = (typlen != -1) ? double_pad((unsigned int) typlen) :
      double_pad(VARSIZE(value_from));
  }
  size += value_size;
  result = palloc0(size);
  void *value_to = ((char *) result) + value_offset;
  memcpy(value_to, value_from, value_size);
  /* Initialize fixed-size values */
  result->temptype = temptype;
  result->subtype = INSTANT;
  result->t = t;
  SET_VARSIZE(result, size);
  MOBDB_FLAGS_SET_BYVAL(result->flags, typbyval);
  bool continuous = temptype_continuous(temptype);
  MOBDB_FLAGS_SET_CONTINUOUS(result->flags, continuous);
  MOBDB_FLAGS_SET_LINEAR(result->flags, continuous);
  MOBDB_FLAGS_SET_X(result->flags, true);
  MOBDB_FLAGS_SET_T(result->flags, true);
  if (tgeo_type(temptype))
  {
    GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(value);
    MOBDB_FLAGS_SET_Z(result->flags, FLAGS_GET_Z(GS_FLAGS(gs)));
    MOBDB_FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(GS_FLAGS(gs)));
    PG_FREE_IF_COPY_P(gs, DatumGetPointer(value));
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Return a copy of the temporal instant value.
 */
TInstant *
tinstant_copy(const TInstant *inst)
{
  TInstant *result = palloc0(VARSIZE(inst));
  memcpy(result, inst, VARSIZE(inst));
  return result;
}

/**
 * Sets the value and the timestamp of the temporal instant value
 *
 * @param[in,out] inst Temporal value to be modified
 * @param[in] value Value
 * @param[in] t Timestamp
 * @pre This function only works for for base types passed by value.
 * This should be ensured by the calling function!
 */
void
tinstant_set(TInstant *inst, Datum value, TimestampTz t)
{
  inst->t = t;
  Datum *value_ptr = tinstant_value_ptr(inst);
  *value_ptr = value;
}

/*****************************************************************************
 * Intput/output functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the string representation of the temporal value.
 *
 * @param[in] inst Temporal value
 * @param[in] value_out Function called to output the base value
 *    depending on its Oid
 */
char *
tinstant_to_string(const TInstant *inst, char *(*value_out)(Oid, Datum))
{
  char *t = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst->t));
  CachedType basetype = temptype_basetype(inst->temptype);
  char *value = value_out(type_oid(basetype), tinstant_value(inst));
  char *result;
  if (inst->temptype == T_TTEXT)
  {
    result = palloc(strlen(value) + strlen(t) + 4);
    sprintf(result, "\"%s\"@%s", value, t);
  }
  else
  {
    result = palloc(strlen(value) + strlen(t) + 2);
    sprintf(result, "%s@%s", value, t);
  }
  pfree(t);
  pfree(value);
  return result;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Write the binary representation of the temporal value into the buffer.
 *
 * @param[in] inst Temporal value
 * @param[in] buf Buffer
 */
void
tinstant_write(const TInstant *inst, StringInfo buf)
{
  CachedType basetype = temptype_basetype(inst->temptype);
  bytea *bt = call_send(TIMESTAMPTZOID, TimestampTzGetDatum(inst->t));
  bytea *bv = call_send(type_oid(basetype), tinstant_value(inst));
  pq_sendbytes(buf, VARDATA(bt), VARSIZE(bt) - VARHDRSZ);
  pq_sendint32(buf, VARSIZE(bv) - VARHDRSZ) ;
  pq_sendbytes(buf, VARDATA(bv), VARSIZE(bv) - VARHDRSZ);
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a new temporal value from its binary representation read from
 * the buffer.
 *
 * @param[in] buf Buffer
 * @param[in] temptype Temporal type
 */
TInstant *
tinstant_read(StringInfo buf, CachedType temptype)
{
  TimestampTz t = call_recv(TIMESTAMPTZOID, buf);
  int size = pq_getmsgint(buf, 4) ;
  StringInfoData buf2 =
  {
    .cursor = 0,
    .len = size,
    .maxlen = size,
    .data = buf->data + buf->cursor
  };
  Datum value = call_recv(temptype_basetypid(temptype), &buf2);
  buf->cursor += size ;
  return tinstant_make(value, t, temptype);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of base values of the temporal value.
 */
Datum *
tinstant_values(const TInstant *inst)
{
  Datum *result = palloc(sizeof(Datum));
  result[0] = tinstant_value(inst);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of ranges of the temporal float value.
 */
RangeType **
tfloatinst_ranges(const TInstant *inst)
{
  RangeType **result = palloc(sizeof(RangeType));
  Datum value = tinstant_value(inst);
  result[0] = range_make(value, value, true, true, T_FLOAT8);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the time on which the temporal value is defined as a period set.
 */
PeriodSet *
tinstant_time(const TInstant *inst)
{
  PeriodSet *result = timestamp_periodset(inst->t);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the bounding period of the temporal instant value.
 */
void
tinstant_period(const TInstant *inst, Period *p)
{
  return period_set(inst->t, inst->t, true, true, p);
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of segments of the temporal value.
 */
TSequence **
tinstant_sequences(const TInstant *inst)
{
  TSequence **result = palloc(sizeof(TSequence *));
  result[0] = tinstant_tsequence(inst,
    MOBDB_FLAGS_GET_CONTINUOUS(inst->flags));
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of timestamps of the temporal value.
 */
TimestampTz *
tinstant_timestamps(const TInstant *inst)
{
  TimestampTz *result = palloc(sizeof(TimestampTz));
  result[0] = inst->t;
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of instants of the temporal value.
 */
const TInstant **
tinstant_instants(const TInstant *inst)
{
  const TInstant **result = palloc(sizeof(TInstant *));
  result[0] = inst;
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the base value of the temporal value at the timestamp.
 *
 * @note Since the corresponding function for temporal sequences need to
 * interpolate the value, it is necessary to return a copy of the value
 */
bool
tinstant_value_at_timestamp(const TInstant *inst, TimestampTz t, Datum *result)
{
  if (t != inst->t)
    return false;
  *result = tinstant_value_copy(inst);
  return true;
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast the temporal integer value as a temporal float value.
 */
TInstant *
tintinst_tfloatinst(const TInstant *inst)
{
  TInstant *result = tinstant_copy(inst);
  result->temptype = T_TFLOAT;
  MOBDB_FLAGS_SET_LINEAR(result->flags, true);
  Datum *value_ptr = tinstant_value_ptr(result);
  *value_ptr = Float8GetDatum((double)DatumGetInt32(tinstant_value(inst)));
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast the temporal float value as a temporal integer value.
 */
TInstant *
tfloatinst_tintinst(const TInstant *inst)
{
  TInstant *result = tinstant_copy(inst);
  result->temptype = T_TINT;
  MOBDB_FLAGS_SET_LINEAR(result->flags, true);
  Datum *value_ptr = tinstant_value_ptr(result);
  *value_ptr = Int32GetDatum((double)DatumGetFloat8(tinstant_value(inst)));
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal instant set value into a temporal instant value.
 */
TInstant *
tinstantset_tinstant(const TInstantSet *ti)
{
  if (ti->count != 1)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Cannot transform input to a temporal instant")));

  return tinstant_copy(tinstantset_inst_n(ti, 0));
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal sequence value into a temporal instant value.
 */
TInstant *
tsequence_tinstant(const TSequence *seq)
{
  if (seq->count != 1)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Cannot transform input to a temporal instant")));

  return tinstant_copy(tsequence_inst_n(seq, 0));
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Transform the temporal sequence set value into a temporal instant value.
 */
TInstant *
tsequenceset_tinstant(const TSequenceSet *ts)
{
  const TSequence *seq = tsequenceset_seq_n(ts, 0);
  if (ts->count != 1 || seq->count != 1)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Cannot transform input to a temporal instant")));

   return tinstant_copy(tsequence_inst_n(seq, 0));
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Shift the time span of the temporal value by the interval.
 */
TInstant *
tinstant_shift(const TInstant *inst, const Interval *interval)
{
  TInstant *result = tinstant_copy(inst);
  result->t = DatumGetTimestampTz(
    DirectFunctionCall2(timestamptz_pl_interval,
    TimestampTzGetDatum(inst->t), PointerGetDatum(interval)));
  return result;
}

/*****************************************************************************
 * Ever/always functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if temporal value is ever equal to the base value.
 */
bool
tinstant_ever_eq(const TInstant *inst, Datum value)
{
  return datum_eq(tinstant_value(inst), value,
    temptype_basetype(inst->temptype));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if temporal value is always equal to the base value.
 */
bool
tinstant_always_eq(const TInstant *inst, Datum value)
{
  return tinstant_ever_eq(inst, value);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is ever less than the base value.
 */
bool
tinstant_ever_lt(const TInstant *inst, Datum value)
{
  return datum_lt(tinstant_value(inst), value,
    temptype_basetype(inst->temptype));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is ever less than or equal to
 * the base value.
 */
bool
tinstant_ever_le(const TInstant *inst, Datum value)
{
  return datum_le(tinstant_value(inst), value,
    temptype_basetype(inst->temptype));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is always less than the base value.
 */
bool
tinstant_always_lt(const TInstant *inst, Datum value)
{
  return datum_lt(tinstant_value(inst), value,
    temptype_basetype(inst->temptype));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if the temporal value is always less than or equal to the
 * base value.
 */
bool
tinstant_always_le(const TInstant *inst, Datum value)
{
  return datum_le(tinstant_value(inst), value,
    temptype_basetype(inst->temptype));
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to (the complement of) the base value.
 */
TInstant *
tinstant_restrict_value(const TInstant *inst, Datum value, bool atfunc)
{
  if (datum_eq(value, tinstant_value(inst),
      temptype_basetype(inst->temptype)))
    return atfunc ? tinstant_copy(inst) : NULL;
  return atfunc ? NULL : tinstant_copy(inst);
}

/**
 * Return true if the temporal value satisfies the restriction to the
 * (complement of the) array of base values
 *
 * @pre There are no duplicates values in the array
 * @note This function is called for each composing instant in a temporal
 * instant set.
 */
bool
tinstant_restrict_values_test(const TInstant *inst, const Datum *values,
  int count, bool atfunc)
{
  Datum value = tinstant_value(inst);
  for (int i = 0; i < count; i++)
  {
    if (datum_eq(value, values[i], temptype_basetype(inst->temptype)))
      return atfunc ? true : false;
  }
  return atfunc ? false : true;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the array of base values.
 */
TInstant *
tinstant_restrict_values(const TInstant *inst, const Datum *values,
  int count, bool atfunc)
{
  if (tinstant_restrict_values_test(inst, values, count, atfunc))
    return tinstant_copy(inst);
  return NULL;
}

/**
 * Return true if the temporal number satisfies the restriction to the
 * (complement of the) range of base values
 *
 * @param[in] inst Temporal number
 * @param[in] range Range of base values
 * @param[in] atfunc True when the restriction is at, false for minus
 * @return Resulting temporal number
 * @note This function is called for each composing instant in a temporal instant set.
 */

bool
tnumberinst_restrict_range_test(const TInstant *inst, const RangeType *range,
  bool atfunc)
{
  Datum d = tinstant_value(inst);
  TypeCacheEntry *typcache = lookup_type_cache(range->rangetypid, TYPECACHE_RANGE_INFO);
#if POSTGRESQL_VERSION_NUMBER < 130000
  bool contains = range_contains_elem_internal(typcache, (RangeType *) range, d);
#else
  bool contains = range_contains_elem_internal(typcache, range, d);
#endif
  return atfunc ? contains : ! contains;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal number to the (complement of the) range of
 * base values.
 *
 * @param[in] inst Temporal number
 * @param[in] range Range of base values
 * @param[in] atfunc True when the restriction is at, false for minus
 * @return Resulting temporal number
 */
TInstant *
tnumberinst_restrict_range(const TInstant *inst, const RangeType *range,
  bool atfunc)
{
  if (tnumberinst_restrict_range_test(inst, range, atfunc))
    return tinstant_copy(inst);
  return NULL;
}

/**
 * Return true if the temporal number satisfies the restriction to the
 * (complement of the) array of ranges of base values
 * @pre The ranges are normalized
 * @note This function is called for each composing instant in a temporal
 * instant set.
 */
bool
tnumberinst_restrict_ranges_test(const TInstant *inst, RangeType **normranges,
  int count, bool atfunc)
{
  Datum d = tinstant_value(inst);
  TypeCacheEntry *typcache = lookup_type_cache(normranges[0]->rangetypid,
     TYPECACHE_RANGE_INFO);
  for (int i = 0; i < count; i++)
  {
    if (range_contains_elem_internal(typcache, normranges[i], d))
      return atfunc ? true : false;
  }
  /* Since the array of ranges has been filtered with the bounding box of
   * the temporal instant, normally we never reach here */
  return atfunc ? false : true;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal number to the (complement of the) array of
 * ranges of base values.
 */
TInstant *
tnumberinst_restrict_ranges(const TInstant *inst, RangeType **normranges,
  int count, bool atfunc)
{
  if (tnumberinst_restrict_ranges_test(inst, normranges, count, atfunc))
    return tinstant_copy(inst);
  return NULL;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the (complement of the) timestamp.
 *
 * @note Since the corresponding function for temporal sequences need to
 * interpolate the value, it is necessary to return a copy of the value
 */
TInstant *
tinstant_restrict_timestamp(const TInstant *inst, TimestampTz t, bool atfunc)
{
  if (t == inst->t)
    return atfunc ? tinstant_copy(inst) : NULL;
  return atfunc ? NULL : tinstant_copy(inst);
}

/**
 * Return true if the temporal value satisfies the restriction to the
 * timestamp set.
 *
 * @note This function is called for each composing instant in a temporal
 * instant set.
 */
bool
tinstant_restrict_timestampset_test(const TInstant *inst, const TimestampSet *ts,
  bool atfunc)
{
  for (int i = 0; i < ts->count; i++)
    if (inst->t == timestampset_time_n(ts, i))
      return atfunc ? true : false;
  return atfunc ? false : true;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the timestamp set.
 */
TInstant *
tinstant_restrict_timestampset(const TInstant *inst, const TimestampSet *ts,
  bool atfunc)
{
  if (tinstant_restrict_timestampset_test(inst, ts, atfunc))
    return tinstant_copy(inst);
  return NULL;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the period.
 */
TInstant *
tinstant_restrict_period(const TInstant *inst, const Period *period, bool atfunc)
{
  bool contains = contains_period_timestamp(period, inst->t);
  if ((atfunc && ! contains) || (! atfunc && contains))
    return NULL;
  return tinstant_copy(inst);
}

/**
 * Return true if the temporal value satisfies the restriction to the
 * timestamp set.
 * @note This function is called for each composing instant in a temporal
 * instant set.
 */
bool
tinstant_restrict_periodset_test(const TInstant *inst, const PeriodSet *ps, bool atfunc)
{
  for (int i = 0; i < ps->count; i++)
    if (contains_period_timestamp(periodset_per_n(ps, i), inst->t))
      return atfunc ? true : false;
  return atfunc ? false : true;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the period set.
 */
TInstant *
tinstant_restrict_periodset(const TInstant *inst,const  PeriodSet *ps, bool atfunc)
{
  if (tinstant_restrict_periodset_test(inst, ps, atfunc))
    return tinstant_copy(inst);
  return NULL;
}

/*****************************************************************************
 * Merge functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_transf
 * @brief Merge the two temporal instant values.
 */
Temporal *
tinstant_merge(const TInstant *inst1, const TInstant *inst2)
{
  const TInstant *instants[] = {inst1, inst2};
  return tinstant_merge_array(instants, 2);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Merge the array of temporal instant values.
 *
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @pre The number of elements in the array is greater than 1
 */
Temporal *
tinstant_merge_array(const TInstant **instants, int count)
{
  assert(count > 1);
  tinstarr_sort((TInstant **) instants, count);
  /* Ensure validity of the arguments */
  ensure_valid_tinstarr(instants, count, MERGE, INSTANT);

  const TInstant **newinstants = palloc(sizeof(TInstant *) * count);
  memcpy(newinstants, instants, sizeof(TInstant *) * count);
  int newcount = tinstarr_remove_duplicates(newinstants, count);
  Temporal *result = (newcount == 1) ?
    (Temporal *) tinstant_copy(newinstants[0]) :
    (Temporal *) tinstantset_make1(newinstants, newcount);
  pfree(newinstants);
  return result;
}

/*****************************************************************************
 * Intersection function
 *****************************************************************************/

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] inst1,inst2 Input values
 * @param[out] inter1, inter2 Output values
 * @return Return false if the values do not overlap on time
 */
bool
intersection_tinstant_tinstant(const TInstant *inst1, const TInstant *inst2,
  TInstant **inter1, TInstant **inter2)
{
  /* Test whether the two temporal values overlap on time */
  if (inst1->t != inst2->t)
    return false;
  *inter1 = tinstant_copy(inst1);
  *inter2 = tinstant_copy(inst2);
  return true;
}

/*****************************************************************************
 * Intersects functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersects the timestamp.
 */
bool
tinstant_intersects_timestamp(const TInstant *inst, TimestampTz t)
{
  return (inst->t == t);
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersects the timestamp set.
 */
bool
tinstant_intersects_timestampset(const TInstant *inst,
  const TimestampSet *ts)
{
  for (int i = 0; i < ts->count; i++)
    if (inst->t == timestampset_time_n(ts, i))
      return true;
  return false;
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersects the period.
 */
bool
tinstant_intersects_period(const TInstant *inst, const Period *p)
{
  return contains_period_timestamp(p, inst->t);
}

/**
 * @ingroup libmeos_temporal_time
 * @brief Return true if the temporal value intersects the period set.
 */
bool
tinstant_intersects_periodset(const TInstant *inst, const PeriodSet *ps)
{
  for (int i = 0; i < ps->count; i++)
    if (contains_period_timestamp(periodset_per_n(ps, i), inst->t))
      return true;
  return false;
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the two temporal instant values are equal.
 *
 * @pre The arguments are of the same base type
 * @note The internal B-tree comparator is not used to increase efficiency.
 * @note This function supposes for optimization purposes that the flags of
 * two temporal instant values of the same base type are equal.
 * This hypothesis may change in the future and the function must be
 * adapted accordingly.
 */
bool
tinstant_eq(const TInstant *inst1, const TInstant *inst2)
{
  assert(inst1->temptype == inst2->temptype);
  /* Compare values and timestamps */
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  return inst1->t == inst2->t && datum_eq(value1, value2,
    temptype_basetype(inst1->temptype));
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal value is
 * less than, equal, or greater than the second one.
 *
 * @pre The arguments are of the same base type
 * @note The internal B-tree comparator is not used to increase efficiency.
 * @note This function supposes for optimization purposes that the flags of
 * two temporal instant values of the same base type are equal.
 * This hypothesis may change in the future and the function must be
 * adapted accordingly.
 */
int
tinstant_cmp(const TInstant *inst1, const TInstant *inst2)
{
  assert(inst1->temptype == inst2->temptype);
  /* Compare timestamps */
  int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
  if (cmp < 0)
    return -1;
  if (cmp > 0)
    return 1;
  /* Compare values */
  if (datum_lt(tinstant_value(inst1), tinstant_value(inst2),
      temptype_basetype(inst1->temptype)))
    return -1;
  if (datum_gt(tinstant_value(inst1), tinstant_value(inst2),
      temptype_basetype(inst1->temptype)))
    return 1;
  /* The two values are equal */
  return 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for range types for combining the hash of
 * the lower and upper bounds.
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the hash value of the temporal value.
 */
uint32
tinstant_hash(const TInstant *inst)
{
  uint32 result;
  uint32 time_hash;

  Datum value = tinstant_value(inst);
  /* Apply the hash function according to the subtype */
  uint32 value_hash = 0;
  ensure_temporal_type(inst->temptype);
  if (inst->temptype == T_TBOOL)
    value_hash = DatumGetUInt32(call_function1(hashchar, value));
  else if (inst->temptype == T_TINT)
    value_hash = DatumGetUInt32(call_function1(hashint4, value));
  else if (inst->temptype == T_TFLOAT)
    value_hash = DatumGetUInt32(call_function1(hashfloat8, value));
  else if (inst->temptype == T_TTEXT)
    value_hash = DatumGetUInt32(call_function1(hashtext, value));
  else if (tgeo_type(inst->temptype))
    value_hash = DatumGetUInt32(call_function1(lwgeom_hash, value));
  else if (inst->temptype == T_TNPOINT)
  {
    value_hash = DatumGetUInt32(call_function1(hashint8, value));
    value_hash ^= DatumGetUInt32(call_function1(hashfloat8, value));
  }
  else
    elog(ERROR, "unknown hash function for temporal type: %d", inst->temptype);
  /* Apply the hash function according to the timestamp */
  time_hash = DatumGetUInt32(call_function1(hashint8, TimestampTzGetDatum(inst->t)));

  /* Merge hashes of value and timestamp */
  result = value_hash;
  result = (result << 1) | (result >> 31);
  result ^= time_hash;

  return result;
}

/*****************************************************************************/
