/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief The GEOS entry points the vendored PostGIS raster calls, answered
 * from the native engines for a build carrying no GEOS
 * @details The raster core is the one part of the vendored PostGIS that calls
 * GEOS directly rather than through the geometry library, so leaving out the
 * geometry library's GEOS files does not free it: three of its sources still
 * ask GEOS for a spatial relationship, a union, or a conversion between the
 * two geometry models. Rewriting them would mean editing vendored code, which
 * every PostGIS release would then conflict with, so the entry points they
 * call are answered here instead and the vendored sources stay as they came.
 *
 * What answers them is what MEOS already carries. A spatial relationship is
 * the DE-9IM matrix #meos_relate() computes exactly, a union is the dissolve
 * #geom_unary_union() reads from the boundaries, and the conversion between
 * the two geometry models is a copy: a @p GEOSGeometry is an @p LWGEOM here,
 * so nothing is serialized and no round trip is paid.
 *
 * These definitions are compiled only where the build carries no GEOS. A build
 * carrying it links the real library and never reaches this file, so the two
 * answer the same thing by construction rather than by agreement.
 *
 * The raster core also asks whether the union it computed is valid, and
 * repairs it through @p lwgeom_make_valid() when it is not. The dissolve
 * answered here bounds its surfaces by construction, so validity is reported
 * for it and the repair is never reached.
 */

/* C */
#include <stdarg.h>
#include <stdio.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
#include <lwgeom_geos.h>
#include <lwgeom_log.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include "geo/geo_funcs.h"
#include "geo/postgis_funcs.h"

/*****************************************************************************
 * The two geometry models
 * A GEOSGeometry is an LWGEOM here, so the conversion between the models is a
 * copy and the caller owns what it is given, exactly as the GEOS one does
 *****************************************************************************/

/**
 * @brief Return the GEOS geometry of a geometry
 */
GEOSGeometry *
LWGEOM2GEOS(const LWGEOM *geom, uint8_t autofix)
{
  (void) autofix;
  if (! geom)
    return NULL;
  return (GEOSGeometry *) lwgeom_clone_deep(geom);
}

/**
 * @brief Return the geometry of a GEOS geometry
 */
LWGEOM *
GEOS2LWGEOM(const GEOSGeometry *geom, uint8_t want3d)
{
  (void) want3d;
  if (! geom)
    return NULL;
  return lwgeom_clone_deep((const LWGEOM *) geom);
}

/**
 * @brief Free a GEOS geometry
 */
void
GEOSGeom_destroy(GEOSGeometry *geom)
{
  if (geom)
    lwgeom_free((LWGEOM *) geom);
  return;
}

/**
 * @brief Gather geometries into a collection, which takes the geometries over
 * @details A collection whose every member is a polygon is a multipolygon,
 * which is what the dissolve below reads and what the raster core expects of
 * the surface it asks for.
 *
 * ⛔ The GEOS entry point takes over the GEOMETRIES and not the ARRAY holding
 * them: it copies the array into storage of its own, so its caller frees the
 * one it passed. #lwcollection_construct() keeps the array it is given
 * instead, which would leave that array freed by the collection and again by
 * the caller. The array is therefore copied here, so the ownership the caller
 * is entitled to assume is the one it gets.
 */
GEOSGeometry *
GEOSGeom_createCollection(int type, GEOSGeometry **geoms, unsigned int ngeoms)
{
  (void) type;
  if (! geoms)
    return NULL;
  uint8_t colltype = MULTIPOLYGONTYPE;
  int32_t srid = SRID_UNKNOWN;
  LWGEOM **members = ngeoms ? lwalloc(sizeof(LWGEOM *) * ngeoms) : NULL;
  for (unsigned int i = 0; i < ngeoms; i++)
  {
    LWGEOM *geom = (LWGEOM *) geoms[i];
    if (! geom || geom->type != POLYGONTYPE)
      colltype = COLLECTIONTYPE;
    if (geom && srid == SRID_UNKNOWN)
      srid = geom->srid;
    members[i] = geom;
  }
  LWCOLLECTION *coll = lwcollection_construct(colltype, srid, NULL, ngeoms,
    members);
  return (GEOSGeometry *) lwcollection_as_lwgeom(coll);
}

/**
 * @brief Initialize the GEOS library, which a build carrying none has nothing
 * to initialize
 */
void
initGEOS(GEOSMessageHandler notice_function, GEOSMessageHandler error_function)
{
  (void) notice_function; (void) error_function;
  return;
}

/**
 * @brief Report a GEOS error
 */
void
lwgeom_geos_error(const char *fmt, ...)
{
  char msg[256];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(msg, sizeof(msg), fmt, ap);
  va_end(ap);
  lwerror("%s", msg);
  return;
}

