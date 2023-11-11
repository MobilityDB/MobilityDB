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
 * @brief Functions for temporal bounding boxes.
 */

#include "general/tbox.h"

/* C */
#include <assert.h>
#include <limits.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/set.h"
#include "general/spanset.h"
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
 * @ingroup libmeos_box_inout
 * @brief Return a temporal box from its Well-Known Text (WKT) representation.
 *
 * Examples of input:
 * @code
 * TBOX X([1.0, 4.0)) -> only value
 * TBOX XT([1.0, 4.0),[2001-01-01, 2001-01-02]) -> value and time
 * TBOX T([2001-01-01, 2001-01-02]) -> only time
 * @endcode
 * where the commas are optional.
 * @return On error return NULL
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
 * @ingroup libmeos_box_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal box.
 * @return On error return NULL
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
 * @ingroup libmeos_box_constructor
 * @brief Construct a temporal box from a number span and a period.
 * @return On error return NULL
 * @sqlfunc tbox()
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
 * @ingroup libmeos_internal_box_constructor
 * @brief Set a temporal box from a number span and a period
 * @note This function is equivalent to @ref tbox_make without memory
 * allocation
 */
void
tbox_set(const Span *s, const Span *p, TBox *box)
{
  /* At least on of the X or T dimensions should be given */
  assert(s || p);
  assert(box);
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
 * @ingroup libmeos_box_constructor
 * @brief Return a copy of a temporal box.
 */
TBox *
tbox_copy(const TBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box))
    return NULL;

  TBox *result = palloc(sizeof(TBox));
  memcpy(result, box, sizeof(TBox));
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_box_constructor
 * @brief Return a temporal box from an integer and a timestamp
 * @sqlfunc tbox()
 */
TBox *
number_timestamptz_to_tbox(Datum d, meosType basetype, TimestampTz t)
{
  Span s, p;
  span_set(d, d, true, true, basetype, &s);
  Datum dt = TimestampTzGetDatum(t);
  span_set(dt, dt, true, true, T_TIMESTAMPTZ, &p);
  return tbox_make(&s, &p);
}

#if MEOS
/**
 * @ingroup libmeos_box_constructor
 * @brief Return a temporal box from an integer and a timestamp
 * @sqlfunc tbox()
 */
TBox *
int_timestamptz_to_tbox(int i, TimestampTz t)
{
  return number_timestamptz_to_tbox(Int32GetDatum(i), T_INT4, t);
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Return a temporal box from a float and a timestamp
 * @sqlfunc tbox()
 */
TBox *
float_timestamptz_to_tbox(double d, TimestampTz t)
{
  return number_timestamptz_to_tbox(Float8GetDatum(d), T_FLOAT8, t);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_box_constructor
 * @brief Return a temporal box from an integer and a timestamptz span
 * @sqlfunc tbox()
 */
TBox *
number_tstzspan_to_tbox(Datum d, meosType basetype, const Span *s)
{
  assert(s);
  Span s1;
  span_set(d, d, true, true, basetype, &s1);
  return tbox_make(&s1, s);
}

#if MEOS
/**
 * @ingroup libmeos_box_constructor
 * @brief Return a temporal box from an integer and a timestamptz span
 * @sqlfunc tbox()
 */
TBox *
int_tstzspan_to_tbox(int i, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_TSTZSPAN))
    return NULL;
  return number_tstzspan_to_tbox(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Return a temporal box from a float and a timestamptz span
 * @sqlfunc tbox()
 */
TBox *
float_tstzspan_to_tbox(double d, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_TSTZSPAN))
    return NULL;
  return number_tstzspan_to_tbox(Float8GetDatum(d), T_FLOAT8, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_box_constructor
 * @brief Return a temporal box from a span and a timestamp
 * @sqlfunc tbox()
 */
TBox *
numspan_timestamptz_to_tbox(const Span *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_numspan_type(s->spantype))
    return NULL;

  Datum dt = TimestampTzGetDatum(t);
  Span p;
  span_set(dt, dt, true, true, T_TIMESTAMPTZ, &p);
  return tbox_make(s, &p);
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Return a temporal box from a span and a timestamptz span
 * @sqlfunc tbox()
 */
TBox *
numspan_tstzspan_to_tbox(const Span *s, const Span *p)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) p) ||
      ! ensure_numspan_type(s->spantype) ||
      ! ensure_span_has_type(p, T_TSTZSPAN))
    return NULL;
  return tbox_make(s, p);
}

