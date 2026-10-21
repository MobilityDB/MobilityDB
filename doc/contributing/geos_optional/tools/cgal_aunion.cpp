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

/*
 * cgal_aunion -- judge the union of an array of surfaces by the point set it
 * covers, with CGAL's exact kernel.
 *
 *   usage: cgal_aunion <corpus> <answers> <label>
 *
 * The corpus carries one array per row, its members separated by ';', and
 * <answers> the head's union of each row, one WKT per line, or a line starting
 * with DECLINED. Members and answers are any polygonal WKT: POLYGON, TRIANGLE,
 * MULTIPOLYGON, TIN, POLYHEDRALSURFACE, with or without Z or M, whose rings are
 * straight. Every innermost parenthesised group is a ring and its parent group
 * the polygon it bounds, first ring the shell; ordinates beyond X and Y are
 * read and ignored, since the union is a figure of the plane.
 *
 * A row agrees when BOTH hold:
 *  (1) SAME POINT SET: the exact area of (members XOR answer) is zero, up to
 *      the rounding of the crossings the answer had to compute;
 *  (2) DISSOLVED: the faces of the answer do not overlap, i.e. the sum of their
 *      own areas equals the area of the point set they cover.
 * A region is built by inserting its shell and subtracting each hole, so no
 * operand is ever a polygon-with-holes CGAL must validate.
 * A DECLINED row is a disagreement: a change that turns answers into declines
 * fails this witness. Prints:  ORACLE <label>: <agree> of <total> agree, <d> disagree
 */
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <list>
#include <string>
#include <vector>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Polygon_set_2.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef K::FT FT;
typedef CGAL::Polygon_2<K> Poly;
typedef CGAL::Polygon_with_holes_2<K> PolyH;
typedef CGAL::Polygon_set_2<K> PSet;

/* The rounding of a computed crossing moves the area by the last bits of a
 * coordinate times an edge length; a lost or doubled face moves it by a face */
static const double REL_TOLERANCE = 1e-9;
static double worst_rel = 0.0;

typedef std::vector<std::vector<std::pair<double, double>>> Rings;

static int
dims_of(const std::string &w)
{
  /* The head names the dimensions as a word of its own after the type, or as
   * a suffix glued to it: POLYGON Z (, POLYGONZM(, TIN M ( */
  std::string head = w.substr(0, w.find('('));
  while (! head.empty() && head.back() == ' ')
    head.pop_back();
  size_t sp = head.find_last_of(' ');
  std::string word = sp == std::string::npos ? head : head.substr(sp + 1);
  auto ends = [&](const char *s) {
    size_t n = strlen(s);
    return word.size() >= n && word.compare(word.size() - n, n, s) == 0;
  };
  if (ends("ZM"))
    return 4;
  if (word == "Z" || word == "M")
    return 3;
  /* a glued suffix: the type names never end in Z, and of those ending in M
   * none is a polygonal type this reads */
  if (ends("Z") || (ends("M") && sp == std::string::npos && word != "POLYGON"))
    return 3;
  return 2;
}

/* Each polygon as its list of rings, shell first */
static bool
parse_polys(const std::string &w, std::vector<Rings> &polys)
{
  int dims = dims_of(w);
  int depth = 0;
  std::vector<int> ringdepth;
  Rings current;
  std::vector<std::pair<double, double>> ring;
  bool inring = false;
  for (size_t i = 0; i < w.size(); i++)
  {
    char c = w[i];
    if (c == '(')
    {
      depth++;
      ring.clear();
      inring = true;
    }
    else if (c == ')')
    {
      if (inring && ! ring.empty())
      {
        current.push_back(ring);
        ringdepth.push_back(depth);
      }
      else if (! current.empty() && depth == ringdepth.back() - 1)
      {
        polys.push_back(current);
        current.clear();
        ringdepth.clear();
      }
      inring = false;
      ring.clear();
      depth--;
    }
    else if (inring && (isdigit(c) || c == '-' || c == '.'))
    {
      double v[4];
      char *end = (char *) w.c_str() + i;
      for (int d = 0; d < dims; d++)
      {
        v[d] = strtod(end, &end);
        while (*end == ' ')
          end++;
      }
      ring.emplace_back(v[0], v[1]);
      i = (size_t) (end - w.c_str());
      if (*end == ',')
        continue;
      i--;
    }
  }
  return true;
}

static Poly
as_poly(const std::vector<std::pair<double, double>> &r)
{
  Poly p;
  size_t n = r.size();
  if (n > 1 && r.front() == r.back())
    n--;
  for (size_t i = 0; i < n; i++)
    p.push_back(K::Point_2(r[i].first, r[i].second));
  /* orientation() presumes a simple ring, so a ring that is not one is
   * returned as written and refused by the caller's is_simple() test */
  if (p.size() >= 3 && p.is_simple() && p.orientation() == CGAL::CLOCKWISE)
    p.reverse_orientation();
  return p;
}

