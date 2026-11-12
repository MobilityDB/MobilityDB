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
 * @brief A simple program that tests the deletion of a timestamp from a
 * temporal value
 * @details A timestamp is the timestamp set of one element, so deleting it
 * answers what deleting that set answers, with and without connecting the
 * instants before and after it. Connecting removes the instant at the
 * timestamp, if any, and joins its neighbours, the first and the last
 * instants of the result being instants the value takes, so its bounds are
 * inclusive; not connecting leaves a gap at the timestamp. The program
 * compares the two deletions on linear and step sequences and on sequence
 * sets, at an instant, at a timestamp that is no instant and at the first and
 * last instants, and reads the answers the manual states for some of them.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o delete_timestamptz_test delete_timestamptz_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>

static const char *VALUES[] = {
  "[1@2001-01-01, 2@2001-01-02, 3@2001-01-03]",
  "Interp=Step;[1@2001-01-01, 2@2001-01-02, 3@2001-01-03]",
  "{[1@2001-01-01, 2@2001-01-02, 3@2001-01-03], [4@2001-01-04, 5@2001-01-05]}",
  "Interp=Step;{[1@2001-01-01, 2@2001-01-02, 3@2001-01-03], "
    "[4@2001-01-04, 5@2001-01-05]}",
};

/* An instant, a timestamp that is no instant, the first instant and the last
 * instant of the first sequence */
static const char *TIMES[] = {"2001-01-02", "2001-01-02 12:00:00",
  "2001-01-01", "2001-01-03"};

static const struct
{
  const char *value;
  const char *t;
  bool connect;
  const char *expected;
} STATED[] = {
  {"[1@2001-01-01, 2@2001-01-02, 3@2001-01-03]", "2001-01-02", true,
   "[1@2001-01-01 00:00:00+00, 3@2001-01-03 00:00:00+00]"},
  {"[1@2001-01-01, 2@2001-01-02, 3@2001-01-03]", "2001-01-02", false,
   "{[1@2001-01-01 00:00:00+00, 2@2001-01-02 00:00:00+00), "
     "(2@2001-01-02 00:00:00+00, 3@2001-01-03 00:00:00+00]}"},
  {"[1@2001-01-01, 1@2001-01-03]", "2001-01-02", true,
   "[1@2001-01-01 00:00:00+00, 1@2001-01-03 00:00:00+00]"},
  {"[1@2001-01-01, 1@2001-01-03]", "2001-01-02", false,
   "{[1@2001-01-01 00:00:00+00, 1@2001-01-02 00:00:00+00), "
     "(1@2001-01-02 00:00:00+00, 1@2001-01-03 00:00:00+00]}"},
  {"Interp=Step;[1@2001-01-01, 2@2001-01-02, 3@2001-01-03]", "2001-01-03", true,
   "Interp=Step;[1@2001-01-01 00:00:00+00, 2@2001-01-02 00:00:00+00]"},
};

static char *
value_text(const Temporal *temp)
{
  size_t size;
  return temp ? temporal_out(temp, 6, &size) : NULL;
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  int failures = 0, checks = 0;
  int nvalues = (int) (sizeof(VALUES) / sizeof(VALUES[0]));
  int ntimes = (int) (sizeof(TIMES) / sizeof(TIMES[0]));
  for (int i = 0; i < nvalues; i++)
  {
    Temporal *temp = tfloat_in(VALUES[i]);
    for (int j = 0; j < ntimes; j++)
    {
      TimestampTz t = timestamptz_in(TIMES[j], -1);
      Set *s = timestamptz_to_set(t);
      for (int c = 0; c < 2; c++)
      {
        /* Without connecting, the last instant of a sequence is the case of
         * meos/test/minus_last_instant_test.c */
        if (! c && j == ntimes - 1)
          continue;
        checks++;
        Temporal *d1 = temporal_delete_timestamptz(temp, t, c);
        Temporal *d2 = temporal_delete_tstzset(temp, s, c);
        char *r1 = value_text(d1), *r2 = value_text(d2);
        if ((r1 == NULL) != (r2 == NULL) || (r1 && strcmp(r1, r2) != 0))
        {
          printf("FAILED: delete %s from %s, connect %d: %s, as a set %s\n",
            TIMES[j], VALUES[i], c, r1 ? r1 : "NULL", r2 ? r2 : "NULL");
          failures++;
        }
        free(r1); free(r2); free(d1); free(d2);
      }
      free(s);
    }
    free(temp);
  }

  int nstated = (int) (sizeof(STATED) / sizeof(STATED[0]));
  for (int i = 0; i < nstated; i++)
  {
    checks++;
    Temporal *temp = tfloat_in(STATED[i].value);
    TimestampTz t = timestamptz_in(STATED[i].t, -1);
    Temporal *d = temporal_delete_timestamptz(temp, t, STATED[i].connect);
    char *r = value_text(d);
    if (! r || strcmp(r, STATED[i].expected) != 0)
    {
      printf("FAILED: delete %s from %s, connect %d: %s, expected %s\n",
        STATED[i].t, STATED[i].value, STATED[i].connect, r ? r : "NULL",
        STATED[i].expected);
      failures++;
    }
    free(r); free(d); free(temp);
  }

  meos_finalize();
  printf("delete_timestamptz_test: %d checks, %d failed\n", checks, failures);
  return failures ? 1 : 0;
}
