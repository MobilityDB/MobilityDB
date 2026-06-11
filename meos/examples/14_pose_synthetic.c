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
 *****************************************************************************/

/**
 * @file
 * @brief End-to-end demonstration of the @p pose / @p tpose API on a
 * synthetic GPS+IMU-style trajectory.
 *
 * Walks through the production flow that a sensor-fusion application
 * typically follows: build a temporal pose from per-instant
 * (position, orientation) samples, query it via the static-pose
 * accessors, and verify that round-trips through the WKB binary I/O
 * preserve the value byte-for-byte. Acts as living documentation for
 * the master-pure subset of the @p pose / @p tpose API surface — every
 * function called here is part of the supported MEOS C-API on master.
 *
 * The program can be built as follows:
 * @code
 * gcc -Wall -g -I/usr/local/include -o 14_pose_synthetic 14_pose_synthetic.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <meos.h>
#include <meos_pose.h>

#define N_INSTANTS 5
#define BUFSZ      4096

int main(void)
{
  meos_initialize();

  /* Build a synthetic trajectory in WKT form: a body translates 1 m/s
   * along the +X axis while yawing at 0.1 rad/s about the world Z
   * axis. The first instant is the identity orientation; each
   * subsequent quaternion is exp(0.1 * i / 2 * Z), which in component
   * form is (cos(0.05 * i), 0, 0, sin(0.05 * i)). */
  char buf[BUFSZ];
  size_t off = 0;
  off += snprintf(buf + off, BUFSZ - off, "[");
  for (int i = 0; i < N_INSTANTS; i++)
  {
    double half = 0.05 * i;
    double w = cos(half), z = sin(half);
    off += snprintf(buf + off, BUFSZ - off,
      "%sPose(Point(%d 0 0), %.10f, 0, 0, %.10f)@2026-01-01 00:00:%02d+00",
      i == 0 ? "" : ", ", i, w, z, i);
  }
  off += snprintf(buf + off, BUFSZ - off, "]");

  Temporal *traj = tpose_in(buf);
  if (! traj)
  {
    fprintf(stderr, "Failed to parse synthetic trajectory.\n");
    meos_finalize();
    return 1;
  }

  /* Print the trajectory. */
  char *traj_text = temporal_out(traj, 6);
  printf("Trajectory (5 instants, 1-second steps, +X translation, yaw 0.1 rad/s):\n  %s\n\n",
    traj_text);
  free(traj_text);

  /* Inspect a single instant via the static pose API. */
  Pose *first = (Pose *) tpose_start_value(traj);
  printf("First-instant pose (identity orientation):\n");
  char *first_text = pose_out(first, 6);
  printf("  %s\n", first_text);
  printf("  SRID:                %d\n", pose_srid(first));
  free(first_text);
  free(first);

  Pose *last = (Pose *) tpose_end_value(traj);
  char *last_text = pose_out(last, 6);
  printf("\nLast-instant pose (yawed by 0.4 rad about Z, translated by 4 m on +X):\n  %s\n",
    last_text);
  free(last_text);
  free(last);

  /* WKB round-trip: serialise to bytes, deserialise, verify byte-equal
   * to the original. This is the canonical durability check for the
   * pose / tpose binary representation. */
  size_t wkb_size;
  uint8_t *wkb = temporal_as_wkb(traj, 1 /* WKB_NDR */, &wkb_size);
  Temporal *back = temporal_from_wkb(wkb, wkb_size);
  printf("\nWKB round-trip (%zu bytes): %s\n",
    wkb_size, temporal_eq(traj, back) ? "OK" : "BROKEN");
  free(wkb);
  free(back);

  free(traj);
  meos_finalize();
  return 0;
}
