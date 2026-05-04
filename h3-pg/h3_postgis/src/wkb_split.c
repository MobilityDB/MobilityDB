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

#include <fmgr.h>
#include <float.h>
#include <math.h>

#include "constants.h"
#include "wkb_split.h"
#include "wkb_bbox3.h"
#include "wkb_linked_geo.h"

/*

Example: O-shaped polygon crossed by antimeridian.

After intersections are found and sorted by latitude,
intersection pairs 0-1 and 2-3 become segments in
exterior rings of the polygons in result.

		  |
  +------(0)------+
  |		  |		  |
  |  +---(1)---+  |
  |  |	  |    |  |
  |  |	  |    |  |
  |  +---(2)---+  |
  |		  |		  |
  +------(3)------+
		  |


Algorithm overview:

1. Initialization:
  - empty array of vertices
  - empty array of intersections
  - empty array of interior rings not split by antimeridian

2. Processing polygon rings:
  for each ring in the polygon:
	if ring is crossed by prime or antimeridian:
	  for each segment in the ring:
		- add first endpoint to array of vertices
		if segment crosses antimeridian:
		  - add an intersection, link to first endpoint
	else:
	  - add ring to array of non-split holes

3. Preparing data:
  - sort intersections by latitude
  - set sort order value for each intersection

4. Building multipolygon:
  while there are unused vertices:
	- create empty exterior ring
	- start traversing vertex array forward starting from next unused vertex
	while current vertex is unused:
	  - add current vertex to exterior ring
	  - get next vertex (depends on traversal direction)
	  if there is an intersection between vertices:
		- add intersection point to exterior ring
		- get adjacent intersection from sorted array
		- add next intersection point to exterior ring
		- update traversal direction based on intersection direction value
		- get next vertex
	  - move to text vertex
	- check which non-split holes are inside the exterior ring and add them to polygon
	- add polygon to result

 */

#define SIGN(x) ((x < 0) ? -1 : (x > 0) ? 1 \
										: 0)

#define FP_EQUAL(v1, v2) ((v1) == (v2) || fabs((v1) - (v2)) < DBL_EPSILON)

#define SPLIT_ASSERT(condition, message)			\
	ASSERT(										\
		condition,									\
		ERRCODE_EXTERNAL_ROUTINE_EXCEPTION,		\
		message)

#define SPLIT_ASSERT_VALID_VERTEX_IDX(split, idx) \
	SPLIT_ASSERT(0 <= idx && idx < split->vertexNum, "vertex index out of bounds")

#define INTERSECT_ARRAY_SIZE_INIT (4)

/* Direction of segment intersecting antimeridian: */
/* West-to-East or East-to-West */
typedef enum
{
	SplitIntersectDir_None = 0,
	SplitIntersectDir_WE,
	SplitIntersectDir_EW
}	SplitIntersectDir;

typedef struct
{
	SplitIntersectDir dir;
	bool		isPrime;
	double		lat;
	int			vertexIdx;
	int			sortOrder;
}	SplitIntersect;

typedef struct
{
	const LatLng *latlngPtr;
	int			intersectIdx;
	short		sign;			/* longitude sign is set explicitly in case
								 * longitude of the vertex itself is zero */
	int			link;			/* links first and last vertices in a ring */
}	SplitVertex;

typedef struct
{
	/* Vertices */
	int			vertexNum;
	SplitVertex *vertices;

	/* Intersections */
	int			maxIntersectNum;
	int			intersectNum;
	SplitIntersect *intersects;
	SplitIntersect **sortedIntersects;

	/* Non-split holes */
	int			holeNum;
	const LinkedGeoLoop **holes;
}	Split;

static bool
			is_polygon_crossed(const LinkedGeoPolygon * polygon);

static LinkedGeoPolygon *
			split_polygon(const LinkedGeoPolygon * polygon);

static bool
			is_ring_crossed(const LinkedGeoLoop * ring);

static void
			split_init(Split * split, int ringNum, int vertexNum);

