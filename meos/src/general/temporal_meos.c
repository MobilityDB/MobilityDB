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
 * @brief Basic functions for temporal types of any subtype
 */

#include "general/temporal.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/span.h"
#include "general/spanset.h"

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal boolean restricted to a boolean
 * @param[in] temp Temporal value
 * @param[in] b Value
 * @csqlfn #Temporal_at_value()
 */
Temporal *
tbool_at_value(const Temporal *temp, bool b)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return NULL;
  return temporal_restrict_value(temp, BoolGetDatum(b), REST_AT);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal integer restricted to an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Temporal_at_value()
 */
Temporal *
tint_at_value(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return temporal_restrict_value(temp, Int32GetDatum(i), REST_AT);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal float restricted to a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Temporal_at_value()
 */
Temporal *
tfloat_at_value(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return temporal_restrict_value(temp, Float8GetDatum(d), REST_AT);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal text restricted to a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Temporal_at_value()
 */
Temporal *
ttext_at_value(const Temporal *temp, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return temporal_restrict_value(temp, PointerGetDatum(txt), REST_AT);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal point restricted to a point
 * @param[in] temp Temporal value
 * @param[in] gs Value
 * @csqlfn #Temporal_at_value()
 */
Temporal *
tpoint_at_value(const Temporal *temp, GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_tgeo_type(temp->temptype))
    return NULL;
  return temporal_restrict_value(temp, PointerGetDatum(gs), REST_AT);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal boolean restricted to the complement of a boolean
 * @param[in] temp Temporal value
 * @param[in] b Value
 * @csqlfn #Temporal_minus_value()
 */
Temporal *
tbool_minus_value(const Temporal *temp, bool b)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return NULL;
  return temporal_restrict_value(temp, BoolGetDatum(b), REST_MINUS);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal integer restricted to the complement of an integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @csqlfn #Temporal_minus_value()
 */
Temporal *
tint_minus_value(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return temporal_restrict_value(temp, Int32GetDatum(i), REST_MINUS);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal float restricted to the complement of a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @csqlfn #Temporal_minus_value()
 */
Temporal *
tfloat_minus_value(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return temporal_restrict_value(temp, Float8GetDatum(d), REST_MINUS);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal text restricted to the complement of a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Temporal_minus_value()
 */
Temporal *
ttext_minus_value(const Temporal *temp, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return temporal_restrict_value(temp, PointerGetDatum(txt), REST_MINUS);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal point restricted to the complement of a point
 * @param[in] temp Temporal value
 * @param[in] gs Value
 * @csqlfn #Temporal_minus_value()
 */
Temporal *
tpoint_minus_value(const Temporal *temp, GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_tgeo_type(temp->temptype))
    return NULL;
  return temporal_restrict_value(temp, PointerGetDatum(gs), REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to a set of values
 * @param[in] temp Temporal value
 * @param[in] s Set
 * @csqlfn #Temporal_at_values()
 */
Temporal *
temporal_at_values(const Temporal *temp, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) s) ||
      ! ensure_temporal_isof_basetype(temp, s->basetype))
    return NULL;
  return temporal_restrict_values(temp, s, REST_AT);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to the complement of a set of
 * values
 * @param[in] temp Temporal value
 * @param[in] s Set
 * @csqlfn #Temporal_minus_values()
 */
Temporal *
temporal_minus_values(const Temporal *temp, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) s) ||
      ! ensure_temporal_isof_basetype(temp, s->basetype))
    return NULL;
  return temporal_restrict_values(temp, s, REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the value of a temporal boolean at a timestamptz
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] strict True if the timestamp must belong to the temporal value,
 * false when it may be at an exclusive bound
 * @param[out] value Resulting value
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
tbool_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  bool *value)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) value) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return false;

  Datum res;
  bool result = temporal_value_at_timestamptz(temp, t, strict, &res);
  *value = DatumGetBool(res);
  return result;
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the value of a temporal integer at a timestamptz
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] strict True if the timestamp must belong to the temporal value,
 * false when it may be at an exclusive bound
 * @param[out] value Resulting value
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
tint_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  int *value)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) value) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return false;

  Datum res;
  bool result = temporal_value_at_timestamptz(temp, t, strict, &res);
  *value = DatumGetInt32(res);
  return result;
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the value of a temporal integer at a timestamptz
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] strict True if the timestamp must belong to the temporal value,
 * false when it may be at an exclusive bound
 * @param[out] value Resulting value
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
tfloat_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  double *value)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) value) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return false;

  Datum res;
  bool result = temporal_value_at_timestamptz(temp, t, strict, &res);
  *value = DatumGetFloat8(res);
  return result;
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the value of a temporal integer at a timestamptz
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] strict True if the timestamp must belong to the temporal value,
 * false when it may be at an exclusive bound
 * @param[out] value Resulting value
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
ttext_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  text **value)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) value) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;

  Datum res;
  bool result = temporal_value_at_timestamptz(temp, t, strict, &res);
  *value = DatumGetTextP(res);
  return result;
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the value of a temporal geometry point at a timestamptz
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] strict True if the timestamp must belong to the temporal value,
 * false when it may be at an exclusive bound
 * @param[out] value Resulting value
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
tpoint_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  GSERIALIZED **value)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) value) ||
      ! ensure_tgeo_type(temp->temptype))
    return false;

  Datum res;
  bool result = temporal_value_at_timestamptz(temp, t, strict, &res);
  *value = DatumGetGserializedP(res);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to its minimum base value
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_at_min()
 */
