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
 * @brief Randomized test of the closest-feature walk of
 * meos/src/rgeo/trgeo_distwalk.c
 * @details The test runs the walk on random segments and compares the events
 * with the true distance, see #tu_check().  Body A is a random convex polygon
 * that moves and rotates.  Body B is, in turn:
 * - a static point;
 * - a moving point;
 * - a static segment;
 * - a moving and rotating segment;
 * - a static convex polygon;
 * - a moving and rotating convex polygon, which rotates at the same rate as
 *   A in half of the cases.  Then pairs of edges keep their relative
 *   direction, and some can stay parallel for the whole segment.
 *
 * Half of the segments keep B away from the path of A, so that both the
 * disjoint and the overlapping cases are frequent.  Half of the segments
 * also give a random distance level, whose crossings must be events, as for
 * tDwithin.
 *
 * The argument is the number of segments.  ctest uses a small number; run
 * the program with 20000 for a deeper check.
 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "trgeo_distwalk_testutil.h"

extern void meos_initialize(void);
extern void meos_finalize(void);

static uint64_t rs = 0x0123456789ABCDEFULL;

/** @brief Return a random number in [lo, hi) with xorshift64, which gives the
 * same sequence on all platforms */
static double
rnd(double lo, double hi)
{
  rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17;
  return lo + (hi - lo) * ((double) (rs >> 11) / 9007199254740992.0);
}

static int
cmp_point(const void *a, const void *b)
{
  const double *x = a, *y = b;
  if (x[0] != y[0])
    return x[0] < y[0] ? -1 : 1;
  return x[1] < y[1] ? -1 : (x[1] > y[1]);
}

static double
cross3(const double *o, const double *a, const double *b)
{
  return (a[0] - o[0]) * (b[1] - o[1]) - (a[1] - o[1]) * (b[0] - o[0]);
}

/**
 * @brief Set @p out to the convex hull of 3 to 10 random points in a square
 * of half side @p r, and return its number of vertices
 */
static int
random_hull(double *out, double r)
{
  double pts[20] = {0};
  int n = 3 + (int) rnd(0, 7.99);
  for (int i = 0; i < n; i++)
  {
    pts[2 * i] = rnd(-r, r);
    pts[2 * i + 1] = rnd(-r, r);
  }
  /* Andrew's monotone chain */
  qsort(pts, n, 2 * sizeof(double), cmp_point);
  int k = 0;
  for (int i = 0; i < n; i++)
  {
    while (k >= 2 && cross3(out + 2 * (k - 2), out + 2 * (k - 1), pts + 2 * i) <= 0)
      k--;
    out[2 * k] = pts[2 * i]; out[2 * k + 1] = pts[2 * i + 1]; k++;
  }
  int lo = k + 1;
  for (int i = n - 2; i >= 0; i--)
  {
    while (k >= lo && cross3(out + 2 * (k - 2), out + 2 * (k - 1), pts + 2 * i) <= 0)
      k--;
    out[2 * k] = pts[2 * i]; out[2 * k + 1] = pts[2 * i + 1]; k++;
  }
  return k - 1;
}

