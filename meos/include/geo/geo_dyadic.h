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
 * @brief The exact arithmetic of the geometry engine (geo_dyadic.c)
 * @details The macros and types of the representations, the functions the
 * algorithms call, and the operations of the first tiers, static inline so
 * that every file calling them expands them as geo_funcs.c does
 */

#ifndef __GEO_DYADIC_H__
#define __GEO_DYADIC_H__

/* C */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
/* PostgreSQL */
#include <postgres.h>
#include <meos_error.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/temporal.h"
#include <port/pg_bitutils.h>

/**
 * @brief Split the difference of two doubles into its rounded value and the
 * error of that rounding, which together are the difference exactly
 * @details The difference twin of #two_sum, defined here so that every file
 * including this header expands it
 */
static inline void
two_diff(double a, double b, double *x, double *y)
{
  *x = a - b;
  double bv = a - *x;
  double av = *x + bv;
  *y = (a - av) + (bv - b);
}

extern int cross_product_sign_exact(double ax, double ay, double bx, double by,
  double cx, double cy, double dx, double dy);

extern int cross_product_sign_offset(double ax, double ay, double bx,
  double by, double cx, double cy, double dx, double dy, double oab,
  double ocd);

extern int point_segment_offset_sign(const double *p, const double *a,
  const double *b, int ndims, double r);

extern int segment_interior_offset_sign(const double *a, const double *b,
  const double *c, const double *d, double r);

extern int vector_triple_sign(const double *a0, const double *a1,
  const double *b0, const double *b1, const double *c0, const double *c1);

extern int point_segment_dot_sign_inputs(const double *p, const double *s,
  const double *a, const double *b, int ndims);

extern int plane_cross_dot_sign(const double *a, const double *b,
  const double *c, const double *x0, const double *x1, const double *y0,
  const double *y1);

extern int plane_point_offset_sign(const double *a, const double *b,
  const double *c, const double *p, double d);

/* A sum of up to EXACT_SUM_MAXTERMS doubles held exactly as an expansion,
 * with the double nearest to it and a bound on the error of that double */
#define EXACT_SUM_MAXTERMS 4

typedef struct
{
  double e[EXACT_SUM_MAXTERMS];  /**< Components, increasing in magnitude */
  int n;                         /**< Number of components */
  double approx;                 /**< Double nearest to the sum */
  double err;                    /**< Bound on the error of approx */
} ExactSum;

/* A term of a polynomial in exact sums: a coefficient, exactly representable,
 * times the product of up to POLY_TERM_MAXDEG exact sums given by index */
#define POLY_TERM_MAXDEG 6

typedef struct
{
  double coef;                   /**< Coefficient */
  int deg;                       /**< Number of factors */
  int sum[POLY_TERM_MAXDEG];     /**< Indices of the exact sums multiplied */
} PolyTerm;

extern void exact_sum_set(ExactSum *sum, const double *terms, int nterms);

extern int triple_product_sign_exact(const POINT3D *p, const POINT3D *q,
  const POINT3D *r);

extern int incircle_sign(double ax, double ay, double bx, double by,
  double cx, double cy, double dx, double dy);

/**
 * @brief Return the sign of the cross product of the vectors B - A and D - C
 * @details The operands are coordinates of input vertices, and every double is
 * an exact rational, so the sign has one answer. A filter gives it where the
 * double evaluation carries it: Shewchuk's bound on the rounding of a
 * difference of two products of rounded differences, computed from the
 * operands. The bound is relative, so it holds where the products lie clear
 * of the subnormal range, whose rounding is absolute; a product of two
 * nonzero differences can also underflow to zero. Two products each with a
 * zero difference need no more: a rounded difference is zero only where the
 * two coordinates are equal. Where the filter cannot tell otherwise,
 * #cross_product_sign_exact decides the same cross product exactly, for any
 * finite coordinates. With C = A the sign is the side of the line AB that D
 * lies on
 * @return 1 or -1 for the two turns, 0 exactly where the vectors are parallel
 */
static inline int
cross_product_sign(double ax, double ay, double bx, double by, double cx,
  double cy, double dx, double dy)
{
  double l1 = bx - ax, l2 = dy - cy, r1 = by - ay, r2 = dx - cx;
  double left = l1 * l2, right = r1 * r2;
  double mag = fabs(left) + fabs(right);
  double bound = (3.0 + 16.0 * DBL_EPSILON) * DBL_EPSILON * mag;
  if (mag >= 0x1p-969)
  {
    double det = left - right;
    if (det > bound)
      return 1;
    if (det < - bound)
      return -1;
  }
  if ((l1 == 0.0 || l2 == 0.0) && (r1 == 0.0 || r2 == 0.0))
    return 0;
  return cross_product_sign_exact(ax, ay, bx, by, cx, cy, dx, dy);
}

/**
 * @brief Return the sign of the dot product of two vectors
 * @details The coordinates are exact rationals, so the sign has one answer,
 * that of `(p - 0) . (q - 0)`, which #point_segment_dot_sign_inputs gives
 * exactly for any finite doubles. With `p` the normal of a plane the sign is
 * the side of that plane `q` lies on
 * @return 1 or -1, 0 exactly where the vectors are orthogonal
 */
static inline int
dot_product_sign(const POINT3D *p, const POINT3D *q)
{
  const double origin[3] = {0.0, 0.0, 0.0};
  const double pc[3] = {p->x, p->y, p->z}, qc[3] = {q->x, q->y, q->z};
  return point_segment_dot_sign_inputs(pc, origin, origin, qc, 3);
}

/**
 * @brief Return the sign of the triple product `<p, q x r>` of three vectors
 * @details The coordinates are exact rationals, so the sign has one answer,
 * that of the determinant `(p - 0) . ((q - 0) x (r - 0))`, which
 * #vector_triple_sign gives exactly for any finite doubles. With `p` and `q`
 * two points of a unit sphere the sign is the side of the great circle
 * through them, taken from `p` to `q`, that `r` lies on
 * @return 1 where `r` lies on the side of the plane of `p` and `q` that their
 * cross product points to, -1 on the other, 0 exactly where the three vectors
 * are coplanar
 */
static inline int
triple_product_sign(const POINT3D *p, const POINT3D *q, const POINT3D *r)
{
  const double origin[3] = {0.0, 0.0, 0.0};
  const double pc[3] = {p->x, p->y, p->z}, qc[3] = {q->x, q->y, q->z},
    rc[3] = {r->x, r->y, r->z};
  return vector_triple_sign(origin, pc, origin, qc, origin, rc);
}

/**
 * @brief A dyadic rational `sign * magnitude * 2^exp`, the magnitude held in
 * as many 64-bit limbs as it takes, the lowest first, with exact sums,
 * differences and products of any two
 */
typedef struct
{
  int sign;         /**< -1, 0 or 1 */
  int len;          /**< Number of limbs of the magnitude, 0 for the value 0 */
  int exp;          /**< Weight of the lowest bit of the magnitude */
  uint64 limb[FLEXIBLE_ARRAY_MEMBER];
} DyadicWide;

