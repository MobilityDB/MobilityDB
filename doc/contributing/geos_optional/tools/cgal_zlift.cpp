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

// Judge the ORDINATES of a planar overlay's answer by CGAL's exact kernel on
// the doubles the corpus denotes: the rule of meos_lift_ordinates, stated on
// its own.
//
// GEOS is used ONLY to parse WKT into coordinates: decimal text becomes the
// nearest double, the double MEOS reads, and no GEOS arithmetic enters the
// answer.
//
// THE RULE. An input carrying Z determines, at a point of the plane, the Z of
// every one of its segments passing through that point, read along the
// segment, and the Z of every one of its vertices standing there; an input
// carrying none determines nothing. An answer vertex is LIFTED where the
// values the inputs determine there are all one value. The answer carries Z
// exactly where every one of its vertices is lifted, and then carries that
// value at each; otherwise it is the planar figure.
//
// Whether a point lies on a segment is decided on the squared distance, which
// CGAL computes exactly. A vertex the overlay CONSTRUCTS where two segments
// cross is rounded to a double, so it lies on neither segment exactly: the
// test admits a distance up to TOL of the coordinates' magnitude, the band a
// constructed point carries, and a value is compared to another up to ZTOL of
// the ordinates' magnitude. The run prints nothing it does not judge.
//
// usage: cgal_zlift <corpus> <answers> <label>
// Corpus lines are `A|B`; answer lines `<tag>|<answer>` repeated, one pair per
// operation -- `I|<intersection>|D|<difference>` from zoverlay_dump,
// `U|<union>` from zunion_dump. Each answer is judged. Prints
// `DIFF n op: why` per disagreement and one line
//   ORACLE <label>: <agree> of <judged> agree, <disagree> disagree
// after its own controls, which must pass or nothing is judged.

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <geos_c.h>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef K::FT FT;
typedef K::Point_2 P;
typedef K::Segment_2 S;

static const double TOL = 1e-9;
static const double ZTOL = 1e-9;

struct V3 { double x, y, z; };
struct ZSeg { V3 a, b; };

static GEOSContextHandle_t ctx;

static void
nh(const char *fmt, ...)
{
  (void) fmt;
}

// Every coordinate sequence of a geometry, in order, with its Z
static void
sequences(const GEOSGeometry *g, std::vector<std::vector<V3>> &out)
{
  int type = GEOSGeomTypeId_r(ctx, g);
  if (type == GEOS_POLYGON)
  {
    std::vector<const GEOSGeometry *> rings;
    rings.push_back(GEOSGetExteriorRing_r(ctx, g));
    for (int i = 0; i < GEOSGetNumInteriorRings_r(ctx, g); i++)
      rings.push_back(GEOSGetInteriorRingN_r(ctx, g, i));
    for (const GEOSGeometry *r : rings)
      sequences(r, out);
    return;
  }
  if (type == GEOS_MULTIPOINT || type == GEOS_MULTILINESTRING ||
      type == GEOS_MULTIPOLYGON || type == GEOS_GEOMETRYCOLLECTION)
  {
    for (int i = 0; i < GEOSGetNumGeometries_r(ctx, g); i++)
      sequences(GEOSGetGeometryN_r(ctx, g, i), out);
    return;
  }
  if (GEOSisEmpty_r(ctx, g))
    return;
  const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq_r(ctx, g);
  unsigned int n;
  if (! cs || ! GEOSCoordSeq_getSize_r(ctx, cs, &n))
    return;
  std::vector<V3> seq;
  for (unsigned int i = 0; i < n; i++)
  {
    V3 v;
    GEOSCoordSeq_getX_r(ctx, cs, i, &v.x);
    GEOSCoordSeq_getY_r(ctx, cs, i, &v.y);
    if (! GEOSCoordSeq_getZ_r(ctx, cs, i, &v.z))
      v.z = NAN;
    seq.push_back(v);
  }
  out.push_back(seq);
}

