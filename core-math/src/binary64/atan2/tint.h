/* Fast 192-bit arithmetic routines.

Copyright (c) 2023-2024 Paul Zimmermann and Alexei Sibidanov

This file is part of the CORE-MATH project
(https://core-math.gitlabpages.inria.fr/).

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <inttypes.h>
#if defined(__x86_64__)
#include <x86intrin.h>
#endif

#if (defined(__clang__) && __clang_major__ >= 14) || (defined(__GNUC__) && __GNUC__ >= 14 && __BITINT_MAXWIDTH__ && __BITINT_MAXWIDTH__ >= 128)
typedef unsigned _BitInt(128) u128;
#else
typedef unsigned __int128 u128;
#endif

#include <stdlib.h>

// the following represent (-1)^sgn*(h/2^64+m/2^128+l/2^192)*2^ex
// we have either h=m=l=0 to represent +0 or -0
// or the most significant bit of h is 1
typedef union {
  struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint64_t m, h, l; // put m before h on little-endian processor
#else
    uint64_t h, m, l; // put h before m on big-endian processor
#endif
    int64_t ex;
    uint64_t sgn;
  };
  struct {
    u128 _h;
    uint64_t _l;
    int64_t _ex;
    uint64_t _sgn;
  };
} tint_t;

// ZERO is a tint_t representation of 0
static const tint_t ZERO = {.h = 0, .m = 0, .l = 0, .ex = -1076, .sgn = 0};

// ONE is a tint_t representation of 1
static const tint_t ONE = {
  .h = 0x8000000000000000, .m = 0, .l = 0, .ex = 1, .sgn = 0};

// PI is a tint_t representation of pi, with error < 2^-196.96
static const tint_t PI = {
  .h = 0xc90fdaa22168c234, .m = 0xc4c6628b80dc1cd1, .l = 0x29024e088a67cc74,
  .ex = 2, .sgn = 0};

// PI2 is a tint_t representation of pi/2, with error < 2^-197.96
static const tint_t PI2 = {
  .h = 0xc90fdaa22168c234, .m = 0xc4c6628b80dc1cd1, .l = 0x29024e088a67cc74,
  .ex = 1, .sgn = 0};

// Print a tint_t value for debugging purposes
static inline void print_tint (const tint_t *a) {
  printf("{.h=0x%"PRIx64", .m=0x%"PRIx64", .l=0x%"PRIx64", .ex=%"PRId64", .sgn=0x%"PRIx64"}\n",
         a->h, a->m, a->l, a->ex, a->sgn);
}
// Copy a tint_t value
static inline void cp_tint(tint_t *r, const tint_t *a) {
  r->_h = a->_h;
  r->_l = a->_l;
  r->_ex = a->_ex;
  r->_sgn = a->_sgn;
}

static inline int
is_normalized (const tint_t *a)
{
  if (a->h == 0 && a->m == 0 && a->l == 0)
    return 1;
  return a->h >> 63;
}

// Multiply two tint_t numbers, with error < 10 ulps or 2^-187.67 * |r|
// Overlap between r and a or b is allowed
static inline void
mul_tint (tint_t *r, const tint_t *a, const tint_t *b)
{
  r->ex = a->ex + b->ex;
  r->sgn = a->sgn ^ b->sgn;

  u128 ah = a->h, am = a->m, al = a->l;
  u128 bh = b->h, bm = b->m, bl = b->l;
  u128 rh = ah * bh, rm1 = ah * bm, rm2 = am * bh;
  u128 rl1 = ah * bl, rl2 = am * bm, rl3 = al * bh;
  uint64_t h, l, cm;
  r->h = rh >> 64;
  r->m = rh; // cast to low 64 bits
  // accumulate rm1
  r->l = rm1; // cast to low 64 bits
  h = rm1 >> 64;
  r->m += h;
  r->h += r->m < h; // no overflow possible
  // accumulate rm2
  l = rm2; // cast to low 64 bits
  h = rm2 >> 64;
  r->l += l;
  cm = r->l < l; // carry at r->m
  r->m += h;
  r->h += r->m < h; // no overflow possible
  // accumulate rl1+rl2+rl3
  rl1 = (rl1 >> 64) + (rl2 >> 64) + (rl3 >> 64);
  l = rl1; // cast to low 64 bits
  cm += rl1 >> 64;
  r->l += l;
  cm += r->l < l; // carry at r->m
  // accumulate cm
  r->m += cm;
  r->h += r->m < cm;

  /* Note: if one of the operands was zero, then r->h = r->m = r->l = 0,
     and the normalization keeps r=0. */
  if (!(r->h >> 63)) // normalize
  {
    r->h = (r->h << 1) | (r->m >> 63);
    r->m = (r->m << 1) | (r->l >> 63);
    r->l = r->l << 1;
    r->ex --;
  }

  /* We ignored the following terms, denoting B=2^64, related to r->l:
     am*bl + al*bm <= 2*(B-1)^2/B^2 = 2 - 4/B + 2/B^2
     al*bl <= (B-1)^2/B^3 = 1/B - 2/B^2 + 1/B^3
     And we truncated rl1+rl2+rl3:
     low(rl1) + low(rl2) + low(rl3) <= 3*(B-1)/B = 3-3/B
     This sums up to: 5 - 6/B + 1/B^3 < 5
     thus the rounding error is bounded by 5 ulps,
     and after normalization by 10 ulps. */
}

