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
 * @brief A simple program that reads from a CSV file a set of records
 * containing text sets, group them by the key % 10, and at the end apply a
 * union aggregate to the groups.
 * @note The function tests the expandable set data structure.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o textset_group_agg textset_group_agg.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <meos.h>
/* The expandable functions are in the internal MEOS API */
#include <meos_internal.h>

/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LENGTH_HEADER 1024
/* Maximum length in characters of a set in the input data */
#define MAX_LENGTH_SET 1024

typedef struct
{
  int k;
  Set *set;
} textset_record;

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize(NULL, NULL);

  /* Get start time */
  clock_t t;
  t = clock();

  /* Array of sets for aggregating the input sets grouping them by k%10 */
  Set *state[10] = {0};

  /* Substitute the full file path in the first argument of fopen */
  FILE *file = fopen("data/textset.csv", "r");

  if (! file)
  {
    printf("Error opening input file\n");
    return 1;
  }

  textset_record rec;
  int no_records = 0;
  int no_nulls = 0;
  char header_buffer[MAX_LENGTH_HEADER];
  char set_buffer[MAX_LENGTH_SET];

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", header_buffer);

  /* Continue reading the file */
  do
  {
    int read = fscanf(file, "%d,'%[^']'\n", &rec.k, set_buffer);

    if (read != 2 && ! feof(file))
    {
      printf("Record with missing values ignored\n");
      no_nulls++;
      continue;
    }

    if (ferror(file))
    {
      printf("Error reading input file\n");
      fclose(file);
    }

    no_records++;

    /* Transform the string representing the set into a set value */
    rec.set = textset_in(set_buffer);

    /* Aggregate the input set into the corresponding group */
    state[rec.k % 10] = set_union_transfn(state[rec.k % 10], rec.set);

    /* Free memory of the input set */
    free(rec.set);
  } while (!feof(file));

  /* Close the file */
  fclose(file);

  printf("\n%d records read.\n%d incomplete record%s ignored.\n\n",
    no_records, no_nulls, (no_nulls > 1) ? "s" : "");

  /* Compute the final result */
  for (int i = 0; i < 10; i++)
  {
    Set *final = set_union_finalfn(state[i]);
    /* Print the accumulated set */
    char *final_out = textset_out(final);
    printf("-------------------------------\n");
    printf("Set: %d, Number of elements: %d\n", i, final->count);
    printf("-------------------------------\n");
    printf("%s\n", final_out);
    free(final_out);
    free(final);    
    free(state[i]);
  }

  /* Calculate the elapsed time */
  t = clock() - t;
  double time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("\nThe program took %f seconds to execute\n", time_taken);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