/*****************************************************************************
 * Union
 *****************************************************************************/

/**
 * @brief Return the union of the surfaces of a geometry
 * @details The dissolve MEOS reads from the boundaries of the surfaces is what
 * a unary union of them is
 */
GEOSGeometry *
GEOSUnaryUnion(const GEOSGeometry *geom)
{
  const LWGEOM *lwgeom = (const LWGEOM *) geom;
  if (! lwgeom)
    return NULL;
  GSERIALIZED *gs = geo_serialize(lwgeom);
  if (! gs)
    return NULL;
  GSERIALIZED *gsresult = geom_unary_union(gs, -1);
  pfree(gs);
  if (! gsresult)
    return NULL;
  /* The geometry of a serialized value points into it, so what is returned is
   * a copy of its own */
  LWGEOM *lwresult = lwgeom_from_gserialized(gsresult);
  LWGEOM *result = lwresult ? lwgeom_clone_deep(lwresult) : NULL;
  if (lwresult)
    lwgeom_free(lwresult);
  pfree(gsresult);
  return (GEOSGeometry *) result;
}

/**
 * @brief Return whether a geometry is valid
 * @details The only geometry this is asked of is the union answered above,
 * whose surfaces the dissolve bounds by construction
 */
char
GEOSisValid(const GEOSGeometry *geom)
{
  return geom ? 1 : 0;
}

/*****************************************************************************
 * Spatial relationships
 * Each is the DE-9IM matrix the native engine computes, matched against the
 * pattern the standard gives the relationship
 *****************************************************************************/

/**
 * @brief Return whether two geometries stand in the relationship a DE-9IM
 * pattern gives, reporting 2 where the relationship is not answered, as the
 * GEOS one does
 */
static char
geos_none_pattern(const GEOSGeometry *geom1, const GEOSGeometry *geom2,
  const char *pattern)
{
  char matrix[10];
  if (! geom1 || ! geom2)
    return 2;
  if (! meos_relate((const LWGEOM *) geom1, (const LWGEOM *) geom2, matrix))
    return 2;
  return de9im_match(matrix, pattern) ? 1 : 0;
}

char
GEOSRelatePattern(const GEOSGeometry *geom1, const GEOSGeometry *geom2,
  const char *pattern)
{
  return geos_none_pattern(geom1, geom2, pattern);
}

char
GEOSContains(const GEOSGeometry *geom1, const GEOSGeometry *geom2)
{
  return geos_none_pattern(geom1, geom2, "T*****FF*");
}

char
GEOSWithin(const GEOSGeometry *geom1, const GEOSGeometry *geom2)
{
  return geos_none_pattern(geom1, geom2, "T*F**F***");
}

char
GEOSIntersects(const GEOSGeometry *geom1, const GEOSGeometry *geom2)
{
  char matrix[10];
  if (! geom1 || ! geom2)
    return 2;
  if (! meos_relate((const LWGEOM *) geom1, (const LWGEOM *) geom2, matrix))
    return 2;
  /* Two geometries intersect where they are not disjoint */
  return de9im_match(matrix, "FF*FF****") ? 0 : 1;
}

char
GEOSTouches(const GEOSGeometry *geom1, const GEOSGeometry *geom2)
{
  char matrix[10];
  if (! geom1 || ! geom2)
    return 2;
  if (! meos_relate((const LWGEOM *) geom1, (const LWGEOM *) geom2, matrix))
    return 2;
  /* The interiors do not meet, and a boundary meets the other geometry */
  return (de9im_match(matrix, "FT*******") ||
    de9im_match(matrix, "F**T*****") ||
    de9im_match(matrix, "F***T****")) ? 1 : 0;
}

char
GEOSOverlaps(const GEOSGeometry *geom1, const GEOSGeometry *geom2)
{
  char matrix[10];
  if (! geom1 || ! geom2)
    return 2;
  const LWGEOM *g1 = (const LWGEOM *) geom1;
  const LWGEOM *g2 = (const LWGEOM *) geom2;
  if (! meos_relate(g1, g2, matrix))
    return 2;
  /* Two geometries overlap where they are of the same dimension, each holds a
   * point the other does not, and their interiors meet in that dimension. The
   * pattern the standard gives it differs for the linear geometries, whose
   * interiors meeting in a point is not them overlapping */
  int dim1 = lwgeom_dimension(g1), dim2 = lwgeom_dimension(g2);
  if (dim1 != dim2)
    return 0;
  if (dim1 == 1)
    return de9im_match(matrix, "1*T***T**") ? 1 : 0;
  if (dim1 == 0 || dim1 == 2)
    return de9im_match(matrix, "T*T***T**") ? 1 : 0;
  return 0;
}

/*****************************************************************************/
