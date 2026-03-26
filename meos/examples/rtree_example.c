/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief A simple program that demonstrates the RTree index for searching
 * MEOS bounding boxes: floatspan, tstzspan, tbox, and stbox.
 *
 * The program runs all four bounding box types, inserting random boxes into
 * the index and verifying search results against a brute-force scan.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o rtree_example rtree_example.c -L/usr/local/lib -lmeos -lproj
 * @endcode
 */

/* C */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>

/* Maximum number of bounding boxes */
#define NO_BBOX 10000
/* Maximum length in characters of a bounding box string */
#define MAX_LEN_BBOX 128

/* Return a random integer in [min, max] */
static int
random_int(int min, int max)
{
  return rand() % (max - min + 1) + min;
}

/**
 * @brief Generic test harness: compare R-tree search results against a
 * brute-force overlap scan
 * @param[in] name Label for the test (e.g., "FLOATSPAN")
 * @param[in] rtree The R-tree to search
 * @param[in] query The query bounding box
 * @param[in] boxes Array of stored bounding boxes
 * @param[in] bboxsize Size of each bounding box in bytes
 * @param[in] overlaps_fn Function that checks if two boxes overlap
 */
static void
verify_search(const char *name, const RTree *rtree, const void *query,
  const char *boxes, size_t bboxsize,
  bool (*overlaps_fn)(const void *, const void *))
{
  /* Index search */
  clock_t t = clock();
  int index_count;
  int *ids = rtree_search(rtree, RTREE_OVERLAPS, query, &index_count);
  double index_time = (double)(clock() - t) / CLOCKS_PER_SEC;

  /* Brute-force search */
  t = clock();
  int brute_count = 0;
  bool *actual = calloc(NO_BBOX, sizeof(bool));
  for (int i = 0; i < NO_BBOX; i++)
  {
    if (overlaps_fn(boxes + i * bboxsize, query))
    {
      brute_count++;
      actual[i] = true;
    }
  }
  double brute_time = (double)(clock() - t) / CLOCKS_PER_SEC;

  /* Compare results */
  bool *indexed = calloc(NO_BBOX, sizeof(bool));
  for (int i = 0; i < index_count; i++)
    indexed[ids[i]] = true;

  int errors = 0;
  for (int i = 0; i < NO_BBOX; i++)
  {
    if (indexed[i] != actual[i])
      errors++;
  }

  printf("  %-10s  index: %.6fs (%d hits)  brute: %.6fs (%d hits)  %s\n",
    name, index_time, index_count, brute_time, brute_count,
    errors == 0 ? "OK" : "MISMATCH");

  free(actual);
  free(indexed);
  free(ids);
}

/*****************************************************************************/

static bool
overlaps_span_wrapper(const void *a, const void *b)
{
  return overlaps_span_span((Span *) a, (Span *) b);
}

static bool
overlaps_tbox_wrapper(const void *a, const void *b)
{
  return overlaps_tbox_tbox((TBox *) a, (TBox *) b);
}

static bool
overlaps_stbox_wrapper(const void *a, const void *b)
{
  return overlaps_stbox_stbox((STBox *) a, (STBox *) b);
}

/*****************************************************************************/

static void
test_floatspan(void)
{
  char str[MAX_LEN_BBOX];
  Span *boxes = malloc(sizeof(Span) * NO_BBOX);
  RTree *rtree = rtree_create_floatspan();

  for (int i = 0; i < NO_BBOX; i++)
  {
    int lo = random_int(1, 1000);
    int hi = lo + random_int(1, 10);
    snprintf(str, sizeof(str), "[%d, %d]", lo, hi);
    Span *s = floatspan_in(str);
    boxes[i] = *s;
    rtree_insert(rtree, &boxes[i], i);
    free(s);
  }

  Span *query = floatspan_in("[0, 100]");
  verify_search("FLOATSPAN", rtree, query, (char *) boxes, sizeof(Span),
    overlaps_span_wrapper);

  free(query);
  rtree_free(rtree);
  free(boxes);
}

