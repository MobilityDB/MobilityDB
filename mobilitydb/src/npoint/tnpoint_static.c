/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @file tnpoint_static.c
 * @brief Network-based static point and segment types.
 *
 * Several functions are commented out since they are not currently used.
 * They are kept if needed in the future.
 */

#include "npoint/tnpoint_static.h"

/* PostgreSQL */
#include <libpq/pqformat.h>
#include <executor/spi.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include "general/temporal_util.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/tnumber_mathfuncs.h"
#include "pg_npoint/tnpoint_static.h"

/*****************************************************************************
 * Send/receive functions
 *****************************************************************************/

/**
 * @brief Return a network point from its binary representation read
 * from a buffer.
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
 *
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

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @brief Set the precision of the position of a network point to the number of
 * decimal places
 * @note Funcion used by the lifting infrastructure
 */
Datum
datum_npoint_round(Datum npoint, Datum size)
{
  /* Set precision of position */
  Npoint *np = (Npoint *) DatumGetPointer(npoint);
  Npoint *result = npoint_round(np, size);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the position of a network point to the number of
 * decimal places
 */
Npoint *
npoint_round(const Npoint *np, Datum size)
{
  /* Set precision of position */
  double pos = DatumGetFloat8(datum_round_float(Float8GetDatum(np->pos), size));
  Npoint *result = npoint_make(np->rid, pos);
  return result;
}

/**
 * @brief Set the precision of the position of a network point to the number of
 * decimal places
 */
Nsegment *
nsegment_round(const Nsegment *ns, Datum size)
{
  /* Set precision of positions */
  double pos1 = DatumGetFloat8(datum_round_float(Float8GetDatum(ns->pos1),
    size));
  double pos2 = DatumGetFloat8(datum_round_float(Float8GetDatum(ns->pos2),
    size));
  Nsegment *result = nsegment_make(ns->rid, pos1, pos2);
  return result;
}

/*****************************************************************************
 * Input/Output functions for network point
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Npoint_in);
/**
 * Input function for network points
 * Example of input:
 *    (1, 0.5)
 */
PGDLLEXPORT Datum
Npoint_in(PG_FUNCTION_ARGS)
{
  char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_POINTER(npoint_in(str, true));
}

PG_FUNCTION_INFO_V1(Npoint_out);
/**
 * Output function for network points
 */
PGDLLEXPORT Datum
Npoint_out(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_CSTRING(npoint_out(np));
}

PG_FUNCTION_INFO_V1(Npoint_recv);
/**
 * Receive function for network points
 */
PGDLLEXPORT Datum
Npoint_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PG_RETURN_POINTER(npoint_recv(buf));
}

PG_FUNCTION_INFO_V1(Npoint_send);
/**
 * Send function for network points
 */
PGDLLEXPORT Datum
Npoint_send(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_BYTEA_P(npoint_send(np));
}

/*****************************************************************************
 * Input/Output functions for network segment
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Nsegment_in);
/**
 * Input function for network segments
 * Example of input:
 *    (1, 0.5, 0.6)
 */
PGDLLEXPORT Datum
Nsegment_in(PG_FUNCTION_ARGS)
{
  char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_POINTER(nsegment_in(str));
}

PG_FUNCTION_INFO_V1(Nsegment_out);
/**
 * Output function for network segments
 */
PGDLLEXPORT Datum
Nsegment_out(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_CSTRING(nsegment_out(ns));
}

PG_FUNCTION_INFO_V1(Nsegment_recv);
/**
 * Receive function for network segments
 */
PGDLLEXPORT Datum
Nsegment_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PG_RETURN_POINTER(nsegment_recv(buf));
}

PG_FUNCTION_INFO_V1(Nsegment_send);
/**
 * Send function for network segments
 */
PGDLLEXPORT Datum
Nsegment_send(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_BYTEA_P(nsegment_send(ns));
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Npoint_constructor);
/**
 * Construct a network segment from the arguments
 */
PGDLLEXPORT Datum
Npoint_constructor(PG_FUNCTION_ARGS)
{
  int64 rid = PG_GETARG_INT64(0);
  double pos = PG_GETARG_FLOAT8(1);
  PG_RETURN_POINTER(npoint_make(rid, pos));
}

PG_FUNCTION_INFO_V1(Nsegment_constructor);
/**
 * Construct a network segment from the arguments
 */
PGDLLEXPORT Datum
Nsegment_constructor(PG_FUNCTION_ARGS)
{
  int64 rid = PG_GETARG_INT64(0);
  double pos1 = PG_GETARG_FLOAT8(1);
  double pos2 = PG_GETARG_FLOAT8(2);
  PG_RETURN_POINTER(nsegment_make(rid, pos1, pos2));
}

PG_FUNCTION_INFO_V1(Npoint_to_nsegment);
/**
 * Cast a network segment from a network point
 */
PGDLLEXPORT Datum
Npoint_to_nsegment(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_POINTER(npoint_to_nsegment(np));
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Npoint_route);
/**
 * Return the route of the network point
 */
PGDLLEXPORT Datum
Npoint_route(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_INT64(npoint_route(np));
}

PG_FUNCTION_INFO_V1(Npoint_position);
/**
 * Return the position of the network point
 */
PGDLLEXPORT Datum
Npoint_position(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_FLOAT8(npoint_position(np));
}

PG_FUNCTION_INFO_V1(Nsegment_route);
/**
 * Return the route of the network segment
 */
PGDLLEXPORT Datum
Nsegment_route(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_INT64(nsegment_route(ns));
}

PG_FUNCTION_INFO_V1(Nsegment_start_position);
/**
 * Return the start position of the network segment
 */
