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
 * @brief Test the bin and the tile that contain a value
 * @details A program that tests the functions answering the SQL functions
 * `getBin`, `getValueTile`, `getTBoxTimeTile` and `getValueTimeTile`, under
 * the noexit error handler.
 *
 * The bin of a value is the half-open span of the size of the bins starting
 * at the initial value of the bin, which the `<type>_get_bin` functions
 * return. The tile of a value and a timestamp is the box whose value span and
 * period are the bins of the value and of the timestamp.
 *
 * The program verifies the answers that the SQL functions give for the same
 * arguments, that each bin starts at the initial value of the bin and has the
 * size of the bins, that each tile is made of the bins of its value and its
 * timestamp, and that a size that is not positive, a missing duration and a
 * bin outside the range of the type are reported by setting #meos_errno.
 *
 * The program also verifies that `tbox_time_tiles`, which answers the SQL
 * function `timeTiles(tbox, ...)`, gives for a box of any number type the
 * time tiles of the function of its value type.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o bin_tile_test bin_tile_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>

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

/* A span that must equal the span of a text */
static void
check_span(const char *what, Span *s, Span *expected)
{
  check(what, s && expected && span_eq(s, expected));
  free(s); free(expected);
}

/* A box that must equal the box of a text */
static void
check_box(const char *what, TBox *box, const char *expected)
{
  TBox *exp = tbox_in(expected);
  check(what, box && exp && tbox_eq(box, exp));
  free(box); free(exp);
}

/* An array of boxes that must equal the boxes of the texts */
static void
check_boxes(const char *what, TBox *boxes, int count, const char **expected,
  int nexpected)
{
  bool ok = boxes && count == nexpected;
  for (int i = 0; ok && i < count; i++)
  {
    TBox *exp = tbox_in(expected[i]);
    ok = exp && tbox_eq(&boxes[i], exp);
    free(exp);
  }
  check(what, ok);
  free(boxes);
}

/* Two arrays of boxes that must be equal */
static void
check_same_boxes(const char *what, TBox *boxes1, int count1, TBox *boxes2,
  int count2)
{
  bool ok = boxes1 && boxes2 && count1 == count2;
  for (int i = 0; ok && i < count1; i++)
    ok = tbox_eq(&boxes1[i], &boxes2[i]);
  check(what, ok);
  free(boxes1); free(boxes2);
}

