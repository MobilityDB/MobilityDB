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
 * @brief A program that tests that the sort key of a span and of a span set
 * orders them as #span_cmp and #spanset_cmp do.
 *
 * Over every pair of a corpus of spans of each span type (negative values,
 * zero of both signs, large magnitudes, infinities and every bound flag) and
 * of span sets built from them (one, two and three spans, so that a set whose
 * spans prefix another's is compared with it), the sign of a byte comparison
 * of the two keys must be the sign of the comparison of the two values, and
 * two values the comparison holds equal must have identical keys. The program
 * also verifies that #span_sortkey and #spanset_sortkey report a null value
 * and a null size.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o sortkey_test sortkey_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>

#define MAXS 4096

static Span *spans[MAXS];
static int nspans;
static long failures;

static int
order_sign(int x)
{
  return (x > 0) - (x < 0);
}

/* The sign of an unsigned byte comparison, a shorter prefix sorting first */
static int
sortkey_order(const uint8_t *a, size_t na, const uint8_t *b, size_t nb)
{
  size_t n = na < nb ? na : nb;
  int c = memcmp(a, b, n);
  if (c)
    return order_sign(c);
  return (na > nb) - (na < nb);
}

static void
corpus_add(Span *s)
{
  if (s && nspans < MAXS)
    spans[nspans++] = s;
  meos_errno_reset();
}

/* Compare every pair of the corpus by key and by value, then free it */
static void
corpus_check(const char *name)
{
  long pairs = 0, bad = 0;
  for (int i = 0; i < nspans; i++)
    for (int j = 0; j < nspans; j++)
    {
      size_t na, nb;
      uint8_t *ka = span_sortkey(spans[i], &na);
      uint8_t *kb = span_sortkey(spans[j], &nb);
      int c = order_sign(span_cmp(spans[i], spans[j]));
      if (sortkey_order(ka, na, kb, nb) != c)
        bad++;
      free(ka); free(kb);
      pairs++;
    }
  printf("%s: %ld span pairs, %ld ordered or equated otherwise than span_cmp\n",
    name, pairs, bad);
  failures += bad;

  SpanSet *sets[3 * MAXS];
  int nsets = 0;
  for (int i = 0; i < nspans && nsets < 3 * MAXS - 3; i += 3)
  {
    sets[nsets++] = span_to_spanset(spans[i]);
    SpanSet *u2 = union_span_span(spans[i], spans[(i * 7 + 5) % nspans]);
    if (u2)
    {
      sets[nsets++] = u2;
      SpanSet *u3 = union_spanset_span(u2, spans[(i * 13 + 11) % nspans]);
      if (u3)
        sets[nsets++] = u3;
    }
  }
  pairs = bad = 0;
  for (int i = 0; i < nsets; i++)
    for (int j = 0; j < nsets; j++)
    {
      size_t na, nb;
      uint8_t *ka = spanset_sortkey(sets[i], &na);
      uint8_t *kb = spanset_sortkey(sets[j], &nb);
      int c = order_sign(spanset_cmp(sets[i], sets[j]));
      if (sortkey_order(ka, na, kb, nb) != c)
        bad++;
      free(ka); free(kb);
      pairs++;
    }
  printf("%s: %ld span set pairs, %ld ordered or equated otherwise than "
    "spanset_cmp\n", name, pairs, bad);
  failures += bad;

  for (int i = 0; i < nsets; i++)
    free(sets[i]);
  for (int i = 0; i < nspans; i++)
    free(spans[i]);
  nspans = 0;
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting, so that a span the constructors refuse (an
   * empty integer span) is skipped */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  const int ints[] = {-1000000000, -65536, -256, -5, -1, 0, 1, 2, 255, 256,
    257, 65536, 1000000000};
  const int ni = sizeof(ints) / sizeof(ints[0]);
  const double dbl[] = {-INFINITY, -1e300, -3.5, -1.0, -0.25, -0.0, 0.0, 0.25,
    1.0, 9.0, 1e300, INFINITY};
  const int nd = sizeof(dbl) / sizeof(dbl[0]);
  const int64 scale = 1000003;

  for (int a = 0; a < ni; a++)
    for (int b = a; b < ni; b++)
      for (int f = 0; f < 4; f++)
        corpus_add(intspan_make(ints[a], ints[b] + (a == b), f & 1, f & 2));
  corpus_check("intspan");

  for (int a = 0; a < ni; a++)
    for (int b = a; b < ni; b++)
      for (int f = 0; f < 4; f++)
      {
        int64 lower = ints[a], upper = ints[b];
        corpus_add(bigintspan_make(lower * scale, upper * scale + (a == b), f & 1,
          f & 2));
      }
  corpus_check("bigintspan");

  for (int a = 0; a < nd; a++)
    for (int b = a; b < nd; b++)
      for (int f = 0; f < 4; f++)
      {
        bool lower_inc = f & 1, upper_inc = f & 2;
        if (dbl[a] == dbl[b])
          lower_inc = upper_inc = true;
        corpus_add(floatspan_make(dbl[a], dbl[b], lower_inc, upper_inc));
      }
  corpus_check("floatspan");

  for (int a = 0; a < ni; a++)
    for (int b = a; b < ni; b++)
      for (int f = 0; f < 4; f++)
        corpus_add(datespan_make(ints[a] / 1000, ints[b] / 1000 + 1, f & 1, f & 2));
  corpus_check("datespan");

  for (int a = 0; a < ni; a++)
    for (int b = a; b < ni; b++)
      for (int f = 0; f < 4; f++)
      {
        bool lower_inc = f & 1, upper_inc = f & 2;
        if (a == b)
          lower_inc = upper_inc = true;
        TimestampTz lower = ints[a], upper = ints[b];
        corpus_add(tstzspan_make(lower * scale, upper * scale, lower_inc, upper_inc));
      }
  corpus_check("tstzspan");

  /* A null value or a null size is reported rather than dereferenced */
  size_t size;
  Span *s = intspan_make(1, 2, true, true);
  SpanSet *ss = span_to_spanset(s);
  meos_errno_reset();
  uint8_t *key = span_sortkey(NULL, &size);
  printf("span_sortkey(NULL, size): %s, errno %d\n", key ? "a key" : "NULL",
    meos_errno());
  assert(key == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  key = span_sortkey(s, NULL);
  printf("span_sortkey(span, NULL): %s, errno %d\n", key ? "a key" : "NULL",
    meos_errno());
  assert(key == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  key = spanset_sortkey(NULL, &size);
  printf("spanset_sortkey(NULL, size): %s, errno %d\n", key ? "a key" : "NULL",
    meos_errno());
  assert(key == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  key = spanset_sortkey(ss, &size);
  printf("spanset_sortkey(span set, size): %s of %zu bytes, errno %d\n",
    key ? "a key" : "NULL", size, meos_errno());
  assert(key != NULL && meos_errno() == 0);
  free(key); free(ss); free(s);

  printf("%ld pair(s) ordered or equated otherwise than their comparison\n",
    failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
