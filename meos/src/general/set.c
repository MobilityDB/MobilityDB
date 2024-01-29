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
 * @brief General functions for set values composed of an ordered list of
 * distinct values
 */

#include "general/set.h"

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
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/span.h"
#include "general/tnumber_mathfuncs.h"
#include "general/ttext_textfuncs.h"
#include "general/type_parser.h"
#include "general/type_util.h"
#include "point/tpoint_out.h"
#include "point/tpoint_spatialfuncs.h"
#if NPOINT
  #include "npoint/tnpoint_boxops.h"
#endif

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensure that a set is of a given set type
 */
bool
ensure_set_isof_type(const Set *s, meosType settype)
{
  if (s->settype != settype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The set must be of type %s", meostype_name(settype));
    return false;
  }
  return true;
}

#if MEOS
/**
 * @brief Ensure that a set is of a given base type
 */
bool
ensure_set_isof_basetype(const Set *s, meosType basetype)
{
  if (s->basetype != basetype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Operation on mixed set and base types: %s and %s",
      meostype_name(s->settype), meostype_name(basetype));
    return false;
  }
  return true;
}
#endif /* MEOS */

/**
 * @brief Ensure that the sets have the same type to be able to apply
 * operations to them
 */
bool
ensure_same_set_type(const Set *s1, const Set *s2)
{
  if (s1->settype != s2->settype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Operation on mixed set types: %s and %s",
      meostype_name(s1->settype), meostype_name(s2->settype));
    return false;
  }
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

#if 0 /* not used */
/**
 * @brief Return the location of a value in a ranked set (which is unordered)
 * using sequential search
 * @param[in] s Set
 * @param[in] d Value
 * @param[out] loc Location of the value if found
 * @result Return true if the value is contained in the vecctor
 * @note Contrary to function `set_find_value`, if the value is not found the
 * returned location is always 0.
 */
