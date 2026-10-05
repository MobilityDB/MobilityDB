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

/* Which inputs reach the four GEOS fall-backs of meos/src/geo/postgis_funcs.c
   in a build with GEOS compiled out.

   It takes one representative geometry of each of 15 types and:
   - identifies the fall-back by its own MESSAGE, "which this build
     excludes", through an installed error handler, since other native
     refusals share its error code;
   - covers the four fall-backs: intersection and difference over every
     ordered pair, the array union over every ordered pair, and the unary
     union of every shape, each flat and with Z on either or both sides;
   - adds the geometric special cases a type sweep cannot see.
   Controls: a unary union on a precision grid MUST reach the fall-back
   (positive), and a geodetic array MUST be refused before it (negative).
   The program exits 1 where a control fails or any input other than the
   positive control reaches a fall-back, so a job can run it as a check. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

static int geos_hit = 0, other_err = 0;
static char last_msg[512];

static void
handler(int level, int code, const char *msg)
{
  (void) level;
  meos_errno_set(code);
  snprintf(last_msg, sizeof last_msg, "%s", msg ? msg : "");
  if (msg && strstr(msg, "which this build excludes"))
    geos_hit = 1;
  else
    other_err = 1;
}

static void reset(void) { geos_hit = other_err = 0; last_msg[0] = 0; meos_errno_reset(); }

typedef struct { const char *name, *wkt, *wktz; } Shape;
static const Shape S[] = {
  {"POINT", "POINT(2 2)", "POINT Z(2 2 1)"},
  {"MULTIPOINT", "MULTIPOINT((1 1),(3 3))", "MULTIPOINT Z((1 1 1),(3 3 2))"},
  {"LINE", "LINESTRING(0 2,4 2)", "LINESTRING Z(0 2 1,4 2 2)"},
  {"MULTILINE", "MULTILINESTRING((0 1,4 1),(0 3,4 3))", "MULTILINESTRING Z((0 1 1,4 1 1),(0 3 2,4 3 2))"},
  {"POLYGON", "POLYGON((0 0,4 0,4 4,0 4,0 0))", "POLYGON Z((0 0 1,4 0 1,4 4 1,0 4 1,0 0 1))"},
  {"MULTIPOLYGON", "MULTIPOLYGON(((0 0,2 0,2 2,0 2,0 0)),((3 3,4 3,4 4,3 4,3 3)))", "MULTIPOLYGON Z(((0 0 1,2 0 1,2 2 1,0 2 1,0 0 1)),((3 3 2,4 3 2,4 4 2,3 4 2,3 3 2)))"},
  {"TRIANGLE", "TRIANGLE((0 0,4 0,2 4,0 0))", "TRIANGLE Z((0 0 1,4 0 1,2 4 1,0 0 1))"},
  {"CIRCSTRING", "CIRCULARSTRING(0 2,2 4,4 2)", "CIRCULARSTRING Z(0 2 1,2 4 1,4 2 1)"},
  {"COMPOUND", "COMPOUNDCURVE((0 2,0 0),CIRCULARSTRING(0 0,2 2,4 0))", "COMPOUNDCURVE Z((0 2 1,0 0 1),CIRCULARSTRING Z(0 0 1,2 2 1,4 0 1))"},
  {"MULTICURVE", "MULTICURVE(CIRCULARSTRING(0 2,2 4,4 2),(0 0,4 0))", "MULTICURVE Z(CIRCULARSTRING Z(0 2 1,2 4 1,4 2 1),(0 0 1,4 0 1))"},
  {"CURVEPOLY", "CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2))", "CURVEPOLYGON Z(CIRCULARSTRING Z(0 2 1,2 4 1,4 2 1,2 0 1,0 2 1))"},
  {"MULTISURFACE", "MULTISURFACE(CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2)),((3 3,4 3,4 4,3 4,3 3)))", "MULTISURFACE Z(CURVEPOLYGON Z(CIRCULARSTRING Z(0 2 1,2 4 1,4 2 1,2 0 1,0 2 1)),((3 3 2,4 3 2,4 4 2,3 4 2,3 3 2)))"},
  {"TIN", "TIN(((0 0,4 0,2 4,0 0)),((0 0,2 4,0 4,0 0)))", "TIN Z(((0 0 1,4 0 1,2 4 1,0 0 1)),((0 0 1,2 4 1,0 4 1,0 0 1)))"},
  {"POLYHEDRAL", "POLYHEDRALSURFACE(((0 0,4 0,4 4,0 4,0 0)))", "POLYHEDRALSURFACE Z(((0 0 1,4 0 1,4 4 1,0 4 1,0 0 1)))"},
  {"COLLECTION", "GEOMETRYCOLLECTION(POINT(1 1),LINESTRING(0 2,4 2))", "GEOMETRYCOLLECTION Z(POINT Z(1 1 1),LINESTRING Z(0 2 1,4 2 2))"},
};
enum { N = sizeof(S) / sizeof(S[0]) };

/* Geometric special cases, each a pair (a, b) */
typedef struct { const char *name, *a, *b; } Case;
static const Case C[] = {
  {"identical arcs", "CIRCULARSTRING(0 0,1 1,2 0)", "CIRCULARSTRING(0 0,1 1,2 0)"},
  {"arcs coinciding over part of one circle", "CIRCULARSTRING(0 0,1 1,2 0)", "CIRCULARSTRING(1 1,1.7071067811865475 0.7071067811865476,2 0)"},
  {"arc reversed on the same circle", "CIRCULARSTRING(0 0,1 1,2 0)", "CIRCULARSTRING(2 0,1 1,0 0)"},
  {"lines that share a stretch then fork", "LINESTRING(0 0,3 0)", "LINESTRING(1 0,2 0,2 1)"},
  {"overlapping collinear lines", "LINESTRING(0 0,2 0)", "LINESTRING(1 0,3 0)"},
  {"a line doubling back on itself", "LINESTRING(0 0,2 0,1 0,3 0)", "POINT(5 5)"},
  {"polygons sharing an edge", "POLYGON((0 0,1 0,1 1,0 1,0 0))", "POLYGON((1 0,2 0,2 1,1 1,1 0))"},
  {"polygons touching at a corner", "POLYGON((0 0,1 0,1 1,0 1,0 0))", "POLYGON((1 1,2 1,2 2,1 2,1 1))"},
  {"polygons sharing part of an edge", "POLYGON((0 0,2 0,2 1,0 1,0 0))", "POLYGON((1 1,3 1,3 2,1 2,1 1))"},
  {"discs tangent at one point", "CURVEPOLYGON(CIRCULARSTRING(0 0,1 1,2 0,1 -1,0 0))", "CURVEPOLYGON(CIRCULARSTRING(2 0,3 1,4 0,3 -1,2 0))"},
  {"disc and square sharing an arc chord region", "CURVEPOLYGON(CIRCULARSTRING(0 0,1 1,2 0,1 -1,0 0))", "POLYGON((1 -2,3 -2,3 2,1 2,1 -2))"},
  {"polygon and the hole it fills", "POLYGON((0 0,4 0,4 4,0 4,0 0),(1 1,3 1,3 3,1 3,1 1))", "POLYGON((1 1,3 1,3 3,1 3,1 1))"},
  {"a centimetre triangle at projected coordinates", "POLYGON((593909.1750097702 6218274.6089919759,593909.16701386357 6218274.5990193365,593909.18297354609 6218274.6090548495,593909.1750097702 6218274.6089919759))", "POLYGON((593910 6218270,593911 6218270,593911 6218271,593910 6218271,593910 6218270))"},
  {"line along a polygon edge", "LINESTRING(0 0,1 0)", "POLYGON((0 0,1 0,1 1,0 1,0 0))"},
  {"line along an arc boundary", "CIRCULARSTRING(0 0,1 1,2 0)", "CURVEPOLYGON(CIRCULARSTRING(0 0,1 1,2 0,1 -1,0 0))"},
  {"line of zero length", "LINESTRING(1 1,1 1)", "LINESTRING(0 0,2 2)"},
};
enum { NC = sizeof(C) / sizeof(C[0]) };

