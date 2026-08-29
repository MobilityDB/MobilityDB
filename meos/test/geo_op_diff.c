/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Check a native spatial operation against the assertions of the GEOS
 * test suite
 * @details The corpus is produced by @p geos_harvest.py and holds one
 * @p wkt|arg|expected record per line, where @p expected is the result the
 * GEOS project asserts for the operation. The comparison criterion is the one
 * the reference project applies to that operation:
 * - a convex hull is an exact answer, so the result must equal the assertion;
 * - the GEOS suite asserts no oriented envelope, so that operation is checked
 *   against the answer of GEOS itself, exactly, both being rectangles that no
 *   approximation stands between;
 * - a buffer is an approximation, and this implementation answers it with
 *   exact circular arcs where GEOS polygonizes them, so the two results
 *   necessarily differ vertex by vertex. The criterion is therefore the one
 *   @p BufferResultMatcher of the GEOS test runner applies: the symmetric
 *   difference covers less than @p MAX_RELATIVE_AREA_DIFFERENCE of the
 *   expected area, and the oriented discrete Hausdorff distance between the
 *   two boundaries stays under a hundredth of the buffer distance. The arcs of
 *   the native answer are stroked before the comparison, since an area and a
 *   Hausdorff distance are defined on the polygonal representation.
 *
 * Usage:
 * @code
 * geo_op_diff convexhull        < convexhull_corpus_geos.txt
 * geo_op_diff buffer            < buffer_corpus_geos.txt
 * geo_op_diff orientedenvelope  < any corpus, whose first field is read
 * geo_op_diff issimple          < issimple_corpus_geos.txt
 * @endcode
 *
 * The program compares two engines, so unlike its siblings it does not build
 * against the installed MEOS alone: it reads the geometry through liblwgeom
 * and answers the reference through GEOS, and neither header nor symbol is
 * part of the MEOS library surface. It builds against the source and build
 * trees, where @p $SRC is a MobilityDB checkout and @p $BLD its build
 * directory
 * @code
 * gcc -Wall -g -DMEOS=1 -I$SRC/meos/include -isystem $SRC/pgtypes \
 *   -isystem $BLD/pgtypes -isystem $SRC/postgis -isystem $BLD/postgis \
 *   -isystem $SRC/postgis/liblwgeom -isystem $BLD/postgis/liblwgeom \
 *   -o geo_op_diff geo_op_diff.c -lmeos $BLD/postgis/libpostgis.a \
 *   -lgeos_c -lproj -ljson-c -lm
 * @endcode
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* GEOS */
#include <geos_c.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>

/* The tolerances of BufferResultMatcher of the GEOS test runner */
#define MAX_RELATIVE_AREA_DIFFERENCE 1.0e-3
#define MAX_HAUSDORFF_DISTANCE_FACTOR 100.0
#define MIN_DISTANCE_TOLERANCE 1.0e-8
/* Two answers that are both exact agree to a rounding artefact of the area */
#define EXACT_AREA_DIFFERENCE 1.0e-9
/* The densify fraction DiscreteHausdorffDistance is given there */
#define HAUSDORFF_DENSIFY_FRACTION 0.25
/* The number of segments per quadrant the exact arcs of the native answer are
 * sampled at before the comparison. It is fine enough for the sampling to
 * carry no error of its own, which leaves the assertion the only approximate
 * side and its polygonization the whole tolerance. Sampling the native answer
 * as coarsely as the assertion instead makes the comparison depend on the
 * radii each side carries: the buffer of a circular string is bounded by arcs
 * of the radius of that string, which the assertion never holds, and
 * polygonizing them at the same rate loses more area than the assertion loses
 * on its own joins. */
#define STROKE_SEGS_PER_QUAD 256
/* The quad_segs GEOS answers a buffer with, which is what the assertion
 * carries and therefore what the comparison tolerates */
#define ASSERTION_SEGS_PER_QUAD 8

static void
geos_notice(const char *fmt __attribute__((unused)), ...)
{
  return;
}

/**
 * @brief Return the GEOS geometry of a serialized geometry, with every
 * circular arc stroked
 */
static GEOSGeometry *
geos_stroked(const GSERIALIZED *gs)
{
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  LWGEOM *stroked = lwgeom_stroke(lwgeom, STROKE_SEGS_PER_QUAD);
  char *hex = lwgeom_to_hexwkb_buffer(stroked, WKB_ISO | WKB_NDR);
  GEOSGeometry *result = GEOSGeomFromHEX_buf((const unsigned char *) hex,
    strlen(hex));
  lwfree(hex);
  lwgeom_free(stroked);
  lwgeom_free(lwgeom);
  return result;
}

