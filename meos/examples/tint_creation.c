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
 * @brief A simple program that compares the execution time to create a given
 * number of temporal integer instants using two different methods 
 * - using the `temporal_in(string)` function
 * - using the `tinstant_make(value, timestamp)` function
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tint_benchmark tint_benchmark.c -L/usr/local/lib -lmeos
 * @endcode
 *
 * The output of the program is as follows
 * @code
 * Number of instants to generate: 500000
 * The generation using 'temporal_in' took 0.389140 seconds
 * The generation using 'tinstant_make()' took 0.022794 seconds
 * @endcode
 */

/* C */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>

/* Maximum number of instants generated */
#define MAX_NO_INSTS 500000
/* Maximum length in characters of an instant string */
#define MAX_LEN_INST 64

int main(void)
{
  /* Input string */
  const char *input_str = "5@2025-01-01 12:00:00";
  /* Array to hold the instants created */
  Temporal *instants[MAX_NO_INSTS] = {0};

  /* Initialize MEOS */
  meos_initialize();

  /* Get start time */
  clock_t time = clock();

  /* Create temporal instants from a string input */
  printf("Number of instants to generate: %d\n", MAX_NO_INSTS);
  for (int i = 0; i < MAX_NO_INSTS; i++)
    instants[i] = temporal_in(input_str, T_TINT);
  
  time = clock() - time;
  double time_taken = ((double) time) / CLOCKS_PER_SEC;
  printf("The generation using 'temporal_in()' took %f seconds\n", time_taken);
  for (int i = 0; i < MAX_NO_INSTS; i++)
    free(instants[i]);

  /* Create temporal instants using the constructor */
  time = clock();
  TimestampTz t = pg_timestamptz_in("2025-01-01 12:00:00", -1);
  for (int i = 0; i < MAX_NO_INSTS; i++)
  {
    int value = i % 2 + 1;
    instants[i] = (Temporal *) tinstant_make(value, T_TINT, t);
  }
  time = clock() - time;
  time_taken = ((double) time) / CLOCKS_PER_SEC;
  printf("The generation using 'tinstant_make()' took %f seconds\n", time_taken);
  for (int i = 0; i < MAX_NO_INSTS; i++)
    free(instants[i]);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}