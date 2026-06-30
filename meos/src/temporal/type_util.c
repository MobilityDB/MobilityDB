/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief General utility functions for temporal types
 */

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
#include <utils/timestamp.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
#include "utils/varlena.h"
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/span.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#if CBUFFER
  #include <meos_cbuffer.h>
  #include "cbuffer/cbuffer.h"
#endif
#if NPOINT
  // #include <meos_npoint.h>
  #include "npoint/tnpoint.h"
#endif
#if POSE
  #include "pose/pose.h"
#endif
#if RGEO
  #include "rgeo/trgeo.h"
#endif

#include <utils/jsonb.h>
#include <utils/numeric.h>
#include <pgtypes.h>
/* Function defined in formatting.c */
extern bool scanner_isspace(char ch);

#if JSON
  #include <utils/jsonb.h>
  #include "json/tjsonb.h"
#endif /* JSON */

/*****************************************************************************
 * Comparison functions on datums
 *****************************************************************************/

/**
 * @ingroup meos_base_int
 * @brief Return -1, 0, or 1 depending on whether the first value is less than, 
 * equal to, or greater than the second one
 */
int
int32_cmp(int32 l, int32 r)
{
  return (l < r) ? -1 : ((l > r) ? 1 : 0);
}

/**
 * @ingroup meos_base_int
 * @brief Return -1, 0, or 1 depending on whether the first value is less than, 
 * equal to, or greater than the second one
 */
int
int64_cmp(int64 l, int64 r)
{
  return (l < r) ? -1 : ((l > r) ? 1 : 0);
}

/**
 * @brief Return -1, 0, or 1 depending on whether the first value is less than, 
 * equal to, or greater than the second one
 */
int
datum_cmp(Datum l, Datum r, MeosType type)
{
  assert(meos_basetype(type));
  switch (type)
  {
    case T_TIMESTAMPTZ:
      return timestamptz_cmp_internal(DatumGetTimestampTz(l),
        DatumGetTimestampTz(r));
    case T_DATE:
      return (DatumGetDateADT(l) < DatumGetDateADT(r)) ? -1 :
        ((DatumGetDateADT(l) > DatumGetDateADT(r)) ? 1 : 0);
    case T_BOOL:
      return (DatumGetBool(l) < DatumGetBool(r)) ? -1 :
        ((DatumGetBool(l) > DatumGetBool(r)) ? 1 : 0);
    case T_INT4:
      return (DatumGetInt32(l) < DatumGetInt32(r)) ? -1 :
        ((DatumGetInt32(l) > DatumGetInt32(r)) ? 1 : 0);
#if H3
    case T_H3INDEX:
#endif
#if QUADBIN
    case T_QUADBIN:
#endif
    case T_INT8:
      return (DatumGetInt64(l) < DatumGetInt64(r)) ? -1 :
        ((DatumGetInt64(l) > DatumGetInt64(r)) ? 1 : 0);
    case T_FLOAT8:
      return pg_float8_cmp(DatumGetFloat8(l), DatumGetFloat8(r));
    case T_TEXT:
      return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID);
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      return gserialized_cmp(DatumGetGserializedP(l), DatumGetGserializedP(r));
#if CBUFFER
    case T_CBUFFER:
      return cbuffer_cmp(DatumGetCbufferP(l), DatumGetCbufferP(r));
#endif
#if JSON
    case T_JSONB:
      return pg_jsonb_cmp((Jsonb *) l, (Jsonb *) r);
#endif /* JSON */
#if NPOINT
    case T_NPOINT:
      return npoint_cmp(DatumGetNpointP(l), DatumGetNpointP(r));
#endif
#if POSE || RGEO
    case T_POSE:
      return pose_cmp(DatumGetPoseP(l), DatumGetPoseP(r));
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown compare function for type: %s", meostype_name(type));
    return INT_MAX;
  }
}

/**
 * @brief Return true if the first value is less than the second one
 */
bool
datum_lt(Datum l, Datum r, MeosType type)
{
  return datum_cmp(l, r, type) < 0;
}

/**
 * @brief Return true if the first value is less than or equal to the second
 * one
 */
bool
datum_le(Datum l, Datum r, MeosType type)
{
  return datum_cmp(l, r, type) <= 0;
}

/**
 * @brief Return true if the first value is greater than the second one
 */
