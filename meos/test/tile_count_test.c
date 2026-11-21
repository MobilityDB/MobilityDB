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
 * @brief Test that a grid holding more tiles or bins than an integer counts is
 * refused
 * @details A program that tests, under the noexit error handler, the number
 * of tiles and bins the tiling functions lay over a value. The number is
 * returned through an integer, so a grid holding more of them than the
 * largest integer, along one dimension or as the product of its dimensions,
 * is reported by setting #meos_errno to MEOS_ERR_VALUE_OUT_OF_RANGE and
 * answers no tiles, rather than a count that wrapped around. The last bin of
 * an integer or time grid that ends past the range of its type is refused in
 * the same way, before the bins are counted. A grid within these limits keeps
 * its tiles.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tile_count_test tile_count_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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

/* A call that must be refused with a value out of range and no tiles */
static void
check_refused(const char *what, const void *result, int count)
{
  check(what, result == NULL && count == 0 &&
    meos_errno() == MEOS_ERR_VALUE_OUT_OF_RANGE);
}

/* A call that must answer a given number of tiles with no error */
static void
check_count(const char *what, void *result, int count, int expected)
{
  check(what, result && count == expected && meos_errno() == 0);
  free(result);
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  int count;
  Interval *day = interval_in("1 day", -1);
  Interval *sec = interval_in("1 second", -1);
  Interval *usec = interval_in("1 microsecond", -1);
  TimestampTz torig = timestamptz_in("2001-01-01", -1);
  GSERIALIZED *o2 = geompoint_make2d(0, 0, 0);
  GSERIALIZED *o3 = geompoint_make3dz(0, 0, 0, 0);
  void *r;

  /* The bins along one dimension, past the largest integer */
  TBox *intbox = tbox_in("TBOXINT X([-2147483648, 2147483646])");
  r = tintbox_value_tiles(intbox, 1, 0, true, &count);
  check_refused("tintbox_value_tiles, the integers by 1", r, count);
  TBox *bigintbox = tbox_in("TBOXBIGINT X([0, 9000000000000000000])");
  r = tbigintbox_value_tiles(bigintbox, 1, 0, true, &count);
  check_refused("tbigintbox_value_tiles, 9e18 by 1", r, count);
  TBox *yearbox = tbox_in("TBOXINT XT([0, 10],[2001-01-01, 2002-01-01])");
  r = tintbox_time_tiles(yearbox, usec, torig, true, &count);
  check_refused("tintbox_time_tiles, a year by 1 microsecond", r, count);

  /* The last bin ending past the range of the type */
  TBox *intmax = tbox_in("TBOXINT X([2147483600, 2147483646])");
  r = tintbox_value_tiles(intmax, 10, 0, true, &count);
  check_refused("tintbox_value_tiles, a bin past the integers", r, count);
  TBox *bigintmax = tbox_in("TBOXBIGINT X([0, 9223372036854775806])");
  r = tbigintbox_value_tiles(bigintmax, 4611686018427387904, 0, true, &count);
  check_refused("tbigintbox_value_tiles, a bin past the big integers", r,
    count);

  /* The product of the dimensions past the largest integer */
  TBox *floatbox = tbox_in("TBOXFLOAT XT([0, 100000],[2001-01-01, 2001-01-31])");
  r = tfloatbox_value_time_tiles(floatbox, 1.0, sec, 0.0, torig, true,
    &count);
  check_refused("tfloatbox_value_time_tiles, 1e5 by 30 days of seconds", r,
    count);
  STBox *bigbox = stbox_in("STBOX X((0,0),(100000,100000))");
  r = stbox_space_tiles(bigbox, 1, 1, 1, o2, true, &count);
  check_refused("stbox_space_tiles, 1e5 by 1e5", r, count);
  /* 65536 times 65537 tiles, whose product wraps around to 65536 */
  STBox *wrapbox = stbox_in("STBOX X((0,0),(65535.5,65536.5))");
  r = stbox_space_tiles(wrapbox, 1, 1, 1, o2, true, &count);
  check_refused("stbox_space_tiles, 65536 by 65537", r, count);
  STBox *box3d = stbox_in("STBOX Z((0,0,0),(2000,2000,1000))");
  r = stbox_space_tiles(box3d, 1, 1, 1, o3, true, &count);
  check_refused("stbox_space_tiles, 2000 by 2000 by 1000", r, count);
  Temporal *tpt = tgeompoint_in(
    "[Point(0 0)@2001-01-01, Point(100000 100000)@2001-01-02]");
  r = tgeo_space_boxes(tpt, 1, 1, 1, o2, false, true, &count);
  check_refused("tgeo_space_boxes, 1e5 by 1e5", r, count);
  r = tgeo_space_boxes(tpt, 1, 1, 1, o2, true, true, &count);
  check_refused("tgeo_space_boxes, 1e5 by 1e5 with a bit matrix", r, count);
  GSERIALIZED **space_bins;
  TimestampTz *time_bins;
  r = tgeo_space_time_split(tpt, 1, 1, 1, day, o2, torig, false, true,
    &space_bins, &time_bins, &count);
  check_refused("tgeo_space_time_split, 1e5 by 1e5", r, count);
  /* The split writes a fragment per tile the value enters, past an array
   * sized by a wrapped count */
  Temporal *tfloat = tfloat_in(
    "[0@2001-01-01, 65535.5@2001-01-01 18:12:16.5]");
  double *value_bins;
  r = tfloat_value_time_split(tfloat, 1.0, sec, 0.0, torig, true,
    &value_bins, &time_bins, &count);
  check_refused("tfloat_value_time_split, 65536 by 65537", r, count);

  /* A grid within the limits keeps its tiles */
  TBox *smallint = tbox_in("TBOXINT X([0, 100])");
  r = tintbox_value_tiles(smallint, 7, 0, true, &count);
  check_count("tintbox_value_tiles, 0..100 by 7", r, count, 15);
  TBox *smallfloat = tbox_in("TBOXFLOAT XT([0, 10],[2001-01-01, 2001-01-05])");
  r = tfloatbox_value_time_tiles(smallfloat, 1.0, day, 0.0, torig, false,
    &count);
  check_count("tfloatbox_value_time_tiles, 0..10 by 5 days", r, count, 40);
  STBox *smallbox = stbox_in("STBOX X((0,0),(10,10))");
  r = stbox_space_tiles(smallbox, 3, 3, 3, o2, true, &count);
  check_count("stbox_space_tiles, 0..10 by 3", r, count, 16);
  Temporal *smallpt = tgeompoint_in(
    "[Point(0 0)@2001-01-01, Point(10 10)@2001-01-02]");
  r = tgeo_space_boxes(smallpt, 1, 1, 1, o2, true, true, &count);
  check_count("tgeo_space_boxes, 0..10 by 1", r, count, 11);

  free(intbox); free(bigintbox); free(yearbox); free(intmax);
  free(bigintmax); free(floatbox); free(bigbox); free(wrapbox); free(box3d);
  free(tpt); free(tfloat); free(smallint); free(smallfloat); free(smallbox);
  free(smallpt); free(day); free(sec); free(usec); free(o2); free(o3);
  meos_finalize();

  printf("%d check(s) failed\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