/* Rings that touch themselves at a vertex, counted apart from the verdict:
 * the point set they bound is read, their spelling is reported */
static long touching_rings = 0;
/* Answers holding at least one such ring */
static long touching_answers = 0;

/* A ring touching itself at a vertex as the simple loops it is made of: a
 * vertex met again closes the loop walked since its first visit */
static std::vector<std::vector<std::pair<double, double>>>
ring_loops(const std::vector<std::pair<double, double>> &r)
{
  std::vector<std::vector<std::pair<double, double>>> loops;
  std::vector<std::pair<double, double>> stack;
  size_t n = r.size();
  if (n > 1 && r.front() == r.back())
    n--;
  for (size_t i = 0; i < n; i++)
  {
    size_t k = 0;
    while (k < stack.size() && stack[k] != r[i])
      k++;
    if (k < stack.size())
    {
      loops.emplace_back(stack.begin() + (long) k, stack.end());
      stack.resize(k);
    }
    stack.push_back(r[i]);
  }
  if (! stack.empty())
    loops.push_back(stack);
  return loops;
}

/* The region a ring bounds, built by set operations on simple polygons, which
 * CGAL always accepts: a simple ring as itself, a ring
 * touching itself at a vertex as the XOR of its simple loops, which is the
 * region its boundary encloses. False where a loop crosses itself */
static bool
ring_region(const std::vector<std::pair<double, double>> &r, PSet &out)
{
  Poly p = as_poly(r);
  if (p.size() >= 3 && p.is_simple())
  {
    out.insert(p);
    return true;
  }
  std::vector<std::vector<std::pair<double, double>>> loops = ring_loops(r);
  if (loops.size() < 2)
    return false;
  for (const auto &l : loops)
  {
    Poly q = as_poly(l);
    if (q.size() < 3)
      continue;
    if (! q.is_simple())
      return false;
    if (q.area() != 0)
      out.symmetric_difference(q);
  }
  touching_rings++;
  return true;
}

/* The region one polygon covers, or false where a ring crosses itself */
static bool
poly_region(const Rings &rings, PSet &out)
{
  Poly shell = as_poly(rings[0]);
  if (shell.size() < 3)
    return false;
  if (! shell.is_simple())
  {
    PSet s;
    if (! ring_region(rings[0], s))
      return false;
    out.join(s);
    for (size_t i = 1; i < rings.size(); i++)
    {
      PSet h;
      if (! ring_region(rings[i], h))
        return false;
      out.difference(h);
    }
    return true;
  }
  if (shell.area() == 0)
    return true;           /* a face of no area covers no area */
  out.insert(shell);
  for (size_t i = 1; i < rings.size(); i++)
  {
    Poly hole = as_poly(rings[i]);
    if (hole.size() < 3)
      return false;
    if (! hole.is_simple())
    {
      PSet h;
      if (! ring_region(rings[i], h))
        return false;
      out.difference(h);
    }
    else if (hole.area() != 0)
      out.difference(hole);
  }
  return true;
}

static FT
set_area(const PSet &s)
{
  std::list<PolyH> pwhs;
  s.polygons_with_holes(std::back_inserter(pwhs));
  FT a = 0;
  for (const PolyH &p : pwhs)
  {
    a += p.outer_boundary().area();
    for (auto h = p.holes_begin(); h != p.holes_end(); ++h)
      a += h->area();            /* a hole is clockwise, its area negative */
  }
  return a;
}

/* -1: unreadable, 0: disagree, 1: agree */
static int
judge_row(const std::string &members, const std::string &answer, std::string &why)
{
  PSet in;
  size_t start = 0;
  while (start <= members.size())
  {
    size_t semi = members.find(';', start);
    std::string m = members.substr(start, semi == std::string::npos ?
      std::string::npos : semi - start);
    std::vector<Rings> polys;
    parse_polys(m, polys);
    for (const Rings &r : polys)
    {
      PSet one;
      if (! poly_region(r, one))
      {
        why = "member ring not simple";
        return -1;
      }
      in.join(one);
    }
    if (semi == std::string::npos)
      break;
    start = semi + 1;
  }
  long touching_before = touching_rings;
  std::vector<Rings> apolys;
  parse_polys(answer, apolys);
  PSet out;
  FT facesum = 0;
  for (const Rings &r : apolys)
  {
    PSet one;
    if (! poly_region(r, one))
    {
      why = "answer ring not simple";
      return 0;
    }
    facesum += set_area(one);
    out.join(one);
  }
  if (touching_rings > touching_before)
    touching_answers++;
  FT ain = set_area(in);
  PSet x = in;
  x.symmetric_difference(out);
  double sym = CGAL::to_double(set_area(x));
  double base = std::max(CGAL::to_double(ain), 1e-300);
  double r1 = sym / base;
  double r2 = std::fabs(CGAL::to_double(facesum - set_area(out))) / base;
  worst_rel = std::max(worst_rel, std::max(r1, r2));
  if (r1 > REL_TOLERANCE)
  {
    why = "covers a different point set, |xor|/|in| = " + std::to_string(r1);
    return 0;
  }
  if (r2 > REL_TOLERANCE)
  {
    why = "faces overlap, (sum - |answer|)/|in| = " + std::to_string(r2);
    return 0;
  }
  return 1;
}

