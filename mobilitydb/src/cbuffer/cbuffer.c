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
 * @brief Static circular buffer type
 */

#include "cbuffer/tcbuffer.h"

/* PostgreSQL */
#include <libpq/pqformat.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_cbuffer.h>
#include "general/temporal.h"
#include "general/tnumber_mathfuncs.h"
#include "general/type_out.h"
#include "general/type_round.h"
#include "general/type_util.h"
#include "cbuffer/tcbuffer.h"
/* MobilityDB */
#include "pg_general/type_util.h"
#include "pg_point/postgis.h"

/*****************************************************************************
 * Input/Output functions for circular buffers
 *****************************************************************************/

 /*****************************************************************************
 * Send/receive functions
 *****************************************************************************/

/**
 * @brief Return a circular buffer from its binary representation read
 * from a buffer
 */
Cbuffer *
cbuffer_recv(StringInfo buf)
{
  int size = pq_getmsgint(buf, 4);
  StringInfoData cbuf2 =
  {
    .cursor = 0,
    .len = size,
    .maxlen = size,
    .data = buf->data + buf->cursor
  };
  Cbuffer *result = DatumGetCbufferP(call_recv(T_CBUFFER, &cbuf2));
  buf->cursor += size;
  return result;
}

/**
 * @brief Return the binary representation of a circular buffer
 * @param[in] cbuf Circular buffer
 */
bytea *
cbuffer_send(const Cbuffer *cbuf)
{
  StringInfoData buf;
  pq_begintypsend(&buf);
  // pq_sendint64(&buf, (uint64) cbuf->point);
  bytea *bv = call_send(T_CBUFFER, cbuf->point);
  pq_sendbytes(&buf, VARDATA(bv), VARSIZE(bv) - VARHDRSZ);
  pq_sendfloat8(&buf, cbuf->radius);
  return pq_endtypsend(&buf);
}

/*****************************************************************************/

PGDLLEXPORT Datum Cbuffer_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_in);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return a circular buffer from its Well-Known Text (WKT) representation
 *
 * Example of input:
 * @code
 *    Cbuffer(Point(1 1), 10)
 * @endcode
 * @sqlfn cbuffer_in()
 */
Datum
Cbuffer_in(PG_FUNCTION_ARGS)
{
  const char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_CBUFFER_P(cbuffer_in(str));
}

PGDLLEXPORT Datum Cbuffer_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_out);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a circular buffer
 * @sqlfn cbuffer_out()
 */
Datum
Cbuffer_out(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  PG_RETURN_CSTRING(cbuffer_out(cbuf, OUT_DEFAULT_DECIMAL_DIGITS));
}

PGDLLEXPORT Datum Cbuffer_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_recv);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return a circular buffer from its Well-Known Binary (WKB)
 * representation
 * @sqlfn cbuffer_recv()
 */
Datum
Cbuffer_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PG_RETURN_CBUFFER_P(cbuffer_recv(buf));
}

PGDLLEXPORT Datum Cbuffer_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_send);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Well-Known Binary (WKB) representation of a circular buffer
 * @sqlfn cbuffer_send()
 */
Datum
Cbuffer_send(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  PG_RETURN_BYTEA_P(cbuffer_send(cbuf));
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_constructor);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a circular buffer from a point and a radius
 * @sqlfn cbuffer()
 */
Datum
Cbuffer_constructor(PG_FUNCTION_ARGS)
{
  GSERIALIZED *point = PG_GETARG_GSERIALIZED_P(0);
  double rad = PG_GETARG_FLOAT8(1);
  PG_RETURN_CBUFFER_P(cbuffer_make(point, rad));
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_point);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the point of a circular buffer
 * @sqlfn point()
 */
Datum
Cbuffer_point(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  Datum d = PointerGetDatum(&cbuf->point);
  PG_RETURN_DATUM(datum_copy(d, T_GEOMETRY));
}

PGDLLEXPORT Datum Cbuffer_radius(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_radius);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the radius of a circular buffer
 * @sqlfn radius()
 */
