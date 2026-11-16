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
 * @brief The exact arithmetic of the geometry engine: the representations,
 * the tiers, the expression ladder, the searches and the exact signs
 * @details The algorithms of geo_funcs.c read it through geo_dyadic.h. Every
 * value it holds is a dyadic rational, a double, a pair or an expansion of
 * doubles, or a #DyadicWide
 */

#include <limits.h>
#include <math.h>
#include "postgres.h"
#include <utils/float.h>
#include <utils/memutils.h>  /* TopMemoryContext */
#include <pgtypes.h>
#include <port/pg_bitutils.h>
#include "liblwgeom.h"
#include "liblwgeom_internal.h"
#include "meos.h"
#include "meos_internal_geo.h"
#include "meos_tls.h"
#include "geo/geo_funcs.h"
#include "geo/postgis_funcs.h"
#include "geo/meos_transform.h"
#include "temporal/type_util.h"
#include "geo/geo_dyadic.h"

static int cross_product_sign_wide(double ax, double ay, double bx,
  double by, double cx, double cy, double dx, double dy);
static inline void dbl_cross3(const double *a0, const double *a1,
  const double *b0, const double *b1, double *x, double *mx);

/*****************************************************************************
 * The sign of a cross product where the filter cannot tell
 *****************************************************************************/

/**
 * @brief Add the cross product of the vectors B - A and D - C, exactly, into
 * an expansion held in one of two buffers, and return its length
 * @details Each difference is its rounded value plus the error of the
 * rounding, and each product of two terms is its rounded value plus its
 * error, so the sixteen terms add up to the cross product exactly; a term
 * with a zero factor adds nothing and is skipped. The last component carries
 * the sign of the whole
 * @param[in] buf1,buf2 Room for 32 components each
 * @param[out] result The buffer holding the expansion
 * @return Number of components, 0 exactly where the vectors are parallel
 */
int
cross_product_expansion(double ax, double ay, double bx, double by,
  double cx, double cy, double dx, double dy, double *buf1, double *buf2,
  const double **result)
{
  double d[4], t[4];
  two_diff(bx, ax, &d[0], &t[0]);
  two_diff(dy, cy, &d[1], &t[1]);
  two_diff(by, ay, &d[2], &t[2]);
  two_diff(dx, cx, &d[3], &t[3]);
  /* (d0 + t0) (d1 + t1) - (d2 + t2) (d3 + t3), term by term */
  const double lf[2] = {d[0], t[0]}, lg[2] = {d[1], t[1]};
  const double rf[2] = {d[2], t[2]}, rg[2] = {d[3], t[3]};
  double *e = buf1, *h = buf2, *swap;
  int elen = 0;
  for (int i = 0; i < 2; i++)
    for (int j = 0; j < 2; j++)
    {
      double x, y;
      if (lf[i] != 0.0 && lg[j] != 0.0)
      {
        two_product(lf[i], lg[j], &x, &y);
        elen = grow_expansion(elen, e, x, h);
        swap = e; e = h; h = swap;
        elen = grow_expansion(elen, e, y, h);
        swap = e; e = h; h = swap;
      }
      if (rf[i] != 0.0 && rg[j] != 0.0)
      {
        two_product(rf[i], rg[j], &x, &y);
        elen = grow_expansion(elen, e, - x, h);
        swap = e; e = h; h = swap;
        elen = grow_expansion(elen, e, - y, h);
        swap = e; e = h; h = swap;
      }
    }
  *result = e;
  return elen;
}

/**
 * @brief Return the sign of the cross product of the vectors B - A and D - C,
 * decided exactly
 * @details Each difference is its rounded value plus the error of the
 * rounding. Where the four differences are exact, which the calls the filter
 * of #cross_product_sign cannot sign mostly are, the cross product is a
 * difference of two products of doubles: rounding is monotone, so two rounded
 * products that differ order as the exact ones do, and two that are equal
 * leave the sign to the difference of their rounding errors, which are exact.
 * Otherwise each product of two terms is its rounded value plus its error,
 * and the sixteen terms are added into an expansion whose last component
 * carries the sign; a term with a zero factor adds nothing and is skipped.
 * #cross_product_sign calls it where its filter cannot tell, which keeps this
 * out of every call the filter decides, and a product overflowing the
 * doubles is one it cannot tell. The sign does not change when every
 * coordinate is multiplied by one power of two, which is exact, so
 * coordinates beyond 2^510 are first brought below it, where no product of
 * two differences overflows. Where that scaling would round a coordinate
 * below the normal range, or a product of the differences could underflow,
 * the sign is read on #DyadicWide (#cross_product_sign_wide), exact for any
 * finite coordinates.
 * @return 1 or -1 for the two turns, 0 exactly where the vectors are parallel
 */
int
cross_product_sign_exact(double ax, double ay, double bx, double by,
  double cx, double cy, double dx, double dy)
{
  double v[8] = {ax, ay, bx, by, cx, cy, dx, dy};
  double big = 0.0;
  for (int i = 0; i < 8; i++)
    big = Max(big, fabs(v[i]));
  if (big > 0x1p510)
  {
    int k = ilogb(big) - 509;
    for (int i = 0; i < 8; i++)
    {
      double s = ldexp(v[i], -k);
      if (ldexp(s, k) != v[i])
        return cross_product_sign_wide(ax, ay, bx, by, cx, cy, dx, dy);
      v[i] = s;
    }
  }
  double d[4], t[4];
  two_diff(v[2], v[0], &d[0], &t[0]);
  two_diff(v[7], v[5], &d[1], &t[1]);
  two_diff(v[3], v[1], &d[2], &t[2]);
  two_diff(v[6], v[4], &d[3], &t[3]);
  /* A product of two terms is exact where the sum of the exponents of their
   * lowest bits reaches that of the least subnormal */
  int low[4];
  for (int i = 0; i < 4; i++)
  {
    low[i] = INT_MAX;
    if (d[i] != 0.0)
      low[i] = double_lowest_set_bit(d[i]);
    if (t[i] != 0.0)
      low[i] = Min(low[i], double_lowest_set_bit(t[i]));
  }
  if ((low[0] != INT_MAX && low[1] != INT_MAX && low[0] + low[1] < -1074) ||
      (low[2] != INT_MAX && low[3] != INT_MAX && low[2] + low[3] < -1074))
    return cross_product_sign_wide(ax, ay, bx, by, cx, cy, dx, dy);
  ax = v[0]; ay = v[1]; bx = v[2]; by = v[3];
  cx = v[4]; cy = v[5]; dx = v[6]; dy = v[7];
  if (t[0] == 0.0 && t[1] == 0.0 && t[2] == 0.0 && t[3] == 0.0)
  {
    double l = d[0] * d[1], r = d[2] * d[3];
    if (l != r)
      return (l > r) ? 1 : -1;
    double err = fma(d[0], d[1], - l) - fma(d[2], d[3], - r);
    return (err > 0.0) ? 1 : ((err < 0.0) ? -1 : 0);
  }
  double buf1[32], buf2[32];
  const double *e;
  int elen = cross_product_expansion(ax, ay, bx, by, cx, cy, dx, dy, buf1,
    buf2, &e);
  /* Every term with a zero factor: the cross product is zero */
  if (elen == 0)
    return 0;
  double top = e[elen - 1];
  return (top > 0.0) ? 1 : ((top < 0.0) ? -1 : 0);
}

/**
 * @brief Return the sign of the cross product of the vectors B - A and D - C
 * where each x difference carries an exact offset
 * @details The sign of `(bx - ax + oab) * (dy - cy) - (by - ay) *
 * (dx - cx + ocd)`. A cell of a grid read across the antimeridian holds
 * vertices whose longitude is shifted by a multiple of 360 degrees, and that
 * shift is not exact in doubles: 21.599649137590685 + 360 rounds, and two
 * cells then no longer state a vertex they share with the same number. Each
 * shifted difference is an exact sum of doubles instead, read through
 * #polynomial_sign_exact, whose filter in doubles decides first. Without an
 * offset the sign is #cross_product_sign's
 * @param[in] ax,ay,bx,by,cx,cy,dx,dy Coordinates
 * @param[in] oab,ocd Offsets of `bx - ax` and of `dx - cx`, exact doubles such
 * as a multiple of 360
 * @return 1 or -1 for the two turns, 0 exactly where the vectors are parallel
 * @note Exact where no product of the components of the sums overflows or
 * underflows, as #polynomial_sign_exact states it
 */
int
cross_product_sign_offset(double ax, double ay, double bx, double by,
  double cx, double cy, double dx, double dy, double oab, double ocd)
{
  if (oab == 0.0 && ocd == 0.0)
    return cross_product_sign(ax, ay, bx, by, cx, cy, dx, dy);
  ExactSum sums[4];
  const double t0[3] = {bx, - ax, oab}, t1[2] = {dy, - cy};
  const double t2[2] = {by, - ay}, t3[3] = {dx, - cx, ocd};
  exact_sum_set(&sums[0], t0, 3);
  exact_sum_set(&sums[1], t1, 2);
  exact_sum_set(&sums[2], t2, 2);
  exact_sum_set(&sums[3], t3, 3);
  PolyTerm terms[2] = {
    {.coef = 1.0, .deg = 2, .sum = {0, 1}},
    {.coef = -1.0, .deg = 2, .sum = {2, 3}}};
  return polynomial_sign_exact(sums, terms, 2);
}

/*****************************************************************************
 * The circle of an arc, decided exactly on its input points
 *****************************************************************************/

/**
 * @brief Return the sign of a bounded value where its bound decides it, 0 for
 * an exact zero, and 2 where the bound cannot tell
 */
static pg_attribute_always_inline int
ddb_sign_known(DDBound x)
{
  if (ddb_is_zero(x))
    return 0;
  int sign = ddb_sign(x);
  return (sign != 0) ? sign : 2;
}

/**
 * @brief Multiply an expansion by a double exactly, and return the length of
 * the product
 * @details Each component times the double is its rounded value plus its
 * error (#two_product), and the terms are carried through in increasing order
 * of magnitude, so the product is again an expansion, its zero components
 * dropped and its last component carrying its sign
 * @param[out] h Room for 2 * elen components
 */
static int
scale_expansion(int elen, const double *e, double b, double *h)
{
  int hlen = 0;
  double q, hh;
  two_product(e[0], b, &q, &hh);
  if (hh != 0.0)
    h[hlen++] = hh;
  for (int i = 1; i < elen; i++)
  {
    double p1, p0, sum;
    two_product(e[i], b, &p1, &p0);
    two_sum(q, p0, &sum, &hh);
    if (hh != 0.0)
      h[hlen++] = hh;
    two_sum(p1, sum, &q, &hh);
    if (hh != 0.0)
      h[hlen++] = hh;
  }
  if (q != 0.0 || hlen == 0)
    h[hlen++] = q;
  return hlen;
}

/**
 * @brief Add two expansions exactly, and return the length of the sum
 * @details Each component of the second is added into the first
 * (#grow_expansion); the sum is left in @p h, which may be @p e itself
 * @param[out] h Room for elen + flen components
 * @param tmp Scratch of the same room
 */
static int
expansion_sum(int elen, const double *e, int flen, const double *f,
  double *h, double *tmp)
{
  double *cur = h, *nxt = tmp;
  if (cur != e)
    memcpy(cur, e, (size_t) elen * sizeof(double));
  int len = elen;
  for (int i = 0; i < flen; i++)
  {
    len = grow_expansion(len, cur, f[i], nxt);
    double *swap = cur; cur = nxt; nxt = swap;
  }
  if (cur != h)
    memcpy(h, cur, (size_t) len * sizeof(double));
  return len;
}

/**
 * @brief Multiply two expansions exactly, and return the length of the
 * product
 * @param[out] h Room for 2 * elen * flen components
 * @param tmp,part Scratch of that room and of 2 * elen components
 */
static int
expansion_product(int elen, const double *e, int flen, const double *f,
  double *h, double *tmp, double *part)
{
  int hlen = 0;
  for (int j = 0; j < flen; j++)
  {
    int plen = scale_expansion(elen, e, f[j], part);
    if (hlen == 0)
    {
      memcpy(h, part, (size_t) plen * sizeof(double));
      hlen = plen;
    }
    else
      hlen = expansion_sum(hlen, h, plen, part, h, tmp);
  }
  return hlen;
}

/*****************************************************************************
 * The sign of a polynomial in exact sums of doubles
 *****************************************************************************/

/**
 * @brief Compress an expansion into one of the same value with fewer
 * components, and return its length
 * @details The components are summed from the largest down and then from the
 * smallest up, each rounding error kept (Shewchuk's Compress), so the result
 * is again an expansion whose last component carries the sign of the whole
 * @param[out] h Room for elen components, distinct from @p e
 */
static int
compress_expansion(int elen, const double *e, double *h)
{
  int bottom = elen - 1;
  double q = e[bottom];
  for (int i = elen - 2; i >= 0; i--)
  {
    double qnew, err;
    two_sum(q, e[i], &qnew, &err);
    if (err != 0.0)
    {
      h[bottom--] = qnew;
      q = err;
    }
    else
      q = qnew;
  }
  h[bottom] = q;
  int top = 0;
  for (int i = bottom + 1; i < elen; i++)
  {
    double qnew, err;
    two_sum(h[i], q, &qnew, &err);
    if (err != 0.0)
      h[top++] = err;
    q = qnew;
  }
  h[top++] = q;
  return top;
}

/**
 * @brief Set an exact sum to the sum of doubles
 * @details The sum is held exactly as an expansion (#grow_expansion), with the
 * double nearest to it and a bound on the error of that double, which the
 * filter of #polynomial_sign_exact reads
 * @param[out] sum Exact sum
 * @param[in] terms Doubles to add, a subtraction given as a negated term
 * @param[in] nterms Number of terms, at most EXACT_SUM_MAXTERMS
 */
void
exact_sum_set(ExactSum *sum, const double *terms, int nterms)
{
  assert(nterms > 0 && nterms <= EXACT_SUM_MAXTERMS);
  int len = 0;
  if (nterms == 2)
  {
    /* The second step of #grow_expansion on the first term, with no copy:
     * the error of the sum where it is not zero, then the sum */
    double q = terms[1], e0 = terms[0];
    double s = q + e0, bv = s - q, av = s - bv;
    double err = (q - av) + (e0 - bv);
    if (err != 0.0)
      sum->e[len++] = err;
    if (s != 0.0 || len == 0)
      sum->e[len++] = s;
  }
  else
  {
    double buf[EXACT_SUM_MAXTERMS + 1];
    for (int i = 0; i < nterms; i++)
    {
      len = grow_expansion(len, sum->e, terms[i], buf);
      memcpy(sum->e, buf, (size_t) len * sizeof(double));
    }
  }
  sum->n = len;
  double approx = 0.0, mag = 0.0;
  for (int i = 0; i < len; i++)
  {
    approx += sum->e[i];
    mag += fabs(sum->e[i]);
  }
  sum->approx = approx;
  sum->err = len * DBL_EPSILON * mag;
  return;
}

/**
 * @brief Return the sign of a polynomial in exact sums where its evaluation
 * in pairs of doubles carries it, and 2 where it cannot tell
 * @details The second filter of #polynomial_sign_exact. Each exact sum is read
 * as a pair, its largest component and the rounded sum of the others, within
 * 2 u of those others, u being DBL_EPSILON / 2. Each term is a pair, its
 * coefficient times its sums: the product of two pairs is the exact product of
 * their larger parts (#two_product) plus the rounded cross terms, within
 * 8.1 u^2 of the product of the larger parts, besides the errors the factors
 * carry, which the bound carries through the product. The terms are added
 * into a pair (#two_sum), each rounding of the smaller part adding u of it.
 * Where the pair stands clear of twice the bound, every bound inflated for its
 * own rounding, its sign is the polynomial's. A product of the larger parts
 * outside 2^-900 to 2^900, which could lose its error to underflow, declines.
 * Mirrors #incircle_sign_filter
 * @param[in] sums Exact sums the terms refer to
 * @param[in] terms,nterms Terms of the polynomial and their number
 */
int
polynomial_sign_pairs(const ExactSum *sums, const PolyTerm *terms,
  int nterms)
{
  const double u = DBL_EPSILON / 2.0, inflate = 1.0 + 16.0 * DBL_EPSILON;
  double H = 0.0, L = 0.0, E = 0.0;
  for (int i = 0; i < nterms; i++)
  {
    /* The term, starting from its coefficient, which is exact */
    double h = terms[i].coef, l = 0.0, e = 0.0;
    for (int k = 0; k < terms[i].deg; k++)
    {
      const ExactSum *s = &sums[terms[i].sum[k]];
      /* The sum as a pair */
      double rest = 0.0, restmag = 0.0;
      for (int j = 0; j < s->n - 1; j++)
      {
        rest += s->e[j];
        restmag += fabs(s->e[j]);
      }
      double sh, sl;
      two_sum(s->e[s->n - 1], rest, &sh, &sl);
      double se = 2.0 * u * restmag * inflate;
      /* The product of the two pairs */
      double p, pe;
      two_product(h, sh, &p, &pe);
      double ap = fabs(p);
      if (! isfinite(p) || (ap != 0.0 && (ap < 0x1p-900 || ap > 0x1p900)))
        return 2;
      double c = (pe + h * sl) + l * sh;
      double ne = (e * (fabs(sh) + fabs(sl)) + (fabs(h) + fabs(l)) * se +
        e * se + 8.1 * u * u * fabs(h) * fabs(sh)) * inflate;
      two_sum(p, c, &h, &l);
      e = ne;
    }
    /* The term added to the total */
    double x;
    two_sum(H, h, &H, &x);
    double l1 = L + x;
    L = l1 + l;
    E = (E + e + u * (fabs(l1) + fabs(L))) * inflate;
  }
  double v = H + L;
  double bound = 2.0 * (E + 8.0 * DBL_MIN * nterms * POLY_TERM_MAXDEG) *
    inflate;
  if (! isfinite(v) || ! isfinite(bound))
    return 2;
  if (v > bound)
    return 1;
  if (v < - bound)
    return -1;
  return 2;
}

/**
 * @brief Return the exponent of a power of two, the multiplier of a product
 */
static inline int
double_pow2_exponent(double f)
{
  uint64_t bits;
  memcpy(&bits, &f, sizeof(bits));
  return (int) ((bits >> 52) & 0x7FF) - 1023;
}

/**
 * @brief Return true if #square_products_expansion holds every product
 * exactly, as its rounded value and the error of that rounding, each times its
 * multiplier, and adds them without overflow
 * @details The error is a double exactly where the lowest bit the product
 * times its multiplier can carry, the sum of the lowest bits of its factors
 * and of the exponent of the multiplier, is not below 2^-1074, the lowest bit
 * of a double, and a product below 2^1016 leaves the sum of fifteen of them
 * finite; a sum of doubles is otherwise exact (#two_sum)
 */