bool
rset_find_value(const Set *s, Datum d, int *loc)
{
  assert(s); assert(loc);
  for (int i = 0; i < s->count; i++)
  {
    Datum d1 = SET_VAL_N(s, i);
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

#if MEOS
/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
intset_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return set_parse(&str, T_INTSET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
bigintset_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return set_parse(&str, T_BIGINTSET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
floatset_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return set_parse(&str, T_FLOATSET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
textset_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return set_parse(&str, T_TEXTSET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
dateset_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return set_parse(&str, T_DATESET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
tstzset_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return set_parse(&str, T_TSTZSET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
geomset_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return set_parse(&str, T_GEOMSET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
geogset_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return set_parse(&str, T_GEOGSET);
}
#endif /* MEOS */


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
  /* Ensure validity of the arguments */
  if (! ensure_not_negative(maxdd))
    return NULL;

  /* Get the SRID if it is a geo set  */
  int32 srid;
  char str1[20];
  str1[0] = '\0';
  outfunc value_out1 = value_out;
  if (geoset_type(s->settype) && value_out == ewkt_out)
  {
    srid = geoset_srid(s);
    if (srid > 0)
      sprintf(str1, "SRID=%d;", srid);
    /* Since the SRID is output at the begining it is not output for the
     * elements */
    value_out1 = wkt_out;
  }

  char **strings = palloc(sizeof(char *) * s->count);
  size_t outlen = 0;
  for (int i = 0; i < s->count; i++)
  {
    strings[i] = value_out1(SET_VAL_N(s, i), s->basetype, maxdd);
    outlen += strlen(strings[i]) + 1;
  }
  bool quotes = set_basetype_quotes(s->basetype);
  char *str2 = stringarr_to_string(strings, s->count, outlen, "", '{', '}',
    quotes, SPACES);
  char *result = palloc(strlen(str1) + strlen(str2) + 1);
  strcpy(result, str1);
  strcat(result, str2);
  pfree(str2);
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
  /* Ensure validity of the arguments */
  if (! ensure_not_negative(maxdd))
    return NULL;
  return set_out_fn(s, maxdd, &basetype_out);
}

#if MEOS
/**
 * @ingroup meos_setspan_inout
 * @brief Return the string representation of an integer set
 * @param[in] s Set
 * @csqlfn #Set_out()
 */
char *
intset_out(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_INTSET))
    return NULL;
  return set_out(s, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the string representation of a big integer set
 * @param[in] s Set
 * @csqlfn #Set_out()
 */
char *
bigintset_out(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_BIGINTSET))
    return NULL;
  return set_out(s, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the string representation of a float set
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Set_out()
 */
char *
floatset_out(const Set *s, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_FLOATSET) ||
      ! ensure_not_negative(maxdd))
    return NULL;
  return set_out(s, maxdd);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the string representation of a text set
 * @param[in] s Set
 * @csqlfn #Set_out()
 */
char *
textset_out(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_TEXTSET))
    return NULL;
  return set_out(s, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the string representation of a date set
 * @param[in] s Set
 * @csqlfn #Set_out()
 */
char *
dateset_out(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_DATESET))
    return NULL;
  return set_out(s, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the string representation of a timestamptz set
 * @param[in] s Set
 * @csqlfn #Set_out()
 */
char *
tstzset_out(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_TSTZSET))
    return NULL;
  return set_out(s, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the string representation of a geometry set
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Set_out()
 */
char *
geoset_out(const Set *s, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_geoset_type(s->settype))
    return NULL;
  return set_out(s, maxdd);
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a geo set
 * @csqlfn #Geoset_as_text()
 */
char *
geoset_as_text(const Set *s, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_geoset_type(s->settype))
    return NULL;
  return set_out_fn(s, maxdd, &wkt_out);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a geo set
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Geoset_as_ewkt()
 */
char *
geoset_as_ewkt(const Set *s, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_geoset_type(s->settype))
    return NULL;
  return set_out_fn(s, maxdd, &ewkt_out);
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
  if (alphanumset_type(settype))
    return 0;
  if (spatialset_type(settype))
    return sizeof(STBox);
  meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
    "Unknown set type when determining the size of the bounding box: %d",
    settype);
  return SIZE_MAX;
}

/**
 * @brief Set a bounding box from an array of set values
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
  assert(set_basetype(basetype)); assert(! alphanum_basetype(basetype));
  if (geo_basetype(basetype))
    geoarr_set_stbox(values, count, (STBox *) box);
#if NPOINT
  else if (basetype == T_NPOINT)
    npointarr_set_stbox(values, count, (STBox *) box);
#endif
  else
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Unknown set type for computing the bounding box: %d", basetype);
  return;
}

#ifdef DEBUG_BUILD
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
 * @param[in] ordered True when the values are ordered and without duplicates
 */
Set *
set_make_exp(const Datum *values, int count, int maxcount, meosType basetype,
  bool ordered)
{
  assert(values); assert(count > 0); assert(count <= maxcount);
  bool hasz = false;
  bool geodetic = false;
  if (geo_basetype(basetype))
  {
    /* Ensure the spatial validity of the elements */
    GSERIALIZED *gs1 = DatumGetGserializedP(values[0]);
    int srid = gserialized_get_srid(gs1);
    hasz = (bool) FLAGS_GET_Z(gs1->gflags);
    geodetic = FLAGS_GET_GEODETIC(gs1->gflags);
    /* Test the validity of the values */
    for (int i = 0; i < count; i++)
    {
      /* Test that the geometry is not empty */
      GSERIALIZED *gs2 = DatumGetGserializedP(values[i]);
      if (! ensure_point_type(gs2) ||
          ! ensure_same_srid(srid, gserialized_get_srid(gs2)) ||
          ! ensure_same_dimensionality_gs(gs1, gs2) ||
          ! ensure_not_empty(gs2))
        return NULL;
    }
  }

  /* Sort the values and remove duplicates */
  Datum *newvalues;
  int newcount;
  if (! ordered && count > 1)
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
      values_size = DOUBLE_PAD(typlen) * newcount;
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
  MEOS_FLAGS_SET_ORDERED(result->flags, ordered);
  if (geo_basetype(basetype))
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
    valuearr_compute_bbox(newvalues, basetype, newcount, SET_BBOX_PTR(result));

  if (! ordered && count > 1)
    pfree(newvalues);
  return result;
}

#if MEOS
/**
 * @ingroup meos_internal_setspan_constructor
 * @brief Return a set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements in the array
 * @param[in] basetype Type of the values
 * @param[in] ordered True when the values are ordered and without duplicates
 * @csqlfn #Set_constructor()
 */
Set *
set_make(const Datum *values, int count, meosType basetype, bool ordered)
{
  assert(values); assert(count > 0);
  return set_make_exp(values, count, count, basetype, ordered);
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return an integer set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
intset_make(const int *values, int count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) values) || ! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = Int32GetDatum(values[i]);
  return set_make_free(datums, count, T_INT4, ORDERED_NO);
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a big integer set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
bigintset_make(const int64 *values, int count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) values) || ! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = Int64GetDatum(values[i]);
  return set_make_free(datums, count, T_INT8, ORDERED_NO);
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a float set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
floatset_make(const double *values, int count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) values) || ! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = Float8GetDatum(values[i]);
  return set_make_free(datums, count, T_FLOAT8, ORDERED_NO);
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a text set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
textset_make(const text **values, int count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) values) || ! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = PointerGetDatum(values[i]);
  return set_make_free(datums, count, T_TEXT, ORDERED_NO);
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a date set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
dateset_make(const DateADT *values, int count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) values) || ! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = DateADTGetDatum(values[i]);
  return set_make_free(datums, count, T_DATE, ORDERED_NO);
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a timestamptz set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
tstzset_make(const TimestampTz *values, int count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) values) || ! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = TimestampTzGetDatum(values[i]);
  return set_make_free(datums, count, T_TIMESTAMPTZ, ORDERED_NO);
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a geometry set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
geoset_make(const GSERIALIZED **values, int count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) values) || ! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = PointerGetDatum(values[i]);
  meosType geotype = FLAGS_GET_GEODETIC(values[0]->gflags) ?
    T_GEOMETRY : T_GEOGRAPHY;
  return set_make_free(datums, count, geotype, ORDERED_NO);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_constructor
 * @brief Return a set from the array of values and free the input array
 * after the creation
 * @param[in] values Array of values
 * @param[in] count Number of elements in the array
 * @param[in] basetype Type of the values
 * @param[in] ordered True when the values are ordered and without duplicates
 */
