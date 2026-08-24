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
 * @brief A program that tests the WKT input functions of the geo API on
 * malformed input, and the text representation of the PostGIS @p box3d type.
 *
 * A parse failure is only observable once
 * #meos_initialize_noexit_error_handler is installed — the handler every
 * language binding uses, since a binding must return an exception to its host
 * rather than terminate it. Under it the parse-failure branch of the input
 * functions is reached, and the program verifies that they return NULL and set
 * #meos_errno rather than dereferencing the null geometry the parser leaves
 * behind. The default handler ends the process inside the failing check, so
 * this path is exercised only here.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o geo_test geo_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* A malformed WKT geography returns NULL and sets the error status, rather
   * than dereferencing the null geometry the parser leaves in its result */
  meos_errno_reset();
  GSERIALIZED *bad_geog = geog_in("this is not well-known text", -1);
  printf("geog_in(bad WKT): %s, errno %d\n", bad_geog ? "non-NULL" : "NULL",
    meos_errno());
  assert(bad_geog == NULL);
  assert(meos_errno() != 0);

  /* Same for the geometry/geography from-text constructor */
  meos_errno_reset();
  GSERIALIZED *bad_geo = geo_from_text("this is not well-known text", 0);
  printf("geo_from_text(bad WKT): %s, errno %d\n",
    bad_geo ? "non-NULL" : "NULL", meos_errno());
  assert(bad_geo == NULL);
  assert(meos_errno() != 0);

  /* A valid WKT geography still parses */
  meos_errno_reset();
  GSERIALIZED *good = geog_in("Point(1 1)", -1);
  printf("geog_in(\"Point(1 1)\"): %s, errno %d\n",
    good ? "non-NULL" : "NULL", meos_errno());
  assert(good != NULL);
  assert(meos_errno() == 0);
  free(good);

  /* The text representation of a box3d is the one PostGIS gives the type it
   * declares, so that the value crosses between a PostgreSQL session and any
   * other binding. PostGIS prints the ordinates with at most 15 decimal
   * digits, the value of maxdd that reproduces its output */
  const char *box3d_strs[][2] = {
    /* Three dimensions, the form a PostgreSQL session prints for
     * stbox 'STBOX Z((1,1,1),(5,5,5))'::box3d */
    {"BOX3D(1 1 1,5 5 5)", "BOX3D(1 1 1,5 5 5)"},
    /* The Z ordinates may be left out on input and are then zero, and they
     * are printed back, as PostGIS does */
    {"BOX3D(1 2,3 4)", "BOX3D(1 2 0,3 4 0)"},
    /* Negative and fractional ordinates */
    {"BOX3D(-1.5 -2.25 -3.125,5 5 5)", "BOX3D(-1.5 -2.25 -3.125,5 5 5)"},
    /* The bounds are ordered on input */
    {"BOX3D(5 5 5,1 1 1)", "BOX3D(1 1 1,5 5 5)"},
    /* The format written by previous versions is still read, and the SRID it
     * carries is not printed back, as PostGIS never prints one */
    {"BOX3D((1,2,3),(4,5,6))", "BOX3D(1 2 3,4 5 6)"},
    {"SRID=4326;BOX3D((1,2,3),(4,5,6))", "BOX3D(1 2 3,4 5 6)"}
  };
  for (size_t i = 0; i < sizeof(box3d_strs) / sizeof(box3d_strs[0]); i++)
  {
    meos_errno_reset();
    BOX3D *box = box3d_in(box3d_strs[i][0]);
    assert(box != NULL);
    assert(meos_errno() == 0);
    char *str = box3d_out(box, 15);
    printf("box3d_in(\"%s\") -> \"%s\"\n", box3d_strs[i][0], str);
    assert(strcmp(str, box3d_strs[i][1]) == 0);
    /* The output of box3d_out is read back by box3d_in */
    BOX3D *box1 = box3d_in(str);
    assert(box1 != NULL);
    char *str1 = box3d_out(box1, 15);
    assert(strcmp(str, str1) == 0);
    free(str); free(str1); free(box); free(box1);
  }

  /* A box3d whose SRID is set still prints without one, as PostGIS does */
  meos_errno_reset();
  BOX3D *box_srid = box3d_make(1, 5, 1, 5, 1, 5, 4326);
  assert(box_srid != NULL);
  char *str_srid = box3d_out(box_srid, 15);
  printf("box3d_out(box3d_make(..., 4326)): \"%s\"\n", str_srid);
  assert(strcmp(str_srid, "BOX3D(1 1 1,5 5 5)") == 0);
  free(str_srid); free(box_srid);

  /* A pattern is matched against the native matrix, which relates a collection
   * of mixed dimensions like any other geometry, so the answer carries no
   * error status. A line far from the collection does not meet it, and a line
   * through it meets it in their interiors */
  GSERIALIZED *coll = geom_in(
    "GeometryCollection(Point(0 0),Linestring(2 2,3 3))", -1);
  assert(coll != NULL);
  GSERIALIZED *away = geom_in("Linestring(10 10,20 20)", -1);
  assert(away != NULL);
  char patt[10] = "T********";
  meos_errno_reset();
  bool rel = geom_relate_pattern(away, coll, patt);
  int rel_errno = meos_errno();
  printf("geom_relate_pattern(away, collection): %d, errno %d\n", rel,
    rel_errno);
  assert(rel == false);
  assert(rel_errno == 0);
  GSERIALIZED *through = geom_in("Linestring(0 0,3 3)", -1);
  assert(through != NULL);
  meos_errno_reset();
  bool rel_ok = geom_relate_pattern(through, coll, patt);
  int errno_ok = meos_errno();
  printf("geom_relate_pattern(through, collection): %d, errno %d\n", rel_ok,
    errno_ok);
  assert(rel_ok == true);
  assert(errno_ok == 0);
  /* A TIN is outside the edge decomposition and GEOS carries one, so a build
   * including GEOS answers it and a build excluding GEOS reports the failure
   * and answers false, the value a language binding reads safely */
  GSERIALIZED *tin = geom_in(
    "Tin(((0 0,0 1,1 1,0 0)),((0 0,1 0,1 1,0 0)))", -1);
  assert(tin != NULL);
  meos_errno_reset();
  bool rel_tin = geom_relate_pattern(through, tin, patt);
  int errno_tin = meos_errno();
  printf("geom_relate_pattern(line, tin): %d, errno %d\n", rel_tin,
    errno_tin);
  /* The option is read from the library at run time: this file is compiled
   * against the installed headers, which carry no definition of it, so a
   * preprocessor test here would describe the test's own build and not the
   * library's */
  if (strstr(meos_full_version(), "GEOS none") == NULL)
    assert(errno_tin == 0);
  else
  {
    assert(rel_tin == false);
    assert(errno_tin != 0);
  }
  /* A polyhedral surface is outside the edge decomposition AND outside GEOS,
   * whose LWGEOM2GEOS reaches an lwerror that ends the process rather than
   * returning, so it is reported the same way in either build and reaches
   * GEOS in neither */
  GSERIALIZED *phs = geom_in(
    "PolyhedralSurface Z (((0 0 0,0 1 0,1 1 0,1 0 0,0 0 0)),"
    "((0 0 0,0 1 0,0 1 1,0 0 1,0 0 0)))", -1);
  assert(phs != NULL);
  meos_errno_reset();
  bool rel_phs = geom_relate_pattern(through, phs, patt);
  int errno_phs = meos_errno();
  printf("geom_relate_pattern(line, polyhedral surface): %d, errno %d\n",
    rel_phs, errno_phs);
  assert(rel_phs == false);
  assert(errno_phs != 0);
  free(coll); free(away); free(through); free(tin); free(phs);
  meos_errno_reset();

  /* A geometry covers itself, whatever its coordinates are and however short
   * its edges. Both records below answered otherwise: the first is a five
   * metre feature in a projected CRS, where the engine split its own edges at
   * points that are not there and answered 2F2F11212; the second is the
   * boundary of an H3 cell, whose edges are a few hundredths of a degree, where
   * the interior went missing and it answered FFFF1FFF2 -- a matrix under which
   * a cell touches itself and contains nothing. Neither is exotic: the two
   * cover the ranges every projected dataset and every cell index live in */
  const char *self_wkt[] = {
    "POLYGON((509005 6617000,509002.5 6617004.3301270185,"
      "508997.5 6617004.3301270185,508995 6617000,"
      "508997.5 6616995.6698729815,509002.5 6616995.6698729815,"
      "509005 6617000))",
    "POLYGON((110.56363551525037 -23.502418506349507,"
      "110.56870217314606 -23.532108271018778,"
      "110.5941357885684 -23.54380157929298,"
      "110.61448404730675 -23.525811891613976,"
      "110.60940975095393 -23.49613923666697,"
      "110.58399482710622 -23.48443916400023,"
      "110.56363551525037 -23.502418506349507))"
  };
  char covers[10] = "T*****FF*";
  for (int i = 0; i < 2; i++)
  {
    GSERIALIZED *self = geom_in(self_wkt[i], -1);
    assert(self != NULL);
    meos_errno_reset();
    bool self_covers = geom_relate_pattern(self, self, covers);
    printf("geom_relate_pattern(self %d, self %d, covers): %d, errno %d\n",
      i, i, self_covers, meos_errno());
    assert(self_covers == true);
    assert(meos_errno() == 0);
    free(self);
  }

  /* A boundary node that two pieces of a buffer meet at is computed twice,
   * once by each of them, and on projected coordinates the two results sit
   * apart by far more than a coordinate is stored to. Reading them as two
   * nodes leaves a ring whose pieces are all present unable to close, and the
   * buffer of a geometry carrying a hole is refused. The witness is one real
   * protected area of thirteen vertices and one hole, at coordinates near 9e5
   * and 6.1e6, whose buffer exists and covers the geometry it is taken of */
  const char *holed =
    "MULTIPOLYGON(((898914.9403076661 6122116.964651632,"
    "892000.0000313906 6128240.709886038,881878.7649123298 6134742.834215,"
    "895337.2320800173 6149482.958137999,911403.2986009999 6138679.914453,"
    "898914.9403076661 6122116.964651632),"
    "(907236.5285999999 6136625.731400001,894477.7245000005 6145526.389400002,"
    "891484.5062000006 6142245.358900003,893274.0940000005 6139118.099300001,"
    "900857.8630000008 6128190.306200001,903017.3059000005 6131044.047500003,"
    "907236.5285999999 6136625.731400001)))";
  char holed_covers_patt[10] = "T*****FF*";
  GSERIALIZED *holed_geo = geom_in(holed, -1);
  assert(holed_geo != NULL);
  meos_errno_reset();
  GSERIALIZED *holed_buf = geom_buffer(holed_geo, 1.0, "");
  printf("geom_buffer(a holed area at projected coordinates): %d, errno %d\n",
    holed_buf != NULL, meos_errno());
  assert(holed_buf != NULL);
  assert(meos_errno() == 0);
  bool holed_covers = geom_relate_pattern(holed_buf, holed_geo,
    holed_covers_patt);
  printf("  it covers the geometry it is taken of: %d\n", holed_covers);
  assert(holed_covers == true);
  assert(meos_errno() == 0);
  free(holed_geo); free(holed_buf);
  meos_errno_reset();

  /* Two computations of one node read as one node when they lie nearer than
   * the tolerance the buffer allows a node, and that tolerance takes the
   * square root of the machine epsilon and scales it by the COORDINATES. The
   * root belongs to the rounding those coordinates carry instead, so the
   * scaled form bounds a node by 0.318 metres at a projected 6.4e6 and reads
   * two crossings a centimetre apart as one. The witness is a rectangle with
   * a notch narrower than that. A buffer is the same shape wherever the
   * geometry sits, so the two copies below answer the SAME surface: the
   * rounded 32 by 22 rectangle of area 703.14, the notch being too narrow for
   * the offset to reach into. The copy at the origin, where every tolerance
   * is far from its limits, is what the projected one is read against */
  const char *notched[] = {
    "Polygon((0 0,30 0,30 20,15.025 20,15.025 18,14.975 18,14.975 20,"
      "0 20,0 0))",
    "Polygon((600000 6360000,600030 6360000,600030 6360020,"
      "600015.025 6360020,600015.025 6360018,600014.975 6360018,"
      "600014.975 6360020,600000 6360020,600000 6360000))"
  };
  char notched_patt[10] = "T*****FF*";
  for (int i = 0; i < 2; i++)
  {
    GSERIALIZED *g = geom_in(notched[i], -1);
    assert(g != NULL);
    meos_errno_reset();
    GSERIALIZED *b = geom_buffer(g, 1.0, "");
    printf("geom_buffer(a notched rectangle %s the origin): answered %d, "
      "errno %d\n", i ? "away from" : "at", b != NULL, meos_errno());
    assert(b != NULL);
    assert(meos_errno() == 0);
    bool covers = geom_relate_pattern(b, g, notched_patt);
    printf("  it covers the geometry it is taken of: %d\n", covers);
    assert(covers == true);
    assert(meos_errno() == 0);
    free(b); free(g);
    meos_errno_reset();
  }

  /* A ring of a polygon can enclose no area at all -- a slit that runs out to
   * a point and comes back along itself -- and real survey data carries them.
   * A hole is what a positive buffer ERODES, and a slit holds no disc of any
   * radius, so it closes completely and the answer is the buffer of the
   * rectangle alone: the rounded 32 by 22 of area 703.14. Offsetting the slit
   * instead sweeps a band of the buffer distance about it and punches that
   * band out, leaving a hole the geometry never had and 26.9 of the 600 the
   * rectangle covers outside its own buffer */
  const char *slit =
    "Polygon((0 0,30 0,30 20,0 20,0 0),(10 10,20 10,20 12,20 10,10 10))";
  char slit_patt[10] = "T*****FF*";
  GSERIALIZED *slit_geo = geom_in(slit, -1);
  assert(slit_geo != NULL);
  meos_errno_reset();
  GSERIALIZED *slit_buf = geom_buffer(slit_geo, 1.0, "");
  printf("geom_buffer(a rectangle whose hole encloses no area): %d, errno %d\n",
    slit_buf != NULL, meos_errno());
  assert(slit_buf != NULL);
  assert(meos_errno() == 0);
  bool slit_covers = geom_relate_pattern(slit_buf, slit_geo, slit_patt);
  printf("  it covers the geometry it is taken of: %d\n", slit_covers);
  assert(slit_covers == true);
  assert(meos_errno() == 0);
  free(slit_geo); free(slit_buf);
  meos_errno_reset();

  /* An offset that degenerates at one exact radius: the two offsets of a U
   * meet with no width left where the radius is half the gap between its arms,
   * and the inward offset of an arc lands on the centre where the radius
   * equals the arc's own. Both bound a buffer that exists on either side of
   * that radius, so the answer has to exist AT it too, and it has to sit
   * between its neighbours: a buffer grows with its radius, so the critical
   * one covers the smaller and is covered by the larger. That sandwich is the
   * oracle -- a buffer that merely answers proves nothing about its shape */
  const char *degenerate[] = {
    "Linestring(0 0,2 0,2 2,0 2)",      /* arms 2 apart, critical radius 1 */
    "Circularstring(20 0,21 1,22 0)"    /* radius 1, critical radius 1 */
  };
  char covers_patt[10] = "T*****FF*";
  for (int i = 0; i < 2; i++)
  {
    GSERIALIZED *g = geom_in(degenerate[i], -1);
    assert(g != NULL);
    meos_errno_reset();
    GSERIALIZED *below = geom_buffer(g, 0.999, "");
    GSERIALIZED *at = geom_buffer(g, 1.0, "");
    GSERIALIZED *above = geom_buffer(g, 1.001, "");
    printf("geom_buffer(%s) below %d at %d above %d, errno %d\n",
      degenerate[i], below != NULL, at != NULL, above != NULL, meos_errno());
    assert(below != NULL); assert(at != NULL); assert(above != NULL);
    assert(meos_errno() == 0);
    bool grows = geom_relate_pattern(at, below, covers_patt);
    bool grown = geom_relate_pattern(above, at, covers_patt);
    printf("  covers(at, below): %d, covers(above, at): %d\n", grows, grown);
    assert(grows == true);
    assert(grown == true);
    free(g); free(below); free(at); free(above);
    meos_errno_reset();
  }

  /* A repeated vertex draws an edge of no length, and such an edge lies under
   * every point of the plane unless the test that reads a point against it
   * rejects the ones its bounding box cannot hold. The witness is a square
   * that repeats a vertex and is contained in a larger one: the same square
   * written without the repetition is the control, and answers alike either
   * way, so what the pair isolates is the degenerate edge and nothing else */
  GSERIALIZED *outer = geom_in("POLYGON((0 0,0 2,2 2,2 0,0 0))", -1);
  GSERIALIZED *plain = geom_in("POLYGON((0 0,0 1,1 1,1 0,0 0))", -1);
  GSERIALIZED *repeated = geom_in("POLYGON((0 0,0 1,0 1,1 1,1 0,0 0))", -1);
  assert(outer != NULL); assert(plain != NULL); assert(repeated != NULL);
  meos_errno_reset();
  int has_plain = geom_contains(outer, plain);
  int has_repeated = geom_contains(outer, repeated);
  printf("geom_contains(outer, plain): %d, geom_contains(outer, repeated): "
    "%d, errno %d\n", has_plain, has_repeated, meos_errno());
  assert(has_plain == 1);
  assert(has_repeated == 1);
  assert(meos_errno() == 0);
  free(outer); free(plain); free(repeated);

  /* Equality is read from the native DE-9IM matrix, so two circular strings
   * describing the SAME arc through DIFFERENT defining points are equal. The
   * three points lie on the circle of centre (0 0) and radius 5, which they
   * determine exactly, so the answer rests on the arcs rather than on the
   * rounding of a circumcentre. A linearization answers about the chords and
   * calls the two different */
  GSERIALIZED *arc1 = geom_in("Circularstring(5 0,4 3,0 5)", -1);
  GSERIALIZED *arc2 = geom_in("Circularstring(5 0,3 4,0 5)", -1);
  assert(arc1 != NULL); assert(arc2 != NULL);
  meos_errno_reset();
  int arc_eq = geo_equals(arc1, arc2);
  printf("geo_equals(Circularstring(5 0,4 3,0 5), "
    "Circularstring(5 0,3 4,0 5)): %d, errno %d\n", arc_eq, meos_errno());
  assert(arc_eq == 1);
  assert(meos_errno() == 0);
  /* The same arc read backwards is the same point set */
  GSERIALIZED *arc3 = geom_in("Circularstring(0 5,3 4,5 0)", -1);
  assert(arc3 != NULL);
  assert(geo_equals(arc1, arc3) == 1);
  /* A part of that arc is not the whole of it, and a polyline on the three
   * points is not the arc through them */
  GSERIALIZED *part = geom_in("Circularstring(5 0,4 3,3 4)", -1);
  GSERIALIZED *chords = geom_in("Linestring(5 0,4 3,0 5)", -1);
  assert(part != NULL); assert(chords != NULL);
  assert(geo_equals(arc1, part) == 0);
  assert(geo_equals(arc1, chords) == 0);
  free(arc1); free(arc2); free(arc3); free(part); free(chords);
  meos_errno_reset();

  /* The minimum area does not name ONE rectangle. Every side of an acute
   * triangle carries a rectangle of area twice the triangle's, so all three
   * tie: the triangle below has area 35 and each of its three rectangles has
   * area 70. They are not the same rectangle -- their perimeters are 34,
   * 33.622 and 33.489 -- and the diagonal a caller reads as the size of the
   * region differs with them. The tightest is the answer, so that the envelope
   * is a function of its points rather than of the order in which the
   * candidate directions are visited */
  GSERIALIZED *tri = geom_in("Multipoint(0 0,10 0,4 7)", -1);
  assert(tri != NULL);
  meos_errno_reset();
  GSERIALIZED *env = geom_oriented_envelope(tri);
  assert(env != NULL);
  double env_perimeter = geom_perimeter(env);
  printf("geom_oriented_envelope(acute triangle) perimeter: %.6f, errno %d\n",
    env_perimeter, meos_errno());
  assert(env_perimeter > 33.48 && env_perimeter < 33.50);
  assert(meos_errno() == 0);
  free(tri); free(env);
  meos_errno_reset();

  /* A relationship of a curved geometry is read on the arc itself. The
   * polygon below is a circle of centre (49.092934300 -78.568502344) and
   * radius 17.469913928; the segment's ends lie 1.646e-3 and 5.64 INSIDE it,
   * so the circle contains and covers the segment. A linearization answers
   * about the chords it puts in the arc's place, and its chord at that angle
   * dips about 5e-3 in -- three times the clearance the near end has -- so it
   * places that end outside and answers neither */
  GSERIALIZED *circle = geom_in(
    "Curvepolygon(Circularstring("
    "66.56284822756567 -78.56850234445656,63.22639155760828 -68.29994457883014,"
    "54.491434593668544 -61.953626864231325,43.69443400570384 -61.95362686423132,"
    "34.9594770417641 -68.29994457883012,31.62302037180671 -78.56850234445656,"
    "34.9594770417641 -88.83706011008299,43.69443400570384 -95.1833778246818,"
    "54.49143459366854 -95.1833778246818,63.22639155760828 -88.83706011008299,"
    "66.56284822756567 -78.56850234445656))", -1);
  GSERIALIZED *inside = geom_in(
    "Linestring(43.28384893138511 -95.04256998228166,"
    "50.64080603370938 -90.29607546387359)", -1);
  assert(circle != NULL); assert(inside != NULL);
  meos_errno_reset();
  bool arc_contains = geom_contains(circle, inside);
  bool arc_covers = geom_covers(circle, inside);
  printf("geom_contains(circle, segment inside it): %d, "
    "geom_covers: %d, errno %d\n", arc_contains, arc_covers, meos_errno());
  assert(arc_contains == true);
  assert(arc_covers == true);
  assert(meos_errno() == 0);
  free(circle); free(inside);
  meos_errno_reset();

  /* A malformed box3d returns NULL and sets the error status */
  meos_errno_reset();
  BOX3D *bad_box3d = box3d_in("BOX3D(1 2 3)");
  printf("box3d_in(\"BOX3D(1 2 3)\"): %s, errno %d\n",
    bad_box3d ? "non-NULL" : "NULL", meos_errno());
  assert(bad_box3d == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
