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

/* floor(180 / pi 2^2048), 32-bit limbs, the lowest first, computed as
 * #PI_TABLE is, in mpmath at 2300 and 4000 bits, which agree, and lying in
 * the bracket #PI_TABLE gives; #degree_angle_enclose reads it */
#define DEG_TABLE_BITS 2048
static const uint32_t DEG_TABLE[DEG_TABLE_BITS / 32 + 1] = {
  0x5940b061, 0xfa6ef8e9, 0xa8ad6367, 0xa37fb02b, 0x81c03402, 0xd88227fb,
  0x525e3fa1, 0x5ba877d8, 0x9ecbd806, 0xef9e3198, 0x12c612a9, 0x2d8f11ad,
  0xa99338f2, 0x0eb27e5f, 0x5db73af1, 0x0f9f81b1, 0x2ca956cc, 0x1e238083,
  0x3a7bc73c, 0xebf21d1b, 0x4064a6b1, 0xb7f8e36d, 0x5ce580f3, 0xba011c3b,
  0x79db066d, 0xa9082376, 0xc363da72, 0x3d1ef8f4, 0x17b2b4ee, 0xf65fa445,
  0xa107a669, 0x748a6106, 0xeed6e553, 0x2ff7aa48, 0xadb853fb, 0xdeb24a13,
  0xfd616a71, 0x652894d3, 0xa7aa5d0c, 0x168c2afd, 0x0df7759a, 0x3d656ff4,
  0xc99824c9, 0x06899720, 0x2b425fff, 0x3d455038, 0x699262ee, 0x20ff6d9b,
  0x6dd616ba, 0xbdc6f832, 0x48a4cc1d, 0x5684a816, 0x73ad5e31, 0x35ba7e47,
  0x4f5c22c2, 0x62cec07b, 0x4e03645a, 0x689d0a6c, 0x2fc0b733, 0x20a897df,
  0x3495f5cd, 0xa5d4dfd0, 0x83ef70c2, 0x4bb834c7, 0x00000039,
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

/* The 64-bit limbs of an integer of @p L 32-bit limbs, the lowest first, in
 * `(L + 1) / 2` limbs, and their number, the highest not 0 */
static int
fix_to_limbs(const uint32_t *a, int L, uint64 *o)
{
  int n = (L + 1) / 2;
  for (int i = 0; i < n; i++)
    o[i] = (uint64) a[2 * i] |
      ((2 * i + 1 < L) ? (uint64) a[2 * i + 1] << 32 : 0);
  while (n > 0 && o[n - 1] == 0)
    n--;
  return n;
}

/* An integer of @p L 32-bit limbs from @p n 64-bit limbs, which it holds */
static void
fix_of_limbs(const uint64 *o, int n, uint32_t *a, int L)
{
  for (int i = 0; i < L; i++)
    a[i] = (i / 2 < n) ? (uint32_t) (o[i / 2] >> (32 * (i % 2))) : 0;
}

/* Number of 64-bit words of scratch the square root and the quotient take
 * on the stack, more being allocated */
#define FIX_LIMBS_STACK 192

/* r = floor(sqrt(a)), or its ceiling where @p up, on the limbs of the wide
 * number (#wide_limbs_isqrt); r may be a */
static void
fix_isqrt(const uint32_t *a, uint32_t *r, int L, bool up)
{
  int n64 = (L + 1) / 2;
  size_t need = (size_t) n64 + (size_t) (n64 / 2 + 2);
  uint64 sbuf[FIX_LIMBS_STACK];
  uint64 *buf = (need <= FIX_LIMBS_STACK) ? sbuf :
    palloc(sizeof(uint64) * need);
  uint64 *n = buf, *x = buf + n64;
  int nn = fix_to_limbs(a, L, n);
  bool exact;
  int nx = wide_limbs_isqrt(n, nn, x, &exact);
  /* The root of an integer below 2^(32 L) is below 2^(16 L), so one more
   * stays within the limbs */
  if (up && ! exact)
  {
    int i = 0;
    while (i < nx && ++x[i] == 0)
      i++;
    if (i == nx)
      x[nx++] = 1;
  }
  fix_of_limbs(x, nx, r, L);
  if (buf != sbuf)
    pfree(buf);
}

/* q = floor(a 2^k / d), or its ceiling where @p up, on the limbs of the wide
 * number (#wide_limbs_divmod), d not 0 and the quotient within the limbs;
 * q may be a or d */
static void
fix_div_shift(const uint32_t *a, int k, const uint32_t *d, uint32_t *q,
  int L, bool up)
{
  int n64 = (L + 1) / 2, ks = k / 64;
  int nu_cap = n64 + ks + 1;
  size_t need = 2 * (size_t) nu_cap + 2 * (size_t) n64 + 1;
  uint64 sbuf[FIX_LIMBS_STACK];
  uint64 *buf = (need <= FIX_LIMBS_STACK) ? sbuf :
    palloc(sizeof(uint64) * need);
  uint64 *u = buf, *dv = u + nu_cap, *qt = dv + n64, *rm = qt + nu_cap + 1;
  /* a 2^k: the limbs of a moved up by ks limbs and kb bits */
  memset(u, 0, sizeof(uint64) * (size_t) nu_cap);
  int na = fix_to_limbs(a, L, u + ks);
  int nu = 0;
  if (na)
  {
    nu = na + ks;
    int kb = k % 64;
    if (kb)
    {
      for (int i = nu; i > ks; i--)
        u[i] = (u[i] << kb) | (u[i - 1] >> (64 - kb));
      u[ks] <<= kb;
      nu++;
    }
  }
  int nd = fix_to_limbs(d, L, dv);
  int nq = 0, nr = 0;
  if (nu)
    nq = wide_limbs_divmod(u, nu, dv, nd, qt, rm, &nr);
  if (up && nr)
  {
    int i = 0;
    while (i < nq && ++qt[i] == 0)
      i++;
    if (i == nq)
      qt[nq++] = 1;
  }
  fix_of_limbs(qt, nq, q, L);
  if (buf != sbuf)
    pfree(buf);
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
 * s >= 0 and c, not both 0, or atan2(s, c) where @p root, s being then its
 * own square root
 * @details The pair is scaled by a power of 2, which the angle does not see,
 * so the larger of sqrt(s) and |c| lies below 2^(K-1) at the scale 2^-K;
 * sqrt(s) is the integer square root of s read at twice that scale, every
 * bound rounded outward, and s itself read at that scale where @p root
 */
static void
fix_atan2_enclose_root(const WideDyadic *s, bool root, const WideDyadic *c,
  uint32_t *lo, uint32_t *hi, int L, int K)
{
  int ss = wide_sign(s), sc = wide_sign(c);
  int es = ss ? wide_log2(s) : INT_MIN / 4;
  int ec = sc ? wide_log2(c) : INT_MIN / 4;
  int E = Max(root ? es : (es + 1) / 2, ec);
  int scale = K - 3 - E;
  int n64 = (L + 1) / 2;
  uint64 *buf = palloc(n64 * sizeof(uint64));
  uint32_t *v[4];
  for (int k = 0; k < 4; k++)
  {
    /* 0: s down, 1: s up, 2: |c| down, 3: |c| up */
    v[k] = palloc(L * sizeof(uint32_t));
    int sg = wide_dyadic_to_fixed(k < 2 ? s : c,
      k < 2 && ! root ? 2 * scale : scale, k % 2 == 1, buf, n64);
    assert(sg != 2);
    if (k >= 2)
      sc = sg;
    for (int i = 0; i < L; i++)
      v[k][i] = (uint32_t) (buf[i / 2] >> (32 * (i % 2)));
  }
  if (! root)
  {
    fix_isqrt(v[0], v[0], L, false);
    fix_isqrt(v[1], v[1], L, true);
  }
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

/**
 * @brief Enclose atan2(sqrt(s), c) at the scale 2^-K in [lo, hi], for exact
 * s >= 0 and c, not both 0 (#fix_atan2_enclose_root)
 */
static void
fix_atan2_enclose(const WideDyadic *s, const WideDyadic *c, uint32_t *lo,
  uint32_t *hi, int L, int K)
{
  fix_atan2_enclose_root(s, false, c, lo, hi, L, K);
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

/*****************************************************************************
 * Directions read exactly
 *
 * The cells of a geodetic trajectory are decided on directions in space: the
 * geodetic position of a vertex or of an instant (#lonlat_vector_nearest),
 * the normal of the great circle of an edge or of a segment, the direction in
 * which two great circles meet, and the sum of two directions, which lies
 * between them on their circle. Each is a vector of exact values
 * (#wide_cross, #wide_dot), so every sign read on it is exact.
 *****************************************************************************/

/** @brief A direction in space whose components are exact dyadic values */
struct SphereDir
{
  WideDyadic *c[3];
};

static SphereDir *
sphere_dir_alloc(void)
{
  SphereDir *r = palloc(sizeof(SphereDir));
  for (int i = 0; i < 3; i++)
    r->c[i] = wide_dyadic_alloc();
  return r;
}

/**
 * @brief Return the direction of a vector of doubles, read exactly
 */
SphereDir *
sphere_dir_make(const double *v)
{
  SphereDir *r = sphere_dir_alloc();
  for (int i = 0; i < 3; i++)
    wide_dyadic_set_double(r->c[i], v[i]);
  return r;
}

/**
 * @brief Return the cross product of two directions, exactly
 */
SphereDir *
sphere_dir_cross(const SphereDir *a, const SphereDir *b)
{
  SphereDir *r = sphere_dir_alloc();
  wide_cross(a->c, b->c, r->c);
  return r;
}

/**
 * @brief Return the sum of two directions, exactly
 */
SphereDir *
sphere_dir_add(const SphereDir *a, const SphereDir *b)
{
  SphereDir *r = sphere_dir_alloc();
  for (int i = 0; i < 3; i++)
    wide_dyadic_add(a->c[i], b->c[i], r->c[i]);
  return r;
}

/**
 * @brief Return the opposite of a direction, exactly
 */
SphereDir *
sphere_dir_neg(const SphereDir *a)
{
  SphereDir *r = sphere_dir_alloc();
  WideDyadic *zero = wide_dyadic_alloc();
  for (int i = 0; i < 3; i++)
    wide_dyadic_sub(zero, a->c[i], r->c[i]);
  pfree(zero);
  return r;
}

/**
 * @brief Free a direction
 */
void
sphere_dir_free(SphereDir *a)
{
  if (! a)
    return;
  for (int i = 0; i < 3; i++)
    pfree(a->c[i]);
  pfree(a);
}

/**
 * @brief Return the sign of the dot product of two directions
 */
int
sphere_dir_dot_sign(const SphereDir *a, const SphereDir *b)
{
  WideDyadic *t = wide_dyadic_alloc();
  wide_dot(a->c, b->c, t);
  int result = wide_sign(t);
  pfree(t);
  return result;
}

/**
 * @brief Return the sign of the triple product (a x b) . c
 */
int
sphere_dir_triple_sign(const SphereDir *a, const SphereDir *b,
  const SphereDir *c)
{
  SphereDir *n = sphere_dir_cross(a, b);
  int result = sphere_dir_dot_sign(n, c);
  sphere_dir_free(n);
  return result;
}

/**
 * @brief Return the sign of (a . b)(c . d) - (a . d)(c . b)
 * @details Where the dot products of a and c with d are positive, the sign
 * compares the ratios (a . b) / (a . d) and (c . b) / (c . d), the coordinates
 * along b of a and c in the gnomonic chart about d
 */
int
sphere_dir_dot_diff_sign(const SphereDir *a, const SphereDir *b,
  const SphereDir *c, const SphereDir *d)
{
  WideDyadic *ab = wide_dyadic_alloc(), *cd = wide_dyadic_alloc();
  WideDyadic *ad = wide_dyadic_alloc(), *cb = wide_dyadic_alloc();
  WideDyadic *l = wide_dyadic_alloc(), *r = wide_dyadic_alloc();
  WideDyadic *t = wide_dyadic_alloc();
  wide_dot(a->c, b->c, ab);
  wide_dot(c->c, d->c, cd);
  wide_dot(a->c, d->c, ad);
  wide_dot(c->c, b->c, cb);
  wide_dyadic_mul(ab, cd, l);
  wide_dyadic_mul(ad, cb, r);
  wide_dyadic_sub(l, r, t);
  int result = wide_sign(t);
  pfree(ab); pfree(cd); pfree(ad); pfree(cb); pfree(l); pfree(r); pfree(t);
  return result;
}

/**
 * @brief Return true if two directions lie on one line through the origin,
 * their cross product being exactly 0
 */
bool
sphere_dir_parallel(const SphereDir *a, const SphereDir *b)
{
  SphereDir *n = sphere_dir_cross(a, b);
  bool result = wide_vec_is_zero(n->c);
  sphere_dir_free(n);
  return result;
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

/*****************************************************************************
 * The degrees of an exact vector
 *
 * The inverse of #lonlat_vector_nearest: the longitude and the latitude of a
 * vector of integers, read exactly, as the doubles nearest their degrees. A
 * corner of an S2 cell is such a vector (decision D7 of the cell walks), so
 * every platform reads the same corner.
 *****************************************************************************/

/* The magnitude of an int64 */
static uint64
int64_magnitude(int64 x)
{
  return x < 0 ? (uint64) 0 - (uint64) x : (uint64) x;
}

/* Read the magnitude @p m exactly, negated where @p neg: two parts of at
 * most 32 bits, each a double, the sign applied last; a result is never an
 * operand */
static void
wide_set_magnitude(WideDyadic *r, uint64 m, bool neg)
{
  WideDyadic *h = wide_dyadic_alloc(), *w = wide_dyadic_alloc();
  WideDyadic *t = wide_dyadic_alloc(), *l = wide_dyadic_alloc();
  WideDyadic *zero = wide_dyadic_alloc();
  wide_dyadic_set_double(h, (double) (m >> 32));
  wide_dyadic_set_double(w, 4294967296.0);
  wide_dyadic_mul(h, w, t);
  wide_dyadic_set_double(l, (double) (m & UINT64CONST(0xffffffff)));
  if (neg)
  {
    WideDyadic *a = wide_dyadic_alloc();
    wide_dyadic_add(t, l, a);
    wide_dyadic_sub(zero, a, r);
    pfree(a);
  }
  else
    wide_dyadic_add(t, l, r);
  pfree(h); pfree(w); pfree(t); pfree(l); pfree(zero);
}

/* Read an int64 exactly (#wide_set_magnitude) */
static void
wide_set_int64(WideDyadic *r, int64 x)
{
  wide_set_magnitude(r, int64_magnitude(x), x < 0);
}

/** @brief The degrees of an angle atan2(sqrt(s), c) in [0, 180], or 90 less
 * it, and an enclosure of their magnitude, the twin of #TrigProduct */
typedef struct
{
  const WideDyadic *s, *c;  /**< The angle atan2(sqrt(s), c) */
  bool root;                /**< The angle is atan2(s, c) */
  bool complement;          /**< The degrees are 90 less the angle */
  int sign;                 /**< Sign of the degrees, -1 or 1 */
  int bits;                 /**< Precision of the enclosure */
  int exp;                  /**< Weight of the lowest bit of the bounds */
  int n64;                  /**< Number of 64-bit limbs of each bound */
  uint64 *lo64, *hi64;      /**< The bounds of the magnitude */
} DegreeAngle;

/**
 * @brief Enclose the magnitude of the degrees of an angle at a precision of
 * @p bits, the twin of #trig_product_enclose
 * @details The angle in radians is enclosed by #fix_atan2_enclose_root, and
 * its degrees are the radians times 180 over pi, each bound multiplied by
 * the bound of 180 / pi of #DEG_TABLE that moves it outward, or, beyond the
 * table, divided by the bound of pi (#fix_pi) that does
 */
static void
degree_angle_enclose(DegreeAngle *p, int bits)
{
  /* The divisions by pi shift by K bits, so the limbs hold twice K, as the
   * enclosures of #geodetic_arc_fraction_cmp size them */
  int K = ((bits + 31) / 32) * 32 + 32;
  int L = (2 * K + 192) / 32;
  uint32_t *tlo = palloc(L * sizeof(uint32_t));
  uint32_t *thi = palloc(L * sizeof(uint32_t));
  uint32_t *plo = palloc(L * sizeof(uint32_t));
  uint32_t *phi = palloc(L * sizeof(uint32_t));
  uint32_t *dlo = palloc(L * sizeof(uint32_t));
  uint32_t *dhi = palloc(L * sizeof(uint32_t));
  fix_atan2_enclose_root(p->s, p->root, p->c, tlo, thi, L, K);
  if (K <= DEG_TABLE_BITS)
  {
    /* 180 / pi at the scale 2^-K: the table shifted down, the floor of a
     * floor being the floor, below it, and that floor plus one above it */
    int n = DEG_TABLE_BITS / 32 + 1;
    uint32_t *t = palloc0(Max(L, n) * sizeof(uint32_t));
    memcpy(t, DEG_TABLE, n * sizeof(uint32_t));
    fix_shift_right(t, DEG_TABLE_BITS - K, t, Max(L, n), false);
    memcpy(plo, t, L * sizeof(uint32_t));
    pfree(t);
    memcpy(phi, plo, L * sizeof(uint32_t));
    fix_increment(phi, L);
    fix_mul(tlo, plo, dlo, L, K, false);
    fix_mul(thi, phi, dhi, L, K, true);
  }
  else
  {
    fix_pi(plo, phi, L, K);
    fix_mul_small(tlo, 180, tlo, L);
    fix_mul_small(thi, 180, thi, L);
    fix_div_shift(tlo, K, phi, dlo, L, false);
    fix_div_shift(thi, K, plo, dhi, L, true);
  }
  if (p->complement)
  {
    /* 90 - d: its lower bound from the upper one of d, and conversely */
    uint32_t *n = palloc(L * sizeof(uint32_t));
    fix_set_pow2(n, L, K);
    fix_mul_small(n, 90, n, L);
    fix_sub_clamp(n, dhi, tlo, L);
    fix_sub_clamp(n, dlo, thi, L);
    memcpy(dlo, tlo, L * sizeof(uint32_t));
    memcpy(dhi, thi, L * sizeof(uint32_t));
    pfree(n);
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
    p->lo64[i / 2] |= (uint64) dlo[i] << (32 * (i % 2));
    p->hi64[i / 2] |= (uint64) dhi[i] << (32 * (i % 2));
  }
  pfree(tlo); pfree(thi); pfree(plo); pfree(phi); pfree(dlo); pfree(dhi);
  p->bits = bits;
  p->exp = - K;
  p->n64 = n64;
}

/**
 * @brief Return the sign of the degrees of an angle less @p mid, raising
 * the precision of their enclosure until it lies clear of @p mid, the twin
 * of #trig_side
 * @note The degrees of the angles that reach this test are irrational, the
 * rational ones being answered before, so they are never the dyadic @p mid
 */
static int
degree_angle_side(const WideDyadic *mid, const void *arg)
{
  DegreeAngle *p = *(DegreeAngle * const *) arg;
  while (true)
  {
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
        "The nearest double of the degrees of an angle is undecided");
      return 0;
    }
    degree_angle_enclose(p, 2 * p->bits);
  }
}

/**
 * @brief Return the double nearest the degrees of an angle, @p approx its
 * estimate in doubles
 */
static double
degree_angle_nearest(DegreeAngle *p, double approx)
{
  degree_angle_enclose(p, TRIG_FIRST_BITS);
  DegreeAngle *pp = p;
  double result = nearest_double_exact(approx, degree_angle_side, &pp);
  pfree(p->lo64);
  pfree(p->hi64);
  return result;
}

/* -1, 0 or 1 as a < b, a = b or a > b */
static int
wide_cmp(const WideDyadic *a, const WideDyadic *b)
{
  WideDyadic *d = wide_dyadic_alloc();
  wide_dyadic_sub(a, b, d);
  int result = wide_sign(d);
  pfree(d);
  return result;
}

/**
 * @brief Return the double nearest the longitude in degrees of the vector of
 * integers @p v, read exactly, atan2(y, x) in (-180, 180]
 * @details The inverse of #lonlat_vector_nearest, with
 * #vector_lat_nearest. The degrees are rational exactly where Niven's
 * theorem allows, a tangent y / x of 0, 1 or -1, read on the integers; any
 * other is irrational: the double nearest it is read first in bounded pairs
 * of doubles (#ddb_vector_lon_nearest), and where their bound cannot tell,
 * by #nearest_double_exact on an enclosure of its degrees
 * (#degree_angle_enclose), atan2(|y|, x) read with |y| as its own square
 * root. A vector on the z axis answers 0.
 * @param[in] v Vector of three integers
 */
double
vector_lon_nearest(const int64 *v)
{
  int64 x = v[0], y = v[1];
  if (y == 0)
    return x < 0 ? 180.0 : 0.0;
  if (x == 0)
    return y > 0 ? 90.0 : -90.0;
  uint64 ay = int64_magnitude(y);
  if (int64_magnitude(x) == ay)
    return (x > 0 ? 45.0 : 135.0) * (y > 0 ? 1.0 : -1.0);
  /* Bounded pairs of doubles first, the enclosure where they cannot tell */
  double first;
  if (ddb_vector_lon_nearest(v, &first))
    return first;
  /* The angle is atan2(|y|, x), signed as y */
  WideDyadic *X = wide_dyadic_alloc(), *AY = wide_dyadic_alloc();
  wide_set_int64(X, x);
  wide_set_magnitude(AY, ay, false);
  DegreeAngle p;
  memset(&p, 0, sizeof(p));
  p.s = AY;
  p.root = true;
  p.c = X;
  p.sign = y > 0 ? 1 : -1;
  double result = degree_angle_nearest(&p, atan2((double) y, (double) x) *
    (180.0 / M_PI));
  pfree(X); pfree(AY);
  return result;
}

/**
 * @brief Return the double nearest the latitude in degrees of the vector of
 * integers @p v, read exactly, atan2(z, sqrt(x^2 + y^2)) in [-90, 90]
 * @details The inverse of #lonlat_vector_nearest, with
 * #vector_lon_nearest. The degrees are rational exactly where Niven's
 * theorem allows, a squared tangent z^2 / (x^2 + y^2) of 0, 1/3, 1 or 3,
 * read on the exact integers; any other is irrational and is rounded by
 * #nearest_double_exact on an enclosure of its degrees
 * (#degree_angle_enclose), 90 less the angle atan2(sqrt(x^2 + y^2), |z|).
 * Before both, bounded pairs of doubles answer wherever their bound places
 * the degrees inside the rounding cell of one double
 * (#ddb_vector_lat_nearest), a rational latitude, a double, included.
 * A zero vector answers 0.
 * @param[in] v Vector of three integers
 */
double
vector_lat_nearest(const int64 *v)
{
  int64 x = v[0], y = v[1], z = v[2];
  if (z == 0)
    return 0.0;
  double sz = z > 0 ? 1.0 : -1.0;
  if (x == 0 && y == 0)
    return 90.0 * sz;
  /* Bounded pairs of doubles first, the exact reading where they cannot
   * tell */
  double result;
  if (ddb_vector_lat_nearest(v, &result))
    return result;
  WideDyadic *X = wide_dyadic_alloc(), *Y = wide_dyadic_alloc();
  WideDyadic *AZ = wide_dyadic_alloc();
  wide_set_int64(X, x);
  wide_set_int64(Y, y);
  wide_set_magnitude(AZ, int64_magnitude(z), false);
  WideDyadic *XX = wide_dyadic_alloc(), *YY = wide_dyadic_alloc();
  WideDyadic *ZZ = wide_dyadic_alloc(), *RR = wide_dyadic_alloc();
  WideDyadic *T = wide_dyadic_alloc(), *three = wide_dyadic_alloc();
  wide_dyadic_mul(X, X, XX);
  wide_dyadic_mul(Y, Y, YY);
  wide_dyadic_mul(AZ, AZ, ZZ);
  wide_dyadic_add(XX, YY, RR);
  wide_dyadic_set_double(three, 3.0);
  bool rational = true;
  if (wide_cmp(ZZ, RR) == 0)
    result = 45.0 * sz;
  else
  {
    wide_dyadic_mul(three, RR, T);
    if (wide_cmp(ZZ, T) == 0)
      result = 60.0 * sz;
    else
    {
      wide_dyadic_mul(three, ZZ, T);
      if (wide_cmp(T, RR) == 0)
        result = 30.0 * sz;
      else
        rational = false;
    }
  }
  if (! rational)
  {
    /* The angle is 90 less atan2(sqrt(x^2 + y^2), |z|) */
    DegreeAngle p;
    memset(&p, 0, sizeof(p));
    p.s = RR;
    p.c = AZ;
    p.complement = true;
    p.sign = z > 0 ? 1 : -1;
    double approx = atan2((double) z, sqrt((double) x * (double) x +
      (double) y * (double) y)) * (180.0 / M_PI);
    result = degree_angle_nearest(&p, approx);
  }
  pfree(X); pfree(Y); pfree(AZ); pfree(XX); pfree(YY); pfree(ZZ); pfree(RR);
  pfree(T); pfree(three);
  return result;
}

/** @brief A quotient of two exact values, for #rational_side */
typedef struct
{
  const WideDyadic *n, *d;
} RationalQuotient;

/* The sign of n / d - mid, d > 0, that of n - mid d */
static int
rational_side(const WideDyadic *mid, const void *arg)
{
  const RationalQuotient *q = (const RationalQuotient *) arg;
  WideDyadic *md = wide_dyadic_alloc(), *diff = wide_dyadic_alloc();
  wide_dyadic_mul(mid, q->d, md);
  wide_dyadic_sub(q->n, md, diff);
  int result = wide_sign(diff);
  pfree(md); pfree(diff);
  return result;
}

/**
 * @brief Return the double nearest the rational @p num / @p den, @p den > 0
 * @details The coordinate of an S2 corner on its cube face is such a
 * rational (decision D7 of the cell walks), rounded by
 * #nearest_double_exact on the exact sign of num - mid den
 */
double
rational_nearest(int64 num, int64 den)
{
  WideDyadic *N = wide_dyadic_alloc(), *D = wide_dyadic_alloc();
  wide_set_int64(N, num);
  wide_set_int64(D, den);
  RationalQuotient q = { N, D };
  double result = nearest_double_exact((double) num / (double) den,
    rational_side, &q);
  pfree(N); pfree(D);
  return result;
}
