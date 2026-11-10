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
 * @brief Test that the MEOS build refuses a route of the ways CSV file that
 * is not a planar line in the spatial reference system of the network
 * @details A program that writes three network files in the current
 * directory and removes them at the end, as ways_index_test.c writes its
 * files. The first holds a planar line of length 5 in SRID 5676, then a line
 * with a Z dimension, a line with an M dimension, a multiline, a line in SRID
 * 4326, a point, a line with an infinite coordinate and a line with a NaN
 * coordinate; the second holds the planar line alone; the third holds the
 * line with an infinite coordinate, then the planar line.
 *
 * The program verifies that #route_exists and #route_length answer the
 * planar line, that #route_exists refuses each other route with
 * MEOS_ERR_INVALID_ARG_VALUE, that #geompoint_to_npoint refuses the first
 * network, whose scan reaches the line with a Z dimension, and that it
 * answers the second, and that #get_srid_ways and #geompoint_to_npoint
 * refuse the third, whose first line holds a coordinate no geometry holds.
 * The error handler does not exit, so a reader that went on with the line it
 * refused would be seen.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o ways_route_validity_test ways_route_validity_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_npoint.h>

/* The three network files the program writes */
static const char *WAYS_A = "ways_route_validity_test_a.csv";
static const char *WAYS_B = "ways_route_validity_test_b.csv";
static const char *WAYS_C = "ways_route_validity_test_c.csv";

/* The routes, as hex EWKB: LINESTRING(0 0,3 4) in SRID 5676, the same line
 * with a Z and with an M dimension, the MULTILINESTRING holding it, the line
 * in SRID 4326, POINT(1 1) in SRID 5676, and the line in SRID 5676 with its
 * second x infinite and with its second y NaN */
static const char *ROUTES_A =
  "1,01020000202C160000020000000000000000000000000000000000000000000000"
  "000008400000000000001040\n"
  "2,01020000A02C16000002000000000000000000000000000000000000000000000000"
  "000000000000000000084000000000000010400000000000000000\n"
  "3,01020000602C16000002000000000000000000000000000000000000000000000000"
  "000000000000000000084000000000000010400000000000000000\n"
  "4,01050000202C16000001000000010200000002000000000000000000000000000000"
  "0000000000000000000008400000000000001040\n"
  "5,0102000020E6100000020000000000000000000000000000000000000000000000"
  "000008400000000000001040\n"
  "6,01010000202C160000000000000000F03F000000000000F03F\n"
  "7,01020000202C160000020000000000000000000000000000000000000000000000"
  "0000F07F0000000000001040\n"
  "8,01020000202C160000020000000000000000000000000000000000000000000000"
  "00000840000000000000F87F\n";
static const char *ROUTES_B =
  "1,01020000202C160000020000000000000000000000000000000000000000000000"
  "000008400000000000001040\n";
static const char *ROUTES_C =
  "7,01020000202C160000020000000000000000000000000000000000000000000000"
  "0000F07F0000000000001040\n"
  "1,01020000202C160000020000000000000000000000000000000000000000000000"
  "000008400000000000001040\n";

static int failures = 0;

static void
check_route(const char *what, int64 rid, bool want_exists)
{
  meos_errno_reset();
  bool exists = route_exists(rid);
  int err = meos_errno();
  double length = exists ? route_length(rid) : 0.0;
  bool ok = want_exists ?
    (exists && err == 0 && length == 5.0) :
    (! exists && err == MEOS_ERR_INVALID_ARG_VALUE);
  if (! ok)
    failures++;
  printf("%s %s: exists %d, length %g, errno %d\n", ok ? "ok  " : "FAIL",
    what, exists, length, err);
}

static void
check_nearest(const char *what, bool want_npoint)
{
  GSERIALIZED *gs = geom_in("SRID=5676;POINT(1 1)", -1);
  meos_errno_reset();
  Npoint *np = geompoint_to_npoint(gs, 10.0);
  int err = meos_errno();
  bool ok = want_npoint ?
    (np && err == 0 && npoint_route(np) == 1) :
    (! np && err == MEOS_ERR_INVALID_ARG_VALUE);
  if (! ok)
    failures++;
  printf("%s %s: %s, errno %d\n", ok ? "ok  " : "FAIL", what,
    np ? "a network point" : "NULL", err);
  free(np);
  free(gs);
}

/* Check that #get_srid_ways refuses the network, as #check_route checks that
 * #route_exists refuses a route */
static void
check_srid(const char *what)
{
  meos_errno_reset();
  int32_t srid = get_srid_ways();
  int err = meos_errno();
  bool ok = (srid != 5676 && err == MEOS_ERR_INVALID_ARG_VALUE);
  if (! ok)
    failures++;
  printf("%s %s: SRID %d, errno %d\n", ok ? "ok  " : "FAIL", what, srid,
    err);
}

static bool
write_file(const char *path, const char *text)
{
  FILE *file = fopen(path, "w");
  if (! file)
    return false;
  fputs(text, file);
  fclose(file);
  return true;
}

int
main(void)
{
  if (! write_file(WAYS_A, ROUTES_A) || ! write_file(WAYS_B, ROUTES_B) ||
      ! write_file(WAYS_C, ROUTES_C))
    return 1;

  meos_initialize();
  meos_initialize_noexit_error_handler();
  meos_set_ways_csv(WAYS_A);
  check_route("route 1, a planar line", 1, true);
  check_route("route 2, a line with a Z dimension", 2, false);
  check_route("route 3, a line with an M dimension", 3, false);
  check_route("route 4, a multiline", 4, false);
  check_route("route 5, a line in another SRID", 5, false);
  check_route("route 6, a point", 6, false);
  check_route("route 7, a line with an infinite coordinate", 7, false);
  check_route("route 8, a line with a NaN coordinate", 8, false);
  check_nearest("nearest route on the first network", false);
  meos_finalize_ways();
  meos_set_ways_csv(WAYS_B);
  check_nearest("nearest route on the second network", true);
  meos_finalize_ways();
  meos_set_ways_csv(WAYS_C);
  check_srid("SRID of the third network");
  check_nearest("nearest route on the third network", false);
  meos_finalize();

  remove(WAYS_A);
  remove(WAYS_B);
  remove(WAYS_C);
  printf("%d failure(s)\n", failures);
  return failures ? 1 : 0;
}
