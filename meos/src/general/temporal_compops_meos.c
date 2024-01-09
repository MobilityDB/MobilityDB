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
 * @brief Ever/always and temporal comparison operators
 */

#include "general/temporal_compops.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h"
#include "general/type_util.h"

/*****************************************************************************
 * Ever/always comparisons
 *****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a boolean is ever equal to a temporal boolean
 * @param[in] b Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_base_temporal()
 */
bool
ever_eq_bool_tbool(bool b, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return false;
  return ever_eq_temporal_base(temp, BoolGetDatum(b));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is ever equal to a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_base_temporal()
 */
bool
ever_eq_int_tint(int i, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;
  return ever_eq_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a float is ever equal to a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_base_temporal()
 */
bool
ever_eq_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;
  return ever_eq_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a text is ever equal to a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_base_temporal()
 */
bool
ever_eq_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return false;
  return ever_eq_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal boolean is ever equal to a boolean
 * @param[in] temp Temporal value
 * @param[in] b Value
 * @csqlfn #Ever_eq_temporal_base()
 */
bool
ever_eq_tbool_bool(const Temporal *temp, bool b)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return false;
  return ever_eq_temporal_base(temp, BoolGetDatum(b));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is ever equal to an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_eq_temporal_base()
 */
bool
ever_eq_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;
  return ever_eq_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is ever equal to a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Ever_eq_temporal_base()
 */
bool
ever_eq_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;
  return ever_eq_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is ever equal to a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Ever_eq_temporal_base()
 */
bool
ever_eq_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return false;
  return ever_eq_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a boolean is always equal to a temporal boolean
 * @param[in] b Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_base_temporal()
 */
bool
always_eq_bool_tbool(bool b, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return false;
  return always_eq_temporal_base(temp, BoolGetDatum(b));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is always equal to a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_base_temporal()
 */
bool
always_eq_int_tint(int i, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;
  return always_eq_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a float is always equal to a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_base_temporal()
 */
bool
always_eq_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;
  return always_eq_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a text is always equal to a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_base_temporal()
 */
bool
always_eq_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return false;
  return always_eq_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal boolean is always equal to a boolean
 * @param[in] temp Temporal value
 * @param[in] b Value
 * @csqlfn #Always_eq_temporal_base()
 */
bool
always_eq_tbool_bool(const Temporal *temp, bool b)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return false;
  return always_eq_temporal_base(temp, BoolGetDatum(b));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is always equal to an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_eq_temporal_base()
 */
bool
always_eq_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;
  return always_eq_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is always equal to a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Always_eq_temporal_base()
 */
bool
always_eq_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) &&
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;
  return always_eq_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is always equal to a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Always_eq_temporal_base()
 */
bool
always_eq_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return false;
  return always_eq_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a boolean is ever different from a temporal boolean
 * @param[in] b Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_base_temporal()
 */
bool
ever_ne_bool_tbool(bool b, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return false;
  return ever_ne_temporal_base(temp, BoolGetDatum(b));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is ever different from a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_base_temporal()
 */
bool
ever_ne_int_tint(int i, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;
  return ever_ne_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a float is ever different from a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_base_temporal()
 */
bool
ever_ne_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;
  return ever_ne_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a text is ever different from a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_base_temporal()
 */
bool
ever_ne_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return false;
  return ever_ne_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal boolean is ever different from a boolean
 * @param[in] temp Temporal value
 * @param[in] b Value
 * @csqlfn #Ever_ne_temporal_base()
 */
bool
ever_ne_tbool_bool(const Temporal *temp, bool b)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return false;
  return ever_ne_temporal_base(temp, BoolGetDatum(b));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is ever different from an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_ne_temporal_base()
 */
bool
ever_ne_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;
  return ever_ne_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is ever different from a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Ever_ne_temporal_base()
 */
bool
ever_ne_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;
  return ever_ne_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is ever different from a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Ever_ne_temporal_base()
 */
bool
ever_ne_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return false;
  return ever_ne_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a boolean is always different from a temporal boolean
 * @param[in] b Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_base_temporal()
 */
bool
always_ne_bool_tbool(bool b, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return false;
  return always_ne_temporal_base(temp, BoolGetDatum(b));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if an integer is always different from a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_base_temporal()
 */
bool
always_ne_int_tint(int i, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;
  return always_ne_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a float is always different from a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_base_temporal()
 */
bool
always_ne_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;
  return always_ne_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a text is always different from a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_base_temporal()
 */
bool
always_ne_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return false;
  return always_ne_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal boolean is always different from a boolean
 * @param[in] temp Temporal value
 * @param[in] b Value
 * @csqlfn #Always_ne_temporal_base()
 */
bool
always_ne_tbool_bool(const Temporal *temp, bool b)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return false;
  return always_ne_temporal_base(temp, BoolGetDatum(b));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is always different from an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_ne_temporal_base()
 */
bool
always_ne_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;
  return always_ne_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is always different from a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Always_ne_temporal_base()
 */
bool
always_ne_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) &&
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;
  return always_ne_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is always different from a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Always_ne_temporal_base()
 */
bool
always_ne_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return false;
  return always_ne_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is ever less than an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_lt_temporal_base()
 */
bool
ever_lt_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;
  return ever_lt_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is ever less than a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Ever_lt_temporal_base()
 */
bool
ever_lt_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;
  return ever_lt_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is ever less than a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Ever_lt_temporal_base()
 */
bool
ever_lt_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return false;
  return ever_lt_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is always less than an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_lt_temporal_base()
 */
bool
always_lt_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;
  return always_lt_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is always less than a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Always_lt_temporal_base()
 */
bool
always_lt_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;
  return always_lt_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is always less than a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Always_lt_temporal_base()
 */
bool
always_lt_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return false;
  return always_lt_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is ever less than or equal to an
 * integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_le_temporal_base()
 */
bool
ever_le_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;
  return ever_le_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is ever less than or equal to a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Ever_le_temporal_base()
 */
bool
ever_le_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;
  return ever_le_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is ever less than or equal to a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Ever_le_temporal_base()
 */
bool
ever_le_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return false;
  return ever_le_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is always less than or equal to an
 * integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_le_temporal_base()
 */
bool
always_le_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;
  return always_le_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is always less than or equal to a
 * float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Always_le_temporal_base()
 */
bool
always_le_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;
  return always_le_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is always less than or equal to a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Always_le_temporal_base()
 */
bool
always_le_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return false;
  return always_le_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is ever less than an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_lt_temporal_base()
 */
bool
ever_gt_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;
  return ever_gt_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is ever less than a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Ever_lt_temporal_base()
 */
bool
ever_gt_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;
  return ever_gt_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is ever less than a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Ever_lt_temporal_base()
 */
bool
ever_gt_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return false;
  return ever_gt_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is always less than an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_lt_temporal_base()
 */
bool
always_gt_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;
  return always_gt_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is always less than a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Always_lt_temporal_base()
 */
bool
always_gt_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;
  return always_gt_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is always less than a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Always_lt_temporal_base()
 */
bool
always_gt_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return false;
  return always_gt_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is ever less than or equal to an
 * integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Ever_le_temporal_base()
 */
bool
ever_ge_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;
  return ever_ge_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is ever less than or equal to a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Ever_le_temporal_base()
 */
bool
ever_ge_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;
  return ever_ge_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is ever less than or equal to a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Ever_le_temporal_base()
 */
bool
ever_ge_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return false;
  return ever_ge_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal integer is always less than or equal to an
 * integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Always_le_temporal_base()
 */
bool
always_ge_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;
  return always_ge_temporal_base(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal float is always less than or equal to a
 * float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Always_le_temporal_base()
 */
bool
always_ge_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;
  return always_ge_temporal_base(temp, Float8GetDatum(d));
}

/**
 * @ingroup meos_temporal_comp_ever
 * @brief Return true if a temporal text is always less than or equal to a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Always_le_temporal_base()
 */
bool
always_ge_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return false;
  return always_ge_temporal_base(temp, PointerGetDatum(txt));
}

/*****************************************************************************
 * Temporal eq
 *****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a boolean and a temporal boolean
 * @param[in] b Value
 * @param[in] temp Temporal value
 * @csqlfn #Teq_base_temporal()
 */
Temporal *
teq_bool_tbool(bool b, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return NULL;
  return tcomp_temporal_base(temp, BoolGetDatum(b), &datum2_eq, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a temporal boolean and a boolean
 * @param[in] temp Temporal value
 * @param[in] b Value
 * @csqlfn #Teq_temporal_base()
 */
Temporal *
teq_tbool_bool(const Temporal *temp, bool b)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return NULL;
  return tcomp_temporal_base(temp, BoolGetDatum(b), &datum2_eq, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of an integer and a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Teq_base_temporal()
 */
Temporal *
teq_int_tint(int i, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_eq, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a temporal integer and an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Teq_temporal_base()
 */
Temporal *
teq_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_eq, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a float and a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Teq_base_temporal()
 */
Temporal *
teq_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_eq, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a temporal float and a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Teq_temporal_base()
 */
Temporal *
teq_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_eq, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a text and a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Teq_base_temporal()
 */
Temporal *
teq_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_eq, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal equality of a temporal text and a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Teq_temporal_base()
 */
Temporal *
teq_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_eq, INVERT_NO);
}

/*****************************************************************************
 * Temporal ne
 *****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of a boolean and a temporal boolean
 * @param[in] b Value
 * @param[in] temp Temporal value
 * @csqlfn #Tne_base_temporal()
 */
Temporal *
tne_bool_tbool(bool b, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return NULL;
  return tcomp_temporal_base(temp, BoolGetDatum(b), &datum2_ne, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of a temporal boolean and a boolean
 * @param[in] temp Temporal value
 * @param[in] b Value
 * @csqlfn #Tne_temporal_base()
 */
Temporal *
tne_tbool_bool(const Temporal *temp, bool b)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return NULL;
  return tcomp_temporal_base(temp, BoolGetDatum(b), &datum2_ne, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of an integer and a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Tne_base_temporal()
 */
Temporal *
tne_int_tint(int i, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_ne, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of a temporal integer and an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Tne_temporal_base()
 */
Temporal *
tne_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_ne, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of a float and a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Tne_base_temporal()
 */
Temporal *
tne_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_ne, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of a temporal float and a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Tne_temporal_base()
 */
Temporal *
tne_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_ne, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of a text and a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Tne_base_temporal()
 */
Temporal *
tne_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_ne, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal inequality of a temporal text and a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Tne_temporal_base()
 */
Temporal *
tne_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_ne, INVERT_NO);
}

/*****************************************************************************
 * Temporal lt
 *****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than of an integer and a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Tlt_base_temporal()
 */
Temporal *
tlt_int_tint(int i, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_lt, INVERT);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than of a float and a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Tlt_base_temporal()
 */
Temporal *
tlt_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_lt, INVERT);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than of a text and a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Tlt_base_temporal()
 */
Temporal *
tlt_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_lt, INVERT);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than of a temporal integer and an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Tlt_temporal_base()
 */
Temporal *
tlt_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_lt, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than of a temporal float and a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Tlt_temporal_base()
 */
Temporal *
tlt_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_lt, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than of a temporal text and a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Tlt_temporal_base()
 */
Temporal *
tlt_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_lt, INVERT_NO);
}

/*****************************************************************************
 * Temporal le
 *****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than or equal to of an integer and a
 * temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Tle_base_temporal()
 */
Temporal *
tle_int_tint(int i, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_le, INVERT);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than or equal to of a float and a temporal
 * float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Tle_base_temporal()
 */
Temporal *
tle_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_le, INVERT);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less or equal to than of a text and a temporal
 * text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Tle_base_temporal()
 */
Temporal *
tle_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_le, INVERT);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than or equal to of a temporal integer and
 * an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Tle_temporal_base()
 */
Temporal *
tle_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_le, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than or equal to of a temporal float and a
 * float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Tle_temporal_base()
 */
Temporal *
tle_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_le, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal less than or equal to of a temporal text and a
 * text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Tle_temporal_base()
 */
Temporal *
tle_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_le, INVERT_NO);
}

/*****************************************************************************
 * Temporal gt
 *****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than of an integer and a temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Tgt_base_temporal()
 */
Temporal *
tgt_int_tint(int i, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_gt, INVERT);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than of a float and a temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Tgt_base_temporal()
 */
Temporal *
tgt_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_gt, INVERT);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than of a text and a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Tgt_base_temporal()
 */
