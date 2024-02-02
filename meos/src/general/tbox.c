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
 * @brief Functions for temporal bounding boxes
 */

#include "general/tbox.h"

/* C */
#include <assert.h>
#include <limits.h>
/* PostgreSQL */
#include "utils/timestamp.h"
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/meos_catalog.h"
#include "general/set.h"
#include "general/span.h"
#include "general/spanset.h"
#include "general/temporal.h"
#include "general/tnumber_mathfuncs.h"
#include "general/type_parser.h"
#include "general/type_util.h"

/** Buffer size for input and output of TBox values */
#define MAXTBOXLEN    128

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensure that a temporal box has X values
 */
bool
ensure_has_X_tbox(const TBox *box)
{
  assert(box);
  if (! MEOS_FLAGS_GET_X(box->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The box must have value dimension");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a temporal box has T values
 */
bool
ensure_has_T_tbox(const TBox *box)
{
  assert(box);
  if (! MEOS_FLAGS_GET_T(box->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The box must have time dimension");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a temporal boxes have the same dimensionality
 */
bool
ensure_same_dimensionality_tbox(const TBox *box1, const TBox *box2)
{
  if (MEOS_FLAGS_GET_X(box1->flags) != MEOS_FLAGS_GET_X(box2->flags) ||
    MEOS_FLAGS_GET_T(box1->flags) != MEOS_FLAGS_GET_T(box2->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The boxes must be of the same dimensionality");
    return false;
  }
  return true;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_box_inout
 * @brief Return a temporal box from its Well-Known Text (WKT) representation
 * @details Examples of input:
 * @code
 * TBOX X([1.0, 4.0)) -> only value
 * TBOX XT([1.0, 4.0),[2001-01-01, 2001-01-02]) -> value and time
 * TBOX T([2001-01-01, 2001-01-02]) -> only time
 * @endcode
 * where the commas are optional.
 * @return On error return @p NULL
 * @param[in] str String
 * @csqlfn #Tbox_in()
 */
TBox *
tbox_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return tbox_parse(&str);
}

/**
 * @ingroup meos_box_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal box
 * @param[in] box Temporal box
 * @param[in] maxdd Maximum number of decimal digits
 * @return On error return @p NULL
 * @csqlfn #Tbox_out(), #Tbox_as_text()
 */
char *
tbox_out(const TBox *box, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_negative(maxdd))
    return NULL;

  static size_t size = MAXTBOXLEN + 1;
  char *result = palloc(size);
  char *span = NULL, *period = NULL;
  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  assert(hasx || hast);

  /* Generate the strings for the span and/or the period */
  if (hasx)
    span = span_out(&box->span, maxdd);
  if (hast)
    /* The second argument is not used for periods */
    period = span_out(&box->period, maxdd);

  /* Print the box */
  if (hasx)
  {
    char *spantype = (box->span.basetype == T_INT4) ? "INT" : "FLOAT";
    if (hast)
      snprintf(result, size, "TBOX%s XT(%s,%s)", spantype, span, period);
    else
      snprintf(result, size, "TBOX%s X(%s)", spantype, span);
  }
  else if (hast) /* make compiler quiet */
    snprintf(result, size, "TBOX T(%s)", period);

  if (hasx)
    pfree(span);
  if (hast)
    pfree(period);
  return result;
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/
/**
 * @ingroup meos_box_constructor
 * @brief Return a temporal box from a number span and a timestamptz span
 * @param[in] s Value span
 * @param[in] p Time span
 * @return On error return @p NULL
 */
TBox *
tbox_make(const Span *s, const Span *p)
{
  if (! ensure_one_not_null((void *) s, (void *) p))
    return NULL;
  /* Note: zero-fill is done in function tbox_set */
  TBox *result = palloc(sizeof(TBox));
  tbox_set(s, p, result);
  return result;
}

/**
 * @ingroup meos_internal_box_constructor
 * @brief Return the last argument initialized with a temporal box constructed
 * from a number span and a timestamptz span
 * @param[in] s Value span
 * @param[in] p Time span
 * @param[out] box Result
 * @note This function is equivalent to #tbox_make without memory allocation
 */
void
tbox_set(const Span *s, const Span *p, TBox *box)
{
  /* At least on of the X or T dimensions should be given */
  assert(s || p); assert(box);
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(TBox));
  if (s)
  {
    memcpy(&box->span, s, sizeof(Span));
    MEOS_FLAGS_SET_X(box->flags, true);
  }
  if (p)
  {
    memcpy(&box->period, p, sizeof(Span));
    MEOS_FLAGS_SET_T(box->flags, true);
  }
  return;
}

/**
 * @ingroup meos_internal_box_constructor
 * @brief Return a copy of a temporal box
 * @param[in] box Temporal box
 */
TBox *
tbox_cp(const TBox *box)
{
  assert(box);
  TBox *result = palloc(sizeof(TBox));
  memcpy(result, box, sizeof(TBox));
  return result;
}

#if MEOS
/**
 * @ingroup meos_box_constructor
 * @brief Return a copy of a temporal box
 * @param[in] box Temporal box
 */
TBox *
tbox_copy(const TBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box))
    return NULL;
  return tbox_cp(box);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_internal_box_constructor
 * @brief Return a temporal box from an integer and a timestamptz
 * @param[in] value Value
 * @param[in] basetype Type of the value
 * @param[in] t Timestamp
 * @csqlfn #Number_timestamptz_to_tbox()
 */
TBox *
number_timestamptz_to_tbox(Datum value, meosType basetype, TimestampTz t)
{
  Span s, p;
  meosType spantype = basetype_spantype(basetype);
  span_set(value, value, true, true, basetype, spantype, &s);
  Datum dt = TimestampTzGetDatum(t);
  span_set(dt, dt, true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &p);
  return tbox_make(&s, &p);
}

#if MEOS
/**
 * @ingroup meos_box_constructor
 * @brief Return a temporal box from an integer and a timestamptz
 * @param[in] i Value
 * @param[in] t Timestamp
 * @csqlfn #Number_timestamptz_to_tbox()
 */
TBox *
int_timestamptz_to_tbox(int i, TimestampTz t)
{
  return number_timestamptz_to_tbox(Int32GetDatum(i), T_INT4, t);
}

/**
 * @ingroup meos_box_constructor
 * @brief Return a temporal box from a float and a timestamptz
 * @param[in] d Value
 * @param[in] t Timestamp
 * @csqlfn #Number_timestamptz_to_tbox()
 */
TBox *
float_timestamptz_to_tbox(double d, TimestampTz t)
{
  return number_timestamptz_to_tbox(Float8GetDatum(d), T_FLOAT8, t);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_box_constructor
 * @brief Return a temporal box from an integer and a timestamptz span
 * @param[in] value Value
 * @param[in] basetype Type of the value
 * @param[in] s Time span
 * @csqlfn #Number_tstzspan_to_tbox()
 */
TBox *
number_tstzspan_to_tbox(Datum value, meosType basetype, const Span *s)
{
  assert(s);
  meosType spantype = basetype_spantype(basetype);
  Span s1;
  span_set(value, value, true, true, basetype, spantype, &s1);
  return tbox_make(&s1, s);
}

#if MEOS
/**
 * @ingroup meos_box_constructor
 * @brief Return a temporal box from an integer and a timestamptz span
 * @param[in] i Value
 * @param[in] s Time span
 * @csqlfn #Number_tstzspan_to_tbox()
 */
TBox *
int_tstzspan_to_tbox(int i, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;
  return number_tstzspan_to_tbox(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup meos_box_constructor
 * @brief Return a temporal box from a float and a timestamptz span
 * @param[in] d Value
 * @param[in] s Time span
 * @csqlfn #Number_tstzspan_to_tbox()
 */
TBox *
float_tstzspan_to_tbox(double d, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;
  return number_tstzspan_to_tbox(Float8GetDatum(d), T_FLOAT8, s);
}
#endif /* MEOS */

/**
 * @ingroup meos_box_constructor
 * @brief Return a temporal box from a number span and a timestamptz
 * @param[in] s Value span
 * @param[in] t Timestamp
 * @csqlfn #Numspan_timestamptz_to_tbox()
 */
TBox *
numspan_timestamptz_to_tbox(const Span *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_numspan_type(s->spantype))
    return NULL;

  Datum dt = TimestampTzGetDatum(t);
  Span p;
  span_set(dt, dt, true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &p);
  return tbox_make(s, &p);
}

/**
 * @ingroup meos_box_constructor
 * @brief Return a temporal box from a number span and a timestamptz span
 * @param[in] s Value span
 * @param[in] p Time span
 * @csqlfn #Numspan_timestamptz_to_tbox()
 */
TBox *
numspan_tstzspan_to_tbox(const Span *s, const Span *p)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) p) ||
      ! ensure_numspan_type(s->spantype) ||
      ! ensure_span_isof_type(p, T_TSTZSPAN))
    return NULL;
  return tbox_make(s, p);
}

/*****************************************************************************
 * Conversion functions
 * The set functions start by setting the output argument box to 0
 *****************************************************************************/

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return the last argument initialized with a temporal box constructed
 * from a number
 * @param[in] value Value
 * @param[in] basetype Type of the value
 * @param[out] box Result
 */
void
number_set_tbox(Datum value, meosType basetype, TBox *box)
{
  assert(box); assert(tnumber_basetype(basetype));
  Span s;
  meosType spantype = basetype_spantype(basetype);
  span_set(value, value, true, true, basetype, spantype, &s);
  tbox_set(&s, NULL, box);
  return;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return a numberconverted to a temporal box
 * @param[in] value Value
 * @param[in] basetype Type of the value
 * @csqlfn #Number_to_tbox()
 */
TBox *
number_to_tbox(Datum value, meosType basetype)
{
  if (! ensure_tnumber_basetype(basetype))
    return NULL;
  TBox *result = palloc(sizeof(TBox));
  number_set_tbox(value, basetype, result);
  return result;
}

#if MEOS
/**
 * @ingroup meos_internal_box_conversion
 * @brief Return the last argument initialized with a temporal box constructed
 * from an integer
 * @param[in] i Value
 * @param[out] box Result
 */
void
int_set_tbox(int i, TBox *box)
{
  assert(box);
  number_set_tbox(Int32GetDatum(i), T_INT4, box);
  return;
}

/**
 * @ingroup meos_box_conversion
 * @brief Return an integer converted to a temporal box
 * @param[in] i Value
 * @csqlfn #Number_to_tbox()
 */
TBox *
int_to_tbox(int i)
{
  TBox *result = palloc(sizeof(TBox));
  int_set_tbox(i, result);
  return result;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return the last argument initialized with a temporal box constructed
 * from a float
 * @param[in] d Value
 * @param[out] box Result
 */
void
float_set_tbox(double d, TBox *box)
{
  assert(box);
  number_set_tbox(Float8GetDatum(d), T_FLOAT8, box);
  return;
}

/**
 * @ingroup meos_box_conversion
 * @brief Return a float converted to a temporal box
 * @param[in] d Value
 * @csqlfn #Number_to_tbox()
 */
TBox *
float_to_tbox(double d)
{
  TBox *result = palloc(sizeof(TBox));
  float_set_tbox(d, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return the last argument initialized with a temporal box constructed
 * from a timestamptz
 * @param[in] t Timestamp
 * @param[out] box Result
 */
void
timestamptz_set_tbox(TimestampTz t, TBox *box)
{
  assert(box);
  Span p;
  Datum dt = TimestampTzGetDatum(t);
  span_set(dt, dt, true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &p);
  tbox_set(NULL, &p, box);
  return;
}

/**
 * @ingroup meos_box_conversion
 * @brief Return a timestamptz converted to a temporal box
 * @param[in] t Timestamp
 * @csqlfn #Timestamptz_to_tbox()
 */
TBox *
timestamptz_to_tbox(TimestampTz t)
{
  TBox *result = palloc(sizeof(TBox));
  timestamptz_set_tbox(t, result);
  return result;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return the last argument initialized with a temporal box constructed
 * from a number set
 * @param[in] s Set
 * @param[out] box Result
 */
void
numset_set_tbox(const Set *s, TBox *box)
{
  assert(s); assert(box); assert(numset_type(s->settype));
  Span span;
  set_set_span(s, &span);
  tbox_set(&span, NULL, box);
  return;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return the last argument initialized with a temporal box constructed
 * from a timestamptz set
 * @param[in] s Set
 * @param[out] box Result
 */
void
tstzset_set_tbox(const Set *s, TBox *box)
{
  assert(s); assert(box); assert(s->settype == T_TSTZSET);
  Span p;
  set_set_span(s, &p);
  tbox_set(NULL, &p, box);
  return;
}

/**
 * @ingroup meos_box_conversion
 * @brief Return a number or timestamptz set converted to a temporal box
 * @param[in] s Set
 * @csqlfn #Set_to_tbox()
 */
TBox *
set_to_tbox(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return NULL;
  TBox *result = palloc(sizeof(TBox));
  if (numset_type(s->settype))
    numset_set_tbox(s, result);
  else if (s->settype == T_TSTZSET)
    tstzset_set_tbox(s, result);
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Unknown set type for converting to temporal box: %s",
      meostype_name(s->settype));
    return NULL;
  }
  return result;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return the last argument initialized with a temporal box constructed
 * from a number span
 * @param[in] s Span
 * @param[out] box Result
 */
void
numspan_set_tbox(const Span *s, TBox *box)
{
  assert(s); assert(box); assert(numspan_type(s->spantype));
  tbox_set(s, NULL, box);
  return;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return the last argument initialized with a temporal box
 * constructed from a timestamptz span
 * @param[in] s Span
 * @param[out] box Result
 */
void
tstzspan_set_tbox(const Span *s, TBox *box)
{
  assert(s); assert(box); assert(s->spantype == T_TSTZSPAN);
  tbox_set(NULL, s, box);
  return;
}

/**
 * @ingroup meos_box_conversion
 * @brief Return a number span converted to a temporal box
 * @param[in] s Span
 * @csqlfn #Span_to_tbox()
 */
TBox *
span_to_tbox(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return NULL;

  TBox *result = palloc(sizeof(TBox));
  if (numspan_type(s->spantype))
    numspan_set_tbox(s, result);
  else if (s->spantype == T_TSTZSPAN)
    tstzspan_set_tbox(s, result);
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Unknown span type for converting to temporal box: %s",
      meostype_name(s->spantype));
    return NULL;
  }
  return result;
}

#if MEOS
/**
 * @ingroup meos_internal_box_conversion
 * @brief Return the last argument initialized with a temporal box constructed
 * from a span set
 * @param[in] ss Span set
 * @param[out] box Result
 */
void
numspanset_set_tbox(const SpanSet *ss, TBox *box)
{
  assert(ss); assert(box); assert(tnumber_spansettype(ss->spansettype));
  tbox_set(&ss->span, NULL, box);
  return;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return the last argument initialized with a temporal box constructed
 * from a timestamptz span set
 * @param[in] ss Span set
 * @param[out] box Result
 */
void
tstzspanset_set_tbox(const SpanSet *ss, TBox *box)
{
  assert(ss); assert(box);
  tbox_set(NULL, &ss->span, box);
  return;
}

/**
 * @ingroup meos_box_conversion
 * @brief Return a number span set converted to a temporal box
 * @param[in] ss Span set
 * @csqlfn #Spanset_to_tbox()
 */
TBox *
spanset_to_tbox(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;

  TBox *result = palloc(sizeof(TBox));
  if (tnumber_spansettype(ss->spansettype))
    numspanset_set_tbox(ss, result);
  else if (ss->spansettype == T_TSTZSPANSET)
    tstzspanset_set_tbox(ss, result);
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Unknown span set type for converting to temporal box: %s",
      meostype_name(ss->spansettype));
    return NULL;
  }
  return result;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_box_conversion
 * @brief Return a temporal box converted to an integer span
 * @param[in] box Temporal box
 * @csqlfn #Tbox_to_intspan()
 */
Span *
tbox_to_intspan(const TBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_X_tbox(box))
    return NULL;

  if (box->span.basetype == T_INT4)
    return span_cp(&box->span);
  /* Convert the integer span to a float span */
  Span *result = palloc(sizeof(Span));
  floatspan_set_intspan(&box->span, result);
  return result;
}

/**
 * @ingroup meos_box_conversion
 * @brief Return a temporal box converted to a float span
 * @param[in] box Temporal box
 * @csqlfn #Tbox_to_floatspan()
 */
Span *
tbox_to_floatspan(const TBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_X_tbox(box))
    return NULL;

  if (box->span.basetype == T_FLOAT8)
    return span_cp(&box->span);
  /* Convert the integer span to a float span */
  Span *result = palloc(sizeof(Span));
  intspan_set_floatspan(&box->span, result);
  return result;
}

/**
 * @ingroup meos_box_conversion
 * @brief Return a temporal box converted to a timestamptz span
 * @param[in] box Temporal box
 * @csqlfn #Tbox_to_tstzspan()
 */
Span *
tbox_to_tstzspan(const TBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_T_tbox(box))
    return NULL;
  return span_cp(&box->period);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_box_accessor
 * @brief Return true if a temporal box has value dimension
 * @param[in] box Temporal box
 * @csqlfn #Tbox_hasx()
 */
bool
tbox_hasx(const TBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box))
    return false;
  return MEOS_FLAGS_GET_X(box->flags);
}

/**
 * @ingroup meos_box_accessor
 * @brief Return true if a temporal box has time dimension
 * @param[in] box Temporal box
 * @csqlfn #Tbox_hast()
 */
bool
tbox_hast(const TBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box))
    return false;
  return MEOS_FLAGS_GET_T(box->flags);
}

/**
 * @ingroup meos_box_accessor
 * @brief Return in the last argument the minimum X value of a temporal box
 * @param[in] box Box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Tbox_xmin()
 */
bool
tbox_xmin(const TBox *box, double *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result) ||
      ! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = datum_double(box->span.lower, box->span.basetype);
  return true;
}

