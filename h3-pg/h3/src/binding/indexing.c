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

#if POSTGRESQL_VERSION_MAJOR >= 16
#include "varatt.h" //VAR_SIZE and friends moved to here from postgres.h
#endif

#include <h3api.h>

#include <fmgr.h>			 // PG_FUNCTION_INFO_V1
#include <utils/geo_decls.h> // PG_GETARG_POINT_P
#include <math.h> // fabs

#include "constants.h"
#include "error.h"
#include "type.h"
#include "guc.h"

PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_latlng_to_cell);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_cell_to_latlng);
PGDLLEXPORT PG_FUNCTION_INFO_V1(h3_cell_to_boundary);

/* Indexes the location at the specified resolution */
Datum
h3_latlng_to_cell(PG_FUNCTION_ARGS)
{
	H3Index		cell;
	LatLng		location;
	Point	   *point = PG_GETARG_POINT_P(0);
	int			resolution = PG_GETARG_INT32(1);

	if (h3_guc_strict)
	{
		ASSERT(
			   point->x >= -180 && point->x <= 180,
			   ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE,
			   "Longitude must be between -180 and 180 degrees inclusive, but got %f.",
			   point->x
			);
		ASSERT(
			   point->y >= -90 && point->y <= 90,
			   ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE,
		"Latitude must be between -90 and 90 degrees inclusive, but got %f.",
			   point->y
			);
	}

	location.lng = degsToRads(point->x);
	location.lat = degsToRads(point->y);

	h3_assert(latLngToCell(&location, resolution, &cell));

	PG_FREE_IF_COPY(point, 0);
	PG_RETURN_H3INDEX(cell);
}

/* Finds the centroid of the index */
Datum
h3_cell_to_latlng(PG_FUNCTION_ARGS)
{
	LatLng		center;
	Point	   *point = palloc(sizeof(Point));
	H3Index		cell = PG_GETARG_H3INDEX(0);

	h3_assert(cellToLatLng(cell, &center));

	point->x = radsToDegs(center.lng);
	point->y = radsToDegs(center.lat);

	PG_RETURN_POINT_P(point);
}

/* Finds the boundary of the index */
Datum
h3_cell_to_boundary(PG_FUNCTION_ARGS)
{
	H3Index		cell = PG_GETARG_H3INDEX(0);

	double		delta,
				firstLon,
				lon,
				lat;
	int			size;
	POLYGON    *polygon;
	CellBoundary boundary;

	/* DEPRECATION BEGIN: Remove next major */
	bool		extend;

	if (PG_NARGS() == 1)
	{
		extend = h3_guc_extend_antimeridian;
	}
	else
	{
		extend = PG_GETARG_BOOL(1);
		H3_DEPRECATION("Please use `SET h3.extend_antimeridian TO true` instead of extend flag");
	}
	/* DEPRECATION END */

	h3_assert(cellToBoundary(cell, &boundary));

	size = offsetof(POLYGON, p) +sizeof(polygon->p[0]) * boundary.numVerts;
	polygon = (POLYGON *) palloc(size);
	SET_VARSIZE(polygon, size);
	polygon->npts = boundary.numVerts;

	firstLon = boundary.verts[0].lng;
	if (firstLon < 0)
	{
		delta = -2 * M_PI;
	}
	else
	{
		delta = +2 * M_PI;
	}

	for (int v = 0; v < boundary.numVerts; v++)
	{
		lon = boundary.verts[v].lng;
		lat = boundary.verts[v].lat;

		/* check if different sign */
		if (extend && fabs(lon - firstLon) > M_PI)
			lon = lon + delta;

		polygon->p[v].x = radsToDegs(lon);
		polygon->p[v].y = radsToDegs(lat);
	}

	PG_RETURN_POLYGON_P(polygon);
}
