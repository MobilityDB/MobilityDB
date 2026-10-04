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

/* Public temporal functions on temporal geometries whose values are a
   MULTIPOLYGON or a collection: which reach GEOS in a build without it.
   Error handler mirrors #handler of the sweep probe in
   doc/contributing/geos_optional/tools/. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

static char msg[512];
static void handler(int l, int c, const char *m) { (void) l; meos_errno_set(c); snprintf(msg, sizeof msg, "%s", m ? m : ""); }

static void
report(const char *fn, const void *r)
{
  const char *state = r ? "answered" :
    (strstr(msg, "which this build excludes") ? "REACHES GEOS" : (msg[0] ? "refused" : "NULL, no error"));
  printf("  %-24s %s%s%.70s\n", fn, state, (! r && msg[0]) ? ": " : "", (! r) ? msg : "");
}
#define CALL(fn, expr) do { msg[0] = 0; meos_errno_reset(); const void *r_ = (expr); report(fn, r_); } while (0)

static void
run(const char *tgeo, const char *geo)
{
  Temporal *t = tgeometry_in(tgeo);
  GSERIALIZED *g = geom_in(geo, -1);
  printf("%s  with  %s\n", tgeo, geo);
  CALL("tgeo_traversed_area", tgeo_traversed_area(t, true));
  CALL("tgeo_convex_hull", tgeo_convex_hull(t));
  CALL("tgeo_at_geom", tgeo_at_geom(t, g));
  CALL("tgeo_minus_geom", tgeo_minus_geom(t, g));
  CALL("tintersects_tgeo_geo", tintersects_tgeo_geo(t, g));
  CALL("tcontains_geo_tgeo", tcontains_geo_tgeo(g, t));
  msg[0] = 0; meos_errno_reset();
  int e = eintersects_tgeo_geo(t, g);
  printf("  %-24s %s%.70s\n", "eintersects_tgeo_geo", e >= 0 ? "answered" :
    (strstr(msg, "which this build excludes") ? "REACHES GEOS: " : "refused: "), e >= 0 ? "" : msg);
}

int
main(void)
{
  meos_initialize();
  meos_initialize_error_handler(handler);
  const char *mp = "MULTIPOLYGON(((0 0,2 0,2 2,0 2,0 0)),((3 3,4 3,4 4,3 4,3 3)))";
  const char *gc = "GEOMETRYCOLLECTION(POINT(2 2),POLYGON((0 0,4 0,4 4,0 4,0 0)))";
  char buf[1024];
  snprintf(buf, sizeof buf, "{%s@2000-01-01, POLYGON((1 1,5 1,5 5,1 5,1 1))@2000-01-02}", mp);
  run(buf, "POLYGON((0 0,3 0,3 3,0 3,0 0))");
  run(buf, mp);
  snprintf(buf, sizeof buf, "{%s@2000-01-01, POLYGON((1 1,5 1,5 5,1 5,1 1))@2000-01-02}", gc);
  run(buf, "POLYGON((0 0,3 0,3 3,0 3,0 0))");
  run(buf, mp);
  run("{POLYGON((0 0,4 0,4 4,0 4,0 0))@2000-01-01, POLYGON((1 1,5 1,5 5,1 5,1 1))@2000-01-02}", mp);
  meos_finalize();
  return 0;
}
