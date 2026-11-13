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
 * @brief The double nearest the sine and cosine of input degrees
 * @details The cosine and sine of a double read as degrees are transcendental
 * except at the multiples of 30 (Niven), so no finite computation states them;
 * the double nearest each is decided exactly all the same. The angle is
 * reduced to [0, 45] by subtractions that are exact (Sterbenz), the rational
 * values are read from their table, and every other value is enclosed in an
 * interval of fixed-point integers: pi by Machin's formula, the sine and the
 * cosine by their Taylor series, each an alternating series of decreasing
 * terms whose next term bounds what it leaves out, every product and quotient
 * rounded outward. The one search for the nearest double,
 * #nearest_double_exact, reads the enclosure through its side function
 * #trig_side, which raises the precision until the interval lies clear of the
 * midpoint the search asks about. The geodetic position of decision D6 of the
 * cell walks is the vector of the doubles nearest cos(lat) cos(lon),
 * cos(lat) sin(lon) and sin(lat) (#lonlat_vector_nearest)
 */

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "geo/geo_funcs.h"

/* The precision, in bits below the leading bit of an angle, the enclosure
 * starts from, and the one beyond which it gives up: the distance from a sine
 * or cosine of a double to a midpoint between two doubles is far above it */
#define TRIG_FIRST_BITS 96
#define TRIG_LAST_BITS (1 << 15)

/*****************************************************************************
 * Fixed-point integers
 *****************************************************************************/

/* A non-negative integer of L 32-bit limbs, the lowest first, read as a value
 * at the scale 2^-S of its enclosure */

/* a = 2^k */
static void
fix_set_pow2(uint32_t *a, int L, int k)
{
  memset(a, 0, L * sizeof(uint32_t));
  a[k / 32] = (uint32_t) 1 << (k % 32);
}

static bool
fix_is_zero(const uint32_t *a, int L)
{
  for (int i = 0; i < L; i++)
    if (a[i])
      return false;
  return true;
}

/* a += 1 */
static void
fix_increment(uint32_t *a, int L)
{
  for (int i = 0; i < L; i++)
    if (++a[i] != 0)
      return;
  assert(false);
}

/* r = a + b */
static void
fix_add(const uint32_t *a, const uint32_t *b, uint32_t *r, int L)
{
  uint64_t carry = 0;
  for (int i = 0; i < L; i++)
  {
    uint64_t s = (uint64_t) a[i] + b[i] + carry;
    r[i] = (uint32_t) s;
    carry = s >> 32;
  }
  assert(carry == 0);
}

/* r = max(a - b, 0) */
static void
fix_sub_clamp(const uint32_t *a, const uint32_t *b, uint32_t *r, int L)
{
  uint64_t borrow = 0;
  for (int i = 0; i < L; i++)
  {
    uint64_t s = (uint64_t) a[i] - b[i] - borrow;
    r[i] = (uint32_t) s;
    borrow = (s >> 32) & 1;
  }
  if (borrow)
    memset(r, 0, L * sizeof(uint32_t));
}

/* r = a * m */
static void
fix_mul_small(const uint32_t *a, uint32_t m, uint32_t *r, int L)
{
  uint64_t carry = 0;
  for (int i = 0; i < L; i++)
  {
    uint64_t p = (uint64_t) a[i] * m + carry;
    r[i] = (uint32_t) p;
    carry = p >> 32;
  }
  assert(carry == 0);
}

/* r = a / d, rounded down or, where @p up, up */
static void
fix_div_small(const uint32_t *a, uint32_t d, uint32_t *r, int L, bool up)
{
  uint64_t rem = 0;
  for (int i = L - 1; i >= 0; i--)
  {
    uint64_t cur = (rem << 32) | a[i];
    r[i] = (uint32_t) (cur / d);
    rem = cur % d;
  }
  if (up && rem)
    fix_increment(r, L);
}

/* r = a / 2^k, rounded down or, where @p up, up */
static void
fix_shift_right(const uint32_t *a, int k, uint32_t *r, int L, bool up)
{
  int limbs = k / 32, bits = k % 32;
  bool lost = false;
  for (int i = 0; i < L && i <= limbs; i++)
  {
    if (i < limbs)
      lost |= a[i] != 0;
    else if (bits)
      lost |= (a[i] & (((uint32_t) 1 << bits) - 1)) != 0;
  }
  for (int i = 0; i < L; i++)
  {
    int j = i + limbs;
    uint64_t lo = (j < L) ? a[j] : 0, hi = (j + 1 < L) ? a[j + 1] : 0;
    r[i] = (uint32_t) (((hi << 32) | lo) >> bits);
  }
  if (up && lost)
    fix_increment(r, L);
}

/* r = a * b / 2^S, S a multiple of 32, rounded down or, where @p up, up */
static void
fix_mul(const uint32_t *a, const uint32_t *b, uint32_t *r, int L, int S,
  bool up)
{
  uint32_t *prod = palloc0(2 * L * sizeof(uint32_t));
  for (int i = 0; i < L; i++)
  {
    if (! a[i])
      continue;
    uint64_t carry = 0;
    for (int j = 0; j < L; j++)
    {
      uint64_t p = (uint64_t) a[i] * b[j] + prod[i + j] + carry;
      prod[i + j] = (uint32_t) p;
      carry = p >> 32;
    }
    prod[i + L] = (uint32_t) carry;
  }
  int shift = S / 32;
  bool lost = false;
  for (int i = 0; i < shift; i++)
    lost |= prod[i] != 0;
  for (int i = 0; i < L; i++)
    r[i] = prod[i + shift];
  for (int i = L + shift; i < 2 * L; i++)
    assert(prod[i] == 0);
  pfree(prod);
  if (up && lost)
    fix_increment(r, L);
}

/* p = p * m, m of at most 64 bits */
static void
fix_mul_u64(uint32_t *p, uint64_t m, int L)
{
  uint32_t *u = palloc(L * sizeof(uint32_t));
  fix_mul_small(p, (uint32_t) (m >> 32), u, L);
  fix_mul_small(p, (uint32_t) m, p, L);
  assert(u[L - 1] == 0);
  uint64_t carry = 0;
  for (int i = 0; i < L; i++)
  {
    uint64_t s = (uint64_t) p[i] + (i > 0 ? u[i - 1] : 0) + carry;
    p[i] = (uint32_t) s;
    carry = s >> 32;
  }
  assert(carry == 0);
  pfree(u);
}

/*****************************************************************************
 * The enclosures
 *****************************************************************************/

/**
 * @brief Enclose atan(1/k) at the scale 2^-S in [lo, hi]
 * @details The series sum of (-1)^n / ((2n+1) k^(2n+1)) alternates with
 * decreasing terms, so a partial sum is off by less than its next term; each
 * power is read rounded down for @p lo and up for @p hi
 */
static void
fix_atan_inv(uint32_t k, uint32_t *lo, uint32_t *hi, int L, int S)
{
  uint32_t *plo = palloc(L * sizeof(uint32_t));
  uint32_t *phi = palloc(L * sizeof(uint32_t));
  uint32_t *t = palloc(L * sizeof(uint32_t));
  memset(lo, 0, L * sizeof(uint32_t));
  memset(hi, 0, L * sizeof(uint32_t));
  fix_set_pow2(plo, L, S);
  fix_div_small(plo, k, phi, L, true);
  fix_div_small(plo, k, plo, L, false);
  for (uint32_t n = 0; ; n++)
  {
    if (fix_is_zero(plo, L))
    {
      /* The terms left are below the power read up */
      fix_sub_clamp(lo, phi, lo, L);
      fix_add(hi, phi, hi, L);
      break;
    }
    if (n % 2 == 0)
    {
      fix_div_small(plo, 2 * n + 1, t, L, false);
      fix_add(lo, t, lo, L);
      fix_div_small(phi, 2 * n + 1, t, L, true);
      fix_add(hi, t, hi, L);
    }
    else
    {
      fix_div_small(phi, 2 * n + 1, t, L, true);
      fix_sub_clamp(lo, t, lo, L);
      fix_div_small(plo, 2 * n + 1, t, L, false);
      fix_sub_clamp(hi, t, hi, L);
    }
    fix_div_small(plo, k * k, plo, L, false);
    fix_div_small(phi, k * k, phi, L, true);
  }
  pfree(plo); pfree(phi); pfree(t);
}

/* floor(pi 2^2048), 32-bit limbs, the lowest first, computed in mpmath at
 * 2300 and 4000 bits, which agree; every vector trig_nearest_unit states
 * reads it */
#define PI_TABLE_BITS 2048
static const uint32_t PI_TABLE[PI_TABLE_BITS / 32 + 1] = {
  0x7c72e993, 0xa15486af, 0x1141e8ce, 0xb4cc5c34, 0x2aab10b6, 0x55ca396a,
  0x63e81440, 0x57489862, 0xaa55ab94, 0xe65525f3, 0x55605c60, 0x78af2fda,
  0xbd314b27, 0xd71577c1, 0xb01e8a3e, 0x6c9e0e8b, 0x603a180e, 0x8e79dcb0,
  0xb8db38ef, 0xca417918, 0x286085f0, 0xc5d1b023, 0x2af26013, 0x9c30d539,
  0xc25a59b5, 0x7b54a41d, 0x82154aee, 0x718bcd58, 0x728eb658, 0x0d95748f,
  0xf4933d7e, 0xa458fea3, 0x71574e69, 0x636920d8, 0x858efc16, 0x0801f2e2,
  0xb3916cf7, 0x24a19947, 0xf12c7f99, 0xba7c9045, 0x6a267e96, 0xb8e1afed,
  0xd01adfb7, 0x2ffd72db, 0x98dfb5ac, 0xd1310ba6, 0x8979fb1b, 0x9216d5d9,
  0xb5470917, 0x3f84d5b5, 0xc97c50dd, 0xc0ac29b7, 0x34e90c6c, 0xbe5466cf,
  0x38d01377, 0x452821e6, 0xec4e6c89, 0x082efa98, 0x299f31d0, 0xa4093822,
  0x03707344, 0x13198a2e, 0x85a308d3, 0x243f6a88, 0x00000003,
};

/**
 * @brief Enclose pi at the scale 2^-S in [lo, hi]
 * @details Up to #PI_TABLE_BITS, lo is the table shifted down, the floor of
 * a floor being the floor, and hi is lo + 1; beyond, Machin's formula
 * pi = 16 atan(1/5) - 4 atan(1/239)
 */
static void
fix_pi(uint32_t *lo, uint32_t *hi, int L, int S)
{
  if (S <= PI_TABLE_BITS)
  {
    int n = PI_TABLE_BITS / 32 + 1;
    uint32_t *t = palloc0(Max(L, n) * sizeof(uint32_t));
    memcpy(t, PI_TABLE, n * sizeof(uint32_t));
    fix_shift_right(t, PI_TABLE_BITS - S, t, Max(L, n), false);
    memcpy(lo, t, L * sizeof(uint32_t));
    pfree(t);
    memcpy(hi, lo, L * sizeof(uint32_t));
    fix_increment(hi, L);
    return;
  }
  uint32_t *alo = palloc(L * sizeof(uint32_t));
  uint32_t *ahi = palloc(L * sizeof(uint32_t));
  uint32_t *blo = palloc(L * sizeof(uint32_t));
  uint32_t *bhi = palloc(L * sizeof(uint32_t));
  fix_atan_inv(5, alo, ahi, L, S);
  fix_atan_inv(239, blo, bhi, L, S);
  fix_mul_small(alo, 16, alo, L);
  fix_mul_small(ahi, 16, ahi, L);
  fix_mul_small(blo, 4, blo, L);
  fix_mul_small(bhi, 4, bhi, L);
  fix_sub_clamp(alo, bhi, lo, L);
  fix_sub_clamp(ahi, blo, hi, L);
  pfree(alo); pfree(ahi); pfree(blo); pfree(bhi);
}

/**
 * @brief Enclose the sine or the cosine of @p a degrees, 0 < @p a <= 45, at
 * the scale 2^-S in [lo, hi]
 * @details The radians x = a pi / 180 are read from the enclosure of pi and
 * the exact double @p a; the Taylor series of the sine (x - x^3/3! + ...) and
 * of the cosine (1 - x^2/2! + ...) alternate with decreasing terms as x < 1,
 * so the sum stops at a term of at most one unit, which bounds the rest
 */
static void
fix_trig(double a, bool is_sin, uint32_t *lo, uint32_t *hi, int L, int S)
{
  uint32_t *xlo = palloc(L * sizeof(uint32_t));
  uint32_t *xhi = palloc(L * sizeof(uint32_t));
  uint32_t *x2lo = palloc(L * sizeof(uint32_t));
  uint32_t *x2hi = palloc(L * sizeof(uint32_t));
  uint32_t *tlo = palloc(L * sizeof(uint32_t));
  uint32_t *thi = palloc(L * sizeof(uint32_t));

  /* a = m 2^e exactly, m an integer of 53 bits */
  int e;
  double frac = frexp(a, &e);
  uint64_t m = (uint64_t) ldexp(frac, 53);
  e -= 53;
  assert(e < 0);
  /* x = m pi 2^e / 180, rounded outward */
  fix_pi(xlo, xhi, L, S);
  fix_mul_u64(xlo, m, L);
  fix_mul_u64(xhi, m, L);
  fix_shift_right(xlo, - e, xlo, L, false);
  fix_shift_right(xhi, - e, xhi, L, true);
  fix_div_small(xlo, 180, xlo, L, false);
  fix_div_small(xhi, 180, xhi, L, true);
  fix_mul(xlo, xlo, x2lo, L, S, false);
  fix_mul(xhi, xhi, x2hi, L, S, true);

  /* The first term: x for the sine, 1 for the cosine */
  if (is_sin)
  {
    memcpy(tlo, xlo, L * sizeof(uint32_t));
    memcpy(thi, xhi, L * sizeof(uint32_t));
  }
  else
  {
    fix_set_pow2(tlo, L, S);
    fix_set_pow2(thi, L, S);
  }
  memcpy(lo, tlo, L * sizeof(uint32_t));
  memcpy(hi, thi, L * sizeof(uint32_t));
  for (uint32_t k = 1; ; k++)
  {
    /* The next term: the last times x^2 / ((2k)(2k+1)) or ((2k-1)(2k)) */
    uint32_t d1 = is_sin ? 2 * k : 2 * k - 1, d = d1 * (d1 + 1);
    fix_mul(tlo, x2lo, tlo, L, S, false);
    fix_div_small(tlo, d, tlo, L, false);
    fix_mul(thi, x2hi, thi, L, S, true);
    fix_div_small(thi, d, thi, L, true);
    bool last = thi[0] <= 1;
    for (int i = 1; i < L && last; i++)
      last = thi[i] == 0;
    if (last)
    {
      /* The rest is less than this term */
      fix_sub_clamp(lo, thi, lo, L);
      fix_add(hi, thi, hi, L);
      break;
    }
    if (k % 2)
    {
      fix_sub_clamp(lo, thi, lo, L);
      fix_sub_clamp(hi, tlo, hi, L);
    }
    else
    {
      fix_add(lo, tlo, lo, L);
      fix_add(hi, thi, hi, L);
    }
  }
  pfree(xlo); pfree(xhi); pfree(x2lo); pfree(x2hi);
  pfree(tlo); pfree(thi);
}

/*****************************************************************************
 * The reduction and the products
 *****************************************************************************/

/* A factor of a product: the sine or the cosine of degrees, reduced */
typedef struct
{
  int sign;         /**< -1 or 1 */
  bool is_sin;      /**< Sine (true) or cosine (false) of @p a */
  double a;         /**< Degrees in [0, 45] */
} TrigFactor;

/**
 * @brief Reduce the sine or the cosine of @p deg degrees to the sine or the
 * cosine of an angle in [0, 45], every step exact
 * @details fmod by 360 is exact; 360 - r, 180 - r and 90 - r are exact as r
 * lies within a factor 2 of the subtrahend (Sterbenz)
 */
static void
trig_reduce(double deg, bool is_sin, TrigFactor *f)
{
  f->sign = 1;
  f->is_sin = is_sin;
  double r = fmod(deg, 360.0);
  if (r < 0)
  {
    r = - r;
    if (is_sin)
      f->sign = - f->sign;
  }
  if (r > 180.0)
  {
    r = 360.0 - r;
    if (is_sin)
      f->sign = - f->sign;
  }
  if (r > 90.0)
  {
    r = 180.0 - r;
    if (! is_sin)
      f->sign = - f->sign;
  }
  if (r > 45.0)
  {
    r = 90.0 - r;
    f->is_sin = ! f->is_sin;
  }
  f->a = r;
}

/**
 * @brief Return in @p num the value of a reduced factor where it is rational
 * (Niven: 0, 1/2 or 1 on [0, 45]) and true, else false
 */
static bool
trig_factor_exact(const TrigFactor *f, double *num)
{
  if (f->a == 0.0)
  {
    *num = f->is_sin ? 0.0 : 1.0;
    return true;
  }
  if (f->a == 30.0 && f->is_sin)
  {
    *num = 0.5;
    return true;
  }
  return false;
}

/* The product a side function reads: its factors and its current enclosure */
typedef struct
{
  int nfactors;           /**< Number of irrational factors, 1 or 2 */
  TrigFactor f[2];        /**< Those factors */
  int sign;               /**< Sign of the product */
  int halves;             /**< Number of rational factors 1/2 */
  int bits;               /**< Precision of the enclosure */
  int exp;                /**< Weight of the lowest bit of the bounds */
  int n64;                /**< Number of 64-bit limbs of each bound */
  uint64 *lo64, *hi64;    /**< The bounds of the magnitude */
} TrigProduct;

/**
 * @brief Enclose the magnitude of a product at a precision of @p bits
 */
static void
trig_product_enclose(TrigProduct *p, int bits)
{
  /* The scale keeps @p bits below the leading bit of the smallest angle */
  int lead = 0;
  for (int i = 0; i < p->nfactors; i++)
  {
    int e = ilogb(p->f[i].a);
    if (- e > lead)
      lead = - e;
  }
  int S = ((bits + lead + 64 + 31) / 32) * 32;
  int L = S / 32 + 4;
  uint32_t *lo = palloc(L * sizeof(uint32_t));
  uint32_t *hi = palloc(L * sizeof(uint32_t));
  fix_trig(p->f[0].a, p->f[0].is_sin, lo, hi, L, S);
  if (p->nfactors == 2)
  {
    uint32_t *lo2 = palloc(L * sizeof(uint32_t));
    uint32_t *hi2 = palloc(L * sizeof(uint32_t));
    fix_trig(p->f[1].a, p->f[1].is_sin, lo2, hi2, L, S);
    fix_mul(lo, lo2, lo, L, S, false);
    fix_mul(hi, hi2, hi, L, S, true);
    pfree(lo2); pfree(hi2);
  }
  int n64 = (L + 1) / 2;
  if (p->lo64)
  {
    pfree(p->lo64);
    pfree(p->hi64);
  }
  p->lo64 = palloc0(n64 * sizeof(uint64));
  p->hi64 = palloc0(n64 * sizeof(uint64));
  for (int i = 0; i < L; i++)
  {
    p->lo64[i / 2] |= (uint64) lo[i] << (32 * (i % 2));
    p->hi64[i / 2] |= (uint64) hi[i] << (32 * (i % 2));
  }
  pfree(lo); pfree(hi);
  p->bits = bits;
  /* A rational factor 1/2 moves the weight of the lowest bit */
  p->exp = - S - p->halves;
  p->n64 = n64;
}

/**
 * @brief Return the sign of a product of sines and cosines less @p mid,
 * raising the precision of its enclosure until it lies clear of @p mid
 * @note The product is irrational, so it is never the dyadic @p mid
 */
static int
trig_side(const WideDyadic *mid, const void *arg)
{
  TrigProduct *p = *(TrigProduct * const *) arg;
  while (true)
  {
    /* The product lies between sign * lo and sign * hi */
    int clo = wide_dyadic_cmp_fixed(mid, p->sign, p->lo64, p->n64, p->exp);
    int chi = wide_dyadic_cmp_fixed(mid, p->sign, p->hi64, p->n64, p->exp);
    if (p->sign > 0)
    {
      if (clo < 0)
        return 1;
      if (chi > 0)
        return -1;
    }
    else
    {
      if (chi < 0)
        return 1;
      if (clo > 0)
        return -1;
    }
    if (p->bits >= TRIG_LAST_BITS)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "The nearest double of a sine or a cosine is undecided");
      return 0;
    }
    trig_product_enclose(p, 2 * p->bits);
  }
}

