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

/* Temporal circular buffers against a POLYGON, a MULTIPOLYGON and a
   collection: which public functions reach GEOS in a build without it.
   Error handler and report mirror #handler and #report of temporal_reach.c in this directory. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_cbuffer.h>

static char msg[512];
static void handler(int l, int c, const char *m) { (void) l; meos_errno_set(c); snprintf(msg, sizeof msg, "%s", m ? m : ""); }

static void
report(const char *fn, const void *r)
{
  const char *state = r ? "answered" :
    (strstr(msg, "which this build excludes") ? "REACHES GEOS" : (msg[0] ? "refused" : "NULL, no error"));
  printf("  %-26s %s%s%.70s\n", fn, state, (! r && msg[0]) ? ": " : "", (! r) ? msg : "");
}
#define CALL(fn, expr) do { msg[0] = 0; meos_errno_reset(); const void *r_ = (expr); report(fn, r_); } while (0)

static void
run(Temporal *t, const char *geo)
{
  GSERIALIZED *g = geom_in(geo, -1);
  printf("with %s\n", geo);
  CALL("tcbuffer_traversed_area", tcbuffer_traversed_area(t, true));
  CALL("tcbuffer_at_geom", tcbuffer_at_geom(t, g));
  CALL("tintersects_tcbuffer_geo", tintersects_tcbuffer_geo(t, g));
  CALL("tdwithin_tcbuffer_geo", tdwithin_tcbuffer_geo(t, g, 1.0));
  msg[0] = 0; meos_errno_reset();
  int e = eintersects_tcbuffer_geo(t, g);
  printf("  %-26s %s%.70s\n", "eintersects_tcbuffer_geo", e >= 0 ? "answered" :
    (strstr(msg, "which this build excludes") ? "REACHES GEOS: " : "refused: "), e >= 0 ? "" : msg);
}

int
main(void)
{
  meos_initialize();
  meos_initialize_error_handler(handler);
  Temporal *t = tcbuffer_in("[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(4 4),0.5)@2000-01-02]");
  printf("[Cbuffer(Point(0 0),0.5)@2000-01-01, Cbuffer(Point(4 4),0.5)@2000-01-02]\n");
  if (! t) { printf("UNREADABLE: %s\n", msg); return 1; }
  run(t, "POLYGON((1 1,3 1,3 3,1 3,1 1))");
  run(t, "MULTIPOLYGON(((1 1,2 1,2 2,1 2,1 1)),((3 3,5 3,5 5,3 5,3 3)))");
  run(t, "GEOMETRYCOLLECTION(POINT(2 2),POLYGON((1 1,3 1,3 3,1 3,1 1)))");
  meos_finalize();
  return 0;
}