static bool
square_products_exact(const double *left, const double *right,
  const double *factor, int n)
{
  for (int j = 0; j < n; j++)
  {
    if (left[j] == 0.0 || right[j] == 0.0)
      continue;
    int ef = double_pow2_exponent(factor[j]);
    if (double_lowest_set_bit(left[j]) + double_lowest_set_bit(right[j]) +
        ef < -1074)
      return false;
    /* A double is below 2^53 times its unit in the last place */
    int la = double_lowest_bit(left[j]), lb = double_lowest_bit(right[j]);
    if (la + lb + ef + 106 > 1016)
      return false;
  }
  return true;
}

/**
 * @brief Set the product of two 64-bit integers as two 64-bit halves, from
 * the four products of their 32-bit halves, each held in 64 bits
 */
static inline void
wide_mul64(uint64 a, uint64 b, uint64 *hi, uint64 *lo)
{
  uint64 a0 = a & 0xFFFFFFFFULL, a1 = a >> 32;
  uint64 b0 = b & 0xFFFFFFFFULL, b1 = b >> 32;
  uint64 p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
  uint64 mid = (p00 >> 32) + (p01 & 0xFFFFFFFFULL) + (p10 & 0xFFFFFFFFULL);
  *lo = (p00 & 0xFFFFFFFFULL) | (mid << 32);
  *hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
  return;
}

/**
 * @brief Set in @p out, of @p nx + @p ny limbs, the product of two
 * magnitudes of @p nx and @p ny limbs, the lowest first
 */
static void
wide_limbs_mul(const uint64 *x, int nx, const uint64 *y, int ny,
  uint64 *out)
{
  memset(out, 0, (size_t) (nx + ny) * sizeof(uint64));
  for (int i = 0; i < nx; i++)
  {
    uint64 carry = 0;
    for (int j = 0; j < ny; j++)
    {
      uint64 hi, lo;
      wide_mul64(x[i], y[j], &hi, &lo);
      uint64 s = out[i + j] + lo;
      hi += (s < lo);
      uint64 s2 = s + carry;
      hi += (s2 < s);
      out[i + j] = s2;
      carry = hi;
    }
    /* No earlier row reaches past limb i + ny - 1 */
    out[i + ny] = carry;
  }
  return;
}

/*****************************************************************************
 * The wide number
 *****************************************************************************/

/**
 * @brief Set an empty #DyadicWideArena
 */
void
dyadic_wide_arena_init(DyadicWideArena *ar)
{
  ar->used = 0;
  ar->cap = DYADIC_WIDE_ARENA_WORDS;
  ar->blocks = NULL;
  ar->caps = NULL;
  ar->nblocks = ar->nkept = ar->capblocks = 0;
  return;
}

/**
 * @brief Free the blocks of a #DyadicWideArena, which is then empty
 */
void
dyadic_wide_arena_free(DyadicWideArena *ar)
{
  for (int i = 0; i < ar->nkept; i++)
    pfree(ar->blocks[i]);
  if (ar->blocks)
  {
    pfree(ar->blocks);
    pfree(ar->caps);
  }
  dyadic_wide_arena_init(ar);
  return;
}

/**
 * @brief Return the point a #DyadicWideArena has reached, which
 * #dyadic_wide_arena_release returns it to
 */
DyadicWideMark
dyadic_wide_arena_mark(const DyadicWideArena *ar)
{
  DyadicWideMark m;
  m.nblocks = ar->nblocks;
  m.used = ar->used;
  return m;
}

/**
 * @brief Return a #DyadicWideArena to a point it reached, every value taken
 * after it being released; the blocks are kept for the values taken next
 */
void
dyadic_wide_arena_release(DyadicWideArena *ar, DyadicWideMark m)
{
  ar->nblocks = m.nblocks;
  ar->cap = m.nblocks ? ar->caps[m.nblocks - 1] : DYADIC_WIDE_ARENA_WORDS;
  ar->used = m.used;
  return;
}

/* Make the current block of an arena a block of at least @p words free:
 * the next block held where it is large enough, and otherwise a new one of
 * twice the words of the current block, the held ones past it freed */
pg_noinline void
dyadic_wide_arena_grow(DyadicWideArena *ar, size_t words)
{
  if (ar->nblocks == ar->nkept || ar->caps[ar->nblocks] < words)
  {
    for (int i = ar->nblocks; i < ar->nkept; i++)
      pfree(ar->blocks[i]);
    ar->nkept = ar->nblocks;
    if (ar->nkept == ar->capblocks)
    {
      ar->capblocks = ar->capblocks ? 2 * ar->capblocks : 8;
      ar->blocks = ar->blocks ?
        repalloc(ar->blocks, sizeof(uint64 *) * ar->capblocks) :
        palloc(sizeof(uint64 *) * ar->capblocks);
      ar->caps = ar->caps ?
        repalloc(ar->caps, sizeof(size_t) * ar->capblocks) :
        palloc(sizeof(size_t) * ar->capblocks);
    }
    size_t cap = Max(2 * ar->cap, 2 * words);
    ar->blocks[ar->nkept] = palloc(sizeof(uint64) * cap);
    ar->caps[ar->nkept] = cap;
    ar->nkept++;
  }
  ar->cap = ar->caps[ar->nblocks];
  ar->nblocks++;
  ar->used = 0;
  return;
}

/* Drop the zero limbs at both ends of a value, those at the bottom raising
 * its exponent */
static void
dyadic_wide_normalize(DyadicWide *a)
{
  while (a->len > 0 && a->limb[a->len - 1] == 0)
    a->len--;
  if (a->len == 0)
  {
    a->sign = 0;
    a->exp = 0;
    return;
  }
  int z = 0;
  while (a->limb[z] == 0)
    z++;
  if (z > 0)
  {
    memmove(a->limb, a->limb + z, (size_t) (a->len - z) * sizeof(uint64));
    a->len -= z;
    a->exp += 64 * z;
  }
  return;
}

/**
 * @brief Return a finite double, exactly: its integer mantissa times the
 * weight of its lowest bit
 */
DyadicWide *
dyadic_wide_of_double(DyadicWideArena *ar, double x)
{
  DyadicWide *r = dyadic_wide_alloc(ar, 1);
  if (x == 0.0)
    return r;
  uint64 bits;
  memcpy(&bits, &x, sizeof(bits));
  int e = (int) ((bits >> 52) & 0x7FF);
  uint64 m = bits & 0xFFFFFFFFFFFFFULL;
  if (e)
  {
    m |= 1ULL << 52;
    r->exp = e - 1075;
  }
  else
    r->exp = -1074;
  r->sign = (bits >> 63) ? -1 : 1;
  r->limb[0] = m;
  r->len = 1;
  return r;
}

/**
 * @brief Return a copy of a value, @p sign times it
 */
DyadicWide *
dyadic_wide_signed(DyadicWideArena *ar, const DyadicWide *a, int sign)
{
  DyadicWide *r = dyadic_wide_alloc(ar, a->len);
  memcpy(r->limb, a->limb, sizeof(uint64) * (size_t) a->len);
  r->len = a->len;
  r->exp = a->exp;
  r->sign = a->sign * sign;
  return r;
}

/* Limb @p i of the magnitude of a value moved up by @p ls limbs and @p bs
 * bits, bs below 64 */
static inline uint64
dyadic_wide_read(const DyadicWide *a, int ls, int bs, int i)
{
  int j = i - ls;
  uint64 v = 0;
  if ((unsigned) j < (unsigned) a->len)
    v = a->limb[j] << bs;
  if (bs && (unsigned) (j - 1) < (unsigned) a->len)
    v |= a->limb[j - 1] >> (64 - bs);
  return v;
}

/* Limb @p i of the magnitude of a value read at the weight 2^e of its
 * lowest bit, e not above its exponent */
static inline uint64
dyadic_wide_limb_at(const DyadicWide *a, int e, int i)
{
  int s = a->exp - e;
  return dyadic_wide_read(a, s >> 6, s & 63, i);
}

/* The weight of the bit above the highest one of a value that is not 0 */
static inline int
dyadic_wide_top(const DyadicWide *a)
{
  return a->exp + 64 * (a->len - 1) +
    pg_leftmost_one_pos64(a->limb[a->len - 1]) + 1;
}

/* -1, 0 or 1 as the magnitude of @p a is below, equal to or above that of
 * @p b: their highest bits, then their limbs read at the lower of their
 * lowest bits */
static int
dyadic_wide_cmp_mag(const DyadicWide *a, const DyadicWide *b)
{
  if (a->sign == 0 || b->sign == 0)
    return (a->sign != 0) - (b->sign != 0);
  int ta = dyadic_wide_top(a), tb = dyadic_wide_top(b);
  if (ta != tb)
    return (ta > tb) ? 1 : -1;
  int e = Min(a->exp, b->exp);
  for (int i = (ta - e + 63) / 64; i >= 0; i--)
  {
    uint64 x = dyadic_wide_limb_at(a, e, i), y = dyadic_wide_limb_at(b, e, i);
    if (x != y)
      return (x > y) ? 1 : -1;
  }
  return 0;
}

/**
 * @brief Return -1, 0 or 1 as @p a is below, equal to or above @p b
 */
int
dyadic_wide_cmp(const DyadicWide *a, const DyadicWide *b)
{
  if (a->sign != b->sign)
    return (a->sign > b->sign) ? 1 : -1;
  if (a->sign == 0)
    return 0;
  return a->sign * dyadic_wide_cmp_mag(a, b);
}

/**
 * @brief Return `sa a + sb b`, @p sa and @p sb each -1 or 1, exactly
 * @details The magnitudes are read at the lower of their lowest bits as they
 * are added, or the second subtracted from the first, a borrow out of the
 * highest limb telling that the second is the larger, the difference then
 * negated
 */
DyadicWide *
dyadic_wide_add_signed(DyadicWideArena *ar, const DyadicWide *a, int sa,
  const DyadicWide *b, int sb)
{
  int xa = a->sign * sa, xb = b->sign * sb;
  if (xa == 0)
    return dyadic_wide_signed(ar, b, sb);
  if (xb == 0)
    return dyadic_wide_signed(ar, a, sa);
  int e = Min(a->exp, b->exp);
  int n = (Max(dyadic_wide_top(a), dyadic_wide_top(b)) - e + 63) / 64;
  DyadicWide *r = dyadic_wide_alloc(ar, n + 1);
  int da = a->exp - e, db = b->exp - e;
  int la = da >> 6, ba = da & 63, lb = db >> 6, bb = db & 63;
  r->exp = e;
  if (xa == xb)
  {
    uint64 carry = 0;
    for (int i = 0; i < n; i++)
    {
      uint64 x = dyadic_wide_read(a, la, ba, i),
        y = dyadic_wide_read(b, lb, bb, i);
      uint64 s = x + y;
      uint64 c1 = s < x;
      r->limb[i] = s + carry;
      carry = c1 + (r->limb[i] < s);
    }
    r->limb[n] = carry;
    r->len = n + 1;
    r->sign = xa;
  }
  else
  {
    uint64 borrow = 0;
    for (int i = 0; i < n; i++)
    {
      uint64 x = dyadic_wide_read(a, la, ba, i),
        y = dyadic_wide_read(b, lb, bb, i);
      uint64 d = x - y;
      uint64 b1 = x < y;
      r->limb[i] = d - borrow;
      borrow = b1 + (d < borrow);
    }
    r->len = n;
    r->sign = xa;
    if (borrow)
    {
      /* The difference was taken modulo 2^(64 n): its negation is the
       * magnitude, of the sign of the second */
      uint64 c = 1;
      for (int i = 0; i < n; i++)
      {
        r->limb[i] = ~r->limb[i] + c;
        c = c && r->limb[i] == 0;
      }
      r->sign = xb;
    }
  }
  dyadic_wide_normalize(r);
  return r;
}

/* The product of a value that is not 0 and one limb @p m that is not 0,
 * times @p sign and the weight 2^e: one product of 64 bits a limb */
static DyadicWide *
dyadic_wide_mul_limb(DyadicWideArena *ar, const DyadicWide *a, uint64 m,
  int sign, int e)
{
  DyadicWide *r = dyadic_wide_alloc(ar, a->len + 1);
  r->sign = a->sign * sign;
  r->exp = a->exp + e;
  if (a->len == 1)
  {
    /* One product of 128 bits, either half possibly 0 */
    uint64 hi, lo;
    wide_mul64(a->limb[0], m, &hi, &lo);
    if (lo == 0)
    {
      r->limb[0] = hi;
      r->len = 1;
      r->exp += 64;
    }
    else
    {
      r->limb[0] = lo;
      r->limb[1] = hi;
      r->len = hi ? 2 : 1;
    }
    return r;
  }
  uint64 carry = 0;
  for (int i = 0; i < a->len; i++)
  {
    uint64 hi, lo;
    wide_mul64(a->limb[i], m, &hi, &lo);
    lo += carry;
    carry = hi + (lo < carry);
    r->limb[i] = lo;
  }
  r->limb[a->len] = carry;
  r->len = a->len + 1;
  dyadic_wide_normalize(r);
  return r;
}

/**
 * @brief Return the product of two values, exactly, a factor of one limb
 * read as one
 */
DyadicWide *
dyadic_wide_mul(DyadicWideArena *ar, const DyadicWide *a, const DyadicWide *b)
{
  if (a->sign == 0 || b->sign == 0)
    return dyadic_wide_alloc(ar, 1);
  if (b->len == 1)
    return dyadic_wide_mul_limb(ar, a, b->limb[0], b->sign, b->exp);
  if (a->len == 1)
    return dyadic_wide_mul_limb(ar, b, a->limb[0], a->sign, a->exp);
  DyadicWide *r = dyadic_wide_alloc(ar, a->len + b->len);
  wide_limbs_mul(a->limb, a->len, b->limb, b->len, r->limb);
  r->len = a->len + b->len;
  r->sign = a->sign * b->sign;
  r->exp = a->exp + b->exp;
  dyadic_wide_normalize(r);
  return r;
}

/* The sign of `a b - c d`, exactly: the signs of the products, then their
 * magnitudes */
int
dyadic_wide_product_cmp(DyadicWideArena *ar, const DyadicWide *a,
  const DyadicWide *b, const DyadicWide *c, const DyadicWide *d)
{
  int s1 = a->sign * b->sign, s2 = c->sign * d->sign;
  if (s1 != s2)
    return (s1 > s2) ? 1 : -1;
  if (s1 == 0)
    return 0;
  DyadicWideMark m = dyadic_wide_arena_mark(ar);
  int result = s1 * dyadic_wide_cmp_mag(dyadic_wide_mul(ar, a, b),
    dyadic_wide_mul(ar, c, d));
  dyadic_wide_arena_release(ar, m);
  return result;
}

/**
 * @brief Return the quotient of the 128-bit number `hi 2^64 + lo` by @p d,
 * `hi < d`, and set its remainder, from 32-bit digits (Hacker's Delight,
 * divlu), so that no division wider than 64 bits is taken
 */
static uint64
wide_div128(uint64 hi, uint64 lo, uint64 d, uint64 *rem)
{
  const uint64 b = UINT64CONST(1) << 32;
  int s = 63 - pg_leftmost_one_pos64(d);
  d <<= s;
  uint64 un32 = s ? (hi << s) | (lo >> (64 - s)) : hi;
  uint64 un10 = lo << s;
  uint64 vn1 = d >> 32, vn0 = d & 0xFFFFFFFFULL;
  uint64 un1 = un10 >> 32, un0 = un10 & 0xFFFFFFFFULL;
  uint64 q1 = un32 / vn1, rhat = un32 - q1 * vn1;
  while (q1 >= b || q1 * vn0 > b * rhat + un1)
  {
    q1--;
    rhat += vn1;
    if (rhat >= b)
      break;
  }
  uint64 un21 = un32 * b + un1 - q1 * d;
  uint64 q0 = un21 / vn1;
  rhat = un21 - q0 * vn1;
  while (q0 >= b || q0 * vn0 > b * rhat + un0)
  {
    q0--;
    rhat += vn1;
    if (rhat >= b)
      break;
  }
  *rem = (un21 * b + un0 - q0 * d) >> s;
  return q1 * b + q0;
}

/* The number of limbs of a magnitude of at most @p n limbs, its highest not
 * 0 */
static inline int
wide_limbs_len(const uint64 *a, int n)
{
  while (n > 0 && a[n - 1] == 0)
    n--;
  return n;
}

/* -1, 0 or 1 as a magnitude of @p na limbs is below, equal to or above one
 * of @p nb, the highest limb of each not 0 */
static int
wide_limbs_cmp(const uint64 *a, int na, const uint64 *b, int nb)
{
  if (na != nb)
    return (na > nb) ? 1 : -1;
  for (int i = na - 1; i >= 0; i--)
    if (a[i] != b[i])
      return (a[i] > b[i]) ? 1 : -1;
  return 0;
}

/**
 * @brief Set the quotient and the remainder of two magnitudes, exactly, by
 * words: Knuth's algorithm D on 64-bit limbs, each digit of the quotient
 * estimated from the two highest limbs of the remainder (#wide_div128) and
 * corrected, every product formed by #wide_mul64
 * @param[in] u,nu Dividend, of @p nu limbs, the lowest first
 * @param[in] v,nv Divisor, of @p nv limbs, not 0
 * @param[out] q Quotient, room for `nu - nv + 1` limbs, and at least one
 * @param[out] r Remainder, room for @p nv limbs
 * @param[out] nr Number of limbs of the remainder, its highest not 0
 * @return Number of limbs of the quotient, its highest not 0
 */
