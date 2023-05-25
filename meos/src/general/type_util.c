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
 * @brief General utility functions for temporal types.
 */

#include "general/type_util.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
#include <utils/timestamp.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h"
#include "general/pg_types.h"
#include "general/doublen.h"
#include "general/type_parser.h"
#include "point/pgis_call.h"
#include "point/tpoint_spatialfuncs.h"
#if NPOINT
  #include "npoint/tnpoint_static.h"
  #include "npoint/tnpoint_parser.h"
#endif

/* To avoid including varlena.h */
extern int varstr_cmp(const char *arg1, int len1, const char *arg2, int len2, Oid collid);

/*****************************************************************************
 * Comparison functions on datums
 *****************************************************************************/

/* Version of the functions where the types of both arguments is equal */

/**
 * @brief Return true if the first value is less than the second one
 */
int
datum_cmp(Datum l, Datum r, meosType type)
{
  return datum_cmp2(l, r, type, type);
}

/**
 * @brief Return true if the values are equal
 */
bool
datum_eq(Datum l, Datum r, meosType type)
{
  return datum_eq2(l, r, type, type);
}

/**
 * @brief Return true if the values are different
 */
bool
datum_ne(Datum l, Datum r, meosType type)
{
  return ! datum_eq2(l, r, type, type);
}

/**
 * @brief Return true if the first value is less than the second one
 */
bool
datum_lt(Datum l, Datum r, meosType type)
{
  return datum_lt2(l, r, type, type);
}

/**
 * @brief Return true if the first value is less than or equal to the second one
 */
bool
datum_le(Datum l, Datum r, meosType type)
{
  return datum_eq2(l, r, type, type) || datum_lt2(l, r, type, type);
}

/**
 * @brief Return true if the first value is greater than the second one
 */
bool
datum_gt(Datum l, Datum r, meosType type)
{
  return datum_lt2(r, l, type, type);
}

/**
 * @brief Return true if the first value is greater than or equal to the second one
 */
bool
datum_ge(Datum l, Datum r, meosType type)
{
  return datum_eq2(l, r, type, type) || datum_lt2(r, l, type, type);
}

/*****************************************************************************/

/*
 * Version of the functions where the types of both arguments may be different
 * but compatible, e.g., integer and float
 */

/**
 * @brief Return true if the first value is less than the second one
 */
int
datum_cmp2(Datum l, Datum r, meosType typel, meosType typer)
{
  assert(meos_basetype(typel));
  if (typel != typer)
    assert(meos_basetype(typer));
  if (typel == T_TIMESTAMPTZ && typer == T_TIMESTAMPTZ)
    return timestamptz_cmp_internal(DatumGetTimestampTz(l),
      DatumGetTimestampTz(r));
  if (typel == T_BOOL && typer == T_BOOL)
    return (DatumGetBool(l) < DatumGetBool(r)) ? -1 :
      ((DatumGetBool(l) > DatumGetBool(r)) ? 1 : 0);
  if (typel == T_INT4 && typer == T_INT4)
    return (DatumGetInt32(l) < DatumGetInt32(r)) ? -1 :
      ((DatumGetInt32(l) > DatumGetInt32(r)) ? 1 : 0);
  if (typel == T_INT8 && typer == T_INT8)
    return (DatumGetInt64(l) < DatumGetInt64(r)) ? -1 :
      ((DatumGetInt64(l) > DatumGetInt64(r)) ? 1 : 0);
  if (typel == T_FLOAT8 && typer == T_FLOAT8)
    return float8_cmp_internal(DatumGetFloat8(l), DatumGetFloat8(r));
  if (typel == T_INT4 && typer == T_FLOAT8)
    return float8_cmp_internal((double) DatumGetInt32(l), DatumGetFloat8(r));
  if (typel == T_FLOAT8 && typer == T_INT4)
    return float8_cmp_internal(DatumGetFloat8(l), (double) DatumGetInt32(r));
  if (typel == T_TEXT && typer == T_TEXT)
    return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID);
