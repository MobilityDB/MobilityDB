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
 * @brief Whether a 2D pose turns between two instants is decided exactly
 * @details A segment of 2D poses whose two angles differ turns, however small
 * the difference: the pose interpolated on it carries an angle between the
 * two, and the location of that pose on the segment, which compares the
 * angles exactly, finds it at the ratio it was interpolated at. A segment
 * whose two angles are equal does not turn.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <meos.h>
#include <meos_pose.h>
#include <pg_timestamp.h>

#include "pose/pose.h"

static int failures = 0;
static int checks = 0;

static void
check(bool ok, const char *what)
{
  checks++;
  if (! ok)
  {
    failures++;
    printf("FAIL: %s\n", what);
  }
}

/* The segment from the pose at (0 0) with angle t1 to the pose at (2 0) with
 * angle t2, interpolated at the ratio 1/2 and located back */
static void
check_turn(double t1, double t2, const char *name)
{
  char msg[256];
  Pose *start = pose_make_2d(0, 0, t1, false, 0);
  Pose *end = pose_make_2d(2, 0, t2, false, 0);
  Pose *mid = posesegm_interpolate(start, end, 0.5);
  double theta = mid->data[2];
  if (t1 == t2)
  {
    snprintf(msg, sizeof(msg), "%s: equal angles keep the angle", name);
    check(theta == t1, msg);
  }
  else
  {
    snprintf(msg, sizeof(msg), "%s: the angle at 1/2 lies strictly between "
      "the two (%.17g)", name, theta);
    check(fmin(t1, t2) < theta && theta < fmax(t1, t2), msg);
  }
  long double r = posesegm_locate(start, end, mid);
  snprintf(msg, sizeof(msg), "%s: the pose at 1/2 is located at 1/2 (%.17g)",
    name, (double) r);
  check(r == 0.5L, msg);
  free(start); free(end); free(mid);
}

int
main(void)
{
  meos_initialize();

  /* Turns smaller than MEOS_EPSILON, 1e-6 */
  check_turn(0.0, 1e-8, "turn of 1e-8");
  check_turn(0.0, -4e-8, "turn of -4e-8");
  check_turn(1.0, 1.0 + 1e-9, "turn of 1e-9 from 1");
  check_turn(0.25, 0.25 + 5e-7, "turn of 5e-7 from 0.25");
  check_turn(-2.0, -2.0 - 6e-8, "turn of -6e-8 from -2");
  /* Controls: a turn above 1e-6 and no turn */
  check_turn(0.0, 1e-5, "turn of 1e-5");
  check_turn(0.5, 0.5, "no turn");

  /* The same through the temporal pose: the value at the middle instant of
   * a segment turning by 1e-8 is a value the temporal pose takes */
  Temporal *temp = tpose_in("[Pose(Point(0 0), 0)@2001-01-01, "
    "Pose(Point(2 0), 0.00000001)@2001-01-03]");
  Pose *value = NULL;
  bool found = tpose_value_at_timestamptz(temp,
    timestamptz_in("2001-01-02", -1), true, &value);
  check(found && value != NULL, "tpose: a value at the middle instant");
  if (found && value)
  {
    check(value->data[2] > 0.0, "tpose: the value at the middle instant "
      "has turned");
    check(ever_eq_tpose_pose(temp, value) == 1, "tpose: the value at the "
      "middle instant is ever equal to the temporal pose");
    free(value);
  }
  free(temp);

  printf("%d/%d checks passed\n", checks - failures, checks);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
