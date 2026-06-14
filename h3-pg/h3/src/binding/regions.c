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

#include <fmgr.h>				 // PG_FUNCTION_ARGS
#include <funcapi.h>			 // SRF_IS_FIRSTCALL
#include <access/htup_details.h> // HeapTuple
#include <utils/array.h>		 // ArrayType
#include <utils/geo_decls.h>	 // PG_GETARG_POLYGON_P
#include <utils/lsyscache.h>	 // get_typlenbyvalalign
#include <catalog/pg_type.h>	 // POLYGONOID
#include <utils/builtins.h>		 // text_to_cstring

#include "error.h"
#include "type.h"
#include "srf.h"

PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_polygon_to_cells);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_polygon_to_cells_experimental);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_cells_to_multi_polygon);

static void
polygonToGeoLoop(POLYGON *polygon, GeoLoop * geoloop)
{
	geoloop->numVerts = polygon->npts;
	geoloop->verts = (LatLng *) palloc(geoloop->numVerts * sizeof(LatLng));

	for (int i = 0; i < geoloop->numVerts; i++)
	{
		geoloop->verts[i].lng = degsToRads(polygon->p[i].x);
		geoloop->verts[i].lat = degsToRads(polygon->p[i].y);
	}
}

static int
linkedGeoLoopToNativePolygonSize(LinkedGeoLoop * linkedLoop)
{
	int			count = 0;
	LinkedLatLng *linkedCoord = linkedLoop->first;

	while (linkedCoord != NULL)
	{
		count++;
		linkedCoord = linkedCoord->next;
	}
	return count;
}

static void
linkedGeoLoopToNativePolygon(LinkedGeoLoop * linkedLoop, POLYGON *polygon)
{
	int			count;
	LinkedLatLng *linkedCoord = linkedLoop->first;

	count = 0;
	while (linkedCoord != NULL)
	{
		(polygon->p[count]).x = radsToDegs(linkedCoord->vertex.lng);
		(polygon->p[count]).y = radsToDegs(linkedCoord->vertex.lat);
		linkedCoord = linkedCoord->next;
		count++;
	}
}

/*
 * H3Error polygonToCells(const GeoPolygon *geoPolygon, int res, uint32_t flags, H3Index *out);
 */
Datum
h3_polygon_to_cells(PG_FUNCTION_ARGS)
{
	if (SRF_IS_FIRSTCALL())
	{
		FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
		MemoryContext oldcontext =
		MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		int64_t		maxSize;
		H3Index    *indices;
		ArrayType  *holes;
		int			nelems = 0;
		int			resolution;
		GeoPolygon	polygon;
		Datum		value;
		bool		isnull;
		POLYGON    *exterior;

		if (PG_ARGISNULL(0))
			ASSERT(0, ERRCODE_INVALID_PARAMETER_VALUE, "No polygon given to polyfill");

		/* get function arguments */
		exterior = PG_GETARG_POLYGON_P(0);

		if (!PG_ARGISNULL(1))
		{
			holes = PG_GETARG_ARRAYTYPE_P(1);
			nelems = ArrayGetNItems(ARR_NDIM(holes), ARR_DIMS(holes));
		}
		resolution = PG_GETARG_INT32(2);

		/* build polygon */
		polygonToGeoLoop(exterior, &(polygon.geoloop));

		if (nelems)
		{
			int			i = 0;
			ArrayIterator iterator = array_create_iterator(holes, 0, NULL);

			polygon.numHoles = nelems;
			polygon.holes = (GeoLoop *) palloc(polygon.numHoles * sizeof(GeoLoop));

			while (array_iterate(iterator, &value, &isnull))
			{
				if (isnull)
				{
					polygon.numHoles--;
				}
				else
				{
					POLYGON    *hole = DatumGetPolygonP(value);

					polygonToGeoLoop(hole, &(polygon.holes[i]));
					i++;
				}
			}
		}
		else
		{
			polygon.numHoles = 0;
		}

		/* produce hexagons into allocated memory */
		h3_assert(maxPolygonToCellsSize(&polygon, resolution, 0, &maxSize));
		indices = palloc_extended(maxSize * sizeof(H3Index),
								  MCXT_ALLOC_HUGE | MCXT_ALLOC_ZERO);
		h3_assert(polygonToCells(&polygon, resolution, 0, indices));

		funcctx->user_fctx = indices;
		funcctx->max_calls = maxSize;
		MemoryContextSwitchTo(oldcontext);
	}

	SRF_RETURN_H3_INDEXES_FROM_USER_FCTX();
}