#if 0 /* not used */
  if (typel == T_DOUBLE2 && typel == typer)
    return double2_cmp(DatumGetDouble2P(l), DatumGetDouble2P(r));
  if (typel == T_DOUBLE3 && typel == typer)
    return double3_cmp(DatumGetDouble3P(l), DatumGetDouble3P(r));
  if (typel == T_DOUBLE4 && typel == typer)
    return double4_cmp(DatumGetDouble4P(l), DatumGetDouble4P(r));
#endif /* not used */
  if ((typel == T_GEOMETRY || typel == T_GEOGRAPHY) && typel == typer)
    return gserialized_cmp(DatumGetGserializedP(l), DatumGetGserializedP(r));
#if NPOINT
  if (typel == T_NPOINT && typel == typer)
    return npoint_cmp(DatumGetNpointP(l), DatumGetNpointP(r));
#endif
  elog(ERROR, "unknown compare function for base type: %d", typel);
  return 0; /* make compiler quiet */
}

/**
 * @brief Return true if the values are equal even if their type is not the same
 * @note This function should be faster than the function datum_cmp2
 */
bool
datum_eq2(Datum l, Datum r, meosType typel, meosType typer)
{
  assert(meos_basetype(typel));
  if (typel != typer)
    assert(meos_basetype(typer));
  if ((typel == T_TIMESTAMPTZ && typer == T_TIMESTAMPTZ) ||
    (typel == T_BOOL && typer == T_BOOL) ||
    (typel == T_INT4 && typer == T_INT4) ||
    (typel == T_INT8 && typer == T_INT8))
    return l == r;
  if (typel == T_FLOAT8 && typer == T_FLOAT8)
    return float8_eq(DatumGetFloat8(l), DatumGetFloat8(r));
  if (typel == T_INT4 && typer == T_FLOAT8)
    return float8_eq((double) DatumGetInt32(l), DatumGetFloat8(r));
  if (typel == T_FLOAT8 && typer == T_INT4)
    return float8_eq(DatumGetFloat8(l), (double) DatumGetInt32(r));
  if (typel == T_TEXT && typer == T_TEXT)
    return text_cmp(DatumGetTextP(l), DatumGetTextP(r),
      DEFAULT_COLLATION_OID) == 0;
  if (typel == T_DOUBLE2 && typel == typer)
    return double2_eq(DatumGetDouble2P(l), DatumGetDouble2P(r));
  if (typel == T_DOUBLE3 && typel == typer)
    return double3_eq(DatumGetDouble3P(l), DatumGetDouble3P(r));
  if (typel == T_DOUBLE4 && typel == typer)
    return double4_eq(DatumGetDouble4P(l), DatumGetDouble4P(r));
  if ((typel == T_GEOMETRY || typel == T_GEOGRAPHY) && typel == typer)
    return (gserialized_cmp(DatumGetGserializedP(l),
      DatumGetGserializedP(r)) == 0);
#if NPOINT
  if (typel == T_NPOINT && typel == typer)
    return npoint_eq(DatumGetNpointP(l), DatumGetNpointP(r));
#endif
  elog(ERROR, "unknown datum_eq2 function for base type: %d", typel);
  return false; /* make compiler quiet */
}

/**
 * @brief Return true if the values are different
 */
bool
datum_ne2(Datum l, Datum r, meosType typel, meosType typer)
{
  return ! datum_eq2(l, r, typel, typer);
}

/**
 * @brief Return true if the first value is less than the second one
 */
bool
datum_lt2(Datum l, Datum r, meosType typel, meosType typer)
{
  return datum_cmp2(l, r, typel, typer) < 0;
}


/**
 * @brief Return true if the first value is less than or equal to the second one
 */
bool
datum_le2(Datum l, Datum r, meosType typel, meosType typer)
{
  return datum_cmp2(l, r, typel, typer) <= 0;
}

/**
 * @brief Return true if the first value is greater than the second one
 */
bool
datum_gt2(Datum l, Datum r, meosType typel, meosType typer)
{
  return datum_cmp2(l, r, typel, typer) > 0;
}

