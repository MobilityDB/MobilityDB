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
 * @brief A program that tests that the comparison of temporal and
 * spatiotemporal boxes is a total order
 * @details The B-tree index, the sorting and the sort key of a binding over
 * the temporal and spatiotemporal boxes rely on #tbox_cmp and #stbox_cmp
 * being a total order consistent with #tbox_eq and #stbox_eq. The program
 * compares every pair of a set of boxes of different dimensions, value types,
 * SRIDs and geodetic flags, and verifies that the comparison is antisymmetric
 * and transitive, and that it answers 0 exactly when the equality answers
 * true.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o box_order_test box_order_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

static const char *STBOXES[] = {
  "STBOX XT(((0,0),(0,0)),[2001-01-02,2001-01-02])",
  "STBOX X((1,0),(1,0))",
  "STBOX XT(((2,0),(2,0)),[2001-01-01,2001-01-01])",
  "STBOX T([2001-01-03,2001-01-03])",
  "STBOX T([2001-01-01,2001-01-01])",
  "STBOX ZT(((0,0,0),(0,0,0)),[2001-01-01,2001-01-01])",
  "STBOX Z((1,0,5),(1,0,5))",
  "STBOX Z((0,0,9),(0,0,9))",
  "STBOX X((0,0),(0,0))",
  "GEODSTBOX X((0,0),(0,0))",
  "GEODSTBOX XT(((1,0),(1,0)),[2001-01-01,2001-01-01])",
  "GEODSTBOX T([2001-01-02,2001-01-02])",
  "SRID=4326;STBOX X((0,0),(0,0))",
  "SRID=4326;STBOX XT(((5,0),(5,0)),[2001-01-01,2001-01-01])",
  "SRID=3812;STBOX X((-1,0),(-1,0))",
};
#define NUM_STBOXES ((int) (sizeof(STBOXES) / sizeof(STBOXES[0])))

static const char *TBOXES[] = {
  "TBOXFLOAT XT([0,0],[2001-01-02,2001-01-02])",
  "TBOXFLOAT X([1,1])",
  "TBOXFLOAT XT([2,2],[2001-01-01,2001-01-01])",
  "TBOX T([2001-01-03,2001-01-03])",
  "TBOX T([2001-01-01,2001-01-01])",
  "TBOXFLOAT X([0,0])",
  "TBOXINT X([1,1])",
  "TBOXINT XT([0,0],[2001-01-03,2001-01-03])",
  "TBOXINT XT([2,2],[2001-01-01,2001-01-01])",
  "TBOXBIGINT X([1,1])",
};
#define NUM_TBOXES ((int) (sizeof(TBOXES) / sizeof(TBOXES[0])))

#define MAX_BOXES 16

static int failures = 0;

/* Verify that a matrix of comparisons is a total order consistent with the
 * equality of the boxes */
static void
check_order(const char *type, const char **boxes, int n,
  int cmp[MAX_BOXES][MAX_BOXES], bool eq[MAX_BOXES][MAX_BOXES])
{
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      if (cmp[i][j] != -1 && cmp[i][j] != 0 && cmp[i][j] != 1)
      {
        printf("FAIL %s comparison of %s and %s answers %d\n", type,
          boxes[i], boxes[j], cmp[i][j]);
        failures++;
      }
      if (cmp[i][j] != -cmp[j][i])
      {
        printf("FAIL %s comparison is not antisymmetric on %s and %s\n", type,
          boxes[i], boxes[j]);
        failures++;
      }
      if ((cmp[i][j] == 0) != eq[i][j])
      {
        printf("FAIL %s comparison answers %d and equality %s on %s and %s\n",
          type, cmp[i][j], eq[i][j] ? "true" : "false", boxes[i], boxes[j]);
        failures++;
      }
      for (int k = 0; k < n; k++)
        if (cmp[i][j] < 0 && cmp[j][k] < 0 && cmp[i][k] >= 0)
        {
          printf("FAIL %s comparison is not transitive: %s < %s < %s\n",
            type, boxes[i], boxes[j], boxes[k]);
          failures++;
        }
    }
  return;
}

/* Main program */
int main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  int cmp[MAX_BOXES][MAX_BOXES];
  bool eq[MAX_BOXES][MAX_BOXES];

  STBox *stboxes[NUM_STBOXES];
  for (int i = 0; i < NUM_STBOXES; i++)
    stboxes[i] = stbox_in(STBOXES[i]);
  for (int i = 0; i < NUM_STBOXES; i++)
    for (int j = 0; j < NUM_STBOXES; j++)
    {
      cmp[i][j] = stbox_cmp(stboxes[i], stboxes[j]);
      eq[i][j] = stbox_eq(stboxes[i], stboxes[j]);
    }
  check_order("STBox", STBOXES, NUM_STBOXES, cmp, eq);

  TBox *tboxes[NUM_TBOXES];
  for (int i = 0; i < NUM_TBOXES; i++)
    tboxes[i] = tbox_in(TBOXES[i]);
  for (int i = 0; i < NUM_TBOXES; i++)
    for (int j = 0; j < NUM_TBOXES; j++)
    {
      cmp[i][j] = tbox_cmp(tboxes[i], tboxes[j]);
      eq[i][j] = tbox_eq(tboxes[i], tboxes[j]);
    }
  check_order("TBox", TBOXES, NUM_TBOXES, cmp, eq);

  for (int i = 0; i < NUM_STBOXES; i++)
    free(stboxes[i]);
  for (int i = 0; i < NUM_TBOXES; i++)
    free(tboxes[i]);

  if (failures == 0)
    printf("Box order test: all tests passed\n");
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
