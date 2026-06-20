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
 * @brief Basic functions for for JSONB sets
 */

/* PostgreSQL */
#include <postgres.h>
#include "utils/jsonb.h"
/* MEOS */
#include <meos.h>
#include <meos_json.h>
#include "temporal/set.h"
#include "temporal/span.h"
#include "json/tjsonb.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Jsonbset_as_textset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_as_textset);
/**
 * @ingroup mobilitydb_json_jsonbset
 * @brief Transform a JSONB set into a text set
 * @sqlfn textset()
 * @sqlop @p ::
 */
Datum
Jsonbset_as_textset(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Set *result = jsonbfunc_jsonbset(s, &datum_jsonb_to_text, T_JSONBSET,
    T_TEXT);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Textset_as_jsonbset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Textset_as_jsonbset);
/**
 * @ingroup mobilitydb_json_jsonbset
 * @brief Transform a text set into a JSONB set
 * @sqlfn ttext()
 * @sqlop @p ::
 */
Datum
Textset_as_jsonbset(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Set *result = jsonbfunc_jsonbset(s, &datum_text_to_jsonb, T_TEXTSET,
    T_JSONB);
  PG_FREE_IF_COPY(s, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * JSONB functions
 *****************************************************************************/

// PGDLLEXPORT Datum Concat_jsonb_jsonbset(PG_FUNCTION_ARGS);
// PG_FUNCTION_INFO_V1(Concat_jsonb_jsonbset);
// /**
 // * @ingroup mobilitydb_json_jsonbset
 // * @brief Concat a JSONB constant with a JSONB set
 // * @sqlfn jsonb_concat()
 // * @sqlop @p ||
 // */
// Datum
// Concat_jsonb_jsonbset(PG_FUNCTION_ARGS)
// {
  // Jsonb *jb = PG_GETARG_JSONB_P(0);
  // Set *s = PG_GETARG_SET_P(1);
  // Set *result = jsonbfunc_jsonbset_jsonb(s, jb, &datum_jsonb_concat, true);
  // PG_FREE_IF_COPY(s, 1);
  // PG_RETURN_SET_P(result);
// }

// PGDLLEXPORT Datum Concat_jsonbset_jsonb(PG_FUNCTION_ARGS);
// PG_FUNCTION_INFO_V1(Concat_jsonbset_jsonb);
// /**
 // * @ingroup mobilitydb_json_jsonbset
 // * @brief Concat a JSONB set with a JSONB constant
 // * @sqlfn jsonb_concat()
 // * @sqlop @p ||
 // */
// Datum
// Concat_jsonbset_jsonb(PG_FUNCTION_ARGS)
// {
  // Set *s = PG_GETARG_SET_P(0);
  // Jsonb *jb = PG_GETARG_JSONB_P(1);
  // Set *result = jsonbfunc_jsonbset_jsonb(s, jb, &datum_jsonb_concat, false);
  // PG_FREE_IF_COPY(s, 0);
  // PG_RETURN_SET_P(result);
// }

/*****************************************************************************/

// PGDLLEXPORT Datum Delete_jsonbset_key(PG_FUNCTION_ARGS);
// PG_FUNCTION_INFO_V1(Delete_jsonbset_key);
// /**
 // * @ingroup mobilitydb_json_jsonbset
 // * @brief Delete a key from a JSONB set
 // * @sqlfn jsonb_delte()
 // * @sqlop @p -
 // */
// Datum
// Delete_jsonbset_key(PG_FUNCTION_ARGS)
// {
  // Set *s = PG_GETARG_SET_P(0);
  // text *key = PG_GETARG_TEXT_P(1);
  // Set *result = jsonbfunc_jsonbset_text(s, key, &datum_jsonb_delete);
  // PG_FREE_IF_COPY(s, 0);
  // PG_FREE_IF_COPY(key, 1);
  // if (! result)
    // PG_RETURN_NULL();
  // PG_RETURN_TEMPORAL_P(result);
// }

/*****************************************************************************/