static int
controls(void)
{
  std::string why;
  int bad = 0;
  /* overlapping squares dissolved into one polygon */
  bad += judge_row("POLYGON((0 0,2 0,2 2,0 2,0 0));POLYGON((1 0,3 0,3 2,1 2,1 0))",
    "POLYGON((0 0,3 0,3 2,0 2,0 0))", why) != 1;
  /* the same answer left undissolved must fail (2) */
  bad += judge_row("POLYGON((0 0,2 0,2 2,0 2,0 0));POLYGON((1 0,3 0,3 2,1 2,1 0))",
    "MULTIPOLYGON(((0 0,2 0,2 2,0 2,0 0)),((1 0,3 0,3 2,1 2,1 0)))", why) != 0;
  /* a lost member must fail (1) */
  bad += judge_row("POLYGON((0 0,1 0,1 1,0 1,0 0));MULTIPOLYGON(((5 5,6 5,6 6,5 6,5 5)))",
    "POLYGON((0 0,1 0,1 1,0 1,0 0))", why) != 0;
  /* a hole is subtracted, a TIN read face by face */
  bad += judge_row("POLYGON((0 0,4 0,4 4,0 4,0 0),(1 1,1 3,3 3,3 1,1 1));"
    "TIN(((1 1,3 1,3 3,1 1)),((1 1,3 3,1 3,1 1)))",
    "POLYGON((0 0,4 0,4 4,0 4,0 0))", why) != 1;
  /* a hole touching its shell at a corner, spelled as one ring touching
   * itself there, is the region of the valid spelling */
  long before = touching_rings;
  bad += judge_row("POLYGON((0 0,4 0,4 4,0 4,0 0),(0 0,1 2,2 1,0 0))",
    "POLYGON((0 0,4 0,4 4,0 4,0 0,2 1,1 2,0 0))", why) != 1;
  bad += touching_rings != before + 1;
  touching_rings = 0;
  touching_answers = 0;
  worst_rel = 0.0;
  return bad;
}

int
main(int argc, char **argv)
{
  if (argc != 4)
  {
    fprintf(stderr, "usage: %s <corpus> <answers> <label>\n", argv[0]);
    return 2;
  }
  int bad = controls();
  fprintf(stderr, "controls: %d of 6 wrong\n", bad);
  if (bad)
  {
    fprintf(stderr, "cgal_aunion: CONTROL FAILED, nothing judged\n");
    return 2;
  }
  std::ifstream fc(argv[1]), fa(argv[2]);
  std::string mem, ans;
  long agree = 0, total = 0, disagree = 0, row = 0;
  while (std::getline(fc, mem))
  {
    if (mem.empty() || mem[0] == '#')
      continue;
    row++;
    if (! std::getline(fa, ans))
    {
      fprintf(stderr, "answers end before row %ld\n", row);
      return 2;
    }
    total++;
    if (ans.rfind("DECLINED", 0) == 0)
    {
      disagree++;
      fprintf(stderr, "row %ld DECLINED: %s\n", row, mem.c_str());
      continue;
    }
    if (ans.rfind("UNREADABLE", 0) == 0)
    {
      /* a member the head cannot read is a corpus fault, not an answer */
      fprintf(stderr, "row %ld UNREADABLE member: %s\n", row, mem.c_str());
      return 2;
    }
    std::string why;
    int v;
    try
    {
      v = judge_row(mem, ans, why);
    }
    catch (const std::exception &e)
    {
      fprintf(stderr, "row %ld JUDGE CANNOT READ (%s)\n  members %s\n  answer  %s\n",
        row, e.what(), mem.c_str(), ans.c_str());
      return 2;
    }
    if (v == 1)
      agree++;
    else
    {
      disagree++;
      fprintf(stderr, "row %ld %s: %s\n", row, why.c_str(), mem.c_str());
    }
  }
  fprintf(stderr, "rings touching themselves at a vertex, read as their loops: %ld,"
    " in %ld answers\n", touching_rings, touching_answers);
  fprintf(stderr, "worst relative gap seen: %.3g (tolerance %.0e)\n", worst_rel,
    REL_TOLERANCE);
  printf("ORACLE %s: %ld of %ld agree, %ld disagree\n", argv[3], agree, total,
    disagree);
  return 0;
}
