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
 * @brief A simple program that generates a given number of tint instants
 * using temporal_in(string) and tinstant_make(value, timestamp) to compare
 * the runtime between two methods for creating temporal instants.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tint_benchmark tint_benchmark.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <postgres.h>
#include <utils/timestamp.h>
#include <meos.h>
#include <meos_internal.h>

#define MAX_INSTANTS 500000
#define MAX_LENGTH_INST 64

int main(void) {
  /* Initialize MEOS */
  meos_initialize();

  /* Measure runtime of temporal_in(string) */
  /* Get start time */
  clock_t time;
  time = clock();
  const char *input_str = "5@2025-01-01 12:00:00";
  Temporal *instants[MAX_INSTANTS] = {0};
  /* Create temporal instants */
  for (int i = 0; i < MAX_INSTANTS; i++) {
      instants[i] = temporal_in(input_str, T_TINT);
  }
  time = clock() - time;
  double time_taken = ((double) time) / CLOCKS_PER_SEC;
  printf("temporal_in() took %f seconds to execute\n", time_taken);
  for (int i = 0; i < MAX_INSTANTS; i++) {
      free(instants[i]);
  }

  /* Measure runtime of tinstant_make(value, timestamp) */
  time = clock();
  Temporal *instants2[MAX_INSTANTS] = {0};
  TimestampTz t = pg_timestamptz_in("2025-01-01 12:00:00", -1);
  for (int i = 0; i < MAX_INSTANTS; i++) {
      int value = i % 2 + 1;
      instants2[i] = (Temporal *) tinstant_make(value, T_TINT, t);
  }
  time = clock() - time;
  time_taken = ((double) time) / CLOCKS_PER_SEC;
  printf("tinstant_make() took %f seconds to execute\n", time_taken);
  for (int i = 0; i < MAX_INSTANTS; i++) {
      free(instants2[i]);
  }

  /* Finalize MEOS */
  meos_finalize();
  return 0;
}