/** Number of 64-bit words a #DyadicWideArena holds before it takes a block */
#define DYADIC_WIDE_ARENA_WORDS 1024

/**
 * @brief The memory of the #DyadicWide values of one exact evaluation: taken
 * in order from a buffer of its own, then from blocks taken as it is used up,
 * and released in one step
 */
typedef struct
{
  uint64 buf[DYADIC_WIDE_ARENA_WORDS];
  size_t used;      /**< Words taken of the current block */
  size_t cap;       /**< Words of the current block */
  uint64 **blocks; /**< The blocks after @p buf */
  size_t *caps;     /**< The words of each block */
  int nblocks;      /**< Number of blocks in use, the last the current one */
  int nkept;        /**< Number of blocks held, those past @p nblocks free */
  int capblocks;
} DyadicWideArena;

/** @brief A point of a #DyadicWideArena that a release returns it to */
typedef struct
{
  int nblocks;
  size_t used;
} DyadicWideMark;

extern void dyadic_wide_arena_init(DyadicWideArena *ar);

extern void dyadic_wide_arena_free(DyadicWideArena *ar);

extern DyadicWideMark dyadic_wide_arena_mark(const DyadicWideArena *ar);

extern void dyadic_wide_arena_release(DyadicWideArena *ar, DyadicWideMark m);

extern DyadicWide *dyadic_wide_of_double(DyadicWideArena *ar, double x);

extern DyadicWide *dyadic_wide_of_int64(DyadicWideArena *ar, int64 v);

extern DyadicWide *dyadic_wide_dup(const DyadicWide *a);

extern DyadicWide *dyadic_wide_signed(DyadicWideArena *ar, const DyadicWide *a,
  int sign);

extern DyadicWide *dyadic_wide_add_signed(DyadicWideArena *ar,
  const DyadicWide *a, int sa, const DyadicWide *b, int sb);

extern DyadicWide *dyadic_wide_mul(DyadicWideArena *ar, const DyadicWide *a,
  const DyadicWide *b);

extern int dyadic_wide_cmp(const DyadicWide *a, const DyadicWide *b);

extern int dyadic_wide_to_fixed(const DyadicWide *a, int scale, bool up,
  uint64 *mag, int nlimbs);

extern int dyadic_wide_cmp_fixed(DyadicWideArena *ar, const DyadicWide *mid,
  int sign, const uint64 *mag, int nlimbs, int exp);

extern void wide_cross3(DyadicWideArena *ar, DyadicWide *const *u,
  DyadicWide *const *v, DyadicWide **r);

extern DyadicWide *wide_dot3(DyadicWideArena *ar, DyadicWide *const *u,
  DyadicWide *const *v);

/**
 * @brief Return the sum of two values, exactly
 */
static inline DyadicWide *
dyadic_wide_add(DyadicWideArena *ar, const DyadicWide *a, const DyadicWide *b)
{
  return dyadic_wide_add_signed(ar, a, 1, b, 1);
}

/**
 * @brief Return the difference of two values, exactly
 */
static inline DyadicWide *
dyadic_wide_sub(DyadicWideArena *ar, const DyadicWide *a, const DyadicWide *b)
{
  return dyadic_wide_add_signed(ar, a, 1, b, -1);
}

/**
 * @brief Return the negation of a value
 */
static inline DyadicWide *
dyadic_wide_neg(DyadicWideArena *ar, const DyadicWide *a)
{
  return dyadic_wide_signed(ar, a, -1);
}

/**
 * @brief Return the sign of a value, -1, 0 or 1
 */
static inline int
wide_sign(const DyadicWide *a)
{
  return a->sign;
}

extern int wide_limbs_divmod(const uint64 *u, int nu, const uint64 *v,
  int nv, uint64 *q, uint64 *r, int *nr);

extern int wide_limbs_isqrt(const uint64 *n, int nn, uint64 *x,
  bool *exact);

/* The sign of the value a search reads less @p mid, the values it forms taken
 * from @p ar, which releases them once it answers */
typedef int (*NearestSide)(DyadicWideArena *ar, const DyadicWide *mid,
  const void *arg);

extern double nearest_double_exact(double c, NearestSide side,
  const void *arg);

extern bool ddb_vector_lon_nearest(const int64 *v, double *result);

extern bool ddb_vector_lat_nearest(const int64 *v, double *result);

extern bool ddb_mercator_lat_nearest(double r, double *result);

/**
 * @brief A value held as the unevaluated sum of two doubles, with a bound on
 * its distance to the exact value it stands for, the filter of the 3D interior distance
 * @details The sum is formed by the error-free transforms #two_diff,
 * #two_product and #two_sum, so it carries about 104 bits on every platform
 * and under every emulator that rounds a double and a fused multiply-add
 * correctly
 */
typedef struct
{
  double hi;  /**< The value, rounded */
  double lo;  /**< What the rounding leaves out */
  double e;   /**< A bound on the error of hi + lo */
} DDBound;

/* The relative rounding of one operation on two-double values, with a wide
 * margin, and an absolute term for a product that underflows */
#define DDB_EPS 0x1p-100

#define DDB_TINY 0x1p-1060

/* The least magnitude of a product of two doubles that #two_product splits
 * exactly: below it the part that rounding leaves out may itself round */
#define DDB_EXACT_PRODUCT 0x1p-969

/**
 * @brief The side of a value against the upper midpoint of a double: the sign
 * of the value less the midpoint from the double of bit pattern @p bits to the
 * next one, or 2 where the side cannot tell
 */
typedef int (*NearestBitsSide)(uint64 bits, void *arg);

/* The bit pattern of infinity, above that of every double */
#define NEAREST_INF_BITS UINT64CONST(0x7FF0000000000000)

#define NEAREST_ASK(b, res) \
  do { \
    if (budget > 0 && asked++ >= budget) \
      return false; \
    int s_ = side((b), arg); \
    if (s_ == 2) \
      (res) = 2; \
    else \
      (res) = nearest_bits_beyond_sign(s_, (b)) ? 1 : 0; \
  } while (0)

/* The number of sides a search of a filter on doubles asks before it leaves
 * the answer to the exact search behind it */
#define NEAREST_FILTER_BUDGET 8

/* Number of words of the head of a #DyadicWide, before its limbs */
#define DYADIC_WIDE_HEAD ((offsetof(DyadicWide, limb) + sizeof(uint64) - 1) / \
  sizeof(uint64))

/** Number of words of scratch the division and the square root take on
 * the stack, more being allocated */
#define WIDE_LIMBS_STACK 256

/* A side read exactly, the arena its values are taken from, and whether the
 * value searched is its negation */
typedef struct
{
  DyadicWideArena *ar;
  NearestSide side;
  const void *arg;
  bool neg;
} NearestExactSide;

/* A root `sqrt(n / d)`, n > 0 and d > 0 */
typedef struct
{
  const DyadicWide *n, *d;
} NearestRoot;

