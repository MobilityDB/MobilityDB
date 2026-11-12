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
 * @brief A simple program that tests the generic temporal functions on a
 * temporal rigid geometry
 * @details A temporal rigid geometry is a temporal pose carrying a reference
 * geometry, and every generic temporal function of an accessor, a restriction
 * to time or a deletion answers it as the poses the function answers for its
 * temporal pose, carrying the same reference geometry. For each function and
 * each value, the program checks both: the temporal pose of the answer is
 * the answer of the function on the temporal pose of the value, and the
 * reference geometry of the answer is the one of the value. The values are a
 * linear sequence, a step sequence, a sequence set, whose sequences carry no
 * reference geometry of their own, and a sequence of one instant, which a
 * discrete interpolation rebuilds from its instant. Where the
 * function refuses the temporal pose it refuses the rigid geometry too, so the
 * error handler returns rather than exits, and both answers are NULL. The
 * generic writers, temporal_out, tspatial_as_text and tspatial_as_ewkt, write
 * a rigid geometry as its own writers do, its reference geometry before its
 * poses, and the output reads back as the value.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o trgeo_generic_test trgeo_generic_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_rgeo.h>

static const char *INPUTS[] = {
  "Interp=Linear;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, "
    "Pose(Point(1.5 1.25), 0.4)@2001-01-02, Pose(Point(2.75 1.5), 0.5)@2001-01-03]",
  "Interp=Step;Polygon((-1 -1,1 -1,1 1,-1 1,-1 -1));[Pose(Point(0 0), 0)@2001-01-01, "
    "Pose(Point(10 0), 0.5)@2001-01-03, Pose(Point(12 0), 0.7)@2001-01-05]",
  "Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, "
    "Pose(Point(2 1), 0.4)@2001-01-02], [Pose(Point(3 1), 0.5)@2001-01-04, "
    "Pose(Point(4 1), 0.6)@2001-01-05]}",
  "Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.3)@2001-01-01]",
};

static int failures = 0, checks = 0;

/* A non-exiting error handler, as parse_test.c installs it, so a refusal
 * answers NULL instead of ending the process */
static void
quiet_error_handler(int errlevel, int errcode, const char *errmsg)
{
  (void) errlevel; (void) errcode; (void) errmsg;
}

/* The text of a temporal pose, or NULL */
static char *
pose_text(const Temporal *temp)
{
  size_t n;
  return temp ? tspatial_as_text(temp, 6, &n) : NULL;
}

/*
 * Check that the answer for the rigid geometry is the answer for its poses
 * carrying the reference geometry of the value
 */
static void
check(const char *name, int input, const Temporal *trgeo,
  const Temporal *answer, const Temporal *pose_answer)
{
  checks++;
  char *expected = pose_text(pose_answer);
  Temporal *poses = answer ? trgeometry_to_tpose(answer) : NULL;
  char *got = pose_text(poses);
  bool same_poses = (! expected && ! got) ||
    (expected && got && strcmp(expected, got) == 0);
  bool same_geom = true;
  if (answer)
  {
    size_t n;
    GSERIALIZED *g1 = trgeometry_geom(trgeo), *g2 = trgeometry_geom(answer);
    char *s1 = geo_as_text(g1, 6, &n), *s2 = geo_as_text(g2, 6, &n);
    same_geom = strcmp(s1, s2) == 0;
    free(g1); free(g2); free(s1); free(s2);
  }
  if (! same_poses || ! same_geom)
  {
    printf("FAILED: %s on input %d: poses %s against %s%s\n", name, input,
      got ? got : "NULL", expected ? expected : "NULL",
      same_geom ? "" : ", reference geometry changed");
    failures++;
  }
  free(expected); free(got); free(poses);
}

/*
 * Check that a generic writer writes what the writer of the rigid geometry
 * writes, as #check compares the poses of two answers, with its counters and
 * its message; the two strings carry the same timestamps, so that no time
 * zone moves one without the other, and are freed
 */
static void
check_text(const char *name, int input, char *got, char *expected)
{
  checks++;
  if (! got || ! expected || strcmp(got, expected) != 0)
  {
    printf("FAILED: %s on input %d: %s against %s\n", name, input,
      got ? got : "NULL", expected ? expected : "NULL");
    failures++;
  }
  free(got); free(expected);
}

