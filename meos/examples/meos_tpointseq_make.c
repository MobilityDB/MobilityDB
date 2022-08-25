/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief A simple program that uses the MEOS library for creating some
 * temporal values and output them in MF-JSON format.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o meos_tpointseq_make meos_tpointseq_make.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>  /* for printf */

/* Include the MEOS API header */
#include "meos.h"

#define MAX_COUNT 20

int main()
{
  double xcoords[MAX_COUNT] = {1, 2, 1, 2};
  double ycoords[MAX_COUNT] = {1, 2, 1, 2};
  double zcoords[MAX_COUNT] = {1, 2, 1, 2};
  char *times_str[MAX_COUNT] = {"2000-01-01 00:00:00", "2000-01-02 00:00:00",
    "2000-01-03 00:00:00", "2000-01-04 00:00:00"};
  TimestampTz times[MAX_COUNT];

  /* Initialize MEOS */
  meos_initialize();

  for (int i = 0; i < 4; i++)
    times[i] = pg_timestamptz_in(times_str[i], -1);

  /* Input temporal points in WKT format */
  TSequence *seq1 = tpointseq_make_coords(xcoords, ycoords, zcoords, times,
    4, 5676, false, true, true, true, true);
  TSequence *seq2 = tpointseq_make_coords(xcoords, ycoords, NULL, times,
    4, 5676, false, true, true, true, true);
  TSequence *seq3 = tpointseq_make_coords(xcoords, ycoords, NULL, times,
    4, 4326, true, true, true, true, true);

  /* Print result in WKT */
  char *seq1_wkt = tpoint_as_ewkt((Temporal *) seq1, 2);
  char *seq2_wkt = tpoint_as_ewkt((Temporal *) seq2, 2);
  char *seq3_wkt = tpoint_as_ewkt((Temporal *) seq3, 2);
  printf("\nseql: %s\nseq2: %s\nseq3: %s\n\n",
    seq1_wkt, seq2_wkt, seq3_wkt);

  /* Clean up allocated objects */
  free(seq1); free(seq1_wkt);
  free(seq2); free(seq2_wkt);
  free(seq3); free(seq3_wkt);

  /* Finalize MEOS */
  meos_finish();

  /* Return */
  return 0;
}
