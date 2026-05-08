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
 * @brief Functions for temporal bounding boxes (MEOS wrappers)
 */

#include "temporal/tbox.h"

/* C */
#include <assert.h>
#include <limits.h>
#include <assert.h>
/* PostgreSQL */
#include "common/hashfn.h"
#include "port/pg_bitutils.h"
#include "utils/timestamp.h"
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/meos_catalog.h"
#include "temporal/postgres_types.h"
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/temporal.h"
#include "temporal/tnumber_mathfuncs.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"

/** Buffer size for input and output of TBox values */
#define TBOX_MAXLEN    128

/*****************************************************************************/

/**
 * @ingroup meos_box_constructor
 * @brief Return a temporal box from an integer and a timestamptz
 * @param[in] i Value
 * @param[in] t Timestamp
 * @csqlfn #Number_timestamptz_to_tbox()
 */
inline TBox *
int_timestamptz_to_tbox(int i, TimestampTz t)
{
  return number_timestamptz_to_tbox(Int32GetDatum(i), T_INT4, t);
}

/**
 * @ingroup meos_box_constructor
 * @brief Return a temporal box from a big integer and a timestamptz
 * @param[in] i Value
 * @param[in] t Timestamp
 * @csqlfn #Number_timestamptz_to_tbox()
 */
inline TBox *
bigint_timestamptz_to_tbox(int64 i, TimestampTz t)
{
  return number_timestamptz_to_tbox(Int64GetDatum(i), T_INT8, t);
}

/**
 * @ingroup meos_box_constructor
 * @brief Return a temporal box from a float and a timestamptz
 * @param[in] d Value
 * @param[in] t Timestamp
 * @csqlfn #Number_timestamptz_to_tbox()
 */
inline TBox *
float_timestamptz_to_tbox(double d, TimestampTz t)
{
  return number_timestamptz_to_tbox(Float8GetDatum(d), T_FLOAT8, t);
}

/*****************************************************************************/

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
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, NULL);
  return number_tstzspan_to_tbox(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup meos_box_constructor
 * @brief Return a temporal box from a big integer and a timestamptz span
 * @param[in] i Value
 * @param[in] s Time span
 * @csqlfn #Number_tstzspan_to_tbox()
 */
TBox *
bigint_tstzspan_to_tbox(int64 i, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, NULL);
  return number_tstzspan_to_tbox(Int64GetDatum(i), T_INT8, s);
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
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, NULL);
  return number_tstzspan_to_tbox(Float8GetDatum(d), T_FLOAT8, s);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument a temporal box constructed from an
 * integer
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
 * @brief Convert an integer into a temporal box
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
 * @brief Return in the last argument a temporal box constructed from a big
 * integer
 * @param[in] i Value
 * @param[out] box Result
 */
void
bigint_set_tbox(int64 i, TBox *box)
{
  assert(box);
  number_set_tbox(Int64GetDatum(i), T_INT8, box);
  return;
}

/**
 * @ingroup meos_box_conversion
 * @brief Convert a big integer into a temporal box
 * @param[in] i Value
 * @csqlfn #Number_to_tbox()
 */
TBox *
bigint_to_tbox(int64 i)
{
  TBox *result = palloc(sizeof(TBox));
  int_set_tbox(i, result);
  return result;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument a temporal box constructed from a float
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
 * @brief Convert a float into a temporal box
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

/*****************************************************************************/

/**
 * @ingroup meos_box_conversion
 * @brief Convert a number span into a temporal box
 * @param[in] s Span
 * @csqlfn #Span_to_tbox()
 */
TBox *
span_to_tbox(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(s, NULL);
  if (! ensure_span_tbox_type(s->spantype))
    return NULL;
  return span_tbox(s);
}

/**
 * @ingroup meos_box_conversion
 * @brief Convert a number span set into a temporal box
 * @param[in] ss Span set
 * @csqlfn #Spanset_to_tbox()
 */
TBox *
spanset_tbox(const SpanSet *ss)
{
  assert(ss); assert(span_tbox_type(ss->spantype));
  TBox *result = palloc(sizeof(TBox));
  if (tnumber_spantype(ss->spantype))
    tbox_set(&ss->span, NULL, result);
  else /* ss->spantype == T_TSTZSPAN */
    tbox_set(NULL, &ss->span, result);
  return result;
}

/**
 * @ingroup meos_box_conversion
 * @brief Convert a number span set into a temporal box
 * @param[in] ss Span set
 * @csqlfn #Spanset_to_tbox()
 * @note This function is only used in MEOS since for MobilityDB we use the
 * #spanset_tbox_slice function for obtaining the first bytes of the spanset
 * to get the precomputed span bounding box
 */
TBox *
spanset_to_tbox(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL);
  if (! ensure_span_tbox_type(ss->spantype))
    return NULL;
  return spanset_tbox(ss);
}

/*****************************************************************************/

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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_X(box->flags) ||
      ! ensure_span_isof_type(&box->span, T_INTSPAN))
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
tboxbigint_xmin(const TBox *box, int64 *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_X(box->flags) ||
      ! ensure_span_isof_type(&box->span, T_BIGINTSPAN))
    return false;
  *result = DatumGetInt64(box->span.lower);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_X(box->flags) ||
      ! ensure_span_isof_type(&box->span, T_FLOATSPAN) )
    return false;
  *result = DatumGetFloat8(box->span.lower);
  return true;
}

/*****************************************************************************/

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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_X(box->flags) ||
      ! ensure_span_isof_type(&box->span, T_INTSPAN))
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
tboxbigint_xmax(const TBox *box, int64 *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_X(box->flags) ||
      ! ensure_span_isof_type(&box->span, T_BIGINTSPAN))
    return false;
  *result = DatumGetInt64(box->span.upper) - 1;
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_X(box->flags) ||
      ! ensure_span_isof_type(&box->span, T_FLOATSPAN))
    return false;
  *result = DatumGetFloat8(box->span.upper);
  return true;
}

/*****************************************************************************/

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
tintbox_shift_scale(const TBox *box, int shift, int width, bool hasshift,
  bool haswidth)
{
  VALIDATE_NOT_NULL(box, NULL);
  if (! ensure_span_isof_type(&box->span, T_INTSPAN))
    return NULL;

  return tbox_shift_scale_value(box, Int32GetDatum(shift),
    Int32GetDatum(width), hasshift, haswidth);
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
tbigintbox_shift_scale(const TBox *box, int64 shift, int64 width,
  bool hasshift, bool haswidth)
{
  VALIDATE_NOT_NULL(box, NULL);
  if (! ensure_span_isof_type(&box->span, T_BIGINTSPAN))
    return NULL;

  return tbox_shift_scale_value(box, Int64GetDatum(shift),
    Int64GetDatum(width), hasshift, haswidth);
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
tfloatbox_shift_scale(const TBox *box, double shift, double width,
  bool hasshift, bool haswidth)
{
  VALIDATE_NOT_NULL(box, NULL);
  if (! ensure_span_isof_type(&box->span, T_FLOATSPAN))
    return NULL;

  return tbox_shift_scale_value(box, Float8GetDatum(shift),
    Float8GetDatum(width), hasshift, haswidth);
}

/*****************************************************************************/
