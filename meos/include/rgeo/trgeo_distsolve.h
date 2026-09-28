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
 * @details See meos/src/rgeo/trgeo_distsolve.c.  This header includes no
 * PostgreSQL, PostGIS or MEOS header, so that the solver builds on its own,
 * without libmeos, for its test program.
 */

#ifndef __TRGEO_DISTSOLVE_H__
#define __TRGEO_DISTSOLVE_H__

/* C */
#include <stdbool.h>

/*****************************************************************************
 * The function family
 *
 * A function of the family is
 *
 *   F(t) = SUM_k [ (a1_k*t + a0_k) * cos(w_k*t)
 *                + (b1_k*t + b0_k) * sin(w_k*t) ]
 *        + c2*t^2 + c1*t + c0
 *
 * Each term has phase zero and a frequency that is not negative.  In one
 * temporal segment, the frequencies are wA, wB and wA - wB, where wA and wB
 * are the rotations of the two bodies.  Thus a function has at most three
 * terms.  If the bodies do not rotate, the function is a quadratic.
 *****************************************************************************/

/** Maximum number of terms of a function */
#define DISTFUN_MAXTERMS   3

/** Size of a root array for one call.  In tests, a function of the family
 * had at most four roots in [0, 1].  A touch of the level gives a group of
 * roots, of which the solver keeps two.  A function with more roots than the
 * array gives #DISTFUN_OVERFLOW, not a wrong answer. */
#define DISTFUN_MAXROOTS   8

/**
 * @brief One term of a #DistFun
 */
typedef struct
{
  double w;            /**< Frequency, not negative */
  double a1, a0;       /**< (a1*t + a0) * cos(w*t) */
  double b1, b0;       /**< (b1*t + b0) * sin(w*t) */
} DistFunTerm;

/**
 * @brief A function of the family
 */
typedef struct
{
  DistFunTerm term[DISTFUN_MAXTERMS];
  int nterm;           /**< Number of terms */
  double c2, c1, c0;   /**< Polynomial part */
} DistFun;

/* Negative results of the root functions.  #DISTFUN_ZERO is not an error:
 * it happens, for example, for two edges that stay parallel because the two
 * bodies rotate at the same rate, and the caller must handle it. */
#define DISTFUN_ZERO      (-1)  /**< The function is zero on the interval */
#define DISTFUN_BUDGET    (-2)  /**< Too many cells */
#define DISTFUN_OVERFLOW  (-3)  /**< Too many roots for the array */
#define DISTFUN_NONFINITE (-4)  /**< A coefficient is not finite */

/*****************************************************************************/

extern void distfun_init(DistFun *f);
extern bool distfun_add(DistFun *f, double w, double g, double A1, double A0,
  double B1, double B0);
extern void distfun_add_poly(DistFun *f, double c2, double c1, double c0);

extern double distfun_eval(const DistFun *f, double t);
extern void distfun_deriv(const DistFun *f, DistFun *df);
extern double distfun_absmax(const DistFun *f, double lo, double hi);
extern bool distfun_is_zero(const DistFun *f, double ftol);
extern int distfun_sign_after(const DistFun *f, double level, double t,
  double ftol);

extern int distfun_roots(const DistFun *f, double level, double lo, double hi,
  double ftol, double *roots, int maxroots);
extern int distfun_crossings(const DistFun *f, double level, double lo,
  double hi, double ftol, double *roots, int *dirs, int maxroots);
extern int distfun_first_crossing(const DistFun *f, double level, double lo,
  double hi, double ftol, int dir, double *root);

/*****************************************************************************/

#endif /* __TRGEO_DISTSOLVE_H__ */