static void
test_tstzspan(void)
{
  char str[MAX_LEN_BBOX];
  Span *boxes = malloc(sizeof(Span) * NO_BBOX);
  RTree *rtree = rtree_create_tstzspan();

  for (int i = 0; i < NO_BBOX; i++)
  {
    int tmin = random_int(1, 29);
    int tmax = tmin + random_int(1, 29);
    snprintf(str, sizeof(str),
      "[2023-01-01 01:00:%02d+00, 2023-01-01 01:00:%02d+00]", tmin, tmax);
    Span *s = tstzspan_in(str);
    boxes[i] = *s;
    rtree_insert(rtree, &boxes[i], i);
    free(s);
  }

  Span *query = tstzspan_in(
    "[2023-01-01 01:00:00+00, 2023-01-01 01:00:60+00]");
  verify_search("TSTZSPAN", rtree, query, (char *) boxes, sizeof(Span),
    overlaps_span_wrapper);

  free(query);
  rtree_free(rtree);
  free(boxes);
}

static void
test_tbox(void)
{
  char str[MAX_LEN_BBOX];
  TBox *boxes = malloc(sizeof(TBox) * NO_BBOX);
  RTree *rtree = rtree_create_tbox();

  for (int i = 0; i < NO_BBOX; i++)
  {
    int xmin = random_int(1, 1000);
    int xmax = xmin + random_int(1, 10);
    int tmin = random_int(1, 29);
    int tmax = tmin + random_int(1, 29);
    snprintf(str, sizeof(str),
      "TBOX XT([%d, %d],[2023-01-01 01:00:%02d+00, 2023-01-01 01:00:%02d+00])",
      xmin, xmax, tmin, tmax);
    TBox *b = tbox_in(str);
    boxes[i] = *b;
    rtree_insert(rtree, &boxes[i], i);
    free(b);
  }

  TBox *query = tbox_in(
    "TBOX XT([0,100],[2023-01-01 01:00:00+00, 2023-01-01 01:00:60+00])");
  verify_search("TBOX", rtree, query, (char *) boxes, sizeof(TBox),
    overlaps_tbox_wrapper);

  free(query);
  rtree_free(rtree);
  free(boxes);
}

static void
test_stbox(void)
{
  char str[MAX_LEN_BBOX];
  STBox *boxes = malloc(sizeof(STBox) * NO_BBOX);
  RTree *rtree = rtree_create_stbox();

  for (int i = 0; i < NO_BBOX; i++)
  {
    int xmin = random_int(1, 1000);
    int xmax = xmin + random_int(1, 10);
    int ymin = random_int(1, 1000);
    int ymax = ymin + random_int(1, 10);
    int tmin = random_int(1, 29);
    int tmax = tmin + random_int(1, 29);
    snprintf(str, sizeof(str),
      "SRID=25832;STBOX XT(((%d %d),(%d %d)),"
      "[2023-01-01 01:00:%02d+00, 2023-01-01 01:00:%02d+00])",
      xmin, xmax, ymin, ymax, tmin, tmax);
    STBox *b = stbox_in(str);
    boxes[i] = *b;
    rtree_insert(rtree, &boxes[i], i);
    free(b);
  }

  STBox *query = stbox_in(
    "SRID=25832;STBOX XT(((0 0),(100 100)),"
    "[2023-01-01 01:00:00+00, 2023-01-01 01:00:60+00])");
  verify_search("STBOX", rtree, query, (char *) boxes, sizeof(STBox),
    overlaps_stbox_wrapper);

  free(query);
  rtree_free(rtree);
  free(boxes);
}

/*****************************************************************************/

int main(void)
{
  meos_initialize();
  /* Use fixed seed for reproducibility */
  srand(1);

  printf("RTree index test (%d boxes per type)\n", NO_BBOX);
  test_floatspan();
  test_tstzspan();
  test_tbox();
  test_stbox();

  meos_finalize();
  return EXIT_SUCCESS;
}
