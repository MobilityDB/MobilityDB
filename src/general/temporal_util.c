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
 * @file temporal_util.c
 * @brief General utility functions for temporal types.
 */

#include "general/temporal_util.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <catalog/pg_collation_d.h>
#include <utils/lsyscache.h>
#include <utils/varlena.h>
#if POSTGRESQL_VERSION_NUMBER >= 120000
  #include <utils/float.h>
#endif
#ifndef MEOS
  #include <utils/builtins.h>
#endif
/* MobilityDB */
#include "general/pg_call.h"
#include "general/span.h"
#include "general/temporaltypes.h"
#include "general/doublen.h"
#include "point/pgis_call.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"
#ifndef MEOS
  #include "npoint/tnpoint_static.h"
#endif

/*****************************************************************************
 * Comparison functions on datums
 *****************************************************************************/

/* Version of the functions where the types of both arguments is equal */

/**
 * Return true if the first value is less than the second one
 */
int
datum_cmp(Datum l, Datum r, CachedType type)
{
  return datum_cmp2(l, r, type, type);
}

/**
 * Return true if the two values are equal
 */
bool
datum_eq(Datum l, Datum r, CachedType type)
{
  return datum_eq2(l, r, type, type);
}

/**
 * Return true if the two values are different
 */
bool
datum_ne(Datum l, Datum r, CachedType type)
{
  return ! datum_eq2(l, r, type, type);
}

/**
 * Return true if the first value is less than the second one
 */
bool
datum_lt(Datum l, Datum r, CachedType type)
{
  return datum_lt2(l, r, type, type);
}

/**
 * Return true if the first value is less than or equal to the second one
 */
bool
datum_le(Datum l, Datum r, CachedType type)
{
  return datum_eq2(l, r, type, type) || datum_lt2(l, r, type, type);
}

/**
 * Return true if the first value is greater than the second one
 */
bool
datum_gt(Datum l, Datum r, CachedType type)
{
  return datum_lt2(r, l, type, type);
}

#if 0 /* Not used */
/**
 * Return true if the first value is greater than or equal to the second one
 */
bool
datum_ge(Datum l, Datum r, CachedType type)
{
  return datum_eq2(l, r, type, type) || datum_lt2(r, l, type, type);
}
#endif

/*****************************************************************************/

/*
 * Version of the functions where the types of both arguments may be different
 * but compatible, e.g., integer and float
 */

/**
 * Return true if the first value is less than the second one
 * (base type dispatch function)
 */
int
datum_cmp2(Datum l, Datum r, CachedType typel, CachedType typer)
{
  ensure_span_basetype(typel);
  if (typel != typer)
    ensure_span_basetype(typer);
  if (typel == T_TIMESTAMPTZ && typer == T_TIMESTAMPTZ)
    return timestamp_cmp_internal(DatumGetTimestampTz(l),
      DatumGetTimestampTz(r));
  if (typel == T_INT4 && typer == T_INT4)
    return (DatumGetInt32(l) < DatumGetInt32(r)) ? -1 :
      ((DatumGetInt32(l) > DatumGetInt32(r)) ? 1 : 0);
  if (typel == T_INT4 && typer == T_FLOAT8)
    return float8_cmp_internal((double) DatumGetInt32(l), DatumGetFloat8(r));
  if (typel == T_FLOAT8 && typer == T_INT4)
    return float8_cmp_internal(DatumGetFloat8(l), (double) DatumGetInt32(r));
  if (typel == T_FLOAT8 && typer == T_FLOAT8)
    return float8_cmp_internal(DatumGetFloat8(l), DatumGetFloat8(r));
  elog(ERROR, "unknown span_elem_cmp function for span base type: %d", typel);
}

/**
 * Return true if the two values are equal even if their type is not the same
 */
bool
datum_eq2(Datum l, Datum r, CachedType typel, CachedType typer)
{
  ensure_temporal_basetype(typel);
  if (typel != typer)
    ensure_temporal_basetype(typer);
  if ((typel == T_TIMESTAMPTZ && typer == T_TIMESTAMPTZ) ||
    (typel == T_BOOL && typer == T_BOOL) ||
    (typel == T_INT4 && typer == T_INT4))
    return l == r;
  if (typel == T_FLOAT8 && typer == T_FLOAT8)
    return MOBDB_FP_EQ(DatumGetFloat8(l), DatumGetFloat8(r));
  if (typel == T_INT4 && typer == T_FLOAT8)
    return MOBDB_FP_EQ((double) DatumGetInt32(l), DatumGetFloat8(r));
  if (typel == T_FLOAT8 && typer == T_INT4)
    return MOBDB_FP_EQ(DatumGetFloat8(l), (double) DatumGetInt32(r));
  if (typel == T_TEXT && typer == T_TEXT)
    return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) == 0;
  if (typel == T_DOUBLE2 && typel == typer)
    return double2_eq(DatumGetDouble2P(l), DatumGetDouble2P(r));
  if (typel == T_DOUBLE3 && typel == typer)
    return double3_eq(DatumGetDouble3P(l), DatumGetDouble3P(r));
  if (typel == T_DOUBLE4 && typel == typer)
    return double4_eq(DatumGetDouble4P(l), DatumGetDouble4P(r));
  if (typel == T_GEOMETRY && typel == typer)
    return datum_point_eq(l, r);
  if (typel == T_GEOGRAPHY && typel == typer)
    return datum_point_eq(l, r);
