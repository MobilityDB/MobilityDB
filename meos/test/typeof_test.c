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
 * @brief A program that tests the @ref meos_typeof_hexwkb type discriminator,
 * which peeks the MeosType tag from the header of a hex-encoded Well-Known
 * Binary (WKB) string of a set, span, span set, or temporal value.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o typeof_test typeof_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_catalog.h>

/* Number of failed checks */
static int fails = 0;
/* Scratch output size for the @c *_as_hexwkb functions */
static size_t sz;

/**
 * @brief Check that the type peeked from a hex-WKB string matches the expected
 * MeosType, accumulating into the failure counter
 */
static void
check(const char *label, const char *hexwkb, MeosType expected)
{
  MeosType got = meos_typeof_hexwkb(hexwkb);
  printf("%-12s -> %2d (expect %2d) %s\n", label, got, expected,
    got == expected ? "OK" : "FAIL");
  if (got != expected)
    fails++;
}

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Self-describing serializations: the concrete type is recovered. The value
   * and the text it serializes to are both allocations of this program, so
   * each is held in a variable long enough to be freed: a value handed
   * straight to the serializer, or a serialization handed straight to the
   * check, is one nothing can free */
  void *val;
  char *hex;

  val = tint_in("1@2001-01-01");
  hex = temporal_as_hexwkb(val, 0, &sz);
  check("tint", hex, T_TINT);
  free(hex); free(val);

  val = tfloat_in("1.5@2001-01-01");
  hex = temporal_as_hexwkb(val, 0, &sz);
  check("tfloat", hex, T_TFLOAT);
  free(hex); free(val);

  val = intspan_in("[1, 3]");
  hex = span_as_hexwkb(val, 0, &sz);
  check("intspan", hex, T_INTSPAN);
  free(hex); free(val);

  val = floatspan_in("[1.0, 3.0]");
  hex = span_as_hexwkb(val, 0, &sz);
  check("floatspan", hex, T_FLOATSPAN);
  free(hex); free(val);

  val = tstzspan_in("[2001-01-01, 2001-01-02]");
  hex = span_as_hexwkb(val, 0, &sz);
  check("tstzspan", hex, T_TSTZSPAN);
  free(hex); free(val);

  val = intset_in("{1, 2, 3}");
  hex = set_as_hexwkb(val, 0, &sz);
  check("intset", hex, T_INTSET);
  free(hex); free(val);

  val = intspanset_in("{[1,2],[4,5]}");
  hex = spanset_as_hexwkb(val, 0, &sz);
  check("intspanset", hex, T_INTSPANSET);
  free(hex); free(val);

  /* Malformed input yields T_UNKNOWN without raising */
  check("null", NULL, T_UNKNOWN);
  check("short", "AB", T_UNKNOWN);
  check("bad_endian", "FFFFFF", T_UNKNOWN);

  printf("\n%s (%d failure%s)\n", fails ? "FAILED" : "ALL PASS", fails,
    fails == 1 ? "" : "s");

  /* Finalize MEOS */
  meos_finalize();
  return fails;
}
