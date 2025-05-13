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
 * @brief Temporal spatial relationships for temporal circular buffers
 * @details These relationships are applied at each instant and result in a
 * temporal Boolean.
 *
 * Depending on the parameters various relationships are available
 * - For a temporal geometry geo and a geometry:
 *   `tcontains`, `tdisjoint`, `tintersects`, `ttouches`, and `tdwithin`.
 * - For two temporal **geometries**:
 *   `tdisjoint`, `tintersects`, `tdwithin`.
 */

#include "geo/tgeo_tempspatialrels.h"

/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_cbuffer.h>
#include "cbuffer/cbuffer.h"
#include "cbuffer/tcbuffer_tempspatialrels.h"
/* MobilityDB */
#include "pg_geo/postgis.h"
#include "pg_geo/tspatial.h"

/*****************************************************************************
 * Generic functions for computing the spatiotemporal relationships of
 * temporal circular buffers
 *****************************************************************************/

/**
 * @brief Return the spatiotemporal relationship between a circular buffer and
 * a temporal circular buffer
 */
static Datum
Tinterrel_cbuffer_tcbuffer(FunctionCallInfo fcinfo, bool tinter)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tcbuffer_cbuffer(temp, cb, tinter, restr,
    atvalue);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Return the spatiotemporal relationship between a temporal circular
 * buffer and a circular buffer
 */
static Datum
Tinterrel_tcbuffer_cbuffer(FunctionCallInfo fcinfo, bool tinter)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Cbuffer *cb = PG_GETARG_CBUFFER_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tcbuffer_cbuffer(temp, cb, tinter, restr,
    atvalue);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Return the spatiotemporal relationship between a geometry and a
 * temporal circular buffer
 */
static Datum
Tinterrel_geo_tcbuffer(FunctionCallInfo fcinfo, bool tinter)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tcbuffer_geo(temp, gs, tinter, restr, atvalue);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Return the spatiotemporal relationship between a temporal circular
 * buffer and a geometry
 */
static Datum
Tinterrel_tcbuffer_geo(FunctionCallInfo fcinfo, bool tinter)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tcbuffer_geo(temp, gs, tinter, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Return the spatiotemporal relationship between two temporal circular
 * buffers
 */
static Datum
Tinterrel_tcbuffer_tcbuffer(FunctionCallInfo fcinfo, bool tinter)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tcbuffer_tcbuffer(temp1, temp2, tinter, restr,
    atvalue);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

PGDLLEXPORT Datum Tcontains_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcontains_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a circular buffer
 * contains a temporal circular buffer
 * @sqlfn tContains()
 */
Datum
Tcontains_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = tcontains_cbuffer_tcbuffer(cb, temp, restr, atvalue);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tcontains_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcontains_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * buffer contains a circular buffer
 * @sqlfn tContains()
 */
Datum
Tcontains_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Cbuffer *cb = PG_GETARG_CBUFFER_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = tcontains_tcbuffer_cbuffer(temp, cb, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tcontains_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcontains_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a geometry contains a
 * temporal circular buffer
 * @sqlfn tContains()
 */
Datum
Tcontains_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = tcontains_geo_tcbuffer(gs, temp, restr, atvalue);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tcontains_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcontains_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * buffer contains a geometry
 * @sqlfn tContains()
 */
Datum
Tcontains_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = tcontains_tcbuffer_geo(temp, gs, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tcontains_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcontains_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * buffer contains another one
 * @sqlfn tContains()
 */
Datum
Tcontains_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = tcontains_tcbuffer_tcbuffer(temp1, temp2, restr, atvalue);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal covers
 *****************************************************************************/

PGDLLEXPORT Datum Tcovers_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcovers_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a circular buffer
 * covers a temporal circular buffer
 * @sqlfn tCovers()
 */
Datum
Tcovers_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = tcovers_cbuffer_tcbuffer(cb, temp, restr, atvalue);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tcovers_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcovers_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * buffer covers a circular buffer
 * @sqlfn tCovers()
 */
Datum
Tcovers_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Cbuffer *cb = PG_GETARG_CBUFFER_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = tcovers_tcbuffer_cbuffer(temp, cb, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tcovers_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcovers_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a geometry covers a
 * temporal circular buffer
 * @sqlfn tCovers()
 */
Datum
Tcovers_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = tcovers_geo_tcbuffer(gs, temp, restr, atvalue);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tcovers_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcovers_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * buffer covers a geometry
 * @sqlfn tCovers()
 */
Datum
Tcovers_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = tcovers_tcbuffer_geo(temp, gs, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tcovers_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcovers_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * buffer covers another one
 * @sqlfn tCovers()
 */
Datum
Tcovers_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = tcovers_tcbuffer_tcbuffer(temp1, temp2, restr, atvalue);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

PGDLLEXPORT Datum Tdisjoint_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * buffer is disjoint from a circular buffer
 * @sqlfn tDisjoint()
 */
inline Datum
Tdisjoint_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Tinterrel_cbuffer_tcbuffer(fcinfo, TDISJOINT);
}

PGDLLEXPORT Datum Tdisjoint_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * bufffer is disjoint from a circular buffer
 * @sqlfn tDisjoint()
 */
inline Datum
Tdisjoint_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return Tinterrel_tcbuffer_cbuffer(fcinfo, TDISJOINT);
}

PGDLLEXPORT Datum Tdisjoint_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a geometry is disjoint
 * from a temporal circular buffer 
 * @sqlfn tDisjoint()
 */
inline Datum
Tdisjoint_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  return Tinterrel_geo_tcbuffer(fcinfo, TDISJOINT);
}

PGDLLEXPORT Datum Tdisjoint_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * buffer is disjoint from a geometry
 * @sqlfn tDisjoint()
 */
inline Datum
Tdisjoint_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  return Tinterrel_tcbuffer_geo(fcinfo, TDISJOINT);
}

PGDLLEXPORT Datum Tdisjoint_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether two temporal circular
 * buffers are disjoint
 * @sqlfn tDisjoint()
 */
inline Datum
Tdisjoint_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Tinterrel_tcbuffer_tcbuffer(fcinfo, TDISJOINT);
}