Set *
set_make_free(Datum *values, int count, meosType basetype, bool ordered)
{
  assert(values); assert(count >= 0);
  Set *result = NULL;
  if (count > 0)
    result = set_make_exp(values, count, count, basetype, ordered);
  pfree(values);
  return result;
}

/**
 * @ingroup meos_internal_setspan_constructor
 * @brief Return a copy of a set
 * @param[in] s Set
 */
Set *
set_cp(const Set *s)
{
  assert(s);
  Set *result = palloc(VARSIZE(s));
  memcpy(result, s, VARSIZE(s));
  return result;
}

#if MEOS
/**
 * @ingroup meos_setspan_constructor
 * @brief Return a copy of a set
 * @param[in] s Set
 */
Set *
set_copy(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return NULL;
  return set_cp(s);
}
#endif /* MEOS */

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return a value converted to a set
 * @param[in] value Value
 * @param[in] basetype Type of the value
 * @csqlfn #Value_to_set()
 */
Set *
value_to_set(Datum value, meosType basetype)
{
  return set_make_exp(&value, 1, 1, basetype, ORDERED);
}

#if MEOS
/**
 * @ingroup meos_setspan_conversion
 * @brief Return an integer converted to a set
 * @param[in] i Value
 * @csqlfn #Value_to_set()
 */