/**
 * @brief Return the fraction of the area the symmetric difference may cover
 * @details The assertion of the GEOS suite polygonizes every arc, while this
 * implementation answers it exactly, so the two boundaries are two different
 * polygonizations of the same circles once the exact answer is stroked. Both
 * are inscribed in those circles and therefore lie between the circle and the
 * regular polygon of @p STROKE_SEGS_PER_QUAD segments per quadrant inscribed
 * in it, which bounds their symmetric difference by the area that polygon
 * leaves out of the circle,
 * @f$1 - \frac{2q}{\pi}\sin\frac{\pi}{2q}@f$ for @p q segments per
 * quadrant. That bound is the criterion whenever it exceeds the one the GEOS
 * test runner applies between two polygonal results.
 * @note Only the assertion is polygonal here, the native answer being sampled
 * finely enough to carry no error of its own.
 */
static double
area_tolerance(void)
{
  double q = ASSERTION_SEGS_PER_QUAD;
  double deficit = 1.0 - (2.0 * q / M_PI) * sin(M_PI / (2.0 * q));
  /* The deficit is a fraction of the circle and the comparison is a fraction
   * of the polygon inscribed in it */
  deficit /= 1.0 - deficit;
  return deficit > MAX_RELATIVE_AREA_DIFFERENCE ?
    deficit : MAX_RELATIVE_AREA_DIFFERENCE;
}

/**
 * @brief Return the oriented envelope GEOS answers for a geometry
 */
static GSERIALIZED *
geos_oriented_envelope(const GSERIALIZED *gs)
{
  GEOSGeometry *input = geos_stroked(gs);
  if (! input)
    return NULL;
  GEOSGeometry *mrr = GEOSMinimumRotatedRectangle(input);
  GEOSGeom_destroy(input);
  if (! mrr)
    return NULL;
  size_t size;
  unsigned char *hex = GEOSGeomToHEX_buf(mrr, &size);
  GEOSGeom_destroy(mrr);
  if (! hex)
    return NULL;
  GSERIALIZED *result = geom_in((const char *) hex, -1);
  GEOSFree(hex);
  /* The round trip through GEOS drops the reference system */
  if (result)
  {
    GSERIALIZED *located = geo_set_srid(result, geo_srid(gs));
    free(result);
    result = located;
  }
  return result;
}

/**
 * @brief Return true if the symmetric difference of two geometries covers a
 * negligible fraction of the expected area
 */
static bool
symdiff_area_in_tolerance(const GEOSGeometry *actual,
  const GEOSGeometry *expected, char *reason, size_t size)
{
  double area, diff;
  if (! GEOSArea(expected, &area))
  {
    snprintf(reason, size, "the expected area is not available");
    return false;
  }
  GEOSGeometry *sym = GEOSSymDifference(actual, expected);
  if (! sym)
  {
    snprintf(reason, size, "the symmetric difference is not available");
    return false;
  }
  bool ok = GEOSArea(sym, &diff) != 0;
  GEOSGeom_destroy(sym);
  if (! ok)
  {
    snprintf(reason, size, "the symmetric difference area is not available");
    return false;
  }
  if (diff <= 0.0)
    return true;
  if (area <= 0.0)
  {
    snprintf(reason, size, "the expected area is %g", area);
    return false;
  }
  double tolerance = area_tolerance();
  if (diff / area < tolerance)
    return true;
  snprintf(reason, size, "the symmetric difference covers %g of the area, "
    "over the tolerated %g", diff / area, tolerance);
  return false;
}

/**
 * @brief Return how far apart the two boundaries may lie
 * @details The assertion polygonizes every arc of the answer and therefore
 * lies within one sagitta of it, @f$r(1 - \cos\frac{\pi}{2q})@f$ for an arc
 * of radius @p r and @p q segments per quadrant. The arcs bounding the buffer
 * of a curved geometry carry the radius of that geometry rather than the
 * buffer distance, so the larger of the two governs. The criterion of the GEOS
 * test runner, a hundredth of the buffer distance, applies whenever it is
 * larger still.
 */
