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
 * @brief A program that tests the WKT input functions of the geo API on
 * malformed input, and the text representation of the PostGIS @p box3d type.
 *
 * A parse failure is only observable once
 * #meos_initialize_noexit_error_handler is installed — the handler every
 * language binding uses, since a binding must return an exception to its host
 * rather than terminate it. Under it the parse-failure branch of the input
 * functions is reached, and the program verifies that they return NULL and set
 * #meos_errno rather than dereferencing the null geometry the parser leaves
 * behind. The default handler ends the process inside the failing check, so
 * this path is exercised only here.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o geo_test geo_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* A malformed WKT geography returns NULL and sets the error status, rather
   * than dereferencing the null geometry the parser leaves in its result */
  meos_errno_reset();
  GSERIALIZED *bad_geog = geog_in("this is not well-known text", -1);
  printf("geog_in(bad WKT): %s, errno %d\n", bad_geog ? "non-NULL" : "NULL",
    meos_errno());
  assert(bad_geog == NULL);
  assert(meos_errno() != 0);

  /* Same for the geometry/geography from-text constructor */
  meos_errno_reset();
  GSERIALIZED *bad_geo = geo_from_text("this is not well-known text", 0);
  printf("geo_from_text(bad WKT): %s, errno %d\n",
    bad_geo ? "non-NULL" : "NULL", meos_errno());
  assert(bad_geo == NULL);
  assert(meos_errno() != 0);

  /* A valid WKT geography still parses */
  meos_errno_reset();
  GSERIALIZED *good = geog_in("Point(1 1)", -1);
  printf("geog_in(\"Point(1 1)\"): %s, errno %d\n",
    good ? "non-NULL" : "NULL", meos_errno());
  assert(good != NULL);
  assert(meos_errno() == 0);
  free(good);

  /* The text representation of a box3d is the one PostGIS gives the type it
   * declares, so that the value crosses between a PostgreSQL session and any
   * other binding. PostGIS prints the ordinates with at most 15 decimal
   * digits, the value of maxdd that reproduces its output */
  const char *box3d_strs[][2] = {
    /* Three dimensions, the form a PostgreSQL session prints for
     * stbox 'STBOX Z((1,1,1),(5,5,5))'::box3d */
    {"BOX3D(1 1 1,5 5 5)", "BOX3D(1 1 1,5 5 5)"},
    /* The Z ordinates may be left out on input and are then zero, and they
     * are printed back, as PostGIS does */
    {"BOX3D(1 2,3 4)", "BOX3D(1 2 0,3 4 0)"},
    /* Negative and fractional ordinates */
    {"BOX3D(-1.5 -2.25 -3.125,5 5 5)", "BOX3D(-1.5 -2.25 -3.125,5 5 5)"},
    /* The bounds are ordered on input */
    {"BOX3D(5 5 5,1 1 1)", "BOX3D(1 1 1,5 5 5)"},
    /* The format written by previous versions is still read, and the SRID it
     * carries is not printed back, as PostGIS never prints one */
    {"BOX3D((1,2,3),(4,5,6))", "BOX3D(1 2 3,4 5 6)"},
    {"SRID=4326;BOX3D((1,2,3),(4,5,6))", "BOX3D(1 2 3,4 5 6)"}
  };
  for (size_t i = 0; i < sizeof(box3d_strs) / sizeof(box3d_strs[0]); i++)
  {
    meos_errno_reset();
    BOX3D *box = box3d_in(box3d_strs[i][0]);
    assert(box != NULL);
    assert(meos_errno() == 0);
    char *str = box3d_out(box, 15);
    printf("box3d_in(\"%s\") -> \"%s\"\n", box3d_strs[i][0], str);
    assert(strcmp(str, box3d_strs[i][1]) == 0);
    /* The output of box3d_out is read back by box3d_in */
    BOX3D *box1 = box3d_in(str);
    assert(box1 != NULL);
    char *str1 = box3d_out(box1, 15);
    assert(strcmp(str, str1) == 0);
    free(str); free(str1); free(box); free(box1);
  }

  /* A box3d whose SRID is set still prints without one, as PostGIS does */
  meos_errno_reset();
  BOX3D *box_srid = box3d_make(1, 5, 1, 5, 1, 5, 4326);
  assert(box_srid != NULL);
  char *str_srid = box3d_out(box_srid, 15);
  printf("box3d_out(box3d_make(..., 4326)): \"%s\"\n", str_srid);
  assert(strcmp(str_srid, "BOX3D(1 1 1,5 5 5)") == 0);
  free(str_srid); free(box_srid);

  /* A malformed box3d returns NULL and sets the error status */
  meos_errno_reset();
  BOX3D *bad_box3d = box3d_in("BOX3D(1 2 3)");
  printf("box3d_in(\"BOX3D(1 2 3)\"): %s, errno %d\n",
    bad_box3d ? "non-NULL" : "NULL", meos_errno());
  assert(bad_box3d == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