int
wide_limbs_divmod(const uint64 *u, int nu, const uint64 *v, int nv,
  uint64 *q, uint64 *r, int *nr)
{
  nu = wide_limbs_len(u, nu);
  nv = wide_limbs_len(v, nv);
  assert(nv > 0);
  if (wide_limbs_cmp(u, nu, v, nv) < 0)
  {
    memcpy(r, u, sizeof(uint64) * (size_t) nu);
    *nr = nu;
    return 0;
  }
  if (nv == 1)
  {
    uint64 rem = 0;
    for (int i = nu - 1; i >= 0; i--)
      q[i] = wide_div128(rem, u[i], v[0], &rem);
    r[0] = rem;
    *nr = rem ? 1 : 0;
    return wide_limbs_len(q, nu);
  }
  /* The divisor shifted until its highest bit is set, and the dividend by
   * as many bits, in one more limb */
  uint64 sbuf[WIDE_LIMBS_STACK];
  size_t need = (size_t) (nu + 1 + nv);
  uint64 *un = (need <= WIDE_LIMBS_STACK) ? sbuf :
    palloc(sizeof(uint64) * need);
  uint64 *vn = un + nu + 1;
  int s = 63 - pg_leftmost_one_pos64(v[nv - 1]);
  for (int i = nv - 1; i > 0; i--)
    vn[i] = (v[i] << s) | (s ? v[i - 1] >> (64 - s) : 0);
  vn[0] = v[0] << s;
  un[nu] = s ? u[nu - 1] >> (64 - s) : 0;
  for (int i = nu - 1; i > 0; i--)
    un[i] = (u[i] << s) | (s ? u[i - 1] >> (64 - s) : 0);
  un[0] = u[0] << s;
  const uint64 vtop = vn[nv - 1], vnext = vn[nv - 2];
  for (int j = nu - nv; j >= 0; j--)
  {
    /* The digit from the two highest limbs, at most one above the base;
     * where the highest limb equals that of the divisor the estimate is the
     * largest digit, its remainder past 64 bits when the sum carries */
    uint64 qhat, rhat;
    bool rbig;
    if (un[j + nv] >= vtop)
    {
      qhat = ~UINT64CONST(0);
      rhat = un[j + nv - 1] + vtop;
      rbig = rhat < vtop;
    }
    else
    {
      qhat = wide_div128(un[j + nv], un[j + nv - 1], vtop, &rhat);
      rbig = false;
    }
    /* At most two corrections, read on the next limb of the divisor */
    while (! rbig)
    {
      uint64 ph, pl;
      wide_mul64(qhat, vnext, &ph, &pl);
      if (ph < rhat || (ph == rhat && pl <= un[j + nv - 2]))
        break;
      qhat--;
      rhat += vtop;
      rbig = rhat < vtop;
    }
    /* The remainder less the digit times the divisor */
    uint64 borrow = 0, carry = 0;
    for (int i = 0; i < nv; i++)
    {
      uint64 ph, pl;
      wide_mul64(qhat, vn[i], &ph, &pl);
      pl += carry;
      carry = ph + (pl < carry);
      uint64 t = un[i + j] - pl;
      uint64 b1 = un[i + j] < pl;
      un[i + j] = t - borrow;
      borrow = b1 + (t < borrow);
    }
    uint64 t = un[j + nv] - carry;
    uint64 b1 = un[j + nv] < carry;
    un[j + nv] = t - borrow;
    bool neg = b1 || t < borrow;
    q[j] = qhat;
    if (neg)
    {
      /* The digit was one too large: add the divisor back */
      q[j]--;
      uint64 c = 0;
      for (int i = 0; i < nv; i++)
      {
        uint64 sum = un[i + j] + c;
        uint64 c1 = sum < c;
        un[i + j] = sum + vn[i];
        c = c1 + (un[i + j] < vn[i]);
      }
      un[j + nv] += c;
    }
  }
  for (int i = 0; i < nv; i++)
    r[i] = (un[i] >> s) | (s ? un[i + 1] << (64 - s) : 0);
  *nr = wide_limbs_len(r, nv);
  if (un != sbuf)
    pfree(un);
  return wide_limbs_len(q, nu - nv + 1);
}

/**
 * @brief Set the floor of the square root of a magnitude, exactly, by
 * Newton's iteration from above
 * @details The iteration `x <- (x + n / x) / 2` (#wide_limbs_divmod) decreases
 * from any start at or above the root to its floor, and then stops
 * decreasing. It starts from the square root in doubles of the highest 62
 * or 63 bits of @p n, an even number t of bits below them dropped, plus 2,
 * times 2^(t/2): the root of those bits is at most one off in doubles, and
 * `sqrt(n) < (floor(sqrt(m)) + 1) 2^(t/2)` for those bits m
 * @param[in] n,nn The magnitude, of @p nn limbs
 * @param[out] x The root, room for `nn / 2 + 2` limbs
 * @param[out] exact Whether the root is exact, unless NULL
 * @return Number of limbs of the root, its highest not 0
 */
int
wide_limbs_isqrt(const uint64 *n, int nn, uint64 *x, bool *exact)
{
  nn = wide_limbs_len(n, nn);
  if (nn == 0)
  {
    if (exact)
      *exact = true;
    return 0;
  }
  int bits = 64 * (nn - 1) + pg_leftmost_one_pos64(n[nn - 1]) + 1;
  int t = (bits > 62) ? ((bits - 62) & ~1) : 0;
  int ls = t / 64, bs = t % 64;
  uint64 m = n[ls] >> bs;
  if (bs && ls + 1 < nn)
    m |= n[ls + 1] << (64 - bs);
  uint64 x0 = (uint64) sqrt((double) m) + 2;
  /* The scratch: the next iterate, the quotient and the remainder, and the
   * square of the root */
  int nx_cap = nn / 2 + 2;
  uint64 sbuf[WIDE_LIMBS_STACK];
  size_t need = (size_t) (nx_cap + 1) + (size_t) (nn + 1) +
    (size_t) nx_cap + 2 * (size_t) nx_cap;
  uint64 *y = (need <= WIDE_LIMBS_STACK) ? sbuf :
    palloc(sizeof(uint64) * need);
  uint64 *qt = y + nx_cap + 1, *rm = qt + nn + 1, *sq = rm + nx_cap;
  int hs = t / 2, hls = hs / 64, hbs = hs % 64;
  memset(x, 0, sizeof(uint64) * (size_t) (hls + 2));
  x[hls] = x0 << hbs;
  if (hbs)
    x[hls + 1] = x0 >> (64 - hbs);
  int nx = wide_limbs_len(x, hls + 2);
  while (true)
  {
    int nr;
    int nq = wide_limbs_divmod(n, nn, x, nx, qt, rm, &nr);
    /* y = (x + q) / 2 */
    int ny = Max(nx, nq);
    uint64 c = 0;
    for (int i = 0; i < ny; i++)
    {
      uint64 xi = (i < nx) ? x[i] : 0, qi = (i < nq) ? qt[i] : 0;
      uint64 s = xi + qi, c1 = s < xi;
      y[i] = s + c;
      c = c1 + (y[i] < s);
    }
    y[ny] = c;
    for (int i = 0; i <= ny; i++)
      y[i] = (y[i] >> 1) | ((i < ny) ? y[i + 1] << 63 : 0);
    ny = wide_limbs_len(y, ny + 1);
    if (wide_limbs_cmp(y, ny, x, nx) >= 0)
      break;
    memcpy(x, y, sizeof(uint64) * (size_t) ny);
    nx = ny;
  }
  if (exact)
  {
    wide_limbs_mul(x, nx, x, nx, sq);
    *exact = wide_limbs_cmp(sq, wide_limbs_len(sq, 2 * nx), n, nn) == 0;
  }
  if (y != sbuf)
    pfree(y);
  return nx;
}

/**
 * @brief Return a value as a double @p m and a power of two
 * 2^exp whose product is within a relative 2^-63 of its magnitude, from its
 * two highest limbs, so that a caller divides two such values, or scales a
 * quotient, without leaving the doubles
 */
double
dyadic_wide_frexp(const DyadicWide *a, int *exp)
{
  if (a->sign == 0)
  {
    *exp = 0;
    return 0.0;
  }
  int i = a->len - 1;
  double m = (double) a->limb[i];
  if (i > 0)
    m += ldexp((double) a->limb[i - 1], -64);
  *exp = a->exp + 64 * i;
  return m;
}

/**
 * @brief Return the value of a #DyadicWide, rounded, or an infinity or zero
 * outside the doubles
 */
double
dyadic_wide_approx(const DyadicWide *a)
{
  int e;
  double m = dyadic_wide_frexp(a, &e);
  return a->sign * ldexp(m, e);
}

/**
 * @brief Return a 64-bit integer, exactly
 */
DyadicWide *
dyadic_wide_of_int64(DyadicWideArena *ar, int64 v)
{
  DyadicWide *r = dyadic_wide_alloc(ar, 1);
  if (v == 0)
    return r;
  r->sign = (v < 0) ? -1 : 1;
  r->limb[0] = (v < 0) ? (uint64) 0 - (uint64) v : (uint64) v;
  r->len = 1;
  return r;
}

/**
 * @brief Return a copy of a value allocated on its own, freed by pfree, for
 * a value kept past its arena
 */
DyadicWide *
dyadic_wide_dup(const DyadicWide *a)
{
  DyadicWide *r = palloc(offsetof(DyadicWide, limb) +
    sizeof(uint64) * (size_t) Max(a->len, 1));
  memcpy(r, a, offsetof(DyadicWide, limb) +
    sizeof(uint64) * (size_t) a->len);
  return r;
}

/**
 * @brief Write the magnitude of a value as a fixed-point integer and return
 * its sign
 * @details `mag` receives floor(|a| 2^scale), or its ceiling where @p up,
 * in @p nlimbs 64-bit limbs, the lowest first, as #dyadic_wide_cmp_fixed
 * reads one
 * @param[in] a The value
 * @param[in] scale The power of 2 the magnitude is scaled by
 * @param[in] up True for the ceiling, false for the floor
 * @param[out] mag The fixed-point magnitude
 * @param[in] nlimbs Number of limbs of @p mag
 * @return -1, 0 or 1, the sign of @p a; 2 where the magnitude needs more
 * than @p nlimbs limbs, @p mag then undefined: the caller tests it, as
 * nothing is truncated
 */
int
dyadic_wide_to_fixed(const DyadicWide *a, int scale, bool up, uint64 *mag,
  int nlimbs)
{
  memset(mag, 0, nlimbs * sizeof(uint64));
  if (a->sign == 0)
    return 0;
  /* The weight of the lowest bit of the magnitude in the output */
  int shift = a->exp + scale;
  bool lost = false;
  for (int i = 0; i < a->len; i++)
  {
    uint64 l = a->limb[i];
    if (! l)
      continue;
    for (int b = 0; b < 64; b++)
    {
      if (! ((l >> b) & 1))
        continue;
      int pos = 64 * i + b + shift;
      if (pos < 0)
      {
        lost = true;
        continue;
      }
      if (pos >= 64 * nlimbs)
        return 2;
      mag[pos / 64] |= (uint64) 1 << (pos % 64);
    }
  }
  if (up && lost)
  {
    for (int i = 0; i < nlimbs; i++)
      if (++mag[i] != 0)
        return a->sign;
    return 2;
  }
  return a->sign;
}

/**
 * @brief Return the sign of a value less a fixed-point value
 * `sign * mag * 2^exp`, exactly, -1, 0 or 1
 * @details It lets a side function of #nearest_double_exact compare the
 * midpoint the search reads with the bounds of an enclosure it holds as
 * integers, the fixed-point value read in @p ar
 * @param[in] ar Arena of the fixed-point value
 * @param[in] mid The value
 * @param[in] sign Sign of the fixed-point value, -1 or 1; a zero @p mag is
 * the value 0 whatever @p sign says
 * @param[in] mag Its magnitude, @p nlimbs 64-bit limbs, the lowest first, as
 * a #DyadicWide holds its magnitude
 * @param[in] nlimbs Number of limbs of @p mag
 * @param[in] exp Weight of the lowest bit of @p mag
 */
int
dyadic_wide_cmp_fixed(DyadicWideArena *ar, const DyadicWide *mid, int sign,
  const uint64 *mag, int nlimbs, int exp)
{
  assert(nlimbs >= 0);
  assert(sign == -1 || sign == 1);
  DyadicWide *v = dyadic_wide_alloc(ar, nlimbs);
  memcpy(v->limb, mag, sizeof(uint64) * (size_t) nlimbs);
  v->len = nlimbs;
  v->exp = exp;
  v->sign = sign;
  dyadic_wide_normalize(v);
  return dyadic_wide_cmp(mid, v);
}

/**
 * @brief Return in @p ar a sum of products of doubles, each times a power of
 * two from 1/4 to 2 or its negation, computed exactly
 * @details A finite double is an integer mantissa of at most 53 bits times a
 * power of two, so a product of two is the product of their mantissas times
 * the product of their lowest bits and of the multiplier, which #DyadicWide
 * holds exactly whatever the magnitudes of the products, where an expansion
 * loses the low bits of a product below 2^-1074 and overflows past the
 * largest double. #square_products_sign calls it where
 * #square_products_exact says it must
 * @param[in] ar Arena of the sum
 * @param[in] left,right,factor Products, as two factors and a multiplier
 * @param[in] n Number of products
 */
DyadicWide *
square_products_wide(DyadicWideArena *ar, const double *left,
  const double *right, const double *factor, int n)
{
  DyadicWide *acc = dyadic_wide_alloc(ar, 1);
  for (int j = 0; j < n; j++)
  {
    if (left[j] == 0.0 || right[j] == 0.0)
      continue;
    DyadicWide *p = dyadic_wide_mul(ar, dyadic_wide_of_double(ar, left[j]),
      dyadic_wide_of_double(ar, right[j]));
    /* The multiplier moves the lowest bit and may negate */
    if (factor[j] < 0.0)
      p->sign = - p->sign;
    p->exp += double_pow2_exponent(factor[j]);
    acc = dyadic_wide_add(ar, acc, p);
  }
  return acc;
}

/**
 * @brief Return the sign of a sum of products of doubles, each times a power
 * of two from 1/4 to 2 or its negation, computed exactly for any finite
 * doubles
 * @details The sign of the last component of #square_products_expansion where
 * #square_products_exact holds, and of the sum #square_products_wide forms
 * otherwise
 */
int
square_products_sign(const double *left, const double *right,
  const double *factor, int n)
{
  if (! square_products_exact(left, right, factor, n))
  {
    DyadicWideArena ar;
    dyadic_wide_arena_init(&ar);
    int sign = wide_sign(square_products_wide(&ar, left, right, factor, n));
    dyadic_wide_arena_free(&ar);
    return sign;
  }
  double buf1[40], buf2[40];
  const double *ex;
  int len = square_products_expansion(left, right, factor, n, buf1, buf2, &ex);
  if (len == 0)
    return 0;
  double top = ex[len - 1];
  return (top > 0.0) ? 1 : ((top < 0.0) ? -1 : 0);
}

/**
 * @brief Return the sign of the squared distance between two points less the
 * square of `r + rlo + c + g/2`, computed exactly for any finite doubles
 * @details This is the side of a midpoint #point_distance_offset_exact
 * decides, `r + rlo` the radius, `c` a candidate and `g` the signed gap to its
 * neighbour, so that the midpoint is no double where `c` is subnormal. The
 * squared distance is the sum over the coordinates of `q*q - 2*q*p + p*p` and
 * the square the sum of the squares of the terms and of twice the product of
 * each pair, as #point_distance_square_terms sets them, each a product of the
 * input doubles times a power of two, so #square_products_sign decides it
 * where a coordinate difference overflows and where the products of a
 * candidate far below the radius underflow
 * @param[in] p,q Coordinates of the two points
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r,rlo Radius as the sum of two doubles, @p rlo 0 for one double
 * @param[in] c Candidate
 * @param[in] g Gap from the candidate to its upper neighbour, or the negated
 * gap to its lower one
 */
int
point_distance_offset_pair_side_sign(const double *p, const double *q,
  int ndims, double r, double rlo, double c, double g)
{
  /* On two doubles with a bound first (#DDBound), as #segment_interior_mid_sign
   * reads its own: the differences of the coordinates exact as two doubles
   * (#two_diff), the radius `r + rlo` and the candidate exact, half the gap
   * exact where it is; each product and sum adds its bound (#ddb_mul,
   * #ddb_add), and the sign is told where the value lies beyond it */
  double hg = g / 2.0;
  if (hg * 2.0 == g)
  {
    DDBound sq = {0.0, 0.0, 0.0};
    for (int i = 0; i < ndims; i++)
    {
      DDBound d;
      two_diff(q[i], p[i], &d.hi, &d.lo);
      d.e = 0.0;
      sq = ddb_add(sq, ddb_mul(d, d));
    }
    DDBound rad = {r, rlo, 0.0}, cand = {c, 0.0, 0.0}, half = {hg, 0.0, 0.0};
    DDBound t = ddb_add(ddb_add(rad, cand), half);
    int sign = ddb_sign(ddb_sub(sq, ddb_mul(t, t)));
    if (sign != 0)
      return sign;
  }
  double left[19], right[19], factor[19];
  int n = 0;
  for (int i = 0; i < ndims; i++)
  {
    left[n] = q[i]; right[n] = q[i]; factor[n++] = 1.0;
    left[n] = q[i]; right[n] = p[i]; factor[n++] = -2.0;
    left[n] = p[i]; right[n] = p[i]; factor[n++] = 1.0;
  }
  left[n] = r; right[n] = r; factor[n++] = -1.0;
  left[n] = c; right[n] = c; factor[n++] = -1.0;
  left[n] = g; right[n] = g; factor[n++] = -0.25;
  left[n] = r; right[n] = c; factor[n++] = -2.0;
  left[n] = r; right[n] = g; factor[n++] = -1.0;
  left[n] = c; right[n] = g; factor[n++] = -1.0;
  if (rlo != 0.0)
  {
    left[n] = rlo; right[n] = rlo; factor[n++] = -1.0;
    left[n] = r; right[n] = rlo; factor[n++] = -2.0;
    left[n] = rlo; right[n] = c; factor[n++] = -2.0;
    left[n] = rlo; right[n] = g; factor[n++] = -1.0;
  }
  return square_products_sign(left, right, factor, n);
}

/**
 * @brief Return the sign of the cross product of the vectors B - A and D - C,
 * read on #DyadicWide for any finite coordinates
 * @details The four differences are exact, and the sign is that of
 * `(bx - ax)(dy - cy) - (by - ay)(dx - cx)`, a comparison of two products
 * (#dyadic_wide_product_cmp), as #segment_interior_offset_sign reads its
 * signs. #cross_product_sign_exact calls it where its doubles cannot hold the
 * products exactly
 */
static int
cross_product_sign_wide(double ax, double ay, double bx, double by,
  double cx, double cy, double dx, double dy)
{
  const double ends[4][2] = {{bx, ax}, {dy, cy}, {by, ay}, {dx, cx}};
  DyadicWideArena arena, *ar = &arena;
  dyadic_wide_arena_init(ar);
  DyadicWide *diff[4];
  for (int i = 0; i < 4; i++)
  {
    diff[i] = dyadic_wide_sub(ar, dyadic_wide_of_double(ar, ends[i][0]),
      dyadic_wide_of_double(ar, ends[i][1]));
  }
  int sign = dyadic_wide_product_cmp(ar, diff[0], diff[1], diff[2], diff[3]);
  dyadic_wide_arena_free(ar);
  return sign;
}

/*****************************************************************************
 * The nearest double of a value read exactly
 *****************************************************************************/

