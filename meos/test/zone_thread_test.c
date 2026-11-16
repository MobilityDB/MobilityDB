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
 * @brief Test that a thread naming no time zone reads and writes timestamps
 * in the zone the program names last
 * @details The main thread names Asia/Kathmandu (+05:45), a zone no test
 * machine keeps, so that an answer in the zone of the machine shows. A worker
 * thread, which names no zone and calls no initialization, as a goroutine or a
 * thread of a pool runs, writes the input with the offset +05:45. The main
 * thread then names America/New_York while the worker waits, and the worker
 * writes it with -05, the last naming winning, as every thread reads the ways
 * file #meos_set_ways_csv names last (ways_thread_test.c). The worker then
 * names Europe/Moscow and writes +03; the main thread, which names its own
 * zone, still writes -05, and a second worker naming none writes +03, the
 * zone named last. Each step of a thread waits for the step of the other
 * before it.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -pthread -o zone_thread_test zone_thread_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <pg_timestamp.h>

#define INPUT "2001-12-06 00:00:00"

/* The step the threads have reached, each waiting for its own */
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

/* Read the input in the zone of the thread and compare its output with the
 * expected text; the failures are counted under the lock, as every thread
 * counts them */
static void
check(const char *who, const char *what, const char *expected)
{
  size_t size_out;
  TimestampTz t = timestamptz_in(INPUT, -1);
  char *out = timestamptz_out(t, &size_out);
  bool ok = out && strcmp(out, expected) == 0;
  pthread_mutex_lock(&lock);
  if (! ok)
    failures++;
  printf("%s %s, %s: %s, expected %s\n", ok ? "ok  " : "FAIL", who, what,
    out ? out : "(null)", expected);
  pthread_mutex_unlock(&lock);
  free(out);
}

static void *
worker(void *arg)
{
  (void) arg;
  wait_step(1);
  check("worker", "naming none, in the zone the program names",
    "2001-12-06 00:00:00+05:45");
  next_step();
  wait_step(3);
  check("worker", "naming none, once the program names another",
    "2001-12-06 00:00:00-05");
  meos_initialize_timezone("Europe/Moscow");
  check("worker", "in the zone it names", "2001-12-06 00:00:00+03");
  meos_finalize_timezone();
  next_step();
  return NULL;
}

static void *
late_worker(void *arg)
{
  (void) arg;
  check("late worker", "naming none, in the zone named last",
    "2001-12-06 00:00:00+03");
  meos_finalize_timezone();
  return NULL;
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("Asia/Kathmandu");
  pthread_t tid;
  if (pthread_create(&tid, NULL, worker, NULL) != 0)
    return 1;
  next_step();                        /* 1: the worker reads in Kathmandu */
  wait_step(2);
  meos_initialize_timezone("America/New_York");
  next_step();                        /* 3: the worker reads in New York */
  wait_step(4);
  pthread_join(tid, NULL);
  check("main", "in the zone it names, after the worker names another",
    "2001-12-06 00:00:00-05");
  pthread_t late;
  if (pthread_create(&late, NULL, late_worker, NULL) != 0)
    return 1;
  pthread_join(late, NULL);
  meos_finalize();
  printf("%d failure(s)\n", failures);
  return failures ? 1 : 0;
}