bool
datum_gt(Datum l, Datum r, MeosType type)
{
  return datum_cmp(l, r, type) > 0;
}

/**
 * @brief Return true if the first value is greater than or equal to the second
 * one
 */
bool
datum_ge(Datum l, Datum r, MeosType type)
{
  return datum_cmp(l, r, type) >= 0;
}

/**
 * @brief Return true if the values are equal
 * @note This function should be faster than the function #datum_cmp()
 */
bool
datum_eq(Datum l, Datum r, MeosType type)
{
  assert(meos_basetype(type));
  switch (type)
  {
    /* For types passed by value */
    case T_TIMESTAMPTZ:
    case T_DATE:
    case T_BOOL:
    case T_INT4:
#if H3
    case T_H3INDEX:
#endif
#if QUADBIN
    case T_QUADBIN:
#endif
    case T_INT8:
      return l == r;
    case T_FLOAT8:
      return float8_eq(DatumGetFloat8(l), DatumGetFloat8(r));
    case T_TEXT:
      return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) == 0;
    case T_DOUBLE2:
      return double2_eq(DatumGetDouble2P(l), DatumGetDouble2P(r));
    case T_DOUBLE3:
      return double3_eq(DatumGetDouble3P(l), DatumGetDouble3P(r));
    case T_DOUBLE4:
      return double4_eq(DatumGetDouble4P(l), DatumGetDouble4P(r));
    case T_GEOMETRY:
      return geo_equals(DatumGetGserializedP(l), DatumGetGserializedP(r));
    case T_GEOGRAPHY:
    {
      GSERIALIZED *gs1 = DatumGetGserializedP(l);
      GSERIALIZED *gs2 = DatumGetGserializedP(r);
      if (gserialized_get_type(gs1) == POINTTYPE &&
          gserialized_get_type(gs2) == POINTTYPE)
        return geopoint_same(gs1, gs2);
      else
        return geo_same(gs1, gs2);
    }
#if CBUFFER
    case T_CBUFFER:
      return cbuffer_eq(DatumGetCbufferP(l), DatumGetCbufferP(r));
#endif
#if JSON
    case T_JSONB:
      return pg_jsonb_eq(DatumGetJsonbP(l), DatumGetJsonbP(r));
#endif /* JSON */
#if NPOINT
    case T_NPOINT:
      return npoint_eq(DatumGetNpointP(l), DatumGetNpointP(r));
#endif
#if POSE || RGEO
    case T_POSE:
      return pose_eq(DatumGetPoseP(l), DatumGetPoseP(r));
#endif
    default: /* Error! */
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Unknown equality function for type: %s", meostype_name(type));
    return false;
  }
}

/**
 * @brief Return true if the values are different
 */
bool
datum_ne(Datum l, Datum r, MeosType type)
{
  return ! datum_eq(l, r, type);
}


/*****************************************************************************/

/**
 * @brief Return a Datum true if the values are equal
 */
Datum
datum2_eq(Datum l, Datum r, MeosType type)
{
  return BoolGetDatum(datum_eq(l, r, type));
}

/**
 * @brief Return a Datum true if the values are different
 */
Datum
datum2_ne(Datum l, Datum r, MeosType type)
{
  return BoolGetDatum(datum_ne(l, r, type));
}

/**
 * @brief Return a Datum true if the first value is less than the second one
 */
Datum
datum2_lt(Datum l, Datum r, MeosType type)
{
  return BoolGetDatum(datum_lt(l, r, type));
}

/**
 * @brief Return a Datum true if the first value is less than or equal to the
 * second one
 */
Datum
datum2_le(Datum l, Datum r, MeosType type)
{
  return BoolGetDatum(datum_le(l, r, type));
}

/**
 * @brief Return a Datum true if the first value is greater than the second one
 */
Datum
datum2_gt(Datum l, Datum r, MeosType type)
{
  return BoolGetDatum(datum_gt(l, r, type));
}

/**
 * @brief Return a Datum true if the first value is greater than or equal to
 * the second one
 */
Datum
datum2_ge(Datum l, Datum r, MeosType type)
{
  return BoolGetDatum(datum_ge(l, r, type));
}

/*****************************************************************************
 * Arithmetic functions on datums
 * N.B. The validity of the types must be done in the calling function.
 *****************************************************************************/

/**
 * @brief Return the addition of the two numbers
 */