static void
			split_cleanup(Split * split);

static void
			split_process_ring(Split * split, const LinkedGeoLoop * ring);

static void
			split_prepare(Split * split);

static LinkedGeoPolygon *
			split_create_multi_polygon(Split * split);

static int
			split_add_vertex(Split * split, const LatLng * latlng);

static void
			split_add_intersect_after(Split * split, int vertexIdx, SplitIntersectDir dir, bool isPrime, double lat);

static int
			split_add_intersect(Split * split, SplitIntersectDir dir, bool isPrime, double lat);

static void
			split_link_vertices(Split * split, int idx1, int idx2);

static void
			split_add_hole(Split * split, const LinkedGeoLoop * hole);

static
void		split_sort_intersects(Split * split);

static int
			split_intersect_ptr_cmp(const void *a, const void *b);

static int
			split_find_next_vertex(Split * split, int *start);

static LinkedGeoPolygon *
			split_create_polygon_vertex(Split * split, int vertexIdx);

static void
			split_polygon_assign_holes(Split * split, short sign, LinkedGeoPolygon * polygon);

static const SplitIntersect *
			split_get_intersect_after(Split * split, int vertexIdx);

static void
			split_intersect_get_lat_lng(const SplitIntersect * intersect, short sign, LatLng * latlng);

static int
			count_polygon_vertices(const LinkedGeoPolygon * polygon, int *ringNum);

static short
			lat_lng_ring_pos(const LinkedGeoLoop * ring, short sign, const Bbox3 * bbox, const LatLng * latlng);

static short
			segment_intersect(const Vect3 * v1, const Vect3 * v2, const Vect3 * u1, const Vect3 * u2);

static short
			point_segment_pos(const Vect3 * v1, const Vect3 * v2, const Vect3 * p);

static void
			add_lat_lng(LinkedGeoLoop * loop, const LatLng * latlng);

bool
is_linked_polygon_crossed_by_180(const LinkedGeoPolygon * multiPolygon)
{
	FOREACH_LINKED_POLYGON(multiPolygon, polygon)
	{
		if (is_polygon_crossed(polygon))
			return true;
	}
	return false;
}

LinkedGeoPolygon *
split_linked_polygon_by_180(const LinkedGeoPolygon * multiPolygon)
{
	LinkedGeoPolygon *result = NULL;
	LinkedGeoPolygon *last = NULL;

	FOREACH_LINKED_POLYGON(multiPolygon, polygon)
	{
		/* Split or copy next polygon */
		LinkedGeoPolygon *nextResult = is_polygon_crossed(polygon)
		? split_polygon(polygon)
		: copy_linked_geo_polygon(polygon);

		/* Add to result */
		if (!result)
		{
			result = nextResult;
			last = nextResult;
		}
		else
		{
			last->next = nextResult;
		}
		while (last->next)
			last = last->next;
	}

	return result;
}

double
split_180_lat(const LatLng * coord1, const LatLng * coord2)
{
	Vect3		p1,
				p2,
				normal,
				s;
	double		y;

	/* Normal of circle containing points: normal = p1 x p2 */
	vect3_from_lat_lng(coord1, &p1);
	vect3_from_lat_lng(coord2, &p2);
	vect3_cross(&p1, &p2, &normal);

	/* y coordinate of 0/180 meridian circle normal */
	y = (coord1->lng < 0 || coord2->lng > 0) ? -1 : 1;

	/* Circle plane intersection vector: s = (p1 x p2) x {0, y, 0} */
	s.x = -(normal.z * y);
	s.y = 0;
	s.z = normal.x * y;
	vect3_normalize(&s);		/* intersection point coordinates on unit
								 * sphere */

	return asin(s.z);			/* latitude */
}

bool
is_polygon_crossed(const LinkedGeoPolygon * polygon)
{
	return polygon->first
		? is_ring_crossed(polygon->first)
		: false;
}

