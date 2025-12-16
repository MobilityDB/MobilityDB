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
 * @brief A simple program that a CSV file containing temporal values and
 * applies a function to them
 *
 * The corresponding SQL query would be
 * @code
 * SELECT k, numInstants(shiftScaleValue(temp, 5, 10))
   FROM tbl_tfloat;
 * @endcode
 *
 * The program can be tested with several functions such as simplification
 * with Douglas-Peucker ... reduction such as `tsample`, `tprecision`, ...
 * 
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tbl_temporal tbl_temporal.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_cbuffer.h>
#include <meos_geo.h>

/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LEN_HEADER 1024
/* Maximum length in characters of a temporal value in the input data as
 * computed by the following query on the corresponding table
 * SELECT MAX(length(temp::text)) FROM tbl_tgeometry;
 * -- 6273
 */
#define MAX_LEN_TEMP 8192


/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* You may substitute the full file path in the first argument of fopen */
  // FILE *file = fopen("csv/tbl_tbool.csv", "r");
  // FILE *file = fopen("csv/tbl_tint.csv", "r");
  // FILE *file = fopen("csv/tbl_tfloat.csv", "r");
  FILE *file = fopen("csv/tbl_ttext.csv", "r");
  // FILE *file = fopen("csv/tbl_tgeompoint.csv", "r");
  // FILE *file = fopen("csv/tbl_tgeogpoint.csv", "r");
  // FILE *file = fopen("csv/tbl_tgeometry.csv", "r");
  // FILE *file = fopen("csv/tbl_tgeography.csv", "r");
  // FILE *file = fopen("csv/tbl_tcbuffer.csv", "r");
  // FILE *file = fopen("csv/tbl_tnpoint.csv", "r");
  // FILE *file = fopen("csv/tbl_tpose2d.csv", "r");
  // FILE *file = fopen("csv/tbl_trgeometry.csv", "r");
  if (! file)
  {
    printf("Error opening input file\n");
    return 1;
  }

  char header_buffer[MAX_LEN_HEADER];
  char temporal_buffer[MAX_LEN_TEMP];

  /* Read the first line of the first file with the headers */
  fscanf(file, "%1023s\n", header_buffer);

  /* Continue reading the first file */
  int nrows = 0;
  do
  {
    int k;
    int read = fscanf(file, "%d,%7500[^\n]\n", &k, temporal_buffer);

    if (ferror(file))
    {
      printf("Error reading input file\n");
      fclose(file);
      return 1;
    }
    /* Ignore records with NULL values and continue reading */
    if (read != 2)
      continue;

    /* Transform the string read into a temporal value */
    // Temporal *temp = tbool_in(temporal_buffer);
    // Temporal *temp = tint_in(temporal_buffer);
    // Temporal *temp = tfloat_in(temporal_buffer);
    Temporal *temp = ttext_in(temporal_buffer);
    // Temporal *temp = tgeompoint_in(temporal_buffer);
    // Temporal *temp = tgeogpoint_in(temporal_buffer);
    // Temporal *temp = tgeometry_in(temporal_buffer);
    // Temporal *temp = tgeography_in(temporal_buffer);
    // Temporal *temp = tcbuffer_in(temporal_buffer);
    // Temporal *temp = tnpoint_in(temporal_buffer);
    // Temporal *temp = tpose_in(temporal_buffer);
    // Temporal *temp = trgeometry_in(temporal_buffer);

    /* Additional values required for the functions, uncomment as needed */
    // Interval *interv = interval_in("5 minutes", -1);
    // TimestampTz start = temporal_start_timestamptz(temp);

    /* Uncomment the desired function to compute */
    // Temporal *rest = temporal_simplify_dp(temp, 5, true);
    // Temporal *rest = temporal_tsample(temp, interv, start, "linear");
    // Temporal *rest = temporal_tprecision(temp, interv, start);
    // Temporal *rest = tfloat_shift_scale_value(temp, 5, 10);
    Temporal *rest = ttext_initcap(temp);
    // Temporal *rest = tgeometry_to_tcbuffer(temp);
    if (rest)
    {
      /* Get the number of instants of the result */
      int count = temporal_num_instants(rest);
      printf("k: %d, Number of instants: %d\n", k, count);
      // int count1 = temporal_num_instants(temp);
      // int count2 = temporal_num_instants(rest);
      // printf("k: %d, Number of instants: %d -> %d\n", k, count1, count2);

      free(rest);
      nrows++;
    }
    free(temp);
    // free(interv);
  } while (! feof(file));

  printf("Number of non-empty answers: %d\n", nrows);

  /* Close the files */
  fclose(file);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