/**
 * @brief Return the double nearest the product of the sines and cosines of
 * the degrees of @p deg, @p is_sin telling which of each
 * @param[in] deg,is_sin The factors
 * @param[in] n Number of factors, 1 or 2
 */
static double
trig_product_nearest(const double *deg, const bool *is_sin, int n)
{
  TrigProduct p;
  memset(&p, 0, sizeof(p));
  p.sign = 1;
  double exact = 1.0, approx = 1.0;
  for (int i = 0; i < n; i++)
  {
    if (! isfinite(deg[i]))
      return NAN;
    TrigFactor f;
    trig_reduce(deg[i], is_sin[i], &f);
    p.sign *= f.sign;
    double num;
    if (trig_factor_exact(&f, &num))
    {
      exact *= num;
      if (num == 0.5)
        p.halves++;
      continue;
    }
    p.f[p.nfactors++] = f;
    double rad = f.a * (M_PI / 180.0);
    approx *= f.is_sin ? sin(rad) : cos(rad);
  }
  /* A product of 0, 1/2 and 1 is a double; the value 0 is +0 */
  if (exact == 0.0)
    return 0.0;
  if (p.nfactors == 0)
    return p.sign * exact;
  trig_product_enclose(&p, TRIG_FIRST_BITS);
  TrigProduct *pp = &p;
  double result = nearest_double_exact(p.sign * exact * approx, trig_side,
    &pp);
  pfree(p.lo64);
  pfree(p.hi64);
  return result;
}

