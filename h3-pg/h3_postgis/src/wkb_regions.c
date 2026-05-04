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

#include <fmgr.h>		 // PG_FUNCTION_ARGS
#include <utils/array.h> // using arrays

#include "error.h"
#include "type.h"
#include "wkb_linked_geo.h"
#include "wkb_split.h"
#include "wkb.h"

PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_cells_to_multi_polygon_wkb);

/* Converts LinkedGeoPolygon vertex coordinates to degrees in place */
static void
			linked_geo_polygon_to_degs(LinkedGeoPolygon * multiPolygon);

Datum
h3_cells_to_multi_polygon_wkb(PG_FUNCTION_ARGS)
{
	ArrayType  *array = PG_GETARG_ARRAYTYPE_P(0);
	LinkedGeoPolygon *linkedPolygon;
	H3Error		error;
	int			numHexes;
	ArrayIterator iterator;
	Datum		value;
	bool		isnull;
	H3Index    *h3set;
	bytea	   *wkb;

	numHexes = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
	h3set = palloc(numHexes * sizeof(H3Index));

	/* Extract data from array into h3set */

	iterator = array_create_iterator(array, 0, NULL);
	numHexes = 0;
	while (array_iterate(iterator, &value, &isnull))
	{
		h3set[numHexes++] = DatumGetH3Index(value);
	}

	/* produce hexagons into allocated memory */
	linkedPolygon = palloc(sizeof(LinkedGeoPolygon));
	h3_assert(cellsToLinkedMultiPolygon(h3set, numHexes, linkedPolygon));

	if (is_linked_polygon_crossed_by_180(linkedPolygon))
	{
		/* Split by 180th meridian */
		LinkedGeoPolygon *splitPolygon = split_linked_polygon_by_180(linkedPolygon);

		linked_geo_polygon_to_degs(splitPolygon);
		wkb = linked_geo_polygon_to_wkb(splitPolygon);
		free_linked_geo_polygon(splitPolygon);
	}
	else
	{
		linked_geo_polygon_to_degs(linkedPolygon);
		wkb = linked_geo_polygon_to_wkb(linkedPolygon);
	}

	destroyLinkedMultiPolygon(linkedPolygon);
	pfree(linkedPolygon);

	PG_RETURN_BYTEA_P(wkb);
}

void
linked_geo_polygon_to_degs(LinkedGeoPolygon * multiPolygon)
{
	FOREACH_LINKED_POLYGON_NOCONST(multiPolygon, polygon)
	{
		FOREACH_LINKED_LOOP_NOCONST(polygon, loop)
		{
			FOREACH_LINKED_LAT_LNG_NOCONST(loop, latlng)
			{
				LatLng	   *vertex = &latlng->vertex;

				vertex->lat = radsToDegs(vertex->lat);
				vertex->lng = radsToDegs(vertex->lng);
			}
		}
	}
}
