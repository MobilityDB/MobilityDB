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
 * @brief Test of the root isolation of meos/src/rgeo/trgeo_distsolve.c
 * @details The solver needs no PostgreSQL, PostGIS or MEOS header, so this
 * test compiles it directly and links only libm.  Thus the test checks the
 * same code that MEOS uses, on many random functions, in a few seconds.
 *
 * CMake builds the test in a MEOS build, with a small number of trials.  To
 * build and run it by hand, from the root of the repository:
 * @code
 * gcc -O2 -Wall -I meos/include -o trgeo_distsolve_unit \
 *     meos/test/unit/rgeo/trgeo_distsolve_unit.c \
 *     meos/src/rgeo/trgeo_distsolve.c -lm
 * ./trgeo_distsolve_unit [trials]
 * @endcode
 *
 * The checks are:
 * 1. distfun_add() gives the same values as the terms that it adds, with a
 *    phase, a negative frequency and a zero frequency.
 * 2. distfun_deriv() agrees with a central difference.
 * 3. distfun_absmax() is an upper bound.
 * 4. distfun_roots() finds all the roots: each sign change of a dense scan
 *    has a root, and each root is a root.
 * 5. The closed-form solutions agree with the general method.
 * 6. distfun_crossings() gives the sign changes of a dense scan, with their
 *    directions.
 * 7. A touch is not a crossing, a crossing at the start of the interval is
 *    not reported, and distfun_sign_after() gives the correct side.
 * 8. A touch of a function with terms gives a few roots and no crossing,
 *    and a dip just past the level gives two crossings.
 *
 * Check 4 is the most important.  A method that samples the function and
 * guesses the roots from the samples passes the other tests of the
 * distance, but it fails this check.
 */

/* C */
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
/* MEOS */
#include "rgeo/trgeo_distsolve.h"

#define SCAN 200003     /* dense scan points; prime, to avoid resonating with
                         * the solver's own uniform cell boundaries */

static uint64_t rng_state = 88172645463325252ULL;

/** @brief xorshift64, so that a failing seed reproduces everywhere */
static double
rnd(double lo, double hi)
{
  rng_state ^= rng_state << 13;
  rng_state ^= rng_state >> 7;
  rng_state ^= rng_state << 17;
  return lo + (hi - lo) * ((double) (rng_state >> 11) / 9007199254740992.0);
}

/**
 * @brief A random member of the family, and the unfolded terms that built it
 */
typedef struct
{
  double w[DISTFUN_MAXTERMS], g[DISTFUN_MAXTERMS];
  double A1[DISTFUN_MAXTERMS], A0[DISTFUN_MAXTERMS];
  double B1[DISTFUN_MAXTERMS], B0[DISTFUN_MAXTERMS];
  int n;
  double c2, c1, c0;
} Recipe;

static void
recipe_random(Recipe *r, int nterm, double wmax)
{
  r->n = nterm;
  for (int i = 0; i < nterm; i++)
  {
    r->w[i] = rnd(-wmax, wmax);
    r->g[i] = rnd(-M_PI, M_PI);
    r->A1[i] = rnd(-5, 5); r->A0[i] = rnd(-5, 5);
    r->B1[i] = rnd(-5, 5); r->B0[i] = rnd(-5, 5);
  }
  r->c2 = rnd(-3, 3); r->c1 = rnd(-3, 3); r->c0 = rnd(-3, 3);
}

static void
recipe_build(const Recipe *r, DistFun *f)
{
  distfun_init(f);
  for (int i = 0; i < r->n; i++)
    distfun_add(f, r->w[i], r->g[i], r->A1[i], r->A0[i], r->B1[i], r->B0[i]);
  distfun_add_poly(f, r->c2, r->c1, r->c0);
}

/** @brief Evaluate the recipe directly, without any folding */
static double
recipe_eval(const Recipe *r, double t)
{
  double v = (r->c2 * t + r->c1) * t + r->c0;
  for (int i = 0; i < r->n; i++)
  {
    double u = r->w[i] * t + r->g[i];
    v += (r->A1[i] * t + r->A0[i]) * cos(u) +
      (r->B1[i] * t + r->B0[i]) * sin(u);
  }
  return v;
}