/*****************************************************************************
 * The angle between two exact vectors
 *****************************************************************************/

/* The precision, in bits of an angle, the comparison of a crossing fraction
 * starts from, and the one beyond which it answers undecided; multiples of
 * 32, as #fix_mul shifts by whole limbs */
#define ANGLE_FIRST_BITS 128
#define ANGLE_LAST_BITS (1 << 14)

/* -1, 0 or 1 as a < b, a = b or a > b */
static int
fix_cmp(const uint32_t *a, const uint32_t *b, int L)
{
  for (int i = L - 1; i >= 0; i--)
    if (a[i] != b[i])
      return a[i] < b[i] ? -1 : 1;
  return 0;
}

/* Index of the highest set bit, -1 for 0 */
static int
fix_top_bit(const uint32_t *a, int L)
{
  for (int i = L - 1; i >= 0; i--)
    if (a[i])
    {
      int b = 31;
      while (! ((a[i] >> b) & 1))
        b--;
      return 32 * i + b;
    }
  return -1;
}

static bool
fix_get_bit(const uint32_t *a, int k)
{
  return (a[k / 32] >> (k % 32)) & 1;
}

static void
fix_set_bit(uint32_t *a, int k)
{
  a[k / 32] |= (uint32_t) 1 << (k % 32);
}

