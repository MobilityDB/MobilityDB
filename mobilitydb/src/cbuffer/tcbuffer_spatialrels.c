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
 * filtering purposes before applying the corresponding spatiotemporal
 * relationship.
 *
 * The following relationships are supported: `eContains`, `aContains`,
 * `eCovers`, `aCovers`, `eDisjoint`, `aDisjoint`, `eIntersects`, 
 * `aIntersects`, `eTouches`, `aTouches`, `eDwithin`, and `aDwithin`.
 *
 * Most of these relationships support the following combination of arguments
 * `({geo, cbuffer, tcbuffer}, {geo, cbuffer, tcbuffer})`.
 * One exception is for the non-symmetric relationships `eContains` and 
 * `eCovers` since there is no efficient algorithm for enabling the
 * combination of arguments `(geo, tcbuffer)`.
 * Another exception is that only `eDisjoint`, `aDisjoint`, `eIntersects`,
 * `aIntersects`, `eDwithin`, and `aDwithin` support the following arguments
 * `(tcbuffer, tcbuffer)`.
 */

#include "cbuffer/tcbuffer_spatialrels.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/temporal.h" /* For varfunc */
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tgeo_spatialrels.h"
#include "cbuffer/cbuffer.h"
#include "cbuffer/tcbuffer_spatialfuncs.h"
/* MobilityDB */
#include "pg_geo/postgis.h"
#include "pg_geo/tspatial.h"

/*****************************************************************************
 * Generic ever/always spatial relationship functions
 *****************************************************************************/

/**
 * @brief Return true if a circular buffer and a temporal circular buffer 
 * ever/always satisfy a spatial relationship
 */
Datum
EA_spatialrel_cbuffer_tcbuffer(FunctionCallInfo fcinfo,
  int (*func)(const Cbuffer *, const Temporal *, bool), bool ever)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = func(cb, temp, ever);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_INT32(result);
}

/**
 * @brief Return true if a geometry and a spatiotemporal value ever/always
 * satisfy a spatial relationship
 */
Datum
EA_spatialrel_tcbuffer_cbuffer(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const Cbuffer *, bool), bool ever)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Cbuffer *cb = PG_GETARG_CBUFFER_P(1);
  int result = func(temp, cb, ever);
  PG_FREE_IF_COPY(temp, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_INT32(result);
}

/*****************************************************************************
 * Ever/always contains
 *****************************************************************************/

/* Econtains_geo_tcbuffer is not supported */

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

PGDLLEXPORT Datum Econtains_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Econtains_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer ever contains a geometry
 * @sqlfn eContains()
 */
inline Datum
Econtains_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_contains_tcbuffer_geo, EVER);
}

PGDLLEXPORT Datum Acontains_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acontains_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer always contains a geometry
 * @sqlfn aContains()
 */
inline Datum
Acontains_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_contains_tcbuffer_geo, ALWAYS);
}

/*****************************************************************************/

PGDLLEXPORT Datum Econtains_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Econtains_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a geometry ever contains a temporal circular buffer
 * @sqlfn eContains()
 */
inline Datum
Econtains_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_cbuffer_tcbuffer(fcinfo, &ea_contains_cbuffer_tcbuffer,
    EVER);
}

PGDLLEXPORT Datum Acontains_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acontains_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a geometry always contains a temporal circular buffer
 * @sqlfn aContains()
 */
inline Datum
Acontains_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_cbuffer_tcbuffer(fcinfo, &ea_contains_cbuffer_tcbuffer,
    ALWAYS);
}

PGDLLEXPORT Datum Econtains_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Econtains_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer ever contains a geometry
 * @sqlfn eContains()
 */
inline Datum
Econtains_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tcbuffer_cbuffer(fcinfo, &ea_contains_tcbuffer_cbuffer,
    EVER);
}

PGDLLEXPORT Datum Acontains_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acontains_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer always contains a geometry
 * @sqlfn aContains()
 */
inline Datum
Acontains_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tcbuffer_cbuffer(fcinfo, &ea_contains_tcbuffer_cbuffer,
    ALWAYS);
}

/*****************************************************************************
 * Ever/always covers
 *****************************************************************************/

/* Ecovers_geo_tcbuffer is not supported */

PGDLLEXPORT Datum Acovers_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acovers_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a geometry always covers a temporal circular buffer
 * @sqlfn aCovers()
 */
inline Datum
Acovers_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_covers_geo_tcbuffer, ALWAYS);
}

PGDLLEXPORT Datum Ecovers_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ecovers_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer ever covers a geometry
 * @sqlfn eCovers()
 */
inline Datum
Ecovers_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_covers_tcbuffer_geo, EVER);
}

PGDLLEXPORT Datum Acovers_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acovers_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer always covers a geometry
 * @sqlfn aCovers()
 */
inline Datum
Acovers_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_covers_tcbuffer_geo, ALWAYS);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ecovers_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ecovers_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a geometry ever covers a temporal circular buffer
 * @sqlfn eCovers()
 */
inline Datum
Ecovers_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_cbuffer_tcbuffer(fcinfo, &ea_covers_cbuffer_tcbuffer,
    EVER);
}

PGDLLEXPORT Datum Acovers_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acovers_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a geometry always covers a temporal circular buffer
 * @sqlfn aCovers()
 */
inline Datum
Acovers_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_cbuffer_tcbuffer(fcinfo, &ea_covers_cbuffer_tcbuffer,
    ALWAYS);
}

PGDLLEXPORT Datum Ecovers_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ecovers_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer ever covers a geometry
 * @sqlfn eCovers()
 */
