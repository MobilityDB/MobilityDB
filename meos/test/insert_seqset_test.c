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
 * @brief A simple program that tests the insertion of a temporal sequence
 * set into another one
 * @details With connect, `temporal_insert` connects each composing sequence
 * of one value to the composing sequences of the other value before and after
 * it when the bounds they meet on are inclusive, as it does for two
 * sequences; without connect, it answers the merge of the two values. The
 * program reads the answer for one sequence in each value, for sequences of
 * the two values alternating, and for two sequences of the second value in
 * one gap of the first, for a sequence of the second value on the exclusive
 * lower bound of a sequence of the first, and that values overlapping on time
 * are refused.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o insert_seqset_test insert_seqset_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>

static const struct
{
  const char *ss1;
  const char *ss2;
  bool connect;
  const char *expected;
} INSERTS[] = {
  /* One sequence in each value */
  {"{[1@2001-01-01, 2@2001-01-02]}", "{[5@2001-01-04, 6@2001-01-05]}", true,
   "{[1@2001-01-01 00:00:00+00, 2@2001-01-02 00:00:00+00, "
     "5@2001-01-04 00:00:00+00, 6@2001-01-05 00:00:00+00]}"},
  {"{[1@2001-01-01, 2@2001-01-02]}", "{[5@2001-01-04, 6@2001-01-05]}", false,
   "{[1@2001-01-01 00:00:00+00, 2@2001-01-02 00:00:00+00], "
     "[5@2001-01-04 00:00:00+00, 6@2001-01-05 00:00:00+00]}"},
  /* The sequences of the two values alternate */
  {"{[1@2001-01-01, 2@2001-01-02], [3@2001-01-06, 4@2001-01-07]}",
   "{[5@2001-01-03, 6@2001-01-04], [7@2001-01-08, 8@2001-01-09]}", true,
   "{[1@2001-01-01 00:00:00+00, 2@2001-01-02 00:00:00+00, "
     "5@2001-01-03 00:00:00+00, 6@2001-01-04 00:00:00+00, "
     "3@2001-01-06 00:00:00+00, 4@2001-01-07 00:00:00+00, "
     "7@2001-01-08 00:00:00+00, 8@2001-01-09 00:00:00+00]}"},
  {"{[1@2001-01-01, 2@2001-01-02], [3@2001-01-06, 4@2001-01-07]}",
   "{[5@2001-01-03, 6@2001-01-04], [7@2001-01-08, 8@2001-01-09]}", false,
   "{[1@2001-01-01 00:00:00+00, 2@2001-01-02 00:00:00+00], "
     "[5@2001-01-03 00:00:00+00, 6@2001-01-04 00:00:00+00], "
     "[3@2001-01-06 00:00:00+00, 4@2001-01-07 00:00:00+00], "
     "[7@2001-01-08 00:00:00+00, 8@2001-01-09 00:00:00+00]}"},
  /* Two sequences of the second value in one gap of the first: each is
   * connected to the sequence of the first value next to it, not to the
   * other */
  {"{[1@2001-01-01, 4@2001-01-02], [9@2001-01-09, 8@2001-01-10]}",
   "{[2@2001-01-03, 5@2001-01-04], [3@2001-01-05, 6@2001-01-06]}", true,
   "{[1@2001-01-01 00:00:00+00, 4@2001-01-02 00:00:00+00, "
     "2@2001-01-03 00:00:00+00, 5@2001-01-04 00:00:00+00], "
     "[3@2001-01-05 00:00:00+00, 6@2001-01-06 00:00:00+00, "
     "9@2001-01-09 00:00:00+00, 8@2001-01-10 00:00:00+00]}"},
  /* A sequence of the second value on the timestamp of the exclusive lower
   * bound of a sequence of the first starts before it */
  {"{(1@2001-01-01, 2@2001-01-02], [1@2001-01-03, 1@2001-01-05]}",
   "{[2@2001-01-01]}", true,
   "{[2@2001-01-01 00:00:00+00], "
     "(1@2001-01-01 00:00:00+00, 2@2001-01-02 00:00:00+00], "
     "[1@2001-01-03 00:00:00+00, 1@2001-01-05 00:00:00+00]}"},
  {"{(1@2001-01-01, 2@2001-01-02], [1@2001-01-03, 1@2001-01-05]}",
   "{[2@2001-01-01]}", false,
   "{[2@2001-01-01 00:00:00+00], "
     "(1@2001-01-01 00:00:00+00, 2@2001-01-02 00:00:00+00], "
     "[1@2001-01-03 00:00:00+00, 1@2001-01-05 00:00:00+00]}"},
  /* Values overlapping on time are refused */
  {"{[1@2001-01-01, 2@2001-01-05], [3@2001-01-08, 3@2001-01-09]}",
   "{[5@2001-01-04, 6@2001-01-06]}", true, NULL},
};

static int
check(const char *what, Temporal *result, const char *expected)
{
  size_t size;
  char *out = result ? temporal_out(result, 6, &size) : NULL;
  int failed = expected ?
    (! out || strcmp(out, expected) != 0) :
    (out != NULL || meos_errno() != MEOS_ERR_INVALID_ARG_VALUE);
  if (failed)
    printf("FAILED: %s: %s, expected %s\n", what, out ? out : "NULL",
      expected ? expected : "NULL");
  else
    printf("OK: %s: %s\n", what, out ? out : "refused");
  free(out); free(result);
  meos_errno_reset();
  return failed;
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  /* An invalid argument answers NULL and sets meos_errno instead of exiting */
  meos_initialize_noexit_error_handler();

  int failures = 0, count = 0;
  for (size_t i = 0; i < sizeof(INSERTS) / sizeof(INSERTS[0]); i++)
  {
    Temporal *ss1 = tfloat_in(INSERTS[i].ss1);
    Temporal *ss2 = tfloat_in(INSERTS[i].ss2);
    char what[64];
    snprintf(what, sizeof(what), "insert %zu, connect %s", i + 1,
      INSERTS[i].connect ? "true" : "false");
    failures += check(what, temporal_insert(ss1, ss2, INSERTS[i].connect),
      INSERTS[i].expected);
    count++;
    free(ss1); free(ss2);
  }

  meos_finalize();
  printf("insert_seqset_test: %d cases, %d failed\n", count, failures);
  return failures ? 1 : 0;
}
