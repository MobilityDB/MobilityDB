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
 * @brief A program that tests that the sort key of a temporal value orders
 * temporal values as #temporal_cmp does
 * @details Over every pair of a corpus of temporal values of each temporal
 * type with a sort key, the sign of a byte comparison of the two keys must be
 * the sign of #temporal_cmp, and two values #temporal_cmp holds equal must
 * have identical keys and be equal by #temporal_eq. The corpus holds, over
 * three values at three times, instants, discrete sequences, step and linear
 * sequences with every bound flag and sequence sets, among them the
 * one-element wrappers of an instant and of a sequence and a discrete
 * sequence beside the set of its one-instant sequences, which #temporal_eq
 * holds equal. The program also verifies that #temporal_sortkey reports a
 * null value, a null size and a temporal type without a sort key.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o temporal_sortkey_test temporal_sortkey_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <assert.h>
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

#define MAXT 4000

typedef Temporal *(*tempcorpus_in)(const char *);

static Temporal *values[MAXT];
static int nvalues;
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

/* Add the value a string spells, skipping a string the parser refuses */
static void
tempcorpus_add(tempcorpus_in in, const char *str)
{
  Temporal *temp = in(str);
  if (temp && nvalues < MAXT)
    values[nvalues++] = temp;
  meos_errno_reset();
}

/* Compare every pair of the corpus by key, by value and by equality, then
 * free it */
static void
tempcorpus_check(const char *name)
{
  long pairs = 0, bad = 0;
  for (int i = 0; i < nvalues; i++)
    for (int j = 0; j < nvalues; j++)
    {
      size_t na, nb;
      uint8_t *ka = temporal_sortkey(values[i], &na);
      uint8_t *kb = temporal_sortkey(values[j], &nb);
      int c = order_sign(temporal_cmp(values[i], values[j]));
      if (sortkey_order(ka, na, kb, nb) != c ||
          (c == 0) != temporal_eq(values[i], values[j]))
        bad++;
      free(ka); free(kb);
      pairs++;
    }
  printf("%s: %d values, %ld pairs, %ld ordered or equated otherwise than "
    "temporal_cmp\n", name, nvalues, pairs, bad);
  failures += bad;
  for (int i = 0; i < nvalues; i++)
    free(values[i]);
  nvalues = 0;
}

/* Add every shape over the values a, b and c at three times */
static void
tempcorpus_shapes(tempcorpus_in in, const char *prefix, const char *a,
  const char *b, const char *c, bool linear, bool step)
{
  const char *t1 = "2001-01-01", *t2 = "2001-01-02", *t3 = "2001-01-03";
  char s[1024];
#define TEMPCORPUS_ADD(...) \
  do { snprintf(s, sizeof(s), __VA_ARGS__); tempcorpus_add(in, s); } while (0)
  TEMPCORPUS_ADD("%s%s@%s", prefix, a, t1);
  TEMPCORPUS_ADD("%s%s@%s", prefix, b, t2);
  TEMPCORPUS_ADD("%s{%s@%s}", prefix, a, t1);
  TEMPCORPUS_ADD("%s{%s@%s, %s@%s}", prefix, a, t1, b, t2);
  TEMPCORPUS_ADD("%s{%s@%s, %s@%s, %s@%s}", prefix, a, t1, b, t2, c, t3);
  TEMPCORPUS_ADD("%s{[%s@%s], [%s@%s]}", prefix, a, t1, b, t2);
  TEMPCORPUS_ADD("%s{[%s@%s]}", prefix, a, t1);
  TEMPCORPUS_ADD("%s[%s@%s]", prefix, a, t1);
  /* The continuous shapes, under the default interpolation of the type and,
   * for a type whose default is linear, under step interpolation */
  const char *interps[2] = {"", "Interp=Step;"};
  for (int k = 0; k < 2; k++)
  {
    if ((k == 0 && ! (linear || step)) || (k == 1 && ! (linear && step)))
      continue;
    const char *ip = interps[k];
    TEMPCORPUS_ADD("%s%s[%s@%s, %s@%s]", prefix, ip, a, t1, b, t2);
    TEMPCORPUS_ADD("%s%s(%s@%s, %s@%s]", prefix, ip, a, t1, b, t2);
    TEMPCORPUS_ADD("%s%s[%s@%s, %s@%s)", prefix, ip, a, t1, b, t2);
    TEMPCORPUS_ADD("%s%s(%s@%s, %s@%s)", prefix, ip, a, t1, b, t2);
    TEMPCORPUS_ADD("%s%s[%s@%s, %s@%s, %s@%s]", prefix, ip, a, t1, b, t2, c,
      t3);
    TEMPCORPUS_ADD("%s%s{[%s@%s, %s@%s], [%s@%s]}", prefix, ip, a, t1, b, t2,
      c, t3);
    /* Bounds inside a sequence set, which the period of the bounding box
     * does not hold */
    TEMPCORPUS_ADD("%s%s{[%s@%s, %s@%s), [%s@%s]}", prefix, ip, a, t1, b, t2,
      c, t3);
    TEMPCORPUS_ADD("%s%s{[%s@%s], (%s@%s, %s@%s]}", prefix, ip, a, t1, b, t2,
      c, t3);
    TEMPCORPUS_ADD("%s%s{[%s@%s, %s@%s)}", prefix, ip, a, t1, b, t2);
  }
#undef TEMPCORPUS_ADD
}