int
main(int argc, char **argv)
{
  int trials = (argc > 1) ? atoi(argv[1]) : 20000;
  int fail = 0;

  /* 1. Phase, sign and zero-frequency folding */
  {
    double worst = 0.0;
    for (int k = 0; k < trials; k++)
    {
      Recipe r;
      recipe_random(&r, 1 + (int) rnd(0, 2.999), 4.0);
      if (k % 7 == 0)
        r.w[0] = 0.0;                          /* exercise the affine fold */
      DistFun f;
      recipe_build(&r, &f);
      for (int j = 0; j <= 20; j++)
      {
        double t = j / 20.0;
        double d = fabs(distfun_eval(&f, t) - recipe_eval(&r, t));
        if (d > worst)
          worst = d;
      }
    }
    printf("1. distfun_add folding vs unfolded expression   max err %.3e %s\n",
      worst, worst < 1e-11 ? "ok" : "FAIL");
    if (! (worst < 1e-11))
      fail++;
  }

  /* 2. Derivative against a central difference */
  {
    double worst = 0.0;
    for (int k = 0; k < trials; k++)
    {
      Recipe r;
      recipe_random(&r, 1 + (int) rnd(0, 2.999), M_PI);
      DistFun f, df;
      recipe_build(&r, &f);
      distfun_deriv(&f, &df);
      for (int j = 1; j < 20; j++)
      {
        double t = j / 20.0, h = 1e-5;
        double fd = (distfun_eval(&f, t + h) - distfun_eval(&f, t - h)) /
          (2.0 * h);
        double d = fabs(distfun_eval(&df, t) - fd);
        if (d > worst)
          worst = d;
      }
    }
    printf("2. distfun_deriv vs central difference          max err %.3e %s\n",
      worst, worst < 1e-6 ? "ok" : "FAIL");
    if (! (worst < 1e-6))
      fail++;
  }

  /* 3. distfun_absmax is an upper bound */
  {
    int violations = 0;
    double worst_ratio = 0.0;
    for (int k = 0; k < trials; k++)
    {
      Recipe r;
      recipe_random(&r, 1 + (int) rnd(0, 2.999), M_PI);
      DistFun f;
      recipe_build(&r, &f);
      double lo = rnd(0.0, 0.5), hi = lo + rnd(0.05, 0.5);
      double bound = distfun_absmax(&f, lo, hi), peak = 0.0;
      for (int j = 0; j <= 400; j++)
      {
        double v = fabs(distfun_eval(&f, lo + (hi - lo) * j / 400.0));
        if (v > peak)
          peak = v;
      }
      if (peak > bound * (1.0 + 1e-12))
        violations++;
      if (bound > 0.0 && peak / bound > worst_ratio)
        worst_ratio = peak / bound;
    }
    printf("3. distfun_absmax bound violations              %d %s "
      "(tightest peak/bound %.3f)\n", violations,
      violations == 0 ? "ok" : "FAIL", worst_ratio);
    if (violations)
      fail++;
  }

  /* 4. Root isolation: nothing is missed, nothing is invented */
  {
    long missed = 0, spurious = 0, nroots_total = 0, nzero = 0, nbudget = 0;
    int maxroots_seen = 0;
    for (int k = 0; k < trials; k++)
    {
      Recipe r;
      /* Most cases stay inside the paper's theta in [-pi,pi]; a tenth push
       * past it to check that the cell count keeps up */
      recipe_random(&r, 1 + (int) rnd(0, 2.999),
        (k % 10 == 0) ? 3.0 * M_PI : M_PI);
      DistFun f;
      recipe_build(&r, &f);
      double level = rnd(-4, 4);

      double roots[DISTFUN_MAXROOTS];
      int n = distfun_roots(&f, level, 0.0, 1.0, 1e-12, roots,
        DISTFUN_MAXROOTS);
      if (n == DISTFUN_ZERO) { nzero++; continue; }
      if (n == DISTFUN_BUDGET) { nbudget++; continue; }
      if (n < 0) { fail++; continue; }
      nroots_total += n;
      if (n > maxroots_seen)
        maxroots_seen = n;

      /* Every sign change of a dense scan must be matched by a returned
       * root */
      double tprev = 0.0, vprev = distfun_eval(&f, 0.0) - level;
      for (int j = 1; j <= SCAN; j++)
      {
        double t = (double) j / SCAN;
        double v = distfun_eval(&f, t) - level;
        if (vprev * v < 0.0)
        {
          bool found = false;
          for (int i = 0; i < n; i++)
            if (roots[i] >= tprev - 1e-7 && roots[i] <= t + 1e-7)
              found = true;
          if (! found)
          {
            if (missed < 3)
              printf("   MISSED root in [%.9f, %.9f] (trial %d, "
                "returned %d)\n", tprev, t, k, n);
            missed++;
          }
        }
        tprev = t; vprev = v;
      }
      /* Every returned root must be one */
      for (int i = 0; i < n; i++)
        if (fabs(distfun_eval(&f, roots[i]) - level) > 1e-6)
          spurious++;
    }
    printf("4. root isolation over %d fuzzed functions\n", trials);
    printf("     missed roots   %ld %s\n", missed, missed ? "FAIL" : "ok");
    printf("     spurious roots %ld %s\n", spurious, spurious ? "FAIL" : "ok");
    printf("     roots found    %ld (max %d in one call)\n", nroots_total,
      maxroots_seen);
    printf("     identically zero %ld, budget exhausted %ld\n", nzero,
      nbudget);
    if (missed || spurious || nbudget)
      fail++;
  }

  /* 5. The closed-form paths agree with the general path */
  {
    double worst = 0.0;
    int cases = 0;
    for (int k = 0; k < trials; k++)
    {
      DistFun f, gen;
      double roots_a[DISTFUN_MAXROOTS], roots_b[DISTFUN_MAXROOTS];
      int na, nb;
      if (k % 2 == 0)
      {
        /* Non-rotating: nterm == 0, answered as a quadratic.  The comparison
         * function adds a negligible term so that the general path runs. */
        distfun_init(&f);
        distfun_add_poly(&f, rnd(-3, 3), rnd(-3, 3), rnd(-3, 3));
        gen = f;
        distfun_add(&gen, 1.0, 0.0, 0.0, 1e-13, 0.0, 0.0);
      }
      else
      {
        /* Parallel-edge shape: one constant-amplitude sinusoid over a
         * constant, answered by direct enumeration */
        distfun_init(&f);
        distfun_add(&f, rnd(0.3, M_PI), rnd(-M_PI, M_PI), 0.0, rnd(-4, 4),
          0.0, rnd(-4, 4));
        distfun_add_poly(&f, 0.0, 0.0, rnd(-2, 2));
        gen = f;
        distfun_add_poly(&gen, 1e-13, 0.0, 0.0);
      }
      na = distfun_roots(&f, 0.0, 0.0, 1.0, 1e-12, roots_a, DISTFUN_MAXROOTS);
      nb = distfun_roots(&gen, 0.0, 0.0, 1.0, 1e-12, roots_b,
        DISTFUN_MAXROOTS);
      if (na < 0 || nb < 0 || na != nb)
      {
        if (cases < 3)
          printf("   closed form %d roots, general path %d (trial %d)\n",
            na, nb, k);
        cases++;
        continue;
      }
      for (int i = 0; i < na; i++)
        if (fabs(roots_a[i] - roots_b[i]) > worst)
          worst = fabs(roots_a[i] - roots_b[i]);
    }
    printf("5. closed form vs general path   count mismatches %d %s, "
      "max root diff %.3e\n", cases, cases ? "FAIL" : "ok", worst);
    if (cases || worst > 1e-7)
      fail++;
  }

  /* 6. Crossings agree with the sign changes of a dense scan */
  {
    long mismatch = 0, ncross = 0;
    for (int k = 0; k < trials; k++)
    {
      Recipe r;
      recipe_random(&r, 1 + (int) rnd(0, 2.999), M_PI);
      DistFun f;
      recipe_build(&r, &f);
      double level = rnd(-4, 4), roots[DISTFUN_MAXROOTS];
      int dirs[DISTFUN_MAXROOTS];
      int n = distfun_crossings(&f, level, 0.0, 1.0, 1e-12, roots, dirs,
        DISTFUN_MAXROOTS);
      if (n < 0)
        continue;
      ncross += n;
      /* The sign changes of a scan, in order, must be the crossings */
      int found = 0;
      double vprev = distfun_eval(&f, 0.0) - level;
      for (int j = 1; j <= SCAN / 20; j++)
      {
        double t = (double) j / (SCAN / 20);
        double v = distfun_eval(&f, t) - level;
        if (vprev * v < 0.0)
        {
          int dir = (v > 0.0) ? 1 : -1;
          if (found >= n || dirs[found] != dir ||
              fabs(roots[found] - t) > 2.0 / (SCAN / 20))
            mismatch++;
          found++;
        }
        vprev = v;
      }
      if (found != n)
        mismatch++;
    }
    printf("6. crossings vs sign changes of a scan   mismatches %ld %s "
      "(%ld crossings)\n", mismatch, mismatch ? "FAIL" : "ok", ncross);
    if (mismatch)
      fail++;
  }

  /* 7. A touch is not a crossing, and the side just after a time */
  {
    int bad = 0;
    for (int k = 0; k < trials; k++)
    {
      /* f = c * (t - r)^2 touches zero at r */
      double r0 = rnd(0.1, 0.9), c = rnd(0.5, 5.0) * (k % 2 ? 1 : -1);
      DistFun f;
      distfun_init(&f);
      distfun_add_poly(&f, c, -2.0 * c * r0, c * r0 * r0);
      double roots[DISTFUN_MAXROOTS];
      int dirs[DISTFUN_MAXROOTS];
      if (distfun_crossings(&f, 0.0, 0.0, 1.0, 1e-12, roots, dirs,
          DISTFUN_MAXROOTS) != 0)
        bad++;
      if (distfun_sign_after(&f, 0.0, r0, 1e-12) != (c > 0 ? 1 : -1))
        bad++;
      /* f = c * (t - r) crosses zero at r */
      distfun_init(&f);
      distfun_add_poly(&f, 0.0, c, -c * r0);
      if (distfun_sign_after(&f, 0.0, r0, 1e-12) != (c > 0 ? 1 : -1))
        bad++;
      if (distfun_first_crossing(&f, 0.0, 0.0, 1.0, 1e-12, c > 0 ? 1 : -1,
          &roots[0]) != 1 || fabs(roots[0] - r0) > 1e-12)
        bad++;
      /* A crossing at the start of the interval is not reported */
      if (distfun_first_crossing(&f, 0.0, r0, 1.0, 1e-12, c > 0 ? 1 : -1,
          &roots[0]) != 0)
        bad++;
    }
    /* A function that stays on the level has no side */
    DistFun z;
    distfun_init(&z);
    if (distfun_sign_after(&z, 0.0, 0.5, 1e-12) != 0)
      bad++;
    printf("7. touches, and side just after a time   failures %d %s\n", bad,
      bad ? "FAIL" : "ok");
    if (bad)
      fail++;
  }

  /* 8. A touch of a function with terms, which the cells find.  Near the
   * touch, many cell boundaries are within the tolerance.  They must give a
   * few roots, not more than the array holds. */
  {
    int bad = 0;
    for (int k = 0; k < trials; k++)
    {
      /* f = s * [c*(t - r)^2 + A*(1 - cos(w*(t - r)))] touches zero at r
       * only, and a scale s tests the tolerance at different sizes */
      double r0 = rnd(0.05, 0.95), c = rnd(0.1, 5.0), A = rnd(0.1, 5.0);
      double w = rnd(0.5, 20.0), s = pow(10.0, rnd(-3.0, 3.0));
      if (k % 2)
        s = -s;
      double ftol = 1e-12 * fabs(s);
      DistFun f;
      distfun_init(&f);
      distfun_add(&f, w, -w * r0, 0.0, -s * A, 0.0, 0.0);
      distfun_add_poly(&f, s * c, -2.0 * s * c * r0, s * (c * r0 * r0 + A));
      double roots[DISTFUN_MAXROOTS];
      int dirs[DISTFUN_MAXROOTS];
      int n = distfun_roots(&f, 0.0, 0.0, 1.0, ftol, roots, DISTFUN_MAXROOTS);
      if (n < 1)
        bad++;
      for (int i = 0; i < n; i++)
        if (fabs(roots[i] - r0) > 1e-4)
          bad++;
      if (distfun_crossings(&f, 0.0, 0.0, 1.0, ftol, roots, dirs,
          DISTFUN_MAXROOTS) != 0)
        bad++;
      /* A level a little inside the side of the function cuts it twice, on
       * each side of r.  For s > 0, it goes down and then up. */
      double level = 1e-6 * s;
      n = distfun_crossings(&f, level, 0.0, 1.0, ftol, roots, dirs,
        DISTFUN_MAXROOTS);
      int side = (s > 0) ? 1 : -1;
      if (n != 2 || roots[0] >= r0 || roots[1] <= r0 || dirs[0] != -side ||
          dirs[1] != side)
        bad++;
    }
    printf("8. touches that the cells find           failures %d %s\n", bad,
      bad ? "FAIL" : "ok");
    if (bad)
      fail++;
  }

  printf("\n%s\n", fail ? "FAILURES" : "all checks passed");
  return fail ? 1 : 0;
}