#if MEOS
/**
 * @ingroup meos_box_accessor
 * @brief Return in the last argument the minimum X value of a temporal box
 * @param[in] box Box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Tbox_xmin()
 */
bool
tboxint_xmin(const TBox *box, int *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result) ||
      ! MEOS_FLAGS_GET_X(box->flags) ||
      ! ensure_span_isof_basetype(&box->span, T_INT4))
    return false;
  *result = DatumGetInt32(box->span.lower);
  return true;
}

/**
 * @ingroup meos_box_accessor
 * @brief Return in the last argument the minimum X value of a temporal box
 * @param[in] box Box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Tbox_xmin()
 */
bool
tboxfloat_xmin(const TBox *box, double *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result)||
      ! MEOS_FLAGS_GET_X(box->flags) ||
      ! ensure_span_isof_basetype(&box->span, T_FLOAT8) )
    return false;
  *result = DatumGetFloat8(box->span.lower);
  return true;
}
#endif /* MEOS */

/**
 * @ingroup meos_box_accessor
 * @brief Return in the last argument whether the minimum X value of a temporal
 * box is inclusive
 * @param[in] box Box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Tbox_xmin_inc()
 */
bool
tbox_xmin_inc(const TBox *box, bool *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result) ||
      ! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = DatumGetBool(box->span.lower_inc);
  return true;
}