static double
hausdorff_tolerance(double distance, double radius)
{
  double q = ASSERTION_SEGS_PER_QUAD;
  /* The assertion polygonizes every arc it holds, and the largest of them
   * governs how far it lies from the exact boundary */
  double largest = radius > fabs(distance) ? radius : fabs(distance);
  double sagitta = largest * (1.0 - cos(M_PI / (2.0 * q)));
  double geos = fabs(distance) / MAX_HAUSDORFF_DISTANCE_FACTOR;
  double result = sagitta > geos ? sagitta : geos;
  return result < MIN_DISTANCE_TOLERANCE ? MIN_DISTANCE_TOLERANCE : result;
}

/**
 * @brief Return the radius of the largest circular arc of a geometry
 */
static double
largest_arc_radius(const LWGEOM *geom)
{
  double result = 0.0;
  if (! geom)
    return 0.0;
  if (geom->type == CIRCSTRINGTYPE)
  {
    const POINTARRAY *pa = ((const LWCIRCSTRING *) geom)->points;
    for (uint32_t i = 0; i + 2 < pa->npoints; i += 2)
    {
      POINT4D a, b, c;
      getPoint4d_p(pa, i, &a);
      getPoint4d_p(pa, i + 1, &b);
      getPoint4d_p(pa, i + 2, &c);
      double d = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) +
        c.x * (a.y - b.y));
      if (fabs(d) < 1.0e-12)
        continue;
      double a2 = a.x * a.x + a.y * a.y, b2 = b.x * b.x + b.y * b.y;
      double c2 = c.x * c.x + c.y * c.y;
      double cx = (a2 * (b.y - c.y) + b2 * (c.y - a.y) + c2 * (a.y - b.y)) / d;
      double cy = (a2 * (c.x - b.x) + b2 * (a.x - c.x) + c2 * (b.x - a.x)) / d;
      double r = hypot(a.x - cx, a.y - cy);
      if (r > result)
        result = r;
    }
    return result;
  }
  if (geom->type == CURVEPOLYTYPE)
  {
    const LWCURVEPOLY *cp = (const LWCURVEPOLY *) geom;
    for (uint32_t i = 0; i < cp->nrings; i++)
    {
      double r = largest_arc_radius(cp->rings[i]);
      if (r > result)
        result = r;
    }
    return result;
  }
  if (lwgeom_is_collection(geom))
  {
    const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
    for (uint32_t i = 0; i < col->ngeoms; i++)
    {
      double r = largest_arc_radius(col->geoms[i]);
      if (r > result)
        result = r;
    }
  }
  return result;
}

/**
 * @brief Return true if the two boundaries stay within the polygonization of
 * each other
 */
static bool
boundary_hausdorff_in_tolerance(const GEOSGeometry *actual,
  const GEOSGeometry *expected, double distance, double radius, char *reason,
  size_t size)
{
  GEOSGeometry *b1 = GEOSBoundary(actual);
  GEOSGeometry *b2 = GEOSBoundary(expected);
  if (! b1 || ! b2)
  {
    if (b1) GEOSGeom_destroy(b1);
    if (b2) GEOSGeom_destroy(b2);
    snprintf(reason, size, "a boundary is not available");
    return false;
  }
  double found;
  int ok = GEOSHausdorffDistanceDensify(b1, b2, HAUSDORFF_DENSIFY_FRACTION,
    &found);
  GEOSGeom_destroy(b1); GEOSGeom_destroy(b2);
  if (! ok)
  {
    snprintf(reason, size, "the Hausdorff distance is not available");
    return false;
  }
  double tolerance = hausdorff_tolerance(distance, radius);
  if (found <= tolerance)
    return true;
  snprintf(reason, size, "the boundaries are %g apart, over the tolerated %g",
    found, tolerance);
  return false;
}

/**
 * @brief Return true if every coordinate of a geometry is a finite number
 * @details At coordinates near the range of a double the rotation GEOS applies
 * to place a rectangle overflows, and it answers a geometry carrying NaN. Such
 * an answer asserts nothing, so a record it comes from is reported rather than
 * counted against the native implementation.
 */
static bool
geo_is_finite(const GSERIALIZED *gs)
{
  GEOSGeometry *g = geos_stroked(gs);
  if (! g)
    return false;
  double xmin, xmax, ymin, ymax, area;
  /* The extent is read as well as the area, since the extent of a ring
   * carrying a NaN vertex is still the extent of the vertices that are not */
  bool result = GEOSGeom_getXMin(g, &xmin) && GEOSGeom_getXMax(g, &xmax) &&
    GEOSGeom_getYMin(g, &ymin) && GEOSGeom_getYMax(g, &ymax) &&
    GEOSArea(g, &area) &&
    isfinite(xmin) && isfinite(xmax) && isfinite(ymin) && isfinite(ymax) &&
    isfinite(area);
  GEOSGeom_destroy(g);
  return result;
}

