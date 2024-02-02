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
 * @brief A simple program that reads integer span values from a CSV file and
 * aggregates them into a single spanset value.
 *

 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o intspan_agg intspan_agg.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <meos.h>
/* The aggregate functions are in the internal MEOS API */
#include <meos_internal.h>

/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LENGTH_HEADER 1024
/* Maximum length in characters of a span in the input data */
#define MAX_LENGTH_SPAN 64

typedef struct
{
  int k;
  Span *span;
} intspan_record;

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize("", NULL);

  /* Get start time */
  clock_t t;
  t = clock();

  /* Spanset for aggregating the spans */
  SpanSet *state = NULL;

  /* Substitute the full file path in the first argument of fopen */
  FILE *file = fopen("data/intspan.csv", "r");

  if (! file)
  {
    printf("Error opening input file\n");
    return 1;
  }

  intspan_record rec;
  int no_records = 0;
  int no_nulls = 0;
  char header_buffer[MAX_LENGTH_HEADER];
  char span_buffer[MAX_LENGTH_SPAN];

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", header_buffer);

  /* Continue reading the file */
  do
  {
    int read = fscanf(file, "%d,\"%[^\"]\"\n", &rec.k, span_buffer);

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

    /* Transform the string representing the span into a span value */
    rec.span = intspan_in(span_buffer);

    state = span_union_transfn(state, rec.span);

    /* Output the input span */
    char *span_out = intspan_out(rec.span);
    printf("k: %d, span: %s\n", rec.k, span_out);
    free(span_out);
    free(rec.span);
  } while (!feof(file));

  printf("\n%d records read.\n%d incomplete records ignored.\n",
    no_records, no_nulls);

  /* Close the file */
  fclose(file);

  /* Compute the final result */
  SpanSet *final = spanset_union_finalfn(state);

  /* Print the accumulated span set */
  char *spanset_out = intspanset_out(final);
  printf("spanset: %s\n", spanset_out);
  free(spanset_out);
  free(state);
  free(final);

  /* Calculate the elapsed time */
  t = clock() - t;
  double time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