/**
 * @ingroup meos_box_accessor
 * @brief Return in the last argument the maximum X value of a temporal box
 * @param[in] box Box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Tbox_xmax()
 */
bool
tbox_xmax(const TBox *box, double *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result) ||
      ! MEOS_FLAGS_GET_X(box->flags))
    return false;
  if (box->span.basetype == T_INT4)
    /* Integer spans are canonicalized, i.e., the upper bound is exclusive */
    *result = (double) (DatumGetInt32(box->span.upper) - 1);
  else
    *result = DatumGetFloat8(box->span.upper);
  return true;
}

#if MEOS
/**
 * @ingroup meos_box_accessor
 * @brief Return in the last argument the maximum X value of a temporal box
 * @param[in] box Box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Tbox_xmax()
 */
bool
tboxint_xmax(const TBox *box, int *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result) ||
      ! MEOS_FLAGS_GET_X(box->flags) ||
      ! ensure_span_isof_basetype(&box->span, T_INT4))
    return false;
  *result = DatumGetInt32(box->span.upper) - 1;
  return true;
}

/**
 * @ingroup meos_box_accessor
 * @brief Return in the last argument the maximum X value of a temporal box
 * @param[in] box Box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Tbox_xmax()
 */
bool
tboxfloat_xmax(const TBox *box, double *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result) ||
      ! MEOS_FLAGS_GET_X(box->flags) ||
      ! ensure_span_isof_basetype(&box->span, T_FLOAT8))
    return false;
  *result = DatumGetFloat8(box->span.upper);
  return true;
}
#endif /* MEOS */

