/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief MEOS error handling inspired by GEOS and Proj
 * https://github.com/libgeos/geos/blob/main/capi/geos_c.h.in
 * https://github.com/OSGeo/PROJ/blob/master/src/4D_api.cpp
 */

/* C */
#include <errno.h>
#include <stdarg.h>
/* Postgres */
#include <postgres.h>
/* MEOS */
#include <meos.h>

/*****************************************************************************
 * Global variables
 *****************************************************************************/

/**
 * @brief Per-thread variable that keeps the last error number.
 * Mirrors the libc errno convention: each thread sees its own value, so
 * concurrent MEOS calls from multiple threads do not race on the error
 * status.
 */
static MEOS_TLS int MEOS_ERR_NO = 0;

/**
 * @brief Read an error number
 */
int
meos_errno(void)
{
  return MEOS_ERR_NO;
}

#if MEOS
/**
 * @brief Set an error number
 */
int
meos_errno_set(int err)
{
  /* Use #meos_errno_reset to explicitly clear the error status */
  if (err == 0)
    return 0;

  MEOS_ERR_NO = err;
  return err;
}

/**
 * @brief Set an error number
 * @details Use #meos_errno_restore when the current function succeeds, but the
 * error flag was set on entry, and stored/reset using #meos_errno_reset in
 * order to monitor for new errors.
 * @see See usage example under #meos_errno_reset()
 */
int
meos_errno_restore(int err)
{
  if (err == 0)
    return 0;
  meos_errno_set(err);
  return 0;
}

/**
 * @brief Clears an error number
 * @return Returns the previous value of the errno, for convenient reset/restore
 * operations
 *
 * @code
 * int foo(void)
 * {
 *   // errno may be set on entry, but we need to reset it to be able to
 *   // check for errors from "do_something()"
 *   int last_errno = meos_errno_reset();
 *
 *   // local failure
 *   if (0==P)
 *     return meos_errno_set(42);
 *
 *   // call to function that may fail
 *   do_something();
 *
 *   // failure in do_something? - keep latest error status
 *   if (meos_errno())
 *     return meos_errno();
 *
 *   // success - restore previous error status, return 0
 *   return meos_errno_restore(last_errno);
 * }
 * @endcode
 */

int meos_errno_reset(void)
{
  int last_errno = meos_errno();
  meos_errno_set(0);
  errno = 0;
  return last_errno;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Process-global error handler function. Reads and writes use
 * the GCC/Clang __atomic_* builtins so that multiple threads calling
 * meos_initialize() (which transitively calls
 * meos_initialize_error_handler) do not race on the same word. Reads
 * in meos_error() use __ATOMIC_ACQUIRE; writes in
 * meos_initialize_error_handler() use __ATOMIC_RELEASE. The value
 * installed is identical across threads in normal usage
 * (default_error_handler), so this is a benign-race fix that lets
 * existing call patterns continue working unchanged.
 *
 * Builtins are used in preference to <stdatomic.h> _Atomic + atomic_*
 * functions because the latter triggers spurious
 * cppcheck-missingIncludeSystem alerts on Codacy's wrapper, which
 * doesn't see the project's compile_commands.json.
 */
typedef void (*meos_error_handler_t)(int, int, const char *);
static meos_error_handler_t MEOS_ERROR_HANDLER = NULL;

#if MEOS
/**
 * @brief Default error handler function that prints the error message to
 * stderr, sets the errcode, and exits if error level is equal to `ERROR`
 */
void
default_error_handler(int errlevel, int errcode, const char *errmsg)
{
  fprintf(stderr, "%s\n", errmsg);
  meos_errno_set(errcode);
  if (errlevel == ERROR)
    exit(EXIT_FAILURE);
  return;
}

/**
 * @brief Error handler function that sets the errcode and the error message
 */
void
error_handler_errno(int errlevel __attribute__((__unused__)), int errcode,
  const char *errmsg)
{
  perror(errmsg);
  meos_errno_set(errcode);
  return;
}

/*
 * Initialize error handler function
 */
void
meos_initialize_error_handler(error_handler_fn err_handler)
{
  meos_error_handler_t h = err_handler ? err_handler : &default_error_handler;
  __atomic_store_n(&MEOS_ERROR_HANDLER, h, __ATOMIC_RELEASE);
  return;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Function handling error messages
 *
 * @note Return-or-not contract is *undefined*: depending on the
 * installed handler this function MAY return to the caller. The
 * default handler `exit(EXIT_FAILURE)`s on `ERROR` (safe for one-shot
 * CLI use); any custom handler installed via
 * `meos_initialize_error_handler` may return. Code in MEOS that calls
 * `meos_error(ERROR, ...)` MUST be immediately followed by a `return`,
 * `goto`, `break`, or sentinel assignment -- NEVER let execution fall
 * through. Historic fall-through bugs caused by relying on the
 * exit-on-ERROR path: MobilityDB#1089 (closed by PR #1090), #1093
 * (wider audit). See the corresponding doc comment on the public
 * declaration of `meos_error` in `meos/include/meos.h`.
 */
void
meos_error(int errlevel, int errcode, const char *format, ...)
{
  char buffer[1024];
  va_list args;
  va_start(args, format);
  /* TODO: maybe check if the error message was truncated */
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  /* Execute the error handler function */
  meos_error_handler_t handler =
    __atomic_load_n(&MEOS_ERROR_HANDLER, __ATOMIC_ACQUIRE);
  if (handler)
    handler(errlevel, errcode, buffer);
  else
#if ! MEOS
    elog(errlevel, "%s", buffer);
#else
  {
    fprintf (stderr, "%s\n", buffer);
    if (errlevel == ERROR)
      exit(EXIT_FAILURE);
  }
#endif /* ! MEOS */
  return;
}

/*****************************************************************************/
