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
 * @brief A simple program to use an RTree index for searching
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o rtree_example rtree_example.c -L/usr/lib -lproj -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <meos.h>
#include <meos_internal.h>

/* Maximum length in characters of a trip  */
#define MAX_LEN_TRIP 100
/* Maximum number of stboxes */
#define NO_STBOX 10000

bool index_result[NO_STBOX];
bool actual[NO_STBOX];
STBox * stboxes;

void print_stbox(const STBox *stbox, char *prefix)
{
  printf("\n%s\nXmin = %f, Xmax = %f\nymin: %f ymax: %f\ndate_min: %s date_max: %s\n", 
    prefix, stbox -> xmin, stbox -> xmax, stbox -> ymin, stbox -> ymax,
    timestamp_out(stbox -> period.lower),
    timestamp_out(stbox -> period.upper));
}

int get_random_number(int min, int max)
{
  return rand() % (max - min + 1) + min;
}

int main()
{
  stboxes = malloc(sizeof(STBox) * NO_STBOX);
  // This can be srand(time(NULL)) for random.
  srand(1);
  clock_t t;
  double time_taken;
  RTree * rtree;
  rtree = rtree_create_stbox();
  char trip_str[MAX_LEN_TRIP];

  for (int i = 0; i < NO_STBOX; ++i)
  {
    int xmin = get_random_number(1, 1000);
    int xmax = xmin + get_random_number(1, 10);
    int ymin = get_random_number(1, 1000);
    int ymax = ymin + get_random_number(1, 10);
    int time_min = get_random_number(1, 29);
    int time_max = time_min + get_random_number(1, 29);
    snprintf(trip_str, MAX_LEN_TRIP - 1,
      "SRID=25832;[POINT(%d %d)@2023-01-01 00:00:%02d+00,POINT(%d %d)@2023-01-01 00:00:%02d+00]", 
      xmin, ymin, time_min, xmax, ymax, time_max);
    Temporal * trip = tgeompoint_in(trip_str);
    tspatial_set_stbox(trip, & stboxes[i]);
    rtree_insert(rtree, & stboxes[i], i);
  }

  int count = 0;
  int real_count = 0;
  snprintf(trip_str, MAX_LEN_TRIP - 1,
    "SRID=25832;[POINT(0 0)@2023-01-01 00:00:00+00,POINT(100 100)@2023-01-01 00:00:60+00]");
  STBox * stbox = malloc(sizeof(STBox));
  Temporal * trip = tgeompoint_in(trip_str);
  tspatial_set_stbox(trip, stbox);

  t = clock();
  int * ids = rtree_search(rtree, stbox, & count);
  t = clock() - t;
  time_taken = ((double) t) / CLOCKS_PER_SEC; // in seconds 
  printf("Index lookup took %f seconds to execute \n", time_taken);

  t = clock();
  for (int i = 0; i < NO_STBOX; ++i)
  {
    if (overlaps_stbox_stbox( & stboxes[i], stbox))
    {
      real_count++;
      actual[i] = true;
    }
  }
  t = clock() - t;
  time_taken = ((double) t) / CLOCKS_PER_SEC; // in seconds 
  printf("Brute foce took %f seconds to execute \n", time_taken);

  for (int i = 0; i < count; ++i)
  {
    index_result[ids[i]] = true;
  }

  for (int i = 0; i < NO_STBOX; ++i)
  {
    /* Print if there is an error, if everything is ok, nothing
     * should be printed. */
    if (index_result[i] != actual[i])
    {
      printf("\n========\n%d) actual: %d index: %d\n", 
        i, actual[i], index_result[i]);
      print_stbox( & stboxes[i], "-------------");
    }
  }

  printf("\nEXPECTED HITS = %d \n", real_count);
  printf("\nINDEX HITS    = %d\n", count);
  rtree_free(rtree);
}
