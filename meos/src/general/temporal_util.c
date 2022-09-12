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
 * @brief General utility functions for temporal types.
 */

#include "general/temporal_util.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h"
#include "general/pg_call.h"
#include "general/doublen.h"
#include "general/temporal_parser.h"
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
 * Return true if the first value is less than the second one
 */
int
datum_cmp(Datum l, Datum r, mobdbType type)
{
  return datum_cmp2(l, r, type, type);
}

/**
 * Return true if the values are equal
 */
bool
datum_eq(Datum l, Datum r, mobdbType type)
{
  return datum_eq2(l, r, type, type);
}

/**
 * Return true if the values are different
 */
bool
datum_ne(Datum l, Datum r, mobdbType type)
{
  return ! datum_eq2(l, r, type, type);
}

/**
 * Return true if the first value is less than the second one
 */
bool
datum_lt(Datum l, Datum r, mobdbType type)
{
  return datum_lt2(l, r, type, type);
}

/**
 * Return true if the first value is less than or equal to the second one
 */
bool
datum_le(Datum l, Datum r, mobdbType type)
{
  return datum_eq2(l, r, type, type) || datum_lt2(l, r, type, type);
}

/**
 * Return true if the first value is greater than the second one
 */
bool
datum_gt(Datum l, Datum r, mobdbType type)
{
  return datum_lt2(r, l, type, type);
}

/**
 * Return true if the first value is greater than or equal to the second one
 */
bool
datum_ge(Datum l, Datum r, mobdbType type)
{
  return datum_eq2(l, r, type, type) || datum_lt2(r, l, type, type);
}

/*****************************************************************************/

/*
 * Version of the functions where the types of both arguments may be different
 * but compatible, e.g., integer and float
 */

/**
 * Return true if the first value is less than the second one
 */
int
datum_cmp2(Datum l, Datum r, mobdbType typel, mobdbType typer)
{
  ensure_span_basetype(typel);
  if (typel != typer)
    ensure_span_basetype(typer);
  if (typel == T_TIMESTAMPTZ && typer == T_TIMESTAMPTZ)
    return timestamptz_cmp_internal(DatumGetTimestampTz(l),
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
 * Return true if the values are equal even if their type is not the same
 */
bool
datum_eq2(Datum l, Datum r, mobdbType typel, mobdbType typer)
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
#if NPOINT
  if (typel == T_NPOINT && typel == typer)
    return npoint_eq(DatumGetNpointP(l), DatumGetNpointP(r));
#endif
  elog(ERROR, "unknown datum_eq2 function for base type: %d", typel);
}

/**
 * Return true if the values are different
 */
bool
datum_ne2(Datum l, Datum r, mobdbType typel, mobdbType typer)
{
  return ! datum_eq2(l, r, typel, typer);
}

/**
 * Return true if the first value is less than the second one
 */
bool
datum_lt2(Datum l, Datum r, mobdbType typel, mobdbType typer)
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
    return gserialized_cmp(DatumGetGserializedP(l),
      DatumGetGserializedP(r)) < 0;
  if (typel == T_GEOGRAPHY && typel == typer)
    return gserialized_cmp(DatumGetGserializedP(l),
      DatumGetGserializedP(r)) < 0;
#if NPOINT
  if (typel == T_NPOINT && typel == typer)
    return npoint_lt(DatumGetNpointP(l), DatumGetNpointP(r));
#endif
  elog(ERROR, "unknown datum_lt2 function for base type: %d", typel);
}


/**
 * Return true if the first value is less than or equal to the second one
 */
bool
datum_le2(Datum l, Datum r, mobdbType typel, mobdbType typer)
{
  return datum_eq2(l, r, typel, typer) || datum_lt2(l, r, typel, typer);
}

/**
 * Return true if the first value is greater than the second one
 */
bool
datum_gt2(Datum l, Datum r, mobdbType typel, mobdbType typer)
{
  return datum_lt2(r, l, typer, typel);
}

/**
 * Return true if the first value is greater than or equal to the second one
 */
bool
datum_ge2(Datum l, Datum r, mobdbType typel, mobdbType typer)
{
  return datum_eq2(l, r, typel, typer) || datum_gt2(l, r, typel, typer);
}

/*****************************************************************************/

/**
 * Return a Datum true if the values are equal
 */
Datum
datum2_eq2(Datum l, Datum r, mobdbType typel, mobdbType typer)
{
  return BoolGetDatum(datum_eq2(l, r, typel, typer));
}