/**
 * @ingroup meos_box_accessor
 * @brief Return in the last argument whether the maximum X value of a temporal
 * box is inclusive
 * @param[in] box Box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Tbox_xmax_inc()
 */
bool
tbox_xmax_inc(const TBox *box, bool *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result) ||
      ! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = DatumGetBool(box->span.upper_inc);
  return true;
}

/**
 * @ingroup meos_box_accessor
 * @brief Return in the last argument the minimum T value of a temporal box
 * @param[in] box Box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Tbox_tmin()
 */
bool
tbox_tmin(const TBox *box, TimestampTz *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result) ||
      ! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = DatumGetTimestampTz(box->period.lower);
  return true;
}

/**
 * @ingroup meos_box_accessor
 * @brief Return in the last argument whether the minimum T value of a temporal
 * box is inclusive
 * @param[in] box Box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Tbox_tmin_inc()
 */
bool
tbox_tmin_inc(const TBox *box, bool *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result) ||
      ! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = DatumGetBool(box->period.lower_inc);
  return true;
}

/**
 * @ingroup meos_box_accessor
 * @brief Return in the last argument the maximum T value of a temporal box
 * @param[in] box Box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Tbox_tmax()
 */
bool
tbox_tmax(const TBox *box, TimestampTz *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result) ||
      ! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = DatumGetTimestampTz(box->period.upper);
  return true;
}

