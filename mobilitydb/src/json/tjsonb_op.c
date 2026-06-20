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
 * @brief Operators for temporal JSONB
 */

/* PostgreSQL */
#include <postgres.h>
#include "utils/jsonb.h"
#include "utils/jsonpath.h"
#include "executor/spi.h"

#include "utils/jsonpath.h"

/* MEOS */
#include <meos.h>
#include <meos_json.h>
#include "temporal/span.h"
#include "json/tjsonb.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"

/*****************************************************************************/

PGDLLEXPORT Datum Tjsonb_exists(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_exists);
/**
 * @ingroup mobilitydb_temporal_jsonb
 * @brief Return a temporal boolean that states if the text string exist as a
 * top-level key or array element within the JSONB value
 * @sqlfn tjsonb_exists()
 * @sqlop @p ?
 */
Datum
Tjsonb_exists(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  text *key = PG_GETARG_TEXT_P(1);
  Temporal *result = tjsonb_exists(temp, key);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(key, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @ingroup mobilitydb_temporal_jsonb
 * @brief Return a temporal boolean indicating if any/all of the given keys
 *        exist as top-level keys or array elements in a temporal JSONB value
 * @sqlfn tjsonb_exists_any()
 * @sqlfn tjsonb_exists_all()
 */
Datum
Tjsonb_exists_array(FunctionCallInfo fcinfo, bool any)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  ArrayType *keys = PG_GETARG_ARRAYTYPE_P(1);
  if (ARR_NDIM(keys) > 1)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
       errmsg("wrong number of array subscripts")));

  /* Extract keys from the array */
  int keys_len;
  Datum *keys_elems;
  bool *keys_nulls;
  deconstruct_array(keys, TEXTOID, -1, false, 'i', &keys_elems, &keys_nulls,
    &keys_len);
  if (keys_len == 0)
    PG_RETURN_TEMPORAL_P(temp);

  /* Compute the result */
  Temporal *result = tjsonb_exists_array(temp, (text **) keys_elems, keys_len,
    any);
  pfree(keys_elems); pfree(keys_nulls);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(keys, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @ingroup mobilitydb_temporal_jsonb
 * @brief Return true if any of the strings in the text array exist as
 * top-level keys or array elements
 * @sqlfn tjsonb_exists_any()
 * @sqlop @p ?|
 */
PGDLLEXPORT Datum Tjsonb_exists_any(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_exists_any);
Datum
Tjsonb_exists_any(PG_FUNCTION_ARGS)
{
  return Tjsonb_exists_array(fcinfo, true);
}

/**
 * @ingroup mobilitydb_temporal_jsonb
 * @brief Return true if all of the strings in the text array exist as
 * top-level keys or array elements
 * @sqlfn tjsonb_exists_all()
 * @sqlop @p ?|
 */
PGDLLEXPORT Datum Tjsonb_exists_all(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_exists_all);
Datum
Tjsonb_exists_all(PG_FUNCTION_ARGS)
{
  return Tjsonb_exists_array(fcinfo, false);
}

/*****************************************************************************/

/**
 * @ingroup mobilitydb_temporal_jsonb
 * @brief Return true if a JSONB value contains a temporal JSONB value
 * @sqlfn tjsonb_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum Contains_jsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_jsonb_tjsonb);
Datum
Contains_jsonb_tjsonb(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = contains_tjsonb_jsonb(temp, jb, INVERT);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @ingroup mobilitydb_temporal_jsonb
 * @brief Return true if a temporal JSONB value contains a JSONB value
 * @sqlfn tjsonb_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum Contains_tjsonb_jsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tjsonb_jsonb);
Datum
Contains_tjsonb_jsonb(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Jsonb *jb = PG_GETARG_JSONB_P(1);
  Temporal *result = contains_tjsonb_jsonb(temp, jb, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(jb, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @ingroup mobilitydb_temporal_jsonb
 * @brief Return true if the first TJSONB value contains the second one
 * @sqlfn tjsonb_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum Contains_tjsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tjsonb_tjsonb);
Datum
Contains_tjsonb_tjsonb(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = contains_tjsonb_tjsonb(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

/**
 * @ingroup mobilitydb_temporal_jsonb
 * @brief Return true if a JSONB value is contained into a temporal JSONB value
 * @sqlfn tjsonb_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum Contained_jsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_jsonb_tjsonb);
Datum
Contained_jsonb_tjsonb(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = contains_tjsonb_jsonb(temp, jb, INVERT);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @ingroup mobilitydb_temporal_jsonb
 * @brief Return true if a temporal JSONB value is contained into a JSONB value
 * @sqlfn tjsonb_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum Contained_tjsonb_jsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tjsonb_jsonb);
Datum
Contained_tjsonb_jsonb(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Jsonb *jb = PG_GETARG_JSONB_P(1);
  Temporal *result = contains_tjsonb_jsonb(temp, jb, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(jb, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @ingroup mobilitydb_temporal_jsonb
 * @brief Return true if the first TJSONB value is contained into the second one
 * @sqlfn tjsonb_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum Contained_tjsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tjsonb_tjsonb);
Datum
Contained_tjsonb_tjsonb(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = contains_tjsonb_tjsonb(temp2, temp1);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
