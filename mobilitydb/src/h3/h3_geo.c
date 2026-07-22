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
 * @brief PostgreSQL wrappers for the static-geometry → H3 cell helpers
 * declared in meos/include/meos_h3.h.
 *
 * Calls geo_to_h3index_set() and ever_eq_h3indexset_th3index() in
 * the MEOS core; thin shims for argument extraction and result return.
 */

#include <postgres.h>
#include <fmgr.h>
#include <utils/timestamp.h>

#include <meos.h>
#include <meos_geo.h>
#include <meos_h3.h>

#include "geo/stbox.h"                /* PG_RETURN_STBOX_P */
#include "temporal/set.h"             /* PG_GETARG_SET_P / PG_RETURN_SET_P */
#include "temporal/span.h"            /* PG_GETARG_SPAN_P */
#include "pg_temporal/temporal.h"     /* PG_GETARG_TEMPORAL_P */
#include "pg_geo/postgis.h"           /* PG_GETARG_GSERIALIZED_P */

/* h3index is stored on the int8 payload; DatumGetH3Index lives in
 * h3/h3index.h but this file only needs the fmgr-layer getter. */
#define PG_GETARG_H3INDEX(n) ((H3Index) PG_GETARG_INT64(n))

PGDLLEXPORT Datum Geo_point_to_h3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_point_to_h3index);
/**
 * @ingroup mobilitydb_h3_conversion
 * @brief Single H3 cell covering a POINT geometry at the given resolution
 * @sqlfn geoToH3Cell()
 */
Datum
Geo_point_to_h3index(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  int32 resolution = PG_GETARG_INT32(1);
  H3Index cell = geo_to_h3index_cell(gs, resolution);
  PG_FREE_IF_COPY(gs, 0);
  if (cell == (H3Index) 0)
    PG_RETURN_NULL();
  PG_RETURN_INT64((int64) cell);
}

PGDLLEXPORT Datum Geo_to_h3indexset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_to_h3indexset);
/**
 * @ingroup mobilitydb_h3_conversion
 * @brief Set of H3 cells covering a static geometry at the given resolution
 *
 * Accepts POINT, LINESTRING, POLYGON, and MULTI* / GEOMETRYCOLLECTION
 * combinations.  Returns NULL when the geometry produces no valid cells.
 *
 * @sqlfn geoToH3IndexSet()
 */
Datum
Geo_to_h3indexset(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  int32 resolution = PG_GETARG_INT32(1);
  Set *result = geo_to_h3index_set(gs, resolution);
  PG_FREE_IF_COPY(gs, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Ever_eq_h3indexset_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_h3indexset_th3index);
/**
 * @ingroup mobilitydb_h3_comp
 * @brief True iff the th3index value sequence ever lies in any cell of
 *        the candidate set
 *
 * Cross-platform spatial prefilter consumer: typical use is
 * `geoToH3IndexSet(p.geom, 7) ?= t.trip_h3`
 * before the exact spatial predicate.
 *
 * @sqlfn eEq()
 * @sqlop @p ?=
 */
Datum
Ever_eq_h3indexset_th3index(PG_FUNCTION_ARGS)
{
  Set *cells = PG_GETARG_SET_P(0);
  Temporal *th3idx = PG_GETARG_TEMPORAL_P(1);
  int r = ever_eq_h3indexset_th3index(cells, th3idx);
  PG_FREE_IF_COPY(cells, 0);
  PG_FREE_IF_COPY(th3idx, 1);
  if (r < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(r == 1);
}

/*****************************************************************************
 * Bounding box
 *****************************************************************************/

PGDLLEXPORT Datum H3index_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_to_stbox);
/**
 * @ingroup mobilitydb_h3_conversion
 * @brief Return the spatiotemporal bounding box of an H3 cell
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
H3index_to_stbox(PG_FUNCTION_ARGS)
{
  PG_RETURN_STBOX_P(h3index_to_stbox(PG_GETARG_H3INDEX(0)));
}

PGDLLEXPORT Datum H3index_timestamptz_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_timestamptz_to_stbox);
/**
 * @ingroup mobilitydb_h3_conversion
 * @brief Return the spatiotemporal bounding box of an H3 cell and a timestamptz
 * @sqlfn stbox()
 */
Datum
H3index_timestamptz_to_stbox(PG_FUNCTION_ARGS)
{
  H3Index cell = PG_GETARG_H3INDEX(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_STBOX_P(h3index_timestamptz_to_stbox(cell, t));
}

PGDLLEXPORT Datum H3index_tstzspan_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_tstzspan_to_stbox);
/**
 * @ingroup mobilitydb_h3_conversion
 * @brief Return the spatiotemporal bounding box of an H3 cell and a timestamptz
 * span
 * @sqlfn stbox()
 */
Datum
H3index_tstzspan_to_stbox(PG_FUNCTION_ARGS)
{
  H3Index cell = PG_GETARG_H3INDEX(0);
  Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_STBOX_P(h3index_tstzspan_to_stbox(cell, s));
}
