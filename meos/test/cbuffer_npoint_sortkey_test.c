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
 * @brief A program that tests that the comparison of circular buffers and of
 * network points is a total order consistent with their equality, and that
 * their sort keys order the sets and the temporal values of these types as
 * their comparison does
 * @details A sort key is the string of bytes whose unsigned byte order is the
 * order of a comparison and whose byte equality is the equality of the type.
 * An engine that sorts, groups or joins values by their bytes answers as
 * MobilityDB does only when such a key exists, and a key exists only when the
 * comparison is a total order that holds two values equal exactly when the
 * equality of the type does. The program therefore checks, over a corpus of
 * circular buffers (three SRIDs, coordinates that are negative, zero of both
 * signs, large, infinite or NaN, radii that are zero of both signs, close
 * neighbours, infinite or NaN) and of network points (several routes,
 * positions zero of both signs, interior and one, and NaN):
 * - which of the special inputs the input functions accept;
 * - over every pair, that the comparison answers 0 exactly when the equality
 *   holds, and that swapping the operands swaps the sign;
 * - over every triple, that the comparison is transitive;
 * - over every pair of sets of one and of two values, and of temporal values
 *   of one and of two instants, that the byte order of the keys is the order
 *   of #set_cmp and #temporal_cmp, over the pairs these comparisons accept,
 *   since they refuse two values of different SRIDs.
 *
 * The routes of the network points come from the ways file the environment
 * variable MEOS_WAYS_CSV names, by default `meos/examples/data/ways1000.csv`
 * of the source tree, read from the directory the program runs in.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o cbuffer_npoint_sortkey_test cbuffer_npoint_sortkey_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_cbuffer.h>
#include <meos_npoint.h>

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

typedef int (*cmpfn)(const void *, const void *);
typedef bool (*eqfn)(const void *, const void *);

/* Check that a comparison is antisymmetric, transitive and consistent with
 * the equality of the type over every pair and every triple of values */
static void
base_check(const char *name, void **v, int n, cmpfn cmp, eqfn eq)
{
  long pairs = 0, inconsistent = 0, asymmetric = 0, triples = 0,
    intransitive = 0;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      int c = order_sign(cmp(v[i], v[j]));
      if ((c == 0) != eq(v[i], v[j]))
        inconsistent++;
      if (c != -order_sign(cmp(v[j], v[i])))
        asymmetric++;
      pairs++;
    }
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      int cij = order_sign(cmp(v[i], v[j]));
      if (cij > 0)
        continue;
      for (int k = 0; k < n; k++)
      {
        int cjk = order_sign(cmp(v[j], v[k]));
        if (cjk > 0)
          continue;
        /* i <= j and j <= k, so i <= k, and i == k only when both are equal */
        int cik = order_sign(cmp(v[i], v[k]));
        if (cik > 0 || (cik == 0 && (cij != 0 || cjk != 0)))
          intransitive++;
        triples++;
      }
    }
  printf("%s: %ld pairs, %ld whose comparison answers 0 otherwise than their "
    "equality, %ld whose swapped comparison is not the opposite; %ld ordered "
    "triples, %ld not transitive\n", name, pairs, inconsistent, asymmetric,
    triples, intransitive);
  failures += inconsistent + asymmetric + intransitive;
}

static int
cbuffer_cmp_v(const void *a, const void *b)
{
  return cbuffer_cmp((const Cbuffer *) a, (const Cbuffer *) b);
}

static bool
cbuffer_eq_v(const void *a, const void *b)
{
  return cbuffer_eq((const Cbuffer *) a, (const Cbuffer *) b);
}

static int
npoint_cmp_v(const void *a, const void *b)
{
  return npoint_cmp((const Npoint *) a, (const Npoint *) b);
}

static bool
npoint_eq_v(const void *a, const void *b)
{
  return npoint_eq((const Npoint *) a, (const Npoint *) b);
}

/* Read a value, report whether the input function accepts it, and keep it */
static void *
input_add(void **v, int *n, const char *str, void *(*in)(const char *),
  bool report)
{
  meos_errno_reset();
  void *value = in(str);
  if (report)
    printf("  %-40s %s\n", str, value ? "accepted" : "refused");
  meos_errno_reset();
  if (value && *n < MAXV)
    v[(*n)++] = value;
  return value;
}