/* A quotient `n / d`, d > 0 */
typedef struct
{
  const DyadicWide *n, *d;
} NearestQuotient;

/* A value `c + b / sqrt(s)`, s > 0 */
typedef struct
{
  const DyadicWide *c, *b, *s;
} NearestOffsetRoot;

/* The root of `n / d` that #ddb_nearest_root places, and the candidate whose
 * residual it formed last */
typedef struct
{
  DDBound n, d;
  double dv;        /**< `d` in one double */
  uint64 cbits;     /**< Bit pattern of the candidate, infinity's for none */
  double c, gd;     /**< The candidate and the gap to its lower neighbour */
  double rv, rerr;  /**< Its residual `n - c^2 d` and the bound of its error */
} DDBRootSide;

/* 180 / pi as the sum of two doubles, within 2^-102.2 of it (mpmath at 4000
 * bits), and the bound kept for it */
#define DDB_DEG_HI 0x1.ca5dc1a63c1f8p+5

#define DDB_DEG_LO -0x1.1e7ab456405f9p-49

#define DDB_DEG_ERR 0x1p-100

/* A factor above 1 covering the rounding of a bound formed by a few
 * operations on doubles */
#define DDB_UP (1.0 + 0x1p-40)

/* pi as the sum of two doubles, within 2^-108 of it (mpmath at 4000 bits),
 * and the bound kept for it */
#define DDB_PI_HI 0x1.921fb54442d18p+1

#define DDB_PI_LO 0x1.1a62633145c07p-53

#define DDB_PI_ERR 0x1p-100

/**
 * @brief The square of the distance of a point to a segment as a quotient of
 * polynomials in exact sums, `num / den` (#segment_distance_square)
 */
typedef struct
{
  PolyTerm num[9];       /**< Terms of the numerator */
  int nnum;              /**< Number of terms of the numerator */
  PolyTerm den[3];       /**< Terms of the denominator */
  int nden;              /**< Number of terms of the denominator */
} SegmentSquare;

/**
 * @brief Return true where the nearest points of the lines of two segments
 * lie strictly inside both, the signs read by @p sign on `n . n`,
 * `(e x v) . n` and `(e x u) . n` and their differences
 * @details With `n` not zero the parameters of the nearest points are
 * `s = ((e x v) . n) / (n . n)` along the first segment and
 * `t = ((e x u) . n) / (n . n)` along the second; a parameter at 0 or 1
 * places the nearest point at an end, which the ends answer
 */
#define SEGMENT_INSIDE(snn, ss, ss1, st, st1) \
  ((snn) > 0 && (ss) > 0 && (ss1) < 0 && (st) > 0 && (st1) < 0)

/* A value in doubles and a bound on the error of it */
typedef struct
{
  double v;
  double e;
} ArcsegFilter;

/* The doubles one exact evaluation takes at most without a block of its
 * own: the expansions of a meeting of a segment and an arc of three points */
#define ARCSEG_ARENA_SIZE 4096

/* The memory one exact evaluation takes, released in one step: taken in
 * order from a buffer on the stack, and from blocks of its own once the
 * buffer is used up */
typedef struct
{
  double buf[ARCSEG_ARENA_SIZE];
  int used;
  double **blocks;
  int n;
  int cap;
  bool unsafe;      /**< A product may have left the range where the
                         arithmetic of expansions is exact */
  bool wide;        /**< Values are #DyadicWide rather than expansions */
  bool filter;      /**< Values are doubles carrying a bound on their error,
                         as #ArcsegFilter reads them: one component and
                         the bound in its @p err, so a sign the bound cannot
                         tell marks the arena unsafe */
  DyadicWideArena *warena; /**< The values of a wide arena, NULL otherwise */
} ArcsegArena;

/* An exact value: an expansion whose last component carries its sign, and a
 * bound on how far it may lie from the value, 0 unless a product's error
 * term underflowed; in a wide arena, a #DyadicWide instead */
typedef struct
{
  int n;
  double *e;
  double err;
  DyadicWide *w;     /**< The value in a wide arena, NULL otherwise */
} ArcsegExact;

/* The arithmetic the numbers of a tower are computed in: expansions in
 * @p arena, or #DyadicWide in @p warena where @p wide is set: the numbers of
 * a tower spread over a number of bits that grows with the squarings of its
 * recursion, which no capacity fixed beforehand bounds for any finite
 * coordinates, so each takes the limbs it holds */
typedef struct
{
  ArcsegArena arena;
  bool wide;
  bool filter;         /**< Doubles carrying a bound on their error, a sign
                            the bound cannot tell marking the arena unsafe */
  bool dd;             /**< Pairs of doubles carrying a bound on their error
                            (#DDBound), as @p filter */
  DyadicWideArena warena;
} DyadicCtx;

/* A value of the arithmetic of a #DyadicCtx, the whole union zeroed where it
 * is formed so that a copy reads no byte left unset */
typedef union
{
  ArcsegExact x;
  DyadicWide *w;
  ArcsegFilter f;
  DDBound d;
} DyadicX;

/* A question about meetings, answered in the arithmetic of @p ctx: its
 * answer, or 2 where it is not told there */
typedef int (*DyadicPred)(DyadicCtx *ctx, const void *arg);

/* A polynomial in exact sums, the question #polynomial_sign_pred reads */
typedef struct
{
  const ExactSum *sums;
  const PolyTerm *terms;
  int nterms;
} PolySignArg;

/* The exact sums a polynomial reads on the stack, as #segment_square_cmp
 * forms them; a polynomial reading more takes them from the heap */
#define POLY_SIGN_STACK_SUMS 64

/* The functions of geo_dyadic.c the algorithms and this header call */
extern int
cross_product_expansion(double ax, double ay, double bx, double by,
  double cx, double cy, double dx, double dy, double *buf1, double *buf2,
  const double **result);
extern int
polynomial_sign_pairs(const ExactSum *sums, const PolyTerm *terms,
  int nterms);
extern pg_noinline void
dyadic_wide_arena_grow(DyadicWideArena *ar, size_t words);
extern int
dyadic_wide_product_cmp(DyadicWideArena *ar, const DyadicWide *a,
  const DyadicWide *b, const DyadicWide *c, const DyadicWide *d);
extern double
dyadic_wide_frexp(const DyadicWide *a, int *exp);
extern double
dyadic_wide_approx(const DyadicWide *a);
extern DyadicWide *
square_products_wide(DyadicWideArena *ar, const double *left,
  const double *right, const double *factor, int n);
extern int
square_products_sign(const double *left, const double *right,
  const double *factor, int n);
extern int
point_distance_offset_pair_side_sign(const double *p, const double *q,
  int ndims, double r, double rlo, double c, double g);
extern double
dyadic_wide_nearest_root(DyadicWideArena *ar, const DyadicWide *n,
  const DyadicWide *d);
