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

#ifndef PGH3_VECT3_H
#define PGH3_VECT3_H

#include <postgres.h>
#include <h3api.h>

#include <fmgr.h>

typedef struct
{
	double		x;
	double		y;
	double		z;
}	Vect3;

void
			vect3_from_lat_lng(const LatLng * coord, Vect3 * vect);

void
			vect3_to_lat_lng(const Vect3 * vect, LatLng * coord);

int
			vect3_eq(const Vect3 * vect1, const Vect3 * vect2);

void
			vect3_normalize(Vect3 * vect);

void
			vect3_sum(const Vect3 * vect1, const Vect3 * vect2, Vect3 * sum);

void
			vect3_diff(const Vect3 * vect1, const Vect3 * vect2, Vect3 * diff);

void
			vect3_scale(Vect3 * vect, double factor);

void
			vect3_cross(const Vect3 * vect1, const Vect3 * vect2, Vect3 * prod);

double
			vect3_dot(const Vect3 * vect1, const Vect3 * vect2);

#endif
