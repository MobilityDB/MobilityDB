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
 * @brief Test of the relationships of areas whose rings repeat a vertex,
 * #geom_relate, #geom_contains and #geom_covers
 * @details A ring that repeats a vertex holds the same point set as the ring
 * written once, so each relationship answers alike. The square from (0, 0)
 * to (2, 2) is written with one vertex repeated at each of its positions,
 * and in the middle of a side, against the square written once, which shares
 * its whole boundary, in both orders: the DE-9IM matrix of two equal areas,
 * 2FFF1FFF2, and contains and covers true. Against a larger square holding
 * it, a square sharing one side and a square overlapping it, each matrix is
 * the one the square written once answers. The cases mirror #arc_cases of
 * arc_cross_unit.c in being built so that their answer is known; every
 * coordinate is a small integer, exact on doubles.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <meos.h>
#include <meos_geo.h>

static int failures = 0;
static int checks = 0;

static void
check(bool ok, const char *what, const char *wkt)
{
  checks++;
  if (! ok)
  {
    failures++;
    printf("FAIL %s: %s\n", what, wkt);
  }
  return;
}

static const char *DUPS[] = {
  "POLYGON((0 0,0 0,2 0,2 2,0 2,0 0))",
  "POLYGON((0 0,2 0,2 0,2 2,0 2,0 0))",
  "POLYGON((0 0,2 0,2 2,2 2,0 2,0 0))",
  "POLYGON((0 0,2 0,2 2,0 2,0 2,0 0))",
  "POLYGON((0 0,2 0,2 2,0 2,0 0,0 0))",
  "POLYGON((0 0,1 0,1 0,2 0,2 2,0 2,0 0))",
};

static const char *ONE = "POLYGON((0 0,2 0,2 2,0 2,0 0))";

static const char *OTHERS[] = {
  "POLYGON((-1 -1,3 -1,3 3,-1 3,-1 -1))",
  "POLYGON((2 0,4 0,4 2,2 2,2 0))",
  "POLYGON((1 1,3 1,3 3,1 3,1 1))",
};

int
main(void)
{
  meos_initialize();
  GSERIALIZED *one = geom_in(ONE, -1);
  size_t sz;
  for (size_t i = 0; i < sizeof(DUPS) / sizeof(DUPS[0]); i++)
  {
    GSERIALIZED *dup = geom_in(DUPS[i], -1);
    char *m1 = geom_relate(dup, one, &sz), *m2 = geom_relate(one, dup, &sz);
    check(m1 && strcmp(m1, "2FFF1FFF2") == 0, "equal squares, repeated first",
      DUPS[i]);
    check(m2 && strcmp(m2, "2FFF1FFF2") == 0, "equal squares, repeated second",
      DUPS[i]);
    check(geom_contains(dup, one) && geom_contains(one, dup), "contains",
      DUPS[i]);
    check(geom_covers(dup, one) && geom_covers(one, dup), "covers", DUPS[i]);
    free(m1); free(m2);
    for (size_t k = 0; k < sizeof(OTHERS) / sizeof(OTHERS[0]); k++)
    {
      GSERIALIZED *o = geom_in(OTHERS[k], -1);
      char *r1 = geom_relate(one, o, &sz), *r2 = geom_relate(o, one, &sz);
      char *d1 = geom_relate(dup, o, &sz), *d2 = geom_relate(o, dup, &sz);
      check(r1 && d1 && strcmp(r1, d1) == 0 && r2 && d2 &&
        strcmp(r2, d2) == 0, "against another square as written once",
        DUPS[i]);
      free(r1); free(r2); free(d1); free(d2); free(o);
    }
    free(dup);
  }
  free(one);
  printf("%d of %d checks pass\n", checks - failures, checks);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
