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
#include "pg_point/postgis.h"

/*****************************************************************************
 * Ever/always comparison functions
 *****************************************************************************/

/**
 * @brief Generic function for the temporal ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
EAcomp_point_tpoint(FunctionCallInfo fcinfo,
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
EAcomp_tpoint_point(FunctionCallInfo fcinfo,
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
EAcomp_tpoint_tpoint(FunctionCallInfo fcinfo,
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

PGDLLEXPORT Datum Ever_eq_point_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_point_tpoint);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is ever equal to a point
 * @sqlfn ever_eq()
 */
Datum
Ever_eq_point_tpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_point_tpoint(fcinfo, &ever_eq_point_tpoint);
}

PGDLLEXPORT Datum Always_eq_point_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_point_tpoint);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is always equal to a point
 * @sqlfn always_eq()
 */
Datum
Always_eq_point_tpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_point_tpoint(fcinfo, &always_eq_point_tpoint);
}

PGDLLEXPORT Datum Ever_ne_point_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_point_tpoint);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is ever different from a point
 * @sqlfn ever_ne()
 */
Datum
Ever_ne_point_tpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_point_tpoint(fcinfo, &always_ne_point_tpoint);
}

PGDLLEXPORT Datum Always_ne_point_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_point_tpoint);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is always different from a point
 * @sqlfn always_ne()
 */
Datum
Always_ne_point_tpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_point_tpoint(fcinfo, &always_ne_point_tpoint);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tpoint_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tpoint_point);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is ever equal to a point
 * @sqlfn ever_eq()
 */
Datum
Ever_eq_tpoint_point(PG_FUNCTION_ARGS)
{
  return EAcomp_tpoint_point(fcinfo, &ever_eq_tpoint_point);
}

PGDLLEXPORT Datum Always_eq_tpoint_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tpoint_point);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is always equal to a point
 * @sqlfn always_eq()
 */
Datum
Always_eq_tpoint_point(PG_FUNCTION_ARGS)
{
  return EAcomp_tpoint_point(fcinfo, &always_eq_tpoint_point);
}

PGDLLEXPORT Datum Ever_ne_tpoint_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tpoint_point);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is ever different from a point
 * @sqlfn ever_ne()
 */
Datum
Ever_ne_tpoint_point(PG_FUNCTION_ARGS)
{
  return EAcomp_tpoint_point(fcinfo, &ever_ne_tpoint_point);
}

PGDLLEXPORT Datum Always_ne_tpoint_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tpoint_point);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is always different from a point
 * @sqlfn always_ne()
 */
Datum
Always_ne_tpoint_point(PG_FUNCTION_ARGS)
{
  return EAcomp_tpoint_point(fcinfo, &always_ne_tpoint_point);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if two temporal points are ever equal
 * @sqlfn ever_eq()
 */
Datum
Ever_eq_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tpoint_tpoint(fcinfo, &ever_eq_tpoint_tpoint);
}

PGDLLEXPORT Datum Always_eq_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if two temporal points are always equal
 * @sqlfn always_eq()
 */
Datum
Always_eq_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tpoint_tpoint(fcinfo, &always_eq_tpoint_tpoint);
}

PGDLLEXPORT Datum Ever_ne_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if two temporal points are ever different
 * @sqlfn ever_ne()
 */
Datum
Ever_ne_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tpoint_tpoint(fcinfo, &ever_ne_tpoint_tpoint);
}

PGDLLEXPORT Datum Always_ne_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if two temporal points are always different
 * @sqlfn always_ne()
 */
Datum
Always_ne_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tpoint_tpoint(fcinfo, &always_ne_tpoint_tpoint);
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
Tcomp_point_tpoint(FunctionCallInfo fcinfo,
  Temporal * (*func)(const GSERIALIZED *, const Temporal *))
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = func(gs, temp);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * @brief Generic function for the temporal ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
Tcomp_tpoint_point(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const GSERIALIZED *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = func(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * @brief Generic function for the temporal ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
Tcomp_tpoint_tpoint(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const Temporal *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = func(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_point_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_point_tpoint);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is ever equal to a point
 * @sqlfn teq()
 */
Datum
Teq_point_tpoint(PG_FUNCTION_ARGS)
{
  return Tcomp_point_tpoint(fcinfo, &teq_point_tpoint);
}

PGDLLEXPORT Datum Tne_point_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_point_tpoint);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is ever different from a point
 * @sqlfn tne()
 */
Datum
Tne_point_tpoint(PG_FUNCTION_ARGS)
{
  return Tcomp_point_tpoint(fcinfo, &tne_point_tpoint);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_tpoint_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tpoint_point);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is ever equal to a point
 * @sqlfn teq()
 */
Datum
Teq_tpoint_point(PG_FUNCTION_ARGS)
{
  return Tcomp_tpoint_point(fcinfo, &teq_tpoint_point);
}

PGDLLEXPORT Datum Tne_tpoint_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tpoint_point);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal point is ever different from a point
 * @sqlfn tne()
 */
Datum
Tne_tpoint_point(PG_FUNCTION_ARGS)
{
  return Tcomp_tpoint_point(fcinfo, &tne_tpoint_point);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if two temporal points are ever equal
 * @sqlfn teq()
 */
Datum
Teq_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return Tcomp_tpoint_tpoint(fcinfo, &teq_temporal_temporal);
}

PGDLLEXPORT Datum Tne_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if two temporal points are ever different
 * @sqlfn tne()
 */
Datum
Tne_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return Tcomp_tpoint_tpoint(fcinfo, &tne_temporal_temporal);
}

/*****************************************************************************/