static void *
cbuffer_in_v(const char *str)
{
  return cbuffer_in(str);
}

static void *
npoint_in_v(const char *str)
{
  return npoint_in(str);
}

/*****************************************************************************
 * The keys of the sets and of the temporal values
 *****************************************************************************/

/* Check that the keys of every pair of sets order them as #set_cmp does */
static void
set_check(const char *name, Set **sets, int n)
{
  long pairs = 0, bad = 0, refused = 0, mixed = 0;
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
      /* The comparison refuses two values of different SRIDs */
      else if (c == INT_MAX)
        mixed++;
      else if (sortkey_order(ka, na, kb, nb) != order_sign(c))
        bad++;
      free(ka); free(kb);
      pairs++;
    }
  printf("%s: %ld set pairs, %ld without a key, %ld of different SRIDs, "
    "%ld ordered or equated otherwise than set_cmp\n", name, pairs, refused,
    mixed, bad);
  failures += refused + bad;
}

/* Check that the keys of every pair of temporal values order them as
 * #temporal_cmp does */
static void
temporal_check(const char *name, Temporal **temps, int n)
{
  long pairs = 0, bad = 0, refused = 0, mixed = 0;
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
      /* The comparison refuses two values of different SRIDs */
      else if (c == INT_MAX)
        mixed++;
      else if (sortkey_order(ka, na, kb, nb) != order_sign(c))
        bad++;
      free(ka); free(kb);
      pairs++;
    }
  printf("%s: %ld temporal pairs, %ld without a key, %ld of different "
    "SRIDs, %ld ordered or equated otherwise than temporal_cmp\n", name,
    pairs, refused, mixed, bad);
  failures += refused + bad;
}

