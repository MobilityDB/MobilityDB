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
 * @brief A simple program that generates a given number of tgeompoint instants,
 * assembles the instants into a sequence at the end of the generation process,
 * and outputs the number of instants and the distance travelled.
 *
 * The instants are generated so they are not redundant, that is, all input
 * instants will appear in the final sequence.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tpoint_assemble tpoint_assemble.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <meos.h>
#include <meos_geo.h>

/*
 * Define whether geometric or geodetic points are used. Possible values are
 * - false => use geometric points
 * - true => use geodetic points
 */
#define GEODETIC false
/* Maximum number of instants */
#define MAX_INSTANTS 1000000
/* Number of instants in a batch for printing a marker */
#define NO_INSTANTS_BATCH 10000
/* Maximum length in characters of the input instant */
#define MAX_LENGTH_INST 64

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();

  /* Get start time */
  clock_t time;
  time = clock();

  /* Input instants that are accumulated */
  TInstant *instants[MAX_INSTANTS] = {0};
  /* Sequence constructed from the input instants */
  Temporal *seq = NULL;
  /* Interval to add */
  Interval *oneday = interval_in("1 day", -1);
  /* Iterator variable */
  int i;

  printf("Reading the instants (one '*' marker every %d instants)\n",
    NO_INSTANTS_BATCH);

  TimestampTz t = timestamptz_in("1999-12-31", -1);
  for (i = 0; i < MAX_INSTANTS; i++)
  {
    if (i % NO_INSTANTS_BATCH == 0)
    {
      printf("*");
      fflush(stdout);
    }
    t = add_timestamptz_interval(t, oneday);
    int value = i % 2 + 1;
#if GEODETIC == true
    GSERIALIZED *gs = geogpoint_make2d(4326, value, value);
#else
    GSERIALIZED *gs = geogpoint_make2d(5676, value, value);
#endif
    instants[i] = tpointinst_make(gs, t);
    free(gs);
  }

  printf("\nAssembing the instants ...\n");

  seq = (Temporal *) tsequence_make((const TInstant **) instants, MAX_INSTANTS,
    true, true, LINEAR, true);

  /* Print information about the sequence */
  printf("\nNumber of instants: %d, Distance : %lf\n",
    temporal_num_instants(seq), tpoint_length(seq));

  /* Free memory */
  free(seq); free(oneday);
  for (i = 0; i < MAX_INSTANTS; i++)
    free(instants[i]);

  /* Calculate the elapsed time */
  time = clock() - time;
  double time_taken = ((double) time) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);
  printf("Accumulating the instants and constructing the sequence at the end\n");

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
