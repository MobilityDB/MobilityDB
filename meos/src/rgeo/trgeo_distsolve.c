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
 * @brief Function family and root isolation for the distance between rigid
 * geometries
 * @details Every equation that the distance algorithm solves has the form
 * @code
 * F(t) = SUM_k [ (a1_k*t + a0_k) * cos(w_k*t) + (b1_k*t + b0_k) * sin(w_k*t) ]
 *      + c2*t^2 + c1*t + c0
 * @endcode
 * This file evaluates such functions, differentiates them, and finds all
 * their roots on an interval.
 *
 * The file includes no PostgreSQL, PostGIS or MEOS header and does no
 * allocation, so that it builds on its own, without libmeos.  This lets
 * meos/test/unit/rgeo/trgeo_distsolve_unit.c test the root isolation against
 * dense sampling on millions of random functions, with the same code that
 * MEOS uses.  Do not add palloc, Assert or meos_error: they need MEOS, and
 * Assert and meos_error can jump out of a function, which would also make the
 * control flow differ between the two builds.
 *
 * See meos/src/rgeo/trgeo_distance.txt for the derivation of the family.
 */

#include "rgeo/trgeo_distsolve.h"

/* C */
#include <assert.h>
#include <math.h>
#include <stddef.h>

/** Maximum number of cells on the subdivision stack.  The first split gives
 * at most 64 cells and each bisection level adds one cell to the stack. */
#define DISTFUN_STACKMAX   160

/** Maximum number of cells that the subdivision processes */
#define DISTFUN_CELLMAX    4000

/*****************************************************************************
 * Static helpers
 *****************************************************************************/

/**
 * @brief Return the maximum of |(a1*t + a0)*cos(w*t) + (b1*t + b0)*sin(w*t)|
 * over [lo, hi]
 * @details The maximum is at most the amplitude hypot(a1*t + a0, b1*t + b0).
 * The amplitude is the distance from the origin to a point that moves on a
 * line, thus it is convex and its maximum is at an end of the interval.
 */
static double
term_absmax(const DistFunTerm *k, double lo, double hi)
{
  double vlo = hypot(k->a1 * lo + k->a0, k->b1 * lo + k->b0);
  double vhi = hypot(k->a1 * hi + k->a0, k->b1 * hi + k->b0);
  return (vlo > vhi) ? vlo : vhi;
}

/**
 * @brief Return the maximum of |c2*t^2 + c1*t + c0| over [lo, hi]
 * @details The maximum is at an end of the interval or at the vertex of the
 * parabola.
 */
static double
quad_absmax(double c2, double c1, double c0, double lo, double hi)
{
  double best = fabs(c2 * lo * lo + c1 * lo + c0);
  double v = fabs(c2 * hi * hi + c1 * hi + c0);
  if (v > best)
    best = v;
  if (c2 != 0.0)
  {
    double tv = -c1 / (2.0 * c2);
    if (tv > lo && tv < hi)
    {
      v = fabs(c2 * tv * tv + c1 * tv + c0);
      if (v > best)
        best = v;
    }
  }
  return best;
}

/**
 * @brief Insert @p t in the sorted array of roots, unless the array already
 * has a root at a distance of at most @p xacc
 * @return False if the array is full
 */
static bool
root_insert(double *roots, int *nroots, int maxroots, double t, double xacc)
{
  int i;
  for (i = 0; i < *nroots; i++)
  {
    if (fabs(roots[i] - t) <= xacc)
      return true;
    if (roots[i] > t)
      break;
  }
  if (*nroots >= maxroots)
    return false;
  for (int j = *nroots; j > i; j--)
    roots[j] = roots[j - 1];
  roots[i] = t;
  (*nroots)++;
  return true;
}

/**
 * @brief Return true if |g| <= @p ftol in the middle of [@p a, @p b]
 */
static inline bool
gap_is_zero(const DistFun *g, double a, double b, double ftol)
{
  return fabs(distfun_eval(g, 0.5 * (a + b))) <= ftol;
}

/**
 * @brief Remove each root that is inside a group of roots between which
 * |g| <= @p ftol
 * @details Near a touch of zero, g stays within @p ftol on a small interval,
 * and each cell boundary in that interval is a root.  A touch can thus give
 * tens of roots.  The group is one root for #distfun_crossings(), which
 * reads the sign in the gaps, thus only its first and last members are
 * necessary.  With them, the number of roots does not depend on the
 * subdivision.
 */
