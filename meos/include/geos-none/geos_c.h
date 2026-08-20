/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @file
 * @brief The declarations of the GEOS C API a build carrying no GEOS still
 * reads
 * @details A geometry of GEOS names the types of the entry points that convert
 * between the two geometry models, and the vendored PostGIS reads them from
 * @p geos_c.h whether or not the library is there. A build carrying no GEOS
 * has no such file to read, so this one stands in its place: it is put on the
 * include path only where the GEOS option is off, and the vendored sources
 * stay exactly as they came.
 *
 * What stands in is the types and the constants alone. Every entry point of
 * the library itself is left out on purpose, so a call to one that the
 * conditional compilation failed to leave out is a compilation error naming
 * the function rather than a link that quietly resolves against a GEOS the
 * build was told not to carry.
 *
 * The geometry MEOS answers with in such a build is the @p LWGEOM one, the
 * two models being the same there; @p meos/src/geo/geo_geos_none.c defines
 * the entry points the vendored raster core calls in those terms.
 */

#ifndef __GEOS_C_H
#define __GEOS_C_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * The opaque types the signatures name
 *****************************************************************************/

typedef struct GEOSGeom_t GEOSGeometry;
typedef struct GEOSPrepGeom_t GEOSPreparedGeometry;
typedef struct GEOSCoordSeq_t GEOSCoordSequence;
typedef struct GEOSSTRtree_t GEOSSTRtree;
typedef struct GEOSBufParams_t GEOSBufferParams;
typedef struct GEOSContextHandle_HS *GEOSContextHandle_t;

/* The deprecated spellings the vendored sources still use */
typedef GEOSGeometry *GEOSGeom;
typedef GEOSCoordSequence *GEOSCoordSeq;

typedef void (*GEOSMessageHandler)(const char *fmt, ...);
typedef void (*GEOSMessageHandler_r)(const char *message, void *userdata);
typedef void (*GEOSQueryCallback)(void *item, void *userdata);
typedef void (*GEOSDistanceCallback)(const void *item1, const void *item2,
  double *distance, void *userdata);

/*****************************************************************************
 * The constants the signatures name
 *****************************************************************************/

enum GEOSGeomTypes
{
  GEOS_POINT,
  GEOS_LINESTRING,
  GEOS_LINEARRING,
  GEOS_POLYGON,
  GEOS_MULTIPOINT,
  GEOS_MULTILINESTRING,
  GEOS_MULTIPOLYGON,
  GEOS_GEOMETRYCOLLECTION
};

enum GEOSBufCapStyles
{
  GEOSBUF_CAP_ROUND = 1,
  GEOSBUF_CAP_FLAT = 2,
  GEOSBUF_CAP_SQUARE = 3
};

enum GEOSBufJoinStyles
{
  GEOSBUF_JOIN_ROUND = 1,
  GEOSBUF_JOIN_MITRE = 2,
  GEOSBUF_JOIN_BEVEL = 3
};

/*****************************************************************************
 * The entry points a build carrying no GEOS answers itself
 * @details These are the ones the vendored raster core calls, defined in
 * @p meos/src/geo/geo_geos_none.c from the native engines. They carry the
 * signatures of the GEOS library so the vendored sources call them exactly as
 * they call it. Every other entry point of the library is left out on
 * purpose: a call to one that the conditional compilation failed to leave out
 * is then a compilation error naming the function.
 *****************************************************************************/

extern void initGEOS(GEOSMessageHandler notice_function,
  GEOSMessageHandler error_function);

extern void GEOSGeom_destroy(GEOSGeometry *geom);
extern GEOSGeometry *GEOSGeom_createCollection(int type, GEOSGeometry **geoms,
  unsigned int ngeoms);
extern GEOSGeometry *GEOSUnaryUnion(const GEOSGeometry *geom);

extern char GEOSisValid(const GEOSGeometry *geom);
extern char GEOSRelatePattern(const GEOSGeometry *geom1,
  const GEOSGeometry *geom2, const char *pattern);
extern char GEOSContains(const GEOSGeometry *geom1, const GEOSGeometry *geom2);
extern char GEOSWithin(const GEOSGeometry *geom1, const GEOSGeometry *geom2);
extern char GEOSIntersects(const GEOSGeometry *geom1,
  const GEOSGeometry *geom2);
extern char GEOSTouches(const GEOSGeometry *geom1, const GEOSGeometry *geom2);
extern char GEOSOverlaps(const GEOSGeometry *geom1, const GEOSGeometry *geom2);

#ifdef __cplusplus
}
#endif

#endif /* __GEOS_C_H */
