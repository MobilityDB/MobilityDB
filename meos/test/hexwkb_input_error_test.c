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
 * @brief A program that tests that the hexadecimal Well-Known Binary (HexWKB)
 * input of every value refuses a string that encodes no bytes
 * @details A HexWKB string encodes a byte by two hexadecimal digits, so a
 * string of an odd length, or one carrying a character that is not a
 * hexadecimal digit, encodes nothing, and the empty string encodes no byte.
 * The decoder raises an error and returns no bytes for the first two, and the
 * reader of the third finds no endian flag, so the reader must then return no
 * value instead of parsing the bytes it was not given.
 *
 * For each reader, the program writes a well-formed value as HexWKB, reads it
 * back, and reads it with its last digit removed, with its last character a
 * letter that is no hexadecimal digit, and as the empty string: each of the
 * three must be refused with #meos_errno set. #meos_typeof_hexwkb, which
 * never raises an error, must answer `T_UNKNOWN` for the first two.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o hexwkb_input_error_test hexwkb_input_error_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_catalog.h>
#include <meos_cbuffer.h>
#include <meos_geo.h>
#include <meos_npoint.h>
#include <meos_pose.h>

typedef void *(*input_fn)(const char *);

static void *set_v(const char *str) { return set_from_hexwkb(str); }
static void *span_v(const char *str) { return span_from_hexwkb(str); }
static void *spanset_v(const char *str) { return spanset_from_hexwkb(str); }
static void *tbox_v(const char *str) { return tbox_from_hexwkb(str); }
static void *temporal_v(const char *str) { return temporal_from_hexwkb(str); }
static void *stbox_v(const char *str) { return stbox_from_hexwkb(str); }
static void *cbuffer_v(const char *str) { return cbuffer_from_hexwkb(str); }
static void *npoint_v(const char *str) { return npoint_from_hexwkb(str); }
static void *pose_v(const char *str) { return pose_from_hexwkb(str); }

static int failures = 0;

/* Read a string that must be refused, or accepted, and free what is
 * returned */
static void
check(const char *what, input_fn in, const char *str, bool accept)
{
  meos_errno_reset();
  void *value = in(str);
  bool ok = accept ? (value != NULL && meos_errno() == 0) :
    (value == NULL && meos_errno() != 0);
  if (! ok)
  {
    printf("  %s(\"%s\"): expected %s, got %s\n", what, str,
      accept ? "a value" : "an error", value ? "a value" : "an error");
    failures++;
  }
  free(value);
  meos_errno_reset();
}

/* Read the HexWKB of a value, then the string with its last digit removed
 * and with its last character a letter that is no hexadecimal digit */
static void
check_reader(const char *what, input_fn in, char *hex)
{
  check(what, in, hex, true);
  size_t len = strlen(hex);
  char *odd = strdup(hex);
  odd[len - 1] = '\0';
  check(what, in, odd, false);
  char *nothex = strdup(hex);
  nothex[len - 1] = 'G';
  check(what, in, nothex, false);
  check(what, in, "", false);
  /* The discriminator answers without raising an error */
  meos_errno_reset();
  if (meos_typeof_hexwkb(odd) != T_UNKNOWN ||
      meos_typeof_hexwkb(nothex) != T_UNKNOWN)
  {
    printf("  meos_typeof_hexwkb of a %s that encodes nothing: expected "
      "T_UNKNOWN\n", what);
    failures++;
  }
  meos_errno_reset();
  free(odd); free(nothex); free(hex);
}

/* Main program */
int main(void)
{
  /* Install the error handler that reports through meos_errno instead of
   * exiting, after meos_initialize, which installs the default one */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  size_t size;
  Set *s = intset_in("{1, 2, 3}");
  check_reader("set", set_v, set_as_hexwkb(s, 0, &size));
  Span *sp = floatspan_in("[1, 3]");
  check_reader("span", span_v, span_as_hexwkb(sp, 0, &size));
  SpanSet *ss = intspanset_in("{[1, 2], [4, 5]}");
  check_reader("spanset", spanset_v, spanset_as_hexwkb(ss, 0, &size));
  TBox *box = tbox_in("TBOXFLOAT XT([1, 3],[2001-01-01, 2001-01-02])");
  check_reader("tbox", tbox_v, tbox_as_hexwkb(box, 0, &size));
  Temporal *temp = tfloat_in("[1@2001-01-01, 3@2001-01-02]");
  check_reader("temporal", temporal_v, temporal_as_hexwkb(temp, 0, &size));
  STBox *sbox = stbox_in("STBOX XT(((1,1),(2,2)),[2001-01-01, 2001-01-02])");
  check_reader("stbox", stbox_v, stbox_as_hexwkb(sbox, 0, &size));
  Cbuffer *cb = cbuffer_in("Cbuffer(Point(1 1), 0.5)");
  check_reader("cbuffer", cbuffer_v, cbuffer_as_hexwkb(cb, 0, &size));
  Npoint *np = npoint_in("Npoint(1, 0.5)");
  check_reader("npoint", npoint_v, npoint_as_hexwkb(np, 0, &size));
  Pose *pose = pose_in("Pose(Point(1 1), 0.5)");
  check_reader("pose", pose_v, pose_as_hexwkb(pose, 0, &size));
  free(s); free(sp); free(ss); free(box); free(temp); free(sbox); free(cb);
  free(np); free(pose);

  printf("%d check(s) failed\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
