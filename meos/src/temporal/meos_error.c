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
 * @brief Error handling of the MEOS library
 * @details A message reaches its reader through two channels, which differ in
 * what the caller has to do next.
 * - The error handler receives every message when it is raised, with its
 *   level, its code and its text: #meos_error() calls the handler installed
 *   by #meos_initialize_error_handler(), which prints it, logs it or hands it
 *   to the host. The levels are those of #errorLevel.
 * - The error number, read by #meos_errno(), records that an error happened:
 *   the function that raised it returned its sentinel, and every frame above
 *   it passed that failure up, freeing what it had built. It keeps the code of
 *   the last error raised until #meos_errno_reset() clears it.
 *
 * Only a message of level @p MEOS_ERROR reaches the second channel. A warning,
 * a notice or an information accompanies an answer, as liblwgeom's notices and
 * the defaults the MF-JSON reader takes do, so it goes to the handler and
 * leaves the error number as it is. This is how PostgreSQL reports them: an
 * error ends the statement, while a warning is sent to the client and the log
 * and the statement goes on. In the extension, #meos_error() hands every
 * message to ereport() at its own level.
 *
 * liblwgeom reports through the same two channels, its errors and its notices
 * being routed to #meos_error() by the two handlers #meos_initialize() installs
 * (meos/src/temporal/meos.c). MEOS calls no GEOS function, so it installs no
 * GEOS handler: the GEOS names the vendored raster core calls are answered in
 * meos/src/geo/geo_geos_none.c, which ignores the handlers given to initGEOS()
 * and routes an error to liblwgeom's.
 *
 * A host that wants both channels installs its own handler, records the code
 * of an error with #meos_errno_set() and logs the other levels:
 * @code
 * static void
 * host_handler(int errlevel, int errcode, const char *errmsg)
 * {
 *   if (errlevel == MEOS_ERROR)
 *   {
 *     meos_errno_set(errcode);
 *     host_remember_error(errmsg);
 *   }
 *   else
 *     host_log_warning(errcode, errmsg);
 * }
 * @endcode
 * After each call it reads #meos_errno(): a set error number means the answer
 * is a sentinel, and the host raises its own exception with the message it
 * remembered and resets the error number.
 */

/* C */
#include <errno.h>
#include <stdarg.h>
/* Postgres */
#include <postgres.h>
#if ! MEOS
  #include "utils/elog.h"
  #include <miscadmin.h> /* For CHECK_FOR_INTERRUPTS */
#endif /* ! MEOS */
/* MEOS */
#include <meos.h>
#include <meos_tls.h>     /* MEOS_TLS — per-thread error number (restore #815) */
#include "temporal/temporal.h"

/*****************************************************************************
 * Global variables
 *****************************************************************************/

/**
 * @brief Global variable that keeps the last error number
 */
static MEOS_TLS int MEOS_ERR_NO = 0;

/**
 * @ingroup meos_misc
 * @brief Read an error number
 */
int
meos_errno(void)
{
  return MEOS_ERR_NO;
}

#if MEOS
/**
 * @ingroup meos_misc
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
 * @ingroup meos_misc
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

/**
 * @ingroup meos_misc
 * @brief Reset the error number to zero and return its previous value
 */
