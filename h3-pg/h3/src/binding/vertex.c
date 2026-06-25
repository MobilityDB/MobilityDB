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

#include <fmgr.h>			 // PG_FUNCTION_ARGS
#include <funcapi.h>		 // SRF_IS_FIRSTCALL
#include <utils/geo_decls.h> // PG_RETURN_POINT_P

#include "error.h"
#include "type.h"
#include "srf.h"

PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_cell_to_vertex);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_cell_to_vertexes);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_vertex_to_latlng);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_is_valid_vertex);

/* Returns a single vertex for a given cell, as an H3 index */
Datum
h3_cell_to_vertex(PG_FUNCTION_ARGS)
{
	H3Index		vertex;
	H3Index		cell = PG_GETARG_H3INDEX(0);
	int			vertexNum = PG_GETARG_INT32(1);

	h3_assert(cellToVertex(cell, vertexNum, &vertex));

	PG_RETURN_H3INDEX(vertex);
}

/* Returns all vertexes for a given cell, as H3 indexes */
Datum
h3_cell_to_vertexes(PG_FUNCTION_ARGS)
{
	/* stuff done only on the first call of the function */
	if (SRF_IS_FIRSTCALL())
	{
		int			maxSize;
		int			size;
		H3Index    *vertexes;

		/* create a function context for cross-call persistence */
		FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();

		/* switch to memory context appropriate for multiple function calls */
		MemoryContext oldcontext =
		MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		/* BEGIN One-time setup code */

		H3Index		cell = PG_GETARG_H3INDEX(0);

		maxSize = 6;
		size = maxSize * sizeof(H3Index);

		vertexes = palloc(size);
		h3_assert(cellToVertexes(cell, vertexes));

		funcctx->user_fctx = vertexes;
		funcctx->max_calls = maxSize;

		/* END One-time setup code */

		MemoryContextSwitchTo(oldcontext);
	}

	SRF_RETURN_H3_INDEXES_FROM_USER_FCTX();
}

/* Get the geocoordinates of an H3 vertex */
Datum
h3_vertex_to_latlng(PG_FUNCTION_ARGS)
{
	H3Index		vertex = PG_GETARG_H3INDEX(0);

	Point	   *point = palloc(sizeof(Point));
	LatLng		latlng;

	h3_assert(vertexToLatLng(vertex, &latlng));

	point->x = radsToDegs(latlng.lng);
	point->y = radsToDegs(latlng.lat);

	PG_RETURN_POINT_P(point);
}


/* Whether the input is a valid H3 vertex */
Datum
h3_is_valid_vertex(PG_FUNCTION_ARGS)
{
	H3Index		vertex = PG_GETARG_H3INDEX(0);
	bool		is_valid = isValidVertex(vertex);

	PG_RETURN_BOOL(is_valid);
}
