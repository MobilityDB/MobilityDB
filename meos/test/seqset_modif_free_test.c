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
 * @brief A simple program that tests the deletion of a span from a temporal
 * sequence set of several sequences
 * @details The deletion builds the sequences of its answer in an array,
 * normalizes it and makes the sequence set from the normalized array. The
 * program reads the answers of `temporal_delete_tstzspan` on a sequence set of
 * three sequences, connecting and not, so that a run under valgrind reads what
 * the deletion allocates. Deleting the middle sequence leaves the gaps around
 * it, since its neighbours do not touch the deleted span.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o seqset_modif_free_test seqset_modif_free_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>

static const char *SEQSET =
  "{[1@2001-01-01, 2@2001-01-02], [3@2001-01-04, 4@2001-01-05], "
  "[5@2001-01-07, 6@2001-01-08]}";

static const struct
{
  const char *span;
  bool connect;
  const char *expected;
} DELETES[] = {
  {"[2001-01-04, 2001-01-05]", true,
   "{[1@2001-01-01 00:00:00+00, 2@2001-01-02 00:00:00+00], "
     "[5@2001-01-07 00:00:00+00, 6@2001-01-08 00:00:00+00]}"},
  {"[2001-01-04, 2001-01-05]", false,
   "{[1@2001-01-01 00:00:00+00, 2@2001-01-02 00:00:00+00], "
     "[5@2001-01-07 00:00:00+00, 6@2001-01-08 00:00:00+00]}"},
  {"[2001-01-03, 2001-01-06]", true,
   "{[1@2001-01-01 00:00:00+00, 2@2001-01-02 00:00:00+00], "
     "[5@2001-01-07 00:00:00+00, 6@2001-01-08 00:00:00+00]}"},
};

static int
check(const char *what, Temporal *result, const char *expected)
{
  size_t size;
  char *out = result ? temporal_out(result, 6, &size) : NULL;
  int failed = (! out || strcmp(out, expected) != 0);
  if (failed)
    printf("FAILED: %s: %s, expected %s\n", what, out ? out : "NULL", expected);
  else
    printf("OK: %s: %s\n", what, out);
  free(out); free(result);
  return failed;
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  int failures = 0, count = 0;
  Temporal *ss = tfloat_in(SEQSET);
  for (size_t i = 0; i < sizeof(DELETES) / sizeof(DELETES[0]); i++)
  {
    Span *s = tstzspan_in(DELETES[i].span);
    char what[128];
    snprintf(what, sizeof(what), "delete %s, connect %s", DELETES[i].span,
      DELETES[i].connect ? "true" : "false");
    failures += check(what, temporal_delete_tstzspan(ss, s,
      DELETES[i].connect), DELETES[i].expected);
    count++;
    free(s);
  }
  free(ss);

  meos_finalize();
  printf("seqset_modif_free_test: %d cases, %d failed\n", count, failures);
  return failures ? 1 : 0;
}