/*****************************************************************************
 * Conversion functions
 * The set functions start by setting the output argument box to 0
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_box_conversion
 * @brief Set a temporal box from a number.
 */
void
number_set_tbox(Datum value, meosType basetype, TBox *box)
{
  assert(box);
  assert(tnumber_basetype(basetype));
  Span s;
  span_set(value, value, true, true, basetype, &s);
  tbox_set(&s, NULL, box);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_internal_box_conversion
 * @brief Set a temporal box from an integer.
 */
void
int_set_tbox(int i, TBox *box)
{
  assert(box);
  return number_set_tbox(Int32GetDatum(i), T_INT4, box);
}

/**
 * @ingroup libmeos_box_conversion
 * @brief Convert an integer to a temporal box.
 * @sqlfunc tbox()
 * @sqlop @p ::
 */
TBox *
int_to_tbox(int i)
{
  TBox *result = palloc(sizeof(TBox));
  int_set_tbox(i, result);
  return result;
}

/**
 * @ingroup libmeos_internal_box_conversion
 * @brief Set a temporal box from a float.
 */
void
float_set_tbox(double d, TBox *box)
{
  assert(box);
  return number_set_tbox(Float8GetDatum(d), T_FLOAT8, box);
}

/**
 * @ingroup libmeos_box_conversion
 * @brief Convert a float to a temporal box.
 * @sqlfunc tbox()
 * @sqlop @p ::
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
 * @ingroup libmeos_internal_box_conversion
 * @brief Set a temporal box from a timestamp.
 */
void
timestamptz_set_tbox(TimestampTz t, TBox *box)
{
  assert(box);
  Span p;
  Datum dt = TimestampTzGetDatum(t);
  span_set(dt, dt, true, true, T_TIMESTAMPTZ, &p);
  tbox_set(NULL, &p, box);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_box_conversion
 * @brief Convert a timestamp to a temporal box.
 * @sqlfunc tbox()
 * @sqlop @p ::
 */
TBox *
timestamptz_to_tbox(TimestampTz t)
{
  TBox *result = palloc(sizeof(TBox));
  timestamptz_set_tbox(t, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_box_conversion
 * @brief Set a temporal box from a number set.
 */
void
numset_set_tbox(const Set *s, TBox *box)
{
  assert(s); assert(box);
  Span span;
  set_set_span(s, &span);
  tbox_set(&span, NULL, box);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_box_conversion
 * @brief Convert a number set to a temporal box.
 * @sqlfunc tbox()
 * @sqlop @p ::
 */
TBox *
numset_to_tbox(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_numset_type(s->settype))
    return NULL;

  TBox *result = palloc(sizeof(TBox));
  numset_set_tbox(s, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_box_conversion
 * @brief Set a temporal box from a timestamp set.
 */
void
tstzset_set_tbox(const Set *s, TBox *box)
{
  assert(s); assert(box);
  Span p;
  set_set_span(s, &p);
  tbox_set(NULL, &p, box);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_box_conversion
 * @brief Convert a timestamp set to a temporal box.
 * @sqlfunc tbox()
 * @sqlop @p ::
 */
TBox *
tstzset_to_tbox(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_has_type(s, T_TSTZSET))
    return NULL;

  TBox *result = palloc(sizeof(TBox));
  tstzset_set_tbox(s, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_box_conversion
 * @brief Set a temporal box from a number span.
 */
void
numspan_set_tbox(const Span *s, TBox *box)
{
  assert(s); assert(box);
  tbox_set(s, NULL, box);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_box_conversion
 * @brief Convert a span to a temporal box.
 * @sqlfunc tbox()
 * @sqlop @p ::
 */
TBox *
numspan_to_tbox(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_numspan_type(s->spantype))
    return NULL;

  TBox *result = palloc(sizeof(TBox));
  numspan_set_tbox(s, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_box_conversion
 * @brief Set a temporal box from a period.
 */
void
tstzspan_set_tbox(const Span *s, TBox *box)
{
  assert(s); assert(box);
  tbox_set(NULL, s, box);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_box_conversion
 * @brief Convert a period to a temporal box.
 * @sqlfunc tbox()
 * @sqlop @p ::
 */
TBox *
tstzspan_to_tbox(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_span_has_type(s, T_TSTZSPAN))
    return NULL;

  TBox *result = palloc(sizeof(TBox));
  tstzspan_set_tbox(s, result);
  return result;
}

/**
 * @ingroup libmeos_internal_box_conversion
 * @brief Set a temporal box from a span set.
 */
void
numspanset_set_tbox(const SpanSet *ss, TBox *box)
{
  assert(ss); assert(box);
  assert(tnumber_spansettype(ss->spansettype));
  tbox_set(&ss->span, NULL, box);
  return;
}

/**
 * @ingroup libmeos_box_conversion
 * @brief Convert a span set to a temporal box.
 * @sqlfunc tbox()
 * @sqlop @p ::
 */
TBox *
numspanset_to_tbox(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_numspan_type(ss->spantype))
    return NULL;

  TBox *result = palloc(sizeof(TBox));
  numspanset_set_tbox(ss, result);
  return result;
}

/**
 * @ingroup libmeos_internal_box_conversion
 * @brief Set a temporal box from a period set.
 */
void
tstzspanset_set_tbox(const SpanSet *ss, TBox *box)
{
  assert(ss); assert(box);
  tbox_set(NULL, &ss->span, box);
  return;
}

/**
 * @ingroup libmeos_box_conversion
 * @brief Convert a period set to a temporal box.
 * @sqlfunc tbox()
 * @sqlop @p ::
 */
TBox *
tstzspanset_to_tbox(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_TSTZSPANSET))
    return NULL;

  TBox *result = palloc(sizeof(TBox));
  tstzspanset_set_tbox(ss, result);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_box_conversion
 * @brief Convert a temporal box as a span.
 * @sqlop @p ::
 */
Span *
tbox_to_floatspan(const TBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box))
    return NULL;

  if (! MEOS_FLAGS_GET_X(box->flags))
    return NULL;
  if (box->span.basetype == T_FLOAT8)
    return span_copy(&box->span);
  /* Convert the integer span to a float span */
  Span *result = palloc(sizeof(Span));
  intspan_set_floatspan(&box->span, result);
  return result;
}

/**
 * @ingroup libmeos_box_conversion
 * @brief Convert a temporal box as a period
 * @sqlop @p ::
 */
Span *
tbox_to_tstzspan(const TBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box))
    return NULL;

  if (! MEOS_FLAGS_GET_T(box->flags))
    return NULL;
  return span_copy(&box->period);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if a temporal box has value dimension
 * @sqlfunc hasX()
 */
bool
tbox_hasx(const TBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box))
    return false;
  bool result = MEOS_FLAGS_GET_X(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if a temporal box has time dimension
 * @sqlfunc hasT()
 */
bool
tbox_hast(const TBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box))
    return false;
  bool result = MEOS_FLAGS_GET_T(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return the minimum X value of a temporal box.
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Xmin()
 */
bool
tbox_xmin(const TBox *box, double *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = datum_double(box->span.lower, box->span.basetype);
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the minimum X value of a temporal box is inclusive.
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Xmin_inc()
 */
bool
tbox_xmin_inc(const TBox *box, bool *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = DatumGetBool(box->span.lower_inc);
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return the maximum X value of a temporal box.
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Xmax()
 */
bool
tbox_xmax(const TBox *box, double *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_X(box->flags))
    return false;
  if (box->span.basetype == T_INT4)
    /* Integer spans are canonicalized, i.e., the upper bound is exclusive */
    *result = (double) (DatumGetInt32(box->span.upper) - 1);
  else
    *result = DatumGetFloat8(box->span.upper);
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the maximum X value of a temporal box is inclusive.
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Xmax_inc()
 */
bool
tbox_xmax_inc(const TBox *box, bool *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = DatumGetBool(box->span.upper_inc);
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return the minimum T value of a temporal box.
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Tmin()
 */
bool
tbox_tmin(const TBox *box, TimestampTz *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = DatumGetTimestampTz(box->period.lower);
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the minimum T value of a temporal box is inclusive.
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Tmin_inc()
 */
bool
tbox_tmin_inc(const TBox *box, bool *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = DatumGetBool(box->period.lower_inc);
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Compute the maximum T value of a temporal box.
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Tmax()
 */
bool
tbox_tmax(const TBox *box, TimestampTz *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = DatumGetTimestampTz(box->period.upper);
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the maximum T value of a temporal box is inclusive.
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Tmax_inc()
 */
bool
tbox_tmax_inc(const TBox *box, bool *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = DatumGetBool(box->period.upper_inc);
  return true;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_box_transf
 * @brief Shift and/or scale the span of a temporal box by the values.
 */
TBox *
tbox_shift_scale_value(const TBox *box, Datum shift, Datum width,
  bool hasshift, bool haswidth)
{
  assert(box);
  /* Ensure validity of the arguments */
  if (! ensure_has_X_tbox(box) ||
      ! ensure_one_shift_width(hasshift, haswidth) ||
      (width && ! ensure_positive_datum(width, box->span.basetype)))
    return NULL;

  /* Copy the input box to the result */
  TBox *result = tbox_copy(box);
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
 * @ingroup libmeos_box_transf
 * @brief Shift and/or scale the span of a temporal box by the values.
 * @sqlfunc shiftValue(), scaleValue(), shiftScaleValue()
 */
TBox *
tbox_shift_scale_int(const TBox *box, int shift, int width,
  bool hasshift, bool haswidth)
{
  if (! ensure_not_null((void *) box) ||
      ! ensure_same_span_basetype(&box->span, T_FLOAT8))
    return NULL;

  return tbox_shift_scale_value(box, Int32GetDatum(shift),
    Int32GetDatum(width), hasshift, haswidth);
}

/**
 * @ingroup libmeos_box_transf
 * @brief Shift and/or scale the span of a temporal box by the values.
 * @sqlfunc shiftValue(), scaleValue(), shiftScaleValue()
 */
TBox *
tbox_shift_scale_float(const TBox *box, double shift, double width,
  bool hasshift, bool haswidth)
{
  if (! ensure_not_null((void *) box) ||
      ! ensure_same_span_basetype(&box->span, T_FLOAT8))
    return NULL;

  return tbox_shift_scale_value(box, Float8GetDatum(shift),
    Float8GetDatum(width), hasshift, haswidth);
}

#endif /* MEOS */

/**
 * @ingroup libmeos_box_transf
 * @brief Shift and/or scale the period of a temporal box by the intervals.
 * @sqlfunc shiftTime(), scaleTime(), shiftScaleTime()
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
  TBox *result = tbox_copy(box);
  /* Shift and/or scale the resulting period */
  TimestampTz lower = DatumGetTimestampTz(box->period.lower);
  TimestampTz upper = DatumGetTimestampTz(box->period.upper);
  lower_upper_shift_scale_time(shift, duration, &lower, &upper);
  result->period.lower = TimestampTzGetDatum(lower);
  result->period.upper = TimestampTzGetDatum(upper);
  return result;
}

/**
 * @ingroup libmeos_internal_box_transf
 * @brief Expand the second temporal box with the first one
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
 * @ingroup libmeos_box_transf
 * @brief Return a temporal box expanded in the value dimension by a double.
 * @sqlfunc @p expandValue()
 */
TBox *
tbox_expand_value(const TBox *box, const double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_X_tbox(box))
    return NULL;

  TBox *result = tbox_copy(box);
  result->span.lower = Float8GetDatum(DatumGetFloat8(result->span.lower) - d);
  result->span.upper = Float8GetDatum(DatumGetFloat8(result->span.upper) + d);
  return result;
}

/**
 * @ingroup libmeos_box_transf
 * @brief Return a temporal box expanded in the time dimension by an interval.
 * @sqlfunc expandTime()
 */
TBox *
tbox_expand_time(const TBox *box, const Interval *interval)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) interval) ||
      ! ensure_has_T_tbox(box))
    return NULL;

  TBox *result = tbox_copy(box);
  TimestampTz tmin = pg_timestamp_mi_interval(DatumGetTimestampTz(
    box->period.lower), interval);
  TimestampTz tmax = pg_timestamp_pl_interval(DatumGetTimestampTz(
    box->period.upper), interval);
  result->period.lower = TimestampTzGetDatum(tmin);
  result->period.upper = TimestampTzGetDatum(tmax);
  return result;
}

/**
 * @ingroup libmeos_box_transf
 * @brief Set the precision of the value dimension of the temporal box to
 * the number of decimal places.
 */
TBox *
tbox_round(const TBox *box, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_X_tbox(box) ||
      ! ensure_not_negative(maxdd))
    return NULL;

  TBox *result = tbox_copy(box);
  Datum size = Int32GetDatum(maxdd);
  result->span.lower = datum_round_float(box->span.lower, size);
  result->span.upper = datum_round_float(box->span.upper, size);
  return result;
}


/*****************************************************************************
 * Topological operators
 *****************************************************************************/

/**
 * @brief Set the ouput variables with the values of the flags of the boxes.
 *
 * @param[in] box1,box2 Input boxes
 * @param[out] hasx,hast Boolean variables
 */
static void
tbox_tbox_flags(const TBox *box1, const TBox *box2, bool *hasx, bool *hast)
{
  assert(box1); assert(box2); assert(hasx); assert(hast);
  *hasx = MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags);
  *hast = MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags);
  return;
}

/**
 * @brief Set the ouput variables with the values of the flags of the boxes.
 *
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
 * @ingroup libmeos_box_bbox_topo
 * @brief Return true if the first temporal box contains the second one.
 * @sqlop @p \@>
 */
bool
contains_tbox_tbox(const TBox *box1, const TBox *box2)
{
  bool hasx, hast;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! topo_tbox_tbox_init(box1, box2, &hasx, &hast))
    return false;

  if (hasx && ! contains_span_span(&box1->span, &box2->span))
    return false;
  if (hast && ! contains_span_span(&box1->period, &box2->period))
    return false;
  return true;
}

/**
 * @ingroup libmeos_box_bbox_topo
 * @brief Return true if the first temporal box is contained in the second one.
 * @sqlop @p <@
 */
bool
contained_tbox_tbox(const TBox *box1, const TBox *box2)
{
  return contains_tbox_tbox(box2, box1);
}

/**
 * @ingroup libmeos_box_bbox_topo
 * @brief Return true if the temporal boxes overlap.
 * @sqlop @p &&
 */
bool
overlaps_tbox_tbox(const TBox *box1, const TBox *box2)
{
  bool hasx, hast;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! topo_tbox_tbox_init(box1, box2, &hasx, &hast))
    return false;

  if (hasx && ! overlaps_span_span(&box1->span, &box2->span))
    return false;
  if (hast && ! overlaps_span_span(&box1->period, &box2->period))
    return false;
  return true;
}

/**
 * @ingroup libmeos_box_bbox_topo
 * @brief Return true if the temporal boxes are equal in the common dimensions.
 * @sqlop @p ~=
 */
bool
same_tbox_tbox(const TBox *box1, const TBox *box2)
{
  bool hasx, hast;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! topo_tbox_tbox_init(box1, box2, &hasx, &hast))
    return false;

  if (hasx && ! span_eq(&box1->span, &box2->span))
    return false;
  if (hast && ! span_eq(&box1->period, &box2->period))
    return false;
  return true;
}

/**
 * @ingroup libmeos_box_bbox_topo
 * @brief Return true if the temporal boxes are adjacent.
 * @sqlop @p -|-
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
    adjx = adjacent_span_span(&box1->span, &box2->span);
  if (hast)
    adjt = adjacent_span_span(&box1->period, &box2->period);
  return (adjx || adjt);
}

/*****************************************************************************
 * Position operators
 *****************************************************************************/

/**
 * @ingroup libmeos_box_bbox_pos
 * @brief Return true if the first temporal box is strictly to the left of
 * the second one.
 * @sqlop @p <<
 */
bool
left_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_tbox(box1) || ! ensure_has_X_tbox(box2) ||
      ! ensure_same_span_type(&box1->span, &box2->span))
    return false;
  return left_span_span(&box1->span, &box2->span);
}

