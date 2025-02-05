/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Ever/always and temporal comparison functions for temporal points
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
#include "general/temporal.h"
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
EAcomp_geo_tgeo(FunctionCallInfo fcinfo,
  int (*func)(const GSERIALIZED *, const Temporal *))
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = func(gs, temp);
  PG_FREE_IF_COPY(gs, 0);
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
EAcomp_tgeo_geo(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const GSERIALIZED *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  int result = func(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
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
EAcomp_tgeo_tgeo(FunctionCallInfo fcinfo,
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

PGDLLEXPORT Datum Ever_eq_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_geo_tgeo);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is ever equal to a point
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
Datum
Ever_eq_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EAcomp_geo_tgeo(fcinfo, &ever_eq_geo_tgeo);
}

PGDLLEXPORT Datum Always_eq_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_geo_tgeo);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is always equal to a point
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
Datum
Always_eq_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EAcomp_geo_tgeo(fcinfo, &always_eq_geo_tgeo);
}

PGDLLEXPORT Datum Ever_ne_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_geo_tgeo);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is ever different from a point
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
Datum
Ever_ne_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EAcomp_geo_tgeo(fcinfo, &ever_ne_geo_tgeo);
}

PGDLLEXPORT Datum Always_ne_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_geo_tgeo);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is always different from a point
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
Datum
Always_ne_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EAcomp_geo_tgeo(fcinfo, &always_ne_geo_tgeo);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tgeo_geo);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is ever equal to a point
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
Datum
Ever_eq_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EAcomp_tgeo_geo(fcinfo, &ever_eq_tgeo_geo);
}

PGDLLEXPORT Datum Always_eq_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tgeo_geo);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is always equal to a point
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
Datum
Always_eq_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EAcomp_tgeo_geo(fcinfo, &always_eq_tgeo_geo);
}

PGDLLEXPORT Datum Ever_ne_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tgeo_geo);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is ever different from a point
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
Datum
Ever_ne_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EAcomp_tgeo_geo(fcinfo, &ever_ne_tgeo_geo);
}

PGDLLEXPORT Datum Always_ne_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tgeo_geo);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is always different from a point
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
Datum
Always_ne_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EAcomp_tgeo_geo(fcinfo, &always_ne_tgeo_geo);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tgeo_tgeo);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if two temporal points are ever equal
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
Datum
Ever_eq_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EAcomp_tgeo_tgeo(fcinfo, &ever_eq_tgeo_tgeo);
}

PGDLLEXPORT Datum Always_eq_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tgeo_tgeo);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if two temporal points are always equal
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
Datum
Always_eq_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EAcomp_tgeo_tgeo(fcinfo, &always_eq_tgeo_tgeo);
}

PGDLLEXPORT Datum Ever_ne_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tgeo_tgeo);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if two temporal points are ever different
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
Datum
Ever_ne_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EAcomp_tgeo_tgeo(fcinfo, &ever_ne_tgeo_tgeo);
}

PGDLLEXPORT Datum Always_ne_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tgeo_tgeo);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if two temporal points are always different
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
Datum
Always_ne_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EAcomp_tgeo_tgeo(fcinfo, &always_ne_tgeo_tgeo);
}

/*****************************************************************************
 * Temporal comparison functions
 *****************************************************************************/

/**
 * @brief Generic function for the temporal ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
Tcomp_geo_tgeo(FunctionCallInfo fcinfo,
  Temporal * (*func)(const GSERIALIZED *, const Temporal *))
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = func(gs, temp);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Generic function for the temporal ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
Tcomp_tgeo_geo(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const GSERIALIZED *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = func(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Generic function for the temporal ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
Tcomp_tgeo_tgeo(FunctionCallInfo fcinfo,
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

PGDLLEXPORT Datum Teq_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_geo_tgeo);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return true if a temporal point is ever equal to a point
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
Datum
Teq_geo_tgeo(PG_FUNCTION_ARGS)
{
  return Tcomp_geo_tgeo(fcinfo, &teq_geo_tgeo);
}

PGDLLEXPORT Datum Tne_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_geo_tgeo);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return true if a temporal point is ever different from a point
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
Datum
Tne_geo_tgeo(PG_FUNCTION_ARGS)
{
  return Tcomp_geo_tgeo(fcinfo, &tne_geo_tgeo);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tgeo_geo);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return true if a temporal point is ever equal to a point
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
Datum
Teq_tgeo_geo(PG_FUNCTION_ARGS)
{
  return Tcomp_tgeo_geo(fcinfo, &teq_tgeo_geo);
}

PGDLLEXPORT Datum Tne_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tgeo_geo);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return true if a temporal point is ever different from a point
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
Datum
Tne_tgeo_geo(PG_FUNCTION_ARGS)
{
  return Tcomp_tgeo_geo(fcinfo, &tne_tgeo_geo);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tgeo_tgeo);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return true if two temporal points are ever equal
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
Datum
Teq_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return Tcomp_tgeo_tgeo(fcinfo, &teq_temporal_temporal);
}

PGDLLEXPORT Datum Tne_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tgeo_tgeo);
/**
 * @ingroup mobilitydb_temporal_comp_temp
 * @brief Return true if two temporal points are ever different
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
Datum
Tne_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return Tcomp_tgeo_tgeo(fcinfo, &tne_temporal_temporal);
}

/*****************************************************************************/
