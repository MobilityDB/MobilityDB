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
 * @brief General functions for temporal instants.
 */

#include "general/tinstant.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
#if POSTGRESQL_VERSION_NUMBER >= 130000
  #include <common/hashfn.h>
#else
  #include <access/hash.h>
#endif
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/spanset.h"
#include "general/temporaltypes.h"
#include "general/type_parser.h"
#include "general/type_util.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_spatialfuncs.h"
#if NPOINT
  #include "npoint/tnpoint.h"
  #include "npoint/tnpoint_static.h"
#endif

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the base value of a temporal instant
 * @sqlfunc getValue()
 */
Datum
tinstant_value(const TInstant *inst)
{
  /* For base types passed by value */
  if (MEOS_FLAGS_GET_BYVAL(inst->flags))
    return inst->value;
  /* For base types passed by reference */
  return PointerGetDatum(&inst->value);
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return a copy of the base value of a temporal instant
 */
Datum
tinstant_value_copy(const TInstant *inst)
{
  /* For base types passed by value */
  if (MEOS_FLAGS_GET_BYVAL(inst->flags))
    return inst->value;
  /* For base types passed by reference */
  int16 typlen =  basetype_length(temptype_basetype(inst->temptype));
  size_t value_size = (typlen != -1) ?
    (unsigned int) typlen : VARSIZE(&inst->value);
  void *result = palloc0(value_size);
  memcpy(result, &inst->value, value_size);
  return PointerGetDatum(result);
}

/**
 * @brief Set the value and the timestamp of a temporal instant
 *
 * @param[in,out] inst Temporal instant to be modified
 * @param[in] value Value
 * @param[in] t Timestamp
 * @pre This function only works for for base types passed by value.
 * This should be ensured by the calling function!
 */
void
tinstant_set(TInstant *inst, Datum value, TimestampTz t)
{
  inst->t = t;
  inst->value = value;
}

/**
 * @brief Convert the value of temporal number instant to a double
 */
double
tnumberinst_double(const TInstant *inst)
{
  assert(tnumber_type(inst->temptype));
  Datum d = tinstant_value(inst);
  if (inst->temptype == T_TINT)
    return (double)(DatumGetInt32(d));
  else /* inst->temptype == T_TFLOAT */
    return DatumGetFloat8(d);
}

/*****************************************************************************
 * Intput/output functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return a temporal instant from its Well-Known Text (WKT) representation.
 *
 * @param[in] str String
 * @param[in] temptype Temporal type
 */
TInstant *
tinstant_in(const char *str, meosType temptype)
{
  return tinstant_parse(&str, temptype, true, true);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return a temporal instant boolean from its Well-Known Text (WKT)
 * representation.
 */
TInstant *
tboolinst_in(const char *str)
{
  return tinstant_parse(&str, T_TBOOL, true, true);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return a temporal instant integer from its Well-Known Text (WKT)
 * representation.
 */
TInstant *
tintinst_in(const char *str)
{
  return tinstant_parse(&str, T_TINT, true, true);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return a temporal instant float from its Well-Known Text (WKT)
 * representation.
 */
TInstant *
tfloatinst_in(const char *str)
{
  return tinstant_parse(&str, T_TFLOAT, true, true);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return a temporal instant text from its Well-Known Text (WKT)
 * representation.
 */
TInstant *
ttextinst_in(const char *str)
{
  return tinstant_parse(&str, T_TTEXT, true, true);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return a temporal instant geometric point from its Well-Known Text
 * (WKT) representation.
 */
TInstant *
tgeompointinst_in(const char *str)
{
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tpoint_parse(&str, T_TGEOMPOINT);
  assert (temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return a temporal instant geographic point from its Well-Known Text
 * (WKT) representation.
 */
TInstant *
tgeogpointinst_in(const char *str)
{
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tpoint_parse(&str, T_TGEOGPOINT);
  assert (temp->subtype == TINSTANT);
  return (TInstant *) temp;
}
#endif

/**
 * @brief Return the Well-Known Text (WKT) representation of a temporal instant.
 * @param[in] inst Temporal instant
 * @param[in] maxdd Maximum number of decimal digits to output for floating
 * point values
 * @param[in] value_out Function called to output the base value depending on
 * its type
 */
char *
tinstant_to_string(const TInstant *inst, int maxdd, outfunc value_out)
{
  char *t = pg_timestamptz_out(inst->t);
  meosType basetype = temptype_basetype(inst->temptype);
  char *value = value_out(tinstant_value(inst), basetype, maxdd);
  char *result = palloc(strlen(value) + strlen(t) + 2);
  sprintf(result, "%s@%s", value, t);
  pfree(t);
  pfree(value);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal instant.
 */
char *
tinstant_out(const TInstant *inst, int maxdd)
{
  return tinstant_to_string(inst, maxdd, &basetype_out);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_constructor
 * @brief Construct a temporal instant from the arguments.
 *
 * The memory structure of a temporal instant is as follows
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
 * @param value Base value
 * @param t Timestamp
 * @param temptype Base type
 * @sqlfunc tbool_inst(), tint_inst(), tfloat_inst(), ttext_inst(), etc.
 */
TInstant *
tinstant_make(Datum value, meosType temptype, TimestampTz t)
{
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
  bool continuous = temptype_continuous(temptype);
  MEOS_FLAGS_SET_CONTINUOUS(result->flags, continuous);
  MEOS_FLAGS_SET_X(result->flags, true);
  MEOS_FLAGS_SET_T(result->flags, true);
  if (tgeo_type(temptype))
  {
    GSERIALIZED *gs = DatumGetGserializedP(value);
    MEOS_FLAGS_SET_Z(result->flags, FLAGS_GET_Z(gs->gflags));
    MEOS_FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(gs->gflags));
    PG_FREE_IF_COPY_P(gs, DatumGetPointer(value));
  }
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal instant boolean from the arguments.
 * @sqlfunc tbool_inst()
 */
TInstant *
tboolinst_make(bool b, TimestampTz t)
{
  return tinstant_make(BoolGetDatum(b), T_TBOOL, t);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal instant integer from the arguments.
 * @sqlfunc tbool_inst(), tint_inst(), tfloat_inst(), ttext_inst(), etc.
 */
TInstant *
tintinst_make(int i, TimestampTz t)
{
  return tinstant_make(Int32GetDatum(i), T_TINT, t);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal instant float from the arguments.
 * @sqlfunc tfloat_inst()
 */
TInstant *
tfloatinst_make(double d, TimestampTz t)
{
  return tinstant_make(Float8GetDatum(d), T_TFLOAT, t);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal instant text from the arguments.
 * @sqlfunc tint_inst()
 */
TInstant *
ttextinst_make(const text *txt, TimestampTz t)
{
  return tinstant_make(PointerGetDatum(txt), T_TTEXT, t);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal instant geometric point from the arguments.
 * @sqlfunc tgeompoint_inst()
 */
TInstant *
tgeompointinst_make(const GSERIALIZED *gs, TimestampTz t)
{
  return tinstant_make(PointerGetDatum(gs), T_TGEOMPOINT, t);
}
/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal instant geographic point from the arguments.
 * @sqlfunc tgeogpoint_inst().
 */
TInstant *
tgeogpointinst_make(const GSERIALIZED *gs, TimestampTz t)
{
  return tinstant_make(PointerGetDatum(gs), T_TGEOGPOINT, t);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_temporal_constructor
 * @brief Return a copy of a temporal instant.
 */
TInstant *
tinstant_copy(const TInstant *inst)
{
  TInstant *result = palloc0(VARSIZE(inst));
  memcpy(result, inst, VARSIZE(inst));
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the singleton base value of a temporal instant.
 * @post The output parameter @p count is equal to 1
 * @sqlfunc getValues()
 */
Datum *
tinstant_values(const TInstant *inst, int *count)
{
  Datum *result = palloc(sizeof(Datum));
  result[0] = tinstant_value(inst);
  *count = 1;
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the base values of a temporal instant number as a span set.
 * @sqlfunc getValues()
 */
SpanSet *
tnumberinst_valuespans(const TInstant *inst)
{
  Datum value = tinstant_value(inst);
  Span s;
  span_set(value, value, true, true, temptype_basetype(inst->temptype), &s);
  return span_to_spanset(&s);
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the time frame of a temporal instant as a period set.
 * @sqlfunc getTime()
 */
SpanSet *
tinstant_time(const TInstant *inst)
{
  SpanSet *result = timestamp_to_periodset(inst->t);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Compute the bounding period of a temporal instant.
 * @sqlfunc period()
 * @sqlop @p ::
 */
void
tinstant_set_period(const TInstant *inst, Span *p)
{
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, p);
  return;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the singleton array of timestamps of a temporal instant.
 * @post The output parameter @p count is equal to 1
 * @sqlfunc timestamps()
 */
TimestampTz *
tinstant_timestamps(const TInstant *inst, int *count)
{
  TimestampTz *result = palloc(sizeof(TimestampTz));
  result[0] = inst->t;
  *count = 1;
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the singleton array of instants of a temporal instant.
 * @post The output parameter @p count is equal to 1
 * @sqlfunc instants()
 */
const TInstant **
tinstant_instants(const TInstant *inst, int *count)
{
  const TInstant **result = palloc(sizeof(TInstant *));
  result[0] = inst;
  *count = 1;
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Compute the base value of a temporal instant at a timestamp.
 * @note Since the corresponding function for temporal sequences need to
 * interpolate the value, it is necessary to return a copy of the value
 * @sqlfunc valueAtTimestamp()
 * @pymeosfunc TInstant.valueAtTimestamp()
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
 * @ingroup libmeos_internal_temporal_cast
 * @brief Cast a temporal instant integer to a temporal instant float.
 * @sqlop @p ::
 */
TInstant *
tintinst_to_tfloatinst(const TInstant *inst)
{
  TInstant *result = tinstant_copy(inst);
  result->temptype = T_TFLOAT;
  MEOS_FLAGS_SET_CONTINUOUS(result->flags, true);
  result->value = Float8GetDatum((double) DatumGetInt32(tinstant_value(inst)));
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_cast
 * @brief Cast a temporal instant float to a temporal instant integer.
 * @sqlop @p ::
 */
TInstant *
tfloatinst_to_tintinst(const TInstant *inst)
{
  TInstant *result = tinstant_copy(inst);
  result->temptype = T_TINT;
  MEOS_FLAGS_SET_CONTINUOUS(result->flags, false);
  result->value = Int32GetDatum((double) DatumGetFloat8(tinstant_value(inst)));
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Return a temporal sequence transformed into a temporal instant.
 * @sqlfunc tbool_inst(), tint_inst(), tfloat_inst(), ttext_inst(), etc.
 */
TInstant *
tsequence_to_tinstant(const TSequence *seq)
{
  if (seq->count != 1)
    elog(ERROR, "Cannot transform input value to a temporal instant");

  return tinstant_copy(TSEQUENCE_INST_N(seq, 0));
}

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Return a temporal sequence set transformed into a temporal instant.
 * @sqlfunc tbool_inst(), tint_inst(), tfloat_inst(), ttext_inst(), etc.
 */
TInstant *
tsequenceset_to_tinstant(const TSequenceSet *ss)
{
  if (ss->totalcount != 1)
    elog(ERROR, "Cannot transform input value to a temporal instant");
  return tinstant_copy(TSEQUENCE_INST_N(TSEQUENCESET_SEQ_N(ss, 0), 0));
}

/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Return a temporal instant shifted by an interval.
 * @sqlfunc shift()
 */
TInstant *
tinstant_shift(const TInstant *inst, const Interval *interval)
{
  TInstant *result = tinstant_copy(inst);
  result->t = pg_timestamp_pl_interval(inst->t, interval);
  return result;
}

/*****************************************************************************
 * Ever/always functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal instant is ever equal to a base value.
 * @sqlop @p ?=
 */
bool
tinstant_ever_eq(const TInstant *inst, Datum value)
{
  return datum_eq(tinstant_value(inst), value,
    temptype_basetype(inst->temptype));
}

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal instant is always equal to a base value.
 * @sqlop @p %=
 */
bool
tinstant_always_eq(const TInstant *inst, Datum value)
{
  return tinstant_ever_eq(inst, value);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal instant is ever less than a base value.
 * @sqlop @p ?<
 */
bool
tinstant_ever_lt(const TInstant *inst, Datum value)
{
  return datum_lt(tinstant_value(inst), value,
    temptype_basetype(inst->temptype));
}

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal instant is ever less than or equal to
 * a base value.
 * @sqlop @p ?<=
 */
bool
tinstant_ever_le(const TInstant *inst, Datum value)
{
  return datum_le(tinstant_value(inst), value,
    temptype_basetype(inst->temptype));
}

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal instant is always less than a base value.
 * @sqlop @p %<
 */
bool
tinstant_always_lt(const TInstant *inst, Datum value)
{
  return datum_lt(tinstant_value(inst), value,
    temptype_basetype(inst->temptype));
}

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal instant is always less than or equal to a
 * base value.
 * @sqlop @p %<=
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
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal instant to (the complement of) a base value.
 * @sqlfunc atValue(), minusValue()
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
 * @brief Return true if a temporal instant satisfies the restriction to
 * (the complement of) an array of base values
 *
 * @pre There are no duplicates values in the array
 * @note This function is called for each composing instant in a temporal
 * discrete sequence.
 */
bool
tinstant_restrict_values_test(const TInstant *inst, const Set *set,
  bool atfunc)
{
  Datum value = tinstant_value(inst);
  meosType basetype = temptype_basetype(inst->temptype);
  for (int i = 0; i < set->count; i++)
  {
    if (datum_eq(value, SET_VAL_N(set, i), basetype))
      return atfunc ? true : false;
  }
  return atfunc ? false : true;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal instant to an array of base values.
 * @sqlfunc atValues(), minusValues()
 */
TInstant *
tinstant_restrict_values(const TInstant *inst, const Set *set, bool atfunc)
{
  if (tinstant_restrict_values_test(inst, set, atfunc))
    return tinstant_copy(inst);
  return NULL;
}

/**
 * @brief Return true if a temporal number instant satisfies the restriction to
 * (the complement of) a span of base values
 *
 * @param[in] inst Temporal number
 * @param[in] span Span of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @return Resulting temporal number
 * @note This function is called for each composing instant in a temporal
 * discrete sequence.
 */
bool
tnumberinst_restrict_span_test(const TInstant *inst, const Span *span,
  bool atfunc)
{
  Datum d = tinstant_value(inst);
  meosType basetype = temptype_basetype(inst->temptype);
  bool contains = contains_span_value(span, d, basetype);
  return atfunc ? contains : ! contains;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal number instant to (the complement of) a span of
 * base values.
 *
 * @param[in] inst Temporal number
 * @param[in] span Span of base values
 * @param[in] atfunc True if the restriction is at, false for minus
 * @return Resulting temporal number
 * @sqlfunc atSpan(), minusSpan()
 */
TInstant *
tnumberinst_restrict_span(const TInstant *inst, const Span *span,
  bool atfunc)
{
  if (tnumberinst_restrict_span_test(inst, span, atfunc))
    return tinstant_copy(inst);
  return NULL;
}

/**
 * @brief Return true if a temporal number satisfies the restriction to
 * (the complement of) an array of spans of base values
 * @note This function is called for each composing instant in a temporal
 * discrete sequence.
 */
bool
tnumberinst_restrict_spanset_test(const TInstant *inst, const SpanSet *ss,
  bool atfunc)
{
  Datum d = tinstant_value(inst);
  meosType basetype = temptype_basetype(inst->temptype);
  for (int i = 0; i < ss->count; i++)
  {
    const Span *s = spanset_sp_n(ss, i);
    if (contains_span_value(s, d, basetype))
      return atfunc ? true : false;
  }
  return atfunc ? false : true;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal number instant to (the complement of) a span set.
 * @sqlfunc atSpanset(), minusSpanset()
 */
TInstant *
tnumberinst_restrict_spanset(const TInstant *inst, const SpanSet *ss,
  bool atfunc)
{
  if (tnumberinst_restrict_spanset_test(inst, ss, atfunc))
    return tinstant_copy(inst);
  return NULL;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal instant to (the complement of) a timestamp.
 *
 * @note Since the corresponding function for temporal sequences need to
 * interpolate the value, it is necessary to return a copy of the value
 * @sqlfunc atTimestamp(), minusTimestamp()
 */
TInstant *
tinstant_restrict_timestamp(const TInstant *inst, TimestampTz t, bool atfunc)
{
  if (t == inst->t)
    return atfunc ? tinstant_copy(inst) : NULL;
  return atfunc ? NULL : tinstant_copy(inst);
}

/**
 * @brief Return true if a temporal instant satisfies the restriction to
 * (the complement of) a timestamp set.
 *
 * @note This function is called for each composing instant in a temporal
 * discrete sequence.
 */
bool
tinstant_restrict_timestampset_test(const TInstant *inst, const Set *ts,
  bool atfunc)
{
  for (int i = 0; i < ts->count; i++)
    if (inst->t == DatumGetTimestampTz(SET_VAL_N(ts, i)))
      return atfunc ? true : false;
  return atfunc ? false : true;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal instant to (the complement of) a timestamp set.
 * @sqlfunc atTstzSet(), minusTstzSet()
 */
TInstant *
tinstant_restrict_timestampset(const TInstant *inst, const Set *ts,
  bool atfunc)
{
  if (tinstant_restrict_timestampset_test(inst, ts, atfunc))
    return tinstant_copy(inst);
  return NULL;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal instant to (the complement of) a period.
 * @sqlfunc atPeriod(), minusPeriod()
 */
TInstant *
tinstant_restrict_period(const TInstant *inst, const Span *period,
  bool atfunc)
{
  bool contains = contains_period_timestamp(period, inst->t);
  if ((atfunc && ! contains) || (! atfunc && contains))
    return NULL;
  return tinstant_copy(inst);
}

/**
 * @brief Return true if a temporal instant satisfies the restriction to
 * (the complement of) a timestamp set.
 * @note This function is called for each composing instant in a temporal
 * discrete sequence.
 */
bool
tinstant_restrict_periodset_test(const TInstant *inst, const SpanSet *ps,
  bool atfunc)
{
  for (int i = 0; i < ps->count; i++)
    if (contains_period_timestamp(spanset_sp_n(ps, i), inst->t))
      return atfunc ? true : false;
  return atfunc ? false : true;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal instant to (the complement of) a period set.
 * @sqlfunc atTime(), minusTime()
 */
TInstant *
tinstant_restrict_periodset(const TInstant *inst,const  SpanSet *ps,
  bool atfunc)
{
  if (tinstant_restrict_periodset_test(inst, ps, atfunc))
    return tinstant_copy(inst);
  return NULL;
}

/*****************************************************************************
 * Merge functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_modif
 * @brief Merge two temporal instants.
 */
Temporal *
tinstant_merge(const TInstant *inst1, const TInstant *inst2)
{
  const TInstant *instants[] = {inst1, inst2};
  return tinstant_merge_array(instants, 2);
}

/**
 * @ingroup libmeos_internal_temporal_modif
 * @brief Merge an array of temporal instants.
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
  /* Ensure validity of the arguments and compute the bounding box */
  ensure_valid_tinstarr(instants, count, MERGE, DISCRETE);
  const TInstant **newinstants = palloc(sizeof(TInstant *) * count);
  memcpy(newinstants, instants, sizeof(TInstant *) * count);
  int newcount = tinstarr_remove_duplicates(newinstants, count);
  Temporal *result = (newcount == 1) ?
    (Temporal *) tinstant_copy(newinstants[0]) :
    (Temporal *) tsequence_make1_exp(newinstants, newcount, newcount, true,
      true, DISCRETE, NORMALIZE_NO, NULL);
  pfree(newinstants);
  return result;
}

/*****************************************************************************
 * Intersection function
 *****************************************************************************/

/**
 * @brief Temporally intersect two temporal instants
 *
 * @param[in] inst1,inst2 Input values
 * @param[out] inter1, inter2 Output values
 * @return Return false if the values do not overlap on time
 */
bool
intersection_tinstant_tinstant(const TInstant *inst1, const TInstant *inst2,
  TInstant **inter1, TInstant **inter2)
{
  /* Test whether the two temporal instants overlap on time */
  if (inst1->t != inst2->t)
    return false;
  *inter1 = tinstant_copy(inst1);
  *inter2 = tinstant_copy(inst2);
  return true;
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_comp
 * @brief Return true if two temporal instants are equal.
 *
 * @pre The arguments are of the same base type
 * @note The internal B-tree comparator is not used to increase efficiency.
 * @note This function supposes for optimization purposes that the flags of
 * two temporal instants of the same base type are equal.
 * This hypothesis may change in the future and the function must be
 * adapted accordingly.
 * @sqlop @p =
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
 * @ingroup libmeos_internal_temporal_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal instant is
 * less than, equal, or greater than the second one.
 *
 * @pre The arguments are of the same base type
 * @note The internal B-tree comparator is not used to increase efficiency.
 * @note This function supposes for optimization purposes that the flags of
 * two temporal instants of the same base type are equal.
 * This hypothesis may change in the future and the function must be
 * adapted accordingly.
 * @sqlfunc tbool_cmp(), tint_cmp(), tfloat_cmp(), ttext_cmp(), etc.
 */
int
tinstant_cmp(const TInstant *inst1, const TInstant *inst2)
{
  assert(inst1->temptype == inst2->temptype);
  /* Compare timestamps */
  int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
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
 * The function reuses the approach for span types for combining the hash of
 * the lower and upper bounds.
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the 32-bit hash value of a temporal instant.
 * @sqlfunc tbool_hash(), tint_hash(), tfloat_hash(), ttext_hash(), etc.
 */
uint32
tinstant_hash(const TInstant *inst)
{
  Datum value = tinstant_value(inst);
  meosType basetype = temptype_basetype(inst->temptype);
  /* Apply the hash function to the base type */
  uint32 value_hash = datum_hash(value, basetype);
  /* Apply the hash function to the timestamp */
  uint32 time_hash = pg_hashint8(inst->t);
  /* Merge hashes of value and timestamp */
  uint32 result = value_hash;
  result = (result << 1) | (result >> 31);
  result ^= time_hash;
  return result;
}

/*****************************************************************************/
