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
 * @brief General functions for set values composed of an ordered list of
 * distinct values.
 */

#include "general/set.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/type_out.h"
#include "general/type_parser.h"
#include "general/type_util.h"
#include "point/tpoint_out.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint_boxops.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_accessor
 * @brief Return the location of a value in a set using binary search.
 *
 * If the value is found, the index of the value is returned in the output
 * parameter. Otherwise, return a number encoding whether it is before, between
 * two values, or after the set.
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
 *
 * @param[in] s Set
 * @param[in] d Value
 * @param[out] loc Location
 * @result Return true if the value is contained in the set
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
    Datum d1 = set_val_n(s, middle);
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

#if 0 /* not used */
/**
 * @ingroup libmeos_internal_setspan_accessor
 * @brief Return the location of a value in a ranked set (which is unordered)
 * using sequential search.
 * @note Contrary to function `set_find_value`, if the value is not found the
 * returned location is always 0.
 *
 * @param[in] s Set
 * @param[in] d Value
 * @param[out] loc Location of the value if found
 * @result Return true if the value is contained in the vecctor
 */
bool
rset_find_value(const Set *s, Datum d, int *loc)
{
  for (int i = 0; i < s->count; i++)
  {
    Datum d1 = set_val_n(s, i);
    if (datum_eq(d, d1, s->basetype))
    {
      *loc = i;
      return true;
    }
  }
  *loc = 0;
  return false;
}
#endif /* not used */

/*****************************************************************************
 * Input/output functions in string format
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation.
 */
