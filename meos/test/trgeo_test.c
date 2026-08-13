/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief A simple program that tests trgeometry_in(), the public MEOS entry
 * point for reading a temporal rigid geometry from its text representation
 *
 * A trgeometry value carries a leading reference geometry ahead of its
 * temporal (pose) part. This test exercises trgeometry_in() directly on a
 * well-formed value and checks that the result round-trips through
 * trgeometry_out().
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o trgeo_test trgeo_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_rgeo.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS, using the noexit error handler so a parse failure
   * returns NULL instead of exiting the process */
  meos_initialize();
  meos_initialize_noexit_error_handler();
  meos_initialize_timezone("UTC");

  int result = 0;

  /* Well-formed trgeometry value: a reference geometry followed by the
   * ';'-delimited temporal (pose) part */
  const char *trgeo1_in =
    "Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 2),0.5)@2000-01-01";

  /* Temporal *trgeometry_in(const char *str); */
  Temporal *trgeo1 = trgeometry_in(trgeo1_in);
  if (! trgeo1)
  {
    printf("FAILED: trgeometry_in(%s) returned NULL\n", trgeo1_in);
    result = 1;
  }
  else
  {
    /* char *trgeometry_out(const Temporal *temp); */
    char *trgeo1_out = trgeometry_out(trgeo1);
    printf("trgeometry_in(%s): %s\n", trgeo1_in, trgeo1_out);

    /* Round-trip: parse the rendered representation again and check that
     * rendering it a second time yields the same string */
    Temporal *trgeo2 = trgeometry_in(trgeo1_out);
    if (! trgeo2)
    {
      printf("FAILED: trgeometry_in(%s) (round-trip) returned NULL\n",
        trgeo1_out);
      result = 1;
    }
    else
    {
      char *trgeo2_out = trgeometry_out(trgeo2);
      if (strcmp(trgeo1_out, trgeo2_out) != 0)
      {
        printf("FAILED: round-trip mismatch: %s != %s\n", trgeo1_out,
          trgeo2_out);
        result = 1;
      }
      else
        printf("OK: trgeometry_in/trgeometry_out round-trip matches\n");
      free(trgeo2); free(trgeo2_out);
    }
    free(trgeo1); free(trgeo1_out);
  }

  /* Finalize MEOS */
  meos_finalize();

  return result;
}
