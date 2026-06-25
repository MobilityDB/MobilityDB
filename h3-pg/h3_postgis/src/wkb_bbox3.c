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

#include <math.h>
#include <string.h>

#include "wkb_bbox3.h"
#include "wkb_linked_geo.h"

typedef struct
{
	double		x;
	double		y;
}	Vect2;

static void
			vect2_normalize(Vect2 * vect);

static short
			vect2_segment_side(const Vect2 * start, const Vect2 * end, const Vect2 * point);

static void
			bbox3_merge_vect3(const Vect3 * vect, Bbox3 * bbox);

void
bbox3_from_vect3(const Vect3 * vect, Bbox3 * bbox)
{
	bbox->xmin = bbox->xmax = vect->x;
	bbox->ymin = bbox->ymax = vect->y;
	bbox->zmin = bbox->zmax = vect->z;
}

void
bbox3_merge(const Bbox3 * other, Bbox3 * bbox)
{
	if (other->xmin < bbox->xmin)
		bbox->xmin = other->xmin;
	if (other->xmax > bbox->xmax)
		bbox->xmax = other->xmax;
	if (other->ymin < bbox->ymin)
		bbox->ymin = other->ymin;
	if (other->ymax > bbox->ymax)
		bbox->ymax = other->ymax;
	if (other->zmin < bbox->zmin)
		bbox->zmin = other->zmin;
	if (other->zmax > bbox->zmax)
		bbox->zmax = other->zmax;
}

void
bbox3_from_linked_loop(const LinkedGeoLoop * loop, Bbox3 * bbox)
{
	Vect3		vect,
				nextVect;

	vect3_from_lat_lng(&loop->first->vertex, &vect);
	bbox3_from_vect3(&vect, bbox);
	if (!loop->first->next)
		return;

	FOREACH_LINKED_LAT_LNG_PAIR(loop, cur, next)
	{
		Bbox3		segmentBbox;

		vect3_from_lat_lng(&next->vertex, &nextVect);

		if (!vect3_eq(&vect, &nextVect))
		{
			bbox3_from_segment_vect3(&vect, &nextVect, &segmentBbox);
			bbox3_merge(&segmentBbox, bbox);
		}

		vect = nextVect;
	}
}

int
bbox3_contains_vect3(const Bbox3 * bbox, const Vect3 * vect)
{
	return bbox->xmin <= vect->x && vect->x <= bbox->xmax
		&& bbox->ymin <= vect->y && vect->y <= bbox->ymax
		&& bbox->zmin <= vect->z && vect->z <= bbox->zmax;
}

int
bbox3_contains_lat_lng(const Bbox3 * bbox, const LatLng * coord)
{
	Vect3		vect;

	vect3_from_lat_lng(coord, &vect);
	return bbox3_contains_vect3(bbox, &vect);
}

void
bbox3_from_segment_vect3(const Vect3 * vect1, const Vect3 * vect2, Bbox3 * bbox)
{
	Vect3		vn,
				vect3;
	Vect3		axes[6];
	Vect2		r1,
				r2,
				orig;
	short		orig_side;

	/* Init bbox */
	bbox3_from_vect3(vect1, bbox);
	bbox3_merge_vect3(vect2, bbox);

	/* Check if end points are the same */
	if (vect3_eq(vect1, vect2))
		return;

	/* Normal of plain containing endpoint vectors */
	vect3_cross(vect1, vect2, &vn);
	vect3_normalize(&vn);

	/* Vector orthogonal to first endpoint vector */
	vect3_cross(&vn, vect1, &vect3);

	/* Project endpoint vectors onto the plane, using vect1/vect3 as basis */
	r1.x = 1.0;
	r1.y = 0.0;
	r2.x = vect3_dot(vect2, vect1);
	r2.y = vect3_dot(vect2, &vect3);

	/* Origin side relative to the projected segment (r1, r2) */
	orig.x = 0.0;
	orig.y = 0.0;
	orig_side = vect2_segment_side(&r1, &r2, &orig);

	/* Axis points: (1, 0, 0), (-1, 0, 0), (0, 1, 0), ... */
	memset(axes, 0, 6 * sizeof(Vect3));
	axes[0].x = axes[2].y = axes[4].z = 1.0;
	axes[1].x = axes[3].y = axes[5].z = -1.0;

	for (int i = 0; i < 6; ++i)
	{
		/* Project axis onto the plane, normalize */
		Vect2		rx;

		rx.x = vect3_dot(&axes[i], vect1);
		rx.y = vect3_dot(&axes[i], &vect3);
		vect2_normalize(&rx);

		/* Is projected axis vector between r1 and r2 */
		/* (is origin on the opposite side of segment (r1, r2))? */
		if (vect2_segment_side(&r1, &r2, &rx) != orig_side)
		{
			Vect3		vx;

			vx.x = rx.x * vect1->x + rx.y * vect3.x;
			vx.y = rx.x * vect1->y + rx.y * vect3.y;
			vx.z = rx.x * vect1->z + rx.y * vect3.z;
			bbox3_merge_vect3(&vx, bbox);
		}
	}
}

void
bbox3_from_segment_lat_lng(const LatLng * coord1, const LatLng * coord2, Bbox3 * bbox)
{
	Vect3		vect1,
				vect2;

	vect3_from_lat_lng(coord1, &vect1);
	vect3_from_lat_lng(coord2, &vect2);

	bbox3_from_segment_vect3(&vect1, &vect2, bbox);
}

static void
vect2_normalize(Vect2 * vect)
{
	double		len = sqrt(vect->x * vect->x + vect->y * vect->y);

	if (len > 0.0)
	{
		vect->x /= len;
		vect->y /= len;
	}
	else
	{
		vect->x = 0.0;
		vect->y = 0.0;
	}
}

static short
vect2_segment_side(const Vect2 * start, const Vect2 * end, const Vect2 * point)
{
	double		side = (point->x - start->x) * (end->y - start->y)
	- (end->x - start->x) * (point->y - start->y);

	return (side == 0) ? 0 : (side < 0) ? -1 : 1;
}

static void
bbox3_merge_vect3(const Vect3 * vect, Bbox3 * bbox)
{
	Bbox3		vect_bbox;

	bbox3_from_vect3(vect, &vect_bbox);
	bbox3_merge(&vect_bbox, bbox);
}
