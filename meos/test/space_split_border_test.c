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
 * @brief Test the split of a temporal point by a space grid on the bounds of
 * its tiles
 * @details Every tile contains its lower bound and excludes its upper one,
 * and holds exactly the microsecond instants whose exact position lies in
 * it, so each instant of a trajectory belongs to exactly one tile. With
 * `borderInc` false, a tile starting on the upper bound of the extent of the
 * trajectory is left out, and an instant whose position lies on that bound is
 * held by no tile.
 *
 * The program splits by unit tiles trajectories lying on, and within a few
 * ulps of, an upper tile bound, and random trajectories whose coordinates lie
 * on a quarter of a tile and up to 4 ulps off it, with and without the bit
 * matrix, and verifies at the input instants, at the bounds, the midpoints
 * and the neighbours of the bounds of every fragment, and around every
 * crossing, that the instant is held by exactly the tile of its exact
 * position. That tile is decided on the input doubles: the position at the
 * instant `t` of the segment from `a` at `t1` to `b` at `t1 + d` lies at or
 * above the bound `k` when `(a - k) d + (b - a) (t - t1)` is not negative,
 * whose sign is read exactly from the rounding errors of its differences and
 * products, so the program shares no arithmetic with the split.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <meos.h>
#include <meos_geo.h>

#define DAY 86400000000L
#define NAMED_E 7

typedef struct
{
  int n;
  int hasz;
  double c[4][3];
} Trip;

static long ncheck = 0, nfail = 0;

/*****************************************************************************
 * Exact sign of a sum of products of doubles
 *****************************************************************************/

/* The sum and the error of the sum of two doubles */
static void
two_sum(double a, double b, double *s, double *e)
{
  *s = a + b;
  double bv = *s - a;
  *e = (a - (*s - bv)) + (b - bv);
}

/* Add a double to a nonoverlapping expansion of increasing magnitude */
static int
grow(double *h, int n, double b)
{
  double q = b;
  int m = 0;
  for (int i = 0; i < n; i++)
  {
    double s, e;
    two_sum(q, h[i], &s, &e);
    if (e != 0.0)
      h[m++] = e;
    q = s;
  }
  if (q != 0.0)
    h[m++] = q;
  return m;
}

/* Sign of the exact sum of n doubles */
static int
sum_sign(const double *t, int n)
{
  double h[64];
  int m = 0;
  for (int i = 0; i < n; i++)
    if (t[i] != 0.0)
      m = grow(h, m, t[i]);
  return (m == 0) ? 0 : (h[m - 1] > 0.0) - (h[m - 1] < 0.0);
}

/* Sign of (a - k) * d + (b - a) * dt, exactly: each difference is its
 * rounded value plus its error, and each product of one of them by an
 * integer below 2^53 is its rounded value plus its error, read by fma */
static int
above_sign(double a, double b, double k, double d, double dt)
{
  double s1 = a - k, bv1 = s1 - a, e1 = (a - (s1 - bv1)) + (- k - bv1);
  double s2 = b - a, bv2 = s2 - b, e2 = (b - (s2 - bv2)) + (- a - bv2);
  double t[8];
  t[0] = s1 * d;  t[1] = fma(s1, d, - t[0]);
  t[2] = e1 * d;  t[3] = fma(e1, d, - t[2]);
  t[4] = s2 * dt; t[5] = fma(s2, dt, - t[4]);
  t[6] = e2 * dt; t[7] = fma(e2, dt, - t[6]);
  return sum_sign(t, 8);
}

/* Tile index along an axis of the exact position at the offset dt of the
 * segment from a to b of duration d: the integer k with
 * k <= position < k + 1 */
static int
tile_index(double a, double b, double d, double dt)
{
  double est = (a == b) ? a : a + (b - a) * (dt / d);
  int k = (int) floor(est);
  while (above_sign(a, b, (double) k, d, dt) < 0)
    k--;
  while (above_sign(a, b, (double) (k + 1), d, dt) >= 0)
    k++;
  return k;
}

/*****************************************************************************
 * The split and its judgement
 *****************************************************************************/