/* A call that must be refused with a given error */
static void
check_refused(const char *what, const void *result, int err)
{
  check(what, result == NULL && meos_errno() == err);
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* The answers of the SQL functions, from 025_temporal_tile.test.sql */
  check_span("intspan_get_bin(3, 2, 0)", intspan_get_bin(3, 2, 0),
    intspan_in("[2, 4)"));
  check_span("intspan_get_bin(3, 2, 1)", intspan_get_bin(3, 2, 1),
    intspan_in("[3, 5)"));
  check_span("intspan_get_bin(-3, 2, -2)", intspan_get_bin(-3, 2, -2),
    intspan_in("[-4, -2)"));
  check_span("bigintspan_get_bin(3, 2, 1)", bigintspan_get_bin(3, 2, 1),
    bigintspan_in("[3, 5)"));
  check_span("floatspan_get_bin(3.5, 2.5, 1.5)",
    floatspan_get_bin(3.5, 2.5, 1.5), floatspan_in("[1.5, 4)"));
  check_span("floatspan_get_bin(-3.5, 2, -2)", floatspan_get_bin(-3.5, 2, -2),
    floatspan_in("[-4, -2)"));

  Interval *week = interval_in("1 week", -1);
  Interval *hour = interval_in("1 hour", -1);
  check_span("datespan_get_bin(2001-01-01, 1 week, 2001-01-01)",
    datespan_get_bin(date_in("2001-01-01"), week, date_in("2001-01-01")),
    datespan_in("[2001-01-01, 2001-01-08)"));
  check_span("tstzspan_get_bin(2001-01-01 10:20, 1 hour, 2001-01-01)",
    tstzspan_get_bin(timestamptz_in("2001-01-01 10:20:00", -1), hour,
      timestamptz_in("2001-01-01", -1)),
    tstzspan_in("[2001-01-01 10:00:00, 2001-01-01 11:00:00)"));

  /* Each bin starts at the initial value of the bin and has the size of the
   * bins */
  bool ok = true;
  for (int v = -9; v <= 9 && ok; v++)
    for (int size = 1; size <= 4 && ok; size++)
      for (int orig = -2; orig <= 2 && ok; orig++)
      {
        Span *s = intspan_get_bin(v, size, orig);
        int start = int_get_bin(v, size, orig);
        ok = s && intspan_lower(s) == start &&
          intspan_upper(s) == start + size && contains_span_int(s, v);
        free(s);
      }
  check("intspan_get_bin starts at int_get_bin and contains the value", ok);
  ok = true;
  for (double v = -4.75; v <= 4.75 && ok; v += 0.5)
  {
    Span *s = floatspan_get_bin(v, 1.5, 0.25);
    double start = float_get_bin(v, 1.5, 0.25);
    ok = s && floatspan_lower(s) == start && floatspan_upper(s) ==
      start + 1.5 && contains_span_float(s, v);
    free(s);
  }
  check("floatspan_get_bin starts at float_get_bin and contains the value",
    ok);

  /* The tiles, from 025_temporal_tile.test.sql */
  TimestampTz t = timestamptz_in("2001-01-15", -1);
  TimestampTz torig = timestamptz_in("2001-01-01", -1);
  check_box("tbox_get_value_tile(15.5, 2.5, 1.5)",
    tbox_get_value_tile(15.5, 2.5, 1.5), "TBOXFLOAT X([14, 16.5))");
  check_box("tbox_get_time_tile(2001-01-15, 1 week, 2001-01-01)",
    tbox_get_time_tile(t, week, torig),
    "TBOX T([2001-01-15, 2001-01-22))");
  check_box("tbox_get_value_time_tile(15.5, 2001-01-15, 2.5, 1 week)",
    tbox_get_value_time_tile(15.5, t, 2.5, week, 0.0, torig),
    "TBOXFLOAT XT([15, 17.5),[2001-01-15, 2001-01-22))");

  /* A tile is made of the bins of its value and its timestamp */
  TBox *tile = tbox_get_value_time_tile(7.25, t, 2.5, hour, 1.5, torig);
  Span *xspan = tile ? tbox_to_floatspan(tile) : NULL;
  Span *tspan = tile ? tbox_to_tstzspan(tile) : NULL;
  Span *xbin = floatspan_get_bin(7.25, 2.5, 1.5);
  Span *tbin = tstzspan_get_bin(t, hour, torig);
  check("tbox_get_value_time_tile is made of the bins",
    xspan && tspan && xbin && tbin && span_eq(xspan, xbin) &&
    span_eq(tspan, tbin));
  free(tile); free(xspan); free(tspan); free(xbin); free(tbin);

  /* The time tiles of a box of any number type, the float and time ones
   * from 025_temporal_tile.test.sql */
  Interval *day = interval_in("1 day", -1);
  TimestampTz sep1 = timestamptz_in("2001-09-01", -1);
  TBox *intbox = tbox_in("TBOXINT XT([1, 5],[2001-09-01, 2001-09-03])");
  TBox *bigintbox = tbox_in("TBOXBIGINT XT([1, 5],[2001-09-01, 2001-09-03])");
  TBox *floatbox = tbox_in("TBOXFLOAT XT([1, 5],[2001-09-01, 2001-09-03])");
  TBox *timebox = tbox_in("TBOX T([2001-01-15, 2001-01-25])");
  int count1, count2;
  const char *float_tiles[] = {
    "TBOXFLOAT XT([1, 5],[2001-09-01, 2001-09-02))",
    "TBOXFLOAT XT([1, 5],[2001-09-02, 2001-09-03))",
    "TBOXFLOAT XT([1, 5],[2001-09-03, 2001-09-04))"};
  TBox *boxes = tbox_time_tiles(floatbox, day, sep1, true, &count1);
  check_boxes("tbox_time_tiles(TBOXFLOAT, 1 day, 2001-09-01, true)", boxes,
    count1, float_tiles, 3);
  boxes = tbox_time_tiles(floatbox, day, sep1, false, &count1);
  check_boxes("tbox_time_tiles(TBOXFLOAT, 1 day, 2001-09-01, false)", boxes,
    count1, float_tiles, 2);
  const char *int_tiles[] = {
    "TBOXINT XT([1, 6),[2001-09-01, 2001-09-02))",
    "TBOXINT XT([1, 6),[2001-09-02, 2001-09-03))",
    "TBOXINT XT([1, 6),[2001-09-03, 2001-09-04))"};
  boxes = tbox_time_tiles(intbox, day, sep1, true, &count1);
  check_boxes("tbox_time_tiles(TBOXINT, 1 day, 2001-09-01, true)", boxes,
    count1, int_tiles, 3);
  const char *bigint_tiles[] = {
    "TBOXBIGINT XT([1, 6),[2001-09-01, 2001-09-02))",
    "TBOXBIGINT XT([1, 6),[2001-09-02, 2001-09-03))",
    "TBOXBIGINT XT([1, 6),[2001-09-03, 2001-09-04))"};
  boxes = tbox_time_tiles(bigintbox, day, sep1, true, &count1);
  check_boxes("tbox_time_tiles(TBOXBIGINT, 1 day, 2001-09-01, true)", boxes,
    count1, bigint_tiles, 3);
  const char *time_tiles[] = {
    "TBOX T([2001-01-15, 2001-01-22))", "TBOX T([2001-01-22, 2001-01-29))"};
  boxes = tbox_time_tiles(timebox, week, torig, true, &count1);
  check_boxes("tbox_time_tiles(TBOX T, 1 week, 2001-01-01, true)", boxes,
    count1, time_tiles, 2);

  /* The time tiles of a box are those of the function of its value type */
  TBox *boxes1 = tbox_time_tiles(intbox, hour, sep1, true, &count1);
  TBox *boxes2 = tintbox_time_tiles(intbox, hour, sep1, true, &count2);
  check_same_boxes("tbox_time_tiles is tintbox_time_tiles", boxes1, count1,
    boxes2, count2);
  boxes1 = tbox_time_tiles(bigintbox, hour, sep1, false, &count1);
  boxes2 = tbigintbox_time_tiles(bigintbox, hour, sep1, false, &count2);
  check_same_boxes("tbox_time_tiles is tbigintbox_time_tiles", boxes1, count1,
    boxes2, count2);
  boxes1 = tbox_time_tiles(floatbox, hour, sep1, true, &count1);
  boxes2 = tfloatbox_time_tiles(floatbox, hour, sep1, true, &count2);
  check_same_boxes("tbox_time_tiles is tfloatbox_time_tiles", boxes1, count1,
    boxes2, count2);

  /* A size that is not positive and a missing duration are refused */
  Interval *zero = interval_in("0 seconds", -1);
  check_refused("intspan_get_bin, size 0", intspan_get_bin(3, 0, 0),
    MEOS_ERR_INVALID_ARG_VALUE);
  check_refused("bigintspan_get_bin, negative size",
    bigintspan_get_bin(3, -2, 0), MEOS_ERR_INVALID_ARG_VALUE);
  check_refused("floatspan_get_bin, negative size",
    floatspan_get_bin(3.5, -2.5, 0), MEOS_ERR_INVALID_ARG_VALUE);
  check_refused("tstzspan_get_bin, duration 0",
    tstzspan_get_bin(t, zero, torig), MEOS_ERR_INVALID_ARG_VALUE);
  check_refused("datespan_get_bin, no duration",
    datespan_get_bin(date_in("2001-01-01"), NULL, date_in("2001-01-01")),
    MEOS_ERR_INVALID_ARG);
  check_refused("tbox_get_value_tile, size 0",
    tbox_get_value_tile(15.5, 0.0, 0.0), MEOS_ERR_INVALID_ARG_VALUE);
  check_refused("tbox_get_value_time_tile, negative size",
    tbox_get_value_time_tile(15.5, t, -2.5, week, 0.0, torig),
    MEOS_ERR_INVALID_ARG_VALUE);
  check_refused("tbox_get_time_tile, no duration",
    tbox_get_time_tile(t, NULL, torig), MEOS_ERR_INVALID_ARG);
  check_refused("tbox_time_tiles, no duration",
    tbox_time_tiles(floatbox, NULL, sep1, true, &count1),
    MEOS_ERR_INVALID_ARG);
  check_refused("tbox_time_tiles, duration 0",
    tbox_time_tiles(intbox, zero, sep1, true, &count1),
    MEOS_ERR_INVALID_ARG_VALUE);

  /* A bin outside the range of the type is refused */
  check_refused("intspan_get_bin, bin past the greatest integer",
    intspan_get_bin(INT_MAX, 2, 0), MEOS_ERR_VALUE_OUT_OF_RANGE);
  check_refused("bigintspan_get_bin, bin past the greatest big integer",
    bigintspan_get_bin(LLONG_MAX, 3, 0), MEOS_ERR_VALUE_OUT_OF_RANGE);
  check_refused("tstzspan_get_bin, infinite timestamp",
    tstzspan_get_bin(timestamptz_in("infinity", -1), hour, torig),
    MEOS_ERR_VALUE_OUT_OF_RANGE);

  free(week); free(hour); free(zero);
  free(day);
  free(intbox); free(bigintbox); free(floatbox); free(timebox);
  meos_finalize();

  printf("%d check(s) failed\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