Set *
int_to_set(int i)
{
  Datum v = Int32GetDatum(i);
  return set_make_exp(&v, 1, 1, T_INT4, ORDERED);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Return a big integer converted to a set
 * @param[in] i Value
 * @csqlfn #Value_to_set()
 */
Set *
bigint_to_set(int64 i)
{
  Datum v = Int64GetDatum(i);
  return set_make_exp(&v, 1, 1, T_INT8, ORDERED);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Return a float converted to a set
 * @param[in] d Value
 * @csqlfn #Value_to_set()
 */
Set *
float_to_set(double d)
{
  Datum v = Float8GetDatum(d);
  return set_make_exp(&v, 1, 1, T_FLOAT8, ORDERED);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Return a text converted to a set
 * @param[in] txt Value
 * @csqlfn #Value_to_set()
 */
Set *
text_to_set(text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) txt))
    return NULL;
  Datum v = PointerGetDatum(txt);
  return set_make_exp(&v, 1, 1, T_TEXT, ORDERED);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Return a date converted to a set
 * @param[in] d Value
 * @csqlfn #Value_to_set()
 */
Set *
date_to_set(DateADT d)
{
  Datum v = DateADTGetDatum(d);
  return set_make_exp(&v, 1, 1, T_DATE, ORDERED);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Return a timestamptz converted to a set
 * @param[in] t Value
 * @csqlfn #Value_to_set()
 */
Set *
timestamptz_to_set(TimestampTz t)
{
  Datum v = TimestampTzGetDatum(t);
  return set_make_exp(&v, 1, 1, T_TIMESTAMPTZ, ORDERED);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Return a geometry/geography converted to a geo set
 * @param[in] gs Value
 * @csqlfn #Value_to_set()
 */
Set *
geo_to_set(GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs) || ! ensure_not_empty(gs))
    return NULL;
  Datum v = PointerGetDatum(gs);
  meosType geotype = FLAGS_GET_GEODETIC(gs->gflags) ? T_GEOGRAPHY : T_GEOMETRY;
  return set_make_exp(&v, 1, 1, geotype, ORDERED);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return an integer set converted to a float set
 * @param[in] s Set
 * @csqlfn #Intset_to_floatset()
 */
Set *
intset_floatset(const Set *s)
{
  assert(s);
  /* Ensure validity of the arguments */
  if (! ensure_set_isof_type(s, T_INTSET))
    return NULL;
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = Float8GetDatum((double) DatumGetInt32(SET_VAL_N(s, i)));
  /* All distinct integers will yield distinct floats */
  return set_make_free(values, s->count, T_FLOAT8, ORDERED);
}

#if MEOS
/**
 * @ingroup meos_setspan_conversion
 * @brief Return an integer set converted to a float set
 * @param[in] s Set
 * @csqlfn #Intset_to_floatset()
 */
Set *
intset_to_floatset(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return NULL;
  return intset_floatset(s);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return a float set converted to an integer set
 * @param[in] s Set
 * @csqlfn #Floatset_to_intset()
 */
Set *
floatset_intset(const Set *s)
{
  assert(s);
  /* Ensure validity of the arguments */
  if (! ensure_set_isof_type(s, T_FLOATSET))
    return NULL;
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = Int32GetDatum((int) DatumGetFloat8(SET_VAL_N(s, i)));
  /* Two distinct floats can yield the same integer */
  return set_make_free(values, s->count, T_INT4, ORDERED_NO);
}

#if MEOS
/**
 * @ingroup meos_setspan_conversion
 * @brief Return a float set converted to an integer set
 * @param[in] s Set
 * @csqlfn #Floatset_to_intset()
 */
Set *
floatset_to_intset(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return NULL;
  return floatset_intset(s);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return a date set converted to a timestamptz set
 * @param[in] s Set
 * @csqlfn #Dateset_to_tstzset()
 */
Set *
dateset_tstzset(const Set *s)
{
  assert(s);
  /* Ensure validity of the arguments */
  if (! ensure_set_isof_type(s, T_DATESET))
    return NULL;
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = TimestampTzGetDatum(date_to_timestamptz(DatumGetDateADT(
      SET_VAL_N(s, i))));
  /* All distinct dates will yield distinct timestamptz */
  return set_make_free(values, s->count, T_TIMESTAMPTZ, ORDERED);
}

#if MEOS
/**
 * @ingroup meos_setspan_conversion
 * @brief Return a date set converted to a timestamptz set
 * @param[in] s Set
 * @csqlfn #Dateset_to_tstzset()
 */
Set *
dateset_to_tstzset(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return NULL;
  return dateset_tstzset(s);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return a timestamptz set converted to a date set
 * @param[in] s Set
 * @csqlfn #Tstzset_to_dateset()
 */
Set *
tstzset_dateset(const Set *s)
{
  assert(s);
  /* Ensure validity of the arguments */
  if (! ensure_set_isof_type(s, T_TSTZSET))
    return NULL;
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = DateADTGetDatum(timestamptz_to_date(DatumGetTimestampTz(
      SET_VAL_N(s, i))));
  /* Two distinct timestamptz can yield the same date */
  return set_make_free(values, s->count, T_DATE, ORDERED_NO);
}

#if MEOS
/**
 * @ingroup meos_setspan_conversion
 * @brief Return a timestamptz set converted to a date set
 * @param[in] s Set
 * @csqlfn #Tstzset_to_dateset()
 */
Set *
tstzset_to_dateset(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return NULL;
  return tstzset_dateset(s);
}
#endif /* MEOS */

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
  assert(s);
  return (int) VARSIZE(DatumGetPointer(s));
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return -1;
  return s->count;
}

/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return (a copy of) the start value of a set
 * @param[in] s Set
 */
Datum
set_start_value(const Set *s)
{
  assert(s);
  return MEOS_FLAGS_GET_BYVAL(s->flags) ? SET_VAL_N(s, 0) :
    datum_copy(SET_VAL_N(s, 0), s->basetype);
}

#if MEOS
/**
 * @ingroup meos_setspan_accessor
 * @brief Return the start value of an integer set
 * @param[in] s Set
 * @return On error return @p INT_MAX
 * @csqlfn #Set_start_value()
 */
int
intset_start_value(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_INTSET))
    return INT_MAX;
  return DatumGetInt32(SET_VAL_N(s, 0));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the start value of a big integer set
 * @param[in] s Set
 * @return On error return @p INT_MAX
 * @csqlfn #Set_start_value()
 */
int64
bigintset_start_value(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_BIGINTSET))
    return INT_MAX;
  return DatumGetInt64(SET_VAL_N(s, 0));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the start value of a float set
 * @param[in] s Set
 * @return On error return @p DBL_MAX
 * @csqlfn #Set_start_value()
 */
double
floatset_start_value(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_FLOATSET))
    return DBL_MAX;
  return DatumGetFloat8(SET_VAL_N(s, 0));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return a copy of the start value of a text set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_start_value()
 */
text *
textset_start_value(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_TEXTSET))
    return NULL;
  return DatumGetTextP(datum_copy(SET_VAL_N(s, 0), s->basetype));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the start value of a date set
 * @param[in] s Set
 * @return On error return DATEVAL_NOEND
 * @csqlfn #Set_start_value()
 */
