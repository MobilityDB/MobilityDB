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
 * gcc -Wall -g -I/usr/local/include -o meos_expand_tfloat meos_expand_tfloat.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <meos.h>

/* Maximum number of instants */
#define MAX_INSTANTS 1000000

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize(NULL);

  /* Get start time */
  clock_t tm;
  tm = clock();

  /* Expandable sequence */
  Temporal *seq = NULL;
  /* Interval to add */
  Interval *oneday = pg_interval_in("1 day", -1);
  /* Iterator variable */
  int i;

  TimestampTz t = pg_timestamptz_in("1999-12-31", -1);
  for (i = 0; i < MAX_INSTANTS; i++)
  {
    t = pg_timestamp_pl_interval(t, oneday);
    TInstant *inst = tfloatinst_make(i % 2 + 1, t);
    if (! seq)
      seq = (Temporal *) tsequence_make_exp((const TInstant **) &inst, 1, 2,
        true, true, LINEAR, false);
    else
      seq = temporal_append_tinstant((Temporal *) seq, (TInstant *) inst, true);
    if (i >= 2)
      free(inst);
  }

  /* Print information about the sequence */
  printf("Number of instants: %d, Time-weighted average: %lf\n",
    temporal_num_instants(seq), tnumber_twavg(seq));

  /* Calculate the elapsed time */
  tm = clock() - tm;
  double time_taken = ((double) tm) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);
  printf("Accumulating the instants and constructing the sequence at the end\n");

  /* Free memory */
  free(seq);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
