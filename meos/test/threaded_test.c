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
 * @brief Concurrent stress test for MEOS thread-local state (issue #404).
 *
 * Each worker thread independently runs the full lifecycle:
 *   meos_initialize() →
 *   parse a unique WKT temporal point in a hot loop (exercises the
 *     per-thread WKT lexer/parser globals + GMT bootstrap) →
 *   read-only queries on the parsed value →
 *   thread-unique meos_errno round-trip →
 *   meos_finalize()
 *
 * Verifies:
 *   - meos_initialize/finalize can be called concurrently without
 *     racing on shared library state;
 *   - WKT parsing (geometry_in / tgeompoint_in / ...) works concurrently
 *     because the lwgeom flex/bison globals are now MEOS_TLS;
 *   - meos_errno reads/writes remain isolated per thread;
 *   - the GMT timezone bootstrap (postgres/timezone/localtime.c) is
 *     no longer racy under concurrent first-use.
 *
 * Build (Linux, after `cmake --install` to a prefix):
 * @code
 * gcc -Wall -g -O2 -I<prefix>/include -pthread \
 *     -o threaded_test threaded_test.c -L<prefix>/lib -lmeos
 * ./threaded_test 8 5000   # 8 threads, 5000 iterations each
 * @endcode
 *
 * Run under TSan for race detection: rebuild MEOS and the test with
 * `-fsanitize=thread`.
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
  char wkt[256];

  /* Each thread allocates its own MEOS state. */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Each thread uses a coordinate set that's unique to its id; if WKT
   * parsing were still process-shared, parse states would collide and
   * we'd see corrupted geometries or random parse failures. */
  double base_x = (double) w->id;
  double base_y = (double) w->id * 0.5;

  for (int i = 0; i < w->iters; i++)
  {
    snprintf(wkt, sizeof(wkt),
      "[POINT(%.3f %.3f)@2024-01-01, POINT(%.3f %.3f)@2024-01-02]",
      base_x, base_y, base_x + 1.0, base_y + 1.0);

    Temporal *t = (Temporal *) tgeompoint_in(wkt);
    if (! t)
    {
      atomic_fetch_add(&w->errors, 1);
      continue;
    }

    /* Read-only queries that hit per-thread caches. */
    if (tspatial_srid(t) < 0)
      atomic_fetch_add(&w->errors, 1);
    if (temporal_num_instants(t) <= 0)
      atomic_fetch_add(&w->errors, 1);

    free(t);

    /* Verify per-thread errno isolation in the same hot loop. */
    int marker = w->id * 1000003 + i;
    meos_errno_set(marker);
    if (meos_errno() != marker)
      atomic_fetch_add(&w->errors, 1);
    meos_errno_reset();
  }

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

  printf("threaded_test: %d threads x %d iters, %d errors\n",
    nthreads, iters, total_errors);
  return total_errors == 0 ? 0 : 1;
}
