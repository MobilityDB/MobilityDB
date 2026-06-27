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
 * @brief PG V1 wrappers for the tquadbin comparison operators.
 *
 * Each wrapper unpacks a Quadbin from a bigint Datum (the SQL
 * surface for bare cells) or a Temporal from a TOAST datum, calls
 * the tquadbin-specific MEOS function declared in `meos_quadbin.h`, and
 * returns the result.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_quadbin.h>
#include "temporal/temporal.h"
#include "quadbin/quadbin.h"
#include "quadbin/quadbin_meos.h"   /* DatumGetQuadbin / QuadbinGetDatum */
/* MobilityDB */
#include "pg_temporal/temporal.h"

/* PG_GETARG_QUADBIN / PG_RETURN_QUADBIN are defined locally in
 * `quadbin.c`; replicate here so we can use them in the comparison
 * wrappers without cross-file include. */
#define PG_GETARG_QUADBIN(n) DatumGetQuadbin(PG_GETARG_DATUM(n))

/*****************************************************************************
 * Shared helpers
 *****************************************************************************/

static Datum
EAcomp_quadbin_tquadbin(FunctionCallInfo fcinfo,
  int (*func)(Quadbin, const Temporal *))
{
  Quadbin cell = PG_GETARG_QUADBIN(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = func(cell, temp);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

static Datum
EAcomp_tquadbin_quadbin(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, Quadbin))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Quadbin cell = PG_GETARG_QUADBIN(1);
  int result = func(temp, cell);
  PG_FREE_IF_COPY(temp, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

static Datum
EAcomp_tquadbin_tquadbin(FunctionCallInfo fcinfo,
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

static Datum
Tcomp_quadbin_tquadbin(FunctionCallInfo fcinfo,
  Temporal *(*func)(Quadbin, const Temporal *))
{
  Quadbin cell = PG_GETARG_QUADBIN(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = func(cell, temp);
  PG_FREE_IF_COPY(temp, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

static Datum
Tcomp_tquadbin_quadbin(FunctionCallInfo fcinfo,
  Temporal *(*func)(const Temporal *, Quadbin))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Quadbin cell = PG_GETARG_QUADBIN(1);
  Temporal *result = func(temp, cell);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

static Datum
Tcomp_tquadbin_tquadbin(FunctionCallInfo fcinfo,
  Temporal *(*func)(const Temporal *, const Temporal *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = func(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Ever equal
 *****************************************************************************/

PGDLLEXPORT Datum Ever_eq_quadbin_tquadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_quadbin_tquadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_ever
 * @brief Return true if a QUADBIN cell is ever equal to a temporal QUADBIN cell
 * @sqlfn eEq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_quadbin_tquadbin(PG_FUNCTION_ARGS)
{ return EAcomp_quadbin_tquadbin(fcinfo, &ever_eq_quadbin_tquadbin); }

PGDLLEXPORT Datum Ever_eq_tquadbin_quadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tquadbin_quadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_ever
 * @brief Return true if a temporal QUADBIN cell is ever equal to a QUADBIN cell
 * @sqlfn eEq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_tquadbin_quadbin(PG_FUNCTION_ARGS)
{ return EAcomp_tquadbin_quadbin(fcinfo, &ever_eq_tquadbin_quadbin); }

PGDLLEXPORT Datum Ever_eq_tquadbin_tquadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tquadbin_tquadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_ever
 * @brief Return true if two temporal QUADBIN cells are ever equal at a shared
 * instant
 * @sqlfn eEq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_tquadbin_tquadbin(PG_FUNCTION_ARGS)
{ return EAcomp_tquadbin_tquadbin(fcinfo, &ever_eq_tquadbin_tquadbin); }

/*****************************************************************************
 * Always equal
 *****************************************************************************/

PGDLLEXPORT Datum Always_eq_quadbin_tquadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_quadbin_tquadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_ever
 * @brief Return true if a QUADBIN cell is always equal to a temporal QUADBIN cell
 * @sqlfn aEq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_quadbin_tquadbin(PG_FUNCTION_ARGS)
{ return EAcomp_quadbin_tquadbin(fcinfo, &always_eq_quadbin_tquadbin); }

PGDLLEXPORT Datum Always_eq_tquadbin_quadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tquadbin_quadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_ever
 * @brief Return true if a temporal QUADBIN cell is always equal to a QUADBIN cell
 * @sqlfn aEq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_tquadbin_quadbin(PG_FUNCTION_ARGS)
{ return EAcomp_tquadbin_quadbin(fcinfo, &always_eq_tquadbin_quadbin); }

PGDLLEXPORT Datum Always_eq_tquadbin_tquadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tquadbin_tquadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_ever
 * @brief Return true if two temporal QUADBIN cells are always equal across
 * their shared time axis
 * @sqlfn aEq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_tquadbin_tquadbin(PG_FUNCTION_ARGS)
{ return EAcomp_tquadbin_tquadbin(fcinfo, &always_eq_tquadbin_tquadbin); }

/*****************************************************************************
 * Ever not equal
 *****************************************************************************/

PGDLLEXPORT Datum Ever_ne_quadbin_tquadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_quadbin_tquadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_ever
 * @brief Return true if a QUADBIN cell is ever different from a temporal QUADBIN cell
 * @sqlfn eNe()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_quadbin_tquadbin(PG_FUNCTION_ARGS)
{ return EAcomp_quadbin_tquadbin(fcinfo, &ever_ne_quadbin_tquadbin); }

PGDLLEXPORT Datum Ever_ne_tquadbin_quadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tquadbin_quadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_ever
 * @brief Return true if a temporal QUADBIN cell is ever different from a QUADBIN cell
 * @sqlfn eNe()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_tquadbin_quadbin(PG_FUNCTION_ARGS)
{ return EAcomp_tquadbin_quadbin(fcinfo, &ever_ne_tquadbin_quadbin); }

PGDLLEXPORT Datum Ever_ne_tquadbin_tquadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tquadbin_tquadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_ever
 * @brief Return true if two temporal QUADBIN cells are ever different at a shared
 * instant
 * @sqlfn eNe()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_tquadbin_tquadbin(PG_FUNCTION_ARGS)
{ return EAcomp_tquadbin_tquadbin(fcinfo, &ever_ne_tquadbin_tquadbin); }

/*****************************************************************************
 * Always not equal
 *****************************************************************************/

PGDLLEXPORT Datum Always_ne_quadbin_tquadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_quadbin_tquadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_ever
 * @brief Return true if a QUADBIN cell is always different from a temporal QUADBIN cell
 * @sqlfn aNe()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_quadbin_tquadbin(PG_FUNCTION_ARGS)
{ return EAcomp_quadbin_tquadbin(fcinfo, &always_ne_quadbin_tquadbin); }

PGDLLEXPORT Datum Always_ne_tquadbin_quadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tquadbin_quadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_ever
 * @brief Return true if a temporal QUADBIN cell is always different from a QUADBIN cell
 * @sqlfn aNe()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_tquadbin_quadbin(PG_FUNCTION_ARGS)
{ return EAcomp_tquadbin_quadbin(fcinfo, &always_ne_tquadbin_quadbin); }

PGDLLEXPORT Datum Always_ne_tquadbin_tquadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tquadbin_tquadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_ever
 * @brief Return true if two temporal QUADBIN cells are always different across
 * their shared time axis
 * @sqlfn aNe()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_tquadbin_tquadbin(PG_FUNCTION_ARGS)
{ return EAcomp_tquadbin_tquadbin(fcinfo, &always_ne_tquadbin_tquadbin); }

/*****************************************************************************
 * Temporal equal
 *****************************************************************************/

PGDLLEXPORT Datum Teq_quadbin_tquadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_quadbin_tquadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_temp
 * @brief Return the temporal equality of a QUADBIN cell and a temporal QUADBIN cell
 * @sqlfn tEq()
 * @sqlop @p #=
 */
inline Datum
Teq_quadbin_tquadbin(PG_FUNCTION_ARGS)
{ return Tcomp_quadbin_tquadbin(fcinfo, &teq_quadbin_tquadbin); }

PGDLLEXPORT Datum Teq_tquadbin_quadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tquadbin_quadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_temp
 * @brief Return the temporal equality of a temporal QUADBIN cell and a QUADBIN cell
 * @sqlfn tEq()
 * @sqlop @p #=
 */
inline Datum
Teq_tquadbin_quadbin(PG_FUNCTION_ARGS)
{ return Tcomp_tquadbin_quadbin(fcinfo, &teq_tquadbin_quadbin); }

PGDLLEXPORT Datum Teq_tquadbin_tquadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tquadbin_tquadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_temp
 * @brief Return the temporal equality of two temporal QUADBIN cells across their
 * shared time axis
 * @sqlfn tEq()
 * @sqlop @p #=
 */
inline Datum
Teq_tquadbin_tquadbin(PG_FUNCTION_ARGS)
{ return Tcomp_tquadbin_tquadbin(fcinfo, &teq_tquadbin_tquadbin); }

/*****************************************************************************
 * Temporal not equal
 *****************************************************************************/

PGDLLEXPORT Datum Tne_quadbin_tquadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_quadbin_tquadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_temp
 * @brief Return the temporal inequality of a QUADBIN cell and a temporal QUADBIN cell
 * @sqlfn tNe()
 * @sqlop @p #<>
 */
inline Datum
Tne_quadbin_tquadbin(PG_FUNCTION_ARGS)
{ return Tcomp_quadbin_tquadbin(fcinfo, &tne_quadbin_tquadbin); }

PGDLLEXPORT Datum Tne_tquadbin_quadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tquadbin_quadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_temp
 * @brief Return the temporal inequality of a temporal QUADBIN cell and a QUADBIN cell
 * @sqlfn tNe()
 * @sqlop @p #<>
 */
inline Datum
Tne_tquadbin_quadbin(PG_FUNCTION_ARGS)
{ return Tcomp_tquadbin_quadbin(fcinfo, &tne_tquadbin_quadbin); }

PGDLLEXPORT Datum Tne_tquadbin_tquadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tquadbin_tquadbin);
/**
 * @ingroup mobilitydb_quadbin_comp_temp
 * @brief Return the temporal inequality of two temporal QUADBIN cells across
 * their shared time axis
 * @sqlfn tNe()
 * @sqlop @p #<>
 */
inline Datum
Tne_tquadbin_tquadbin(PG_FUNCTION_ARGS)
{ return Tcomp_tquadbin_tquadbin(fcinfo, &tne_tquadbin_tquadbin); }

/*****************************************************************************/
