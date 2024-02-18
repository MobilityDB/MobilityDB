/*****************************************************************************
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
 * @brief Network-based static point and segment types.
 *
 * Several functions are commented out since they are not currently used.
 * They are kept if needed in the future.
 */

#include "npoint/tnpoint_static.h"

/* PostgreSQL */
#include <libpq/pqformat.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h"
#include "general/tnumber_mathfuncs.h"
#include "general/type_out.h"
#include "general/type_round.h"
#include "npoint/tnpoint_static.h"
/* MobilityDB */
#include "pg_point/postgis.h"

/*****************************************************************************
 * Input/Output functions for network point
 *****************************************************************************/

 /*****************************************************************************
 * Send/receive functions
 *****************************************************************************/

/**
 * @brief Return a network point from its binary representation read
 * from a buffer
 */
Npoint *
npoint_recv(StringInfo buf)
{
  Npoint *result = palloc0(sizeof(Npoint));
  result->rid = pq_getmsgint64(buf);
  result->pos = pq_getmsgfloat8(buf);
  return result;
}

/**
 * @brief Return the binary representation of a network point
 * @param[in] np Network point
 */
bytea *
npoint_send(const Npoint *np)
{
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendint64(&buf, (uint64) np->rid);
  pq_sendfloat8(&buf, np->pos);
  return pq_endtypsend(&buf);
}

/**
 * @brief Receive function for network segments
 */
Nsegment *
nsegment_recv(StringInfo buf)
{
  Nsegment *result = palloc0(sizeof(Nsegment));
  result->rid = pq_getmsgint64(buf);
  result->pos1 = pq_getmsgfloat8(buf);
  result->pos2 = pq_getmsgfloat8(buf);
  return result;
}

/**
 * @brief Send function for network segments
 */
bytea *
nsegment_send(const Nsegment *ns)
{
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendint64(&buf, (uint64) ns->rid);
  pq_sendfloat8(&buf, ns->pos1);
  pq_sendfloat8(&buf, ns->pos2);
  return pq_endtypsend(&buf);
}

/*****************************************************************************/

PGDLLEXPORT Datum Npoint_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_in);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return a network point from its Well-Known Text (WKT) representation
 *
 * Example of input:
 * @code
 *    (1, 0.5)
 * @endcode
 * @sqlfn npoint_in()
 */
Datum
Npoint_in(PG_FUNCTION_ARGS)
{
  const char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_NPOINT_P(npoint_in(str, true));
}

PGDLLEXPORT Datum Npoint_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_out);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a a network point
 * @sqlfn npoint_out()
 */
Datum
Npoint_out(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_CSTRING(npoint_out(np, OUT_DEFAULT_DECIMAL_DIGITS));
}

PGDLLEXPORT Datum Npoint_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_recv);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return a network point from its Well-Known Binary (WKB)
 * representation
 * @sqlfn npoint_recv()
 */
Datum
Npoint_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PG_RETURN_NPOINT_P(npoint_recv(buf));
}

PGDLLEXPORT Datum Npoint_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_send);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Well-Known Binary (WKB) representation of a a network
 * point
 * @sqlfn npoint_send()
 */
Datum
Npoint_send(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_BYTEA_P(npoint_send(np));
}

/*****************************************************************************
 * Input/Output functions for network segment
 *****************************************************************************/

PGDLLEXPORT Datum Nsegment_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_in);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return a network segment from its Well-Known Text (WKT)
 * representation
 *
 * Example of input:
 * @code
 *    (1, 0.5, 0.6)
 * @endcode
 * @sqlfn nsegment_in()
 */
Datum
Nsegment_in(PG_FUNCTION_ARGS)
{
  const char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_NSEGMENT_P(nsegment_in(str));
}

PGDLLEXPORT Datum Nsegment_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_out);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a network segment
 * @sqlfn nsegment_out()
 */
Datum
Nsegment_out(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_CSTRING(nsegment_out(ns, OUT_DEFAULT_DECIMAL_DIGITS));
}

PGDLLEXPORT Datum Nsegment_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_recv);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return a network segment from its Well-Known Binary (WKB)
 * representation
 * @sqlfn nsegment_recv()
 */
Datum
Nsegment_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PG_RETURN_NSEGMENT_P(nsegment_recv(buf));
}

