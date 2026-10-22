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
 * @brief A program that tests the version functions of the MEOS API
 * @details Each of the four functions answers a string the caller frees, and
 * each string starts with the name of the library. The program frees every
 * string with the libc allocator, which the default allocator of MEOS is.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o version_test version_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>

/* Assert that a version string names the library, and free it */
static void
check_version(const char *fn, char *version)
{
  assert(version);
  assert(strncmp(version, "MobilityDB", strlen("MobilityDB")) == 0);
  printf("%s: %s\n", fn, version);
  free(version);
}

/* Main program */
int main(void)
{
  meos_initialize();

  /* Two calls answer two strings, so freeing the first leaves the second */
  char *first = meos_version();
  char *second = meos_version();
  assert(first != second);
  assert(strcmp(first, second) == 0);
  check_version("meos_version", first);
  check_version("meos_version", second);

  check_version("meos_full_version", meos_full_version());
  check_version("mobilitydb_version", mobilitydb_version());
  check_version("mobilitydb_full_version", mobilitydb_full_version());

  meos_finalize();
  printf("version_test: OK\n");
  return 0;
}