DateADT
dateset_start_value(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_DATESET))
    return DATEVAL_NOEND;
  return DatumGetDateADT(SET_VAL_N(s, 0));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the start value of a timestamptz set
 * @param[in] s Set
 * @return On error return DT_NOEND
 * @csqlfn #Set_start_value()
 */
TimestampTz
tstzset_start_value(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_TSTZSET))
    return DT_NOEND;
  return DatumGetTimestampTz(SET_VAL_N(s, 0));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return a copy of the start value of a geo set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_start_value()
 */
GSERIALIZED *
geoset_start_value(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_geoset_type(s->settype))
    return NULL;
  return DatumGetGserializedP(datum_copy(SET_VAL_N(s, 0), s->basetype));
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return (a copy of) the end value of a set
 * @param[in] s Set
 */
Datum
set_end_value(const Set *s)
{
  assert(s);
  return MEOS_FLAGS_GET_BYVAL(s->flags) ? SET_VAL_N(s, s->count - 1) :
    datum_copy(SET_VAL_N(s, s->count - 1), s->basetype);
}

#if MEOS
/**
 * @ingroup meos_setspan_accessor
 * @brief Return the end value of an integer set
 * @param[in] s Set
 * @return On error return @p INT_MAX
 * @csqlfn #Set_end_value()
 */
int
intset_end_value(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_INTSET))
    return INT_MAX;
  return DatumGetInt32(SET_VAL_N(s, s->count - 1));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the end value of a big integer set
 * @param[in] s Set
 * @return On error return @p INT_MAX
 * @csqlfn #Set_end_value()
 */
int64
bigintset_end_value(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_BIGINTSET))
    return INT_MAX;
  return DatumGetInt64(SET_VAL_N(s, s->count - 1));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the end value of a float set
 * @param[in] s Set
 * @return On error return @p DBL_MAX
 * @csqlfn #Set_end_value()
 */
double
floatset_end_value(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_FLOATSET))
    return DBL_MAX;
  return DatumGetFloat8(SET_VAL_N(s, s->count - 1));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return a copy of the end value of a text set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_end_value()
 */
text *
textset_end_value(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_TEXTSET))
    return NULL;
  return DatumGetTextP(datum_copy(SET_VAL_N(s, s->count - 1), s->basetype));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the end value of a date set
 * @param[in] s Set
 * @return On error return DATEVAL_NOEND
 * @csqlfn #Set_end_value()
 */
DateADT
dateset_end_value(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_DATESET))
    return DATEVAL_NOEND;
  return DatumGetDateADT(SET_VAL_N(s, s->count - 1));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the end value of a timestamptz set
 * @param[in] s Set
 * @return On error return DT_NOEND
 * @csqlfn #Set_end_value()
 */
TimestampTz
tstzset_end_value(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_TSTZSET))
    return DT_NOEND;
  return DatumGetTimestampTz(SET_VAL_N(s, s->count - 1));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return a copy of the end value of a geo set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_end_value()
 */
GSERIALIZED *
geoset_end_value(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_geoset_type(s->settype))
    return NULL;
  return DatumGetGserializedP(datum_copy(SET_VAL_N(s, s->count - 1),
    s->basetype));
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return the last argument initialized to (a copy of) the n-th value of
 * a set
 * @param[in] s Set
 * @param[in] n Number
 * @param[out] result Value
 * @result Return true if the value is found
 * @note It is assumed that n is 1-based
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

#if MEOS
/**
 * @ingroup meos_setspan_accessor
 * @brief Return the last argument initialized to the n-th value of an integer
 * set
 * @param[in] s Integer set
 * @param[in] n Number
 * @param[out] result Value
 * @result Return true if the value is found
 * @note It is assumed that n is 1-based
 * @csqlfn #Set_value_n()
 */
bool
intset_value_n(const Set *s, int n, int *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_set_isof_type(s, T_INTSET) || n < 1 || n > s->count)
    return false;
  *result = DatumGetInt32(SET_VAL_N(s, n - 1));
  return true;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the last argument initialized to the n-th value of a big
 * integer set
 * @param[in] s Integer set
 * @param[in] n Number
 * @param[out] result Value
 * @result Return true if the value is found
 * @note It is assumed that n is 1-based
 * @csqlfn #Set_value_n()
 */
bool
bigintset_value_n(const Set *s, int n, int64 *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_set_isof_type(s, T_BIGINTSET) || n < 1 || n > s->count)
    return false;
  *result = DatumGetInt64(SET_VAL_N(s, n - 1));
  return true;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the last argument initialized to the n-th value of a float set
 * @param[in] s Float set
 * @param[in] n Number
 * @param[out] result Value
 * @result Return true if the value is found
 * @note It is assumed that n is 1-based
 * @csqlfn #Set_value_n()
 */