#ifndef MEOS
  if (typel == T_NPOINT && typel == typer)
    return npoint_eq(DatumGetNpointP(l), DatumGetNpointP(r));
#endif
  elog(ERROR, "unknown datum_eq2 function for base type: %d", typel);
}

/**
 * Return true if the two values are different
 */
bool
datum_ne2(Datum l, Datum r, CachedType typel, CachedType typer)
{
  return ! datum_eq2(l, r, typel, typer);
}

/**
 * Return true if the first value is less than the second one
 */
bool
datum_lt2(Datum l, Datum r, CachedType typel, CachedType typer)
{
  ensure_temporal_basetype(typel);
  if (typel != typer)
    ensure_temporal_basetype(typer);
  if (typel == T_TIMESTAMPTZ && typer == T_TIMESTAMPTZ)
    return DatumGetTimestampTz(l) < DatumGetTimestampTz(r);
  if (typel == T_BOOL && typer == T_BOOL)
    return DatumGetBool(l) < DatumGetBool(r);
  if (typel == T_INT4 && typer == T_INT4)
    return DatumGetInt32(l) < DatumGetInt32(r);
  if (typel == T_INT4 && typer == T_FLOAT8)
    return MOBDB_FP_LT((double) DatumGetInt32(l), DatumGetFloat8(r));
  if (typel == T_FLOAT8 && typer == T_INT4)
    return MOBDB_FP_LT(DatumGetFloat8(l), (double) DatumGetInt32(r));
  if (typel == T_FLOAT8 && typer == T_FLOAT8)
    return MOBDB_FP_LT(DatumGetFloat8(l), DatumGetFloat8(r));
  if (typel == T_TEXT && typel == typer)
    return text_cmp(DatumGetTextP(l), DatumGetTextP(r),
      DEFAULT_COLLATION_OID) < 0;
  if (typel == T_GEOMETRY && typel == typer)
    return gserialized_cmp((GSERIALIZED *) DatumGetPointer(l),
      (GSERIALIZED *) DatumGetPointer(r)) < 0;
  if (typel == T_GEOGRAPHY && typel == typer)
    return gserialized_cmp((GSERIALIZED *) DatumGetPointer(l),
      (GSERIALIZED *) DatumGetPointer(r)) < 0;
#ifndef MEOS
  if (typel == T_NPOINT && typel == typer)
    return npoint_lt(DatumGetNpointP(l), DatumGetNpointP(r));
#endif
  elog(ERROR, "unknown datum_lt2 function for base type: %d", typel);
}


/**
 * Return true if the first value is less than or equal to the second one
 */
bool
datum_le2(Datum l, Datum r, CachedType typel, CachedType typer)
{
  return datum_eq2(l, r, typel, typer) || datum_lt2(l, r, typel, typer);
}

/**
 * Return true if the first value is greater than the second one
 */
bool
datum_gt2(Datum l, Datum r, CachedType typel, CachedType typer)
{
  return datum_lt2(r, l, typer, typel);
}

/**
 * Return true if the first value is greater than or equal to the second one
 */
bool
datum_ge2(Datum l, Datum r, CachedType typel, CachedType typer)
{
  return datum_eq2(l, r, typel, typer) || datum_gt2(l, r, typel, typer);
}

/*****************************************************************************/

/**
 * Return a Datum true if the two values are equal
 */
Datum
datum2_eq2(Datum l, Datum r, CachedType typel, CachedType typer)
{
  return BoolGetDatum(datum_eq2(l, r, typel, typer));
}

/**
 * Return a Datum true if the two values are different
 */
Datum
datum2_ne2(Datum l, Datum r, CachedType typel, CachedType typer)
{
  return BoolGetDatum(datum_ne2(l, r, typel, typer));
}

/**
 * Return a Datum true if the first value is less than the second one
 */
Datum
datum2_lt2(Datum l, Datum r, CachedType typel, CachedType typer)
{
  return BoolGetDatum(datum_lt2(l, r, typel, typer));
}

/**
 * Return a Datum true if the first value is less than or equal to the second one
 */
Datum
datum2_le2(Datum l, Datum r, CachedType typel, CachedType typer)
{
  return BoolGetDatum(datum_le2(l, r, typel, typer));
}

/**
 * Return a Datum true if the first value is greater than the second one
 */
Datum
datum2_gt2(Datum l, Datum r, CachedType typel, CachedType typer)
{
  return BoolGetDatum(datum_gt2(l, r, typel, typer));
}

/**
 * Return a Datum true if the first value is greater than or equal to the second one
 */
Datum
datum2_ge2(Datum l, Datum r, CachedType typel, CachedType typer)
{
  return BoolGetDatum(datum_ge2(l, r, typel, typer));
}