/**
 * @brief Return true if the first value is greater than or equal to the second one
 */
bool
datum_ge2(Datum l, Datum r, meosType typel, meosType typer)
{
  return datum_cmp2(l, r, typel, typer) >= 0;
}

/*****************************************************************************/

/**
 * @brief Return a Datum true if the values are equal
 */
Datum
datum2_eq2(Datum l, Datum r, meosType typel, meosType typer)
{
  return BoolGetDatum(datum_eq2(l, r, typel, typer));
}

/**
 * @brief Return a Datum true if the values are different
 */
Datum
datum2_ne2(Datum l, Datum r, meosType typel, meosType typer)
{
  return BoolGetDatum(datum_ne2(l, r, typel, typer));
}

/**
 * @brief Return a Datum true if the first value is less than the second one
 */
Datum
datum2_lt2(Datum l, Datum r, meosType typel, meosType typer)
{
  return BoolGetDatum(datum_lt2(l, r, typel, typer));
}

/**
 * @brief Return a Datum true if the first value is less than or equal to the second one
 */
Datum
datum2_le2(Datum l, Datum r, meosType typel, meosType typer)
{
  return BoolGetDatum(datum_le2(l, r, typel, typer));
}

/**
 * @brief Return a Datum true if the first value is greater than the second one
 */
Datum
datum2_gt2(Datum l, Datum r, meosType typel, meosType typer)
{
  return BoolGetDatum(datum_gt2(l, r, typel, typer));
}

/**
 * @brief Return a Datum true if the first value is greater than or equal to the second one
 */
Datum
datum2_ge2(Datum l, Datum r, meosType typel, meosType typer)
{
  return BoolGetDatum(datum_ge2(l, r, typel, typer));
}

/*****************************************************************************
 * Arithmetic functions on datums
 * N.B. The validity of the types must be done in the calling function.
 *****************************************************************************/

/**
 * @brief Return the addition of the two numbers
 */
