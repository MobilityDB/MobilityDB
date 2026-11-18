/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
#include "temporal/span.h"
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
 * @sqlfn eEqual()
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
 * @sqlfn aEqual()
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
 * @sqlfn eNotEqual()
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
 * @sqlfn aNotEqual()
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
 * @sqlfn eEqual()
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
 * @sqlfn aEqual()
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
 * @sqlfn eNotEqual()
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
 * @sqlfn aNotEqual()
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
 * @sqlfn eEqual()
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
 * @sqlfn aEqual()
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
 * @sqlfn eNotEqual()
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
 * @sqlfn aNotEqual()
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
 * @sqlfn tEqual()
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
 * @sqlfn tNotEqual()
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
 * @sqlfn tEqual()
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
 * @sqlfn tNotEqual()
 * @sqlop @p #<>
 */
inline Datum
Tne_tjsonb_jsonb(PG_FUNCTION_ARGS)
{
  return Tcomp_tjsonb_jsonb(fcinfo, &tne_tjsonb_jsonb);
}

/*****************************************************************************/

/*****************************************************************************
 * Ever/always overlaps with the bounding box of the type
 *****************************************************************************/

/**
 * @brief Generic function for the ever/always overlaps of a timestamptz span
 * and a temporal JSONB value, as #EAcomp_jsonb_tjsonb answers an ever or
 * always comparison
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always overlaps
 */
static Datum
EAoverlaps_tstzspan_tjsonb(FunctionCallInfo fcinfo,
  int (*func)(const Span *, const Temporal *))
{
  Span *s = PG_GETARG_SPAN_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = func(s, temp);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic function for the ever/always overlaps of a temporal JSONB
 * value and a timestamptz span, as #EAcomp_tjsonb_jsonb answers an ever or
 * always comparison
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always overlaps
 */
static Datum
EAoverlaps_tjsonb_tstzspan(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const Span *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  int result = func(temp, s);
  PG_FREE_IF_COPY(temp, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Ever_overlaps_tstzspan_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_overlaps_tstzspan_tjsonb);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if a temporal JSONB value is ever defined in a
 * timestamptz span, as #Ever_overlaps_tstzspan_temporal answers it for the
 * temporal types
 * @sqlfn eOverlaps()
 * @sqlop @p ?&&
 */
inline Datum
Ever_overlaps_tstzspan_tjsonb(PG_FUNCTION_ARGS)
{
  return EAoverlaps_tstzspan_tjsonb(fcinfo, &ever_overlaps_tstzspan_temporal);
}

PGDLLEXPORT Datum Always_overlaps_tstzspan_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_overlaps_tstzspan_tjsonb);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if a temporal JSONB value is always defined in a
 * timestamptz span, as #Always_overlaps_tstzspan_temporal answers it for the
 * temporal types
 * @sqlfn aOverlaps()
 * @sqlop @p %&&
 */
inline Datum
Always_overlaps_tstzspan_tjsonb(PG_FUNCTION_ARGS)
{
  return EAoverlaps_tstzspan_tjsonb(fcinfo, &always_overlaps_tstzspan_temporal);
}

PGDLLEXPORT Datum Ever_overlaps_tjsonb_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_overlaps_tjsonb_tstzspan);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if a temporal JSONB value is ever defined in a
 * timestamptz span, as #Ever_overlaps_temporal_tstzspan answers it for the
 * temporal types
 * @sqlfn eOverlaps()
 * @sqlop @p ?&&
 */
inline Datum
Ever_overlaps_tjsonb_tstzspan(PG_FUNCTION_ARGS)
{
  return EAoverlaps_tjsonb_tstzspan(fcinfo, &ever_overlaps_temporal_tstzspan);
}

PGDLLEXPORT Datum Always_overlaps_tjsonb_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_overlaps_tjsonb_tstzspan);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if a temporal JSONB value is always defined in a
 * timestamptz span, as #Always_overlaps_temporal_tstzspan answers it for the
 * temporal types
 * @sqlfn aOverlaps()
 * @sqlop @p %&&
 */
inline Datum
Always_overlaps_tjsonb_tstzspan(PG_FUNCTION_ARGS)
{
  return EAoverlaps_tjsonb_tstzspan(fcinfo, &always_overlaps_temporal_tstzspan);
}

/*****************************************************************************/

/**
 * @brief Generic function for the ever/always overlaps of two temporal JSONB
 * values, as #EAcomp_tjsonb_jsonb answers an ever or always comparison
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always overlaps
 */
static Datum
EAoverlaps_tjsonb_tjsonb(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const Temporal *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  int result = func(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Ever_overlaps_tjsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_overlaps_tjsonb_tjsonb);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if two temporal JSONB values are ever defined at the
 * same time, as #Ever_overlaps_temporal_temporal answers it for the temporal
 * types
 * @sqlfn eOverlaps()
 * @sqlop @p ?&&
 */
inline Datum
Ever_overlaps_tjsonb_tjsonb(PG_FUNCTION_ARGS)
{
  return EAoverlaps_tjsonb_tjsonb(fcinfo, &ever_overlaps_temporal_temporal);
}

PGDLLEXPORT Datum Always_overlaps_tjsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_overlaps_tjsonb_tjsonb);
/**
 * @ingroup mobilitydb_json_comp_ever
 * @brief Return true if two temporal JSONB values are always defined at the
 * same time, as #Always_overlaps_temporal_temporal answers it for the
 * temporal types
 * @sqlfn aOverlaps()
 * @sqlop @p %&&
 */
inline Datum
Always_overlaps_tjsonb_tjsonb(PG_FUNCTION_ARGS)
{
  return EAoverlaps_tjsonb_tjsonb(fcinfo, &always_overlaps_temporal_temporal);
}

/*****************************************************************************/