/* The same check for each element of two arrays */
static void
check_array(const char *name, int input, const Temporal *trgeo,
  Temporal **answer, int count, Temporal **pose_answer, int pose_count)
{
  if (count != pose_count)
  {
    checks++;
    printf("FAILED: %s on input %d: %d values against %d\n", name, input,
      count, pose_count);
    failures++;
    return;
  }
  for (int i = 0; i < count; i++)
    check(name, input, trgeo, answer[i], pose_answer[i]);
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_error_handler(quiet_error_handler);

  TimestampTz ts = timestamptz_in("2001-01-02", -1);
  Set *tset = tstzset_in("{2001-01-02, 2001-01-04}");
  Span *tspan = tstzspan_in("[2001-01-02, 2001-01-04]");
  SpanSet *tss = tstzspanset_in(
    "{[2001-01-02, 2001-01-02 12:00], [2001-01-04, 2001-01-05]}");

  for (int i = 0; i < (int) (sizeof(INPUTS) / sizeof(INPUTS[0])); i++)
  {
    Temporal *t = trgeometry_in(INPUTS[i]);
    Temporal *p = trgeometry_to_tpose(t);
    Temporal *a, *b;
    Temporal **aa, **bb;
    int c1, c2;

    a = (Temporal *) temporal_start_instant(t);
    b = (Temporal *) temporal_start_instant(p);
    check("temporal_start_instant", i, t, a, b); free(a); free(b);
    a = (Temporal *) temporal_end_instant(t);
    b = (Temporal *) temporal_end_instant(p);
    check("temporal_end_instant", i, t, a, b); free(a); free(b);
    a = (Temporal *) temporal_instant_n(t, 2);
    b = (Temporal *) temporal_instant_n(p, 2);
    check("temporal_instant_n", i, t, a, b); free(a); free(b);
    aa = (Temporal **) temporal_instants(t, &c1);
    bb = (Temporal **) temporal_instants(p, &c2);
    check_array("temporal_instants", i, t, aa, c1, bb, c2);
    for (int k = 0; k < c1; k++) free(aa[k]);
    for (int k = 0; k < c2; k++) free(bb[k]);
    free(aa); free(bb);

    a = (Temporal *) temporal_start_sequence(t);
    b = (Temporal *) temporal_start_sequence(p);
    check("temporal_start_sequence", i, t, a, b); free(a); free(b);
    a = (Temporal *) temporal_end_sequence(t);
    b = (Temporal *) temporal_end_sequence(p);
    check("temporal_end_sequence", i, t, a, b); free(a); free(b);
    a = (Temporal *) temporal_sequence_n(t, 1);
    b = (Temporal *) temporal_sequence_n(p, 1);
    check("temporal_sequence_n", i, t, a, b); free(a); free(b);
    aa = (Temporal **) temporal_sequences(t, &c1);
    bb = (Temporal **) temporal_sequences(p, &c2);
    check_array("temporal_sequences", i, t, aa, c1, bb, c2);
    for (int k = 0; k < c1; k++) free(aa[k]);
    for (int k = 0; k < c2; k++) free(bb[k]);
    free(aa); free(bb);
    aa = (Temporal **) temporal_segments(t, &c1);
    bb = (Temporal **) temporal_segments(p, &c2);
    check_array("temporal_segments", i, t, aa, c1, bb, c2);
    for (int k = 0; k < c1; k++) free(aa[k]);
    for (int k = 0; k < c2; k++) free(bb[k]);
    free(aa); free(bb);

    a = temporal_delete_tstzset(t, tset, true);
    b = temporal_delete_tstzset(p, tset, true);
    check("temporal_delete_tstzset", i, t, a, b); free(a); free(b);
    a = temporal_delete_tstzspan(t, tspan, true);
    b = temporal_delete_tstzspan(p, tspan, true);
    check("temporal_delete_tstzspan", i, t, a, b); free(a); free(b);
    a = temporal_delete_tstzspanset(t, tss, true);
    b = temporal_delete_tstzspanset(p, tss, true);
    check("temporal_delete_tstzspanset", i, t, a, b); free(a); free(b);

    a = temporal_after_timestamptz(t, ts, false);
    b = temporal_after_timestamptz(p, ts, false);
    check("temporal_after_timestamptz", i, t, a, b); free(a); free(b);
    a = temporal_before_timestamptz(t, ts, false);
    b = temporal_before_timestamptz(p, ts, false);
    check("temporal_before_timestamptz", i, t, a, b); free(a); free(b);
    a = temporal_at_tstzset(t, tset);
    b = temporal_at_tstzset(p, tset);
    check("temporal_at_tstzset", i, t, a, b); free(a); free(b);
    a = temporal_minus_tstzset(t, tset);
    b = temporal_minus_tstzset(p, tset);
    check("temporal_minus_tstzset", i, t, a, b); free(a); free(b);
    a = temporal_at_tstzspan(t, tspan);
    b = temporal_at_tstzspan(p, tspan);
    check("temporal_at_tstzspan", i, t, a, b); free(a); free(b);
    a = temporal_minus_tstzspanset(t, tss);
    b = temporal_minus_tstzspanset(p, tss);
    check("temporal_minus_tstzspanset", i, t, a, b); free(a); free(b);
    a = temporal_round(t, 1);
    b = temporal_round(p, 1);
    check("temporal_round", i, t, a, b); free(a); free(b);
    a = temporal_set_interp(t, DISCRETE);
    b = temporal_set_interp(p, DISCRETE);
    check("temporal_set_interp", i, t, a, b); free(a); free(b);

    /* The writers: a generic writer writes the reference geometry before the
     * poses, as the writer of the rigid geometry does; the output writes the
     * poses with the number of decimal digits it is given, as the output of
     * the temporal pose does, and reads back as the value */
    size_t n;
    check_text("temporal_out", i, temporal_out(t, 6, &n),
      trgeometry_out(t, 6, &n));
    check_text("tspatial_as_text", i, tspatial_as_text(t, 6, &n),
      trgeometry_as_text(t, 6, &n));
    check_text("tspatial_as_ewkt", i, tspatial_as_ewkt(t, 6, &n),
      trgeometry_as_ewkt(t, 6, &n));
    /* The interpolation of a value in step interpolation precedes its
     * reference geometry, where the parser reads it, so it is compared apart
     * from the poses that follow the reference geometry */
    static const char step[] = "Interp=Step;";
    size_t steplen = sizeof(step) - 1;
    char *out1 = trgeometry_out(t, 1, &n);
    char *poses1 = temporal_out(p, 1, &n);
    bool step1 = out1 && strncmp(out1, step, steplen) == 0;
    bool step2 = poses1 && strncmp(poses1, step, steplen) == 0;
    const char *semi = out1 ? strchr(out1 + (step1 ? steplen : 0), ';') : NULL;
    check_text("trgeometry_out digits", i,
      strdup(semi && step1 == step2 ? semi + 1 : "NULL"),
      strdup(poses1 ? poses1 + (step2 ? steplen : 0) : "NULL"));
    free(out1); free(poses1);
    char *out = temporal_out(t, 15, &n);
    a = out ? trgeometry_in(out) : NULL;
    checks++;
    if (! a || ! temporal_eq(t, a))
    {
      printf("FAILED: trgeometry_in(temporal_out) on input %d: %s\n", i,
        out ? out : "NULL");
      failures++;
    }
    free(out); free(a);

    free(t); free(p);
  }

  free(tset); free(tspan); free(tss);

  /* The reference geometry is rounded as the poses are, a polyhedral surface
   * included */
  {
    checks++;
    Temporal *t = trgeometry_in("PolyhedralSurface Z(((0 0 0,2.123456789 0 0,"
      "2.123456789 2 0,0 0 0)),((0 0 0,2.123456789 2 0,0 0 2.123456789,"
      "0 0 0)));[Pose(Point(1.123456789 1 5),1,0,0,0)@2001-01-01, "
      "Pose(Point(2 2 5),1,0,0,0)@2001-01-02]");
    Temporal *r = t ? temporal_round(t, 2) : NULL;
    GSERIALIZED *g = t ? trgeometry_geom(t) : NULL;
    GSERIALIZED *gr = g ? geo_round(g, 2) : NULL;
    GSERIALIZED *rg = r ? trgeometry_geom(r) : NULL;
    size_t n;
    char *s1 = gr ? geo_as_text(gr, 6, &n) : NULL;
    char *s2 = rg ? geo_as_text(rg, 6, &n) : NULL;
    if (! s1 || ! s2 || strcmp(s1, s2) != 0)
    {
      printf("FAILED: temporal_round of a polyhedral surface: body %s "
        "against %s\n", s2 ? s2 : "NULL", s1 ? s1 : "NULL");
      failures++;
    }
    free(t); free(r); free(g); free(gr); free(rg); free(s1); free(s2);
  }

  meos_finalize();
  printf("trgeo_generic_test: %d checks, %d failed\n", checks, failures);
  return failures ? 1 : 0;
}
