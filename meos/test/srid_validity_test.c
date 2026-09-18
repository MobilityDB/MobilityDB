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
 * @brief A program that tests how the constructors and the SRID setters
 * report an invalid SRID under the noexit error handler.
 *
 * A public MEOS function that refuses an argument raises an error before it
 * returns its sentinel, so a binding calling it can tell an erroneous
 * argument from a missing value. The SRID_INVALID a failed SRID getter
 * returns is the value a binding most readily passes on by mistake.
 *
 * The program obtains SRID_INVALID as the error value of #geo_srid on a null
 * geometry, which the public headers do not otherwise name, and verifies
 * that every function refusing it returns NULL and sets #meos_errno. It also
 * verifies that #geog_in and #geom_to_geog refuse a coordinate system that
 * is not lon/lat with the same error, that #stbox_make reports a time span of
 * another type, and that a valid SRID still answers with no error left
 * behind.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o srid_validity_test srid_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_cbuffer.h>
#include <meos_pose.h>

/* Report the answer of a call and check that it refused its argument with
 * the expected error code */
static void
check_refused(const char *call, const void *result, int errcode)
{
  printf("%s: %s, errno %d\n", call, result ? "a value" : "NULL",
    meos_errno());
  assert(result == NULL);
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

  /* The SRID a failed getter returns, as a binding receives it */
  int32_t invalid = geo_srid(NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  Cbuffer *cb = cbuffer_in("Cbuffer(Point(1 1),0.5)");
  assert(cb);
  Pose *pose = pose_in("Pose(Point(1 1),0.5)");
  assert(pose);
  PoseChain *pc = posechain_in("PoseChain(Pose(Point(0 0),0),"
    "Pose(Point(1 0),1))");
  assert(pc);
  Span *ispan = intspan_in("[1, 3]");
  assert(ispan);
  meos_errno_reset();

  /* Each function refusing an invalid SRID reports it */
  check_refused("box3d_make(..., SRID_INVALID)",
    box3d_make(1, 2, 1, 2, 0, 0, invalid), MEOS_ERR_INVALID_ARG_VALUE);
  check_refused("geompoint_make2d(SRID_INVALID, ...)",
    geompoint_make2d(invalid, 1, 1), MEOS_ERR_INVALID_ARG_VALUE);
  check_refused("geogpoint_make2d(SRID_INVALID, ...)",
    geogpoint_make2d(invalid, 1, 1), MEOS_ERR_INVALID_ARG_VALUE);
  check_refused("geompoint_make3dz(SRID_INVALID, ...)",
    geompoint_make3dz(invalid, 1, 1, 1), MEOS_ERR_INVALID_ARG_VALUE);
  check_refused("geogpoint_make3dz(SRID_INVALID, ...)",
    geogpoint_make3dz(invalid, 1, 1, 1), MEOS_ERR_INVALID_ARG_VALUE);
  check_refused("stbox_make(..., SRID_INVALID, ...)",
    stbox_make(true, false, false, invalid, 1, 2, 1, 2, 0, 0, NULL),
    MEOS_ERR_INVALID_ARG_VALUE);
  check_refused("cbuffer_set_srid(cb, SRID_INVALID)",
    cbuffer_set_srid(cb, invalid), MEOS_ERR_INVALID_ARG_VALUE);
  check_refused("pose_set_srid(pose, SRID_INVALID)",
    pose_set_srid(pose, invalid), MEOS_ERR_INVALID_ARG_VALUE);
  check_refused("posechain_set_srid(pc, SRID_INVALID)",
    posechain_set_srid(pc, invalid), MEOS_ERR_INVALID_ARG_VALUE);

  /* A geography refuses a coordinate system that is not lon/lat, whether it
   * is read from text or converted from a geometry */
  GSERIALIZED *planar = geom_in("SRID=3857;Point(1 1)", -1);
  assert(planar);
  check_refused("geog_in(\"SRID=3857;Point(1 1)\", -1)",
    geog_in("SRID=3857;Point(1 1)", -1), MEOS_ERR_INVALID_ARG_VALUE);
  check_refused("geom_to_geog(SRID=3857 point)", geom_to_geog(planar),
    MEOS_ERR_INVALID_ARG_VALUE);

  /* A time span of another type is reported rather than read as a period */
  check_refused("stbox_make(..., [1, 3])",
    stbox_make(true, false, false, 0, 1, 2, 1, 2, 0, 0, ispan),
    MEOS_ERR_INVALID_ARG_TYPE);

  /* A valid SRID still answers, and the guards leave no error behind */
  GSERIALIZED *gs = geompoint_make2d(4326, 1, 1);
  printf("geompoint_make2d(4326, 1, 1): %s, errno %d\n",
    gs ? "a value" : "NULL", meos_errno());
  assert(gs != NULL);
  assert(meos_errno() == 0);
  STBox *box = stbox_make(true, false, false, 4326, 1, 2, 1, 2, 0, 0, NULL);
  printf("stbox_make(..., 4326, ...): %s, errno %d\n",
    box ? "a value" : "NULL", meos_errno());
  assert(box != NULL);
  assert(meos_errno() == 0);
  Pose *pose2 = pose_set_srid(pose, 4326);
  printf("pose_set_srid(pose, 4326): %s, errno %d\n",
    pose2 ? "a value" : "NULL", meos_errno());
  assert(pose2 != NULL);
  assert(meos_errno() == 0);

  free(gs); free(box); free(pose2); free(planar);
  free(cb); free(pose); free(pc); free(ispan);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
