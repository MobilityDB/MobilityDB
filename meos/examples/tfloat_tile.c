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
 * float according to value and/or time buckets.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tfloat_tile tfloat_tile.c -L/usr/local/lib -lmeos
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

  Temporal *tfloat = tfloat_in("[1@2020-03-01, 10@2020-03-10]");
  Interval *interv = pg_interval_in("2 days", -1);
  double vorigin = 0.0;
  TimestampTz torigin = pg_timestamptz_in("2020-03-01", -1);

  bool valuesplit = true; /* Set this parameter to enable/disable value split */
  bool timesplit = true; /* Set this parameter to enable/disable time split */

  double *value_buckets = NULL;
  TimestampTz *time_buckets = NULL;
  Temporal **result;
  int count;
  if (valuesplit)
    result = tfloat_value_time_split(tfloat, 2.0, timesplit ? interv : NULL,
      vorigin, torigin, &value_buckets, &time_buckets, &count);
  else
    result = temporal_time_split(tfloat, interv, torigin, &time_buckets,
      &count);

  /* Print the input value to split */
  char *tfloat_str = tfloat_out(tfloat, 3);
  printf("------------------\n");
  printf("| Value to split |\n");
  printf("------------------\n\n");
  printf("%s\n\n", tfloat_str);
  free(tfloat_str);

  /* Output the resulting fragments */
  printf("----------\n");
  printf("Fragments:\n");
  printf("----------\n\n");
  int i;
  for (i = 0; i < count; i++)
  {
    char *time_str = timesplit ? pg_timestamptz_out(time_buckets[i]) : "";
    char *temp_str = tfloat_out(result[i], 3);
    if (valuesplit)
      sprintf(output_buffer, "%f, %s%s%s\n", value_buckets[i],
        time_str, timesplit ? ", " : "", temp_str);
    else
      sprintf(output_buffer, "%s, %s\n", time_str, temp_str);
    printf("%s", output_buffer);
    if (timesplit) free(time_str);
    free(temp_str);
  }

  /* Print information about the result */
  printf("\nNumber of fragments: %d\n", count);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