LinkedGeoPolygon *
split_polygon(const LinkedGeoPolygon * polygon)
{
	int			ringNum,
				vertexNum;
	Split		split;
	LinkedGeoPolygon *result;

	/* Init data */
	vertexNum = count_polygon_vertices(polygon, &ringNum);
	split_init(&split, ringNum, vertexNum);

	/* Process rings */
	FOREACH_LINKED_LOOP(polygon, ring)
	{
		if (ring == polygon->first || is_ring_crossed(ring))
			split_process_ring(&split, ring);
		else
			split_add_hole(&split, ring);
	}

	/* Prepare data */
	split_prepare(&split);

	/* Build result */
	result = split_create_multi_polygon(&split);

	/* Cleanup */
	split_cleanup(&split);

	return result;
}

bool
is_ring_crossed(const LinkedGeoLoop * ring)
{
	if (!ring->first || !ring->first->next)
		return false;

	FOREACH_LINKED_LAT_LNG_PAIR(ring, cur, next)
	{
		double		lng = cur->vertex.lng;
		double		nextLng = next->vertex.lng;

		if (SIGN(lng) != SIGN(nextLng)
			&& fabs(lng - nextLng) > M_PI)
		{
			return true;
		}
	}
	return false;
}


void
split_init(Split * split, int ringNum, int vertexNum)
{
	*split = (Split)
	{
		0
	};

	split->vertices = palloc0(vertexNum * sizeof(SplitVertex));

	split->maxIntersectNum = INTERSECT_ARRAY_SIZE_INIT;
	split->intersects = palloc0(split->maxIntersectNum * sizeof(SplitIntersect));

	if (ringNum > 1)
		split->holes = palloc0((ringNum - 1) * sizeof(LinkedGeoLoop *));
}

void
split_cleanup(Split * split)
{
	if (split->vertices)
		pfree(split->vertices);
	if (split->intersects)
		pfree(split->intersects);
	if (split->sortedIntersects)
		pfree(split->sortedIntersects);
	if (split->holes)
		pfree(split->holes);

	*split = (Split)
	{
		0
	};
}

void
split_process_ring(Split * split, const LinkedGeoLoop * ring)
{
	short		sign = 0;
	int			vertexIdx = -1;
	int			firstVertexIdx = -1;

	SPLIT_ASSERT(ring->first && ring->first->next, "polygon ring must have at least 2 vertices");

	FOREACH_LINKED_LAT_LNG_PAIR(ring, cur, next)
	{
		double		lng,
					nextLng;
		short		nextSign;

		/* Add vertex */
		vertexIdx = split_add_vertex(split, &cur->vertex);
		if (firstVertexIdx < 0)
			firstVertexIdx = vertexIdx;

		lng = cur->vertex.lng;
		nextLng = next->vertex.lng;
		nextSign = SIGN(nextLng);

		if (sign == 0)
		{
			sign = SIGN(lng);
			if (sign != 0)
			{
				/* Set sign for vertices traversed so far */
				for (int i = firstVertexIdx; i <= vertexIdx; ++i)
					split->vertices[i].sign = sign;
			}
		}
		else
		{
			/* Set vertex sign */
			split->vertices[vertexIdx].sign = sign;
		}

		if (sign != 0 && nextSign != 0 && nextSign != sign)
		{
			/* Prime or antimeridian crossed */
			/* Add intersection after current vertex */
			SplitIntersectDir dir = (sign < 0) ? SplitIntersectDir_WE : SplitIntersectDir_EW;
			int			isPrime = (fabs(lng - nextLng) < M_PI);
			double		lat = split_180_lat(&cur->vertex, &next->vertex);

			split_add_intersect_after(split, vertexIdx, dir, isPrime, lat);

			sign = nextSign;
		}
	}

	/* Link first and last vertices */
	split_link_vertices(split, firstVertexIdx, vertexIdx);
}

void
split_prepare(Split * split)
{
	split_sort_intersects(split);
}

