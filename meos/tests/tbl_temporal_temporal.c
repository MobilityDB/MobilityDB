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
 * @brief A simple program that reads a CSV files containing temporal values
 * and performs a "self join" by applying to them an operation
 *
 * The corresponding SQL query would be
 * @code
 * SELECT t1.k, t2.k, numInstants(t1.temp <-> t2.temp))
   FROM tbl_tfloat t1, tbl_tfloat t2
   WHERE t1.temp <-> t2.temp IS NOT NULL;
 * @endcode
 *
 * The program can be tested with several functions such as ever/alwayas and 
 * temporal comparisons `ever_eq`, `always_eq`, `teq_`, ..., `ever_lt`,
 * `always_lt`, `tlt_`, ...
 * 
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tbl_temporal_temporal tbl_temporal_temporal.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_internal.h>
#include <meos_geo.h>

/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LENGTH_HEADER 1024
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

  /* You may substitute the full file path in the first argument of fopen */
  // FILE *file = fopen("csv/tbl_tbool.csv", "r");
  // FILE *file = fopen("csv/tbl_tint.csv", "r");
  // FILE *file = fopen("csv/tbl_tfloat.csv", "r");
  // FILE *file = fopen("csv/tbl_ttext.csv", "r");
  FILE *file = fopen("csv/tbl_tgeompoint.csv", "r");
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

  char header_buffer[MAX_LENGTH_HEADER];
  char temporal_buffer[MAX_LENGTH_TEMP];

  int k = 1, k1, k2, nrows = 0;
  do
  {
    /* Read the first line of the file with the headers */
    fscanf(file, "%1023s\n", header_buffer);
    if (ferror(file))
    {
      printf("Error reading input file\n");
      fclose(file);
      return 1;
    }

    /* Continue reading the file until the like identified by the key `k` */
    do
    {
      int read1 = fscanf(file, "%d,%8191[^\n]\n", &k1, temporal_buffer);
      if (ferror(file))
      {
        printf("Error reading input file\n");
        fclose(file);
        return 1;
      }
      /* If we have found the record we are looking for */
      if (k == k1)
      {
        if (read1 == 2)
          break;
        /* Ignore records with NULL values and continue */
        k++;
        continue;
      }
    } while (! feof(file));
    if (k != k1)
    {
      printf("Error reading input file, key %d not found\n", k);
      fclose(file);
      return 1;
    }

    /* Print only 1 out of X records, change the condition as needed */
    if (true) // (k1 % 10 == 0)
    {
      /* Transform the string read into a temporal value */
      // Temporal *temp1 = tbool_in(temporal_buffer);
      // Temporal *temp1 = tint_in(temporal_buffer);
      // Temporal *temp1 = tfloat_in(temporal_buffer);
      // Temporal *temp1 = ttext_in(temporal_buffer);
      Temporal *temp1 = tgeompoint_in(temporal_buffer);
      // Temporal *temp1 = tgeogpoint_in(temporal_buffer);
      // Temporal *temp1 = tgeometry_in(temporal_buffer);
      // Temporal *temp1 = tgeography_in(temporal_buffer);
      // Temporal *temp1 = tcbuffer_in(temporal_buffer);
      // Temporal *temp1 = tnpoint_in(temporal_buffer);
      // Temporal *temp1 = tpose_in(temporal_buffer);
      // Temporal *temp1 = trgeometry_in(temporal_buffer);

      /* Rewind the file to the beginning */
      rewind(file);
      /* Read the first line of the second file with the headers */
      fscanf(file, "%1023s\n", header_buffer);

      /* For each line in the file loop for every line in the second file */
      do
      {
        int read2 = fscanf(file, "%d,%8191[^\n]\n", &k2, temporal_buffer);
        if (ferror(file))
        {
          printf("Error reading input file\n");
          fclose(file);
          return 1;
        }
        /* Ignore records with NULL values and continue */
        if (read2 != 2)
          continue;

        /* Print only 1 out of X records, change the condition as needed */
        if (true) // (k2 % 2 == 0)
        {
          /* Transform the string read into a temporal value */
          // Temporal *temp2 = tbool_in(temporal_buffer);
          // Temporal *temp2 = tint_in(temporal_buffer);
          // Temporal *temp2 = tfloat_in(temporal_buffer);
          // Temporal *temp2 = ttext_in(temporal_buffer);
          Temporal *temp2 = tgeompoint_in(temporal_buffer);
          // Temporal *temp2 = tgeogpoint_in(temporal_buffer);
          // Temporal *temp2 = tgeometry_in(temporal_buffer);
          // Temporal *temp2 = tgeography_in(temporal_buffer);
          // Temporal *temp2 = tcbuffer_in(temporal_buffer);
          // Temporal *temp2 = tnpoint_in(temporal_buffer);
          // Temporal *temp2 = tpose_in(temporal_buffer);
          // Temporal *temp2 = trgeometry_in(temporal_buffer);

          /* Uncomment if we need to select temporal values of a given subtype */
          // if (temp2->subtype != TINSTANT)
          // if (temp2->subtype != TSEQUENCE)
          // {
            // free(temp2);
            // continue;
          // }

          /* Uncomment for custom processing needed by the function to apply */

          /* Processing for appendInstant */
          // TimestampTz t = temporal_end_timestamptz(temp1);
          // Interval *interv = interval_in("5 minutes", -1);
          // TimestampTz shift = add_timestamptz_interval(t, interv);
          // Datum value = tinstant_value_p((TInstant *) temp2);
          // TInstant *inst = tinstant_make(value, temp2->temptype, shift);
          // free(interv);

          /* Processing for appendSequence */
          // TimestampTz end = temporal_end_timestamptz(temp1);
          // Interval *interv = interval_in("5 minutes", -1);
          // TimestampTz shift = add_timestamptz_interval(end, interv);
          // TimestampTz start = temporal_start_timestamptz(temp2);
          // /* Continue reading if the values have different interpolation */
          // interpType interp1 = MEOS_FLAGS_GET_INTERP(temp1->flags);
          // interpType interp2 = MEOS_FLAGS_GET_INTERP(temp2->flags);
          // if (interp1 != INTERP_NONE && interp1 != interp2)
          // {
            // free(temp2);
            // continue;
          // }
          // /* If the sequence starts before the end of the temporal value */
          // TSequence *seq;
          // if (start <= shift)
          // {
            // Interval *delta = minus_timestamptz_timestamptz(shift, start);
            // seq = (TSequence *) temporal_shift_time(temp2, delta);
            // free(delta);
          // }
          // else
            // seq = (TSequence *) temp2;
          // free(interv);

          /* Result is a temporal value */

          /* Compute the function, uncomment the desired function */
          // Temporal *rest = tand_tbool_tbool(temp1, temp2);
          // Temporal *rest = teq_temporal_temporal(temp1, temp2);
          // Temporal *rest = tlt_temporal_temporal(temp1, temp2);
          // Temporal *rest = add_tnumber_tnumber(temp1, temp2);
          // Temporal *rest = sub_tnumber_tnumber(temp1, temp2);
          // Temporal *rest = mult_tnumber_tnumber(temp1, temp2);
          // Temporal *rest = div_tnumber_tnumber(temp1, temp2);
          // Temporal *rest = tdistance_tnumber_tnumber(temp1, temp2);
          // Temporal *rest = textcat_ttext_ttext(temp1, temp2);
          // Temporal *rest = tdistance_tgeo_tgeo(temp1, temp2);
          // Temporal *rest = tdwithin_tspatial_tspatial(temp1, temp2, 5, false, false);
          // Temporal *rest = temporal_append_tinstant(temp1, inst, LINEAR, 0.0,
          // Temporal *rest = temporal_append_tsequence(temp1, seq, false);

          // if (rest)
          // {
            // /* Increment the number of non-empty answers found */
            // nrows++;
            // /* Get the number of instants of the result */
            // int count = temporal_num_instants(rest);
            // free(rest);
            // printf("k1: %d, k2: %d: Number of instants of the result: %d\n",
              // k1, k2, count);
          // }

          /* Result is a value */

         /* Compute the function, uncomment the desired function */
          // Temporal *rest = tand_tbool_tbool(temp1, temp2);

          /******************* Similarity functions *******************/

          // double result = temporal_dyntimewarp_distance(temp1, temp2);
          // double result = temporal_frechet_distance(temp1, temp2);
          // double result = temporal_hausdorff_distance(temp1, temp2);
          // int count;
          // Match *result = temporal_dyntimewarp_path(temp1, temp2, &count);
          // Match *result = temporal_frechet_path(temp1, temp2, &count);

          // if (result != DBL_MAX)
          if (count > 0)
          {
            /* Increment the number of non-empty answers found */
            nrows++;
            printf("k1: %d, k2: %d: Result of the function: %lf\n",
              k1, k2, result);
            printf("k1: %d, k2: %d: Result of the function: %d\n",
              k1, k2, count);
            free(result);
          }

          // free(inst);
          /* append_sequence: Free if the sequence was shifted */
          // if (temp2 != (Temporal *) seq) free(seq);
          free(temp2);
        }
      } while (! feof(file)); /* Inner loop */
      /* Clean up for the next iteration */
      free(temp1);
    }
    
    /* Increment the key value to be found for the outer loop and rewind the
     * file to the beginning */
    k++;
    if (k > 100)
      break;
    rewind(file);
  } while (! feof(file)); /* Outer loop */

  printf("Number of non-empty answers: %d\n", nrows);

  /* Close the files */
  fclose(file);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