/* a = 2 a; the callers keep a below 2^(32 L - 1) */
static void
fix_shl1(uint32_t *a, int L)
{
  for (int i = L - 1; i > 0; i--)
    a[i] = (a[i] << 1) | (a[i - 1] >> 31);
  a[0] <<= 1;
}

/* r = floor(sqrt(a)), or its ceiling where @p up, by the digit-by-digit
 * method on exact integers; r may be a */
static void
fix_isqrt(const uint32_t *a, uint32_t *r, int L, bool up)
{
  uint32_t *rem = palloc(L * sizeof(uint32_t));
  uint32_t *bit = palloc0(L * sizeof(uint32_t));
  uint32_t *t = palloc(L * sizeof(uint32_t));
  memcpy(rem, a, L * sizeof(uint32_t));
  memset(r, 0, L * sizeof(uint32_t));
  int top = fix_top_bit(rem, L);
  if (top >= 0)
  {
    fix_set_bit(bit, top & ~1);
    while (! fix_is_zero(bit, L))
    {
      fix_add(r, bit, t, L);
      if (fix_cmp(rem, t, L) >= 0)
      {
        fix_sub_clamp(rem, t, rem, L);
        fix_shift_right(r, 1, r, L, false);
        fix_add(r, bit, r, L);
      }
      else
        fix_shift_right(r, 1, r, L, false);
      fix_shift_right(bit, 2, bit, L, false);
    }
    if (up && ! fix_is_zero(rem, L))
      fix_increment(r, L);
  }
  pfree(rem); pfree(bit); pfree(t);
}