// The values the Z-carrying inputs determine at one point of the plane
static void
determined(const V3 &v, const std::vector<ZSeg> &segs,
  const std::vector<V3> &points, double tol, std::vector<double> &values)
{
  P p(v.x, v.y);
  FT tol2 = FT(tol) * FT(tol);
  for (const V3 &q : points)
    if (CGAL::squared_distance(p, P(q.x, q.y)) <= tol2)
      values.push_back(q.z);
  for (const ZSeg &s : segs)
  {
    P a(s.a.x, s.a.y), b(s.b.x, s.b.y);
    if (a == b)
      continue;
    if (CGAL::squared_distance(p, S(a, b)) > tol2)
      continue;
    // The parameter of the point's projection onto the segment, exactly
    FT dx = b.x() - a.x(), dy = b.y() - a.y();
    FT t = ((p.x() - a.x()) * dx + (p.y() - a.y()) * dy) / (dx * dx + dy * dy);
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    values.push_back(s.a.z + CGAL::to_double(t) * (s.b.z - s.a.z));
  }
}

struct Inputs
{
  std::vector<ZSeg> segs;
  std::vector<V3> points;
  double scale = 1.0, zscale = 1.0;
};

static bool
read_inputs(const char *wkt, GEOSWKTReader *rd, Inputs &in)
{
  GEOSGeometry *g = GEOSWKTReader_read_r(ctx, rd, wkt);
  if (! g)
    return false;
  if (GEOSHasZ_r(ctx, g))
  {
    std::vector<std::vector<V3>> seqs;
    sequences(g, seqs);
    for (const auto &seq : seqs)
    {
      /* Every vertex determines its own Z, which a segment of no length in
       * the plane -- a vertical step -- does not otherwise report */
      for (const V3 &v : seq)
        in.points.push_back(v);
      for (size_t i = 0; i + 1 < seq.size(); i++)
        in.segs.push_back(ZSeg{seq[i], seq[i + 1]});
      for (const V3 &v : seq)
      {
        in.scale = std::fmax(in.scale, std::fmax(std::fabs(v.x), std::fabs(v.y)));
        in.zscale = std::fmax(in.zscale, std::fabs(v.z));
      }
    }
  }
  GEOSGeom_destroy_r(ctx, g);
  return true;
}

// Judge one answer against the rule; an empty reason is agreement
static std::string
judge(const std::string &answer, const Inputs &in, GEOSWKTReader *rd)
{
  GEOSGeometry *g = GEOSWKTReader_read_r(ctx, rd, answer.c_str());
  if (! g)
    return "the answer does not parse";
  bool hasz = GEOSHasZ_r(ctx, g) && ! GEOSisEmpty_r(ctx, g);
  std::vector<std::vector<V3>> seqs;
  sequences(g, seqs);
  GEOSGeom_destroy_r(ctx, g);
  bool alllifted = true;
  std::string why;
  double tol = TOL * in.scale, ztol = ZTOL * in.zscale;
  for (const auto &seq : seqs)
    for (const V3 &v : seq)
    {
      std::vector<double> values;
      determined(v, in.segs, in.points, tol, values);
      bool lifted = ! values.empty();
      double lo = 0.0, hi = 0.0;
      if (lifted)
      {
        lo = hi = values[0];
        for (double z : values) { lo = std::fmin(lo, z); hi = std::fmax(hi, z); }
        lifted = (hi - lo) <= ztol;
      }
      if (! lifted)
        alllifted = false;
      else if (hasz && std::fabs(v.z - lo) > ztol && why.empty())
      {
        char buf[160];
        snprintf(buf, sizeof(buf), "vertex (%.17g %.17g) carries %.17g, the "
          "inputs determine %.17g", v.x, v.y, v.z, lo);
        why = buf;
      }
    }
  if (! why.empty())
    return why;
  if (hasz && ! alllifted)
    return "the answer carries Z where the inputs determine none or several";
  if (! hasz && alllifted && ! seqs.empty() && (! in.segs.empty() ||
      ! in.points.empty()))
    return "the answer is planar where the inputs determine every ordinate";
  return "";
}