/**
 * @ingroup meos_box_accessor
 * @brief Return in the last argument whether the maximum T value of a temporal
 * box is inclusive
 * @param[in] box Box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Tbox_tmax_inc()
 */
bool
tbox_tmax_inc(const TBox *box, bool *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result) ||
      ! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = DatumGetBool(box->period.upper_inc);
  return true;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_box_transf
 * @brief Return a temporal box with the value span shifted and/or scaled by
 * the values
 * @param[in] box Temporal box
 * @param[in] shift Value to shift the value span
 * @param[in] width Width of the result
 * @param[in] basetype Type of the values
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 */
TBox *
tbox_shift_scale_value(const TBox *box, Datum shift, Datum width,
  meosType basetype, bool hasshift, bool haswidth)
{
  assert(box);
  /* Ensure validity of the arguments */
  if (! ensure_has_X_tbox(box) || ! ensure_one_true(hasshift, haswidth) ||
      ! ensure_span_isof_basetype(&box->span, basetype) ||
      (haswidth && ! ensure_positive_datum(width, box->span.basetype)))
    return NULL;

  /* Copy the input box to the result */
  TBox *result = tbox_cp(box);
  /* Shift and/or scale the span of the resulting box */
  Datum lower = box->span.lower;
  Datum upper = box->span.upper;
  lower_upper_shift_scale_value(shift, width, box->span.basetype, hasshift,
    haswidth, &lower, &upper);
  result->span.lower = lower;
  result->span.upper = upper;
  return result;
}

#if MEOS
/**
 * @ingroup meos_box_transf
 * @brief Return a temporal box with the value span shifted and/or scaled by
 * the values
 * @param[in] box Temporal box
 * @param[in] shift Value to shift the value span
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Tbox_shift_value(), #Tbox_scale_value(), #Tbox_shift_scale_value()
 */
TBox *
tbox_shift_scale_int(const TBox *box, int shift, int width, bool hasshift,
  bool haswidth)
{
  if (! ensure_not_null((void *) box) ||
      ! ensure_span_isof_basetype(&box->span, T_INT4))
    return NULL;

  return tbox_shift_scale_value(box, Int32GetDatum(shift),
    Int32GetDatum(width), T_INT4, hasshift, haswidth);
}

/**
 * @ingroup meos_box_transf
 * @brief Return a temporal box with the value span shifted and/or scaled by
 * the values
 * @param[in] box Temporal box
 * @param[in] shift Value to shift the value span
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Tbox_shift_value(), #Tbox_scale_value(), #Tbox_shift_scale_value()
 */
TBox *
tbox_shift_scale_float(const TBox *box, double shift, double width,
  bool hasshift, bool haswidth)
{
  if (! ensure_not_null((void *) box) ||
      ! ensure_span_isof_basetype(&box->span, T_FLOAT8))
    return NULL;

  return tbox_shift_scale_value(box, Float8GetDatum(shift),
    Float8GetDatum(width), T_FLOAT8, hasshift, haswidth);
}

