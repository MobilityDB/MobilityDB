/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief PostgreSQL wrappers for the static-geometry → H3 cell helpers
 * declared in meos/include/meos_h3.h.
 *
 * Calls geo_to_h3index_set() and ever_eq_anyof_h3indexset_th3index() in
 * the MEOS core; thin shims for argument extraction and result return.
 */

#include <postgres.h>
#include <fmgr.h>

#include <meos.h>
#include <meos_geo.h>
#include <meos_h3.h>

#include "temporal/set.h"             /* PG_GETARG_SET_P / PG_RETURN_SET_P */
#include "pg_temporal/temporal.h"     /* PG_GETARG_TEMPORAL_P */
#include "pg_geo/postgis.h"           /* PG_GETARG_GSERIALIZED_P */

PGDLLEXPORT Datum Geo_gs_point_to_h3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_gs_point_to_h3index);
/**
 * @ingroup mobilitydb_h3_conversion
 * @brief Single H3 cell covering a POINT geometry at the given resolution
 * @sqlfn geoToH3Cell(geometry, integer)
 */
Datum
Geo_gs_point_to_h3index(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  int32 resolution = PG_GETARG_INT32(1);
  H3Index cell = h3_gs_point_to_cell(gs, resolution);
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
 * @sqlfn geoToH3IndexSet(geometry, integer)
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

PGDLLEXPORT Datum Ever_eq_anyof_h3indexset_th3index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ever_eq_anyof_h3indexset_th3index);
/**
 * @ingroup mobilitydb_h3_comp
 * @brief True iff the th3index value sequence ever lies in any cell of
 *        the candidate set
 *
 * Cross-platform spatial prefilter consumer: typical use is
 * `everIntersectsH3IndexSet_Th3Index(geoToH3IndexSet(p.geom, 7), t.trip_h3)`
 * before the exact spatial predicate.
 *
 * @sqlfn everIntersectsH3IndexSet_Th3Index(h3indexset, th3index)
 */
Datum
Ever_eq_anyof_h3indexset_th3index(PG_FUNCTION_ARGS)
{
  Set *cells = PG_GETARG_SET_P(0);
  Temporal *th3idx = PG_GETARG_TEMPORAL_P(1);
  int r = ever_eq_anyof_h3indexset_th3index(cells, th3idx);
  PG_FREE_IF_COPY(cells, 0);
  PG_FREE_IF_COPY(th3idx, 1);
  if (r < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(r == 1);
}