bool
floatset_value_n(const Set *s, int n, double *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_set_isof_type(s, T_FLOATSET) || n < 1 || n > s->count)
    return false;
  *result = DatumGetFloat8(SET_VAL_N(s, n - 1));
  return true;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the last argument initialized to a copy of the n-th value of a
 * text set
 * @param[in] s Text set
 * @param[in] n Number
 * @param[out] result Value
 * @result Return true if the value is found
 * @note It is assumed that n is 1-based
 * @csqlfn #Set_value_n()
 */
bool
textset_value_n(const Set *s, int n, text **result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_set_isof_type(s, T_TEXTSET) || n < 1 || n > s->count)
    return false;
  *result = DatumGetTextP(datum_copy(SET_VAL_N(s, n - 1), s->basetype));
  return true;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the last argument initialized to the n-th value of a date set
 * @param[in] s Date set
 * @param[in] n Number
 * @param[out] result Date
 * @result Return true if the date is found
 * @note It is assumed that n is 1-based
 * @csqlfn #Set_value_n()
 */
bool
dateset_value_n(const Set *s, int n, DateADT *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_set_isof_type(s, T_DATESET) || n < 1 || n > s->count)
    return false;
  *result = DatumGetDateADT(SET_VAL_N(s, n - 1));
  return true;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the last argument initialized to the n-th value of a
 * timestamptz set
 * @param[in] s Timestamptz set
 * @param[in] n Number
 * @param[out] result Timestamptz
 * @result Return true if the timestamptz is found
 * @note It is assumed that n is 1-based
 * @csqlfn #Set_value_n()
 */
bool
tstzset_value_n(const Set *s, int n, TimestampTz *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_set_isof_type(s, T_TSTZSET) || n < 1 || n > s->count)
    return false;
  *result = DatumGetTimestampTz(SET_VAL_N(s, n - 1));
  return true;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the last argument initialized to a copy of the n-th value of a
 * geo set
 * @param[in] s Geo set
 * @param[in] n Number
 * @param[out] result Value
 * @result Return true if the value is found
 * @note It is assumed that n is 1-based
 * @csqlfn #Set_value_n()
 */
bool
geoset_value_n(const Set *s, int n, GSERIALIZED **result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_geoset_type(s->settype) || n < 1 || n > s->count)
    return false;
  *result = DatumGetGserializedP(datum_copy(SET_VAL_N(s, n - 1), s->basetype));
  return true;
}
#endif /* MEOS */

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

#if MEOS
/**
 * @ingroup meos_setspan_accessor
 * @brief Return the array of values of an integer set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
int *
intset_values(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_INTSET))
    return NULL;

  int *result = palloc(sizeof(int) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetInt32(SET_VAL_N(s, i));
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the array of values of a big integer set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
int64 *
bigintset_values(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_BIGINTSET))
    return NULL;

  int64 *result = palloc(sizeof(int64) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetInt64(SET_VAL_N(s, i));
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the array of values of a float set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
double *
floatset_values(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_FLOATSET))
    return NULL;

  double *result = palloc(sizeof(double) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetFloat8(SET_VAL_N(s, i));
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the array of copies of the values of a text set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
text **
textset_values(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_TEXTSET))
    return NULL;

  text **result = palloc(sizeof(text *) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetTextP(datum_copy(SET_VAL_N(s, i), s->basetype));
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the array of values of a date set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
DateADT *
dateset_values(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_DATESET))
    return NULL;

  DateADT *result = palloc(sizeof(DateADT) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetDateADT(SET_VAL_N(s, i));
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the array of values of a timestamptz set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
TimestampTz *
tstzset_values(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_TSTZSET))
    return NULL;

  TimestampTz *result = palloc(sizeof(TimestampTz) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetTimestampTz(SET_VAL_N(s, i));
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the array of copies of the values of a geo set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
GSERIALIZED **
geoset_values(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_geoset_type(s->settype))
    return NULL;

  GSERIALIZED **result = palloc(sizeof(GSERIALIZED *) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetGserializedP(datum_copy(SET_VAL_N(s, i), s->basetype));
  return result;
}
#endif /* MEOS */

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

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
  /* Collect the values which may be UNORDERED */
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = SET_VAL_N(s, i);

  Set *result = set_make_exp(values, s->count, s->count, s->basetype, ORDERED);
  pfree(values);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_transf
 * @brief Return a float set with the values converted to degrees
 * @param[in] s Set
 * @param[in] normalize True when the result must be normalized
 * @csqlfn #Floatset_degrees()
 */
