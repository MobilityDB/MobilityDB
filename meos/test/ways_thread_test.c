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
 * @brief Test that every thread reads its routes from the ways CSV file
 * named last
 * @details A program that writes two network files in the current directory
 * and removes them at the end, as ways_index_test.c writes its files: the
 * first holds a route 1 of length 5 and a route 4 of length 10, the second a
 * route 1 of length 2 and a route 10 of length 1. The main thread names the
 * first file; a worker thread, which keeps the routes it reads in a cache of
 * its own, reads route 1 and route 4 of it. The main thread then names the
 * second file while the worker waits, and the worker reads route 1 of the
 * second file, no route 4 and route 10; the main thread reads the same, then
 * names the first file again, and the worker reads route 1 of the first file.
 * Each step of a thread waits for the step of the other before it, so the
 * threads never read and name a file at once.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -pthread -o ways_thread_test ways_thread_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_npoint.h>

/* The two network files the program writes */
static const char *WAYS_A = "ways_thread_test_a.csv";
static const char *WAYS_B = "ways_thread_test_b.csv";

/* The routes, as hex EWKB of LINESTRINGs with SRID 5676 */
static const char *ROUTES_A =
  "1,01020000202C160000020000000000000000000000000000000000000000000000"
  "000008400000000000001040\n"
  "4,01020000202C160000020000000000000000000000000000000000000000000000"
  "000018400000000000002040\n";
static const char *ROUTES_B =
  "1,01020000202C160000020000000000000000000000000000000000000000000000"
  "000000000000000000000040\n"
  "10,01020000202C16000002000000000000000000000000000000000000000000000"
  "00000F03F0000000000000000\n";

/* The step the two threads have reached, each waiting for its own */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t moved = PTHREAD_COND_INITIALIZER;
static int step = 0;

static int failures = 0;

static void
wait_step(int want)
{
  pthread_mutex_lock(&lock);
  while (step != want)
    pthread_cond_wait(&moved, &lock);
  pthread_mutex_unlock(&lock);
}

static void
next_step(void)
{
  pthread_mutex_lock(&lock);
  step++;
  pthread_cond_broadcast(&moved);
  pthread_mutex_unlock(&lock);
}

/* The failures are counted under the lock, as both threads count them */
static void
check(const char *who, const char *what, int64 rid, bool want_exists,
  double want_length)
{
  bool exists = route_exists(rid);
  double length = exists ? route_length(rid) : 0.0;
  bool ok = exists == want_exists && (! want_exists || length == want_length);
  pthread_mutex_lock(&lock);
  if (! ok)
    failures++;
  printf("%s %s, %s: exists %d, length %g\n", ok ? "ok  " : "FAIL", who,
    what, exists, length);
  pthread_mutex_unlock(&lock);
}

static void *
worker(void *arg)
{
  (void) arg;
  meos_initialize();
  meos_initialize_noexit_error_handler();
  wait_step(1);
  check("worker", "route 1 of length 5 of the first file", 1, true, 5.0);
  check("worker", "route 4 of length 10 of the first file", 4, true, 10.0);
  next_step();
  wait_step(3);
  check("worker", "route 1 of length 2 once the second file is named", 1,
    true, 2.0);
  check("worker", "route 4 the second file lacks", 4, false, 0.0);
  check("worker", "route 10 of length 1 of the second file", 10, true, 1.0);
  next_step();
  wait_step(5);
  check("worker", "route 1 of length 5 once the first file is named again",
    1, true, 5.0);
  meos_finalize();
  return NULL;
}

static bool
write_file(const char *path, const char *text)
{
  FILE *file = fopen(path, "wb");
  if (! file)
    return false;
  fputs(text, file);
  fclose(file);
  return true;
}

int
main(void)
{
  if (! write_file(WAYS_A, ROUTES_A) || ! write_file(WAYS_B, ROUTES_B))
    return 1;

  meos_initialize();
  meos_initialize_noexit_error_handler();
  meos_set_ways_csv(WAYS_A);
  pthread_t tid;
  if (pthread_create(&tid, NULL, worker, NULL) != 0)
    return 1;
  next_step();                                  /* 1: the worker reads A */
  wait_step(2);
  meos_set_ways_csv(WAYS_B);
  next_step();                                  /* 3: the worker reads B */
  wait_step(4);
  check("main", "route 1 of length 2 of the second file", 1, true, 2.0);
  check("main", "route 4 the second file lacks", 4, false, 0.0);
  meos_set_ways_csv(WAYS_A);
  next_step();                                  /* 5: the worker reads A */
  pthread_join(tid, NULL);
  check("main", "route 1 of length 5 of the first file", 1, true, 5.0);
  meos_finalize();

  remove(WAYS_A);
  remove(WAYS_B);
  printf("%d failure(s)\n", failures);
  return failures ? 1 : 0;
}