/*
 * H3Error polygonToCells(const GeoPolygon *geoPolygon, int res, uint32_t flags, H3Index *out);
 */
Datum
h3_polygon_to_cells_experimental(PG_FUNCTION_ARGS)
{
	if (SRF_IS_FIRSTCALL())
	{
		FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
		MemoryContext oldcontext =
		MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		char       *containment_mode;
		int64_t		maxSize;
		H3Index    *indices;
		ArrayType  *holes;
		int			nelems = 0;
		uint32_t	flags = 0;
		int			resolution;
		GeoPolygon	polygon;
		Datum		value;
		bool		isnull;
		POLYGON    *exterior;

		if (PG_ARGISNULL(0))
			ASSERT(0, ERRCODE_INVALID_PARAMETER_VALUE, "No polygon given to polyfill");

		/* get function arguments */
		exterior = PG_GETARG_POLYGON_P(0);

		if (!PG_ARGISNULL(1))
		{
			holes = PG_GETARG_ARRAYTYPE_P(1);
			nelems = ArrayGetNItems(ARR_NDIM(holes), ARR_DIMS(holes));
		}
		resolution = PG_GETARG_INT32(2);
		if (!PG_ARGISNULL(3))
		{
			containment_mode = text_to_cstring(PG_GETARG_TEXT_PP(3));
			if (strcmp(containment_mode, "center") == 0)
				flags = 0;
			else if (strcmp(containment_mode, "full") == 0)
				flags = 1;
			else if (strcmp(containment_mode, "overlapping") == 0)
				flags = 2;
			else if (strcmp(containment_mode, "overlapping_bbox") == 0)
				flags = 3;
			else
				ASSERT(0, ERRCODE_INVALID_PARAMETER_VALUE, "Containment Mode must be center, full, overlapping, or overlapping_bbox.");
		}

		/* build polygon */
		polygonToGeoLoop(exterior, &(polygon.geoloop));

		if (nelems)
		{
			int			i = 0;
			ArrayIterator iterator = array_create_iterator(holes, 0, NULL);

			polygon.numHoles = nelems;
			polygon.holes = (GeoLoop *) palloc(polygon.numHoles * sizeof(GeoLoop));

			while (array_iterate(iterator, &value, &isnull))
			{
				if (isnull)
				{
					polygon.numHoles--;
				}
				else
				{
					POLYGON    *hole = DatumGetPolygonP(value);

					polygonToGeoLoop(hole, &(polygon.holes[i]));
					i++;
				}
			}
		}
		else
		{
			polygon.numHoles = 0;
		}

		/* produce hexagons into allocated memory */
		h3_assert(maxPolygonToCellsSizeExperimental(&polygon, resolution, flags, &maxSize));
		indices = palloc_extended(maxSize * sizeof(H3Index),
								  MCXT_ALLOC_HUGE | MCXT_ALLOC_ZERO);
		h3_assert(polygonToCellsExperimental(&polygon, resolution, flags, maxSize, indices));

		funcctx->user_fctx = indices;
		funcctx->max_calls = maxSize;
		MemoryContextSwitchTo(oldcontext);
	}

	SRF_RETURN_H3_INDEXES_FROM_USER_FCTX();
}

/*
 * https://stackoverflow.com/questions/51127189/how-to-return-array-into-array-with-custom-type-in-postgres-c-function
 */