inline Datum
Ecovers_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tcbuffer_cbuffer(fcinfo, &ea_covers_tcbuffer_cbuffer,
    EVER);
}

PGDLLEXPORT Datum Acovers_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acovers_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer always covers a geometry
 * @sqlfn aCovers()
 */
inline Datum
Acovers_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tcbuffer_cbuffer(fcinfo, &ea_covers_tcbuffer_cbuffer,
    ALWAYS);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ecovers_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ecovers_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a circular buffer 
 * ever touch
 * @sqlfn eTouches()
 */
inline Datum
Ecovers_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_covers_tcbuffer_tcbuffer,
    EVER);
}

PGDLLEXPORT Datum Acovers_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acovers_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a circular buffer 
 * always touch
 * @sqlfn aTouches()
 */
inline Datum
Acovers_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_covers_tcbuffer_tcbuffer,
    ALWAYS);
}
/*****************************************************************************
 * Ever/always disjoint
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
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_disjoint_geo_tcbuffer, EVER);
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
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_disjoint_geo_tcbuffer, ALWAYS);
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
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_disjoint_tcbuffer_geo, EVER);
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
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_disjoint_tcbuffer_geo, ALWAYS);
}

/*****************************************************************************/

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
  return EA_spatialrel_cbuffer_tcbuffer(fcinfo, &ea_disjoint_cbuffer_tcbuffer,
    EVER);
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
  return EA_spatialrel_cbuffer_tcbuffer(fcinfo, &ea_disjoint_cbuffer_tcbuffer,
    ALWAYS);
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
  return EA_spatialrel_tcbuffer_cbuffer(fcinfo, &ea_disjoint_tcbuffer_cbuffer,
    EVER);
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
  return EA_spatialrel_tcbuffer_cbuffer(fcinfo, &ea_disjoint_tcbuffer_cbuffer,
    ALWAYS);
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
  return EA_spatialrel_tspatial_tspatial(fcinfo,
    &ea_disjoint_tcbuffer_tcbuffer, EVER);
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
  return EA_spatialrel_tspatial_tspatial(fcinfo,
    &ea_disjoint_tcbuffer_tcbuffer, ALWAYS);
}

/*****************************************************************************
 * Ever/always intersects
 *****************************************************************************/

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
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_intersects_geo_tcbuffer,
    EVER);
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
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_intersects_geo_tcbuffer,
    ALWAYS);
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
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_intersects_tcbuffer_geo,
    EVER);
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
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_intersects_tcbuffer_geo,
    ALWAYS);
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
  return EA_spatialrel_cbuffer_tcbuffer(fcinfo,
    &ea_intersects_cbuffer_tcbuffer, EVER);
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
  return EA_spatialrel_cbuffer_tcbuffer(fcinfo,
    &ea_intersects_cbuffer_tcbuffer, ALWAYS);
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
  return EA_spatialrel_tcbuffer_cbuffer(fcinfo,
    &ea_intersects_tcbuffer_cbuffer, EVER);
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
  return EA_spatialrel_tcbuffer_cbuffer(fcinfo,
    &ea_intersects_tcbuffer_cbuffer, ALWAYS);
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
  return EA_spatialrel_tspatial_tspatial(fcinfo,
    &ea_intersects_tcbuffer_tcbuffer, EVER);
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
  return EA_spatialrel_tspatial_tspatial(fcinfo, 
    &ea_intersects_tcbuffer_tcbuffer, ALWAYS);
}

/*****************************************************************************
 * Ever/always touches
 *****************************************************************************/

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
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_touches_geo_tcbuffer, EVER);
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
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_touches_geo_tcbuffer, EVER);
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
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_touches_tcbuffer_geo, EVER);
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
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_touches_tcbuffer_geo, ALWAYS);
}

/*****************************************************************************/

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
  return EA_spatialrel_cbuffer_tcbuffer(fcinfo, &ea_touches_cbuffer_tcbuffer,
    EVER);
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
  return EA_spatialrel_cbuffer_tcbuffer(fcinfo, &ea_touches_cbuffer_tcbuffer,
    ALWAYS);
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
  return EA_spatialrel_tcbuffer_cbuffer(fcinfo, &ea_touches_tcbuffer_cbuffer,
    EVER);
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
  return EA_spatialrel_tcbuffer_cbuffer(fcinfo, &ea_touches_tcbuffer_cbuffer,
    ALWAYS);
}

/*****************************************************************************/

PGDLLEXPORT Datum Etouches_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a circular buffer 
 * ever touch
 * @sqlfn eTouches()
 */
inline Datum
Etouches_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_touches_tcbuffer_tcbuffer,
    EVER);
}

PGDLLEXPORT Datum Atouches_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_ever
 * @brief Return true if a temporal circular buffer and a circular buffer 
 * always touch
 * @sqlfn aTouches()
 */
inline Datum
Atouches_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_touches_tcbuffer_tcbuffer,
    ALWAYS);
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
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = ever ?
    edwithin_tcbuffer_cbuffer(temp, cb, dist) :
    adwithin_tcbuffer_cbuffer(temp, cb, dist);
  PG_FREE_IF_COPY(cb, 0);
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
Datum
EA_dwithin_tcbuffer_cbuffer(FunctionCallInfo fcinfo, bool ever)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(2);
  int result = ever ? edwithin_tcbuffer_cbuffer(temp, cb, dist) :
    adwithin_tcbuffer_cbuffer(temp, cb, dist);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(cb, 1);
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