static GSERIALIZED *
rd(const char *wkt)
{
  reset();
  GSERIALIZED *g = geom_in(wkt, -1);
  if (! g)
    printf("UNREADABLE %s : %s\n", wkt, last_msg);
  return g;
}

/* op: 0 intersection, 1 difference, 2 array union of the pair */
static int
run(int op, GSERIALIZED *a, GSERIALIZED *b, int *other)
{
  reset();
  GSERIALIZED *r = NULL;
  if (op == 0)
    r = geom_intersection2d(a, b);
  else if (op == 1)
    r = geom_difference2d(a, b);
  else
  {
    GSERIALIZED *arr[2] = {a, b};
    r = geom_array_union(arr, 2);
  }
  if (r) free(r);
  *other += (! r && other_err && ! geos_hit);
  return geos_hit;
}

int
main(void)
{
  meos_initialize();
  meos_initialize_error_handler(handler);
  GSERIALIZED *g[N], *gz[N];
  for (int i = 0; i < N; i++) { g[i] = rd(S[i].wkt); gz[i] = rd(S[i].wktz); }

  /* Controls */
  reset();
  GSERIALIZED *r = geom_unary_union(g[2], 0.5);
  if (r) free(r);
  printf("CONTROL positive: unary union on a precision grid reaches the fall-back: %s\n",
    geos_hit ? "yes" : "NO -- the detector is broken");
  int controls_ok = geos_hit;
  /* Every input that reaches a fall-back, the controls apart */
  int total = 0;
  reset();
  GSERIALIZED *l1 = geog_in("LINESTRING(0 0,1 1)", -1), *l2 = geog_in("LINESTRING(1 0,0 1)", -1);
  reset();
  GSERIALIZED *garr[2] = {l1, l2};
  r = geog_array_union(garr, 2);
  if (r) free(r);
  printf("CONTROL negative: geodetic array of lines reaches the fall-back: %s (refused otherwise: %s)\n",
    geos_hit ? "YES -- unexpected" : "no", other_err ? last_msg : "-");
  controls_ok = controls_ok && ! geos_hit;

  const char *opname[3] = {"intersection", "difference", "array union"};
  const char *modename[4] = {"flat x flat", "Z x Z", "Z x flat", "flat x Z"};
  for (int op = 0; op < 3; op++)
    for (int mode = 0; mode < 4; mode++)
    {
      int hits = 0, other = 0, pairs = 0;
      for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
        {
          GSERIALIZED *a = (mode == 1 || mode == 2) ? gz[i] : g[i];
          GSERIALIZED *b = (mode == 1 || mode == 3) ? gz[j] : g[j];
          if (! a || ! b) continue;
          pairs++;
          if (run(op, a, b, &other))
          {
            hits++;
            printf("  REACHES GEOS: %s %s: %s x %s\n", opname[op], modename[mode],
              S[i].name, S[j].name);
          }
        }
      printf("%-12s %-11s: %3d of %3d ordered pairs reach the fall-back; other refusals %d\n",
        opname[op], modename[mode], hits, pairs, other);
      total += hits;
    }

  int uhits = 0, uother = 0;
  for (int z = 0; z < 2; z++)
    for (int i = 0; i < N; i++)
    {
      GSERIALIZED *a = z ? gz[i] : g[i];
      if (! a) continue;
      reset();
      r = geom_unary_union(a, -1);
      if (r) free(r);
      if (geos_hit) { uhits++; printf("  REACHES GEOS: unary union %s%s\n", S[i].name, z ? " Z" : ""); }
      else if (! r && other_err) { uother++; printf("  other refusal: unary union %s%s: %s\n", S[i].name, z ? " Z" : "", last_msg); }
    }
  printf("unary union  flat and Z : %d of %d shapes reach the fall-back; other refusals %d\n",
    uhits, 2 * N, uother);
  total += uhits;

  /* Collections: a collection is answered part by part, so a type that is
   * answered alone may still reach a fall-back as a member. Every ordered pair
   * of types becomes GEOMETRYCOLLECTION(a, b), whose unary union is taken and
   * which is intersected with and subtracted from every type, in both orders */
  printf("\nCOLLECTIONS OF TWO TYPES\n");
  int cu = 0, ci = 0, cd = 0, cu_n = 0, cb_n = 0;
  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
    {
      char coll[1024];
      snprintf(coll, sizeof coll, "GEOMETRYCOLLECTION(%s,%s)", S[i].wkt, S[j].wkt);
      GSERIALIZED *c = rd(coll);
      if (! c) continue;
      cu_n++;
      reset();
      r = geom_unary_union(c, -1);
      if (r) free(r);
      if (geos_hit) { cu++; printf("  REACHES GEOS: unary union of GEOMETRYCOLLECTION(%s, %s)\n", S[i].name, S[j].name); }
      int hi = 0, hd = 0;
      for (int k = 0; k < N; k++)
      {
        if (! g[k]) continue;
        int other = 0;
        cb_n += 2;
        hi += run(0, c, g[k], &other) + run(0, g[k], c, &other);
        hd += run(1, c, g[k], &other) + run(1, g[k], c, &other);
      }
      ci += hi; cd += hd;
      if (hi || hd)
        printf("  REACHES GEOS: GEOMETRYCOLLECTION(%s, %s) against the 15 types: intersection %d, difference %d of 30 calls each\n",
          S[i].name, S[j].name, hi, hd);
    }
  printf("collections: unary union reaches the fall-back for %d of %d; "
    "intersection %d and difference %d of %d calls each\n", cu, cu_n, ci, cd, cb_n);
  total += cu + ci + cd;

  printf("\nGEOMETRIC SPECIAL CASES\n");
  for (int k = 0; k < NC; k++)
  {
    GSERIALIZED *a = rd(C[k].a), *b = rd(C[k].b);
    if (! a || ! b) continue;
    int other = 0;
    int hi = run(0, a, b, &other), hd = run(1, a, b, &other), hu = run(2, a, b, &other);
    char coll[1024];
    snprintf(coll, sizeof coll, "GEOMETRYCOLLECTION(%s,%s)", C[k].a, C[k].b);
    GSERIALIZED *c = rd(coll);
    int huu = 0;
    if (c) { reset(); r = geom_unary_union(c, -1); if (r) free(r); huu = geos_hit; }
    printf("%-45s intersection %s  difference %s  array union %s  unary union of both %s  other refusals %d\n",
      C[k].name, hi ? "GEOS" : "ok", hd ? "GEOS" : "ok", hu ? "GEOS" : "ok",
      huu ? "GEOS" : "ok", other);
    total += hi + hd + hu + huu;
  }
  meos_finalize();
  printf("\nVERDICT: controls %s, %d input(s) reach a fall-back\n",
    controls_ok ? "pass" : "FAIL", total);
  return (controls_ok && total == 0) ? 0 : 1;
}
