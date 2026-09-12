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
 * @brief A program that tests how the bounding box operators of two
 * spatiotemporal values, and of a temporal point cloud, report an erroneous
 * operand under the noexit error handler.
 *
 * A public MEOS function tests the conditions its internal form asserts, so a
 * binding calling it with a null pointer or a value of another type receives
 * an error it can raise in its host language. The topological and position
 * operators of two spatiotemporal values, and those of a temporal point
 * cloud, delegate to one generic helper each; the helper for a
 * spatiotemporal value and a box already tests its operands, while these
 * read theirs without a test.
 *
 * The program verifies that a null operand, a temporal value of another type
 * and two values of different SRID are reported by setting #meos_errno, and
 * that a valid call still answers with no error left behind.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o box_validity_test box_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_pointcloud.h>

/* Report the answer of a predicate and check the error code it leaves */
static void
check(const char *call, bool result, bool expected, int errcode)
{
  printf("%s: %s, errno %d\n", call, result ? "true" : "false",
    meos_errno());
  assert(result == expected);
  assert(meos_errno() == errcode);
  meos_errno_reset();
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Temporal *tpt = tgeompoint_in("[Point(1 1)@2001-01-01, "
    "Point(2 2)@2001-01-02]");
  assert(tpt);
  Temporal *tpt3857 = tgeompoint_in("SRID=3857;[Point(1 1)@2001-01-01, "
    "Point(2 2)@2001-01-02]");
  assert(tpt3857);
  Temporal *tint = tint_in("[1@2001-01-01, 2@2001-01-02]");
  assert(tint);
  TPCBox *box = tpcbox_in("TPCBOX(XT(((1,1),(3,3)),[2024-01-01,2024-01-02]), 1)");
  assert(box);
  meos_errno_reset();

  /* A null operand of the spatiotemporal operators is reported rather than
   * read, whichever side it is on */
  check("overlaps_tspatial_tspatial(NULL, tpt)",
    overlaps_tspatial_tspatial(NULL, tpt), false, MEOS_ERR_INVALID_ARG);
  check("left_tspatial_tspatial(tpt, NULL)",
    left_tspatial_tspatial(tpt, NULL), false, MEOS_ERR_INVALID_ARG);

  /* A temporal value of another type is reported rather than read as a
   * spatiotemporal value */
  check("overlaps_tspatial_tspatial(tint, tpt)",
    overlaps_tspatial_tspatial(tint, tpt), false, MEOS_ERR_INVALID_ARG_TYPE);

  /* Two values of different SRID are reported */
  check("overlaps_tspatial_tspatial(tpt, tpt3857)",
    overlaps_tspatial_tspatial(tpt, tpt3857), false,
    MEOS_ERR_INVALID_ARG_VALUE);

  /* A null temporal point cloud is reported rather than read */
  check("overlaps_tpcbox_tpointcloud(box, NULL)",
    overlaps_tpcbox_tpointcloud(box, NULL), false, MEOS_ERR_INVALID_ARG);
  check("overlaps_tpointcloud_tpcbox(NULL, box)",
    overlaps_tpointcloud_tpcbox(NULL, box), false, MEOS_ERR_INVALID_ARG);
  check("overlaps_tpointcloud_tpointcloud(NULL, NULL)",
    overlaps_tpointcloud_tpointcloud(NULL, NULL), false,
    MEOS_ERR_INVALID_ARG);

  /* A valid call still answers, and the guards leave no error behind */
  check("overlaps_tspatial_tspatial(tpt, tpt)",
    overlaps_tspatial_tspatial(tpt, tpt), true, 0);

  free(tpt); free(tpt3857); free(tint); free(box);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