/*****************************************************************************/

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();
  const char *ways = getenv("MEOS_WAYS_CSV");
  meos_set_ways_csv(ways ? ways : "meos/examples/data/ways1000.csv");

  /* Circular buffers: the special inputs, then a grid of finite ones */
  void *cbufs[MAXV];
  int ncbufs = 0;
  printf("Circular buffer inputs:\n");
  const char *cbuf_special[] = {
    "Cbuffer(Point(-0 1), 1)", "Cbuffer(Point(0 1), -0)",
    "Cbuffer(Point(NaN 1), 1)", "Cbuffer(Point(1 NaN), 1)",
    "Cbuffer(Point(1 1), NaN)", "Cbuffer(Point(Infinity 1), 1)",
    "Cbuffer(Point(-Infinity 1), 1)", "Cbuffer(Point(1 1), Infinity)",
  };
  for (size_t i = 0; i < sizeof(cbuf_special) / sizeof(char *); i++)
    input_add(cbufs, &ncbufs, cbuf_special[i], cbuffer_in_v, true);
  const char *srids[] = {"", "SRID=3857;", "SRID=4326;"};
  const char *coords[] = {"-1e300", "-2", "-0", "0", "0.5", "256"};
  const char *radii[] = {"0", "1", "1.0000006", "1e300"};
  char str[256];
  for (int s = 0; s < 3; s++)
    for (int x = 0; x < 6; x++)
      for (int y = 0; y < 6; y += 2)
        for (int r = 0; r < 4; r++)
        {
          snprintf(str, sizeof(str), "%sCbuffer(Point(%s %s), %s)", srids[s],
            coords[x], coords[y], radii[r]);
          input_add(cbufs, &ncbufs, str, cbuffer_in_v, false);
        }
  base_check("cbuffer", cbufs, ncbufs, cbuffer_cmp_v, cbuffer_eq_v);

  /* Network points: the special inputs, then a grid */
  void *npoints[MAXV];
  int nnpoints = 0;
  printf("Network point inputs:\n");
  const char *npoint_special[] = {
    "NPoint(1, -0)", "NPoint(1, NaN)", "NPoint(1, Infinity)",
  };
  for (size_t i = 0; i < sizeof(npoint_special) / sizeof(char *); i++)
    input_add(npoints, &nnpoints, npoint_special[i], npoint_in_v, true);
  const char *rids[] = {"1", "2", "256", "999"};
  const char *poss[] = {"0", "0.25", "0.5", "0.5000000000000001", "1"};
  for (int r = 0; r < 4; r++)
    for (int p = 0; p < 5; p++)
    {
      snprintf(str, sizeof(str), "NPoint(%s, %s)", rids[r], poss[p]);
      input_add(npoints, &nnpoints, str, npoint_in_v, false);
    }
  base_check("npoint", npoints, nnpoints, npoint_cmp_v, npoint_eq_v);

  /* Sets of one and of two values */
  Set *sets[MAXV];
  int nsets = 0;
  for (int i = 0; i < ncbufs && nsets < MAXV - 1; i += 3)
  {
    Cbuffer one[1] = {*(Cbuffer *) cbufs[i]};
    sets[nsets++] = cbufferset_make(one, 1);
    Cbuffer two[2] = {*(Cbuffer *) cbufs[i],
      *(Cbuffer *) cbufs[(i * 7 + 5) % ncbufs]};
    Set *s = cbufferset_make(two, 2);
    if (s)
      sets[nsets++] = s;
    meos_errno_reset();
  }
  set_check("cbufferset", sets, nsets);
  for (int i = 0; i < nsets; i++)
    free(sets[i]);
  nsets = 0;
  for (int i = 0; i < nnpoints && nsets < MAXV - 1; i++)
  {
    Npoint one[1] = {*(Npoint *) npoints[i]};
    sets[nsets++] = npointset_make(one, 1);
    Npoint two[2] = {*(Npoint *) npoints[i],
      *(Npoint *) npoints[(i * 7 + 5) % nnpoints]};
    Set *s = npointset_make(two, 2);
    if (s)
      sets[nsets++] = s;
    meos_errno_reset();
  }
  set_check("npointset", sets, nsets);
  for (int i = 0; i < nsets; i++)
    free(sets[i]);

  /* Temporal values of one and of two instants, step interpolation */
  Temporal *temps[MAXV];
  int ntemps = 0;
  for (int i = 0; i < ncbufs && ntemps < MAXV - 1; i += 5)
  {
    char *v1 = cbuffer_out((Cbuffer *) cbufs[i], 17);
    char *v2 = cbuffer_out((Cbuffer *) cbufs[(i * 7 + 5) % ncbufs], 17);
    snprintf(str, sizeof(str), "%s@2001-01-01", v1);
    Temporal *t = tcbuffer_in(str);
    if (t)
      temps[ntemps++] = t;
    snprintf(str, sizeof(str), "Interp=Step;[%s@2001-01-01, %s@2001-01-02]",
      v1, v2);
    t = tcbuffer_in(str);
    if (t)
      temps[ntemps++] = t;
    meos_errno_reset();
    free(v1); free(v2);
  }
  temporal_check("tcbuffer", temps, ntemps);
  for (int i = 0; i < ntemps; i++)
    free(temps[i]);
  ntemps = 0;
  for (int i = 0; i < nnpoints && ntemps < MAXV - 1; i++)
  {
    char *v1 = npoint_out((Npoint *) npoints[i], 17);
    char *v2 = npoint_out((Npoint *) npoints[(i * 7 + 5) % nnpoints], 17);
    snprintf(str, sizeof(str), "%s@2001-01-01", v1);
    Temporal *t = tnpoint_in(str);
    if (t)
      temps[ntemps++] = t;
    snprintf(str, sizeof(str), "Interp=Step;[%s@2001-01-01, %s@2001-01-02]",
      v1, v2);
    t = tnpoint_in(str);
    if (t)
      temps[ntemps++] = t;
    meos_errno_reset();
    free(v1); free(v2);
  }
  temporal_check("tnpoint", temps, ntemps);
  for (int i = 0; i < ntemps; i++)
    free(temps[i]);

  for (int i = 0; i < ncbufs; i++)
    free(cbufs[i]);
  for (int i = 0; i < nnpoints; i++)
    free(npoints[i]);
  printf("%ld check(s) failed\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