static void
root_compact(const DistFun *g, double ftol, double *roots, int *nroots)
{
  int k = 1;
  while (k < *nroots - 1)
  {
    if (gap_is_zero(g, roots[k - 1], roots[k], ftol) &&
        gap_is_zero(g, roots[k], roots[k + 1], ftol))
    {
      for (int j = k; j < *nroots - 1; j++)
        roots[j] = roots[j + 1];
      (*nroots)--;
      /* The removal gives a new gap before the root at k */
      if (k > 1)
        k--;
    }
    else
      k++;
  }
}

/**
 * @brief Insert @p t in the sorted array of roots of @p g, and remove the
 * roots that are then inside a group, see #root_compact()
 * @details The array has room for one root more than @p maxroots, because a
 * new root can be inside a group and go at once.
 * @return False if the array is full
 */
static bool
root_add(const DistFun *g, double ftol, double *roots, int *nroots,
  int maxroots, double t, double xacc)
{
  if (! root_insert(roots, nroots, maxroots + 1, t, xacc))
    return false;
  root_compact(g, ftol, roots, nroots);
  return true;
}

/**
 * @brief Insert the roots of c2*t^2 + c1*t + c0 in [lo, hi]
 * @details The larger root comes from the quadratic formula and the smaller
 * root comes from the product of the roots.  This form does not subtract two
 * almost equal numbers.
 * @return False if the array of roots is full
 */
static bool
quad_roots(double c2, double c1, double c0, double lo, double hi,
  double *roots, int *nroots, int maxroots, double xacc)
{
  if (c2 == 0.0)
  {
    if (c1 == 0.0)
      return true;
    double t = -c0 / c1;
    if (t >= lo && t <= hi)
      return root_insert(roots, nroots, maxroots, t, xacc);
    return true;
  }
  double disc = c1 * c1 - 4.0 * c2 * c0;
  if (disc < 0.0)
    return true;
  double sd = sqrt(disc);
  double q = -0.5 * (c1 + ((c1 >= 0.0) ? sd : -sd));
  double t1 = q / c2;
  if (t1 >= lo && t1 <= hi && ! root_insert(roots, nroots, maxroots, t1, xacc))
    return false;
  if (q != 0.0)
  {
    double t2 = c0 / q;
    if (t2 >= lo && t2 <= hi &&
        ! root_insert(roots, nroots, maxroots, t2, xacc))
      return false;
  }
  return true;
}

/**
 * @brief Insert the roots of A*cos(w*t) + B*sin(w*t) + C in [lo, hi]
 * @details With R = hypot(A, B) and phi = atan2(B, A), the equation is
 * cos(w*t - phi) = -C/R, which has closed-form solutions.
 * @return 1 on success, 0 if the array of roots is full, and -1 if the
 * enumeration is too long, in which case the caller uses the general method
 */
static int
sinusoid_roots(double w, double A, double B, double C, double lo, double hi,
  double *roots, int *nroots, int maxroots, double xacc)
{
  double R = hypot(A, B);
  if (R == 0.0)
    return -1;
  double ratio = -C / R;
  if (ratio > 1.0 || ratio < -1.0)
    return 1;
  double phi = atan2(B, A);
  double u0 = acos(ratio);
  double twopi = 2.0 * M_PI;
  double klo = floor((w * lo - phi - M_PI) / twopi) - 1.0;
  double khi = ceil((w * hi - phi + M_PI) / twopi) + 1.0;
  if (! (khi - klo <= 64.0))
    return -1;
  for (double k = klo; k <= khi; k += 1.0)
  {
    for (int s = 0; s < 2; s++)
    {
      double u = phi + (s ? u0 : -u0) + k * twopi;
      double t = u / w;
      if (t >= lo && t <= hi &&
          ! root_insert(roots, nroots, maxroots, t, xacc))
        return 0;
    }
  }
  return 1;
}

/**
 * @brief Return the root of @p f in [x1, x2], where @p f changes sign
 * @details Newton's method, with a bisection step when the Newton step leaves
 * the bracket or does not reduce the bracket quickly.  The bracket must never
 * be lost, because a point where f' = 0 can be arbitrarily near the root,
 * and there a Newton step can go far away.
 */
