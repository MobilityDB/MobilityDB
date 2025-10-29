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
 * @brief General functions for set values composed of an ordered list of
 * distinct values
 */

#include "temporal/set.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/span.h"
#include "temporal/ttext_funcs.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_boxops.h"

#include <utils/jsonb.h>
#include <utils/numeric.h>
#include <pgtypes.h>

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensure that a set is of a given set type
 */
bool
ensure_set_isof_type(const Set *s, meosType settype)
{
  if (s->settype == settype)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "The set must be of type %s", meostype_name(settype));
  return false;
}

/**
 * @brief Ensure that a temporal number and a temporal box have the same span
 * type
 * @param[in] s1,s2 Sets
 */
bool
ensure_valid_set_set(const Set *s1, const Set *s2)
{
  VALIDATE_NOT_NULL(s1, false); VALIDATE_NOT_NULL(s2, false);
  if (s1->settype != s2->settype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Operation on different set types: %s, %s",
      meostype_name(s1->settype), meostype_name(s2->settype));
    return false;
  }
  if (spatialset_type(s1->settype) && 
      ! ensure_same_srid(spatialset_srid(s1), spatialset_srid(s2)))
    return false;
  return true;
}


/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Return the location of a value in a set using binary search
 * @details If the value is found, the index of the value is returned in the
 * output parameter. Otherwise, return a number encoding whether it is before,
 * between two values, or after the set.
 * For example, given a set composed of 3 values and a parameter
 * value, the result of the function is as follows:
 * @code
 *            0       1        2
 *            |       |        |
 * 1)    d^                            => loc = 0
 * 2)        d^                        => loc = 0
 * 3)            d^                    => loc = 1
 * 4)                    d^            => loc = 2
 * 5)                            d^    => loc = 3
 * @endcode
 * @param[in] s Set
 * @param[in] d Value
 * @param[out] loc Location
 * @return Return true if the value is contained in the set
 */
bool
set_find_value(const Set *s, Datum d, int *loc)
{
  int first = 0;
  int last = s->count - 1;
  int middle = 0; /* make compiler quiet */
  while (first <= last)
  {
    middle = (first + last) / 2;
    Datum d1 = SET_VAL_N(s, middle);
    int cmp = datum_cmp(d, d1, s->basetype);
    if (cmp == 0)
    {
      *loc = middle;
      return true;
    }
    if (cmp < 0)
      last = middle - 1;
    else
      first = middle + 1;
  }
  *loc = middle;
  return false;
}

/*****************************************************************************
 * Input/output functions in string format
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @param[in] settype Set type
 * @csqlfn #Set_in()
 */
Set *
set_in(const char *str, meosType settype)
{
  assert(str);
  return set_parse(&str, settype);
}

/**
 * @brief Return true if the base type value is output enclosed into quotes
 */
static bool
set_basetype_quotes(meosType type)
{
  /* Text values are already output with quotes in the #basetype_out function */
  if (type == T_TIMESTAMPTZ || spatial_basetype(type))
    return true;
  return false;
}

/**
 * @brief Return the output representation of a set given by a function
 */
char *
set_out_fn(const Set *s, int maxdd, outfunc value_out)
{
  assert(s);
  /* Ensure the validity of the arguments */
  if (! ensure_not_negative(maxdd))
    return NULL;

  char **strings = palloc(sizeof(char *) * s->count);
  size_t outlen = 0;
  for (int i = 0; i < s->count; i++)
  {
    strings[i] = value_out(SET_VAL_N(s, i), s->basetype, maxdd);
    outlen += strlen(strings[i]) + 1;
  }
  bool quotes = set_basetype_quotes(s->basetype);
  char *result = stringarr_to_string(strings, s->count, outlen, "", '{', '}',
    quotes, SPACES);
  return result;
}

/**
 * @ingroup meos_internal_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a set
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Set_out()
 */
