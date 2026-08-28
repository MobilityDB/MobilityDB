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
 * @brief The entry points of the geometry library's GEOS files that the rest of
 * that library still calls, for a build carrying no GEOS
 * @details The files of the vendored PostGIS geometry library that reach GEOS
 * are left out of a build carrying none, and four of the files that remain
 * still call into them: @p lwkmeans.c seeds its clusters from a centroid,
 * @p lwgeom.c reduces a geometry to a grid through an intersection,
 * @p lwlinearreferencing.c offsets a curve, and @p lwgeom_wrapx.c splits a
 * geometry on a blade and dissolves what it gets back. Each of those calls
 * would leave a symbol undefined, which a shared library accepts at link time
 * and fails on the first call to, so they are answered here by the error
 * saying the operation needs GEOS.
 *
 * That error travels the channel liblwgeom's own errors travel: MEOS installs
 * @p meos_lwerror_handler(), so it reaches the caller as a MEOS error rather
 * than ending the process.
 *
 * ONE of them is answered rather than refused. @p lwgeom_centroid() IS reached
 * from MEOS: #geo_cluster_kmeans() calls @p lwgeom_cluster_kmeans(), which
 * takes the centroid of every input that is not a point, so a build carrying
 * no GEOS could not cluster a polygon. #meos_centroid() answers it natively,
 * so the clustering works wherever the library does. The rest are reached only
 * from entry points MEOS never calls, and the definitions exist so that the
 * set of entry points the left-out files carry is answered as a whole.
 */

/* PostGIS */
#include <liblwgeom.h>
#include <lwgeom_geos.h>
#include <lwgeom_log.h>
/* MEOS */
#include "geo/geo_funcs.h"

/**
 * @brief The message the geometry library's GEOS files leave for their callers
 * @details A caller reads it after an answer of NULL, so a build carrying no
 * GEOS has to define it even though nothing here ever writes it. Its size is
 * the one @p lwgeom_geos.c gives it, which that file states privately rather
 * than in a header
 */
#define LWGEOM_GEOS_ERRMSG_MAXSIZE 256
/* MEOS */ __thread char lwgeom_geos_errmsg[LWGEOM_GEOS_ERRMSG_MAXSIZE];

/**
 * @brief Report that an operation needs the GEOS library
 */
static LWGEOM *
lwgeom_needs_geos(const char *name)
{
  lwerror("%s: the operation is answered by the GEOS library, which this build "
    "excludes: configure with -DGEOS=ON", name);
  return NULL;
}

/**
 * @brief Return the centroid of a geometry
 * @details Answered natively, because @p lwkmeans.c takes the centroid of
 * every clustered geometry that is not a point and MEOS reaches it there
 */
LWGEOM *
lwgeom_centroid(const LWGEOM *geom)
{
  return meos_centroid(geom);
}

LWGEOM *
lwgeom_make_valid(LWGEOM *geom)
{
  (void) geom;
  return lwgeom_needs_geos(__func__);
}

LWGEOM *
lwgeom_intersection_prec(const LWGEOM *geom1, const LWGEOM *geom2,
  double gridSize)
{
  (void) geom1; (void) geom2; (void) gridSize;
  return lwgeom_needs_geos(__func__);
}

LWGEOM *
lwgeom_unaryunion(const LWGEOM *geom)
{
  (void) geom;
  return lwgeom_needs_geos(__func__);
}

LWGEOM *
lwgeom_split(const LWGEOM *geom, const LWGEOM *blade)
{
  (void) geom; (void) blade;
  return lwgeom_needs_geos(__func__);
}

LWGEOM *
lwgeom_offsetcurve(const LWGEOM *geom, double size, int quadsegs, int joinStyle,
  double mitreLimit)
{
  (void) geom; (void) size; (void) quadsegs; (void) joinStyle;
  (void) mitreLimit;
  return lwgeom_needs_geos(__func__);
}

/*****************************************************************************/
