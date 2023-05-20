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
 * @brief A simple program that generates a given number of ttext instants,
 * appends the instant into a sequence at each generation, and outputs the
 * number of instants and the last value of the sequence at the end.
 *
 * The instants are generated so they are not redundant, that is, all input
 * instants will appear in the final sequence. A compiler option allows to
 * either use expandable structures or to create a new sequence at every new
 * instant generated.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o meos_expand_ttext meos_expand_ttext.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <meos.h>

/* Define the approach for processing the input instants. Possible values are
 * - false => static: produce a new sequence value after adding every instant
 * - true => expand: use expandable structures
 */
#define EXPAND true
/* Maximum number of instants */
#define MAX_INSTANTS 1000000
/* Number of instants in a batch for printing a marker */
#define NO_INSTANTS_BATCH 1000
/* Maximum length in characters of the input instant */
#define MAX_LENGTH_INST 64
/* Maximum length in characters of the text values in the instants */
#define MAX_LENGTH_TEXT 20

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize(NULL);

  /* Get start time */
  clock_t tm;
  tm = clock();

  /* Sequence constructed from the input instants */
  Temporal *seq = NULL;
  /* Interval to add */
  Interval *oneday = pg_interval_in("1 day", -1);
  /* Iterator variable */
  int i;
  /* Seed the random number generator with the current time in seconds. */
  srandom (time (0));

#if EXPAND == false
  printf("Generating the instants (one marker every %d instants)\n",
    NO_INSTANTS_BATCH);
#endif

  TimestampTz t = pg_timestamptz_in("1999-12-31", -1);
  for (i = 0; i < MAX_INSTANTS; i++)
  {
    /* Generate the instant */
    /* Use a random generator to set the length of the text value */
    int len = random() % MAX_LENGTH_TEXT + 1;
    char *value = malloc(sizeof(char) * (len + 2));
    memset(value, i % 2 == 0 ? 'A' : 'B', len);
    value[len] = '\0';
    text *txt = cstring2text(value);
    t = pg_timestamp_pl_interval(t, oneday);
    TInstant *inst = ttextinst_make(txt, t);
    free(value); free(txt);
    if (! seq)
      seq = (Temporal *) tsequence_make_exp((const TInstant **) &inst, 1,
        EXPAND ? 2 : 1, true, true, STEP, false);
    else
    {
      Temporal *oldseq = seq;
      seq = temporal_append_tinstant((Temporal *) seq, inst, EXPAND);
      if (oldseq != seq)
        free(oldseq);
    }
    free(inst);

#if EXPAND == false
    /* Print a marker every X instants generated */
    if (i % NO_INSTANTS_BATCH == 0)
    {
      printf("*");
      fflush(stdout);
    }
#endif
  }

  /* Print information about the sequence */
  // Uncomment the next line to see the resulting sequence value
  // printf("%s\n", ttext_out(seq));
  char *str = text2cstring(ttext_end_value(seq));
  printf("\nNumber of instants: %d, Last value : %s\n",
    temporal_num_instants(seq), str);

  /* Free memory */
  free(seq);
  free(str);

  /* Calculate the elapsed time */
  tm = clock() - tm;
  double time_taken = ((double) tm) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);
  printf("Using %s structures\n", EXPAND ? "expandable" : "static");

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
