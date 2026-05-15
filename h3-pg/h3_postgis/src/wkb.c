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

#include <stddef.h>
#include <string.h>

#include "error.h"
#include "wkb.h"
#include "wkb_linked_geo.h"

#define WKB_BYTE_SIZE 1
#define WKB_INT_SIZE 4
#define WKB_DOUBLE_SIZE 8

#define WKB_NDR 1
#define WKB_XDR 0

#define WKB_POLYGON_TYPE 3
#define WKB_MULTIPOLYGON_TYPE 6

#define WKB_SRID_FLAG 0x20000000

#define WKB_SRID_DEFAULT 4326

#define ASSERT_WKB_DATA_WRITTEN(wkb, data) \
	ASSERT( \
		(uint8 *)wkb + VARSIZE(wkb) == data, \
		ERRCODE_EXTERNAL_ROUTINE_EXCEPTION, \
		"# of written bytes (%d) must match allocation size (%d)", \
		(int)(data - (uint8 *)wkb), VARSIZE(wkb))

static bool
			boundary_is_empty(const CellBoundary * boundary);

static bool
			boundary_is_closed(const CellBoundary * boundary);

static size_t
			boundary_array_data_size(const CellBoundary * boundaries, int num);

static size_t
			boundary_data_size(const CellBoundary * boundary);

static size_t
			linked_geo_polygon_data_size(const LinkedGeoPolygon * multiPolygon);

static size_t
			linked_geo_loop_data_size(const LinkedGeoLoop * loop);

static uint8 *
			wkb_write_boundary_array_data(uint8 *data, const CellBoundary * boundaries, int num);

static uint8 *
			wkb_write_boundary_data(uint8 *data, const CellBoundary * boundary);

static uint8 *
			wkb_write_linked_geo_polygon_data(uint8 *data, const LinkedGeoPolygon * multiPolygon);

static uint8 *
			wkb_write_lat_lng_array(uint8 *data, const LatLng * coord, int num);

static uint8 *
			wkb_write_linked_geo_loop_data(uint8 *data, const LinkedGeoLoop * loop);

static uint8 *
			wkb_write_lat_lng(uint8 *data, const LatLng * coord);

static uint8 *
			wkb_write_endian(uint8 *data);

static uint8 *
			wkb_write_int(uint8 *data, uint32 value);

static uint8 *
			wkb_write(uint8 *data, const void *value, size_t size);

bytea *
boundary_array_to_wkb(const CellBoundary * boundaries, size_t num)
{
	uint8	   *data;
	bytea	   *wkb;
	size_t		size = boundary_array_data_size(boundaries, num);

	wkb = palloc(VARHDRSZ + size);
	SET_VARSIZE(wkb, VARHDRSZ + size);

	data = (uint8 *) VARDATA(wkb);
	data = wkb_write_boundary_array_data(data, boundaries, num);

	ASSERT_WKB_DATA_WRITTEN(wkb, data);
	return wkb;
}

bytea *
boundary_to_wkb(const CellBoundary * boundary)
{
	bytea	   *wkb;
	uint8	   *data;
	size_t		size = boundary_data_size(boundary);

	wkb = palloc(VARHDRSZ + size);
	SET_VARSIZE(wkb, VARHDRSZ + size);

	data = (uint8 *) VARDATA(wkb);
	data = wkb_write_boundary_data(data, boundary);

	ASSERT_WKB_DATA_WRITTEN(wkb, data);
	return wkb;
}

bytea *
linked_geo_polygon_to_wkb(const LinkedGeoPolygon * multiPolygon)
{
	bytea	   *wkb;
	uint8	   *data;
	size_t		size = linked_geo_polygon_data_size(multiPolygon);

	wkb = palloc(VARHDRSZ + size);
	SET_VARSIZE(wkb, VARHDRSZ + size);

	data = (uint8 *) VARDATA(wkb);
	data = wkb_write_linked_geo_polygon_data(data, multiPolygon);

	ASSERT_WKB_DATA_WRITTEN(wkb, data);
	return wkb;
}

bool
boundary_is_empty(const CellBoundary * boundary)
{
	return boundary->numVerts < 1;
}

bool
boundary_is_closed(const CellBoundary * boundary)
{
	const LatLng *verts;
	int			numVerts;

	if (boundary_is_empty(boundary))
		return true;

	verts = boundary->verts;
	numVerts = boundary->numVerts;
	return verts[0].lng == verts[numVerts - 1].lng
		&& verts[1].lat == verts[numVerts - 1].lat;
}

size_t
boundary_array_data_size(const CellBoundary * boundaries, int num)
{
	/* byte order + type + srid + # of polygons */
	size_t		size = WKB_BYTE_SIZE + WKB_INT_SIZE * 3;

	/* boundaries */
	for (int i = 0; i < num; i++)
		size += boundary_data_size(&boundaries[i]);
	return size;
}

size_t
boundary_data_size(const CellBoundary * boundary)
{
	/* byte order + type + srid + # of rings */
	size_t		size = WKB_BYTE_SIZE + WKB_INT_SIZE * 3;

	/* points */
	if (!boundary_is_empty(boundary))
	{
		int			numVerts = boundary->numVerts;

		if (!boundary_is_closed(boundary))
			numVerts++;
		/* # of points, point data */
		size += WKB_INT_SIZE + numVerts * WKB_DOUBLE_SIZE * 2;
	}
	return size;
}

