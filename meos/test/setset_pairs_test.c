/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief A program that tests the ever/always set-set relationships over
 * arrays of temporal geos, i.e., the `*_tgeoarr_tgeoarr` functions, against
 * the scalar relationship applied to every pair.
 *
 * The oracle is the scalar relationship of the same name, so the array
 * function is checked against the per-pair answer it is meant to reproduce,
 * including the cases its bounding box prefilter decides without an exact
 * test. A pair with no common time is excluded, matching the drivers.
 *
 * The array sizes straddle the number of pairs above which the drivers index
 * both sides and join the trees rather than comparing every pair, so both
 * paths are exercised and asserted to give the same answer.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o setset_pairs_test setset_pairs_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

/* Sizes below the number of pairs at which the drivers switch to the index */
#define SMALL1 120
#define SMALL2 120
/* Sizes above it */
#define LARGE1 500
#define LARGE2 400
/* Distance used by the within-distance relationships */
#define DIST 3.0

static int failures = 0;

static void
check(const char *name, bool ok)
{
  printf("  %-58s %s\n", name, ok ? "OK" : "FAIL");
  if (! ok)
    failures++;
}

/* Return a pseudo-random double in [min, max] */
static double
random_double(double min, double max)
{
  return min + (max - min) * ((double) rand() / (double) RAND_MAX);
}

/* Return a temporal geo moving between two random points of a grid over a
 * random hour of a random day, so that the two coordinates and the period
 * select different pairs */
static Temporal *
random_tgeo(void)
{
  double x = random_double(0, 40), y = random_double(0, 40);
  double dx = random_double(-4, 4), dy = random_double(-4, 4);
  int day = 1 + rand() % 5;
  int h = rand() % 20;
  char buf[512];
  snprintf(buf, sizeof(buf),
    "[POINT(%.4f %.4f)@2000-01-%02d %02d:00:00+00, "
    "POINT(%.4f %.4f)@2000-01-%02d %02d:00:00+00]",
    x, y, day, h, x + dx, y + dy, day, h + 3);
  return (Temporal *) tgeompoint_in(buf);
}

/* Return a temporal geometry holding a unit square of a grid over a random
 * hour of a random day. Squares of a grid share edges and corners, so the
 * touches relationships hold for real pairs, which they never do for points:
 * touching needs boundaries to meet and a point has none. */
static Temporal *
random_tgeometry(void)
{
  int gx = rand() % 12, gy = rand() % 12;
  int day = 1 + rand() % 5;
  int h = rand() % 20;
  char buf[768];
  snprintf(buf, sizeof(buf),
    "Interp=Step;[Polygon((%d %d,%d %d,%d %d,%d %d,%d %d))"
    "@2000-01-%02d %02d:00:00+00, "
    "Polygon((%d %d,%d %d,%d %d,%d %d,%d %d))@2000-01-%02d %02d:00:00+00]",
    gx, gy, gx + 1, gy, gx + 1, gy + 1, gx, gy + 1, gx, gy, day, h,
    gx, gy, gx + 1, gy, gx + 1, gy + 1, gx, gy + 1, gx, gy, day, h + 3);
  return (Temporal *) tgeometry_in(buf);
}

/* The relationships under test, each with its array function and the scalar
 * the oracle applies to every pair */
typedef enum
{
  P_EDWITHIN, P_ADWITHIN, P_EINTERSECTS, P_AINTERSECTS,
  P_ETOUCHES, P_ATOUCHES, P_EDISJOINT, P_ADISJOINT,
} Pred;

static const char *PRED_NAME[] = {
  "eDwithin", "aDwithin", "eIntersects", "aIntersects",
  "eTouches", "aTouches", "eDisjoint", "aDisjoint",
};

static int *
pred_array(Pred p, const Temporal **a1, int c1, const Temporal **a2, int c2,
  int *count)
{
  switch (p)
  {
    case P_EDWITHIN:
      return edwithin_tgeoarr_tgeoarr(a1, c1, a2, c2, DIST, count);
    case P_ADWITHIN:
      return adwithin_tgeoarr_tgeoarr(a1, c1, a2, c2, DIST, count);
    case P_EINTERSECTS:
      return eintersects_tgeoarr_tgeoarr(a1, c1, a2, c2, count);
    case P_AINTERSECTS:
      return aintersects_tgeoarr_tgeoarr(a1, c1, a2, c2, count);
    case P_ETOUCHES:
      return etouches_tgeoarr_tgeoarr(a1, c1, a2, c2, count);
    case P_ATOUCHES:
      return atouches_tgeoarr_tgeoarr(a1, c1, a2, c2, count);
    case P_EDISJOINT:
      return edisjoint_tgeoarr_tgeoarr(a1, c1, a2, c2, count);
    default:
      return adisjoint_tgeoarr_tgeoarr(a1, c1, a2, c2, count);
  }
}

static int
pred_scalar(Pred p, const Temporal *t1, const Temporal *t2)
{
  switch (p)
  {
    case P_EDWITHIN:    return edwithin_tgeo_tgeo(t1, t2, DIST);
    case P_ADWITHIN:    return adwithin_tgeo_tgeo(t1, t2, DIST);
    case P_EINTERSECTS: return eintersects_tgeo_tgeo(t1, t2);
    case P_AINTERSECTS: return aintersects_tgeo_tgeo(t1, t2);
    case P_ETOUCHES:    return etouches_tgeo_tgeo(t1, t2);
    case P_ATOUCHES:    return atouches_tgeo_tgeo(t1, t2);
    case P_EDISJOINT:   return edisjoint_tgeo_tgeo(t1, t2);
    default:            return adisjoint_tgeo_tgeo(t1, t2);
  }
}

