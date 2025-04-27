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
 * @brief Ever/always and temporal comparisons for temporal circular buffers
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
#include <meos_cbuffer.h>
#include "general/temporal.h"
#include "cbuffer/cbuffer.h"
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
EAcomp_cbuffer_tcbuffer(FunctionCallInfo fcinfo,
  int (*func)(const Cbuffer *, const Temporal *))
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = func(cbuf, temp);
  PG_FREE_IF_COPY(cbuf, 0);
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
EAcomp_tcbuffer_cbuffer(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const Cbuffer *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(1);
  int result = func(temp, cbuf);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(cbuf, 1);
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
EAcomp_tcbuffer_tcbuffer(FunctionCallInfo fcinfo,
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

PGDLLEXPORT Datum Ever_eq_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_ever
 * @brief Return true if a temporal circular buffer is ever equal to a circular
 * buffer
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EAcomp_cbuffer_tcbuffer(fcinfo, &ever_eq_cbuffer_tcbuffer);
}

PGDLLEXPORT Datum Always_eq_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_ever
 * @brief Return true if a temporal circular buffer is always equal to a
 * circular buffer
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EAcomp_cbuffer_tcbuffer(fcinfo, &always_eq_cbuffer_tcbuffer);
}

PGDLLEXPORT Datum Ever_ne_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_ever
 * @brief Return true if a temporal circular buffer is ever different from a
 * circular buffer
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EAcomp_cbuffer_tcbuffer(fcinfo, &ever_ne_cbuffer_tcbuffer);
}

PGDLLEXPORT Datum Always_ne_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_ever
 * @brief Return true if a temporal circular buffer is always different from a
 * circular buffer
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EAcomp_cbuffer_tcbuffer(fcinfo, &always_ne_cbuffer_tcbuffer);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_ever
 * @brief Return true if a temporal circular buffer is ever equal to a circular
 * buffer
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return EAcomp_tcbuffer_cbuffer(fcinfo, &ever_eq_tcbuffer_cbuffer);
}

PGDLLEXPORT Datum Always_eq_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_ever
 * @brief Return true if a temporal circular buffer is always equal to a
 * circular buffer
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return EAcomp_tcbuffer_cbuffer(fcinfo, &always_eq_tcbuffer_cbuffer);
}

PGDLLEXPORT Datum Ever_ne_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_ever
 * @brief Return true if a temporal circular buffer is ever different from a
 * circular buffer
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return EAcomp_tcbuffer_cbuffer(fcinfo, &ever_ne_tcbuffer_cbuffer);
}

PGDLLEXPORT Datum Always_ne_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_ever
 * @brief Return true if a temporal circular buffer is always different from a
 * circular buffer
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return EAcomp_tcbuffer_cbuffer(fcinfo, &always_ne_tcbuffer_cbuffer);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ever_eq_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_ever
 * @brief Return true if two temporal circular buffers are ever equal
 * @sqlfn ever_eq()
 * @sqlop @p ?=
 */
inline Datum
Ever_eq_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EAcomp_tcbuffer_tcbuffer(fcinfo, &ever_eq_tcbuffer_tcbuffer);
}

PGDLLEXPORT Datum Always_eq_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_eq_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_ever
 * @brief Return true if two temporal circular buffers are always equal
 * @sqlfn always_eq()
 * @sqlop @p %=
 */
inline Datum
Always_eq_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EAcomp_tcbuffer_tcbuffer(fcinfo, &always_eq_tcbuffer_tcbuffer);
}

PGDLLEXPORT Datum Ever_ne_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_ne_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_ever
 * @brief Return true if two temporal circular buffers are ever different
 * @sqlfn ever_ne()
 * @sqlop @p ?<>
 */
inline Datum
Ever_ne_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EAcomp_tcbuffer_tcbuffer(fcinfo, &ever_ne_tcbuffer_tcbuffer);
}

PGDLLEXPORT Datum Always_ne_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Always_ne_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_ever
 * @brief Return true if two temporal circular buffers are always different
 * @sqlfn always_ne()
 * @sqlop @p %<>
 */
inline Datum
Always_ne_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EAcomp_tcbuffer_tcbuffer(fcinfo, &always_ne_tcbuffer_tcbuffer);
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
Tcomp_cbuffer_tcbuffer(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Cbuffer *, const Temporal *))
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = func(cbuf, temp);
  PG_FREE_IF_COPY(cbuf, 0);
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
Tcomp_tcbuffer_cbuffer(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const Cbuffer *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(1);
  Temporal *result = func(temp, cbuf);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(cbuf, 1);
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
Tcomp_tcbuffer_tcbuffer(FunctionCallInfo fcinfo,
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

PGDLLEXPORT Datum Teq_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_temp
 * @brief Return a temporal Boolean that states whether a circular buffer is
 * equal to a temporal circular buffer
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Tcomp_cbuffer_tcbuffer(fcinfo, &teq_cbuffer_tcbuffer);
}

PGDLLEXPORT Datum Tne_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_temp
 * @brief Return a temporal Boolean that states whether a circular buffer is
 * different from a temporal circular buffer
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Tcomp_cbuffer_tcbuffer(fcinfo, &tne_cbuffer_tcbuffer);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer is equal to a circular buffer
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return Tcomp_tcbuffer_cbuffer(fcinfo, &teq_tcbuffer_cbuffer);
}

PGDLLEXPORT Datum Tne_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_temp
 * @brief Return a temporal Boolean that states whether a temporal circular
 * buffer is different from a circular buffer
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return Tcomp_tcbuffer_cbuffer(fcinfo, &tne_tcbuffer_cbuffer);
}

/*****************************************************************************/

PGDLLEXPORT Datum Teq_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Teq_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_temp
 * @brief Return a temporal Boolean that states whether two temporal circular
 * buffers are equal
 * @sqlfn temporal_teq()
 * @sqlop @p #=
 */
inline Datum
Teq_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Tcomp_tcbuffer_tcbuffer(fcinfo, &teq_temporal_temporal);
}

PGDLLEXPORT Datum Tne_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tne_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_comp_temp
 * @brief Return a temporal Boolean that states whether two temporal circular
 * buffers are different
 * @sqlfn temporal_tne()
 * @sqlop @p #<>
 */
inline Datum
Tne_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Tcomp_tcbuffer_tcbuffer(fcinfo, &tne_temporal_temporal);
}

/*****************************************************************************/