#endif /* MEOS */

/**
 * @ingroup meos_box_transf
 * @brief Return a temporal box with the value span shifted and/or scaled by
 * the values
 * @param[in] box Temporal box
 * @param[in] shift Interval to shift the value span, may be NULL
 * @param[in] duration Duration of the result, may be NULL
 * @csqlfn #Tbox_shift_time(), #Tbox_scale_time(), #Tbox_shift_scale_time()
 */
TBox *
tbox_shift_scale_time(const TBox *box, const Interval *shift,
  const Interval *duration)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_T_tbox(box) ||
      ! ensure_one_not_null((void *) shift, (void *) duration) ||
      (duration && ! ensure_valid_duration(duration)))
    return NULL;

  /* Copy the input period to the result */
  TBox *result = tbox_cp(box);
  /* Shift and/or scale the resulting period */
  TimestampTz lower = DatumGetTimestampTz(box->period.lower);
  TimestampTz upper = DatumGetTimestampTz(box->period.upper);
  lower_upper_shift_scale_time(shift, duration, &lower, &upper);
  result->period.lower = TimestampTzGetDatum(lower);
  result->period.upper = TimestampTzGetDatum(upper);
  return result;
}

/**
 * @ingroup meos_internal_box_transf
 * @brief Return the second temporal box expanded with the first one
 * @param[in] box1 Temporal box
 * @param[in,out] box2 Temporal box
 */
void
tbox_expand(const TBox *box1, TBox *box2)
{
  assert(box1); assert(box2);
  if (MEOS_FLAGS_GET_X(box2->flags))
    span_expand(&box1->span, &box2->span);
  if (MEOS_FLAGS_GET_T(box2->flags))
    span_expand(&box1->period, &box2->period);
  return;
}

/**
 * @ingroup meos_box_transf
 * @brief Return a temporal box with the value span expanded by an integer
 * @param[in] box Temporal box
 * @param[in] i Value
 * @csqlfn #Tbox_expand_int()
 */
TBox *
tbox_expand_int(const TBox *box, const int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_X_tbox(box) ||
      ! ensure_span_isof_type(&box->span, T_INTSPAN))
    return NULL;

  TBox *result = tbox_cp(box);
  result->span.lower = Int32GetDatum(DatumGetInt32(result->span.lower) - i);
  result->span.upper = Int32GetDatum(DatumGetInt32(result->span.upper) + i);
  return result;
}

/**
 * @ingroup meos_box_transf
 * @brief Return a temporal box with the value span expanded by a double
 * @param[in] box Temporal box
 * @param[in] d Value
 * @csqlfn #Tbox_expand_float()
 */
TBox *
tbox_expand_float(const TBox *box, const double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_X_tbox(box) ||
      ! ensure_span_isof_type(&box->span, T_FLOATSPAN))
    return NULL;

  TBox *result = tbox_cp(box);
  result->span.lower = Float8GetDatum(DatumGetFloat8(result->span.lower) - d);
  result->span.upper = Float8GetDatum(DatumGetFloat8(result->span.upper) + d);
  return result;
}

/**
 * @ingroup meos_box_transf
 * @brief Return a temporal box with the time span expanded by an interval
 * @param[in] box Temporal box
 * @param[in] interv Interval
 * @csqlfn #Tbox_expand_time()
 */
TBox *
tbox_expand_time(const TBox *box, const Interval *interv)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) interv) ||
      ! ensure_has_T_tbox(box))
    return NULL;

  TBox *result = tbox_cp(box);
  TimestampTz tmin = minus_timestamptz_interval(DatumGetTimestampTz(
    box->period.lower), interv);
  TimestampTz tmax = add_timestamptz_interval(DatumGetTimestampTz(
    box->period.upper), interv);
  result->period.lower = TimestampTzGetDatum(tmin);
  result->period.upper = TimestampTzGetDatum(tmax);
  return result;
}

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

/**
 * @brief Return the ouput variables initialized  with the flag values of the
 * boxes
 * @param[in] box1,box2 Input boxes
 * @param[out] hasx,hast Boolean variables
 */
static bool
topo_tbox_tbox_init(const TBox *box1, const TBox *box2, bool *hasx, bool *hast)
{
  assert(box1); assert(box2); assert(hasx); assert(hast);
  if (! ensure_common_dimension(box1->flags, box2->flags))
    return false;
  *hasx = MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags);
  *hast = MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags);
  if (*hasx && ! ensure_same_span_type(&box1->span, &box2->span))
    return false;
  return true;
}

/**
 * @ingroup meos_box_bbox_topo
 * @brief Return true if the first temporal box contains the second one
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Contains_tbox_tbox()
 */
bool
contains_tbox_tbox(const TBox *box1, const TBox *box2)
{
  bool hasx, hast;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! topo_tbox_tbox_init(box1, box2, &hasx, &hast))
    return false;

  if (hasx && ! cont_span_span(&box1->span, &box2->span))
    return false;
  if (hast && ! cont_span_span(&box1->period, &box2->period))
    return false;
  return true;
}

