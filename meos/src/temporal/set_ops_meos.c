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
 * @brief Operators for set types
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/set.h"
#include "temporal/temporal.h"
#include "temporal/type_util.h"

/*****************************************************************************
 * Contains
 *****************************************************************************/

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a set contains an integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_int(const Set *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
  return contains_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a set contains a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_bigint(const Set *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
  return contains_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a set contains a float
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_float(const Set *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  return contains_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a set contains a text
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_text(const Set *s, text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return contains_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a set contains a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_date(const Set *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return contains_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a set contains a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return contains_set_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if an integer is contained in a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_int_set(int i, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
  return contained_value_set(Int32GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a big integer is contained in a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_bigint_set(int64 i, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
  return contained_value_set(Int64GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a float is contained in a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_float_set(double d, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  return contained_value_set(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a text is contained in a set
 * @param[in] txt Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_text_set(const text *txt, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return contained_value_set(PointerGetDatum(txt), s);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a date is contained in a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_date_set(DateADT d, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return contains_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a timestamptz is contained in a set
 * @param[in] t Value
 * @param[in] s Set
 * @csqlfn #Contained_value_set()
 */
bool
contained_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return contains_set_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Strictly to the left of
 *****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if an integer is to the left of a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Left_value_set()
 */
bool
left_int_set(int i, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
  return left_value_set(Int32GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a big integer is to the left of a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Left_value_set()
 */
bool
left_bigint_set(int64 i, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
  return left_value_set(Int64GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a float is to the left of a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Left_value_set()
 */
bool
left_float_set(double d, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  return left_value_set(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a text is to the left of a set
 * @param[in] txt Value
 * @param[in] s Set
 * @csqlfn #Left_value_set()
 */
bool
left_text_set(const text *txt, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return left_value_set(PointerGetDatum(txt), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is before a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Left_value_set()
 */
bool
before_date_set(DateADT d, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return left_value_set(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is before a set
 * @param[in] t Value
 * @param[in] s Set
 * @csqlfn #Left_value_set()
 */
bool
before_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return left_value_set(TimestampTzGetDatum(t), s);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is to the left of an integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Left_set_value()
 */
bool
left_set_int(const Set *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
  return left_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is to the left of a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Left_set_value()
 */
bool
left_set_bigint(const Set *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
  return left_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is to the left of a float
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Left_set_value()
 */
bool
left_set_float(const Set *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  return left_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is to the left of a text
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Left_set_value()
 */
bool
left_set_text(const Set *s, text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return left_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is before a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Left_set_value()
 */
bool
before_set_date(const Set *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return left_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is before a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Left_set_value()
 */
bool
before_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return left_set_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Strictly right of
 *****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if an integer is to the right of a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Right_value_set()
 */
bool
right_int_set(int i, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
  return left_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a big integer is to the right of a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Right_value_set()
 */
bool
right_bigint_set(int64 i, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
  return left_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a float is to the right of a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Right_value_set()
 */
bool
right_float_set(double d, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  return left_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a text is to the right of a set
 * @param[in] txt Value
 * @param[in] s Set
 * @csqlfn #Right_value_set()
 */
bool
right_text_set(const text *txt, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return left_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is after a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Right_value_set()
 */
bool
after_date_set(DateADT d, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return left_set_value(s, DateADTGetDatum(d));
}
/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is after a set
 * @param[in] t Value
 * @param[in] s Set
 * @csqlfn #Right_value_set()
 */
bool
after_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return left_set_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is to the right of an integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Right_set_value()
 */
bool
right_set_int(const Set *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
  return left_value_set(Int32GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is to the right of a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Right_set_value()
 */
bool
right_set_bigint(const Set *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
  return left_value_set(Int64GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is to the right of a float
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Right_set_value()
 */
bool
right_set_float(const Set *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  return left_value_set(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is to the right of a text
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Right_set_value()
 */
bool
right_set_text(const Set *s, text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return left_value_set(PointerGetDatum(txt), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is after a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Right_set_value()
 */
bool
after_set_date(const Set *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return left_value_set(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is after a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Right_set_value()
 */
bool
after_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return left_value_set(TimestampTzGetDatum(t), s);
}

/*****************************************************************************
 * Does not extend to the right of
 *****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if an integer does not extend to the right of a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Overleft_value_set()
 */
bool
overleft_int_set(int i, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
  return overleft_value_set(Int32GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a big integer does not extend to the right of a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Overleft_value_set()
 */
bool
overleft_bigint_set(int64 i, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
  return overleft_value_set(Int64GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a float does not extend to the right of a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Overleft_value_set()
 */
bool
overleft_float_set(double d, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  return overleft_value_set(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a text does not extend to the right of a set
 * @param[in] txt Value
 * @param[in] s Set
 * @csqlfn #Overleft_value_set()
 */
bool
overleft_text_set(const text *txt, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return overleft_value_set(PointerGetDatum(txt), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is not after a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Overleft_value_set()
 */
bool
overbefore_date_set(DateADT d, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return overleft_value_set(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is not after a set
 * @csqlfn #Overleft_value_set()
 * @param[in] t Value
 * @param[in] s Set
 */
bool
overbefore_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return overleft_value_set(TimestampTzGetDatum(t), s);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set does not extend to the right of an integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Overleft_set_value()
 */
bool
overleft_set_int(const Set *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
  return overleft_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set does not extend to the right of a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Overleft_set_value()
 */
bool
overleft_set_bigint(const Set *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
  return overleft_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set does not extend to the right of a float
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Overleft_set_value()
 */
bool
overleft_set_float(const Set *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  return overleft_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set does not extend to the right of a text
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Overleft_set_value()
 */
bool
overleft_set_text(const Set *s, text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return overleft_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is not after a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Overleft_set_value()
 */
bool
overbefore_set_date(const Set *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return overleft_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is not after a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Overleft_set_value()
 */
bool
overbefore_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return overleft_set_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Does not extend to the left of
 *****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if an integer does not extend to the the left of a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Overright_value_set()
 */
bool
overright_int_set(int i, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
  return overright_value_set(Int32GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a big integer does not extend to the the left of a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Overright_value_set()
 */
bool
overright_bigint_set(int64 i, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
  return overright_value_set(Int64GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a float does not extend to the left of a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Overright_value_set()
 */
bool
overright_float_set(double d, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  return overright_value_set(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a text does not extend to the left of a set
 * @param[in] txt Value
 * @param[in] s Set
 * @csqlfn #Overright_value_set()
 */
bool
overright_text_set(const text *txt, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return overright_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is not before a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Overright_value_set()
 */
bool
overafter_date_set(DateADT d, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return overright_value_set(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is not before a set
 * @param[in] t Value
 * @param[in] s Set
 * @csqlfn #Overright_value_set()
 */
bool
overafter_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);

  return overright_value_set(TimestampTzGetDatum(t), s);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set does not extend to the left of an integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Overright_set_value()
 */
bool
overright_set_int(const Set *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false);
  return overright_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set does not extend to the left of a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Overright_set_value()
 */
bool
overright_set_bigint(const Set *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false);
  return overright_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set does not extend to the left of a float
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Overright_set_value()
 */
bool
overright_set_float(const Set *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false);
  return overright_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set does not extend to the left of a text
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Overright_set_value()
 */
bool
overright_set_text(const Set *s, text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(txt, false);
  return overright_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is not before a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Overright_set_value()
 */
bool
overafter_set_date(const Set *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false);
  return overright_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a set is not before a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Overright_set_value()
 */
bool
overafter_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false);
  return overright_set_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a set and an integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_int(const Set *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, NULL);
  return union_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a set and a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_bigint(const Set *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, NULL);
  return union_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a set and a float
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_float(const Set *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, NULL);
  return union_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a set and a text
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_text(const Set *s, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, NULL); VALIDATE_NOT_NULL(txt, NULL);
  return union_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a set and a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_date(const Set *s, const DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, NULL);
  return union_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a set and a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Union_set_value()
 */
Set *
union_set_timestamptz(const Set *s, const TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, NULL);

  return union_set_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of an integer and a set
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Union_set_value()
 */
inline Set *
union_int_set(int i, const Set *s)
{
  return union_set_int(s, i);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a big integer and a set
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Union_set_value()
 */
inline Set *
union_bigint_set(int64 i, const Set *s)
{
  return union_set_bigint(s, i);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a float and a set
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Union_set_value()
 */
inline Set *
union_float_set(double d, const Set *s)
{
  return union_set_float(s, d);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a text and a set
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Union_set_value()
 */
inline Set *
union_text_set(const text *txt, const Set *s)
{
  return union_set_text(s, txt);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a date and a set
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Union_set_value()
 */
inline Set *
union_date_set(const DateADT d, const Set *s)
{
  return union_set_date(s, d);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a timestamptz and a set
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Union_set_value()
 */
inline Set *
union_timestamptz_set(const TimestampTz t, const Set *s)
{
  return union_set_timestamptz(s, t);
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a set and an integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_int(const Set *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, NULL);
  return intersection_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a set and a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_bigint(const Set *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, NULL);
  return intersection_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a set and a float
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_float(const Set *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, NULL);
  return intersection_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a set and a text
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_text(const Set *s, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, NULL); VALIDATE_NOT_NULL(txt, NULL);
  return intersection_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a set and a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_date(const Set *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, NULL);
  return intersection_set_value(s, DateADTGetDatum(d));
}
/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a set and a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, NULL);

  return intersection_set_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of an integer and a set
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Union_set_value()
 */
inline Set *
intersection_int_set(int i, const Set *s)
{
  return intersection_set_int(s, i);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a big integer and a set
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Union_set_value()
 */
inline Set *
intersection_bigint_set(int64 i, const Set *s)
{
  return intersection_set_bigint(s, i);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a float and a set
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Union_set_value()
 */
inline Set *
intersection_float_set(double d, const Set *s)
{
  return intersection_set_float(s, d);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a text and a set
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Union_set_value()
 */
inline Set *
intersection_text_set(const text *txt, const Set *s)
{
  return intersection_set_text(s, txt);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a date and a set
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Union_set_value()
 */
inline Set *
intersection_date_set(const DateADT d, const Set *s)
{
  return intersection_set_date(s, d);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a timestamptz and a set
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Union_set_value()
 */
inline Set *
intersection_timestamptz_set(const TimestampTz t, const Set *s)
{
  return intersection_set_timestamptz(s, t);
}

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed
 *****************************************************************************/

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of an integer and a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_int_set(int i, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, NULL);
  return minus_value_set(Int32GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a big integer and a set
 * @param[in] i Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_bigint_set(int64 i, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, NULL);
  return minus_value_set(Int64GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a float and a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_float_set(double d, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, NULL);
  return minus_value_set(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a text and a set
 * @param[in] txt Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_text_set(const text *txt, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, NULL); VALIDATE_NOT_NULL(txt, NULL);
  return minus_value_set(PointerGetDatum(txt), s);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a date and a set
 * @param[in] d Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_date_set(DateADT d, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, NULL);
  return minus_value_set(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a timestamptz and a set
 * @param[in] t Value
 * @param[in] s Set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_timestamptz_set(TimestampTz t, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, NULL);

  return minus_value_set(TimestampTzGetDatum(t), s);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a set and an integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_int(const Set *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, NULL);
  return minus_set_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a set and a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_bigint(const Set *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, NULL);
  return minus_set_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a set and a float
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_float(const Set *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, NULL);
  return minus_set_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a set and a text
 * @param[in] s Set
 * @param[in] txt Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_text(const Set *s, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, NULL); VALIDATE_NOT_NULL(txt, NULL);
  return minus_set_value(s, PointerGetDatum(txt));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a set and a date
 * @param[in] s Set
 * @param[in] d Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_date(const Set *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, NULL);
  return minus_set_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a set and a timestamptz
 * @param[in] s Set
 * @param[in] t Value
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, NULL);
  return minus_set_value(s, TimestampTzGetDatum(t));
}

/******************************************************************************
 * Distance functions
 ******************************************************************************/

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a set and an integer
 * @param[in] s Set
 * @param[in] i Value
 * @return On error return -1
 * @csqlfn #Distance_set_value()
 */
int
distance_set_int(const Set *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, -1);
  return DatumGetInt32(distance_set_value(s, Int32GetDatum(i)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a set and a big integer
 * @param[in] s Set
 * @param[in] i Value
 * @return On error return -1.0
 * @csqlfn #Distance_set_value()
 */
int64
distance_set_bigint(const Set *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, -1);
  return DatumGetInt64(distance_set_value(s, Int64GetDatum(i)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a set and a float
 * @param[in] s Set
 * @param[in] d Value
 * @return On error return -1.0
 * @csqlfn #Distance_set_value()
 */
double
distance_set_float(const Set *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, -1.0);
  return DatumGetFloat8(distance_set_value(s, Float8GetDatum(d)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in days between a set and a date
 * @param[in] s Set
 * @param[in] d Value
 * @return On error return -1.0
 * @csqlfn #Distance_set_value()
 */
int
distance_set_date(const Set *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, -1);
  return DatumGetInt32(distance_set_value(s, DateADTGetDatum(d)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in seconds between a set and a timestamptz as a
 * double
 * @param[in] s Set
 * @param[in] t Value
 * @return On error return -1.0
 * @csqlfn #Distance_set_value()
 */
double
distance_set_timestamptz(const Set *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, -1.0);
  return DatumGetFloat8(distance_set_value(s, TimestampTzGetDatum(t)));
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two integer sets
 * @param[in] s1,s2 Sets
 * @return On error return -1
 * @csqlfn #Distance_set_set()
 */
int
distance_intset_intset(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2) || ! ensure_set_isof_type(s1, T_INTSET))
    return -1;
  return DatumGetInt32(distance_set_set(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two big integer sets
 * @param[in] s1,s2 Sets
 * @return On error return -1
 * @csqlfn #Distance_set_set()
 */
int64
distance_bigintset_bigintset(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2) || ! ensure_set_isof_type(s1, T_BIGINTSET))
    return -1;
  return DatumGetInt64(distance_set_set(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two float sets
 * @param[in] s1,s2 Sets
 * @return On error return -1.0
 * @csqlfn #Distance_set_set()
 */
double
distance_floatset_floatset(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2) || ! ensure_set_isof_type(s1, T_FLOATSET))
    return -1.0;
  return DatumGetFloat8(distance_set_set(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in days between two date sets
 * @param[in] s1,s2 Sets
 * @return On error return -1
 * @csqlfn #Distance_set_set()
 */
int
distance_dateset_dateset(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2) || ! ensure_set_isof_type(s1, T_DATESET))
    return -1;
  return DatumGetInt32(distance_set_set(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in seconds between two timestamptz sets
 * @param[in] s1,s2 Sets
 * @return On error return -1.0
 * @csqlfn #Distance_set_set()
 */
double
distance_tstzset_tstzset(const Set *s1, const Set *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_set_set(s1, s2) || ! ensure_set_isof_type(s1, T_TSTZSET))
    return -1.0;
  return DatumGetFloat8(distance_set_set(s1, s2));
}

/******************************************************************************/