PGDLLEXPORT Datum Nsegment_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_send);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Well-Known Binary (WKB) representation of a network
 * segment
 * @sqlfn nsegment_send()
 */
Datum
Nsegment_send(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_BYTEA_P(nsegment_send(ns));
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PGDLLEXPORT Datum Npoint_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_constructor);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a network segment from a route identifier and a position
 * @sqlfn npoint()
 */
Datum
Npoint_constructor(PG_FUNCTION_ARGS)
{
  int64 rid = PG_GETARG_INT64(0);
  double pos = PG_GETARG_FLOAT8(1);
  PG_RETURN_NPOINT_P(npoint_make(rid, pos));
}

PGDLLEXPORT Datum Nsegment_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_constructor);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a network segment from a route identifier and two positions
 * @sqlfn nsegment()
 */
Datum
Nsegment_constructor(PG_FUNCTION_ARGS)
{
  int64 rid = PG_GETARG_INT64(0);
  double pos1 = PG_GETARG_FLOAT8(1);
  double pos2 = PG_GETARG_FLOAT8(2);
  PG_RETURN_NSEGMENT_P(nsegment_make(rid, pos1, pos2));
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Npoint_to_nsegment(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_to_nsegment);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a network point converted to a network segment
 * @sqlfn nsegment()
 * @sqlop @p ::
 */
Datum
Npoint_to_nsegment(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_NSEGMENT_P(npoint_to_nsegment(np));
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Npoint_route(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_route);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the route of a network point
 * @sqlfn route()
 */
Datum
Npoint_route(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_INT64(npoint_route(np));
}

PGDLLEXPORT Datum Npoint_position(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_position);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the position of a network point
 * @sqlfn position()
 */
Datum
Npoint_position(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_FLOAT8(npoint_position(np));
}

PGDLLEXPORT Datum Nsegment_route(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_route);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the route of a network segment
 * @sqlfn route()
 */
Datum
Nsegment_route(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_INT64(nsegment_route(ns));
}

PGDLLEXPORT Datum Nsegment_start_position(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_start_position);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the start position of a network segment
 * @sqlfn startPosition()
 */
Datum
Nsegment_start_position(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_FLOAT8(nsegment_start_position(ns));
}

PGDLLEXPORT Datum Nsegment_end_position(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_end_position);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the end position of a network segment
 * @sqlfn endPosition()
 */
Datum
Nsegment_end_position(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_FLOAT8(nsegment_end_position(ns));
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Npoint_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_round);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a network point with the precision of the position set to a
 * number of decimal places
 * @sqlfn round()
 */
Datum
Npoint_round(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Datum size = PG_GETARG_DATUM(1);
  PG_RETURN_NPOINT_P(npoint_round(np, size));
}

PGDLLEXPORT Datum Nsegment_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_round);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a network segment with the precision of the positions set to a
 * number of decimal places
 * @sqlfn round()
 */
Datum
Nsegment_round(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  Datum size = PG_GETARG_DATUM(1);
  PG_RETURN_NSEGMENT_P(nsegment_round(ns, size));
}

/*****************************************************************************
 * Conversions between network and Euclidean space
 *****************************************************************************/

PGDLLEXPORT Datum Npoint_to_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_to_geom);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a network point converted to a geometry
 * @sqlfn geometry()
 * @sqlop @p ::
 */
Datum
Npoint_to_geom(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_GSERIALIZED_P(npoint_geom(np));
}

PGDLLEXPORT Datum Geom_to_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_to_npoint);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a geometry converted to a network point
 * @sqlfn npoint()
 * @sqlop @p ::
 */
Datum
Geom_to_npoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Npoint *result = geom_npoint(gs);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_NPOINT_P(result);
}

PGDLLEXPORT Datum Nsegment_to_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_to_geom);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a network segment converted to a geometry
 * @sqlfn geometry()
 * @sqlop @p ::
 */
Datum
Nsegment_to_geom(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_GSERIALIZED_P(nsegment_geom(ns));
}

PGDLLEXPORT Datum Geom_to_nsegment(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_to_nsegment);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a geometry converted to a network segment
 * @sqlfn nsegment()
 * @sqlop @p ::
 */
