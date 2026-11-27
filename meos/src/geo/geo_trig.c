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
#include <port/pg_bitutils.h>
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
static void fix_trig_fixed(const uint32_t *xlo, const uint32_t *xhi,
  bool is_sin, uint32_t *lo, uint32_t *hi, int L, int S);

static void
fix_trig(double a, bool is_sin, uint32_t *lo, uint32_t *hi, int L, int S)
{
  uint32_t *xlo = palloc(L * sizeof(uint32_t));
  uint32_t *xhi = palloc(L * sizeof(uint32_t));

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
  fix_trig_fixed(xlo, xhi, is_sin, lo, hi, L, S);
  pfree(xlo); pfree(xhi);
}

/**
 * @brief Enclose the sine or the cosine of every radian angle of
 * [@p xlo, @p xhi] at the scale 2^-S in [lo, hi], 0 <= xlo <= xhi < 1
 * @details The series of #fix_trig on an angle given by its enclosure: each
 * term is read from the end of the interval that moves the bound outward, so
 * the bounds hold over the whole interval
 */
static void
fix_trig_fixed(const uint32_t *xlo, const uint32_t *xhi, bool is_sin,
  uint32_t *lo, uint32_t *hi, int L, int S)
{
  uint32_t *x2lo = palloc(L * sizeof(uint32_t));
  uint32_t *x2hi = palloc(L * sizeof(uint32_t));
  uint32_t *tlo = palloc(L * sizeof(uint32_t));
  uint32_t *thi = palloc(L * sizeof(uint32_t));

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
  pfree(x2lo); pfree(x2hi);
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
trig_side(DyadicWideArena *ar, const DyadicWide *mid, const void *arg)
{
  TrigProduct *p = *(TrigProduct * const *) arg;
  while (true)
  {
    /* The product lies between sign * lo and sign * hi */
    int clo = dyadic_wide_cmp_fixed(ar, mid, p->sign, p->lo64, p->n64, p->exp);
    int chi = dyadic_wide_cmp_fixed(ar, mid, p->sign, p->hi64, p->n64, p->exp);
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

/**
 * @brief Return floor(log2 |a|) of a #DyadicWide that is not 0, the weight of
 * its highest bit
 */
static int
wide_log2(const DyadicWide *a)
{
  return a->exp + 64 * (a->len - 1) +
    pg_leftmost_one_pos64(a->limb[a->len - 1]);
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
fix_atan2_enclose_root(const DyadicWide *s, bool root, const DyadicWide *c,
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
    int sg = dyadic_wide_to_fixed(k < 2 ? s : c,
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
fix_atan2_enclose(const DyadicWide *s, const DyadicWide *c, uint32_t *lo,
  uint32_t *hi, int L, int K)
{
  fix_atan2_enclose_root(s, false, c, lo, hi, L, K);
}

/* Whether the three components of an exact vector are 0 */
static bool
wide_vec_is_zero(DyadicWide *const *a)
{
  return wide_sign(a[0]) == 0 && wide_sign(a[1]) == 0 && wide_sign(a[2]) == 0;
}

/* The three components of a vector of doubles, exactly */
static void
wide_vec_of_double(DyadicWideArena *ar, const double *v, DyadicWide **r)
{
  for (int i = 0; i < 3; i++)
    r[i] = dyadic_wide_of_double(ar, v[i]);
}

/*****************************************************************************
 * Directions read exactly
 *
 * The cells of a geodetic trajectory are decided on directions in space: the
 * geodetic position of a vertex or of an instant (#lonlat_vector_nearest),
 * the normal of the great circle of an edge or of a segment, the direction in
 * which two great circles meet, and the sum of two directions, which lies
 * between them on their circle. Each is a vector of exact values
 * (#wide_cross3, #wide_dot3), so every sign read on it is exact.
 *****************************************************************************/

/** @brief A direction in space whose components are exact dyadic values,
 * each allocated on its own (#dyadic_wide_dup) */
struct SphereDir
{
  DyadicWide *c[3];
};

/* A direction of the three components of an exact vector, copied out of
 * their arena */
static SphereDir *
sphere_dir_of(DyadicWide *const *v)
{
  SphereDir *r = palloc(sizeof(SphereDir));
  for (int i = 0; i < 3; i++)
    r->c[i] = dyadic_wide_dup(v[i]);
  return r;
}

/**
 * @brief Return the direction of a vector of doubles, read exactly
 */
SphereDir *
sphere_dir_make(const double *v)
{
  DyadicWideArena ar;
  dyadic_wide_arena_init(&ar);
  DyadicWide *w[3];
  wide_vec_of_double(&ar, v, w);
  SphereDir *r = sphere_dir_of(w);
  dyadic_wide_arena_free(&ar);
  return r;
}

/**
 * @brief Return the cross product of two directions, exactly
 */
SphereDir *
sphere_dir_cross(const SphereDir *a, const SphereDir *b)
{
  DyadicWideArena ar;
  dyadic_wide_arena_init(&ar);
  DyadicWide *w[3];
  wide_cross3(&ar, a->c, b->c, w);
  SphereDir *r = sphere_dir_of(w);
  dyadic_wide_arena_free(&ar);
  return r;
}

/**
 * @brief Return the sum of two directions, exactly
 */
SphereDir *
sphere_dir_add(const SphereDir *a, const SphereDir *b)
{
  DyadicWideArena ar;
  dyadic_wide_arena_init(&ar);
  DyadicWide *w[3];
  for (int i = 0; i < 3; i++)
    w[i] = dyadic_wide_add(&ar, a->c[i], b->c[i]);
  SphereDir *r = sphere_dir_of(w);
  dyadic_wide_arena_free(&ar);
  return r;
}

/**
 * @brief Return the opposite of a direction, exactly
 */
SphereDir *
sphere_dir_neg(const SphereDir *a)
{
  SphereDir *r = palloc(sizeof(SphereDir));
  for (int i = 0; i < 3; i++)
  {
    r->c[i] = dyadic_wide_dup(a->c[i]);
    r->c[i]->sign = - r->c[i]->sign;
  }
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
  DyadicWideArena ar;
  dyadic_wide_arena_init(&ar);
  int result = wide_sign(wide_dot3(&ar, a->c, b->c));
  dyadic_wide_arena_free(&ar);
  return result;
}

/**
 * @brief Return the sign of the triple product (a x b) . c
 */
int
sphere_dir_triple_sign(const SphereDir *a, const SphereDir *b,
  const SphereDir *c)
{
  DyadicWideArena ar;
  dyadic_wide_arena_init(&ar);
  DyadicWide *n[3];
  wide_cross3(&ar, a->c, b->c, n);
  int result = wide_sign(wide_dot3(&ar, n, c->c));
  dyadic_wide_arena_free(&ar);
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
  DyadicWideArena ar;
  dyadic_wide_arena_init(&ar);
  const DyadicWide *ab = wide_dot3(&ar, a->c, b->c),
    *cd = wide_dot3(&ar, c->c, d->c);
  const DyadicWide *ad = wide_dot3(&ar, a->c, d->c),
    *cb = wide_dot3(&ar, c->c, b->c);
  int result = dyadic_wide_cmp(dyadic_wide_mul(&ar, ab, cd),
    dyadic_wide_mul(&ar, ad, cb));
  dyadic_wide_arena_free(&ar);
  return result;
}

/**
 * @brief Return true if two directions lie on one line through the origin,
 * their cross product being exactly 0
 */
bool
sphere_dir_parallel(const SphereDir *a, const SphereDir *b)
{
  DyadicWideArena ar;
  dyadic_wide_arena_init(&ar);
  DyadicWide *n[3];
  wide_cross3(&ar, a->c, b->c, n);
  bool result = wide_vec_is_zero(n);
  dyadic_wide_arena_free(&ar);
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
 * #DyadicWide and each angle enclosed (#fix_atan2_enclose) at a precision
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
  DyadicWideArena arena, *ar = &arena;
  dyadic_wide_arena_init(ar);
  DyadicWide *P0[3], *P1[3], *A[3], *B[3];
  wide_vec_of_double(ar, p0, P0);
  wide_vec_of_double(ar, p1, P1);
  wide_vec_of_double(ar, a, A);
  wide_vec_of_double(ar, b, B);
  DyadicWide *M[3], *N[3], *X[3], *C[3], *Y[3], *S[3];
  wide_cross3(ar, P0, P1, M);
  wide_cross3(ar, A, B, N);
  wide_cross3(ar, M, N, X);
  /* x ahead of p0: (p0 x x) . m > 0 */
  wide_cross3(ar, P0, X, C);
  if (wide_sign(wide_dot3(ar, C, M)) < 0)
    for (int i = 0; i < 3; i++)
      X[i] = dyadic_wide_neg(ar, X[i]);
  int result = 2;
  /* The exact fractions */
  wide_cross3(ar, X, P0, Y);
  bool at0 = wide_vec_is_zero(Y);
  wide_cross3(ar, X, P1, Y);
  bool at1 = wide_vec_is_zero(Y);
  for (int i = 0; i < 3; i++)
    S[i] = dyadic_wide_add(ar, P0[i], P1[i]);
  wide_cross3(ar, X, S, Y);
  bool athalf = wide_vec_is_zero(Y) &&
    dyadic_wide_cmp(wide_dot3(ar, P0, P0), wide_dot3(ar, P1, P1)) == 0;
  if (at0)
    result = fraction_cmp_rational(0, 1, num, den);
  else if (at1)
    result = fraction_cmp_rational(1, 1, num, den);
  else if (athalf)
    result = fraction_cmp_rational(1, 2, num, den);
  else
  {
    /* theta = atan2(|m|, p0 . p1), phi = atan2(|p0 x x|, p0 . x) */
    const DyadicWide *s1 = wide_dot3(ar, M, M), *c1 = wide_dot3(ar, P0, P1);
    wide_cross3(ar, P0, X, C);
    const DyadicWide *s2 = wide_dot3(ar, C, C), *c2 = wide_dot3(ar, P0, X);
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
  dyadic_wide_arena_free(ar);
  return result;
}

/*****************************************************************************
 * Intervals of exact values
 *
 * The within distance of two geography point segments on the sphere
 * (#geog_motion_dwithin_at_wide) reads positions that are transcendental in
 * the input: the trigonometry of the input degrees and of an angle along a
 * great circle. Each is enclosed in an interval whose ends are exact values
 * (#DyadicWide), so that sums and products of intervals are exact; only the
 * transcendental steps (a square root, an arc tangent, a sine or a cosine)
 * are read in the fixed-point integers of the enclosures at the scale 2^-K,
 * rounded outward, and the precision is raised until the interval of the
 * sign asked lies clear of 0, as #geodetic_arc_fraction_cmp raises it.
 *****************************************************************************/

/* An interval of exact values, lo <= hi */
typedef struct
{
  DyadicWide *lo;
  DyadicWide *hi;
} WideInterval;

/**
 * @brief Return the value sign * v * 2^-K of a fixed-point integer of @p L
 * 32-bit limbs, exactly
 * @details Each limb is a value of its own made by #dyadic_wide_of_int64 and
 * given the weight 2^(32 i - K) of its lowest bit, as #nearest_exact_side of
 * geo_dyadic.c gives a bound its weight
 */
static DyadicWide *
wide_interval_of_fixed(DyadicWideArena *ar, const uint32_t *v, int L, int K,
  int sign)
{
  DyadicWide *r = dyadic_wide_of_int64(ar, 0);
  for (int i = 0; i < L; i++)
  {
    if (! v[i])
      continue;
    DyadicWide *t = dyadic_wide_of_int64(ar, v[i]);
    t->exp = 32 * i - K;
    r = dyadic_wide_add(ar, r, t);
  }
  return (sign < 0) ? dyadic_wide_neg(ar, r) : r;
}

/**
 * @brief Write floor(|a| 2^K), or its ceiling where @p up, in @p L 32-bit
 * limbs and return the sign of @p a, 2 where it does not fit
 * (#dyadic_wide_to_fixed, #fix_of_limbs)
 */
static int
fix_of_wide(const DyadicWide *a, int K, bool up, uint32_t *v, int L)
{
  int n64 = (L + 1) / 2;
  uint64 *buf = palloc(n64 * sizeof(uint64));
  int sign = dyadic_wide_to_fixed(a, K, up, buf, n64);
  if (sign != 2)
    fix_of_limbs(buf, n64, v, L);
  pfree(buf);
  return sign;
}

/* The interval of one exact value */
static WideInterval
wide_interval_point(DyadicWide *a)
{
  WideInterval r = { a, a };
  return r;
}

/* The sum of two intervals, exactly */
static WideInterval
wide_interval_add(DyadicWideArena *ar, WideInterval a, WideInterval b)
{
  WideInterval r = { dyadic_wide_add(ar, a.lo, b.lo),
    dyadic_wide_add(ar, a.hi, b.hi) };
  return r;
}

/* The difference of two intervals, exactly */
static WideInterval
wide_interval_sub(DyadicWideArena *ar, WideInterval a, WideInterval b)
{
  WideInterval r = { dyadic_wide_sub(ar, a.lo, b.hi),
    dyadic_wide_sub(ar, a.hi, b.lo) };
  return r;
}

/**
 * @brief The product of two intervals, exactly: the least and the greatest of
 * the products of their ends; the square of an interval, @p a and @p b the
 * same, is not negative
 */
static WideInterval
wide_interval_mul(DyadicWideArena *ar, WideInterval a, WideInterval b)
{
  DyadicWide *p[4] = { dyadic_wide_mul(ar, a.lo, b.lo),
    dyadic_wide_mul(ar, a.lo, b.hi), dyadic_wide_mul(ar, a.hi, b.lo),
    dyadic_wide_mul(ar, a.hi, b.hi) };
  WideInterval r = { p[0], p[0] };
  for (int i = 1; i < 4; i++)
  {
    if (dyadic_wide_cmp(p[i], r.lo) < 0)
      r.lo = p[i];
    if (dyadic_wide_cmp(p[i], r.hi) > 0)
      r.hi = p[i];
  }
  if (a.lo == b.lo && a.hi == b.hi && wide_sign(r.lo) < 0)
    r.lo = dyadic_wide_of_int64(ar, 0);
  return r;
}

/**
 * @brief The square root of an interval of values that are not negative, at
 * the scale 2^-K, rounded outward; false where it does not fit the limbs
 */
static bool
wide_interval_sqrt(DyadicWideArena *ar, WideInterval a, int L, int K,
  WideInterval *r)
{
  uint32_t *v = palloc(L * sizeof(uint32_t));
  bool ok = true;
  DyadicWide *ends[2] = { NULL, NULL };
  for (int k = 0; k < 2 && ok; k++)
  {
    /* sqrt(floor(a 2^2K)) bounds sqrt(a) 2^K below, the ceilings above */
    int sign = fix_of_wide(k ? a.hi : a.lo, 2 * K, k == 1, v, L);
    if (sign == 2)
      ok = false;
    else
    {
      if (sign < 0)
        memset(v, 0, L * sizeof(uint32_t));
      fix_isqrt(v, v, L, k == 1);
      ends[k] = wide_interval_of_fixed(ar, v, L, K, 1);
    }
  }
  pfree(v);
  if (ok)
  {
    r->lo = ends[0];
    r->hi = ends[1];
  }
  return ok;
}

/**
 * @brief Bound atan2(y, x) at the scale 2^-K below or, where @p up, above,
 * for y >= 0 and x of either sign given by its sign @p sx and its magnitude,
 * the angle in [0, pi], as #fix_atan2_enclose_root reads it
 */
static void
fix_atan2_signed(const uint32_t *y, int sx, const uint32_t *x, uint32_t *r,
  int L, int K, bool up)
{
  if (sx >= 0)
  {
    fix_atan2_bound(y, x, r, L, K, up);
    return;
  }
  /* pi - atan2(y, |x|) */
  uint32_t *pl = palloc(L * sizeof(uint32_t));
  uint32_t *ph = palloc(L * sizeof(uint32_t));
  uint32_t *a = palloc(L * sizeof(uint32_t));
  fix_pi(pl, ph, L, K);
  fix_atan2_bound(y, x, a, L, K, ! up);
  fix_sub_clamp(up ? ph : pl, a, r, L);
  pfree(pl); pfree(ph); pfree(a);
}

/**
 * @brief Enclose atan2(y, x) over the box of two intervals, y >= 0, at the
 * scale 2^-K, an angle in [0, pi]; false where the box reaches the origin
 * or does not fit the limbs
 * @details The angle decreases with x; it increases with y where x > 0 and
 * decreases where x < 0, so its least is at the greatest x and its greatest
 * at the least x, each at the end of y its sign there selects, each
 * magnitude read rounded in the direction that moves the bound outward
 */
static bool
wide_interval_atan2(DyadicWideArena *ar, WideInterval y, WideInterval x,
  int L, int K, WideInterval *r)
{
  if (wide_sign(y.hi) <= 0)
    return false;
  uint32_t *vy = palloc(L * sizeof(uint32_t));
  uint32_t *vx = palloc(L * sizeof(uint32_t));
  uint32_t *a = palloc(L * sizeof(uint32_t));
  bool ok = true;
  for (int k = 0; k < 2 && ok; k++)
  {
    /* k = 0: the least angle, at x = x.hi; k = 1: the greatest, at x.lo */
    bool up = (k == 1);
    const DyadicWide *xe = up ? x.lo : x.hi;
    int sx = wide_sign(xe);
    /* Where x > 0 the angle increases with y, else it decreases */
    bool yhi = (sx > 0) == up;
    int sy = fix_of_wide(yhi ? y.hi : y.lo, K, yhi, vy, L);
    /* A greater |x| lowers the angle where x > 0 and raises it where x < 0 */
    int sxx = fix_of_wide(xe, K, (sx > 0) != up, vx, L);
    if (sy == 2 || sxx == 2)
      ok = false;
    else
    {
      if (sy < 0)
        memset(vy, 0, L * sizeof(uint32_t));
      if (fix_is_zero(vy, L) && fix_is_zero(vx, L))
        ok = false;
      else
      {
        fix_atan2_signed(vy, sx, vx, a, L, K, up);
        if (up)
          r->hi = wide_interval_of_fixed(ar, a, L, K, 1);
        else
          r->lo = wide_interval_of_fixed(ar, a, L, K, 1);
      }
    }
  }
  pfree(vy); pfree(vx); pfree(a);
  return ok;
}

/**
 * @brief Enclose the sine and the cosine of every angle of an interval in
 * [0, pi] at the scale 2^-K; false where it does not fit the limbs
 * @details The quarter y of the angle lies in [0, pi / 4], below 1, where
 * the series of #fix_trig_fixed enclose sin y and cos y, both not negative;
 * two doublings, sin 2u = 2 sin u cos u and cos 2u = cos^2 u - sin^2 u,
 * give the sine and the cosine of the angle, cos 2y being not negative and
 * sin 4y the double of such a product, every bound of those read in fixed
 * point rounded outward, and cos 4y, of either sign, the exact difference of
 * the two squares
 * @param[in] x Angle in radians
 * @param[out] s,c Sine and cosine, @p c NULL where not asked
 */
static bool
wide_interval_sincos(DyadicWideArena *ar, WideInterval x, int L, int K,
  WideInterval *s, WideInterval *c)
{
  uint32_t *v[12];
  for (int i = 0; i < 12; i++)
    v[i] = palloc(L * sizeof(uint32_t));
  uint32_t *ylo = v[0], *yhi = v[1], *sl = v[2], *sh = v[3], *cl = v[4],
    *ch = v[5], *t = v[6], *u = v[7], *s2l = v[8], *s2h = v[9], *c2l = v[10],
    *c2h = v[11];
  /* y = x / 4, floor(x 2^(K-2)) and its ceiling */
  int sg0 = fix_of_wide(x.lo, K - 2, false, ylo, L);
  int sg1 = fix_of_wide(x.hi, K - 2, true, yhi, L);
  bool ok = (sg0 != 2 && sg1 != 2);
  if (ok)
  {
    if (sg0 < 0)
      memset(ylo, 0, L * sizeof(uint32_t));
    if (sg1 < 0)
      memset(yhi, 0, L * sizeof(uint32_t));
    fix_trig_fixed(ylo, yhi, true, sl, sh, L, K);
    fix_trig_fixed(ylo, yhi, false, cl, ch, L, K);
    /* sin 2y = 2 sin y cos y */
    fix_mul(sl, cl, s2l, L, K, false);
    fix_mul_small(s2l, 2, s2l, L);
    fix_mul(sh, ch, s2h, L, K, true);
    fix_mul_small(s2h, 2, s2h, L);
    /* cos 2y = cos^2 y - sin^2 y, not negative */
    fix_mul(cl, cl, t, L, K, false);
    fix_mul(sh, sh, u, L, K, true);
    fix_sub_clamp(t, u, c2l, L);
    fix_mul(ch, ch, t, L, K, true);
    fix_mul(sl, sl, u, L, K, false);
    fix_sub_clamp(t, u, c2h, L);
    /* sin 4y = 2 sin 2y cos 2y */
    fix_mul(s2l, c2l, t, L, K, false);
    fix_mul_small(t, 2, t, L);
    s->lo = wide_interval_of_fixed(ar, t, L, K, 1);
    fix_mul(s2h, c2h, t, L, K, true);
    fix_mul_small(t, 2, t, L);
    s->hi = wide_interval_of_fixed(ar, t, L, K, 1);
    if (c)
    {
      /* cos 4y = cos^2 2y - sin^2 2y */
      fix_mul(c2l, c2l, t, L, K, false);
      fix_mul(s2h, s2h, u, L, K, true);
      c->lo = dyadic_wide_sub(ar, wide_interval_of_fixed(ar, t, L, K, 1),
        wide_interval_of_fixed(ar, u, L, K, 1));
      fix_mul(c2h, c2h, t, L, K, true);
      fix_mul(s2l, s2l, u, L, K, false);
      c->hi = dyadic_wide_sub(ar, wide_interval_of_fixed(ar, t, L, K, 1),
        wide_interval_of_fixed(ar, u, L, K, 1));
    }
  }
  for (int i = 0; i < 12; i++)
    pfree(v[i]);
  return ok;
}

/**
 * @brief Enclose the sine and the cosine of @p deg degrees at the scale
 * 2^-K, exactly where they are rational (#trig_factor_exact)
 * @details Each is reduced to the sine or the cosine of an angle in [0, 45]
 * (#trig_reduce) and enclosed by #fix_trig, as #sind_nearest reads it
 */
static void
wide_interval_sincos_deg(DyadicWideArena *ar, double deg, int L, int K,
  WideInterval *s, WideInterval *c)
{
  uint32_t *lo = palloc(L * sizeof(uint32_t));
  uint32_t *hi = palloc(L * sizeof(uint32_t));
  for (int k = 0; k < 2; k++)
  {
    TrigFactor f;
    trig_reduce(deg, k == 0, &f);
    WideInterval *r = (k == 0) ? s : c;
    double num;
    if (trig_factor_exact(&f, &num))
    {
      *r = wide_interval_point(dyadic_wide_of_double(ar, f.sign * num));
      continue;
    }
    fix_trig(f.a, f.is_sin, lo, hi, L, K);
    if (f.sign > 0)
    {
      r->lo = wide_interval_of_fixed(ar, lo, L, K, 1);
      r->hi = wide_interval_of_fixed(ar, hi, L, K, 1);
    }
    else
    {
      r->lo = wide_interval_of_fixed(ar, hi, L, K, -1);
      r->hi = wide_interval_of_fixed(ar, lo, L, K, -1);
    }
  }
  pfree(lo); pfree(hi);
}

/**
 * @brief Enclose the quotient of two positive integers below 2^53 at the
 * scale 2^-K (#fix_div_shift)
 */
static WideInterval
wide_interval_ratio(DyadicWideArena *ar, uint64 num, uint64 den, int L,
  int K)
{
  uint32_t *a = palloc0(L * sizeof(uint32_t));
  uint32_t *d = palloc0(L * sizeof(uint32_t));
  uint32_t *q = palloc(L * sizeof(uint32_t));
  a[0] = (uint32_t) num; a[1] = (uint32_t) (num >> 32);
  d[0] = (uint32_t) den; d[1] = (uint32_t) (den >> 32);
  WideInterval r;
  fix_div_shift(a, K, d, q, L, false);
  r.lo = wide_interval_of_fixed(ar, q, L, K, 1);
  fix_div_shift(a, K, d, q, L, true);
  r.hi = wide_interval_of_fixed(ar, q, L, K, 1);
  pfree(a); pfree(d); pfree(q);
  return r;
}

/* The unit vector of a longitude and a latitude in degrees, enclosed */
static void
wide_interval_unit(DyadicWideArena *ar, const POINT2D *p, int L, int K,
  WideInterval *v)
{
  WideInterval sla, cla, slo, clo;
  wide_interval_sincos_deg(ar, p->y, L, K, &sla, &cla);
  wide_interval_sincos_deg(ar, p->x, L, K, &slo, &clo);
  v[0] = wide_interval_mul(ar, cla, clo);
  v[1] = wide_interval_mul(ar, cla, slo);
  v[2] = sla;
}

/**
 * @brief Set the scaled position N and the scale s of a geography point
 * segment at the fraction num / den, its position being N / s, enclosed
 * @details With A and B the unit vectors of its ends and om the angle between
 * them, the position at the fraction f is the spherical interpolation
 * (sin((1 - f) om) A + sin(f om) B) / sin(om), and sin(om) = |A x B|, so no
 * quotient is taken; a segment whose ends are one position stands at A, s
 * being 1
 * @return False where the precision does not suffice
 */
static bool
wide_interval_geogsegm(DyadicWideArena *ar, const POINT2D *p0,
  const POINT2D *p1, uint64 num, uint64 den, int L, int K, WideInterval *N,
  WideInterval *s)
{
  WideInterval A[3], B[3], X[3];
  wide_interval_unit(ar, p0, L, K, A);
  if (p0->x == p1->x && p0->y == p1->y)
  {
    for (int i = 0; i < 3; i++)
      N[i] = A[i];
    *s = wide_interval_point(dyadic_wide_of_int64(ar, 1));
    return true;
  }
  wide_interval_unit(ar, p1, L, K, B);
  X[0] = wide_interval_sub(ar, wide_interval_mul(ar, A[1], B[2]),
    wide_interval_mul(ar, A[2], B[1]));
  X[1] = wide_interval_sub(ar, wide_interval_mul(ar, A[2], B[0]),
    wide_interval_mul(ar, A[0], B[2]));
  X[2] = wide_interval_sub(ar, wide_interval_mul(ar, A[0], B[1]),
    wide_interval_mul(ar, A[1], B[0]));
  WideInterval x2 = wide_interval_add(ar, wide_interval_add(ar,
    wide_interval_mul(ar, X[0], X[0]), wide_interval_mul(ar, X[1], X[1])),
    wide_interval_mul(ar, X[2], X[2]));
  WideInterval dot = wide_interval_add(ar, wide_interval_add(ar,
    wide_interval_mul(ar, A[0], B[0]), wide_interval_mul(ar, A[1], B[1])),
    wide_interval_mul(ar, A[2], B[2]));
  WideInterval om, th, ph, sth, sph;
  if (! wide_interval_sqrt(ar, x2, L, K, s) ||
      ! wide_interval_atan2(ar, *s, dot, L, K, &om))
    return false;
  th = wide_interval_mul(ar, om, wide_interval_ratio(ar, num, den, L, K));
  ph = wide_interval_mul(ar, om,
    wide_interval_ratio(ar, den - num, den, L, K));
  if (! wide_interval_sincos(ar, th, L, K, &sth, NULL) ||
      ! wide_interval_sincos(ar, ph, L, K, &sph, NULL))
    return false;
  for (int i = 0; i < 3; i++)
    N[i] = wide_interval_add(ar, wide_interval_mul(ar, sph, A[i]),
      wide_interval_mul(ar, sth, B[i]));
  return true;
}

/**
 * @brief Enclose dist / (2 radius) at the scale 2^-K, both positive doubles
 * and the quotient below 2
 */
static WideInterval
wide_interval_half_angle(DyadicWideArena *ar, double dist, double radius,
  int L, int K)
{
  int ed, er;
  uint64 md = (uint64) ldexp(frexp(dist, &ed), 53);
  uint64 mr = (uint64) ldexp(frexp(radius, &er), 53);
  /* dist / (2 radius) = md / mr 2^(ed - er - 1) */
  int k = K + ed - er - 1;
  WideInterval r;
  if (k < 0)
  {
    /* Below 2^-K: [0, 2^-K] */
    r.lo = dyadic_wide_of_int64(ar, 0);
    r.hi = dyadic_wide_of_int64(ar, 1);
    r.hi->exp = - K;
    return r;
  }
  r = wide_interval_ratio(ar, md, mr, L, k);
  /* The ratio read at the scale 2^-k is the value at the scale 2^-K */
  r.lo = dyadic_wide_signed(ar, r.lo, 1);
  r.hi = dyadic_wide_signed(ar, r.hi, 1);
  r.lo->exp += k - K;
  r.hi->exp += k - K;
  return r;
}

/**
 * @brief Return 1 or 0 where two geography point segments are within a
 * distance on the sphere at the microsecond @p n of a segment of @p D
 * microseconds, or not, 2 where the precision does not decide it
 * @details The precise tier of #geog_motion_dwithin_at_exact, the geodetic
 * twin of the wide tier of the planar kernel
 * (#point_motion_dwithin_period_exact). With each position P = N / s
 * (#wide_interval_geogsegm), the two are within the distance d where
 * |P1 - P2| <= 2 sin(d / 2R), that is where
 * 4 sin^2(d / 2R) s1^2 s2^2 - |s2 N1 - s1 N2|^2 is not negative, read on
 * intervals of exact values at a precision raised until the interval lies
 * clear of 0
 * @param[in] a0,a1 Ends of the first segment, longitude and latitude
 * @param[in] b0,b1 Ends of the second segment
 * @param[in] dist Distance, positive, below pi times @p radius
 * @param[in] radius Radius of the sphere
 * @param[in] D Duration of the segment in microseconds, below 2^52
 * @param[in] n Microsecond from the start of the segment
 */
int
geog_motion_dwithin_at_wide(const POINT2D *a0, const POINT2D *a1,
  const POINT2D *b0, const POINT2D *b1, double dist, double radius, int64 D,
  int64 n)
{
  assert(D > 0); assert(0 <= n && n <= D); assert(dist > 0.0);
  const POINT2D *ends[2][2] = { { a0, a1 }, { b0, b1 } };
  DyadicWideArena arena, *ar = &arena;
  dyadic_wide_arena_init(ar);
  int result = 2;
  for (int K = ANGLE_FIRST_BITS; K <= ANGLE_LAST_BITS && result == 2; K *= 2)
  {
    DyadicWideMark mark = dyadic_wide_arena_mark(ar);
    int L = (2 * K + 192) / 32;
    WideInterval N[2][3], s[2], sx;
    bool ok = true;
    for (int k = 0; k < 2 && ok; k++)
      ok = wide_interval_geogsegm(ar, ends[k][0], ends[k][1], (uint64) n,
        (uint64) D, L, K, N[k], &s[k]);
    if (ok)
      ok = wide_interval_sincos(ar, wide_interval_half_angle(ar, dist,
        radius, L, K), L, K, &sx, NULL);
    if (ok)
    {
      /* 4 sin^2(d / 2R) s1^2 s2^2 - |s2 N1 - s1 N2|^2 */
      WideInterval lim = wide_interval_mul(ar, sx, wide_interval_mul(ar,
        s[0], s[1]));
      WideInterval rhs = wide_interval_mul(ar, lim, lim);
      rhs = wide_interval_add(ar, rhs, rhs);
      rhs = wide_interval_add(ar, rhs, rhs);
      WideInterval sq = wide_interval_point(dyadic_wide_of_int64(ar, 0));
      for (int i = 0; i < 3; i++)
      {
        WideInterval v = wide_interval_sub(ar,
          wide_interval_mul(ar, s[1], N[0][i]),
          wide_interval_mul(ar, s[0], N[1][i]));
        sq = wide_interval_add(ar, sq, wide_interval_mul(ar, v, v));
      }
      WideInterval h = wide_interval_sub(ar, rhs, sq);
      if (wide_sign(h.lo) >= 0)
        result = 1;
      else if (wide_sign(h.hi) < 0)
        result = 0;
    }
    dyadic_wide_arena_release(ar, mark);
  }
  dyadic_wide_arena_free(ar);
  return result;
}

/*****************************************************************************
 * Bodies in rigid motion
 *
 * A body in rigid motion in the plane, from the pose (x0, y0, th0) to the
 * pose (x1, y1, th1), places its point u, given in its own frame, at the
 * fraction t of the segment at C(t) + R(th(t)) u, where
 * C(t) = C0 + (C1 - C0) t and th(t) = th0 + om t, om being the turn
 * #posesegm_interpolate takes: the difference of the two angles, less or plus
 * a full turn where it exceeds a half turn.  A body at rest has its point u
 * at u.  Each position is enclosed on the intervals of exact values above,
 * the sine and the cosine of the angle being those of radians.
 *****************************************************************************/

/* The number of cells #rigid_convex_apart reads before it answers 2 */
#define RIGID_CELL_BUDGET 4096

/** @brief The motion of a body in a segment, enclosed */
typedef struct
{
  bool still;              /**< True for a body at rest */
  WideInterval cx, cy;     /**< Position at the start, C0 */
  WideInterval dx, dy;     /**< Displacement, C1 - C0 */
  WideInterval th0;        /**< Angle at the start */
  WideInterval om;         /**< Turn */
} RigidMotion;

/** @brief A vertex of a body placed at a time or over an interval of time,
 * with its first and second derivatives in time */
typedef struct
{
  WideInterval x, y;
  WideInterval dx, dy;
  WideInterval ddx, ddy;
} RigidPlaced;

/* The interval of the opposites of the values of an interval */
static WideInterval
wide_interval_neg(DyadicWideArena *ar, WideInterval a)
{
  WideInterval r = { dyadic_wide_neg(ar, a.hi), dyadic_wide_neg(ar, a.lo) };
  return r;
}

/**
 * @brief Enclose the sine and the cosine of the angle @p x in radians, an
 * interval of width at most pi / 4, at the scale 2^-K
 * @details The angle is reduced by the multiple k pi / 2 of a right angle
 * that brings its middle into [pi / 4, 3 pi / 4], pi enclosed (#fix_pi), so
 * that the reduced angle lies in [0, pi], where #wide_interval_sincos
 * encloses its sine and cosine; the quadrant k mod 4 turns them back
 * @return False where the width or the reduction does not allow it
 */
static bool
wide_interval_sincos_rad(DyadicWideArena *ar, WideInterval x, int L, int K,
  WideInterval *s, WideInterval *c)
{
  double xlo = dyadic_wide_approx(x.lo), xhi = dyadic_wide_approx(x.hi);
  if (! isfinite(xlo) || ! isfinite(xhi) || xhi - xlo > M_PI_4 ||
      fabs(xlo) > 1.0e6 || fabs(xhi) > 1.0e6)
    return false;
  int64 k = llround(0.5 * (xlo + xhi) / M_PI_2) - 1;
  uint32_t *lo = palloc(L * sizeof(uint32_t));
  uint32_t *hi = palloc(L * sizeof(uint32_t));
  /* pi / 2 at the scale 2^-K is pi at the scale 2^-(K - 1) */
  fix_pi(lo, hi, L, K - 1);
  WideInterval half = { wide_interval_of_fixed(ar, lo, L, K, 1),
    wide_interval_of_fixed(ar, hi, L, K, 1) };
  pfree(lo); pfree(hi);
  WideInterval y = wide_interval_sub(ar, x, wide_interval_mul(ar,
    wide_interval_point(dyadic_wide_of_int64(ar, k)), half));
  if (wide_sign(y.lo) < 0)
    return false;
  WideInterval sy, cy;
  if (! wide_interval_sincos(ar, y, L, K, &sy, &cy))
    return false;
  int64 quadrant = ((k % 4) + 4) % 4;
  switch (quadrant)
  {
    case 0: *s = sy; *c = cy; break;
    case 1: *s = cy; *c = wide_interval_neg(ar, sy); break;
    case 2: *s = wide_interval_neg(ar, sy); *c = wide_interval_neg(ar, cy);
      break;
    default: *s = wide_interval_neg(ar, cy); *c = sy; break;
  }
  return true;
}

/**
 * @brief Set the motion of a body from its poses @p pose0 and @p pose1,
 * x, y and the angle in radians, or at rest where they are NULL
 * @details The turn is the one of #posesegm_interpolate, decided on the same
 * difference of the angles in doubles, and read exactly: the difference of
 * the two angles, less or plus 2 pi enclosed
 */
static void
rigid_motion_set(DyadicWideArena *ar, const double *pose0,
  const double *pose1, int L, int K, RigidMotion *m)
{
  WideInterval zero = wide_interval_point(dyadic_wide_of_int64(ar, 0));
  m->still = (pose0 == NULL);
  m->cx = m->cy = m->dx = m->dy = m->th0 = m->om = zero;
  if (m->still)
    return;
  DyadicWide *x0 = dyadic_wide_of_double(ar, pose0[0]);
  DyadicWide *y0 = dyadic_wide_of_double(ar, pose0[1]);
  m->cx = wide_interval_point(x0);
  m->cy = wide_interval_point(y0);
  m->dx = wide_interval_point(dyadic_wide_sub(ar,
    dyadic_wide_of_double(ar, pose1[0]), x0));
  m->dy = wide_interval_point(dyadic_wide_sub(ar,
    dyadic_wide_of_double(ar, pose1[1]), y0));
  DyadicWide *t0 = dyadic_wide_of_double(ar, pose0[2]);
  m->th0 = wide_interval_point(t0);
  double delta = pose1[2] - pose0[2];
  if (delta == 0.0)
    return;
  m->om = wide_interval_point(dyadic_wide_sub(ar,
    dyadic_wide_of_double(ar, pose1[2]), t0));
  int turn = 0;
  if (delta > M_PI)
    turn = -1;
  else if (delta <= -M_PI)
    turn = 1;
  if (turn != 0)
  {
    uint32_t *lo = palloc(L * sizeof(uint32_t));
    uint32_t *hi = palloc(L * sizeof(uint32_t));
    /* 2 pi at the scale 2^-K is pi at the scale 2^-(K + 1) */
    fix_pi(lo, hi, L, K + 1);
    WideInterval twopi = { wide_interval_of_fixed(ar, lo, L, K, 1),
      wide_interval_of_fixed(ar, hi, L, K, 1) };
    pfree(lo); pfree(hi);
    m->om = (turn > 0) ? wide_interval_add(ar, m->om, twopi) :
      wide_interval_sub(ar, m->om, twopi);
  }
  return;
}

/**
 * @brief Place the @p n vertices @p xy of a body moving with @p m at the
 * times @p t, an interval, with their first and, where @p second, their
 * second derivatives in time
 * @return False where the angle is not enclosed (#wide_interval_sincos_rad)
 */
static bool
rigid_place(DyadicWideArena *ar, const RigidMotion *m, const double *xy,
  int n, WideInterval t, bool second, int L, int K, RigidPlaced *p)
{
  WideInterval s, c;
  if (m->still)
  {
    s = wide_interval_point(dyadic_wide_of_int64(ar, 0));
    c = wide_interval_point(dyadic_wide_of_int64(ar, 1));
  }
  else if (! wide_interval_sincos_rad(ar, wide_interval_add(ar, m->th0,
      wide_interval_mul(ar, m->om, t)), L, K, &s, &c))
    return false;
  WideInterval cx = wide_interval_add(ar, m->cx, wide_interval_mul(ar, m->dx,
    t));
  WideInterval cy = wide_interval_add(ar, m->cy, wide_interval_mul(ar, m->dy,
    t));
  WideInterval om2 = wide_interval_mul(ar, m->om, m->om);
  for (int i = 0; i < n; i++)
  {
    WideInterval ux = wide_interval_point(dyadic_wide_of_double(ar,
      xy[2 * i]));
    WideInterval uy = wide_interval_point(dyadic_wide_of_double(ar,
      xy[2 * i + 1]));
    /* R(th) u */
    WideInterval rx = wide_interval_sub(ar, wide_interval_mul(ar, c, ux),
      wide_interval_mul(ar, s, uy));
    WideInterval ry = wide_interval_add(ar, wide_interval_mul(ar, s, ux),
      wide_interval_mul(ar, c, uy));
    p[i].x = wide_interval_add(ar, cx, rx);
    p[i].y = wide_interval_add(ar, cy, ry);
    /* C1 - C0 + om R(th + pi / 2) u */
    p[i].dx = wide_interval_sub(ar, m->dx, wide_interval_mul(ar, m->om, ry));
    p[i].dy = wide_interval_add(ar, m->dy, wide_interval_mul(ar, m->om, rx));
    if (second)
    {
      /* - om^2 R(th) u */
      p[i].ddx = wide_interval_neg(ar, wide_interval_mul(ar, om2, rx));
      p[i].ddy = wide_interval_neg(ar, wide_interval_mul(ar, om2, ry));
    }
  }
  return true;
}

/* The greatest magnitude of the values of an interval */
static DyadicWide *
wide_interval_absmax(DyadicWideArena *ar, WideInterval a)
{
  DyadicWide *lo = (wide_sign(a.lo) < 0) ? dyadic_wide_neg(ar, a.lo) : a.lo;
  DyadicWide *hi = (wide_sign(a.hi) < 0) ? dyadic_wide_neg(ar, a.hi) : a.hi;
  return (dyadic_wide_cmp(lo, hi) > 0) ? lo : hi;
}

/* The cross product a.x b.y - a.y b.x of two vectors of intervals */
static WideInterval
wide_interval_cross(DyadicWideArena *ar, WideInterval ax, WideInterval ay,
  WideInterval bx, WideInterval by)
{
  return wide_interval_sub(ar, wide_interval_mul(ar, ax, by),
    wide_interval_mul(ar, ay, bx));
}

/**
 * @brief Return true where @p sign c(t), c(t) = (b - a) x (v - a) the side of
 * the vertex @p v of the edge from @p a to @p b, is positive at every time of
 * the cell of half width @p h
 * @details It reads the value and the first derivative of c at the middle of
 * the cell (@p am, @p bm, @p vm) and its second derivative over the cell
 * (@p ac, @p bc, @p vc), with the bound by which the root finder of the
 * temporal rigid geometries excludes a root from a cell (#distfun_roots):
 * c(t) >= c(m) - |c'(m)| h - max |c''| h^2 / 2.  With @p h NULL it reads the
 * value at the middle alone.
 */
static bool
rigid_side_holds(DyadicWideArena *ar, const RigidPlaced *am,
  const RigidPlaced *bm, const RigidPlaced *vm, const RigidPlaced *ac,
  const RigidPlaced *bc, const RigidPlaced *vc, const DyadicWide *h,
  int sign)
{
  WideInterval ex = wide_interval_sub(ar, bm->x, am->x);
  WideInterval ey = wide_interval_sub(ar, bm->y, am->y);
  WideInterval wx = wide_interval_sub(ar, vm->x, am->x);
  WideInterval wy = wide_interval_sub(ar, vm->y, am->y);
  WideInterval c = wide_interval_cross(ar, ex, ey, wx, wy);
  const DyadicWide *low = (sign > 0) ? c.lo : dyadic_wide_neg(ar, c.hi);
  if (h == NULL || wide_sign(low) <= 0)
    return wide_sign(low) > 0;
  /* c' = e' x w + e x w' at the middle */
  WideInterval c1 = wide_interval_add(ar,
    wide_interval_cross(ar, wide_interval_sub(ar, bm->dx, am->dx),
      wide_interval_sub(ar, bm->dy, am->dy), wx, wy),
    wide_interval_cross(ar, ex, ey, wide_interval_sub(ar, vm->dx, am->dx),
      wide_interval_sub(ar, vm->dy, am->dy)));
  /* c'' = e'' x w + 2 e' x w' + e x w'' over the cell */
  WideInterval Ex = wide_interval_sub(ar, bc->x, ac->x);
  WideInterval Ey = wide_interval_sub(ar, bc->y, ac->y);
  WideInterval Wx = wide_interval_sub(ar, vc->x, ac->x);
  WideInterval Wy = wide_interval_sub(ar, vc->y, ac->y);
  WideInterval mid = wide_interval_cross(ar,
    wide_interval_sub(ar, bc->dx, ac->dx),
    wide_interval_sub(ar, bc->dy, ac->dy),
    wide_interval_sub(ar, vc->dx, ac->dx),
    wide_interval_sub(ar, vc->dy, ac->dy));
  WideInterval c2 = wide_interval_add(ar, wide_interval_add(ar,
    wide_interval_cross(ar, wide_interval_sub(ar, bc->ddx, ac->ddx),
      wide_interval_sub(ar, bc->ddy, ac->ddy), Wx, Wy),
    wide_interval_add(ar, mid, mid)),
    wide_interval_cross(ar, Ex, Ey, wide_interval_sub(ar, vc->ddx, ac->ddx),
      wide_interval_sub(ar, vc->ddy, ac->ddy)));
  const DyadicWide *r1 = dyadic_wide_mul(ar, wide_interval_absmax(ar, c1), h);
  DyadicWide *r2 = dyadic_wide_mul(ar, wide_interval_absmax(ar, c2),
    dyadic_wide_mul(ar, h, h));
  if (r2->sign)
    r2->exp -= 1;
  return wide_sign(dyadic_wide_sub(ar, dyadic_wide_sub(ar, low, r1),
    r2)) > 0;
}

/**
 * @brief Return the orientation of the polygon of @p n >= 3 vertices @p xy,
 * the sign of twice its signed area, exactly
 */
static int
rigid_orientation(DyadicWideArena *ar, const double *xy, int n)
{
  const DyadicWide *area = dyadic_wide_of_int64(ar, 0);
  for (int i = 0; i < n; i++)
  {
    int j = (i + 1) % n;
    area = dyadic_wide_add(ar, area, dyadic_wide_sub(ar,
      dyadic_wide_mul(ar, dyadic_wide_of_double(ar, xy[2 * i]),
        dyadic_wide_of_double(ar, xy[2 * j + 1])),
      dyadic_wide_mul(ar, dyadic_wide_of_double(ar, xy[2 * j]),
        dyadic_wide_of_double(ar, xy[2 * i + 1]))));
  }
  return wide_sign(area);
}

/**
 * @brief Return 1 where the cell [@p t0, @p t1] proves two convex bodies apart
 * over it, 0 where it proves them meeting at the time @p tmid inside it, 2
 * where it proves neither, and 3 where it is too wide to be read whole, at
 * the scale 2^-K, see #rigid_convex_apart
 */
static int
rigid_cell(DyadicWideArena *ar, const double *const *xy, const int *n,
  const RigidMotion *m, const int *outer, double t0, double tmid, double t1,
  int L, int K)
{
  DyadicWide *a = dyadic_wide_of_double(ar, t0);
  DyadicWide *b = dyadic_wide_of_double(ar, t1);
  /* The middle and the greater of its distances to the ends, exactly */
  DyadicWide *tm = dyadic_wide_of_double(ar, tmid);
  const DyadicWide *h = dyadic_wide_sub(ar, tm, a);
  const DyadicWide *h2 = dyadic_wide_sub(ar, b, tm);
  if (dyadic_wide_cmp(h2, h) > 0)
    h = h2;
  WideInterval cellt = { a, b };
  RigidPlaced *pm[2], *pc[2];
  for (int k = 0; k < 2; k++)
  {
    pm[k] = palloc(sizeof(RigidPlaced) * n[k]);
    pc[k] = palloc(sizeof(RigidPlaced) * n[k]);
  }
  bool ok = true, whole = true;
  for (int k = 0; k < 2 && ok; k++)
  {
    ok = rigid_place(ar, &m[k], xy[k], n[k], wide_interval_point(tm), false,
      L, K, pm[k]);
    /* A cell over which an angle turns too far is not read whole */
    if (ok && whole)
      whole = rigid_place(ar, &m[k], xy[k], n[k], cellt, true, L, K, pc[k]);
  }
  int result = 2;
  /* Apart: an edge of one body with every vertex of the other strictly on
   * its outer side over the cell */
  for (int k = 0; k < 2 && ok && whole && result == 2; k++)
  {
    int o = 1 - k;
    if (n[k] < 2 || outer[k] == 0)
      continue;
    for (int i = 0; i < n[k] && result == 2; i++)
    {
      int j = (i + 1) % n[k];
      bool all = true;
      for (int v = 0; v < n[o] && all; v++)
        all = rigid_side_holds(ar, &pm[k][i], &pm[k][j], &pm[o][v],
          &pc[k][i], &pc[k][j], &pc[o][v], h, outer[k]);
      if (all)
        result = 1;
    }
  }
  /* Meeting at the middle: a vertex of one body strictly inside the other */
  for (int k = 0; k < 2 && ok && result == 2; k++)
  {
    int o = 1 - k;
    if (n[k] < 3 || outer[k] == 0)
      continue;
    for (int v = 0; v < n[o] && result == 2; v++)
    {
      bool inside = true;
      for (int i = 0; i < n[k] && inside; i++)
        inside = rigid_side_holds(ar, &pm[k][i], &pm[k][(i + 1) % n[k]],
          &pm[o][v], NULL, NULL, NULL, NULL, - outer[k]);
      if (inside)
        result = 0;
    }
  }
  /* Or an edge of each body with the ends of the other strictly on its two
   * sides */
  for (int i = 0; i < n[0] && ok && n[0] >= 2 && n[1] >= 2 && result == 2;
      i++)
    for (int j = 0; j < n[1] && result == 2; j++)
    {
      const RigidPlaced *p = &pm[0][i], *q = &pm[0][(i + 1) % n[0]];
      const RigidPlaced *r = &pm[1][j], *s = &pm[1][(j + 1) % n[1]];
      for (int sg = -1; sg <= 1 && result == 2; sg += 2)
        for (int sh = -1; sh <= 1 && result == 2; sh += 2)
          if (rigid_side_holds(ar, p, q, r, NULL, NULL, NULL, NULL, sg) &&
              rigid_side_holds(ar, p, q, s, NULL, NULL, NULL, NULL, - sg) &&
              rigid_side_holds(ar, r, s, p, NULL, NULL, NULL, NULL, sh) &&
              rigid_side_holds(ar, r, s, q, NULL, NULL, NULL, NULL, - sh))
            result = 0;
    }
  for (int k = 0; k < 2; k++)
  {
    pfree(pm[k]); pfree(pc[k]);
  }
  if (! ok)
    return 2;
  return (result == 2 && ! whole) ? 3 : result;
}

/**
 * @brief Return the time at which #rigid_convex_apart halves the cell
 * [@p a, @p b] of fractions of a segment, @p a below @p b
 * @details A cell at least 2^-53 long is halved at its middle, where the
 * Taylor bound of #rigid_cell is the tightest.  A shorter one is halved at
 * the middle of its doubles, as #monotone_halve halves a bracket of the bit
 * patterns of the doubles, which order as the doubles do from 0 up.  Halving
 * the length of a cell next to 0, where the doubles go down to 2^-1074,
 * takes over a thousand halvings to reach a cell with no double inside,
 * where a cell elsewhere takes about 53; halving its doubles takes 64 at
 * most.  Two bodies that touch at the start of a segment, which no cell next
 * to it proves apart or meeting, spent those thousand cells at each
 * precision.  Above 2^-53 the two middles are the same within a range of
 * doubles of one spacing, and below it a cell is shorter than the spacing of
 * the doubles next to 1.
 */
static inline double
rigid_cell_mid(double a, double b)
{
  if (b - a >= 0x1p-53)
    return a + 0.5 * (b - a);
  uint64 abits, bbits;
  memcpy(&abits, &a, sizeof(abits));
  memcpy(&bbits, &b, sizeof(bbits));
  uint64 mbits = abits + (bbits - abits) / 2;
  double mid;
  memcpy(&mid, &mbits, sizeof(mid));
  return mid;
}

/**
 * @brief Return 1 where two convex bodies in rigid motion are apart at every
 * time of [@p t0, @p t1], 0 where they meet at a time of it, 2 where the
 * precision decides neither
 * @details Each body is a point, a segment or a convex polygon of vertices
 * @p axy, @p bxy, x, y pairs in its own frame, the first not repeated, which
 * moves from the pose @p apose0 to @p apose1, @p bpose0 to @p bpose1, x, y
 * and the angle in radians, as #posesegm_interpolate places it, or is at rest
 * where its poses are NULL; the times are fractions of the segment.  Two
 * convex bodies are apart where an edge of one has every vertex of the other
 * strictly on its outer side, which a cell of time proves by the Taylor
 * bound of the side at its middle (#rigid_side_holds), and meet where a
 * vertex of one is strictly inside the other, or an edge of each has the
 * ends of the other strictly on its two sides.  A cell that proves neither
 * is halved (#rigid_cell_mid), within a budget of cells, and the precision is
 * raised where the budget is spent, as #geog_motion_dwithin_at_wide raises
 * it.  The outer
 * side of the edges of a polygon is decided by its orientation, exactly; a
 * segment is read as its two edges, one each way, each with its right side
 * as the outer one.
 * @param[out] tmeet A time at which the bodies meet, where they do
 */
int
rigid_convex_apart(const double *axy, int na, const double *apose0,
  const double *apose1, const double *bxy, int nb, const double *bpose0,
  const double *bpose1, double t0, double t1, double *tmeet)
{
  assert(axy); assert(bxy); assert(na >= 1); assert(nb >= 1);
  assert(0.0 <= t0 && t0 <= t1 && t1 <= 1.0);
  const double *xy[2] = { axy, bxy };
  const int n[2] = { na, nb };
  const double *pose0[2] = { apose0, bpose0 };
  const double *pose1[2] = { apose1, bpose1 };
  DyadicWideArena arena, *ar = &arena;
  dyadic_wide_arena_init(ar);
  /* The outer side of the edges: the right of a counterclockwise polygon,
   * the right of each edge of a segment, none for a point or a polygon that
   * bounds no area */
  int outer[2];
  for (int k = 0; k < 2; k++)
    outer[k] = (n[k] == 2) ? -1 :
      ((n[k] >= 3) ? - rigid_orientation(ar, xy[k], n[k]) : 0);
  int result = 2;
  for (int K = ANGLE_FIRST_BITS; K <= 4 * ANGLE_FIRST_BITS && result == 2;
      K *= 4)
  {
    int L = (2 * K + 192) / 32;
    DyadicWideMark mark = dyadic_wide_arena_mark(ar);
    RigidMotion m[2];
    for (int k = 0; k < 2; k++)
      rigid_motion_set(ar, pose0[k], pose1[k], L, K, &m[k]);
    /* The cells on a stack, the earliest on top */
    int size = 64, count = 1, cells = 0;
    double *stack = palloc(sizeof(double) * 2 * size);
    stack[0] = t0;
    stack[1] = t1;
    result = 1;
    while (count > 0 && result == 1)
    {
      count--;
      double a = stack[2 * count], b = stack[2 * count + 1];
      double mid = rigid_cell_mid(a, b);
      DyadicWideMark cmark = dyadic_wide_arena_mark(ar);
      int r = rigid_cell(ar, xy, n, m, outer, a, mid, b, L, K);
      dyadic_wide_arena_release(ar, cmark);
      if (r == 1)
        continue;
      if (r == 0)
      {
        *tmeet = mid;
        result = 0;
        break;
      }
      if (++cells > RIGID_CELL_BUDGET || mid <= a || mid >= b)
      {
        result = 2;
        break;
      }
      if (count + 2 > size)
      {
        size *= 2;
        stack = repalloc(stack, sizeof(double) * 2 * size);
      }
      stack[2 * count] = mid; stack[2 * count + 1] = b; count++;
      stack[2 * count] = a; stack[2 * count + 1] = mid; count++;
    }
    pfree(stack);
    dyadic_wide_arena_release(ar, mark);
  }
  dyadic_wide_arena_free(ar);
  return result;
}

/**
 * @brief Set bounds @p lon / @p lod and @p hin / @p hid of the square of the
 * distance between the vertex @p v and the edge from @p a to @p b, whose
 * square of length is @p len2, exactly
 * @details The distance to the line of the edge, |e x w| / |e|, is no more
 * than the distance to the edge, and the distances to its ends are no less.
 * Where the projection of the vertex lies strictly before or after the edge,
 * the distance is the one to the nearer end; where it lies on the edge, the
 * one to the line.
 */
static void
rigid_edge_dist2(DyadicWideArena *ar, const RigidPlaced *a,
  const RigidPlaced *b, const RigidPlaced *v, const DyadicWide *len2,
  const DyadicWide **lon, const DyadicWide **lod, const DyadicWide **hin,
  const DyadicWide **hid)
{
  WideInterval ex = wide_interval_sub(ar, b->x, a->x);
  WideInterval ey = wide_interval_sub(ar, b->y, a->y);
  WideInterval wx = wide_interval_sub(ar, v->x, a->x);
  WideInterval wy = wide_interval_sub(ar, v->y, a->y);
  WideInterval zx = wide_interval_sub(ar, v->x, b->x);
  WideInterval zy = wide_interval_sub(ar, v->y, b->y);
  WideInterval cr = wide_interval_cross(ar, ex, ey, wx, wy);
  WideInterval perp = wide_interval_mul(ar, cr, cr);
  WideInterval s = wide_interval_add(ar, wide_interval_mul(ar, ex, wx),
    wide_interval_mul(ar, ey, wy));
  WideInterval wa = wide_interval_add(ar, wide_interval_mul(ar, wx, wx),
    wide_interval_mul(ar, wy, wy));
  WideInterval wb = wide_interval_add(ar, wide_interval_mul(ar, zx, zx),
    wide_interval_mul(ar, zy, zy));
  const DyadicWide *one = dyadic_wide_of_int64(ar, 1);
  bool before = wide_sign(s.hi) < 0;
  bool after = dyadic_wide_cmp(s.lo, len2) > 0;
  bool within = wide_sign(s.lo) >= 0 && dyadic_wide_cmp(s.hi, len2) <= 0;
  if (before)
  {
    *lon = wa.lo;
    *lod = one;
  }
  else if (after)
  {
    *lon = wb.lo;
    *lod = one;
  }
  else
  {
    *lon = perp.lo;
    *lod = len2;
  }
  *hin = within ? perp.hi :
    ((dyadic_wide_cmp(wa.hi, wb.hi) < 0) ? wa.hi : wb.hi);
  *hid = within ? len2 : one;
  return;
}

/* -1, 0 or 1 as n1 / d1 is below, equal to or above n2 / d2, d1, d2 > 0 */
static int
rigid_ratio_cmp(DyadicWideArena *ar, const DyadicWide *n1,
  const DyadicWide *d1, const DyadicWide *n2, const DyadicWide *d2)
{
  return dyadic_wide_cmp(dyadic_wide_mul(ar, n1, d2),
    dyadic_wide_mul(ar, n2, d1));
}

/**
 * @brief Return the double nearest the distance at the time @p t between two
 * convex bodies in rigid motion that do not meet then, or -1 where the
 * precision does not decide it
 * @details The bodies are those of #rigid_convex_apart.  The distance of two
 * convex bodies apart is the least of the distances between a vertex of one
 * and an edge of the other, or the distance of their vertices where both are
 * points, each bounded by exact quotients (#rigid_edge_dist2), the least of
 * which bound the least; the length of an edge is that of its vertices in
 * the frame of its body, exactly.  The double nearest the square root of the
 * lower bound is the answer where it is also the one of the upper bound, the
 * rounding being monotone, and the precision is raised until it is.
 */
double
rigid_convex_distance(const double *axy, int na, const double *apose0,
  const double *apose1, const double *bxy, int nb, const double *bpose0,
  const double *bpose1, double t)
{
  assert(axy); assert(bxy); assert(na >= 1); assert(nb >= 1);
  const double *xy[2] = { axy, bxy };
  const int n[2] = { na, nb };
  const double *pose0[2] = { apose0, bpose0 };
  const double *pose1[2] = { apose1, bpose1 };
  DyadicWideArena arena, *ar = &arena;
  dyadic_wide_arena_init(ar);
  double result = -1.0;
  for (int K = ANGLE_FIRST_BITS; K <= ANGLE_LAST_BITS && result < 0.0;
      K *= 2)
  {
    DyadicWideMark mark = dyadic_wide_arena_mark(ar);
    int L = (2 * K + 192) / 32;
    WideInterval tt = wide_interval_point(dyadic_wide_of_double(ar, t));
    RigidPlaced *p[2];
    bool ok = true;
    for (int k = 0; k < 2; k++)
    {
      RigidMotion m;
      rigid_motion_set(ar, pose0[k], pose1[k], L, K, &m);
      p[k] = palloc(sizeof(RigidPlaced) * n[k]);
      if (ok)
        ok = rigid_place(ar, &m, xy[k], n[k], tt, false, L, K, p[k]);
    }
    const DyadicWide *lon = NULL, *lod = NULL, *hin = NULL, *hid = NULL;
    for (int k = 0; k < 2 && ok; k++)
    {
      int o = 1 - k;
      for (int i = 0; i < n[k] && n[k] >= 2; i++)
      {
        int j = (i + 1) % n[k];
        const DyadicWide *ex = dyadic_wide_sub(ar,
          dyadic_wide_of_double(ar, xy[k][2 * j]),
          dyadic_wide_of_double(ar, xy[k][2 * i]));
        const DyadicWide *ey = dyadic_wide_sub(ar,
          dyadic_wide_of_double(ar, xy[k][2 * j + 1]),
          dyadic_wide_of_double(ar, xy[k][2 * i + 1]));
        const DyadicWide *len2 = dyadic_wide_add(ar,
          dyadic_wide_mul(ar, ex, ex), dyadic_wide_mul(ar, ey, ey));
        if (! len2->sign)
          continue;
        for (int v = 0; v < n[o]; v++)
        {
          const DyadicWide *a, *b, *c, *d;
          rigid_edge_dist2(ar, &p[k][i], &p[k][j], &p[o][v], len2, &a, &b,
            &c, &d);
          if (! lon || rigid_ratio_cmp(ar, a, b, lon, lod) < 0)
          {
            lon = a;
            lod = b;
          }
          if (! hin || rigid_ratio_cmp(ar, c, d, hin, hid) < 0)
          {
            hin = c;
            hid = d;
          }
        }
      }
    }
    if (ok && n[0] == 1 && n[1] == 1)
    {
      WideInterval dx = wide_interval_sub(ar, p[1][0].x, p[0][0].x);
      WideInterval dy = wide_interval_sub(ar, p[1][0].y, p[0][0].y);
      WideInterval d2 = wide_interval_add(ar, wide_interval_mul(ar, dx, dx),
        wide_interval_mul(ar, dy, dy));
      lon = d2.lo;
      hin = d2.hi;
      lod = hid = dyadic_wide_of_int64(ar, 1);
    }
    if (ok && lon && hin)
    {
      double lo = dyadic_wide_nearest_root(ar, lon, lod);
      double hi = dyadic_wide_nearest_root(ar, hin, hid);
      if (lo == hi)
        result = lo;
    }
    pfree(p[0]); pfree(p[1]);
    dyadic_wide_arena_release(ar, mark);
  }
  dyadic_wide_arena_free(ar);
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

/* The magnitude of an int64, exactly, 2^63 included */
static DyadicWide *
wide_of_magnitude(DyadicWideArena *ar, int64 x)
{
  DyadicWide *r = dyadic_wide_of_int64(ar, x);
  if (r->sign < 0)
    r->sign = 1;
  return r;
}

/*****************************************************************************
 * A point of a face of an icosahedral gnomonic grid
 *****************************************************************************/

/**
 * @brief Return the integer N of the decimal literal @p s, of value
 * N 10^-q, and q in @p q, exactly
 */
static DyadicWide *
wide_of_decimal(DyadicWideArena *ar, const char *s, int *q)
{
  bool neg = (*s == '-');
  if (*s == '-' || *s == '+')
    s++;
  DyadicWide *r = dyadic_wide_of_int64(ar, 0);
  const DyadicWide *ten = dyadic_wide_of_int64(ar, 10);
  int frac = -1;
  for (; *s; s++)
  {
    if (*s == '.')
    {
      frac = 0;
      continue;
    }
    assert(*s >= '0' && *s <= '9');
    r = dyadic_wide_add(ar, dyadic_wide_mul(ar, r, ten),
      dyadic_wide_of_int64(ar, *s - '0'));
    if (frac >= 0)
      frac++;
  }
  *q = frac < 0 ? 0 : frac;
  return neg ? dyadic_wide_neg(ar, r) : r;
}

/* b^e exactly, e >= 0 */
static DyadicWide *
wide_pow(DyadicWideArena *ar, int64 b, int e)
{
  DyadicWide *r = dyadic_wide_of_int64(ar, 1);
  const DyadicWide *w = dyadic_wide_of_int64(ar, b);
  for (int i = 0; i < e; i++)
    r = dyadic_wide_mul(ar, r, w);
  return r;
}

/* The interval a 2^e, exactly */
static WideInterval
wide_interval_ldexp(DyadicWideArena *ar, WideInterval a, int e)
{
  const DyadicWide *f = dyadic_wide_of_double(ar, ldexp(1.0, e));
  WideInterval r = { dyadic_wide_mul(ar, a.lo, f),
    dyadic_wide_mul(ar, a.hi, f) };
  return r;
}

/**
 * @brief Enclose the value of a decimal literal at the scale 2^-K, its
 * digits over the power of ten divided outward (#fix_div_shift), as
 * #wide_interval_ratio encloses a quotient
 */
static WideInterval
wide_interval_decimal(DyadicWideArena *ar, const char *s, int L, int K)
{
  int q;
  const DyadicWide *n = wide_of_decimal(ar, s, &q);
  int sign = n->sign;
  int n64 = (L + 1) / 2;
  uint64 *buf = palloc0(n64 * sizeof(uint64));
  uint32_t *a = palloc0(L * sizeof(uint32_t));
  uint32_t *d = palloc0(L * sizeof(uint32_t));
  uint32_t *v = palloc(L * sizeof(uint32_t));
  dyadic_wide_to_fixed(n, 0, false, buf, n64);
  fix_of_limbs(buf, n64, a, L);
  dyadic_wide_to_fixed(wide_pow(ar, 10, q), 0, false, buf, n64);
  fix_of_limbs(buf, n64, d, L);
  WideInterval r;
  /* The magnitude rounded down and up, the ends swapped for a negative */
  fix_div_shift(a, K, d, v, L, sign < 0);
  r.lo = wide_interval_of_fixed(ar, v, L, K, sign < 0 ? -1 : 1);
  fix_div_shift(a, K, d, v, L, sign >= 0);
  r.hi = wide_interval_of_fixed(ar, v, L, K, sign < 0 ? -1 : 1);
  if (sign < 0)
  {
    DyadicWide *t = r.lo;
    r.lo = r.hi;
    r.hi = t;
  }
  pfree(buf); pfree(a); pfree(d); pfree(v);
  return r;
}

/**
 * @brief The exact parts of a point of a face of an icosahedral gnomonic
 * grid (#GnomonicFacePoint), read once
 * @details H3's formula (`_hex2dToVec3` and `vec3ToLatLng` of its
 * faceijk.c and vec3d.h) places the planar point p = (x, y) of a face of
 * center c at the distance r = atan(|p| k) along the azimuth
 * az = axis - atan2(y, x): v = cos r c + sin r (cos az n + sin az e), with n
 * the unit north at c (the pole less its part along c, normalized), e = n x c
 * and k the gnomonic length of the unit at the resolution. Its longitude and
 * latitude are those of the direction of v, so a positive factor leaves them
 * unchanged: tan r = |p| k and |p| cos az = cos(axis) x + sin(axis) y, |p|
 * sin az = sin(axis) x - cos(axis) y turn it into
 * v ~ |n| c + k [(cos(axis) x + sin(axis) y) n + (sin(axis) x - cos(axis) y)
 * (n x c)] with n unnormalized. With the center C / P and the gnomonic length
 * G 10^-g read on their literals, the point (U, W sqrt 3) / (2D) and
 * k = 7^-m [7^-1/2] [1/3] G 10^-g at the resolution 2m [+1] of a [substrate]
 * grid, the factor 2D 7^m [sqrt 7] [3] 10^g P^3 leaves
 * v ~ f1 [sqrt 7] |N| C + G P a N + G b (N x C), with N = P^2 n integer,
 * a = cos(axis) U + sin(axis) W sqrt 3, b = sin(axis) U - cos(axis) W sqrt 3
 * and f1 = 2D 7^m [3] 10^g: integers but for sqrt 3, sqrt 7, |N| and the sine
 * and cosine of the axis, which #gnomonic_vector encloses at each precision
 */
typedef struct
{
  DyadicWideArena ar;        /**< Arena of the values below */
  const char *axis;          /**< Literal of the azimuth of the first axis */
  const char *rotation;      /**< Literal of the turn of the angle, or NULL */
  bool odd;                  /**< The resolution is odd: a factor sqrt 7 */
  DyadicWide *C[3];          /**< Center of the face, its digits */
  DyadicWide *N[3];          /**< North at the center, unnormalized */
  DyadicWide *NC[3];         /**< N x C */
  DyadicWide *NN;            /**< N . N 2^(-2 nn_e), in [1, 4) */
  int nn_e;                  /**< Its scale */
  DyadicWide *f1;            /**< Factor of the term along C */
  DyadicWide *GP;            /**< Factor of the term along N */
  DyadicWide *G;             /**< Factor of the term along N x C */
  DyadicWide *U, *W;         /**< The point, (U, W sqrt 3) / (2D) */
} GnomonicExact;

/* Read the exact parts of the point p (#GnomonicExact) */
static void
gnomonic_exact_init(GnomonicExact *ge, const GnomonicFacePoint *p)
{
  DyadicWideArena *ar = &ge->ar;
  dyadic_wide_arena_init(ar);
  ge->axis = p->axis;
  ge->rotation = p->rotation;
  ge->odd = (p->res % 2) == 1;
  /* The center on the scale of its longest literal */
  int q[3], qc = 0;
  for (int i = 0; i < 3; i++)
  {
    ge->C[i] = wide_of_decimal(ar, p->center[i], &q[i]);
    qc = Max(qc, q[i]);
  }
  for (int i = 0; i < 3; i++)
    ge->C[i] = dyadic_wide_mul(ar, ge->C[i], wide_pow(ar, 10, qc - q[i]));
  const DyadicWide *P = wide_pow(ar, 10, qc);
  /* N = P^2 (pole - (pole . c) c) = (-Cz Cx, -Cz Cy, P^2 - Cz^2) */
  ge->N[0] = dyadic_wide_neg(ar, dyadic_wide_mul(ar, ge->C[2], ge->C[0]));
  ge->N[1] = dyadic_wide_neg(ar, dyadic_wide_mul(ar, ge->C[2], ge->C[1]));
  ge->N[2] = dyadic_wide_sub(ar, dyadic_wide_mul(ar, P, P),
    dyadic_wide_mul(ar, ge->C[2], ge->C[2]));
  for (int i = 0; i < 3; i++)
  {
    int j = (i + 1) % 3, k = (i + 2) % 3;
    ge->NC[i] = dyadic_wide_sub(ar, dyadic_wide_mul(ar, ge->N[j], ge->C[k]),
      dyadic_wide_mul(ar, ge->N[k], ge->C[j]));
  }
  const DyadicWide *nn = dyadic_wide_of_int64(ar, 0);
  for (int i = 0; i < 3; i++)
    nn = dyadic_wide_add(ar, nn, dyadic_wide_mul(ar, ge->N[i], ge->N[i]));
  /* N . N scaled by an even power of 2 into [1, 4) */
  int e;
  (void) frexp(dyadic_wide_approx(nn), &e);
  ge->nn_e = (e - 1) / 2;
  ge->NN = dyadic_wide_mul(ar, nn,
    dyadic_wide_of_double(ar, ldexp(1.0, - 2 * ge->nn_e)));
  /* The point: a vertex of the lattice, or the crossing of p0 p1 with the
   * line e0 e1 at the fraction num / den of the segment, all on the lattice,
   * where crossing two lines commutes with the map (x, y) -> (2x, 2y/sqrt 3) */
  int64 D = 1;
  if (! p->crossing)
  {
    ge->U = dyadic_wide_of_int64(ar, p->p0[0]);
    ge->W = dyadic_wide_of_int64(ar, p->p0[1]);
  }
  else
  {
    int64 s1x = p->p1[0] - p->p0[0], s1y = p->p1[1] - p->p0[1];
    int64 s2x = p->e1[0] - p->e0[0], s2y = p->e1[1] - p->e0[1];
    int64 num = s2x * (p->p0[1] - p->e0[1]) - s2y * (p->p0[0] - p->e0[0]);
    D = - s2x * s1y + s1x * s2y;
    if (D < 0)
    {
      D = - D;
      num = - num;
    }
    const DyadicWide *wD = dyadic_wide_of_int64(ar, D);
    const DyadicWide *wnum = dyadic_wide_of_int64(ar, num);
    ge->U = dyadic_wide_add(ar, dyadic_wide_mul(ar,
      dyadic_wide_of_int64(ar, p->p0[0]), wD), dyadic_wide_mul(ar, wnum,
      dyadic_wide_of_int64(ar, s1x)));
    ge->W = dyadic_wide_add(ar, dyadic_wide_mul(ar,
      dyadic_wide_of_int64(ar, p->p0[1]), wD), dyadic_wide_mul(ar, wnum,
      dyadic_wide_of_int64(ar, s1y)));
  }
  int qg;
  ge->G = wide_of_decimal(ar, p->gnomonic, &qg);
  ge->GP = dyadic_wide_mul(ar, ge->G, P);
  ge->f1 = dyadic_wide_mul(ar, dyadic_wide_of_int64(ar, 2 * D),
    dyadic_wide_mul(ar, wide_pow(ar, 7, p->res / 2), wide_pow(ar, 10, qg)));
  if (p->substrate)
    ge->f1 = dyadic_wide_mul(ar, ge->f1, dyadic_wide_of_int64(ar, 3));
}

/* The azimuth of the first axis less the turn of the angle of the point,
 * enclosed: H3 turns the angle of the center of a Class III cell by
 * M_AP7_ROT_RADS before reading it from the axis (`_hex2dToVec3`) */
static WideInterval
gnomonic_axis(GnomonicExact *ge, int L, int K)
{
  WideInterval a = wide_interval_decimal(&ge->ar, ge->axis, L, K);
  if (ge->rotation)
    a = wide_interval_sub(&ge->ar, a,
      wide_interval_decimal(&ge->ar, ge->rotation, L, K));
  return a;
}

/**
 * @brief Enclose a vector of the direction of a point of a face at the scale
 * 2^-K (#GnomonicExact), scaled exactly by a power of 2 so that its largest
 * component lies below 2; false where an enclosure does not fit the limbs
 */
static bool
gnomonic_vector(GnomonicExact *ge, int L, int K, WideInterval *v)
{
  DyadicWideArena *ar = &ge->ar;
  WideInterval s3, s7, nn, sa, ca;
  if (! wide_interval_sqrt(ar, wide_interval_point(dyadic_wide_of_int64(ar,
        3)), L, K, &s3) ||
      ! wide_interval_sqrt(ar, wide_interval_point(ge->NN), L, K, &nn) ||
      ! wide_interval_sincos_rad(ar, gnomonic_axis(ge, L, K), L, K, &sa,
        &ca))
    return false;
  nn = wide_interval_ldexp(ar, nn, ge->nn_e);
  if (ge->odd)
  {
    if (! wide_interval_sqrt(ar, wide_interval_point(dyadic_wide_of_int64(ar,
          7)), L, K, &s7))
      return false;
  }
  else
    s7 = wide_interval_point(dyadic_wide_of_int64(ar, 1));
  WideInterval U = wide_interval_point(ge->U);
  WideInterval W3 = wide_interval_mul(ar, wide_interval_point(ge->W), s3);
  WideInterval a = wide_interval_add(ar, wide_interval_mul(ar, ca, U),
    wide_interval_mul(ar, sa, W3));
  WideInterval b = wide_interval_sub(ar, wide_interval_mul(ar, sa, U),
    wide_interval_mul(ar, ca, W3));
  WideInterval t1 = wide_interval_mul(ar, wide_interval_point(ge->f1),
    wide_interval_mul(ar, s7, nn));
  WideInterval ta = wide_interval_mul(ar, wide_interval_point(ge->GP), a);
  WideInterval tb = wide_interval_mul(ar, wide_interval_point(ge->G), b);
  int emax = INT_MIN;
  for (int i = 0; i < 3; i++)
  {
    v[i] = wide_interval_add(ar, wide_interval_add(ar,
      wide_interval_mul(ar, t1, wide_interval_point(ge->C[i])),
      wide_interval_mul(ar, ta, wide_interval_point(ge->N[i]))),
      wide_interval_mul(ar, tb, wide_interval_point(ge->NC[i])));
    for (int k = 0; k < 2; k++)
    {
      double d = dyadic_wide_approx(k ? v[i].hi : v[i].lo);
      if (d != 0.0)
      {
        int e;
        (void) frexp(d, &e);
        emax = Max(emax, e);
      }
    }
  }
  if (emax == INT_MIN)
    return false;
  for (int i = 0; i < 3; i++)
    v[i] = wide_interval_ldexp(ar, v[i], - emax);
  return true;
}

/**
 * @brief Enclose the magnitude of the longitude, or where @p lat the
 * latitude, of a point of a face in radians at the scale 2^-K and return its
 * sign, or 0 where this precision does not decide it
 * @details The longitude is atan2(vy, vx) and the latitude
 * atan2(vz, sqrt(vx^2 + vy^2)), as `vec3ToLatLng` reads them; the magnitude
 * is atan2 of the magnitude of the first over the second
 * (#wide_interval_atan2)
 */
static int
gnomonic_angle_enclose(GnomonicExact *ge, bool lat, int L, int K,
  uint32_t *lo, uint32_t *hi)
{
  DyadicWideArena *ar = &ge->ar;
  DyadicWideMark mark = dyadic_wide_arena_mark(ar);
  int sign = 0;
  WideInterval v[3];
  if (gnomonic_vector(ge, L, K, v))
  {
    WideInterval y, x, r;
    bool ok = true;
    if (lat)
    {
      y = v[2];
      ok = wide_interval_sqrt(ar, wide_interval_add(ar,
        wide_interval_mul(ar, v[0], v[0]), wide_interval_mul(ar, v[1], v[1])),
        L, K, &x);
    }
    else
    {
      y = v[1];
      x = v[0];
    }
    if (ok)
    {
      if (wide_sign(y.lo) > 0)
        sign = 1;
      else if (wide_sign(y.hi) < 0)
      {
        sign = -1;
        y = wide_interval_neg(ar, y);
      }
    }
    if (sign != 0 && (! wide_interval_atan2(ar, y, x, L, K, &r) ||
        fix_of_wide(r.lo, K, false, lo, L) == 2 ||
        fix_of_wide(r.hi, K, true, hi, L) == 2))
      sign = 0;
  }
  dyadic_wide_arena_release(ar, mark);
  return sign;
}

/** @brief The degrees of an angle atan2(sqrt(s), c) in [0, 180], or 90 less
 * it, or of the latitude atan(sinh(pi r)) of a Web Mercator row, or of the
 * longitude or latitude of a point of a face of a gnomonic grid, and an
 * enclosure of their magnitude, the twin of #TrigProduct */
typedef struct
{
  const DyadicWide *s, *c;   /**< The angle atan2(sqrt(s), c) */
  bool root;                /**< The angle is atan2(s, c) */
  bool mercator;            /**< The angle is atan(sinh(pi r)), not s and c */
  GnomonicExact *gnomonic;  /**< The angle of a point of a face, not s and c */
  bool lat;                 /**< The latitude of that point, not its longitude */
  double r;                 /**< The row of a Mercator angle, 0 < r <= 1 */
  bool complement;          /**< The degrees are 90 less the angle */
  int sign;                 /**< Sign of the degrees, -1 or 1 */
  int bits;                 /**< Precision of the enclosure */
  int exp;                  /**< Weight of the lowest bit of the bounds */
  int n64;                  /**< Number of 64-bit limbs of each bound */
  uint64 *lo64, *hi64;      /**< The bounds of the magnitude */
} DegreeAngle;

/**
 * @brief Enclose e^x at the scale 2^-K in [lo, hi], x in [0, 4) given by its
 * enclosure [xlo, xhi], K a multiple of 32
 * @details e^x is (e^(x/8))^8: x/8 lies below 1/2, where every term of the
 * Taylor series 1 + u + u^2/2! + ... is positive and the terms from the k-th
 * on sum to less than twice the k-th, so the sum stops at a term of at most
 * one unit and adds two units above; each term is read rounded down for
 * @p lo and up for @p hi, and so are the three squarings, as e^x increases
 * with x
 */
static void
fix_exp_enclose(const uint32_t *xlo, const uint32_t *xhi, uint32_t *lo,
  uint32_t *hi, int L, int K)
{
  uint32_t *ulo = palloc(L * sizeof(uint32_t));
  uint32_t *uhi = palloc(L * sizeof(uint32_t));
  uint32_t *tlo = palloc(L * sizeof(uint32_t));
  uint32_t *thi = palloc(L * sizeof(uint32_t));
  fix_shift_right(xlo, 3, ulo, L, false);
  fix_shift_right(xhi, 3, uhi, L, true);
  fix_set_pow2(tlo, L, K);
  fix_set_pow2(thi, L, K);
  memcpy(lo, tlo, L * sizeof(uint32_t));
  memcpy(hi, thi, L * sizeof(uint32_t));
  for (uint32_t k = 1; ; k++)
  {
    fix_mul(tlo, ulo, tlo, L, K, false);
    fix_div_small(tlo, k, tlo, L, false);
    fix_mul(thi, uhi, thi, L, K, true);
    fix_div_small(thi, k, thi, L, true);
    if (fix_top_bit(thi, L) <= 0)
    {
      /* The rest, this term included, is below two units */
      fix_set_pow2(tlo, L, 1);
      fix_add(hi, tlo, hi, L);
      break;
    }
    fix_add(lo, tlo, lo, L);
    fix_add(hi, thi, hi, L);
  }
  for (int i = 0; i < 3; i++)
  {
    fix_mul(lo, lo, lo, L, K, false);
    fix_mul(hi, hi, hi, L, K, true);
  }
  pfree(ulo); pfree(uhi); pfree(tlo); pfree(thi);
}

/**
 * @brief Enclose the latitude atan(sinh(pi r)) in radians of the row @p r of
 * the Web Mercator grid, 0 < r <= 1, at the scale 2^-K in [lo, hi]
 * @details The latitude is the Gudermannian 2 atan(e^t) - pi/2 of
 * t = pi r, which increases with t: t is read from the enclosure of pi and
 * the exact double @p r, e^t is enclosed by #fix_exp_enclose and its
 * arctangent by #fix_atan2_bound, every bound rounded outward
 */
static void
fix_mercator_enclose(double r, uint32_t *lo, uint32_t *hi, int L, int K)
{
  uint32_t *tlo = palloc(L * sizeof(uint32_t));
  uint32_t *thi = palloc(L * sizeof(uint32_t));
  uint32_t *elo = palloc(L * sizeof(uint32_t));
  uint32_t *ehi = palloc(L * sizeof(uint32_t));
  uint32_t *one = palloc(L * sizeof(uint32_t));
  /* r = m 2^e exactly, m an integer of 53 bits, and e < 0 as r <= 1 */
  int e;
  double frac = frexp(r, &e);
  uint64_t m = (uint64_t) ldexp(frac, 53);
  e -= 53;
  assert(e < 0);
  fix_pi(tlo, thi, L, K);
  fix_mul_u64(tlo, m, L);
  fix_mul_u64(thi, m, L);
  fix_shift_right(tlo, - e, tlo, L, false);
  fix_shift_right(thi, - e, thi, L, true);
  fix_exp_enclose(tlo, thi, elo, ehi, L, K);
  /* atan(e^t), then twice it less pi/2, pi/2 at the scale 2^-K being pi at
   * the scale 2^-(K-1) */
  fix_set_pow2(one, L, K);
  fix_atan2_bound(elo, one, tlo, L, K, false);
  fix_atan2_bound(ehi, one, thi, L, K, true);
  fix_mul_small(tlo, 2, tlo, L);
  fix_mul_small(thi, 2, thi, L);
  fix_pi(elo, ehi, L, K - 1);
  fix_sub_clamp(tlo, ehi, lo, L);
  fix_sub_clamp(thi, elo, hi, L);
  pfree(tlo); pfree(thi); pfree(elo); pfree(ehi); pfree(one);
}

/**
 * @brief Enclose the magnitude of the degrees of an angle at a precision of
 * @p bits, the twin of #trig_product_enclose
 * @details The angle in radians is enclosed by #fix_atan2_enclose_root, or
 * for the latitude of a Mercator row by #fix_mercator_enclose, and
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
  if (p->gnomonic)
  {
    /* A precision leaving the sign undecided gives the widest enclosure,
     * [0, pi], which the side reads as undecided and raises */
    if (gnomonic_angle_enclose(p->gnomonic, p->lat, L, K, tlo, thi) !=
        p->sign)
    {
      memset(tlo, 0, L * sizeof(uint32_t));
      fix_pi(plo, thi, L, K);
    }
  }
  else if (p->mercator)
    fix_mercator_enclose(p->r, tlo, thi, L, K);
  else
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
degree_angle_side(DyadicWideArena *ar, const DyadicWide *mid, const void *arg)
{
  DegreeAngle *p = *(DegreeAngle * const *) arg;
  while (true)
  {
    int clo = dyadic_wide_cmp_fixed(ar, mid, p->sign, p->lo64, p->n64, p->exp);
    int chi = dyadic_wide_cmp_fixed(ar, mid, p->sign, p->hi64, p->n64, p->exp);
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
  DyadicWideArena ar;
  dyadic_wide_arena_init(&ar);
  DegreeAngle p;
  memset(&p, 0, sizeof(p));
  p.s = wide_of_magnitude(&ar, y);
  p.root = true;
  p.c = dyadic_wide_of_int64(&ar, x);
  p.sign = y > 0 ? 1 : -1;
  double result = degree_angle_nearest(&p, atan2((double) y, (double) x) *
    (180.0 / M_PI));
  dyadic_wide_arena_free(&ar);
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
  DyadicWideArena arena, *ar = &arena;
  dyadic_wide_arena_init(ar);
  const DyadicWide *X = dyadic_wide_of_int64(ar, x),
    *Y = dyadic_wide_of_int64(ar, y);
  const DyadicWide *AZ = wide_of_magnitude(ar, z);
  const DyadicWide *ZZ = dyadic_wide_mul(ar, AZ, AZ);
  const DyadicWide *RR = dyadic_wide_add(ar, dyadic_wide_mul(ar, X, X), dyadic_wide_mul(ar, Y, Y));
  const DyadicWide *three = dyadic_wide_of_int64(ar, 3);
  bool rational = true;
  if (dyadic_wide_cmp(ZZ, RR) == 0)
    result = 45.0 * sz;
  else if (dyadic_wide_cmp(ZZ, dyadic_wide_mul(ar, three, RR)) == 0)
    result = 60.0 * sz;
  else if (dyadic_wide_cmp(dyadic_wide_mul(ar, three, ZZ), RR) == 0)
    result = 30.0 * sz;
  else
    rational = false;
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
  dyadic_wide_arena_free(ar);
  return result;
}

/**
 * @brief Return the double nearest the latitude in degrees of the row @p r of
 * the Web Mercator grid, atan(sinh(pi r)) in degrees, -1 <= @p r <= 1
 * @details A row boundary y of a grid of n tiles a side, or its centroid
 * y + 1/2, is the row r = 1 - 2 y / n, a double exactly for the grids of
 * QUADBIN. The latitude is odd in r and irrational for every r but 0
 * (e^(pi r) is transcendental, Gelfond): bounded pairs of doubles answer
 * wherever their bound places it inside the rounding cell of one double
 * (#ddb_mercator_lat_nearest), and elsewhere it is rounded by
 * #nearest_double_exact on an enclosure of its degrees
 * (#degree_angle_enclose, #fix_mercator_enclose)
 * @param[in] r Row, -1 to 1
 */
double
mercator_lat_nearest(double r)
{
  if (isnan(r))
    return NAN;
  assert(fabs(r) <= 1.0);
  if (r == 0.0)
    return 0.0;
  double result;
  if (ddb_mercator_lat_nearest(r, &result))
    return result;
  DegreeAngle p;
  memset(&p, 0, sizeof(p));
  p.mercator = true;
  p.r = fabs(r);
  p.sign = r > 0 ? 1 : -1;
  return degree_angle_nearest(&p, atan(sinh(M_PI * r)) * (180.0 / M_PI));
}

/** @brief A quotient of two exact values, for #rational_side */
typedef struct
{
  const DyadicWide *n, *d;
} RationalQuotient;

/* The sign of n / d - mid, d > 0, that of n - mid d */
static int
rational_side(DyadicWideArena *ar, const DyadicWide *mid, const void *arg)
{
  const RationalQuotient *q = (const RationalQuotient *) arg;
  return dyadic_wide_cmp(q->n, dyadic_wide_mul(ar, mid, q->d));
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
  DyadicWideArena ar;
  dyadic_wide_arena_init(&ar);
  RationalQuotient q = { dyadic_wide_of_int64(&ar, num),
    dyadic_wide_of_int64(&ar, den) };
  double result = nearest_double_exact((double) num / (double) den,
    rational_side, &q);
  dyadic_wide_arena_free(&ar);
  return result;
}

/**
 * @brief Return in @p lon and @p lat the doubles nearest the degrees of the
 * longitude and the latitude of a point of a face of an icosahedral gnomonic
 * grid, the value H3's formula gives a vertex of a cell (decision D8 of the
 * cell walks), every constant read on its decimal literal
 * @details Bounded pairs of doubles answer each coordinate their bound
 * places inside the rounding cell of one double
 * (#ddb_gnomonic_face_point_nearest). Elsewhere the direction of the point
 * is enclosed exactly (#GnomonicExact, #gnomonic_vector) and its longitude
 * and latitude are rounded by #nearest_double_exact on an enclosure of their
 * degrees
 * (#degree_angle_enclose), the sign of each read first at the precision that
 * decides it. The degrees are irrational, so the rounding never meets a
 * midpoint between two doubles.
 * @param[in] p Point
 * @param[out] lon,lat Its longitude and latitude in degrees
 */
void
gnomonic_face_point_nearest(const GnomonicFacePoint *p, double *lon,
  double *lat)
{
  /* Bounded pairs of doubles first, the enclosure where they cannot tell,
   * as #vector_lon_nearest reads #ddb_vector_lon_nearest first */
  bool decided[2];
  ddb_gnomonic_face_point_nearest(p, lon, lat, decided);
  if (decided[0] && decided[1])
    return;
  GnomonicExact ge;
  gnomonic_exact_init(&ge, p);
  for (int k = 0; k < 2; k++)
  {
    if (decided[k])
      continue;
    DegreeAngle a;
    memset(&a, 0, sizeof(a));
    a.gnomonic = &ge;
    a.lat = (k == 1);
    /* The sign and an estimate of the degrees, at the first precision that
     * decides the sign */
    double approx = 0.0;
    for (int bits = TRIG_FIRST_BITS; bits <= TRIG_LAST_BITS && ! a.sign;
         bits *= 2)
    {
      int K = ((bits + 31) / 32) * 32 + 32;
      int L = (2 * K + 192) / 32;
      uint32_t *tlo = palloc(L * sizeof(uint32_t));
      uint32_t *thi = palloc(L * sizeof(uint32_t));
      a.sign = gnomonic_angle_enclose(&ge, a.lat, L, K, tlo, thi);
      if (a.sign)
      {
        DyadicWideMark mark = dyadic_wide_arena_mark(&ge.ar);
        approx = a.sign * dyadic_wide_approx(wide_interval_of_fixed(&ge.ar,
          tlo, L, K, 1)) * (180.0 / M_PI);
        dyadic_wide_arena_release(&ge.ar, mark);
      }
      pfree(tlo);
      pfree(thi);
    }
    if (! a.sign)
    {
      dyadic_wide_arena_free(&ge.ar);
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "The sign of a coordinate of a point of a gnomonic face is undecided");
      return;
    }
    double d = degree_angle_nearest(&a, approx);
    if (k == 0)
      *lon = d;
    else
      *lat = d;
  }
  dyadic_wide_arena_free(&ge.ar);
}
