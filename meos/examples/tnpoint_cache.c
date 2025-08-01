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
 * @brief A simple program that generates a given number of tnpoint instants
 * in order to test the ways cache
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tnpoint_cache tnpoint_cache.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <meos.h>
#include <meos_npoint.h>

#define MAX_INSTANTS 100000
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
  meos_initialize();

  /* Get start time */
  clock_t tm;
  tm = clock();

  /* Input instants that are accumulated */
  TInstant *instants[MAX_INSTANTS] = {0};
  /* Iterator variable */
  int i;
  /* Seed the random number generator with the current time in seconds. */
  srandom (time (0));

  printf("Generating the instants (one '*' marker every %d instants)\n",
    NO_INSTANTS_BATCH);

  TimestampTz t = timestamptz_in("1999-12-31", -1);
  for (i = 0; i < MAX_INSTANTS; i++)
  {
    /* Generate the instant */
    if (i % NO_INSTANTS_BATCH == 0)
    {
      printf("*");
      fflush(stdout);
    }
    /* Use a random generator to set the gid of the route identifier in 
     * [0, 1000] */
    int gid = random() % 1000;
    Npoint *np = npoint_make(gid, 1);
    instants[i] = tnpointinst_make(np, t);
    free(np);
  }

  /* Print information about the instants */
  char *str = tnpoint_out((Temporal *) instants[MAX_INSTANTS - 1], 0);
  printf("\nNumber of instants: %d, Last value : %s\n",
    MAX_INSTANTS, str);

  /* Free memory */
  free(str);
  for (i = 0; i < MAX_INSTANTS; i++)
    free(instants[i]);

  /* Calculate the elapsed time */
  tm = clock() - tm;
  double time_taken = ((double) tm) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);

  /* Finalize MEOS */
  meos_finalize();

  /* Return */
  return EXIT_SUCCESS;
}
