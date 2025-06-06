/*****************************************************************************
 * @brief
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
 * @brief Temporal text functions: `textcat`, `lower`, `upper`
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include "temporal/ttext_funcs.h"

/*****************************************************************************
 * Text concatenation
 *****************************************************************************/

PGDLLEXPORT Datum Textcat_text_ttext(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Textcat_text_ttext);
/**
 * @ingroup mobilitydb_temporal_text
 * @brief Return the concatenation of a text and a temporal text
 * @sqlfn textcat()
 * @sqlop @p ||
 */
Datum
Textcat_text_ttext(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = textfunc_ttext_text(temp, value, &datum_textcat, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Textcat_ttext_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Textcat_ttext_text);
/**
 * @ingroup mobilitydb_temporal_text
 * @brief Return the concatenation of a temporal text and a text
 * @sqlfn textcat()
 * @sqlop @p ||
 */
Datum
Textcat_ttext_text(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_DATUM(1);
  Temporal *result = textfunc_ttext_text(temp, value, &datum_textcat, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Textcat_ttext_ttext(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Textcat_ttext_ttext);
/**
 * @ingroup mobilitydb_temporal_text
 * @brief Return the concatenation of the two temporal texts
 * @sqlfn textcat()
 * @sqlop @p ||
 */
Datum
Textcat_ttext_ttext(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = textfunc_ttext_ttext(temp1, temp2, &datum_textcat);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ttext_lower(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttext_lower);
/**
 * @ingroup mobilitydb_temporal_text
 * @brief Return a temporal text transformed to lowercase
 * @sqlfn lower()
 */
Datum
Ttext_lower(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = textfunc_ttext(temp, &datum_lower);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ttext_upper(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttext_upper);
/**
 * @ingroup mobilitydb_temporal_text
 * @brief Return a temporal text transformed to uppercase
 * @sqlfn upper()
 */
Datum
Ttext_upper(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = textfunc_ttext(temp, &datum_upper);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ttext_initcap(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttext_initcap);
/**
 * @ingroup mobilitydb_temporal_text
 * @brief Return a temporal text transformed to uppercase
 * @sqlfn initcap()
 */
Datum
Ttext_initcap(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = textfunc_ttext(temp, &datum_initcap);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
