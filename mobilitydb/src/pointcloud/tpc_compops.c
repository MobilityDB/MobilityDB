/***********************************************************************
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
 * @brief Ever/always and temporal comparisons for the temporal pgpointcloud
 *   types
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"
#include "pointcloud/pcpoint.h"
#include "pointcloud/pcpatch.h"
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
EAcomp_pcpoint_tpcpoint(FunctionCallInfo fcinfo,
  int (*func)(const Pcpoint *, const Temporal *))
{
  Pcpoint *pt = PG_GETARG_PCPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = func(pt, temp);
  PG_FREE_IF_COPY(pt, 0);
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
EAcomp_tpcpoint_pcpoint(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const Pcpoint *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Pcpoint *pt = PG_GETARG_PCPOINT_P(1);
  int result = func(temp, pt);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(pt, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic function for the ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 * @note Serves both temporal pgpointcloud types: the operands are carried as
 *   the generic temporal type, so the point and the patch flavours share it
 */
static Datum
EAcomp_tpc_tpc(FunctionCallInfo fcinfo,
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

/**
 * @brief Generic function for the ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
EAcomp_pcpatch_tpcpatch(FunctionCallInfo fcinfo,
  int (*func)(const Pcpatch *, const Temporal *))
{
  Pcpatch *pa = PG_GETARG_PCPATCH_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = func(pa, temp);
  PG_FREE_IF_COPY(pa, 0);
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
EAcomp_tpcpatch_pcpatch(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const Pcpatch *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Pcpatch *pa = PG_GETARG_PCPATCH_P(1);
  int result = func(temp, pa);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(pa, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_pcpoint_tpcpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_pcpoint_tpcpoint);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if a pcpoint is ever equal to a temporal pgpointcloud
 * point
 * @sqlfn eEq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_pcpoint_tpcpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_pcpoint_tpcpoint(fcinfo, &ever_eq_pcpoint_tpcpoint);
}

PGDLLEXPORT Datum Always_eq_pcpoint_tpcpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_pcpoint_tpcpoint);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if a pcpoint is always equal to a temporal pgpointcloud
 * point
 * @sqlfn aEq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_pcpoint_tpcpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_pcpoint_tpcpoint(fcinfo, &always_eq_pcpoint_tpcpoint);
}

PGDLLEXPORT Datum Ever_ne_pcpoint_tpcpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_pcpoint_tpcpoint);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if a pcpoint is ever different from a temporal
 * pgpointcloud point
 * @sqlfn eNe()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_pcpoint_tpcpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_pcpoint_tpcpoint(fcinfo, &ever_ne_pcpoint_tpcpoint);
}

PGDLLEXPORT Datum Always_ne_pcpoint_tpcpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_pcpoint_tpcpoint);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if a pcpoint is always different from a temporal
 * pgpointcloud point
 * @sqlfn aNe()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_pcpoint_tpcpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_pcpoint_tpcpoint(fcinfo, &always_ne_pcpoint_tpcpoint);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tpcpoint_pcpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tpcpoint_pcpoint);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if a temporal pgpointcloud point is ever equal to a
 * pcpoint
 * @sqlfn eEq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_tpcpoint_pcpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tpcpoint_pcpoint(fcinfo, &ever_eq_tpcpoint_pcpoint);
}

PGDLLEXPORT Datum Always_eq_tpcpoint_pcpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tpcpoint_pcpoint);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if a temporal pgpointcloud point is always equal to a
 * pcpoint
 * @sqlfn aEq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_tpcpoint_pcpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tpcpoint_pcpoint(fcinfo, &always_eq_tpcpoint_pcpoint);
}

PGDLLEXPORT Datum Ever_ne_tpcpoint_pcpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tpcpoint_pcpoint);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if a temporal pgpointcloud point is ever different from a
 * pcpoint
 * @sqlfn eNe()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_tpcpoint_pcpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tpcpoint_pcpoint(fcinfo, &ever_ne_tpcpoint_pcpoint);
}

PGDLLEXPORT Datum Always_ne_tpcpoint_pcpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tpcpoint_pcpoint);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if a temporal pgpointcloud point is always different from
 * a pcpoint
 * @sqlfn aNe()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_tpcpoint_pcpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tpcpoint_pcpoint(fcinfo, &always_ne_tpcpoint_pcpoint);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tpcpoint_tpcpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tpcpoint_tpcpoint);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if two temporal pgpointcloud points are ever equal
 * @sqlfn eEq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_tpcpoint_tpcpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tpc_tpc(fcinfo, &ever_eq_tpcpoint_tpcpoint);
}

PGDLLEXPORT Datum Always_eq_tpcpoint_tpcpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tpcpoint_tpcpoint);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if two temporal pgpointcloud points are always equal
 * @sqlfn aEq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_tpcpoint_tpcpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tpc_tpc(fcinfo, &always_eq_tpcpoint_tpcpoint);
}

PGDLLEXPORT Datum Ever_ne_tpcpoint_tpcpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tpcpoint_tpcpoint);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if two temporal pgpointcloud points are ever different
 * @sqlfn eNe()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_tpcpoint_tpcpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tpc_tpc(fcinfo, &ever_ne_tpcpoint_tpcpoint);
}

PGDLLEXPORT Datum Always_ne_tpcpoint_tpcpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tpcpoint_tpcpoint);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if two temporal pgpointcloud points are always different
 * @sqlfn aNe()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_tpcpoint_tpcpoint(PG_FUNCTION_ARGS)
{
  return EAcomp_tpc_tpc(fcinfo, &always_ne_tpcpoint_tpcpoint);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_pcpatch_tpcpatch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_pcpatch_tpcpatch);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if a pcpatch is ever equal to a temporal pgpointcloud
 * patch
 * @sqlfn eEq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_pcpatch_tpcpatch(PG_FUNCTION_ARGS)
{
  return EAcomp_pcpatch_tpcpatch(fcinfo, &ever_eq_pcpatch_tpcpatch);
}

PGDLLEXPORT Datum Always_eq_pcpatch_tpcpatch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_pcpatch_tpcpatch);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if a pcpatch is always equal to a temporal pgpointcloud
 * patch
 * @sqlfn aEq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_pcpatch_tpcpatch(PG_FUNCTION_ARGS)
{
  return EAcomp_pcpatch_tpcpatch(fcinfo, &always_eq_pcpatch_tpcpatch);
}

PGDLLEXPORT Datum Ever_ne_pcpatch_tpcpatch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_pcpatch_tpcpatch);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if a pcpatch is ever different from a temporal
 * pgpointcloud patch
 * @sqlfn eNe()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_pcpatch_tpcpatch(PG_FUNCTION_ARGS)
{
  return EAcomp_pcpatch_tpcpatch(fcinfo, &ever_ne_pcpatch_tpcpatch);
}

PGDLLEXPORT Datum Always_ne_pcpatch_tpcpatch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_pcpatch_tpcpatch);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if a pcpatch is always different from a temporal
 * pgpointcloud patch
 * @sqlfn aNe()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_pcpatch_tpcpatch(PG_FUNCTION_ARGS)
{
  return EAcomp_pcpatch_tpcpatch(fcinfo, &always_ne_pcpatch_tpcpatch);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tpcpatch_pcpatch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tpcpatch_pcpatch);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if a temporal pgpointcloud patch is ever equal to a
 * pcpatch
 * @sqlfn eEq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_tpcpatch_pcpatch(PG_FUNCTION_ARGS)
{
  return EAcomp_tpcpatch_pcpatch(fcinfo, &ever_eq_tpcpatch_pcpatch);
}

PGDLLEXPORT Datum Always_eq_tpcpatch_pcpatch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tpcpatch_pcpatch);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if a temporal pgpointcloud patch is always equal to a
 * pcpatch
 * @sqlfn aEq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_tpcpatch_pcpatch(PG_FUNCTION_ARGS)
{
  return EAcomp_tpcpatch_pcpatch(fcinfo, &always_eq_tpcpatch_pcpatch);
}

PGDLLEXPORT Datum Ever_ne_tpcpatch_pcpatch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tpcpatch_pcpatch);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if a temporal pgpointcloud patch is ever different from a
 * pcpatch
 * @sqlfn eNe()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_tpcpatch_pcpatch(PG_FUNCTION_ARGS)
{
  return EAcomp_tpcpatch_pcpatch(fcinfo, &ever_ne_tpcpatch_pcpatch);
}

PGDLLEXPORT Datum Always_ne_tpcpatch_pcpatch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tpcpatch_pcpatch);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if a temporal pgpointcloud patch is always different from
 * a pcpatch
 * @sqlfn aNe()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_tpcpatch_pcpatch(PG_FUNCTION_ARGS)
{
  return EAcomp_tpcpatch_pcpatch(fcinfo, &always_ne_tpcpatch_pcpatch);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tpcpatch_tpcpatch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tpcpatch_tpcpatch);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if two temporal pgpointcloud patches are ever equal
 * @sqlfn eEq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_tpcpatch_tpcpatch(PG_FUNCTION_ARGS)
{
  return EAcomp_tpc_tpc(fcinfo, &ever_eq_tpcpatch_tpcpatch);
}

PGDLLEXPORT Datum Always_eq_tpcpatch_tpcpatch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tpcpatch_tpcpatch);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if two temporal pgpointcloud patches are always equal
 * @sqlfn aEq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_tpcpatch_tpcpatch(PG_FUNCTION_ARGS)
{
  return EAcomp_tpc_tpc(fcinfo, &always_eq_tpcpatch_tpcpatch);
}

PGDLLEXPORT Datum Ever_ne_tpcpatch_tpcpatch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tpcpatch_tpcpatch);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if two temporal pgpointcloud patches are ever different
 * @sqlfn eNe()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_tpcpatch_tpcpatch(PG_FUNCTION_ARGS)
{
  return EAcomp_tpc_tpc(fcinfo, &ever_ne_tpcpatch_tpcpatch);
}

PGDLLEXPORT Datum Always_ne_tpcpatch_tpcpatch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tpcpatch_tpcpatch);
/**
 * @ingroup mobilitydb_pointcloud_comp_ever
 * @brief Return true if two temporal pgpointcloud patches are always different
 * @sqlfn aNe()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_tpcpatch_tpcpatch(PG_FUNCTION_ARGS)
{
  return EAcomp_tpc_tpc(fcinfo, &always_ne_tpcpatch_tpcpatch);
}

/*****************************************************************************
 * Temporal comparison functions
 *****************************************************************************/

/**
 * @brief Generic function for the temporal comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the temporal comparison
 */
static Datum
Tcomp_pcpoint_tpcpoint(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Pcpoint *, const Temporal *))
{
  Pcpoint *pt = PG_GETARG_PCPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = func(pt, temp);
  PG_FREE_IF_COPY(pt, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Generic function for the temporal comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the temporal comparison
 */
static Datum
Tcomp_tpcpoint_pcpoint(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const Pcpoint *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Pcpoint *pt = PG_GETARG_PCPOINT_P(1);
  Temporal *result = func(temp, pt);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(pt, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Generic function for the temporal comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the temporal comparison
 */
static Datum
Tcomp_pcpatch_tpcpatch(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Pcpatch *, const Temporal *))
{
  Pcpatch *pa = PG_GETARG_PCPATCH_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = func(pa, temp);
  PG_FREE_IF_COPY(pa, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Generic function for the temporal comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the temporal comparison
 */
static Datum
Tcomp_tpcpatch_pcpatch(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const Pcpatch *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Pcpatch *pa = PG_GETARG_PCPATCH_P(1);
  Temporal *result = func(temp, pa);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(pa, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_pcpoint_tpcpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_pcpoint_tpcpoint);
/**
 * @ingroup mobilitydb_pointcloud_comp_temp
 * @brief Return the temporal equality of a pcpoint and a temporal
 * pgpointcloud point
 * @sqlfn tEq()
 * @sqlop @p #=
 */
inline Datum
Teq_pcpoint_tpcpoint(PG_FUNCTION_ARGS)
{
  return Tcomp_pcpoint_tpcpoint(fcinfo, &teq_pcpoint_tpcpoint);
}

PGDLLEXPORT Datum Tne_pcpoint_tpcpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_pcpoint_tpcpoint);
/**
 * @ingroup mobilitydb_pointcloud_comp_temp
 * @brief Return the temporal inequality of a pcpoint and a temporal
 * pgpointcloud point
 * @sqlfn tNe()
 * @sqlop @p #<>
 */
inline Datum
Tne_pcpoint_tpcpoint(PG_FUNCTION_ARGS)
{
  return Tcomp_pcpoint_tpcpoint(fcinfo, &tne_pcpoint_tpcpoint);
}

PGDLLEXPORT Datum Teq_tpcpoint_pcpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tpcpoint_pcpoint);
/**
 * @ingroup mobilitydb_pointcloud_comp_temp
 * @brief Return the temporal equality of a temporal pgpointcloud point and a
 * pcpoint
 * @sqlfn tEq()
 * @sqlop @p #=
 */
inline Datum
Teq_tpcpoint_pcpoint(PG_FUNCTION_ARGS)
{
  return Tcomp_tpcpoint_pcpoint(fcinfo, &teq_tpcpoint_pcpoint);
}

PGDLLEXPORT Datum Tne_tpcpoint_pcpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tpcpoint_pcpoint);
/**
 * @ingroup mobilitydb_pointcloud_comp_temp
 * @brief Return the temporal inequality of a temporal pgpointcloud point and a
 * pcpoint
 * @sqlfn tNe()
 * @sqlop @p #<>
 */
inline Datum
Tne_tpcpoint_pcpoint(PG_FUNCTION_ARGS)
{
  return Tcomp_tpcpoint_pcpoint(fcinfo, &tne_tpcpoint_pcpoint);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_pcpatch_tpcpatch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_pcpatch_tpcpatch);
/**
 * @ingroup mobilitydb_pointcloud_comp_temp
 * @brief Return the temporal equality of a pcpatch and a temporal
 * pgpointcloud patch
 * @sqlfn tEq()
 * @sqlop @p #=
 */
inline Datum
Teq_pcpatch_tpcpatch(PG_FUNCTION_ARGS)
{
  return Tcomp_pcpatch_tpcpatch(fcinfo, &teq_pcpatch_tpcpatch);
}

PGDLLEXPORT Datum Tne_pcpatch_tpcpatch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_pcpatch_tpcpatch);
/**
 * @ingroup mobilitydb_pointcloud_comp_temp
 * @brief Return the temporal inequality of a pcpatch and a temporal
 * pgpointcloud patch
 * @sqlfn tNe()
 * @sqlop @p #<>
 */
inline Datum
Tne_pcpatch_tpcpatch(PG_FUNCTION_ARGS)
{
  return Tcomp_pcpatch_tpcpatch(fcinfo, &tne_pcpatch_tpcpatch);
}

PGDLLEXPORT Datum Teq_tpcpatch_pcpatch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tpcpatch_pcpatch);
/**
 * @ingroup mobilitydb_pointcloud_comp_temp
 * @brief Return the temporal equality of a temporal pgpointcloud patch and a
 * pcpatch
 * @sqlfn tEq()
 * @sqlop @p #=
 */
inline Datum
Teq_tpcpatch_pcpatch(PG_FUNCTION_ARGS)
{
  return Tcomp_tpcpatch_pcpatch(fcinfo, &teq_tpcpatch_pcpatch);
}

PGDLLEXPORT Datum Tne_tpcpatch_pcpatch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tpcpatch_pcpatch);
/**
 * @ingroup mobilitydb_pointcloud_comp_temp
 * @brief Return the temporal inequality of a temporal pgpointcloud patch and a
 * pcpatch
 * @sqlfn tNe()
 * @sqlop @p #<>
 */
inline Datum
Tne_tpcpatch_pcpatch(PG_FUNCTION_ARGS)
{
  return Tcomp_tpcpatch_pcpatch(fcinfo, &tne_tpcpatch_pcpatch);
}

/*****************************************************************************/
