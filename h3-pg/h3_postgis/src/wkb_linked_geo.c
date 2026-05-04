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

#include "wkb_linked_geo.h"

static void
			free_linked_geo_loop(LinkedGeoLoop * loop);

int
count_linked_polygons(const LinkedGeoPolygon * multiPolygon)
{
	int			num = 0;

	FOREACH_LINKED_POLYGON(multiPolygon, _)
		++ num;
	return num;
}

int
count_linked_geo_loops(const LinkedGeoPolygon * polygon)
{
	int			num = 0;

	FOREACH_LINKED_LOOP(polygon, _)
		++ num;
	return num;
}

int
count_linked_lat_lng(const LinkedGeoLoop * loop)
{
	int			num = 0;

	FOREACH_LINKED_LAT_LNG(loop, _)
		++ num;
	return num;
}

LinkedGeoPolygon *
copy_linked_geo_polygon(const LinkedGeoPolygon * polygon)
{
	LinkedGeoPolygon *copy = palloc0(sizeof(LinkedGeoPolygon));

	FOREACH_LINKED_LOOP(polygon, loop)
	{
		LinkedGeoLoop *loopCopy = copy_linked_geo_loop(loop);

		add_linked_geo_loop(copy, loopCopy);
	}
	return copy;
}

LinkedGeoLoop *
copy_linked_geo_loop(const LinkedGeoLoop * loop)
{
	LinkedGeoLoop *copy = palloc0(sizeof(LinkedGeoLoop));

	FOREACH_LINKED_LAT_LNG(loop, latlng)
	{
		LinkedLatLng *latlng_copy = copy_linked_lat_lng(latlng);

		add_linked_lat_lng(copy, latlng_copy);
	}
	return copy;
}

LinkedLatLng *
copy_linked_lat_lng(const LinkedLatLng * latlng)
{
	LinkedLatLng *copy = palloc0(sizeof(LinkedLatLng));

	copy->vertex = latlng->vertex;
	return copy;
}

void
add_linked_geo_loop(LinkedGeoPolygon * polygon, LinkedGeoLoop * loop)
{
	LinkedGeoLoop *last = polygon->last;

	if (!last)
		polygon->first = loop;
	else
		last->next = loop;
	polygon->last = loop;
}

void
add_linked_lat_lng(LinkedGeoLoop * loop, LinkedLatLng * latlng)
{
	LinkedLatLng *last = loop->last;

	if (!last)
		loop->first = latlng;
	else
		last->next = latlng;
	loop->last = latlng;
}

void
free_linked_geo_polygon(LinkedGeoPolygon * multiPolygon)
{
	LinkedGeoPolygon *polygon = multiPolygon;

	while (polygon)
	{
		LinkedGeoPolygon *nextPolygon = polygon->next;

		/* Free loops */
		LinkedGeoLoop *loop = polygon->first;

		while (loop)
		{
			LinkedGeoLoop *nextLoop = loop->next;

			free_linked_geo_loop(loop);
			loop = nextLoop;
		}

		pfree(polygon);
		polygon = nextPolygon;
	}
}

void
free_linked_geo_loop(LinkedGeoLoop * loop)
{
	LinkedLatLng *latlng = loop->first;

	while (latlng)
	{
		LinkedLatLng *next = latlng->next;

		pfree(latlng);
		latlng = next;
	}
	pfree(loop);
}
