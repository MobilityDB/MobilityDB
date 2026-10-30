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
 * @brief A program that tests that the text input of circular buffers, poses
 * and span sets refuses a malformed value and keeps nothing it allocated
 * @details A text parser that reads a part of a value into an allocation,
 * such as the centre of a circular buffer or of a pose, and then finds the
 * rest malformed returns an error, and must free what it read before. Under
 * the noexit error handler a binding keeps running after such an error, so a
 * part left behind is memory lost for each malformed value it reads.
 *
 * The program reads one malformed value for each error return that follows
 * an allocation in #cbuffer_in, #pose_in and #intspanset_in, verifies that
 * each is refused with #meos_errno set, and reads well-formed values that
 * must be accepted. Run under valgrind with `--leak-check=full`, it reports
 * no block definitely lost.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o text_input_error_test text_input_error_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_cbuffer.h>
#include <meos_pose.h>

typedef void *(*input_fn)(const char *);

static void *
cbuffer_in_v(const char *str)
{
  return cbuffer_in(str);
}

static void *
pose_in_v(const char *str)
{
  return pose_in(str);
}

static void *
intspanset_in_v(const char *str)
{
  return intspanset_in(str);
}

static int failures = 0;

/* Read a value that must be refused, or accepted, and free what is returned */
static void
check(input_fn in, const char *str, bool accept)
{
  meos_errno_reset();
  void *value = in(str);
  bool ok = accept ? (value != NULL && meos_errno() == 0) :
    (value == NULL && meos_errno() != 0);
  if (! ok)
  {
    printf("  %s: expected %s, got %s\n", str, accept ? "a value" : "an error",
      value ? "a value" : "an error");
    failures++;
  }
  free(value);
  meos_errno_reset();
}

/* Main program */
int main(void)
{
  /* Install the error handler that reports through meos_errno instead of
   * exiting, after meos_initialize, which installs the default one */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* The circular buffer, after its centre is read: the radius unreadable or
   * negative, the closing parenthesis missing, input after it */
  check(cbuffer_in_v, "Cbuffer(Point(1 1), x)", false);
  check(cbuffer_in_v, "Cbuffer(Point(1 1), -1)", false);
  check(cbuffer_in_v, "Cbuffer(Point(1 1), 1", false);
  check(cbuffer_in_v, "Cbuffer(Point(1 1), 1) x", false);
  check(cbuffer_in_v, "Cbuffer(Point(1 1), 1)", true);

  /* The planar pose, after its position is read: the angle unreadable or out
   * of range, then after the pose is made: the closing parenthesis missing,
   * input after it */
  check(pose_in_v, "Pose(Point(1 1), x)", false);
  check(pose_in_v, "Pose(Point(1 1), 7)", false);
  check(pose_in_v, "Pose(Point(1 1), 0.5", false);
  check(pose_in_v, "Pose(Point(1 1), 0.5) x", false);
  check(pose_in_v, "Pose(Point(1 1), 0.5)", true);

  /* The spatial pose, after its position is read: each component of the
   * quaternion unreadable, the quaternion not of unit norm */
  check(pose_in_v, "Pose(Point Z(1 1 1), x, 0, 0, 0)", false);
  check(pose_in_v, "Pose(Point Z(1 1 1), 1, x, 0, 0)", false);
  check(pose_in_v, "Pose(Point Z(1 1 1), 1, 0, x, 0)", false);
  check(pose_in_v, "Pose(Point Z(1 1 1), 1, 0, 0, x)", false);
  check(pose_in_v, "Pose(Point Z(1 1 1), 2, 0, 0, 0)", false);
  check(pose_in_v, "Pose(Point Z(1 1 1), 1, 0, 0, 0)", true);

  /* A geometry read before the keyword of the pose, which the input refuses
   * whatever follows it: the keyword missing, the opening parenthesis
   * missing, and a well-formed pose after it */
  check(pose_in_v, "Point(1 1),Xose(Point(1 1), 0.5)", false);
  check(pose_in_v, "Point(1 1),Pose Point(1 1), 0.5)", false);
  check(pose_in_v, "Point(1 1),Pose(Point(1 1), 0.5)", false);

  /* The span set, after its spans are read: the closing brace missing, input
   * after it */
  check(intspanset_in_v, "{[1, 2]", false);
  check(intspanset_in_v, "{[1, 2]} x", false);
  check(intspanset_in_v, "{[1, 2], [4, 5]}", true);

  printf("%d check(s) failed\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
