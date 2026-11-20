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
 * @brief A program that tests that a temporal rigid geometry is compared,
 * equated and ordered by its value, its reference geometry and its poses, and
 * that its sort key orders it as its comparison does
 * @details A temporal rigid geometry is a reference geometry moved by a
 * temporal pose, so two values are equal exactly when their reference
 * geometries are equal by value (#geo_eq) and their poses are equal, and the
 * comparison reads the reference geometry (#geo_cmp) right after the
 * bounding box, before the instants. The program therefore checks, over the
 * values of three reference geometries moved by five spellings of poses, three
 * of them one value spelled as an instant, a sequence of one instant and a
 * sequence set of one sequence:
 * - over pairs stated beside each rule, that the equality and the comparison
 *   answer as the rule states;
 * - over every pair, that the comparison answers 0 exactly when the equality
 *   holds and that swapping the operands swaps the sign, and over every
 *   triple, that the comparison is transitive;
 * - over every pair, that the byte order of the keys is the order of
 *   #temporal_cmp.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o trgeo_sortkey_test trgeo_sortkey_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_rgeo.h>

#define MAXV 64

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

static const char *GEOMS[] = {
  "Polygon((1 1,2 2,3 1,1 1))",
  "Polygon((0 0,5 0,5 5,0 0))",
  "Polygon((0 0,1 0,1 1,0 0))",
};

/* The first, the fourth and the fifth spell one value */
static const char *POSES[] = {
  "Pose(Point(1 2),0.5)@2001-01-01",
  "[Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02]",
  "Pose(Point(1 2),0.25)@2001-01-01",
  "[Pose(Point(1 2),0.5)@2001-01-01]",
  "{[Pose(Point(1 2),0.5)@2001-01-01]}",
};

static Temporal *
rigid(int g, int p)
{
  char str[512];
  snprintf(str, sizeof(str), "%s;%s", GEOMS[g], POSES[p]);
  return trgeometry_in(str);
}

/* Pairs whose equality a rule states: the same poses on two reference
 * geometries are two values, and one value spelled twice is one */
static void
stated_check(void)
{
  const struct { int g1, p1, g2, p2; bool equal; } STATED[] = {
    {0, 0, 1, 0, false},
    {0, 1, 1, 1, false},
    {0, 0, 2, 0, false},
    {0, 0, 0, 3, true},
    {0, 0, 0, 4, true},
    {1, 3, 1, 4, true},
    {0, 0, 0, 2, false},
  };
  long pairs = 0, bad = 0;
  for (size_t i = 0; i < sizeof(STATED) / sizeof(STATED[0]); i++)
  {
    Temporal *a = rigid(STATED[i].g1, STATED[i].p1);
    Temporal *b = rigid(STATED[i].g2, STATED[i].p2);
    bool eq = a && b && temporal_eq(a, b);
    int c = (a && b) ? order_sign(temporal_cmp(a, b)) : 9;
    if (eq != STATED[i].equal || (c == 0) != STATED[i].equal)
    {
      printf("  %s;%s against %s;%s: equality %d, comparison %d\n",
        GEOMS[STATED[i].g1], POSES[STATED[i].p1], GEOMS[STATED[i].g2],
        POSES[STATED[i].p2], eq, c);
      bad++;
    }
    meos_errno_reset();
    free(a); free(b);
    pairs++;
  }
  printf("trgeometry: %ld stated pairs, %ld answered otherwise than stated\n",
    pairs, bad);
  failures += bad;
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  stated_check();

  Temporal *v[MAXV];
  int n = 0;
  int ngeoms = (int) (sizeof(GEOMS) / sizeof(GEOMS[0]));
  int nposes = (int) (sizeof(POSES) / sizeof(POSES[0]));
  for (int g = 0; g < ngeoms; g++)
    for (int p = 0; p < nposes; p++)
    {
      Temporal *t = rigid(g, p);
      if (t)
        v[n++] = t;
      else
        printf("  %s;%s is refused\n", GEOMS[g], POSES[p]);
      meos_errno_reset();
    }

  long pairs = 0, inconsistent = 0, asymmetric = 0, refused = 0, unordered = 0,
    triples = 0, intransitive = 0;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      int c = order_sign(temporal_cmp(v[i], v[j]));
      if ((c == 0) != temporal_eq(v[i], v[j]))
        inconsistent++;
      if (c != -order_sign(temporal_cmp(v[j], v[i])))
        asymmetric++;
      size_t na, nb;
      meos_errno_reset();
      uint8_t *ka = temporal_sortkey(v[i], &na);
      uint8_t *kb = temporal_sortkey(v[j], &nb);
      meos_errno_reset();
      if (! ka || ! kb)
        refused++;
      else if (sortkey_order(ka, na, kb, nb) != c)
        unordered++;
      free(ka); free(kb);
      pairs++;
    }
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      int cij = order_sign(temporal_cmp(v[i], v[j]));
      if (cij > 0)
        continue;
      for (int k = 0; k < n; k++)
      {
        int cjk = order_sign(temporal_cmp(v[j], v[k]));
        if (cjk > 0)
          continue;
        /* i <= j and j <= k, so i <= k, and i == k only when both are equal */
        int cik = order_sign(temporal_cmp(v[i], v[k]));
        if (cik > 0 || (cik == 0 && (cij != 0 || cjk != 0)))
          intransitive++;
        triples++;
      }
    }
  printf("trgeometry: %ld pairs, %ld whose comparison answers 0 otherwise than "
    "their equality, %ld whose swapped comparison is not the opposite, %ld "
    "without a key, %ld ordered or equated otherwise than temporal_cmp by "
    "their keys; %ld ordered triples, %ld not transitive\n", pairs,
    inconsistent, asymmetric, refused, unordered, triples, intransitive);
  failures += inconsistent + asymmetric + refused + unordered + intransitive;

  for (int i = 0; i < n; i++)
    free(v[i]);
  meos_finalize();
  printf("%ld failure(s)\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