Datum
datum_add(Datum l, Datum r, MeosType type)
{
  switch (type)
  {
    case T_INT4:
      return Int32GetDatum(DatumGetInt32(l) + DatumGetInt32(r));
#if H3
    case T_H3INDEX:
#endif
#if QUADBIN
    case T_QUADBIN:
#endif
    case T_INT8:
      return Int64GetDatum(DatumGetInt64(l) + DatumGetInt64(r));
    case T_FLOAT8:
      return Float8GetDatum(DatumGetFloat8(l) + DatumGetFloat8(r));
    case T_DATE:
      /* For dates we ALWAYS add integers */
      return DateADTGetDatum(DatumGetDateADT(l) + DatumGetInt32(r));
    case T_TIMESTAMPTZ:
      /* For timestamps we ALWAYS add big integers */
      return TimestampTzGetDatum(DatumGetTimestampTz(l) + DatumGetInt64(r));
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown add function for type: %s", meostype_name(type));
      return 0;
  }
}

/**
 * @brief Return the subtraction of the two numbers
 */
Datum
datum_sub(Datum l, Datum r, MeosType type)
{
  switch (type)
  {
    case T_INT4:
      return Int32GetDatum(DatumGetInt32(l) - DatumGetInt32(r));
#if H3
    case T_H3INDEX:
#endif
#if QUADBIN
    case T_QUADBIN:
#endif
    case T_INT8:
      return Int64GetDatum(DatumGetInt64(l) - DatumGetInt64(r));
    case T_FLOAT8:
      return Float8GetDatum(DatumGetFloat8(l) - DatumGetFloat8(r));
    case T_DATE:
      /* For dates we ALWAYS substract integers */
      return DateADTGetDatum(DatumGetDateADT(l) - DatumGetInt32(r));
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown subtract function for type: %s", meostype_name(type));
      return 0;
  }
}

/**
 * @brief Return the multiplication of the two numbers
 */
Datum
datum_mul(Datum l, Datum r, MeosType type)
{
  switch (type)
  {
    case T_INT4:
      return Int32GetDatum(DatumGetInt32(l) * DatumGetInt32(r));
#if H3
    case T_H3INDEX:
#endif
#if QUADBIN
    case T_QUADBIN:
#endif
    case T_INT8:
      return Int64GetDatum(DatumGetInt64(l) * DatumGetInt64(r));
    case T_FLOAT8:
      return Float8GetDatum(DatumGetFloat8(l) * DatumGetFloat8(r));
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown multiplication function for type: %s", meostype_name(type));
      return 0;
  }
}

/**
 * @brief Return the division of the two numbers
 */
Datum
datum_div(Datum l, Datum r, MeosType type)
{
  switch (type)
  {
    case T_INT4:
      return Int32GetDatum(DatumGetInt32(l) / DatumGetInt32(r));
#if H3
    case T_H3INDEX:
#endif
#if QUADBIN
    case T_QUADBIN:
#endif
    case T_INT8:
      return Int64GetDatum(DatumGetInt64(l) / DatumGetInt64(r));
    case T_FLOAT8:
      return Float8GetDatum(DatumGetFloat8(l) / DatumGetFloat8(r));
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown division function for type: %s", meostype_name(type));
    return 0;
  }
}

/*****************************************************************************
 * Hash functions on datums
 *****************************************************************************/

/**
 * @brief Return the 32-bit hash of a value
 * @param[in] d Value
 * @param[in] type Type of the value
 * @return On error return @p INT_MAX
 */
uint32
datum_hash(Datum d, MeosType type)
{
  assert(meos_basetype(type));
  switch (type)
  {
    case T_TIMESTAMPTZ:
      return int64_hash(TimestampTzGetDatum(d));
    case T_DATE:
      return int32_hash(DateADTGetDatum(d));
    case T_BOOL:
      return char_hash((int32) DatumGetBool(d));
    case T_INT4:
      return int32_hash(DatumGetInt32(d));
#if H3
    case T_H3INDEX:
#endif
#if QUADBIN
    case T_QUADBIN:
#endif
    case T_INT8:
      return int64_hash(DatumGetInt64(d));
    case T_FLOAT8:
      return float8_hash(DatumGetFloat8(d));
    case T_TEXT:
      return text_hash(DatumGetTextP(d), DEFAULT_COLLATION_OID);
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      return gserialized_hash(DatumGetGserializedP(d));
#if CBUFFER
    case T_CBUFFER:
      return cbuffer_hash(DatumGetCbufferP(d));
#endif
#if JSON
    case T_JSONB:
      return pg_jsonb_hash(DatumGetJsonbP(d));
#endif /* JSON */
#if NPOINT
    case T_NPOINT:
      return npoint_hash(DatumGetNpointP(d));
#endif
#if POSE || RGEO
    case T_POSE:
      return pose_hash(DatumGetPoseP(d));
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown hash function for type: %s", meostype_name(type));
      return INT_MAX;
  }
}