/* q = floor(a 2^k / d), or its ceiling where @p up, by binary long division
 * on exact integers, d not 0; q may be a or d */
static void
fix_div_shift(const uint32_t *a, int k, const uint32_t *d, uint32_t *q,
  int L, bool up)
{
  uint32_t *rem = palloc0(L * sizeof(uint32_t));
  uint32_t *res = palloc0(L * sizeof(uint32_t));
  int top = fix_top_bit(a, L);
  /* The bits of a 2^k, from the highest: those of a, then k zeros */
  for (int i = top + k; i >= 0; i--)
  {
    fix_shl1(rem, L);
    if (i >= k && fix_get_bit(a, i - k))
      rem[0] |= 1;
    if (fix_cmp(rem, d, L) >= 0)
    {
      fix_sub_clamp(rem, d, rem, L);
      fix_set_bit(res, i);
    }
  }
  if (up && ! fix_is_zero(rem, L))
    fix_increment(res, L);
  memcpy(q, res, L * sizeof(uint32_t));
  pfree(rem); pfree(res);
}

/**
 * @brief Bound atan(t), 0 <= t <= 1 at the scale 2^-K, below or, where @p up,
 * above, K a multiple of 32
 * @details Two halvings, t / (1 + sqrt(1 + t^2)) each, an increasing
 * function read rounded in the direction of the bound, bring t below
 * tan(pi / 16); the series of the arctangent alternates with decreasing
 * terms there, each power carried in two chains, one rounded up and one
 * down, so every term is bounded in the direction its sign needs; it stops
 * at a term of at most one unit, which bounds the rest; the result is four
 * times that sum
 */
