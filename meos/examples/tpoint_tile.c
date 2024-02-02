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
 * @brief A simple program that applies multidimensional tiling to a temporal
 * point according to value and/or time buckets. 
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tpoint_tile tpoint_tile.c -L/usr/local/lib -lmeos
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

  /* Initialize MEOS */
  meos_initialize(NULL, NULL);

  Temporal *tpoint = tgeompoint_in("[Point(1 1)@2020-03-01, Point(10 10)@2020-03-10]");
  Interval *interv = pg_interval_in("2 days", -1);
  GSERIALIZED *sorigin = pgis_geometry_in("Point(0 0 0)", -1);
  TimestampTz torigin = pg_timestamptz_in("2020-03-01", -1);

  bool spacesplit = true; /* Set this parameter to enable/disable space split */
  bool timesplit = true; /* Set this parameter to enable/disable time split */
  bool bitmatrix = true; /* Set this parameter to enable/disable the bit matrix */

  GSERIALIZED **space_buckets = NULL;
  TimestampTz *time_buckets = NULL;
  Temporal **result;
  int count;
  if (spacesplit)
    result = tpoint_space_time_split(tpoint, 2.0, 2.0, 2.0,
      timesplit ? interv : NULL, sorigin, torigin, bitmatrix,
      &space_buckets, &time_buckets, &count);
  else
    result = temporal_time_split(tpoint, interv, torigin, &time_buckets,
      &count);

  /* Print the input value to split */
  char *tpoint_str = tpoint_as_ewkt(tpoint, 3);
  printf("------------------\n");
  printf("| Value to split |\n");
  printf("------------------\n\n");
  printf("%s\n\n", tpoint_str);
  free(tpoint_str);

  /* Output the resulting fragments */
  printf("-------------\n");
  printf("| Fragments |\n");
  printf("-------------\n\n");
  for (int i = 0; i < count; i++)
  {
    char *space_str = spacesplit ?
      geo_as_ewkt(space_buckets[i], 3) : "";
    char *time_str = timesplit ? pg_timestamptz_out(time_buckets[i]) : "";
    char *temp_str = tpoint_as_ewkt(result[i], 3);
    sprintf(output_buffer, "%s%s%s%s%s\n", space_str, spacesplit ? ", " : "",
      time_str, timesplit ? ", " : "", temp_str);
    printf("%s", output_buffer);
    if (spacesplit) free(space_str);
    if (timesplit) free(time_str);
    free(temp_str);
  }

  /* Print information about the result */
  printf("\nNumber of fragments: %d\n", count);
  if (bitmatrix)
    printf("Using a bitmatrix for the fragmentation\n");

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
