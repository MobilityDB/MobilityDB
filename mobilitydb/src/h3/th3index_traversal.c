/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief PG V1 wrappers for th3index grid-traversal functions.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_h3.h>
#include "temporal/temporal.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"

/*****************************************************************************
 * h3_grid_distance — also the procedure behind the `<->` operator.
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_grid_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_grid_distance);
/**
 * @ingroup mobilitydb_h3_traversal
 * @brief Return the temporal grid-hop distance between two temporal H3 cells
 * @sqlfn h3_grid_distance()
 * @sqlop @p \<->
 */
Datum
Th3index_grid_distance(PG_FUNCTION_ARGS)
{
  Temporal *origin = PG_GETARG_TEMPORAL_P(0);
  Temporal *dest = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = th3index_grid_distance(origin, dest);
  PG_FREE_IF_COPY(origin, 0);
  PG_FREE_IF_COPY(dest, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_cell_to_local_ij
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_cell_to_local_ij(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_cell_to_local_ij);
/**
 * @ingroup mobilitydb_h3_traversal
 * @brief Return the temporal local (I, J) coordinate pair of a cell with
 * respect to an origin cell, as a temporal planar point
 * @sqlfn h3_cell_to_local_ij()
 */
Datum
Th3index_cell_to_local_ij(PG_FUNCTION_ARGS)
{
  Temporal *origin = PG_GETARG_TEMPORAL_P(0);
  Temporal *cell = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = th3index_cell_to_local_ij(origin, cell);
  PG_FREE_IF_COPY(origin, 0);
  PG_FREE_IF_COPY(cell, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_local_ij_to_cell
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_local_ij_to_cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_local_ij_to_cell);
/**
 * @ingroup mobilitydb_h3_traversal
 * @brief Return the temporal H3 cell at the given local (I, J) coordinate
 * relative to an origin cell
 * @sqlfn h3_local_ij_to_cell()
 */
Datum
Th3index_local_ij_to_cell(PG_FUNCTION_ARGS)
{
  Temporal *origin = PG_GETARG_TEMPORAL_P(0);
  Temporal *coord = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = th3index_local_ij_to_cell(origin, coord);
  PG_FREE_IF_COPY(origin, 0);
  PG_FREE_IF_COPY(coord, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
