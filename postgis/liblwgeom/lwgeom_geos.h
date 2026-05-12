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
 * Copyright 2011 Sandro Santilli <strk@kbt.io>
 * Copyright 2018 Darafei Praliaskouski <me@komzpa.net>
 *
 **********************************************************************/

#if POSTGIS_GEOS_VERSION < 31300
/* See https://github.com/libgeos/geos/pull/1097 */
typedef void (*GEOSMessageHandler)(const char *fmt, ...) __attribute__ (( format(printf, 1, 2) ));
#endif

#include "geos_c.h"

#include "liblwgeom.h"
#include "lwunionfind.h"

/*
** Public prototypes for GEOS utility functions.
*/
LWGEOM* GEOS2LWGEOM(const GEOSGeometry* geom, uint8_t want3d);
GEOSGeometry* LWGEOM2GEOS(const LWGEOM* g, uint8_t autofix);
GEOSGeometry* GBOX2GEOS(const GBOX* g);
GEOSGeometry* make_geos_point(double x, double y);
GEOSGeometry* make_geos_segment(double x1, double y1, double x2, double y2);

int cluster_intersecting(GEOSGeometry **geoms, uint32_t num_geoms, GEOSGeometry ***clusterGeoms, uint32_t *num_clusters);
int union_intersecting_pairs(GEOSGeometry** geoms, uint32_t num_geoms, UNIONFIND* uf);
int cluster_within_distance(LWGEOM **geoms, uint32_t num_geoms, double tolerance, LWGEOM ***clusterGeoms, uint32_t *num_clusters);
int union_dbscan(LWGEOM **geoms, uint32_t num_geoms, UNIONFIND *uf, double eps, uint32_t min_points, char **is_in_cluster_ret);

POINTARRAY* ptarray_from_GEOSCoordSeq(const GEOSCoordSequence* cs, uint8_t want3d);

extern char lwgeom_geos_errmsg[];
extern void lwgeom_geos_error(const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));

/* MEOS: per-thread GEOS context.  Every liblwgeom GEOS helper retrieves the
 * thread-local handle via lwgeom_geos_context() and uses the reentrant
 * GEOSXxx_r API. */
/* MEOS */ extern GEOSContextHandle_t lwgeom_geos_context(void);
/* MEOS */ extern void lwgeom_geos_finalize(void);


/*
 * Debug macros
 */
#if POSTGIS_DEBUG_LEVEL > 0

/* Display a notice and a WKT representation of a geometry
 * at the given debug level */
/* MEOS */ #define LWDEBUGGEOS(level, geom, msg) \
/* MEOS */   if (POSTGIS_DEBUG_LEVEL >= level) \
/* MEOS */   do { \
/* MEOS */ 		GEOSContextHandle_t _ctx = lwgeom_geos_context(); \
/* MEOS */ 		GEOSWKTWriter *wktwriter = GEOSWKTWriter_create_r(_ctx); \
/* MEOS */ 		char *wkt = GEOSWKTWriter_write_r(_ctx, wktwriter, (geom)); \
/* MEOS */ 		LWDEBUGF(1, msg " (GEOS): %s", wkt); \
/* MEOS */ 		GEOSFree_r(_ctx, wkt); \
/* MEOS */ 		GEOSWKTWriter_destroy_r(_ctx, wktwriter); \
/* MEOS */   } while (0);

#else /* POSTGIS_DEBUG_LEVEL <= 0 */

/* Empty prototype that can be optimised away by the compiler
 * for non-debug builds */
#define LWDEBUGGEOS(level, geom, msg) \
        ((void) 0)

#endif /*POSTGIS_DEBUG_LEVEL <= 0 */
