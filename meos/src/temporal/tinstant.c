/*****************************************************************************
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
 * @brief General functions for temporal instants
 */

#include "temporal/tinstant.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
#include <common/hashfn.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/meos_catalog.h"
#include "temporal/postgres_types.h"
#include "temporal/tsequence.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_parser.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return (a pointer to) the base value of a temporal instant
 * @param[in] inst Temporal instant
 * @csqlfn #Tinstant_value()
 */
Datum
tinstant_value_p(const TInstant *inst)
{
  assert(inst);
  /* For base types passed by value */
  if (MEOS_FLAGS_GET_BYVAL(inst->flags))
    return inst->value;
  /* For base types passed by reference */
  return PointerGetDatum(&inst->value);
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return a copy of the base value of a temporal instant
 * @param[in] inst Temporal instant
 */
Datum
tinstant_value(const TInstant *inst)
{
  assert(inst);
  return datum_copy(tinstant_value_p(inst), temptype_basetype(inst->temptype));
}

/**
 * @brief Return the first argument initialized with the value and the
 * timestamptz
 * @param[in,out] inst Temporal instant to be modified
 * @param[in] value Value
 * @param[in] t Timestamp
 * @pre This function only works for for base types passed by value.
 * This should be ensured by the calling function!
 */
void
tinstant_set(TInstant *inst, Datum value, TimestampTz t)
{
  assert(inst);
  inst->t = t;
  inst->value = value;
  return;
}

/**
 * @brief Return the value of a temporal number instant converted to a double
 */
double
tnumberinst_double(const TInstant *inst)
{
  assert(inst);
  assert(tnumber_type(inst->temptype));
  Datum value = tinstant_value_p(inst);
  if (inst->temptype == T_TINT)
    return (double)(DatumGetInt32(value));
  else /* inst->temptype == T_TFLOAT */
    return DatumGetFloat8(value);
}

/*****************************************************************************
 * Intput/output functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal instant from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @param[in] temptype Temporal type
 */
TInstant *
tinstant_in(const char *str, meosType temptype)
{
  assert(str);
  TInstant *result;
  if (! tinstant_parse(&str, temptype, true, &result))
    return NULL;
  return result;
}
#endif /* MEOS */

/**
 * @brief Return the Well-Known Text (WKT) representation of a temporal instant
 * @param[in] inst Temporal instant
 * @param[in] maxdd Maximum number of decimal digits
 * @param[in] value_out Function called to output the base value depending on
 * its type
 */
char *
tinstant_to_string(const TInstant *inst, int maxdd, outfunc value_out)
{
  assert(inst); assert(maxdd >= 0);
  char *t = pg_timestamptz_out(inst->t);
  meosType basetype = temptype_basetype(inst->temptype);
  char *value = value_out(tinstant_value_p(inst), basetype, maxdd);
  size_t size = strlen(value) + strlen(t) + 2;
  char *result = palloc(size);
  snprintf(result, size, "%s@%s", value, t);
  pfree(t); pfree(value);
  return result;
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal instant
 * @param[in] inst Temporal instant
 * @param[in] maxdd Maximum number of decimal digits
 */
inline char *
tinstant_out(const TInstant *inst, int maxdd)
{
  return tinstant_to_string(inst, maxdd, &basetype_out);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_constructor
 * @brief Return a temporal instant from the arguments
 * @details The memory structure of a temporal instant is as follows
 * @code
 * ----------------
 * ( TInstant )_X |
 * ----------------
 * @endcode
 * where the attribute `value` of type `Datum` in the struct stores the base
 * value independently of whether it is passed by value or by reference.
 * For values passed by reference, the additional bytes needed are appended at
 * the end of the struct upon creation.
 *
 * @param[in] value Value
 * @param[in] temptype Temporal type
 * @param[in] t Timestamp
 */
TInstant *
tinstant_make(Datum value, meosType temptype, TimestampTz t)
{
  /* Ensure validity of arguments */
  int32_t tspatial_srid;
  // TODO Should we bypass the tests on tnpoint ?
  if (tspatial_type(temptype) && temptype != T_TNPOINT)
  {
    meosType basetype = temptype_basetype(temptype);
    tspatial_srid = spatial_srid(value, basetype);
    /* Ensure that the SRID is geodetic for geography */
    if (tgeodetic_type(temptype) && tspatial_srid != SRID_UNKNOWN && 
        ! ensure_srid_is_latlong(tspatial_srid))
      return NULL;
    /* Ensure that a geometry/geography is not empty */
    if (tgeo_type_all(temptype) && 
        ! ensure_not_empty(DatumGetGserializedP(value)))
      return NULL;
  }

  size_t value_offset = sizeof(TInstant) - sizeof(Datum);
  size_t size = value_offset;
  /* Create the temporal instant */
  size_t value_size;
  void *value_from;
  meosType basetype = temptype_basetype(temptype);
  bool typbyval = basetype_byvalue(basetype);
  /* Copy value */
  if (typbyval)
  {
    /* For base types passed by value */
    value_size = DOUBLE_PAD(sizeof(Datum));
    value_from = &value;
  }
  else
  {
    /* For base types passed by reference */
    int16 typlen = basetype_length(basetype);
    value_from = DatumGetPointer(value);
    value_size = (typlen != -1) ? DOUBLE_PAD((unsigned int) typlen) :
      DOUBLE_PAD(VARSIZE(value_from));
  }
  size += value_size;
  TInstant *result = palloc0(size);
  void *value_to = ((char *) result) + value_offset;
  memcpy(value_to, value_from, value_size);
  /* Initialize fixed-size values */
  result->temptype = temptype;
  result->subtype = TINSTANT;
  result->t = t;
  SET_VARSIZE(result, size);
  MEOS_FLAGS_SET_BYVAL(result->flags, typbyval);
  MEOS_FLAGS_SET_CONTINUOUS(result->flags, temptype_continuous(temptype));
  MEOS_FLAGS_SET_X(result->flags, true);
  MEOS_FLAGS_SET_T(result->flags, true);
  // TODO Should we bypass the tests on tnpoint ?
  if (tspatial_type(temptype) && temptype != T_TNPOINT)
  {
    int16 flags = spatial_flags(value, basetype);
    MEOS_FLAGS_SET_Z(result->flags, MEOS_FLAGS_GET_Z(flags));
    MEOS_FLAGS_SET_GEODETIC(result->flags, MEOS_FLAGS_GET_GEODETIC(flags));
  }
  return result;
}

/**
 * @brief Return a temporal instant created from the values
 * and free the base value after the creation
 * @param[in] value Values
 * @param[in] temptype Temporal type
 * @param[in] t Timestamp
 */
TInstant *
tinstant_make_free(Datum value, meosType temptype, TimestampTz t)
{
  TInstant *result = tinstant_make(value, temptype, t);
  DATUM_FREE(value, temptype_basetype(temptype));
  return result;
}

/**
 * @ingroup meos_internal_temporal_constructor
 * @brief Return a copy of a temporal instant
 * @param[in] inst Temporal instant
 */
TInstant *
tinstant_copy(const TInstant *inst)
{
  assert(inst);
  TInstant *result = palloc0(VARSIZE(inst));
  memcpy(result, inst, VARSIZE(inst));
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the singleton (pointer to the) base value of a temporal instant
 * @param[in] inst Temporal instant
 * @param[out] count Number of values in the output array
 * @post The output parameter @p count is equal to 1
 * @csqlfn #Temporal_valueset()
 */
Datum *
tinstant_values_p(const TInstant *inst, int *count)
{
  assert(inst); assert(count);
  Datum *result = palloc(sizeof(Datum));
  result[0] = tinstant_value_p(inst);
  *count = 1;
  return result;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the base values of a temporal instant number as a span set
 * @param[in] inst Temporal instant
 * @csqlfn #Temporal_valueset()
 */
SpanSet *
tnumberinst_valuespans(const TInstant *inst)
{
  assert(inst);
  Datum value = tinstant_value_p(inst);
  meosType basetype = temptype_basetype(inst->temptype);
  meosType spantype = basetype_spantype(basetype);
  Span s;
  span_set(value, value, true, true, basetype, spantype, &s);
  return span_to_spanset(&s);
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the time frame of a temporal instant as a span set
 * @param[in] inst Temporal instant
 * @csqlfn #Temporal_time()
 */
SpanSet *
tinstant_time(const TInstant *inst)
{
  assert(inst);
  return value_spanset(inst->t, T_TIMESTAMPTZ);
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return in the last argument the time span of a temporal instant
 * @param[in] inst Temporal instant
 * @param[out] s Result
 */
void
tinstant_set_tstzspan(const TInstant *inst, Span *s)
{
  assert(inst); assert(s);
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, s);
  return;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the singleton array of timestamps of a temporal instant
 * @param[in] inst Temporal instant
 * @param[out] count Number of values in the output array
 * @post The output parameter @p count is equal to 1
 * @csqlfn #Temporal_timestamps()
 */
TimestampTz *
tinstant_timestamps(const TInstant *inst, int *count)
{
  assert(inst); assert(count);
  TimestampTz *result = palloc(sizeof(TimestampTz));
  result[0] = inst->t;
  *count = 1;
  return result;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the singleton array of instants of a temporal instant
 * @param[in] inst Temporal instant
 * @param[out] count Number of values in the output array
 * @post The output parameter @p count is equal to 1
 * @csqlfn #Temporal_instants()
 */
const TInstant **
tinstant_insts(const TInstant *inst, int *count)
{
  assert(inst); assert(count);
  const TInstant **result = palloc(sizeof(TInstant *));
  result[0] = inst;
  *count = 1;
  return result;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return in the last argument a copy of the the value of a temporal
 * instant at a timestamptz
 * @param[in] inst Temporal instant
 * @param[in] t Timestamp
 * @param[out] result Result
 * @note Since the corresponding function for temporal sequences need to
 * interpolate the value, it is necessary to return a copy of the value
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
tinstant_value_at_timestamptz(const TInstant *inst, TimestampTz t,
  Datum *result)
{
  assert(inst); assert(result);
  if (t != inst->t)
    return false;
  *result = tinstant_value(inst);
  return true;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal sequence transformed into a temporal instant
 * @param[in] seq Temporal sequence
 * @csqlfn #Temporal_to_tinstant()
 */
TInstant *
tsequence_to_tinstant(const TSequence *seq)
{
  assert(seq);
  if (seq->count != 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot transform input value to a temporal instant");
    return NULL;
  }
  return tinstant_copy(TSEQUENCE_INST_N(seq, 0));
}

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal sequence set transformed into a temporal instant
 * @param[in] ss Temporal sequence set
 * @csqlfn #Temporal_to_tinstant()
 */
TInstant *
tsequenceset_to_tinstant(const TSequenceSet *ss)
{
  assert(ss);
  if (ss->totalcount != 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot transform input value to a temporal instant");
    return NULL;
  }
  return tinstant_copy(TSEQUENCE_INST_N(TSEQUENCESET_SEQ_N(ss, 0), 0));
}

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal instant whose value is shifted by a value
 * @param[in] inst Temporal instant
 * @param[in] shift Value to shift the instant
 * @csqlfn #Tnumber_shift_value()
 */
TInstant *
tnumberinst_shift_value(const TInstant *inst, Datum shift)
{
  assert(inst);
  TInstant *result = tinstant_copy(inst);
  Datum value = tinstant_value_p(result);
  meosType basetype = temptype_basetype(result->temptype);
  value = datum_add(value, shift, basetype);
  tinstant_set(result, value, result->t);
  return result;
}

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal instant shifted by an interval
 * @param[in] inst Temporal instant
 * @param[in] interv Interval to shift the instant
 * @csqlfn #Temporal_shift_time()
 */
TInstant *
tinstant_shift_time(const TInstant *inst, const Interval *interv)
{
  assert(inst); assert(interv);
  TInstant *result = tinstant_copy(inst);
  result->t = add_timestamptz_interval(inst->t, interv);
  return result;
}

/*****************************************************************************
 * Intersection function
 *****************************************************************************/

/**
 * @brief Temporally intersect two temporal instants
 * @param[in] inst1,inst2 Input values
 * @param[out] inter1, inter2 Output values
 * @return Return false if the values do not overlap on time
 */
bool
intersection_tinstant_tinstant(const TInstant *inst1, const TInstant *inst2,
  TInstant **inter1, TInstant **inter2)
{
  assert(inst1); assert(inst2);
  assert(inter1); assert(inter2);
  /* Test whether the two temporal instants overlap on time */
  if (inst1->t != inst2->t)
    return false;
  *inter1 = tinstant_copy(inst1);
  *inter2 = tinstant_copy(inst2);
  return true;
}

/*****************************************************************************
 * Comparison functions for defining B-tree index
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_comp_trad
 * @brief Return true if two temporal instants are equal
 * @param[in] inst1,inst2 Temporal instants
 * @pre The arguments are of the same base type
 * @note The function #tinstant_cmp() is not used to increase efficiency.
 * This function supposes for optimization purposes that the flags of
 * two temporal instants of the same base type are equal.
 * This hypothesis may change in the future and the function must be
 * adapted accordingly.
 * @csqlfn #Temporal_eq()
 */
bool
tinstant_eq(const TInstant *inst1, const TInstant *inst2)
{
  assert(inst1); assert(inst2); assert(inst1->temptype == inst2->temptype);
  /* Compare values and timestamps */
  Datum value1 = tinstant_value_p(inst1);
  Datum value2 = tinstant_value_p(inst2);
  return inst1->t == inst2->t && datum_eq(value1, value2,
    temptype_basetype(inst1->temptype));
}

/**
 * @ingroup meos_internal_temporal_comp_trad
 * @brief Return -1, 0, or 1 depending on whether the first temporal instant is
 * less than, equal, or greater than the second one
 * @param[in] inst1,inst2 Temporal instants
 * @pre The arguments are of the same base type
 * @note The internal B-tree comparator is not used to increase efficiency.
 * This function supposes for optimization purposes that the flags of
 * two temporal instants of the same base type are equal.
 * This hypothesis may change in the future and the function must be
 * adapted accordingly.
 * @csqlfn #Temporal_cmp()
 */
int
tinstant_cmp(const TInstant *inst1, const TInstant *inst2)
{
  assert(inst1); assert(inst2); assert(inst1->temptype == inst2->temptype);
  /* Compare timestamps */
  int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
  if (cmp < 0)
    return -1;
  if (cmp > 0)
    return 1;
  /* Compare values */
  if (datum_lt(tinstant_value_p(inst1), tinstant_value_p(inst2),
      temptype_basetype(inst1->temptype)))
    return -1;
  if (datum_gt(tinstant_value_p(inst1), tinstant_value_p(inst2),
      temptype_basetype(inst1->temptype)))
    return 1;
  /* The two values are equal */
  return 0;
}

/*****************************************************************************
 * Function for defining hash indexes
 * The function reuses the approach for span types for combining the hash of
 * the lower and upper bounds.
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the 32-bit hash value of a temporal instant
 * @param[in] inst Temporal instant
 * @csqlfn #Temporal_hash()
 */
uint32
tinstant_hash(const TInstant *inst)
{
  assert(inst);
  meosType basetype = temptype_basetype(inst->temptype);
  /* Apply the hash function to the base type */
  uint32 value_hash = datum_hash(tinstant_value_p(inst), basetype);
  /* Apply the hash function to the timestamp */
  uint32 time_hash = pg_hashint8(inst->t);
  /* Merge hashes of value and timestamp */
  uint32 result = value_hash;
  result = (result << 1) | (result >> 31);
  result ^= time_hash;
  return result;
}

/*****************************************************************************/
