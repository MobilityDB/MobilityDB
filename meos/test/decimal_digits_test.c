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
 * @brief A program that tests that a count of zero decimal digits is
 * accepted wherever a function tests a decimal-digit count.
 *
 * A count of decimal digits says how many digits a value keeps after the
 * decimal point, and zero keeps none; tspatial_as_text, geo_round and
 * posearr_round accept it, as every output and rounding function of MEOS
 * does. The program verifies that pcpoint_as_hexwkb and pcpatch_as_hexwkb,
 * which hand their hex output a count of zero, answer the hex their input
 * reads back with no error left behind; that tspatial_out and pose_round
 * answer for a count of zero; and that a negative count is still reported
 * with MEOS_ERR_INVALID_ARG_VALUE.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o decimal_digits_test decimal_digits_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_pointcloud.h>
#include <meos_pose.h>

/* A pcpoint of pcid 1 holding X=1.0 Y=2.0 Z=3.0, and a pcpatch of pcid 1
 * holding the two points (1,1,1) and (2,2,2), in the hex WKB pgPointCloud
 * serializes, the form that carries no schema, as tpointcloud_test reads
 * them */
#define PCPOINT_HEX \
  "2300000001000000000000000000F03F0000000000000040000000000000084000" \
  "0000"
#define PCPATCH_HEX \
  "4F000000010000000000000002000000000000000000F03F000000000000F03F00" \
  "0000000000F03F0000000000000040000000000000004000000000000000400000" \
  "00000000000000000000000000"

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* A pcpoint writes the hex its input reads back */
  Pcpoint *pt = pcpoint_from_hexwkb(PCPOINT_HEX);
  assert(pt && meos_errno() == 0);
  char *hex1 = pcpoint_as_hexwkb(pt);
  printf("pcpoint_as_hexwkb: %s, errno %d\n", hex1 ? hex1 : "NULL",
    meos_errno());
  assert(hex1 && meos_errno() == 0);
  Pcpoint *pt2 = pcpoint_from_hexwkb(hex1);
  char *hex2 = pt2 ? pcpoint_as_hexwkb(pt2) : NULL;
  assert(hex2 && strcmp(hex1, hex2) == 0);
  free(pt); free(pt2); free(hex1); free(hex2);

  /* So does a pcpatch */
  Pcpatch *pa = pcpatch_from_hexwkb(PCPATCH_HEX);
  assert(pa && meos_errno() == 0);
  hex1 = pcpatch_as_hexwkb(pa);
  printf("pcpatch_as_hexwkb: %s, errno %d\n", hex1 ? hex1 : "NULL",
    meos_errno());
  assert(hex1 && meos_errno() == 0);
  Pcpatch *pa2 = pcpatch_from_hexwkb(hex1);
  hex2 = pa2 ? pcpatch_as_hexwkb(pa2) : NULL;
  assert(hex2 && strcmp(hex1, hex2) == 0);
  free(pa); free(pa2); free(hex1); free(hex2);

  /* A spatiotemporal value is written with no decimal digits */
  Temporal *temp = tgeompoint_in(
    "[Point(1.25 2.75)@2001-01-01, Point(2 2)@2001-01-02]");
  assert(temp);
  char *str = tspatial_out(temp, 0);
  printf("tspatial_out(temp, 0): %s, errno %d\n", str ? str : "NULL",
    meos_errno());
  assert(str && meos_errno() == 0);
  free(str);
  str = tspatial_out(temp, -1);
  printf("tspatial_out(temp, -1): %s, errno %d\n", str ? "a value" : "NULL",
    meos_errno());
  assert(str == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  free(temp);

  /* A pose is rounded to no decimal digits */
  Pose *pose = pose_in("Pose(Point(1.25 2.75),0.5)");
  assert(pose);
  Pose *rounded = pose_round(pose, 0);
  str = rounded ? pose_out(rounded, 15) : NULL;
  printf("pose_round(pose, 0): %s, errno %d\n", str ? str : "NULL",
    meos_errno());
  assert(rounded && meos_errno() == 0);
  free(str); free(rounded);
  rounded = pose_round(pose, -1);
  printf("pose_round(pose, -1): %s, errno %d\n", rounded ? "a value" : "NULL",
    meos_errno());
  assert(rounded == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  free(pose);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