/*****************************************************************************
 * Arithmetic functions on datums
 * N.B. The validity of the Oids must be done in the calling function.
 *****************************************************************************/

/**
 * Return the addition of the two numbers
 */
Datum
datum_add(Datum l, Datum r, CachedType typel, CachedType typer)
{
  Datum result = 0;
  if (typel == T_INT4)
  {
    if (typer == T_INT4)
      result = Int32GetDatum(DatumGetInt32(l) + DatumGetInt32(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum(DatumGetInt32(l) + DatumGetFloat8(r));
  }
  else /* typel == T_FLOAT8 */
  {
    if (typer == T_INT4)
      result = Float8GetDatum(DatumGetFloat8(l) + DatumGetInt32(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum(DatumGetFloat8(l) + DatumGetFloat8(r));
  }
  return result;
}

/**
 * Return the subtraction of the two numbers
 */
Datum
datum_sub(Datum l, Datum r, CachedType typel, CachedType typer)
{
  Datum result = 0;
  if (typel == T_INT4)
  {
    if (typer == T_INT4)
      result = Int32GetDatum(DatumGetInt32(l) - DatumGetInt32(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum(DatumGetInt32(l) - DatumGetFloat8(r));
  }
  else /* typel == T_FLOAT8 */
  {
    if (typer == T_INT4)
      result = Float8GetDatum(DatumGetFloat8(l) - DatumGetInt32(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum(DatumGetFloat8(l) - DatumGetFloat8(r));
  }
  return result;
}

/**
 * Return the multiplication of the two numbers
 */
Datum
datum_mult(Datum l, Datum r, CachedType typel, CachedType typer)
{
  Datum result = 0;
  if (typel == T_INT4)
  {
    if (typer == T_INT4)
      result = Int32GetDatum(DatumGetInt32(l) * DatumGetInt32(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum(DatumGetInt32(l) * DatumGetFloat8(r));
  }
  else /* typel == T_FLOAT8 */
  {
    if (typer == T_INT4)
      result = Float8GetDatum(DatumGetFloat8(l) * DatumGetInt32(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum(DatumGetFloat8(l) * DatumGetFloat8(r));
  }
  return result;
}

/**
 * Return the division of the two numbers
 */
Datum
datum_div(Datum l, Datum r, CachedType typel, CachedType typer)
{
  Datum result;
  if (typel == T_INT4)
  {
    if (typer == T_INT4)
      result = Int32GetDatum(DatumGetInt32(l) / DatumGetInt32(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum(DatumGetInt32(l) / DatumGetFloat8(r));
  }
  else /* typel == T_FLOAT8 */
  {
    if (typer == T_INT4)
      result = Float8GetDatum(DatumGetFloat8(l) / DatumGetInt32(r));
    else /* typer == T_FLOAT8 */
      result = Float8GetDatum(DatumGetFloat8(l) / DatumGetFloat8(r));
  }
  return result;
}

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

/**
 * Align to double
 */
size_t
double_pad(size_t size)
{
  if (size % 8)
    return size + (8 - size % 8);
  return size;
}

/**
 * Copy a Datum if it is passed by reference
 */
Datum
datum_copy(Datum value, CachedType basetype)
{
  /* For types passed by value */
  if (basetype_byvalue(basetype))
    return value;
  /* For types passed by reference */
  int typlen = basetype_length(basetype);
  size_t value_size = (typlen != -1) ? (size_t) typlen : VARSIZE(value);
  void *result = palloc0(value_size);
  memcpy(result, DatumGetPointer(value), value_size);
  return PointerGetDatum(result);
}

/**
 * Convert a number to a double
 */
double
datum_double(Datum d, CachedType basetype)
{
  ensure_tnumber_basetype(basetype);
  if (basetype == T_INT4)
    return (double)(DatumGetInt32(d));
  else /* basetype == T_FLOAT8 */
    return DatumGetFloat8(d);
}

/*****************************************************************************
 * Sort functions
 *****************************************************************************/

/**
 * Comparator function for datums
 */
static int
datum_sort_cmp(const Datum *l, const Datum *r, const CachedType *type)
{
  Datum x = *l;
  Datum y = *r;
  CachedType t = *type;
  if (datum_eq(x, y, t))
    return 0;
  else if (datum_lt(x, y, t))
    return -1;
  else
    return 1;
}

/**
 * Comparator function for timestamps
 */
static int
timestamp_sort_cmp(const TimestampTz *l, const TimestampTz *r)
{
  TimestampTz x = *l;
  TimestampTz y = *r;
  return timestamp_cmp_internal(x, y);
}

/**
 * Comparator function for spans
 */
static int
span_sort_cmp(const Span **l, const Span **r)
{
  return span_cmp(*l, *r);
}

/**
 * Comparator function for temporal instants
 */
static int
tinstarr_sort_cmp(const TInstant **l, const TInstant **r)
{
  return timestamp_cmp_internal((*l)->t, (*r)->t);
}

/**
 * Comparator function for temporal sequences
 */
static int
tseqarr_sort_cmp(TSequence **l, TSequence **r)
{
  Period lp = (*l)->period;
  Period rp = (*r)->period;
  return span_cmp(&lp, &rp);
}

/*****************************************************************************/

/**
 * Sort function for datums
 */
void
datumarr_sort(Datum *values, int count, CachedType type)
{
  qsort_arg(values, (size_t) count, sizeof(Datum),
    (qsort_arg_comparator) &datum_sort_cmp, &type);
}

/**
 * Sort function for timestamps
 */
void
timestamparr_sort(TimestampTz *times, int count)
{
  qsort(times, (size_t) count, sizeof(TimestampTz),
    (qsort_comparator) &timestamp_sort_cmp);
}

#if 0 /* Not used */
/**
 * Sort function for double2
 */
void
double2arr_sort(double2 *doubles, int count)
{
  qsort(doubles, count, sizeof(double2), (qsort_comparator) &double2_cmp);
}

/**
 * Sort function for double3
 */
void
double3arr_sort(double3 *triples, int count)
{
  qsort(triples, count, sizeof(double3),
    (qsort_comparator) &double3_cmp);
}
#endif

/**
 * Sort function for spans
 */
void
spanarr_sort(Span **spans, int count)
{
  qsort(spans, (size_t) count, sizeof(Span *),
    (qsort_comparator) &span_sort_cmp);
}

/**
 * Sort function for temporal instants
 */
void
tinstarr_sort(TInstant **instants, int count)
{
  qsort(instants, (size_t) count, sizeof(TInstant *),
    (qsort_comparator) &tinstarr_sort_cmp);
}

/**
 * Sort function for temporal sequences
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
 * Remove duplicates from an array of datums
 */
int
datumarr_remove_duplicates(Datum *values, int count, CachedType type)
{
  assert (count > 0);
  int newcount = 0;
  for (int i = 1; i < count; i++)
    if (datum_ne(values[newcount], values[i], type))
      values[++ newcount] = values[i];
  return newcount + 1;
}

/**
 * Remove duplicates from an array of timestamps
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
 * Remove duplicates from an array of temporal instants
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
 * Text functions
 *****************************************************************************/

/**
 * Convert a C string into a text value
 *
 * @note We don't include <utils/builtins.h> to avoid collisions with json-c/json.h
 * @note Function taken from PostGIS file lwgeom_in_geojson.c
 */

text *
cstring2text(const char *cstring)
{
  size_t len = strlen(cstring);
  text *result = (text *) palloc(len + VARHDRSZ);
  SET_VARSIZE(result, len + VARHDRSZ);
  memcpy(VARDATA(result), cstring, len);
  return result;
}

/**
 * Convert a text value into a C string
 *
 * @note We don't include <utils/builtins.h> to avoid collisions with json-c/json.h
 * @note Function taken from PostGIS file lwgeom_in_geojson.c
 */
char *
text2cstring(const text *textptr)
{
  size_t size = VARSIZE_ANY_EXHDR(textptr);
  char *str = palloc(size + 1);
  memcpy(str, VARDATA(textptr), size);
  str[size]='\0';
  return str;
}

/**
 * Comparison function for text values
 *
 * @note Function copied from PostgreSQL since it is not exported
 */
int
text_cmp(text *arg1, text *arg2, Oid collid)
{
  char  *a1p,
      *a2p;
  int    len1,
      len2;

  a1p = VARDATA_ANY(arg1);
  a2p = VARDATA_ANY(arg2);

  len1 = (int) VARSIZE_ANY_EXHDR(arg1);
  len2 = (int) VARSIZE_ANY_EXHDR(arg2);

  return varstr_cmp(a1p, len1, a2p, len2, collid);
}

/*****************************************************************************
 * Hypotenuse functions
 *****************************************************************************/

/**
 * Determine the 3D hypotenuse.
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

/**
 * Determine the 4D hypotenuse.
 *
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

/*****************************************************************************
 * Input/output PostgreSQL functions
 *****************************************************************************/

/**
 * Call input function of the base type
 */
Datum
basetype_input(CachedType basetype, char *str)
{
  ensure_temporal_basetype(basetype);
  if (basetype == T_TIMESTAMPTZ)
    return TimestampTzGetDatum(pg_timestamptz_in(str, -1));
  if (basetype == T_BOOL)
    return BoolGetDatum(pg_boolin(str));
  if (basetype == T_INT4)
    return Int32GetDatum(pg_int4in(str));
  if (basetype == T_INT8)
    return Int64GetDatum(pg_int8in(str));
  if (basetype == T_FLOAT8)
    return Float8GetDatum(float8in_internal(str, NULL, "double precision",
      str));
  if (basetype == T_TEXT)
    return PointerGetDatum(cstring_to_text(str));
  if (basetype == T_GEOMETRY)
#if POSTGIS_VERSION_NUMBER < 30000
    return call_input(type_oid(T_GEOMETRY), str);
#else
    return PointerGetDatum(PGIS_LWGEOM_in(str, -1));
#endif
  if (basetype == T_GEOGRAPHY)
#ifndef MEOS
    return call_input(type_oid(T_GEOGRAPHY), str);
#else
    return PointerGetDatum(PGIS_geography_in(str, -1));
#endif
#ifndef MEOS
  if (basetype == T_NPOINT)
    return PointerGetDatum(npoint_in(str));
#endif
  elog(ERROR, "unknown type_input function for base type: %d", basetype);
}

/**
 * Call output function of the base type
 */
char *
basetype_output(CachedType basetype, Datum value)
{
  ensure_temporal_basetype(basetype);
  if (basetype == T_TIMESTAMPTZ)
    return pg_timestamptz_out(DatumGetTimestampTz(value));
  if (basetype == T_BOOL)
    return pg_boolout(DatumGetBool(value));
  if (basetype == T_INT4)
    return pg_int4out(DatumGetInt32(value));
  if (basetype == T_INT8)
    return pg_int8out(DatumGetInt64(value));
  if (basetype == T_FLOAT8)
    return float8out_internal(DatumGetFloat8(value));
  if (basetype == T_TEXT)
    return text_to_cstring(DatumGetTextP(value));
  if (basetype == T_GEOMETRY)
    return PGIS_LWGEOM_out((GSERIALIZED *) DatumGetPointer(value));
  if (basetype == T_GEOGRAPHY)
    return PGIS_geography_out((GSERIALIZED *) DatumGetPointer(value));
#ifndef MEOS
  if (basetype == T_NPOINT)
    return npoint_out(DatumGetNpointP(value));
#endif
  elog(ERROR, "unknown type_input function for base type: %d", basetype);
}

/**
 * Call receive function of the base type
 */
Datum
basetype_recv(CachedType basetype, StringInfo buf)
{
  ensure_temporal_basetype(basetype);
  if (basetype == T_TIMESTAMPTZ)
    return TimestampTzGetDatum(pg_timestamptz_recv(buf));
  if (basetype == T_BOOL)
    return BoolGetDatum(pg_boolrecv(buf));
  if (basetype == T_INT4)
    return Int32GetDatum(pg_int4recv(buf));
  if (basetype == T_INT8)
    return Int64GetDatum(pg_int8recv(buf));
  if (basetype == T_FLOAT8)
    return Float8GetDatum(pg_float8recv(buf));
  if (basetype == T_TEXT)
    return PointerGetDatum(pg_textrecv(buf));
  if (basetype == T_DOUBLE2)
    return PointerGetDatum(double2_recv(buf));
  if (basetype == T_DOUBLE3)
    return PointerGetDatum(double3_recv(buf));
  if (basetype == T_DOUBLE4)
    return PointerGetDatum(double4_recv(buf));
  if (basetype == T_GEOMETRY)
    return PointerGetDatum(PGIS_LWGEOM_recv(buf));
  if (basetype == T_GEOGRAPHY)
#ifndef MEOS
    return call_recv(type_oid(T_GEOGRAPHY), buf);
#else
    return PointerGetDatum(PGIS_geography_recv(buf));
#endif
#ifndef MEOS
  if (basetype == T_NPOINT)
    return PointerGetDatum(npoint_recv(buf));
#endif
  elog(ERROR, "unknown type_input function for base type: %d", basetype);
}

/**
 * Call send function of the base type
 */
bytea *
basetype_send(CachedType basetype, Datum value)
{
  ensure_temporal_basetype(basetype);
  if (basetype == T_TIMESTAMPTZ)
    return pg_timestamptz_send(DatumGetTimestampTz(value));
  if (basetype == T_BOOL)
    return pg_boolsend(DatumGetBool(value));
  if (basetype == T_INT4)
    return pg_int4send(DatumGetInt32(value));
  if (basetype == T_INT8)
    return pg_int8send(DatumGetInt64(value));
  if (basetype == T_FLOAT8)
    return pg_float8send(DatumGetFloat8(value));
  if (basetype == T_TEXT)
    return pg_textsend(DatumGetTextP(value));
  if (basetype == T_DOUBLE2)
    return double2_send(DatumGetDouble2P(value));
  if (basetype == T_DOUBLE3)
    return double3_send(DatumGetDouble3P(value));
  if (basetype == T_DOUBLE4)
    return double4_send(DatumGetDouble4P(value));
  if (basetype == T_GEOMETRY)
    return PGIS_LWGEOM_send((GSERIALIZED *) DatumGetPointer(value));
  if (basetype == T_GEOGRAPHY)
    return PGIS_geography_send((GSERIALIZED *) DatumGetPointer(value));
#ifndef MEOS
  if (basetype == T_NPOINT)
    return npoint_send(DatumGetNpointP(value));
#endif
  elog(ERROR, "unknown type_input function for base type: %d", basetype);
}

/*****************************************************************************
 * Call PostgreSQL functions
 * The call_input and call_output functions are needed for the geography
 * type to call the function srid_is_latlong(fcinfo, srid)
 *****************************************************************************/

/**
 * Call input function of the base type
 */
Datum
call_input(Oid typid, char *str)
{
  Oid infunc;
  Oid basetypid;
  FmgrInfo infuncinfo;
  getTypeInputInfo(typid, &infunc, &basetypid);
  fmgr_info(infunc, &infuncinfo);
  return InputFunctionCall(&infuncinfo, str, basetypid, -1);
}

/**
 * Call receive function of the base type
 */
Datum
call_recv(Oid typid, StringInfo buf)
{
  Oid recvfunc;
  Oid basetypid;
  FmgrInfo recvfuncinfo;
  getTypeBinaryInputInfo(typid, &recvfunc, &basetypid);
  fmgr_info(recvfunc, &recvfuncinfo);
  return ReceiveFunctionCall(&recvfuncinfo, buf, basetypid, -1);
}

/**
 * Call PostgreSQL function with 1 argument
 */
#if POSTGRESQL_VERSION_NUMBER >= 120000
Datum
call_function1(PGFunction func, Datum arg1)
{
  LOCAL_FCINFO(fcinfo, 1);
  FmgrInfo flinfo;
  memset(&flinfo, 0, sizeof(flinfo));
  flinfo.fn_mcxt = CurrentMemoryContext;
  Datum result;
  InitFunctionCallInfoData(*fcinfo, &flinfo, 1, DEFAULT_COLLATION_OID, NULL, NULL);
  fcinfo->args[0].value = arg1;
  fcinfo->args[0].isnull = false;
  result = (*func) (fcinfo);
  if (fcinfo->isnull)
    elog(ERROR, "Function %p returned NULL", (void *) func);
  return result;
}

/**
 * Call PostgreSQL function with 2 arguments
 */
Datum
call_function2(PGFunction func, Datum arg1, Datum arg2)
{
  LOCAL_FCINFO(fcinfo, 2);
  FmgrInfo flinfo;
  memset(&flinfo, 0, sizeof(flinfo)) ;
  flinfo.fn_nargs = 2;
  flinfo.fn_mcxt = CurrentMemoryContext;
  Datum result;
  InitFunctionCallInfoData(*fcinfo, &flinfo, 2, DEFAULT_COLLATION_OID, NULL, NULL);
  fcinfo->args[0].value = arg1;
  fcinfo->args[0].isnull = false;
  fcinfo->args[1].value = arg2;
  fcinfo->args[1].isnull = false;
  result = (*func) (fcinfo);
  if (fcinfo->isnull)
    elog(ERROR, "function %p returned NULL", (void *) func);
  return result;
}

/**
 * Call PostgreSQL function with 3 arguments
 */
Datum
call_function3(PGFunction func, Datum arg1, Datum arg2, Datum arg3)
{
  LOCAL_FCINFO(fcinfo, 3);
  FmgrInfo flinfo;
  memset(&flinfo, 0, sizeof(flinfo)) ;
  flinfo.fn_mcxt = CurrentMemoryContext;
  Datum result;
  InitFunctionCallInfoData(*fcinfo, &flinfo, 3, DEFAULT_COLLATION_OID, NULL, NULL);
  fcinfo->args[0].value = arg1;
  fcinfo->args[0].isnull = false;
  fcinfo->args[1].value = arg2;
  fcinfo->args[1].isnull = false;
  fcinfo->args[2].value = arg3;
  fcinfo->args[2].isnull = false;
  result = (*func) (fcinfo);
  if (fcinfo->isnull)
    elog(ERROR, "function %p returned NULL", (void *) func);
  return result;
}
#else /* POSTGRESQL_VERSION_NUMBER < 120000 */
/**
 * Call PostgreSQL function with 1 argument
 */
Datum
call_function1(PGFunction func, Datum arg1)
{
  FunctionCallInfoData fcinfo;
  FmgrInfo flinfo;
  memset(&flinfo, 0, sizeof(flinfo));
  flinfo.fn_mcxt = CurrentMemoryContext;
  Datum result;
  InitFunctionCallInfoData(fcinfo, &flinfo, 1, DEFAULT_COLLATION_OID, NULL, NULL);
  fcinfo.arg[0] = arg1;
  fcinfo.argnull[0] = false;
  result = (*func) (&fcinfo);
  if (fcinfo.isnull)
    elog(ERROR, "Function %p returned NULL", (void *) func);
  return result;
}

/**
 * Call PostgreSQL function with 2 arguments
 */
Datum
call_function2(PGFunction func, Datum arg1, Datum arg2)
{
  FunctionCallInfoData fcinfo;
  FmgrInfo flinfo;
  memset(&flinfo, 0, sizeof(flinfo)) ;
  flinfo.fn_mcxt = CurrentMemoryContext;
  Datum result;
  InitFunctionCallInfoData(fcinfo, &flinfo, 2, DEFAULT_COLLATION_OID, NULL, NULL);
  fcinfo.arg[0] = arg1;
  fcinfo.argnull[0] = false;
  fcinfo.arg[1] = arg2;
  fcinfo.argnull[1] = false;
  result = (*func) (&fcinfo);
  if (fcinfo.isnull)
    elog(ERROR, "function %p returned NULL", (void *) func);
  return result;
}

/**
 * Call PostgreSQL function with 3 arguments
 */
Datum
call_function3(PGFunction func, Datum arg1, Datum arg2, Datum arg3)
{
  FunctionCallInfoData fcinfo;
  FmgrInfo flinfo;
  memset(&flinfo, 0, sizeof(flinfo)) ;
  flinfo.fn_mcxt = CurrentMemoryContext;
  Datum result;
  InitFunctionCallInfoData(fcinfo, &flinfo, 3, DEFAULT_COLLATION_OID, NULL, NULL);
  fcinfo.arg[0] = arg1;
  fcinfo.argnull[0] = false;
  fcinfo.arg[1] = arg2;
  fcinfo.argnull[1] = false;
  fcinfo.arg[2] = arg3;
  fcinfo.argnull[2] = false;
  result = (*func) (&fcinfo);
  if (fcinfo.isnull)
    elog(ERROR, "function %p returned NULL", (void *) func);
  return result;
}
#endif

/*****************************************************************************/

/* CallerFInfoFunctionCall 1 to 3 are provided by PostGIS */

#if POSTGRESQL_VERSION_NUMBER < 120000
Datum
CallerFInfoFunctionCall4(PGFunction func, FmgrInfo *flinfo, Oid collid,
  Datum arg1, Datum arg2, Datum arg3, Datum arg4)
{
  FunctionCallInfoData fcinfo;
  Datum    result;

  InitFunctionCallInfoData(fcinfo, flinfo, 3, collid, NULL, NULL);

  fcinfo.arg[0] = arg1;
  fcinfo.arg[1] = arg2;
  fcinfo.arg[2] = arg3;
  fcinfo.arg[3] = arg4;
  fcinfo.argnull[0] = false;
  fcinfo.argnull[1] = false;
  fcinfo.argnull[2] = false;
  fcinfo.argnull[3] = false;

  result = (*func) (&fcinfo);

  /* Check for null result, since caller is clearly not expecting one */
  if (fcinfo.isnull)
    elog(ERROR, "function %p returned NULL", (void *) func);

  return result;
}
#else
/* PgSQL 12+ still lacks 3-argument version of these functions */
Datum
CallerFInfoFunctionCall4(PGFunction func, FmgrInfo *flinfo, Oid collid,
  Datum arg1, Datum arg2, Datum arg3, Datum arg4)
{
    LOCAL_FCINFO(fcinfo, 4);
    Datum       result;

    InitFunctionCallInfoData(*fcinfo, flinfo, 3, collid, NULL, NULL);

    fcinfo->args[0].value = arg1;
    fcinfo->args[0].isnull = false;
    fcinfo->args[1].value = arg2;
    fcinfo->args[1].isnull = false;
    fcinfo->args[2].value = arg3;
    fcinfo->args[2].isnull = false;
    fcinfo->args[3].value = arg4;
    fcinfo->args[3].isnull = false;

    result = (*func) (fcinfo);

    /* Check for null result, since caller is clearly not expecting one */
    if (fcinfo->isnull)
      elog(ERROR, "function %p returned NULL", (void *) func);

    return result;
}
#endif

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/*****************************************************************************
 * Array functions
 *****************************************************************************/

/**
 * Free a C array of pointers
 */
void
pfree_array(void **array, int count)
{
  for (int i = 0; i < count; i++)
    pfree(array[i]);
  pfree(array);
  return;
}

/**
 * Free a C array of Datum pointers
 */
void
pfree_datumarr(Datum *array, int count)
{
  for (int i = 0; i < count; i++)
    pfree(DatumGetPointer(array[i]));
  pfree(array);
  return;
}

/**
 * Return the string resulting from assembling the array of strings.
 * The function frees the memory of the input strings after finishing.
 */
char *
stringarr_to_string(char **strings, int count, int outlen,
  char *prefix, char open, char close)
{
  char *result = palloc(strlen(prefix) + outlen + 3);
  result[outlen] = '\0';
  size_t pos = 0;
  strcpy(result, prefix);
  pos += strlen(prefix);
  result[pos++] = open;
  for (int i = 0; i < count; i++)
  {
    strcpy(result + pos, strings[i]);
    pos += strlen(strings[i]);
    result[pos++] = ',';
    result[pos++] = ' ';
    pfree(strings[i]);
  }
  result[pos - 2] = close;
  result[pos - 1] = '\0';
  pfree(strings);
  return result;
}

/**
 * Extract a C array from a PostgreSQL array containing datums
 * If array elements are pass-by-ref data type, the returned Datums will
 * be pointers into the array object.
 */
Datum *
datumarr_extract(ArrayType *array, int *count)
{
  bool byval;
  int16 typlen;
  char align;
  get_typlenbyvalalign(array->elemtype, &typlen, &byval, &align);
  Datum *result;
  deconstruct_array(array, array->elemtype, typlen, byval, align,
    &result, NULL, count);
  return result;
}

/**
 * Extract a C array from a PostgreSQL array containing timestamps
 */
TimestampTz *
timestamparr_extract(ArrayType *array, int *count)
{
  return (TimestampTz *) datumarr_extract(array, count);
}

/**
 * Extract a C array from a PostgreSQL array containing periods
 */
Period **
periodarr_extract(ArrayType *array, int *count)
{
  return (Period **) datumarr_extract(array, count);
}

/**
 * Extract a C array from a PostgreSQL array containing spans
 */
Span **
spanarr_extract(ArrayType *array, int *count)
{
  return (Span **) datumarr_extract(array, count);
}

/**
 * Extract a C array from a PostgreSQL array containing temporal values
 */
Temporal **
temporalarr_extract(ArrayType *array, int *count)
{
  Temporal **result;
  deconstruct_array(array, array->elemtype, -1, false, 'd',
    (Datum **) &result, NULL, count);
  return result;
}

/*****************************************************************************/

/**
 * Convert a C array of datums into a PostgreSQL array.
 * Note that the values will be copied into the object even if pass-by-ref type
 */
ArrayType *
datumarr_to_array(Datum *values, int count, CachedType type)
{
  int16 elmlen;
  bool elmbyval;
  char elmalign;
  assert(count > 0);
  Oid typid = type_oid(type);
  get_typlenbyvalalign(typid, &elmlen, &elmbyval, &elmalign);
  ArrayType *result = construct_array(values, count, typid, elmlen, elmbyval,
    elmalign);
  return result;
}

/**
 * Convert a C array of timestamps into a PostgreSQL array
 */
ArrayType *
timestamparr_to_array(const TimestampTz *times, int count)
{
  assert(count > 0);
  ArrayType *result = construct_array((Datum *) times, count, TIMESTAMPTZOID,
    8, true, 'd');
  return result;
}

/**
 * Convert a C array of periods into a PostgreSQL array
 */
ArrayType *
periodarr_to_array(const Period **periods, int count)
{
  assert(count > 0);
  ArrayType *result = construct_array((Datum *) periods, count,
    type_oid(T_PERIOD), sizeof(Period), false, 'd');
  return result;
}

/**
 * Convert a C array of spans into a PostgreSQL array
 */
ArrayType *
spanarr_to_array(Span **spans, int count)
{
  assert(count > 0);
  ArrayType *result = construct_array((Datum *) spans, count,
    type_oid(spans[0]->spantype), sizeof(Span), false, 'd');
  return result;
}

/**
 * Convert a C array of text values into a PostgreSQL array
 */
ArrayType *
strarr_to_textarray(char **strarr, int count)
{
  assert(count > 0);
  text **textarr = (text **) palloc(sizeof(text *) * count);
  for (int i = 0; i < count; i++)
    textarr[i] = cstring_to_text(strarr[i]);
  ArrayType *result = construct_array((Datum *) textarr, count, TEXTOID, -1,
    false, 'i');
  pfree_array((void **)textarr, count);
  return result;
}

/**
 * Convert a C array of temporal values into a PostgreSQL array
 */
ArrayType *
temporalarr_to_array(const Temporal **temporalarr, int count)
{
  assert(count > 0);
  Oid temptypid = type_oid(temporalarr[0]->temptype);
  ArrayType *result = construct_array((Datum *) temporalarr, count, temptypid,
    -1, false, 'd');
  return result;
}

/**
 * Convert a C array of spatiotemporal boxes into a PostgreSQL array
 */
ArrayType *
stboxarr_to_array(STBOX *boxarr, int count)
{
  assert(count > 0);
  STBOX **boxes = palloc(sizeof(STBOX *) * count);
  for (int i = 0; i < count; i++)
    boxes[i] = &boxarr[i];
  ArrayType *result = construct_array((Datum *) boxes, count,
    type_oid(T_STBOX), sizeof(STBOX), false, 'd');
  pfree(boxes);
  return result;
}

/*****************************************************************************
 * Range functions
 *****************************************************************************/

/**
 * Construct a range value from given arguments
 */
RangeType *
range_make(Datum from, Datum to, bool lower_inc, bool upper_inc,
  CachedType basetype)
{
  Oid rangetypid = 0;
  assert (basetype == T_INT4 || basetype == T_TIMESTAMPTZ);
  if (basetype == T_INT4)
    rangetypid = type_oid(T_INT4RANGE);
  else /* basetype == T_TIMESTAMPTZ */
    rangetypid = type_oid(T_TSTZRANGE);

  TypeCacheEntry* typcache = lookup_type_cache(rangetypid, TYPECACHE_RANGE_INFO);
  RangeBound lower;
  RangeBound upper;
  lower.val = from;
  lower.infinite = false;
  lower.inclusive = lower_inc;
  lower.lower = true;
  upper.val = to;
  upper.infinite = false;
  upper.inclusive = upper_inc;
  upper.lower = false;
  return make_range(typcache, &lower, &upper, false);
}

#endif /* #ifndef MEOS */

/*****************************************************************************/