int meos_errno_reset(void)
{
  int last_errno = meos_errno();
  /* Clear the error status directly: #meos_errno_set ignores a zero argument
   * so that a caller cannot clear the status through it by accident, and
   * points here instead */
  MEOS_ERR_NO = 0;
  errno = 0;
  return last_errno;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Global variable that keeps the error handler function
 */
static void (*MEOS_ERROR_HANDLER)(int, int, const char *) = NULL;

/* The levels a handler receives are PostgreSQL's, which #errorLevel publishes
 * under its own names */
StaticAssertDecl(MEOS_INFO == INFO && MEOS_NOTICE == NOTICE &&
  MEOS_WARNING == WARNING && MEOS_ERROR == ERROR,
  "errorLevel must state the values of the PostgreSQL message levels");

#if MEOS
/**
 * @brief Default error handler function that prints the message to stderr
 * and, for an error, records its code and exits
 * @details A message of a lower level than @p ERROR accompanies an answer, so
 * it is printed and leaves the error number as it is
 */
void
default_error_handler(int errlevel, int errcode, const char *errmsg)
{
  fprintf(stderr, "%s\n", errmsg);
  if (errlevel == ERROR)
  {
    meos_errno_set(errcode);
    exit(EXIT_FAILURE);
  }
  return;
}

/**
 * @brief Error handler function that prints the message through perror and,
 * for an error, records its code
 */
void
error_handler_errno(int errlevel, int errcode, const char *errmsg)
{
  perror(errmsg);
  if (errlevel == ERROR)
    meos_errno_set(errcode);
  return;
}

/**
 * @ingroup meos_setup
 * @brief Initialize error handler function
 */
void
meos_initialize_error_handler(error_handler_fn err_handler)
{
  /* Publish with release semantics so a thread that acquire-loads the handler
   * (in meos_error) sees a fully-published value — concurrent host worker
   * threads each call meos_initialize() (restore #815's thread-safety, which the
   * pgtypes vendoring reverted). */
  __atomic_store_n(&MEOS_ERROR_HANDLER,
    err_handler ? err_handler : &default_error_handler, __ATOMIC_RELEASE);
  return;
}

/**
 * @brief No-exit error handler reporting via meos_errno() only
 * @details Never calls exit(); safe to invoke from a foreign thread in a
 * JVM (JNR-FFI on Spark / JMEOS) where exit() would tear the host process
 * down. Errors are reported through meos_errno() only, recorded at level
 * @p ERROR as #default_error_handler records them, and a message of a lower
 * level, which accompanies an answer, is dropped: a host that wants it
 * installs its own handler, as the file comment shows.
 */
static void
noexit_error_handler(int errlevel, int errcode,
  const char *errmsg __attribute__((__unused__)))
{
  if (errlevel == ERROR)
    meos_errno_set(errcode);
  return;
}

/**
 * @ingroup meos_setup
 * @brief Install the no-exit error handler
 * @details Safe to call from multiple threads -- the underlying atomic
 * store is idempotent and always sets the same function pointer.
 */
void
meos_initialize_noexit_error_handler(void)
{
  __atomic_store_n(&MEOS_ERROR_HANDLER, &noexit_error_handler, __ATOMIC_RELEASE);
  return;
}

/**
 * @brief Global variable that keeps the interrupt handler function
 */
static void (*MEOS_INTERRUPT_HANDLER)(void) = NULL;

/**
 * @ingroup meos_setup
 * @brief Initialize interrupt handler function
 * @details The long computations of MEOS call the handler between two of
 * their steps. A handler stops the computation by not returning, as a
 * PostgreSQL backend leaves through its error on a cancel request, and one
 * that returns lets the computation go on. A NULL handler removes the one
 * installed
 */
void
meos_initialize_interrupt_handler(interrupt_handler_fn handler)
{
  /* Published as the error handler is, see #meos_initialize_error_handler */
  __atomic_store_n(&MEOS_INTERRUPT_HANDLER, handler, __ATOMIC_RELEASE);
  return;
}
#endif /* MEOS */

/**
 * @brief Give a long computation the chance to be stopped between two of its
 * steps
 * @details MobilityDB answers the cancel request of the backend, and MEOS calls
 * the handler installed by #meos_initialize_interrupt_handler(), if any
 */
void
meos_check_for_interrupts(void)
{
#if MEOS
  interrupt_handler_fn handler =
    __atomic_load_n(&MEOS_INTERRUPT_HANDLER, __ATOMIC_ACQUIRE);
  if (handler)
    handler();
#else
  CHECK_FOR_INTERRUPTS();
#endif /* MEOS */
  return;
}

/*****************************************************************************/

#if ! MEOS
/**
 * @brief Output an error message in MobilityDB
 */
static void
pg_error(int errlevel, const char *errmsg)
{
  ereport(errlevel, (errmsg_internal("%s", errmsg)));
  return;
}
#endif /* ! MEOS */

/**
 * @ingroup meos_misc
 * @brief Function handling error messages
 *
 * @note Return-or-not contract is *undefined*: depending on the
 * installed handler this function MAY return to the caller. The
 * default handler `exit(EXIT_FAILURE)`s on `ERROR` (safe for one-shot
 * CLI use); any custom handler installed via
 * `meos_initialize_error_handler` may return. Code in MEOS that calls
 * `meos_error(ERROR, ...)` MUST be immediately followed by a `return`,
 * `goto`, `break`, or sentinel assignment -- NEVER let execution fall
 * through and assume the call did not return. See the corresponding
 * doc comment on the public declaration of `meos_error` in
 * `meos/include/meos_error.h`.
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
  /* Execute the error handler function (acquire-load, paired with the
   * release-store in meos_initialize_error_handler — restore #815) */
  void (*handler)(int, int, const char *) =
    __atomic_load_n(&MEOS_ERROR_HANDLER, __ATOMIC_ACQUIRE);
  if (handler)
    handler(errlevel, errcode, buffer);
  else
#if MEOS
  {
    fprintf (stderr, "%s\n", buffer);
    if (errlevel == ERROR)
      exit(EXIT_FAILURE);
  }
#else
    pg_error(errlevel, strdup(buffer));
#endif /* MEOS */
  return;
}

/*****************************************************************************/
