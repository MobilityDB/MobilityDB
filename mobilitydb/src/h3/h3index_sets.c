/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief PG V1 wrappers for the static set-returning h3 helpers.
 *
 * Every wrapper unpacks arguments via `PG_GETARG_H3INDEX` /
 * `PG_GETARG_SET_P` / `PG_GETARG_INT32`, delegates to the
 * MEOS-layer implementation in `meos/src/h3/h3index_sets.c`, and
 * returns the resulting `Set *` as a SET Datum.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* H3 */
#include <h3api.h>
/* MEOS */
#include <meos.h>
#include "temporal/set.h"
#include "h3/h3index.h"
#include "h3/h3index_sets.h"
/* MobilityDB */
#include "pg_temporal/type_util.h"

#define PG_GETARG_H3INDEX(n) DatumGetH3Index(PG_GETARG_DATUM(n))

/*****************************************************************************
 * Grid traversal
 *****************************************************************************/

PGDLLEXPORT Datum H3_grid_disk(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3_grid_disk);
/**
 * @ingroup mobilitydb_h3_set
 * @brief Return all cells within k grid-steps of the origin
 * @sqlfn h3_grid_disk()
 */
Datum
H3_grid_disk(PG_FUNCTION_ARGS)
{
  H3Index origin = PG_GETARG_H3INDEX(0);
  int k = PG_GETARG_INT32(1);
  Set *result = h3_grid_disk(origin, k);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum H3_grid_ring(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3_grid_ring);
/**
 * @ingroup mobilitydb_h3_set
 * @brief Return the ring of cells at exactly k grid-steps from the origin
 * @sqlfn h3_grid_ring()
 */
Datum
H3_grid_ring(PG_FUNCTION_ARGS)
{
  H3Index origin = PG_GETARG_H3INDEX(0);
  int k = PG_GETARG_INT32(1);
  Set *result = h3_grid_ring(origin, k);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum H3_grid_path_cells(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3_grid_path_cells);
/**
 * @ingroup mobilitydb_h3_set
 * @brief Return the cells on the inclusive path from start to end
 * @sqlfn h3_grid_path_cells()
 */
Datum
H3_grid_path_cells(PG_FUNCTION_ARGS)
{
  H3Index start = PG_GETARG_H3INDEX(0);
  H3Index end = PG_GETARG_H3INDEX(1);
  Set *result = h3_grid_path_cells(start, end);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

/*****************************************************************************
 * Hierarchy
 *****************************************************************************/

PGDLLEXPORT Datum H3_cell_to_children(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3_cell_to_children);
/**
 * @ingroup mobilitydb_h3_set
 * @brief Return all children of the cell at the target resolution
 * @sqlfn h3_cell_to_children()
 */
Datum
H3_cell_to_children(PG_FUNCTION_ARGS)
{
  H3Index origin = PG_GETARG_H3INDEX(0);
  int child_res = PG_GETARG_INT32(1);
  Set *result = h3_cell_to_children(origin, child_res);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum H3_compact_cells(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3_compact_cells);
/**
 * @ingroup mobilitydb_h3_set
 * @brief Return the compacted representation of an h3indexset
 * @sqlfn h3_compact_cells()
 */
Datum
H3_compact_cells(PG_FUNCTION_ARGS)
{
  Set *cells = PG_GETARG_SET_P(0);
  Set *result = h3_compact_cells(cells);
  PG_FREE_IF_COPY(cells, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum H3_uncompact_cells(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3_uncompact_cells);
/**
 * @ingroup mobilitydb_h3_set
 * @brief Return the uncompacted representation of an h3indexset at
 * the target resolution
 * @sqlfn h3_uncompact_cells()
 */
Datum
H3_uncompact_cells(PG_FUNCTION_ARGS)
{
  Set *cells = PG_GETARG_SET_P(0);
  int res = PG_GETARG_INT32(1);
  Set *result = h3_uncompact_cells(cells, res);
  PG_FREE_IF_COPY(cells, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

/*****************************************************************************
 * Edges and vertexes
 *****************************************************************************/

PGDLLEXPORT Datum H3_origin_to_directed_edges(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3_origin_to_directed_edges);
/**
 * @ingroup mobilitydb_h3_set
 * @brief Return the outgoing directed edges of a cell
 * @sqlfn h3_origin_to_directed_edges()
 */
Datum
H3_origin_to_directed_edges(PG_FUNCTION_ARGS)
{
  H3Index origin = PG_GETARG_H3INDEX(0);
  Set *result = h3_origin_to_directed_edges(origin);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum H3_cell_to_vertexes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3_cell_to_vertexes);
/**
 * @ingroup mobilitydb_h3_set
 * @brief Return the vertexes of a cell
 * @sqlfn h3_cell_to_vertexes()
 */
Datum
H3_cell_to_vertexes(PG_FUNCTION_ARGS)
{
  H3Index cell = PG_GETARG_H3INDEX(0);
  Set *result = h3_cell_to_vertexes(cell);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

/*****************************************************************************
 * Icosahedron faces
 *****************************************************************************/

PGDLLEXPORT Datum H3_get_icosahedron_faces(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3_get_icosahedron_faces);
/**
 * @ingroup mobilitydb_h3_set
 * @brief Return the icosahedron face indexes intersected by a cell
 * @sqlfn h3_get_icosahedron_faces()
 */
Datum
H3_get_icosahedron_faces(PG_FUNCTION_ARGS)
{
  H3Index cell = PG_GETARG_H3INDEX(0);
  Set *result = h3_get_icosahedron_faces(cell);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

/*****************************************************************************/