Set *
floatset_deg(const Set *s, bool normalize)
{
  assert(s);
  Set *result = set_cp(s);
  for (int i = 0; i < s->count; i++)
    (SET_OFFSETS_PTR(result))[i] = datum_degrees(SET_VAL_N(s, i), normalize);
  return result;
}

#if MEOS
/**
 * @ingroup meos_setspan_transf
 * @brief Return a float set with the values converted to degrees
 * @param[in] s Set
 * @param[in] normalize True when the result must be normalized
 * @csqlfn #Floatset_degrees()
 */
Set *
floatset_degrees(const Set *s, bool normalize)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_FLOATSET))
    return NULL;
  return floatset_deg(s, normalize);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_transf
 * @brief Return a float set with the values converted to radians
 * @param[in] s Set
 * @csqlfn #Floatset_radians()
 */
Set *
floatset_rad(const Set *s)
{
  assert(s);
  Set *result = set_cp(s);
  for (int i = 0; i < s->count; i++)
    (SET_OFFSETS_PTR(result))[i] = datum_radians(SET_VAL_N(s, i));
  return result;
}

#if MEOS
/**
 * @ingroup meos_setspan_transf
 * @brief Return a float set with the values converted to radians
 * @param[in] s Set
 * @csqlfn #Floatset_radians()
 */
Set *
floatset_radians(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_FLOATSET))
    return NULL;
  return floatset_rad(s);
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_transf
 * @brief Return a text set transformed to lowercase
 * @param[in] s Set
 * @csqlfn #Textset_lower()
 */
Set *
textset_lower(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_TEXTSET))
    return NULL;

  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = datum_lower(SET_VAL_N(s, i));
  return set_make_free(values, s->count, T_TEXT, ORDERED);
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_TEXTSET))
    return NULL;

  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = datum_upper(SET_VAL_N(s, i));
  return set_make_free(values, s->count, T_TEXT, ORDERED);
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_TEXTSET))
    return NULL;

  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = datum_initcap(SET_VAL_N(s, i));
  return set_make_free(values, s->count, T_TEXT, ORDERED);
}

/**
 * @brief Return the concatenation of a text set and a set
 * @param[in] s Set
 * @param[in] txt Text
 * @param[in] invert True when the arguments must be inverted
 */
Set *
textcat_textset_text_int(const Set *s, const text *txt, bool invert)
{
  assert(s); assert(txt); assert((s->settype == T_TEXTSET));
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = invert ?
      datum_textcat(PointerGetDatum(txt), SET_VAL_N(s, i)) :
      datum_textcat(SET_VAL_N(s, i), PointerGetDatum(txt));
  return set_make_free(values, s->count, T_TEXT, ORDERED);
}

#if MEOS
/**
 * @ingroup meos_setspan_transf
 * @brief Return the concatenation of a text and a text set
 * @param[in] txt Text
 * @param[in] s Set
 * @csqlfn #Textcat_text_textset()
 */
Set *
textcat_text_textset(const text *txt, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_type(s, T_TEXTSET))
    return NULL;
  return textcat_textset_text_int(s, txt, INVERT);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return the concatenation of a text set and a text
 * @param[in] s Set
 * @param[in] txt Text
 * @csqlfn #Textcat_textset_text()
 */
