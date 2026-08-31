/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief PG V1 wrappers for the ts2cell comparison operators.
 *
 * Each wrapper unpacks an S2 cell identifier or a Temporal from its
 * Datum, calls the ts2cell-specific MEOS function declared in
 * `meos_s2cell.h`, and returns the result.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_s2cell.h>
#include "temporal/temporal.h"
#include "s2cell/s2cell.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"

/*****************************************************************************
 * Shared helpers
 *****************************************************************************/

static Datum
EAcomp_s2cell_ts2cell(FunctionCallInfo fcinfo,
  int (*func)(S2CellId, const Temporal *))
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = func(cell, temp);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

static Datum
EAcomp_ts2cell_s2cell(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, S2CellId))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  S2CellId cell = PG_GETARG_S2CELL(1);
  int result = func(temp, cell);
  PG_FREE_IF_COPY(temp, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

static Datum
EAcomp_ts2cell_ts2cell(FunctionCallInfo fcinfo,
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
Tcomp_s2cell_ts2cell(FunctionCallInfo fcinfo,
  Temporal *(*func)(S2CellId, const Temporal *))
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = func(cell, temp);
  PG_FREE_IF_COPY(temp, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

static Datum
Tcomp_ts2cell_s2cell(FunctionCallInfo fcinfo,
  Temporal *(*func)(const Temporal *, S2CellId))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  S2CellId cell = PG_GETARG_S2CELL(1);
  Temporal *result = func(temp, cell);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

static Datum
Tcomp_ts2cell_ts2cell(FunctionCallInfo fcinfo,
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

PGDLLEXPORT Datum Ever_eq_s2cell_ts2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_s2cell_ts2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_ever
 * @brief Return true if a S2CELL cell is ever equal to a temporal S2CELL cell
 * @sqlfn eEq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_s2cell_ts2cell(PG_FUNCTION_ARGS)
{ return EAcomp_s2cell_ts2cell(fcinfo, &ever_eq_s2cell_ts2cell); }

PGDLLEXPORT Datum Ever_eq_ts2cell_s2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_ts2cell_s2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_ever
 * @brief Return true if a temporal S2CELL cell is ever equal to a S2CELL cell
 * @sqlfn eEq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_ts2cell_s2cell(PG_FUNCTION_ARGS)
{ return EAcomp_ts2cell_s2cell(fcinfo, &ever_eq_ts2cell_s2cell); }

PGDLLEXPORT Datum Ever_eq_ts2cell_ts2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_ts2cell_ts2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_ever
 * @brief Return true if two temporal S2CELL cells are ever equal at a shared
 * instant
 * @sqlfn eEq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_ts2cell_ts2cell(PG_FUNCTION_ARGS)
{ return EAcomp_ts2cell_ts2cell(fcinfo, &ever_eq_ts2cell_ts2cell); }

/*****************************************************************************
 * Always equal
 *****************************************************************************/

PGDLLEXPORT Datum Always_eq_s2cell_ts2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_s2cell_ts2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_ever
 * @brief Return true if a S2CELL cell is always equal to a temporal S2CELL cell
 * @sqlfn aEq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_s2cell_ts2cell(PG_FUNCTION_ARGS)
{ return EAcomp_s2cell_ts2cell(fcinfo, &always_eq_s2cell_ts2cell); }

PGDLLEXPORT Datum Always_eq_ts2cell_s2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_ts2cell_s2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_ever
 * @brief Return true if a temporal S2CELL cell is always equal to a S2CELL cell
 * @sqlfn aEq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_ts2cell_s2cell(PG_FUNCTION_ARGS)
{ return EAcomp_ts2cell_s2cell(fcinfo, &always_eq_ts2cell_s2cell); }

PGDLLEXPORT Datum Always_eq_ts2cell_ts2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_ts2cell_ts2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_ever
 * @brief Return true if two temporal S2CELL cells are always equal across
 * their shared time axis
 * @sqlfn aEq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_ts2cell_ts2cell(PG_FUNCTION_ARGS)
{ return EAcomp_ts2cell_ts2cell(fcinfo, &always_eq_ts2cell_ts2cell); }

/*****************************************************************************
 * Ever not equal
 *****************************************************************************/

PGDLLEXPORT Datum Ever_ne_s2cell_ts2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_s2cell_ts2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_ever
 * @brief Return true if a S2CELL cell is ever different from a temporal S2CELL cell
 * @sqlfn eNe()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_s2cell_ts2cell(PG_FUNCTION_ARGS)
{ return EAcomp_s2cell_ts2cell(fcinfo, &ever_ne_s2cell_ts2cell); }

PGDLLEXPORT Datum Ever_ne_ts2cell_s2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_ts2cell_s2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_ever
 * @brief Return true if a temporal S2CELL cell is ever different from a S2CELL cell
 * @sqlfn eNe()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_ts2cell_s2cell(PG_FUNCTION_ARGS)
{ return EAcomp_ts2cell_s2cell(fcinfo, &ever_ne_ts2cell_s2cell); }

PGDLLEXPORT Datum Ever_ne_ts2cell_ts2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_ts2cell_ts2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_ever
 * @brief Return true if two temporal S2CELL cells are ever different at a shared
 * instant
 * @sqlfn eNe()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_ts2cell_ts2cell(PG_FUNCTION_ARGS)
{ return EAcomp_ts2cell_ts2cell(fcinfo, &ever_ne_ts2cell_ts2cell); }

/*****************************************************************************
 * Always not equal
 *****************************************************************************/

PGDLLEXPORT Datum Always_ne_s2cell_ts2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_s2cell_ts2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_ever
 * @brief Return true if a S2CELL cell is always different from a temporal S2CELL cell
 * @sqlfn aNe()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_s2cell_ts2cell(PG_FUNCTION_ARGS)
{ return EAcomp_s2cell_ts2cell(fcinfo, &always_ne_s2cell_ts2cell); }

PGDLLEXPORT Datum Always_ne_ts2cell_s2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_ts2cell_s2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_ever
 * @brief Return true if a temporal S2CELL cell is always different from a S2CELL cell
 * @sqlfn aNe()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_ts2cell_s2cell(PG_FUNCTION_ARGS)
{ return EAcomp_ts2cell_s2cell(fcinfo, &always_ne_ts2cell_s2cell); }

PGDLLEXPORT Datum Always_ne_ts2cell_ts2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_ts2cell_ts2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_ever
 * @brief Return true if two temporal S2CELL cells are always different across
 * their shared time axis
 * @sqlfn aNe()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_ts2cell_ts2cell(PG_FUNCTION_ARGS)
{ return EAcomp_ts2cell_ts2cell(fcinfo, &always_ne_ts2cell_ts2cell); }

/*****************************************************************************
 * Temporal equal
 *****************************************************************************/

PGDLLEXPORT Datum Teq_s2cell_ts2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_s2cell_ts2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_temp
 * @brief Return the temporal equality of a S2CELL cell and a temporal S2CELL cell
 * @sqlfn tEq()
 * @sqlop @p #=
 */
inline Datum
Teq_s2cell_ts2cell(PG_FUNCTION_ARGS)
{ return Tcomp_s2cell_ts2cell(fcinfo, &teq_s2cell_ts2cell); }

PGDLLEXPORT Datum Teq_ts2cell_s2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_ts2cell_s2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_temp
 * @brief Return the temporal equality of a temporal S2CELL cell and a S2CELL cell
 * @sqlfn tEq()
 * @sqlop @p #=
 */
inline Datum
Teq_ts2cell_s2cell(PG_FUNCTION_ARGS)
{ return Tcomp_ts2cell_s2cell(fcinfo, &teq_ts2cell_s2cell); }

PGDLLEXPORT Datum Teq_ts2cell_ts2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_ts2cell_ts2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_temp
 * @brief Return the temporal equality of two temporal S2CELL cells across their
 * shared time axis
 * @sqlfn tEq()
 * @sqlop @p #=
 */
inline Datum
Teq_ts2cell_ts2cell(PG_FUNCTION_ARGS)
{ return Tcomp_ts2cell_ts2cell(fcinfo, &teq_ts2cell_ts2cell); }

/*****************************************************************************
 * Temporal not equal
 *****************************************************************************/

PGDLLEXPORT Datum Tne_s2cell_ts2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_s2cell_ts2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_temp
 * @brief Return the temporal inequality of a S2CELL cell and a temporal S2CELL cell
 * @sqlfn tNe()
 * @sqlop @p #<>
 */
inline Datum
Tne_s2cell_ts2cell(PG_FUNCTION_ARGS)
{ return Tcomp_s2cell_ts2cell(fcinfo, &tne_s2cell_ts2cell); }

PGDLLEXPORT Datum Tne_ts2cell_s2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_ts2cell_s2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_temp
 * @brief Return the temporal inequality of a temporal S2CELL cell and a S2CELL cell
 * @sqlfn tNe()
 * @sqlop @p #<>
 */
inline Datum
Tne_ts2cell_s2cell(PG_FUNCTION_ARGS)
{ return Tcomp_ts2cell_s2cell(fcinfo, &tne_ts2cell_s2cell); }

PGDLLEXPORT Datum Tne_ts2cell_ts2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_ts2cell_ts2cell);
/**
 * @ingroup mobilitydb_s2cell_comp_temp
 * @brief Return the temporal inequality of two temporal S2CELL cells across
 * their shared time axis
 * @sqlfn tNe()
 * @sqlop @p #<>
 */
inline Datum
Tne_ts2cell_ts2cell(PG_FUNCTION_ARGS)
{ return Tcomp_ts2cell_ts2cell(fcinfo, &tne_ts2cell_ts2cell); }

/*****************************************************************************/