/**
 * @ingroup libmeos_box_bbox_pos
 * @brief Return true if the first temporal box does not extend to the right
 * of the second one.
 * @sqlop @p &<
 */
bool
overleft_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_tbox(box1) || ! ensure_has_X_tbox(box2) ||
      ! ensure_same_span_type(&box1->span, &box2->span))
    return false;
  return overleft_span_span(&box1->span, &box2->span);
}

/**
 * @ingroup libmeos_box_bbox_pos
 * @brief Return true if the first temporal box is strictly to the right of
 * the second one.
 * @sqlop @p >>
 */
bool
right_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_tbox(box1) || ! ensure_has_X_tbox(box2) ||
      ! ensure_same_span_type(&box1->span, &box2->span))
    return false;
  return right_span_span(&box1->span, &box2->span);
}

/**
 * @ingroup libmeos_box_bbox_pos
 * @brief Return true if the first temporal box does not extend to the left of
 * the second one.
 * @sqlop @p &>
 */
bool
overright_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_tbox(box1) || ! ensure_has_X_tbox(box2) ||
      ! ensure_same_span_type(&box1->span, &box2->span))
    return false;

  return overright_span_span(&box1->span, &box2->span);
}

/**
 * @ingroup libmeos_box_bbox_pos
 * @brief Return true if the first temporal box is strictly before
 * the second one.
 * @sqlop @p <<#
 */
