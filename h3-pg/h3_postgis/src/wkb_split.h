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

#ifndef PGH3_WKB_SPLIT_H
#define PGH3_WKB_SPLIT_H

#include <h3api.h>

#include <stdbool.h>

#include "error.h"

bool
			is_linked_polygon_crossed_by_180(const LinkedGeoPolygon * multiPolygon);

LinkedGeoPolygon *
			split_linked_polygon_by_180(const LinkedGeoPolygon * multiPolygon);

double
			split_180_lat(const LatLng * coord1, const LatLng * coord2);

#endif
