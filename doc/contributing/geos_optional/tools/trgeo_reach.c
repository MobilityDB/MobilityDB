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

/* Rigid temporal geometries whose reference geometry is a POLYGON or a
   MULTIPOLYGON: which public functions reach GEOS in a build without it.
   Error handler mirrors #handler of the sweep probe in
   doc/contributing/geos_optional/tools/. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_rgeo.h>

static char msg[512];
static void handler(int l, int c, const char *m) { (void) l; meos_errno_set(c); snprintf(msg, sizeof msg, "%s", m ? m : ""); }

static void
report(const char *fn, const void *r)
{
  const char *state = r ? "answered" :
    (strstr(msg, "which this build excludes") ? "REACHES GEOS" : (msg[0] ? "refused" : "NULL, no error"));
  printf("  %-28s %s%s%.70s\n", fn, state, (! r && msg[0]) ? ": " : "", (! r) ? msg : "");
}

static void
run(const char *lit)
{
  msg[0] = 0;
  Temporal *t = trgeometry_in(lit);
  printf("%s\n", lit);
  if (! t) { printf("  UNREADABLE: %s\n", msg); return; }
  msg[0] = 0; meos_errno_reset(); report("trgeometry_traversed_area", trgeometry_traversed_area(t, true));
  msg[0] = 0; meos_errno_reset(); report("trgeometry_convex_hull", trgeometry_convex_hull(t));
}

int
main(void)
{
  meos_initialize();
  meos_initialize_error_handler(handler);
  run("Polygon((0 0,1 0,1 1,0 1,0 0));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]");
  run("MultiPolygon(((0 0,1 0,1 1,0 1,0 0)),((2 2,3 2,3 3,2 3,2 2)));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-05]");
  run("PolyhedralSurface Z(((0 0 0,1 0 0,1 1 0,0 1 0,0 0 0)),((0 0 0,0 1 0,0 1 1,0 0 1,0 0 0)));[Pose(Point Z(0 0 0),0,0,0,1)@2001-01-01, Pose(Point Z(4 0 0),0,0,0,1)@2001-01-05]");
  meos_finalize();
  return 0;
}