bool
before_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_T_tbox(box1) || ! ensure_has_T_tbox(box2))
    return false;
  return left_span_span(&box1->period, &box2->period);
}

/**
 * @ingroup libmeos_box_bbox_pos
 * @brief Return true if the first temporal box does not extend after
 * the second one.
 * @sqlop @p &<#
 */
bool
overbefore_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_T_tbox(box1) || ! ensure_has_T_tbox(box2))
    return false;
  return overleft_span_span(&box1->period, &box2->period);
}

/**
 * @ingroup libmeos_box_bbox_pos
 * @brief Return true if the first temporal box is strictly after the
 * second one.
 * @sqlop @p #>>
 */
bool
after_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_T_tbox(box1) || ! ensure_has_T_tbox(box2))
    return false;
  return right_span_span(&box1->period, &box2->period);
}

/**
 * @ingroup libmeos_box_bbox_pos
 * @brief Return true if the first temporal box does not extend before
 * the second one.
 * @sqlop @p #&>
 */
bool
overafter_tbox_tbox(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_T_tbox(box1) || ! ensure_has_T_tbox(box2))
    return false;
  return overright_span_span(&box1->period, &box2->period);
}

/*****************************************************************************
 * Set operators
 *****************************************************************************/

/**
 * @ingroup libmeos_box_set
 * @brief Return the union of the temporal boxes.
 * @sqlop @p +
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
  TBox *result = tbox_make(hasx ? &span : NULL, hast ? &period : NULL);
  return result;
}

/**
 * @ingroup libmeos_internal_box_set
 * @brief Set a temporal box with the result of the intersection of the first
 * two temporal boxes.
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
    (hasx && ! overlaps_span_span(&box1->span, &box2->span)) ||
    (hast && ! overlaps_span_span(&box1->period, &box2->period)))
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
 * @ingroup libmeos_box_set
 * @brief Return the intersection of the temporal boxes.
 * @sqlop @p *
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
 * Comparison functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the temporal boxes are equal
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
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
  if (! span_eq(&box1->span, &box2->span) ||
      ! span_eq(&box1->period, &box2->period))
    return false;
  /* The two boxes are equal */
  return true;
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the temporal boxes are different
 * @sqlop @p <>
 */