LinkedGeoPolygon *
split_create_multi_polygon(Split * split)
{
	LinkedGeoPolygon *multiPolygon = NULL;
	LinkedGeoPolygon *lastPolygon = NULL;
	int			vertexIdxStart = 0;

	while (true)
	{
		LinkedGeoPolygon *polygon;

		/* Get next unused vertex */
		int			vertexIdx = split_find_next_vertex(split, &vertexIdxStart);

		if (vertexIdx < 0)
			break;				/* done */

		/* Create next polygon */
		polygon = split_create_polygon_vertex(split, vertexIdx);

		/* Add to result */
		if (!multiPolygon)
			multiPolygon = polygon;
		else
			lastPolygon->next = polygon;
		lastPolygon = polygon;
	}

	return multiPolygon;
}

int
split_add_vertex(Split * split, const LatLng * latlng)
{
	int			idx = split->vertexNum++;
	SplitVertex *vertex = &split->vertices[idx];

	vertex->latlngPtr = latlng;
	vertex->intersectIdx = -1;
	vertex->sign = 0;
	vertex->link = -1;
	return idx;
}

void
split_add_intersect_after(Split * split, int vertexIdx, SplitIntersectDir dir, bool isPrime, double lat)
{
	int			idx = split_add_intersect(split, dir, isPrime, lat);
	SplitIntersect *intersect = &split->intersects[idx];

	intersect->vertexIdx = vertexIdx;
	split->vertices[vertexIdx].intersectIdx = idx;
}

int
split_add_intersect(Split * split, SplitIntersectDir dir, bool isPrime, double lat)
{
	int			idx;
	SplitIntersect *intersect;

	if (split->intersectNum == split->maxIntersectNum)
	{
		/* Reallocate memory for intersections */
		int			maxNum = split->maxIntersectNum * 2;

		if (maxNum > split->vertexNum)
			maxNum = split->vertexNum;
		split->intersects = repalloc(split->intersects, maxNum * sizeof(SplitIntersect));
		split->maxIntersectNum = maxNum;
	}

	idx = split->intersectNum++;
	intersect = &split->intersects[idx];
	intersect->dir = dir;
	intersect->isPrime = isPrime;
	intersect->lat = lat;
	intersect->vertexIdx = -1;
	intersect->sortOrder = -1;
	return idx;
}

void
split_link_vertices(Split * split, int idx1, int idx2)
{
	SPLIT_ASSERT_VALID_VERTEX_IDX(split, idx1);
	SPLIT_ASSERT_VALID_VERTEX_IDX(split, idx2);

	split->vertices[idx1].link = idx2;
	split->vertices[idx2].link = idx1;
}

void
split_add_hole(Split * split, const LinkedGeoLoop * hole)
{
	split->holes[split->holeNum++] = hole;
}

void
split_sort_intersects(Split * split)
{
	SPLIT_ASSERT(split->intersectNum % 2 == 0, "intersection number must be even");

	/* Collect intesection pointers */
	split->sortedIntersects = palloc(split->intersectNum * sizeof(SplitIntersect *));
	for (int i = 0; i < split->intersectNum; ++i)
	{
		SplitIntersect *intersect = &split->intersects[i];

		split->sortedIntersects[i] = intersect;
	}

	/* Sort intersection pointers */
	qsort(
		  split->sortedIntersects,
		  split->intersectNum,
		  sizeof(SplitIntersect *),
		  &split_intersect_ptr_cmp);
	/* Assign sort order values to intersections */
	for (int i = 0; i < split->intersectNum; ++i)
		split->sortedIntersects[i]->sortOrder = i;
}

/**
	Sort intersections by latitude

	Sort value for points on prime meridian:
	*  180 - lat, if lat >= 0
	* -180 - lat, if lat < 0

				   -90		   0		  90
			  ------*----------+----------*------>
	meridian:  prime	 antimeridian	   prime
	value:	 -180-lat		  lat		  180-lat
 */