/**
 * @brief Return the 64-bit hash of a value using a seed
 * @param[in] d Value
 * @param[in] type Type of the value
 * @param[in] seed Seed
 * @return On error return @p INT_MAX
 */
uint64
datum_hash_extended(Datum d, MeosType type, uint64 seed)
{
  assert(meos_basetype(type));
  switch (type)
  {
    case T_TIMESTAMPTZ:
      return int64_hash_extended(DatumGetTimestampTz(d), seed);
    case T_DATE:
      return int32_hash_extended((int32) DatumGetDateADT(d), seed);
    case T_BOOL:
      return char_hash_extended((int32) DatumGetBool(d), seed);
    case T_INT4:
      return int32_hash_extended(DatumGetInt32(d), seed);
#if H3
    case T_H3INDEX:
#endif
#if QUADBIN
    case T_QUADBIN:
#endif
    case T_INT8:
      return int64_hash_extended(DatumGetInt64(d), seed);
    case T_FLOAT8:
      return float8_hash_extended(DatumGetFloat8(d), seed);
    case T_TEXT:
      return text_hash_extended(DatumGetTextP(d), seed, DEFAULT_COLLATION_OID);
    // PostGIS currently does not provide an extended hash function
    // case T_GEOMETRY:
    // case T_GEOGRAPHY:
      // return gserialized_hash_extended(DatumGetGserializedP(d), seed);
#if NPOINT
    case T_NPOINT:
      return npoint_hash_extended(DatumGetNpointP(d), seed);
#endif
#if POSE || RGEO
    case T_POSE:
      return pose_hash_extended(DatumGetPoseP(d), seed);
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown extended hash function for type: %s", meostype_name(type));
      return INT_MAX;
  }
}

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

/**
 * @brief Copy a Datum if it is passed by reference
 */
Datum
datum_copy(Datum value, MeosType basetype)
{
  /* For types passed by value */
  if (basetype_byvalue(basetype))
    return value;
  /* For types passed by reference */
  int typlen = meostype_length(basetype);
  size_t value_size = (typlen != -1) ? (size_t) typlen :
    VARSIZE(DatumGetPointer(value));
  void *result = palloc(value_size);
  memcpy(result, DatumGetPointer(value), value_size);
  return PointerGetDatum(result);
}

/**
 * @brief Return a double from a Datum value
 * @return On error return @p DBL_MAX
 */
double
datum_double(Datum d, MeosType type)
{
  assert(numspan_basetype(type));
  switch (type)
  {
    case T_INT4:
      return (double) DatumGetInt32(d);
#if H3
    case T_H3INDEX:
#endif
#if QUADBIN
    case T_QUADBIN:
#endif
    case T_INT8:
      return (double) DatumGetInt64(d);
    case T_FLOAT8:
      return DatumGetFloat8(d);
    case T_DATE:
      return (double) DatumGetDateADT(d);
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown conversion to double function for type: %s",
        meostype_name(type));
      return DBL_MAX;
  }
}

/**
 * @brief Return a Datum of a given type from a double value
 */
Datum
double_datum(double d, MeosType type)
{
  assert(numspan_basetype(type));
  switch (type)
  {
    case T_INT4:
      return Int32GetDatum((int) d);
#if H3
    case T_H3INDEX:
#endif
#if QUADBIN
    case T_QUADBIN:
#endif
    case T_INT8:
      return Int64GetDatum((int64) d);
    case T_FLOAT8:
      return Float8GetDatum(d);
    case T_DATE:
      return DateADTGetDatum((DateADT) d);
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown conversion to Datum function for type: %s",
        meostype_name(type));
      return (Datum) 0;
  }
}

/*****************************************************************************
 * Sort functions
 *****************************************************************************/

/**
 * @brief Comparator function for datums
 */