bool
tbox_ne(const TBox *box1, const TBox *box2)
{
  return ! tbox_eq(box1, box2);
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal box
 * is less than, equal to, or greater than the second one.
 *
 * The time dimension is compared first and then the value dimension.
 * @note Function used for B-tree comparison
 * @sqlfunc tbox_cmp()
 */
int
tbox_cmp(const TBox *box1, const TBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2))
    return INT_MAX;

  bool hasx, hast;
  tbox_tbox_flags(box1, box2, &hasx, &hast);
  int cmp;
  if (hast)
  {
    cmp = span_cmp(&box1->period, &box2->period);
    /* Compare the box minima */
    if (cmp != 0)
      return cmp;
  }
  if (hasx)
  {
    cmp = span_cmp(&box1->span, &box2->span);
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
 * @ingroup libmeos_box_comp
 * @brief Return true if the first temporal box is less than the second one
 * @sqlop @p <
 */
bool
tbox_lt(const TBox *box1, const TBox *box2)
{
  int cmp = tbox_cmp(box1, box2);
  return cmp < 0;
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the first temporal box is less than or equal to
 * the second one
 * @sqlop @p <=
 */
bool
tbox_le(const TBox *box1, const TBox *box2)
{
  int cmp = tbox_cmp(box1, box2);
  return cmp <= 0;
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the first temporal box is greater than or equal
 * to the second one
 * @sqlop @p >=
 */
bool
tbox_ge(const TBox *box1, const TBox *box2)
{
  int cmp = tbox_cmp(box1, box2);
  return cmp >= 0;
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the first temporal box is greater than the second one
 * @sqlop @p >
 */
bool
tbox_gt(const TBox *box1, const TBox *box2)
{
  int cmp = tbox_cmp(box1, box2);
  return cmp > 0;
}

/*****************************************************************************/