Set *
set_in(const char *str, meosType settype)
{
  return set_parse(&str, settype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation.
 */
Set *
intset_in(const char *str)
{
  return set_parse(&str, T_INT4);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation.
 */
Set *
bigintset_in(const char *str)
{
  return set_parse(&str, T_INT8);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation.
 */
Set *
floatset_in(const char *str)
{
  return set_parse(&str, T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation.
 */
Set *
tstzset_in(const char *str)
{
  return set_parse(&str, T_TSTZSET);
}
#endif /* MEOS */


/**
 * @ingroup libmeos_setspan_inout
 * @brief Return true if the base type value is output enclosed into quotes.
 */
static bool
set_basetype_quotes(meosType type)
{
  /* Text values are output with quotes in the `basetype_out` function */
  if (type == T_TIMESTAMPTZ || spatial_basetype(type))
    return true;
  return false;
}

/**
 * @ingroup libmeos_internal_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a set.
 */
char *
set_out_fn(const Set *s, int maxdd, outfunc value_out)
{
  char **strings = palloc(sizeof(char *) * s->count);
  size_t outlen = 0;
  for (int i = 0; i < s->count; i++)
  {
    Datum d = set_val_n(s, i);
    strings[i] = value_out(d, s->basetype, maxdd);
    outlen += strlen(strings[i]) + 1;
  }
  bool quotes = set_basetype_quotes(s->basetype);
  return stringarr_to_string(strings, s->count, outlen, "", '{', '}', quotes,
    SPACES);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a set.
 */
char *
set_out(const Set *s, int maxdd)
{
  return set_out_fn(s, maxdd, &basetype_out);
}

/**
 * @ingroup libmeos_spanset_inout
 * @brief Return the Well-Known Text (WKT) representation a geoset.
 * @sqlfunc asText()
 */
char *
geoset_as_text(const Set *set, int maxdd)
{
  return set_out_fn(set, maxdd, &wkt_out);
}

/**
 * @ingroup libmeos_spanset_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation a geoset.
 * @sqlfunc asEWKT()
 */
char *
geoset_as_ewkt(const Set *set, int maxdd)
{
  return set_out_fn(set, maxdd, &ewkt_out);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * Return the size of a bounding box of a temporal type
 */
static size_t
set_bbox_size(meosType settype)
{
  if (alphanumset_type(settype))
    return 0;
  if (spatialset_type(settype))
    return sizeof(STBox);
  elog(ERROR, "unknown set_bbox_size function for set type: %d",
    settype);
}

/**
 * Set a bounding box from an array of set values
 *
 * @param[in] values Values
 * @param[in] basetype Type of the values
 * @param[in] count Number of elements in the array
 * @param[out] box Bounding box
 */
void
valuearr_compute_bbox(const Datum *values, meosType basetype, int count,
  void *box)
{
  /* Currently, only geo set types have bounding box */
  ensure_set_basetype(basetype);
  if (alphanum_basetype(basetype))
    ;
  else if (geo_basetype(basetype))
    geoarr_set_stbox(values, count, (STBox *) box);
#if NPOINT
  else if (basetype == T_NPOINT)
   npointarr_set_stbox(values, count, (STBox *) box);
#endif
  else
    elog(ERROR, "unknown set type for computint bounding box: %d", basetype);
  return;
}

/**
 * Set a bounding box from an array of set values
 *
 * @param[in] set Set
 * @param[in] d Value to append to the set
 * @param[in] basetype Type of the value
 * @param[out] box Bounding box
 */
void
set_expand_bbox(Datum d, meosType basetype, void *box)
{
  /* Currently, only geo set types have bounding box */
  ensure_set_basetype(basetype);
  if (alphanum_basetype(basetype))
    ;
  else if (geo_basetype(basetype))
  {
    STBox box1;
    geo_set_stbox(DatumGetGserializedP(d), &box1);
    stbox_expand(&box1, (STBox *) box);
  }
#if NPOINT
  else if (basetype == T_NPOINT)
  {
    STBox box1;
    npoint_set_stbox(DatumGetNpointP(d), &box1);
    stbox_expand(&box1, (STBox *) box);
  }
#endif
  else
    elog(ERROR, "unknown set type for expanding bounding box: %d", basetype);
  return;
}

/**
 * Return a pointer to the bounding box of a temporal sequence
 */
void *
set_bbox_ptr(const Set *s)
{
  return (void *)( ((char *) s) + double_pad(sizeof(Set)) );
}

/**
 * Return a pointer to the offsets array of a set
 */
size_t *
set_offsets_ptr(const Set *s)
{
  return (size_t *)( ((char *) s) + double_pad(sizeof(Set)) +
    double_pad(s->bboxsize) );
}

/**
 * @ingroup libmeos_internal_setspan_accessor
 * @brief Return the n-th value of a set
 * @pre The argument @p index is less than the number of values in the set
 */
Datum
set_val_n(const Set *s, int index)
{
  assert(index >= 0);
  /* For base types passed by value */
  if (MOBDB_FLAGS_GET_BYVAL(s->flags))
    return (set_offsets_ptr(s))[index];
  /* For base types passed by reference */
  return PointerGetDatum(
    /* start of data : start address + size of struct + size of bbox + */
    ((char *) s) + double_pad(sizeof(Set)) + double_pad(s->bboxsize) +
      /* offset array + offset */
      (sizeof(size_t) * s->maxcount) + (set_offsets_ptr(s))[index] );
}

/**
 * @ingroup libmeos_internal_setspan_constructor
 * @brief Construct a set from an array of values.
 *
 * The memory structure depends on whether the value is passed by value or
 * by reference. For example, the memory structure of a set with 2 values
 * passed by value and passed by reference are, respectively, as follows
 *
 * @code
 * ------------------------------------------------------------
 * Header | count | bboxsize | ( bbox )_X | Value_0 | Value_1 |
 * ------------------------------------------------------------
 * @endcode
 *
 * @code
 * ---------------------------------------------------------------------
 * | Header | count | bboxsize | ( bbox )_X | offset_0 | offset_1 | ...
 * ---------------------------------------------------------------------
 * --------------------------
 *  ... | Value_0 | Value_1 |
 * --------------------------
 * @endcode
 * where
 * - `Header` contains internal information (size, type identifiers, flags)
 * - `count` is the number of values
 * - `bboxsize` is the size of the bounding box
 * - `bbox` is the bounding box and `X` are unused bytes added for double
 *   padding.
 * - `offset_i` are offsets from the begining of the struct for the values
 *
 * @param[in] values Array of values
 * @param[in] count Number of elements in the array
 * @param[in] maxcount Maximum number of elements in the array
 * @param[in] basetype Base type
 * @param[in] ordered True for ordered sets
 * @sqlfunc intset(), bigintset(), floatset(), textset(), tstzset()
 * @pymeosfunc TstzSet()
 */
Set *
set_make_exp(const Datum *values, int count, int maxcount, meosType basetype,
  bool ordered)
{
  assert(maxcount >= count);

  bool hasz = false;
  bool isgeodetic = false;
  if (geo_basetype(basetype))
  {
    /* Ensure the spatial validity of the elements */
    GSERIALIZED *gs1 = DatumGetGserializedP(values[0]);
    int srid = gserialized_get_srid(gs1);
    hasz = FLAGS_GET_Z(gs1->gflags);
    isgeodetic = FLAGS_GET_GEODETIC(gs1->gflags);
    /* Test the validity of the values */
    for (int i = 0; i < count; i++)
    {
      /* Test that the geometry is not empty */
      GSERIALIZED *gs2 = DatumGetGserializedP(values[i]);
      ensure_point_type(gs2);
      ensure_same_srid(srid, gserialized_get_srid(gs2));
      ensure_same_dimensionality_gs(gs1, gs2);
      ensure_non_empty(gs2);
    }
  }

  /* Sort the values and remove duplicates */
  Datum *newvalues;
  int newcount;
  if (ordered && count > 1)
  {
  /* Sort the values and remove duplicates */
    newvalues = palloc(sizeof(Datum *) * count);
    memcpy(newvalues, values, sizeof(Datum *) * count);
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
  size_t bboxsize = double_pad(set_bbox_size(settype));

  /* Determine whether the values are passed by value or by reference  */
  int16 typlen;
  bool typbyval = basetype_byvalue(basetype);
  if (typbyval)
    /* For base values passed by value */
    typlen = double_pad(sizeof(Datum));
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
        values_size += double_pad(VARSIZE_ANY(DatumGetPointer(newvalues[i])));
    }
    else
      values_size = double_pad(typlen) * newcount;
  }

  /* Compute the total size for maxcount elements as a proportion of the size
   * of the count elements provided. Note that this is only an INITIAL
   * ESTIMATION. The functions adding elements to a set must verify BOTH
   * the maximum number of elements AND the remaining space for adding an
   * additional variable-length element of arbitrary size */
  if (count != maxcount)
    values_size = (double) values_size * (double) maxcount / (double) count;

  /* Total size of the struct */
  size_t memsize = double_pad(sizeof(Set)) + double_pad(bboxsize) +
    (sizeof(size_t) * maxcount) + values_size;

  /* Create the Set */
  Set *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  MOBDB_FLAGS_SET_BYVAL(result->flags, typbyval);
  MOBDB_FLAGS_SET_ORDERED(result->flags, ordered);
  if (geo_basetype(basetype))
  {
    MOBDB_FLAGS_SET_X(result->flags, true);
    MOBDB_FLAGS_SET_Z(result->flags, hasz);
    MOBDB_FLAGS_SET_GEODETIC(result->flags, isgeodetic);
  }
  result->count = newcount;
  result->maxcount = maxcount;
  result->settype = settype;
  result->basetype = basetype;
  result->bboxsize = bboxsize;
  /* Copy the array of values */
  if (typbyval)
  {
    for (int i = 0; i < newcount; i++)
      (set_offsets_ptr(result))[i] = newvalues[i];
  }
  else
  {
    /* Store the composing values */
    size_t pdata = double_pad(sizeof(Set)) + double_pad(bboxsize) +
      sizeof(size_t) * maxcount;
    size_t pos = 0;
    for (int i = 0; i < newcount; i++)
    {
      /* VARSIZE_ANY is used for oblivious data alignment, see postgres.h */
      size_t size_elem = (typlen == -1) ?
        VARSIZE_ANY(newvalues[i]) : (uint32) typlen;
      memcpy(((char *) result) + pdata + pos, DatumGetPointer(newvalues[i]),
        size_elem);
      (set_offsets_ptr(result))[i] = pos;
      pos += double_pad(size_elem);
    }
  }

  /* Compute the bounding box */
  if (bboxsize != 0)
    valuearr_compute_bbox(newvalues, basetype, newcount, set_bbox_ptr(result));

  if (ordered && count > 1)
    pfree(newvalues);
  return result;
}

/**
 * @ingroup libmeos_internal_setspan_constructor
 * @brief Construct a set from an array of values.
 * @param[in] values Array of values
 * @param[in] count Number of elements in the array
 * @param[in] maxcount Maximum number of elements in the array
 * @param[in] basetype Base type
 * @param[in] ordered True for ordered sets
 * @sqlfunc intset(), bigintset(), floatset(), textset(), tstzset()
 * @pymeosfunc TstzSet()
 */
Set *
set_make(const Datum *values, int count, meosType basetype, bool ordered)
{
  return set_make_exp(values, count, count, basetype, ordered);
}

/**
 * @ingroup libmeos_internal_setspan_constructor
 * @brief Construct a set from the array of values and free the
 * array after the creation.
 *
 * @param[in] values Array of values
 * @param[in] count Number of elements in the array
 * @param[in] basetype Base type
 * @param[in] ordered True when the values are stored ordered
 */
Set *
set_make_free(Datum *values, int count, meosType basetype, bool ordered)
{
  if (count == 0)
  {
    pfree(values);
    return NULL;
  }
  Set *result = set_make(values, count, basetype, ordered);
  pfree(values);
  return result;
}

/**
 * @ingroup libmeos_setspan_constructor
 * @brief Return a copy of a set.
 */
Set *
set_copy(const Set *s)
{
  Set *result = palloc(VARSIZE(s));
  memcpy(result, s, VARSIZE(s));
  return result;
}

/*****************************************************************************
 * Cast function
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_cast
 * @brief Cast a value as a set
 * @sqlop @p ::
 */
Set *
value_to_set(Datum d, meosType basetype)
{
  return set_make(&d, 1, basetype, ORDERED);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast a value as a set
 * @sqlop @p ::
 */
Set *
int_to_intset(int i)
{
  Datum v = Int32GetDatum(i);
  return set_make(&v, 1, T_INT4, ORDERED);
}

/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast a value as a set
 * @sqlop @p ::
 */
Set *
bigint_to_bigintset(int64 i)
{
  Datum v = Int64GetDatum(i);
  return set_make(&v, 1, T_INT8, ORDERED);
}

/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast a value as a set
 * @sqlop @p ::
 */
Set *
float_to_floatset(double d)
{
  Datum v = Float8GetDatum(d);
  return set_make(&v, 1, T_FLOAT8, ORDERED);
}

/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast a value as a set
 * @sqlop @p ::
 */
Set *
timestamp_to_tstzset(TimestampTz t)
{
  Datum v = TimestampTzGetDatum(t);
  return set_make(&v, 1, T_TIMESTAMPTZ, ORDERED);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_cast
 * @brief Set the last argument to the bounding span of a set.
 */
void
set_set_span(const Set *set, Span *s)
{
  span_set(set_val_n(set, MINIDX), set_val_n(set, set->MAXIDX), true, true,
    set->basetype, s);
  return;
}

/**
 * @ingroup libmeos_setspan_cast
 * @brief Return the bounding span of a set.
 * @sqlfunc span()
 * @sqlop @p ::
 * @pymeosfunc span()
 */
Span *
set_to_span(const Set *s)
{
  Span *result = palloc(sizeof(Span));
  set_set_span(s, result);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_setspan_cast
 * @brief Set the last argument to the bounding box of a spatial set.
 */
void
spatialset_set_stbox(const Set *set, STBox *box)
{
  assert(spatialset_type(set->settype));
  memset(box, 0, sizeof(STBox));
  memcpy(box, set_bbox_ptr(set), sizeof(STBox));
  return;
}

/**
 * @ingroup libmeos_setspan_cast
 * @brief Return the bounding box of a spatial set.
 * @sqlfunc stbox()
 * @sqlop @p ::
 * @pymeosfunc stbox()
 */
STBox *
spatialset_to_stbox(const Set *s)
{
  STBox *result = palloc(sizeof(STBox));
  spatialset_set_stbox(s, result);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the size in bytes of a set.
 * @sqlfunc memorySize()
 */
int
set_memory_size(const Set *s)
{
  return (int) VARSIZE(DatumGetPointer(s));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the number of values of a set.
 * @sqlfunc numTimestamps()
 * @pymeosfunc numTimestamps()
 */
int
set_num_values(const Set *s)
{
  return s->count;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the start value of a set.
 * @sqlfunc startTimestamp()
 * @pymeosfunc startTimestamp()
 */
Datum
set_start_value(const Set *s)
{
  return set_val_n(s, 0);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the start value of an integer set.
 * @sqlfunc startValue()
 * @pymeosfunc startValue()
 */
int
intset_start_value(const Set *s)
{
  int result = DatumGetInt32(set_val_n(s, 0));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the start value of a big integer set.
 * @sqlfunc startValue()
 * @pymeosfunc startValue()
 */
int64
bigintset_start_value(const Set *s)
{
  int64 result = DatumGetInt64(set_val_n(s, 0));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the start value of a float set.
 * @sqlfunc startValue()
 * @pymeosfunc startValue()
 */
double
floatset_start_value(const Set *s)
{
  double result = DatumGetFloat8(set_val_n(s, 0));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the start value of a set.
 * @sqlfunc startTimestamp()
 * @pymeosfunc startTimestamp()
 */
TimestampTz
tstzset_start_timestamp(const Set *ts)
{
  TimestampTz result = DatumGetTimestampTz(set_val_n(ts, 0));
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the end value of a set.
 * @sqlfunc endTimestamp()
 * @pymeosfunc endTimestamp()
 */
Datum
set_end_value(const Set *s)
{
  return set_val_n(s, s->count - 1);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the end value of an integer set.
 * @sqlfunc endValue()
 * @pymeosfunc endValue()
 */
int
intset_end_value(const Set *s)
{
  int result = DatumGetInt32(set_val_n(s, 0));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the end value of a big integer set.
 * @sqlfunc endValue()
 * @pymeosfunc endValue()
 */
int64
bigintset_end_value(const Set *s)
{
  int64 result = DatumGetInt64(set_val_n(s, 0));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the end value of a float set.
 * @sqlfunc endValue()
 * @pymeosfunc endValue()
 */
double
floatset_end_value(const Set *s)
{
  double result = DatumGetFloat8(set_val_n(s, 0));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the end value of a set.
 * @sqlfunc endTimestamp()
 * @pymeosfunc endTimestamp()
 */
TimestampTz
tstzset_end_timestamp(const Set *ts)
{
  TimestampTz result = DatumGetTimestampTz(set_val_n(ts, ts->count - 1));
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the n-th value of a set.
 *
 * @param[in] s Set
 * @param[in] n Number
 * @param[out] result Timestamp
 * @result Return true if the value is found
 * @note It is assumed that n is 1-based
 * @sqlfunc valueN(), timestampN()
 * @pymeosfunc timestampN()
 */
bool
set_value_n(const Set *s, int n, Datum *result)
{
  if (n < 1 || n > s->count)
    return false;
  *result = set_val_n(s, n - 1);
  return true;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the n-th value of an integer set.
 *
 * @param[in] s Integer set
 * @param[in] n Number
 * @param[out] result Value
 * @result Return true if the value is found
 * @note It is assumed that n is 1-based
 * @sqlfunc valueN()
 * @pymeosfunc valueN()
 */
bool
intset_value_n(const Set *s, int n, int *result)
{
  if (n < 1 || n > s->count)
    return false;
  *result = DatumGetInt32(set_val_n(s, n - 1));
  return true;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the n-th value of a big integer set.
 *
 * @param[in] s Integer set
 * @param[in] n Number
 * @param[out] result Value
 * @result Return true if the value is found
 * @note It is assumed that n is 1-based
 * @sqlfunc valueN()
 * @pymeosfunc valueN()
 */
bool
bigintset_value_n(const Set *s, int n, int64 *result)
{
  if (n < 1 || n > s->count)
    return false;
  *result = DatumGetInt64(set_val_n(s, n - 1));
  return true;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the n-th value of a float set.
 *
 * @param[in] s Float set
 * @param[in] n Number
 * @param[out] result Value
 * @result Return true if the value is found
 * @note It is assumed that n is 1-based
 * @sqlfunc valueN()
 * @pymeosfunc valueN()
 */
bool
floatset_value_n(const Set *s, int n, double *result)
{
  if (n < 1 || n > s->count)
    return false;
  *result = DatumGetFloat8(set_val_n(s, n - 1));
  return true;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the n-th value of a set.
 *
 * @param[in] ts Timestamp set
 * @param[in] n Number
 * @param[out] result Timestamp
 * @result Return true if the timestamp is found
 * @note It is assumed that n is 1-based
 * @sqlfunc timestampN()
 * @pymeosfunc timestampN()
 */
bool
tstzset_timestamp_n(const Set *ts, int n, TimestampTz *result)
{
  if (n < 1 || n > ts->count)
    return false;
  *result = DatumGetTimestampTz(set_val_n(ts, n - 1));
  return true;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the array of values of a set.
 * @sqlfunc values(), timestamps()
 * @pymeosfunc timestamps()
 */
Datum *
set_values(const Set *s)
{
  Datum *result = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = set_val_n(s, i);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the array of values of an integer set.
 * @sqlfunc values()
 * @pymeosfunc values()
 */
int *
intset_values(const Set *s)
{
  int *result = palloc(sizeof(int) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetInt32(set_val_n(s, i));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the array of values of a big integer set.
 * @sqlfunc values()
 * @pymeosfunc values()
 */
int64 *
bigintset_values(const Set *s)
{
  int64 *result = palloc(sizeof(int64) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetInt64(set_val_n(s, i));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the array of values of a float set.
 * @sqlfunc values()
 * @pymeosfunc values()
 */
double *
floatset_values(const Set *s)
{
  double *result = palloc(sizeof(double) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetFloat8(set_val_n(s, i));
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the array of timestamps of a set.
 * @sqlfunc timestamps()
 * @pymeosfunc timestamps()
 */
TimestampTz *
tstzset_timestamps(const Set *ts)
{
  TimestampTz *result = palloc(sizeof(TimestampTz) * ts->count);
  for (int i = 0; i < ts->count; i++)
    result[i] = DatumGetTimestampTz(set_val_n(ts, i));
  return result;
}

/**
 * @ingroup libmeos_internal_setspan_accessor
 * @brief Return the SRID of a geoset point.
 * @sqlfunc SRID()
 */
int
geoset_srid(const Set *set)
{
  assert(geo_basetype(set->basetype));
  GSERIALIZED *gs = DatumGetGserializedP(set_val_n(set, 0));
  return gserialized_get_srid(gs);
}

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

/**
 * @brief Shift the values of set.
 */
Set *
set_shift(const Set *s, Datum shift)
{
  assert(MOBDB_FLAGS_GET_BYVAL(s->flags));
  Set *result = set_copy(s);
  for (int i = 0; i < s->count; i++)
    (set_offsets_ptr(result))[i] =
      datum_add(set_val_n(s, i), shift, s->basetype, s->basetype);
  return result;
}

/**
 * @ingroup libmeos_setspan_transf
 * @brief Return a timestamp set uned and/or scaled by the intervals
 * @sqlfunc shift(), tscale(), shiftTscale()
 * @pymeosfunc shift()
 */
Set *
tstzset_shift_tscale(const Set *s, const Interval *shift,
  const Interval *duration)
{
  assert(shift != NULL || duration != NULL);
  if (duration != NULL)
    ensure_valid_duration(duration);
  Set *result = set_copy(s);

  /* Set the first and last instants */
  TimestampTz lower, lower1, upper, upper1;
  lower = lower1 = DatumGetTimestampTz(set_val_n(s, 0));
  upper = upper1 = DatumGetTimestampTz(set_val_n(s, s->count - 1));
  lower_upper_shift_tscale(&lower1, &upper1, shift, duration);
  (set_offsets_ptr(result))[0] = TimestampTzGetDatum(lower1);
  (set_offsets_ptr(result))[s->count - 1] = TimestampTzGetDatum(upper1);
  if (s->count > 1)
  {
    /* Shift and/or scale from the second to the penultimate instant */
    TimestampTz delta;
    if (shift != NULL)
      delta = lower1 - lower;
    double scale;
    if (duration != NULL)
      scale = (double) (upper1 - lower1) / (double) (upper - lower);
    for (int i = 1; i < s->count - 1; i++)
    {
      if (shift != NULL)
        (set_offsets_ptr(result))[i] += delta;
      if (duration != NULL)
        (set_offsets_ptr(result))[i] = lower1 +
          (set_val_n(result, i) - lower1) * scale;
    }
  }
  return result;
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first set is equal to the second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
 * @pymeosfunc __eq__()
 */
bool
set_eq(const Set *s1, const Set *s2)
{
  assert(s1->settype == s2->settype);
  if (s1->count != s2->count)
    return false;
  /* s1 and s2 have the same number of values */
  for (int i = 0; i < s1->count; i++)
  {
    Datum v1 = set_val_n(s1, i);
    Datum v2 = set_val_n(s2, i);
    if (datum_ne(v1, v2, s1->basetype))
      return false;
  }
  /* All values of the two sets are equal */
  return true;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first set is different from the
 * second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p <>
 */
bool
set_ne(const Set *s1, const Set *s2)
{
  return ! set_eq(s1, s2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first set is less
 * than, equal, or greater than the second one.
 * @note Function used for B-tree comparison
 * @sqlfunc set_cmp()
 */
int
set_cmp(const Set *s1, const Set *s2)
{
  assert(s1->settype == s2->settype);
  int count = Min(s1->count, s2->count);
  int result = 0;
  for (int i = 0; i < count; i++)
  {
    Datum v1 = set_val_n(s1, i);
    Datum v2 = set_val_n(s2, i);
    result = datum_cmp(v1, v2, s1->basetype);
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
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first set is less than the second one
 * @sqlop @p <
 */
bool
set_lt(const Set *s1, const Set *s2)
{
  return set_cmp(s1, s2) < 0;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first set is less than or equal to the
 * second one
 * @sqlop @p <=
 */
bool
set_le(const Set *s1, const Set *s2)
{
  return set_cmp(s1, s2) <= 0;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first set is greater than or equal to
 * the second one
 * @sqlop @p >=
 */
bool
set_ge(const Set *s1, const Set *s2)
{
  return set_cmp(s1, s2) >= 0;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first set is greater than the second one
 * @sqlop @p >
 */
bool
set_gt(const Set *s1, const Set *s2)
{
  return set_cmp(s1, s2) > 0;
}

/*****************************************************************************
 * Functions for defining hash index
 * The function reuses PostgreSQL approach for array types for combining the
 * hash of the elements.
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the 32-bit hash of a set.
 * @sqlfunc tstzset_hash()
 */
uint32
set_hash(const Set *s)
{
  uint32 result = 1;
  for (int i = 0; i < s->count; i++)
  {
    Datum d = set_val_n(s, i);
    uint32 value_hash = datum_hash(d, s->basetype);
    result = (result << 5) - result + value_hash;
  }
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the 64-bit hash of a set using a seed.
 * @sqlfunc tstzset_hash_extended()
 */
uint64
set_hash_extended(const Set *s, uint64 seed)
{
  uint64 result = 1;
  for (int i = 0; i < s->count; i++)
  {
    Datum d = set_val_n(s, i);
    uint64 value_hash = datum_hash_extended(d, s->basetype, seed);
    result = (result << 5) - result + value_hash;
  }
  return result;
}

/*****************************************************************************/
