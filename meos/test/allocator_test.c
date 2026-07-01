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
 * @brief A program that tests the pluggable allocator hook of the MEOS API.
 *
 * It installs a counting allocator via #meos_initialize_allocator, builds and
 * frees a temporal value, and verifies that (a) the custom allocator was
 * actually invoked (interposition) and (b) the tracked live bytes return to
 * zero after freeing (no leak, so free routes through the hook too). Finally it
 * reinstalls the default (libc) allocator.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o allocator_test allocator_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>

/* The palloc/pfree entry points route MEOS working-memory allocations through
 * the hook. They are exported by libmeos but are not part of the typed API, so
 * declare them here. */
extern void *palloc(size_t size);
extern void pfree(void *ptr);

/*
 * Counting allocator. It is pointer-preserving (as a real embedder such as
 * DuckDB is), tracking the live byte count through malloc_usable_size so that
 * a value allocated through it can be released either with the hook (pfree) or
 * with libc free.
 */
static size_t live_bytes = 0;
static long n_malloc = 0;
static long n_realloc = 0;
static long n_free = 0;

static void *
counting_malloc(size_t size)
{
  void *p = malloc(size);
  if (p)
  {
    live_bytes += malloc_usable_size(p);
    n_malloc++;
  }
  return p;
}

static void *
counting_realloc(void *ptr, size_t size)
{
  size_t old = ptr ? malloc_usable_size(ptr) : 0;
  void *p = realloc(ptr, size);
  if (p)
  {
    live_bytes += malloc_usable_size(p);
    live_bytes -= old;
    n_realloc++;
  }
  return p;
}

static void
counting_free(void *ptr)
{
  if (ptr)
  {
    live_bytes -= malloc_usable_size(ptr);
    n_free++;
    free(ptr);
  }
}

/* Main program */
int main(void)
{
  const char *tint_str = "[1@2000-01-01, 2@2000-01-02, 3@2000-01-03]";

  /* Initialize MEOS (which installs the default libc allocator) */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Warm up the lazily-initialized global caches (type cache, timezone and
   * collation) under the default allocator, so that they do not leave residual
   * live bytes once the counting allocator is installed. */
  Temporal *warm = tint_in(tint_str);
  char *warm_out = tint_out(warm);
  pfree(warm_out);
  pfree(warm);

  printf("****************************************************************\n");
  printf("* Allocator hook *\n");
  printf("****************************************************************\n");

  /* Install the counting allocator */
  meos_initialize_allocator(counting_malloc, counting_realloc, counting_free);

  /* Nothing has been allocated through the hook yet */
  assert(live_bytes == 0);

  /* Build a temporal value: its working memory now routes through the hook */
  Temporal *temp = tint_in(tint_str);
  assert(n_malloc > 0);         /* interposition: the custom malloc ran */
  assert(live_bytes > 0);       /* the value's memory is tracked by the hook */
  size_t peak = live_bytes;

  /* Exercise more of the allocation path */
  char *temp_out = tint_out(temp);
  assert(temp_out != NULL);

  printf("custom malloc calls: %ld\n", n_malloc);
  printf("custom realloc calls: %ld\n", n_realloc);
  printf("live bytes after building a tint value: %zu\n", peak);

  /* Free everything through the hook (pfree routes to the custom free) */
  pfree(temp_out);
  pfree(temp);

  printf("custom free calls: %ld\n", n_free);
  printf("live bytes after freeing: %zu\n", live_bytes);

  /* Interposition of free + zero-leak: every hook allocation was released
   * through the hook, so the tracked live bytes return to zero */
  assert(n_free > 0);
  assert(live_bytes == 0);

  printf("Allocator hook interposition and zero-leak: OK\n");
  printf("****************************************************************\n");

  /* Reinstall the default allocator and finalize MEOS */
  meos_initialize_allocator(NULL, NULL, NULL);
  meos_finalize();

  return 0;
}
