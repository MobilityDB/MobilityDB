/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief A simple program that a CSV file containing temporal points and
 * applies a function to the temporal points.
 *
 * The corresponding SQL query would be
 * @code
 * SELECT k, numInstants(douglasPeuckerSimplify(temp, 5, true))
   FROM tbl_tgeompoint;
 * @endcode
 *
 * The program can be tested with several functions such as simplification
 * with Douglas-Peucker ... reduction such as `tsample`, `tprecision`, ...
 * 
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tbl_tpoint tbl_tpoint.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LENGTH_HEADER 1024
/* Maximum length in characters of a temporal point in the input data as
 * computed by the following query on the corresponding table
 * SELECT MAX(length(temp::text)) FROM tbl_tgeompoint;
 * -- 3770
 */
#define MAX_LENGTH_TGEO 7501

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* You may substitute the full file path in the first argument of fopen */
  FILE *file = fopen("data/tbl_tgeompoint.csv", "r");

  if (! file)
  {
    printf("Error opening input file\n");
    return 1;
  }

  char header_buffer[MAX_LENGTH_HEADER];
  char tpoint_buffer[MAX_LENGTH_TGEO];

  /* Read the first line of the first file with the headers */
  fscanf(file, "%1023s\n", header_buffer);

  /* Continue reading the first file */
  int nrows = 0;
  do
  {
    int k;
    int read = fscanf(file, "%d,%7500[^\n]\n", &k, tpoint_buffer);

    if (ferror(file) || read != 2)
    {
      printf("Error reading input file\n");
      fclose(file);
      return 1;
    }

    /* Transform the string read into a tcbuffer value */
    Temporal *temp = tgeompoint_in(tpoint_buffer);

    /* Additional values required for the functions, uncomment as needed */
    Interval *interv = interval_in("5 minutes", -1);
    TimestampTz start = temporal_start_timestamptz(temp);

    /* Uncomment the desired function to compute */
    // Temporal *rest = temporal_simplify_dp(temp, 5, true);
    Temporal *rest = temporal_tsample(temp, interv, start, "linear");
    // Temporal *rest = temporal_tprecision(temp, interv, start);
    if (rest)
    {
      /* Get the number of instants of the result */
      int count1 = temporal_num_instants(temp);
      int count2 = temporal_num_instants(rest);
      free(rest);

      printf("k: %d, Number of instants: %d -> %d\n",
        k, count1, count2);
      nrows++;
    }
    free(temp);
    free(interv);
  } while (! feof(file));

  printf("Number of non-empty answers: %d\n", nrows);

  /* Close the files */
  fclose(file);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
