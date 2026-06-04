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
 * @brief Concurrent stress test combining the GEOS spatial-relationship
 * surface, WKT parsing of temporal points, WKB roundtrip, temporal
 * accessors, and per-thread geometry distance calls.
 *
 * Each worker thread independently runs the full lifecycle:
 *   meos_initialize() →
 *   parse a unique polygon + point pair and a unique tgeompoint trajectory →
 *   hot loop:
 *     geom_intersects2d / geom_contains / geom_covers / geom_distance2d
 *       on the polygon/point pair (per-thread GEOS context handle),
 *     tgeompoint_in WKT parse (lwgeom flex/bison TLS state),
 *     temporal_as_wkb + temporal_from_wkb roundtrip (binary IO path),
 *     temporal_num_instants / temporal_start_instant accessors,
 *     tpoint_trajectory extracts the polyline (exercises the temporal →
 *       GEOS conversion path),
 *     meos_errno round-trip (thread-local errno) →
 *   meos_finalize()
 *
 * Verifies that the per-thread GEOS context handle, lwgeom WKT/parser
 * state, GMT bootstrap, GSL state, and meos_errno are all isolated:
 * any sharing between worker threads would produce inconsistent results
 * or a SIGSEGV.
 *
 * Build (Linux, after `cmake --install` to a prefix):
 * @code
 * gcc -Wall -g -O2 -I<prefix>/include -pthread \
 *     -o threaded_geos_test threaded_geos_test.c -L<prefix>/lib -lmeos
 * ./threaded_geos_test 16 20000   # 16 threads, 20000 iterations each
 * @endcode
 */

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

typedef struct
{
  int id;
  int iters;
  atomic_int errors;
} worker_arg;

static void *
worker(void *arg)
{
  worker_arg *w = (worker_arg *) arg;

  meos_initialize();
  meos_initialize_timezone("UTC");

  /* A unique polygon per thread; a point that is always inside it. */
  char poly_wkt[256];
  char point_wkt[256];
  double base = (double) w->id;
  snprintf(poly_wkt, sizeof(poly_wkt),
    "POLYGON((%.1f %.1f,%.1f %.1f,%.1f %.1f,%.1f %.1f,%.1f %.1f))",
    base, base, base + 10.0, base, base + 10.0, base + 10.0,
    base, base + 10.0, base, base);
  snprintf(point_wkt, sizeof(point_wkt),
    "POINT(%.1f %.1f)", base + 5.0, base + 5.0);

  GSERIALIZED *poly = geom_in(poly_wkt, -1);
  GSERIALIZED *pt = geom_in(point_wkt, -1);
  if (! poly || ! pt)
  {
    atomic_fetch_add(&w->errors, 1);
    meos_finalize();
    return NULL;
  }

  /* Per-thread tgeompoint WKT (unique coordinates by thread id). */
  char tgeo_wkt[256];

  for (int i = 0; i < w->iters; i++)
  {
    /* GEOS spatial-rel surface: every call dispatches through the
     * per-thread GEOSContextHandle_t. */
    if (! geom_intersects2d(poly, pt))
      atomic_fetch_add(&w->errors, 1);
    if (! geom_intersects2d(pt, poly))
      atomic_fetch_add(&w->errors, 1);
    if (! geom_contains(poly, pt))
      atomic_fetch_add(&w->errors, 1);
    if (! geom_covers(poly, pt))
      atomic_fetch_add(&w->errors, 1);
    if (geom_distance2d(poly, pt) != 0.0)
      atomic_fetch_add(&w->errors, 1);

    /* WKT parse + WKB roundtrip on a unique tgeompoint trajectory.
     * The coordinates depend on i so each iteration parses a distinct
     * string — the lwgeom flex/bison parser state must be per-thread for
     * this to work concurrently. */
    snprintf(tgeo_wkt, sizeof(tgeo_wkt),
      "[POINT(%.3f %.3f)@2024-01-01, POINT(%.3f %.3f)@2024-01-02]",
      base + 0.001 * i, base + 0.001 * i,
      base + 0.001 * i + 1.0, base + 0.001 * i + 1.0);
    Temporal *t = tgeompoint_in(tgeo_wkt);
    if (! t)
    {
      atomic_fetch_add(&w->errors, 1);
      continue;
    }

    /* Binary IO roundtrip exercises the WKB encoder/decoder paths and
     * the GMT timezone bootstrap on first call per thread. */
    size_t wkb_size = 0;
    uint8_t *wkb = temporal_as_wkb(t, 0, &wkb_size);
    if (! wkb || wkb_size == 0)
      atomic_fetch_add(&w->errors, 1);
    else
    {
      Temporal *t2 = temporal_from_wkb(wkb, wkb_size);
      if (! t2 || temporal_num_instants(t2) != 2)
        atomic_fetch_add(&w->errors, 1);
      free(t2);
      free(wkb);
    }

    /* Temporal → GEOS conversion path: tpoint_trajectory builds a
     * GEOSGeometry from the temporal value, then converts back to
     * GSERIALIZED. */
    GSERIALIZED *traj = tpoint_trajectory(t, false);
    if (! traj)
      atomic_fetch_add(&w->errors, 1);
    else
    {
      /* The trajectory must intersect itself. */
      if (! geom_intersects2d(traj, traj))
        atomic_fetch_add(&w->errors, 1);
      free(traj);
    }

    free(t);

    /* Per-thread errno marker. */
    int marker = w->id * 1000003 + i;
    meos_errno_set(marker);
    if (meos_errno() != marker)
      atomic_fetch_add(&w->errors, 1);
    meos_errno_reset();
  }

  free(poly);
  free(pt);
  meos_finalize();
  return NULL;
}

int
main(int argc, char **argv)
{
  int nthreads = (argc > 1) ? atoi(argv[1]) : 4;
  int iters = (argc > 2) ? atoi(argv[2]) : 1000;

  if (nthreads <= 0 || iters <= 0)
  {
    fprintf(stderr, "Usage: %s <nthreads> <iters>\n", argv[0]);
    return 2;
  }

  pthread_t *tids = calloc((size_t) nthreads, sizeof(pthread_t));
  worker_arg *args = calloc((size_t) nthreads, sizeof(worker_arg));
  if (! tids || ! args)
  {
    fprintf(stderr, "alloc failed\n");
    return 2;
  }

  for (int i = 0; i < nthreads; i++)
  {
    args[i].id = i;
    args[i].iters = iters;
    atomic_init(&args[i].errors, 0);
    if (pthread_create(&tids[i], NULL, worker, &args[i]) != 0)
    {
      fprintf(stderr, "pthread_create failed for thread %d\n", i);
      return 2;
    }
  }

  int total_errors = 0;
  for (int i = 0; i < nthreads; i++)
  {
    pthread_join(tids[i], NULL);
    total_errors += atomic_load(&args[i].errors);
  }

  free(tids);
  free(args);

  printf("threaded_geos_test: %d threads x %d iters, %d errors\n",
    nthreads, iters, total_errors);
  return total_errors == 0 ? 0 : 1;
}