extern double
dyadic_wide_nearest_quotient(const DyadicWide *n, const DyadicWide *d);
extern double
dyadic_wide_nearest_offset_root(DyadicWideArena *ar, double c,
  const DyadicWide *b, const DyadicWide *s);
extern int
point_segment_side_sign_inputs(const double *p, const double *a,
  const double *b, int ndims, double r, double c, double g);
extern void
ddb_triple3_points(const double *a, const double *b, const double *c,
  const double *d, DDBound *r);
extern bool
ddb_nearest_root(DDBound n, DDBound d, double *result);
extern int
segment_square_cmp(const ExactSum *sums, const SegmentSquare *s1,
  const SegmentSquare *s2);
extern bool
segment_interior_wide_values(DyadicWideArena *ar, const double *a,
  const double *b, const double *c, const double *d, DyadicWide **en,
  DyadicWide **nn);
extern void
arcseg_arena_free(ArcsegArena *arena);
extern void
arcseg_arena_init(ArcsegArena *arena, bool wide);
extern void
arcseg_arena_init_filter(ArcsegArena *arena);
extern ArcsegExact
arcseg_x_diff(ArcsegArena *arena, double a, double b);
extern ArcsegExact
arcseg_x_add(ArcsegArena *arena, ArcsegExact a, ArcsegExact b);
extern ArcsegExact
arcseg_x_scale(ArcsegArena *arena, ArcsegExact a, double k);
extern ArcsegExact
arcseg_x_sub(ArcsegArena *arena, ArcsegExact a, ArcsegExact b);
extern ArcsegExact
arcseg_x_mul(ArcsegArena *arena, ArcsegExact a, ArcsegExact b);
extern int
arcseg_x_sign_checked(ArcsegArena *arena, ArcsegExact a);
extern void
dyadic_ctx_init(DyadicCtx *ctx, bool wide);
extern void
dyadic_ctx_free(DyadicCtx *ctx);
extern int
dx_sign(DyadicCtx *ctx, DyadicX a);
extern ArcsegFilter
dx_filter(const DyadicCtx *ctx, DyadicX a);
extern int
dyadic_pred_run(DyadicPred fn, const void *arg);
extern DyadicWide *
dyadic_wide_sum(DyadicWideArena *ar, const double *t, int k);

static inline void two_product(double a, double b, double *x, double *y);
static inline int grow_expansion(int elen, const double *e, double b,
  double *h);
static inline int double_lowest_bit(double a);
static inline int double_lowest_set_bit(double a);
static inline int double_lowest_set_bit(double a);

/**
 * @brief Return true if the product of two doubles splits exactly into two
 * doubles (#two_product), its lowest set bit not below 2^-1074
 */
static inline bool
two_product_exact(double a, double b)
{
  return a == 0.0 || b == 0.0 ||
    double_lowest_set_bit(a) + double_lowest_set_bit(b) >= -1074;
}

/**
 * @brief Split the product of two doubles into its rounded value and the
 * error of that rounding, which together are the product exactly
 * @note Exact where the product neither overflows nor underflows
 */
static inline void
two_product(double a, double b, double *x, double *y)
{
  *x = a * b;
  *y = fma(a, b, - *x);
}

/**
 * @brief Split the square of a double into its rounded value and the error of
 * that rounding, which together are the square exactly
 * @details The square of #two_product computed with no call: Dekker's product
 * on the halves of 26 bits that splitting by 2^27 + 1 gives, each of their
 * products exact
 * @note Exact where the square neither overflows nor underflows and the
 * value is at most 2^996 in magnitude
 */
static inline void
two_square(double a, double *x, double *y)
{
  double t = 134217729.0 * a;
  double ah = t - (t - a);
  double al = a - ah;
  *x = a * a;
  *y = ((ah * ah - *x) + 2.0 * ah * al) + al * al;
}

/**
 * @brief Split the product of two doubles into its rounded value and the
 * error of that rounding, which together are the product exactly
 * @details The product of #two_product computed with no call, as #two_square
 * computes a square: Dekker's product on the halves of 26 bits that splitting
 * by 2^27 + 1 gives, each of their products exact. A build without a fused
 * multiply-add instruction calls a library function for `fma`
 * @note Exact where the product neither overflows nor underflows and both
 * values are at most 2^995 in magnitude
 */
static inline void
two_product_split(double a, double b, double *x, double *y)
{
  double ta = 134217729.0 * a, tb = 134217729.0 * b;
  double ah = ta - (ta - a), bh = tb - (tb - b);
  double al = a - ah, bl = b - bh;
  *x = a * b;
  *y = ((ah * bh - *x) + ah * bl + al * bh) + al * bl;
}

/**
 * @brief Add a double to an expansion, a sum of doubles that do not overlap,
 * held in increasing order of magnitude, and return its new length
 * @details The sum is exact, and a zero component is dropped, so the last
 * component carries the sign of the whole expansion
 */
static inline int
grow_expansion(int elen, const double *e, double b, double *h)
{
  double q = b;
  int hlen = 0;
  for (int i = 0; i < elen; i++)
  {
    double sum = q + e[i];
    double bv = sum - q;
    double av = sum - bv;
    double err = (q - av) + (e[i] - bv);
    q = sum;
    if (err != 0.0)
      h[hlen++] = err;
  }
  if (q != 0.0 || hlen == 0)
    h[hlen++] = q;
  return hlen;
}

/**
 * @brief Split the sum of two doubles into its rounded value and the error of
 * that rounding, which together are the sum exactly
 */
static inline void
two_sum(double a, double b, double *x, double *y)
{
  *x = a + b;
  double bv = *x - a;
  double av = *x - bv;
  *y = (a - av) + (b - bv);
}

/**
 * @brief Set the three coordinates of `q - p`, exact as two doubles
 */
static pg_attribute_always_inline void
ddb_diff3(const double *q, const double *p, DDBound *r)
{
  for (int c = 0; c < 3; c++)
  {
    two_diff(q[c], p[c], &r[c].hi, &r[c].lo);
    r[c].e = 0.0;
  }
  return;
}

/**
 * @brief Return true if a bounded value is exactly zero: both doubles zero and
 * no error
 */
static pg_attribute_always_inline bool
ddb_is_zero(DDBound a)
{
  return a.hi == 0.0 && a.lo == 0.0 && a.e == 0.0;
}

/**
 * @brief Return true if a bounded value is a double held exactly
 */
static pg_attribute_always_inline bool
ddb_is_double(DDBound a)
{
  return a.lo == 0.0 && a.e == 0.0;
}

/**
 * @brief Return the product of two bounded values, with its bound
 * @details A product with an exact zero is an exact zero, and the product of
 * two doubles held exactly is exact as two doubles (#two_product)
 */
