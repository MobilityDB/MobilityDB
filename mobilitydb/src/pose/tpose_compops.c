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
 * @brief Ever/always and temporal comparisons for temporal poses
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
#include <meos_pose.h>
#include "general/temporal.h"
#include "pose/tpose.h"
/* MobilityDB */
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Ever/always comparison functions
 *****************************************************************************/

/**
 * @brief Generic function for the ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
EAcomp_pose_tpose(FunctionCallInfo fcinfo,
  int (*func)(const Pose *, const Temporal *))
{
  Pose *pose = PG_GETARG_POSE_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = func(pose, temp);
  PG_FREE_IF_COPY(pose, 0);
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
EAcomp_tpose_pose(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const Pose *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Pose *pose = PG_GETARG_POSE_P(1);
  int result = func(temp, pose);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(pose, 1);
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
EAcomp_tpose_tpose(FunctionCallInfo fcinfo,
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

PGDLLEXPORT Datum Ever_eq_pose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_pose_tpose);
/**
 * @ingroup mobilitydb_pose_comp_ever
 * @brief Return true if a temporal pose is ever equal to a pose
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_pose_tpose(PG_FUNCTION_ARGS)
{
  return EAcomp_pose_tpose(fcinfo, &ever_eq_pose_tpose);
}

PGDLLEXPORT Datum Always_eq_pose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_pose_tpose);
/**
 * @ingroup mobilitydb_pose_comp_ever
 * @brief Return true if a temporal pose is always equal to a
 * pose
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_pose_tpose(PG_FUNCTION_ARGS)
{
  return EAcomp_pose_tpose(fcinfo, &always_eq_pose_tpose);
}

PGDLLEXPORT Datum Ever_ne_pose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_pose_tpose);
/**
 * @ingroup mobilitydb_pose_comp_ever
 * @brief Return true if a temporal pose is ever different from a
 * pose
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_pose_tpose(PG_FUNCTION_ARGS)
{
  return EAcomp_pose_tpose(fcinfo, &ever_ne_pose_tpose);
}

PGDLLEXPORT Datum Always_ne_pose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_pose_tpose);
/**
 * @ingroup mobilitydb_pose_comp_ever
 * @brief Return true if a temporal pose is always different from a
 * pose
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_pose_tpose(PG_FUNCTION_ARGS)
{
  return EAcomp_pose_tpose(fcinfo, &always_ne_pose_tpose);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tpose_pose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tpose_pose);
/**
 * @ingroup mobilitydb_pose_comp_ever
 * @brief Return true if a temporal pose is ever equal to a pose
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_tpose_pose(PG_FUNCTION_ARGS)
{
  return EAcomp_tpose_pose(fcinfo, &ever_eq_tpose_pose);
}

PGDLLEXPORT Datum Always_eq_tpose_pose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tpose_pose);
/**
 * @ingroup mobilitydb_pose_comp_ever
 * @brief Return true if a temporal pose is always equal to a
 * pose
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_tpose_pose(PG_FUNCTION_ARGS)
{
  return EAcomp_tpose_pose(fcinfo, &always_eq_tpose_pose);
}

PGDLLEXPORT Datum Ever_ne_tpose_pose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tpose_pose);
/**
 * @ingroup mobilitydb_pose_comp_ever
 * @brief Return true if a temporal pose is ever different from a
 * pose
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_tpose_pose(PG_FUNCTION_ARGS)
{
  return EAcomp_tpose_pose(fcinfo, &ever_ne_tpose_pose);
}

PGDLLEXPORT Datum Always_ne_tpose_pose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tpose_pose);
/**
 * @ingroup mobilitydb_pose_comp_ever
 * @brief Return true if a temporal pose is always different from a
 * pose
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_tpose_pose(PG_FUNCTION_ARGS)
{
  return EAcomp_tpose_pose(fcinfo, &always_ne_tpose_pose);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tpose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tpose_tpose);
/**
 * @ingroup mobilitydb_pose_comp_ever
 * @brief Return true if two temporal poses are ever equal
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_tpose_tpose(PG_FUNCTION_ARGS)
{
  return EAcomp_tpose_tpose(fcinfo, &ever_eq_tpose_tpose);
}

PGDLLEXPORT Datum Always_eq_tpose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tpose_tpose);
/**
 * @ingroup mobilitydb_pose_comp_ever
 * @brief Return true if two temporal poses are always equal
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_tpose_tpose(PG_FUNCTION_ARGS)
{
  return EAcomp_tpose_tpose(fcinfo, &always_eq_tpose_tpose);
}

PGDLLEXPORT Datum Ever_ne_tpose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tpose_tpose);
/**
 * @ingroup mobilitydb_pose_comp_ever
 * @brief Return true if two temporal poses are ever different
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_tpose_tpose(PG_FUNCTION_ARGS)
{
  return EAcomp_tpose_tpose(fcinfo, &ever_ne_tpose_tpose);
}

PGDLLEXPORT Datum Always_ne_tpose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tpose_tpose);
/**
 * @ingroup mobilitydb_pose_comp_ever
 * @brief Return true if two temporal poses are always different
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_tpose_tpose(PG_FUNCTION_ARGS)
{
  return EAcomp_tpose_tpose(fcinfo, &always_ne_tpose_tpose);
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
Tcomp_pose_tpose(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Pose *, const Temporal *))
{
  Pose *pose = PG_GETARG_POSE_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = func(pose, temp);
  PG_FREE_IF_COPY(pose, 0);
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
Tcomp_tpose_pose(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const Pose *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Pose *pose = PG_GETARG_POSE_P(1);
  Temporal *result = func(temp, pose);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(pose, 1);
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
Tcomp_tpose_tpose(FunctionCallInfo fcinfo,
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

PGDLLEXPORT Datum Teq_pose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_pose_tpose);
/**
 * @ingroup mobilitydb_pose_comp_temp
 * @brief Return a temporal Boolean that states whether a pose is equal to a
 * temporal pose
 * @brief Return true if a temporal pose is ever equal to a pose
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_pose_tpose(PG_FUNCTION_ARGS)
{
  return Tcomp_pose_tpose(fcinfo, &teq_pose_tpose);
}

PGDLLEXPORT Datum Tne_pose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_pose_tpose);
/**
 * @ingroup mobilitydb_pose_comp_temp
 * @brief Return a temporal Boolean that states whether a pose is different
 * from a temporal pose
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_pose_tpose(PG_FUNCTION_ARGS)
{
  return Tcomp_pose_tpose(fcinfo, &tne_pose_tpose);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_tpose_pose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tpose_pose);
/**
 * @ingroup mobilitydb_pose_comp_temp
 * @brief Return a temporal Boolean that states whether a temporal pose is
 * equal to a pose
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_tpose_pose(PG_FUNCTION_ARGS)
{
  return Tcomp_tpose_pose(fcinfo, &teq_tpose_pose);
}

PGDLLEXPORT Datum Tne_tpose_pose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tpose_pose);
/**
 * @ingroup mobilitydb_pose_comp_temp
 * @brief Return a temporal Boolean that states whether a temporal pose is
 * different from a pose
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_tpose_pose(PG_FUNCTION_ARGS)
{
  return Tcomp_tpose_pose(fcinfo, &tne_tpose_pose);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_tpose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tpose_tpose);
/**
 * @ingroup mobilitydb_pose_comp_temp
 * @brief Return a temporal Boolean that states whether two temporal poses
 * are equal
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_tpose_tpose(PG_FUNCTION_ARGS)
{
  return Tcomp_tpose_tpose(fcinfo, &teq_temporal_temporal);
}

PGDLLEXPORT Datum Tne_tpose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tpose_tpose);
/**
 * @ingroup mobilitydb_pose_comp_temp
 * @brief Return a temporal Boolean that states whether two temporal poses
 * are different
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_tpose_tpose(PG_FUNCTION_ARGS)
{
  return Tcomp_tpose_tpose(fcinfo, &tne_temporal_temporal);
}

/*****************************************************************************/
