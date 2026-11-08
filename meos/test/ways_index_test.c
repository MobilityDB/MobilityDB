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
 * @brief Test how the MEOS build reads a route of the ways CSV file
 * @details A program that tests which line of the ways CSV file answers a
 * route, on two files it writes in the current directory and removes at the
 * end, as tnpoint_stops_test.c writes its network: the first holds a route
 * of length 5, a route whose only line has an empty geometry, a route whose
 * first line is empty and whose second has length 2, and a route of length
 * 10 on a last line without its newline; the second holds a route of
 * length 1.
 *
 * The program verifies that #route_exists and #route_length answer the route
 * of each line, the first line with a geometry that is not empty for a route
 * on two lines, no route for one whose geometry is empty or that the file
 * lacks, the routes of the second file once #meos_set_ways_csv names it, and
 * the first file again after #meos_finalize_ways.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o ways_index_test ways_index_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_npoint.h>

/* The two network files the program writes */
static const char *WAYS_A = "ways_index_test_a.csv";
static const char *WAYS_B = "ways_index_test_b.csv";

/* The routes, as hex EWKB of LINESTRINGs with SRID 5676 */
static const char *ROUTES_A =
  "1,01020000202C160000020000000000000000000000000000000000000000000000"
  "000008400000000000001040\n"
  "2,01020000202C16000000000000\n"
  "3,01020000202C16000000000000\n"
  "3,01020000202C160000020000000000000000000000000000000000000000000000"
  "000000000000000000000040\n"
  "4,01020000202C160000020000000000000000000000000000000000000000000000"
  "000018400000000000002040";
static const char *ROUTES_B =
  "10,01020000202C16000002000000000000000000000000000000000000000000000"
  "00000F03F0000000000000000\n";

static int failures = 0;

static void
check(const char *what, int64 rid, bool want_exists, double want_length)
{
  bool exists = route_exists(rid);
  double length = exists ? route_length(rid) : 0.0;
  bool ok = exists == want_exists && (! want_exists || length == want_length);
  if (! ok)
    failures++;
  printf("%s %s: exists %d, length %g\n", ok ? "ok  " : "FAIL", what, exists,
    length);
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
  if (! write_file(WAYS_A, ROUTES_A) || ! write_file(WAYS_B, ROUTES_B))
    return 1;

  meos_initialize();
  meos_initialize_noexit_error_handler();
  meos_set_ways_csv(WAYS_A);
  check("route 1 of length 5", 1, true, 5.0);
  check("route 2 whose only line is empty", 2, false, 0.0);
  check("route 3 whose first line is empty", 3, true, 2.0);
  check("route 4 on a last line without its newline", 4, true, 10.0);
  check("route 9 the file lacks", 9, false, 0.0);
  check("route 10 of the second file, before it is named", 10, false, 0.0);
  meos_set_ways_csv(WAYS_B);
  check("route 10 once the second file is named", 10, true, 1.0);
  meos_finalize_ways();
  meos_set_ways_csv(WAYS_A);
  check("route 1 of the first file again, after finalizing", 1, true, 5.0);
  meos_finalize();

  remove(WAYS_A);
  remove(WAYS_B);
  printf("%d failure(s)\n", failures);
  return failures ? 1 : 0;
}