Datum
Cbuffer_radius(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  PG_RETURN_FLOAT8(cbuffer_radius(cbuf));
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_get_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_get_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the SRID of a circular buffer
 * @sqlfn SRID()
 */
Datum
Cbuffer_get_srid(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  int result = cbuffer_srid(cbuf);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Cbuffer_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_set_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return a circular buffer with the coordinates of the point set to 
 * an SRID
 * @sqlfn setSRID()
 */
Datum
Cbuffer_set_srid(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  int32_t srid = PG_GETARG_INT32(1);
  Cbuffer *result = cbuffer_set_srid(cbuf, srid);
  PG_RETURN_CBUFFER_P(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_round);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a circular buffer with the precision of the radius set to a
 * number of decimal places
 * @sqlfn round()
 */
Datum
Cbuffer_round(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  Datum size = PG_GETARG_DATUM(1);
  PG_RETURN_CBUFFER_P(cbuffer_round(cbuf, size));
}

/*****************************************************************************
 * Conversions between circular buffer and geometry
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_to_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_to_geom);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a circular buffer converted to a geometry
 * @sqlfn geometry()
 * @sqlop @p ::
 */
Datum
Cbuffer_to_geom(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  PG_RETURN_GSERIALIZED_P(cbuffer_geom(cbuf));
}

PGDLLEXPORT Datum Geom_to_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_to_cbuffer);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a geometry converted to a circular buffer
 * @sqlfn cbuffer()
 * @sqlop @p ::
 */
Datum
Geom_to_cbuffer(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Cbuffer *result = geom_to_cbuffer(gs);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_CBUFFER_P(result);
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_eq);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first circular buffer is equal to the second one
 * @sqlfn cbuffer_eq()
 * @sqlop @p =
 */
Datum
Cbuffer_eq(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cbuf2 = PG_GETARG_CBUFFER_P(1);
  PG_RETURN_BOOL(cbuffer_eq(cbuf1, cbuf2));
}

PGDLLEXPORT Datum Cbuffer_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_ne);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first circular buffer is not equal to the second one
 * @sqlfn cbuffer_ne()
 * @sqlop @p <>
 */
Datum
Cbuffer_ne(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cbuf2 = PG_GETARG_CBUFFER_P(1);
  PG_RETURN_BOOL(cbuffer_ne(cbuf1, cbuf2));
}

PGDLLEXPORT Datum Cbuffer_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_cmp);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return -1, 0, or 1 depending on whether the first circular buffer
 * is less than, equal to, or greater than the second one
 * @note Function used for B-tree comparison
 * @sqlfn cbuffer_cmp()
 */
Datum
Cbuffer_cmp(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cbuf2 = PG_GETARG_CBUFFER_P(1);
  PG_RETURN_INT32(cbuffer_cmp(cbuf1, cbuf2));
}

PGDLLEXPORT Datum Cbuffer_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_lt);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first circular buffer is less than the second one
 * @sqlfn cbuffer_lt()
 * @sqlop @p <
 */
Datum
Cbuffer_lt(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cbuf2 = PG_GETARG_CBUFFER_P(1);
  PG_RETURN_BOOL(cbuffer_lt(cbuf1, cbuf2));
}

PGDLLEXPORT Datum Cbuffer_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_le);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first circular buffer is less than or equal to the
 * second one
 * @sqlfn cbuffer_le()
 * @sqlop @p <=
 */
Datum
Cbuffer_le(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cbuf2 = PG_GETARG_CBUFFER_P(1);
  PG_RETURN_BOOL(cbuffer_le(cbuf1, cbuf2));
}

PGDLLEXPORT Datum Cbuffer_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_ge);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first circular buffer is greater than or equal to
 * the second one
 * @sqlfn cbuffer_ge()
 * @sqlop @p >=
 */
Datum
Cbuffer_ge(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cbuf2 = PG_GETARG_CBUFFER_P(1);
  PG_RETURN_BOOL(cbuffer_ge(cbuf1, cbuf2));
}

PGDLLEXPORT Datum Cbuffer_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_gt);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first circular buffer is greater than the second one
 * @sqlfn cbuffer_gt()
 * @sqlop @p >
 */
Datum
Cbuffer_gt(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cbuf2 = PG_GETARG_CBUFFER_P(1);
  PG_RETURN_BOOL(cbuffer_gt(cbuf1, cbuf2));
}

/*****************************************************************************/
