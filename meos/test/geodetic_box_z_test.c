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
 * @brief Test the restriction of a temporal geography point with Z to a
 * geodetic box with Z
 * @details A program that tests, under the noexit error handler, that the
 * restriction of a temporal geography point with Z to a geodetic box with Z
 * keeps the instants and periods whose height lies in the Z dimension of the
 * box, and no other. An end of a segment is held by its own longitude,
 * latitude and height, and a segment holding one position keeps the stretch
 * its height spends in the Z dimension of the box.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o geodetic_box_z_test geodetic_box_z_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

static int failures = 0;

/* Report a check and count it when it fails */
static void
check(const char *what, bool ok)
{
  printf("%s: %s\n", ok ? "ok  " : "FAIL", what);
  if (! ok)
    failures++;
  meos_errno_reset();
}

/* The restriction of a value to a box must be the value of a text, or none */
static void
check_at(const char *what, const char *value, const char *box,
  const char *expected)
{
  Temporal *temp = tgeogpoint_in(value);
  STBox *b = stbox_in(box);
  Temporal *at = tgeo_at_stbox(temp, b, true);
  bool ok;
  if (! expected)
    ok = (at == NULL && meos_errno() == 0);
  else
  {
    Temporal *exp = tgeogpoint_in(expected);
    ok = at && exp && temporal_eq(at, exp);
    free(exp);
  }
  check(what, ok);
  free(temp); free(b); free(at);
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* The trip climbs through the Z dimension of the box on its first segment
   * and stays above it on the others: the end at 15:47 lies in the spatial
   * dimensions of the box, 4.7 above its Z dimension */
  const char *trip = "SRID=4326;[POINT Z(-3.714842326024667 69.6463030556779 "
    "813.6940407796993)@2001-06-01 15:41:00+00, POINT Z(1.4200937305580128 "
    "68.65016061727098 819.5813973045573)@2001-06-01 15:44:00+00, "
    "POINT Z(3.129117866999249 63.468930560239784 820.307046610663)"
    "@2001-06-01 15:47:00+00, POINT Z(7.965963272819576 61.588508967723065 "
    "826.913800757691)@2001-06-01 15:56:00+00]";
  const char *below = "SRID=4326;GEODSTBOX Z((-6.731185506897736,"
    "48.58588557262055,801.2817106761466),(8.891908066965673,"
    "66.08661948953407,815.6023182529326))";
  check_at("an end above the box", trip, below, NULL);
  Temporal *temp = tgeogpoint_in(trip);
  STBox *box = stbox_in(below);
  check("the trip does not ever overlap the box",
    ever_overlaps_tgeo_stbox(temp, box) == 0);
  free(box);
  /* The same box reaching the height of that end holds it */
  box = stbox_in("SRID=4326;GEODSTBOX Z((-6.731185506897736,"
    "48.58588557262055,801.2817106761466),(8.891908066965673,"
    "66.08661948953407,821))");
  check("the trip ever overlaps a box reaching the height of its end",
    ever_overlaps_tgeo_stbox(temp, box) == 1);
  Temporal *at = tgeo_at_stbox(temp, box, true);
  check("the restriction holds the end the box reaches",
    at && temporal_num_instants(at) >= 1);
  free(at); free(box); free(temp);

  /* A segment holding one position, which climbs from 100 to 300 */
  const char *climb = "SRID=4326;[POINT Z(5 50 100)@2001-06-01 10:00:00+00, "
    "POINT Z(5 50 300)@2001-06-01 10:20:00+00]";
  check_at("one position, a box above its height", climb,
    "SRID=4326;GEODSTBOX Z((4,49,400),(6,51,500))", NULL);
  check_at("one position, the stretch in the Z dimension of the box", climb,
    "SRID=4326;GEODSTBOX Z((4,49,150),(6,51,200))",
    "SRID=4326;{[POINT Z(5 50 150)@2001-06-01 10:05:00+00, "
    "POINT Z(5 50 200)@2001-06-01 10:10:00+00]}");
  check_at("one position, a box holding all its heights", climb,
    "SRID=4326;GEODSTBOX Z((4,49,0),(6,51,1000))",
    "SRID=4326;{[POINT Z(5 50 100)@2001-06-01 10:00:00+00, "
    "POINT Z(5 50 300)@2001-06-01 10:20:00+00]}");

  meos_finalize();
  printf("%d check(s) failed\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