/**
 * @ingroup meos_box_bbox_topo
 * @brief Return true if the first temporal box is contained in the second one
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Contained_tbox_tbox()
 */
bool
contained_tbox_tbox(const TBox *box1, const TBox *box2)
{
  return contains_tbox_tbox(box2, box1);
}

/**
 * @ingroup meos_box_bbox_topo
 * @brief Return true if two temporal boxes overlap
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Overlaps_tbox_tbox()
 */
bool
overlaps_tbox_tbox(const TBox *box1, const TBox *box2)
{
  bool hasx, hast;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! topo_tbox_tbox_init(box1, box2, &hasx, &hast))
    return false;

  if (hasx && ! over_span_span(&box1->span, &box2->span))
    return false;
  if (hast && ! over_span_span(&box1->period, &box2->period))
    return false;
  return true;
}

/**
 * @ingroup meos_box_bbox_topo
 * @brief Return true if two temporal boxes are equal in the common dimensions
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Same_tbox_tbox()
 */
bool
same_tbox_tbox(const TBox *box1, const TBox *box2)
{
  bool hasx, hast;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! topo_tbox_tbox_init(box1, box2, &hasx, &hast))
    return false;

  if (hasx && ! span_eq_int(&box1->span, &box2->span))
    return false;
  if (hast && ! span_eq_int(&box1->period, &box2->period))
    return false;
  return true;
}

/**
 * @ingroup meos_box_bbox_topo
 * @brief Return true if two temporal boxes are adjacent
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Adjacent_tbox_tbox()
 */
bool
adjacent_tbox_tbox(const TBox *box1, const TBox *box2)
{
  bool hasx, hast;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! topo_tbox_tbox_init(box1, box2, &hasx, &hast))
    return false;

  /* Boxes are adjacent if they are adjacent in at least one dimension */
  bool adjx = false, adjt = false;
  if (hasx)
    adjx = adj_span_span(&box1->span, &box2->span);
  if (hast)
    adjt = adj_span_span(&box1->period, &box2->period);
  return (adjx || adjt);
}

/*****************************************************************************
 * Position operators
 *****************************************************************************/

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first temporal box is to the left of the second
 * one
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Left_tbox_tbox()
 */
bool
left_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_tbox(box1) || ! ensure_has_X_tbox(box2) ||
      ! ensure_same_span_type(&box1->span, &box2->span))
    return false;
  return lf_span_span(&box1->span, &box2->span);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first temporal box does not extend to the right
 * of the second one
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Overleft_tbox_tbox()
 */
bool
overleft_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_tbox(box1) || ! ensure_has_X_tbox(box2) ||
      ! ensure_same_span_type(&box1->span, &box2->span))
    return false;
  return ovlf_span_span(&box1->span, &box2->span);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first temporal box is to the right of the second
 * one
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Right_tbox_tbox()
 */
bool
right_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_tbox(box1) || ! ensure_has_X_tbox(box2) ||
      ! ensure_same_span_type(&box1->span, &box2->span))
    return false;
  return ri_span_span(&box1->span, &box2->span);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first temporal box does not extend to the left of
 * the second one
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Overright_tbox_tbox()
 */
bool
overright_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_tbox(box1) || ! ensure_has_X_tbox(box2) ||
      ! ensure_same_span_type(&box1->span, &box2->span))
    return false;

  return ovri_span_span(&box1->span, &box2->span);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first temporal box is before the second one
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Before_tbox_tbox()
 */
bool
before_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_T_tbox(box1) || ! ensure_has_T_tbox(box2))
    return false;
  return lf_span_span(&box1->period, &box2->period);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first temporal box is not after the second one
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Overbefore_tbox_tbox()
 */
bool
overbefore_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_T_tbox(box1) || ! ensure_has_T_tbox(box2))
    return false;
  return ovlf_span_span(&box1->period, &box2->period);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first temporal box is after the second one
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #After_tbox_tbox()
 */
bool
after_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_T_tbox(box1) || ! ensure_has_T_tbox(box2))
    return false;
  return ri_span_span(&box1->period, &box2->period);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first temporal box is not before the second one
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Overafter_tbox_tbox()
 */
bool
overafter_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_T_tbox(box1) || ! ensure_has_T_tbox(box2))
    return false;
  return ovri_span_span(&box1->period, &box2->period);
}

/*****************************************************************************
 * Set operators
 *****************************************************************************/

/**
 * @ingroup meos_box_set
 * @brief Return the union of two temporal boxes
 * @param[in] box1,box2 Temporal boxes
 * @param[in] strict True when the boxes must overlap
 * @csqlfn #Union_tbox_tbox()
 */
TBox *
union_tbox_tbox(const TBox *box1, const TBox *box2, bool strict)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_same_dimensionality_tbox(box1, box2) ||
      (MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags) &&
        ! ensure_same_span_type(&box1->span, &box2->span)))
    return NULL;

  /* The union of boxes that do not intersect cannot be represented by a box */
  if (strict && ! overlaps_tbox_tbox(box1, box2))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Result of box union would not be contiguous");
    return NULL;
  }

  bool hasx = MEOS_FLAGS_GET_X(box1->flags);
  bool hast = MEOS_FLAGS_GET_T(box1->flags);
  Span period, span;
  if (hast)
    bbox_union_span_span(&box1->period, &box2->period, &period);
  if (hasx)
    bbox_union_span_span(&box1->span, &box2->span, &span);
  return tbox_make(hasx ? &span : NULL, hast ? &period : NULL);
}

