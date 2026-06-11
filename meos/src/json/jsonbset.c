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
 * @brief Sets of JSONB type
 */

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/jsonb.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/set.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "json/tjsonb.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_json_set_inout
 * @brief Return a JSONB set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
jsonbset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  /* Parse a set of JSONB values */
  return set_parse(&str, T_JSONBSET);
}

/**
 * @ingroup meos_json_set_inout
 * @brief Return the string representation of a JSONB set
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Set_out()
 */
char *
jsonbset_out(const Set *s, int maxdd UNUSED)
{
  /* Ensure the validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_JSONBSET))
    return NULL;
  /* Delegate to the generic set_out, with zero precision */
  return set_out(s, 0);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_json_set_conversion
 * @brief Convert a JSONB value into a JSONB set
 * @param[in] jb JSONB value
 * @csqlfn #Value_to_set()
 */
Set *
jsonb_to_set(const Jsonb *jb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(jb, NULL);
  Datum v = PointerGetDatum(jb);
  /* Make a set of one element, unordered */
  return set_make_exp(&v, 1, 1, T_JSONB, ORDER_NO);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_json_set_constructor
 * @brief Return a JSONB set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
jsonbset_make(const Jsonb **values, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = PointerGetDatum(values[i]);
  return set_make_free(datums, count, T_JSONB, ORDER);
}
#endif /* MEOS */

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_json_set_accessor
 * @brief Return a copy of the first value of a JSONB set
 * @param[in] s JSONB set
 * @return On error return NULL
 * @csqlfn #Set_start_value()
 */
Jsonb *
jsonbset_start_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(s, NULL);
  /* Copy the datum and return as JSONB pointer */
  return DatumGetJsonbP(datum_copy(SET_VAL_N(s, 0), s->basetype));
}

/**
 * @ingroup meos_json_set_accessor
 * @brief Return a copy of the last value of a JSONB set
 * @param[in] s JSONB set
 * @return On error return NULL
 * @csqlfn #Set_end_value()
 */
Jsonb *
jsonbset_end_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(s, NULL);
  /* Copy the datum and return as JSONB pointer */
  return DatumGetJsonbP(datum_copy(SET_VAL_N(s, s->count - 1), s->basetype));
}

/**
 * @ingroup meos_json_set_accessor
 * @brief Return in the last argument a copy of the n-th value of a JSONB set
 * @param[in] s JSONB set
 * @param[in] n Number (1-based)
 * @param[out] result JSONB pointer
 * @return Return true if the value is found
 * @csqlfn #Set_value_n()
 */

bool
jsonbset_value_n(const Set *s, int n, Jsonb **result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(s, false); VALIDATE_NOT_NULL(result, false);
  if (n < 1 || n > s->count)
    return false;
  /* Copy the datum and return as JSONB pointer */
  *result = DatumGetJsonbP(datum_copy(SET_VAL_N(s, n - 1), s->basetype));
  return true;
}

/**
 * @ingroup meos_json_set_accessor
 * @brief Return an array of copies of the values of a JSONB set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
Jsonb **
jsonbset_values(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(s, NULL);
  Jsonb **result = palloc(sizeof(Jsonb *) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetJsonbP(datum_copy(SET_VAL_N(s, i), s->basetype));
  return result;
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Return the concatenation of a text set and a set (internal function)
 * @param[in] s Set
 * @param[in] func Function to apply
 * @param[in] settype Set type 
 * @param[in] restype Result type 
 */
Set *
jsonbfunc_jsonbset(const Set *s, datum_func1 func, MeosType settype UNUSED,
  MeosType restype)
{
  assert(s); assert(s->settype == settype);
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = func(SET_VAL_N(s, i));
  return set_make_free(values, s->count, restype, ORDER_NO);
}

/**
 * @brief Return the concatenation of a text set and a set (internal function)
 * @param[in] s Set
 * @param[in] jb JSONB
 * @param[in] func Function to apply
 * @param[in] invert True when the arguments must be inverted
 */
Set *
jsonbfunc_jsonbset_jsonb(const Set *s, const Jsonb *jb, datum_func2 func,
  bool invert)
{
  assert(s); assert(jb); assert(s->settype == T_JSONBSET);
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = invert ?
      func(PointerGetDatum(jb), SET_VAL_N(s, i)) :
      func(SET_VAL_N(s, i), PointerGetDatum(jb));
  return set_make_free(values, s->count, T_JSONB, ORDER_NO);
}