/* The side of the value, negated where it is negative, against the upper
 * midpoint of the double of bit pattern @p bits, 2^1024 standing for the
 * double after the largest one */
static int
nearest_exact_side(uint64 bits, void *arg)
{
  NearestExactSide *e = (NearestExactSide *) arg;
  DyadicWideArena *ar = e->ar;
  double lo;
  memcpy(&lo, &bits, sizeof(lo));
  DyadicWideMark m = dyadic_wide_arena_mark(ar);
  DyadicWide *up;
  if (bits + 1 == NEAREST_INF_BITS)
  {
    up = dyadic_wide_of_int64(ar, 1);
    up->exp = 1024;
  }
  else
  {
    double u;
    uint64 b2 = bits + 1;
    memcpy(&u, &b2, sizeof(u));
    up = dyadic_wide_of_double(ar, u);
  }
  DyadicWide *mid = dyadic_wide_add(ar, dyadic_wide_of_double(ar, lo), up);
  if (mid->sign != 0)
    mid->exp -= 1;
  /* -v beyond mid is v below -mid */
  if (e->neg)
    mid->sign = - mid->sign;
  int s = e->side(ar, mid, e->arg);
  dyadic_wide_arena_release(ar, m);
  return e->neg ? - s : s;
}

/* The double nearest a value that @p side reads exactly, its values taken
 * from @p ar, which is returned to where it was */
static double
nearest_double_in(DyadicWideArena *ar, double c, NearestSide side,
  const void *arg)
{
  DyadicWideMark m = dyadic_wide_arena_mark(ar);
  int s0 = side(ar, dyadic_wide_zero(ar), arg);
  dyadic_wide_arena_release(ar, m);
  if (s0 == 0)
    return 0.0;
  NearestExactSide e = {ar, side, arg, s0 < 0};
  c = fabs(c);
  if (isnan(c))
    c = DBL_MAX;
  uint64 bits, found = 0;
  memcpy(&bits, &c, sizeof(bits));
  /* An exact side always answers */
  (void) nearest_double_search(bits, &nearest_exact_side, &e, 0, &found);
  double result;
  memcpy(&result, &found, sizeof(result));
  return e.neg ? - result : result;
}

double
nearest_double_exact(double c, NearestSide side, const void *arg)
{
  DyadicWideArena arena;
  dyadic_wide_arena_init(&arena);
  double result = nearest_double_in(&arena, c, side, arg);
  dyadic_wide_arena_free(&arena);
  return result;
}

/* The sign of `sqrt(n / d) - mid`, that of `n - mid^2 d` for `mid >= 0` */
static int
nearest_root_side(DyadicWideArena *ar, const DyadicWide *mid, const void *arg)
{
  const NearestRoot *q = (const NearestRoot *) arg;
  return dyadic_wide_cmp(q->n, dyadic_wide_mul(ar, dyadic_wide_mul(ar, mid,
    mid), q->d));
}

double
dyadic_wide_nearest_root(DyadicWideArena *ar, const DyadicWide *n,
  const DyadicWide *d)
{
  if (n->sign <= 0)
    return 0.0;
  int en, ed;
  double mn = dyadic_wide_frexp(n, &en), md = dyadic_wide_frexp(d, &ed);
  double ratio = mn / md;
  int ex = en - ed;
  if (ex % 2 != 0)
  {
    ratio *= 2.0;
    ex -= 1;
  }
  double c = ldexp(sqrt(ratio), ex / 2);
  if (! (c > 0.0))
    c = 0.0;
  NearestRoot q = {n, d};
  return nearest_double_in(ar, c, &nearest_root_side, &q);
}

/* The sign of `n / d - mid`, that of `n - mid d` */
static int
nearest_quotient_side(DyadicWideArena *ar, const DyadicWide *mid,
  const void *arg)
{
  const NearestQuotient *q = (const NearestQuotient *) arg;
  return dyadic_wide_sub(ar, q->n, dyadic_wide_mul(ar, mid, q->d))->sign;
}

/* The sign of `c + b / sqrt(s) - mid`, that of `b - a sqrt(s)` with
 * `a = mid - c`, read on squares where both sides have one sign */
static int
nearest_offset_root_side(DyadicWideArena *ar, const DyadicWide *mid,
  const void *arg)
{
  const NearestOffsetRoot *v = (const NearestOffsetRoot *) arg;
  const DyadicWide *a = dyadic_wide_sub(ar, mid, v->c);
  int sa = a->sign, sb = v->b->sign;
  if (sa <= 0 && sb >= 0)
    return (sa == 0 && sb == 0) ? 0 : 1;
  if (sa >= 0 && sb <= 0)
    return -1;
  /* Both of one sign: compare b^2 with a^2 s */
  DyadicWide *diff = dyadic_wide_sub(ar, dyadic_wide_mul(ar, v->b, v->b),
    dyadic_wide_mul(ar, dyadic_wide_mul(ar, a, a), v->s));
  return (sb > 0) ? diff->sign : - diff->sign;
}

/* The double nearest `n / d`, d > 0 */
double
dyadic_wide_nearest_quotient(const DyadicWide *n, const DyadicWide *d)
{
  int en, ed;
  double mn = dyadic_wide_frexp(n, &en), md = dyadic_wide_frexp(d, &ed);
  double c = ldexp(mn / md, en - ed);
  NearestQuotient q = {n, d};
  return nearest_double_exact(c, &nearest_quotient_side, &q);
}

/* The double nearest `c + b / sqrt(s)`, s > 0 */
double
dyadic_wide_nearest_offset_root(DyadicWideArena *ar, double c,
  const DyadicWide *b, const DyadicWide *s)
{
  int eb, es;
  double mb = dyadic_wide_frexp(b, &eb), ms = dyadic_wide_frexp(s, &es);
  if (es % 2 != 0)
  {
    ms *= 2.0;
    es -= 1;
  }
  double guess = c + b->sign * ldexp(mb / sqrt(ms), eb - es / 2);
  NearestOffsetRoot v = {dyadic_wide_of_double(ar, c), b, s};
  return nearest_double_exact(guess, &nearest_offset_root_side, &v);
}

/**
 * @brief Return the sign of the squared distance of a point to the line of a
 * segment, times the squared length of the segment, less the square of
 * `r + c + g/2` times that squared length, computed exactly for any finite
 * doubles
 * @details This is the side of a midpoint #point_segment_distance_offset_exact
 * decides, `|w x u|^2 - m^2 (u . u)` with `w = p - a`, `u = b - a` and
 * `m = r + c + g/2`, read on the input coordinates where scaling them would
 * round a value or leave a product of four outside the doubles. By Lagrange's
 * identity `|w x u|^2 = (w . w)(u . u) - (w . u)^2`, in two coordinates as in
 * three, so the value is `(u . u)(w . w - m^2) - (w . u)^2`. The squared
 * length `u . u`, the difference `w . w - m^2` and the dot product `w . u`
 * are each a sum of products of two input doubles, which
 * #square_products_wide forms as #point_distance_offset_side_sign sets its
 * terms, and the two products are compared on their exact values
 * (#dyadic_wide_product_cmp)
 * @param[in] p,a,b The point and the two ends of the segment
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r Radius
 * @param[in] c Candidate
 * @param[in] g Gap from the candidate to its upper neighbour
 */
int
point_segment_side_sign_inputs(const double *p, const double *a,
  const double *b, int ndims, double r, double c, double g)
{
  /* At most 15 products, each of a weight from 1/4 to 2 */
  double left[15], right[15], factor[15];
  DyadicWideArena ar;
  dyadic_wide_arena_init(&ar);
  /* w . w - m^2 */
  int n = 0;
  for (int i = 0; i < ndims; i++)
  {
    left[n] = p[i]; right[n] = p[i]; factor[n++] = 1.0;
    left[n] = p[i]; right[n] = a[i]; factor[n++] = -2.0;
    left[n] = a[i]; right[n] = a[i]; factor[n++] = 1.0;
  }
  left[n] = r; right[n] = r; factor[n++] = -1.0;
  left[n] = c; right[n] = c; factor[n++] = -1.0;
  left[n] = g; right[n] = g; factor[n++] = -0.25;
  left[n] = r; right[n] = c; factor[n++] = -2.0;
  left[n] = r; right[n] = g; factor[n++] = -1.0;
  left[n] = c; right[n] = g; factor[n++] = -1.0;
  const DyadicWide *wm = square_products_wide(&ar, left, right, factor, n);
  /* u . u */
  n = 0;
  for (int i = 0; i < ndims; i++)
  {
    left[n] = b[i]; right[n] = b[i]; factor[n++] = 1.0;
    left[n] = b[i]; right[n] = a[i]; factor[n++] = -2.0;
    left[n] = a[i]; right[n] = a[i]; factor[n++] = 1.0;
  }
  const DyadicWide *len = square_products_wide(&ar, left, right, factor, n);
  /* w . u */
  n = 0;
  for (int i = 0; i < ndims; i++)
  {
    left[n] = p[i]; right[n] = b[i]; factor[n++] = 1.0;
    left[n] = p[i]; right[n] = a[i]; factor[n++] = -1.0;
    left[n] = a[i]; right[n] = b[i]; factor[n++] = -1.0;
    left[n] = a[i]; right[n] = a[i]; factor[n++] = 1.0;
  }
  const DyadicWide *dot = square_products_wide(&ar, left, right, factor, n);
  /* The square of the dot product is not negative; the other product carries
   * the signs of its factors (#dyadic_wide_product_cmp) */
  int sign = dyadic_wide_product_cmp(&ar, len, wm, dot, dot);
  dyadic_wide_arena_free(&ar);
  return sign;
}

/**
 * @brief Return the sign of `(p - s) . (b - a)`, computed exactly for any
 * finite doubles
 * @details A sum of products of two input doubles (#square_products_sign):
 * with `s = a` it is the sign of `w . u`, with `s = b` that of
 * `w . u - u . u`, which decide whether the nearest point of the segment is an
 * end. As #ring_area_sign, the product taken in floating point, on the rounded
 * differences, answers wherever it stands clear of the bound its rounding
 * reaches: each difference is within a half unit in its last place, and the
 * products and their sum add as many of the running magnitude, which twice
 * DBL_EPSILON of it bounds generously, and DBL_MIN covers an underflow. A
 * difference rounds to 0 only where it is 0, so a product of one is exactly 0.
 * Within the bound, the sign is that of the exact sum
 */
int
point_segment_dot_sign_inputs(const double *p, const double *s,
  const double *a, const double *b, int ndims)
{
  bool zero = true;
  double dot = 0.0, mag = 0.0;
  for (int i = 0; i < ndims; i++)
  {
    double w = p[i] - s[i], v = b[i] - a[i];
    if (w != 0.0 && v != 0.0)
      zero = false;
    double t = w * v;
    dot += t;
    mag += fabs(t);
  }
  if (zero)
    return 0;
  double bound = 4.0 * DBL_EPSILON * mag + DBL_MIN;
  if (isfinite(bound))
  {
    if (dot > bound)
      return 1;
    if (dot < - bound)
      return -1;
  }
  double left[12] = {0.0}, right[12] = {0.0}, factor[12] = {0.0};
  int n = 0;
  for (int i = 0; i < ndims; i++)
  {
    left[n] = p[i]; right[n] = b[i]; factor[n++] = 1.0;
    left[n] = p[i]; right[n] = a[i]; factor[n++] = -1.0;
    left[n] = s[i]; right[n] = b[i]; factor[n++] = -1.0;
    left[n] = s[i]; right[n] = a[i]; factor[n++] = 1.0;
  }
  return square_products_sign(left, right, factor, n);
}

/**
 * @brief Set the triple product `(c - a) . ((b - a) x (d - c))` computed
 * exactly from the coordinates and rounded to two doubles
 * @details Each component of the cross product is the cross product of the two
 * vectors in a plane of two coordinates, whose exact expansion
 * #cross_product_exact sums; scaled by the two doubles of each difference
 * (#scale_expansion), added (#expansion_sum) and compressed
 * (#compress_expansion), the triple product keeps its precision relative to
 * itself where the segments nearly meet and the products of the rounded
 * differences of #ddb_cross3 cancel. The two largest components of the
 * compressed expansion, summed exactly, hold it to about twice the precision
 * of a double; an exact zero is held as one
 */
void
ddb_triple3_points(const double *a, const double *b, const double *c,
  const double *d, DDBound *r)
{
  static const int plane[3][2] = {{1, 2}, {2, 0}, {0, 1}};
  double total[512], tmp[512], part[128], comp[512];
  int tlen = 0;
  for (int k = 0; k < 3; k++)
  {
    int i = plane[k][0], j = plane[k][1];
    double buf1[32], buf2[32];
    const double *n;
    int nlen = cross_product_expansion(a[i], a[j], b[i], b[j], c[i], c[j],
      d[i], d[j], buf1, buf2, &n);
    if (nlen == 0)
      continue;
    double eh, el;
    two_diff(c[k], a[k], &eh, &el);
    const double ev[2] = {eh, el};
    for (int h = 0; h < 2; h++)
    {
      if (ev[h] == 0.0)
        continue;
      int plen = scale_expansion(nlen, n, ev[h], part);
      if (tlen == 0)
      {
        memcpy(total, part, (size_t) plen * sizeof(double));
        tlen = plen;
      }
      else
        tlen = expansion_sum(tlen, total, plen, part, total, tmp);
    }
  }
  int clen = (tlen == 0) ? 0 : compress_expansion(tlen, total, comp);
  while (clen > 0 && comp[clen - 1] == 0.0)
    clen--;
  /* The expansions are exact where no product of three differences
   * underflows, which a difference below 2^-320 may make it do: there the
   * value holds the bound of an underflow, and no exact zero */
  double least = INFINITY;
  const double *from[3] = {a, c, a}, *to[3] = {b, d, c};
  for (int w = 0; w < 3; w++)
    for (int k = 0; k < 3; k++)
    {
      double dh, dl;
      two_diff(to[w][k], from[w][k], &dh, &dl);
      if (dh != 0.0)
        least = fmin(least, fabs(dh));
      if (dl != 0.0)
        least = fmin(least, fabs(dl));
    }
  double under = (least < 0x1p-320) ? DDB_TINY : 0.0;
  if (clen == 0)
  {
    r->hi = r->lo = 0.0;
    r->e = under;
    return;
  }
  if (clen == 1)
  {
    r->hi = comp[0];
    r->lo = 0.0;
    r->e = under;
    return;
  }
  double rest = 0.0;
  for (int i = 0; i < clen - 2; i++)
    rest += comp[i];
  two_sum(comp[clen - 1], comp[clen - 2], &r->hi, &r->lo);
  r->lo += rest;
  r->e = ((clen == 2) ? 0.0 : DDB_EPS * fabs(r->hi) + DDB_TINY) + under;
  return;
}

/**
 * @brief The side of the root of `n / d` against the upper midpoint of the
 * double of bit pattern @p bits, or 2 where the bounds cannot tell
 * @details With `c` that double and `gu` the gap to its upper neighbour, the
 * root passes the midpoint `c + gu / 2` exactly where the residual
 * `n - c^2 d`, formed on two doubles with a bound on its error (#ddb_mul,
 * #ddb_sub), exceeds `d gu (c + gu / 4)`. The upper midpoint of the lower
 * neighbour of the candidate read last is that candidate's lower midpoint,
 * passed where its residual exceeds `- d gd (c - gd / 4)`, `gd` the gap below
 * it, so its residual serves again
 */
static pg_attribute_always_inline int
ddb_nearest_root_side(uint64 bits, void *arg)
{
  DDBRootSide *f = (DDBRootSide *) arg;
  double gap;
  if (bits + 1 == f->cbits)
  {
    gap = - f->gd * (f->c - 0.25 * f->gd) * f->dv;
  }
  else
  {
    double c, cu, cd;
    uint64 ubits = bits + 1, dbits = bits - 1;
    memcpy(&c, &bits, sizeof(c));
    if (! (c > 0.0 && c < DBL_MAX))
      return 2;
    memcpy(&cu, &ubits, sizeof(cu));
    memcpy(&cd, &dbits, sizeof(cd));
    if (! (cd > 0.0))
      return 2;
    /* The residual n - c^2 d and a bound on its error */
    DDBound cc;
    two_product(c, c, &cc.hi, &cc.lo);
    cc.e = (fabs(cc.hi) < DDB_EXACT_PRODUCT) ? DDB_TINY : 0.0;
    DDBound r = ddb_sub(f->n, ddb_mul(cc, f->d));
    if (! isfinite(r.hi) || ! isfinite(r.lo) || ! isfinite(r.e))
      return 2;
    f->cbits = bits;
    f->c = c;
    f->gd = c - cd;
    f->rv = r.hi + r.lo;
    f->rerr = r.e + DBL_EPSILON * fabs(f->rv);
    double gu = cu - c;
    gap = gu * (c + 0.25 * gu) * f->dv;
  }
  /* The sign of the residual less the gap, its bound the error of the
   * residual and 32 units of rounding of both magnitudes, for the rounding of
   * the gap, of `d` and of the difference, as
   * #point_distance_offset_filter_sign bounds its value */
  double value = f->rv - gap;
  double bound = f->rerr + 32.0 * DBL_EPSILON * (fabs(f->rv) + fabs(gap));
  if (value > bound)
    return 1;
  if (value < - bound)
    return -1;
  return 2;
}

/**
 * @brief Return true and write in the last argument the double nearest the
 * root of `n / d`, `d` positive, where the bounds place the root strictly
 * inside the rounding cell of one double
 * @details The residual search of #point_distance_nearest, with `d` scaling
 * the gaps: #nearest_double_search reads the sides #ddb_nearest_root_side
 * forms on residuals bounded in two doubles from the candidate
 * `sqrt(n / d)` in doubles, which rounds twice and so may be a neighbour of
 * the answer, asking at most #NEAREST_FILTER_BUDGET of them. A tie, or a
 * root the bounds cannot place, decides nothing
 */
bool
ddb_nearest_root(DDBound n, DDBound d, double *result)
{
  if (ddb_sign(d) <= 0)
    return false;
  /* An exact zero is the root of zero, which is zero */
  if (ddb_is_zero(n))
  {
    *result = 0.0;
    return true;
  }
  if (ddb_sign(n) <= 0)
    return false;
  double dv = d.hi + d.lo;
  /* The bound of d must leave the gaps their meaning */
  if (! (d.e <= 0x1p-60 * dv))
    return false;
  double c = sqrt((n.hi + n.lo) / dv);
  if (! (c > 0.0 && c < DBL_MAX))
    return false;
  DDBRootSide f = {n, d, dv, NEAREST_INF_BITS, 0.0, 0.0, 0.0, 0.0};
  uint64 bits, found;
  memcpy(&bits, &c, sizeof(bits));
  if (! nearest_double_search(bits, &ddb_nearest_root_side, &f,
      NEAREST_FILTER_BUDGET, &found))
    return false;
  memcpy(result, &found, sizeof(double));
  return true;
}