// Return non-zero if a = 0
static inline int
tint_zero_p (const tint_t *a)
{
  return a->h == 0;
}

static inline int cmp(int64_t a, int64_t b) { return (a > b) - (a < b); }
static inline int cmpu64(uint64_t a, uint64_t b) { return (a > b) - (a < b); }
static inline int cmpu128(u128 a, u128 b) { return (a > b) - (a < b); }

// Compare the absolute values of a and b
// Return -1 if |a| < |b|
// Return  0 if |a| = |b|
// Return +1 if |a| > |b|
static inline int
cmp_tint_abs (const tint_t *a, const tint_t *b) {
  if (tint_zero_p (a))
    return tint_zero_p (b) ? 0 : -1;
  if (tint_zero_p (b))
    return +1;
  int c = cmp (a->ex, b->ex);
  if (c)
    return c;
  // now a->ex = b->ex
  c = cmpu128 (a->_h, b->_h);
  if (c)
    return c;
  return cmpu64 (a->_l, b->_l);
}

// shift right by k bits (only deal with the significand)
static inline void
rshift (tint_t *a, const tint_t *b, int k)
{
  if (k == 0)
  {
    a->_h = b->_h;
    a->_l = b->_l;
  }
  else if (k < 64)
  {
    a->_h = b->_h >> k;
    a->_l = (b->_h << (64 - k)) | (b->_l >> k);
  }
  else if (k == 64)
  {
    a->_h = b->_h >> k;
    a->_l = b->_h;
  }
  else if (k < 128)
  {
    a->_h = b->_h >> k;
    a->_l = b->_h >> (k - 64);
  }
  else if (k < 192)
  {
    a->_h = 0;
    a->_l = b->_h >> (k - 64);
  }
  else
    a->_h = a->_l = 0;
  // printf ("exit rshift a="); print_tint (a);
}

// shift left by k bits (only deal with the significand)
static inline void
lshift (tint_t *a, const tint_t *b, int k)
{
  if (k == 0)
  {
    a->_h = b->_h;
    a->_l = b->_l;
  }
  else if (k < 64)
  {
    a->_h = (b->_h << k) | (b->_l >> (64 - k));
    a->_l = b->_l << k;
  }
  else if (k == 64)
  {
    a->_h = (b->_h << k) | (u128) b->_l;
    a->_l = 0;
  }
  else if (k < 128)
  {
    a->_h = b->_h << k | ((u128) b->_l << (k - 64));
    a->_l = 0;
  }
  else if (k < 192)
  {
    a->_h = (u128) b->_l << (k - 64);
    a->_l = 0;
  }
  else
    a->_h = a->_l = 0;
}