int
main(int argc, char **argv)
{
  meos_initialize();
  int trials = (argc > 1) ? atoi(argv[1]) : 4000;
#define NKIND 6
  const char *names[NKIND] = {"static point", "moving point",
    "static segment", "moving segment", "static polygon", "moving polygon"};
  long runs[NKIND] = {0}, fails[NKIND] = {0}, overlaps[NKIND] = {0},
    bad[NKIND] = {0};
  double worst_err = 0.0, worst_nad = 0.0;
  long total_events = 0;
  DistEvents events;
  distevents_init(&events);

  for (int k = 0; k < trials; k++)
  {
    int kind = k % NKIND;
    bool point = (kind < 2), segment = (kind == 2 || kind == 3);
    bool moving = (kind % 2 == 1);
    double va[TU_MAXVERT * 2], vb[TU_MAXVERT * 2];
    int n = random_hull(va, 4.0);
    int m;
    if (point)
    {
      m = 1;
      vb[0] = vb[1] = 0.0;
    }
    else if (segment)
    {
      /* A segment with its middle at the origin */
      m = 2;
      vb[0] = rnd(-3, 3); vb[1] = rnd(-3, 3);
      vb[2] = -vb[0]; vb[3] = -vb[1];
    }
    else
      m = random_hull(vb, 3.0);
    DistRefPoly ra, rb;
    if (n < 3 || (! point && ! segment && m < 3) || ! tu_refpoly(&ra, va, n))
      continue;
    if (! tu_refpoly(&rb, vb, m))
    {
      distrefpoly_free(&ra);
      continue;
    }

    DistMotion ma, mb;
    double th1 = rnd(-M_PI, M_PI), th2 = rnd(-M_PI, M_PI);
    distmotion_set(&ma, rnd(-15, -5), rnd(-6, 6), th1, rnd(5, 15),
      rnd(-6, 6), th2);
    double spread = ((k / NKIND) % 2) ? 16.0 : 8.0;
    if (! moving)
      distmotion_set_static(&mb, rnd(-6, 6), rnd(-spread, spread));
    else if (point)
      distmotion_set(&mb, rnd(-6, 6), rnd(-spread, spread), 0.0,
        rnd(-6, 6), rnd(-spread, spread), 0.0);
    else if ((k / (2 * NKIND)) % 2)
    {
      /* Same rotation rate as A */
      double off = rnd(-M_PI, M_PI);
      distmotion_set(&mb, rnd(-6, 6), rnd(-spread, spread), th1 + off,
        rnd(-6, 6), rnd(-spread, spread), th1 + off + ma.w);
      mb.w = ma.w;
    }
    else
      distmotion_set(&mb, rnd(-6, 6), rnd(-spread, spread), rnd(-M_PI, M_PI),
        rnd(-6, 6), rnd(-spread, spread), rnd(-M_PI, M_PI));
    DistSegm seg;
    distsegm_set(&seg, &ma, &mb);

    DistPair cf;
    cf.kind = DISTPAIR_NONE;
    cf.i = cf.j = 0;
    events.count = 0;
    /* A distance level, as for tDwithin, in half of the segments */
    double level = (k % 2) ? rnd(0.0, 6.0) : -1.0;
    int rc = distwalk_segm(&seg, &ra, &rb, distwalk_ftol(&seg, &ra, &rb),
      level, &cf, &events);
    runs[kind]++;
    if (rc != DISTWALK_OK)
      fails[kind]++;
    else
    {
      double ca[TU_MAXVERT * 2], cb[TU_MAXVERT * 2];
      n = tu_vertices(&ra, ca);
      m = tu_vertices(&rb, cb);
      TuCheck res;
      tu_check(&ma, ca, n, &mb, cb, m, &events, 4000, level, &res);
      total_events += events.count;
      if (res.overlap)
        overlaps[kind]++;
      if (res.maxerr > worst_err)
        worst_err = res.maxerr;
      if (res.nadgap > worst_nad)
        worst_nad = res.nadgap;
      if (res.maxerr > 1e-7 || res.order || res.below || res.nadgap > 1e-6 ||
          res.nonzero_overlap || res.zero_apart || res.level_side)
      {
        if (bad[kind] < 3)
          printf("  FAIL %s segment %d: err %.2e order %d below %d nad %.2e "
            "nonzero-overlap %d zero-apart %d level-side %d\n", names[kind],
            k, res.maxerr, res.order, res.below, res.nadgap,
            res.nonzero_overlap, res.zero_apart, res.level_side);
        bad[kind]++;
      }
    }
    distrefpoly_free(&ra);
    distrefpoly_free(&rb);
  }
  distevents_free(&events);

  long nbad = 0, nruns = 0;
  printf("%-16s %8s %8s %8s %8s\n", "second body", "segments", "overlap",
    "failed", "wrong");
  for (int i = 0; i < NKIND; i++)
  {
    printf("%-16s %8ld %8ld %8ld %8ld\n", names[i], runs[i], overlaps[i],
      fails[i], bad[i]);
    nbad += fails[i] + bad[i];
    nruns += runs[i];
  }
  printf("events per segment %.1f, worst event error %.2e, worst NAD gap "
    "%.2e\n", nruns ? (double) total_events / nruns : 0.0, worst_err,
    worst_nad);
  printf("\n%s\n", nbad ? "FAILURES" : "all checks passed");
  meos_finalize();
  return nbad ? 1 : 0;
}
