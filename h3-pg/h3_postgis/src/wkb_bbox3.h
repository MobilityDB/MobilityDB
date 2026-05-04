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

#ifndef PGH3_WKB_BBOX3_H
#define PGH3_WKB_BBOX3_H

#include <h3api.h>

#include "wkb_vect3.h"

typedef struct
{
	double		xmin;
	double		xmax;
	double		ymin;
	double		ymax;
	double		zmin;
	double		zmax;
}	Bbox3;

void
			bbox3_from_vect3(const Vect3 * vect, Bbox3 * bbox);

void
			bbox3_merge(const Bbox3 * other, Bbox3 * bbox);

void
			bbox3_from_linked_loop(const LinkedGeoLoop * loop, Bbox3 * bbox);

int
			bbox3_contains_vect3(const Bbox3 * bbox, const Vect3 * vect);

int
			bbox3_contains_lat_lng(const Bbox3 * bbox, const LatLng * coord);

void
			bbox3_from_segment_vect3(const Vect3 * vect1, const Vect3 * vect2, Bbox3 * bbox);

void
			bbox3_from_segment_lat_lng(const LatLng * coord1, const LatLng * coord2, Bbox3 * bbox);

#endif