Set *
textcat_textset_text(const Set *s, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) txt) ||
      ! ensure_set_isof_type(s, T_TEXTSET))
    return NULL;

  return textcat_textset_text_int(s, txt, INVERT_NO);
}
#endif /* MEOS */

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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_numset_type(s->settype) ||
      ! ensure_one_true(hasshift, haswidth) ||
      (haswidth && ! ensure_positive_datum(width, type)))
    return NULL;

  /* Copy the input set to the output set */
  Set *result = set_cp(s);

  /* Set the first and last instants */
  Datum lower, lower1, upper, upper1;
  lower = lower1 = SET_VAL_N(s, 0);
  upper = upper1 = SET_VAL_N(s, s->count - 1);
  lower_upper_shift_scale_value(shift, width, type, hasshift, haswidth,
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

#if MEOS
/**
 * @ingroup meos_setspan_transf
 * @brief Return an integer set shifted and/or scaled by two values
 * @param[in] s Set
 * @param[in] shift Value for shifting the elements
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numset_shift(), #Numset_scale(), #Numset_shift_scale(),
 */
Set *
intset_shift_scale(const Set *s, int shift, int width, bool hasshift,
  bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_type(s, T_INTSET))
    return NULL;
  return numset_shift_scale(s, Int32GetDatum(shift), Int32GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a big integer set shifted and/or scaled by two values
 * @param[in] s Set
 * @param[in] shift Value for shifting the elements
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numset_shift(), #Numset_scale(), #Numset_shift_scale(),
 */
Set *
bigintset_shift_scale(const Set *s, int64 shift, int64 width, bool hasshift,
  bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_type(s, T_BIGINTSET))
    return NULL;
  return numset_shift_scale(s, Int64GetDatum(shift), Int64GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a float set shifted and/or scaled by two values
 * @param[in] s Set
 * @param[in] shift Value for shifting the elements
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numset_shift(), #Numset_scale(), #Numset_shift_scale(),
 */
Set *
floatset_shift_scale(const Set *s, double shift, double width, bool hasshift,
  bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_type(s, T_FLOATSET))
    return NULL;
  return numset_shift_scale(s, Float8GetDatum(shift), Float8GetDatum(width),
    hasshift, haswidth);
}
/**
 * @ingroup meos_setspan_transf
 * @brief Return a date set shifted and/or scaled by two values
 * @param[in] s Set
 * @param[in] shift Value for shifting the elements
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numset_shift(), #Numset_scale(), #Numset_shift_scale(),
 */
Set *
dateset_shift_scale(const Set *s, int shift, int width, bool hasshift,
  bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_type(s, T_DATESET))
    return NULL;
  return numset_shift_scale(s, Int32GetDatum(shift), Int32GetDatum(width),
    hasshift, haswidth);
}
#endif /* MEOS */

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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_set_isof_type(s, T_TSTZSET) ||
      ! ensure_one_not_null((void *) shift, (void *) duration) ||
      (duration && ! ensure_valid_duration(duration)))
    return NULL;

  Set *result = set_cp(s);
  /* Set the first and last instants */
  TimestampTz lower, lower1, upper, upper1;
  lower = lower1 = DatumGetTimestampTz(SET_VAL_N(s, 0));
  upper = upper1 = DatumGetTimestampTz(SET_VAL_N(s, s->count - 1));
  lower_upper_shift_scale_time(shift, duration, &lower1, &upper1);
  (SET_OFFSETS_PTR(result))[0] = TimestampTzGetDatum(lower1);
  (SET_OFFSETS_PTR(result))[s->count - 1] = TimestampTzGetDatum(upper1);
  if (s->count > 1)
  {
    /* Shift and/or scale from the second to the penultimate instant */
    TimestampTz delta = 0; /* make compiler quiet */
    double scale = 0; /* make compiler quiet */
    if (shift != NULL)
      delta = lower1 - lower;
    if (duration != NULL)
      scale = (double) (upper1 - lower1) / (double) (upper - lower);
    for (int i = 1; i < s->count - 1; i++)
    {
      if (shift != NULL)
        (SET_OFFSETS_PTR(result))[i] += delta;
      if (duration != NULL)
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
  state->set = set_cp(set);
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
  /* Move to the next bucket */
  state->i++;
  if (state->i == state->count)
    state->done = true;
  return;
}

/*****************************************************************************
 * Comparison functions for defining B-tree index
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_comp
 * @brief Return true if the two sets are equal
 * @param[in] s1,s2 Sets
 */
bool
set_eq1(const Set *s1, const Set *s2)
{
  assert(s1); assert(s2); assert(s1->settype == s2->settype);
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
 * @brief Return true if the two sets are equal
 * @param[in] s1,s2 Sets
 * @note The function #set_cmp() is not used to increase efficiency
 * @csqlfn #Set_eq()
 */
bool
set_eq(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_set_type(s1, s2))
    return false;
  return set_eq1(s1, s2);
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first set is not equal to the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Set_ne()
 */
bool
set_ne(const Set *s1, const Set *s2)
{
  return ! set_eq(s1, s2);
}

/**
 * @ingroup meos_internal_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first set is less
 * than, equal, or greater than the second one
 * @param[in] s1,s2 Sets
 */
int
set_cmp_int(const Set *s1, const Set *s2)
{
  assert(s1); assert(s2); assert(s1->settype == s2->settype);
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
 * @brief Return -1, 0, or 1 depending on whether the first set is less
 * than, equal, or greater than the second one
 * @param[in] s1,s2 Sets
 * @return On error return @p INT_MAX
 * @note Function used for B-tree comparison
 * @csqlfn #Set_cmp()
 */
int
set_cmp(const Set *s1, const Set *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_set_type(s1, s2))
    return INT_MAX;
  return set_cmp_int(s1, s2);
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first set is less than the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Set_lt()
 */
bool
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
bool
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
bool
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
bool
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return INT_MAX;

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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return INT_MAX;

  uint64 result = 1;
  for (int i = 0; i < s->count; i++)
  {
    uint64 value_hash = datum_hash_extended(SET_VAL_N(s, i), s->basetype, seed);
    result = (result << 5) - result + value_hash;
  }
  return result;
}

/*****************************************************************************/