static pg_attribute_always_inline DDBound
ddb_mul(DDBound a, DDBound b)
{
  DDBound r;
  if (ddb_is_zero(a) || ddb_is_zero(b))
  {
    r.hi = r.lo = r.e = 0.0;
    return r;
  }
  if (ddb_is_double(a) && ddb_is_double(b))
  {
    two_product(a.hi, b.hi, &r.hi, &r.lo);
    /* A product that underflows is not split exactly */
    r.e = (fabs(r.hi) < DDB_EXACT_PRODUCT) ? DDB_TINY : 0.0;
    return r;
  }
  double p, pe;
  two_product(a.hi, b.hi, &p, &pe);
  pe += a.hi * b.lo + a.lo * b.hi;
  two_sum(p, pe, &r.hi, &r.lo);
  double ma = fabs(a.hi) + fabs(a.lo), mb = fabs(b.hi) + fabs(b.lo);
  r.e = ma * b.e + mb * a.e + a.e * b.e + fabs(a.lo * b.lo) +
    DDB_EPS * fabs(p) + DDB_TINY;
  return r;
}

/**
 * @brief Return the sum of two bounded values, with its bound
 */
static pg_attribute_always_inline DDBound
ddb_add(DDBound a, DDBound b)
{
  /* A sum with an exact zero is the other term, and the sum of two doubles
   * held exactly is exact as two doubles (#two_sum) */
  if (ddb_is_zero(b))
    return a;
  if (ddb_is_zero(a))
    return b;
  double s, se;
  two_sum(a.hi, b.hi, &s, &se);
  DDBound r;
  if (ddb_is_double(a) && ddb_is_double(b))
  {
    r.hi = s;
    r.lo = se;
    r.e = 0.0;
    return r;
  }
  se += a.lo + b.lo;
  two_sum(s, se, &r.hi, &r.lo);
  r.e = a.e + b.e + DDB_EPS * (fabs(a.hi) + fabs(b.hi)) + DDB_TINY;
  return r;
}

/**
 * @brief Return the difference of two bounded values, with its bound
 */
static pg_attribute_always_inline DDBound
ddb_sub(DDBound a, DDBound b)
{
  b.hi = - b.hi;
  b.lo = - b.lo;
  return ddb_add(a, b);
}

/**
 * @brief Return the sign of a bounded value where the bound decides it: 1 or
 * -1, or 0 where the value may lie on either side of zero
 * @details The bound is itself computed in doubles, hence the margin; a value
 * that is not finite decides nothing
 */
static pg_attribute_always_inline int
ddb_sign(DDBound a)
{
  double bound = a.e * (1.0 + 1e-6) + DDB_TINY;
  double mag = fabs(a.hi) - fabs(a.lo);
  if (! isfinite(a.hi) || ! isfinite(a.lo) || ! isfinite(bound))
    return 0;
  if (mag <= bound)
    return 0;
  return (a.hi > 0.0) ? 1 : -1;
}

/**
 * @brief Return the sign of a polynomial in exact sums where its filters
 * carry it, and 2 where they cannot tell
 * @details The two filters of #polynomial_sign_exact, in doubles and else in
 * pairs of doubles (#polynomial_sign_pairs). The polynomial is
 * evaluated in doubles together with a bound on the error of every operation,
 * each rounding contributing at most DBL_EPSILON times its result; where the
 * value exceeds the bound its sign is the exact one. Neither filter answers a
 * value outside the doubles, so a caller holding an exact route of its own
 * reads it where this answers 0
 * @param[in] sums Exact sums the terms refer to
 * @param[in] terms,nterms Terms of the polynomial and their number
 * @return -1 or 1, or 2 where the filters cannot tell
 */
static inline int
polynomial_sign_filter(const ExactSum *sums, const PolyTerm *terms,
  int nterms)
{
  /* The value in doubles and a bound on its error */
  double val = 0.0, err = 0.0;
  for (int i = 0; i < nterms; i++)
  {
    double v = terms[i].coef, e = 0.0;
    for (int k = 0; k < terms[i].deg; k++)
    {
      const ExactSum *s = &sums[terms[i].sum[k]];
      double nv = v * s->approx;
      e = fabs(v) * s->err + fabs(s->approx) * e + e * s->err +
        DBL_EPSILON * fabs(nv);
      v = nv;
    }
    double nval = val + v;
    err += e + DBL_EPSILON * fabs(nval);
    val = nval;
  }
  /* The bound itself is computed in doubles, hence the margin; the absolute
   * term covers a product that underflows */
  double bound = err * (1.0 + 1e-6) + DBL_MIN;
  if (isfinite(val) && isfinite(bound) && fabs(val) > bound)
    return (val > 0.0) ? 1 : -1;
  return polynomial_sign_pairs(sums, terms, nterms);
}

extern int polynomial_sign_dyadic(const ExactSum *sums, const PolyTerm *terms,
  int nterms);

/**
 * @brief Return the sign of a polynomial in exact sums, computed exactly
 * @details The polynomial is first evaluated in doubles together with a bound
 * on the error of every operation, then in pairs of doubles against its own
 * bound (#polynomial_sign_filter); where neither tells, the sign is read on
 * the expression ladder from its expansions on (#polynomial_sign_dyadic), in
 * expansions and in #DyadicWide where those leave their exact range, so it is
 * exact for any finite sums
 * @param[in] sums Exact sums the terms refer to
 * @param[in] terms,nterms Terms of the polynomial and their number
 * @return -1, 0, or 1
 */
static inline int
polynomial_sign_exact(const ExactSum *sums, const PolyTerm *terms,
  int nterms)
{
  int sign = polynomial_sign_filter(sums, terms, nterms);
  if (sign != 2)
    return sign;
  return polynomial_sign_dyadic(sums, terms, nterms);
}

/**
 * @brief Return the exponent of the lowest bit a finite double can carry: its
 * unit in the last place, which is 2^-1074 for a subnormal
 */
static inline int
double_lowest_bit(double a)
{
  uint64 bits;
  memcpy(&bits, &a, sizeof(bits));
  int biased = (int) ((bits >> 52) & 0x7FF);
  return biased ? biased - 1075 : -1074;
}

/**
 * @brief Return the exponent of the lowest set bit of a nonzero finite
 * double, the largest power of two it is a multiple of
 * @details A product, a sum or a scaling by a power of two is exact where the
 * lowest set bit of the exact result is not below 2^-1074, which the lowest
 * set bits of the operands bound. The unit in the last place
 * (#double_lowest_bit) bounds it too, but up to 52 bits lower: 2 has its unit
 * in the last place at 2^-51, so beside a value of 2^134 the scaling that
 * brings the largest into [1, 2) moves it below a bound that its lowest set
 * bit, 2^1, stays above
 */
static inline int
double_lowest_set_bit(double a)
{
  uint64 bits;
  memcpy(&bits, &a, sizeof(bits));
  uint64 mant = bits & 0xFFFFFFFFFFFFFULL;
  if ((bits >> 52) & 0x7FF)
    mant |= 1ULL << 52;
  return double_lowest_bit(a) + pg_rightmost_one_pos64(mant);
}

/* True where the side @p s puts the value beyond the upper midpoint of the
 * double of bit pattern @p bits, a tie being beyond where the last bit of
 * that double is odd, so a tie goes to the even double */