/*****************************************************************************
 * The degrees of the longitude and the latitude of a vector of integers, in
 *****************************************************************************/

/* The least upper bound in doubles of the magnitude of a bounded value */
static double
ddb_mag_up(DDBound a)
{
  return (fabs(a.hi) + fabs(a.lo) + a.e) * DDB_UP;
}

/* A lower bound in doubles of a bounded value lying above its bound, or a
 * value that is not positive */
static double
ddb_low(DDBound a)
{
  return (a.hi - fabs(a.lo) - a.e) / DDB_UP;
}

/* Set @p r to the int64 @p x exactly, its rounding to a double and the
 * remainder, which takes at most 10 bits; false where |x| >= 2^62 */
static bool
ddb_set_int64(int64 x, DDBound *r)
{
  if (x >= (INT64CONST(1) << 62) || x <= - (INT64CONST(1) << 62))
    return false;
  r->hi = (double) x;
  r->lo = (double) (x - llrint(r->hi));
  r->e = 0.0;
  return true;
}

/* The magnitude of a bounded value */
static DDBound
ddb_abs(DDBound a)
{
  if (a.hi < 0.0)
  {
    a.hi = - a.hi;
    a.lo = - a.lo;
  }
  return a;
}

/**
 * @brief Set @p r to the quotient of two bounded values with its bound, or
 * return false where the divisor does not lie above its bound
 * @details The residual method of #ddb_nearest_root: the quotient in doubles
 * corrected by the residual `a - q b` is a pair of doubles `q`; its bound is
 * the residual of the values held, read again, over `b`, plus what the
 * bounds of the operands move the quotient,
 * `|A / B - a / b| <= (e_a + |a / b| e_b) / (b - e_b)`
 */
static bool
ddb_div(DDBound a, DDBound b, DDBound *r)
{
  double blow = ddb_low(b);
  if (! (blow > 0.0))
    return false;
  DDBound a0 = a, b0 = b;
  a0.e = b0.e = 0.0;
  double bv = b.hi + b.lo;
  DDBound q1;
  q1.hi = (a.hi + a.lo) / bv;
  q1.lo = q1.e = 0.0;
  DDBound res = ddb_sub(a0, ddb_mul(q1, b0));
  DDBound q;
  two_sum(q1.hi, (res.hi + res.lo) / bv, &q.hi, &q.lo);
  q.e = 0.0;
  DDBound res2 = ddb_sub(a0, ddb_mul(q, b0));
  double e1 = ddb_mag_up(res2) / blow;
  double e2 = (a.e + (fabs(q.hi) + fabs(q.lo) + e1) * DDB_UP * b.e) *
    DDB_UP / blow;
  q.e = (e1 + e2) * DDB_UP;
  if (! isfinite(q.hi) || ! isfinite(q.lo) || ! isfinite(q.e))
    return false;
  *r = q;
  return true;
}

/**
 * @brief Set @p r to the square root of a bounded value with its bound, or
 * return false where the value does not lie above its bound
 * @details The residual method of #ddb_div: the root in doubles corrected by
 * the residual `a - s^2` is a pair of doubles `s`, and
 * `|sqrt(a) - s| = |a - s^2| / (sqrt(a) + s) <= |a - s^2| / sqrt(a)`, plus
 * `|sqrt(A) - sqrt(a)| <= e_a / sqrt(a)`, sqrt(a) bounded below by the root
 * of the lower bound of the value
 */
static bool
ddb_sqrt(DDBound a, DDBound *r)
{
  double alow = ddb_low(a);
  if (! (alow > 0.0))
    return false;
  DDBound a0 = a;
  a0.e = 0.0;
  DDBound s1;
  s1.hi = sqrt(a.hi + a.lo);
  s1.lo = s1.e = 0.0;
  DDBound res = ddb_sub(a0, ddb_mul(s1, s1));
  DDBound s;
  two_sum(s1.hi, (res.hi + res.lo) / (2.0 * s1.hi), &s.hi, &s.lo);
  s.e = 0.0;
  DDBound res2 = ddb_sub(a0, ddb_mul(s, s));
  double slow = sqrt(alow) / DDB_UP;
  s.e = (ddb_mag_up(res2) + a.e * DDB_UP) * DDB_UP / slow;
  if (! isfinite(s.hi) || ! isfinite(s.lo) || ! isfinite(s.e))
    return false;
  *r = s;
  return true;
}

/**
 * @brief Return the quotient of a bounded value by a positive integer @p k
 * below 2^53, with its bound, the twin of #ddb_div for an exact divisor
 * @details The remainder of the quotient in doubles, `a.hi - q k`, is exact
 * by a fused multiply-add, so the second double `(rem + a.lo) / k` is the
 * rest of the quotient within twice its own rounding, below 2^-104 of `q`;
 * the residuals #ddb_div forms for any divisor are not needed
 */
static DDBound
ddb_div_int(DDBound a, int k)
{
  double dk = (double) k;
  double q = a.hi / dk;
  double rem = fma(- q, dk, a.hi);
  DDBound r;
  two_sum(q, (rem + a.lo) / dk, &r.hi, &r.lo);
  r.e = (a.e / dk + DDB_EPS * fabs(q)) * DDB_UP + DDB_TINY;
  return r;
}

/**
 * @brief Set @p r to the degrees of atan(t), the true t in [0, 1], with
 * their bound, or return false where a bound cannot be kept
 * @details The twin of #fix_atan_bound in bounded pairs of doubles: three
 * halvings t / (1 + sqrt(1 + t^2)), each halving the angle exactly, bring
 * t below tan(pi / 32) < 0.1, where the series of the arctangent alternates
 * with decreasing terms; it stops at a power of t below 2^-112, which
 * bounds the rest; the result is eight times the sum, times 180 / pi. Every
 * operation carries the bound of its operands (#ddb_mul, #ddb_div,
 * #ddb_div_int, #ddb_sqrt), so the bound is on the degrees of the true t
 */
static bool
ddb_atan_degrees(DDBound t, DDBound *r)
{
  DDBound one;
  one.hi = 1.0;
  one.lo = one.e = 0.0;
  for (int h = 0; h < 3; h++)
  {
    DDBound s, q;
    if (! ddb_sqrt(ddb_add(one, ddb_mul(t, t)), &s) ||
        ! ddb_div(t, ddb_add(one, s), &q))
      return false;
    t = q;
  }
  DDBound t2 = ddb_mul(t, t), p = t, sum = t;
  bool done = false;
  for (int k = 1; k < 40 && ! done; k++)
  {
    p = ddb_mul(p, t2);
    DDBound term = ddb_div_int(p, 2 * k + 1);
    sum = (k % 2) ? ddb_sub(sum, term) : ddb_add(sum, term);
    double pmag = ddb_mag_up(p);
    if (pmag < 0x1p-112)
    {
      /* The rest alternates and decreases, so it lies within its first
       * term, below p t^2 < p */
      sum.e = (sum.e + pmag) * DDB_UP;
      done = true;
    }
  }
  if (! done)
    return false;
  sum.hi *= 8.0;
  sum.lo *= 8.0;
  sum.e *= 8.0;
  DDBound deg;
  deg.hi = DDB_DEG_HI;
  deg.lo = DDB_DEG_LO;
  deg.e = DDB_DEG_ERR;
  *r = ddb_mul(sum, deg);
  return isfinite(r->hi) && isfinite(r->lo) && isfinite(r->e);
}

/**
 * @brief Set @p result to the double nearest the bounded positive value
 * @p a, signed by @p sign, where its bound places it strictly inside the
 * rounding cell of one double, and return false otherwise
 * @details The value is a pair normalised by #two_sum, so `hi` is the double
 * nearest `hi + lo`; the true value lies within `|lo| + e` of `hi`, and
 * where that is below half the smaller gap from `hi` to its neighbours, `hi`
 * is the double nearest it, the test of #ddb_nearest_root
 */
static bool
ddb_degrees_nearest(DDBound a, double sign, double *result)
{
  double c = a.hi;
  if (! (c > 0.0 && c < 360.0) || ! isfinite(a.lo) || ! isfinite(a.e))
    return false;
  uint64_t bits, ubits, dbits;
  memcpy(&bits, &c, sizeof(bits));
  ubits = bits + 1;
  dbits = bits - 1;
  double cu, cd;
  memcpy(&cu, &ubits, sizeof(cu));
  memcpy(&cd, &dbits, sizeof(cd));
  double half = 0.5 * Min(cu - c, c - cd);
  if (! ((fabs(a.lo) + a.e) * DDB_UP < half))
    return false;
  *result = sign * c;
  return true;
}

/**
 * @brief Set @p result to the double nearest the longitude in degrees of the
 * vector of integers @p v, atan2(y, x), x and y not 0, where bounded pairs
 * of doubles decide it, and return false where they cannot
 * @details atan2(|y|, x) is the degrees of atan(|y| / |x|), or 180 less them
 * for a negative x, where |y| < |x|, and 90 less or plus those of
 * atan(|x| / |y|) otherwise (#ddb_atan_degrees, #ddb_degrees_nearest)
 */
bool
ddb_vector_lon_nearest(const int64 *v, double *result)
{
  int64 x = v[0], y = v[1];
  DDBound X, Y;
  if (x == 0 || y == 0 || ! ddb_set_int64(x, &X) || ! ddb_set_int64(y, &Y))
    return false;
  X = ddb_abs(X);
  Y = ddb_abs(Y);
  uint64 ax = x < 0 ? (uint64) 0 - (uint64) x : (uint64) x;
  uint64 ay = y < 0 ? (uint64) 0 - (uint64) y : (uint64) y;
  bool low = ay < ax;
  DDBound t, deg, k;
  if (! ddb_div(low ? Y : X, low ? X : Y, &t) || ! ddb_atan_degrees(t, &deg))
    return false;
  k.lo = k.e = 0.0;
  if (low)
  {
    if (x < 0)
    {
      k.hi = 180.0;
      deg = ddb_sub(k, deg);
    }
  }
  else
  {
    k.hi = 90.0;
    deg = (x > 0) ? ddb_sub(k, deg) : ddb_add(k, deg);
  }
  return ddb_degrees_nearest(deg, y > 0 ? 1.0 : -1.0, result);
}

/**
 * @brief Set @p result to the double nearest the latitude in degrees of the
 * vector of integers @p v, atan2(z, sqrt(x^2 + y^2)), z and x^2 + y^2 not 0,
 * where bounded pairs of doubles decide it, and return false where they
 * cannot
 * @details With r = sqrt(x^2 + y^2), atan2(|z|, r) is the degrees of
 * atan(|z| / r) where |z| < r and 90 less those of atan(r / |z|) where |z|
 * > r, the side read from the sign of z^2 - r^2, which must be decided
 * (#ddb_atan_degrees, #ddb_degrees_nearest)
 */
bool
ddb_vector_lat_nearest(const int64 *v, double *result)
{
  int64 z = v[2];
  DDBound X, Y, Z, r, t, deg;
  if (z == 0 || ! ddb_set_int64(v[0], &X) || ! ddb_set_int64(v[1], &Y) ||
      ! ddb_set_int64(z, &Z))
    return false;
  Z = ddb_abs(Z);
  DDBound rr = ddb_add(ddb_mul(X, X), ddb_mul(Y, Y));
  int side = ddb_sign(ddb_sub(ddb_mul(Z, Z), rr));
  if (side == 0 || ! ddb_sqrt(rr, &r))
    return false;
  if (side < 0)
  {
    if (! ddb_div(Z, r, &t) || ! ddb_atan_degrees(t, &deg))
      return false;
  }
  else
  {
    DDBound k;
    if (! ddb_div(r, Z, &t) || ! ddb_atan_degrees(t, &deg))
      return false;
    k.hi = 90.0;
    k.lo = k.e = 0.0;
    deg = ddb_sub(k, deg);
  }
  return ddb_degrees_nearest(deg, z > 0 ? 1.0 : -1.0, result);
}

/**
 * @brief Set @p result to the double nearest the latitude in degrees of the
 * row @p r of the Web Mercator grid, atan(sinh(pi r)), 0 < |r| <= 1, where
 * bounded pairs of doubles decide it, and return false where they cannot
 * @details The latitude is 2 atan(tanh(t / 2)) for t = pi |r|, and
 * tanh(t / 2) = m / (m + 2) for m = e^t - 1, which keeps its relative
 * accuracy as t nears 0: m is the series of e^(t/8) - 1, every term positive
 * and the rest below the last term read, then three doublings of the angle,
 * m (2 + m) each (#ddb_atan_degrees, #ddb_degrees_nearest)
 */
bool
ddb_mercator_lat_nearest(double r, double *result)
{
  double ar = fabs(r);
  if (! (ar > 0x1p-900 && ar <= 1.0))
    return false;
  DDBound pi, x, two, m, p, t, deg;
  pi.hi = DDB_PI_HI;
  pi.lo = DDB_PI_LO;
  pi.e = DDB_PI_ERR;
  x.hi = ar / 8.0;
  x.lo = x.e = 0.0;
  /* u = pi |r| / 8, below 1/2 */
  DDBound u = ddb_mul(pi, x);
  two.hi = 2.0;
  two.lo = two.e = 0.0;
  m = p = u;
  bool done = false;
  for (int k = 2; k < 60 && ! done; k++)
  {
    p = ddb_div_int(ddb_mul(p, u), k);
    m = ddb_add(m, p);
    double pmag = ddb_mag_up(p);
    if (pmag < 0x1p-112 * ddb_low(m))
    {
      /* The terms left are positive and each at most a quarter of the one
       * before, so they sum to less than this one */
      m.e = (m.e + pmag) * DDB_UP;
      done = true;
    }
  }
  if (! done)
    return false;
  for (int i = 0; i < 3; i++)
    m = ddb_mul(m, ddb_add(two, m));
  if (! ddb_div(m, ddb_add(m, two), &t) || ! ddb_atan_degrees(t, &deg))
    return false;
  deg.hi *= 2.0;
  deg.lo *= 2.0;
  deg.e *= 2.0;
  return ddb_degrees_nearest(deg, r > 0 ? 1.0 : -1.0, result);
}

/**
 * @brief Return the sign of `(u . u)(w . w - r^2) - (w . u)^2` where its
 * evaluation in pairs of doubles decides it, and 2 otherwise
 * @details The value #point_segment_side_sign_inputs decides exactly, with
 * `w = p - a` and `u = b - a`, evaluated on pairs of doubles carrying a bound
 * on their error (#DDBound), as #plane_point_offset_sign reads its value ahead
 * of #DyadicWide and as #point_segment_offset_filter_sign filters the rounding
 * of the same quantity. A value whose bound leaves it on one side of zero has
 * that sign, and a value with no error that is zero is an exact zero
 * @param[in] p,a,b The point and the two ends of the segment
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r Radius, not negative
 * @return -1, 0 or 1, or 2 where the bound cannot tell
 */
static int
point_segment_side_sign_filter(const double *p, const double *a,
  const double *b, int ndims, double r)
{
  DDBound u[3], w[3], rr;
  for (int k = 0; k < 3; k++)
  {
    if (k < ndims)
    {
      two_diff(b[k], a[k], &u[k].hi, &u[k].lo);
      two_diff(p[k], a[k], &w[k].hi, &w[k].lo);
    }
    else
      u[k].hi = u[k].lo = w[k].hi = w[k].lo = 0.0;
    u[k].e = w[k].e = 0.0;
  }
  rr.hi = r; rr.lo = 0.0; rr.e = 0.0;
  DDBound uu = ddb_dot3(u, u), ww = ddb_dot3(w, w), wu = ddb_dot3(w, u);
  DDBound val = ddb_sub(ddb_mul(uu, ddb_sub(ww, ddb_mul(rr, rr))),
    ddb_mul(wu, wu));
  if (ddb_is_zero(val))
    return 0;
  int sign = ddb_sign(val);
  return (sign != 0) ? sign : 2;
}

/**
 * @brief Return the sign of the distance of a point to a segment less a
 * radius, computed exactly for any finite doubles
 * @details The sign #point_segment_distance_offset_exact rounds its answer
 * from, with no rounding: the nearest point of the segment is an end where
 * #point_segment_dot_sign_inputs says so, and the sign is then that of the
 * squared distance of the two points less the squared radius
 * (#point_distance_offset_side_sign); otherwise it is that of
 * `(w x u)^2 - r^2 (u . u)`, read first in pairs of doubles
 * (#point_segment_side_sign_filter) and exactly where they cannot tell
 * (#point_segment_side_sign_inputs). Each is a sum of products of the input
 * doubles decided exactly, so the answer is 0 exactly where the disc of the
 * radius is tangent to the segment
 * @param[in] p,a,b The point and the two ends of the segment
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r Radius, not negative
 * @return -1, 0 or 1
 */
int
point_segment_offset_sign(const double *p, const double *a, const double *b,
  int ndims, double r)
{
  assert(ndims == 2 || ndims == 3);
  assert(r >= 0.0);
  if (point_segment_dot_sign_inputs(p, a, a, b, ndims) <= 0)
    return point_distance_offset_side_sign(p, a, ndims, r, 0.0, 0.0);
  if (point_segment_dot_sign_inputs(p, b, a, b, ndims) >= 0)
    return point_distance_offset_side_sign(p, b, ndims, r, 0.0, 0.0);
  int sign = point_segment_side_sign_filter(p, a, b, ndims, r);
  if (sign != 2)
    return sign;
  return point_segment_side_sign_inputs(p, a, b, ndims, r, 0.0, 0.0);
}

/**
 * @brief Return the sign of `x / y - u / v` for two squares of
 * #segment_distance_square, the sign of `x v - u y` since both denominators
 * are positive
 * @details Read on the exact sums @p sums (#polynomial_sign_exact)
 */
int
segment_square_cmp(const ExactSum *sums, const SegmentSquare *s1,
  const SegmentSquare *s2)
{
  PolyTerm terms[54];
  int n = 0;
  for (int side = 0; side < 2; side++)
  {
    const SegmentSquare *x = side ? s2 : s1, *y = side ? s1 : s2;
    for (int i = 0; i < x->nnum; i++)
      for (int j = 0; j < y->nden; j++)
      {
        PolyTerm *t = &terms[n++];
        t->coef = (side ? -1.0 : 1.0) * x->num[i].coef * y->den[j].coef;
        t->deg = x->num[i].deg + y->den[j].deg;
        memcpy(t->sum, x->num[i].sum, sizeof(int) * x->num[i].deg);
        memcpy(t->sum + x->num[i].deg, y->den[j].sum,
          sizeof(int) * y->den[j].deg);
      }
  }
  return polynomial_sign_exact(sums, terms, n);
}