static void
check(const char *name, const Trip *tr, bool bitmatrix, bool border_inc)
{
  int naxes = tr->hasz ? 3 : 2;
  char buf[2048];
  int off = sprintf(buf, "[");
  for (int i = 0; i < tr->n; i++)
  {
    if (tr->hasz)
      off += sprintf(buf + off, "%sPOINT Z(%.17g %.17g %.17g)@2001-01-%02d",
        i ? "," : "", tr->c[i][0], tr->c[i][1], tr->c[i][2], i + 1);
    else
      off += sprintf(buf + off, "%sPOINT(%.17g %.17g)@2001-01-%02d",
        i ? "," : "", tr->c[i][0], tr->c[i][1], i + 1);
  }
  sprintf(buf + off, "]");
  Temporal *temp = tgeompoint_in(buf);
  TimestampTz t0 = temporal_start_timestamptz(temp);
  GSERIALIZED *origin = geom_in(tr->hasz ? "POINT Z(0 0 0)" : "POINT(0 0)",
    -1);
  GSERIALIZED **bins = NULL;
  int count = 0;
  Temporal **frags = tgeo_space_split(temp, 1.0, 1.0, 1.0, origin, bitmatrix,
    border_inc, &bins, &count);

  /* Tiles and periods of the fragments */
  int tiles[64][3];
  SpanSet *times[64];
  size_t sz;
  for (int f = 0; f < count; f++)
  {
    char *s = geo_as_text(bins[f], 17, &sz);
    double o[3] = {0, 0, 0};
    char *p = strchr(s, '(');
    if (tr->hasz)
      sscanf(p + 1, "%lf %lf %lf", &o[0], &o[1], &o[2]);
    else
      sscanf(p + 1, "%lf %lf", &o[0], &o[1]);
    free(s);
    for (int k = 0; k < 3; k++)
      tiles[f][k] = (int) o[k];
    times[f] = temporal_time(frags[f]);
  }

  /* Extent of the trajectory */
  double extmin[3], extmax[3];
  for (int k = 0; k < naxes; k++)
  {
    extmin[k] = extmax[k] = tr->c[0][k];
    for (int i = 1; i < tr->n; i++)
    {
      extmin[k] = fmin(extmin[k], tr->c[i][k]);
      extmax[k] = fmax(extmax[k], tr->c[i][k]);
    }
  }

  /* Samples */
  long samples[1024];
  int ns = 0;
  long tend = (tr->n - 1) * DAY;
  for (int i = 0; i < tr->n; i++)
    samples[ns++] = i * DAY;
  for (int i = 0; i + 1 < tr->n; i++)
    for (int k = 0; k < naxes; k++)
    {
      double a = tr->c[i][k], b = tr->c[i + 1][k];
      if (a == b)
        continue;
      for (int m = (int) ceil(fmin(a, b)); m <= (int) floor(fmax(a, b)) &&
          ns < 1000; m++)
      {
        long est = i * DAY + (long) ((m - a) / (b - a) * DAY);
        for (long u = est - 2; u <= est + 2; u++)
          samples[ns++] = u;
      }
    }
  for (int f = 0; f < count; f++)
    for (int j = 1; j <= spanset_num_spans(times[f]) && ns < 1015; j++)
    {
      Span *sp = spanset_span_n(times[f], j);
      long lo = tstzspan_lower(sp) - t0, up = tstzspan_upper(sp) - t0;
      free(sp);
      long ss[5] = {lo, up, (lo + up) / 2, lo + 1, up - 1};
      for (int q = 0; q < 5; q++)
        samples[ns++] = ss[q];
    }

  /* Judge every sample */
  for (int s = 0; s < ns; s++)
  {
    long t = samples[s];
    if (t < 0 || t > tend)
      continue;
    int i = (int) (t / DAY);
    if (i >= tr->n - 1)
      i = tr->n - 2;
    double dt = (double) (t - i * DAY);
    int want[3] = {0, 0, 0};
    bool dropped = false;
    for (int k = 0; k < naxes; k++)
    {
      want[k] = tile_index(tr->c[i][k], tr->c[i + 1][k], (double) DAY, dt);
      if (! border_inc && extmin[k] < extmax[k] &&
          (double) want[k] == extmax[k])
        dropped = true;
    }
    int holders = 0;
    bool right = true;
    for (int f = 0; f < count; f++)
      if (contains_spanset_timestamptz(times[f], t + t0))
      {
        holders++;
        for (int k = 0; k < naxes; k++)
          if (tiles[f][k] != want[k])
            right = false;
      }
    ncheck++;
    if (dropped ? holders != 0 : (holders != 1 || ! right))
    {
      nfail++;
      printf("FAIL %s bitmatrix=%d borderInc=%d t=%ld tile=(%d %d %d) "
        "holders=%d %s\n", name, bitmatrix, border_inc, t, want[0], want[1],
        want[2], holders, buf);
    }
  }
  for (int f = 0; f < count; f++)
  {
    free(times[f]);
    free(frags[f]);
    free(bins[f]);
  }
  free(frags);
  free(bins);
  free(origin);
  free(temp);
}

int
main(void)
{
  meos_initialize();
  double es[NAMED_E] = {0.0, 0x1p-53, 0x1p-52, 0x1p-51, 1e-12, 5e-7, 1e-6};
  for (int bm = 1; bm >= 0; bm--)
    for (int inc = 1; inc >= 0; inc--)
      for (int d = 0; d < NAMED_E; d++)
      {
        double e = es[d];
        Trip named[] = {
          {2, 0, {{1 - e, 0.2}, {1 - e, 0.8}}},
          {2, 0, {{1 + e, 0.2}, {1 + e, 0.8}}},
          {2, 0, {{0.2, 1 - e}, {0.8, 1 - e}}},
          {2, 0, {{0.5, 0.5}, {1 - e, 0.5}}},
          {2, 0, {{1 - e, 0.5}, {1.5, 0.5}}},
          {2, 0, {{0.5, 1 - e}, {1.5, 1.25 - e}}},
          {3, 0, {{0.5, 0.5}, {1 - e, 0.5}, {0.5, 0.7}}},
          {2, 0, {{0.5, 0.5}, {1 - e, 1 - e}}},
          {2, 0, {{0.5 - e, 0.5}, {1.5 - e, 1.5}}},
          {2, 1, {{0.2, 0.2, 1 - e}, {0.8, 0.8, 1 - e}}},
        };
        for (size_t j = 0; j < sizeof(named) / sizeof(Trip); j++)
        {
          char name[64];
          sprintf(name, "named %zu e=%g", j, e);
          check(name, &named[j], bm, inc);
        }
      }
  srand(20261110);
  for (int r = 0; r < 2000; r++)
  {
    Trip tr = {4, r % 4 == 0, {{0}}};
    for (int i = 0; i < 4; i++)
      for (int k = 0; k < (tr.hasz ? 3 : 2); k++)
      {
        double v = (rand() % 13) / 4.0;
        int u = rand() % 9 - 4;
        for (; u > 0; u--)
          v = nextafter(v, 10);
        for (; u < 0; u++)
          v = nextafter(v, -10);
        tr.c[i][k] = v;
      }
    char name[64];
    sprintf(name, "random %d", r);
    check(name, &tr, true, r % 2);
    check(name, &tr, false, r % 2);
  }
  printf("%ld checks, %ld failures\n", ncheck, nfail);
  meos_finalize();
  return nfail == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