static inline bool
nearest_bits_beyond_sign(int s, uint64 bits)
{
  return s > 0 || (s == 0 && (bits & 1) != 0);
}

/* The side of a monotone search over keys (#monotone_search): 1 where the
 * answer lies above the key, 0 where it is the key or lies below it, 2 where
 * the side cannot tell */
typedef int (*MonotoneSide)(uint64 key, void *arg);

/**
 * @brief Find the smallest key above @p lo and at most @p hi that @p side
 * does not place below the answer, by halving the bracket, returning false
 * where a side it asks cannot tell or the budget runs out
 * @details The halving of #monotone_search, read alone by a search that has
 * its bracket and no candidate. The key @p lo lies below the answer and
 * @p hi does not; neither is asked
 * @param[in] lo,hi Bracket, @p lo below @p hi
 * @param[in] side,arg The side and its argument
 * @param[in] budget Largest number of sides asked, 0 for no limit
 * @param[in,out] asked Number of sides asked so far
 * @param[out] result The answer where the search answers
 */
static pg_attribute_always_inline bool
monotone_halve(uint64 lo, uint64 hi, MonotoneSide side, void *arg,
  int budget, int *asked, uint64 *result)
{
  while (hi - lo > 1)
  {
    uint64 mid = lo + (hi - lo) / 2;
    if (budget > 0 && (*asked)++ >= budget)
      return false;
    int beyond = side(mid, arg);
    if (beyond == 2)
      return false;
    if (beyond)
      lo = mid;
    else
      hi = mid;
  }
  *result = hi;
  return true;
}

/**
 * @brief Find the smallest key that @p side does not place below the
 * answer, returning false where a side it asks cannot tell or its budget
 * runs out
 * @details The one search of the engine for an answer that a monotone side
 * places among keys: the bit patterns of the doubles (#nearest_double_search)
 * and the microseconds of a period. A gallop from the candidate @p key
 * brackets the answer, its first two steps of one key, so that a candidate a
 * neighbour or two from the answer asks no more sides than a walk, the steps
 * then doubling, and halving the bracket finds it (#monotone_halve); a
 * candidate whose own side
 * cannot tell is read from its lower neighbour, which places the answer
 * below it or decides nothing. The key @p max_key is known not to lie below
 * the answer and is never asked; the key 0 is asked as every other. An exact
 * side never answers 2 and a budget of 0 sets no limit, so the search always
 * answers; a side of a filter on doubles answers 2 where it cannot tell, and
 * its budget bounds the sides it is asked, so that the exact search behind it
 * answers what the filter cannot
 * @param[in] key Candidate, at most @p max_key
 * @param[in] max_key Largest key, not below the answer
 * @param[in] side,arg The side and its argument
 * @param[in] budget Largest number of sides asked, 0 for no limit
 * @param[out] result The answer where the search answers
 */
static pg_attribute_always_inline bool
monotone_search(uint64 key, uint64 max_key, MonotoneSide side, void *arg,
  int budget, uint64 *result)
{
  int asked = 0;
#define MONOTONE_ASK(k, res) \
  do { \
    if (budget > 0 && asked++ >= budget) \
      return false; \
    (res) = side((k), arg); \
  } while (0)
  uint64 lo, hi, step = 1;
  int beyond, moves = 0;
  if (key > max_key)
    key = max_key;
  if (key == max_key)
    beyond = 0;
  else
    MONOTONE_ASK(key, beyond);
  if (beyond == 2)
  {
    /* The candidate cannot tell: below its lower neighbour the answer is at
     * most that neighbour, otherwise nothing is decided */
    if (key == 0)
      return false;
    MONOTONE_ASK(key - 1, beyond);
    if (beyond != 0)
      return false;
    key -= 1;
  }
  if (beyond == 1)
  {
    /* The answer is above: lo is below it, hi is not */
    lo = key;
    while (true)
    {
      hi = (lo < max_key - step) ? lo + step : max_key;
      if (hi == max_key)
        break;
      MONOTONE_ASK(hi, beyond);
      if (beyond == 2)
        return false;
      if (! beyond)
        break;
      lo = hi;
      if (moves++ > 0)
        step *= 2;
    }
  }
  else
  {
    /* The answer is the key or below it: hi is not below it, lo is below it
     * or below 0 */
    hi = key;
    while (true)
    {
      if (hi == 0)
      {
        *result = 0;
        return true;
      }
      lo = (hi > step) ? hi - step : 0;
      MONOTONE_ASK(lo, beyond);
      if (beyond == 2)
        return false;
      if (beyond)
        break;
      if (lo == 0)
      {
        *result = 0;
        return true;
      }
      hi = lo;
      if (moves++ > 0)
        step *= 2;
    }
  }
#undef MONOTONE_ASK
  return monotone_halve(lo, hi, side, arg, budget, &asked, result);
}

/* The side of a search for the nearest double and its argument, read by
 * #nearest_bits_beyond */
typedef struct
{
  NearestBitsSide side;   /**< Sign of the value against a midpoint */
  void *arg;              /**< Argument of the side */
} NearestBeyond;

/**
 * @brief Return 1 where the value lies beyond the upper midpoint of the
 * double of bit pattern @p bits, a tie going to the even double, 0 where it
 * does not and 2 where the side cannot tell: the side #nearest_double_search
 * gives #monotone_search
 * @note @p arg is not const, as #MonotoneSide declares it for the sides that
 * write what they read through it
 */
static pg_attribute_always_inline int
/* cppcheck-suppress constParameterCallback */
nearest_bits_beyond(uint64 bits, void *arg)
{
  const NearestBeyond *n = (const NearestBeyond *) arg;
  int s = n->side(bits, n->arg);
  if (s == 2)
    return 2;
  return nearest_bits_beyond_sign(s, bits) ? 1 : 0;
}

/**
 * @brief Find the bit pattern of the double nearest a nonnegative value that
 * @p side places against midpoints, a tie going to the even double, returning
 * false where a side it asks cannot tell or its budget runs out
 * @details The answer is the smallest bit pattern whose upper midpoint the
 * value does not pass, infinity standing above the largest double, whose
 * upper midpoint is `2^1024 - 2^970`, found by #monotone_search over the bit
 * patterns
 * @param[in] bits Bit pattern of the candidate, at most infinity's
 * @param[in] side,arg The side and its argument
 * @param[in] budget Largest number of sides asked, 0 for no limit
 * @param[out] result The bit pattern of the answer where the search answers
 */
static inline bool
nearest_double_search(uint64 bits, NearestBitsSide side,
  void *arg, int budget, uint64 *result)
{
  NearestBeyond n = {side, arg};
  return monotone_search(bits, NEAREST_INF_BITS, &nearest_bits_beyond, &n,
    budget, result);
}

