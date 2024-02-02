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
 * @brief A simple program that generates a given number of tfloat instants,
 * accumulates each generated instants into an array, and at the end assembles
 * the sequence from the input instants and outputs the number of instants and
 * the last value of the sequence.
 *
 * This program and the program tfloat_expand.c in the same directory can be
 * used to compare the two alternative strategies for
 * (1) assembling the sequence at the end from the input instants
 * (2) expanding the sequence at each input instant
 *
 * The instants are generated so they are not redundant, that is, all input
 * instants will appear in the final sequence.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tfloat_assemble tfloat_assemble.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <meos.h>

#define MAX_INSTANTS 1000000

/* Main program */
int main(void)
{
  /* Get start time */
  clock_t tm;
  tm = clock();

  /* Initialize MEOS */
  meos_initialize(NULL, NULL);

  /* Input instants that are accumulated */
  TInstant *instants[MAX_INSTANTS] = {0};
  /* Sequence constructed from the input instants */
  Temporal *seq = NULL;
  /* Interval to add */
  Interval *oneday = pg_interval_in("1 day", -1);
  /* Iterator variable */
  int i;
  /* Seed the random number generator with the current time in seconds. */
  srandom (time (0));

  TimestampTz t = pg_timestamptz_in("1999-12-31", -1);
  for (i = 0; i < MAX_INSTANTS; i++)
  {
    t = add_timestamptz_interval(t, oneday);
    instants[i] = tfloatinst_make(i % 2 + 1, t);
  }

  seq = (Temporal *) tsequence_make((const TInstant **) instants, MAX_INSTANTS,
    true, true, STEP, true);

  /* Print information about the sequence */
  printf("Number of generated instants: %d, Time-weighted average: %f\n",
    temporal_num_instants(seq), tnumber_twavg(seq));

  /* Free memory */
  free(seq);

  /* Finalize MEOS */
  meos_finalize();

  /* Calculate the elapsed time */
  tm = clock() - tm;
  double time_taken = ((double) tm) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);
  printf("It accumulates the generated instants and constructs the ouput sequence at the end\n");

  return 0;
}
