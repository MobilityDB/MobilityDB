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
 * @brief A program that tests the in-memory RTree index for the numeric span
 * bounding box types, i.e., the indexes built with rtree_create_intspan and
 * rtree_create_floatspan, against an exact brute-force oracle.
 *
 * For each span type a set of random spans is inserted, an overlap search is
 * run for a query span, and the candidate id set is compared with the exact
 * set of spans that overlap the query. Two properties are asserted per type:
 *  (i)  no false negatives: every span overlapping the query is a candidate;
 *  (ii) no false positives: every candidate overlaps the query.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o rtree_span_test rtree_span_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>

/* Number of spans inserted into the index */
#define NUM_SPANS 2048

static int failures = 0;

static void
check(const char *name, bool ok)
{
  printf("  %-58s %s\n", name, ok ? "OK" : "FAIL");
  if (! ok)
    failures++;
}

/* Return a pseudo-random integer in [min, max] */
static int
random_int(int min, int max)
{
  return min + rand() % (max - min + 1);
}

/*****************************************************************************
 * Integer span index
 *****************************************************************************/

static void
test_intspan_rtree(void)
{
  RTree *rtree = rtree_create_intspan();
  Span **spans = malloc(NUM_SPANS * sizeof(Span *));
  for (int i = 0; i < NUM_SPANS; i++)
  {
    int lo = random_int(-10000, 10000);
    spans[i] = intspan_make(lo, lo + random_int(1, 100), true, false);
    rtree_insert(rtree, spans[i], i);
  }
  int qlo = random_int(-10000, 10000);
  Span *query = intspan_make(qlo, qlo + 500, true, false);

  MeosArray *result = meos_array_create(sizeof(int64));
  int count = rtree_search(rtree, RTREE_OVERLAPS, query, result);

  bool *in_index = calloc(NUM_SPANS, sizeof(bool));
  for (int i = 0; i < count; i++)
    in_index[*(int64 *) meos_array_get(result, i)] = true;

  printf("Integer span RTree (%d random spans):\n", NUM_SPANS);

  int missed = 0, extra = 0;
  for (int i = 0; i < NUM_SPANS; i++)
  {
    bool truth = overlaps_span_span(query, spans[i]);
    if (truth && ! in_index[i])
      missed++;
    if (! truth && in_index[i])
      extra++;
  }
  check("(i) no false negatives vs brute-force overlap", missed == 0);
  check("(ii) no false positives vs brute-force overlap", extra == 0);

  for (int i = 0; i < NUM_SPANS; i++)
    free(spans[i]);
  free(spans); free(query); free(in_index);
  meos_array_destroy(result);
  rtree_free(rtree);
}

/*****************************************************************************
 * Float span index
 *****************************************************************************/

static void
test_floatspan_rtree(void)
{
  RTree *rtree = rtree_create_floatspan();
  Span **spans = malloc(NUM_SPANS * sizeof(Span *));
  for (int i = 0; i < NUM_SPANS; i++)
  {
    double lo = random_int(-10000, 10000) / 10.0;
    spans[i] = floatspan_make(lo, lo + random_int(1, 100) / 10.0, true, false);
    rtree_insert(rtree, spans[i], i);
  }
  double qlo = random_int(-10000, 10000) / 10.0;
  Span *query = floatspan_make(qlo, qlo + 50.0, true, false);

  MeosArray *result = meos_array_create(sizeof(int64));
  int count = rtree_search(rtree, RTREE_OVERLAPS, query, result);

  bool *in_index = calloc(NUM_SPANS, sizeof(bool));
  for (int i = 0; i < count; i++)
    in_index[*(int64 *) meos_array_get(result, i)] = true;

  printf("Float span RTree (%d random spans):\n", NUM_SPANS);

  int missed = 0, extra = 0;
  for (int i = 0; i < NUM_SPANS; i++)
  {
    bool truth = overlaps_span_span(query, spans[i]);
    if (truth && ! in_index[i])
      missed++;
    if (! truth && in_index[i])
      extra++;
  }
  check("(i) no false negatives vs brute-force overlap", missed == 0);
  check("(ii) no false positives vs brute-force overlap", extra == 0);

  for (int i = 0; i < NUM_SPANS; i++)
    free(spans[i]);
  free(spans); free(query); free(in_index);
  meos_array_destroy(result);
  rtree_free(rtree);
}

/**
 * @brief Test that an identifier wider than 32 bits survives the index
 * @details The identifier a caller attaches to a box is an int64. A carrier
 * narrower than that anywhere on the insert path changes the value rather than
 * losing the entry, so the box is still found and only the identifier reported
 * for it is wrong.
 */
static void
test_id_width(void)
{
  RTree *rtree = rtree_create_intspan();
  /* Ids above 2^31, so a 32-bit carrier would report different numbers */
  const int64 ids[] = {4000000000LL, 4000000001LL, INT64_C(1) << 40};
  const int nids = (int) (sizeof(ids) / sizeof(ids[0]));
  Span **spans = malloc(nids * sizeof(Span *));
  for (int i = 0; i < nids; i++)
  {
    spans[i] = intspan_make(i * 10, i * 10 + 5, true, false);
    rtree_insert(rtree, spans[i], ids[i]);
  }

  Span *query = intspan_make(-100, 1000, true, false);
  MeosArray *result = meos_array_create(sizeof(int64));
  int count = rtree_search(rtree, RTREE_OVERLAPS, query, result);

  printf("Identifier width (%d spans, ids above 2^31):\n", nids);
  if (count != nids)
  {
    printf("  the query returned %d of %d entries\n", count, nids);
    failures++;
  }
  else
  {
    for (int i = 0; i < count; i++)
    {
      int64 got = *(int64 *) meos_array_get(result, i);
      bool found = false;
      for (int k = 0; k < nids; k++)
        if (ids[k] == got) found = true;
      if (! found)
      {
        printf("  the index reported id %lld, which was never inserted\n",
          (long long) got);
        failures++;
        break;
      }
    }
    if (! failures)
      printf("  all %d identifiers came back unchanged\n", nids);
  }

  for (int i = 0; i < nids; i++)
    free(spans[i]);
  free(spans); free(query);
  meos_array_destroy(result);
  rtree_free(rtree);
}

int
main(void)
{
  meos_initialize();
  srand(1);

  test_intspan_rtree();
  test_floatspan_rtree();
  test_id_width();

  meos_finalize();

  if (failures == 0)
    printf("\nAll span RTree tests passed.\n");
  else
    printf("\n%d span RTree test(s) FAILED.\n", failures);
  return failures == 0 ? 0 : 1;
}
