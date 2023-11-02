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
 * @brief A simple program that uses the MEOS library for creating some
 * temporal values and output them in MF-JSON format.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o test_tstz_inout test_tstz_inout.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>  /* for printf */
#include <stdlib.h>   /* for free */
/* Include the MEOS API header */
#include <meos.h>

int main()
{
  /* Initialize MEOS */
  meos_initialize("Europe/Brussels");

  /* Input temporal points in WKT format */
  // char *tstz_in = "2000-01-01 10:00:00";
  // TimestampTz t = pg_timestamptz_in(tstz_in, -1);
  // char *tstz_out = pg_timestamptz_out(t);

  // printf("Timestamp in: %s\n", tstz_in);
  // printf("Timestamp out: %s\n", tstz_out);

  // char *p1_in = "[2000-01-01 10:00:00+02, 2000-01-02 10:00:00+02]";
  // char *p2_in = "[2002-01-01 10:00:00+02, 2002-01-02 10:00:00+02]";
  // Span *p1 = period_in(p1_in);
  // Span *p2 = period_in(p2_in);
  // SpanSet *p = union_span_span(p1, p2);
  // char *p_out = periodset_out(p);

  // printf("Period in: %s\n", p1_in);
  // printf("Period in: %s\n", p2_in);
  // printf("Period out: %s\n", p_out);

  char *gs1_in = "Point(1 1)";
  char *gs2_in = "Point(2 2)";
  GSERIALIZED *gs1 = geometry_in(gs1_in, -1);
  GSERIALIZED *gs2 = geography_in(gs2_in, -1);
  char *gs1_out = gserialized_as_ewkt(gs1, 3);
  char *gs2_out = gserialized_as_ewkt(gs2, 3);
  printf("Geometry in: %s\n", gs1_in);
  printf("Geography in: %s\n", gs2_in);
  printf("Geometry out: %s\n", gs1_out);
  printf("Geography out: %s\n", gs2_out);
  Set *set1 = geom_to_geomset(gs1);
  Set *set2 = geog_to_geogset(gs2);
  char *set1_out = geoset_as_ewkt(set1, 3);
  char *set2_out = geoset_as_ewkt(set2, 3);
  printf("Set out: %s\n", set1_out);
  printf("Set out: %s\n", set2_out);

  /* Clean up allocated objects */
  // free(ss_step); free(ss_step_mfjson);

  /* Finalize MEOS */
  meos_finalize();

  /* Return */
  return 0;
}
