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
 * @brief A program that tests how the setup functions of the MEOS API report
 * a null value under the noexit error handler
 * @details A public MEOS function tests its arguments at entry, so a binding
 * calling it with a null pointer receives an error it can raise in its host
 * language rather than a crash.
 *
 * The program verifies that #meos_set_datestyle and #meos_set_intervalstyle
 * report a null style by returning false and setting #meos_errno, keeping the
 * style set before, that #meos_set_spatial_ref_sys_csv and #meos_set_ways_csv
 * report a null path by setting #meos_errno, and that a valid call still
 * sets the style with no error left behind.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o setup_validity_test setup_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>

/**
 * @brief Check that a setter answered the expected result and left the
 * expected error, then clear it
 */
static void
check(const char *call, bool result, bool expected, int errcode)
{
  printf("%s: %s, errno %d\n", call, result ? "true" : "false", meos_errno());
  assert(result == expected);
  assert(meos_errno() == errcode);
  meos_errno_reset();
}

/**
 * @brief Check that a style getter answers the expected name
 */
static void
check_style(char *style, const char *want, const char *call)
{
  printf("%s: %s\n", call, style ? style : "NULL");
  assert(style && strcmp(style, want) == 0);
  free(style);
}

/* Main program */
int main(void)
{
  size_t size;
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_noexit_error_handler();

  /* A valid style is set with no error left behind */
  check("meos_set_datestyle(ISO, DMY)", meos_set_datestyle("ISO, DMY", NULL),
    true, 0);
  check("meos_set_intervalstyle(iso_8601)",
    meos_set_intervalstyle("iso_8601", 0), true, 0);

  /* A null style is reported and the style set before stays */
  check("meos_set_datestyle(NULL)", meos_set_datestyle(NULL, NULL), false,
    MEOS_ERR_INVALID_ARG);
  check("meos_set_intervalstyle(NULL)", meos_set_intervalstyle(NULL, 0), false,
    MEOS_ERR_INVALID_ARG);
  check_style(meos_get_datestyle(&size), "ISO, DMY", "meos_get_datestyle");
  check_style(meos_get_intervalstyle(&size), "iso_8601",
    "meos_get_intervalstyle");

  /* A null path is reported rather than copied */
  meos_set_spatial_ref_sys_csv(NULL);
  check("meos_set_spatial_ref_sys_csv(NULL)", false, false,
    MEOS_ERR_INVALID_ARG);
  meos_set_ways_csv(NULL);
  check("meos_set_ways_csv(NULL)", false, false, MEOS_ERR_INVALID_ARG);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
