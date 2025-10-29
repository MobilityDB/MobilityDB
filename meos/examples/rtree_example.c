/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief A simple program to use an RTree index for searching MEOS bounding
 * boxes, i.e., `floatspan`, `tstzspan`, `tbox`, and `stbox`
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o rtree_example rtree_example.c -L/usr/lib -lproj -L/usr/local/lib -lmeos
 * @endcode
 */

/* C */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>

/* Maximum length in characters of a bounding box string to parse */
#define MAX_LEN_BBOX 100
/* Maximum number of bounding boxes */
#define NO_BBOX 10000
/* Type of the bounding boxes */
#define FLOATSPAN 1
#define TSTZSPAN 2
#define TBOX 3
/* Uncomment one of the lines below to select the bounding box type */
// #define BBOX_TYPE FLOATSPAN
// #define BBOX_TYPE TSTZSPAN
// #define BBOX_TYPE TBOX
#define BBOX_TYPE STBOX

bool index_result[NO_BBOX];
bool actual_result[NO_BBOX];
/* STBox is the largest bounding box in MEOS */
STBox *boxes;

/* Print a bounding box according to its type */
void
print_bbox(const void *box, char *prefix)
{
  char *box_str;
#if BBOX_TYPE == FLOATSPAN
  box_str = floatspan_out((Span *) box, 3);
#elif BBOX_TYPE == TSTZSPAN
  box_str = tstzspan_out((Span *) box);
#elif BBOX_TYPE == TBOX
  box_str = tbox_out((TBox *) box, 3);
#else /* BBOX_TYPE == STBOX */
  box_str = stbox_out((STBox *) box, 3);
#endif /* BBOX_TYPE */
  printf("\n%s\n%s\n", prefix, box_str);
  free(box_str);
  return;
}

/* Get a random integer number */
int
get_random_number(int min, int max)
{
  return rand() % (max - min + 1) + min;
}