static int
datum_sort_cmp(const Datum *l, const Datum *r, const MeosType *type)
{
  Datum x = *l;
  Datum y = *r;
  MeosType t = *type;
  return datum_cmp(x, y, t);
}

/**
 * @brief Comparator function for timestamptz values
 */
static int
timestamptz_sort_cmp(const TimestampTz *l, const TimestampTz *r)
{
  TimestampTz x = *l;
  TimestampTz y = *r;
  return timestamptz_cmp_internal(x, y);
}

/**
 * @brief Comparator function for temporal instants
 */
static int
tinstant_sort_cmp(TInstant **l, TInstant **r)
{
  return timestamptz_cmp_internal((*l)->t, (*r)->t);
}

/**
 * @brief Comparator function for temporal sequences
 */
static int
tsequence_sort_cmp(TSequence **l, TSequence **r)
{
  Span lp = (*l)->period;
  Span rp = (*r)->period;
  return span_cmp(&lp, &rp);
}

/*****************************************************************************/

/**
 * @brief Sort function for datums
 */
void
datumarr_sort(Datum *values, int count, MeosType type)
{
  qsort_arg(values, (size_t) count, sizeof(Datum),
    (qsort_arg_comparator) &datum_sort_cmp, &type);
  return;
}

/**
 * @brief Sort function for timestamptz values
 */
void
tstzarr_sort(TimestampTz *times, int count)
{
  qsort(times, (size_t) count, sizeof(TimestampTz),
    (qsort_comparator) &timestamptz_sort_cmp);
  return;
}

/**
 * @brief Sort function for spans
 */
void
spanarr_sort(Span *spans, int count)
{
  qsort(spans, (size_t) count, sizeof(Span),
    (qsort_comparator) &span_cmp);
  return;
}

/**
 * @brief Sort function for temporal instants
 */
void
tinstarr_sort(TInstant **instants, int count)
{
  qsort(instants, (size_t) count, sizeof(TInstant *),
    (qsort_comparator) &tinstant_sort_cmp);
  return;
}

/**
 * @brief Sort function for temporal sequences
 */
void
tseqarr_sort(TSequence **sequences, int count)
{
  qsort(sequences, (size_t) count, sizeof(TSequence *),
    (qsort_comparator) &tsequence_sort_cmp);
  return;
}

/*****************************************************************************
 * Remove duplicate functions
 * These functions assume that the array has been sorted before
 * These functions pack the distinct values at the begining
 *****************************************************************************/

/**
 * @brief Remove duplicates from an array of datums
 * @pre The array has been sorted before
 */
int
datumarr_remove_duplicates(Datum *values, int count, MeosType type)
{
  assert(values); assert(count > 0);
  int newcount = 0;
  for (int i = 1; i < count; i++)
    if (datum_ne(values[newcount], values[i], type))
      values[++ newcount] = values[i];
  return newcount + 1;
}

/**
 * @brief Remove duplicates from an array of timestamptz values
 * @pre The array has been sorted before
 */
int
tstzarr_remove_duplicates(TimestampTz *values, int count)
{
  assert(count > 0);
  int newcount = 0;
  for (int i = 1; i < count; i++)
    if (values[newcount] != values[i])
      values[++ newcount] = values[i];
  return newcount + 1;
}

/**
 * @brief Remove duplicates from an array of temporal instants
 * @pre The array has been sorted before
 */
int
tinstarr_remove_duplicates(TInstant **instants, int count)
{
  assert(count > 0);
  int newcount = 0;
  for (int i = 1; i < count; i++)
    if (! tinstant_eq(instants[newcount], instants[i]))
      instants[++ newcount] = instants[i];
  return newcount + 1;
}

/*****************************************************************************
 * Array functions
 *****************************************************************************/

/**
 * @brief Free a C array of pointers
 */
void
pfree_array(void **array, int count)
{
  assert(array); assert(count >= 0);
  for (int i = 0; i < count; i++)
  {
    if (array[i])
      pfree(array[i]);
  }
  pfree(array);
  return;
}

/**
 * @brief Return the string resulting from escaping the input string
 * @param[in] str String
 * @param[in] quotes True when elements should be enclosed into quotes
 * @param[out] result True when elements should be enclosed into quotes
 * @result True when the string was escaped, false otherwise
 * @note The function is derived from the PostgreSQL array_out() function
 */
