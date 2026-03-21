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
 * @brief Functions for per-thread cleanup for the MEOS library
 * @details The design is as follows:
 * - One Thread-local storage (TLS) structure per thread
 * - Various modules register their thread cleanup once for the timezone cache,
 *   the Proj cache, etc.
 * - Function meos_finalize() calls them explicitly (for single-thread use)
 * - The TLS destructor calls them automatically on thread exit
 */

#include <pthread.h>
#include <stdlib.h>

/* MEOS */
#include <meos.h>
#include <meos_internal.h>

static pthread_key_t meos_tls_key;
static pthread_once_t meos_tls_once = PTHREAD_ONCE_INIT;

/* Destructor executed when thread exits */
static void
meos_tls_destroy(void *ptr)
{
  MEOSThreadState *state = ptr;
  if (!state) return;

  for (int i = 0; i < state->count; i++)
    state->cleanup[i]();

  free(state);
}

static void
meos_tls_init_key(void)
{
  pthread_key_create(&meos_tls_key, meos_tls_destroy);
}

static MEOSThreadState *
meos_tls_get(void)
{
  pthread_once(&meos_tls_once, meos_tls_init_key);

  MEOSThreadState *state = pthread_getspecific(meos_tls_key);
  if (!state)
  {
    state = calloc(1, sizeof(MEOSThreadState));
    pthread_setspecific(meos_tls_key, state);
  }
  return state;
}

void
meos_register_thread_cleanup(meos_thread_cleanup_fn fn)
{
  MEOSThreadState *state = meos_tls_get();
  if (state->count < MEOS_MAX_THREAD_CLEANUPS)
    state->cleanup[state->count++] = fn;
}

/* Explicit finalize for current thread */
void
meos_finalize_thread(void)
{
  MEOSThreadState *state = pthread_getspecific(meos_tls_key);
  if (!state) return;

  for (int i = 0; i < state->count; i++)
    state->cleanup[i]();

  state->count = 0;
}

/*****************************************************************************/