/*****************************************************************************
 * Temporal intersects
 *****************************************************************************/

PGDLLEXPORT Datum Tintersects_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * buffer intersects a circular buffer
 * @sqlfn tIntersects()
 */
inline Datum
Tintersects_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Tinterrel_cbuffer_tcbuffer(fcinfo, TINTERSECTS);
}

PGDLLEXPORT Datum Tintersects_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * buffer intersects a circular buffer
 * @sqlfn tIntersects()
 */
inline Datum
Tintersects_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  return Tinterrel_tcbuffer_cbuffer(fcinfo, TINTERSECTS);
}

PGDLLEXPORT Datum Tintersects_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a geometry intersects a
 * temporal circular buffer  
 * @sqlfn tIntersects()
 */
inline Datum
Tintersects_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  return Tinterrel_geo_tcbuffer(fcinfo, TINTERSECTS);
}

PGDLLEXPORT Datum Tintersects_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * buffer intersects a geometry
 * @sqlfn tIntersects()
 */
inline Datum
Tintersects_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  return Tinterrel_tcbuffer_geo(fcinfo, TINTERSECTS);
}

PGDLLEXPORT Datum Tintersects_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether two temporal circular
 * buffers are disjoint
 * @sqlfn tIntersects()
 */
inline Datum
Tintersects_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Tinterrel_tcbuffer_tcbuffer(fcinfo, TINTERSECTS);
}

/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

PGDLLEXPORT Datum Ttouches_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a circular buffer
 * touches a temporal circular buffer
 * @sqlfn tTouches()
 */
Datum
Ttouches_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = ttouches_tcbuffer_cbuffer(temp, cb, restr, atvalue);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ttouches_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * buffer touches a circular buffer
 * @sqlfn tTouches()
 */
Datum
Ttouches_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Cbuffer *cb = PG_GETARG_CBUFFER_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = ttouches_tcbuffer_cbuffer(temp, cb, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ttouches_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a geometry touches a
 * temporal circular buffer
 * @sqlfn tTouches()
 */
Datum
Ttouches_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = ttouches_tcbuffer_geo(temp, gs, restr, atvalue);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ttouches_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * buffer touches a geometry
 * @sqlfn tTouches()
 */
Datum
Ttouches_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = ttouches_tcbuffer_geo(temp, gs, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ttouches_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * buffer touches another one
 * @sqlfn tTouches()
 */
Datum
Ttouches_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = ttouches_tcbuffer_tcbuffer(temp1, temp2, restr, atvalue);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal dwithin
 *****************************************************************************/

PGDLLEXPORT Datum Tdwithin_cbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_cbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a circular buffer and a
 * temporal circular buffer are within a given distance
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_cbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2))
    PG_RETURN_NULL();
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
  {
    atvalue = PG_GETARG_BOOL(3);
    restr = true;
  }
  Temporal *result = tdwithin_tcbuffer_cbuffer(temp, cb, dist, restr, atvalue);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tdwithin_tcbuffer_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_tcbuffer_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * buffer and a circular buffer are within a given distance
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_tcbuffer_cbuffer(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Cbuffer *cb = PG_GETARG_CBUFFER_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
  {
    atvalue = PG_GETARG_BOOL(3);
    restr = true;
  }
  Temporal *result = tdwithin_tcbuffer_cbuffer(temp, cb, dist, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tdwithin_geo_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_geo_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a geometry and a
 * temporal circular buffer are within a given distance
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_geo_tcbuffer(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2))
    PG_RETURN_NULL();
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
  {
    atvalue = PG_GETARG_BOOL(3);
    restr = true;
  }
  Temporal *result = tdwithin_tcbuffer_geo(temp, gs, dist, restr, atvalue);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tdwithin_tcbuffer_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_tcbuffer_geo);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether a temporal circular
 * buffer and a geometry are within a given distance
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_tcbuffer_geo(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
  {
    atvalue = PG_GETARG_BOOL(3);
    restr = true;
  }
  Temporal *result = tdwithin_tcbuffer_geo(temp, gs, dist, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tdwithin_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_rel_temp
 * @brief Return a temporal boolean that states whether two temporal circular
 * buffers are within a given distance
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2))
    PG_RETURN_NULL();
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
  {
    atvalue = PG_GETARG_BOOL(3);
    restr = true;
  }
  Temporal *result = tdwithin_tcbuffer_tcbuffer(temp1, temp2, dist, restr, atvalue);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