/**
 * @brief Return true if a rectangle answers the oriented envelope of a geometry
 * @details The operation asks for a rectangle of minimum area enclosing the
 * geometry, and that rectangle is not unique: whenever the convex hull has two
 * supporting directions of equal width the minimum is attained twice, and the
 * two answers cover different regions. Asserting the region GEOS happens to
 * return would test which of the tied rectangles an implementation reaches,
 * which no definition fixes. The two properties the definition does fix are
 * asserted instead: the answer encloses the geometry, and its area does not
 * exceed the area of the rectangle GEOS returns.
 */
static bool
envelope_matches(const GSERIALIZED *actual, const GSERIALIZED *expected,
  const GSERIALIZED *input, char *reason, size_t size)
{
  bool e1 = geo_is_empty(actual), e2 = geo_is_empty(expected);
  if (e1 && e2)
    return true;
  if (e1 || e2)
  {
    snprintf(reason, size, "one result is empty and the other is not");
    return false;
  }
  /* The two agree outright, which a degenerate rectangle answers as a line or
   * a point and which no area comparison is defined on */
  if (geo_equals(actual, expected) == 1)
    return true;
  GEOSGeometry *g1 = geos_stroked(actual);
  GEOSGeometry *g2 = geos_stroked(expected);
  GEOSGeometry *in = geos_stroked(input);
  if (! g1 || ! g2 || ! in)
  {
    snprintf(reason, size, "a result does not convert to a polygon");
    if (g1) GEOSGeom_destroy(g1);
    if (g2) GEOSGeom_destroy(g2);
    if (in) GEOSGeom_destroy(in);
    return false;
  }
  double a1, a2;
  bool result = false;
  if (! GEOSArea(g1, &a1) || ! GEOSArea(g2, &a2))
    snprintf(reason, size, "the area of a result is not available");
  else
  {
    /* The enclosure is asked for at the scale the coordinates are written at,
     * since the rectangle is placed by arithmetic on those coordinates */
    double scale = sqrt(a2 > 0.0 ? a2 : 1.0);
    GEOSGeometry *grown = GEOSBuffer(g1, scale * EXACT_AREA_DIFFERENCE, 8);
    if (! grown)
      snprintf(reason, size, "the answer does not admit a tolerance");
    else if (GEOSCovers(grown, in) != 1)
      snprintf(reason, size, "the answer does not enclose the geometry");
    else if (a1 > a2 * (1.0 + EXACT_AREA_DIFFERENCE))
      snprintf(reason, size, "the answer covers %g of the area GEOS answers, "
        "so it is not of minimum area", a2 > 0.0 ? a1 / a2 : a1);
    else
      result = true;
    if (grown)
      GEOSGeom_destroy(grown);
  }
  GEOSGeom_destroy(g1); GEOSGeom_destroy(g2); GEOSGeom_destroy(in);
  return result;
}

/**
 * @brief Return true if a buffer matches the one the GEOS suite asserts
 */
static bool
buffer_matches(const GSERIALIZED *actual, const GSERIALIZED *expected,
  double distance, char *reason, size_t size)
{
  bool e1 = geo_is_empty(actual), e2 = geo_is_empty(expected);
  if (e1 && e2)
    return true;
  if (e1 || e2)
  {
    snprintf(reason, size, "one result is empty and the other is not");
    return false;
  }
  GEOSGeometry *g1 = geos_stroked(actual);
  GEOSGeometry *g2 = geos_stroked(expected);
  if (! g1 || ! g2)
  {
    snprintf(reason, size, "the %s result does not convert to a polygon",
      g1 ? "expected" : "actual");
    if (g1) GEOSGeom_destroy(g1);
    if (g2) GEOSGeom_destroy(g2);
    return false;
  }
  LWGEOM *lwactual = lwgeom_from_gserialized(actual);
  double radius = largest_arc_radius(lwactual);
  lwgeom_free(lwactual);
  bool result = symdiff_area_in_tolerance(g1, g2, reason, size) &&
    boundary_hausdorff_in_tolerance(g1, g2, distance, radius, reason, size);
  GEOSGeom_destroy(g1); GEOSGeom_destroy(g2);
  return result;
}

