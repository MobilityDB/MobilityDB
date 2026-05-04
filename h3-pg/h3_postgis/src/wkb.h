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

#ifndef PGH3_WKB_H
#define PGH3_WKB_H

#include <postgres.h>
#include <h3api.h>

#include <fmgr.h>

#if POSTGRESQL_VERSION_MAJOR >= 16
#include "varatt.h" //VAR_SIZE and friends moved to here from postgres.h
#endif

bytea *
			boundary_array_to_wkb(const CellBoundary * boundaries, size_t num);

bytea *
			boundary_to_wkb(const CellBoundary * boundary);

bytea *
			linked_geo_polygon_to_wkb(const LinkedGeoPolygon * multiPolygon);

#endif