static double
root_refine(const DistFun *f, const DistFun *df, double x1, double x2,
  double f1, double xacc)
{
  double xl, xh;
  if (f1 < 0.0)
  {
    xl = x1; xh = x2;
  }
  else
  {
    xl = x2; xh = x1;
  }
  double rts = 0.5 * (x1 + x2);
  double dxold = fabs(x2 - x1), dx = dxold;
  double fv = distfun_eval(f, rts), dv = distfun_eval(df, rts);
  for (int j = 0; j < 100; j++)
  {
    if ((((rts - xh) * dv - fv) * ((rts - xl) * dv - fv) > 0.0) ||
        (fabs(2.0 * fv) > fabs(dxold * dv)))
    {
      dxold = dx;
      dx = 0.5 * (xh - xl);
      rts = xl + dx;
      if (xl == rts)
        return rts;
    }
    else
    {
      dxold = dx;
      dx = fv / dv;
      double prev = rts;
      rts -= dx;
      if (prev == rts)
        return rts;
    }
    if (fabs(dx) < xacc)
      return rts;
    fv = distfun_eval(f, rts);
    dv = distfun_eval(df, rts);
    if (fv < 0.0)
      xl = rts;
    else
      xh = rts;
  }
  return rts;
}

/**
 * @brief Return the sign of @p v, or 0 if |v| <= @p tol
 */
static inline int
sign_tol(double v, double tol)
{
  return (v > tol) ? 1 : ((v < -tol) ? -1 : 0);
}

/*****************************************************************************
 * Construction
 *****************************************************************************/

/**
 * @brief Set @p f to the zero function
 */
void
distfun_init(DistFun *f)
{
  assert(f);
  f->nterm = 0;
  f->c2 = f->c1 = f->c0 = 0.0;
}

/**
 * @brief Add (A1*t + A0)*cos(w*t + g) + (B1*t + B0)*sin(w*t + g) to @p f
 * @details The function stores each term with phase zero and with a
 * frequency that is not negative.  A term of frequency zero is added to the
 * polynomial part, and a term of a frequency that @p f already has is added
 * to that term.  Thus two functions of the same segment have the same
 * frequencies, and their sum or difference is a sum of terms.
 *
 * The frequencies are compared exactly.  A rotation below #MEOS_EPSILON is
 * already exactly zero (see motion_omega() in trgeo_distwalk.c), and two
 * bodies that rotate at the same rate give a frequency wA - wB that is
 * exactly zero.  A frequency that is only close to zero stays a term, which
 * is exact and costs nothing.
 * @return False if @p f already has #DISTFUN_MAXTERMS terms
 */
bool
distfun_add(DistFun *f, double w, double g, double A1, double A0, double B1,
  double B0)
{
  assert(f);
  /* cos(w*t + g) = cos(g)*cos(w*t) - sin(g)*sin(w*t) and
   * sin(w*t + g) = cos(g)*sin(w*t) + sin(g)*cos(w*t) */
  double cg = cos(g), sg = sin(g);
  double a1 = A1 * cg + B1 * sg;
  double a0 = A0 * cg + B0 * sg;
  double b1 = B1 * cg - A1 * sg;
  double b0 = B0 * cg - A0 * sg;
  /* cos is even and sin is odd */
  if (w < 0.0)
  {
    w = -w;
    b1 = -b1;
    b0 = -b0;
  }
  if (w == 0.0)
  {
    f->c1 += a1;
    f->c0 += a0;
    return true;
  }
  for (int i = 0; i < f->nterm; i++)
  {
    if (f->term[i].w == w)
    {
      f->term[i].a1 += a1;
      f->term[i].a0 += a0;
      f->term[i].b1 += b1;
      f->term[i].b0 += b0;
      return true;
    }
  }
  if (f->nterm >= DISTFUN_MAXTERMS)
    return false;
  f->term[f->nterm].w = w;
  f->term[f->nterm].a1 = a1;
  f->term[f->nterm].a0 = a0;
  f->term[f->nterm].b1 = b1;
  f->term[f->nterm].b0 = b0;
  f->nterm++;
  return true;
}

/**
 * @brief Add c2*t^2 + c1*t + c0 to @p f
 */
void
distfun_add_poly(DistFun *f, double c2, double c1, double c0)
{
  assert(f);
  f->c2 += c2;
  f->c1 += c1;
  f->c0 += c0;
}

/*****************************************************************************
 * Queries
 *****************************************************************************/

/**
 * @brief Return the value of @p f at @p t
 */
double
distfun_eval(const DistFun *f, double t)
{
  assert(f);
  double res = (f->c2 * t + f->c1) * t + f->c0;
  for (int i = 0; i < f->nterm; i++)
  {
    const DistFunTerm *k = &f->term[i];
    double u = k->w * t;
    res += (k->a1 * t + k->a0) * cos(u) + (k->b1 * t + k->b0) * sin(u);
  }
  return res;
}

