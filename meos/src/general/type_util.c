/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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

#include "general/type_util.h"

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
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/span.h"
#include "point/tpoint_spatialfuncs.h"
#if NPOINT
  #include "npoint/tnpoint_static.h"
  #include "npoint/tnpoint_parser.h"
#endif

/*****************************************************************************
 * Comparison functions on datums
 *****************************************************************************/

/**
 * @brief Return true if the first value is less than the second one
 */
int
datum_cmp(Datum l, Datum r, meosType type)
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
    case T_INT8:
      return (DatumGetInt64(l) < DatumGetInt64(r)) ? -1 :
        ((DatumGetInt64(l) > DatumGetInt64(r)) ? 1 : 0);
    case T_FLOAT8:
      return float8_cmp_internal(DatumGetFloat8(l), DatumGetFloat8(r));
    case T_TEXT:
      return text_cmp(DatumGetTextP(l), DatumGetTextP(r));
#if 0 /* not used */
    case T_DOUBLE2:
      return double2_cmp(DatumGetDouble2P(l), DatumGetDouble2P(r));
    case T_DOUBLE3:
      return double3_cmp(DatumGetDouble3P(l), DatumGetDouble3P(r));
    case T_DOUBLE4:
      return double4_cmp(DatumGetDouble4P(l), DatumGetDouble4P(r));
#endif /* not used */
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      return gserialized_cmp(DatumGetGserializedP(l), DatumGetGserializedP(r));
#if NPOINT
    case T_NPOINT:
      return npoint_cmp(DatumGetNpointP(l), DatumGetNpointP(r));
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown compare function for base type: %s", meostype_name(type));
    return INT_MAX;
  }
}

/**
 * @brief Return true if the first value is less than the second one
 */
bool
datum_lt(Datum l, Datum r, meosType type)
{
  return datum_cmp(l, r, type) < 0;
}

/**
 * @brief Return true if the first value is less than or equal to the second
 * one
 */
bool
datum_le(Datum l, Datum r, meosType type)
{
  return datum_cmp(l, r, type) <= 0;
}

/**
 * @brief Return true if the first value is greater than the second one
 */
bool
datum_gt(Datum l, Datum r, meosType type)
{
  return datum_cmp(l, r, type) > 0;
}

/**
 * @brief Return true if the first value is greater than or equal to the second
 * one
 */
bool
datum_ge(Datum l, Datum r, meosType type)
{
  return datum_cmp(l, r, type) >= 0;
}

/**
 * @brief Return true if the values are equal
 * @note This function should be faster than the function #datum_cmp()
 * @pre For geometry and geography types it is supposed that the type is a
 * point
 */
bool
datum_eq(Datum l, Datum r, meosType type)
{
  assert(meos_basetype(type));
  switch (type)
  {
    /* For types passed by value */
    case T_TIMESTAMPTZ:
    case T_DATE:
    case T_BOOL:
    case T_INT4:
    case T_INT8:
      return l == r;
    case T_FLOAT8:
      return float8_eq(DatumGetFloat8(l), DatumGetFloat8(r));
    case T_TEXT:
      return text_cmp(DatumGetTextP(l), DatumGetTextP(r)) == 0;
    case T_DOUBLE2:
      return double2_eq(DatumGetDouble2P(l), DatumGetDouble2P(r));
    case T_DOUBLE3:
      return double3_eq(DatumGetDouble3P(l), DatumGetDouble3P(r));
    case T_DOUBLE4:
      return double4_eq(DatumGetDouble4P(l), DatumGetDouble4P(r));
    case T_GEOMETRY:
      return datum_point_eq(l, r);
    case T_GEOGRAPHY:
      return datum_point_same(l, r);
#if NPOINT
    case T_NPOINT:
      return npoint_eq(DatumGetNpointP(l), DatumGetNpointP(r));
#endif
    default: /* Error! */
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Unknown equality function for base type: %s", meostype_name(type));
    return false;
  }
}