/**
 * @brief Return the concatenation of a text set and a set (internal function)
 * @param[in] s Set
 * @param[in] txt Key
 * @param[in] func Function to apply
 */
Set *
jsonbfunc_jsonbset_text(const Set *s, const text *txt, datum_func2 func)
{
  assert(s); assert(txt); assert(s->settype == T_JSONBSET);
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
    values[i] = func(SET_VAL_N(s, i), PointerGetDatum(txt));
  return set_make_free(values, s->count, T_JSONB, ORDER_NO);
}

/*****************************************************************************
 * Operators
 *****************************************************************************/

// /**
 // * @ingroup meos_json_set_json
 // * @brief Return the concatenation of a JSONB set and a JSONB value
 // * @param[in] s JSONB set
 // * @param[in] jb JSONB value
 // * @csqlfn #Concat_jsonbset_jsonb()
 // */
// Set *
// concat_jsonbset_jsonb(const Set *s, const Jsonb *jb, bool invert)
// {
  // /* Ensure the validity of the arguments */
  // VALIDATE_JSONBSET(s, NULL); VALIDATE_NOT_NULL(jb, NULL);
  // return jsonbfunc_jsonbset_jsonb(s, jb, &datum_jsonb_concat, invert);
// }

/**
 * @ingroup meos_json_set_json
 * @brief Return true if a set contains a JSONB value
 * @param[in] s Set
 * @param[in] jb Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_jsonb(const Set *s, Jsonb *jb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(s, false); VALIDATE_NOT_NULL(jb, false);
  return contains_set_value(s, PointerGetDatum(jb));
}

/**
 * @ingroup meos_json_set_json
 * @brief Return true if a JSONB value is contained in a set
 * @param[in] jb Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_jsonb_set(const Jsonb *jb, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(s, false); VALIDATE_NOT_NULL(jb, false);
  return contained_value_set(PointerGetDatum(jb), s);
}

/**
 * @ingroup meos_json_set_json
 * @brief Return the union of a set and a JSONB value
 * @param[in] s Set
 * @param[in] jb Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_jsonb(const Set *s, const Jsonb *jb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(s, NULL); VALIDATE_NOT_NULL(jb, NULL);
  return union_set_value(s, PointerGetDatum(jb));
}

/**
 * @ingroup meos_json_set_json
 * @brief Return the union of a JSONB value and a set
 * @param[in] jb Value
 * @param[in] s Set
 */
inline Set *
union_jsonb_set(const Jsonb *jb, const Set *s)
{
  return union_set_jsonb(s, jb);
}

/**
 * @ingroup meos_json_set_json
 * @brief Return the intersection of a set and a JSONB value
 * @param[in] s Set
 * @param[in] jb Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_jsonb(const Set *s, const Jsonb *jb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(s, NULL); VALIDATE_NOT_NULL(jb, NULL);
  return intersection_set_value(s, PointerGetDatum(jb));
}

/**
 * @ingroup meos_json_set_json
 * @brief Return the intersection of a JSONB value and a set
 * @param[in] jb Value
 * @param[in] s Set
 */
inline Set *
intersection_jsonb_set(const Jsonb *jb, const Set *s)
{
  return intersection_set_jsonb(s, jb);
}

/**
 * @ingroup meos_json_set_json
 * @brief Return the difference of a JSONB value and a set
 * @param[in] jb Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_jsonb_set(const Jsonb *jb, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(s, NULL); VALIDATE_NOT_NULL(jb, NULL);
  return minus_value_set(PointerGetDatum(jb), s);
}

/**
 * @ingroup meos_json_set_json
 * @brief Return the difference of a set and a JSONB value
 * @param[in] s Set
 * @param[in] jb Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_jsonb(const Set *s, const Jsonb *jb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(s, NULL); VALIDATE_NOT_NULL(jb, NULL);
  return minus_set_value(s, PointerGetDatum(jb));
}

/*****************************************************************************
 * Aggregate functions for set types
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_json_set_json
 * @brief Transition function for set union aggregate of JSONB values
 * @param[in,out] state Current aggregate state
 * @param[in] jb Value
 */
Set *
jsonb_union_transfn(Set *state, const Jsonb *jb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(jb, NULL);
  if (state && ! ensure_set_isof_type(state, T_JSONBSET))
    return NULL;
  return value_union_transfn(state, PointerGetDatum(jb), T_JSONB);
}
#endif /* MEOS */

/*****************************************************************************/
