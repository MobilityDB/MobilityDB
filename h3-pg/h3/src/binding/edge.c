/*
 * Copyright 2024 Zacharias Knudsen
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <postgres.h>
#include <h3api.h>

#include <fmgr.h>				 // PG_FUNCTION_INFO_V1
#include <funcapi.h>			 // SRF_IS_FIRSTCALL
#include <access/htup_details.h> // HeapTuple
#include <utils/geo_decls.h>	 // PG_RETURN_POLYGON_P

#include "error.h"
#include "type.h"
#include "srf.h"

#if POSTGRESQL_VERSION_MAJOR >= 16
#include "varatt.h" //VAR_SIZE and friends moved to here from postgres.h
#endif

PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_are_neighbor_cells);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_cells_to_directed_edge);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_is_valid_directed_edge);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_get_directed_edge_origin);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_get_directed_edge_destination);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_directed_edge_to_cells);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_origin_to_directed_edges);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_directed_edge_to_boundary);

/* Returns whether or not the provided H3 cell indexes are neighbors. */
Datum
h3_are_neighbor_cells(PG_FUNCTION_ARGS)
{
	int			neighboring;
	H3Index		origin = PG_GETARG_H3INDEX(0);
	H3Index		destination = PG_GETARG_H3INDEX(1);

	h3_assert(areNeighborCells(origin, destination, &neighboring));

	PG_RETURN_BOOL(neighboring);
}

/* Returns a unidirectional edge H3 index based on the provided origin and destination. */
Datum
h3_cells_to_directed_edge(PG_FUNCTION_ARGS)
{
	H3Index		edge;
	H3Index		origin = PG_GETARG_H3INDEX(0);
	H3Index		destination = PG_GETARG_H3INDEX(1);

	h3_assert(cellsToDirectedEdge(origin, destination, &edge));

	PG_RETURN_H3INDEX(edge);
}

/* Determines if the provided H3Index is a valid unidirectional edge index. */
Datum
h3_is_valid_directed_edge(PG_FUNCTION_ARGS)
{
	H3Index		edge = PG_GETARG_H3INDEX(0);
	int			valid = isValidDirectedEdge(edge);

	PG_RETURN_BOOL(valid);
}

/* Returns the origin hexagon from the unidirectional edge H3Index. */
Datum
h3_get_directed_edge_origin(PG_FUNCTION_ARGS)
{
	H3Index		origin;
	H3Index		edge = PG_GETARG_H3INDEX(0);

	h3_assert(getDirectedEdgeOrigin(edge, &origin));

	PG_RETURN_H3INDEX(origin);
}

/* Returns the destination hexagon from the unidirectional edge H3Index. */
Datum
h3_get_directed_edge_destination(PG_FUNCTION_ARGS)
{
	H3Index		destination;
	H3Index		edge = PG_GETARG_H3INDEX(0);

	h3_assert(getDirectedEdgeDestination(edge, &destination));

	PG_RETURN_H3INDEX(destination);
}

/* Returns the origin, destination pair of hexagon IDs for the given edge ID. */
Datum
h3_directed_edge_to_cells(PG_FUNCTION_ARGS)
{
	TupleDesc	tuple_desc;
	Datum		values[2];
	bool		nulls[2] = {false};
	HeapTuple	tuple;
	Datum		result;

	H3Index		edge = PG_GETARG_H3INDEX(0);
	H3Index    *cells = palloc(2 * sizeof(H3Index));

	h3_assert(directedEdgeToCells(edge, cells));

	ENSURE_TYPEFUNC_COMPOSITE(get_call_result_type(fcinfo, NULL, &tuple_desc));

	values[0] = H3IndexGetDatum(cells[0]);
	values[1] = H3IndexGetDatum(cells[1]);

	tuple_desc = BlessTupleDesc(tuple_desc);
	tuple = heap_form_tuple(tuple_desc, values, nulls);
	result = HeapTupleGetDatum(tuple);
	PG_RETURN_DATUM(result);
}

/* Provides all of the unidirectional edges from the current H3Index. */
Datum
h3_origin_to_directed_edges(PG_FUNCTION_ARGS)
{
	if (SRF_IS_FIRSTCALL())
	{
		FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
		MemoryContext oldcontext =
		MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		int			max = 6;
		H3Index		origin = PG_GETARG_H3INDEX(0);
		H3Index    *edges = palloc(max * sizeof(H3Index));

		h3_assert(originToDirectedEdges(origin, edges));

		funcctx->user_fctx = edges;
		funcctx->max_calls = max;
		MemoryContextSwitchTo(oldcontext);
	}

	SRF_RETURN_H3_INDEXES_FROM_USER_FCTX();
}

/* Provides the coordinates defining the unidirectional edge. */
Datum
h3_directed_edge_to_boundary(PG_FUNCTION_ARGS)
{
	CellBoundary boundary;
	POLYGON    *polygon;
	int			size;
	H3Index		edge = PG_GETARG_H3INDEX(0);

	h3_assert(directedEdgeToBoundary(edge, &boundary));

	size = offsetof(POLYGON, p[0]) +sizeof(polygon->p[0]) * boundary.numVerts;
	polygon = (POLYGON *) palloc(size);
	SET_VARSIZE(polygon, size);
	polygon->npts = boundary.numVerts;

	for (int v = 0; v < boundary.numVerts; v++)
	{
		polygon->p[v].x = radsToDegs(boundary.verts[v].lat);
		polygon->p[v].y = radsToDegs(boundary.verts[v].lng);
	}
	PG_RETURN_POLYGON_P(polygon);
}