// Add two tint_t values, with error bounded by 2 ulps
static inline void
add_tint (tint_t *r, const tint_t *a, const tint_t *b)
{
  switch (cmp_tint_abs (a, b))
  {
  case 0: // |a| = |b|, return 0 or 2*a (exact)
    if (a->sgn ^ b->sgn) {
      cp_tint (r, &ZERO);
      return;
    }
    cp_tint (r, a);
    r->ex++;
    return;

  case -1: // |a| < |b|
    {
      // swap operands
      const tint_t *tmp = a; a = b; b = tmp;
      break; // fall through the case |a| > |b|
    }
  }

  // From now on, |a| > |b| thus a->ex >= b->ex
  tint_t t[1];
  uint64_t sh = a->ex - b->ex;
  rshift (t, b, sh);

  if (a->sgn ^ b->sgn) { // opposite signs, it's a subtraction
    t->_l = a->_l - t->_l;
    t->_h = a->_h - t->_h - (t->_l > a->_l);
    uint64_t th = t->_h >> 64;
    uint64_t ex =
      th ? __builtin_clzll (th)
      : (t->_h ? 64 + __builtin_clzll (t->_h) : 128 + __builtin_clzll (t->_l));
    if (ex <= 1 || sh == 0) {
      /* The maximal error of 1 ulp for the neglected low part of b is shifted
         by ex bits, thus contributes to < 2 ulps. And for sh=0, there is no
         neglected low part of b, thus the subtraction is exact. */
      lshift (r, t, ex);
      r->ex = a->ex - ex;
    }
    else { // ex >= 2 and sh >= 1
      /* Since we had a cancellation of ex bits with the neglected low part
         of b, we'll have a cancellation of at least ex bits if we don't
         neglect the low part of b, thus we can shift left a and b/2^sh by
         ex bits, and subtract. This case (ex >= 2) can only occur when sh=1,
         since for sh>=2 we have (up to the exponent) a >= 1 and b < 0.5,
         thus a-b >= 0.5, and the exponent decrease ex is at most 1. */
      lshift (t, b, ex - sh);
      lshift (r, a, ex);
      t->_l = r->_l - t->_l;
      t->_h = r->_h - t->_h - (t->_l > r->_l);
      th = t->_h >> 64;
      uint64_t ex1 =
        th ? __builtin_clzll (th)
        : (t->_h ? 64 + __builtin_clzll (t->_h) : 128 + __builtin_clzll (t->_l));
      lshift (r, t, ex1);
      r->ex = a->ex - (ex + ex1);
      /* Since we shifted b left in this case, there is no neglected bit of b,
         and the subtraction is exact. */
    }      
  }
  else { // same signs, it's an addition
    // save values of ah, al in case r=a
    u128 ah = a->_h;
    uint64_t al = a->_l;
    r->_l = al + t->_l;
    uint64_t cl = r->_l < al;
    r->_h = ah + t->_h;
    uint64_t ch = r->_h < ah;
    r->_h += cl;
    ch += r->_h < cl;
    // up to here, the maximal error is < ulp(r) [shifted part of b]
    if (ch) { // can be at most 1
      r->ex = a->ex + 1;
      r->_l = (r->_h << 63) | (r->_l >> 1);
      r->_h = ((u128) ch << 127) | (r->_h >> 1);
      /* the maximal error from the shifted part of b is now < 1/2 ulp(r),
         and in addition the low bit of r->_l that disappeared might give
         1/2 ulp(r), thus the total error is still < ulp(r) */
    }
    else
      r->ex = a->ex;
    // in the addition case, the error is bounded by ulp(r)
  }
  r->sgn = a->sgn;
}

// a <- x, assuming x is not NaN, Inf or 0
// This operation is exact
static inline void tint_fromd (tint_t *a, double x)
{
  d64u64 u = {.f = x};
  a->sgn = u.u >> 63;
  uint64_t ax = u.u & 0x7fffffffffffffffull;
  int64_t e = ax >> 52;
  if (__builtin_expect (e, 1)) { // normal
    // 1 has e=0x3ff
    a->ex = e - 0x3fe;
    a->h = (1ull << 63) | (ax << 11);
  }
  else { // subnormal
    // 2^-1074 has ax=1
    e = __builtin_clzll (ax);
    a->ex = -0x3f2 - e;
    a->h = ax << e;
  }
  a->m = a->l = 0;
}

// copied from ../exp/exp.c
static inline double as_ldexp(double x, int64_t i){
#ifdef __x86_64__
  __m128i sb; sb[0] = (uint64_t)i<<52;
#if defined(__clang__)
    __m128d r = _mm_set_sd(x);
#else
    __m128d r; asm("":"=x"(r):"0"(x));
#endif
    r = (__m128d)_mm_add_epi64(sb, (__m128i)r);
    return r[0];
#else
    d64u64 ix = {.f = x};
    ix.u += (uint64_t)i<<52;
    return ix.f;
#endif
}

