/*****************************************************************************
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
 * @brief A program that tests the idempotency of the MEOS setup lifecycle.
 *
 * Every finalize function in the #meos_finalize chain nulls the slot it
 * released, so the chain is safe to run twice and a cycle can be started
 * again on the same thread. A host that drives the lifecycle per unit of work
 * (a stream query, a worker task) relies on both properties.
 *
 * Each cycle populates the caches the chain releases before releasing them,
 * so the assertions are not vacuous: the collation cache is filled by
 * #meos_initialize_collation, the timezone cache by #meos_initialize_timezone,
 * and the PROJ SRS cache by a #geo_transform between two projections.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o lifecycle_test lifecycle_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

/* A point in WGS 84, transformed to Web Mercator so that the cycle stores a
 * projection in the PROJ SRS cache the finalize chain then releases. */
#define POINT_WKT "SRID=4326;POINT(4.35 50.85)"
#define SRID_TO 3857

/**
 * @brief Fill the caches that the finalize chain releases
 */
static void
populate_caches(void)
{
  GSERIALIZED *geom = geom_in(POINT_WKT, -1);
  assert(geom);
  GSERIALIZED *proj = geo_transform(geom, SRID_TO);
  assert(proj);
  assert(geo_srid(proj) == SRID_TO);
  free(proj);
  free(geom);
}

int
main(void)
{
  /* A cycle that releases populated caches, then a second finalize: the
   * chain runs on slots it has already nulled. */
  meos_initialize();
  meos_initialize_timezone("UTC");
  populate_caches();
  meos_finalize();
  meos_finalize();

  /* A second cycle on the same thread: initialize finds every slot free. */
  meos_initialize();
  meos_initialize_timezone("UTC");
  populate_caches();
  meos_finalize();

  /* The per-cache entry points carry the same contract on their own. */
  meos_initialize();
  meos_finalize_collation();
  meos_finalize_collation();
  meos_finalize_timezone();
  meos_finalize_timezone();
  meos_finalize_projsrs();
  meos_finalize_projsrs();
  meos_finalize();

  printf("lifecycle_test: OK\n");
  return EXIT_SUCCESS;
}
