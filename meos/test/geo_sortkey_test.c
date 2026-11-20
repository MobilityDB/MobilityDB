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
 * @brief A program that tests that geometries and geographies are ordered and
 * equated by value, and that the sort keys of their sets and temporal values
 * order them as their comparison does
 * @details The comparison of two geometries reads their fields in order: the
 * SRID, the Z, M and geodetic flags, the geometry type, so that a point
 * precedes any other geometry, the counts of the parts, rings and vertices
 * before what they govern, then the coordinates as numbers, a zero of either
 * sign being 0 and a geography read on the sphere, its longitude -180 being
 * 180 and any longitude at a pole one point. Their equality holds every field
 * equal. A set and a temporal value hold geometries of one SRID and never an
 * empty one, so the program reads every geometry in the SRID 0 and checks:
 * - over pairs stated beside each rule, for geometries and for geographies,
 *   that the comparison answers as the rule states;
 * - over every pair of a corpus of geometries, each read as the one value of
 *   a set, that the comparison answers 0 exactly when the equality holds and
 *   that swapping the operands swaps the sign, and over every triple, that it
 *   is transitive; the pairs whose comparison answers otherwise than their
 *   equality are printed;
 * - over every pair of sets of one and of two geometries, and of temporal
 *   geometries of one and of two instants, that the byte order of the keys is
 *   the order of #set_cmp and #temporal_cmp.
 *
 * The corpus holds every geometry type, geometries that share a bounding box,
 * and geometries spelled with a coordinate -0, which is the number 0.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o geo_sortkey_test geo_sortkey_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

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

/* The one-value set of a geometry or of a geography, through which the
 * comparison and the equality of two values are read */
static Set *
one_of(const char *wkt, bool geodetic)
{
  GSERIALIZED *gs = geodetic ? geog_in(wkt, -1) : geom_in(wkt, -1);
  if (! gs)
    return NULL;
  GSERIALIZED *v[1] = {gs};
  Set *result = geoset_make(v, 1);
  free(gs);
  return result;
}

static Set *
one(const char *wkt)
{
  return one_of(wkt, false);
}

/*****************************************************************************
 * The comparison of the geometries
 *****************************************************************************/

/* Pairs whose order a rule of the comparison states: the first precedes the
 * second, or equals it when the order is 0; geodetic pairs are geographies */
static const struct
{
  const char *a;
  const char *b;
  int order;
  bool geodetic;
} STATED[] = {
  /* The type: a point precedes a line, a line a polygon */
  {"POINT(9 9)", "LINESTRING(0 0,1 1)", -1, false},
  {"LINESTRING(9 9,8 8)", "POLYGON((0 0,1 0,1 1,0 0))", -1, false},
  /* The counts before the coordinates */
  {"LINESTRING(5 5,6 6)", "LINESTRING(0 0,0 1,0 2)", -1, false},
  {"MULTIPOINT((9 9))", "MULTIPOINT((0 0),(1 1))", -1, false},
  /* The coordinates as numbers, x first */
  {"POINT(1 2)", "POINT(1 3)", -1, false},
  {"POINT(1 2)", "POINT(2 0)", -1, false},
  {"LINESTRING(0 0,1 1)", "LINESTRING(0 1,1 0)", -1, false},
  /* A zero of either sign is 0 */
  {"POINT(0 0)", "POINT(-0 0)", 0, false},
  {"LINESTRING(0 0,1 1)", "LINESTRING(-0 0,1 1)", 0, false},
  {"POLYGON((0 0,1 0,1 1,0 0))", "POLYGON((-0 0,1 0,1 1,-0 0))", 0, false},
  /* A geography is read on the sphere */
  {"POINT(180 10)", "POINT(-180 10)", 0, true},
  {"POINT(10 90)", "POINT(20 90)", 0, true},
  {"LINESTRING(180 10,0 0)", "LINESTRING(-180 10,0 0)", 0, true},
  {"POINT(10 10)", "POINT(-180 10)", -1, true},
};

static void
stated_check(void)
{
  long pairs = 0, bad = 0;
  for (size_t i = 0; i < sizeof(STATED) / sizeof(STATED[0]); i++)
  {
    Set *a = one_of(STATED[i].a, STATED[i].geodetic);
    Set *b = one_of(STATED[i].b, STATED[i].geodetic);
    meos_errno_reset();
    int c = (a && b) ? order_sign(set_cmp(a, b)) : 9;
    bool eq = a && b && set_eq(a, b);
    if (c != STATED[i].order || eq != (STATED[i].order == 0))
    {
      printf("  %s against %s: %d, stated %d\n", STATED[i].a, STATED[i].b,
        c, STATED[i].order);
      bad++;
    }
    free(a); free(b);
    pairs++;
  }
  printf("geometry: %ld stated pairs, %ld answered otherwise than stated\n",
    pairs, bad);
  failures += bad;
}

/* Check that the comparison is antisymmetric, transitive and consistent with
 * the equality, over every pair and every triple of geometries */