// convert a to a double with correct rouding
// If err=0, we are converting a double value.
// Otherwise, err is a bound in ulps on the maximal error on a->l
// y,x are the inputs of atan2 (in case we can't round correctly)
static inline double
tint_tod (const tint_t *a, uint64_t err, double y, double x)
{
  if (a->ex >= 1025) // overflow: |a| >= 2^1024
    return a->sgn ? -0x1p1023 - 0x1p1023 : 0x1p1023 + 0x1p1023;
  if (a->ex <= -1074) // underflow: |a| < 2^-1074
  {
    if (a->ex < -1074) // |a| < 2^-1075
      return (a->sgn ? -0x1p-1074 : 0x1p-1074) * 0.5;
    // 2^-1075 <= |a| < 2^-1074
    int mid = a->h == (1ull << 63) && a->m == 0 && a->l == 0;
    // if mid, |a| = 2^-1075
    return (a->sgn ? -0x1p-1074 : 0x1p-1074) * (mid ? 0.5 : 0.75);
  }
#define MASK53 0x1ffffffffffffful
  uint64_t hh = a->h, mm = a->m, ll = a->l;
  int ex = a->ex;
  uint64_t low = hh & 0x7ff; // low 11 bits from a->h
  /* We can't determine the correct rounding when:
     (a) a->m = 0 and the low 10 bits of a->h are zero and a->l < err
     (b) a->m = 111...111 and the low 10 bits of a->h are 1 and
         a->l > 2^64 - err */
  if (__builtin_expect (mm == 0 || ~mm == 0, 0))
    if ((mm == 0 && (low == 0 || low == 0x400) && ll < err) ||
        (~mm == 0 && (low == 0x3ff || low == 0x7ff) && ~ll < err))
    {
      printf ("Unexpected worst-case found, please report to core-math@inria.fr:\n");
      printf ("Worst-case of atan2 found: y,x=%la,%la\n", y, x);
      exit (1);
    }
  if (ex <= -1022) // subnormal case
  {
    int sh = -1021 - ex; // 1 <= sh <= 52
    ll = (mm << (64 - sh)) | (ll >> sh) | (ll > 0);
    mm = (hh << (64 - sh)) | (mm >> sh);
    hh = hh >> sh;
    low = hh & 0x7ff;
    ex += sh;
  }
  double h = hh >> 11, l; // significant bits from a->h
  /* If err=0, we are converting a double value, thus low=0, and the
     conversion is exact. */
  if (err == 0)
    l = 0;
  else if (low < 0x400)
    l = 0.25; // round to zero
  else if (low > 0x400)
    l = 0.75; // round away
  else // low = 0x400
  {
    if (mm == 0 && ll == 0)
      l = 0.5; // middle case
    else
      l = 0.75; // round away
  }
  static const double S[2] = {1.0, -1.0};
  double s = S[a->sgn];
  h = __builtin_fma (l, s, s * h);
  h *= 0x1p-52;
  // now -1021 <= ex <= 1024, thus 2^(ex-1) does not underflow/overflow
  return h * as_ldexp (1.0, ex - 1);
}

/* Put in r an approximation of 1/A, assuming A is not zero.
   Assuming 1 <= r <= 2, the absolute error is bounded by 2^-103.9.
   In terms of ulp, it is bounded by 2^87.1 * ulp(r). */