int
split_intersect_ptr_cmp(const void *a, const void *b)
{
	const SplitIntersect *i1 = *((const SplitIntersect **) a);
	const SplitIntersect *i2 = *((const SplitIntersect **) b);
	double		v1,
				v2;

	v1 = i1->lat;
	if (i1->isPrime)
		v1 = ((v1 < 0) ? -M_PI : M_PI) - v1;

	v2 = i2->lat;
	if (i2->isPrime)
		v2 = ((v2 < 0) ? -M_PI : M_PI) - v2;

	return (v1 == v2)
		? 0
		: (v1 < v2) ? -1 : 1;
}

int
split_find_next_vertex(Split * split, int *start)
{
	for (int i = *start; i < split->vertexNum; ++i)
	{
		if (split->vertices[i].latlngPtr)
		{
			*start = i + 1;
			return i;
		}
	}
	return -1;
}

LinkedGeoPolygon *
split_create_polygon_vertex(Split * split, int vertexIdx)
{
	LinkedGeoPolygon *polygon;
	LinkedGeoLoop *loop;
	int			idx,
				nextIdx,
				intersectIdx;
	SplitVertex *vertex;
	const SplitIntersect *intersect;
	short		sign,
				step;

	polygon = palloc0(sizeof(LinkedGeoPolygon));

	loop = palloc0(sizeof(LinkedGeoLoop));
	add_linked_geo_loop(polygon, loop);

	idx = vertexIdx;
	vertex = &split->vertices[idx];
	sign = vertex->sign;
	step = 1;					/* vertex array traversal direction */
	while (vertex->latlngPtr)
	{
		/* Add vertex */
		add_lat_lng(loop, vertex->latlngPtr);
		vertex->latlngPtr = NULL;

		/*
		 * Get indices of the other segment endpoint and potential
		 * intersection
		 */
		if (vertex->link > -1 && ((step > 0) == (idx > vertex->link)))
		{
			nextIdx = vertex->link;
			/* Potential intersection is after last ring vertex */
			intersectIdx = (nextIdx > idx) ? nextIdx : idx;
		}
		else
		{
			nextIdx = idx + step;
			/* Potential intersection is after first vertex */
			intersectIdx = (nextIdx > idx) ? idx : nextIdx;
		}

		/* Is there an intersection after the vertex? */
		SPLIT_ASSERT_VALID_VERTEX_IDX(split, intersectIdx);
		intersect = split_get_intersect_after(split, intersectIdx);
		if (intersect)
		{
			LatLng		latlng;
			int			intersectSortOrder;

			/* Add intersection vertex */
			split_intersect_get_lat_lng(intersect, sign, &latlng);
			add_lat_lng(loop, &latlng);

			/* Find next intersection */
			intersectSortOrder = (intersect->sortOrder % 2 == 0)
				? intersect->sortOrder + 1
				: intersect->sortOrder - 1;
			intersect = split->sortedIntersects[intersectSortOrder];
			intersectIdx = intersect->vertexIdx;

			/* Add next intersection vertex */
			split_intersect_get_lat_lng(intersect, sign, &latlng);
			add_lat_lng(loop, &latlng);

			/*
			 * Does intersecting segment end in the same hemisphere where the
			 * polygon is located?
			 */
			step = ((sign > 0) == (intersect->dir == SplitIntersectDir_WE)) ? 1 : -1;
			if (step > 0)
			{
				/* Next vertex is the second endpoint of a segment */
				const SplitVertex *segmentStart = &split->vertices[intersectIdx];

				if (segmentStart->link > -1 && intersectIdx > segmentStart->link)
					nextIdx = segmentStart->link;
				else
					nextIdx = intersectIdx + 1;
			}
			else
			{
				/* Next vertex is the first endpoint of a segment */
				nextIdx = intersectIdx;
			}
		}

		idx = nextIdx;
		vertex = &split->vertices[idx];
	}

	/* Assign holes */
	split_polygon_assign_holes(split, sign, polygon);

	return polygon;
}