Datum
Geom_to_nsegment(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Nsegment *result = geom_nsegment(gs);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_NSEGMENT_P(result);
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

PGDLLEXPORT Datum Npoint_get_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_get_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the SRID of a network point
 * @sqlfn SRID()
 */
Datum
Npoint_get_srid(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  int result = npoint_srid(np);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Nsegment_get_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_get_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the SRID of a network segment
 * @sqlfn SRID()
 */
Datum
Nsegment_get_srid(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  int result = nsegment_srid(ns);
  PG_RETURN_INT32(result);
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

PGDLLEXPORT Datum Npoint_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_eq);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first network point is equal to the second one
 * @sqlfn npoint_eq()
 * @sqlop @p =
 */
Datum
Npoint_eq(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_eq(np1, np2));
}

PGDLLEXPORT Datum Npoint_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_ne);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first network point is not equal to the second one
 * @sqlfn npoint_ne()
 * @sqlop @p <>
 */
Datum
Npoint_ne(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_ne(np1, np2));
}

PGDLLEXPORT Datum Npoint_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_cmp);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return -1, 0, or 1 depending on whether the first network point
 * is less than, equal to, or greater than the second one
 * @note Function used for B-tree comparison
 * @sqlfn npoint_cmp()
 */
Datum
Npoint_cmp(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_INT32(npoint_cmp(np1, np2));
}

PGDLLEXPORT Datum Npoint_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_lt);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first network point is less than the second one
 * @sqlfn npoint_lt()
 * @sqlop @p <
 */
Datum
Npoint_lt(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_lt(np1, np2));
}

PGDLLEXPORT Datum Npoint_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_le);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first network point is less than or equal to the
 * second one
 * @sqlfn npoint_le()
 * @sqlop @p <=
 */
Datum
Npoint_le(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_le(np1, np2));
}

PGDLLEXPORT Datum Npoint_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_ge);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first network point is greater than or equal to the
 * second one
 * @sqlfn npoint_ge()
 * @sqlop @p >=
 */
Datum
Npoint_ge(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_ge(np1, np2));
}

PGDLLEXPORT Datum Npoint_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_gt);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first network point is greater than the second one
 * @sqlfn npoint_gt()
 * @sqlop @p >
 */
Datum
Npoint_gt(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_gt(np1, np2));
}

/*****************************************************************************/

PGDLLEXPORT Datum Nsegment_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_eq);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first network segment is equal to the second one
 * @sqlfn nsegment_eq()
 * @sqlop @p =
 */
Datum
Nsegment_eq(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_eq(ns1, ns2));
}

PGDLLEXPORT Datum Nsegment_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_ne);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first network segment is not equal to the second one
 * @sqlfn nsegment_ne()
 * @sqlop @p <>
 */
Datum
Nsegment_ne(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_ne(ns1, ns2));
}

PGDLLEXPORT Datum Nsegment_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_cmp);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return -1, 0, or 1 depending on whether the first network segment
 * is less than, equal to, or greater than the second one
 * @sqlfn nsegment_cmp()
 */
Datum
Nsegment_cmp(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_INT32(nsegment_cmp(ns1, ns2));
}

/* Inequality operators using the nsegment_cmp function */

PGDLLEXPORT Datum Nsegment_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_lt);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first network segment is less than the second one
 * @sqlfn nsegment_lt()
 * @sqlop @p <
 */
Datum
Nsegment_lt(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_lt(ns1, ns2));
}

PGDLLEXPORT Datum Nsegment_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_le);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first network segment is less than or equal to the
 * second one
 * @sqlfn nsegment_le()
 * @sqlop @p <=
 */
Datum
Nsegment_le(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_le(ns1, ns2));
}

PGDLLEXPORT Datum Nsegment_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_ge);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first network segment is greater than or equal to the
 * second one
 * @sqlfn nsegment_ge()
 * @sqlop @p >=
 */
Datum
Nsegment_ge(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_ge(ns1, ns2));
}

PGDLLEXPORT Datum Nsegment_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_gt);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first network segment is greater than the second one
 * @sqlfn nsegment_gt()
 * @sqlop @p >
 */
Datum
Nsegment_gt(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_gt(ns1, ns2));
}

/*****************************************************************************/
