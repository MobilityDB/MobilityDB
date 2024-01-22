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
 * @brief Temporal text functions
 */

#include "general/ttext_textfuncs.h"

/* MEOS */
#include <meos.h>

/*****************************************************************************
 * Text concatenation
 *****************************************************************************/

/**
 * @ingroup meos_temporal_text
 * @brief Return the concatenation of two texts
 * @param[in] txt1,txt2 Values
 */
text *
textcat_text_text(const text *txt1, const text *txt2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) txt1) || ! ensure_not_null((void *) txt2))
    return NULL;
  return text_catenate(txt1, txt2);
}

/**
 * @ingroup meos_temporal_text
 * @brief Return the concatenation of a text and a temporal text
 * @param[in] txt Value
 * @param[in] temp Temporal value
 * @csqlfn #Textcat_text_ttext()
 */
Temporal *
textcat_text_ttext(const text *txt, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return textfunc_ttext_text(temp, PointerGetDatum(txt), &datum_textcat,
    INVERT);
}

/**
 * @ingroup meos_temporal_text
 * @brief Return the concatenation of a temporal text and a text
 * @param[in] temp Temporal value
 * @param[in] txt Value
 * @csqlfn #Textcat_ttext_text()
 */
Temporal *
textcat_ttext_text(const Temporal *temp, const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return textfunc_ttext_text(temp, PointerGetDatum(txt), &datum_textcat,
    INVERT_NO);
}

/**
 * @ingroup meos_temporal_text
 * @brief Return the concatenation of two temporal text values
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Textcat_ttext_ttext()
 */
Temporal *
textcat_ttext_ttext(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) ||
      ! ensure_temporal_isof_type(temp1, T_TTEXT) ||
      ! ensure_same_temporal_type(temp1, temp2))
    return NULL;
  return textfunc_ttext_ttext(temp1, temp2, &datum_textcat);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_text
 * @brief Return a temporal text transformed to lowercase
 * @param[in] temp Temporal value
 * @csqlfn #Ttext_lower()
 */
Temporal *
ttext_lower(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return textfunc_ttext(temp, &datum_lower);
}

/**
 * @ingroup meos_temporal_text
 * @brief Return a temporal text transformed to uppercase
 * @param[in] temp Temporal value
 * @csqlfn #Ttext_upper()
 */
Temporal *
ttext_upper(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return textfunc_ttext(temp, &datum_upper);
}

/**
 * @ingroup meos_temporal_text
 * @brief Return a temporal text transformed to initcap
 * @param[in] temp Temporal value
 * @csqlfn #Ttext_lower()
 */
Temporal *
ttext_initcap(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return textfunc_ttext(temp, &datum_initcap);
}

/*****************************************************************************/
