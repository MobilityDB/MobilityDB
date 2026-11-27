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
 * @brief A program that tests the flags of the temporal boxes of the
 * instants and segments of a temporal number
 * @details A temporal box of a temporal number has a value and a time
 * dimension, so its flags hold the X and T bits and no other: an index reads
 * the other bits of the flags of the boxes it stores. Every box the splitters
 * of a temporal number return is tested, over an instant, a sequence of each
 * interpolation and a sequence set, so that running the program under
 * valgrind reads the flags of every box they build.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>

/* The bits MEOS_FLAG_X and MEOS_FLAG_T of the flags of a box */
#define TBOX_XT_FLAGS 0x0050

static int
check_boxes(const char *name, TBox *boxes, int count)
{
  assert(boxes != NULL);
  for (int i = 0; i < count; i++)
    assert(boxes[i].flags == TBOX_XT_FLAGS);
  printf("%s: %d boxes, flags X and T\n", name, count);
  free(boxes);
  return count;
}

int
main(void)
{
  meos_initialize();
  const char *values[] = {
    "1.5@2001-01-01",
    "{1@2001-01-01, 3@2001-01-02, 2@2001-01-03}",
    "Interp=Step;[1@2001-01-01, 3@2001-01-02, 2@2001-01-03]",
    "[1@2001-01-01, 3@2001-01-02, 2@2001-01-03, 5@2001-01-04]",
    "{[1@2001-01-01, 3@2001-01-02], [2@2001-01-03, 5@2001-01-04]}",
  };
  int total = 0;
  for (size_t v = 0; v < sizeof(values) / sizeof(values[0]); v++)
  {
    Temporal *temp = tfloat_in(values[v]);
    assert(temp != NULL);
    printf("%s\n", values[v]);
    int count;
    TBox *boxes = tnumber_tboxes(temp, &count);
    total += check_boxes("  tboxes", boxes, count);
    for (int n = 1; n <= 3; n++)
    {
      boxes = tnumber_split_n_tboxes(temp, n, &count);
      total += check_boxes("  splitNTboxes", boxes, count);
      boxes = tnumber_split_each_n_tboxes(temp, n, &count);
      total += check_boxes("  splitEachNTboxes", boxes, count);
    }
    free(temp);
  }
  printf("%d boxes\n", total);
  meos_finalize();
  return EXIT_SUCCESS;
}