/*****************************************************************************
 * The closest parameters of two segments, ported from Geometric Tools
 *****************************************************************************/

/**
 * @brief Set the three coordinates of `q - p`, exactly, in @p ar
 */
static void
wide_diff3(DyadicWideArena *ar, const double *q, const double *p,
  DyadicWide **r)
{
  for (int k = 0; k < 3; k++)
    r[k] = dyadic_wide_sub(ar, dyadic_wide_of_double(ar, q[k]),
      dyadic_wide_of_double(ar, p[k]));
  return;
}

/**
 * @brief Set the cross product of two vectors of three #DyadicWide, exactly,
 * in @p ar
 */
void
wide_cross3(DyadicWideArena *ar, DyadicWide *const *u, DyadicWide *const *v,
  DyadicWide **r)
{
  for (int k = 0; k < 3; k++)
  {
    int i = (k + 1) % 3, j = (k + 2) % 3;
    r[k] = dyadic_wide_sub(ar, dyadic_wide_mul(ar, u[i], v[j]),
      dyadic_wide_mul(ar, u[j], v[i]));
  }
  return;
}

/**
 * @brief Return the dot product of two vectors of three #DyadicWide, exactly,
 * in @p ar
 */
DyadicWide *
wide_dot3(DyadicWideArena *ar, DyadicWide *const *u, DyadicWide *const *v)
{
  DyadicWide *r = dyadic_wide_mul(ar, u[0], v[0]);
  for (int k = 1; k < 3; k++)
    r = dyadic_wide_add(ar, r, dyadic_wide_mul(ar, u[k], v[k]));
  return r;
}

/**
 * @brief Return true where the nearest points of two segments in 3D lie
 * inside both, setting in @p en and @p nn the values `e . n` and `n . n`, read
 * on #DyadicWide in @p ar for any finite coordinates
 * @details With `u = b - a`, `v = d - c`, `e = c - a` and `n = u x v`, the
 * nearest points lie inside both where `n . n`, `(e x v) . n` and
 * `(e x u) . n` have the signs #SEGMENT_INSIDE reads
 */
bool
segment_interior_wide_values(DyadicWideArena *ar, const double *a,
  const double *b, const double *c, const double *d, DyadicWide **en,
  DyadicWide **nn)
{
  DyadicWide *u[3], *v[3], *e[3], *n[3], *ev[3], *eu[3];
  wide_diff3(ar, b, a, u);
  wide_diff3(ar, d, c, v);
  wide_diff3(ar, c, a, e);
  wide_cross3(ar, u, v, n);
  wide_cross3(ar, e, v, ev);
  wide_cross3(ar, e, u, eu);
  const DyadicWide *ds = wide_dot3(ar, ev, n), *dt = wide_dot3(ar, eu, n);
  DyadicWide *dnn = wide_dot3(ar, n, n);
  int ss1 = dyadic_wide_cmp(ds, dnn), st1 = dyadic_wide_cmp(dt, dnn);
  if (! SEGMENT_INSIDE(dnn->sign, ds->sign, ss1, dt->sign, st1))
    return false;
  *nn = dnn;
  *en = wide_dot3(ar, n, e);
  return true;
}

/**
 * @brief Return the sign #segment_interior_offset_sign answers where its
 * evaluation in doubles, then in pairs of doubles, decides it, and 2 otherwise
 * @details The signs placing the nearest points are read first in doubles
 * (#dbl_cross3), one known on the wrong side answering 1. The values #segment_interior_wide_values forms on #DyadicWide,
 * with `u = b - a`, `v = d - c`, `e = c - a` and `n = u x v`, formed on pairs
 * of doubles carrying a bound on their error (#DDBound), as
 * #plane_point_offset_sign reads its value ahead of #DyadicWide. The nearest
 * points lie outside a segment, the answer 1, where one of the signs that
 * place them is known on the wrong side (#SEGMENT_INSIDE); they lie inside
 * both where all are known on the right side, and the answer is then the sign
 * of `(e . n)^2 - r^2 (n . n)` where its bound decides it. A value with no
 * error that is zero is an exact zero (#ddb_sign_known)
 * @param[in] a,b The two ends of the first segment, three coordinates each
 * @param[in] c,d The two ends of the second segment, three coordinates each
 * @param[in] r Radius, not negative
 * @return -1, 0 or 1, or 2 where the bounds cannot tell
 */
static int
segment_interior_offset_filter(const double *a, const double *b,
  const double *c, const double *d, double r)
{
  /* In doubles first: each component of the three cross products lies within
   * 2^-50 of the sum of the magnitudes of its products (#dbl_cross3), so each
   * dot product of two of them lies within 2^-48 of the sum of the products of
   * those sums, and a difference of two within 2^-47 of the sum of theirs; a
   * sign they place on the wrong side is the answer 1, where every sum lies
   * inside 2^-900 and 2^900 */
  {
    double xn[3], mn[3], xev[3], mev[3], xeu[3], meu[3];
    dbl_cross3(a, b, c, d, xn, mn);
    dbl_cross3(a, c, c, d, xev, mev);
    dbl_cross3(a, c, a, b, xeu, meu);
    double nnd = 0.0, sd = 0.0, td = 0.0, bnn = 0.0, bs = 0.0, bt = 0.0;
    for (int k = 0; k < 3; k++)
    {
      nnd += xn[k] * xn[k];
      sd += xev[k] * xn[k];
      td += xeu[k] * xn[k];
      bnn += mn[k] * mn[k];
      bs += mev[k] * mn[k];
      bt += meu[k] * mn[k];
    }
    if (bnn >= 0x1p-900 && bnn <= 0x1p900 && bs >= 0x1p-900 &&
        bs <= 0x1p900 && bt >= 0x1p-900 && bt <= 0x1p900)
    {
      const double vals[4] = {sd, sd - nnd, td, td - nnd};
      const double bnds[4] = {0x1p-48 * bs, 0x1p-47 * (bs + bnn), 0x1p-48 * bt,
        0x1p-47 * (bt + bnn)};
      /* The sign each must have for the nearest points to lie inside both */
      static const int inside[4] = {1, -1, 1, -1};
      for (int k = 0; k < 4; k++)
        if ((inside[k] > 0 && vals[k] < - bnds[k]) ||
            (inside[k] < 0 && vals[k] > bnds[k]))
          return 1;
    }
  }
  DDBound u[3], v[3], e[3], n[3], ev[3], eu[3], rr;
  ddb_diff3(b, a, u);
  ddb_diff3(d, c, v);
  ddb_diff3(c, a, e);
  ddb_cross3(u, v, n);
  ddb_cross3(e, v, ev);
  ddb_cross3(e, u, eu);
  DDBound nn = ddb_dot3(n, n), s = ddb_dot3(ev, n), t = ddb_dot3(eu, n);
  const int q[5] = {ddb_sign_known(nn), ddb_sign_known(s),
    ddb_sign_known(ddb_sub(s, nn)), ddb_sign_known(t),
    ddb_sign_known(ddb_sub(t, nn))};
  /* The sign each must have for the nearest points to lie inside both */
  static const int inside[5] = {1, 1, -1, 1, -1};
  bool known = true;
  for (int k = 0; k < 5; k++)
  {
    if (q[k] == 2)
      known = false;
    else if (q[k] != inside[k])
      return 1;
  }
  if (! known)
    return 2;
  rr.hi = r; rr.lo = 0.0; rr.e = 0.0;
  DDBound en = ddb_dot3(e, n);
  return ddb_sign_known(ddb_sub(ddb_mul(en, en),
    ddb_mul(ddb_mul(rr, rr), nn)));
}

/**
 * @brief Return the sign of the distance of two segments in 3D less a radius
 * where their nearest points lie inside both, computed exactly for any finite
 * doubles, and 1 otherwise
 * @details The sign of `(e . n)^2 - r^2 (n . n)`, as #nearest_lines_side reads
 * it for a midpoint, read first in pairs of doubles
 * (#segment_interior_offset_filter) and on #DyadicWide where they cannot tell
 * (#segment_interior_wide_values). Where the nearest points do not lie inside
 * both, an end of one of the segments is the nearest, which
 * #point_segment_offset_sign answers
 * @param[in] a,b The two ends of the first segment, three coordinates each
 * @param[in] c,d The two ends of the second segment, three coordinates each
 * @param[in] r Radius, not negative
 * @return -1, 0 or 1
 */
int
segment_interior_offset_sign(const double *a, const double *b, const double *c,
  const double *d, double r)
{
  assert(r >= 0.0);
  int sign = segment_interior_offset_filter(a, b, c, d, r);
  if (sign != 2)
    return sign;
  DyadicWideArena ar;
  dyadic_wide_arena_init(&ar);
  DyadicWide *en, *nn;
  if (! segment_interior_wide_values(&ar, a, b, c, d, &en, &nn))
    sign = 1;
  else
  {
    const DyadicWide *wr = dyadic_wide_of_double(&ar, r);
    sign = dyadic_wide_product_cmp(&ar, en, en, dyadic_wide_mul(&ar, wr, wr),
      nn);
  }
  dyadic_wide_arena_free(&ar);
  return sign;
}

/**
 * @brief Return the sign of the determinant of three vectors in 3D, each the
 * difference of two points, computed exactly for any finite doubles
 * @details The determinant `(a1 - a0) . ((b1 - b0) x (c1 - c0))`, of degree
 * three, is first evaluated in doubles with a bound on its error; where that
 * cannot tell its sign, in expansions where every difference is a double held
 * exactly (#triple_product_sign_exact), otherwise in pairs of doubles with a
 * bound on their error (#DDBound), and where that cannot tell, on
 * #DyadicWide. With
 * `a0 = b0 = c0` it is the orientation of four points: 0 exactly where they
 * are coplanar
 * @param[in] a0,a1,b0,b1,c0,c1 Points, three coordinates each
 * @return -1, 0 or 1
 */
int
vector_triple_sign(const double *a0, const double *a1, const double *b0,
  const double *b1, const double *c0, const double *c1)
{
  /* In doubles first, as #triple_product_sign filters a determinant of exact
   * coordinates by Shewchuk's bound: of differences each rounded once, it
   * lies within 2^-48 of its permanent, the sum of the magnitudes of its six
   * products, where none of them leaves the normal doubles, inside 2^-900
   * and 2^900 */
  {
    double u0 = a1[0] - a0[0], u1 = a1[1] - a0[1], u2 = a1[2] - a0[2];
    double v0 = b1[0] - b0[0], v1 = b1[1] - b0[1], v2 = b1[2] - b0[2];
    double w0 = c1[0] - c0[0], w1 = c1[1] - c0[1], w2 = c1[2] - c0[2];
    double det = u0 * (v1 * w2 - v2 * w1) + u1 * (v2 * w0 - v0 * w2) +
      u2 * (v0 * w1 - v1 * w0);
    double perm = fabs(u0) * (fabs(v1 * w2) + fabs(v2 * w1)) +
      fabs(u1) * (fabs(v2 * w0) + fabs(v0 * w2)) +
      fabs(u2) * (fabs(v0 * w1) + fabs(v1 * w0));
    if (perm >= 0x1p-900 && perm <= 0x1p900 && fabs(det) > 0x1p-48 * perm)
      return (det > 0.0) ? 1 : -1;
  }
  DDBound u[3], v[3], w[3], x[3];
  ddb_diff3(a1, a0, u);
  ddb_diff3(b1, b0, v);
  ddb_diff3(c1, c0, w);
  /* Where every difference is a double held exactly, each one zero or within
   * 2^-200 and 2^200, it is a multiple of 2^-252, so every product of three of
   * them and every part of its exact expansion is a multiple of 2^-756 below
   * 2^600: no step leaves the normal doubles, and the determinant of those
   * doubles is exact in expansions (#triple_product_sign_exact), as the
   * orientation of a vertex of a face lying in a plane needs where its
   * coordinates sit on a common grid */
  bool doubles = true;
  const DDBound *vec[3] = {u, v, w};
  for (int i = 0; i < 3 && doubles; i++)
    for (int k = 0; k < 3 && doubles; k++)
    {
      double m = fabs(vec[i][k].hi);
      doubles = vec[i][k].lo == 0.0 &&
        (m == 0.0 || (m >= 0x1p-200 && m <= 0x1p200));
    }
  if (doubles)
  {
    POINT3D pu = {u[0].hi, u[1].hi, u[2].hi},
      pv = {v[0].hi, v[1].hi, v[2].hi}, pw = {w[0].hi, w[1].hi, w[2].hi};
    return triple_product_sign_exact(&pu, &pv, &pw);
  }
  /* The determinant is also `w . (u x v)`. Where the first two differences
   * are doubles held exactly and their cross product is exact in doubles,
   * each rounding having no error (#two_product, #two_diff), the determinant
   * is the sum of the products of each component of that cross product with
   * the two parts of the pair that is exactly the third difference, each
   * product exact as a pair. The sum of those parts is first compensated
   * (#two_sum), whose result lies within u of the sum plus gamma_23^2 times
   * the sum of the magnitudes of the parts (Ogita, Rump and Oishi, Sum2), so
   * a result above twice that bound has the sign of the sum; otherwise an
   * expansion adds them exactly (#grow_expansion). Parts inside 2^-450 to
   * 2^450 keep every product and its error in the normal doubles */
  bool exact = true;
  double n[3];
  for (int k = 0; k < 3 && exact; k++)
    exact = u[k].lo == 0.0 && v[k].lo == 0.0;
  for (int k = 0; k < 3 && exact; k++)
  {
    int i = (k + 1) % 3, j = (k + 2) % 3;
    double p, pe, q, qe, err;
    two_product(u[i].hi, v[j].hi, &p, &pe);
    two_product(u[j].hi, v[i].hi, &q, &qe);
    two_diff(p, q, &n[k], &err);
    exact = pe == 0.0 && qe == 0.0 && err == 0.0 &&
      (n[k] == 0.0 || (fabs(n[k]) >= 0x1p-450 && fabs(n[k]) <= 0x1p450));
  }
  for (int k = 0; k < 3 && exact; k++)
    exact = (w[k].hi == 0.0 || (fabs(w[k].hi) >= 0x1p-450 &&
      fabs(w[k].hi) <= 0x1p450)) &&
      (w[k].lo == 0.0 || fabs(w[k].lo) >= 0x1p-450);
  if (exact)
  {
    double parts[12];
    int np = 0;
    for (int k = 0; k < 3; k++)
    {
      if (n[k] == 0.0)
        continue;
      two_product(n[k], w[k].hi, &parts[np], &parts[np + 1]);
      np += 2;
      if (w[k].lo != 0.0)
      {
        two_product(n[k], w[k].lo, &parts[np], &parts[np + 1]);
        np += 2;
      }
    }
    double s = 0.0, c = 0.0, mag = 0.0;
    for (int i = 0; i < np; i++)
    {
      double se;
      two_sum(s, parts[i], &s, &se);
      c += se;
      mag += fabs(parts[i]);
    }
    double res = s + c, g = 23.0 * DBL_EPSILON;
    if (fabs(res) > 2.0 * g * g * mag * (1.0 + 4.0 * DBL_EPSILON))
      return (res > 0.0) ? 1 : -1;
    double e[12];
    int len = 0;
    for (int i = 0; i < np; i++)
      len = grow_expansion(len, e, parts[i], e);
    return (len == 0 || e[len - 1] == 0.0) ? 0 :
      ((e[len - 1] > 0.0) ? 1 : -1);
  }
  /* Otherwise in pairs of doubles with a bound on their error, where a value
   * with no error that is zero is an exact zero the bound does not tell */
  ddb_cross3(v, w, x);
  DDBound det = ddb_dot3(u, x);
  if (ddb_is_zero(det))
    return 0;
  int sign = ddb_sign(det);
  if (sign != 0)
    return sign;
  DyadicWideArena ar;
  dyadic_wide_arena_init(&ar);
  DyadicWide *wu[3], *wv[3], *ww[3], *wx[3];
  wide_diff3(&ar, a1, a0, wu);
  wide_diff3(&ar, b1, b0, wv);
  wide_diff3(&ar, c1, c0, ww);
  wide_cross3(&ar, wv, ww, wx);
  sign = wide_dot3(&ar, wu, wx)->sign;
  dyadic_wide_arena_free(&ar);
  return sign;
}

/* The cross product of two vectors in doubles, each the difference of two
 * points rounded once, and the sum of the magnitudes of the two products of
 * each component, within 2^-50 of which the component lies where that sum lies
 * inside 2^-900 and 2^900 (#vector_cross_signs) */
static inline void
dbl_cross3(const double *a0, const double *a1, const double *b0,
  const double *b1, double *x, double *mx)
{
  double u0 = a1[0] - a0[0], u1 = a1[1] - a0[1], u2 = a1[2] - a0[2];
  double v0 = b1[0] - b0[0], v1 = b1[1] - b0[1], v2 = b1[2] - b0[2];
  x[0] = u1 * v2 - u2 * v1;
  x[1] = u2 * v0 - u0 * v2;
  x[2] = u0 * v1 - u1 * v0;
  mx[0] = fabs(u1 * v2) + fabs(u2 * v1);
  mx[1] = fabs(u2 * v0) + fabs(u0 * v2);
  mx[2] = fabs(u0 * v1) + fabs(u1 * v0);
  return;
}

/**
 * @brief Return the sign of the dot product of the normal of a plane and of
 * the cross product of two vectors, computed exactly for any finite doubles
 * @details The sign of `((b - a) x (c - a)) . ((x1 - x0) x (y1 - y0))`, of
 * degree four, first in doubles with a bound on its error (#dbl_cross3), then
 * in pairs of doubles with a bound on their error (#DDBound), then on
 * #DyadicWide where the bound cannot tell it
 * @param[in] a,b,c Three points spanning the plane, three coordinates each
 * @param[in] x0,x1,y0,y1 Points, three coordinates each
 * @return -1, 0 or 1
 */