/* Run one relationship over one pair of arrays and compare with the oracle */
static void
test_pred(Pred p, const Temporal **a1, int c1, const Temporal **a2, int c2,
  const char *sizelabel)
{
  int nres = 0;
  int *res = pred_array(p, a1, c1, a2, c2, &nres);

  /* Oracle: the scalar relationship on every pair sharing time */
  int *expected = malloc((size_t) c1 * c2 * 2 * sizeof(int));
  int nexp = 0;
  Span **periods1 = malloc((size_t) c1 * sizeof(Span *));
  Span **periods2 = malloc((size_t) c2 * sizeof(Span *));
  for (int i = 0; i < c1; i++)
    periods1[i] = temporal_to_tstzspan(a1[i]);
  for (int j = 0; j < c2; j++)
    periods2[j] = temporal_to_tstzspan(a2[j]);
  for (int i = 0; i < c1; i++)
  {
    for (int j = 0; j < c2; j++)
    {
      if (! overlaps_span_span(periods1[i], periods2[j]))
        continue;
      if (pred_scalar(p, a1[i], a2[j]) == 1)
      {
        expected[2 * nexp] = i; expected[2 * nexp + 1] = j; nexp++;
      }
    }
  }

  char name[160];
  snprintf(name, sizeof(name), "%s %s: pair count matches the scalar",
    PRED_NAME[p], sizelabel);
  check(name, nres == nexp);

  bool same = (nres == nexp);
  for (int k = 0; k < nres && same; k++)
    if (res[2 * k] != expected[2 * k] || res[2 * k + 1] != expected[2 * k + 1])
      same = false;
  snprintf(name, sizeof(name), "%s %s: same pairs in the same order",
    PRED_NAME[p], sizelabel);
  check(name, same);

  printf("    (%d pairs over %d x %d)\n", nres, c1, c2);

  if (res)
    free(res);
  for (int i = 0; i < c1; i++) free(periods1[i]);
  for (int j = 0; j < c2; j++) free(periods2[j]);
  free(expected); free(periods1); free(periods2);
  return;
}

int
main(void)
{
  meos_initialize();
  srand(1);

  const Temporal **small1 = malloc(SMALL1 * sizeof(Temporal *));
  const Temporal **small2 = malloc(SMALL2 * sizeof(Temporal *));
  const Temporal **large1 = malloc(LARGE1 * sizeof(Temporal *));
  const Temporal **large2 = malloc(LARGE2 * sizeof(Temporal *));
  for (int i = 0; i < SMALL1; i++) small1[i] = random_tgeo();
  for (int j = 0; j < SMALL2; j++) small2[j] = random_tgeo();
  for (int i = 0; i < LARGE1; i++) large1[i] = random_tgeo();
  for (int j = 0; j < LARGE2; j++) large2[j] = random_tgeo();

  /* Two points drawn at random are never at one place at one instant, so the
   * intersects and touches relationships would hold for no pair at all and
   * their comparison against the scalar would compare two empty answers. Every
   * third element of the second side repeats one of the first, which the
   * relationships do hold for. */
  for (int j = 0; j < SMALL2; j += 3)
  {
    free((void *) small2[j]);
    small2[j] = temporal_copy(small1[j % SMALL1]);
  }
  for (int j = 0; j < LARGE2; j += 3)
  {
    free((void *) large2[j]);
    large2[j] = temporal_copy(large1[j % LARGE1]);
  }

  /* Squares of a grid, for the relationships that need boundaries to meet */
  const Temporal **poly1 = malloc(LARGE1 * sizeof(Temporal *));
  const Temporal **poly2 = malloc(LARGE2 * sizeof(Temporal *));
  for (int i = 0; i < LARGE1; i++) poly1[i] = random_tgeometry();
  for (int j = 0; j < LARGE2; j++) poly2[j] = random_tgeometry();

  printf("Testing the set-set relationships against the scalar relationship\n");
  for (int p = 0; p < 8; p++)
  {
    test_pred((Pred) p, small1, SMALL1, small2, SMALL2, "points pairwise");
    test_pred((Pred) p, large1, LARGE1, large2, LARGE2, "points indexed");
    test_pred((Pred) p, poly1, LARGE1, poly2, LARGE2, "squares indexed");
  }

  for (int i = 0; i < LARGE1; i++) free((void *) poly1[i]);
  for (int j = 0; j < LARGE2; j++) free((void *) poly2[j]);
  free(poly1); free(poly2);

  for (int i = 0; i < SMALL1; i++) free((void *) small1[i]);
  for (int j = 0; j < SMALL2; j++) free((void *) small2[j]);
  for (int i = 0; i < LARGE1; i++) free((void *) large1[i]);
  for (int j = 0; j < LARGE2; j++) free((void *) large2[j]);
  free(small1); free(small2); free(large1); free(large2);

  meos_finalize();
  if (failures > 0)
  {
    printf("\n%d test(s) FAILED\n", failures);
    return 1;
  }
  printf("\nAll tests passed\n");
  return 0;
}