bool
string_escape(const char *str, int quotes, char **result)
{
  /* Count total space needed (including any overhead such as escaping
     backslashes), and detect whether the string needs double quotes */
  /* In QUOTES mode the value is always enclosed in double quotes */
  bool needquotes = (quotes == QUOTES);
  const char *tmp;
  /* Size of the input string + '\0' */
  size_t size = strlen(str) + 1;
  /* As in the traditional PostgreSQL array output, in QUOTES_ESCAPE mode an
   * empty string and a value equal (case-insensitively) to "NULL" must be
   * quoted so that the output is unambiguous and round-trips through the
   * parser */
  if (quotes == QUOTES_ESCAPE &&
      (str[0] == '\0' || pg_strcasecmp(str, "NULL") == 0))
    needquotes = true;
  /* Count the backslashes needed to escape embedded double quotes and
   * backslashes (the escaping is performed in both quoting modes) and, in
   * QUOTES_ESCAPE mode, detect the characters that require the value to be
   * quoted */
  for (tmp = str; *tmp != '\0'; tmp++)
  {
    char ch = *tmp;
    if (ch == '"' || ch == '\\')
    {
      needquotes = true;
      size += 1;
    }
    else if (quotes == QUOTES_ESCAPE &&
        (ch == '{' || ch == '}' || ch == ',' || scanner_isspace(ch)))
      needquotes = true;
  }
  /* Return if no quotes are needed */
  if (! needquotes)
    return false;

  /* Count the pair of double quotes */
  size += 2;

  /* Construct the output string */
  *result = (char *) palloc0(size);
  char *p = *result;

  /* Add the prefix, if any */
  *p++ = '"';
  for (tmp = str; *tmp; tmp++)
  {
    char ch = *tmp;
    if (ch == '"' || ch == '\\')
      *p++ = '\\';
    *p++ = ch;
  }
  *p++ = '"';
  *p = '\0';
  return needquotes;
}

/**
 * @brief Return the unescaped value of a double-quoted string
 * @details This function is the exact inverse of function #string_escape: the
 * input must start with a double quote, every backslash is dropped and the
 * character following it is copied verbatim, and parsing stops at the first
 * unescaped double quote. It is used by all the input functions that read a
 * quoted base value (e.g., text, geometry) both for sets and for temporal
 * values, so that the input is symmetric with the output for every binding.
 * @param[in] str Input string, which must start with a double quote
 * @param[out] result Newly allocated unescaped string
 * @return Number of input characters consumed, including both double quotes,
 * or 0 on error (unterminated quoted string)
 * @note The function is derived from the PostgreSQL array input function
 */
size_t
string_unescape(const char *str, char **result)
{
  assert(str); assert(result); assert(str[0] == '"');
  /* Consume the opening double quote */
  const char *p = str + 1;
  /* The output is at most as long as the input (we only ever drop characters) */
  char *buf = palloc(strlen(str) + 1);
  size_t pos = 0;
  bool closed = false;
  while (*p != '\0')
  {
    if (*p == '\\')
    {
      /* Drop the backslash and copy the next character verbatim */
      p++;
      if (*p == '\0')
        break;
      buf[pos++] = *p++;
    }
    else if (*p == '"')
    {
      /* Unescaped double quote closes the value */
      p++;
      closed = true;
      break;
    }
    else
      buf[pos++] = *p++;
  }
  if (! closed)
  {
    pfree(buf);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Malformed quoted string: %s", str);
    return 0;
  }
  buf[pos] = '\0';
  *result = pstrdup(buf);
  pfree(buf);
  return (size_t) (p - str);
}

/**
 * @brief Return the string resulting from assembling an array of strings
 * @param[in] strings Array of strings to ouput
 * @param[in] count Number of elements in the input array
 * @param[in] prefix Prefix to add to the string (e.g., for interpolation)
 * @param[in] open, close Starting/ending character (e.g., '{' and '}')
 * @param[in] quotes True when elements should be enclosed into quotes
 * @param[in] spaces True when elements should be separated by spaces
 * @note The function frees the memory of the input strings after finishing.
 * @note The functin is derived from the PostgreSQL array_out() function
 */