Temporal *
tgt_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_gt, INVERT);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than of a temporal integer and an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Tgt_temporal_base()
 */
Temporal *
tgt_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_gt, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than of a temporal float and a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Tgt_temporal_base()
 */
Temporal *
tgt_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_gt, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than of a temporal text and a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Tgt_temporal_base()
 */
Temporal *
tgt_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_gt, INVERT_NO);
}

/*****************************************************************************
 * Temporal ge
 *****************************************************************************/

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of an integer and a
 * temporal integer
 * @param[in] i Value
 * @param[in] temp Temporal value
 * @csqlfn #Tge_base_temporal()
 */
Temporal *
tge_int_tint(int i, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_ge, INVERT);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of a float and a
 * temporal float
 * @param[in] d Value
 * @param[in] temp Temporal value
 * @csqlfn #Tge_base_temporal()
 */
Temporal *
tge_float_tfloat(double d, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_ge, INVERT);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of a text and a temporal
 * text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Tge_base_temporal()
 */
Temporal *
tge_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_ge, INVERT);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of a temporal integer
 * and an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Tge_temporal_base()
 */
Temporal *
tge_tint_int(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return tcomp_temporal_base(temp, Int32GetDatum(i), &datum2_ge, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of a temporal float and
 * a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Tge_temporal_base()
 */
Temporal *
tge_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return tcomp_temporal_base(temp, Float8GetDatum(d), &datum2_ge, INVERT_NO);
}

/**
 * @ingroup meos_temporal_comp_temp
 * @brief Return the temporal greater than or equal to of a temporal text and a
 * text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Tge_temporal_base()
 */
Temporal *
tge_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(txt), &datum2_ge, INVERT_NO);
}

/*****************************************************************************/
