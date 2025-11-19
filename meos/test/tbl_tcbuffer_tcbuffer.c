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
 * @brief A simple program that reads a CSV files containing temporal circular
 * buffers and performs a "self join" by restricting couples of temporal
 * circular buffers.
 *
 * The corresponding SQL query would be
 * @code
 * SELECT numInstants(tIntersects(temp1, temp2))
 * FROM tbl_tcbuffer t1, tbl_tcbuffer t2
 * @endcode
 *
 * The program can be tested with several predicates such as spatiotemporal
 * relationships `eIntersects`, `eDwithin`, ..., `aIntersects`, `aDwithin`,
 * ..., `tIntersects`, `tDwithin`, ..., temporal `distance` ...
 * 
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tbl_tcbuffer_tcbuffer tbl_tcbuffer_tcbuffer.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_cbuffer.h>

/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LENGTH_HEADER 1024
/* Maximum length in characters of a temporal circular buffer in the input
 * data as computed by the following query on the corresponding table
 * SELECT MAX(length(temp::text)) FROM tbl_tcbuffer;
 * -- 7449
 */
#define MAX_LENGTH_TCBUFFER 7501

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* You may substitute the full file path in the first argument of fopen */
  FILE *file = fopen("data/tbl_tcbuffer.csv", "r");

  if (! file)
  {
    printf("Error opening input file\n");
    return 1;
  }

  char header_buffer[MAX_LENGTH_HEADER];
  char tcbuffer_buffer[MAX_LENGTH_TCBUFFER];

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
      int read1 = fscanf(file, "%d,\"%7500[^\"\n]\"\n", &k1, tcbuffer_buffer);
      if (ferror(file) || read1 != 2)
      {
        printf("Error reading input file\n");
        fclose(file);
        return 1;
      }
      if (k == k1)
        break;
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
      /* Transform the string read into a tcbuffer value */
      Temporal *temp1 = tcbuffer_in(tcbuffer_buffer);

      /* Rewind the file to the beginning */
      rewind(file);
      /* Read the first line of the second file with the headers */
      fscanf(file, "%1023s\n", header_buffer);

      /* For each line in the file loop for every line in the second file */
      do
      {
        int read2 = fscanf(file, "%d,\"%7500[^\"\n]\"\n", &k2, tcbuffer_buffer);
        if (ferror(file) || read2 != 2)
        {
          printf("Error reading input file2\n");
          fclose(file);
          return 1;
        }

        /* Print only 1 out of X records, change the condition as needed */
        if (true) // (k2 % 2 == 0)
        {
          /* Transform the string read into a tcbuffer value */
          Temporal *temp2 = tcbuffer_in(tcbuffer_buffer);

          /* Compute the function, uncomment the desired function */
          // Temporal *rest = tintersects_tcbuffer_tcbuffer(temp1, temp2,
            // false, false);
          // Temporal *rest = tdwithin_tcbuffer_tcbuffer(temp1, temp2, 10,
            // false, false);
          Temporal *rest = tdistance_tcbuffer_tcbuffer(temp1, temp2);
          if (rest)
          {
            /* Increment the number of non-empty answers found */
            nrows++;
            /* Get the number of instants of the result */
            int count = temporal_num_instants(rest);
            free(rest);
            printf("k1: %d, k2: %d: Number of instants of the result: %d\n",
              k1, k2, count);
          }
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