/* A value 0 of an arena with room for @p n limbs */
static inline DyadicWide *
dyadic_wide_alloc(DyadicWideArena *ar, int n)
{
  size_t words = DYADIC_WIDE_HEAD + (size_t) Max(n, 1);
  if (unlikely(ar->used + words > ar->cap))
    dyadic_wide_arena_grow(ar, words);
  uint64 *base = ar->nblocks ? ar->blocks[ar->nblocks - 1] : ar->buf;
  DyadicWide *r = (DyadicWide *) (base + ar->used);
  ar->used += words;
  r->sign = 0;
  r->len = 0;
  r->exp = 0;
  return r;
}

/* The value 0 */
static inline DyadicWide *
dyadic_wide_zero(DyadicWideArena *ar)
{
  return dyadic_wide_alloc(ar, 1);
}

/* The sum of a value and a finite double times @p sign, -1 or 1, exactly */
static inline DyadicWide *
dyadic_wide_add_double(DyadicWideArena *ar, const DyadicWide *a, double x,
  int sign)
{
  return dyadic_wide_add_signed(ar, a, 1, dyadic_wide_of_double(ar, x), sign);
}

/**
 * @brief Return in an expansion a sum of products of doubles, each times 1,
 * -1, 2 or -2, and its length
 * @details Each product is its rounded value plus its error (#two_product),
 * doubling and negating a pair of doubles is exact, and the terms are added
 * into one expansion (#grow_expansion), held in increasing order of
 * magnitude, whose last component carries the sign of the whole
 * @note Exact where #square_products_exact holds
 */
static pg_attribute_always_inline int
square_products_expansion(const double *left, const double *right,
  const double *factor, int n, double *buf1, double *buf2,
  const double **result)
{
  double *cur = buf1, *nxt = buf2;
  int len = 0;
  for (int j = 0; j < n; j++)
  {
    if (left[j] == 0.0 || right[j] == 0.0)
      continue;
    double x, y;
    two_product(left[j], right[j], &x, &y);
    /* A multiplier of 1, -1, 2 or -2 scales both components exactly */
    x *= factor[j];
    y *= factor[j];
    if (y != 0.0)
    {
      len = grow_expansion(len, cur, y, nxt);
      double *swap = cur; cur = nxt; nxt = swap;
    }
    len = grow_expansion(len, cur, x, nxt);
    double *swap = cur; cur = nxt; nxt = swap;
  }
  *result = cur;
  return len;
}

/**
 * @brief Return the sign of the squared distance between two points less the
 * square of `r + c + g/2`, computed exactly for any finite doubles, as
 * #point_distance_offset_pair_side_sign reads it for a radius of one double
 */
static inline int
point_distance_offset_side_sign(const double *p, const double *q, int ndims,
  double r, double c, double g)
{
  return point_distance_offset_pair_side_sign(p, q, ndims, r, 0.0, c, g);
}

/**
 * @brief Return the dot product of two vectors of three bounded values
 * @details Where every component is a double held exactly, each product is
 * exact as two doubles (#two_product), as in #ddb_cross3, and only the sums
 * round
 */
static pg_attribute_always_inline DDBound
ddb_dot3(const DDBound *a, const DDBound *b)
{
  if (a[0].lo == 0.0 && a[0].e == 0.0 && a[1].lo == 0.0 && a[1].e == 0.0 &&
      a[2].lo == 0.0 && a[2].e == 0.0 && b[0].lo == 0.0 && b[0].e == 0.0 &&
      b[1].lo == 0.0 && b[1].e == 0.0 && b[2].lo == 0.0 && b[2].e == 0.0)
  {
    DDBound p[3];
    for (int c = 0; c < 3; c++)
    {
      two_product(a[c].hi, b[c].hi, &p[c].hi, &p[c].lo);
      p[c].e = (a[c].hi != 0.0 && b[c].hi != 0.0 &&
        fabs(p[c].hi) < DDB_EXACT_PRODUCT) ? DDB_TINY : 0.0;
    }
    return ddb_add(ddb_add(p[0], p[1]), p[2]);
  }
  return ddb_add(ddb_add(ddb_mul(a[0], b[0]), ddb_mul(a[1], b[1])),
    ddb_mul(a[2], b[2]));
}

/**
 * @brief Set the cross product of two vectors of three bounded values
 * @details Where every component is a double held exactly, as the difference
 * of two coordinates is wherever it does not round, each product is exact as
 * two doubles (#two_product) and only the sum of the two rounds, by the
 * bound #ddb_add gives it
 */
static pg_attribute_always_inline void
ddb_cross3(const DDBound *a, const DDBound *b, DDBound *r)
{
  bool exact = true;
  for (int c = 0; c < 3; c++)
    exact = exact && a[c].lo == 0.0 && a[c].e == 0.0 && b[c].lo == 0.0 &&
      b[c].e == 0.0;
  if (exact)
  {
    for (int c = 0; c < 3; c++)
    {
      int i = (c + 1) % 3, j = (c + 2) % 3;
      double p1, e1, p2, e2, s, se;
      two_product(a[i].hi, b[j].hi, &p1, &e1);
      two_product(a[j].hi, b[i].hi, &p2, &e2);
      /* Two exact products are equal where their pairs are, the pair of a
       * product being unique, and their difference is then an exact zero;
       * a product that underflows or overflows is not split exactly, so two
       * of them say nothing by being equal: two products past the largest
       * double, of one sign, are both an infinity whatever their values */
      bool under = (p1 != 0.0 && fabs(p1) < DDB_EXACT_PRODUCT) ||
        (p2 != 0.0 && fabs(p2) < DDB_EXACT_PRODUCT) ||
        (p1 == 0.0 && a[i].hi != 0.0 && b[j].hi != 0.0) ||
        (p2 == 0.0 && a[j].hi != 0.0 && b[i].hi != 0.0);
      if (! under && isfinite(p1) && p1 == p2 && e1 == e2)
      {
        r[c].hi = r[c].lo = r[c].e = 0.0;
        continue;
      }
      two_diff(p1, p2, &s, &se);
      se += e1 - e2;
      two_sum(s, se, &r[c].hi, &r[c].lo);
      r[c].e = DDB_EPS * (fabs(p1) + fabs(p2)) + DDB_TINY;
    }
    return;
  }
  for (int c = 0; c < 3; c++)
  {
    int i = (c + 1) % 3, j = (c + 2) % 3;
    r[c] = ddb_sub(ddb_mul(a[i], b[j]), ddb_mul(a[j], b[i]));
  }
  return;
}

static inline ArcsegFilter
arcseg_f_diff(double a, double b)
{
  ArcsegFilter r;
  r.v = a - b;
  r.e = DBL_EPSILON * fabs(r.v);
  return r;
}

static inline ArcsegFilter
arcseg_f_add(ArcsegFilter a, ArcsegFilter b)
{
  ArcsegFilter r;
  r.v = a.v + b.v;
  r.e = a.e + b.e + DBL_EPSILON * fabs(r.v);
  return r;
}

static inline ArcsegFilter
arcseg_f_sub(ArcsegFilter a, ArcsegFilter b)
{
  ArcsegFilter r;
  r.v = a.v - b.v;
  r.e = a.e + b.e + DBL_EPSILON * fabs(r.v);
  return r;
}

