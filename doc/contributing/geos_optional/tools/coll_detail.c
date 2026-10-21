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

/* Which partners and orders make an overlay of a simple collection reach GEOS.
   Error handler and shapes mirror #handler and S[] of the sweep probe in
   doc/contributing/geos_optional/tools/. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

static char msg[512];
static void handler(int l, int c, const char *m) { (void) l; meos_errno_set(c); snprintf(msg, sizeof msg, "%s", m ? m : ""); }

static const char *T[][2] = {
  {"POINT", "POINT(2 2)"}, {"MULTIPOINT", "MULTIPOINT((1 1),(3 3))"},
  {"LINE", "LINESTRING(0 2,4 2)"}, {"MULTILINE", "MULTILINESTRING((0 1,4 1),(0 3,4 3))"},
  {"POLYGON", "POLYGON((0 0,4 0,4 4,0 4,0 0))"},
  {"MULTIPOLYGON", "MULTIPOLYGON(((0 0,2 0,2 2,0 2,0 0)),((3 3,4 3,4 4,3 4,3 3)))"},
  {"TRIANGLE", "TRIANGLE((0 0,4 0,2 4,0 0))"}, {"CIRCSTRING", "CIRCULARSTRING(0 2,2 4,4 2)"},
  {"COMPOUND", "COMPOUNDCURVE((0 2,0 0),CIRCULARSTRING(0 0,2 2,4 0))"},
  {"MULTICURVE", "MULTICURVE(CIRCULARSTRING(0 2,2 4,4 2),(0 0,4 0))"},
  {"CURVEPOLY", "CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2))"},
  {"MULTISURFACE", "MULTISURFACE(CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2)),((3 3,4 3,4 4,3 4,3 3)))"},
  {"TIN", "TIN(((0 0,4 0,2 4,0 0)),((0 0,2 4,0 4,0 0)))"},
  {"POLYHEDRAL", "POLYHEDRALSURFACE(((0 0,4 0,4 4,0 4,0 0)))"},
  {"COLLECTION", "GEOMETRYCOLLECTION(POINT(1 1),LINESTRING(0 2,4 2))"},
};
enum { N = sizeof(T) / sizeof(T[0]) };

static void
detail(const char *collwkt)
{
  GSERIALIZED *c = geom_in(collwkt, -1);
  printf("%s\n", collwkt);
  for (int k = 0; k < N; k++)
  {
    GSERIALIZED *o = geom_in(T[k][1], -1);
    GSERIALIZED *r;
    const char *lab[4] = {"coll n other", "other n coll", "coll - other", "other - coll"};
    for (int m = 0; m < 4; m++)
    {
      msg[0] = 0; meos_errno_reset();
      r = (m == 0) ? geom_intersection2d(c, o) : (m == 1) ? geom_intersection2d(o, c) :
          (m == 2) ? geom_difference2d(c, o) : geom_difference2d(o, c);
      if (! r && strstr(msg, "which this build excludes"))
        printf("  %-12s %-13s : %.60s\n", lab[m], T[k][0], msg);
      if (r) free(r);
    }
  }
}

int
main(void)
{
  meos_initialize();
  meos_initialize_error_handler(handler);
  detail("GEOMETRYCOLLECTION(POINT(2 2),POLYGON((0 0,4 0,4 4,0 4,0 0)))");
  detail("GEOMETRYCOLLECTION(LINESTRING(0 2,4 2),POLYGON((0 0,4 0,4 4,0 4,0 0)))");
  detail("GEOMETRYCOLLECTION(POLYGON((0 0,4 0,4 4,0 4,0 0)),POLYGON((0 0,4 0,4 4,0 4,0 0)))");
  meos_finalize();
  return 0;
}
