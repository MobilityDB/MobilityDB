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

#include "geo/geo_funcs.h"

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
cross_product_sign_offset(double ax, double ay, double bx, double by,
  double cx, double cy, double dx, double dy, double oab, double ocd);
extern void
exact_sum_set(ExactSum *sum, const double *terms, int nterms);
extern int
polynomial_sign_pairs(const ExactSum *sums, const PolyTerm *terms,
  int nterms);
extern int
polynomial_sign_exact(const ExactSum *sums, const PolyTerm *terms,
  int nterms);
extern void
dyadic_wide_arena_init(DyadicWideArena *ar);
extern void
dyadic_wide_arena_free(DyadicWideArena *ar);
extern DyadicWideMark
dyadic_wide_arena_mark(const DyadicWideArena *ar);
extern void
dyadic_wide_arena_release(DyadicWideArena *ar, DyadicWideMark m);
extern pg_noinline void
dyadic_wide_arena_grow(DyadicWideArena *ar, size_t words);
extern DyadicWide *
dyadic_wide_of_double(DyadicWideArena *ar, double x);
extern DyadicWide *
dyadic_wide_signed(DyadicWideArena *ar, const DyadicWide *a, int sign);
extern int
dyadic_wide_cmp(const DyadicWide *a, const DyadicWide *b);
extern DyadicWide *
dyadic_wide_add_signed(DyadicWideArena *ar, const DyadicWide *a, int sa,
  const DyadicWide *b, int sb);
extern DyadicWide *
dyadic_wide_mul(DyadicWideArena *ar, const DyadicWide *a, const DyadicWide *b);
extern int
dyadic_wide_product_cmp(DyadicWideArena *ar, const DyadicWide *a,
  const DyadicWide *b, const DyadicWide *c, const DyadicWide *d);
extern double
dyadic_wide_frexp(const DyadicWide *a, int *exp);
extern double
dyadic_wide_approx(const DyadicWide *a);
extern DyadicWide *
dyadic_wide_of_int64(DyadicWideArena *ar, int64 v);
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
nearest_double_exact(double c, NearestSide side, const void *arg);
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
extern int
point_segment_dot_sign_inputs(const double *p, const double *s,
  const double *a, const double *b, int ndims);
extern void
ddb_triple3_points(const double *a, const double *b, const double *c,
  const double *d, DDBound *r);
extern bool
ddb_nearest_root(DDBound n, DDBound d, double *result);
extern int
point_segment_offset_sign(const double *p, const double *a, const double *b,
  int ndims, double r);
extern int
segment_square_cmp(const ExactSum *sums, const SegmentSquare *s1,
  const SegmentSquare *s2);
extern bool
segment_interior_wide_values(DyadicWideArena *ar, const double *a,
  const double *b, const double *c, const double *d, DyadicWide **en,
  DyadicWide **nn);
extern int
vector_triple_sign(const double *a0, const double *a1, const double *b0,
  const double *b1, const double *c0, const double *c1);
extern int
incircle_sign(double ax, double ay, double bx, double by, double cx,
  double cy, double dx, double dy);
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

/**
 * @brief Find the bit pattern of the double nearest a nonnegative value that
 * @p side places against midpoints, a tie going to the even double, returning
 * false where a side it asks cannot tell or its budget runs out
 * @details The answer is the smallest bit pattern whose upper midpoint the
 * value does not pass, infinity standing above the largest double, whose
 * upper midpoint is `2^1024 - 2^970`. A gallop from the candidate
 * @p bits brackets it, its first two steps of one double, so that a
 * candidate a neighbour or two from the answer asks no more sides than a
 * walk, the steps then doubling, and halving the bracket finds it; a candidate whose own side cannot tell is read from its lower
 * neighbour, which places the value below it or decides nothing. An exact
 * side never answers 2 and a budget of 0 sets no limit, so the search always
 * answers; a side of a filter on doubles answers 2 where it cannot tell, and
 * its budget bounds the sides it is asked, so that the exact search behind it
 * answers what the filter cannot
 * @param[in] bits Bit pattern of the candidate, at most infinity's
 * @param[in] side,arg The side and its argument
 * @param[in] budget Largest number of sides asked, 0 for no limit
 * @param[out] result The bit pattern of the answer where the search answers
 */
static inline bool
nearest_double_search(uint64 bits, NearestBitsSide side,
  void *arg, int budget, uint64 *result)
{
  int asked = 0;
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
  const uint64 max_bits = NEAREST_INF_BITS;
  uint64 lo, hi, step = 1;
  int beyond, moves = 0;
  if (bits > max_bits)
    bits = max_bits;
  if (bits == max_bits)
    beyond = 0;
  else
    NEAREST_ASK(bits, beyond);
  if (beyond == 2)
  {
    /* The candidate cannot tell: below its lower midpoint the answer is at
     * most its lower neighbour, otherwise nothing is decided */
    if (bits == 0)
      return false;
    NEAREST_ASK(bits - 1, beyond);
    if (beyond != 0)
      return false;
    bits -= 1;
  }
  if (beyond == 1)
  {
    /* The answer is above: lo is beyond, hi is not */
    lo = bits;
    while (true)
    {
      hi = (lo < max_bits - step) ? lo + step : max_bits;
      if (hi == max_bits)
        break;
      NEAREST_ASK(hi, beyond);
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
    /* The answer is at or below: hi is not beyond, lo is beyond or below 0 */
    hi = bits;
    while (true)
    {
      if (hi == 0)
      {
        *result = 0;
        return true;
      }
      lo = (hi > step) ? hi - step : 0;
      NEAREST_ASK(lo, beyond);
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
  while (hi - lo > 1)
  {
    uint64 mid = lo + (hi - lo) / 2;
    NEAREST_ASK(mid, beyond);
    if (beyond == 2)
      return false;
    if (beyond)
      lo = mid;
    else
      hi = mid;
  }
#undef NEAREST_ASK
  *result = hi;
  return true;
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
