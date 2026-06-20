/***********************************************************************
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
 * @brief Ever/always and temporal comparisons for temporal JSONB values
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
#include <utils/jsonb.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_json.h>
#include "temporal/temporal.h"
#include "temporal/temporal_compops.h"
#include "json/tjsonb.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"

/*****************************************************************************
 * Ever/always comparison functions
 *****************************************************************************/

/**
 * @brief Generic function for the ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
EAcomp_jsonb_tjsonb(FunctionCallInfo fcinfo,
  int (*func)(const Jsonb *, const Temporal *))
{
  Jsonb *cb = PG_GETARG_JSONB_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = func(cb, temp);
  PG_FREE_IF_COPY(cb, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic function for the ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
EAcomp_tjsonb_jsonb(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const Jsonb *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Jsonb *cb = PG_GETARG_JSONB_P(1);
  int result = func(temp, cb);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(cb, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_jsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_jsonb_tjsonb);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if a JSONB value is ever equal to a temporal JSONB value
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_jsonb_tjsonb(PG_FUNCTION_ARGS)
{
  return EAcomp_jsonb_tjsonb(fcinfo, &ever_eq_jsonb_tjsonb);
}

PGDLLEXPORT Datum Always_eq_jsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_jsonb_tjsonb);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if a temporal JSONB value is always equal to a JSONB
 * value
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_jsonb_tjsonb(PG_FUNCTION_ARGS)
{
  return EAcomp_jsonb_tjsonb(fcinfo, &always_eq_jsonb_tjsonb);
}

PGDLLEXPORT Datum Ever_ne_jsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_jsonb_tjsonb);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if a temporal JSONB value is ever different from a JSONB
 * value
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_jsonb_tjsonb(PG_FUNCTION_ARGS)
{
  return EAcomp_jsonb_tjsonb(fcinfo, &ever_ne_jsonb_tjsonb);
}

PGDLLEXPORT Datum Always_ne_jsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_jsonb_tjsonb);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if a temporal JSONB value is always different from a
 * JSONB value
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_jsonb_tjsonb(PG_FUNCTION_ARGS)
{
  return EAcomp_jsonb_tjsonb(fcinfo, &always_ne_jsonb_tjsonb);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tjsonb_jsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tjsonb_jsonb);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if a temporal JSONB value is ever equal to a JSONB value
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_tjsonb_jsonb(PG_FUNCTION_ARGS)
{
  return EAcomp_tjsonb_jsonb(fcinfo, &ever_eq_tjsonb_jsonb);
}

PGDLLEXPORT Datum Always_eq_tjsonb_jsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tjsonb_jsonb);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if a temporal JSONB value is always equal to a JSONB
 * value
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_tjsonb_jsonb(PG_FUNCTION_ARGS)
{
  return EAcomp_tjsonb_jsonb(fcinfo, &always_eq_tjsonb_jsonb);
}

PGDLLEXPORT Datum Ever_ne_tjsonb_jsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tjsonb_jsonb);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if a temporal JSONB value is ever different from a JSONB
 * value
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_tjsonb_jsonb(PG_FUNCTION_ARGS)
{
  return EAcomp_tjsonb_jsonb(fcinfo, &ever_ne_tjsonb_jsonb);
}

PGDLLEXPORT Datum Always_ne_tjsonb_jsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tjsonb_jsonb);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if a temporal JSONB value is always different from a
 * JSONB value
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_tjsonb_jsonb(PG_FUNCTION_ARGS)
{
  return EAcomp_tjsonb_jsonb(fcinfo, &always_ne_tjsonb_jsonb);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tjsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tjsonb_tjsonb);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if two temporal JSONB values are ever equal
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_tjsonb_tjsonb(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &ever_eq_tjsonb_tjsonb);
}

PGDLLEXPORT Datum Always_eq_tjsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tjsonb_tjsonb);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if two temporal JSONB values are always equal
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_tjsonb_tjsonb(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &always_eq_tjsonb_tjsonb);
}

PGDLLEXPORT Datum Ever_ne_tjsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tjsonb_tjsonb);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if two temporal JSONB values are ever different
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_tjsonb_tjsonb(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &ever_ne_tjsonb_tjsonb);
}

PGDLLEXPORT Datum Always_ne_tjsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tjsonb_tjsonb);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if two temporal JSONB values are always different
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_tjsonb_tjsonb(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &always_ne_tjsonb_tjsonb);
}

/*****************************************************************************
 * Temporal comparison functions
 *****************************************************************************/

/**
 * @brief Generic function for the temporal comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
Tcomp_jsonb_tjsonb(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Jsonb *, const Temporal *))
{
  Jsonb *cb = PG_GETARG_JSONB_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = func(cb, temp);
  PG_FREE_IF_COPY(cb, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Generic function for the temporal comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
Tcomp_tjsonb_jsonb(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const Jsonb *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Jsonb *cb = PG_GETARG_JSONB_P(1);
  Temporal *result = func(temp, cb);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(cb, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_jsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_jsonb_tjsonb);
/**
 * @ingroup mobilitydb_json_comp_temp
 * @brief Return a temporal boolean that states whether a JSONB value is
 * equal to a temporal JSONB value
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_jsonb_tjsonb(PG_FUNCTION_ARGS)
{
  return Tcomp_jsonb_tjsonb(fcinfo, &teq_jsonb_tjsonb);
}

PGDLLEXPORT Datum Tne_jsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_jsonb_tjsonb);
/**
 * @ingroup mobilitydb_json_comp_temp
 * @brief Return a temporal boolean that states whether a JSONB value is
 * different from a temporal JSONB value
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_jsonb_tjsonb(PG_FUNCTION_ARGS)
{
  return Tcomp_jsonb_tjsonb(fcinfo, &tne_jsonb_tjsonb);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_tjsonb_jsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tjsonb_jsonb);
/**
 * @ingroup mobilitydb_json_comp_temp
 * @brief Return a temporal boolean that states whether a temporal JSONB value
 * is equal to a JSONB value
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_tjsonb_jsonb(PG_FUNCTION_ARGS)
{
  return Tcomp_tjsonb_jsonb(fcinfo, &teq_tjsonb_jsonb);
}

PGDLLEXPORT Datum Tne_tjsonb_jsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tjsonb_jsonb);
/**
 * @ingroup mobilitydb_json_comp_temp
 * @brief Return a temporal boolean that states whether a temporal JSONB value
 * is different from a JSONB value
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_tjsonb_jsonb(PG_FUNCTION_ARGS)
{
  return Tcomp_tjsonb_jsonb(fcinfo, &tne_tjsonb_jsonb);
}

/*****************************************************************************/