/**
 * @brief Return true if the values are different
 */
bool
datum_ne(Datum l, Datum r, meosType type)
{
  return ! datum_eq(l, r, type);
}


/*****************************************************************************/

/**
 * @brief Return a Datum true if the values are equal
 */
Datum
datum2_eq(Datum l, Datum r, meosType type)
{
  return BoolGetDatum(datum_eq(l, r, type));
}

/**
 * @brief Return a Datum true if the values are different
 */
Datum
datum2_ne(Datum l, Datum r, meosType type)
{
  return BoolGetDatum(datum_ne(l, r, type));
}

/**
 * @brief Return a Datum true if the first value is less than the second one
 */
Datum
datum2_lt(Datum l, Datum r, meosType type)
{
  return BoolGetDatum(datum_lt(l, r, type));
}

/**
 * @brief Return a Datum true if the first value is less than or equal to the
 * second one
 */
Datum
datum2_le(Datum l, Datum r, meosType type)
{
  return BoolGetDatum(datum_le(l, r, type));
}

/**
 * @brief Return a Datum true if the first value is greater than the second one
 */
Datum
datum2_gt(Datum l, Datum r, meosType type)
{
  return BoolGetDatum(datum_gt(l, r, type));
}

/**
 * @brief Return a Datum true if the first value is greater than or equal to
 * the second one
 */
Datum
datum2_ge(Datum l, Datum r, meosType type)
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
datum_add(Datum l, Datum r, meosType type)
{
  switch (type)
  {
    case T_INT4:
      return Int32GetDatum(DatumGetInt32(l) + DatumGetInt32(r));
    case T_INT8:
      return Int64GetDatum(DatumGetInt64(l) + DatumGetInt64(r));
    case T_FLOAT8:
      return Float8GetDatum(DatumGetFloat8(l) + DatumGetFloat8(r));
    case T_DATE:
      /* For dates we ALWAYS add integers */
      return DateADTGetDatum(DatumGetDateADT(l) + DatumGetInt32(r));
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown add function for base type: %s", meostype_name(type));
      return 0;
  }
}

/**
 * @brief Return the subtraction of the two numbers
 */
Datum
datum_sub(Datum l, Datum r, meosType type)
{
  switch (type)
  {
    case T_INT4:
      return Int32GetDatum(DatumGetInt32(l) - DatumGetInt32(r));
    case T_INT8:
      return Int64GetDatum(DatumGetInt64(l) - DatumGetInt64(r));
    case T_FLOAT8:
      return Float8GetDatum(DatumGetFloat8(l) - DatumGetFloat8(r));
    case T_DATE:
      /* For dates we ALWAYS substract integers */
      return DateADTGetDatum(DatumGetDateADT(l) - DatumGetInt32(r));
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown subtract function for base type: %s", meostype_name(type));
      return 0;
  }
}

/**
 * @brief Return the multiplication of the two numbers
 */
Datum
datum_mult(Datum l, Datum r, meosType type)
{
  switch (type)
  {
    case T_INT4:
      return Int32GetDatum(DatumGetInt32(l) * DatumGetInt32(r));
    case T_INT8:
      return Int64GetDatum(DatumGetInt64(l) * DatumGetInt64(r));
    case T_FLOAT8:
      return Float8GetDatum(DatumGetFloat8(l) * DatumGetFloat8(r));
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown multiplication function for base type: %s",
          meostype_name(type));
      return 0;
  }
}

/**
 * @brief Return the division of the two numbers
 */