Datum
datum_add(Datum l, Datum r, meosType typel, meosType typer)
{
  Datum result = 0;
  if (typel == T_INT4)
  {
    if (typer == T_INT4)
      result = Int32GetDatum(DatumGetInt32(l) + DatumGetInt32(r));
    else if (typer == T_INT8)
      result = Int64GetDatum((int64) DatumGetInt32(l) + DatumGetInt64(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum(DatumGetInt32(l) + DatumGetFloat8(r));
  }
  else if (typel == T_INT8)
  {
    if (typer == T_INT4)
      result = Int64GetDatum(DatumGetInt64(l) + DatumGetInt32(r));
    else if (typer == T_INT8)
      result = Int64GetDatum(DatumGetInt64(l) + DatumGetInt64(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum(DatumGetInt32(l) + DatumGetFloat8(r));
  }
  else /* typel == T_FLOAT8 */
  {
    if (typer == T_INT4)
      result = Float8GetDatum(DatumGetFloat8(l) + (double) DatumGetInt32(r));
    else if (typer == T_INT8)
      result = Float8GetDatum(DatumGetFloat8(l) + (double) DatumGetInt64(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum(DatumGetFloat8(l) + DatumGetFloat8(r));
  }
  return result;
}

/**
 * @brief Return the subtraction of the two numbers
 */
Datum
datum_sub(Datum l, Datum r, meosType typel, meosType typer)
{
  Datum result = 0;
  if (typel == T_INT4)
  {
    if (typer == T_INT4)
      result = Int32GetDatum(DatumGetInt32(l) - DatumGetInt32(r));
    else if (typer == T_INT8)
      result = Int64GetDatum(DatumGetInt32(l) - DatumGetInt64(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum((double) DatumGetInt32(l) - DatumGetFloat8(r));
  }
  else if (typel == T_INT8)
  {
    if (typer == T_INT4)
      result = Int64GetDatum(DatumGetInt64(l) - (int64) DatumGetInt32(r));
    else if (typer == T_INT8)
      result = Int64GetDatum(DatumGetInt64(l) - DatumGetInt64(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum((double) DatumGetInt32(l) - DatumGetFloat8(r));
  }
  else /* typel == T_FLOAT8 */
  {
    if (typer == T_INT4)
      result = Float8GetDatum(DatumGetFloat8(l) - (double) DatumGetInt32(r));
    else if (typer == T_INT8)
      result = Float8GetDatum(DatumGetFloat8(l) - (double) DatumGetInt64(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum(DatumGetFloat8(l) - DatumGetFloat8(r));
  }
  return result;
}

/**
 * @brief Return the multiplication of the two numbers
 */
Datum
datum_mult(Datum l, Datum r, meosType typel, meosType typer)
{
  Datum result = 0;
  if (typel == T_INT4)
  {
    if (typer == T_INT4)
      result = Int32GetDatum(DatumGetInt32(l) * DatumGetInt32(r));
    else if (typer == T_INT8)
      result = Int64GetDatum((int64) DatumGetInt32(l) * DatumGetInt64(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum((double) DatumGetInt32(l) * DatumGetFloat8(r));
  }
  else if (typel == T_INT8)
  {
    if (typer == T_INT4)
      result = Int64GetDatum(DatumGetInt64(l) * (int64) DatumGetInt32(r));
    else if (typer == T_INT8)
      result = Int64GetDatum(DatumGetInt64(l) * DatumGetInt64(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum((double) DatumGetInt64(l) * DatumGetFloat8(r));
  }
  else /* typel == T_FLOAT8 */
  {
    if (typer == T_INT4)
      result = Float8GetDatum(DatumGetFloat8(l) * (double) DatumGetInt32(r));
    else if (typer == T_INT8)
      result = Float8GetDatum(DatumGetFloat8(l) * (double) DatumGetInt64(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum(DatumGetFloat8(l) * DatumGetFloat8(r));
  }
  return result;
}

/**
 * @brief Return the division of the two numbers
 */
Datum
datum_div(Datum l, Datum r, meosType typel, meosType typer)
{
  Datum result;
  if (typel == T_INT4)
  {
    if (typer == T_INT4)
      result = Int32GetDatum(DatumGetInt32(l) / DatumGetInt32(r));
    else if (typer == T_INT8)
      result = Int64GetDatum((int64) DatumGetInt32(l) / DatumGetInt64(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum((double) DatumGetInt32(l) / DatumGetFloat8(r));
  }
  else if (typel == T_INT8)
  {
    if (typer == T_INT4)
      result = Int64GetDatum(DatumGetInt64(l) / (int64) DatumGetInt32(r));
    else if (typer == T_INT8)
      result = Int64GetDatum(DatumGetInt64(l) / DatumGetInt64(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum((double) DatumGetInt64(l) / DatumGetFloat8(r));
  }
  else /* typel == T_FLOAT8 */
  {
    if (typer == T_INT4)
      result = Float8GetDatum(DatumGetFloat8(l) / (double) DatumGetInt32(r));
    else if (typer == T_INT8)
      result = Float8GetDatum(DatumGetFloat8(l) / (double) DatumGetInt64(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum(DatumGetFloat8(l) / DatumGetFloat8(r));
  }
  return result;
}

/*****************************************************************************
 * Hash functions on datums
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_typefuncs
 * @brief Return the 32-bit hash of a value.
 */
uint32
datum_hash(Datum d, meosType type)
{
  assert(meos_basetype(type));
  if (type == T_TIMESTAMPTZ)
    return pg_hashint8(TimestampTzGetDatum(d));
  else if (type == T_BOOL)
    return hash_bytes_uint32((int32) DatumGetBool(d));
  else if (type == T_INT4)
    return hash_bytes_uint32(DatumGetInt32(d));
  else if (type == T_INT8)
    return pg_hashint8(DatumGetInt64(d));
  else if (type == T_FLOAT8)
    return pg_hashfloat8(DatumGetFloat8(d));
  else if (type == T_TEXT)
    return pg_hashtext(DatumGetTextP(d));
  else if (geo_basetype(type))
    return gserialized_hash(DatumGetGserializedP(d));
#if NPOINT
  else if (type == T_NPOINT)
    return npoint_hash(DatumGetNpointP(d));
#endif
  elog(ERROR, "unknown hash function for base type: %d", type);
  return (uint32) 0; /* make compiler quiet */
}

/**
 * @ingroup libmeos_internal_typefuncs
 * @brief Return the 64-bit hash of a value using a seed.
 */
uint64
datum_hash_extended(Datum d, meosType type, uint64 seed)
{
  assert(meos_basetype(type));
  if (type == T_TIMESTAMPTZ)
    return pg_hashint8extended(DatumGetTimestampTz(d), seed);
  else if (type == T_BOOL)
    return hash_bytes_uint32_extended((int32) DatumGetBool(d), seed);
  else if (type == T_INT4)
    return hash_bytes_uint32_extended(DatumGetInt32(d), seed);
  else if (type == T_INT8)
    return pg_hashint8extended(DatumGetInt64(d), seed);
  else if (type == T_FLOAT8)
    return pg_hashfloat8extended(DatumGetFloat8(d), seed);
  else if (type == T_TEXT)
    return pg_hashtextextended(DatumGetTextP(d), seed);
  // PostGIS currently does not provide an extended hash function
  // else if (geo_basetype(type))
    // return gserialized_hash_extended(DatumGetGserializedP(d), seed);
#if NPOINT
  else if (type == T_NPOINT)
    return npoint_hash_extended(DatumGetNpointP(d), seed);
#endif
  elog(ERROR, "unknown extended hash function for base type: %d", type);
  return (uint64) 0; /* make compiler quiet */
}

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

/**
 * @brief Copy a Datum if it is passed by reference
 */
Datum
datum_copy(Datum value, meosType basetype)
{
  /* For types passed by value */
  if (basetype_byvalue(basetype))
    return value;
  /* For types passed by reference */
  int typlen = basetype_length(basetype);
  size_t value_size = (typlen != -1) ? (size_t) typlen : VARSIZE(value);
  void *result = palloc(value_size);
  memcpy(result, DatumGetPointer(value), value_size);
  return PointerGetDatum(result);
}

/**
 * @brief Convert a number to a double
 */
double
datum_double(Datum d, meosType basetype)
{
  assert(tnumber_basetype(basetype));
  if (basetype == T_INT4)
    return (double) DatumGetInt32(d);
  if (basetype == T_INT8)
    return (double) DatumGetInt64(d);
  else /* basetype == T_FLOAT8 */
    return DatumGetFloat8(d);
}

/**
 * @brief Convert a double to a datum
 */
Datum
double_datum(double d, meosType basetype)
{
  assert(tnumber_basetype(basetype));
  if (basetype == T_INT4)
    return Int32GetDatum((int32) d);
  if (basetype == T_INT8)
    return Int64GetDatum((int64) d);
  else /* basetype == T_FLOAT8 */
    return Float8GetDatum(d);
}

/*****************************************************************************
 * Sort functions
 *****************************************************************************/

/**
 * @brief Comparator function for datums
 */
static int
datum_sort_cmp(const Datum *l, const Datum *r, const meosType *type)
{
  Datum x = *l;
  Datum y = *r;
  meosType t = *type;
  return datum_cmp(x, y, t);
}

/**
 * @brief Comparator function for timestamps
 */
static int
timestamp_sort_cmp(const TimestampTz *l, const TimestampTz *r)
{
  TimestampTz x = *l;
  TimestampTz y = *r;
  return timestamptz_cmp_internal(x, y);
}

/**
 * @brief Comparator function for temporal instants
 */
static int
tinstarr_sort_cmp(const TInstant **l, const TInstant **r)
{
  return timestamptz_cmp_internal((*l)->t, (*r)->t);
}

/**
 * @brief Comparator function for temporal sequences
 */
static int
tseqarr_sort_cmp(TSequence **l, TSequence **r)
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
datumarr_sort(Datum *values, int count, meosType type)
{
  qsort_arg(values, (size_t) count, sizeof(Datum),
    (qsort_arg_comparator) &datum_sort_cmp, &type);
}

/**
 * @brief Sort function for timestamps
 */
void
timestamparr_sort(TimestampTz *times, int count)
{
  qsort(times, (size_t) count, sizeof(TimestampTz),
    (qsort_comparator) &timestamp_sort_cmp);
}

#if 0 /* Not used */
/**
 * @brief Sort function for double2
 */
void
double2arr_sort(double2 *doubles, int count)
{
  qsort(doubles, count, sizeof(double2), (qsort_comparator) &double2_cmp);
}

/**
 * @brief Sort function for double3
 */
void
double3arr_sort(double3 *triples, int count)
{
  qsort(triples, count, sizeof(double3), (qsort_comparator) &double3_cmp);
}
#endif

/**
 * @brief Sort function for spans
 */
void
spanarr_sort(Span *spans, int count)
{
  qsort(spans, (size_t) count, sizeof(Span),
    (qsort_comparator) &span_cmp);
}

/**
 * @brief Sort function for temporal instants
 */
void
tinstarr_sort(TInstant **instants, int count)
{
  qsort(instants, (size_t) count, sizeof(TInstant *),
    (qsort_comparator) &tinstarr_sort_cmp);
}

/**
 * @brief Sort function for temporal sequences
 */
void
tseqarr_sort(TSequence **sequences, int count)
{
  qsort(sequences, (size_t) count, sizeof(TSequence *),
    (qsort_comparator) &tseqarr_sort_cmp);
}

/*****************************************************************************
 * Remove duplicate functions
 * These functions assume that the array has been sorted before
 *****************************************************************************/

/**
 * @brief Remove duplicates from an array of datums
 */
int
datumarr_remove_duplicates(Datum *values, int count, meosType type)
{
  assert (count > 0);
  int newcount = 0;
  for (int i = 1; i < count; i++)
    if (datum_ne(values[newcount], values[i], type))
      values[++ newcount] = values[i];
  return newcount + 1;
}

/**
 * @brief Remove duplicates from an array of timestamps
 */
int
timestamparr_remove_duplicates(TimestampTz *values, int count)
{
  assert (count > 0);
  int newcount = 0;
  for (int i = 1; i < count; i++)
    if (values[newcount] != values[i])
      values[++ newcount] = values[i];
  return newcount + 1;
}

/**
 * @brief Remove duplicates from an array of temporal instants
 */
int
tinstarr_remove_duplicates(const TInstant **instants, int count)
{
  assert(count != 0);
  int newcount = 0;
  for (int i = 1; i < count; i++)
    if (! tinstant_eq(instants[newcount], instants[i]))
      instants[++ newcount] = instants[i];
  return newcount + 1;
}

/*****************************************************************************
 * Text and binary string functions
 *****************************************************************************/

/**
 * @brief Convert a C binary string into a bytea
 */
bytea *
bstring2bytea(const uint8_t *wkb, size_t size)
{
  bytea *result = palloc(size + VARHDRSZ);
  memcpy(VARDATA(result), wkb, size);
  SET_VARSIZE(result, size + VARHDRSZ);
  return result;
}

/**
 * @ingroup libmeos_pg_types
 * @brief Convert a C string into a text value
 * @note We don't include <utils/builtins.h> to avoid collisions with json-c/json.h
 * @note Function taken from PostGIS file lwgeom_in_geojson.c
 */
text *
cstring2text(const char *cstring)
{
  size_t len = strlen(cstring);
  text *result = palloc(len + VARHDRSZ);
  SET_VARSIZE(result, len + VARHDRSZ);
  memcpy(VARDATA(result), cstring, len);
  return result;
}

/**
 * @ingroup libmeos_pg_types
 * @brief Convert a text value into a C string
 * @note We don't include <utils/builtins.h> to avoid collisions with json-c/json.h
 * @note Function taken from PostGIS file lwgeom_in_geojson.c
 */
char *
text2cstring(const text *textptr)
{
  size_t size = VARSIZE_ANY_EXHDR(textptr);
  char *str = palloc(size + 1);
  memcpy(str, VARDATA(textptr), size);
  str[size] = '\0';
  return str;
}

#if MEOS
/* Simplified version of the function in varlena.c where LC_COLLATE is C */
int
varstr_cmp(const char *arg1, int len1, const char *arg2, int len2,
  Oid collid __attribute__((unused)))
{
  int result = memcmp(arg1, arg2, Min(len1, len2));
  if ((result == 0) && (len1 != len2))
    result = (len1 < len2) ? -1 : 1;
  return result;
}
#endif /* MEOS */

/**
 * @brief Comparison function for text values
 *
 * @note Function copied from PostgreSQL since it is not exported
 */
int
text_cmp(text *arg1, text *arg2, Oid collid __attribute__((unused)))
{
  char *a1p, *a2p;
  int len1, len2;

  a1p = VARDATA_ANY(arg1);
  a2p = VARDATA_ANY(arg2);

  len1 = (int) VARSIZE_ANY_EXHDR(arg1);
  len2 = (int) VARSIZE_ANY_EXHDR(arg2);

  return varstr_cmp(a1p, len1, a2p, len2, collid);
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
  for (int i = 0; i < count; i++)
  {
    if (array[i])
      pfree(array[i]);
  }
  pfree(array);
  return;
}

/**
 * @brief Return the string resulting from assembling the array of strings.
 * The function frees the memory of the input strings after finishing.
 * @param[in] strings Array of strings to ouput
 * @param[in] count Number of elements in the input array
 * @param[in] outlen Total length of the elements and the additional ','
 * @param[in] prefix Prefix to add to the string (e.g., for interpolation)
 * @param[in] open, close Starting/ending character (e.g., '{' and '}')
 * @param[in] quotes True when elements should be enclosed into quotes
 * @param[in] spaces True when elements should be separated by spaces
 */
char *
stringarr_to_string(char **strings, int count, size_t outlen, char *prefix,
  char open, char close, bool quotes, bool spaces)
{
  size_t size = strlen(prefix) + outlen + 3;
  if (quotes)
    size += count * 4;
  if (spaces)
    size += count;
  char *result = palloc(size);
  size_t pos = 0;
  strcpy(result, prefix);
  pos += strlen(prefix);
  result[pos++] = open;
  for (int i = 0; i < count; i++)
  {
    if (quotes)
      result[pos++] = '"';
    strcpy(result + pos, strings[i]);
    pos += strlen(strings[i]);
    if (quotes)
      result[pos++] = '"';
    result[pos++] = ',';
    if (spaces)
      result[pos++] = ' ';
    pfree(strings[i]);
  }
  if (spaces)
  {
    result[pos - 2] = close;
    result[pos - 1] = '\0';
  }
  else
  {
    result[pos - 1] = close;
    result[pos] = '\0';
  }
  pfree(strings);
  return result;
}

/*****************************************************************************
 * Hypotenuse functions
 *****************************************************************************/

/**
 * @brief Determine the 3D hypotenuse.
 *
 * If required, x, y, and z are swapped to make x the larger number. The
 * traditional formula of x^2+y^2+z^2 is rearranged to factor x outside the
 * sqrt. This allows computation of the hypotenuse for significantly
 * larger values, and with a higher precision than when using the naive
 * formula. In particular, this cannot overflow unless the final result
 * would be out-of-range.
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

#if 0 /* not used */
/**
 * @brief Determine the 4D hypotenuse.
 * @see The function is a generalization of the 3D case in function hypot3d
 */
double
hypot4d(double x, double y, double z, double m)
{
  double yx;
  double zx;
  double mx;
  double temp;

  /* Handle INF and NaN properly */
  if (isinf(x) || isinf(y) || isinf(z) || isinf(m))
    return get_float8_infinity();

  if (isnan(x) || isnan(y) || isnan(z) || isnan(m))
    return get_float8_nan();

  /* Else, drop any minus signs */
  x = fabs(x);
  y = fabs(y);
  z = fabs(z);
  m = fabs(m);

  /* Swap x, y, z, and m if needed to make x the larger one */
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
  if (x < m)
  {
    temp = x;
    x = m;
    m = temp;
  }
  /*
   * If x is zero, the hypotenuse is computed with the 3D case.
   * This test saves a few cycles in such cases, but more importantly
   * it also protects against divide-by-zero errors, since now x >= y.
   */
  if (x == 0)
    return hypot3d(y, z, m);

  /* Determine the hypotenuse */
  yx = y / x;
  zx = z / x;
  mx = m / x;
  return x * sqrt(1.0 + (yx * yx) + (zx * zx) + (mx * mx));
}
#endif /* not used */

/*****************************************************************************
 * Input/output PostgreSQL functions
 *****************************************************************************/

/**
 * @brief Call input function of the base type
 */
Datum
#if NPOINT
basetype_in(const char *str, meosType basetype, bool end)
#else
basetype_in(const char *str, meosType basetype, bool end __attribute__((unused)))
#endif
{
  assert(meos_basetype(basetype));
  switch (basetype)
  {
    case T_TIMESTAMPTZ:
      return TimestampTzGetDatum(pg_timestamptz_in(str, -1));
    case T_BOOL:
      return BoolGetDatum(bool_in(str));
    case T_INT4:
      return Int32GetDatum(int4_in(str));
    case T_INT8:
      return Int64GetDatum(int8_in(str));
    case T_FLOAT8:
      return Float8GetDatum(float8_in(str, "double precision", str));
    case T_TEXT:
      return PointerGetDatum(cstring2text(str));
    case T_GEOMETRY:
      return PointerGetDatum(gserialized_in((char *) str, -1));
    case T_GEOGRAPHY:
      return PointerGetDatum(gserialized_geog_in((char *) str, -1));
#if NPOINT
    case T_NPOINT:
      return PointerGetDatum(npoint_parse(&str, end));
#endif
    default: /* Error! */
      elog(ERROR, "Unknown base type: %d", basetype);
      return Int32GetDatum(0); /* make compiler quiet */
  }
}

/**
 * @brief Call output function of the base type
 */
char *
basetype_out(Datum value, meosType basetype, int maxdd)
{
  assert(meos_basetype(basetype));
  switch (basetype)
  {
    case T_TIMESTAMPTZ:
      return pg_timestamptz_out(DatumGetTimestampTz(value));
    case T_BOOL:
      return bool_out(DatumGetBool(value));
    case T_INT4:
      return int4_out(DatumGetInt32(value));
    case T_INT8:
      return int8_out(DatumGetInt64(value));
    case T_FLOAT8:
      return float8_out(DatumGetFloat8(value), maxdd);
    case T_TEXT:
    {
      char *str = text2cstring(DatumGetTextP(value));
      char *result = palloc(strlen(str) + 4);
      sprintf(result, "\"%s\"", str);
      pfree(str);
      return result;
    }
#if DEBUG_BUILD
    case T_DOUBLE2:
      return double2_out(DatumGetDouble2P(value), maxdd);
    case T_DOUBLE3:
      return double3_out(DatumGetDouble3P(value), maxdd);
    case T_DOUBLE4:
      return double4_out(DatumGetDouble4P(value), maxdd);
#endif
    case T_GEOMETRY:
      return gserialized_out(DatumGetGserializedP(value));
    case T_GEOGRAPHY:
      return gserialized_geog_out(DatumGetGserializedP(value));
#if NPOINT
    case T_NPOINT:
      return npoint_out(DatumGetNpointP(value), maxdd);
#endif
    default: /* Error! */
      elog(ERROR, "Unknown base type: %d", basetype);
      return NULL; /* make compiler quiet */
  }
}

/*****************************************************************************/
