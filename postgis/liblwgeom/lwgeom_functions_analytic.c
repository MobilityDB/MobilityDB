/**********************************************************************
 *
 * PostGIS - Spatial Types for PostgreSQL
 * http://postgis.net
 *
 * PostGIS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * PostGIS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PostGIS.  If not, see <http://www.gnu.org/licenses/>.
 *
 **********************************************************************
 *
 * Copyright (C) 2001-2005 Refractions Research Inc.
 *
 **********************************************************************/

/* MobilityDB: We only include from the original file the functions necessary
 * to test the inclusion of a point in a polygon. This file is originally
 * located in the /postgis directory */

#include <liblwgeom.h>

/*******************************************************************************
 * The following is based on the "Fast Winding Number Inclusion of a Point
 * in a Polygon" algorithm by Dan Sunday.
 * http://softsurfer.com/Archive/algorithm_0103/algorithm_0103.htm#Winding%20Number
 ******************************************************************************/

/*
 * returns: >0 for a point to the left of the segment,
 *          <0 for a point to the right of the segment,
 *          0 for a point on the segment
 */
static double determineSide(const POINT2D *seg1, const POINT2D *seg2, const POINT2D *point)
{
	return ((seg2->x-seg1->x)*(point->y-seg1->y)-(point->x-seg1->x)*(seg2->y-seg1->y));
}

/*
 * This function doesn't test that the point falls on the line defined by
 * the two points.  It assumes that that has already been determined
 * by having determineSide return within the tolerance.  It simply checks
 * that if the point is on the line, it is within the endpoints.
 *
 * returns: 1 if the point is not outside the bounds of the segment
 *          0 if it is
 */
static int isOnSegment(const POINT2D *seg1, const POINT2D *seg2, const POINT2D *point)
{
	double maxX;
	double maxY;
	double minX;
	double minY;

	if (seg1->x > seg2->x)
	{
		maxX = seg1->x;
		minX = seg2->x;
	}
	else
	{
		maxX = seg2->x;
		minX = seg1->x;
	}
	if (seg1->y > seg2->y)
	{
		maxY = seg1->y;
		minY = seg2->y;
	}
	else
	{
		maxY = seg2->y;
		minY = seg1->y;
	}

	if (maxX < point->x || minX > point->x)
	{
		return 0;
	}
	else if (maxY < point->y || minY > point->y)
	{
		return 0;
	}
	return 1;
}

/*
 * return -1 iff point is outside ring pts
 * return 1 iff point is inside ring pts
 * return 0 iff point is on ring pts
 */
static int point_in_ring(POINTARRAY *pts, const POINT2D *point)
{
	int wn = 0;
	uint32_t i;
	double side;
	const POINT2D* seg1;
	const POINT2D* seg2;

	seg2 = getPoint2d_cp(pts, 0);
	for (i=0; i<pts->npoints-1; i++)
	{
		seg1 = seg2;
		seg2 = getPoint2d_cp(pts, i+1);

		side = determineSide(seg1, seg2, point);

		/* zero length segments are ignored. */
		if ((seg2->x == seg1->x) && (seg2->y == seg1->y))
		{
			continue;
		}

		/* a point on the boundary of a ring is not contained. */
		/* WAS: if (fabs(side) < 1e-12), see #852 */
		if (side == 0.0)
		{
			if (isOnSegment(seg1, seg2, point) == 1)
			{
				return 0;
			}
		}

		/*
		 * If the point is to the left of the line, and it's rising,
		 * then the line is to the right of the point and
		 * circling counter-clockwise, so increment.
		 */
		if ((seg1->y <= point->y) && (point->y < seg2->y) && (side > 0))
		{
			++wn;
		}
		/*
		 * If the point is to the right of the line, and it's falling,
		 * then the line is to the right of the point and circling
		 * clockwise, so decrement.
		 */
		else if ((seg2->y <= point->y) && (point->y < seg1->y) && (side < 0))
		{
			--wn;
		}
	}

	if (wn == 0)
		return -1;
	return 1;
}

/*
 * return -1 iff point outside polygon
 * return 0 iff point on boundary
 * return 1 iff point inside polygon
 */
int point_in_polygon(LWPOLY *polygon, LWPOINT *point)
{
	uint32_t i;
	int result, in_ring;
	POINT2D pt;

	getPoint2d_p(point->point, 0, &pt);
	/* assume bbox short-circuit has already been attempted */

	/* everything is outside of an empty polygon */
	if ( polygon->nrings == 0 ) return -1;

	in_ring = point_in_ring(polygon->rings[0], &pt);
	if ( in_ring == -1) /* outside the exterior ring */
	{
		return -1;
	}
	result = in_ring;

	for (i=1; i<polygon->nrings; i++)
	{
		in_ring = point_in_ring(polygon->rings[i], &pt);
		if (in_ring == 1) /* inside a hole => outside the polygon */
		{
			return -1;
		}
		if (in_ring == 0) /* on the edge of a hole */
		{
			return 0;
		}
	}
	return result; /* -1 = outside, 0 = boundary, 1 = inside */
}

/*
 * return -1 iff point outside multipolygon
 * return 0 iff point on multipolygon boundary
 * return 1 iff point inside multipolygon
 */
int point_in_multipolygon(LWMPOLY *mpolygon, LWPOINT *point)
{
	uint32_t i, j;
	int result, in_ring;
	POINT2D pt;

	getPoint2d_p(point->point, 0, &pt);
	/* assume bbox short-circuit has already been attempted */

	result = -1;

	for (j = 0; j < mpolygon->ngeoms; j++ )
	{

		LWPOLY *polygon = mpolygon->geoms[j];

		/* everything is outside of an empty polygon */
		if ( polygon->nrings == 0 ) continue;

		in_ring = point_in_ring(polygon->rings[0], &pt);
		if ( in_ring == -1) /* outside the exterior ring */
		{
			continue;
		}
		if ( in_ring == 0 )
		{
			return 0;
		}

		result = in_ring;

		for (i=1; i<polygon->nrings; i++)
		{
			in_ring = point_in_ring(polygon->rings[i], &pt);
			if (in_ring == 1) /* inside a hole => outside the polygon */
			{
				result = -1;
				break;
			}
			if (in_ring == 0) /* on the edge of a hole */
			{
				return 0;
			}
		}
		if ( result != -1)
		{
			return result;
		}
	}
	return result;
}

/*******************************************************************************
 * End of "Fast Winding Number Inclusion of a Point in a Polygon" derivative.
 ******************************************************************************/
