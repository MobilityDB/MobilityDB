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
 * @brief A program that tests the distance between two spatial sets of the
 * circular buffer, network point, pose and pose chain families
 * @details Each family answers the distance between the boxes of its two sets,
 * as distance_geoset_geoset answers it for two geo sets: two circular buffers
 * of radius 1 centred 5 apart are 3 apart, two poses at (0, 0) and (3, 4) are
 * 5 apart, and a set is 0 from itself. A NULL set, a set of another type and
 * two sets of different SRIDs are refused. The routes of the network points
 * come from the ways file the environment variable MEOS_WAYS_CSV names, by
 * default the one of meos/examples, read from meos/test.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o spatialset_distance_test spatialset_distance_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_cbuffer.h>
#include <meos_npoint.h>
#include <meos_pose.h>

/* Check that a call answered its error value with the expected error code */
#define REFUSED(label, iserrval, code) \
  do { \
    bool errval = (iserrval); \
    printf("%s: %s, errno %d\n", (label), errval ? "error value" : "a value", \
      meos_errno()); \
    assert(errval && meos_errno() == (code)); \
    meos_errno_reset(); \
  } while (0)

/* Check that a distance is the expected one */
#define DISTANCE(label, dist, expected) \
  do { \
    double d = (dist); \
    printf("%s: %g, errno %d\n", (label), d, meos_errno()); \
    assert(fabs(d - (expected)) < 1e-9 && meos_errno() == 0); \
  } while (0)

/* Main program */
int main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();
  const char *ways = getenv("MEOS_WAYS_CSV");
  meos_set_ways_csv(ways ? ways : "../examples/data/ways1000.csv");

  /* Circular buffers */
  Set *cb1 = cbufferset_in("{\"Cbuffer(Point(0 0), 1)\"}");
  Set *cb2 = cbufferset_in("{\"Cbuffer(Point(5 0), 1)\", \"Cbuffer(Point(9 0), 1)\"}");
  Set *cb3 = cbufferset_in("SRID=3857;{\"Cbuffer(Point(0 0), 1)\"}");
  assert(cb1 && cb2 && cb3);
  DISTANCE("distance_cbufferset_cbufferset", distance_cbufferset_cbufferset(cb1, cb2), 3.0);
  DISTANCE("distance_cbufferset_cbufferset(self)", distance_cbufferset_cbufferset(cb2, cb2), 0.0);

  /* Poses and pose chains */
  Set *p1 = poseset_in("{\"Pose(Point(0 0), 0)\"}");
  Set *p2 = poseset_in("{\"Pose(Point(3 4), 0)\", \"Pose(Point(6 8), 0)\"}");
  Set *pc1 = posechainset_in("{\"PoseChain(Pose(Point(0 0), 0))\"}");
  Set *pc2 = posechainset_in("{\"PoseChain(Pose(Point(3 4), 0))\"}");
  assert(p1 && p2 && pc1 && pc2);
  DISTANCE("distance_poseset_poseset", distance_poseset_poseset(p1, p2), 5.0);
  DISTANCE("distance_posechainset_posechainset", distance_posechainset_posechainset(pc1, pc2), 5.0);
  DISTANCE("distance_posechainset_posechainset(self)", distance_posechainset_posechainset(pc1, pc1), 0.0);

  /* Network points */
  Set *np1 = npointset_in("{\"Npoint(1, 0.5)\"}");
  Set *np2 = npointset_in("{\"Npoint(1, 0.5)\", \"Npoint(2, 0.3)\"}");
  assert(np1 && np2);
  DISTANCE("distance_npointset_npointset(shared)", distance_npointset_npointset(np1, np2), 0.0);
  Set *np3 = npointset_in("{\"Npoint(2, 0.3)\"}");
  double dnp = distance_npointset_npointset(np1, np3);
  printf("distance_npointset_npointset: %g, errno %d\n", dnp, meos_errno());
  assert(dnp >= 0.0 && dnp < DBL_MAX && meos_errno() == 0);

  /* A NULL set, a set of another type and two SRIDs are refused */
  REFUSED("distance_cbufferset_cbufferset(NULL)",
    distance_cbufferset_cbufferset(NULL, cb1) == DBL_MAX, MEOS_ERR_INVALID_ARG);
  REFUSED("distance_cbufferset_cbufferset(poseset)",
    distance_cbufferset_cbufferset(cb1, p1) == DBL_MAX, MEOS_ERR_INVALID_ARG_TYPE);
  REFUSED("distance_cbufferset_cbufferset(mixed SRID)",
    distance_cbufferset_cbufferset(cb1, cb3) == DBL_MAX, MEOS_ERR_INVALID_ARG_VALUE);
  REFUSED("distance_npointset_npointset(NULL)",
    distance_npointset_npointset(np1, NULL) == DBL_MAX, MEOS_ERR_INVALID_ARG);
  REFUSED("distance_poseset_poseset(posechainset)",
    distance_poseset_poseset(p1, pc1) == DBL_MAX, MEOS_ERR_INVALID_ARG_TYPE);
  REFUSED("distance_posechainset_posechainset(cbufferset)",
    distance_posechainset_posechainset(pc1, cb1) == DBL_MAX, MEOS_ERR_INVALID_ARG_TYPE);

  free(cb1); free(cb2); free(cb3); free(p1); free(p2); free(pc1); free(pc2);
  free(np1); free(np2); free(np3);
  meos_finalize();
  printf("spatialset_distance_test: all checks passed\n");
  return 0;
}
