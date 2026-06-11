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
 * @brief Basic functions for temporal JSONB
 */

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include "utils/varlena.h"
/* MEOS */
#include <meos.h>
#include <meos_json.h>
#include <meos_internal.h>
#include <pgtypes.h>
#include "temporal/meos_catalog.h"
#include "temporal/temporal.h"
#include "temporal/lifting.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "json/tjsonb.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_json_inout
 * @brief Return a temporal JSONB from its Well-Known Text (WKT) representation
 * @param[in] str String
 */
Temporal *
tjsonb_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return temporal_in(str, T_TJSONB);
}

/**
 * @ingroup meos_internal_json_inout
 * @brief Return a temporal JSONB instant from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TInstant *
tjsonbinst_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  /* Parse the WKT into a TInstant, telling the parser this is a JSONB instant */
  return tinstant_in(str, T_TJSONB);
}

/**
 * @ingroup meos_internal_json_inout
 * @brief Return a temporal JSONB sequence from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
inline TSequence *
tjsonbseq_in(const char *str, interpType interp UNUSED)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  /* Call the superclass function */
  Temporal *temp = temporal_parse(&str, T_TJSONB);
  assert(temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/**
 * @ingroup meos_internal_json_inout
 * @brief Return a temporal JSONB sequence set from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TSequenceSet *
tjsonbseqset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  /* Call the superclass function */
  Temporal *temp = temporal_parse(&str, T_TJSONB);
  assert(temp->subtype == TSEQUENCESET);
  return (TSequenceSet *) temp;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_json_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal JSONB
 * @param[in] temp Temporal JSONB
 */
char *
tjsonb_out(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL);
  return temporal_out(temp, 0);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_json_inout
 * @brief Return a temporal JSONB instant from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 */
inline TInstant *
tjsonbinst_from_mfjson(const json_object *mfjson)
{
  /* false = not linear, 0 = unused */
  return tinstant_from_mfjson(mfjson, false, 0, T_TJSONB);
}

/**
 * @ingroup meos_internal_json_inout
 * @brief Return a temporal JSONB sequence from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 */
inline TSequence *
tjsonbseq_from_mfjson(const json_object *mfjson)
{
  /* false = not linear, 0 = unused */
  return tsequence_from_mfjson(mfjson, false, 0, T_TJSONB, STEP);
}

/**
 * @ingroup meos_internal_json_inout
 * @brief Return a temporal JSONB sequence set from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 */
inline TSequenceSet *
tjsonbseqset_from_mfjson(const json_object *mfjson)
{
  /* false = not linear, 0 = unused */
  return tsequenceset_from_mfjson(mfjson, false, 0, T_TJSONB, STEP);
}

/**
 * @ingroup meos_json_inout
 * @brief Return a temporal JSONB from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @return On error return @p NULL
 * @see #temporal_from_mfjson()
 */

Temporal *
tjsonb_from_mfjson(const char *mfjson)
{
  return temporal_from_mfjson(mfjson, T_TJSONB);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_json_constructor
 * @brief Return a temporal JSONB instant from a JSONB and a timestamptz
 * @param[in] jb Value
 * @param[in] t Timestamp
 * @csqlfn #Tinstant_constructor()
 */
TInstant *
tjsonbinst_make(const Jsonb *jb, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(jb, NULL);
  return tinstant_make(PointerGetDatum(jb), T_TJSONB, t);
}

/**
 * @ingroup meos_json_constructor
 * @brief Return a temporal JSONB discrete sequence from a JSONB value and a
 * timestamptz set
 * @param[in] jb Value
 * @param[in] s Set
 * @csqlfn #Tsequence_from_base_tstzset()
 */
TSequence *
tjsonbseq_from_base_tstzset(const Jsonb *jb, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(jb, NULL); VALIDATE_TSTZSET(s, NULL);
  return tsequence_from_base_tstzset(PointerGetDatum(jb), T_TJSONB, s);
}

/**
 * @ingroup meos_json_constructor
 * @brief Return a temporal JSONB sequence from a JSONB value and a timestamptz
 * span
 * @param[in] jb Value
 * @param[in] sp Span
 * @csqlfn #Tsequence_from_base_tstzspan()
 */
TSequence *
tjsonbseq_from_base_tstzspan(const Jsonb *jb, const Span *sp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(jb, NULL); VALIDATE_TSTZSPAN(sp, NULL);
  return tsequence_from_base_tstzspan(PointerGetDatum(jb), T_TJSONB, sp, STEP);
}

/**
 * @ingroup meos_json_constructor
 * @brief Return a temporal JSONB sequence set from a JSONB value and a
 * timestamptz span set
 * @param[in] jb Value
 * @param[in] ss Span set
 * @csqlfn #Tsequenceset_from_base_tstzspanset()
 */
TSequenceSet *
tjsonbseqset_from_base_tstzspanset(const Jsonb *jb, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(jb, NULL);
  VALIDATE_TSTZSPANSET(ss, NULL);
  /* Delegate to the generic tsequenceset constructor, with STEP interpolation */
  return tsequenceset_from_base_tstzspanset(PointerGetDatum(jb),T_TJSONB, ss,
    STEP);
}

/**
 * @ingroup meos_json_constructor
 * @brief Return a temporal JSONB from a JSONB and the time frame of
 * another temporal value
 * @param[in] jb Value
 * @param[in] temp Temporal value
 */
Temporal *
tjsonb_from_base_temp(const Jsonb *jb, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(jb, NULL); VALIDATE_NOT_NULL(temp, NULL);
  return temporal_from_base_temp(PointerGetDatum(jb), T_TJSONB, temp);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_json_conversion
 * @brief Convert a temporal JSONB value into temporal text
 * @param[in] temp Temporal JSONB value
 * @csqlfn #Tjsonb_as_ttext()
 */
Temporal *
tjsonb_to_ttext(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  assert(temp); assert(temp->temptype == T_TJSONB);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_jsonb_to_text;
  lfinfo.argtype[0] = T_TJSONB;
  lfinfo.restype = T_TTEXT;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_json_conversion
 * @brief Convert a temporal text into a temporal JSONB value
 * @param[in] temp Temporal JSONB value
 * @csqlfn #Ttext_as_tjsonb()
 */
Temporal *
ttext_to_tjsonb(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  assert(temp); assert(temp->temptype == T_TTEXT);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_text_to_jsonb;
  lfinfo.argtype[0] = T_TTEXT;
  lfinfo.restype =  T_TJSONB;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_json_accessor
 * @brief Return the start value of a temporal JSONB
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_start_value()
 */
Jsonb *
tjsonb_start_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL);
  return DatumGetJsonbP(temporal_start_value(temp));
}

/**
 * @ingroup meos_json_accessor
 * @brief Return the end value of a temporal JSONB
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_end_value()
 */
Jsonb *
tjsonb_end_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL);
  return DatumGetJsonbP(temporal_end_value(temp));
}

/**
 * @ingroup meos_json_accessor
 * @brief Return the n-th value of a temporal JSONB
 * @param[in] temp Temporal value
 * @param[in] n Number
 * @param[out] result Value
 * @csqlfn #Temporal_value_n()
 */
bool
tjsonb_value_n(const Temporal *temp, int n, Jsonb **result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, false); VALIDATE_NOT_NULL(result, false);
  Datum dresult;
  if (! temporal_value_n(temp, n, &dresult))
    return false;
  *result = DatumGetJsonbP(dresult);
  return true;
}

