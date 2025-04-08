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
 * @brief Ever/always and temporal comparisons for temporal geos
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_npoint.h>
#include "temporal/temporal.h"
#include "npoint/tnpoint.h"
/* MobilityDB */
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Ever/always comparison functions
 *****************************************************************************/

/**
 * @brief Generic function for the temporal ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
EAcomp_npoint_tnpoint(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const Npoint *))
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = func(temp, np);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic function for the temporal ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
EAcomp_tnpoint_npoint(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const Npoint *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  int result = func(temp, np);
  PG_FREE_IF_COPY(temp, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic function for the temporal ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
EAcomp_tnpoint_tnpoint(FunctionCallInfo fcinfo,
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

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_npoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_comp_ever
 * @brief Return true if a temporal network point is ever equal to a network
 * point
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_npoint_tnpoint(fcinfo, &ever_eq_tnpoint_npoint);
}

PGDLLEXPORT Datum Always_eq_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_npoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_comp_ever
 * @brief Return true if a temporal network point is always equal to a network
 * point
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_npoint_tnpoint(fcinfo, &always_eq_tnpoint_npoint);
}

PGDLLEXPORT Datum Ever_ne_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_npoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_comp_ever
 * @brief Return true if a temporal network point is ever different from a
 * network point
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_npoint_tnpoint(fcinfo, &ever_ne_tnpoint_npoint);
}

PGDLLEXPORT Datum Always_ne_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_npoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_comp_ever
 * @brief Return true if a temporal network point is always different from a
 * network point
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_npoint_tnpoint(fcinfo, &always_ne_tnpoint_npoint);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tnpoint_npoint);
/**
 * @ingroup mobilitydb_npoint_comp_ever
 * @brief Return true if a temporal network point is ever equal to a network point
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tnpoint_npoint(fcinfo, &ever_eq_tnpoint_npoint);
}

PGDLLEXPORT Datum Always_eq_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tnpoint_npoint);
/**
 * @ingroup mobilitydb_npoint_comp_ever
 * @brief Return true if a temporal network point is always equal to a network point
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tnpoint_npoint(fcinfo, &always_eq_tnpoint_npoint);
}

PGDLLEXPORT Datum Ever_ne_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tnpoint_npoint);
/**
 * @ingroup mobilitydb_npoint_comp_ever
 * @brief Return true if a temporal network point is ever different from a
 * network point
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tnpoint_npoint(fcinfo, &ever_ne_tnpoint_npoint);
}

PGDLLEXPORT Datum Always_ne_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tnpoint_npoint);
/**
 * @ingroup mobilitydb_npoint_comp_ever
 * @brief Return true if a temporal network point is always different from a
 * network point
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tnpoint_npoint(fcinfo, &always_ne_tnpoint_npoint);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_comp_ever
 * @brief Return true if two temporal geos are ever equal
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tnpoint_tnpoint(fcinfo, &ever_eq_tnpoint_tnpoint);
}

PGDLLEXPORT Datum Always_eq_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_comp_ever
 * @brief Return true if two temporal geos are always equal
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tnpoint_tnpoint(fcinfo, &always_eq_tnpoint_tnpoint);
}

PGDLLEXPORT Datum Ever_ne_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_comp_ever
 * @brief Return true if two temporal geos are ever different
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tnpoint_tnpoint(fcinfo, &ever_ne_tnpoint_tnpoint);
}

PGDLLEXPORT Datum Always_ne_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_comp_ever
 * @brief Return true if two temporal geos are always different
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tnpoint_tnpoint(fcinfo, &always_ne_tnpoint_tnpoint);
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
Tcomp_npoint_tnpoint(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const Npoint *))
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = func(temp, np);
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
Tcomp_tnpoint_npoint(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const Npoint *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  Temporal *result = func(temp, np);
  PG_FREE_IF_COPY(temp, 0);
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
Tcomp_tnpoint_tnpoint(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const Temporal *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = func(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_npoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_comp_temp
 * @brief Return a temporal Boolean that states whether a temporal network
 * point is equal to a network point
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Tcomp_npoint_tnpoint(fcinfo, &teq_tnpoint_npoint);
}

PGDLLEXPORT Datum Tne_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_npoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_comp_temp
 * @brief Return a temporal Boolean that states whether a temporal network
 * point is different from a network point
 * network point
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Tcomp_npoint_tnpoint(fcinfo, &tne_tnpoint_npoint);
}

PGDLLEXPORT Datum Teq_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tnpoint_npoint);
/**
 * @ingroup mobilitydb_npoint_comp_temp
 * @brief Return a temporal Boolean that states whether a temporal network
 * point is equal to a network point
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return Tcomp_tnpoint_npoint(fcinfo, &teq_tnpoint_npoint);
}

PGDLLEXPORT Datum Tne_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tnpoint_npoint);
/**
 * @ingroup mobilitydb_npoint_comp_temp
 * @brief Return a temporal Boolean that states whether a temporal network
 * point is different from a network point
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return Tcomp_tnpoint_npoint(fcinfo, &tne_tnpoint_npoint);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_comp_temp
 * @brief Return a temporal Boolean that states whether two temporal geos are
 * equal
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Tcomp_tnpoint_tnpoint(fcinfo, &teq_temporal_temporal);
}

PGDLLEXPORT Datum Tne_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_comp_temp
 * @brief Return a temporal Boolean that states whether two temporal geos are
 * different
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Tcomp_tnpoint_tnpoint(fcinfo, &tne_temporal_temporal);
}

/*****************************************************************************/