int
plane_cross_dot_sign(const double *a, const double *b, const double *c,
  const double *x0, const double *x1, const double *y0, const double *y1)
{
  /* In doubles first: each component of the two cross products lies within
   * 2^-50 of the sum of the magnitudes of its products (#dbl_cross3), so their
   * dot product lies within 2^-48 of the sum of the products of those sums */
  {
    double xn[3], mn[3], xm[3], mm[3];
    dbl_cross3(a, b, a, c, xn, mn);
    dbl_cross3(x0, x1, y0, y1, xm, mm);
    double dotd = xn[0] * xm[0] + xn[1] * xm[1] + xn[2] * xm[2];
    double perm = mn[0] * mm[0] + mn[1] * mm[1] + mn[2] * mm[2];
    if (perm >= 0x1p-900 && perm <= 0x1p900 && fabs(dotd) > 0x1p-48 * perm)
      return (dotd > 0.0) ? 1 : -1;
  }
  DDBound u[3], v[3], n[3], p[3], q[3], m[3];
  ddb_diff3(b, a, u);
  ddb_diff3(c, a, v);
  ddb_cross3(u, v, n);
  ddb_diff3(x1, x0, p);
  ddb_diff3(y1, y0, q);
  ddb_cross3(p, q, m);
  /* An exact zero, which the filter does not tell (#vector_triple_sign) */
  DDBound dot = ddb_dot3(n, m);
  if (ddb_is_zero(dot))
    return 0;
  int sign = ddb_sign(dot);
  if (sign != 0)
    return sign;
  DyadicWideArena ar;
  dyadic_wide_arena_init(&ar);
  DyadicWide *wu[3], *wv[3], *wn[3], *wp[3], *wq[3], *wm[3];
  wide_diff3(&ar, b, a, wu);
  wide_diff3(&ar, c, a, wv);
  wide_cross3(&ar, wu, wv, wn);
  wide_diff3(&ar, x1, x0, wp);
  wide_diff3(&ar, y1, y0, wq);
  wide_cross3(&ar, wp, wq, wm);
  sign = wide_dot3(&ar, wn, wm)->sign;
  dyadic_wide_arena_free(&ar);
  return sign;
}

/**
 * @brief Return the sign of the distance of a point to a plane less a
 * distance, computed exactly for any finite doubles
 * @details With `n = (b - a) x (c - a)`, the sign of
 * `(n . (p - a))^2 - d^2 (n . n)`, of degree six, first in doubles with a
 * bound on its error (#dbl_cross3), then in pairs of doubles with a bound on
 * their error (#DDBound), then as a comparison of two products of
 * #DyadicWide of degree at most four (#dyadic_wide_product_cmp)
 * @param[in] a,b,c Three points spanning the plane, not collinear, three
 * coordinates each
 * @param[in] p Point, three coordinates
 * @param[in] d Distance, not negative
 * @return -1, 0 or 1
 */
int
plane_point_offset_sign(const double *a, const double *b, const double *c,
  const double *p, double d)
{
  assert(d >= 0.0);
  /* In doubles first: with each component of n within 2^-50 of the sum of the
   * magnitudes of its products (#dbl_cross3), n . (p - a) lies within 2^-48 of
   * E1, the sum of those sums times |p - a|, and n . n within 2^-48 of S2, the
   * sum of their squares, so the value lies within 2^-46 of E1^2 + d^2 S2,
   * where no step leaves the normal doubles: that sum inside 2^-900 and
   * 2^900, and d^2 normal or zero */
  if (d == 0.0 || d >= 0x1p-450)
  {
    double xn[3], mn[3];
    dbl_cross3(a, b, a, c, xn, mn);
    double w0 = p[0] - a[0], w1 = p[1] - a[1], w2 = p[2] - a[2];
    double en = xn[0] * w0 + xn[1] * w1 + xn[2] * w2;
    double nn = xn[0] * xn[0] + xn[1] * xn[1] + xn[2] * xn[2];
    double e1 = mn[0] * fabs(w0) + mn[1] * fabs(w1) + mn[2] * fabs(w2);
    double s2 = mn[0] * mn[0] + mn[1] * mn[1] + mn[2] * mn[2];
    double val = en * en - d * d * nn, mag = e1 * e1 + d * d * s2;
    if (mag >= 0x1p-900 && mag <= 0x1p900 && fabs(val) > 0x1p-46 * mag)
      return (val > 0.0) ? 1 : -1;
  }
  DDBound u[3], v[3], w[3], n[3], dd;
  ddb_diff3(b, a, u);
  ddb_diff3(c, a, v);
  ddb_diff3(p, a, w);
  ddb_cross3(u, v, n);
  DDBound en = ddb_dot3(n, w), nn = ddb_dot3(n, n);
  dd.hi = d; dd.lo = 0.0; dd.e = 0.0;
  /* An exact zero, which the filter does not tell (#vector_triple_sign) */
  DDBound val = ddb_sub(ddb_mul(en, en), ddb_mul(ddb_mul(dd, dd), nn));
  if (ddb_is_zero(val))
    return 0;
  int sign = ddb_sign(val);
  if (sign != 0)
    return sign;
  DyadicWideArena ar;
  dyadic_wide_arena_init(&ar);
  DyadicWide *wu[3], *wv[3], *ww[3], *wn[3];
  wide_diff3(&ar, b, a, wu);
  wide_diff3(&ar, c, a, wv);
  wide_diff3(&ar, p, a, ww);
  wide_cross3(&ar, wu, wv, wn);
  const DyadicWide *wen = wide_dot3(&ar, wn, ww),
    *wnn = wide_dot3(&ar, wn, wn);
  const DyadicWide *wd = dyadic_wide_of_double(&ar, d);
  sign = dyadic_wide_product_cmp(&ar, wen, wen, dyadic_wide_mul(&ar, wd, wd),
    wnn);
  dyadic_wide_arena_free(&ar);
  return sign;
}

/**
 * @brief Return the sign of the triple product `<p, q x r>` of three vectors,
 * computed exactly
 * @details The triple product is the determinant of the three vectors, a sum
 * of six products of three coordinates. Each product of two of them is its
 * rounded value plus its error (#two_product), that pair times the third is
 * again an expansion (#scale_expansion), and the six are added into one whose
 * last component carries the sign of the whole. #vector_triple_sign calls it
 * where every difference is a double held exactly within a range where no
 * step leaves the normal doubles
 * @note Exact where no product of coordinates overflows or underflows
 * @return 1 where `r` lies on the side of the plane of `p` and `q` that their
 * cross product points to, -1 on the other, 0 exactly where the three vectors
 * are coplanar
 */
int
triple_product_sign_exact(const POINT3D *p, const POINT3D *q,
  const POINT3D *r)
{
  /* The six terms, each a coordinate of `p` times a product of a coordinate
   * of `q` and one of `r`, with its sign */
  const double f[6] = {p->x, - p->x, p->y, - p->y, p->z, - p->z};
  const double g[6] = {q->y, q->z, q->z, q->x, q->x, q->y};
  const double h[6] = {r->z, r->y, r->x, r->z, r->y, r->x};
  double acc[24], tmp[24];
  int len = 0;
  for (int k = 0; k < 6; k++)
  {
    double x, y, e[2], term[4];
    two_product(g[k], h[k], &x, &y);
    int elen = 0;
    if (y != 0.0)
      e[elen++] = y;
    if (x != 0.0)
      e[elen++] = x;
    if (elen == 0 || f[k] == 0.0)
      continue;
    int tlen = scale_expansion(elen, e, f[k], term);
    if (len == 0)
    {
      memcpy(acc, term, (size_t) tlen * sizeof(double));
      len = tlen;
    }
    else
      len = expansion_sum(len, acc, tlen, term, acc, tmp);
  }
  if (len == 0)
    return 0;
  double top = acc[len - 1];
  return (top > 0.0) ? 1 : ((top < 0.0) ? -1 : 0);
}

/**
 * @brief Return the sign of the incircle determinant of four points, read on
 * #DyadicWide for any finite coordinates
 * @details The differences A - D, B - D and C - D are exact, and the
 * determinant is the sum of each lift `|P - D|^2` times the cross product of
 * the other two points taken from D, as #incircle_sign_exact forms it on
 * expansions. #incircle_sign_exact calls it where a product of four of its
 * components could overflow or underflow the doubles, as
 * #cross_product_sign_exact calls #cross_product_sign_wide
 */
static int
incircle_sign_wide(double ax, double ay, double bx, double by, double cx,
  double cy, double dx, double dy)
{
  const double from[6] = {ax, ay, bx, by, cx, cy};
  const double to[6] = {dx, dy, dx, dy, dx, dy};
  DyadicWideArena arena, *ar = &arena;
  dyadic_wide_arena_init(ar);
  const DyadicWide *v[6];
  for (int i = 0; i < 6; i++)
    v[i] = dyadic_wide_sub(ar, dyadic_wide_of_double(ar, from[i]),
      dyadic_wide_of_double(ar, to[i]));
  static const int cu[3] = {2, 4, 0}, cv[3] = {4, 0, 2};
  const DyadicWide *det = dyadic_wide_of_double(ar, 0.0);
  for (int k = 0; k < 3; k++)
  {
    const DyadicWide *lift = dyadic_wide_add(ar,
      dyadic_wide_mul(ar, v[2 * k], v[2 * k]),
      dyadic_wide_mul(ar, v[2 * k + 1], v[2 * k + 1]));
    int u = cu[k], w = cv[k];
    const DyadicWide *cross = dyadic_wide_sub(ar,
      dyadic_wide_mul(ar, v[u], v[w + 1]),
      dyadic_wide_mul(ar, v[w], v[u + 1]));
    det = dyadic_wide_add(ar, det, dyadic_wide_mul(ar, lift, cross));
  }
  int sign = det->sign;
  dyadic_wide_arena_free(ar);
  return sign;
}

/**
 * @brief Return the sign of the incircle determinant of four points, computed
 * exactly from their coordinates
 * @details Each coordinate difference is its rounded value plus the error of
 * the rounding (#two_diff), the three lifts and the three cross products are
 * formed from those as expansions, and the three terms are added, so the last
 * component of the sum carries the sign of the determinant. Every product of
 * the expansions is a product of four components of the differences, exact
 * where the lowest set bits of the four reach that of the least subnormal and
 * the product stays below the largest double: each component of at least
 * 2^-268 in its lowest bit and below 2^250 keeps every product, lift, cross
 * product and term in range. Otherwise the sign is read on #DyadicWide
 * (#incircle_sign_wide), exact for any finite coordinates.
 */
static int
incircle_sign_exact(double ax, double ay, double bx, double by, double cx,
  double cy, double dx, double dy)
{
  /* The differences A - D, B - D and C - D, x then y */
  const double from[6] = {ax, ay, bx, by, cx, cy};
  const double to[6] = {dx, dy, dx, dy, dx, dy};
  double v[6][2];
  int vl[6];
  for (int i = 0; i < 6; i++)
  {
    double x, y;
    two_diff(from[i], to[i], &x, &y);
    vl[i] = 0;
    if (y != 0.0)
      v[i][vl[i]++] = y;
    if (x != 0.0 || vl[i] == 0)
      v[i][vl[i]++] = x;
    for (int j = 0; j < vl[i]; j++)
    {
      double c = v[i][j];
      if (c != 0.0 && (! (fabs(c) < 0x1p250) ||
          double_lowest_set_bit(c) < -268))
        return incircle_sign_wide(ax, ay, bx, by, cx, cy, dx, dy);
    }
  }
  double pa[8], pb[8], scratch[16], part[4];
  /* The lifts |A - D|^2, |B - D|^2, |C - D|^2 */
  double lift[3][16];
  int liftl[3];
  for (int k = 0; k < 3; k++)
  {
    const double *x = v[2 * k], *y = v[2 * k + 1];
    int lx = vl[2 * k], ly = vl[2 * k + 1];
    int l1 = expansion_product(lx, x, lx, x, pa, scratch, part);
    int l2 = expansion_product(ly, y, ly, y, pb, scratch, part);
    liftl[k] = expansion_sum(l1, pa, l2, pb, lift[k], scratch);
  }
  /* The cross products of (B, C), (C, A) and (A, B), each point taken from D */
  static const int cu[3] = {2, 4, 0}, cv[3] = {4, 0, 2};
  double cross[3][16];
  int crossl[3];
  for (int k = 0; k < 3; k++)
  {
    int u = cu[k], w = cv[k];
    int l1 = expansion_product(vl[u], v[u], vl[w + 1], v[w + 1], pa, scratch,
      part);
    int l2 = expansion_product(vl[w], v[w], vl[u + 1], v[u + 1], pb, scratch,
      part);
    for (int i = 0; i < l2; i++)
      pb[i] = - pb[i];
    crossl[k] = expansion_sum(l1, pa, l2, pb, cross[k], scratch);
  }
  /* The determinant, the sum of each lift times the cross product of the
   * other two points */
  double term[512], tmp[512], part2[32], acc[1536], acc2[1536];
  int accl = 0;
  for (int k = 0; k < 3; k++)
  {
    int tl = expansion_product(liftl[k], lift[k], crossl[k], cross[k], term,
      tmp, part2);
    if (accl == 0)
    {
      memcpy(acc, term, (size_t) tl * sizeof(double));
      accl = tl;
    }
    else
      accl = expansion_sum(accl, acc, tl, term, acc, acc2);
  }
  double top = acc[accl - 1];
  return (top > 0.0) ? 1 : ((top < 0.0) ? -1 : 0);
}

/**
 * @brief Return the sign of the incircle determinant of four points where its
 * evaluation on two doubles with a bound carries it, 0 for an exact zero, and
 * 2 where it cannot tell
 * @details The second filter between the one of #incircle_sign and
 * #incircle_sign_exact, for four points whose coordinate differences from D
 * are exact doubles, as those of nearby points are (Sterbenz). Each lift, each
 * cross product, each term and their sum carry the bound of their evaluation
 * (#DDBound, #ddb_mul, #ddb_add, #ddb_sub), and the sign is told where the
 * determinant lies beyond it (#ddb_sign_known). Where a difference is not
 * exact, or lies outside 2^-200 to 2^200, it declines. Mirrors
 * #point_motion_dwithin_filter, the second filter of a distance
 */
static int
incircle_sign_filter(double ax, double ay, double bx, double by, double cx,
  double cy, double dx, double dy)
{
  /* The differences A - D, B - D and C - D, x then y, each a double held
   * exactly */
  const double from[6] = {ax, ay, bx, by, cx, cy};
  const double to[6] = {dx, dy, dx, dy, dx, dy};
  DDBound v[6];
  for (int i = 0; i < 6; i++)
  {
    double err;
    two_diff(from[i], to[i], &v[i].hi, &err);
    double m = fabs(v[i].hi);
    if (err != 0.0 || (m != 0.0 && (m < 0x1p-200 || m > 0x1p200)))
      return 2;
    v[i].lo = v[i].e = 0.0;
  }
  /* Each lift times the cross product of the other two points */
  static const int cu[3] = {2, 4, 0}, cv[3] = {4, 0, 2};
  DDBound det = {0.0, 0.0, 0.0};
  for (int k = 0; k < 3; k++)
  {
    DDBound lift = ddb_add(ddb_mul(v[2 * k], v[2 * k]),
      ddb_mul(v[2 * k + 1], v[2 * k + 1]));
    int u = cu[k], w = cv[k];
    DDBound cross = ddb_sub(ddb_mul(v[u], v[w + 1]), ddb_mul(v[w], v[u + 1]));
    det = ddb_add(det, ddb_mul(lift, cross));
  }
  return ddb_sign_known(det);
}

/**
 * @brief Return the sign of the incircle determinant of four points, decided
 * exactly
 * @details Positive where D lies inside the circle through A, B and C taken
 * counterclockwise, negative where it lies outside, and 0 exactly where the
 * four points lie on one circle. The determinant is evaluated in double and
 * read against Shewchuk's bound for it, formed from its own terms, where
 * every nonzero coordinate difference lies from 2^-240 to 2^250, so that no
 * product of two underflows; where it
 * cannot tell, the evaluation in pairs of doubles of #incircle_sign_filter
 * is read against its own bound. Both are filters, never tolerances, since
 * where neither can tell the exact sign is computed (#incircle_sign_exact)
 */
int
incircle_sign(double ax, double ay, double bx, double by, double cx,
  double cy, double dx, double dy)
{
  double adx = ax - dx, ady = ay - dy, bdx = bx - dx, bdy = by - dy;
  double cdx = cx - dx, cdy = cy - dy;
  double bdxcdy = bdx * cdy, cdxbdy = cdx * bdy;
  double cdxady = cdx * ady, adxcdy = adx * cdy;
  double adxbdy = adx * bdy, bdxady = bdx * ady;
  double alift = adx * adx + ady * ady;
  double blift = bdx * bdx + bdy * bdy;
  double clift = cdx * cdx + cdy * cdy;
  double det = alift * (bdxcdy - cdxbdy) + blift * (cdxady - adxcdy) +
    clift * (adxbdy - bdxady);
  double permanent = (fabs(bdxcdy) + fabs(cdxbdy)) * alift +
    (fabs(cdxady) + fabs(adxcdy)) * blift +
    (fabs(adxbdy) + fabs(bdxady)) * clift;
  double bound = (10.0 + 96.0 * DBL_EPSILON) * DBL_EPSILON * permanent;
  /* The bound holds where no product underflows: each nonzero difference of
   * at least 2^-240 keeps every product of two normal, and a later product
   * that underflows errs by far less than the bound, of at least 2^-1012 */
  const double diff[6] = {adx, ady, bdx, bdy, cdx, cdy};
  bool in_range = true;
  for (int i = 0; i < 6; i++)
    if (diff[i] != 0.0 && ! (fabs(diff[i]) >= 0x1p-240 &&
        fabs(diff[i]) < 0x1p250))
      in_range = false;
  if (in_range && det > bound)
    return 1;
  if (in_range && det < - bound)
    return -1;
  int sign = incircle_sign_filter(ax, ay, bx, by, cx, cy, dx, dy);
  if (sign != 2)
    return sign;
  return incircle_sign_exact(ax, ay, bx, by, cx, cy, dx, dy);
}

/*****************************************************************************
 * Where a straight segment meets a circular arc, decided exactly
 *****************************************************************************/

static double *
arcseg_arena_alloc(ArcsegArena *arena, int ndoubles)
{
  ndoubles = Max(ndoubles, 1);
  if (arena->used + ndoubles <= ARCSEG_ARENA_SIZE)
  {
    double *block = arena->buf + arena->used;
    arena->used += ndoubles;
    return block;
  }
  if (arena->n == arena->cap)
  {
    arena->cap = arena->cap ? 2 * arena->cap : 64;
    arena->blocks = arena->blocks ?
      repalloc(arena->blocks, sizeof(double *) * arena->cap) :
      palloc(sizeof(double *) * arena->cap);
  }
  double *block = palloc(sizeof(double) * ndoubles);
  arena->blocks[arena->n++] = block;
  return block;
}

/**
 * @brief Return true where the buffer of an arena holds room for
 * @p ndoubles past the values it holds
 * @details A sum or a product is formed in that room, at
 * `arena->buf + arena->used`, with its scratch (#arcseg_x_add,
 * #arcseg_x_mul), and its compressed result, written at the start of the
 * room, is all the arena keeps (#arcseg_arena_keep): the scratch, and the
 * components compression merges, cost no room after it
 */