/**
 * @ingroup meos_json_accessor
 * @brief Return an array of copies of base values of a temporal JSONB
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
Jsonb **
tjsonb_values(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  Datum *datumarr = temporal_values_p(temp, count);
  Jsonb **result = palloc(sizeof(Jsonb *) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = pg_jsonb_copy(DatumGetJsonbP(datumarr[i]));
  pfree(datumarr);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_json_accessor
 * @brief Return the value of a temporal JSONB at a timestamptz
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] strict True if the timestamp must belong to the temporal value,
 * false when it may be at an exclusive bound
 * @param[out] value Resulting value
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
tjsonb_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  Jsonb **value)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, false); VALIDATE_NOT_NULL(value, false);
  Datum res;
  bool result = temporal_value_at_timestamptz(temp, t, strict, &res);
  *value = DatumGetJsonbP(res);
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/


/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @ingroup meos_json_restrict
 * @brief Return a temporal JSONB restricted to a specific JSONB value
 * @param[in] temp Temporal value
 * @param[in] jb JSONB value
 * @csqlfn #Temporal_at_value()
 */
Temporal *
tjsonb_at_value(const Temporal *temp, const Jsonb *jb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL); VALIDATE_NOT_NULL(jb, NULL);
  /* Restrict the temporal JSONB to the instants where it equals the given jb */
  return temporal_restrict_value(temp, PointerGetDatum(jb), REST_AT);
}

/**
 * @ingroup meos_json_restrict
 * @brief Return a temporal JSONB restricted to the complement of a specific
 * JSONB value
 * @param[in] temp Temporal value
 * @param[in] jb JSONB value
 * @csqlfn #Temporal_minus_value()
 */
Temporal *
tjsonb_minus_value(const Temporal *temp, const Jsonb *jb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL); VALIDATE_NOT_NULL(jb, NULL);
  /* Restrict the temporal JSONB to the instants where it does not equal the
   * given jb */
  return temporal_restrict_value(temp, PointerGetDatum(jb), REST_MINUS);
}

/*****************************************************************************/

