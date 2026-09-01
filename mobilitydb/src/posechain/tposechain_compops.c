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
 * documentation for any purposechain, without fee, and without a written
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
 * AND FITNESS FOR A PARTICULAR PURPOSECHAIN. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief Ever/always and temporal comparisons for temporal posechains
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
#include "temporal/temporal.h"
#include "temporal/type_util.h"
#include "posechain/posechain.h"
#include "posechain/tposechain.h"
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
EAcomp_posechain_tposechain(FunctionCallInfo fcinfo,
  int (*func)(const PoseChain *, const Temporal *))
{
  PoseChain *posechain = PG_GETARG_POSECHAIN_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = func(posechain, temp);
  PG_FREE_IF_COPY(posechain, 0);
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
EAcomp_tposechain_posechain(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const PoseChain *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PoseChain *posechain = PG_GETARG_POSECHAIN_P(1);
  int result = func(temp, posechain);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(posechain, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_posechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_posechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_comp_ever
 * @brief Return true if a temporal posechain is ever equal to a posechain
 * @sqlfn eEq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_posechain_tposechain(PG_FUNCTION_ARGS)
{
  return EAcomp_posechain_tposechain(fcinfo, &ever_eq_posechain_tposechain);
}

PGDLLEXPORT Datum Always_eq_posechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_posechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_comp_ever
 * @brief Return true if a temporal posechain is always equal to a
 * posechain
 * @sqlfn aEq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_posechain_tposechain(PG_FUNCTION_ARGS)
{
  return EAcomp_posechain_tposechain(fcinfo, &always_eq_posechain_tposechain);
}

PGDLLEXPORT Datum Ever_ne_posechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_posechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_comp_ever
 * @brief Return true if a temporal posechain is ever different from a
 * posechain
 * @sqlfn eNe()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_posechain_tposechain(PG_FUNCTION_ARGS)
{
  return EAcomp_posechain_tposechain(fcinfo, &ever_ne_posechain_tposechain);
}

PGDLLEXPORT Datum Always_ne_posechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_posechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_comp_ever
 * @brief Return true if a temporal posechain is always different from a
 * posechain
 * @sqlfn aNe()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_posechain_tposechain(PG_FUNCTION_ARGS)
{
  return EAcomp_posechain_tposechain(fcinfo, &always_ne_posechain_tposechain);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tposechain_posechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tposechain_posechain);
/**
 * @ingroup mobilitydb_posechain_comp_ever
 * @brief Return true if a temporal posechain is ever equal to a posechain
 * @sqlfn eEq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_tposechain_posechain(PG_FUNCTION_ARGS)
{
  return EAcomp_tposechain_posechain(fcinfo, &ever_eq_tposechain_posechain);
}

PGDLLEXPORT Datum Always_eq_tposechain_posechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tposechain_posechain);
/**
 * @ingroup mobilitydb_posechain_comp_ever
 * @brief Return true if a temporal posechain is always equal to a
 * posechain
 * @sqlfn aEq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_tposechain_posechain(PG_FUNCTION_ARGS)
{
  return EAcomp_tposechain_posechain(fcinfo, &always_eq_tposechain_posechain);
}

PGDLLEXPORT Datum Ever_ne_tposechain_posechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tposechain_posechain);
/**
 * @ingroup mobilitydb_posechain_comp_ever
 * @brief Return true if a temporal posechain is ever different from a
 * posechain
 * @sqlfn eNe()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_tposechain_posechain(PG_FUNCTION_ARGS)
{
  return EAcomp_tposechain_posechain(fcinfo, &ever_ne_tposechain_posechain);
}

PGDLLEXPORT Datum Always_ne_tposechain_posechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tposechain_posechain);
/**
 * @ingroup mobilitydb_posechain_comp_ever
 * @brief Return true if a temporal posechain is always different from a
 * posechain
 * @sqlfn aNe()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_tposechain_posechain(PG_FUNCTION_ARGS)
{
  return EAcomp_tposechain_posechain(fcinfo, &always_ne_tposechain_posechain);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tposechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tposechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_comp_ever
 * @brief Return true if two temporal posechains are ever equal
 * @sqlfn eEq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_tposechain_tposechain(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &ever_eq_tposechain_tposechain);
}

PGDLLEXPORT Datum Always_eq_tposechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tposechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_comp_ever
 * @brief Return true if two temporal posechains are always equal
 * @sqlfn aEq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_tposechain_tposechain(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &always_eq_tposechain_tposechain);
}

PGDLLEXPORT Datum Ever_ne_tposechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tposechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_comp_ever
 * @brief Return true if two temporal posechains are ever different
 * @sqlfn eNe()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_tposechain_tposechain(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &ever_ne_tposechain_tposechain);
}

PGDLLEXPORT Datum Always_ne_tposechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tposechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_comp_ever
 * @brief Return true if two temporal posechains are always different
 * @sqlfn aNe()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_tposechain_tposechain(PG_FUNCTION_ARGS)
{
  return EAcomp_temporal_temporal(fcinfo, &always_ne_tposechain_tposechain);
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
Tcomp_posechain_tposechain(FunctionCallInfo fcinfo,
  Temporal * (*func)(const PoseChain *, const Temporal *))
{
  PoseChain *posechain = PG_GETARG_POSECHAIN_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = func(posechain, temp);
  PG_FREE_IF_COPY(posechain, 0);
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
Tcomp_tposechain_posechain(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const PoseChain *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PoseChain *posechain = PG_GETARG_POSECHAIN_P(1);
  Temporal *result = func(temp, posechain);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(posechain, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_posechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_posechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_comp_temp
 * @brief Return a temporal Boolean that states whether a posechain is equal to a
 * temporal posechain
 * @sqlfn tEq()
 * @sqlop @p #=
 */
inline Datum
Teq_posechain_tposechain(PG_FUNCTION_ARGS)
{
  return Tcomp_posechain_tposechain(fcinfo, &teq_posechain_tposechain);
}

PGDLLEXPORT Datum Tne_posechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_posechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_comp_temp
 * @brief Return a temporal Boolean that states whether a posechain is different
 * from a temporal posechain
 * @sqlfn tNe()
 * @sqlop @p #<>
 */
inline Datum
Tne_posechain_tposechain(PG_FUNCTION_ARGS)
{
  return Tcomp_posechain_tposechain(fcinfo, &tne_posechain_tposechain);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_tposechain_posechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tposechain_posechain);
/**
 * @ingroup mobilitydb_posechain_comp_temp
 * @brief Return a temporal Boolean that states whether a temporal posechain is
 * equal to a posechain
 * @sqlfn tEq()
 * @sqlop @p #=
 */
inline Datum
Teq_tposechain_posechain(PG_FUNCTION_ARGS)
{
  return Tcomp_tposechain_posechain(fcinfo, &teq_tposechain_posechain);
}

PGDLLEXPORT Datum Tne_tposechain_posechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tposechain_posechain);
/**
 * @ingroup mobilitydb_posechain_comp_temp
 * @brief Return a temporal Boolean that states whether a temporal posechain is
 * different from a posechain
 * @sqlfn tNe()
 * @sqlop @p #<>
 */
inline Datum
Tne_tposechain_posechain(PG_FUNCTION_ARGS)
{
  return Tcomp_tposechain_posechain(fcinfo, &tne_tposechain_posechain);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_tposechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tposechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_comp_temp
 * @brief Return a temporal Boolean that states whether two temporal posechains
 * are equal
 * @sqlfn tEq()
 * @sqlop @p #=
 */
inline Datum
Teq_tposechain_tposechain(PG_FUNCTION_ARGS)
{
  return Tcomp_temporal_temporal(fcinfo, &teq_temporal_temporal);
}

PGDLLEXPORT Datum Tne_tposechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tposechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_comp_temp
 * @brief Return a temporal Boolean that states whether two temporal posechains
 * are different
 * @sqlfn tNe()
 * @sqlop @p #<>
 */
inline Datum
Tne_tposechain_tposechain(PG_FUNCTION_ARGS)
{
  return Tcomp_temporal_temporal(fcinfo, &tne_temporal_temporal);
}

/*****************************************************************************/