PGDLLEXPORT Datum
Nsegment_start_position(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_FLOAT8(nsegment_start_position(ns));
}

PG_FUNCTION_INFO_V1(Nsegment_end_position);
/**
 * Return the end position of the network segment
 */
PGDLLEXPORT Datum
Nsegment_end_position(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_FLOAT8(nsegment_end_position(ns));
}

PG_FUNCTION_INFO_V1(Npoint_round);
/**
 * Set the precision of the position of a network point to the number of
 * decimal places
 */
PGDLLEXPORT Datum
Npoint_round(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Datum size = PG_GETARG_DATUM(1);
  PG_RETURN_POINTER(npoint_round(np, size));
}

PG_FUNCTION_INFO_V1(Nsegment_round);
/**
 * Set the precision of the position of a network point to the number of
 * decimal places
 */
PGDLLEXPORT Datum
Nsegment_round(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  Datum size = PG_GETARG_DATUM(1);
  PG_RETURN_POINTER(nsegment_round(ns, size));
}

/*****************************************************************************
 * Conversions between network and Euclidean space
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Npoint_to_geom);
/**
 * Transforms the network point into a geometry
 */
PGDLLEXPORT Datum
Npoint_to_geom(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  GSERIALIZED *result = npoint_geom(np);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Geom_to_npoint);
/**
 * Transforms the geometry into a network point
 */
PGDLLEXPORT Datum
Geom_to_npoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Npoint *result = geom_npoint(PointerGetDatum(gs));
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Nsegment_to_geom);
/**
 * Transforms the network segment into a geometry
 */
PGDLLEXPORT Datum
Nsegment_to_geom(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  GSERIALIZED *result = nsegment_geom(ns);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Geom_to_nsegment);
/**
 * Transforms the geometry into a network segment
 */
PGDLLEXPORT Datum
Geom_to_nsegment(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Nsegment *result = geom_nsegment(PointerGetDatum(gs));
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Npoint_get_srid);
/**
 * Return the SRID of the network point
 */
PGDLLEXPORT Datum
Npoint_get_srid(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  int result = npoint_srid(np);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(Nsegment_get_srid);
/**
 * Return the SRID of the network segment
 */
PGDLLEXPORT Datum
Nsegment_get_srid(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  int result = nsegment_srid(ns);
  PG_RETURN_INT32(result);
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Npoint_eq);
/**
 * Return true if the first network point is equal to the second one
 */
PGDLLEXPORT Datum
Npoint_eq(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_eq(np1, np2));
}

PG_FUNCTION_INFO_V1(Npoint_ne);
/**
 * Return true if the first network point is not equal to the second one
 */
PGDLLEXPORT Datum
Npoint_ne(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_ne(np1, np2));
}

PG_FUNCTION_INFO_V1(Npoint_cmp);
/**
 * Return -1, 0, or 1 depending on whether the first network point
 * is less than, equal, or greater than the second one
 *
 * @note Function used for B-tree comparison
 */
PGDLLEXPORT Datum
Npoint_cmp(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_INT32(npoint_cmp(np1, np2));
}

PG_FUNCTION_INFO_V1(Npoint_lt);
/**
 * Return true if the first network point is less than the second one
 */
PGDLLEXPORT Datum
Npoint_lt(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_lt(np1, np2));
}

PG_FUNCTION_INFO_V1(Npoint_le);
/**
 * Return true if the first network point is less than or equal to the
 * second one
 */
PGDLLEXPORT Datum
Npoint_le(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_le(np1, np2));
}

PG_FUNCTION_INFO_V1(Npoint_ge);
/**
 * Return true if the first network point is greater than or equal to the
 * second one
 */
PGDLLEXPORT Datum
Npoint_ge(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_ge(np1, np2));
}

PG_FUNCTION_INFO_V1(Npoint_gt);
/**
 * Return true if the first network point is greater than the second one
 */
PGDLLEXPORT Datum
Npoint_gt(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_gt(np1, np2));
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Nsegment_eq);
/**
 * Return true if the first network segment is equal to the second one
 */
PGDLLEXPORT Datum
Nsegment_eq(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_eq(ns1, ns2));
}

PG_FUNCTION_INFO_V1(Nsegment_ne);
/**
 * Return true if the first network segment is not equal to the second one
 */
PGDLLEXPORT Datum
Nsegment_ne(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_ne(ns1, ns2));
}

PG_FUNCTION_INFO_V1(Nsegment_cmp);
/**
 * Return -1, 0, or 1 depending on whether the first network segment
 * is less than, equal, or greater than the second one
 */
PGDLLEXPORT Datum
Nsegment_cmp(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_INT32(nsegment_cmp(ns1, ns2));
}

/* Inequality operators using the nsegment_cmp function */

PG_FUNCTION_INFO_V1(Nsegment_lt);
/**
 * Return true if the first network segment is less than the second one
 */
PGDLLEXPORT Datum
Nsegment_lt(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_lt(ns1, ns2));
}

PG_FUNCTION_INFO_V1(Nsegment_le);
/**
 * Return true if the first network segment is less than or equal to the
 * second one
 */
PGDLLEXPORT Datum
Nsegment_le(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_le(ns1, ns2));
}

PG_FUNCTION_INFO_V1(Nsegment_ge);
/**
 * Return true if the first network segment is greater than or equal to the
 * second one
 */
PGDLLEXPORT Datum
Nsegment_ge(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_ge(ns1, ns2));
}

PG_FUNCTION_INFO_V1(Nsegment_gt);
/**
 * Return true if the first network segment is greater than the second one
 */
PGDLLEXPORT Datum
Nsegment_gt(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_gt(ns1, ns2));
}

/*****************************************************************************/
