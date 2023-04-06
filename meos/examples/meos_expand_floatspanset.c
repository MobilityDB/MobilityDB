/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief A simple program that reads AIS data from a CSV file, constructs
 * trips from these records, and outputs for each trip the MMSI, the number of
 * instants, and the distance travelled.
 *
 * Please read the assumptions made about the input file `aisinput.csv` in the
 * file `02_meos_read_ais.c` in the same directory. Furthermore, this program
 * assumes the input file contains less than 50K observations for at most
 * five ships. Also, the program does not cope with erroneous inputs, such as
 * two or more observations for the same ship with equal timestamp values and
 * supposes that the observations are in increasing timestamp value.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o meos_expand_floatspanset meos_expand_floatspanset.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <meos.h>
#include <meos_internal.h>

/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LENGTH_HEADER 1024
/* Maximum length in characters of a span set in the input data */
#define MAX_LENGTH_SPANSET 1024
/* Number of groups for accumulating the input span sets */
#define NUMBER_GROUPS 10

typedef struct
{
  int k;
  SpanSet *ss;
} floatspanset_record;

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize("");

  /* Get start time */
  clock_t t;
  t = clock();

  /* Spanset for aggregating the spans */
  SpanSet *state[NUMBER_GROUPS] = {0};

  /* Substitute the full file path in the first argument of fopen */
  FILE *file = fopen("floatspanset.csv", "r");

  if (! file)
  {
    printf("Error opening file\n");
    return 1;
  }

  floatspanset_record rec;
  int no_records = 0;
  int no_nulls = 0;
  char header_buffer[MAX_LENGTH_HEADER];
  char spanset_buffer[MAX_LENGTH_SPANSET];

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", header_buffer);

  /* Continue reading the file */
  do
  {
    int read = fscanf(file, "%d,\"%[^\"]\"\n", &rec.k, spanset_buffer);

    if (read != 2 && ! feof(file))
    {
      printf("Record with missing values ignored\n");
      no_nulls++;
      continue;
    }

    if (ferror(file))
    {
      printf("Error reading file\n");
      fclose(file);
    }

    no_records++;

    /* Transform the string representing the timestamp into a timestamp value */
    rec.ss = floatspanset_in(spanset_buffer);

    state[rec.k%10] = spanset_union_transfn(state[rec.k%10], rec.ss);

    /*
     * Create the instants and append them in the corresponding ship record.
     * In the input file it is assumed that
     * - The coordinates are given in the WGS84 geographic coordinate system
     * - The timestamps are given in GMT time zone
     */
    char *spanset_out = floatspanset_out(rec.ss, 3);
    printf("k: %d, spanset: %s\n", rec.k, spanset_out);
    free(spanset_out);
    free(rec.ss);
  } while (!feof(file));

  printf("\n%d records read.\n%d incomplete records ignored.\n",
    no_records, no_nulls);

  /* Close the file */
  fclose(file);

  /* Compute the final result */
  for (int i = 0; i < NUMBER_GROUPS; i++)
  {
    SpanSet *final = spanset_union_finalfn(state[i]);
    /* Print the accumulated span set */
    printf("----------\n");
    printf("Group: %d\n", i + 1);
    printf("----------\n");
    char *spanset_out = floatspanset_out(final, 3);
    printf("spanset: %s\n", spanset_out);
    free(spanset_out);
    free(state[i]);
    free(final);
  }

  /* Calculate the elapsed time */
  t = clock() - t;
  double time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