static bool
controls(GEOSWKTReader *rd)
{
  Inputs in;
  read_inputs("LINESTRING Z (0 0 1,4 0 5)", rd, in);
  read_inputs("POLYGON((1 -1,3 -1,3 1,1 1,1 -1))", rd, in);
  bool ok = judge("LINESTRING Z (1 0 2,3 0 4)", in, rd).empty() &&
    ! judge("LINESTRING Z (1 0 3,3 0 4)", in, rd).empty() &&
    ! judge("LINESTRING(1 0,3 0)", in, rd).empty();
  Inputs in2;
  read_inputs("LINESTRING Z (0 0 1,4 0 5)", rd, in2);
  read_inputs("LINESTRING Z (2 -2 7,2 2 7)", rd, in2);
  ok = ok && judge("POINT(2 0)", in2, rd).empty() &&
    ! judge("POINT Z (2 0 5)", in2, rd).empty();
  /* ONE line passing (5.5 2) twice, at 1.5 on its first vertex and at
   * 4.7142857 on its last segment: kept with its Z it is wrong, flat it is
   * right */
  Inputs in3;
  read_inputs("LINESTRING Z (5.5 2 1.5,3.5 7.5 5.5,4.75 2.75 7.5,6.5 1 1)", rd,
    in3);
  ok = ok && ! judge("LINESTRING Z (5.5 2 1.5,3.5 7.5 5.5,4.75 2.75 7.5,"
    "6.5 1 1)", in3, rd).empty() &&
    judge("LINESTRING(5.5 2,3.5 7.5,4.75 2.75,6.5 1)", in3, rd).empty();
  fprintf(stderr, "controls: %s\n", ok ? "pass" : "FAIL");
  return ok;
}

int
main(int argc, char **argv)
{
  if (argc != 4)
  {
    fprintf(stderr, "usage: %s <corpus> <answers> <label>\n", argv[0]);
    return 2;
  }
  ctx = GEOS_init_r();
  GEOSContext_setNoticeHandler_r(ctx, nh);
  GEOSContext_setErrorHandler_r(ctx, nh);
  GEOSWKTReader *rd = GEOSWKTReader_create_r(ctx);
  if (! controls(rd))
  {
    fprintf(stderr, "cgal_zlift: CONTROL FAILED, nothing judged\n");
    return 2;
  }
  FILE *fc = fopen(argv[1], "r"), *fa = fopen(argv[2], "r");
  if (! fc || ! fa)
  {
    fprintf(stderr, "cgal_zlift: cannot read the corpus or the answers\n");
    return 2;
  }
  static char in[1 << 20], out[1 << 20];
  int row = 0, judged = 0, agree = 0, disagree = 0;
  while (fgets(in, sizeof(in), fc))
  {
    char *nl = strchr(in, '\n');
    if (nl) *nl = '\0';
    if (! *in) continue;
    if (! fgets(out, sizeof(out), fa))
    {
      fprintf(stderr, "cgal_zlift: the answers run out at row %d\n", row + 1);
      return 2;
    }
    nl = strchr(out, '\n');
    if (nl) *nl = '\0';
    row++;
    char *bar = strchr(in, '|');
    if (! bar) continue;
    *bar = '\0';
    Inputs inputs;
    if (! read_inputs(in, rd, inputs) || ! read_inputs(bar + 1, rd, inputs))
      continue;
    // <tag>|<answer>|<tag>|<answer>...: one answer per operation, each named
    // by its tag (I intersection, D difference, U union)
    std::vector<std::string> tok;
    {
      std::string line(out);
      size_t start = 0;
      while (true)
      {
        size_t bar2 = line.find('|', start);
        tok.push_back(line.substr(start, bar2 == std::string::npos ?
          std::string::npos : bar2 - start));
        if (bar2 == std::string::npos) break;
        start = bar2 + 1;
      }
    }
    if (tok.size() < 2 || tok.size() % 2 != 0)
    {
      printf("DIFF %d: the answer line is not <tag>|<answer>...\n", row);
      disagree++; judged++;
      continue;
    }
    for (size_t k = 0; k + 1 < tok.size(); k += 2)
    {
      const std::string &op = tok[k], &ans = tok[k + 1];
      judged++;
      if (ans.empty() || ans[0] == '-')
      {
        disagree++;
        printf("DIFF %d %s: the head answers nothing\n", row, op.c_str());
        continue;
      }
      std::string why = judge(ans, inputs, rd);
      if (why.empty())
        agree++;
      else
      {
        disagree++;
        printf("DIFF %d %s: %s\n", row, op.c_str(), why.c_str());
      }
    }
  }
  printf("ORACLE %s: %d of %d agree, %d disagree\n", argv[3], agree, judged,
    disagree);
  GEOSWKTReader_destroy_r(ctx, rd);
  GEOS_finish_r(ctx);
  return 0;
}
