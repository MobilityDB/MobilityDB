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

/* The public functions that can reach a GEOS fall-back reach it through a
   handful of internal gateways, a route question and not an answer: the
   overlay (geom_intersection2d), the trajectory of a temporal point, the array
   union, the merge of a rigid geometry's placements, the traversed area of a
   moving disc and the collection of a temporal geometry's values. This probe
   runs public functions of each gateway on the inputs that reached a fall-back
   in geos_fallback_sweep.c: multi-part areal values and collections, as the
   temporal value and as the geometry argument, in a build with GEOS compiled
   out. The fall-back is identified by its own message, as #handler of
   geos_fallback_sweep.c identifies it. The program exits 1 where any call
   reaches a fall-back, so a job can run it as a check. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_cbuffer.h>

static char msg[512];
static void handler(int l, int c, const char *m) { (void) l; meos_errno_set(c); snprintf(msg, sizeof msg, "%s", m ? m : ""); }

static int n_reach = 0, n_ans = 0, n_other = 0;

static void
report(const char *fn, const char *in, const void *r)
{
  if (r) { n_ans++; return; }
  if (strstr(msg, "which this build excludes"))
  {
    n_reach++;
    printf("  REACHES GEOS  %-26s %s\n", fn, in);
  }
  else
  {
    n_other++;
    printf("  other         %-26s %s: %.60s\n", fn, in, msg[0] ? msg : "NULL, no error");
  }
}
#define CALL(fn, in, expr) do { msg[0] = 0; meos_errno_reset(); const void *r_ = (expr); report(fn, in, r_); } while (0)
static void
report_int(const char *fn, const char *in, int v)
{
  report(fn, in, v >= 0 ? (const void *) 1 : NULL);
}

static const char *G[][2] = {
  {"POLYGON", "POLYGON((1 1,3 1,3 3,1 3,1 1))"},
  {"MULTIPOLYGON", "MULTIPOLYGON(((1 1,2 1,2 2,1 2,1 1)),((3 3,5 3,5 5,3 5,3 3)))"},
  {"MULTISURFACE", "MULTISURFACE(CURVEPOLYGON(CIRCULARSTRING(1 2,2 3,3 2,2 1,1 2)),((3 3,5 3,5 5,3 5,3 3)))"},
  {"TIN", "TIN(((0 0,4 0,2 4,0 0)),((0 0,2 4,0 4,0 0)))"},
  {"POLYHEDRAL", "POLYHEDRALSURFACE(((0 0,4 0,4 4,0 4,0 0)))"},
  {"GC(POINT,MULTIPOLYGON)", "GEOMETRYCOLLECTION(POINT(2 2),MULTIPOLYGON(((1 1,2 1,2 2,1 2,1 1)),((3 3,5 3,5 5,3 5,3 3))))"},
  {"GC(LINE,POLYGON)", "GEOMETRYCOLLECTION(LINESTRING(0 2,4 2),POLYGON((1 1,3 1,3 3,1 3,1 1)))"},
};
enum { NG = sizeof(G) / sizeof(G[0]) };

int
main(void)
{
  meos_initialize();
  meos_initialize_error_handler(handler);
  char in[512];

  /* Temporal points: the overlay gateway and the trajectory gateway */
  const char *tp[] = {
    "[Point(0 0)@2001-01-01, Point(4 4)@2001-01-02]",
    "{[Point(0 0)@2001-01-01, Point(4 4)@2001-01-02], [Point(4 0)@2001-01-03, Point(0 4)@2001-01-04]}" };
  for (int i = 0; i < 2; i++)
  {
    Temporal *t = tgeompoint_in(tp[i]);
    CALL("tgeo_convex_hull", tp[i], tgeo_convex_hull(t));
    CALL("tgeo_traversed_area", tp[i], tgeo_traversed_area(t, true));
    for (int k = 0; k < NG; k++)
    {
      GSERIALIZED *g = geom_in(G[k][1], -1);
      snprintf(in, sizeof in, "%s with %s", i ? "tgeompoint seqset" : "tgeompoint seq", G[k][0]);
      CALL("tintersects_tgeo_geo", in, tintersects_tgeo_geo(t, g));
      CALL("tcontains_geo_tgeo", in, tcontains_geo_tgeo(g, t));
      CALL("tpoint_at_geom", in, tpoint_at_geom(t, g));
      CALL("tgeo_minus_geom", in, tgeo_minus_geom(t, g));
      msg[0] = 0; meos_errno_reset(); report_int("eintersects_tgeo_geo", in, eintersects_tgeo_geo(t, g));
      msg[0] = 0; meos_errno_reset(); report_int("econtains_geo_tgeo", in, econtains_geo_tgeo(g, t));
    }
    STBox *b = stbox_in("STBOX X((1,1),(3,3))");
    CALL("tgeo_at_stbox", tp[i], tgeo_at_stbox(t, b, true));
  }

  /* Temporal geometries whose values are multi-part or collections */
  for (int v = 0; v < NG; v++)
  {
    char lit[1024];
    snprintf(lit, sizeof lit, "{%s@2001-01-01, POLYGON((1 1,5 1,5 5,1 5,1 1))@2001-01-02}", G[v][1]);
    Temporal *t = tgeometry_in(lit);
    if (! t) { printf("  UNREADABLE tgeometry with %s value: %s\n", G[v][0], msg); continue; }
    snprintf(in, sizeof in, "tgeometry with a %s value", G[v][0]);
    CALL("tgeo_traversed_area", in, tgeo_traversed_area(t, true));
    CALL("tgeo_convex_hull", in, tgeo_convex_hull(t));
    STBox *b = stbox_in("STBOX X((1,1),(3,3))");
    CALL("tgeo_at_stbox", in, tgeo_at_stbox(t, b, true));
    for (int k = 0; k < NG; k++)
    {
      GSERIALIZED *g = geom_in(G[k][1], -1);
      snprintf(in, sizeof in, "tgeometry with a %s value, with %s", G[v][0], G[k][0]);
      CALL("tgeo_at_geom", in, tgeo_at_geom(t, g));
      CALL("tgeo_minus_geom", in, tgeo_minus_geom(t, g));
      CALL("tintersects_tgeo_geo", in, tintersects_tgeo_geo(t, g));
      msg[0] = 0; meos_errno_reset(); report_int("eintersects_tgeo_geo", in, eintersects_tgeo_geo(t, g));
    }
  }

  /* Moving discs: the traversed-area and overlay gateways */
  Temporal *tc = tcbuffer_in("{[Cbuffer(Point(0 0),0.5)@2001-01-01, Cbuffer(Point(4 4),0.5)@2001-01-02], "
    "[Cbuffer(Point(4 0),0.5)@2001-01-03, Cbuffer(Point(0 4),0.5)@2001-01-04]}");
  CALL("tcbuffer_traversed_area", "tcbuffer seqset", tcbuffer_traversed_area(tc, true));
  for (int k = 0; k < NG; k++)
  {
    GSERIALIZED *g = geom_in(G[k][1], -1);
    snprintf(in, sizeof in, "tcbuffer seqset with %s", G[k][0]);
    CALL("tcbuffer_at_geom", in, tcbuffer_at_geom(tc, g));
    CALL("tintersects_tcbuffer_geo", in, tintersects_tcbuffer_geo(tc, g));
    CALL("tdwithin_tcbuffer_geo", in, tdwithin_tcbuffer_geo(tc, g, 1.0));
    msg[0] = 0; meos_errno_reset(); report_int("eintersects_tcbuffer_geo", in, eintersects_tcbuffer_geo(tc, g));
  }
  Cbuffer *cb = cbuffer_in("Cbuffer(Point(2 2),5)");
  CALL("tcontains_cbuffer_tcbuffer", "tcbuffer seqset", tcontains_cbuffer_tcbuffer(cb, tc));

  printf("\ncalls answered %d, reaching GEOS %d, other outcomes %d\n", n_ans, n_reach, n_other);
  meos_finalize();
  return n_reach > 0 ? 1 : 0;
}