Datum
h3_cells_to_multi_polygon(PG_FUNCTION_ARGS)
{
	FuncCallContext *funcctx;
	TupleDesc	tuple_desc;

	LinkedGeoPolygon *linkedPolygon;
	LinkedGeoLoop *linkedLoop;

	if (SRF_IS_FIRSTCALL())
	{
		Datum		value;
		bool		isnull;
		int			i = 0;

		FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
		MemoryContext oldcontext =
		MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		ArrayType  *array = PG_GETARG_ARRAYTYPE_P(0);
		int			numHexes = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
		ArrayIterator iterator = array_create_iterator(array, 0, NULL);
		H3Index    *h3set = palloc(numHexes * sizeof(H3Index));

		/* Extract data from array into h3set, and wipe compactedSet memory */
		while (array_iterate(iterator, &value, &isnull))
		{
			h3set[i++] = DatumGetH3Index(value);
		}

		/* produce hexagons into allocated memory */
		linkedPolygon = palloc(sizeof(LinkedGeoPolygon));
		h3_assert(cellsToLinkedMultiPolygon(h3set, numHexes, linkedPolygon));

		ENSURE_TYPEFUNC_COMPOSITE(get_call_result_type(fcinfo, NULL, &tuple_desc));

		funcctx->user_fctx = linkedPolygon;
		funcctx->tuple_desc = BlessTupleDesc(tuple_desc);
		MemoryContextSwitchTo(oldcontext);
	}

	funcctx = SRF_PERCALL_SETUP();
	linkedPolygon = (LinkedGeoPolygon *) funcctx->user_fctx;

	if (linkedPolygon)
	{
		HeapTuple	tuple;
		Datum		result;
		int			count;
		int			size;
		POLYGON    *polygon;

		Datum	   *elems;
		Datum		values[2];
		bool		nulls[2];

		int16		typlen;
		bool		typbyval;
		char		typalign;
		ArrayType  *retarr;

		tuple_desc = funcctx->tuple_desc;

		linkedLoop = linkedPolygon->first;
		count = linkedGeoLoopToNativePolygonSize(linkedLoop);
		size = offsetof(POLYGON, p) +sizeof(polygon->p[0]) * count;
		polygon = palloc0(size);
		SET_VARSIZE(polygon, size);
		polygon->npts = count;
		linkedGeoLoopToNativePolygon(linkedLoop, polygon);

		values[0] = PolygonPGetDatum(polygon);
		nulls[0] = false;

		/* construct array */
		count = 0;
		linkedLoop = linkedPolygon->first->next;
		while (linkedLoop != NULL)
		{
			count++;
			linkedLoop = linkedLoop->next;
		}
		elems = (Datum *) palloc(count * sizeof(Datum));
		if (count)
		{
			linkedLoop = linkedPolygon->first->next;
			for (int i = 0; i < count; i++)
			{
				int			subcount = linkedGeoLoopToNativePolygonSize(linkedLoop);
				POLYGON    *polygon;
				int			size = offsetof(POLYGON, p) +sizeof(polygon->p[0]) * subcount;

				polygon = palloc0(size);
				SET_VARSIZE(polygon, size);
				polygon->npts = subcount;
				linkedGeoLoopToNativePolygon(linkedLoop, polygon);
				elems[i] = PolygonPGetDatum(polygon);
				linkedLoop = linkedLoop->next;
			}
		}

		get_typlenbyvalalign(POLYGONOID, &typlen, &typbyval, &typalign);
		retarr =
			construct_array(elems, count, POLYGONOID, typlen, typbyval, typalign);
		values[1] = PointerGetDatum(retarr);
		nulls[1] = false;

		tuple = heap_form_tuple(tuple_desc, values, nulls);
		result = HeapTupleGetDatum(tuple);

		funcctx->user_fctx = linkedPolygon->next;
		SRF_RETURN_NEXT(funcctx, result);
	}
	else
	{
		destroyLinkedMultiPolygon(linkedPolygon);
		SRF_RETURN_DONE(funcctx);
	}
}

/* ---------------------------------------------------------------------------
 * The GeoPolygon, LinkedLatLng, LinkedLatLng,
 * LinkedGeoLoop, and LinkedGeoPolygon
 *
 * copied from H3 core for reference
 */

/** @struct GeoPolygon
 *	@brief Simplified core of GeoJSON Polygon coordinates definition
 */
/*
typedef struct {
	double lat;  ///< latitude in radians
	double lon;  ///< longitude in radians
} LatLng;

typedef struct {
	int numVerts;
	LatLng *verts;
} GeoLoop;

typedef struct {
	GeoLoop geoloop;	///< exterior boundary of the polygon
	int numHoles;		///< number of elements in the array pointed to by holes
	GeoLoop *holes; ///< interior boundaries (holes) in the polygon
} GeoPolygon;
*/

/** @struct LinkedLatLng
 *	@brief A coordinate node in a linked geo structure, part of a linked list
 *
typedef struct LinkedLatLng LinkedLatLng;
struct LinkedLatLng
{
	LatLng vertex;
	LinkedLatLng *next;
};

** @struct LinkedGeoLoop
 *	@brief A loop node in a linked geo structure, part of a linked list
 *
typedef struct LinkedGeoLoop LinkedGeoLoop;
struct LinkedGeoLoop
{
	LinkedLatLng *first;
	LinkedLatLng *last;
	LinkedGeoLoop *next;
};

** @struct LinkedGeoPolygon
 *	@brief A polygon node in a linked geo structure, part of a linked list.
 *
typedef struct LinkedGeoPolygon LinkedGeoPolygon;
struct LinkedGeoPolygon
{
	LinkedGeoLoop *first;
	LinkedGeoLoop *last;
	LinkedGeoPolygon *next;
};
*/