size_t
linked_geo_polygon_data_size(const LinkedGeoPolygon * multiPolygon)
{
	size_t		size = 0;
	int			isMulti = (multiPolygon->next != NULL);

	/* byte order + type + srid */
	size = WKB_BYTE_SIZE + WKB_INT_SIZE * 2;

	if (isMulti)
	{
		/* # of polygons */
		size += WKB_INT_SIZE;
	}

	FOREACH_LINKED_POLYGON(multiPolygon, polygon)
	{
		if (isMulti)
		{
			/* byte order + type + srid  */
			size += WKB_BYTE_SIZE + WKB_INT_SIZE * 2;
		}

		/* # of rings */
		size += WKB_INT_SIZE;

		FOREACH_LINKED_LOOP(polygon, loop)
		{
			size += linked_geo_loop_data_size(loop);
		}
	}

	return size;
}

size_t
linked_geo_loop_data_size(const LinkedGeoLoop * loop)
{
	/* ring size */
	size_t		size = WKB_INT_SIZE;

	/* point data (including closing point) */
	size += WKB_DOUBLE_SIZE * (count_linked_lat_lng(loop) + 1) * 2;

	return size;
}

uint8 *
wkb_write_boundary_array_data(uint8 *data, const CellBoundary * boundaries, int num)
{
	/* byte order */
	data = wkb_write_endian(data);
	/* type */
	data = wkb_write_int(data, WKB_MULTIPOLYGON_TYPE | WKB_SRID_FLAG);
	/* SRID */
	data = wkb_write_int(data, WKB_SRID_DEFAULT);
	/* # of polygons */
	data = wkb_write_int(data, num);
	/* polygons */
	for (int i = 0; i < num; i++)
		data = wkb_write_boundary_data(data, &boundaries[i]);
	return data;
}

uint8 *
wkb_write_boundary_data(uint8 *data, const CellBoundary * boundary)
{
	/* byte order */
	data = wkb_write_endian(data);
	/* type */
	data = wkb_write_int(data, WKB_POLYGON_TYPE | WKB_SRID_FLAG);
	/* SRID */
	data = wkb_write_int(data, WKB_SRID_DEFAULT);
	/* # of rings */
	data = wkb_write_int(data, boundary_is_empty(boundary) ? 0 : 1);
	/* exterior ring */
	if (!boundary_is_empty(boundary))
	{
		bool		is_closed = boundary_is_closed(boundary);

		data = wkb_write_int(data, boundary->numVerts + (is_closed ? 0 : 1));
		data = wkb_write_lat_lng_array(data, boundary->verts, boundary->numVerts);
		/* close the ring */
		if (!is_closed)
			data = wkb_write_lat_lng(data, &boundary->verts[0]);
	}
	return data;
}

uint8 *
wkb_write_linked_geo_polygon_data(uint8 *data, const LinkedGeoPolygon * multiPolygon)
{
	int			isMulti = (multiPolygon->next != NULL);
	int			type = isMulti ? WKB_MULTIPOLYGON_TYPE : WKB_POLYGON_TYPE;

	/* byte order */
	data = wkb_write_endian(data);
	/* type */
	data = wkb_write_int(data, type | WKB_SRID_FLAG);
	/* SRID */
	data = wkb_write_int(data, WKB_SRID_DEFAULT);

	if (isMulti)
	{
		/* # of polygons */
		data = wkb_write_int(data, count_linked_polygons(multiPolygon));
	}

	FOREACH_LINKED_POLYGON(multiPolygon, polygon)
	{
		if (isMulti)
		{
			/* byte order */
			data = wkb_write_endian(data);
			/* type */
			data = wkb_write_int(data, WKB_POLYGON_TYPE | WKB_SRID_FLAG);
			/* SRID */
			data = wkb_write_int(data, WKB_SRID_DEFAULT);
		}

		/* # of rings */
		data = wkb_write_int(data, count_linked_geo_loops(polygon));

		/* rings */
		FOREACH_LINKED_LOOP(polygon, loop)
		{
			data = wkb_write_linked_geo_loop_data(data, loop);
		}
	}

	return data;
}

uint8 *
wkb_write_lat_lng_array(uint8 *data, const LatLng * coords, int num)
{
	for (int i = 0; i < num; i++)
		data = wkb_write_lat_lng(data, &coords[i]);
	return data;
}

uint8 *
wkb_write_linked_geo_loop_data(uint8 *data, const LinkedGeoLoop * loop)
{
	/* # of points (including closing point) */
	data = wkb_write_int(data, count_linked_lat_lng(loop) + 1);

	/* point data */
	FOREACH_LINKED_LAT_LNG(loop, latlng)
	{
		data = wkb_write_lat_lng(data, &latlng->vertex);
	}
	/* closing point data */
	data = wkb_write_lat_lng(data, &loop->first->vertex);

	return data;
}

uint8 *
wkb_write_lat_lng(uint8 *data, const LatLng * coord)
{
	data = wkb_write(data, &coord->lng, sizeof(coord->lng));
	data = wkb_write(data, &coord->lat, sizeof(coord->lat));
	return data;
}

uint8 *
wkb_write_endian(uint8 *data)
{
	/* Always use native order */
	uint32		order = 0x00000001;

	data[0] = ((uint8 *) &order)[0] ? WKB_NDR : WKB_XDR;
	return data + 1;
}

uint8 *
wkb_write_int(uint8 *data, uint32 value)
{
	return wkb_write(data, &value, sizeof(value));
}

uint8 *
wkb_write(uint8 *data, const void *value, size_t size)
{
	memcpy(data, value, size);
	return data + size;
}