/**
 * @ingroup meos_internal_box_set
 * @brief Return the last argument initialized with the intersection of two
 * temporal boxes
 * @param[in] box1,box2 Temporal boxes
 * @param[out] result Resulting box
 * @note This function is equivalent to @ref intersection_tbox_tbox without
 * memory allocation
 */
bool
inter_tbox_tbox(const TBox *box1, const TBox *box2, TBox *result)
{
  assert(box1); assert(box2);
  bool hasx = MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags);
  bool hast = MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags);
  /* If there is no common dimension */
  if ((! hasx && ! hast) ||
    /* If they do no intersect in one common dimension */
    (hasx && ! over_span_span(&box1->span, &box2->span)) ||
    (hast && ! over_span_span(&box1->period, &box2->period)))
    return false;

  Span period, span;
  if (hast)
    inter_span_span(&box1->period, &box2->period, &period);
  if (hasx)
    inter_span_span(&box1->span, &box2->span, &span);
  tbox_set(hasx ? &span : NULL, hast ? &period : NULL, result);
  return true;
}

/**
 * @ingroup meos_box_set
 * @brief Return the intersection of two temporal boxes
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Intersection_tbox_tbox()
 */
TBox *
intersection_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      (MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags) &&
        ! ensure_same_span_type(&box1->span, &box2->span)))
    return NULL;

  TBox *result = palloc(sizeof(TBox));
  if (! inter_tbox_tbox(box1, box2, result))
  {
    pfree(result);
    return NULL;
  }
  return result;
}

/*****************************************************************************
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

/**
 * @ingroup meos_box_comp
 * @brief Return true if two temporal boxes are equal
 * @note The function #tbox_cmp is not used to increase efficiency
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Tbox_eq()
 */
bool
tbox_eq(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2))
    return false;

  if (MEOS_FLAGS_GET_X(box1->flags) != MEOS_FLAGS_GET_X(box2->flags) ||
    MEOS_FLAGS_GET_T(box1->flags) != MEOS_FLAGS_GET_T(box2->flags))
      return false;
  if (! span_eq_int(&box1->span, &box2->span) ||
      ! span_eq_int(&box1->period, &box2->period))
    return false;
  /* The two boxes are equal */
  return true;
}

/**
 * @ingroup meos_box_comp
 * @brief Return true if two temporal boxes are different
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Tbox_ne()
 */
bool
tbox_ne(const TBox *box1, const TBox *box2)
{
  return ! tbox_eq(box1, box2);
}

/**
 * @ingroup meos_box_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal box
 * is less than, equal to, or greater than the second one
 * @param[in] box1,box2 Temporal boxes
 * @note The time dimension is compared first and then the value dimension
 * @csqlfn #Tbox_cmp()
 */
int
tbox_cmp(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2))
    return INT_MAX;

  bool hasx = MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags);
  bool hast = MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags);
  int cmp;
  if (hast)
  {
    cmp = span_cmp_int(&box1->period, &box2->period);
    /* Compare the box minima */
    if (cmp != 0)
      return cmp;
  }
  if (hasx)
  {
    cmp = span_cmp_int(&box1->span, &box2->span);
    /* Compare the box minima */
    if (cmp != 0)
      return cmp;
  }
  /* Finally compare the flags */
  if (box1->flags < box2->flags)
    return -1;
  if (box1->flags > box2->flags)
    return 1;
  /* The two boxes are equal */
  return 0;
}

/**
 * @ingroup meos_box_comp
 * @brief Return true if the first temporal box is less than the second one
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Tbox_lt()
 */
bool
tbox_lt(const TBox *box1, const TBox *box2)
{
  int cmp = tbox_cmp(box1, box2);
  return cmp < 0;
}

/**
 * @ingroup meos_box_comp
 * @brief Return true if the first temporal box is less than or equal to the
 * second one
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Tbox_le()
 */
bool
tbox_le(const TBox *box1, const TBox *box2)
{
  int cmp = tbox_cmp(box1, box2);
  return cmp <= 0;
}

/**
 * @ingroup meos_box_comp
 * @brief Return true if the first temporal box is greater than or equal
 * to the second one
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Tbox_ge()
 */
bool
tbox_ge(const TBox *box1, const TBox *box2)
{
  int cmp = tbox_cmp(box1, box2);
  return cmp >= 0;
}

/**
 * @ingroup meos_box_comp
 * @brief Return true if the first temporal box is greater than the second one
 * @param[in] box1,box2 Temporal boxes
 * @csqlfn #Tbox_gt()
 */
bool
tbox_gt(const TBox *box1, const TBox *box2)
{
  int cmp = tbox_cmp(box1, box2);
  return cmp > 0;
}

/*****************************************************************************/