char *
stringarr_to_string(char **strings, int count, char *prefix, char open,
  char close, int quotes, bool spaces)
{
  /* Count total space needed (including any overhead such as escaping
     backslashes), and detect whether each item needs double quotes */
  char **escaped = (char **) palloc0(sizeof(char *) * count);
  bool *needquotes = (bool *) palloc0(sizeof(bool) * count);
  /* Prefix size + opening and closing characters */
  size_t prefix_size = strlen(prefix);
  size_t size = prefix_size + 2;

  /* Iterate through the values */
  for (int i = 0; i < count; i++)
  {
    if (quotes == QUOTES)
      needquotes[i] = true;
    else if (quotes == QUOTES_ESCAPE)
      needquotes[i] = string_escape(strings[i], quotes, &escaped[i]);

    if (escaped[i])
      /* The escaped representation already includes the wrapping quotes */
      size += strlen(escaped[i]);
    else if (needquotes[i])
      /* Raw string + opening and closing quotes */
      size += strlen(strings[i]) + 2;
    else
      size += strlen(strings[i]);
    /* and the comma delimiter */
    size += 1;
  }
  /* The last element doesn't have a comma delimiter after it but that's OK,
   * that space is needed for the trailing '\0'.
   * Add in addition the spaces between elements if requested. */
  if (spaces)
    size += count;

  /* Construct the output string */
  char *result = (char *) palloc0(size);
  char *p = result;

  /* Add the prefix, if any */
  if (prefix_size)
  {
    for (char *tmp = prefix; *tmp; tmp++)
      *p++ = *tmp;
  }

  *p++ = open;
  for (int i = 0; i < count; i++)
  {
    if (escaped[i])
    {
      /* QUOTES_ESCAPE path: escaped[i] already includes wrapping quotes */
      strcpy(p, escaped[i]);
      p += strlen(p);
      pfree(escaped[i]);
    }
    else if (needquotes[i])
    {
      /* QUOTES path: wrap raw string in double quotes */
      *p++ = '"';
      strcpy(p, strings[i]);
      p += strlen(p);
      *p++ = '"';
    }
    else
    {
      strcpy(p, strings[i]);
      p += strlen(p);
    }
    if (i < count - 1)
    {
      *p++ = ',';
      if (spaces)
        *p++ = ' ';
    }
  }
  *p++ = close;
  *p = '\0';

  /* Free the input strings (the contract documented above), not only the
   * arrays -- otherwise every set/spanset/tsequence(set) _out leaks the
   * element strings in standalone MEOS (no memory-context reclaim). */
  for (int i = 0; i < count; i++)
    pfree(strings[i]);
  pfree(escaped); pfree(needquotes); pfree(strings);
  return result;
}

/*****************************************************************************
 * Hypotenuse functions
 *****************************************************************************/

/**
 * @brief Return the 3D hypotenuse
 * @details If required, @p x, @p y, and @p z are swapped to make @p x the
 * larger number. The traditional formula of `x^2+y^2+z^2` is rearranged
 * to factor @p x outside the @p sqrt function. This allows computation of the
 * hypotenuse for significantly larger values, and with a higher precision than
 * when using the naive formula. In particular, this cannot overflow unless the
 * final result would be out-of-range.
 * @code
 * sqrt( x^2 + y^2 + z^2 ) = sqrt( x^2( 1 + y^2/x^2 + z^2/x^2) )
 *                         = x * sqrt( 1 + y^2/x^2 + z^2/x^2)
 *                         = x * sqrt( 1 + y/x * y/x + z/x * z/x)
 * @endcode
 */
double
hypot3d(double x, double y, double z)
{
  double yx;
  double zx;
  double temp;

  /* Handle INF and NaN properly */
  if (isinf(x) || isinf(y) || isinf(z))
    return get_float8_infinity();

  if (isnan(x) || isnan(y) || isnan(z))
    return get_float8_nan();

  /* Else, drop any minus signs */
  x = fabs(x);
  y = fabs(y);
  z = fabs(z);

  /* Swap x, y and z if needed to make x the larger one */
  if (x < y)
  {
    temp = x;
    x = y;
    y = temp;
  }
  if (x < z)
  {
    temp = x;
    x = z;
    z = temp;
  }
  /*
   * If x is zero, the hypotenuse is computed with the 2D case.
   * This test saves a few cycles in such cases, but more importantly
   * it also protects against divide-by-zero errors, since now x >= y.
   */
  if (x == 0)
    return hypot(y, z);

  /* Determine the hypotenuse */
  yx = y / x;
  zx = z / x;
  return x * sqrt(1.0 + (yx * yx) + (zx * zx));
}

/*****************************************************************************/
