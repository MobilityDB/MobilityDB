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
 * @brief A program that tests how the hash functions of a pose of the MEOS
 * API report a null pose under the noexit error handler
 * @details A public MEOS function tests its arguments at entry, so a binding
 * calling it with a null pointer receives an error it can raise in its host
 * language rather than a crash.
 *
 * The program verifies that #pose_hash_extended reports a null pose by
 * returning UINT64_MAX and setting #meos_errno, as #cbuffer_hash_extended
 * does, as #pose_hash reports one by returning UINT32_MAX, and that two equal
 * poses hash to the same value with no error left behind.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o pose_validity_test pose_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_pose.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_noexit_error_handler();

  Pose *pose1 = pose_in("Pose(Point(1 1),0.5)");
  Pose *pose2 = pose_in("Pose(Point(1 1),0.5)");
  assert(pose1); assert(pose2); assert(pose_eq(pose1, pose2));
  meos_errno_reset();

  /* A null pose is reported rather than read */
  uint64 h64 = pose_hash_extended(NULL, 1);
  printf("pose_hash_extended(NULL, 1): %llu, errno %d\n",
    (unsigned long long) h64, meos_errno());
  assert(h64 == UINT64_MAX); assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  uint32 h32 = pose_hash(NULL);
  printf("pose_hash(NULL): %u, errno %d\n", (unsigned) h32, meos_errno());
  assert(h32 == UINT32_MAX); assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  /* Two equal poses hash to the same value, with no error left behind */
  uint64 a = pose_hash_extended(pose1, 1), b = pose_hash_extended(pose2, 1);
  printf("pose_hash_extended(pose, 1) twice: %s, errno %d\n",
    a == b ? "equal" : "different", meos_errno());
  assert(a == b); assert(meos_errno() == 0);
  assert(pose_hash(pose1) == pose_hash(pose2)); assert(meos_errno() == 0);

  free(pose1); free(pose2);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
