/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @file ttext_textfuncs.c
 * @brief Temporal text functions: `textcat`, `lower`, `upper`.
 */

#include "general/ttext_textfuncs.h"

/* PostgreSQL */
#include <utils/builtins.h>
/* MobilityDB */
#include "general/temporal.h"
#include "general/temporal_util.h"

/*****************************************************************************
 * Text concatenation
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_text
 * @brief Return the concatenation of the text value and the temporal text values
 */
Temporal *
textcat_text_ttext(Datum value, Temporal *temp)
{
  Temporal *result = textfunc_ttext_text(temp, value, &datum_textcat, INVERT);
  return result;
}

/**
 * @ingroup libmeos_temporal_text
 * @brief Return the concatenation of the temporal text value and the text value
 */
Temporal *
textcat_ttext_text(const Temporal *temp, Datum value)
{
  Temporal *result = textfunc_ttext_text(temp, value, &datum_textcat, INVERT_NO);
  return result;
}

/**
 * @ingroup libmeos_temporal_text
 * @brief Return the concatenation of the two temporal text values
 */
Temporal *
textcat_ttext_ttext(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *result = textfunc_ttext_ttext(temp1, temp2, &datum_textcat);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_text
 * @brief Transform the temporal text value into uppercase
 */
Temporal *
ttext_upper(const Temporal *temp)
{
  Temporal *result = textfunc_ttext(temp, &datum_upper);
  return result;
}

/**
 * @ingroup libmeos_temporal_text
 * @brief Transform the temporal text value into lowercase
 */
Temporal *
ttext_lower(const Temporal *temp)
{
  Temporal *result = textfunc_ttext(temp, &datum_lower);
  return result;
}

/*****************************************************************************/
