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
#include <utils/geo_decls.h> // PG_GETARG_POINT_P

#include "error.h"
#include "type.h"
#include "srf.h"

PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_grid_disk);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_grid_disk_distances);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_grid_ring_unsafe);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_grid_distance);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_grid_path_cells);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_cell_to_local_ij);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_local_ij_to_cell);

/*
 * k-rings produces indices within k distance of the origin index.
 *
 * k-ring 0 is defined as the origin index, k-ring 1 is defined as k-ring 0 and
 * all neighboring indices, and so on.
 *
 * Output is placed in the provided array in no particular order.
 * There may be fewer elements in output, as can happen when crossing a
 * pentagon.
 */
Datum
h3_grid_disk(PG_FUNCTION_ARGS)
{
	if (SRF_IS_FIRSTCALL())
	{
		FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
		MemoryContext oldcontext =
		MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		int64_t		max;
		H3Index    *indices;

		/* get function arguments */
		H3Index		origin = PG_GETARG_H3INDEX(0);
		int			k = PG_GETARG_INT32(1);

		h3_assert(maxGridDiskSize(k, &max));

		indices = palloc(max * sizeof(H3Index));

		h3_assert(gridDisk(origin, k, indices));

		funcctx->user_fctx = indices;
		funcctx->max_calls = max;
		MemoryContextSwitchTo(oldcontext);
	}

	SRF_RETURN_H3_INDEXES_FROM_USER_FCTX();
}

/*
 * k-rings produces indices within k distance of the origin index.
 *
 * k-ring 0 is defined as the origin index, k-ring 1 is defined as k-ring 0 and
 * all neighboring indices, and so on.
 *
 * Output is placed in the provided array in no particular order.
 * There may be fewer elements in output, as can happen when crossing a
 * pentagon.
 */
Datum
h3_grid_disk_distances(PG_FUNCTION_ARGS)
{
	if (SRF_IS_FIRSTCALL())
	{
		FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
		MemoryContext oldcontext =
		MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
		TupleDesc	tuple_desc;

		/* get function arguments */
		H3Index		origin = PG_GETARG_H3INDEX(0);
		int			k = PG_GETARG_INT32(1);

		/*
		 * Allocate memory for the indices, the distances and the tuple used
		 * for
		 */
		/* returning */
		int64_t		maxSize;
		hexDistanceTuple *user_fctx;

		h3_assert(maxGridDiskSize(k, &maxSize));

		user_fctx = palloc(sizeof(hexDistanceTuple));

		user_fctx->indices = palloc(maxSize * sizeof(H3Index));
		user_fctx->distances = palloc(maxSize * sizeof(int));

		h3_assert(gridDiskDistances(origin, k, user_fctx->indices, user_fctx->distances));

		ENSURE_TYPEFUNC_COMPOSITE(get_call_result_type(fcinfo, NULL, &tuple_desc));

		funcctx->tuple_desc = BlessTupleDesc(tuple_desc);
		funcctx->max_calls = maxSize;
		funcctx->user_fctx = user_fctx;

		MemoryContextSwitchTo(oldcontext);
	}

	SRF_RETURN_H3_INDEX_DISTANCES_FROM_USER_FCTX();
}

/*
 * Produces the hollow hexagonal ring centered at origin with sides of length k.
 *
 * Throws if pentagonal distortion was encountered.
 */
Datum
h3_grid_ring_unsafe(PG_FUNCTION_ARGS)
{
	if (SRF_IS_FIRSTCALL())
	{
		FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
		MemoryContext oldcontext =
		MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		/* get function arguments */
		H3Index    *indices;
		H3Index		origin = PG_GETARG_H3INDEX(0);
		int			k = PG_GETARG_INT32(1);

		/*
		 * Find the size of the ring. If k is 0, then it is the same as
		 * k_ring.
		 *
		 * If k is larger than 0, the ring is the size of the circle with k,
		 * minus the circle with k-1
		 */
		int64_t		maxSize;
		int64_t		innerSize;

		h3_assert(maxGridDiskSize(k, &maxSize));

		if (k > 0)
		{
			h3_assert(maxGridDiskSize(k - 1, &innerSize));
			maxSize -= innerSize;
		}
		indices = palloc(maxSize * sizeof(H3Index));

		h3_assert(gridRingUnsafe(origin, k, indices));

		funcctx->user_fctx = indices;
		funcctx->max_calls = maxSize;
		MemoryContextSwitchTo(oldcontext);
	}

	SRF_RETURN_H3_INDEXES_FROM_USER_FCTX();
}

/*
 * Returns the distance in grid cells between the two indexes.
 *
 * Returns a negative number if finding the distance failed.
 * Finding the distance can fail because the two indexes are not comparable
 * (different resolutions), too far apart, or are separated by pentagonal
 * distortion. This is the same set of limitations as the local IJ coordinate
 * space functions.
 */
Datum
h3_grid_distance(PG_FUNCTION_ARGS)
{
	H3Index		originIndex = PG_GETARG_H3INDEX(0);
	H3Index		h3Index = PG_GETARG_H3INDEX(1);
	int64_t		distance;

	h3_assert(gridDistance(originIndex, h3Index, &distance));

	PG_RETURN_INT64(distance);
}

/*
 * Given two H3 indexes, return the line of indexes between them (inclusive).
 *
 * This function may fail to find the line between two indexes, for
 * example if they are very far apart. It may also fail when finding
 * distances for indexes on opposite sides of a pentagon.
 */
Datum
h3_grid_path_cells(PG_FUNCTION_ARGS)
{
	if (SRF_IS_FIRSTCALL())
	{
		FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
		MemoryContext oldcontext =
		MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		/* get function arguments */
		int64_t		size;
		H3Index    *indices;
		H3Index		start = PG_GETARG_H3INDEX(0);
		H3Index		end = PG_GETARG_H3INDEX(1);

		h3_assert(gridPathCellsSize(start, end, &size));

		indices = palloc(size * sizeof(H3Index));

		h3_assert(gridPathCells(start, end, indices));

		funcctx->user_fctx = indices;
		funcctx->max_calls = size;
		MemoryContextSwitchTo(oldcontext);
	}

	SRF_RETURN_H3_INDEXES_FROM_USER_FCTX();
}

/*
 * Produces local IJ coordinates for an H3 index anchored by an origin.
 */
Datum
h3_cell_to_local_ij(PG_FUNCTION_ARGS)
{
	H3Index		origin = PG_GETARG_H3INDEX(0);
	H3Index		index = PG_GETARG_H3INDEX(1);

	Point	   *point = (Point *) palloc(sizeof(Point));
	CoordIJ		coord;

	h3_assert(cellToLocalIj(origin, index, 0, &coord));

	point->x = coord.i;
	point->y = coord.j;
	PG_RETURN_POINT_P(point);
}

/*
 * Produces an H3 index from local IJ coordinates anchored by an origin.
 */
Datum
h3_local_ij_to_cell(PG_FUNCTION_ARGS)
{
	H3Index		origin = PG_GETARG_H3INDEX(0);
	Point	   *point = PG_GETARG_POINT_P(1);

	H3Index    *index = (H3Index *) palloc(sizeof(H3Index));

	CoordIJ		coord;

	coord.i = point->x;
	coord.j = point->y;

	h3_assert(localIjToCell(origin, &coord, 0, index));

	PG_FREE_IF_COPY(point, 1);
	PG_RETURN_H3INDEX(*index);
}
