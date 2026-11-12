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
 * @brief A simple program that tests the restriction of a continuous
 * temporal sequence to the complement of its last instant
 * @details The last instant of a sequence whose upper bound is inclusive is
 * followed by no instant, so the restriction answers the sequence up to that
 * instant, exclusive, and no sequence after it. A step sequence keeps the
 * value it takes before the instant up to the instant, as a step sequence
 * whose upper bound is exclusive states. The program reads the answers of
 * `temporal_minus_timestamptz` on a linear and a step sequence and on a
 * sequence set, whose first sequence ends at the timestamp.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o minus_last_instant_test minus_last_instant_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>

static const struct
{
  const char *value;
  const char *t;
  const char *expected;
} CASES[] = {
  {"[1@2001-01-01, 2@2001-01-02]", "2001-01-02",
   "{[1@2001-01-01 00:00:00+00, 2@2001-01-02 00:00:00+00)}"},
  {"Interp=Step;[1@2001-01-01, 2@2001-01-02]", "2001-01-02",
   "Interp=Step;{[1@2001-01-01 00:00:00+00, 1@2001-01-02 00:00:00+00)}"},
  {"Interp=Step;[1@2001-01-01, 2@2001-01-02, 3@2001-01-03]", "2001-01-03",
   "Interp=Step;{[1@2001-01-01 00:00:00+00, 2@2001-01-02 00:00:00+00, "
     "2@2001-01-03 00:00:00+00)}"},
  {"Interp=Step;{[1@2001-01-01, 2@2001-01-02], [3@2001-01-04, 4@2001-01-05]}",
   "2001-01-02",
   "Interp=Step;{[1@2001-01-01 00:00:00+00, 1@2001-01-02 00:00:00+00), "
     "[3@2001-01-04 00:00:00+00, 4@2001-01-05 00:00:00+00]}"},
};

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  int failures = 0;
  int count = (int) (sizeof(CASES) / sizeof(CASES[0]));
  for (int i = 0; i < count; i++)
  {
    Temporal *temp = tfloat_in(CASES[i].value);
    TimestampTz t = timestamptz_in(CASES[i].t, -1);
    Temporal *result = temporal_minus_timestamptz(temp, t);
    size_t size;
    char *out = result ? temporal_out(result, 6, &size) : NULL;
    if (! out || strcmp(out, CASES[i].expected) != 0)
    {
      printf("FAILED: minus %s from %s: %s, expected %s\n", CASES[i].t,
        CASES[i].value, out ? out : "NULL", CASES[i].expected);
      failures++;
    }
    else
      printf("OK: minus %s from %s: %s\n", CASES[i].t, CASES[i].value, out);
    free(out); free(result); free(temp);
  }

  meos_finalize();
  printf("minus_last_instant_test: %d cases, %d failed\n", count, failures);
  return failures ? 1 : 0;
}
