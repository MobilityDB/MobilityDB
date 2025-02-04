/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @brief API of the Mobility Engine Open Source (MEOS) library.
 */

#ifndef __MEOS_GEO_H__
#define __MEOS_GEO_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
/* PostgreSQL */
#if MEOS
#include "postgres_int_defs.h"
#else
#include <postgres.h>
#include <utils/date.h>
#include <utils/timestamp.h>
#endif
/* PostGIS */
#include <liblwgeom.h>

/*===========================================================================*
 * Functions for PostGIS types
 *===========================================================================*/

extern uint8_t *geo_as_ewkb(const GSERIALIZED *gs, const char *endian, size_t *size);
extern char *geo_as_ewkt(const GSERIALIZED *gs, int precision);
extern char *geo_as_geojson(const GSERIALIZED *gs, int option, int precision, const char *srs);
extern char *geo_as_hexewkb(const GSERIALIZED *gs, const char *endian);
extern char *geo_as_text(const GSERIALIZED *gs, int precision);
extern GSERIALIZED *geo_from_text(const char *wkt, int32_t srid);
extern GSERIALIZED *geo_from_ewkb(const uint8_t *wkb, size_t wkb_size, int32 srid);
extern GSERIALIZED *geo_from_geojson(const char *geojson);
extern bool geo_is_empty(const GSERIALIZED *g);
extern char *geo_out(const GSERIALIZED *gs);
extern bool geo_same(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern int32_t geo_srid(const GSERIALIZED *gs);

extern double geog_area(const GSERIALIZED *g, bool use_spheroid);
extern double geog_distance(const GSERIALIZED *g1, const GSERIALIZED *g2);
extern bool geog_dwithin(const GSERIALIZED *g1, const GSERIALIZED *g2, double tolerance, bool use_spheroid);
extern GSERIALIZED *geog_from_geom(GSERIALIZED *geom);
extern GSERIALIZED *geog_from_hexewkb(const char *wkt);
extern GSERIALIZED *geog_from_text(const char *wkt, int32_t srid);
extern GSERIALIZED *geog_in(const char *str, int32 typmod);
extern bool geog_intersects(const GSERIALIZED *gs1, const GSERIALIZED *gs2, bool use_spheroid);
extern double geog_length(const GSERIALIZED *g, bool use_spheroid);
extern double geog_perimeter(const GSERIALIZED *g, bool use_spheroid);

extern GSERIALIZED *geom_array_union(GSERIALIZED **gsarr, int nelems);
extern bool geom_azimuth(const GSERIALIZED *gs1, const GSERIALIZED *gs2, double *result);
extern GSERIALIZED *geom_boundary(const GSERIALIZED *gs);
extern GSERIALIZED *geom_buffer(const GSERIALIZED *gs, double size, char *params);
extern bool geom_contains(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern GSERIALIZED *geom_convex_hull(const GSERIALIZED *gs);
extern bool geom_covers(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern double geom_distance2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern double geom_distance3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern bool geom_dwithin2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2, double tolerance);
extern bool geom_dwithin3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2, double tolerance);
extern GSERIALIZED *geom_from_geog(GSERIALIZED *geog);
extern GSERIALIZED *geom_from_hexewkb(const char *wkt);
extern GSERIALIZED *geom_from_text(const char *wkt, int32_t srid);
extern GSERIALIZED *geom_in(const char *str, int32 typmod);
extern GSERIALIZED *geom_intersection2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern bool geom_intersects2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern bool geom_intersects3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern double geom_perimeter(const GSERIALIZED *gs);
extern bool geom_relate_pattern(const GSERIALIZED *gs1, const GSERIALIZED *gs2, char *patt);
extern GSERIALIZED *geom_shortestline2d(const GSERIALIZED *gs1, const GSERIALIZED *s2);
extern GSERIALIZED *geom_shortestline3d(const GSERIALIZED *gs1, const GSERIALIZED *s2);
extern bool geom_touches(const GSERIALIZED *gs1, const GSERIALIZED *gs2);

extern GSERIALIZED *line_interpolate_point(GSERIALIZED *gs, double distance_fraction, char repeat);
extern double line_length(const GSERIALIZED *gs);
extern int line_numpoints(const GSERIALIZED *gs);
extern double line_locate_point(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern GSERIALIZED *line_point_n(const GSERIALIZED *geom, int n);
extern GSERIALIZED *line_substring(const GSERIALIZED *gs, double from, double to);

/*****************************************************************************/

#endif
