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
 * @brief A program that tests that the sort key of a set, of a span and of a
 * span set orders them as #set_cmp, #span_cmp and #spanset_cmp do
 * @details Over every pair of a corpus of sets of each set type with a sort
 * key (one, two and three values: negative values, zero of both signs,
 * extremes, infinities, NaN, and texts that are empty, prefixes of one
 * another, hold bytes above 0x7F or a zero byte), of spans of each span type
 * (negative values, zero of both signs, large magnitudes, infinities and
 * every bound flag) and of span sets built from them (one, two and three
 * spans, so that a set whose spans prefix another's is compared with it), the
 * sign of a byte comparison of the two keys must be the sign of the
 * comparison of the two values, and two values the comparison holds equal
 * must have identical keys. The program also verifies that #set_sortkey,
 * #span_sortkey and #spanset_sortkey report a null value and a null size, and
 * that #set_sortkey reports a set type without a sort key.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o sortkey_test sortkey_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_h3.h>
#include <meos_quadbin.h>
#include <meos_s2cell.h>
#include <pg_text.h>

#define MAXS 4096

static Set *sets[MAXS];
static int nsets;
static Span *spans[MAXS];
static int nspans;
static long failures;

static int
order_sign(int x)
{
  return (x > 0) - (x < 0);
}

/* The sign of an unsigned byte comparison, a shorter prefix sorting first */
static int
sortkey_order(const uint8_t *a, size_t na, const uint8_t *b, size_t nb)
{
  size_t n = na < nb ? na : nb;
  int c = memcmp(a, b, n);
  if (c)
    return order_sign(c);
  return (na > nb) - (na < nb);
}

static void
setcorpus_add(Set *s)
{
  if (s && nsets < MAXS)
    sets[nsets++] = s;
  meos_errno_reset();
}

/* Compare every pair of the set corpus by key and by value, then free it */
static void
setcorpus_check(const char *name)
{
  long pairs = 0, bad = 0;
  for (int i = 0; i < nsets; i++)
    for (int j = 0; j < nsets; j++)
    {
      size_t na, nb;
      uint8_t *ka = set_sortkey(sets[i], &na);
      uint8_t *kb = set_sortkey(sets[j], &nb);
      int c = order_sign(set_cmp(sets[i], sets[j]));
      if (sortkey_order(ka, na, kb, nb) != c)
        bad++;
      free(ka); free(kb);
      pairs++;
    }
  printf("%s: %ld set pairs, %ld ordered or equated otherwise than set_cmp\n",
    name, pairs, bad);
  failures += bad;
  for (int i = 0; i < nsets; i++)
    free(sets[i]);
  nsets = 0;
}

/* Add the sets of one value, of two values and of three values of an array */
#define SETCORPUS_BUILD(T, MAKE, VALUES, N) \
  do { \
    for (int a = 0; a < (N); a++) \
    { \
      T v1[1] = {(VALUES)[a]}; \
      setcorpus_add(MAKE(v1, 1)); \
      for (int b = a + 1; b < (N); b++) \
      { \
        T v2[2] = {(VALUES)[a], (VALUES)[b]}; \
        setcorpus_add(MAKE(v2, 2)); \
        for (int c = b + 1; c < (N); c += 2) \
        { \
          T v3[3] = {(VALUES)[a], (VALUES)[b], (VALUES)[c]}; \
          setcorpus_add(MAKE(v3, 3)); \
        } \
      } \
    } \
  } while (0)

static void
corpus_add(Span *s)
{
  if (s && nspans < MAXS)
    spans[nspans++] = s;
  meos_errno_reset();
}

/* Compare every pair of the corpus by key and by value, then free it */
static void
corpus_check(const char *name)
{
  long pairs = 0, bad = 0;
  for (int i = 0; i < nspans; i++)
    for (int j = 0; j < nspans; j++)
    {
      size_t na, nb;
      uint8_t *ka = span_sortkey(spans[i], &na);
      uint8_t *kb = span_sortkey(spans[j], &nb);
      int c = order_sign(span_cmp(spans[i], spans[j]));
      if (sortkey_order(ka, na, kb, nb) != c)
        bad++;
      free(ka); free(kb);
      pairs++;
    }
  printf("%s: %ld span pairs, %ld ordered or equated otherwise than span_cmp\n",
    name, pairs, bad);
  failures += bad;

  SpanSet *sets[3 * MAXS];
  int nsets = 0;
  for (int i = 0; i < nspans && nsets < 3 * MAXS - 3; i += 3)
  {
    sets[nsets++] = span_to_spanset(spans[i]);
    SpanSet *u2 = union_span_span(spans[i], spans[(i * 7 + 5) % nspans]);
    if (u2)
    {
      sets[nsets++] = u2;
      SpanSet *u3 = union_spanset_span(u2, spans[(i * 13 + 11) % nspans]);
      if (u3)
        sets[nsets++] = u3;
    }
  }
  pairs = bad = 0;
  for (int i = 0; i < nsets; i++)
    for (int j = 0; j < nsets; j++)
    {
      size_t na, nb;
      uint8_t *ka = spanset_sortkey(sets[i], &na);
      uint8_t *kb = spanset_sortkey(sets[j], &nb);
      int c = order_sign(spanset_cmp(sets[i], sets[j]));
      if (sortkey_order(ka, na, kb, nb) != c)
        bad++;
      free(ka); free(kb);
      pairs++;
    }
  printf("%s: %ld span set pairs, %ld ordered or equated otherwise than "
    "spanset_cmp\n", name, pairs, bad);
  failures += bad;

  for (int i = 0; i < nsets; i++)
    free(sets[i]);
  for (int i = 0; i < nspans; i++)
    free(spans[i]);
  nspans = 0;
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting, so that a span the constructors refuse (an
   * empty integer span) is skipped */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  const int ints[] = {-1000000000, -65536, -256, -5, -1, 0, 1, 2, 255, 256,
    257, 65536, 1000000000};
  const int ni = sizeof(ints) / sizeof(ints[0]);
  const double dbl[] = {-INFINITY, -1e300, -3.5, -1.0, -0.25, -0.0, 0.0, 0.25,
    1.0, 9.0, 1e300, INFINITY};
  const int nd = sizeof(dbl) / sizeof(dbl[0]);
  const int64 scale = 1000003;

  const int setints[] = {INT_MIN, -1000000000, -65536, -256, -5, -1, 0, 1, 2,
    255, 256, 257, 65536, 1000000000, INT_MAX};
  const int nsi = sizeof(setints) / sizeof(setints[0]);
  SETCORPUS_BUILD(int, intset_make, setints, nsi);
  setcorpus_check("intset");

  const int64 setbigints[] = {INT64_MIN, -1099511627776, -4294967296, -256,
    -1, 0, 1, 255, 256, 4294967296, 1099511627776, INT64_MAX};
  const int nsb = sizeof(setbigints) / sizeof(setbigints[0]);
  SETCORPUS_BUILD(int64, bigintset_make, setbigints, nsb);
  setcorpus_check("bigintset");

  const double setdbl[] = {-INFINITY, -1e300, -3.5, -1.0, -0.25, -0.0, 0.0,
    0.25, 1.0, 9.0, 1e300, INFINITY, NAN};
  const int nsd = sizeof(setdbl) / sizeof(setdbl[0]);
  SETCORPUS_BUILD(double, floatset_make, setdbl, nsd);
  setcorpus_check("floatset");

  DateADT setdates[nsi];
  for (int i = 0; i < nsi; i++)
    setdates[i] = setints[i] / 1000;
  SETCORPUS_BUILD(DateADT, dateset_make, setdates, nsi);
  setcorpus_check("dateset");

  TimestampTz settstz[nsi];
  for (int i = 0; i < nsi; i++)
  {
    settstz[i] = setints[i];
    settstz[i] *= scale;
  }
  SETCORPUS_BUILD(TimestampTz, tstzset_make, settstz, nsi);
  setcorpus_check("tstzset");

  /* Texts that are empty, prefixes of one another, differ in case and hold
   * bytes above 0x7F; the X of "aX" and "aXb" is then made a zero byte */
  const char *setstrs[] = {"", "\x01", "A", "Z", "a", "a b", "aX", "aXb", "ab",
    "abc", "abd", "b", "z", "\xc3\xa9", "\xc3\xa9t\xc3\xa9"};
  const int nst = sizeof(setstrs) / sizeof(setstrs[0]);
  text *settexts[nst];
  for (int i = 0; i < nst; i++)
  {
    settexts[i] = text_in(setstrs[i]);
    /* The bytes of the text follow the four-byte header text_in writes */
    char *x = memchr((char *) settexts[i] + 4, 'X', strlen(setstrs[i]));
    if (x)
      *x = '\0';
  }
  SETCORPUS_BUILD(text *, textset_make, settexts, nst);
  setcorpus_check("textset");
  for (int i = 0; i < nst; i++)
    free(settexts[i]);

  /* Cells, which are ordered as signed 64-bit integers */
  const uint64_t setcells[] = {UINT64_C(0x08001fffffffffff),
    UINT64_C(0x085283473fffffff), UINT64_C(0x08928308280fffff),
    UINT64_C(0x5000000000000000), UINT64_C(0x7fffffffffffffff)};
  const int nsc = sizeof(setcells) / sizeof(setcells[0]);
  SETCORPUS_BUILD(H3Index, h3indexset_make, setcells, nsc);
  setcorpus_check("h3indexset");
  SETCORPUS_BUILD(Quadbin, quadbinset_make, setcells, nsc);
  setcorpus_check("quadbinset");
  SETCORPUS_BUILD(S2CellId, s2cellset_make, setcells, nsc);
  setcorpus_check("s2cellset");

  for (int a = 0; a < ni; a++)
    for (int b = a; b < ni; b++)
      for (int f = 0; f < 4; f++)
        corpus_add(intspan_make(ints[a], ints[b] + (a == b), f & 1, f & 2));
  corpus_check("intspan");

  for (int a = 0; a < ni; a++)
    for (int b = a; b < ni; b++)
      for (int f = 0; f < 4; f++)
      {
        int64 lower = ints[a], upper = ints[b];
        corpus_add(bigintspan_make(lower * scale, upper * scale + (a == b),
          f & 1, f & 2));
      }
  corpus_check("bigintspan");

  for (int a = 0; a < nd; a++)
    for (int b = a; b < nd; b++)
      for (int f = 0; f < 4; f++)
      {
        bool lower_inc = f & 1, upper_inc = f & 2;
        if (dbl[a] == dbl[b])
          lower_inc = upper_inc = true;
        corpus_add(floatspan_make(dbl[a], dbl[b], lower_inc, upper_inc));
      }
  corpus_check("floatspan");

  for (int a = 0; a < ni; a++)
    for (int b = a; b < ni; b++)
      for (int f = 0; f < 4; f++)
        corpus_add(datespan_make(ints[a] / 1000, ints[b] / 1000 + 1, f & 1,
          f & 2));
  corpus_check("datespan");

  for (int a = 0; a < ni; a++)
    for (int b = a; b < ni; b++)
      for (int f = 0; f < 4; f++)
      {
        bool lower_inc = f & 1, upper_inc = f & 2;
        if (a == b)
          lower_inc = upper_inc = true;
        TimestampTz lower = ints[a], upper = ints[b];
        corpus_add(tstzspan_make(lower * scale, upper * scale, lower_inc,
          upper_inc));
      }
  corpus_check("tstzspan");

  /* A null value or a null size is reported rather than dereferenced, and a
   * set type without a sort key is reported */
  size_t size;
  int one = 1;
  Set *set = intset_make(&one, 1);
  Set *geoms = geomset_in("{Point(1 1), Point(2 2)}");
  meos_errno_reset();
  uint8_t *key = set_sortkey(NULL, &size);
  printf("set_sortkey(NULL, size): %s, errno %d\n", key ? "a key" : "NULL",
    meos_errno());
  assert(key == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  key = set_sortkey(set, NULL);
  printf("set_sortkey(set, NULL): %s, errno %d\n", key ? "a key" : "NULL",
    meos_errno());
  assert(key == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  key = set_sortkey(geoms, &size);
  printf("set_sortkey(geomset, size): %s, errno %d\n", key ? "a key" : "NULL",
    meos_errno());
  assert(key == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  meos_errno_reset();
  key = set_sortkey(set, &size);
  printf("set_sortkey(set, size): %s of %zu bytes, errno %d\n",
    key ? "a key" : "NULL", size, meos_errno());
  assert(key != NULL && meos_errno() == 0);
  free(key); free(geoms); free(set);

  Span *s = intspan_make(1, 2, true, true);
  SpanSet *ss = span_to_spanset(s);
  meos_errno_reset();
  key = span_sortkey(NULL, &size);
  printf("span_sortkey(NULL, size): %s, errno %d\n", key ? "a key" : "NULL",
    meos_errno());
  assert(key == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  key = span_sortkey(s, NULL);
  printf("span_sortkey(span, NULL): %s, errno %d\n", key ? "a key" : "NULL",
    meos_errno());
  assert(key == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  key = spanset_sortkey(NULL, &size);
  printf("spanset_sortkey(NULL, size): %s, errno %d\n", key ? "a key" : "NULL",
    meos_errno());
  assert(key == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  key = spanset_sortkey(ss, &size);
  printf("spanset_sortkey(span set, size): %s of %zu bytes, errno %d\n",
    key ? "a key" : "NULL", size, meos_errno());
  assert(key != NULL && meos_errno() == 0);
  free(key); free(ss); free(s);

  printf("%ld pair(s) ordered or equated otherwise than their comparison\n",
    failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
