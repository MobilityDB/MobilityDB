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
 * @brief A program that tests that the comparison of JSON values is the
 * order PostgreSQL states for jsonb, and that their sort keys order the sets
 * and the temporal values of JSON values as their comparison does
 * @details #jsonb_cmp is the order of PostgreSQL's compareJsonbContainers,
 * which reads the two values token by token and stops at the first that
 * differs: a null precedes a string, a number, a Boolean, an array and an
 * object, in that order; two arrays are ordered by their number of elements
 * first, and of two arrays of one element a scalar standing alone precedes
 * an array; two objects are ordered by their number of pairs first; strings
 * are ordered by their bytes, a prefix first, and numbers by their value. A
 * sort key is the string of bytes whose unsigned byte order is the order of
 * the comparison. The program therefore checks:
 * - over pairs stated beside each rule, that the comparison answers as the
 *   rule states;
 * - over every pair of a corpus of values, that the comparison answers 0
 *   exactly when the equality holds and that swapping the operands swaps the
 *   sign, and over every triple, that it is transitive;
 * - over every pair of sets of one and of two values, and of temporal values
 *   of one and of two instants, that the byte order of the keys is the order
 *   of #set_cmp and #temporal_cmp.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o json_sortkey_test json_sortkey_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_json.h>

#define MAXV 256

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

/*****************************************************************************
 * The comparison of the base values
 *****************************************************************************/

/* Pairs whose order a rule of compareJsonbContainers states: the first
 * precedes the second, or equals it when the order is 0 */
static const struct
{
  const char *a;
  const char *b;
  int order;
} STATED[] = {
  /* The scalar types: null, string, number, Boolean */
  {"null", "\"\"", -1},
  {"\"zzz\"", "-1e20", -1},
  {"1e20", "false", -1},
  {"false", "true", -1},
  /* A scalar standing alone is an array of one element, so the empty array
   * precedes it, and it precedes an array of one element */
  {"[]", "null", -1},
  {"5", "[5]", -1},
  {"5", "[]", 1},
  /* Containers: an array precedes an object; the number of elements or of
   * pairs decides before the elements */
  {"[true]", "{}", -1},
  {"[3]", "[1, 2]", -1},
  {"{\"b\": 1}", "{\"a\": 1, \"b\": 1}", -1},
  /* An element that is a container follows one that is a scalar */
  {"[null]", "[[]]", -1},
  /* Strings by their bytes, a prefix first */
  {"\"a\"", "\"ab\"", -1},
  {"\"ab\"", "\"b\"", -1},
  {"{\"aa\": 1}", "{\"b\": 1}", -1},
  /* Numbers by their value, whatever their scale */
  {"1.0", "1.00", 0},
  {"-0", "0", 0},
  {"0.1", "0.10000001", -1},
  {"9999", "10000", -1},
  {"-10000", "-9999.5", -1},
  {"1e-5", "0.00001", 0},
  {"[1, \"a\"]", "[1, \"b\"]", -1},
  {"{\"a\": [1, 2]}", "{\"a\": [1, 3]}", -1},
};

static void
stated_check(void)
{
  long pairs = 0, bad = 0;
  for (size_t i = 0; i < sizeof(STATED) / sizeof(STATED[0]); i++)
  {
    Jsonb *a = jsonb_in(STATED[i].a), *b = jsonb_in(STATED[i].b);
    meos_errno_reset();
    if (! a || ! b || order_sign(jsonb_cmp(a, b)) != STATED[i].order)
    {
      printf("  %s against %s: %d, stated %d\n", STATED[i].a, STATED[i].b,
        (a && b) ? order_sign(jsonb_cmp(a, b)) : 9, STATED[i].order);
      bad++;
    }
    free(a); free(b);
    pairs++;
  }
  printf("jsonb: %ld stated pairs, %ld answered otherwise than stated\n",
    pairs, bad);
  failures += bad;
}

/* Check that the comparison is antisymmetric, transitive and consistent with
 * the equality, over every pair and every triple of values */
static void
base_check(Jsonb **v, int n)
{
  long pairs = 0, inconsistent = 0, asymmetric = 0, triples = 0,
    intransitive = 0;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      int c = order_sign(jsonb_cmp(v[i], v[j]));
      if ((c == 0) != jsonb_eq(v[i], v[j]))
        inconsistent++;
      if (c != -order_sign(jsonb_cmp(v[j], v[i])))
        asymmetric++;
      pairs++;
    }
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      int cij = order_sign(jsonb_cmp(v[i], v[j]));
      if (cij > 0)
        continue;
      for (int k = 0; k < n; k++)
      {
        int cjk = order_sign(jsonb_cmp(v[j], v[k]));
        if (cjk > 0)
          continue;
        /* i <= j and j <= k, so i <= k, and i == k only when both are equal */
        int cik = order_sign(jsonb_cmp(v[i], v[k]));
        if (cik > 0 || (cik == 0 && (cij != 0 || cjk != 0)))
          intransitive++;
        triples++;
      }
    }
  printf("jsonb: %ld pairs, %ld whose comparison answers 0 otherwise than "
    "their equality, %ld whose swapped comparison is not the opposite; %ld "
    "ordered triples, %ld not transitive\n", pairs, inconsistent, asymmetric,
    triples, intransitive);
  failures += inconsistent + asymmetric + intransitive;
}

/*****************************************************************************
 * The keys of the sets and of the temporal values
 *****************************************************************************/

