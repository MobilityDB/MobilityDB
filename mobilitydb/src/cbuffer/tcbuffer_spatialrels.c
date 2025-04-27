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
 * @brief Ever and always spatial relationships for temporal circular buffers
 * @details These relationships compute the ever/always spatial relationship
 * between the arguments and return a Boolean. These functions may be used for
 * filtering purposes before applying the corresponding temporal spatial
 * relationship.
 *
 * The following relationships are supported: `eContains`, `aContains`,
 * `eDisjoint`, `aDisjoint`, `eIntersects`, `aIntersects`, `eTouches`,
 * aTouches`, `eDwithin`, and `aDwithin`.
 */

#include "cbuffer/tcbuffer_spatialrels.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h" /* For varfunc */
#include "geo/tgeo_spatialfuncs.h"
#include "cbuffer/cbuffer.h"
#include "cbuffer/tcbuffer_spatialfuncs.h"
/* MobilityDB */
#include "pg_geo/postgis.h"
#include "pg_geo/tspatial.h"

/*****************************************************************************
 * Ever contains
 *****************************************************************************/

PGDLLEXPORT Datum Econtains_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Econtains_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a geometry ever contains a temporal circular buffer
 * @sqlfn eContains()
 */
inline Datum
Econtains_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_contains_geo_tcbuffer, EVER);
}

PGDLLEXPORT Datum Acontains_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acontains_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a geometry always contains a temporal circular buffer
 * @sqlfn aContains()
 */
inline Datum
Acontains_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_contains_geo_tcbuffer, ALWAYS);
}

/*****************************************************************************
 * Ever disjoint
 *****************************************************************************/

PGDLLEXPORT Datum Edisjoint_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a geometry and a temporal circular buffer are ever
 * disjoint
 * @sqlfn eDisjoint()
 */
inline Datum
Edisjoint_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_disjoint_tcbuffer_geo, EVER);
}

PGDLLEXPORT Datum Adisjoint_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a geometry and a temporal circular buffer are always
 * disjoint
 * @sqlfn aDisjoint()
 */
inline Datum
Adisjoint_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_disjoint_tcbuffer_geo, ALWAYS);
}

/**
 * @brief Return true if a temporal circular buffer and a geometry are
 * ever/always disjoint
 * @sqlfn eDisjoint(), Adisjoint()
 */
static Datum
EA_disjoint_tcbuffer_geo(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  int result = ever ? edisjoint_tcbuffer_geo(temp, gs) :
    adisjoint_tcbuffer_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Edisjoint_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a geometry are ever
 * disjoint
 * @sqlfn eDisjoint()
 */
inline Datum
Edisjoint_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  return EA_disjoint_tcbuffer_geo(fcinfo, EVER);
}

PGDLLEXPORT Datum Adisjoint_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a geometry are always
 * disjoint
 * @sqlfn aDisjoint()
 */
inline Datum
Adisjoint_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  return EA_disjoint_tcbuffer_geo(fcinfo, ALWAYS);
}

/*****************************************************************************/

/**
 * @brief Return true if a circular buffer and a temporal circular buffer are
 * ever/always disjoint
 * @sqlfn eDisjoint(), aDisjoint()
 */
static Datum
EA_disjoint_cbuffer_tcbuffer(FunctionCallInfo fcinfo, bool ever)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = ever ? edisjoint_tcbuffer_cbuffer(temp, cbuf) :
    adisjoint_tcbuffer_cbuffer(temp, cbuf);
  PG_FREE_IF_COPY(temp, 1);
  PG_FREE_IF_COPY(cbuf, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Edisjoint_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a circular buffer and a temporal circular buffer are ever
 * disjoint
 * @sqlfn eDisjoint()
 */
inline Datum
Edisjoint_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_disjoint_cbuffer_tcbuffer(fcinfo, EVER);
}

PGDLLEXPORT Datum Adisjoint_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a circular buffer and a temporal circular buffer are
 * always disjoint
 * @sqlfn aDisjoint()
 */
inline Datum
Adisjoint_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_disjoint_cbuffer_tcbuffer(fcinfo, ALWAYS);
}

/**
 * @brief Return true if a temporal circular buffer and a circular buffer are
 * ever/always disjoint
 * @sqlfn eDisjoint(), Adisjoint()
 */
static Datum
EA_disjoint_tcbuffer_cbuffer(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(1);
  int result = ever ? edisjoint_tcbuffer_cbuffer(temp, cbuf) :
    adisjoint_tcbuffer_cbuffer(temp, cbuf);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(cbuf, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Edisjoint_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a circular buffer are
 * ever disjoint
 * @sqlfn eDisjoint()
 */
inline Datum
Edisjoint_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return EA_disjoint_tcbuffer_cbuffer(fcinfo, EVER);
}

PGDLLEXPORT Datum Adisjoint_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a circular buffer are
 * always disjoint
 * @sqlfn aDisjoint()
 */
inline Datum
Adisjoint_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return EA_disjoint_tcbuffer_cbuffer(fcinfo, ALWAYS);
}

/*****************************************************************************/

PGDLLEXPORT Datum Edisjoint_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if two temporal circular buffers are ever disjoint
 * @sqlfn eDisjoint()
 */
inline Datum
Edisjoint_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &datum2_point_ne,
    &datum2_point_nsame, EVER);
}

PGDLLEXPORT Datum Adisjoint_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if two temporal circular buffers are ever disjoint
 * @sqlfn aDisjoint()
 */
inline Datum
Adisjoint_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &datum2_point_ne,
    &datum2_point_nsame, ALWAYS);
}

/*****************************************************************************
 * Ever intersects
 *****************************************************************************/

/**
 * @brief Return true if a geometry and a temporal circular buffer ever/always
 * intersect
 * @sqlfn eintersects(), aintersects()
 */
static Datum
EA_intersects_geo_tcbuffer(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = ever ?
    eintersects_tcbuffer_geo(temp, gs) : aintersects_tcbuffer_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Eintersects_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a geometry and a temporal circular buffer ever
 * intersect
 * @sqlfn eIntersects()
 */
inline Datum
Eintersects_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_intersects_geo_tcbuffer(fcinfo, EVER);
}

PGDLLEXPORT Datum Aintersects_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a geometry and a temporal circular buffer ever
 * intersect
 * @sqlfn aIntersects()
 */
inline Datum
Aintersects_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_intersects_geo_tcbuffer(fcinfo, ALWAYS);
}

/**
 * @brief Return true if a geometry and a temporal circular buffer ever/always
 * intersect
 * @sqlfn eintersects(), aintersects()
 */
static Datum
EA_intersects_tcbuffer_geo(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  int result = ever ?
    eintersects_tcbuffer_geo(temp, gs) : aintersects_tcbuffer_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Eintersects_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a geometry ever
 * intersect
 * @sqlfn eIntersects()
 */
inline Datum
Eintersects_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  return EA_intersects_tcbuffer_geo(fcinfo, EVER);
}

PGDLLEXPORT Datum Aintersects_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a geometry always
 * intersect
 * @sqlfn aIntersects()
 */
inline Datum
Aintersects_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  return EA_intersects_tcbuffer_geo(fcinfo, ALWAYS);
}

/**
 * @brief Return true if a circular buffer and a temporal circular buffer
 * ever/always intersect
 * @sqlfn eintersects(), aintersects()
 */
static Datum
EA_intersects_cbuffer_tcbuffer(FunctionCallInfo fcinfo, bool ever)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = ever ? eintersects_tcbuffer_cbuffer(temp, cbuf) : 
    aintersects_tcbuffer_cbuffer(temp, cbuf);
  PG_FREE_IF_COPY(cbuf, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Eintersects_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a circular buffer and a temporal
 * circular buffer ever intersect
 * @sqlfn eIntersects()
 */
inline Datum
Eintersects_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_intersects_cbuffer_tcbuffer(fcinfo, EVER);
}

PGDLLEXPORT Datum Aintersects_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a circular buffer and a temporal
 * circular buffer always intersect
 * @sqlfn aIntersects()
 */
inline Datum
Aintersects_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_intersects_cbuffer_tcbuffer(fcinfo, ALWAYS);
}

/**
 * @brief Return true if a circular buffer and a temporal circular buffer
 * ever/always intersect
 * @sqlfn eintersects(), aintersects()
 */
static Datum
EA_intersects_tcbuffer_cbuffer(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(1);
  int result = ever ? eintersects_tcbuffer_cbuffer(temp, cbuf) : 
    aintersects_tcbuffer_cbuffer(temp, cbuf);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(cbuf, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Eintersects_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a circular buffer 
 * ever intersect
 * @sqlfn eIntersects()
 */
inline Datum
Eintersects_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return EA_intersects_tcbuffer_cbuffer(fcinfo, EVER);
}

PGDLLEXPORT Datum Aintersects_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a circular buffer 
 * always intersect
 * @sqlfn aIntersects()
 */
inline Datum
Aintersects_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return EA_intersects_tcbuffer_cbuffer(fcinfo, ALWAYS);
}

/*****************************************************************************/

PGDLLEXPORT Datum Eintersects_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if two temporal circular buffers ever intersect
 * @sqlfn eIntersects()
 */
inline Datum
Eintersects_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &datum2_point_eq,
    &datum2_point_same, EVER);
}

PGDLLEXPORT Datum Aintersects_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if two temporal circular buffers ever intersect
 * @sqlfn aIntersects()
 */
inline Datum
Aintersects_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &datum2_point_eq,
    &datum2_point_same, ALWAYS);
}

/*****************************************************************************
 * Ever/always touches
 *****************************************************************************/

/**
 * @brief Return true if a geometry and a temporal circular buffer ever/always touch
 * @sqlfn eTouches(), aTouches()
 */
static Datum
EA_touches_geo_tcbuffer(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = ever ? etouches_tcbuffer_geo(temp, gs) :
    atouches_tcbuffer_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 1);
  PG_FREE_IF_COPY(gs, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Etouches_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a geometry and a temporal circular buffer ever touch
 * @sqlfn eTouches()
 */
inline Datum
Etouches_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_touches_geo_tcbuffer(fcinfo, EVER);
}

PGDLLEXPORT Datum Atouches_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a geometry and a temporal circular buffer ever touch
 * @sqlfn aTouches()
 */
inline Datum
Atouches_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_touches_geo_tcbuffer(fcinfo, ALWAYS);
}

/**
 * @brief Return true if a temporal circular buffer and a geometry ever/always touch
 * @sqlfn eTouches(), aTouches()
 */
static Datum
EA_touches_tcbuffer_geo(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = ever ? etouches_tcbuffer_geo(temp, gs) :
    atouches_tcbuffer_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Etouches_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a geometry ever touch
 * @sqlfn eTouches()
 */
inline Datum
Etouches_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  return EA_touches_tcbuffer_geo(fcinfo, EVER);
}

PGDLLEXPORT Datum Atouches_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a geometry always touch
 * @sqlfn aTouches()
 */
inline Datum
Atouches_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  return EA_touches_tcbuffer_geo(fcinfo, ALWAYS);
}

/*****************************************************************************/

/**
 * @brief Return true if a circular buffer and a temporal circular buffer
 * ever/always touch
 * @sqlfn etouches(), atouches()
 */
static Datum
EA_touches_cbuffer_tcbuffer(FunctionCallInfo fcinfo, bool ever)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = ever ? etouches_tcbuffer_cbuffer(temp, cbuf) : 
    atouches_tcbuffer_cbuffer(temp, cbuf);
  PG_FREE_IF_COPY(cbuf, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Etouches_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a circular buffer and a temporal
 * circular buffer ever touch
 * @sqlfn eTouches()
 */
inline Datum
Etouches_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_touches_cbuffer_tcbuffer(fcinfo, EVER);
}

PGDLLEXPORT Datum Atouches_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a circular buffer and a temporal
 * circular buffer always touch
 * @sqlfn aTouches()
 */
inline Datum
Atouches_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_touches_cbuffer_tcbuffer(fcinfo, ALWAYS);
}

/**
 * @brief Return true if a circular buffer and a temporal circular buffer
 * ever/always touch
 * @sqlfn etouches(), atouches()
 */
static Datum
EA_touches_tcbuffer_cbuffer(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(1);
  int result = ever ? etouches_tcbuffer_cbuffer(temp, cbuf) : 
    atouches_tcbuffer_cbuffer(temp, cbuf);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(cbuf, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Etouches_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a circular buffer 
 * ever touch
 * @sqlfn eTouches()
 */
inline Datum
Etouches_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return EA_touches_tcbuffer_cbuffer(fcinfo, EVER);
}

PGDLLEXPORT Datum Atouches_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a circular buffer 
 * always touch
 * @sqlfn aTouches()
 */
inline Datum
Atouches_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return EA_touches_tcbuffer_cbuffer(fcinfo, ALWAYS);
}

/*****************************************************************************
 * Ever/always dwithin
 * The function only accepts points and not arbitrary geometries
 *****************************************************************************/

/**
 * @brief Return true if a geometry and a temporal circular buffer are
 * ever/always within a distance
 * @sqlfn eDwithin()
 */
static Datum
EA_dwithin_geo_tcbuffer(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = ever ?
    edwithin_tcbuffer_geo(temp, gs, dist) :
    adwithin_tcbuffer_geo(temp, gs, dist);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Edwithin_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a geometry and a temporal circular buffer are ever
 * within a distance
 * @sqlfn eDwithin()
 */
inline Datum
Edwithin_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_dwithin_geo_tcbuffer(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a geometry and a temporal circular buffer are always
 * within a distance
 * @sqlfn aDwithin()
 */
inline Datum
Adwithin_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_dwithin_geo_tcbuffer(fcinfo, ALWAYS);
}

/**
 * @brief Return true if a temporal circular buffer and a geometry are
 * ever/always within a distance
 * @sqlfn eDwithin()
 */
static Datum
EA_dwithin_tcbuffer_geo(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(2);
  int result = ever ? edwithin_tcbuffer_geo(temp, gs, dist) :
    adwithin_tcbuffer_geo(temp, gs, dist);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Edwithin_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a geometry are ever
 * within a distance
 * @sqlfn eDwithin()
 */
inline Datum
Edwithin_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  return EA_dwithin_tcbuffer_geo(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a geometry are always
 * within a distance
 * @sqlfn aDwithin()
 */
inline Datum
Adwithin_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  return EA_dwithin_tcbuffer_geo(fcinfo, ALWAYS);
}

/**
 * @brief Return true if two temporal circular buffers are even/always within a
 * distance
 * @sqlfn eDwithin(), aDwithin()
 */
static Datum
EA_dwithin_tcbuffer_tcbuffer(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = ever ? edwithin_tcbuffer_tcbuffer(temp1, temp2, dist) :
    adwithin_tcbuffer_tcbuffer(temp1, temp2, dist);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/

/**
 * @brief Return true if a circular buffer and a temporal circular buffer are
 * ever/always within a distance
 * @sqlfn eDwithin()
 */
static Datum
EA_dwithin_cbuffer_tcbuffer(FunctionCallInfo fcinfo, bool ever)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = ever ?
    edwithin_tcbuffer_cbuffer(temp, cbuf, dist) :
    adwithin_tcbuffer_cbuffer(temp, cbuf, dist);
  PG_FREE_IF_COPY(cbuf, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Edwithin_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a circular buffer and a temporal circular buffer are
 * ever within a distance
 * @sqlfn eDwithin()
 */
inline Datum
Edwithin_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_dwithin_cbuffer_tcbuffer(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a circular buffer and a temporal circular buffer are
 * always within a distance
 * @sqlfn aDwithin()
 */
inline Datum
Adwithin_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_dwithin_cbuffer_tcbuffer(fcinfo, ALWAYS);
}

/**
 * @brief Return true if a temporal circular buffer and a circular buffer are
 * ever/always within a distance
 * @sqlfn eDwithin()
 */
static Datum
EA_dwithin_tcbuffer_cbuffer(FunctionCallInfo fcinfo, bool ever)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(2);
  int result = ever ? edwithin_tcbuffer_cbuffer(temp, cbuf, dist) :
    adwithin_tcbuffer_cbuffer(temp, cbuf, dist);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(cbuf, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Edwithin_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a circular buffer are
 * ever within a distance
 * @sqlfn eDwithin()
 */
inline Datum
Edwithin_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return EA_dwithin_tcbuffer_cbuffer(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a circular buffer are
 * always within a distance
 * @sqlfn aDwithin()
 */
inline Datum
Adwithin_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return EA_dwithin_tcbuffer_cbuffer(fcinfo, ALWAYS);
}

/*****************************************************************************/

PGDLLEXPORT Datum Edwithin_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if two temporal circular buffers are ever within a distance
 * @sqlfn eDwithin()
 */
inline Datum
Edwithin_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_dwithin_tcbuffer_tcbuffer(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if two temporal circular buffers are always within a distance
 * @sqlfn aDwithin()
 */
inline Datum
Adwithin_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_dwithin_tcbuffer_tcbuffer(fcinfo, ALWAYS);
}

/*****************************************************************************/