static void
fix_atan_bound(const uint32_t *t, uint32_t *r, int L, int K, bool up)
{
  uint32_t *u = palloc(L * sizeof(uint32_t));
  uint32_t *w = palloc(L * sizeof(uint32_t));
  uint32_t *one = palloc(L * sizeof(uint32_t));
  uint32_t *pup = palloc(L * sizeof(uint32_t));
  uint32_t *pdn = palloc(L * sizeof(uint32_t));
  uint32_t *u2up = palloc(L * sizeof(uint32_t));
  uint32_t *u2dn = palloc(L * sizeof(uint32_t));
  memcpy(u, t, L * sizeof(uint32_t));
  for (int h = 0; h < 2; h++)
  {
    /* w = 2^(2K) + u^2 exactly, its square root at the scale 2^-K rounded
     * against the bound, so the denominator moves the quotient its way */
    fix_mul(u, u, w, L, 0, false);
    fix_set_pow2(one, L, 2 * K);
    fix_add(w, one, w, L);
    fix_isqrt(w, w, L, ! up);
    fix_set_pow2(one, L, K);
    fix_add(w, one, w, L);
    fix_div_shift(u, K, w, u, L, up);
  }
  /* The series u - u^3/3 + u^5/5 - ... */
  fix_mul(u, u, u2up, L, K, true);
  fix_mul(u, u, u2dn, L, K, false);
  memcpy(pup, u, L * sizeof(uint32_t));
  memcpy(pdn, u, L * sizeof(uint32_t));
  memset(r, 0, L * sizeof(uint32_t));
  for (uint32_t n = 0; ; n++)
  {
    /* The term is added where n is even, subtracted where odd: an upper
     * bound adds the larger of a positive term, subtracts the smaller of a
     * negative one, and a lower bound the reverse */
    bool large = (n % 2 == 0) == up;
    fix_div_small(large ? pup : pdn, 2 * n + 1, w, L, large);
    uint32_t *wmax = palloc(L * sizeof(uint32_t));
    fix_div_small(pup, 2 * n + 1, wmax, L, true);
    bool last = fix_top_bit(wmax, L) <= 0;
    pfree(wmax);
    if (last)
    {
      /* The rest, this term included, is at most one unit */
      fix_set_pow2(one, L, 0);
      if (up)
        fix_add(r, one, r, L);
      else
        fix_sub_clamp(r, one, r, L);
      break;
    }
    if (n % 2 == 0)
      fix_add(r, w, r, L);
    else
      fix_sub_clamp(r, w, r, L);
    fix_mul(pup, u2up, pup, L, K, true);
    fix_mul(pdn, u2dn, pdn, L, K, false);
  }
  fix_mul_small(r, 4, r, L);
  pfree(u); pfree(w); pfree(one); pfree(pup); pfree(pdn);
  pfree(u2up); pfree(u2dn);
}

/* Bound atan2(y, x), y, x >= 0 at the scale 2^-K, x and y not both 0, below
 * or, where @p up, above */
static void
fix_atan2_bound(const uint32_t *y, const uint32_t *x, uint32_t *r, int L,
  int K, bool up)
{
  uint32_t *t = palloc(L * sizeof(uint32_t));
  if (fix_is_zero(y, L))
    memset(r, 0, L * sizeof(uint32_t));
  else if (fix_cmp(y, x, L) <= 0)
  {
    fix_div_shift(y, K, x, t, L, up);
    fix_atan_bound(t, r, L, K, up);
  }
  else
  {
    /* pi/2 - atan(x / y), pi/2 at the scale 2^-K being pi at 2^-(K-1) */
    uint32_t *plo = palloc(L * sizeof(uint32_t));
    uint32_t *phi = palloc(L * sizeof(uint32_t));
    fix_div_shift(x, K, y, t, L, ! up);
    fix_atan_bound(t, r, L, K, ! up);
    fix_pi(plo, phi, L, K - 1);
    fix_sub_clamp(up ? phi : plo, r, r, L);
    pfree(plo); pfree(phi);
  }
  pfree(t);
}

static int wide_sign(const WideDyadic *a);

/**
 * @brief Return floor(log2 |a|) of a nonzero #WideDyadic, read through
 * #wide_dyadic_to_fixed into one limb: |a| 2^s fits in 64 bits for every s
 * up to 63 - floor(log2 |a|)
 */
static int
wide_log2(const WideDyadic *a)
{
  uint64 m;
  int lo = -40000, hi = 40000;   /* |a| 2^lo fits, |a| 2^hi does not */
  while (hi - lo > 1)
  {
    int mid = lo + (hi - lo) / 2;
    if (wide_dyadic_to_fixed(a, mid, false, &m, 1) == 2)
      hi = mid;
    else
      lo = mid;
  }
  return 63 - lo;
}

/**
 * @brief Enclose atan2(sqrt(s), c) at the scale 2^-K in [lo, hi], for exact
 * s >= 0 and c, not both 0
 * @details The pair is scaled by a power of 2, which the angle does not see,
 * so the larger of sqrt(s) and |c| lies below 2^(K-1) at the scale 2^-K;
 * sqrt(s) is the integer square root of s read at twice that scale, every
 * bound rounded outward
 */
