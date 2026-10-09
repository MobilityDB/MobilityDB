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
 * @brief A program that tests the transformation of a geography and the byte
 * order of the Extended Well-Known Binary representation of a geometry
 * @details A geography is transformed only into a lon/lat coordinate system,
 * as the cast of a geometry into a geography refuses any other, by
 * #geo_transform, #geo_transform_pipeline and #spatialset_transform alike,
 * while a geometry is transformed into any. #geo_as_ewkb reads the byte order
 * as #geo_as_hexewkb reads it. The refusals are reached under
 * #meos_initialize_noexit_error_handler, the handler every language binding
 * installs.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o geo_transform_test geo_transform_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

/* The pipeline of the transformation from WGS 84 to Web Mercator */
#define PIPELINE "urn:ogc:def:coordinateOperation:EPSG::3856"

int
main(void)
{
  meos_initialize();
  meos_initialize_noexit_error_handler();
  meos_initialize_timezone("UTC");

  GSERIALIZED *geog = geog_in("SRID=4326;Point(4.35 50.85)", -1);
  GSERIALIZED *geom = geom_in("SRID=4326;Point(4.35 50.85)", -1);
  assert(geog && geom);

  /* A geography into a lon/lat system, and a geometry into any */
  GSERIALIZED *res = geo_transform(geog, 4269);
  assert(res && geo_srid(res) == 4269);
  free(res);
  res = geo_transform(geom, 3857);
  assert(res && geo_srid(res) == 3857);
  free(res);

  /* A geography into a projected system is refused */
  res = geo_transform(geog, 3857);
  assert(res == NULL && meos_errno() != 0);
  meos_errno_reset();
  res = geo_transform_pipeline(geog, PIPELINE, 3857, true);
  assert(res == NULL && meos_errno() != 0);
  meos_errno_reset();

  /* The same holds for a set */
  Set *gs = geogset_in("{\"SRID=4326;Point(4.35 50.85)\"}");
  Set *ms = geomset_in("{\"SRID=4326;Point(4.35 50.85)\"}");
  assert(gs && ms);
  Set *sres = spatialset_transform(gs, 3857);
  assert(sres == NULL && meos_errno() != 0);
  meos_errno_reset();
  sres = spatialset_transform(gs, 4269);
  assert(sres != NULL);
  free(sres);
  sres = spatialset_transform(ms, 3857);
  assert(sres != NULL);
  free(sres);

  /* The byte order of the EWKB: XDR writes 0 first, NDR 1, and an order no
   * decoder reads is refused with a size of zero */
  size_t size = 99;
  uint8_t *wkb = geo_as_ewkb(geom, "XDR", &size);
  assert(wkb && size > 0 && wkb[0] == 0);
  free(wkb);
  wkb = geo_as_ewkb(geom, "ndr", &size);
  assert(wkb && size > 0 && wkb[0] == 1);
  free(wkb);
  wkb = geo_as_ewkb(geom, "", &size);
  assert(wkb && size > 0);
  free(wkb);
  size = 99;
  wkb = geo_as_ewkb(geom, "abc", &size);
  assert(wkb == NULL && size == 0 && meos_errno() != 0);
  meos_errno_reset();
  wkb = geo_as_ewkb(geom, "NDR", NULL);
  assert(wkb == NULL && meos_errno() != 0);
  meos_errno_reset();

  free(gs); free(ms); free(geog); free(geom);
  meos_finalize();
  printf("geo_transform_test: OK\n");
  return EXIT_SUCCESS;
}