Temporal *
temporal_at_min(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;
  return temporal_restrict_minmax(temp, GET_MIN, REST_AT);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to the complement of its minimum
 * base value
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_minus_min()
 */
Temporal *
temporal_minus_min(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;
  return temporal_restrict_minmax(temp, GET_MIN, REST_MINUS);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to its maximum base value
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_at_max()
 */
Temporal *
temporal_at_max(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;
  return temporal_restrict_minmax(temp, GET_MAX, REST_AT);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to the complement of its maximum
 * base value
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_minus_max()
 */
Temporal *
temporal_minus_max(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;
  return temporal_restrict_minmax(temp, GET_MAX, REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to a span of base values
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Tnumber_at_span()
 */
Temporal *
tnumber_at_span(const Temporal *temp, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) s) ||
      ! ensure_tnumber_type(temp->temptype) ||
      ! ensure_valid_tnumber_span(temp, s))
    return NULL;
  return tnumber_restrict_span(temp, s, REST_AT);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to the complement of a span of
 * base values
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Tnumber_minus_span()
 */
Temporal *
tnumber_minus_span(const Temporal *temp, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) s) ||
      ! ensure_tnumber_type(temp->temptype) ||
      ! ensure_valid_tnumber_span(temp, s))
    return NULL;
  return tnumber_restrict_span(temp, s, REST_MINUS);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to an array of spans of base
 * values
 * @param[in] temp Temporal value
 * @param[in] ss Span set
 * @csqlfn #Tnumber_at_spanset()
 */
Temporal *
tnumber_at_spanset(const Temporal *temp, const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) ss) ||
      ! ensure_tnumber_type(temp->temptype) ||
      ! ensure_valid_tnumber_spanset(temp, ss))
    return NULL;
  return tnumber_restrict_spanset(temp, ss, REST_AT);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to the complement of an array of
 * spans
 * of base values
 * @param[in] temp Temporal value
 * @param[in] ss Span set
 * @csqlfn #Tnumber_minus_spanset()
 */
Temporal *
tnumber_minus_spanset(const Temporal *temp, const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) ss) ||
      ! ensure_tnumber_type(temp->temptype) ||
      ! ensure_valid_tnumber_spanset(temp, ss))
    return NULL;
  return tnumber_restrict_spanset(temp, ss, REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to a timestamptz
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @csqlfn #Temporal_at_timestamptz()
 */
Temporal *
temporal_at_timestamptz(const Temporal *temp, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;
  return temporal_restrict_timestamptz(temp, t, REST_AT);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to the complement of a timestamptz
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @csqlfn #Temporal_minus_timestamptz()
 */
Temporal *
temporal_minus_timestamptz(const Temporal *temp, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;
  return temporal_restrict_timestamptz(temp, t, REST_MINUS);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to a timestamptz set
 * @param[in] temp Temporal value
 * @param[in] s Timestamp set
 * @csqlfn #Temporal_at_tstzset()
 */
Temporal *
temporal_at_tstzset(const Temporal *temp, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) s) ||
      ! ensure_set_isof_type(s, T_TSTZSET))
    return NULL;
  return temporal_restrict_tstzset(temp, s, REST_AT);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to the complement of a timestamptz
 * set
 * @param[in] temp Temporal value
 * @param[in] s Timestamp set
 * @csqlfn #Temporal_minus_tstzset()
 */
Temporal *
temporal_minus_tstzset(const Temporal *temp, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) s) ||
      ! ensure_set_isof_type(s, T_TSTZSET))
    return NULL;
  return temporal_restrict_tstzset(temp, s, REST_MINUS);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to a timestamptz span
 * @param[in] temp Temporal value
 * @param[in] s Timestamp span
 * @csqlfn #Temporal_at_tstzspan()
 */
Temporal *
temporal_at_tstzspan(const Temporal *temp, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) s) ||
      ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;
  return temporal_restrict_tstzspan(temp, s, REST_AT);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to the complement of a timestamptz
 * span
 * @param[in] temp Temporal value
 * @param[in] s Timestamp span
 * @csqlfn #Temporal_minus_tstzspan()
 */
Temporal *
temporal_minus_tstzspan(const Temporal *temp, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) s) ||
      ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;
  return temporal_restrict_tstzspan(temp, s, REST_MINUS);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to a timestamptz span set
 * @param[in] temp Temporal value
 * @param[in] ss Timestamp span set
 * @csqlfn #Temporal_at_tstzspanset()
 */
Temporal *
temporal_at_tstzspanset(const Temporal *temp, const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_TSTZSPANSET))
    return NULL;
  return temporal_restrict_tstzspanset(temp, ss, REST_AT);
}

/**
 * @ingroup meos_temporal_restrict
 * @brief Return a temporal value restricted to the complement of a timestamptz
 * span set
 * @param[in] temp Temporal value
 * @param[in] ss Timestamp span set
 * @csqlfn #Temporal_minus_tstzspanset()
 */
Temporal *
temporal_minus_tstzspanset(const Temporal *temp, const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_TSTZSPANSET))
    return NULL;
  return temporal_restrict_tstzspanset(temp, ss, REST_MINUS);
}

/*****************************************************************************/