static void
fix_atan2_enclose(const WideDyadic *s, const WideDyadic *c, uint32_t *lo,
  uint32_t *hi, int L, int K)
{
  int ss = wide_sign(s), sc = wide_sign(c);
  int es = ss ? wide_log2(s) : INT_MIN / 4;
  int ec = sc ? wide_log2(c) : INT_MIN / 4;
  int E = Max((es + 1) / 2, ec);
  int scale = K - 3 - E;
  int n64 = (L + 1) / 2;
  uint64 *buf = palloc(n64 * sizeof(uint64));
  uint32_t *v[4];
  for (int k = 0; k < 4; k++)
  {
    /* 0: s down, 1: s up, 2: |c| down, 3: |c| up */
    v[k] = palloc(L * sizeof(uint32_t));
    int sg = wide_dyadic_to_fixed(k < 2 ? s : c, k < 2 ? 2 * scale : scale,
      k % 2 == 1, buf, n64);
    assert(sg != 2);
    if (k >= 2)
      sc = sg;
    for (int i = 0; i < L; i++)
      v[k][i] = (uint32_t) (buf[i / 2] >> (32 * (i % 2)));
  }
  fix_isqrt(v[0], v[0], L, false);
  fix_isqrt(v[1], v[1], L, true);
  if (sc >= 0)
  {
    /* atan(y / x): increasing in y, decreasing in x */
    fix_atan2_bound(v[0], v[3], lo, L, K, false);
    fix_atan2_bound(v[1], v[2], hi, L, K, true);
  }
  else
  {
    /* pi - atan(y / |x|): decreasing in y, increasing in |x| */
    uint32_t *pl = palloc(L * sizeof(uint32_t));
    uint32_t *ph = palloc(L * sizeof(uint32_t));
    uint32_t *a = palloc(L * sizeof(uint32_t));
    fix_pi(pl, ph, L, K);
    fix_atan2_bound(v[1], v[2], a, L, K, true);
    fix_sub_clamp(pl, a, lo, L);
    fix_atan2_bound(v[0], v[3], a, L, K, false);
    fix_sub_clamp(ph, a, hi, L);
    pfree(pl); pfree(ph); pfree(a);
  }
  pfree(buf);
  for (int k = 0; k < 4; k++)
    pfree(v[k]);
}

/* The exact cross product and dot product of #WideDyadic vectors */
static void
wide_cross(WideDyadic *const *a, WideDyadic *const *b, WideDyadic **r)
{
  WideDyadic *t1 = wide_dyadic_alloc(), *t2 = wide_dyadic_alloc();
  for (int i = 0; i < 3; i++)
  {
    int j = (i + 1) % 3, k = (i + 2) % 3;
    wide_dyadic_mul(a[j], b[k], t1);
    wide_dyadic_mul(a[k], b[j], t2);
    wide_dyadic_sub(t1, t2, r[i]);
  }
  pfree(t1); pfree(t2);
}

static void
wide_dot(WideDyadic *const *a, WideDyadic *const *b, WideDyadic *r)
{
  WideDyadic *t = wide_dyadic_alloc(), *u = wide_dyadic_alloc();
  WideDyadic *v = wide_dyadic_alloc();
  wide_dyadic_mul(a[0], b[0], u);
  wide_dyadic_mul(a[1], b[1], t);
  wide_dyadic_add(u, t, v);
  wide_dyadic_mul(a[2], b[2], t);
  wide_dyadic_add(v, t, r);
  pfree(t); pfree(u); pfree(v);
}

/* The sign of an exact value: at a scale far below its lowest bit every bit
 * is dropped, so #wide_dyadic_to_fixed answers the sign alone */
static int
wide_sign(const WideDyadic *a)
{
  uint64 probe;
  return wide_dyadic_to_fixed(a, -100000, false, &probe, 1);
}

static bool
wide_vec_is_zero(WideDyadic *const *a)
{
  return wide_sign(a[0]) == 0 && wide_sign(a[1]) == 0 && wide_sign(a[2]) == 0;
}

static WideDyadic **
wide_vec_alloc(void)
{
  WideDyadic **v = palloc(3 * sizeof(WideDyadic *));
  for (int i = 0; i < 3; i++)
    v[i] = wide_dyadic_alloc();
  return v;
}

static void
wide_vec_free(WideDyadic **v)
{
  for (int i = 0; i < 3; i++)
    pfree(v[i]);
  pfree(v);
}

/* sign(f - num/den) for a rational f = fn/fd given by small integers */
static int
fraction_cmp_rational(int64 fn, int64 fd, int64 num, int64 den)
{
  /* fn/fd - num/den: fn and fd at most 2, so fn den and num fd fit */
  int64 l = fn * den, r = num * fd;
  return (l > r) - (l < r);
}

/**
 * @brief Return the sign of f - num/den, f the fraction of the great-circle
 * arc from @p p0 to @p p1 at which it crosses the great circle of @p a and
 * @p b, read at constant angular speed, -1, 0 or 1; 2 where undecided
 * @details f = angle(p0, x) / angle(p0, p1), x the direction (p0 x p1) x
 * (a x b) or its opposite, whichever lies ahead of p0 on the arc, each angle
 * atan2(|u x v|, u . v) of exact vectors: every product is read exactly in
 * #WideDyadic and each angle enclosed (#fix_atan2_enclose) at a precision
 * raised until f - num/den lies clear of 0. It is 0 exactly where x lies
 * along p0 (f = 0), along p1 (f = 1), or along p0 + p1 with |p0| = |p1|
 * (f = 1/2), as the geodetic oracle of the cell walks decides a crossing
 * @param[in] p0,p1 The geodetic positions of the ends of the arc, D6 vectors
 * (#lonlat_vector_nearest), neither parallel nor antipodal
 * @param[in] a,b Two points of the great circle crossed, not parallel
 * @param[in] num,den The rational, den > 0
 */
