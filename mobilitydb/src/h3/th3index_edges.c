/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief PG V1 wrappers for th3index directed-edge functions.
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
 * h3_are_neighbor_cells
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_are_neighbor_cells(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_are_neighbor_cells);
/**
 * @ingroup mobilitydb_h3_edges
 * @brief Return a temporal boolean stating whether two temporal H3 cells
 * are neighbours at each instant
 * @sqlfn h3_are_neighbor_cells()
 */
Datum
Th3index_are_neighbor_cells(PG_FUNCTION_ARGS)
{
  Temporal *origin = PG_GETARG_TEMPORAL_P(0);
  Temporal *dest = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = th3index_are_neighbor_cells(origin, dest);
  PG_FREE_IF_COPY(origin, 0);
  PG_FREE_IF_COPY(dest, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_cells_to_directed_edge
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_cells_to_directed_edge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_cells_to_directed_edge);
/**
 * @ingroup mobilitydb_h3_edges
 * @brief Return a temporal directed-edge H3 index from origin to destination
 * @sqlfn h3_cells_to_directed_edge()
 */
Datum
Th3index_cells_to_directed_edge(PG_FUNCTION_ARGS)
{
  Temporal *origin = PG_GETARG_TEMPORAL_P(0);
  Temporal *dest = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = th3index_cells_to_directed_edge(origin, dest);
  PG_FREE_IF_COPY(origin, 0);
  PG_FREE_IF_COPY(dest, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_is_valid_directed_edge
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_is_valid_directed_edge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_is_valid_directed_edge);
/**
 * @ingroup mobilitydb_h3_edges
 * @brief Return a temporal boolean stating at each instant whether the
 * value is a valid H3 directed edge
 * @sqlfn h3_is_valid_directed_edge()
 */
Datum
Th3index_is_valid_directed_edge(PG_FUNCTION_ARGS)
{
  Temporal *edge = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_is_valid_directed_edge(edge);
  PG_FREE_IF_COPY(edge, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_get_directed_edge_origin
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_get_directed_edge_origin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_get_directed_edge_origin);
/**
 * @ingroup mobilitydb_h3_edges
 * @brief Return the origin cell of a temporal directed edge
 * @sqlfn h3_get_directed_edge_origin()
 */
Datum
Th3index_get_directed_edge_origin(PG_FUNCTION_ARGS)
{
  Temporal *edge = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_get_directed_edge_origin(edge);
  PG_FREE_IF_COPY(edge, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_get_directed_edge_destination
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_get_directed_edge_destination(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_get_directed_edge_destination);
/**
 * @ingroup mobilitydb_h3_edges
 * @brief Return the destination cell of a temporal directed edge
 * @sqlfn h3_get_directed_edge_destination()
 */
Datum
Th3index_get_directed_edge_destination(PG_FUNCTION_ARGS)
{
  Temporal *edge = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_get_directed_edge_destination(edge);
  PG_FREE_IF_COPY(edge, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * h3_directed_edge_to_boundary
 *****************************************************************************/

PGDLLEXPORT Datum Th3index_directed_edge_to_boundary(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Th3index_directed_edge_to_boundary);
/**
 * @ingroup mobilitydb_h3_edges
 * @brief Return the per-instant polygon boundary of a temporal directed
 * edge as a temporal geography
 * @sqlfn h3_directed_edge_to_boundary()
 */
Datum
Th3index_directed_edge_to_boundary(PG_FUNCTION_ARGS)
{
  Temporal *edge = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = th3index_directed_edge_to_boundary(edge);
  PG_FREE_IF_COPY(edge, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
