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
 * @brief A program that tests the typed text input and output of the temporal
 * cell index and point cloud types.
 *
 * Each temporal type gives its text form through a typed public function
 * that tests its argument, as tint_out does for a temporal integer. The
 * program verifies that th3index_out, tquadbin_out, ts2cell_out,
 * tpcpoint_out and tpcpatch_out answer the text their input reads back with
 * no error left behind, that each reports a null value with
 * MEOS_ERR_INVALID_ARG and a temporal integer with MEOS_ERR_INVALID_ARG_TYPE,
 * and that tpcpoint_in and tpcpatch_in report a null string with
 * MEOS_ERR_INVALID_ARG.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o temporal_text_io_test temporal_text_io_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_h3.h>
#include <meos_pointcloud.h>
#include <meos_quadbin.h>
#include <meos_s2cell.h>

/* A pcpoint and a pcpatch of pcid 1 in the hex WKB pgPointCloud serializes,
 * the form that carries no schema, as tpointcloud_test reads them */
#define TPCPOINT_IN \
  "2300000001000000000000000000F03F0000000000000040000000000000084000" \
  "0000@2024-01-01"
#define TPCPATCH_IN \
  "4F000000010000000000000002000000000000000000F03F000000000000F03F00" \
  "0000000000F03F0000000000000040000000000000004000000000000000400000" \
  "00000000000000000000000000@2024-01-01"

typedef Temporal *(*text_in_fn)(const char *);
typedef char *(*text_out_fn)(const Temporal *);

/* Read a value, write it out, read the output back and write it again: the
 * two outputs agree, the first reading back what the input reads. Then the
 * output reports a null value and a value of another type. */
static void
check(const char *name, text_in_fn in, text_out_fn out, const char *text,
  const Temporal *other)
{
  Temporal *temp = in(text);
  assert(temp && meos_errno() == 0);
  char *str1 = out(temp);
  Temporal *back = str1 ? in(str1) : NULL;
  char *str2 = back ? out(back) : NULL;
  printf("%s: %s, errno %d\n", name, str1 ? str1 : "NULL", meos_errno());
  assert(str1 && str2 && strcmp(str1, str2) == 0);
  assert(meos_errno() == 0);
  free(temp); free(back); free(str1); free(str2);

  char *res = out(NULL);
  printf("%s(NULL): %s, errno %d\n", name, res ? "a value" : "NULL",
    meos_errno());
  assert(res == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  res = out(other);
  printf("%s(tint): %s, errno %d\n", name, res ? "a value" : "NULL",
    meos_errno());
  assert(res == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  meos_errno_reset();
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Temporal *tint = tint_in("{1@2001-01-01}");
  assert(tint);

  check("th3index_out", th3index_in, th3index_out,
    "{880326b885fffff@2001-01-01, 880326b88dfffff@2001-01-02}", tint);
  check("tquadbin_out", tquadbin_in, tquadbin_out,
    "{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}", tint);
  check("ts2cell_out", ts2cell_in, ts2cell_out,
    "{47c3c3@2001-01-01, 54b5c9@2001-01-02}", tint);
  check("tpcpoint_out", tpcpoint_in, tpcpoint_out, TPCPOINT_IN, tint);
  check("tpcpatch_out", tpcpatch_in, tpcpatch_out, TPCPATCH_IN, tint);

  /* A null string is reported rather than parsed */
  Temporal *res = tpcpoint_in(NULL);
  printf("tpcpoint_in(NULL): %s, errno %d\n", res ? "a value" : "NULL",
    meos_errno());
  assert(res == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  res = tpcpatch_in(NULL);
  printf("tpcpatch_in(NULL): %s, errno %d\n", res ? "a value" : "NULL",
    meos_errno());
  assert(res == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  free(tint);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