/* Check that the keys of every pair of sets order them as #set_cmp does */
static void
set_check(Set **sets, int n)
{
  long pairs = 0, bad = 0, refused = 0;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      size_t na, nb;
      meos_errno_reset();
      uint8_t *ka = set_sortkey(sets[i], &na);
      uint8_t *kb = set_sortkey(sets[j], &nb);
      meos_errno_reset();
      int c = set_cmp(sets[i], sets[j]);
      meos_errno_reset();
      if (! ka || ! kb)
        refused++;
      else if (sortkey_order(ka, na, kb, nb) != order_sign(c))
        bad++;
      free(ka); free(kb);
      pairs++;
    }
  printf("jsonbset: %ld set pairs, %ld without a key, %ld ordered or equated "
    "otherwise than set_cmp\n", pairs, refused, bad);
  failures += refused + bad;
}

/* Check that the keys of every pair of temporal values order them as
 * #temporal_cmp does */
static void
temporal_check(Temporal **temps, int n)
{
  long pairs = 0, bad = 0, refused = 0;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      size_t na, nb;
      meos_errno_reset();
      uint8_t *ka = temporal_sortkey(temps[i], &na);
      uint8_t *kb = temporal_sortkey(temps[j], &nb);
      meos_errno_reset();
      int c = temporal_cmp(temps[i], temps[j]);
      meos_errno_reset();
      if (! ka || ! kb)
        refused++;
      else if (sortkey_order(ka, na, kb, nb) != order_sign(c))
        bad++;
      free(ka); free(kb);
      pairs++;
    }
  printf("tjsonb: %ld temporal pairs, %ld without a key, %ld ordered or "
    "equated otherwise than temporal_cmp\n", pairs, refused, bad);
  failures += refused + bad;
}

/*****************************************************************************/

/* A corpus reaching every rule: each scalar type, numbers of every sign,
 * scale and magnitude around the powers of 10000, strings sharing prefixes,
 * arrays and objects of different sizes, nested, and values equal under
 * different spellings */
static const char *CORPUS[] = {
  "null", "true", "false",
  "\"\"", "\"a\"", "\"ab\"", "\"b\"", "\"\\u00e9\"", "\"a b\"",
  "0", "-0", "1", "1.0", "1.00", "-1", "0.5", "-0.5", "0.0001", "-0.0001",
  "9999", "10000", "10001", "-9999", "-10000", "99999999", "100000000",
  "1e20", "-1e20", "1e-20", "-1e-20", "123456789012345678901234567890.5",
  "0.1", "0.10000001", "3.14159",
  "[]", "[null]", "[1]", "[1.0]", "[1, 2]", "[2]", "[[]]", "[\"a\"]",
  "[1, \"a\"]", "[1, \"b\"]", "[true, false]", "[[1], [2]]",
  "{}", "{\"a\": 1}", "{\"a\": 2}", "{\"a\": 1.0}", "{\"b\": 1}",
  "{\"aa\": 1}", "{\"a\": 1, \"b\": 2}", "{\"a\": [1, 2]}",
  "{\"a\": {\"b\": null}}", "{\"a\": \"x\", \"c\": [true]}",
};

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  stated_check();

  Jsonb *values[MAXV];
  int nvalues = 0;
  for (size_t i = 0; i < sizeof(CORPUS) / sizeof(CORPUS[0]); i++)
  {
    Jsonb *jb = jsonb_in(CORPUS[i]);
    if (jb)
      values[nvalues++] = jb;
    else
      printf("  %s is refused\n", CORPUS[i]);
    meos_errno_reset();
  }
  base_check(values, nvalues);

  /* Sets of one and of two values */
  Set *sets[MAXV];
  int nsets = 0;
  for (int i = 0; i < nvalues && nsets < MAXV - 1; i++)
  {
    const Jsonb *one[1] = {values[i]};
    sets[nsets++] = jsonbset_make(one, 1);
    const Jsonb *two[2] = {values[i], values[(i * 7 + 5) % nvalues]};
    Set *s = jsonbset_make(two, 2);
    if (s)
      sets[nsets++] = s;
    meos_errno_reset();
  }
  set_check(sets, nsets);
  for (int i = 0; i < nsets; i++)
    free(sets[i]);

  /* Temporal values of one and of two instants, step interpolation */
  Temporal *temps[MAXV];
  int ntemps = 0;
  TimestampTz t1 = timestamptz_in("2001-01-01", -1);
  TimestampTz t2 = timestamptz_in("2001-01-02", -1);
  for (int i = 0; i < nvalues && ntemps < MAXV - 1; i++)
  {
    TInstant *inst1 = tjsonbinst_make(values[i], t1);
    TInstant *inst2 = tjsonbinst_make(values[(i * 7 + 5) % nvalues], t2);
    if (! inst1 || ! inst2)
    {
      free(inst1); free(inst2);
      meos_errno_reset();
      continue;
    }
    temps[ntemps++] = (Temporal *) inst1;
    TInstant *insts[2] = {inst1, inst2};
    TSequence *seq = tsequence_make(insts, 2, true, true, STEP, false);
    if (seq)
      temps[ntemps++] = (Temporal *) seq;
    meos_errno_reset();
    free(inst2);
  }
  temporal_check(temps, ntemps);
  for (int i = 0; i < ntemps; i++)
    free(temps[i]);

  for (int i = 0; i < nvalues; i++)
    free(values[i]);
  meos_finalize();
  printf("%ld failure(s)\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
