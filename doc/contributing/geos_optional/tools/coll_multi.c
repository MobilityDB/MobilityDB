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

/* Do the unary union and the overlay hand the array union a multi-part member
   when a collection carries one? The error handler mirrors #handler of the
   sweep probe in doc/contributing/geos_optional/tools/. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

static char msg[512];
static void handler(int l, int c, const char *m) { (void) l; meos_errno_set(c); snprintf(msg, sizeof msg, "%s", m ? m : ""); }

static void
show(const char *what, GSERIALIZED *r)
{
  printf("  %-14s -> %s\n", what, r ? geo_as_text(r, 6) : msg);
}

static void
try(const char *wkt)
{
  GSERIALIZED *g = geom_in(wkt, -1);
  GSERIALIZED *p = geom_in("POLYGON((0 0,10 0,10 10,0 10,0 0))", -1);
  printf("%s\n", wkt);
  msg[0] = 0; show("unary union", geom_unary_union(g, -1));
  msg[0] = 0; show("intersection", geom_intersection2d(g, p));
  msg[0] = 0; show("difference", geom_difference2d(g, p));
}

int
main(void)
{
  meos_initialize();
  meos_initialize_error_handler(handler);
  try("GEOMETRYCOLLECTION(POLYGON((0 0,1 0,1 1,0 1,0 0)),MULTIPOLYGON(((5 5,6 5,6 6,5 6,5 5))))");
  try("GEOMETRYCOLLECTION(POINT(9 9),MULTIPOLYGON(((5 5,6 5,6 6,5 6,5 5))))");
  try("GEOMETRYCOLLECTION(LINESTRING(0 0,9 9),MULTIPOLYGON(((5 5,6 5,6 6,5 6,5 5)),((7 7,8 7,8 8,7 8,7 7))))");
  try("GEOMETRYCOLLECTION(GEOMETRYCOLLECTION(POINT(1 1)),MULTIPOLYGON(((5 5,6 5,6 6,5 6,5 5))))");
  try("GEOMETRYCOLLECTION(POINT(1 1),TIN(((0 0,4 0,2 4,0 0))))");
  meos_finalize();
  return 0;
}