int
main(int argc, char **argv)
{
  if (argc != 2 || (strcmp(argv[1], "convexhull") && strcmp(argv[1], "buffer")
      && strcmp(argv[1], "orientedenvelope") && strcmp(argv[1], "issimple")))
  {
    fprintf(stderr,
      "usage: %s {convexhull|buffer|orientedenvelope|issimple} < corpus\n",
      argv[0]);
    return 2;
  }
  bool is_buffer = ! strcmp(argv[1], "buffer");
  bool is_envelope = ! strcmp(argv[1], "orientedenvelope");
  bool is_simple = ! strcmp(argv[1], "issimple");
  meos_initialize();
  /* A geometry the native implementation declines raises an error, and the
   * corpus is walked to its end rather than stopped on the first one */
  meos_initialize_noexit_error_handler();
  initGEOS(geos_notice, geos_notice);

  char *line = NULL;
  size_t cap = 0;
  ssize_t len;
  int nchecked = 0, nfailed = 0, nunsupported = 0, nnoreference = 0;
  while ((len = getline(&line, &cap, stdin)) > 0)
  {
    if (line[0] == '#' || line[0] == '\n')
      continue;
    char *nl = strpbrk(line, "\r\n");
    if (nl)
      *nl = '\0';
    char *arg = strchr(line, '|');
    char *expected_wkt = NULL;
    if (arg)
    {
      *arg++ = '\0';
      expected_wkt = strchr(arg, '|');
      if (expected_wkt)
        *expected_wkt++ = '\0';
    }
    if (! is_envelope && (! arg || ! expected_wkt))
      continue;

    /* The assertion of this operation is a truth value, not a geometry */
    if (is_simple)
    {
      meos_errno_reset();
      GSERIALIZED *gs1 = geom_in(line, -1);
      if (! gs1)
      {
        fprintf(stderr, "PARSE %.90s\n", line);
        meos_errno_reset();
        continue;
      }
      bool want = ! strcmp(expected_wkt, "true");
      meos_errno_reset();
      bool got = geom_is_simple(gs1);
      if (meos_errno())
      {
        meos_errno_reset();
        nunsupported++;
        printf("UNSUPPORTED %.90s\n", line);
      }
      else
      {
        nchecked++;
        if (got != want)
        {
          nfailed++;
          printf("FAIL   in       %.100s\n", line);
          printf("       expected %s, actual %s\n", want ? "true" : "false",
            got ? "true" : "false");
        }
      }
      free(gs1);
      continue;
    }

    GSERIALIZED *gs = geom_in(line, -1);
    /* The GEOS suite asserts no oriented envelope, so GEOS answers it here */
    GSERIALIZED *expected = is_envelope ? geos_oriented_envelope(gs) :
      geom_in(expected_wkt, -1);
    if (! gs || ! expected)
    {
      fprintf(stderr, "PARSE %.90s\n", line);
      free(gs); free(expected);
      continue;
    }
    if (is_envelope && ! geo_is_finite(expected))
    {
      nnoreference++;
      printf("NOREFERENCE %.90s\n", line);
      free(gs); free(expected);
      continue;
    }
    double distance = is_buffer ? atof(arg) : 0.0;
    GSERIALIZED *actual = is_buffer ? geom_buffer(gs, distance, "") :
      (is_envelope ? geom_oriented_envelope(gs) : geom_convex_hull(gs));
    if (! actual)
    {
      nunsupported++;
      printf("UNSUPPORTED %.90s\n", line);
      free(gs); free(expected);
      continue;
    }
    nchecked++;
    char reason[256] = "the result differs from the assertion";
    bool ok = is_buffer ?
      buffer_matches(actual, expected, distance, reason, sizeof(reason)) :
      (is_envelope ?
        envelope_matches(actual, expected, gs, reason, sizeof(reason)) :
        geo_equals(actual, expected) == 1);
    if (! ok)
    {
      nfailed++;
      char *wa = geo_as_ewkt(actual, 8), *we = geo_as_ewkt(expected, 8);
      printf("FAIL   in       %.100s\n", line);
      if (is_buffer)
        printf("       distance %g: %s\n", distance, reason);
      else if (is_envelope)
        printf("       %s\n", reason);
      printf("       expected %.100s\n", we);
      printf("       actual   %.100s\n", wa);
      free(wa); free(we);
    }
    free(actual); free(gs); free(expected);
  }
  free(line);
  if (nnoreference)
    printf("%d records carry no reference, GEOS overflowing on their coordinates\n",
      nnoreference);
  printf("%d of %d checked records pass, %d unsupported\n",
    nchecked - nfailed, nchecked, nunsupported);
  finishGEOS();
  meos_finalize();
  return (nfailed || nunsupported) ? 1 : 0;
}