Datum
datum_div(Datum l, Datum r, meosType type)
{
  switch (type)
  {
    case T_INT4:
      return Int32GetDatum(DatumGetInt32(l) / DatumGetInt32(r));
    case T_INT8:
      return Int64GetDatum(DatumGetInt64(l) / DatumGetInt64(r));
    case T_FLOAT8:
      return Float8GetDatum(DatumGetFloat8(l) / DatumGetFloat8(r));
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown division function for base type: %s", meostype_name(type));
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
datum_hash(Datum d, meosType type)
{
  assert(meos_basetype(type));
  switch (type)
  {
    case T_TIMESTAMPTZ:
      return pg_hashint8(TimestampTzGetDatum(d));
    case T_DATE:
      return hash_bytes_uint32(DateADTGetDatum(d));
    case T_BOOL:
      return hash_bytes_uint32((int32) DatumGetBool(d));
    case T_INT4:
      return hash_bytes_uint32(DatumGetInt32(d));
    case T_INT8:
      return pg_hashint8(DatumGetInt64(d));
    case T_FLOAT8:
      return pg_hashfloat8(DatumGetFloat8(d));
    case T_TEXT:
      return pg_hashtext(DatumGetTextP(d));
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      return gserialized_hash(DatumGetGserializedP(d));
#if NPOINT
    case T_NPOINT:
      return npoint_hash(DatumGetNpointP(d));
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown hash function for base type: %d", type);
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
datum_hash_extended(Datum d, meosType type, uint64 seed)
{
  assert(meos_basetype(type));
  switch (type)
  {
    case T_TIMESTAMPTZ:
      return pg_hashint8extended(DatumGetTimestampTz(d), seed);
    case T_DATE:
      return hash_bytes_uint32_extended((int32) DatumGetDateADT(d), seed);
    case T_BOOL:
      return hash_bytes_uint32_extended((int32) DatumGetBool(d), seed);
    case T_INT4:
      return hash_bytes_uint32_extended(DatumGetInt32(d), seed);
    case T_INT8:
      return pg_hashint8extended(DatumGetInt64(d), seed);
    case T_FLOAT8:
      return pg_hashfloat8extended(DatumGetFloat8(d), seed);
    case T_TEXT:
      return pg_hashtextextended(DatumGetTextP(d), seed);
    // PostGIS currently does not provide an extended hash function
    // case T_GEOMETRY:
    // case T_GEOGRAPHY:
      // return gserialized_hash_extended(DatumGetGserializedP(d), seed);
#if NPOINT
    case T_NPOINT:
      return npoint_hash_extended(DatumGetNpointP(d), seed);
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown extended hash function for base type: %d", type);
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
datum_copy(Datum value, meosType basetype)
{
  /* For types passed by value */
  if (basetype_byvalue(basetype))
    return value;
  /* For types passed by reference */
  int typlen = basetype_length(basetype);
  size_t value_size = (typlen != -1) ? (size_t) typlen :
    VARSIZE(DatumGetPointer(value));
  void *result = palloc(value_size);
  memcpy(result, DatumGetPointer(value), value_size);
  return PointerGetDatum(result);
}

/**
 * @brief Return a number converted to a double
 * @return On error return @p DBL_MAX
 */
double
datum_double(Datum d, meosType type)
{
  assert(numspan_basetype(type));
  switch (type)
  {
    case T_INT4:
      return (double) DatumGetInt32(d);
    case T_INT8:
      return (double) DatumGetInt64(d);
    case T_FLOAT8:
      return DatumGetFloat8(d);
    case T_DATE:
      return (double) DatumGetDateADT(d);
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown conversion to double function for base type: %d", type);
      return DBL_MAX;
  }
}

/**
 * @brief Return a double converted to a number Datum
 */
Datum
double_datum(double d, meosType type)
{
  assert(numspan_basetype(type));
  switch (type)
  {
    case T_INT4:
      return Int32GetDatum((int) d);
    case T_INT8:
      return Int64GetDatum((int64) d);
    case T_FLOAT8:
      return Float8GetDatum(d);
    case T_DATE:
      return DateADTGetDatum((DateADT) d);
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown conversion to Datum function for base type: %d", type);
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
datum_sort_cmp(const Datum *l, const Datum *r, const meosType *type)
{
  Datum x = *l;
  Datum y = *r;
  meosType t = *type;
  return datum_cmp(x, y, t);
}

/**
 * @brief Comparator function for timestamptz values
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
  return span_cmp_int(&lp, &rp);
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
  return;
}

/**
 * @brief Sort function for timestamptz values
 */
void
timestamparr_sort(TimestampTz *times, int count)
{
  qsort(times, (size_t) count, sizeof(TimestampTz),
    (qsort_comparator) &timestamp_sort_cmp);
  return;
}

#if 0 /* Not used */
/**
 * @brief Sort function for double2
 */
void
double2arr_sort(double2 *doubles, int count)
{
  qsort(doubles, count, sizeof(double2), (qsort_comparator) &double2_cmp);
  return;
}

/**
 * @brief Sort function for double3
 */
void
double3arr_sort(double3 *triples, int count)
{
  qsort(triples, count, sizeof(double3), (qsort_comparator) &double3_cmp);
  return;
}
#endif

/**
 * @brief Sort function for spans
 */
void
spanarr_sort(Span *spans, int count)
{
  qsort(spans, (size_t) count, sizeof(Span),
    (qsort_comparator) &span_cmp_int);
  return;
}

/**
 * @brief Sort function for temporal instants
 */
void
tinstarr_sort(TInstant **instants, int count)
{
  qsort(instants, (size_t) count, sizeof(TInstant *),
    (qsort_comparator) &tinstarr_sort_cmp);
  return;
}

/**
 * @brief Sort function for temporal sequences
 */
void
tseqarr_sort(TSequence **sequences, int count)
{
  qsort(sequences, (size_t) count, sizeof(TSequence *),
    (qsort_comparator) &tseqarr_sort_cmp);
  return;
}

/*****************************************************************************
 * Remove duplicate functions
 * These functions assume that the array has been sorted before
 * These functions pack the distinct values at the begining
 *****************************************************************************/

/**
 * @brief Remove duplicates from an array of datums
 */
int
datumarr_remove_duplicates(Datum *values, int count, meosType type)
{
  assert(count > 0);
  int newcount = 0;
  for (int i = 1; i < count; i++)
    if (datum_ne(values[newcount], values[i], type))
      values[++ newcount] = values[i];
  return newcount + 1;
}

/**
 * @brief Remove duplicates from an array of timestamptz values
 */
int
timestamparr_remove_duplicates(TimestampTz *values, int count)
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
 */
int
tinstarr_remove_duplicates(const TInstant **instants, int count)
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
  assert(array);
  for (int i = 0; i < count; i++)
  {
    if (array[i])
      pfree(array[i]);
  }
  pfree(array);
  return;
}

/**
 * @brief Return the string resulting from assembling the array of strings
 * @param[in] strings Array of strings to ouput
 * @param[in] count Number of elements in the input array
 * @param[in] outlen Total length of the elements and the additional ','
 * @param[in] prefix Prefix to add to the string (e.g., for interpolation)
 * @param[in] open, close Starting/ending character (e.g., '{' and '}')
 * @param[in] quotes True when elements should be enclosed into quotes
 * @param[in] spaces True when elements should be separated by spaces
 * @note The function frees the memory of the input strings after finishing
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
    // pfree(strings[i]); /* ??? */
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
 * @brief Determine the 3D hypotenuse
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

#if 0 /* not used */
/**
 * @brief Determine the 4D hypotenuse
 * @note The function is a generalization of the 3D case in function #hypot3d
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
 * Input/output PostgreSQL and PostGIS functions
 *****************************************************************************/

/**
 * @brief Call input function of the base type
 */
bool
#if NPOINT
basetype_in(const char *str, meosType type, bool end, Datum *result)
#else
basetype_in(const char *str, meosType type,
  bool end __attribute__((unused)), Datum *result)
#endif
{
  assert(meos_basetype(type));
  switch (type)
  {
    case T_TIMESTAMPTZ:
    {
      TimestampTz t = pg_timestamptz_in(str, -1);
      if (t == DT_NOEND)
        return false;
      *result = TimestampTzGetDatum(t);
      return true;
    }
    case T_DATE:
    {
      DateADT d = pg_date_in(str);
      if (d == DATEVAL_NOEND)
        return false;
      *result = DateADTGetDatum(d);
      return true;
    }
    case T_BOOL:
    {
      bool b = bool_in(str);
      if (meos_errno())
        return false;
      *result = BoolGetDatum(b);
      return true;
    }
    case T_INT4:
    {
      int i = int4_in(str);
      if (i == PG_INT32_MAX)
        return false;
      *result = Int32GetDatum(i);
      return true;
    }
    case T_INT8:
    {
      int64 i = int8_in(str);
      if (i == PG_INT64_MAX)
        return false;
      *result = Int64GetDatum(i);
      return true;
    }
    case T_FLOAT8:
    {
      double d = float8_in(str, "double precision", str);
      if (d == DBL_MAX)
        return false;
      *result = Float8GetDatum(d);
      return true;
    }
    case T_TEXT:
    {
      text *txt = cstring2text(str);
      if (! txt)
        return false;
      *result = PointerGetDatum(txt);
      return true;
    }
    case T_GEOMETRY:
    {
      GSERIALIZED *gs = pgis_geometry_in((char *) str, -1);
      if (! gs)
        return false;
      *result = PointerGetDatum(gs);
      return true;
    }
    case T_GEOGRAPHY:
    {
      GSERIALIZED *gs = pgis_geography_in((char *) str, -1);
      if (! gs)
        return false;
      *result = PointerGetDatum(gs);
      return true;
    }
#if NPOINT
    case T_NPOINT:
    {
      Npoint *np = npoint_parse(&str, end);
      if (! np)
        return false;
      *result = PointerGetDatum(np);
      return true;
    }
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown input function for base type: %s", meostype_name(type));
      return false;
  }
}

/**
 * @ingroup meos_pg_types
 * @brief Return the string representation of a text
 * @param[in] txt Text
 */
char *
text_out(const text *txt)
{
  assert(txt != NULL);
  char *str = text2cstring(txt);
  char *result = palloc(strlen(str) + 4);
  sprintf(result, "\"%s\"", str);
  pfree(str);
  return result;
}

/**
 * @brief Call output function of the base type
 * @return On error return @p NULL
 */
char *
basetype_out(Datum value, meosType type, int maxdd)
{
  assert(meos_basetype(type));
  assert(maxdd >= 0);

  switch (type)
  {
    case T_TIMESTAMPTZ:
      return pg_timestamptz_out(DatumGetTimestampTz(value));
    case T_DATE:
      return pg_date_out(DatumGetTimestampTz(value));
    case T_BOOL:
      return bool_out(DatumGetBool(value));
    case T_INT4:
      return int4_out(DatumGetInt32(value));
    case T_INT8:
      return int8_out(DatumGetInt64(value));
    case T_FLOAT8:
      return float8_out(DatumGetFloat8(value), maxdd);
    case T_TEXT:
      return text_out(DatumGetTextP(value));
#if DEBUG_BUILD
    case T_DOUBLE2:
      return double2_out(DatumGetDouble2P(value), maxdd);
    case T_DOUBLE3:
      return double3_out(DatumGetDouble3P(value), maxdd);
    case T_DOUBLE4:
      return double4_out(DatumGetDouble4P(value), maxdd);
#endif
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      return geo_out(DatumGetGserializedP(value));
#if NPOINT
    case T_NPOINT:
      return npoint_out(DatumGetNpointP(value), maxdd);
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown output function for base type: %s", meostype_name(type));
      return NULL;
  }
}

/*****************************************************************************/
