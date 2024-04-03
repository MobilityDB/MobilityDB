/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Global variable that keeps the last error number
 */
static int _MEOS_ERRNO = 0;

/**
 * @brief Read an error number
 */
int
meos_errno(void)
{
  return _MEOS_ERRNO;
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

  _MEOS_ERRNO = err;
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
 * @brief Global variable that keeps the error handler function
 */
void (*_ERROR_HANDLER)(int, int, char *) = NULL;

#if MEOS
/**
 * @brief Default error handler function that prints the error message to
 * stderr, sets the errcode, and exits if error level is equal to `ERROR`
 */
void
default_error_handler(int errlevel, int errcode, char *errmsg)
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
  char *errmsg)
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
  if (err_handler)
    _ERROR_HANDLER = err_handler;
  else
    _ERROR_HANDLER = &default_error_handler;
  return;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Function handling error messages
 */
void
meos_error(int errlevel, int errcode, char *format, ...)
{
  char buffer[1024];
  va_list args;
  va_start(args, format);
  /* TODO: maybe check if the error message was truncated */
  vsnprintf(buffer, 1024, format, args);
  va_end(args);
  /* Execute the error handler function */
  if (_ERROR_HANDLER)
    _ERROR_HANDLER(errlevel, errcode, buffer);
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
