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

#ifndef PGH3_WKB_LINKED_GEO_H
#define PGH3_WKB_LINKED_GEO_H

#include <h3api.h>

#define _FOREACH_LINKED_ITEM(first, item, Type)			\
	for (Type *item = (first);								\
		 item != NULL;										\
		 item = item->next)

#define FOREACH_LINKED_POLYGON(multiPolygon, polygon)					\
	_FOREACH_LINKED_ITEM(multiPolygon, polygon, const LinkedGeoPolygon)

#define FOREACH_LINKED_POLYGON_NOCONST(multiPolygon, polygon)			\
	_FOREACH_LINKED_ITEM(multiPolygon, polygon, LinkedGeoPolygon)

#define FOREACH_LINKED_LOOP(polygon, loop)						\
	_FOREACH_LINKED_ITEM(polygon->first, loop, const LinkedGeoLoop)

#define FOREACH_LINKED_LOOP_NOCONST(polygon, loop)		\
	_FOREACH_LINKED_ITEM(polygon->first, loop, LinkedGeoLoop)

#define FOREACH_LINKED_LAT_LNG(loop, latlng)				\
	_FOREACH_LINKED_ITEM(loop->first, latlng, const LinkedLatLng)

#define FOREACH_LINKED_LAT_LNG_NOCONST(loop, latlng)	\
	_FOREACH_LINKED_ITEM(loop->first, latlng, LinkedLatLng)

/* NOTE: loop must contain at least 2 points */
#define FOREACH_LINKED_LAT_LNG_PAIR(loop, cur, next)					\
	for (const LinkedLatLng *cur = loop->first, *next = cur->next;		\
		 cur != NULL;													\
		 cur = cur->next, next = next->next ? next->next : loop->first)

int
			count_linked_polygons(const LinkedGeoPolygon * multiPolygon);

int
			count_linked_geo_loops(const LinkedGeoPolygon * polygon);

int
			count_linked_lat_lng(const LinkedGeoLoop * loop);

LinkedGeoPolygon *
			copy_linked_geo_polygon(const LinkedGeoPolygon * polygon);

LinkedGeoLoop *
			copy_linked_geo_loop(const LinkedGeoLoop * loop);

LinkedLatLng *
			copy_linked_lat_lng(const LinkedLatLng * latlng);

void
			add_linked_geo_loop(LinkedGeoPolygon * polygon, LinkedGeoLoop * loop);

void
			add_linked_lat_lng(LinkedGeoLoop * loop, LinkedLatLng * latlng);

void
			free_linked_geo_polygon(LinkedGeoPolygon * multiPolygon);

#endif