/**
 * Return a Datum true if the values are different
 */
Datum
datum2_ne2(Datum l, Datum r, mobdbType typel, mobdbType typer)
{
  return BoolGetDatum(datum_ne2(l, r, typel, typer));
}

/**
 * Return a Datum true if the first value is less than the second one
 */
Datum
datum2_lt2(Datum l, Datum r, mobdbType typel, mobdbType typer)
{
  return BoolGetDatum(datum_lt2(l, r, typel, typer));
}

/**
 * Return a Datum true if the first value is less than or equal to the second one
 */
Datum
datum2_le2(Datum l, Datum r, mobdbType typel, mobdbType typer)
{
  return BoolGetDatum(datum_le2(l, r, typel, typer));
}

/**
 * Return a Datum true if the first value is greater than the second one
 */
Datum
datum2_gt2(Datum l, Datum r, mobdbType typel, mobdbType typer)
{
  return BoolGetDatum(datum_gt2(l, r, typel, typer));
}

/**
 * Return a Datum true if the first value is greater than or equal to the second one
 */
Datum
datum2_ge2(Datum l, Datum r, mobdbType typel, mobdbType typer)
{
  return BoolGetDatum(datum_ge2(l, r, typel, typer));
}

/*****************************************************************************
 * Arithmetic functions on datums
 * N.B. The validity of the types must be done in the calling function.
 *****************************************************************************/

/**
 * Return the addition of the two numbers
 */
Datum
datum_add(Datum l, Datum r, mobdbType typel, mobdbType typer)
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
datum_sub(Datum l, Datum r, mobdbType typel, mobdbType typer)
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
datum_mult(Datum l, Datum r, mobdbType typel, mobdbType typer)
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
datum_div(Datum l, Datum r, mobdbType typel, mobdbType typer)
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
datum_copy(Datum value, mobdbType basetype)
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
 * Convert a number to a double
 */
double
datum_double(Datum d, mobdbType basetype)
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
datum_sort_cmp(const Datum *l, const Datum *r, const mobdbType *type)
{
  Datum x = *l;
  Datum y = *r;
  mobdbType t = *type;
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
  return timestamptz_cmp_internal(x, y);
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
  return timestamptz_cmp_internal((*l)->t, (*r)->t);
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
datumarr_sort(Datum *values, int count, mobdbType type)
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
  qsort(triples, count, sizeof(double3), (qsort_comparator) &double3_cmp);
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
datumarr_remove_duplicates(Datum *values, int count, mobdbType type)
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
 * Text and binary string functions
 *****************************************************************************/

/**
 * Convert a C binary string into a bytea
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
 * Convert a C string into a text value
 *
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
 * Comparison function for text values
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

#if 0 /* not used */
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
#endif /* not used */

/*****************************************************************************
 * Input/output PostgreSQL functions
 *****************************************************************************/

/**
 * Call input function of the base type
 */
Datum
#if NPOINT
basetype_input(const char *str, mobdbType basetype, bool end)
#else
basetype_input(const char *str, mobdbType basetype, bool end __attribute__((unused)))
#endif
{
  ensure_temporal_basetype(basetype);
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
      break;
  }
}

/**
 * Call output function of the base type
 */
char *
basetype_output(mobdbType basetype, Datum value, Datum arg)
{
  ensure_temporal_basetype(basetype);
  switch (basetype)
  {
    case T_TIMESTAMPTZ:
      return pg_timestamptz_out(DatumGetTimestampTz(value));
    case T_BOOL:
      return bool_out(DatumGetBool(value));
    case T_INT4:
      return int4_out(DatumGetInt32(value));
  #if 0 /* not used */
    case T_INT8:
      return int8_out(DatumGetInt64(value));
  #endif /* not used */
    case T_FLOAT8:
      return float8_out(DatumGetFloat8(value), DatumGetInt32(arg));
    case T_TEXT:
      return text2cstring(DatumGetTextP(value));
    case T_GEOMETRY:
    return gserialized_out(DatumGetGserializedP(value));
    case T_GEOGRAPHY:
      return gserialized_geog_out(DatumGetGserializedP(value));
#if NPOINT
    case T_NPOINT:
      return npoint_out(DatumGetNpointP(value), DatumGetInt32(arg));
#endif
    default: /* Error! */
      elog(ERROR, "Unknown base type: %d", basetype);
      break;
  }
}

/*****************************************************************************/