/* Build the corpus of a temporal type over every triple of its values and
 * check it */
static void
tempcorpus_family(const char *name, tempcorpus_in in, const char *prefix,
  const char **vals, int nv, bool linear, bool step)
{
  for (int a = 0; a < nv; a++)
    for (int b = 0; b < nv; b++)
      for (int c = 0; c < nv; c++)
        tempcorpus_shapes(in, prefix, vals[a], vals[b], vals[c], linear,
          step);
  tempcorpus_check(name);
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting, so that a string the parser refuses (a
   * step sequence ending on a value it does not reach) is skipped */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  const char *bools[] = {"f", "t"};
  tempcorpus_family("tbool", tbool_in, "", bools, 2, false, true);
  const char *ints[] = {"-2147483648", "-1", "0", "256"};
  tempcorpus_family("tint", tint_in, "", ints, 4, false, true);
  /* The largest big integer has no next value, so a box holding it is
   * refused; the values stop one below it */
  const char *bigints[] = {"-9223372036854775808", "-1", "4294967296",
    "9223372036854775806"};
  tempcorpus_family("tbigint", tbigint_in, "", bigints, 4, false, true);
  const char *floats[] = {"-1e300", "-0", "0", "2.5"};
  tempcorpus_family("tfloat", tfloat_in, "", floats, 4, true, true);
  const char *texts[] = {"\"\"", "\"A\"", "\"AB\"", "\"B\""};
  tempcorpus_family("ttext", ttext_in, "", texts, 4, false, true);
  const char *points[] = {"Point(-1 -1)", "Point(-0 1)", "Point(0 1)",
    "Point(1e300 0)"};
  tempcorpus_family("tgeompoint", tgeompoint_in, "", points, 4, true, true);
  const char *points3d[] = {"Point Z(0 0 -1)", "Point Z(-0 0 0)",
    "Point Z(0 0 0)", "Point Z(1 -1 0)"};
  tempcorpus_family("tgeompoint 3D", tgeompoint_in, "SRID=3857;", points3d,
    4, true, true);
  const char *geogs[] = {"Point(-10 -45)", "Point(-0 45)", "Point(0 45)",
    "Point(10 0)"};
  tempcorpus_family("tgeogpoint", tgeogpoint_in, "", geogs, 4, true, true);
  const char *h3cells[] = {"8001fffffffffff", "807ffffffffffff",
    "831c00fffffffff", "831c02fffffffff"};
  tempcorpus_family("th3index", th3index_in, "", h3cells, 4, false, true);
  const char *quadbins[] = {"480fffffffffffff", "48427fffffffffff",
    "48a6227affffffff", "48a6227bffffffff"};
  tempcorpus_family("tquadbin", tquadbin_in, "", quadbins, 4, false, true);
  const char *s2cells[] = {"47c3c3", "47c3c4", "47c3c", "54b5c9"};
  tempcorpus_family("ts2cell", ts2cell_in, "", s2cells, 4, false, true);

  /* A null value or a null size is reported rather than dereferenced, and a
   * temporal type without a sort key is reported */
  size_t size;
  Temporal *temp = tint_in("[1@2001-01-01, 2@2001-01-02]");
  Temporal *geom = tgeometry_in("Polygon((0 0,1 0,1 1,0 0))@2001-01-01");
  meos_errno_reset();
  uint8_t *key = temporal_sortkey(NULL, &size);
  printf("temporal_sortkey(NULL, size): %s, errno %d\n", key ? "a key" : "NULL",
    meos_errno());
  assert(key == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  key = temporal_sortkey(temp, NULL);
  printf("temporal_sortkey(tint, NULL): %s, errno %d\n", key ? "a key" : "NULL",
    meos_errno());
  assert(key == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  key = temporal_sortkey(geom, &size);
  printf("temporal_sortkey(tgeometry, size): %s, errno %d\n",
    key ? "a key" : "NULL", meos_errno());
  assert(key == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  meos_errno_reset();
  key = temporal_sortkey(temp, &size);
  printf("temporal_sortkey(tint, size): %s of %zu bytes, errno %d\n",
    key ? "a key" : "NULL", size, meos_errno());
  assert(key != NULL && meos_errno() == 0);
  free(key); free(geom); free(temp);

  printf("%ld pair(s) ordered or equated otherwise than their comparison\n",
    failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