/**
 * @brief Set @p df to the derivative of @p f
 * @details The derivative is in the same family, with the same frequencies.
 * @p df can be the same as @p f.
 */
void
distfun_deriv(const DistFun *f, DistFun *df)
{
  assert(f); assert(df);
  for (int i = 0; i < f->nterm; i++)
  {
    const DistFunTerm *k = &f->term[i];
    double w = k->w;
    double a1 = w * k->b1;
    double a0 = k->a1 + w * k->b0;
    double b1 = -w * k->a1;
    double b0 = k->b1 - w * k->a0;
    df->term[i].w = w;
    df->term[i].a1 = a1;
    df->term[i].a0 = a0;
    df->term[i].b1 = b1;
    df->term[i].b0 = b0;
  }
  df->nterm = f->nterm;
  double c1 = 2.0 * f->c2, c0 = f->c1;
  df->c2 = 0.0;
  df->c1 = c1;
  df->c0 = c0;
}

/**
 * @brief Return an upper bound of |f| over [lo, hi]
 * @details The bound can be too large but never too small: a bound that is
 * too large only makes #distfun_roots() split one more cell, while a bound
 * that is too small can make it discard a cell that has a root.
 */
double
distfun_absmax(const DistFun *f, double lo, double hi)
{
  assert(f);
  double res = quad_absmax(f->c2, f->c1, f->c0, lo, hi);
  for (int i = 0; i < f->nterm; i++)
    res += term_absmax(&f->term[i], lo, hi);
  return res;
}

/**
 * @brief Return true if no coefficient of @p f is larger than @p ftol
 * @details On the interval [0, 1], each coefficient is a bound of the value
 * that it adds, so @p f is zero to within the tolerance.  The test uses the
 * coefficients and not values at sample points, so that the result does not
 * depend on where the caller looks.
 */
bool
distfun_is_zero(const DistFun *f, double ftol)
{
  assert(f);
  if (fabs(f->c2) > ftol || fabs(f->c1) > ftol || fabs(f->c0) > ftol)
    return false;
  for (int i = 0; i < f->nterm; i++)
  {
    const DistFunTerm *k = &f->term[i];
    if (fabs(k->a1) > ftol || fabs(k->a0) > ftol || fabs(k->b1) > ftol ||
        fabs(k->b0) > ftol)
      return false;
  }
  return true;
}

/**
 * @brief Return the side of @p level on which @p f is just after @p t
 * @details The walk asks this at the times of its events, where f is often
 * exactly on the level, so the value alone cannot decide.  The result is the
 * sign of the first term of the Taylor series of f - level at @p t that is
 * larger than @p ftol: the value, then the first derivative, then the second
 * derivative.  Thus a function that crosses the level at @p t gets the sign
 * of its first derivative, and a function that touches the level gets the
 * sign of its second derivative.
 * @return 1 above the level, -1 below the level, and 0 if the three terms are
 * all within the tolerance
 */
int
distfun_sign_after(const DistFun *f, double level, double t, double ftol)
{
  assert(f);
  int s = sign_tol(distfun_eval(f, t) - level, ftol);
  if (s != 0)
    return s;
  DistFun df;
  distfun_deriv(f, &df);
  s = sign_tol(distfun_eval(&df, t), ftol);
  if (s != 0)
    return s;
  distfun_deriv(&df, &df);
  return sign_tol(distfun_eval(&df, t), ftol);
}

/*****************************************************************************
 * Root isolation
 *****************************************************************************/

