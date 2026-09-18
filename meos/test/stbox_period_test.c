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
 * @brief A program that tests that the period of a spatiotemporal box keeps
 * the inclusivity of its bounds.
 *
 * The spatial extent of a spatiotemporal box is closed, since it is the extent
 * of geometries, but its period is a timestamptz span whose bounds are
 * inclusive or exclusive. The program builds every period on three instants
 * with every combination of bound inclusivity, as a box without and with a
 * spatial extent, and verifies for every pair that the box overlaps, contains
 * and equals the other exactly when its period does, and that an R-tree of the
 * boxes answers every overlap query with the boxes whose periods overlap.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o stbox_period_test stbox_period_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

/* Number of periods: three pairs of instants, four inclusivity combinations */
#define NUM_PERIODS 12

static int failures = 0;

static void
check(const char *what, int i, int j, bool box, bool span)
{
  if (box != span)
  {
    printf("FAIL %s of periods %d and %d: box answers %s, period answers %s\n",
      what, i, j, box ? "true" : "false", span ? "true" : "false");
    failures++;
  }
}

/* Main program */
int main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  TimestampTz t[3];
  t[0] = timestamptz_in("2001-01-01", -1);
  t[1] = timestamptz_in("2001-01-02", -1);
  t[2] = timestamptz_in("2001-01-03", -1);
  static const int bounds[3][2] = {{0, 1}, {0, 2}, {1, 2}};

  Span *periods[NUM_PERIODS];
  STBox *tboxes[NUM_PERIODS], *xtboxes[NUM_PERIODS];
  GSERIALIZED *point = geom_in("POINT(1 1)", -1);
  int n = 0;
  for (int b = 0; b < 3; b++)
    for (int inc = 0; inc < 4; inc++)
    {
      periods[n] = tstzspan_make(t[bounds[b][0]], t[bounds[b][1]],
        (inc & 1) != 0, (inc & 2) != 0);
      tboxes[n] = tstzspan_to_stbox(periods[n]);
      xtboxes[n] = geo_tstzspan_to_stbox(point, periods[n]);
      n++;
    }

  RTree *rtree = rtree_create_stbox();
  for (int i = 0; i < NUM_PERIODS; i++)
    rtree_insert(rtree, tboxes[i], i);
  MeosArray *result = index_result_create();

  for (int i = 0; i < NUM_PERIODS; i++)
  {
    int expected = 0;
    for (int j = 0; j < NUM_PERIODS; j++)
    {
      bool ov = overlaps_span_span(periods[i], periods[j]);
      bool co = contains_span_span(periods[i], periods[j]);
      bool eq = span_eq(periods[i], periods[j]);
      expected += ov;
      for (int x = 0; x < 2; x++)
      {
        STBox *b1 = x ? xtboxes[i] : tboxes[i];
        STBox *b2 = x ? xtboxes[j] : tboxes[j];
        const char *kind = x ? "XT" : "T";
        char what[32];
        snprintf(what, sizeof(what), "overlaps %s", kind);
        check(what, i, j, overlaps_stbox_stbox(b1, b2), ov);
        snprintf(what, sizeof(what), "contains %s", kind);
        check(what, i, j, contains_stbox_stbox(b1, b2), co);
        snprintf(what, sizeof(what), "same %s", kind);
        check(what, i, j, same_stbox_stbox(b1, b2), eq);
      }
    }
    int count = rtree_search(rtree, INDEX_OVERLAPS, tboxes[i], result);
    check("R-tree overlaps count", i, -1, count == expected, true);
  }

  for (int i = 0; i < NUM_PERIODS; i++)
  {
    free(periods[i]); free(tboxes[i]); free(xtboxes[i]);
  }
  free(point);
  meos_array_destroy(result);
  rtree_free(rtree);

  if (failures == 0)
    printf("STBox period test: all tests passed\n");
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
