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
 * @brief A program that tests the occurrences of a temporal value that meet
 * a period
 * @details For a trip of temporal points over a week of daily origins and for
 * an instant, and over a grid of periods with each inclusion of their bounds,
 * the program checks that the occurrences returned are, in order, the
 * occurrences at the origins #temporal_occurrence_origins finds, each equal
 * to the occurrence #temporal_occurrence builds at its origin and with a
 * period overlapping the period, and that a period meeting no occurrence
 * answers NULL with a count of zero.
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <pg_timestamp.h>

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  TimestampTz ref = timestamptz_in("2026-01-14 23:00:00+00", -1);
  Temporal *templates[2] = {
    tgeompoint_in("[Point(0 0)@2026-01-14 23:30:00+00, "
      "Point(1 1)@2026-01-15 00:00:00+00, "
      "Point(2 0)@2026-01-15 00:30:00+00]"),
    tint_in("1@2026-01-14 23:30:00+00") };
  Interval *day = interval_in("1 day", -1);
  Interval *step = interval_in("40 minutes", -1);
  TimestampTz o[7];
  o[0] = ref;
  for (int i = 1; i < 7; i++)
    o[i] = add_timestamptz_interval(o[i - 1], day);
  Set *origins = tstzset_make(o, 7);

  int periods = 0, wrong = 0, empty = 0;
  for (int t = 0; t < 2; t++)
  {
    TimestampTz lower = timestamptz_in("2026-01-14 20:00:00+00", -1);
    for (int i = 0; i < 300; i++)
    {
      TimestampTz upper = lower;
      for (int j = 0; j < 60; j += 7)
      {
        for (int inc = 0; inc < 4; inc++)
        {
          bool lower_inc = inc & 1, upper_inc = (inc & 2) != 0;
          if (lower == upper && ! (lower_inc && upper_inc))
            continue;
          periods++;
          Span *p = tstzspan_make(lower, upper, lower_inc, upper_inc);
          Set *found = temporal_occurrence_origins(templates[t], ref,
            origins, p);
          TimestampTz *occ_origins = NULL;
          int count;
          Temporal **occs = temporal_occurrences(templates[t], ref, origins,
            p, &occ_origins, &count);
          int nfound = found ? set_num_values(found) : 0;
          if (count != nfound || (! occs) != (nfound == 0))
            wrong++;
          else if (! occs)
            empty++;
          else
            for (int k = 0; k < count; k++)
            {
              TimestampTz f;
              tstzset_value_n(found, k + 1, &f);
              Temporal *occ = temporal_occurrence(templates[t], ref, f);
              Span *op = temporal_to_tstzspan(occs[k]);
              if (occ_origins[k] != f || ! temporal_eq(occs[k], occ) ||
                  ! overlaps_span_span(op, p))
                wrong++;
              free(occ); free(op); free(occs[k]);
            }
          free(p); free(found); free(occs); free(occ_origins);
        }
        for (int k = 0; k < 7; k++)
          upper = add_timestamptz_interval(upper, step);
      }
      lower = add_timestamptz_interval(lower, step);
    }
  }

  printf("%d periods, %d meeting no occurrence, %d wrong answer(s)\n",
    periods, empty, wrong);
  free(templates[0]); free(templates[1]); free(origins); free(day);
  free(step);
  meos_finalize();
  return (wrong || empty == 0 || empty == periods) ?
    EXIT_FAILURE : EXIT_SUCCESS;
}
