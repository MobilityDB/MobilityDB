/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief A simple program that generates a given number of tgeompoint instants,
 * assembles the instants into a sequence at the end of the generation process,
 * and outputs the number of instants and the distance travelled.
 *
 * The instants are generated so they are not redundant, that is, all input
 * instants will appear in the final sequence.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tbox_tile tbox_tile.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <meos.h>

/* Maximum length in characters of a line in the output data */
#define MAX_LINE_LENGTH 1024

/* Main program */
int main(void)
{
  char output_buffer[MAX_LINE_LENGTH];

  /* Set this parameter to choose between a temporal integer or float box */
  bool intspan = false;
  /* Set this parameter to enable/disable value split */
  bool valuesplit = true;
  /* Set this parameter to enable/disable time split */
  bool timesplit = false;

  /* Initialize MEOS */
  meos_initialize(NULL, NULL);

  /* Initialize values for tiling */
  TBox *box = intspan ?
    tbox_in("TBOXINT XT([1,10],[2020-03-01, 2020-03-10])") :
    tbox_in("TBOXFLOAT XT([1,10],[2020-03-01, 2020-03-10])");
  Interval *interv = pg_interval_in("5 days", -1);
  TimestampTz torigin = pg_timestamptz_in("2020-03-01", -1);

  /* Perform tiling */
  TBox *boxes;
  Span *spans;
  int count;
  if (valuesplit)
    boxes = intspan ?
      tintbox_tile_list(box, 5, timesplit ? interv : NULL, 1, torigin, &count) :
      tfloatbox_tile_list(box, 5, timesplit ? interv : NULL, 1, torigin, &count);
  else
    spans = tstzspan_bucket_list(&box->period, interv, torigin, &count);

  /* Print the input value to split */
  char *box_str = tbox_out(box, 3);
  printf("------------------\n");
  printf("| Value to split |\n");
  printf("------------------\n\n");
  printf("%s\n\n", box_str);
  free(box_str);

  /* Output the resulting tiles */
  printf("--------\n");
  printf("| Tiles |\n");
  printf("--------\n\n");
  for (int i = 0; i < count; i++)
  {
    char *tile_str = valuesplit ?
      tbox_out(&boxes[i], 3) : floatspan_out(&spans[i], 3);
    sprintf(output_buffer, "%d: %s\n", i + 1, tile_str);
    printf("%s", output_buffer);
    free(tile_str);
  }

  /* Print information about the result */
  printf("\nNumber of tiles: %d\n", count);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
