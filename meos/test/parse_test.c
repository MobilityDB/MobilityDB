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
 * @brief Ground-truth correctness gate for the vendored liblwgeom WKT
 * geometry parser (the flex/bison lexer in postgis/liblwgeom/).
 *
 * Unlike the crash/leak smoke suites, this test asserts that each parse
 * actually SUCCEEDS: every valid WKT string must produce a non-NULL
 * geometry that round-trips through geo_out(), and every malformed string
 * must be rejected (NULL). A parser change that silently breaks parsing
 * (e.g. a reentrancy conversion that mis-threads the lexer state) makes
 * geom_in() return NULL on valid input — which a no-crash stress test
 * cannot see, because nothing parses so nothing races. This test catches
 * exactly that failure mode; it must stay green for any change to the
 * WKT lexer/grammar before that change is published.
 *
 * Build:
 * @code
 * gcc -Wall -g -I<prefix>/include -o parse_test parse_test.c \
 *     -L<prefix>/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

static int failures = 0;

/* A non-exiting error handler so a deliberately malformed WKT makes
 * geom_in() return NULL instead of aborting the process. Installed via
 * meos_initialize_error_handler(), which is available on every MEOS base. */
static void
quiet_error_handler(int errlevel, int errcode, const char *errmsg)
{
  (void) errlevel; (void) errcode; (void) errmsg;
}

#define CHECK(cond, msg) \
  do { \
    if (! (cond)) { \
      printf("[FAIL] %s\n", (msg)); \
      failures++; \
    } \
  } while (0)

/**
 * @brief A valid WKT string must parse to a non-NULL geometry whose
 * output representation is non-empty.
 */
static void
check_valid_geom(const char *wkt)
{
  GSERIALIZED *g = geom_in(wkt, -1);
  if (! g)
  {
    printf("[FAIL] geom_in returned NULL for valid WKT: %s\n", wkt);
    failures++;
    return;
  }
  char *out = geo_out(g);
  CHECK(out != NULL && out[0] != '\0', wkt);
  free(out);
  free(g);
}

/**
 * @brief A valid WKT string must parse to a non-NULL geography.
 */
static void
check_valid_geog(const char *wkt)
{
  GSERIALIZED *g = geog_in(wkt, -1);
  if (! g)
  {
    printf("[FAIL] geog_in returned NULL for valid WKT: %s\n", wkt);
    failures++;
    return;
  }
  free(g);
}

/**
 * @brief A malformed WKT string must be rejected (geom_in returns NULL),
 * confirming the parser's error path still works after any change.
 */
static void
check_invalid_geom(const char *wkt)
{
  GSERIALIZED *g = geom_in(wkt, -1);
  if (g)
  {
    printf("[FAIL] geom_in accepted invalid WKT: %s\n", wkt);
    failures++;
    free(g);
  }
  meos_errno_reset();
}

/* Valid WKT exercising the distinct grammar rules the lexer/parser drives:
 * point/line/polygon/multi/collection, dimensionality (Z/M/ZM), EMPTY,
 * SRID prefixes, and numeric forms (negative, scientific notation). */
static const char *valid_geoms[] = {
  "Point(0 0)",                                   /* lowercase keyword     */
  "POINT(1 2)",
  "POINT(-1.5 -2.5)",                             /* negative coords       */
  "POINT(1e3 2.5e-2)",                            /* scientific notation   */
  "POINT Z (1 2 3)",
  "POINT M (1 2 3)",
  "POINT ZM (1 2 3 4)",
  "POINT EMPTY",
  "SRID=4326;POINT(1 2)",                         /* EWKT SRID prefix      */
  "LINESTRING(0 0,1 1,2 2)",
  "LINESTRING EMPTY",
  "POLYGON((0 0,1 0,1 1,0 1,0 0))",
  "POLYGON((0 0,4 0,4 4,0 4,0 0),(1 1,2 1,2 2,1 2,1 1))", /* with hole     */
  "MULTIPOINT(0 0,1 1)",
  "MULTIPOINT((0 0),(1 1))",                      /* parenthesized form    */
  "MULTILINESTRING((0 0,1 1),(2 2,3 3))",
  "MULTIPOLYGON(((0 0,1 0,1 1,0 1,0 0)))",
  "GEOMETRYCOLLECTION(POINT(0 0),LINESTRING(1 1,2 2))",
  "GEOMETRYCOLLECTION EMPTY",
};

/* Strings the parser must reject. */
static const char *invalid_geoms[] = {
  "Point(0 0",                                    /* unbalanced paren      */
  "NOTAGEOMETRY(1 2)",                            /* unknown keyword       */
  "POINT(1)",                                     /* too few coordinates   */
  "",                                             /* empty string          */
};

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_error_handler(quiet_error_handler);

  printf("****************************************************************\n");
  printf("* WKT parser correctness gate *\n");
  printf("****************************************************************\n");

  for (size_t i = 0; i < sizeof(valid_geoms) / sizeof(valid_geoms[0]); i++)
    check_valid_geom(valid_geoms[i]);

  /* Geography path (the geodetic lexer/grammar entry). */
  check_valid_geog("POINT(1 2)");
  check_valid_geog("SRID=4326;LINESTRING(0 0,1 1)");

  for (size_t i = 0; i < sizeof(invalid_geoms) / sizeof(invalid_geoms[0]); i++)
    check_invalid_geom(invalid_geoms[i]);

  meos_finalize();

  if (failures == 0)
    printf("parse_test: all parses validated\n");
  else
    printf("parse_test: %d FAILURES\n", failures);
  return failures == 0 ? 0 : 1;
}
