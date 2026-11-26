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
 * @brief A program that tests the occurrences of a temporal value at its
 * origins
 * @details A template built from a reference origin occurs at every origin of
 * a set, shifted by the elapsed duration from the reference origin. The
 * program checks one occurrence against the value written by hand, and the
 * origins whose occurrence meets a period in two ways: on cases whose bounds
 * touch, where the inclusion of each bound decides, and against the
 * definition over a grid of periods, every origin's occurrence being built
 * and its period tested for overlap with the period, for templates with each
 * inclusion of their bounds and for an instant.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <pg_timestamp.h>

static int failures = 0, cases = 0;

static TimestampTz
ts(const char *str)
{
  return timestamptz_in(str, -1);
}

/* The origins whose occurrence meets the period, by the definition */
static Set *
brute(const Temporal *temp, TimestampTz ref, const Set *origins,
  const Span *period)
{
  int n = set_num_values(origins), count = 0;
  TimestampTz *values = malloc(sizeof(TimestampTz) * n);
  for (int i = 0; i < n; i++)
  {
    TimestampTz o;
    tstzset_value_n(origins, i + 1, &o);
    Temporal *occ = temporal_occurrence(temp, ref, o);
    Span *p = temporal_to_tstzspan(occ);
    if (overlaps_span_span(p, period))
      values[count++] = o;
    free(occ); free(p);
  }
  Set *result = count ? tstzset_make(values, count) : NULL;
  free(values);
  return result;
}

static void
check_origins(const char *what, const Temporal *temp, TimestampTz ref,
  const Set *origins, const Span *period, const char *expected)
{
  cases++;
  Set *res = temporal_occurrence_origins(temp, ref, origins, period);
  size_t size;
  char *out = res ? set_out(res, 15, &size) : NULL;
  bool ok = expected ? (out && strcmp(out, expected) == 0) : ! res;
  if (! ok)
  {
    printf("  %s: %s\n  expected %s\n", what, out ? out : "NULL",
      expected ? expected : "NULL");
    failures++;
  }
  free(res); free(out);
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* A trip running from 30 to 90 minutes after the origin of its service
   * day */
  TimestampTz ref = ts("2026-01-14 23:00:00+00");
  Temporal *trip = tint_in(
    "[1@2026-01-14 23:30:00+00, 2@2026-01-15 00:30:00+00]");
  Temporal *trip_open = tint_in(
    "(1@2026-01-14 23:30:00+00, 1@2026-01-15 00:30:00+00)");
  TimestampTz o[3] = { ref, ts("2026-01-15 23:00:00+00"),
    ts("2026-01-16 23:00:00+00") };
  Set *origins = tstzset_make(o, 3);

  /* One occurrence, at an origin one hour earlier in the day in UTC: each
   * instant keeps its distance from the origin */
  cases++;
  Temporal *occ = temporal_occurrence(trip, ref,
    ts("2026-03-28 22:00:00+00"));
  Temporal *expected = tint_in(
    "[1@2026-03-28 22:30:00+00, 2@2026-03-28 23:30:00+00]");
  if (! temporal_eq(occ, expected))
  {
    size_t size;
    char *out = temporal_out(occ, 15, &size);
    printf("  temporal_occurrence: %s\n", out);
    free(out);
    failures++;
  }
  free(occ); free(expected);

  /* Cases whose bounds touch */
  Span *p;
  p = tstzspan_make(ts("2026-01-15 00:00:00+00"),
    ts("2026-01-15 00:10:00+00"), true, true);
  check_origins("inside the first occurrence", trip, ref, origins, p,
    "{\"2026-01-14 23:00:00+00\"}");
  free(p);
  p = tstzspan_make(ts("2026-01-15 23:00:00+00"),
    ts("2026-01-15 23:30:00+00"), true, true);
  check_origins("ending at the start of the second", trip, ref, origins, p,
    "{\"2026-01-15 23:00:00+00\"}");
  check_origins("ending at the open start of the second", trip_open, ref,
    origins, p, NULL);
  free(p);
  p = tstzspan_make(ts("2026-01-15 23:00:00+00"),
    ts("2026-01-15 23:30:00+00"), true, false);
  check_origins("ending before the start of the second", trip, ref, origins,
    p, NULL);
  free(p);
  p = tstzspan_make(ts("2026-01-15 00:30:00+00"),
    ts("2026-01-15 01:00:00+00"), true, true);
  check_origins("starting at the end of the first", trip, ref, origins, p,
    "{\"2026-01-14 23:00:00+00\"}");
  check_origins("starting at the open end of the first", trip_open, ref,
    origins, p, NULL);
  free(p);
  p = tstzspan_make(ts("2026-01-01 00:00:00+00"),
    ts("2026-02-01 00:00:00+00"), true, true);
  check_origins("covering every occurrence", trip, ref, origins, p,
    "{\"2026-01-14 23:00:00+00\", \"2026-01-15 23:00:00+00\", "
    "\"2026-01-16 23:00:00+00\"}");
  free(p);
  p = tstzspan_make(ts("2025-12-01 00:00:00+00"),
    ts("2025-12-02 00:00:00+00"), true, true);
  check_origins("before every occurrence", trip, ref, origins, p, NULL);
  free(p);

  /* The definition over a grid of periods, starting every 15 minutes over
   * 52 hours from one hour before the reference origin, lasting from 0 to 26
   * hours by steps of 45 minutes, with each inclusion of their bounds */
  Temporal *templates[3] = { trip, trip_open,
    tint_in("1@2026-01-14 23:30:00+00") };
  TimestampTz start = ts("2026-01-14 22:00:00+00");
  Interval *quarter = interval_in("15 minutes", -1);
  int grid = 0, wrong = 0;
  for (int t = 0; t < 3; t++)
  {
    TimestampTz lower = start;
    for (int i = 0; i < 4 * 52; i++)
    {
      TimestampTz upper = lower;
      for (int j = 0; j <= 4 * 26; j++)
      {
        if (j % 3 == 0)
          for (int inc = 0; inc < 4; inc++)
          {
            bool lower_inc = inc & 1, upper_inc = (inc & 2) != 0;
            if (j == 0 && ! (lower_inc && upper_inc))
              continue;
            p = tstzspan_make(lower, upper, lower_inc, upper_inc);
            Set *res = temporal_occurrence_origins(templates[t], ref,
              origins, p);
            Set *def = brute(templates[t], ref, origins, p);
            grid++;
            if ((res == NULL) != (def == NULL) ||
                (res && ! set_eq(res, def)))
              wrong++;
            free(p); free(res); free(def);
          }
        upper = add_timestamptz_interval(upper, quarter);
      }
      lower = add_timestamptz_interval(lower, quarter);
    }
  }
  cases++;
  if (wrong)
  {
    printf("  %d of %d periods differ from the definition\n", wrong, grid);
    failures++;
  }

  printf("%d of %d case(s) failed, %d periods against the definition\n",
    failures, cases, grid);
  free(trip); free(trip_open); free(templates[2]); free(origins);
  free(quarter);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