char *
set_out(const Set *s, int maxdd)
{
  assert(s);
  /* Ensure the validity of the arguments */
  if (! ensure_not_negative(maxdd))
    return NULL;
  return set_out_fn(s, maxdd, &basetype_out);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @brief Return the size of a bounding box of a temporal type
 * @return On error return SIZE_MAX
 */
static size_t
set_bbox_size(meosType settype)
{
  assert(alphanumset_type(settype) || spatialset_type(settype));
  if (alphanumset_type(settype))
    return 0;
  return sizeof(STBox);
}

#if DEBUG_BUILD
/**
 * @brief Return a pointer to the bounding box of a temporal sequence
 */
void *
SET_BBOX_PTR(const Set *s)
{
  return (void *)( ((char *) s) + DOUBLE_PAD(sizeof(Set)) );
}

/**
 * @brief Return a pointer to the offsets array of a set
 */
size_t *
SET_OFFSETS_PTR(const Set *s)
{
  return (size_t *)( ((char *) s) + DOUBLE_PAD(sizeof(Set)) +
    DOUBLE_PAD(s->bboxsize) );
}

/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return the n-th value of a set
 * @param[in] s Set
 * @param[in] index Element number
 * @pre The argument @p index is less than the number of values in the set
 */
Datum
SET_VAL_N(const Set *s, int index)
{
  assert(s); assert(index >= 0);
  /* For base types passed by value */
  if (MEOS_FLAGS_GET_BYVAL(s->flags))
    return (SET_OFFSETS_PTR(s))[index];
  /* For base types passed by reference */
  return PointerGetDatum(
    /* start of data : start address + size of struct + size of bbox + */
    ((char *) s) + DOUBLE_PAD(sizeof(Set)) + DOUBLE_PAD(s->bboxsize) +
      /* offset array + offset */
      (sizeof(size_t) * s->maxcount) + (SET_OFFSETS_PTR(s))[index] );
}
#endif /* DEBUG_BUILD */

/**
 * @ingroup meos_internal_setspan_constructor
 * @brief Return a set from an array of values enabling the data structure
 * to expand
 * @details The memory structure depends on whether the value is passed by
 * value or by reference. For example, the memory structure of a set with two
 * values passed by value and passed by reference are, respectively, as follows
 *
 * @code
 * --------------------------------------------------------------------------
 * | Header | count |  maxcount | bboxsize | ( bbox )_X | Value_0 | Value_1 |
 * --------------------------------------------------------------------------
 * @endcode
 *
 * @code
 * ----------------------------------------------------------------------
 * | Header | count |  maxcount | bboxsize | ( bbox )_X | offset_0 | ...
 * ----------------------------------------------------------------------
 * -------------------------------------
 *  ... | offset_1 | Value_0 | Value_1 |
 * -------------------------------------
 * @endcode
 * where
 * - `Header` contains internal information (size, type identifiers, flags)
 * - `count` is the number of current values
 * - `max count` is the maximum number of values
 * - `bboxsize` is the size of the bounding box
 * - `bbox` is the bounding box and `X` are unused bytes added for double
 *   padding.
 * - `offset_i` are offsets from the begining of the struct for the values
 *
 * @param[in] values Array of values
 * @param[in] count Number of elements in the array
 * @param[in] maxcount Maximum number of elements in the array
 * @param[in] basetype Type of the values
 * @param[in] order True when the values should be ordered and duplicates
 * should be removed
 */
Set *
set_make_exp(const Datum *values, int count, int maxcount, meosType basetype,
  bool order)
{
  assert(values); assert(count > 0); assert(count <= maxcount);
  bool hasz = false;
  bool geodetic = false;
  // TODO Should we bypass the tests on tnpoint ?
  if (spatial_basetype(basetype) && basetype != T_NPOINT)
  {
    /* Ensure the spatial validity of the elements */
    int32_t srid = spatial_srid(values[0], basetype);
    int16 flags = spatial_flags(values[0], basetype);
    hasz = MEOS_FLAGS_GET_Z(flags);
    geodetic = MEOS_FLAGS_GET_GEODETIC(flags);
    /* Test the validity of the values */
    for (int i = 1; i < count; i++)
    {
      /* Test that the geometry is not empty */
      if (! ensure_same_srid(srid, spatial_srid(values[i], basetype)) ||
          ! ensure_same_spatial_dimensionality(flags,
            spatial_flags(values[i], basetype)) ||
          (geo_basetype(basetype) && ! 
            ensure_not_empty(DatumGetGserializedP(values[i]))))
        return NULL;
    }
  }

  /* Sort the values and remove duplicates */
  Datum *newvalues;
  int newcount;
  if (order && count > 1)
  {
  /* Sort the values and remove duplicates */
    newvalues = palloc(sizeof(Datum) * count);
    memcpy(newvalues, values, sizeof(Datum) * count);
    datumarr_sort(newvalues, count, basetype);
    newcount = datumarr_remove_duplicates(newvalues, count, basetype);
  }
  else
  {
    newvalues = (Datum *) values;
    newcount = count;
  }

  /* Get the bounding box size */
  meosType settype = basetype_settype(basetype);
  size_t bboxsize = DOUBLE_PAD(set_bbox_size(settype));

  /* Determine whether the values are passed by value or by reference  */
  int16 typlen;
  bool typbyval = basetype_byvalue(basetype);
  if (typbyval)
    /* For base values passed by value */
    typlen = DOUBLE_PAD(sizeof(Datum));
  else
    /* For base values passed by reference */
    typlen = basetype_length(basetype);

  /* Compute the size of the set for values passed by reference */
  size_t values_size = 0;
  if (! typbyval)
  {
    if (typlen == -1)
    {
      for (int i = 0; i < newcount; i++)
        /* VARSIZE_ANY is used for oblivious data alignment, see postgres.h */
        values_size += DOUBLE_PAD(VARSIZE_ANY(DatumGetPointer(newvalues[i])));
    }
    else
      values_size = (size_t) DOUBLE_PAD(typlen) * newcount;
  }

#if MEOS
  /* Compute the total size for maxcount elements as a proportion of the size
   * of the count elements provided. Note that this is only an INITIAL
   * ESTIMATION. The functions adding elements to a set must verify BOTH
   * the maximum number of elements AND the remaining space for adding an
   * additional variable-length element of arbitrary size */
  if (count != maxcount)
    values_size = (double) values_size * (double) maxcount / (double) count;
#endif /* MEOS */

  /* Total size of the struct */
  size_t memsize = DOUBLE_PAD(sizeof(Set)) + DOUBLE_PAD(bboxsize) +
    (sizeof(size_t) * maxcount) + values_size;

  /* Create the Set */
  Set *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  MEOS_FLAGS_SET_BYVAL(result->flags, typbyval);
  MEOS_FLAGS_SET_ORDERED(result->flags, true);
  if (spatial_basetype(basetype))
  {
    MEOS_FLAGS_SET_X(result->flags, true);
    MEOS_FLAGS_SET_Z(result->flags, hasz);
    MEOS_FLAGS_SET_GEODETIC(result->flags, geodetic);
  }
  result->count = newcount;
  result->maxcount = maxcount;
  result->settype = settype;
  result->basetype = basetype;
  result->bboxsize = (int16) bboxsize;
  /* Copy the array of values */
  if (typbyval)
  {
    for (int i = 0; i < newcount; i++)
      (SET_OFFSETS_PTR(result))[i] = newvalues[i];
  }
  else
  {
    /* Store the composing values */
    size_t pdata = DOUBLE_PAD(sizeof(Set)) + DOUBLE_PAD(bboxsize) +
      sizeof(size_t) * maxcount;
    size_t pos = 0;
    for (int i = 0; i < newcount; i++)
    {
      /* VARSIZE_ANY is used for oblivious data alignment, see postgres.h */
      size_t size_elem = (typlen == -1) ?
        VARSIZE_ANY(newvalues[i]) : (uint32) typlen;
      memcpy(((char *) result) + pdata + pos, DatumGetPointer(newvalues[i]),
        size_elem);
      (SET_OFFSETS_PTR(result))[i] = pos;
      pos += DOUBLE_PAD(size_elem);
    }
  }

  /* Compute the bounding box */
  if (bboxsize != 0)
    spatialarr_set_bbox(newvalues, basetype, newcount, SET_BBOX_PTR(result));

  if (order && count > 1)
    pfree(newvalues);
  return result;
}

/**
 * @ingroup meos_internal_setspan_constructor
 * @brief Return a set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements in the array
 * @param[in] basetype Type of the values
 * @param[in] order True when the values should be ordered and duplicates
 * should be removed
 * @csqlfn #Set_constructor()
 */
Set *
set_make(const Datum *values, int count, meosType basetype, bool order)
{
  assert(values); assert(count > 0);
  return set_make_exp(values, count, count, basetype, order);
}

/**
 * @ingroup meos_internal_setspan_constructor
 * @brief Return a set from the array of values and free the input array
 * after the creation
 * @param[in] values Array of values
 * @param[in] count Number of elements in the array
 * @param[in] basetype Type of the values
 * @param[in] order True when the values should be ordered and duplicates
 * should be removed
 */
Set *
set_make_free(Datum *values, int count, meosType basetype, bool order)
{
  assert(values); assert(count >= 0);
  if (! count)
  {
    pfree(values);
    return NULL;
  }
  Set *result = set_make_exp(values, count, count, basetype, order);
  pfree(values);
  return result;
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a copy of a set
 * @param[in] s Set
 */
Set *
set_copy(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(s, NULL);
  Set *result = palloc(VARSIZE(s));
  memcpy(result, s, VARSIZE(s));
  return result;
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Convert a value into a set
 * @param[in] value Value
 * @param[in] basetype Type of the value
 * @csqlfn #Value_to_set()
 */
Set *
value_set(Datum value, meosType basetype)
{
  return set_make_exp(&value, 1, 1, basetype, ORDER_NO);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert an integer set into a float set
 * @param[in] s Set
 * @csqlfn #Intset_to_floatset()
 */
Set *
intset_to_floatset(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, NULL);
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = Float8GetDatum((double) DatumGetInt32(SET_VAL_N(s, i)));
  /* All distinct integers will yield distinct floats */
  return set_make_free(values, s->count, T_FLOAT8, ORDER_NO);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a float set into an integer set
 * @param[in] s Set
 * @csqlfn #Floatset_to_intset()
 */
Set *
floatset_to_intset(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, NULL);
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = Int32GetDatum((int) DatumGetFloat8(SET_VAL_N(s, i)));
  /* Two distinct floats can yield the same integer */
  return set_make_free(values, s->count, T_INT4, ORDER);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a date set into a timestamptz set
 * @param[in] s Set
 * @csqlfn #Dateset_to_tstzset()
 */
Set *
dateset_to_tstzset(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, NULL);
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = TimestampTzGetDatum(date_to_timestamptz(DatumGetDateADT(
      SET_VAL_N(s, i))));
  /* All distinct dates will yield distinct timestamptz */
  return set_make_free(values, s->count, T_TIMESTAMPTZ, ORDER_NO);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a timestamptz set into a date set
 * @param[in] s Set
 * @csqlfn #Tstzset_to_dateset()
 */
Set *
tstzset_to_dateset(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, NULL);
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = DateADTGetDatum(timestamptz_to_date(DatumGetTimestampTz(
      SET_VAL_N(s, i))));
  /* Two distinct timestamptz can yield the same date */
  return set_make_free(values, s->count, T_DATE, ORDER);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return the size in bytes of a set
 * @param[in] s Set
 * @csqlfn #Set_mem_size()
 */
int
set_mem_size(const Set *s)
{
  VALIDATE_NOT_NULL(s, -1);
  return (int) VARSIZE(s);
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the number of values of a set
 * @return On error return -1
 * @param[in] s Set
 * @csqlfn #Set_num_values()
 */
int
set_num_values(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(s, -1);
  return s->count;
}

/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return a copy of the start value of a set
 * @param[in] s Set
 */
Datum
set_start_value(const Set *s)
{
  assert(s);
  return MEOS_FLAGS_GET_BYVAL(s->flags) ? SET_VAL_N(s, 0) :
    datum_copy(SET_VAL_N(s, 0), s->basetype);
}

/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return a copy of the end value of a set
 * @param[in] s Set
 */
Datum
set_end_value(const Set *s)
{
  assert(s);
  return MEOS_FLAGS_GET_BYVAL(s->flags) ? SET_VAL_N(s, s->count - 1) :
    datum_copy(SET_VAL_N(s, s->count - 1), s->basetype);
}

/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return in the last argument a copy of the n-th value of a set
 * @param[in] s Set
 * @param[in] n Number (1-based)
 * @param[out] result Value
 * @return Return true if the value is found
 * @csqlfn #Set_value_n()
 */
bool
set_value_n(const Set *s, int n, Datum *result)
{
  assert(s); assert(result);
  if (n < 1 || n > s->count)
    return false;
  *result = MEOS_FLAGS_GET_BYVAL(s->flags) ? SET_VAL_N(s, n - 1) :
    datum_copy(SET_VAL_N(s, n - 1), s->basetype);
  return true;
}

/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return the array of (pointers to the) values of a set
 * @param[in] s Set
 * @csqlfn #Set_values()
 */
Datum *
set_vals(const Set *s)
{
  assert(s);
  Datum *result = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = SET_VAL_N(s, i);
  return result;
}

/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return the array of (copies of) values of a set
 * @param[in] s Set
 * @csqlfn #Set_values()
 */
Datum *
set_values(const Set *s)
{
  assert(s);
  Datum *result = palloc(sizeof(Datum) * s->count);
  bool byval = MEOS_FLAGS_GET_BYVAL(s->flags);
  for (int i = 0; i < s->count; i++)
    result[i] = byval ? SET_VAL_N(s, i) : datum_copy(SET_VAL_N(s, i),
      s->basetype);
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/
 
/**
 * @ingroup meos_setspan_transf
 * @brief Return a set with the precision of the values set to a number of
 * decimal places
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 */
Set *
set_round(const Set *s, int maxdd)
{
  VALIDATE_NOT_NULL(s, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  Datum *values = palloc(sizeof(Datum) * s->count);
  Datum size = Int32GetDatum(maxdd);
  datum_func2 func = round_fn(s->basetype);
  for (int i = 0; i < s->count; i++)
    values[i] = func(SET_VAL_N(s, i), size);
  return set_make_free(values, s->count, s->basetype, ORDER);
}

/*****************************************************************************/

#if MEOS
/**
 * @ingroup meos_internal_setspan_transf
 * @brief Return a copy of a set ordered, without duplicates, and without any
 * additional free space
 * @param[in] s Set
 */
Set *
set_compact(const Set *s)
{
  assert(s);
  /* Collect the values which may be not ordered */
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = SET_VAL_N(s, i);

  Set *result = set_make_exp(values, s->count, s->count, s->basetype, ORDER_NO);
  pfree(values);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Return a float set obtained by applying a function to each element
 */
Set *
floatset_func(const Set *s, Datum (*func)(Datum))
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, NULL);
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = func(SET_VAL_N(s, i));
  Set *result = set_make_exp(values, s->count, s->count, T_FLOAT8, ORDER);
  pfree(values);
  return result;
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a float set rounded down to the nearest integer
 * @csqlfn #Floatset_floor()
 */
Set *
floatset_floor(const Set *s)
{
  return floatset_func(s, &datum_floor);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a float set rounded up to the nearest integer
 * @csqlfn #Floatset_ceil()
 */
Set *
floatset_ceil(const Set *s)
{
  return floatset_func(s, &datum_ceil);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Convert a float set with the values into degrees
 * @param[in] s Set
 * @param[in] normalize True when the result must be normalized
 * @csqlfn #Floatset_degrees()
 */
Set *
floatset_degrees(const Set *s, bool normalize)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, NULL);
  Set *result = set_copy(s);
  for (int i = 0; i < s->count; i++)
    (SET_OFFSETS_PTR(result))[i] = datum_degrees(SET_VAL_N(s, i), normalize);
  return result;
}

/**
 * @ingroup meos_setspan_transf
 * @brief Convert a float set with the values into radians
 * @param[in] s Set
 * @csqlfn #Floatset_radians()
 */
Set *
floatset_radians(const Set *s)
{
  return floatset_func(s, &datum_radians);
}

/*****************************************************************************/

/**
 * @brief Return a float set obtained by applying a function to each element
 */
Set *
textset_func(const Set *s, Datum (*func)(Datum))
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, NULL);
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = func(SET_VAL_N(s, i));
  Set *result = set_make_exp(values, s->count, s->count, T_TEXT, ORDER);
  pfree_array((void **) values, s->count);
  return result;
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a text set transformed to lowercase
 * @param[in] s Set
 * @csqlfn #Textset_lower()
 */
Set *
textset_lower(const Set *s)
{
  return textset_func(s, &datum_lower);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a text set transformed to uppercase
 * @param[in] s Set
 * @csqlfn #Textset_upper()
 */
Set *
textset_upper(const Set *s)
{
  return textset_func(s, &datum_upper);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a text set transformed to initcap
 * @param[in] s Set
 * @csqlfn #Textset_initcap()
 */
Set *
textset_initcap(const Set *s)
{
  return textset_func(s, &datum_initcap);
}

/**
 * @brief Return the concatenation of a text set and a set (internal function)
 * @param[in] s Set
 * @param[in] txt Text
 * @param[in] invert True when the arguments must be inverted
 */
Set *
textcat_textset_text_common(const Set *s, const text *txt, bool invert)
{
  assert(s); assert(txt); assert((s->settype == T_TEXTSET));
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = invert ?
      datum_textcat(PointerGetDatum(txt), SET_VAL_N(s, i)) :
      datum_textcat(SET_VAL_N(s, i), PointerGetDatum(txt));
  Set *result = set_make(values, s->count, T_TEXT, ORDER_NO);
  pfree_array((void **) values, s->count);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_setspan_transf
 * @brief Return a number set shifted and/or scaled by two values
 * @param[in] s Set
 * @param[in] shift Value for shifting the elements
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 */
Set *
numset_shift_scale(const Set *s, Datum shift, Datum width, bool hasshift,
  bool haswidth)
{
  meosType type = s->basetype;
  /* Ensure the validity of the arguments */
  VALIDATE_NUMSET(s, NULL);
  if (! ensure_one_true(hasshift, haswidth) ||
      (haswidth && ! ensure_positive_datum(width, type)))
    return NULL;

  /* Copy the input set to the output set */
  Set *result = set_copy(s);

  /* Set the first and last instants */
  Datum lower, lower1, upper, upper1;
  lower = lower1 = SET_VAL_N(s, 0);
  upper = upper1 = SET_VAL_N(s, s->count - 1);
  span_bounds_shift_scale_value(shift, width, type, hasshift, haswidth,
    &lower1, &upper1);
  (SET_OFFSETS_PTR(result))[0] = lower1;
  (SET_OFFSETS_PTR(result))[s->count - 1] = upper1;
  if (s->count > 1)
  {
    /* Shift and/or scale from the second to the penultimate values */
    Datum delta = 0;    /* make compiler quiet */
    double scale = 1.0; /* make compiler quiet */
    if (hasshift)
      delta = datum_sub(lower1, lower, type);
    if (haswidth)
      scale = datum_double(datum_sub(upper1, lower1, type), type) /
              datum_double(datum_sub(upper, lower, type), type);
    for (int i = 1; i < s->count - 1; i++)
    {
      if (hasshift)
        (SET_OFFSETS_PTR(result))[i] = datum_add((SET_OFFSETS_PTR(result))[i],
          delta, type);
      if (haswidth)
        (SET_OFFSETS_PTR(result))[i] = datum_add(lower1, double_datum(
          datum_double(datum_sub(SET_VAL_N(result, i), lower1, type), type) *
          scale, type), type);
    }
  }
  return result;
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a timestamptz set shifted and/or scaled by two intervals
 * @param[in] s Set
 * @param[in] shift Interval to shift the elements, may be NULL
 * @param[in] duration Duation of the result, may be NULL
 * @csqlfn #Tstzset_shift(), #Tstzset_scale(), #Tstzset_shift_scale()
 */
Set *
tstzset_shift_scale(const Set *s, const Interval *shift,
  const Interval *duration)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, NULL);
  if (! ensure_one_not_null((void *) shift, (void *) duration) ||
      (duration && ! ensure_positive_duration(duration)))
    return NULL;

  Set *result = set_copy(s);
  /* Set the first and last instants */
  TimestampTz lower, lower1, upper, upper1;
  lower = lower1 = DatumGetTimestampTz(SET_VAL_N(s, 0));
  upper = upper1 = DatumGetTimestampTz(SET_VAL_N(s, s->count - 1));
  span_bounds_shift_scale_time(shift, duration, &lower1, &upper1);
  (SET_OFFSETS_PTR(result))[0] = TimestampTzGetDatum(lower1);
  (SET_OFFSETS_PTR(result))[s->count - 1] = TimestampTzGetDatum(upper1);
  if (s->count > 1)
  {
    /* Shift and/or scale from the second to the penultimate instant */
    TimestampTz delta = 0; /* make compiler quiet */
    double scale = 0; /* make compiler quiet */
    if (shift)
      delta = lower1 - lower;
    if (duration)
      scale = (double) (upper1 - lower1) / (double) (upper - lower);
    for (int i = 1; i < s->count - 1; i++)
    {
      if (shift)
        (SET_OFFSETS_PTR(result))[i] += delta;
      if (duration)
        (SET_OFFSETS_PTR(result))[i] = lower1 +
          (TimestampTz) ((SET_VAL_N(result, i) - lower1) * scale);
    }
  }
  return result;
}

/*****************************************************************************
 * Unnest function
 *****************************************************************************/

/**
 * @brief Create the initial state that persists across multiple calls of the
 * function
 * @param[in] set Set value
 */
SetUnnestState *
set_unnest_state_make(const Set *set)
{
  SetUnnestState *state = palloc0(sizeof(SetUnnestState));
  /* Fill in state */
  state->done = false;
  state->i = 0;
  state->count = set->count;
  state->values = set_values(set);
  state->set = set_copy(set);
  return state;
}

/**
 * @brief Increment the current state to the next unnest value
 * @param[in] state State to increment
 */
void
set_unnest_state_next(SetUnnestState *state)
{
  if (! state || state->done)
    return;
  /* Move to the next bin */
  state->i++;
  if (state->i == state->count)
    state->done = true;
  return;
}

/*****************************************************************************
 * Comparison functions for defining B-tree index
 *****************************************************************************/

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the two sets are equal
 * @param[in] s1,s2 Sets
 * @note The function #set_cmp() is not used to increase efficiency
 * @csqlfn #Set_eq()
 */
bool
set_eq(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2))
    return false;

  if (s1->count != s2->count)
    return false;
  /* s1 and s2 have the same number of values */
  for (int i = 0; i < s1->count; i++)
    if (datum_ne(SET_VAL_N(s1, i), SET_VAL_N(s2, i), s1->basetype))
      return false;
  /* All values of the two sets are equal */
  return true;
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first set is not equal to the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Set_ne()
 */
inline bool
set_ne(const Set *s1, const Set *s2)
{
  return ! set_eq(s1, s2);
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first set is less
 * than, equal to, or greater than the second one
 * @param[in] s1,s2 Sets
 * @return On error return @p INT_MAX
 * @note Function used for B-tree comparison
 * @csqlfn #Set_cmp()
 */
int
set_cmp(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2))
    return INT_MAX;
  int count = Min(s1->count, s2->count);
  int result = 0;
  for (int i = 0; i < count; i++)
  {
    result = datum_cmp(SET_VAL_N(s1, i), SET_VAL_N(s2, i), s1->basetype);
    if (result)
      break;
  }
  /* The first count times of the two Set are equal */
  if (! result)
  {
    if (count < s1->count) /* s1 has more values than s2 */
      result = 1;
    else if (count < s2->count) /* s2 has more values than s1 */
      result = -1;
    else
      result = 0;
  }
  return result;
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first set is less than the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Set_lt()
 */
inline bool
set_lt(const Set *s1, const Set *s2)
{
  return set_cmp(s1, s2) < 0;
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first set is less than or equal to the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Set_le()
 */
inline bool
set_le(const Set *s1, const Set *s2)
{
  return set_cmp(s1, s2) <= 0;
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first set is greater than the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Set_gt()
 */
inline bool
set_gt(const Set *s1, const Set *s2)
{
  return set_cmp(s1, s2) > 0;
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first set is greater than or equal to the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Set_ge()
 */
inline bool
set_ge(const Set *s1, const Set *s2)
{
  return set_cmp(s1, s2) >= 0;
}

/*****************************************************************************
 * Functions for defining hash index
 * The function reuses PostgreSQL approach for array types for combining the
 * hash of the elements.
 *****************************************************************************/

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the 32-bit hash of a set
 * @param[in] s Set
 * @csqlfn #Set_hash
 */
uint32
set_hash(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(s, INT_MAX);
  uint32 result = 1;
  for (int i = 0; i < s->count; i++)
  {
    uint32 value_hash = datum_hash(SET_VAL_N(s, i), s->basetype);
    result = (result << 5) - result + value_hash;
  }
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the 64-bit hash of a set using a seed
 * @param[in] s Set
 * @param[in] seed Seed
 * @csqlfn #Set_hash_extended
 */
uint64
set_hash_extended(const Set *s, uint64 seed)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(s, LONG_MAX);
  uint64 result = 1;
  for (int i = 0; i < s->count; i++)
  {
    uint64 value_hash = datum_hash_extended(SET_VAL_N(s, i), s->basetype, seed);
    result = (result << 5) - result + value_hash;
  }
  return result;
}

/*****************************************************************************/
