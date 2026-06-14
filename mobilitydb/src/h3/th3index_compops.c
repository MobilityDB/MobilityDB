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
 * @brief PG V1 wrappers for the th3index comparison operators.
 *
 * Each wrapper unpacks an H3Index from a bigint Datum (the SQL
 * surface for bare cells) or a Temporal from a TOAST datum, calls
 * the th3index-specific MEOS function declared in `meos_h3.h`, and
 * returns the result.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_h3.h>
#include "temporal/temporal.h"
#include "h3/h3index.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"

/* PG_GETARG_H3INDEX / PG_RETURN_H3INDEX are defined locally in
 * `h3index.c`; replicate here so we can use them in the comparison
 * wrappers without cross-file include. */
#define PG_GETARG_H3INDEX(n) DatumGetH3Index(PG_GETARG_DATUM(n))

/*****************************************************************************
 * Shared helpers
 *****************************************************************************/

static Datum
EAcomp_h3index_th3index(FunctionCallInfo fcinfo,
  int (*func)(H3Index, const Temporal *))
{
  H3Index cell = PG_GETARG_H3INDEX(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = func(cell, temp);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

static Datum
EAcomp_th3index_h3index(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, H3Index))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  H3Index cell = PG_GETARG_H3INDEX(1);
  int result = func(temp, cell);
  PG_FREE_IF_COPY(temp, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

static Datum
EAcomp_th3index_th3index(FunctionCallInfo fcinfo,
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
Tcomp_h3index_th3index(FunctionCallInfo fcinfo,
  Temporal *(*func)(H3Index, const Temporal *))
{
  H3Index cell = PG_GETARG_H3INDEX(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = func(cell, temp);
  PG_FREE_IF_COPY(temp, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

static Datum
Tcomp_th3index_h3index(FunctionCallInfo fcinfo,
  Temporal *(*func)(const Temporal *, H3Index))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  H3Index cell = PG_GETARG_H3INDEX(1);
  Temporal *result = func(temp, cell);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

static Datum
Tcomp_th3index_th3index(FunctionCallInfo fcinfo,
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

PGDLLEXPORT Datum Ever_eq_h3index_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_h3index_th3index);
/**
 * @ingroup mobilitydb_h3_comp_ever
 * @brief Return true if an H3 cell is ever equal to a temporal H3 cell
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_h3index_th3index(PG_FUNCTION_ARGS)
{ return EAcomp_h3index_th3index(fcinfo, &ever_eq_h3index_th3index); }

PGDLLEXPORT Datum Ever_eq_th3index_h3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_th3index_h3index);
/**
 * @ingroup mobilitydb_h3_comp_ever
 * @brief Return true if a temporal H3 cell is ever equal to an H3 cell
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_th3index_h3index(PG_FUNCTION_ARGS)
{ return EAcomp_th3index_h3index(fcinfo, &ever_eq_th3index_h3index); }

PGDLLEXPORT Datum Ever_eq_th3index_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_th3index_th3index);
/**
 * @ingroup mobilitydb_h3_comp_ever
 * @brief Return true if two temporal H3 cells are ever equal at a shared
 * instant
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_th3index_th3index(PG_FUNCTION_ARGS)
{ return EAcomp_th3index_th3index(fcinfo, &ever_eq_th3index_th3index); }

/*****************************************************************************
 * Always equal
 *****************************************************************************/

PGDLLEXPORT Datum Always_eq_h3index_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_h3index_th3index);
/**
 * @ingroup mobilitydb_h3_comp_ever
 * @brief Return true if an H3 cell is always equal to a temporal H3 cell
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_h3index_th3index(PG_FUNCTION_ARGS)
{ return EAcomp_h3index_th3index(fcinfo, &always_eq_h3index_th3index); }

PGDLLEXPORT Datum Always_eq_th3index_h3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_th3index_h3index);
/**
 * @ingroup mobilitydb_h3_comp_ever
 * @brief Return true if a temporal H3 cell is always equal to an H3 cell
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_th3index_h3index(PG_FUNCTION_ARGS)
{ return EAcomp_th3index_h3index(fcinfo, &always_eq_th3index_h3index); }

PGDLLEXPORT Datum Always_eq_th3index_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_th3index_th3index);
/**
 * @ingroup mobilitydb_h3_comp_ever
 * @brief Return true if two temporal H3 cells are always equal across
 * their shared time axis
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_th3index_th3index(PG_FUNCTION_ARGS)
{ return EAcomp_th3index_th3index(fcinfo, &always_eq_th3index_th3index); }

/*****************************************************************************
 * Ever not equal
 *****************************************************************************/

PGDLLEXPORT Datum Ever_ne_h3index_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_h3index_th3index);
/**
 * @ingroup mobilitydb_h3_comp_ever
 * @brief Return true if an H3 cell is ever different from a temporal H3 cell
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_h3index_th3index(PG_FUNCTION_ARGS)
{ return EAcomp_h3index_th3index(fcinfo, &ever_ne_h3index_th3index); }

PGDLLEXPORT Datum Ever_ne_th3index_h3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_th3index_h3index);
/**
 * @ingroup mobilitydb_h3_comp_ever
 * @brief Return true if a temporal H3 cell is ever different from an H3 cell
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_th3index_h3index(PG_FUNCTION_ARGS)
{ return EAcomp_th3index_h3index(fcinfo, &ever_ne_th3index_h3index); }

PGDLLEXPORT Datum Ever_ne_th3index_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_th3index_th3index);
/**
 * @ingroup mobilitydb_h3_comp_ever
 * @brief Return true if two temporal H3 cells are ever different at a shared
 * instant
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_th3index_th3index(PG_FUNCTION_ARGS)
{ return EAcomp_th3index_th3index(fcinfo, &ever_ne_th3index_th3index); }

/*****************************************************************************
 * Always not equal
 *****************************************************************************/

PGDLLEXPORT Datum Always_ne_h3index_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_h3index_th3index);
/**
 * @ingroup mobilitydb_h3_comp_ever
 * @brief Return true if an H3 cell is always different from a temporal H3 cell
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_h3index_th3index(PG_FUNCTION_ARGS)
{ return EAcomp_h3index_th3index(fcinfo, &always_ne_h3index_th3index); }

PGDLLEXPORT Datum Always_ne_th3index_h3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_th3index_h3index);
/**
 * @ingroup mobilitydb_h3_comp_ever
 * @brief Return true if a temporal H3 cell is always different from an H3 cell
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_th3index_h3index(PG_FUNCTION_ARGS)
{ return EAcomp_th3index_h3index(fcinfo, &always_ne_th3index_h3index); }

PGDLLEXPORT Datum Always_ne_th3index_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_th3index_th3index);
/**
 * @ingroup mobilitydb_h3_comp_ever
 * @brief Return true if two temporal H3 cells are always different across
 * their shared time axis
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_th3index_th3index(PG_FUNCTION_ARGS)
{ return EAcomp_th3index_th3index(fcinfo, &always_ne_th3index_th3index); }

/*****************************************************************************
 * Temporal equal
 *****************************************************************************/

PGDLLEXPORT Datum Teq_h3index_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_h3index_th3index);
/**
 * @ingroup mobilitydb_h3_comp_temp
 * @brief Return the temporal equality of an H3 cell and a temporal H3 cell
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_h3index_th3index(PG_FUNCTION_ARGS)
{ return Tcomp_h3index_th3index(fcinfo, &teq_h3index_th3index); }

PGDLLEXPORT Datum Teq_th3index_h3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_th3index_h3index);
/**
 * @ingroup mobilitydb_h3_comp_temp
 * @brief Return the temporal equality of a temporal H3 cell and an H3 cell
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_th3index_h3index(PG_FUNCTION_ARGS)
{ return Tcomp_th3index_h3index(fcinfo, &teq_th3index_h3index); }

PGDLLEXPORT Datum Teq_th3index_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_th3index_th3index);
/**
 * @ingroup mobilitydb_h3_comp_temp
 * @brief Return the temporal equality of two temporal H3 cells across their
 * shared time axis
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_th3index_th3index(PG_FUNCTION_ARGS)
{ return Tcomp_th3index_th3index(fcinfo, &teq_th3index_th3index); }

/*****************************************************************************
 * Temporal not equal
 *****************************************************************************/

PGDLLEXPORT Datum Tne_h3index_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_h3index_th3index);
/**
 * @ingroup mobilitydb_h3_comp_temp
 * @brief Return the temporal inequality of an H3 cell and a temporal H3 cell
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_h3index_th3index(PG_FUNCTION_ARGS)
{ return Tcomp_h3index_th3index(fcinfo, &tne_h3index_th3index); }

PGDLLEXPORT Datum Tne_th3index_h3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_th3index_h3index);
/**
 * @ingroup mobilitydb_h3_comp_temp
 * @brief Return the temporal inequality of a temporal H3 cell and an H3 cell
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_th3index_h3index(PG_FUNCTION_ARGS)
{ return Tcomp_th3index_h3index(fcinfo, &tne_th3index_h3index); }

PGDLLEXPORT Datum Tne_th3index_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_th3index_th3index);
/**
 * @ingroup mobilitydb_h3_comp_temp
 * @brief Return the temporal inequality of two temporal H3 cells across
 * their shared time axis
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_th3index_th3index(PG_FUNCTION_ARGS)
{ return Tcomp_th3index_th3index(fcinfo, &tne_th3index_th3index); }

/*****************************************************************************/
