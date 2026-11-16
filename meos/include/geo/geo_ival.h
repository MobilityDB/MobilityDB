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
 * @brief Intervals of doubles that hold the exact value of a computed
 * quantity
 * @details The arrangement of curves (geo_arrange.c) and the walk of the
 * distance of rigid bodies (trgeo_distance.c) read them.
 */

#ifndef __GEO_IVAL_H__
#define __GEO_IVAL_H__

/* C */
#include <math.h>
/* PostgreSQL */
#include "postgres.h"

/*
 * A value known to lie in [lo, hi]. Each operation rounds its bounds outward
 * by one step, which covers the rounding to nearest of the operation itself,
 * so the interval of a computed quantity holds its exact value: a comparison
 * the intervals decide is the exact comparison.
 */
typedef struct
{
  double lo;
  double hi;
} Ival;

/* fmin and fmax inline, as #float8_min and #float8_max are, but keeping their
 * rule for NaN, the operand that is not NaN where one is, rather than the
 * order putting NaN after every number; either of two zeros, which no
 * comparison tells apart */
static inline double
ival_fmin(double a, double b)
{
  return (a < b || isnan(b)) ? a : b;
}

static inline double
ival_fmax(double a, double b)
{
  return (a > b || isnan(b)) ? a : b;
}

static inline Ival
ival(double x)
{
  Ival r = {x, x};
  return r;
}

/* Each bound moved outward by at least one step: |x| 2^-52 is no less than
 * the unit of the last place of x and is exact, 2^-1074 covers the zeros and
 * the subnormals, and the sum or difference rounds to nearest, which never
 * brings a value back past the double next to x. An infinity stays one */
static inline Ival
ival_out(double lo, double hi)
{
  Ival r = {lo - (fabs(lo) * 0x1p-52 + 0x1p-1074),
    hi + (fabs(hi) * 0x1p-52 + 0x1p-1074)};
  return r;
}

static inline Ival
ival_add(Ival a, Ival b)
{
  return ival_out(a.lo + b.lo, a.hi + b.hi);
}

static inline Ival
ival_sub(Ival a, Ival b)
{
  return ival_out(a.lo - b.hi, a.hi - b.lo);
}

/* The interval of four products or quotients: their extremes, rounded
 * outward by #ival_out, and the whole line where one is undefined, an
 * infinity times zero */
static inline Ival
ival_extremes(const double p[4])
{
  if (isnan(p[0]) || isnan(p[1]) || isnan(p[2]) || isnan(p[3]))
  {
    Ival r = {-INFINITY, INFINITY};
    return r;
  }
  double lo = p[0], hi = p[0];
  for (int i = 1; i < 4; i++)
  {
    lo = (p[i] < lo) ? p[i] : lo;
    hi = (p[i] > hi) ? p[i] : hi;
  }
  return ival_out(lo, hi);
}

/* The product: where the sign of either interval is known, its extremes are
 * two of the four products of the bounds, which the signs name; where both
 * hold zero within them, all four are read (#ival_extremes) */
static pg_attribute_always_inline Ival
ival_mul(Ival a, Ival b)
{
  double lo, hi;
  if (a.lo >= 0.0 && b.lo >= 0.0)
  {
    lo = a.lo * b.lo; hi = a.hi * b.hi;
  }
  else if (a.lo >= 0.0 && b.hi <= 0.0)
  {
    lo = a.hi * b.lo; hi = a.lo * b.hi;
  }
  else if (a.lo >= 0.0)
  {
    lo = a.hi * b.lo; hi = a.hi * b.hi;
  }
  else if (a.hi <= 0.0 && b.lo >= 0.0)
  {
    lo = a.lo * b.hi; hi = a.hi * b.lo;
  }
  else if (a.hi <= 0.0 && b.hi <= 0.0)
  {
    lo = a.hi * b.hi; hi = a.lo * b.lo;
  }
  else if (a.hi <= 0.0)
  {
    lo = a.lo * b.hi; hi = a.lo * b.lo;
  }
  else if (b.lo >= 0.0)
  {
    lo = a.lo * b.hi; hi = a.hi * b.hi;
  }
  else if (b.hi <= 0.0)
  {
    lo = a.hi * b.lo; hi = a.lo * b.lo;
  }
  else
  {
    double p[4] = {a.lo * b.lo, a.lo * b.hi, a.hi * b.lo, a.hi * b.hi};
    return ival_extremes(p);
  }
  if (isnan(lo) || isnan(hi))
  {
    Ival r = {-INFINITY, INFINITY};
    return r;
  }
  return ival_out(lo, hi);
}

/* A divisor holding zero bounds nothing */
static pg_attribute_always_inline Ival
ival_div(Ival a, Ival b)
{
  if (b.lo <= 0.0 && b.hi >= 0.0)
  {
    Ival r = {-INFINITY, INFINITY};
    return r;
  }
  /* The two quotients bounding it, as the signs choose them (#ival_mul) */
  double lo, hi;
  if (b.lo > 0.0)
  {
    lo = a.lo / ((a.lo >= 0.0) ? b.hi : b.lo);
    hi = a.hi / ((a.hi <= 0.0) ? b.hi : b.lo);
  }
  else
  {
    lo = a.hi / ((a.hi <= 0.0) ? b.lo : b.hi);
    hi = a.lo / ((a.lo >= 0.0) ? b.lo : b.hi);
  }
  if (isnan(lo) || isnan(hi))
  {
    Ival r = {-INFINITY, INFINITY};
    return r;
  }
  return ival_out(lo, hi);
}

/* The square root of the non-negative part of an interval */
static inline Ival
ival_sqrt(Ival a)
{
  return ival_out(sqrt(ival_fmax(a.lo, 0.0)), sqrt(ival_fmax(a.hi, 0.0)));
}

static inline Ival
ival_neg(Ival a)
{
  Ival r = {- a.hi, - a.lo};
  return r;
}

/* The sign an interval decides, or 2 where it holds zero and another sign */
static inline int
ival_sign(Ival a)
{
  if (a.lo > 0.0)
    return 1;
  if (a.hi < 0.0)
    return -1;
  if (a.lo == 0.0 && a.hi == 0.0)
    return 0;
  return 2;
}

static pg_attribute_always_inline Ival
ival_cross(Ival ax, Ival ay, Ival bx, Ival by)
{
  return ival_sub(ival_mul(ax, by), ival_mul(ay, bx));
}

static pg_attribute_always_inline Ival
ival_dot(Ival ax, Ival ay, Ival bx, Ival by)
{
  return ival_add(ival_mul(ax, bx), ival_mul(ay, by));
}

/* The union of two intervals */
static inline Ival
ival_hull(Ival a, Ival b)
{
  Ival r = {ival_fmin(a.lo, b.lo), ival_fmax(a.hi, b.hi)};
  return r;
}

#endif /* __GEO_IVAL_H__ */