/**
 * @brief Find all the solutions of f(t) = @p level in [@p lo, @p hi]
 * @details The function splits the interval into cells.  The number of cells
 * makes each cell cover at most a quarter turn of the phase of @p f.  For
 * each cell, the function does one of these steps:
 * - If @p f changes sign in the cell, it refines the root.
 * - If |f(m)| > (h/2) * max|f'| over the cell, with m the middle and h the
 *   width of the cell, the cell has no root.
 * - If |f'(m)| > (h/2) * max|f''| over the cell, @p f is monotonic in the
 *   cell.  Since @p f does not change sign, the cell has no root.
 * - Otherwise, it splits the cell in two.
 *
 * A value at a cell boundary that is within @p ftol of the level is also a
 * root.  Thus a function that touches the level gives one or more roots.
 * Near a touch, many cell boundaries are within @p ftol.  Of a group of
 * such roots, the function keeps the first and the last, see
 * root_compact().
 *
 * Two cases have closed-form solutions: a function without terms, which is a
 * quadratic, and a single term of constant amplitude over a constant.
 *
 * @param[in] f Function
 * @param[in] level Value to solve for
 * @param[in] lo,hi Interval, with @p lo < @p hi
 * @param[in] ftol Values with a magnitude of at most @p ftol are zero
 * @param[out] roots Solutions, in increasing order
 * @param[in] maxroots Size of @p roots.  The function gives at most
 * #DISTFUN_MAXROOTS roots.
 * @return Number of roots, or #DISTFUN_ZERO, #DISTFUN_BUDGET,
 * #DISTFUN_OVERFLOW or #DISTFUN_NONFINITE
 */
int
distfun_roots(const DistFun *f, double level, double lo, double hi,
  double ftol, double *roots, int maxroots)
{
  assert(f); assert(roots); assert(maxroots > 0); assert(lo < hi);

  DistFun g = *f;
  g.c0 -= level;

  if (! isfinite(g.c2) || ! isfinite(g.c1) || ! isfinite(g.c0))
    return DISTFUN_NONFINITE;
  double wsum = 0.0;
  for (int i = 0; i < g.nterm; i++)
  {
    const DistFunTerm *k = &g.term[i];
    if (! isfinite(k->w) || ! isfinite(k->a1) || ! isfinite(k->a0) ||
        ! isfinite(k->b1) || ! isfinite(k->b0))
      return DISTFUN_NONFINITE;
    wsum += k->w;
  }
  if (distfun_is_zero(&g, ftol))
    return DISTFUN_ZERO;

  /* Two roots that are closer than this are the same root */
  double xacc = (hi - lo) * 1e-14;
  if (xacc < 1e-16)
    xacc = 1e-16;
  int nroots = 0;

  /* A quadratic */
  if (g.nterm == 0)
  {
    if (! quad_roots(g.c2, g.c1, g.c0, lo, hi, roots, &nroots, maxroots, xacc))
      return DISTFUN_OVERFLOW;
    return nroots;
  }

  /* One term of constant amplitude over a constant */
  if (g.nterm == 1 && g.term[0].a1 == 0.0 && g.term[0].b1 == 0.0 &&
      g.c2 == 0.0 && g.c1 == 0.0)
  {
    int rc = sinusoid_roots(g.term[0].w, g.term[0].a0, g.term[0].b0, g.c0, lo,
      hi, roots, &nroots, maxroots, xacc);
    if (rc == 0)
      return DISTFUN_OVERFLOW;
    if (rc == 1)
      return nroots;
  }

  DistFun dg, d2g;
  distfun_deriv(&g, &dg);
  distfun_deriv(&dg, &d2g);

  /* The number of cells comes from the coefficients, not from a guess.  The
   * phase of the terms advances by at most (hi - lo) * SUM w, plus pi because
   * the affine amplitude of a term turns by less than a half turn. */
  double phase = (hi - lo) * wsum + M_PI;
  int ncell = (int) ceil(phase / (M_PI / 4.0));
  if (ncell < 4)
    ncell = 4;
  if (ncell > 64)
    ncell = 64;

  /* The roots go to an array with room for one more root, see root_add() */
  double buf[DISTFUN_MAXROOTS + 1];
  int cap = (maxroots < DISTFUN_MAXROOTS) ? maxroots : DISTFUN_MAXROOTS;
  nroots = 0;

  /* The stack gives the cells in increasing order */
  double stack[DISTFUN_STACKMAX][2];
  int nstack = 0;
  double step = (hi - lo) / ncell;
  for (int i = ncell - 1; i >= 0; i--)
  {
    stack[nstack][0] = lo + step * i;
    stack[nstack][1] = (i == ncell - 1) ? hi : lo + step * (i + 1);
    nstack++;
  }

  int ncells = 0;
  while (nstack > 0)
  {
    if (++ncells > DISTFUN_CELLMAX)
      return DISTFUN_BUDGET;
    nstack--;
    double a = stack[nstack][0], b = stack[nstack][1];
    double va = distfun_eval(&g, a), vb = distfun_eval(&g, b);

    if (fabs(va) <= ftol &&
        ! root_add(&g, ftol, buf, &nroots, cap, a, xacc))
      return DISTFUN_OVERFLOW;
    if (fabs(vb) <= ftol &&
        ! root_add(&g, ftol, buf, &nroots, cap, b, xacc))
      return DISTFUN_OVERFLOW;

    if (va * vb < 0.0)
    {
      double t = root_refine(&g, &dg, a, b, va, xacc);
      if (! root_add(&g, ftol, buf, &nroots, cap, t, xacc))
        return DISTFUN_OVERFLOW;
      continue;
    }
    if (b - a <= xacc)
      continue;

    double m = 0.5 * (a + b);
    double half = 0.5 * (b - a);
    /* No point of the cell is farther than half its width from the middle,
     * thus f cannot reach zero in the cell */
    if (fabs(distfun_eval(&g, m)) > half * distfun_absmax(&dg, a, b))
      continue;
    /* The same test for f' proves that f is monotonic in the cell.  Near a
     * root, f is small and the first test fails, but f' is not small.
     * Without this test, the cells next to a root split until the budget is
     * spent. */
    if (fabs(distfun_eval(&dg, m)) > half * distfun_absmax(&d2g, a, b))
      continue;

    if (nstack + 2 > DISTFUN_STACKMAX)
      return DISTFUN_BUDGET;
    stack[nstack][0] = m; stack[nstack][1] = b; nstack++;
    stack[nstack][0] = a; stack[nstack][1] = m; nstack++;
  }
  if (nroots > cap)
    return DISTFUN_OVERFLOW;
  for (int i = 0; i < nroots; i++)
    roots[i] = buf[i];
  return nroots;
}

