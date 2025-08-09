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
 * @brief SQL-callable temporal JSONB functions: `concat`, `set`, `merge`
 */

#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include "temporal/tjsonb_funcs.h"
#include "utils/array.h"
#include <meos_internal.h>




extern Datum jsonb_set(PG_FUNCTION_ARGS);

/*****************************************************************************
 * JSONB concatenation
 *****************************************************************************/

PGDLLEXPORT Datum Jsonb_concat_jsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_concat_jsonb_tjsonb);
/**
 * @ingroup mobilitydb_temporal_jsonb
 * @brief Concat a JSONB constant with a temporal JSONB
 * @sqlfn jsonb_concat()
 * @sqlop @p |||
 */
Datum
Jsonb_concat_jsonb_tjsonb(PG_FUNCTION_ARGS)
{
  Datum      jb    = PG_GETARG_DATUM(0);
  Temporal  *temp  = PG_GETARG_TEMPORAL_P(1);
  Temporal  *result = jsonbfunc_tjsonb_jsonb(temp, jb,
                                            &datum_jsonb_concat,
                                            /*invert*/ true);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Jsonb_concat_tjsonb_jsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_concat_tjsonb_jsonb);
/**
 * @ingroup mobilitydb_temporal_jsonb
 * @brief Concat a temporal JSONB with a JSONB constant
 * @sqlfn jsonb_concat()
 * @sqlop @p |||
 */
Datum
Jsonb_concat_tjsonb_jsonb(PG_FUNCTION_ARGS)
{
  Temporal  *temp  = PG_GETARG_TEMPORAL_P(0);
  Datum      jb    = PG_GETARG_DATUM(1);
  Temporal  *result = jsonbfunc_tjsonb_jsonb(temp, jb,
                                            &datum_jsonb_concat,
                                            /*invert*/ false);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Jsonb_concat_tjsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_concat_tjsonb_tjsonb);
/**
 * @ingroup mobilitydb_temporal_jsonb
 * @brief Concat two temporal JSONB values
 * @sqlfn jsonb_concat()
 * @sqlop @p |||
 */
Datum
Jsonb_concat_tjsonb_tjsonb(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = jsonbfunc_tjsonb_tjsonb(temp1, temp2,
                                            &datum_jsonb_concat);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * JSONB set 
 *****************************************************************************/
/* ------------------------------- SQL wrapper ------------------------------- */
/* If the mapper is declared elsewhere, add this extern to silence warnings. */
extern Temporal *jsonbfunc_tjsonb_jsonb(const Temporal *temp,
                                        Datum right,
                                        Datum (*func)(Datum, Datum),
                                        bool invert);

/* ---------- Pack jsonb_set extra args to pass through the 2-arg mapper ----- */
typedef struct JsonbSetPackedArgs
{
  ArrayType *path;      /* text[] */
  Jsonb     *val;       /* jsonb  */
  bool       create_missing;
} JsonbSetPackedArgs;

/* 2-arg adapter for the mapper: left = per-instant jsonb, right = packed args */
static Datum
datum_jsonb_set_packed(Datum left, Datum right)
{
  JsonbSetPackedArgs *args = (JsonbSetPackedArgs *) DatumGetPointer(right);
  return DirectFunctionCall4(jsonb_set,
                             left,
                             PointerGetDatum(args->path),  /* text[] */
                             PointerGetDatum(args->val),   /* jsonb  */
                             BoolGetDatum(args->create_missing));
}

/* ------------------------------- SQL wrapper ------------------------------- */
PGDLLEXPORT Datum Tjsonb_set_path(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_set_path);
Datum
Tjsonb_set_path(PG_FUNCTION_ARGS)
{
  /* Basic safety: STRICT in SQL should already ensure non-null, but double-check */
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2) || PG_ARGISNULL(3))
    ereport(ERROR, (errmsg("tjsonb_set_path arguments must be non-null")));

  Temporal  *temp = PG_GETARG_TEMPORAL_P(0);
  ArrayType *path = PG_GETARG_ARRAYTYPE_P(1); /* text[] */
  Jsonb     *val  = PG_GETARG_JSONB_P(2);
  bool create_missing = PG_GETARG_BOOL(3);

  /* Pack once; mapper will reuse for each instant */
  JsonbSetPackedArgs *packed =
      (JsonbSetPackedArgs *) palloc(sizeof(JsonbSetPackedArgs));
  packed->path = path;
  packed->val  = val;
  packed->create_missing = create_missing;

  Temporal *res = jsonbfunc_tjsonb_jsonb(
                    temp,
                    PointerGetDatum(packed),
                    &datum_jsonb_set_packed,
                    /* invert */ false);

  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(path, 1);
  PG_RETURN_TEMPORAL_P(res);
}