void
split_polygon_assign_holes(Split * split, short sign, LinkedGeoPolygon * polygon)
{
	const LinkedGeoLoop *outerLoop;
	Bbox3		bbox;

	outerLoop = polygon->first;
	bbox3_from_linked_loop(outerLoop, &bbox);

	for (int i = 0; i < split->holeNum; ++i)
	{
		const LinkedGeoLoop *hole;
		short		pos = 0;

		hole = split->holes[i];
		if (!hole)
			continue;

		/* Check if hole vertices are inside the outher shell of the polygon */
		FOREACH_LINKED_LAT_LNG(hole, cur)
		{
			pos = lat_lng_ring_pos(outerLoop, sign, &bbox, &cur->vertex);
			if (pos != 0)
				break;			/* vertex is either inside or outside */
		}

		if (pos != -1)
		{
			/* Add hole loop copy to polygon */
			LinkedGeoLoop *hole_copy = copy_linked_geo_loop(hole);

			add_linked_geo_loop(polygon, hole_copy);

			/* Remove hole from the list */
			split->holes[i] = NULL;
		}
	}
}

const SplitIntersect *
split_get_intersect_after(Split * split, int vertexIdx)
{
	int			idx;

	SPLIT_ASSERT_VALID_VERTEX_IDX(split, vertexIdx);

	idx = split->vertices[vertexIdx].intersectIdx;
	return (idx > -1)
		? &split->intersects[idx]
		: NULL;
}

void
split_intersect_get_lat_lng(const SplitIntersect * intersect, short sign, LatLng * latlng)
{
	latlng->lat = intersect->lat;
	if (intersect->isPrime)
		latlng->lng = 0.0;
	else
		latlng->lng = (sign > 0) ? M_PI : -M_PI;
}

int
count_polygon_vertices(const LinkedGeoPolygon * polygon, int *ringNum)
{
	int			num = 0;

	if (ringNum)
		*ringNum = 0;

	FOREACH_LINKED_LOOP(polygon, ring)
	{
		if (ringNum)
			++(*ringNum);
		num += count_linked_lat_lng(ring);
	}
	return num;
}

short
lat_lng_ring_pos(const LinkedGeoLoop * ring, short sign, const Bbox3 * bbox, const LatLng * latlng)
{
	short		signLatlng;
	Vect3		vect,
				outVect;
	LatLng		out;
	int			intersectNum = 0;
	Vect3		curVect,
				nextVect;

	/* Check longitude sign */
	signLatlng = SIGN(latlng->lng);
	if (signLatlng != 0 && signLatlng != sign)
		return -1;

	vect3_from_lat_lng(latlng, &vect);

	/* Check bbox */
	if (!bbox3_contains_vect3(bbox, &vect))
		return -1;

	/* Create a point that's guaranteed to be outside the polygon */
	out.lng = (latlng->lng == 0) ? -sign * 1e-10 : -latlng->lng;
	out.lat = latlng->lat;
	vect3_from_lat_lng(&out, &outVect);


	/* Check if ring is a single vertex exactly matching the point */
	if (!ring->first->next)
		return true;

	/*
	 * Count a number of intersections between the ring and (latlng, out)
	 * segment
	 */
	intersectNum = 0;
	vect3_from_lat_lng(&ring->first->vertex, &curVect);
	FOREACH_LINKED_LAT_LNG_PAIR(ring, cur, next)
	{
		short		intersect;

		/* Check if point matches ring vertex */
		if (vect3_eq(&vect, &curVect))
			return 0;

		/* Next vertex */
		vect3_from_lat_lng(&next->vertex, &nextVect);

		if (!vect3_eq(&curVect, &nextVect))
		{
			intersect = segment_intersect(&curVect, &nextVect, &vect, &outVect);
			if (intersect == 0)
				return 0;		/* point on ring segment */

			if (intersect > 0)
				++intersectNum;
		}

		curVect = nextVect;
	}

	return (intersectNum % 2 == 0) ? -1 : 1;
}