/**
 * @brief Find the solutions of f(t) = @p level in [@p lo, @p hi] at which
 * @p f crosses the level
 * @details Between two consecutive roots, @p f does not change sign, because
 * #distfun_roots() finds all the roots.  Thus the function reads the sign of
 * f - level at the middle of each gap between roots.  A root is a crossing if
 * the gaps before and after it have opposite signs.  A gap in which
 * |f - level| <= @p ftol gives no sign, and a group of roots with such gaps
 * between them is one root at its first member.
 *
 * A root at @p lo or @p hi has no gap on one side, thus it is never a
 * crossing.  A touch has the same sign on the two sides, thus it is never a
 * crossing.
 * @param[out] roots Crossings, in increasing order
 * @param[out] dirs Direction of each crossing: 1 if @p f goes up through the
 * level and -1 if it goes down
 * @return Number of crossings, or one of the negative results of
 * #distfun_roots()
 */
int
distfun_crossings(const DistFun *f, double level, double lo, double hi,
  double ftol, double *roots, int *dirs, int maxroots)
{
  assert(roots); assert(dirs);
  double r[DISTFUN_MAXROOTS];
  int n = distfun_roots(f, level, lo, hi, ftol, r, DISTFUN_MAXROOTS);
  if (n <= 0)
    return n;
  int count = 0;
  /* Sign of the last gap that has a sign, and the first root after it */
  int before = sign_tol(distfun_eval(f, 0.5 * (lo + r[0])) - level, ftol);
  int first = 0;
  for (int i = 0; i < n; i++)
  {
    double next = (i + 1 < n) ? r[i + 1] : hi;
    int after = sign_tol(distfun_eval(f, 0.5 * (r[i] + next)) - level, ftol);
    if (after == 0)
      continue;
    if (before != 0 && after != before)
    {
      if (count >= maxroots)
        return DISTFUN_OVERFLOW;
      roots[count] = r[first];
      dirs[count] = after;
      count++;
    }
    before = after;
    first = i + 1;
  }
  return count;
}

/**
 * @brief Return in @p root the first crossing of @p level by @p f in
 * [@p lo, @p hi] in the direction @p dir, see #distfun_crossings()
 * @param[in] dir 1 for a crossing up and -1 for a crossing down
 * @return 1 if there is such a crossing, 0 if there is none, or one of the
 * negative results of #distfun_roots()
 */
int
distfun_first_crossing(const DistFun *f, double level, double lo, double hi,
  double ftol, int dir, double *root)
{
  assert(root);
  double roots[DISTFUN_MAXROOTS];
  int dirs[DISTFUN_MAXROOTS];
  int n = distfun_crossings(f, level, lo, hi, ftol, roots, dirs,
    DISTFUN_MAXROOTS);
  if (n < 0)
    return n;
  for (int i = 0; i < n; i++)
    if (dirs[i] == dir)
    {
      *root = roots[i];
      return 1;
    }
  return 0;
}

/*****************************************************************************/