static inline bool
arcseg_arena_room(const ArcsegArena *arena, int ndoubles)
{
  return arena->used + ndoubles <= ARCSEG_ARENA_SIZE;
}

/**
 * @brief Keep the first @p ndoubles of the room #arcseg_arena_room found
 */
static inline void
arcseg_arena_keep(ArcsegArena *arena, int ndoubles)
{
  arena->used += Max(ndoubles, 1);
  return;
}

void
arcseg_arena_free(ArcsegArena *arena)
{
  for (int i = 0; i < arena->n; i++)
    pfree(arena->blocks[i]);
  if (arena->blocks)
    pfree(arena->blocks);
  arena->blocks = NULL;
  arena->n = arena->cap = arena->used = 0;
  arena->unsafe = false;
  if (arena->warena)
  {
    dyadic_wide_arena_free(arena->warena);
    pfree(arena->warena);
    arena->warena = NULL;
  }
  return;
}

/* An empty arena, of expansions or, where @p wide is set, of #DyadicWide */
void
arcseg_arena_init(ArcsegArena *arena, bool wide)
{
  arena->used = arena->n = arena->cap = 0;
  arena->unsafe = false;
  arena->wide = wide;
  arena->filter = false;
  arena->blocks = NULL;
  arena->warena = NULL;
  if (wide)
  {
    arena->warena = palloc(sizeof(DyadicWideArena));
    dyadic_wide_arena_init(arena->warena);
  }
  return;
}

/* An empty arena of doubles carrying a bound on their error, which answers
 * the signs it can tell before any expansion is formed (#ArcsegArena) */
void
arcseg_arena_init_filter(ArcsegArena *arena)
{
  arcseg_arena_init(arena, false);
  arena->filter = true;
  return;
}

/* A value of a filter arena: one component and its bound */
static ArcsegExact
arcseg_x_filtered(ArcsegArena *arena, ArcsegFilter f)
{
  ArcsegExact r;
  r.w = NULL;
  r.e = arcseg_arena_alloc(arena, 1);
  r.e[0] = f.v;
  r.n = 1;
  r.err = f.e;
  /* A value or a bound that is not finite tells no sign */
  if (! isfinite(f.v) || ! isfinite(f.e))
    arena->unsafe = true;
  return r;
}

/* The filter value a value of a filter arena holds */
static inline ArcsegFilter
arcseg_x_as_filter(ArcsegExact a)
{
  ArcsegFilter f;
  f.v = a.e[0];
  f.e = a.err;
  return f;
}

/* The value an arena that left its exact range hands for every sum and
 * product after: none of them is computed, as every answer of an unsafe
 * arena is discarded, and an expansion holding a component that overflowed
 * keeps every component of the values it is added to or multiplied by,
 * compression merging none of them, so that each later sum or product
 * would cost more than the last (#dx_spent) */
static ArcsegExact
arcseg_x_spent(ArcsegArena *arena)
{
  ArcsegExact r;
  r.e = arcseg_arena_alloc(arena, 1);
  r.e[0] = 0.0;
  r.n = 1;
  r.err = 0.0;
  r.w = NULL;
  return r;
}

/* A value of a wide arena */
static inline ArcsegExact
arcseg_w_value(DyadicWide *w)
{
  ArcsegExact r;
  r.e = NULL;
  r.n = 0;
  r.err = 0.0;
  r.w = w;
  return r;
}

ArcsegExact
arcseg_x_diff(ArcsegArena *arena, double a, double b)
{
  /* A filter arena reads the difference as #arcseg_f_diff bounds it */
  if (arena->filter)
    return arcseg_x_filtered(arena, arcseg_f_diff(a, b));
  if (arena->wide)
  {
    /* A double that is not finite has no exact value */
    if (arena->unsafe || ! isfinite(a) || ! isfinite(b))
    {
      arena->unsafe = true;
      return arcseg_x_spent(arena);
    }
    return arcseg_w_value(dyadic_wide_sub(arena->warena,
      dyadic_wide_of_double(arena->warena, a),
      dyadic_wide_of_double(arena->warena, b)));
  }
  ArcsegExact r;
  r.w = NULL;
  r.e = arcseg_arena_alloc(arena, 2);
  double x, y;
  two_sum(a, - b, &x, &y);
  /* A difference of two doubles past the largest double */
  if (! isfinite(x))
    arena->unsafe = true;
  r.err = 0.0;
  r.n = 0;
  if (y != 0.0)
    r.e[r.n++] = y;
  r.e[r.n++] = x;
  return r;
}

ArcsegExact
arcseg_x_add(ArcsegArena *arena, ArcsegExact a, ArcsegExact b)
{
  if (arena->unsafe)
    return arcseg_x_spent(arena);
  /* A filter arena reads the sum as #arcseg_f_add bounds it */
  if (arena->filter)
    return arcseg_x_filtered(arena, arcseg_f_add(arcseg_x_as_filter(a),
      arcseg_x_as_filter(b)));
  if (arena->wide)
    return arcseg_w_value(dyadic_wide_add(arena->warena, a.w, b.w));
  int cap = a.n + b.n;
  ArcsegExact r;
  r.w = NULL;
  double *room = arcseg_arena_room(arena, 2 * cap) ?
    arena->buf + arena->used : NULL;
  double *h;
  if (room)
  {
    r.e = room;
    h = room + cap;
  }
  else
  {
    h = arcseg_arena_alloc(arena, cap);
    r.e = arcseg_arena_alloc(arena, cap);
  }
  int hlen = expansion_sum(a.n, a.e, b.n, b.e, h, r.e);
  r.n = compress_expansion(hlen, h, r.e);
  if (room)
    arcseg_arena_keep(arena, r.n);
  r.err = (a.err + b.err) * (1.0 + 0x1p-50);
  return r;
}

ArcsegExact
arcseg_x_scale(ArcsegArena *arena, ArcsegExact a, double k)
{
  if (arena->unsafe)
    return arcseg_x_spent(arena);
  /* A filter arena reads the product by a power of two as #arcseg_f_scale
   * bounds it */
  if (arena->filter)
    return arcseg_x_filtered(arena, arcseg_f_scale(arcseg_x_as_filter(a), k));
  if (arena->wide)
    return arcseg_w_value(dyadic_wide_mul(arena->warena, a.w,
      dyadic_wide_of_double(arena->warena, k)));
  /* A power of two scales every component exactly */
  ArcsegExact r;
  r.w = NULL;
  r.e = arcseg_arena_alloc(arena, a.n);
  for (int i = 0; i < a.n; i++)
    r.e[i] = a.e[i] * k;
  r.n = a.n;
  r.err = a.err * fabs(k);
  return r;
}

ArcsegExact
arcseg_x_sub(ArcsegArena *arena, ArcsegExact a, ArcsegExact b)
{
  return arcseg_x_add(arena, a, arcseg_x_scale(arena, b, -1.0));
}

/* The sum of the magnitudes of the components of an expansion, read as
 * #arcseg_x_sign reads its last one */
static double
arcseg_x_mag(ArcsegExact a)
{
  double m = 0.0;
  for (int i = 0; i < a.n; i++)
    m += fabs(a.e[i]);
  return m * (1.0 + 0x1p-50);
}

ArcsegExact
arcseg_x_mul(ArcsegArena *arena, ArcsegExact a, ArcsegExact b)
{
  /* The product of two components is exact while its error term is a
   * double: the units of their last places multiply to at least the
   * smallest subnormal, 2^-1074, which holds where the product is at least
   * 2^-968. Below it each of the products of two components may lose up to
   * 2^-1074, which the bound of the result carries; above the largest double
   * nothing is kept. The components grow in magnitude */
  if (arena->unsafe)
    return arcseg_x_spent(arena);
  /* A filter arena reads the product as #arcseg_f_mul bounds it */
  if (arena->filter)
    return arcseg_x_filtered(arena, arcseg_f_mul(arcseg_x_as_filter(a),
      arcseg_x_as_filter(b)));
  if (arena->wide)
    return arcseg_w_value(dyadic_wide_mul(arena->warena, a.w, b.w));
  double under = 0.0;
  if (a.n > 0 && b.n > 0)
  {
    double lo = fabs(a.e[0]) * fabs(b.e[0]);
    double hi = fabs(a.e[a.n - 1]) * fabs(b.e[b.n - 1]);
    if (hi > 0x1p+1000)
    {
      arena->unsafe = true;
      return arcseg_x_spent(arena);
    }
    /* A product of two components that are not zero may itself round to
     * zero, which is underflow and not a zero component */
    if (a.e[0] != 0.0 && b.e[0] != 0.0 && lo < 0x1p-968)
      under = (double) a.n * (double) b.n * 0x1p-1072;
  }
  int cap = 2 * a.n * b.n;
  ArcsegExact r;
  r.w = NULL;
  double *room = arcseg_arena_room(arena, 2 * cap + 2 * a.n) ?
    arena->buf + arena->used : NULL;
  double *h, *part;
  if (room)
  {
    r.e = room;
    h = room + cap;
    part = h + cap;
  }
  else
  {
    h = arcseg_arena_alloc(arena, cap);
    part = arcseg_arena_alloc(arena, 2 * a.n);
    r.e = arcseg_arena_alloc(arena, cap);
  }
  int hlen = expansion_product(a.n, a.e, b.n, b.e, h, r.e, part);
  r.n = compress_expansion(hlen, h, r.e);
  if (room)
    arcseg_arena_keep(arena, r.n);
  r.err = 0.0;
  /* The bound is computed in doubles, which below the normal range round
   * to the nearest multiple of 2^-1074 and so may round it to zero: each of
   * its three products and two sums loses at most half of that */
  if (a.err > 0.0 || b.err > 0.0)
    r.err = (arcseg_x_mag(a) * b.err + arcseg_x_mag(b) * a.err +
      a.err * b.err) * (1.0 + 0x1p-50) + 0x1p-1071;
  r.err += under;
  return r;
}

static inline int
arcseg_x_sign(ArcsegExact a)
{
  if (a.w)
    return a.w->sign;
  double last = a.n > 0 ? a.e[a.n - 1] : 0.0;
  return (last > 0.0) ? 1 : ((last < 0.0) ? -1 : 0);
}

/* The sign of #arcseg_x_sign, recording in the arena where the bound of the
 * expansion does not let the value's sign be told: the value is at least
 * its last component less the others, which together are smaller than it */
int
arcseg_x_sign_checked(ArcsegArena *arena, ArcsegExact a)
{
  /* A filter arena tells the sign #arcseg_f_sign tells, with its margin */
  if (arena->filter)
  {
    int s = arcseg_f_sign(arcseg_x_as_filter(a));
    if (s == 2)
    {
      arena->unsafe = true;
      return 0;
    }
    return s;
  }
  if (a.err > 0.0)
  {
    double low = (a.n > 0) ? fabs(a.e[a.n - 1]) : 0.0;
    for (int i = 0; i + 1 < a.n; i++)
      low -= fabs(a.e[i]);
    if (low * (1.0 - 0x1p-50) <= a.err)
      arena->unsafe = true;
  }
  return arcseg_x_sign(a);
}

/*****************************************************************************
 * Numbers of a tower of square roots, their signs decided exactly
 *****************************************************************************/

void
dyadic_ctx_init(DyadicCtx *ctx, bool wide)
{
  arcseg_arena_init(&ctx->arena, false);
  ctx->wide = wide;
  ctx->filter = false;
  ctx->dd = false;
  dyadic_wide_arena_init(&ctx->warena);
  return;
}

void
dyadic_ctx_free(DyadicCtx *ctx)
{
  arcseg_arena_free(&ctx->arena);
  dyadic_wide_arena_free(&ctx->warena);
  return;
}

/* The sign of a value, the arena recording where an expansion may not
 * carry it (#arcseg_x_sign_checked) */
int
dx_sign(DyadicCtx *ctx, DyadicX a)
{
  if (ctx->filter)
  {
    /* An exact zero, as the coefficients a root does not reach, or a sign
     * the bound tells; any other leaves the question to the expansions */
    if (a.f.v == 0.0 && a.f.e == 0.0)
      return 0;
    int s = arcseg_f_sign(a.f);
    if (s == 2)
    {
      ctx->arena.unsafe = true;
      return 0;
    }
    return s;
  }
  if (ctx->dd)
  {
    if (ddb_is_zero(a.d))
      return 0;
    int s = ddb_sign(a.d);
    if (s == 0)
      ctx->arena.unsafe = true;
    return s;
  }
  if (ctx->wide)
    return a.w->sign;
  return arcseg_x_sign_checked(&ctx->arena, a.x);
}

/* A value as a double and a bound on its error */
ArcsegFilter
dx_filter(const DyadicCtx *ctx, DyadicX a)
{
  ArcsegFilter r;
  if (ctx->filter)
    return a.f;
  if (ctx->dd)
  {
    /* The pair rounds once, its rounding error exactly (fast two-sum) */
    r.v = a.d.hi + a.d.lo;
    r.e = a.d.e + fabs(a.d.lo - (r.v - a.d.hi));
    return r;
  }
  if (ctx->wide)
  {
    /* The two limbs at the top of the value */
    int i = a.w->len - 1;
    double m = 0.0;
    int e = 0;
    if (a.w->sign != 0)
    {
      m = (double) a.w->limb[i];
      if (i > 0)
        m += ldexp((double) a.w->limb[i - 1], -64);
      e = a.w->exp + 64 * i;
    }
    r.v = ldexp(m, e) * (double) a.w->sign;
    /* The two limbs read hold the value to 2^-60 of it, and ldexp rounds
     * once, by up to the least double where it leaves the normal range, down
     * to 0 for a value no double reaches */
    r.e = fabs(r.v) * 0x1p-50 + ((a.w->sign != 0) ? 0x1p-1074 : 0.0);
    return r;
  }
  double s = 0.0, mag = 0.0;
  for (int i = 0; i < a.x.n; i++)
  {
    s += a.x.e[i];
    mag += fabs(a.x.e[i]);
  }
  r.v = s;
  r.e = (double) a.x.n * DBL_EPSILON * mag + a.x.err;
  return r;
}

/*****************************************************************************
 * Segments between tangent points, and the meetings they make
 *****************************************************************************/

/* The answer of a question in expansions, and in #DyadicWide where those
 * leave their exact range, the question building its points again: the
 * tiers of #dyadic_pred_run past its filters, for a question whose own
 * filters read the first tiers */
static int
dyadic_pred_run_expansions(DyadicPred fn, const void *arg)
{
  DyadicCtx ctx;
  dyadic_ctx_init(&ctx, false);
  int r = fn(&ctx, arg);
  bool again = (r == 2) || ctx.arena.unsafe;
  dyadic_ctx_free(&ctx);
  if (! again)
    return r;
  dyadic_ctx_init(&ctx, true);
  r = fn(&ctx, arg);
  dyadic_ctx_free(&ctx);
  return r;
}

/* The answer of a question in doubles, then in pairs of doubles, each
 * carrying a bound on its error, then in expansions and in #DyadicWide where
 * those leave their exact range (#dyadic_pred_run_expansions) */
int
dyadic_pred_run(DyadicPred fn, const void *arg)
{
  DyadicCtx ctx;
  int r = 2;
  bool again = true;
  for (int pass = 0; pass < 2 && again; pass++)
  {
    dyadic_ctx_init(&ctx, false);
    ctx.filter = (pass == 0);
    ctx.dd = (pass == 1);
    r = fn(&ctx, arg);
    again = (r == 2) || ctx.arena.unsafe;
    dyadic_ctx_free(&ctx);
  }
  if (! again)
    return r;
  return dyadic_pred_run_expansions(fn, arg);
}

/* The sign of a polynomial in exact sums, written once in the arithmetic of
 * a #DyadicCtx, as #point_arc_within_pred writes its question: each sum a
 * term reads is the sum of its components, formed once, each term its
 * coefficient times its sums, and the sign that of the total, or 2 where the
 * tier of @p ctx cannot tell. A sum no term reads is never read, since a
 * caller sets only the sums of its question (#segment_square_least) */
static int
polynomial_sign_pred(DyadicCtx *ctx, const void *varg)
{
  const PolySignArg *a = (const PolySignArg *) varg;
  int nsums = 0;
  for (int i = 0; i < a->nterms; i++)
    for (int k = 0; k < a->terms[i].deg; k++)
      nsums = Max(nsums, a->terms[i].sum[k] + 1);
  DyadicX stack[POLY_SIGN_STACK_SUMS];
  bool formed_stack[POLY_SIGN_STACK_SUMS];
  DyadicX *sums = stack;
  bool *formed = formed_stack;
  if (nsums > POLY_SIGN_STACK_SUMS)
  {
    sums = palloc(sizeof(DyadicX) * nsums);
    formed = palloc(sizeof(bool) * nsums);
  }
  memset(formed, 0, sizeof(bool) * nsums);
  DyadicX total = dx_val(ctx, 0.0);
  for (int i = 0; i < a->nterms; i++)
  {
    DyadicX t = dx_val(ctx, a->terms[i].coef);
    for (int k = 0; k < a->terms[i].deg; k++)
    {
      int j = a->terms[i].sum[k];
      if (! formed[j])
      {
        const ExactSum *e = &a->sums[j];
        DyadicX v = dx_val(ctx, e->e[0]);
        for (int c = 1; c < e->n; c++)
          v = dx_add(ctx, v, dx_val(ctx, e->e[c]));
        sums[j] = v;
        formed[j] = true;
      }
      t = dx_mul(ctx, t, sums[j]);
    }
    total = dx_add(ctx, total, t);
  }
  int sign = dx_sign(ctx, total);
  if (sums != stack)
  {
    pfree(sums);
    pfree(formed);
  }
  return dyadic_ctx_unsafe(ctx) ? 2 : sign;
}

/**
 * @brief Return the sign of a polynomial in exact sums on the expression
 * ladder from its expansions on, exact for any finite sums
 * @details #polynomial_sign_exact reads it where its own filters cannot tell
 */
int
polynomial_sign_dyadic(const ExactSum *sums, const PolyTerm *terms,
  int nterms)
{
  PolySignArg arg = {sums, terms, nterms};
  return dyadic_pred_run_expansions(polynomial_sign_pred, &arg);
}

/*****************************************************************************
 * Signs of sums of doubles and of functions linear over a segment
 *****************************************************************************/

/**
 * @brief Return in @p ar the sum of @p k doubles, exactly
 */
DyadicWide *
dyadic_wide_sum(DyadicWideArena *ar, const double *t, int k)
{
  DyadicWide *acc = dyadic_wide_alloc(ar, 1);
  for (int i = 0; i < k; i++)
    acc = dyadic_wide_add(ar, acc, dyadic_wide_of_double(ar, t[i]));
  return acc;
}