short
segment_intersect(const Vect3 * v1, const Vect3 * v2, const Vect3 * u1, const Vect3 * u2)
{
	Vect3		vn,
				un;
	short		v1Side,
				v2Side,
				u1Side,
				u2Side;
	double		normalDot;

	/* Normals of V and U planes */
	vect3_cross(v1, v2, &vn);
	vect3_normalize(&vn);
	vect3_cross(u1, u2, &un);
	vect3_normalize(&un);

	/* Are the planes the same? */
	normalDot = vect3_dot(&vn, &un);
	if (FP_EQUAL(fabs(normalDot), 1.0))
	{
		short		ret = point_segment_pos(v1, v2, u1);

		if (ret == -1)
			ret = point_segment_pos(v1, v2, u2);
		if (ret == -1)
			ret = point_segment_pos(u1, u2, v1);
		if (ret == -1)
			ret = point_segment_pos(u1, u2, v2);
		return ret;
	}

	/* Which side of other segment are segment endpoints? */
	v1Side = SIGN(vect3_dot(&un, v1));
	v2Side = SIGN(vect3_dot(&un, v2));
	u1Side = SIGN(vect3_dot(&vn, u1));
	u2Side = SIGN(vect3_dot(&vn, u2));

	/* v1 and v2 are on the same side of U plane */
	if (v1Side == v2Side && v1Side != 0)
		return -1;

	/* u1 and u2 are on the same side of V plane */
	if (u1Side == u2Side && u2Side != 0)
		return -1;

	if (v1Side != v2Side && (v1Side + v2Side) == 0
		&& u1Side != u2Side && (u1Side + u2Side) == 0)
	{
		/* Intersection point */
		Vect3		intersect;

		vect3_cross(&vn, &un, &intersect);
		vect3_normalize(&intersect);

		/* Is intersection point inside both arcs? */
		if (point_segment_pos(v1, v2, &intersect) != -1
			&& point_segment_pos(u1, u2, &intersect) != -1)
		{
			return 1;
		}

		/* Try antipodal intersection point */
		vect3_scale(&intersect, -1.0);
		if (point_segment_pos(v1, v2, &intersect) != -1
			&& point_segment_pos(u1, u2, &intersect) != -1)
		{
			return 1;
		}

		return -1;
	}

	return 0;
}

short
point_segment_pos(const Vect3 * v1, const Vect3 * v2, const Vect3 * p)
{
	Vect3		middle;
	double		minSimilarity;

	/* Check if point is same as one of segment endpoints */
	if (vect3_eq(p, v1) || vect3_eq(p, v2))
		return 0;

	/* Find vector bisecting anble between segment endpoint vectors */
	vect3_sum(v1, v2, &middle);
	vect3_normalize(&middle);

	/* How similar are endpoint vectors to bisecting vector? */
	minSimilarity = vect3_dot(v1, &middle);

	if (fabs(1.0 - minSimilarity) > 1e-10)
	{
		/* Segment is long enough to use dot product test. */
		/* If point vector is more similar to bisecting vector, */

		/*
		 * then it must be closer to center, so the point is inside the
		 * segment.
		 */
		return (vect3_dot(p, &middle) > minSimilarity) ? 1 : -1;
	}
	else
	{
		/* Segment is too short to use dot product test. */

		/*
		 * Check if vectors from segment endpoints to the point are in
		 * opposite directions.
		 */
		Vect3		d1,
					d2;

		vect3_diff(p, v1, &d1);
		vect3_normalize(&d1);
		vect3_diff(p, v2, &d2);
		vect3_normalize(&d2);
		return (vect3_dot(&d1, &d2) < 0.0) ? 1 : -1;
	}
}

void
add_lat_lng(LinkedGeoLoop * loop, const LatLng * latlng)
{
	LinkedLatLng *linked;

	if (loop->last)
	{
		/* Does new vertex exactly match the last one? */
		const LatLng *last = &loop->last->vertex;

		if (last->lat == latlng->lat && last->lng == latlng->lng)
			return;
	}

	linked = palloc0(sizeof(LinkedLatLng));
	linked->vertex.lat = latlng->lat;
	linked->vertex.lng = latlng->lng;
	add_linked_lat_lng(loop, linked);
}