static inline ArcsegFilter
arcseg_f_mul(ArcsegFilter a, ArcsegFilter b)
{
  ArcsegFilter r;
  r.v = a.v * b.v;
  r.e = fabs(a.v) * b.e + fabs(b.v) * a.e + a.e * b.e +
    DBL_EPSILON * fabs(r.v);
  /* A product below the range of normal doubles, and the terms of its
   * bound, lose less than DBL_MIN each to their rounding, which the bound
   * then carries, unless a factor is exactly zero: an underflow to zero is
   * no exact zero. A normal product carries its rounding already */
  if (fabs(r.v) < DBL_MIN &&
      ! ((a.v == 0.0 && a.e == 0.0) || (b.v == 0.0 && b.e == 0.0)))
    r.e += 4.0 * DBL_MIN;
  return r;
}

/* Multiplied by a power of two, which scales the error without a rounding */
static inline ArcsegFilter
arcseg_f_scale(ArcsegFilter a, double k)
{
  ArcsegFilter r;
  r.v = a.v * k;
  r.e = a.e * fabs(k);
  return r;
}

/* The sign a filtered value carries, or 2 where its bound cannot tell. The
 * bound is computed in doubles, hence the margin, as #polynomial_sign_exact
 * reads its own */
static inline int
arcseg_f_sign(ArcsegFilter a)
{
  double bound = a.e * (1.0 + 1e-6) + DBL_MIN;
  if (! isfinite(a.v) || ! isfinite(bound) || fabs(a.v) <= bound)
    return 2;
  return (a.v > 0.0) ? 1 : -1;
}

/* Whether the answers of the arithmetic of the expansions may not be exact */
static inline bool
dyadic_ctx_unsafe(const DyadicCtx *ctx)
{
  return ! ctx->wide && ctx->arena.unsafe;
}

/* The difference of two doubles, exactly */
static pg_attribute_always_inline DyadicX
dx_diff(DyadicCtx *ctx, double a, double b)
{
  DyadicX r = {0};
  if (ctx->filter)
  {
    /* The rounding error of the difference, exactly (two-sum) */
    double v = a - b, bv = v - a;
    r.f.v = v;
    r.f.e = fabs((a - (v - bv)) + (- b - bv));
    return r;
  }
  if (ctx->dd)
  {
    two_diff(a, b, &r.d.hi, &r.d.lo);
    r.d.e = 0.0;
    return r;
  }
  if (! ctx->wide)
  {
    r.x = arcseg_x_diff(&ctx->arena, a, b);
    return r;
  }
  r.w = dyadic_wide_sub(&ctx->warena, dyadic_wide_of_double(&ctx->warena, a),
    dyadic_wide_of_double(&ctx->warena, b));
  return r;
}

static inline DyadicX
dx_val(DyadicCtx *ctx, double v)
{
  return dx_diff(ctx, v, 0.0);
}

/* Whether the expansions of a tower have left their exact range, so that
 * nothing more is computed in them: every answer of a context that is
 * unsafe is discarded (#dyadic_ctx_unsafe, #dyadic_pred_run), and an
 * expansion holding a component that overflowed keeps every component of
 * the values it is added to or multiplied by, compression merging none of
 * them, so that each later sum or product costs more than the last */
static inline bool
dx_spent(const DyadicCtx *ctx)
{
  return ! ctx->filter && ! ctx->dd && dyadic_ctx_unsafe(ctx);
}

static pg_attribute_always_inline DyadicX
dx_add(DyadicCtx *ctx, DyadicX a, DyadicX b)
{
  DyadicX r = {0};
  if (ctx->filter)
  {
    r.f = arcseg_f_add(a.f, b.f);
    return r;
  }
  if (ctx->dd)
  {
    r.d = ddb_add(a.d, b.d);
    return r;
  }
  if (! ctx->wide)
  {
    if (dx_spent(ctx))
      return a;
    r.x = arcseg_x_add(&ctx->arena, a.x, b.x);
    return r;
  }
  r.w = dyadic_wide_add(&ctx->warena, a.w, b.w);
  return r;
}

/* Multiplied by a power of two, possibly negative, exactly */
static pg_attribute_always_inline DyadicX
dx_scale(DyadicCtx *ctx, DyadicX a, double k)
{
  DyadicX r = {0};
  if (ctx->filter)
  {
    r.f = arcseg_f_scale(a.f, k);
    return r;
  }
  if (ctx->dd)
  {
    r.d.hi = a.d.hi * k;
    r.d.lo = a.d.lo * k;
    r.d.e = a.d.e * fabs(k);
    return r;
  }
  if (! ctx->wide)
  {
    r.x = arcseg_x_scale(&ctx->arena, a.x, k);
    return r;
  }
  int e;
  frexp(k, &e);
  r.w = dyadic_wide_signed(&ctx->warena, a.w, (k < 0.0) ? -1 : 1);
  if (r.w->sign != 0)
    r.w->exp += e - 1;
  return r;
}

static pg_attribute_always_inline DyadicX
dx_sub(DyadicCtx *ctx, DyadicX a, DyadicX b)
{
  if (ctx->wide && ! ctx->filter && ! ctx->dd)
  {
    DyadicX r = {0};
    r.w = dyadic_wide_sub(&ctx->warena, a.w, b.w);
    return r;
  }
  return dx_add(ctx, a, dx_scale(ctx, b, -1.0));
}

static pg_attribute_always_inline DyadicX
dx_mul(DyadicCtx *ctx, DyadicX a, DyadicX b)
{
  DyadicX r = {0};
  if (ctx->filter)
  {
    r.f = arcseg_f_mul(a.f, b.f);
    /* A product that underflows to 0 is not the exact zero #dx_sign reads */
    if (r.f.v == 0.0 && r.f.e == 0.0 && (a.f.v != 0.0 || a.f.e != 0.0) &&
        (b.f.v != 0.0 || b.f.e != 0.0))
      r.f.e = DBL_MIN;
    return r;
  }
  if (ctx->dd)
  {
    r.d = ddb_mul(a.d, b.d);
    return r;
  }
  if (! ctx->wide)
  {
    if (dx_spent(ctx))
      return a;
    r.x = arcseg_x_mul(&ctx->arena, a.x, b.x);
    return r;
  }
  r.w = dyadic_wide_mul(&ctx->warena, a.w, b.w);
  return r;
}

/* Whether a value is zero, read off its representation */
static pg_attribute_always_inline bool
dx_zero(const DyadicCtx *ctx, DyadicX a)
{
  if (ctx->filter)
    return a.f.v == 0.0 && a.f.e == 0.0;
  if (ctx->dd)
    return ddb_is_zero(a.d);
  if (ctx->wide)
    return a.w->sign == 0;
  for (int i = 0; i < a.x.n; i++)
    if (a.x.e[i] != 0.0)
      return false;
  return a.x.err == 0.0;
}

#endif /* __GEO_DYADIC_H__ */
