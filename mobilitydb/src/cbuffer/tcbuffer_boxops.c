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
 * @brief Bounding box operators for temporal circular buffers.
 *
 * These operators test the bounding boxes of temporal circular buffers, which
 * are STBox boxes. The following operators are defined:
 *    overlaps, contains, contained, same
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 */

#include "cbuffer/tcbuffer_boxops.h"

/* PostgreSQL */
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/span.h"
#include "geo/stbox.h"
#include "cbuffer/tcbuffer.h"
/* MobilityDB */
#include "pg_geo/tspatial_boxops.h"

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_to_stbox);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a circular buffer converted to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Cbuffer_to_stbox(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  PG_RETURN_STBOX_P(cbuffer_stbox(cbuf));
}

/*****************************************************************************/

PGDLLEXPORT Datum Cbufferset_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbufferset_to_stbox);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return a circular buffer set converted to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Cbufferset_to_stbox(PG_FUNCTION_ARGS)
{
  Set *set = PG_GETARG_SET_P(0);
  STBox *result = spatialset_stbox(set);
  PG_FREE_IF_COPY(set, 0);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Cbuffer_timestamptz_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_timestamptz_to_stbox);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a circular buffer and a timestamptz to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p
 */
Datum
Cbuffer_timestamptz_to_stbox(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_STBOX_P(cbuffer_timestamptz_to_stbox(cbuf, t));
}

PGDLLEXPORT Datum Cbuffer_tstzspan_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_tstzspan_to_stbox);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a circular buffer and a timestamptz span to a spatiotemporal
 * box
 * @sqlfn stbox()
 * @sqlop @p
 */
Datum
Cbuffer_tstzspan_to_stbox(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_STBOX_P(cbuffer_tstzspan_to_stbox(cbuf, s));
}

/*****************************************************************************/