/* Main program */
int main()
{
  meos_initialize();
  /* STBox is the largest bounding box in MEOS */
  boxes = malloc(sizeof(STBox) * NO_BBOX);
  /* This can be srand(time(NULL)) for random */
  srand(1);
  clock_t t;
  double time_taken;
  char box_str[MAX_LEN_BBOX];

  RTree *rtree;
#if BBOX_TYPE == FLOATSPAN
  rtree = rtree_create_floatspan();
#elif BBOX_TYPE == TSTZSPAN
  rtree = rtree_create_tstzspan();
#elif BBOX_TYPE == TBOX
  rtree = rtree_create_tbox();
#else /* BBOX_TYPE == STBOX */
  rtree = rtree_create_stbox();
#endif /* BBOX_TYPE */

  for (int i = 0; i < NO_BBOX; ++i)
  {
#if BBOX_TYPE == FLOATSPAN || BBOX_TYPE == TBOX || BBOX_TYPE == STBOX
    int xmin = get_random_number(1, 1000);
    int xmax = xmin + get_random_number(1, 10);
#endif
#if BBOX_TYPE == STBOX
    int ymin = get_random_number(1, 1000);
    int ymax = ymin + get_random_number(1, 10);
#endif
#if BBOX_TYPE == TSTZSPAN || BBOX_TYPE == TBOX || BBOX_TYPE == STBOX
    int time_min = get_random_number(1, 29);
    int time_max = time_min + get_random_number(1, 29);
#endif
#if BBOX_TYPE == FLOATSPAN
    snprintf(box_str, MAX_LEN_BBOX - 1,
      "[%d, %d]",
      xmin, xmax);
    Span *box = floatspan_in(box_str);
    memcpy(&boxes[i], box, sizeof(Span));
#elif BBOX_TYPE == TSTZSPAN
    snprintf(box_str, MAX_LEN_BBOX - 1,
      "[2023-01-01 01:00:%02d+00, 2023-01-01 01:00:%02d+00]",
      time_min, time_max);
    Span *box = tstzspan_in(box_str);
    memcpy(&boxes[i], box, sizeof(Span));
#elif BBOX_TYPE == TBOX
    snprintf(box_str, MAX_LEN_BBOX - 1,
      "TBOX XT([%d, %d],[2023-01-01 01:00:%02d+00, 2023-01-01 01:00:%02d+00])",
      xmin, xmax, time_min, time_max);
    TBox *box = tbox_in(box_str);
    memcpy(&boxes[i], box, sizeof(TBox));
#else /* BBOX_TYPE == STBOX */
    snprintf(box_str, MAX_LEN_BBOX - 1,
      "SRID=25832;STBOX XT(((%d %d),(%d %d)),[2023-01-01 01:00:%02d+00, 2023-01-01 01:00:%02d+00])",
      xmin, xmax, ymin, ymax, time_min, time_max);
    STBox *box = stbox_in(box_str);
    memcpy(&boxes[i], box, sizeof(STBox));
#endif /* BBOX_TYPE */
    rtree_insert(rtree, &boxes[i], i);
    free(box);
  }

  int count = 0;
  int real_count = 0;
#if BBOX_TYPE == FLOATSPAN
  snprintf(box_str, MAX_LEN_BBOX - 1,
    "[0, 100]");
  Span *box = floatspan_in(box_str);
#elif BBOX_TYPE == TSTZSPAN
  snprintf(box_str, MAX_LEN_BBOX - 1,
    "[2023-01-01 01:00:00+00, 2023-01-01 01:00:60+00]");
  Span *box = tstzspan_in(box_str);
#elif BBOX_TYPE == TBOX
  snprintf(box_str, MAX_LEN_BBOX - 1,
    "TBOX XT([0,100],[2023-01-01 01:00:00+00, 2023-01-01 01:00:60+00])");
  TBox *box = tbox_in(box_str);
#else /* BBOX_TYPE == STBOX */
  snprintf(box_str, MAX_LEN_BBOX - 1,
    "SRID=25832;STBOX XT(((0 0),(100 100)),[2023-01-01 01:00:00+00, 2023-01-01 01:00:60+00])");
  STBox *box = stbox_in(box_str);
#endif /* BBOX_TYPE */

  t = clock();
  int *ids = rtree_search(rtree, box, &count);
  t = clock() - t;
  time_taken = ((double) t) / CLOCKS_PER_SEC; // in seconds 
  printf("Index lookup took %f seconds to execute \n", time_taken);

  t = clock();
  for (int i = 0; i < NO_BBOX; ++i)
  {
#if BBOX_TYPE == FLOATSPAN || BBOX_TYPE == TSTZSPAN
    if (overlaps_span_span((Span *) &boxes[i], (Span *) box))
#elif BBOX_TYPE == TBOX
    if (overlaps_tbox_tbox((TBox *) &boxes[i], (TBox *) box))
#else /* BBOX_TYPE == STBOX */
    if (overlaps_stbox_stbox((STBox *) &boxes[i], (STBox *) box))
#endif /* BBOX_TYPE */
    {
      real_count++;
      actual_result[i] = true;
    }
  }
  t = clock() - t;
  time_taken = ((double) t) / CLOCKS_PER_SEC; // in seconds 
  printf("Brute foce took %f seconds to execute \n", time_taken);

  for (int i = 0; i < count; ++i)
  {
    index_result[ids[i]] = true;
  }

  for (int i = 0; i < NO_BBOX; ++i)
  {
    /* Print if there is an error, if everything is ok, nothing
     * should be printed. */
    if (index_result[i] != actual_result[i])
    {
      printf("\n========\n%d) actual_result: %d index: %d\n", 
        i, actual_result[i], index_result[i]);
      print_bbox(&boxes[i], "-------------");
    }
  }

#if BBOX_TYPE == FLOATSPAN
  printf("\nBOUNDING BOX = FLOATSPAN\n");
#elif BBOX_TYPE == TSTZSPAN
  printf("\nBOUNDING BOX = TSTZSPAN\n");
#elif BBOX_TYPE == TBOX
  printf("\nBOUNDING BOX = TBOX\n");
#else /* BBOX_TYPE == STBOX */
  printf("\nBOUNDING BOX = STBOX\n");
#endif /* BBOX_TYPE */

  printf("EXPECTED HITS = %d \n", real_count);
  printf("INDEX HITS    = %d\n", count);

  /* Free memory */
  rtree_free(rtree);
  free(box); free(boxes); free(ids);

  meos_finalize();
  return EXIT_SUCCESS;
}