static inline void inv_tint (tint_t *r, const tint_t *A)
{
  tint_t q[1];
  double a = tint_tod (A, 0, 0, 0); // exact
  // To simplify the error analysis, we assume 0.5 <= a < 1
  int subnormal = __builtin_fabs (a) < 0x1p-1022;
  if (subnormal)
    a *= 0x1p53;
  tint_fromd (r, 1.0 / a); // accurate to about 53 bits
  if (subnormal)
    r->ex += 53;
  /* We have 1 <= r <= 2, with |r - 1/a| < ulp(r) = 2^-52. */
  /* We use Newton's iteration: r1 = r0 + r0*(1-a*r0).
     Let e0 = 1-a*r0 and e1 = 1-a*r1 then we have e1 = e0^2.
     Since a < 1 and |r - 1/a| < 2^-52 we have e0 < 2^-52 thus e1 < 2^-104.
  */
  mul_tint (q, A, r);      // approximates a*r
  /* The rounding error in mul_tint is bounded by 10 ulps (on 192 bits),
     and a*r < 2, thus we have |q - ar| < 10*2^-191 < 2^-187.
     This error is multiplied by r below, thus contributes to < 2^-186. */
  q->sgn = 1 - q->sgn;     // -a*r
  add_tint (q, &ONE, q);   // approximates 1-a*r
  /* The rounding error in add_tint is bounded by 2 ulps (on 192 bits),
     and we have |round(1-a*r)| < 2^-51 thus this is bounded by 2*2^-243
     = 2^-242. This error is multiplied by r below, thus contributes to
     < 2^-241. */
  mul_tint (q, r, q);      // approximates r*(1-a*r)
  /* Since |q_in| < 2^-51 and r <= 2, we have |q| < 2^-50, and the rounding
     error on mul_tint is bounded by 10 ulps (on 192 bits), it is bounded
     by 10*2^-242 < 2^-238. */
  add_tint (r, r, q);
  /* The rounding error in add_tint is bounded by 2 ulps (on 192 bits),
     and we have |r| <= 2, thus it is bounded by 2*2^-191 = 2^-190. */

  /* The total error on r is bounded by:
   * 2^-104 for the mathematical error
   * 2^-186 + 2^-241 + 2^-238 + 2^-190 < 2^-185 for the rounding errors,
   thus by 2^-103.9.
   Since 1 <= r <= 2, this is bounded by 2^87.1 * ulp(r). */
}

/* Put in r an approximation of b/a, assuming a is not zero,
   with relative error bounded by 2^-185.53. */
static inline void div_tint (tint_t *r, tint_t *b, tint_t *a)
{
  tint_t Y[1], Z[1];
  // to simplify the error analysis, we assume 0.5 <= a, b < 1
  inv_tint (Y, a); // |Y - 1/a| < 2^-103.9, with Y <= 2
  mul_tint (r, Y, b); // r approximates b/a
  /* The rounding error of mul_tint is at most 10 ulps, thus since r <= 2
     this corresponds to < 2^187: |r - Y*b| < 2^187. Now |Y - 1/a|,
     thus |r - b/a| <= |r - Y*b| + b*|Y-1/a| < 2^187 + 2^-103.9 < 2^-103.89. */
  /* We use Karp-Markstein's trick: r1 = r0 + y*(b-a*r0).
     Let e0 = b-a*r0 and e1 = b-a*r1, we have e1 = e0*(1-a*y).
     Since |Y - 1/a| < 2^-103.9 and a < 1 we have |1-a*y| < 2^-103.9.
     Since |r - b/a| < 2^-103.89 and a < 1 we have |e0| < 2^-103.89.
     This gives |e1| < 2^-103.89 * 2^-103.9 < 2^-207. */
  mul_tint (Z, a, r);  // approximates a*r
  /* We have |z| <= 2 and the rounding error is bounded by 10 ulps,
     thus < 2^-187.67. This error is multiplied by Y <= 2 below,
     thus contributes to < 2^-186.67. */
  Z->sgn = 1 - Z->sgn; // -a*r
  add_tint (Z, b, Z);  // approximates b-a*r
  /* Since |b-a*r| < 2^-103.89, and |Zin-ar| < 2^-186.67, we have
     |b-Zin| < 2^-103.89 + 2^-186.67 < 2^-103.88, thus since the
     rounding error is at most 2 ulps, it is < 2*2^-295 = 2^-294.
     This error is multiplied by Y <= 2 below, thus contributes to < 2^-293. */
  mul_tint (Z, Y, Z);  // approximates y*(b-a*r)
  /* Since Y <= 2 and |Zin| < 2^-103.88, we have |Z| < 2^-102.88.
     The rounding error of mul_tint is thus bounded by 10*ulp(2^-102.88)
     = 10 * 2^-294 < 2^-290.67. */
  add_tint (r, r, Z);
  /* Since r <= 2, and the rounding error is bounded by 2 ulps,
     it is bounded by 2 * ulp(1.5) = 2 * 2^-191 = 2^-190. */

  /* The total error is bounded by:
   * 2^-207 for the mathematical error e1
   * 2^-186.67 + 2^-293 + 2^-290.67 + 2^-190
   This gives a bound of 2^-186.53 for 1/2 <= r <= 2,
   thus a relative error < 2^185.53. */
}

// same as div_tint, taking doubles, same relative error bound of 2^185.53
static inline void div_tint_d (tint_t *r, double b, double a)
{
  tint_t A[1], B[1];

  tint_fromd (A, a);
  tint_fromd (B, b);
  div_tint (r, B, A);
}
