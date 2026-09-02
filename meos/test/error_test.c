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
 * @brief A program that tests the error status functions of the MEOS API.
 *
 * The error status is only observable once
 * #meos_initialize_noexit_error_handler is installed — the handler every
 * language binding uses, since a binding must return an exception to its host
 * rather than terminate it. The program verifies that a failing call sets
 * #meos_errno, that #meos_errno_reset clears it and returns its previous
 * value, and that #meos_errno_restore puts back a stored value, which is the
 * sequence documented for monitoring a section of code for new errors.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o error_test error_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_internal.h>
#include <meos_geo.h>
#include <meos_h3.h>
#include <meos_pose.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* A fresh error status is clear */
  assert(meos_errno() == 0);

  /* A failing call sets the error status */
  Temporal *bad = tfloat_in("this is not a temporal float");
  assert(bad == NULL);
  int failed_errno = meos_errno();
  printf("errno after a failed call: %d\n", failed_errno);
  assert(failed_errno != 0);

  /* Resetting returns the previous value and clears the status */
  int previous = meos_errno_reset();
  printf("meos_errno_reset() returned %d, errno is now %d\n", previous,
    meos_errno());
  assert(previous == failed_errno);
  assert(meos_errno() == 0);

  /* A succeeding call leaves the status clear */
  Temporal *good = tfloat_in("{77@2001-01-01}");
  assert(good != NULL);
  char *good_out = tfloat_out(good, 6);
  printf("tfloat_in(\"{77@2001-01-01}\"): %s, errno %d\n", good_out,
    meos_errno());
  assert(meos_errno() == 0);

  /* Restoring puts back a stored value, so that a function which reset the
   * status to monitor a section of code can hand the caller's error back */
  meos_errno_restore(failed_errno);
  printf("meos_errno_restore(%d): errno is now %d\n", failed_errno,
    meos_errno());
  assert(meos_errno() == failed_errno);

  /* And the status can be cleared again afterwards */
  assert(meos_errno_reset() == failed_errno);
  assert(meos_errno() == 0);

  /* A function returning an array through a count out parameter leaves that
   * count defined when it rejects its arguments, so a caller reading the count
   * of a failed call sees no elements rather than whatever the variable held.
   * Only the noexit handler reaches this: the default one ends the process
   * inside the rejected check. The count starts at a value the functions never
   * produce, so that leaving it untouched is distinguishable from setting it. */
  int count = -559038737;
  Temporal *inst = tint_in("1@2001-01-01");
  assert(inst != NULL);
  meos_errno_reset();
  /* temporal_segments takes a sequence (set), so an instant is rejected */
  assert(temporal_segments(inst, &count) == NULL);
  printf("temporal_segments(instant): NULL, count %d, errno %d\n", count,
    meos_errno());
  assert(count == 0);

  count = -559038737;
  Temporal *seq = tint_in("{1@2001-01-01, 2@2001-01-02}");
  assert(seq != NULL);
  meos_errno_reset();
  /* a span count of zero is rejected */
  assert(temporal_split_n_spans(seq, 0, &count) == NULL);
  printf("temporal_split_n_spans(0): NULL, count %d, errno %d\n", count,
    meos_errno());
  assert(count == 0);

  /* The count is defined whichever argument the function rejects, not only
   * when the count itself is the one at fault, so it is checked here on a
   * rejection of another argument as well */
  count = -559038737;
  meos_errno_reset();
  assert(temporal_segments(NULL, &count) == NULL);
  printf("temporal_segments(NULL): NULL, count %d, errno %d\n", count,
    meos_errno());
  assert(count == 0);

  count = -559038737;
  Temporal *num = tint_in("{1@2001-01-01}");
  assert(num != NULL);
  meos_errno_reset();
  /* the two values must share a type, so a temporal float is rejected */
  assert(temporal_frechet_path(num, good, &count) == NULL);
  printf("temporal_frechet_path(mixed types): NULL, count %d, errno %d\n",
    count, meos_errno());
  assert(count == 0);

  /* A NULL count is itself rejected, rather than written through */
  meos_errno_reset();
  assert(temporal_frechet_path(num, num, NULL) == NULL);
  printf("temporal_frechet_path(count NULL): NULL, errno %d\n", meos_errno());
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);

  /* A set operation between a spatial set and a value of another frame is
   * rejected rather than answered, as every other operation mixing two
   * spatial reference systems is. The set operations of a spatial set take
   * the value through the same validity check as the rest of the library */
  Set *gset = geomset_in("{\"SRID=4326;Point(1 1)\"}");
  GSERIALIZED *other = geom_in("SRID=3812;Point(1 1)", -1);
  assert(gset != NULL); assert(other != NULL);
  meos_errno_reset();
  assert(contains_set_geo(gset, other) == false);
  printf("contains_set_geo(mixed SRID): false, errno %d\n", meos_errno());
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);

  /* The same holds when the two agree on the SRID but not on whether their
   * coordinates are planar or geodetic */
  GSERIALIZED *geodetic = geog_in("SRID=4326;Point(1 1)", -1);
  assert(geodetic != NULL);
  meos_errno_reset();
  assert(contains_set_geo(gset, geodetic) == false);
  printf("contains_set_geo(mixed planar/geodetic): false, errno %d\n",
    meos_errno());
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);

  Set *pset = poseset_in("{\"SRID=4326;Pose(Point(1 1),0.5)\"}");
  Pose *pose_other = pose_in("SRID=3812;Pose(Point(1 1),0.5)");
  assert(pset != NULL); assert(pose_other != NULL);
  meos_errno_reset();
  assert(contains_set_pose(pset, pose_other) == false);
  printf("contains_set_pose(mixed SRID): false, errno %d\n", meos_errno());
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);

  /* A coordinate transformation PROJ cannot perform is reported by liblwgeom,
   * whose errors reach the same channel. Under the no-exit handler the caller
   * keeps its process and reads the failure from meos_errno, which is what a
   * binding needs from a foreign thread. EPSG:22300 has no operation from
   * EPSG:4326, so the transformation is declined rather than answered with the
   * original coordinates relabelled */
  GSERIALIZED *wgs84 = geom_in("SRID=4326;Point(6 51)", -1);
  assert(wgs84 != NULL);
  meos_errno_reset();
  GSERIALIZED *declined = geo_transform(wgs84, 22300);
  printf("geo_transform(4326 -> 22300): %s, errno %d\n",
    declined ? "answered" : "declined", meos_errno());
  assert(declined == NULL);
  assert(meos_errno() != 0);

  /* A transformation PROJ can perform still answers, so the decline above
   * discriminates */
  meos_errno_reset();
  GSERIALIZED *webmerc = geo_transform(wgs84, 3857);
  assert(webmerc != NULL);
  assert(meos_errno() == 0);
  printf("geo_transform(4326 -> 3857): %s\n", geo_as_ewkt(webmerc, 2));

  /* An ordered comparison admits only a temporal type whose base type carries
   * an order. A geometry has a B-tree order so that it can be indexed, which is
   * not a meaning to compare over time, so the call declines */
  meos_errno_reset();
  Temporal *tpt1 = tgeompoint_in("[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]");
  Temporal *tpt2 = tgeompoint_in("[Point(2 2)@2001-01-01, Point(3 3)@2001-01-02]");
  assert(tpt1 != NULL && tpt2 != NULL && meos_errno() == 0);
  Temporal *unordered = tlt_temporal_temporal(tpt1, tpt2);
  printf("tlt_temporal_temporal(tgeompoint, tgeompoint): %s, errno %d\n",
    unordered ? "answered" : "declined", meos_errno());
  assert(unordered == NULL);
  assert(meos_errno() != 0);

  /* The same comparison over a temporal integer answers, so the decline above
   * discriminates */
  meos_errno_reset();
  Temporal *ordered = tlt_temporal_temporal(num, num);
  assert(ordered != NULL);
  assert(meos_errno() == 0);
  printf("tlt_temporal_temporal(tint, tint): answered, errno %d\n",
    meos_errno());
  free(ordered);

  /* Restricting a temporal value to where it takes its minimum or its maximum
   * reads the same order, so it admits the same types. A geometry and a pose
   * carry a B-tree order for indexing and no order to take an extremum over,
   * and the four entries decline them */
  meos_errno_reset();
  assert(temporal_at_min(tpt1) == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  meos_errno_reset();
  assert(temporal_at_max(tpt1) == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  meos_errno_reset();
  assert(temporal_minus_min(tpt1) == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  meos_errno_reset();
  assert(temporal_minus_max(tpt1) == NULL);
  printf("temporal_at_min/at_max/minus_min/minus_max(tgeompoint): declined, "
    "errno %d\n", meos_errno());
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);

  /* A temporal pose declines them for the same reason, and a pose is the value
   * a temporal rigid geometry carries */
  meos_errno_reset();
  Temporal *tps = tpose_in("[Pose(Point(0 0),0)@2001-01-01, "
    "Pose(Point(2 2),0.5)@2001-01-02]");
  assert(tps != NULL && meos_errno() == 0);
  assert(temporal_minus_max(tps) == NULL);
  printf("temporal_minus_max(tpose): declined, errno %d\n", meos_errno());
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  free(tps);

  /* The same restriction over a temporal integer answers, so the declines
   * above discriminate */
  meos_errno_reset();
  Temporal *extremum = temporal_at_max(num);
  assert(extremum != NULL);
  assert(meos_errno() == 0);
  printf("temporal_at_max(tint): answered, errno %d\n", meos_errno());
  free(extremum);

  /* A base type carrying no decimal has no round function, and the two
   * entries that lift one report the decline instead of calling it: a cell
   * index identifier is an integer with nothing to round */
  meos_errno_reset();
  Set *cellset = h3index_to_set((H3Index) 0x831c02fffffffffULL);
  assert(cellset != NULL && meos_errno() == 0);
  assert(set_round(cellset, 2) == NULL);
  printf("set_round(h3indexset): declined, errno %d\n", meos_errno());
  assert(meos_errno() == MEOS_ERR_INTERNAL_TYPE_ERROR);
  free(cellset);

  meos_errno_reset();
  Temporal *cell = th3index_in("[831c02fffffffff@2001-01-01]");
  assert(cell != NULL && meos_errno() == 0);
  assert(temporal_round(cell, 2) == NULL);
  printf("temporal_round(th3index): declined, errno %d\n", meos_errno());
  assert(meos_errno() == MEOS_ERR_INTERNAL_TYPE_ERROR);
  free(cell);

  /* A set of floats rounds, so the two declines discriminate */
  meos_errno_reset();
  Set *fset = floatset_in("{1.234567, 2.345678}");
  assert(fset != NULL);
  Set *frounded = set_round(fset, 2);
  assert(frounded != NULL && meos_errno() == 0);
  char *ftext = floatset_out(frounded, 6);
  printf("set_round(floatset): answered %s\n", ftext);
  assert(strcmp(ftext, "{1.23, 2.35}") == 0);
  free(ftext); free(frounded); free(fset);
  meos_errno_reset();

  /* An aggregation whose values disagree at a timestamp they share reports the
   * disagreement, and under the noexit handler that report RETURNS. The step
   * that raises answers a negative count; folding it into the running length
   * walked that length backwards and handed the normalization a number that is
   * not how many sequences there are, so the process ended inside
   * tseqarr_normalize instead of at the caller's check */
  meos_errno_reset();
  Temporal *magg1 = tint_in("{[1@2001-01-01, 3@2001-01-03]}");
  Temporal *magg2 = tint_in("{[7@2001-01-01, 9@2001-01-03]}");
  assert(magg1 != NULL && magg2 != NULL && meos_errno() == 0);
  SkipList *mst1 = temporal_merge_transfn(NULL, magg1);
  SkipList *mst2 = temporal_merge_transfn(NULL, magg2);
  assert(mst1 != NULL && mst2 != NULL && meos_errno() == 0);
  SkipList *mcomb = temporal_merge_combinefn(mst1, mst2);
  printf("merge combine of values disagreeing at a shared timestamp: "
    "declined, errno %d\n", meos_errno());
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  /* The combine answers a state either way, and which of the two it keeps is
   * what says whose storage is still the caller's */
  if (mst1 != mcomb)
    skiplist_free(mst1);
  if (mst2 != mcomb)
    skiplist_free(mst2);
  if (mcomb)
    skiplist_free(mcomb);
  free(magg1); free(magg2);
  meos_errno_reset();

  free(tpt1); free(tpt2);

  meos_errno_reset();

  free(wgs84); free(webmerc);
  free(gset); free(other); free(geodetic);
  free(pset); free(pose_other);
  free(num);
  free(inst); free(seq);
  free(good); free(good_out);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