static void
base_check(Set **v, const char **wkt, int n)
{
  long pairs = 0, inconsistent = 0, asymmetric = 0, triples = 0,
    intransitive = 0;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      int c = order_sign(set_cmp(v[i], v[j]));
      bool eq = set_eq(v[i], v[j]);
      if ((c == 0) != eq)
      {
        if (i < j)
          printf("  %s against %s: comparison %d, equality %s\n", wkt[i],
            wkt[j], c, eq ? "true" : "false");
        inconsistent++;
      }
      if (c != -order_sign(set_cmp(v[j], v[i])))
        asymmetric++;
      pairs++;
    }
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      int cij = order_sign(set_cmp(v[i], v[j]));
      if (cij > 0)
        continue;
      for (int k = 0; k < n; k++)
      {
        int cjk = order_sign(set_cmp(v[j], v[k]));
        if (cjk > 0)
          continue;
        /* i <= j and j <= k, so i <= k, and i == k only when both are equal */
        int cik = order_sign(set_cmp(v[i], v[k]));
        if (cik > 0 || (cik == 0 && (cij != 0 || cjk != 0)))
          intransitive++;
        triples++;
      }
    }
  printf("geometry: %ld pairs, %ld whose comparison answers 0 otherwise than "
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
  printf("geomset: %ld set pairs, %ld without a key, %ld ordered or equated "
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
  printf("tgeometry: %ld temporal pairs, %ld without a key, %ld ordered or "
    "equated otherwise than temporal_cmp\n", pairs, refused, bad);
  failures += refused + bad;
}

/*****************************************************************************/

/* Every geometry type, geometries sharing a bounding box, and geometries
 * spelled with a coordinate -0 */
static const char *CORPUS[] = {
  "POINT(0 0)", "POINT(-0 0)", "POINT(0 -0)", "POINT(1 2)", "POINT(2 1)",
  "LINESTRING(0 0,1 1)", "LINESTRING(-0 0,1 1)",
  "LINESTRING(1 1,0 0)", "LINESTRING(0 1,1 0)", "LINESTRING(0 0,1 1,2 2)",
  "LINESTRING(0 0,0.5 0.5,1 1)",
  "POLYGON((0 0,1 0,1 1,0 0))", "POLYGON((-0 0,1 0,1 1,-0 0))",
  "POLYGON((0 0,1 1,1 0,0 0))", "POLYGON((0 0,2 0,2 2,0 2,0 0))",
  "POLYGON((0 0,2 0,2 2,0 2,0 0),(0.5 0.5,1 0.5,1 1,0.5 0.5))",
  "MULTIPOINT((0 0),(1 1))", "MULTIPOINT((1 1),(0 0))",
  "MULTIPOINT((-0 0),(1 1))",
  "MULTILINESTRING((0 0,1 1),(1 0,0 1))", "MULTILINESTRING((0 0,1 1))",
  "MULTIPOLYGON(((0 0,1 0,1 1,0 0)),((2 2,3 2,3 3,2 2)))",
  "GEOMETRYCOLLECTION(POINT(0 0),LINESTRING(0 0,1 1))",
  "GEOMETRYCOLLECTION(POINT(-0 0),LINESTRING(0 0,1 1))",
  "CIRCULARSTRING(0 0,1 1,2 0)", "CIRCULARSTRING(-0 0,1 1,2 0)",
  "COMPOUNDCURVE(CIRCULARSTRING(0 0,1 1,2 0),(2 0,3 0))",
  "CURVEPOLYGON(CIRCULARSTRING(0 0,2 0,0 0))",
  "TRIANGLE((0 0,1 0,1 1,0 0))", "TIN(((0 0,1 0,1 1,0 0)))",
  "POLYHEDRALSURFACE(((0 0,1 0,1 1,0 0)))",
};

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  stated_check();

  Set *values[MAXV];
  const char *wkt[MAXV];
  int nvalues = 0;
  for (size_t i = 0; i < sizeof(CORPUS) / sizeof(CORPUS[0]); i++)
  {
    Set *s = one(CORPUS[i]);
    if (s)
    {
      wkt[nvalues] = CORPUS[i];
      values[nvalues++] = s;
    }
    else
      printf("  %s is refused\n", CORPUS[i]);
    meos_errno_reset();
  }
  base_check(values, wkt, nvalues);

  /* Sets of one and of two geometries */
  Set *sets[MAXV];
  int nsets = 0;
  for (int i = 0; i < nvalues && nsets < MAXV - 1; i++)
  {
    sets[nsets++] = values[i];
    GSERIALIZED *a = geom_in(wkt[i], -1);
    GSERIALIZED *b = geom_in(wkt[(i * 7 + 5) % nvalues], -1);
    if (a && b)
    {
      GSERIALIZED *two[2] = {a, b};
      Set *s = geoset_make(two, 2);
      if (s)
        sets[nsets++] = s;
    }
    free(a); free(b);
    meos_errno_reset();
  }
  set_check(sets, nsets);

  /* Temporal geometries of one and of two instants, step interpolation */
  Temporal *temps[MAXV];
  int ntemps = 0;
  TimestampTz t1 = timestamptz_in("2001-01-01", -1);
  TimestampTz t2 = timestamptz_in("2001-01-02", -1);
  for (int i = 0; i < nvalues && ntemps < MAXV - 1; i++)
  {
    GSERIALIZED *a = geom_in(wkt[i], -1);
    GSERIALIZED *b = geom_in(wkt[(i * 7 + 5) % nvalues], -1);
    TInstant *inst1 = a ? tgeoinst_make(a, t1) : NULL;
    TInstant *inst2 = b ? tgeoinst_make(b, t2) : NULL;
    free(a); free(b);
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

  for (int i = 0; i < nsets; i++)
    free(sets[i]);
  meos_finalize();
  printf("%ld failure(s)\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
