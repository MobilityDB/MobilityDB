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
 * @brief A program that checks that the restrictions of a temporal point
 * cloud point and patch to a point cloud box refuse a box of another schema,
 * as the overlaps of their boxes refuses it, and answer a box of their own
 * schema
 * @details The schema gives the dimensions of a point their meaning, so a
 * value and a box of two schemas have no common extent: the restriction
 * reports the error rather than answering that the value lies outside the
 * box. A minus whose value lies in the box answers NULL too, so the error
 * number tells the two answers apart.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tpcbox_restrict_validity_test tpcbox_restrict_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>

/* A pcpoint of pcid 1 holding X=1.0 Y=2.0 Z=3.0, and a pcpatch of pcid 1
 * holding the two points (1,1,1) and (2,2,2), each at one timestamp, laid
 * out by schema 1 of meos/src/pointcloud/pointcloud_schemas.xml: three int32
 * dimensions at scale 0.01 */
#define TPCPOINT_IN \
  "010100000064000000C80000002C010000@2024-01-01"
#define TPCPATCH_IN \
  "01010000000000000002000000640000006400000064000000C8000000C8000000" \
  "C8000000@2024-01-01"

static void
check(const char *call, Temporal *result, bool expected, int errcode)
{
  printf("%s: %s, errno %d\n", call, result ? "a value" : "NULL",
    meos_errno());
  assert((result != NULL) == expected);
  assert(meos_errno() == errcode);
  meos_errno_reset();
  free(result);
}

int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  PCDimensionSpec dims1[3] = {
    { "X", NULL, 1, "int32_t", 0.01, 0, true },
    { "Y", NULL, 2, "int32_t", 0.01, 0, true },
    { "Z", NULL, 3, "int32_t", 0.01, 0, true }
  };
  assert(meos_pc_schema_register_dims(1, 0, "none", dims1, 3));
  Temporal *pt = temporal_in(TPCPOINT_IN, T_TPCPOINT);
  Temporal *pa = temporal_in(TPCPATCH_IN, T_TPCPATCH);
  Temporal *tint = tint_in("1@2024-01-01");
  TPCBox *box1 = tpcbox_in("TPCBOX(XT(((0,0),(5,5)),[2024-01-01,2024-01-02]), 1)");
  TPCBox *box2 = tpcbox_in("TPCBOX(XT(((0,0),(5,5)),[2024-01-01,2024-01-02]), 2)");
  assert(pt); assert(pa); assert(tint); assert(box1); assert(box2);
  meos_errno_reset();

  /* A box of the schema of the value: the value lies in it */
  check("tpcpoint_at_tpcbox(pt, box1)",
    tpcpoint_at_tpcbox(pt, box1, true), true, 0);
  check("tpcpoint_minus_tpcbox(pt, box1)",
    tpcpoint_minus_tpcbox(pt, box1, true), false, 0);
  check("tpcpatch_at_tpcbox(pa, box1)",
    tpcpatch_at_tpcbox(pa, box1, true), true, 0);
  check("tpcpatch_minus_tpcbox(pa, box1)",
    tpcpatch_minus_tpcbox(pa, box1, true), false, 0);

  /* A box of another schema is reported, by at and minus alike */
  check("tpcpoint_at_tpcbox(pt, box2)",
    tpcpoint_at_tpcbox(pt, box2, true), false, MEOS_ERR_INVALID_ARG_VALUE);
  check("tpcpoint_minus_tpcbox(pt, box2)",
    tpcpoint_minus_tpcbox(pt, box2, true), false, MEOS_ERR_INVALID_ARG_VALUE);
  check("tpcpatch_at_tpcbox(pa, box2)",
    tpcpatch_at_tpcbox(pa, box2, true), false, MEOS_ERR_INVALID_ARG_VALUE);
  check("tpcpatch_minus_tpcbox(pa, box2)",
    tpcpatch_minus_tpcbox(pa, box2, true), false, MEOS_ERR_INVALID_ARG_VALUE);

  /* A null value and a value of another type are reported rather than read */
  check("tpcpoint_at_tpcbox(NULL, box1)",
    tpcpoint_at_tpcbox(NULL, box1, true), false, MEOS_ERR_INVALID_ARG);
  check("tpcpatch_minus_tpcbox(NULL, box1)",
    tpcpatch_minus_tpcbox(NULL, box1, true), false, MEOS_ERR_INVALID_ARG);
  check("tpcpoint_at_tpcbox(tint, box1)",
    tpcpoint_at_tpcbox(tint, box1, true), false, MEOS_ERR_INVALID_ARG_TYPE);
  check("tpcpatch_at_tpcbox(tint, box1)",
    tpcpatch_at_tpcbox(tint, box1, true), false, MEOS_ERR_INVALID_ARG_TYPE);

  free(pt); free(pa); free(tint); free(box1); free(box2);
  meos_finalize();
  printf("tpcbox_restrict_validity_test: OK\n");
  return EXIT_SUCCESS;
}
