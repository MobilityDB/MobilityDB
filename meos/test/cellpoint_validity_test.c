/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Test how the functions giving the cell of a point report an error
 * @details A program that tests how the functions giving the cell of a point
 * in
 * the H3, QUADBIN and S2 grids report an erroneous argument under the noexit
 * error handler.
 *
 * A public MEOS function tests the conditions it relies on, so a binding
 * calling it with an argument outside its contract receives an error it can
 * raise in its host language rather than an answer computed from a value the
 * function does not define.
 *
 * The program verifies that #geo_to_h3index_cell, #geo_to_quadbin_cell and
 * #geo_to_s2cell_cell report a null geometry with MEOS_ERR_INVALID_ARG, and a
 * geometry that is not a point, an empty point and a resolution outside the
 * range of their grid with MEOS_ERR_INVALID_ARG_VALUE, by returning no cell;
 * that #geo_to_h3index_set reports a resolution outside the range of its grid
 * the same way; and that each of them answers a point at a resolution of its
 * grid with no error left behind.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o cellpoint_validity_test cellpoint_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_h3.h>
#include <meos_quadbin.h>
#include <meos_s2cell.h>

/**
 * @brief Assert that a call answers no cell and reports an error number, then
 * reset the error number
 */
static void
refused(const char *call, bool nocell, int errnum)
{
  printf("%s: %s, errno %d\n", call, nocell ? "no cell" : "a cell",
    meos_errno());
  assert(nocell);
  assert(meos_errno() == errnum);
  meos_errno_reset();
  return;
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_noexit_error_handler();

  /* A point, a line and an empty point, as geometries for H3 and QUADBIN and
   * as geographies for S2 */
  GSERIALIZED *pt = geom_in("SRID=4326;Point(4.35 50.85)", -1);
  GSERIALIZED *line = geom_in(
    "SRID=4326;LINESTRING(4.35 50.85, 4.36 50.86)", -1);
  GSERIALIZED *empty = geom_in("SRID=4326;POINT EMPTY", -1);
  GSERIALIZED *gpt = geog_in("SRID=4326;Point(4.35 50.85)", -1);
  GSERIALIZED *gline = geog_in(
    "SRID=4326;LINESTRING(4.35 50.85, 4.36 50.86)", -1);
  GSERIALIZED *gempty = geog_in("SRID=4326;POINT EMPTY", -1);
  assert(pt && line && empty && gpt && gline && gempty);
  meos_errno_reset();

  /* H3 */
  refused("geo_to_h3index_cell(NULL, 7)",
    geo_to_h3index_cell(NULL, 7) == 0, MEOS_ERR_INVALID_ARG);
  refused("geo_to_h3index_cell(line, 7)",
    geo_to_h3index_cell(line, 7) == 0, MEOS_ERR_INVALID_ARG_VALUE);
  refused("geo_to_h3index_cell(empty, 7)",
    geo_to_h3index_cell(empty, 7) == 0, MEOS_ERR_INVALID_ARG_VALUE);
  refused("geo_to_h3index_cell(point, 16)",
    geo_to_h3index_cell(pt, 16) == 0, MEOS_ERR_INVALID_ARG_VALUE);
  refused("geo_to_h3index_cell(point, -1)",
    geo_to_h3index_cell(pt, -1) == 0, MEOS_ERR_INVALID_ARG_VALUE);
  Set *cells = geo_to_h3index_set(pt, 16);
  refused("geo_to_h3index_set(point, 16)", cells == NULL,
    MEOS_ERR_INVALID_ARG_VALUE);

  /* QUADBIN */
  refused("geo_to_quadbin_cell(NULL, 10)",
    geo_to_quadbin_cell(NULL, 10) == 0, MEOS_ERR_INVALID_ARG);
  refused("geo_to_quadbin_cell(line, 10)",
    geo_to_quadbin_cell(line, 10) == 0, MEOS_ERR_INVALID_ARG_VALUE);
  refused("geo_to_quadbin_cell(empty, 10)",
    geo_to_quadbin_cell(empty, 10) == 0, MEOS_ERR_INVALID_ARG_VALUE);
  refused("geo_to_quadbin_cell(point, 27)",
    geo_to_quadbin_cell(pt, 27) == 0, MEOS_ERR_INVALID_ARG_VALUE);
  refused("geo_to_quadbin_cell(point, -1)",
    geo_to_quadbin_cell(pt, -1) == 0, MEOS_ERR_INVALID_ARG_VALUE);

  /* S2 */
  refused("geo_to_s2cell_cell(NULL, 10)",
    geo_to_s2cell_cell(NULL, 10) == 0, MEOS_ERR_INVALID_ARG);
  refused("geo_to_s2cell_cell(line, 10)",
    geo_to_s2cell_cell(gline, 10) == 0, MEOS_ERR_INVALID_ARG_VALUE);
  refused("geo_to_s2cell_cell(empty, 10)",
    geo_to_s2cell_cell(gempty, 10) == 0, MEOS_ERR_INVALID_ARG_VALUE);
  refused("geo_to_s2cell_cell(point, 31)",
    geo_to_s2cell_cell(gpt, 31) == 0, MEOS_ERR_INVALID_ARG_VALUE);
  refused("geo_to_s2cell_cell(point, -1)",
    geo_to_s2cell_cell(gpt, -1) == 0, MEOS_ERR_INVALID_ARG_VALUE);

  /* A point at a resolution of each grid answers its cell, with no error left
   * behind */
  H3Index h3 = geo_to_h3index_cell(pt, 7);
  printf("geo_to_h3index_cell(point, 7): %" PRIx64 ", errno %d\n",
    (uint64_t) h3, meos_errno());
  assert(h3 == UINT64_C(0x871fa4418ffffff) && meos_errno() == 0);
  cells = geo_to_h3index_set(pt, 7);
  printf("geo_to_h3index_set(point, 7): %d values, errno %d\n",
    cells ? set_num_values(cells) : -1, meos_errno());
  assert(cells != NULL && set_num_values(cells) >= 1 && meos_errno() == 0);
  Quadbin qb = geo_to_quadbin_cell(pt, 10);
  printf("geo_to_quadbin_cell(point, 10): %" PRIx64 ", errno %d\n",
    (uint64_t) qb, meos_errno());
  assert(qb == UINT64_C(0x48a6227affffffff) && meos_errno() == 0);
  S2CellId s2 = geo_to_s2cell_cell(gpt, 10);
  printf("geo_to_s2cell_cell(point, 10): %" PRIx64 ", errno %d\n",
    (uint64_t) s2, meos_errno());
  assert(s2 != 0 && meos_errno() == 0);

  free(cells);
  free(pt); free(line); free(empty); free(gpt); free(gline); free(gempty);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
