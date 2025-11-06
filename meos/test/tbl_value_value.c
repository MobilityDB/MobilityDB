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
 * @brief A simple program that reads two CSV files containing non-temporal
 * values and applies a function to them.
 *
 * The corresponding SQL query would be
 * @code
 * SELECT t1.k, t2.k, array_length(timeSpans(t, i), 1)
   FROM tbl_tstzspanset t1, tbl_interval t2
   WHERE timeSpans(t, i) IS NOT NULL;
 * @endcode
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tbl_value_value tbl_value_value.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_cbuffer.h>
#include <meos_npoint.h>
#include <meos_pose.h>

/* Maximum length in characters of a header record in the input CSV file1 */
#define MAX_LENGTH_HEADER 1024
#define MAX_LENGTH_VALUE 12001
/* Maximum length in characters of a temporal circular buffer in the input
 * data as computed by the following query on the corresponding table
 * SELECT MAX(length(temp::text)) FROM tbl_tgeometry;
 * -- 6273
 */
#define MAX_LENGTH_TEMP 8192

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* You may substitute the full file1 path in the first argument of fopen */
  // FILE *file1 = fopen("csv/tbl_int.csv", "r");
  // FILE *file1 = fopen("csv/tbl_intset.csv", "r");
  // FILE *file1 = fopen("csv/tbl_intspan.csv", "r");
  // FILE *file1 = fopen("csv/tbl_intspanset.csv", "r");
  // FILE *file1 = fopen("csv/tbl_float.csv", "r");
  // FILE *file1 = fopen("csv/tbl_floatset.csv", "r");
  // FILE *file1 = fopen("csv/tbl_floatspan.csv", "r");
  // FILE *file1 = fopen("csv/tbl_floatspanset.csv", "r");
  // FILE *file1 = fopen("csv/tbl_interval.csv", "r");
  // FILE *file1 = fopen("csv/tbl_timestamptz.csv", "r");
  // FILE *file1 = fopen("csv/tbl_tstzset.csv", "r");
  // FILE *file1 = fopen("csv/tbl_tstzspan.csv", "r");
  FILE *file1 = fopen("csv/tbl_tstzspanset.csv", "r");

  if (! file1)
  {
    printf("Error opening first input file\n");
    return 1;
  }

  /* You may substitute the full file1 path in the first argument of fopen */
  // FILE *file2 = fopen("csv/tbl_int.csv", "r");
  // FILE *file2 = fopen("csv/tbl_intset.csv", "r");
  // FILE *file2 = fopen("csv/tbl_intspan.csv", "r");
  // FILE *file2 = fopen("csv/tbl_intspanset.csv", "r"); 
  // FILE *file2 = fopen("csv/tbl_float.csv", "r");
  // FILE *file2 = fopen("csv/tbl_floatset.csv", "r");
  // FILE *file2 = fopen("csv/tbl_floatspan.csv", "r");
  // FILE *file2 = fopen("csv/tbl_floatspanset.csv", "r");
  FILE *file2 = fopen("csv/tbl_interval.csv", "r");
  // FILE *file2 = fopen("csv/tbl_timestamptz.csv", "r");
  // FILE *file2 = fopen("csv/tbl_tstzset.csv", "r");
  // FILE *file2 = fopen("csv/tbl_tstzspan.csv", "r");
  // FILE *file2 = fopen("csv/tbl_tstzspanset.csv", "r");

  if (! file2)
  {
    printf("Error opening second input file\n");
    fclose(file2);
    return 1;
  }

  char header_buffer[MAX_LENGTH_HEADER];
  char value1_buffer[MAX_LENGTH_VALUE];
  char value2_buffer[MAX_LENGTH_VALUE];

  /* Read the first line of the first file with the headers */
  fscanf(file1, "%1023s\n", header_buffer);

  /* Continue reading the first file */
  int nrows = 0;
  do
  {
    int k1;
    int read1 = fscanf(file1, "%d,%8191[^\n]\n", &k1, value1_buffer);

    if (ferror(file1))
    {
      printf("Error reading input file1\n");
      fclose(file1);
      fclose(file2);
      return 1;
    }
    if (read1 != 2)
      continue;

    /* Transform the string read into a temporal value */
    // int i = int_in(value1_buffer);
    // Set *t = intset_in(value1_buffer);
    // Span *t = intspan_in(value1_buffer);
    // SpanSet *t = intspanset_in(value1_buffer);
    // double d = float_in(value1_buffer);
    // Set *t = floatset_in(value1_buffer);
    // Span *t = floatspan_in(value1_buffer);
    // SpanSet *t = floatspanset_in(value1_buffer);
    // Interval *i = interval_in(value1_buffer, -1);
    // TimestampTz t = timestamptz_in(value1_buffer, -1);
    // Set *s = tstzset_in(value1_buffer);
    // Span *s = tstzspan_in(value1_buffer);
    SpanSet *ss = tstzspanset_in(value1_buffer);

    /* Rewind the second file to the beginning */
    rewind(file2);
    /* Read the first line of the second file with the headers */
    fscanf(file2, "%1023s\n", header_buffer);

    /* For each line in the first file loop for every line in the second file */
    do
    {

      int k2;
      int read2 = fscanf(file2, "%d,%12000[^\n]\n", &k2, value2_buffer);

      if (ferror(file2))
      {
        printf("Error reading input file2\n");
        fclose(file1);
        fclose(file2);
        return 1;
      }
      /* Ignore records with NULL values and continue reading */
      if (read2 != 2)
        continue;

      /* Print only 1 out of X records */
      if (k1 % 5 == 0 && k2 % 5 == 0) // (true)
      {

        /* Transform the string read into a value */
        // int i = int_in(value2_buffer);
        // Set *t = intset_in(value2_buffer);
        // Span *t = intspan_in(value2_buffer);
        // SpanSet *t = intspanset_in(value2_buffer);
        // double d = float_in(value2_buffer);
        // Set *t = floatset_in(value2_buffer);
        // Span *t = floatspan_in(value2_buffer);
        // SpanSet *t = floatspanset_in(value2_buffer);
        Interval *i = interval_in(value2_buffer, -1);
        // TimestampTz t = timestamptz_in(value2_buffer, -1);
        // Set *s = tstzset_in(value2_buffer);
        // Span *s = tstzspan_in(value2_buffer);
        // SpanSet *ss = tstzspanset_in(value2_buffer);

        /* Uncomment the desired function to compute */

        /******************* Restriction functions *******************/

        // Temporal *rest = temporal_at_value(temp, t);
        // Temporal *rest = temporal_at_values(temp, t);
        // Temporal *rest = tnumber_at_span(temp, t);
        // Temporal *rest = tnumber_at_spanset(temp, t);
        // Temporal *rest = temporal_at_timestamptz(temp, t);
        // Temporal *rest = temporal_at_tstzset(temp, t);
        // Temporal *rest = temporal_at_tstzspan(temp, t);
        // Temporal *rest = temporal_at_tstzspanset(temp, t);

        /******************* Reduction functions *******************/

        TimestampTz origin = timestamptz_in("2000-01-03", -1);
        // Temporal *rest = temporal_tprecision(temp, i, origin);
        // Temporal *rest = temporal_tsample(temp, i, origin, "Step");

        /******************* Modification functions *******************/

        // Temporal *rest = temporal_delete_timestamptz(temp, t, false);
        // Temporal *rest = temporal_delete_tstzset(temp, s, false);
        // Temporal *rest = temporal_delete_tstzspan(temp, s, false);
        // Temporal *rest = temporal_delete_tstzspanset(temp, ss, false);

        /******************* Tile functions *******************/

        int count;
        Span *result = tstzspanset_bins(ss, i, origin, &count);
        
        if (count)
        {
          /* Get the number of instants of the result */
          free(result);
          printf("k1: %d, k2: %d: Number of elements of the result: %d\n",
            k1, k2, count);
          nrows++;
        }
        /* Free value, uncomment as necessary */
        free(i); 
      }
    } while (! feof(file2));
    free(ss);
  } while (! feof(file1));

  printf("Number of non-empty answers: %d\n", nrows);

  /* Close the files */
  fclose(file1);
  fclose(file2);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