int
geodetic_arc_fraction_cmp(const double *p0, const double *p1,
  const double *a, const double *b, int64 num, int64 den)
{
  WideDyadic **P0 = wide_vec_alloc(), **P1 = wide_vec_alloc();
  WideDyadic **A = wide_vec_alloc(), **B = wide_vec_alloc();
  for (int i = 0; i < 3; i++)
  {
    wide_dyadic_set_double(P0[i], p0[i]);
    wide_dyadic_set_double(P1[i], p1[i]);
    wide_dyadic_set_double(A[i], a[i]);
    wide_dyadic_set_double(B[i], b[i]);
  }
  WideDyadic **M = wide_vec_alloc(), **N = wide_vec_alloc();
  WideDyadic **X = wide_vec_alloc(), **C = wide_vec_alloc();
  WideDyadic *s1 = wide_dyadic_alloc(), *c1 = wide_dyadic_alloc();
  WideDyadic *s2 = wide_dyadic_alloc(), *c2 = wide_dyadic_alloc();
  WideDyadic *t = wide_dyadic_alloc();
  wide_cross(P0, P1, M);
  wide_cross(A, B, N);
  wide_cross(M, N, X);
  /* x ahead of p0: (p0 x x) . m > 0 */
  wide_cross(P0, X, C);
  wide_dot(C, M, t);
  if (wide_sign(t) < 0)
  {
    /* -x: 0 - x into a fresh value, as a result is never an operand */
    WideDyadic *zero = wide_dyadic_alloc();
    for (int i = 0; i < 3; i++)
    {
      WideDyadic *neg = wide_dyadic_alloc();
      wide_dyadic_sub(zero, X[i], neg);
      pfree(X[i]);
      X[i] = neg;
    }
    pfree(zero);
  }
  int result = 2;
  /* The exact fractions */
  WideDyadic **Y = wide_vec_alloc();
  wide_cross(X, P0, Y);
  bool at0 = wide_vec_is_zero(Y);
  wide_cross(X, P1, Y);
  bool at1 = wide_vec_is_zero(Y);
  WideDyadic **S = wide_vec_alloc();
  for (int i = 0; i < 3; i++)
    wide_dyadic_add(P0[i], P1[i], S[i]);
  wide_cross(X, S, Y);
  wide_dot(P0, P0, s1);
  wide_dot(P1, P1, s2);
  wide_dyadic_sub(s1, s2, t);
  bool athalf = wide_vec_is_zero(Y) && wide_sign(t) == 0;
  if (at0)
    result = fraction_cmp_rational(0, 1, num, den);
  else if (at1)
    result = fraction_cmp_rational(1, 1, num, den);
  else if (athalf)
    result = fraction_cmp_rational(1, 2, num, den);
  else
  {
    /* theta = atan2(|m|, p0 . p1), phi = atan2(|p0 x x|, p0 . x) */
    wide_dot(M, M, s1);
    wide_dot(P0, P1, c1);
    wide_cross(P0, X, C);
    wide_dot(C, C, s2);
    wide_dot(P0, X, c2);
    for (int K = ANGLE_FIRST_BITS; K <= ANGLE_LAST_BITS && result == 2;
        K *= 2)
    {
      int L = (2 * K + 192) / 32;
      uint32_t *tl = palloc(L * sizeof(uint32_t)), *th = palloc(L * sizeof(uint32_t));
      uint32_t *pl = palloc(L * sizeof(uint32_t)), *ph = palloc(L * sizeof(uint32_t));
      uint32_t *lhs = palloc(L * sizeof(uint32_t)), *rhs = palloc(L * sizeof(uint32_t));
      fix_atan2_enclose(s1, c1, tl, th, L, K);
      fix_atan2_enclose(s2, c2, pl, ph, L, K);
      /* f > r where phi den > theta num at every point: pl den > th num;
       * f < r where ph den < tl num */
      memcpy(lhs, pl, L * sizeof(uint32_t)); fix_mul_u64(lhs, (uint64) den, L);
      memcpy(rhs, th, L * sizeof(uint32_t)); fix_mul_u64(rhs, (uint64) num, L);
      if (fix_cmp(lhs, rhs, L) > 0)
        result = 1;
      else
      {
        memcpy(lhs, ph, L * sizeof(uint32_t)); fix_mul_u64(lhs, (uint64) den, L);
        memcpy(rhs, tl, L * sizeof(uint32_t)); fix_mul_u64(rhs, (uint64) num, L);
        if (fix_cmp(lhs, rhs, L) < 0)
          result = -1;
      }
      pfree(tl); pfree(th); pfree(pl); pfree(ph); pfree(lhs); pfree(rhs);
    }
  }
  wide_vec_free(P0); wide_vec_free(P1); wide_vec_free(A); wide_vec_free(B);
  wide_vec_free(M); wide_vec_free(N); wide_vec_free(X); wide_vec_free(C);
  wide_vec_free(Y); wide_vec_free(S);
  pfree(s1); pfree(c1); pfree(s2); pfree(c2); pfree(t);
  return result;
}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

/**
 * @brief Return the double nearest the sine of @p deg degrees
 */
double
sind_nearest(double deg)
{
  bool s = true;
  return trig_product_nearest(&deg, &s, 1);
}

/**
 * @brief Return the double nearest the cosine of @p deg degrees
 */
double
cosd_nearest(double deg)
{
  bool s = false;
  return trig_product_nearest(&deg, &s, 1);
}

/**
 * @brief Return in @p v the geodetic position of a longitude and a latitude
 * in degrees: the doubles nearest cos(lat) cos(lon), cos(lat) sin(lon) and
 * sin(lat), the same on every platform (decision D6 of the cell walks)
 */
void
lonlat_vector_nearest(double lon, double lat, double *v)
{
  const double deg[2] = {lat, lon};
  const bool cc[2] = {false, false}, cs[2] = {false, true};
  v[0] = trig_product_nearest(deg, cc, 2);
  v[1] = trig_product_nearest(deg, cs, 2);
  v[2] = sind_nearest(lat);
}
