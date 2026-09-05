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
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_h3.h>
#include <meos_quadbin.h>
#include <meos_s2cell.h>

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
  /* A TIN decomposes into the triangles it collects, so the edges answer it
   * and neither build consults GEOS for it */
  GSERIALIZED *tin = geom_in(
    "Tin(((0 0,0 1,1 1,0 0)),((0 0,1 0,1 1,0 0)))", -1);
  assert(tin != NULL);
  meos_errno_reset();
  bool rel_tin = geom_relate_pattern(through, tin, patt);
  int errno_tin = meos_errno();
  printf("geom_relate_pattern(line, tin): %d, errno %d\n", rel_tin,
    errno_tin);
  assert(rel_tin == true);
  assert(errno_tin == 0);
  /* A polyhedral surface decomposes into its faces the same way, and it is the
   * one type GEOS cannot be asked about at all -- LWGEOM2GEOS reaches an
   * lwerror that ends the process rather than returning -- so the edges are
   * what answer it in either build */
  GSERIALIZED *phs = geom_in(
    "PolyhedralSurface Z (((0 0 0,0 1 0,1 1 0,1 0 0,0 0 0)),"
    "((0 0 0,0 1 0,0 1 1,0 0 1,0 0 0)))", -1);
  assert(phs != NULL);
  meos_errno_reset();
  bool rel_phs = geom_relate_pattern(through, phs, patt);
  int errno_phs = meos_errno();
  printf("geom_relate_pattern(line, polyhedral surface): %d, errno %d\n",
    rel_phs, errno_phs);
  assert(rel_phs == true);
  assert(errno_phs == 0);
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

  /* What a radial distance misses its arc by is a property of the coordinates
   * it is read from, so the band it is judged against is theirs. At projected
   * coordinates the point this states lies 1.6e-11 off the circle it is placed
   * on, which is under a tenth of what those coordinates express and far over
   * an absolute 1e-12, so a band that does not scale reads the boundary of a
   * disc as its exterior. The same disc at the origin answers the same way,
   * which is what makes the case about the scale rather than the geometry */
  const char *disc_wkt[2] = {
    "CURVEPOLYGON(CIRCULARSTRING(-1000 0,1000 0,-1000 0))",
    "CURVEPOLYGON(CIRCULARSTRING(6399000 4600000,6401000 4600000,"
      "6399000 4600000))" };
  const char *edge_wkt[2] = {
    "POINT(707.10678118654755 707.10678118654755)",
    "POINT(6400707.1067811865 4600707.1067811865)" };
  for (int i = 0; i < 2; i++)
  {
    GSERIALIZED *disc = geom_in(disc_wkt[i], -1);
    GSERIALIZED *edge_pt = geom_in(edge_wkt[i], -1);
    assert(disc != NULL); assert(edge_pt != NULL);
    meos_errno_reset();
    char *disc_matrix = geom_relate(disc, edge_pt);
    printf("a disc %s its own boundary point: %s\n",
      i ? "at projected coordinates against" : "at the origin against",
      disc_matrix);
    assert(disc_matrix != NULL);
    assert(strcmp(disc_matrix, "FF20F1FF2") == 0);
    assert(meos_errno() == 0);
    free(disc); free(edge_pt); free(disc_matrix);
    meos_errno_reset();
  }

  /* A circle is read as the two arcs between one pair of points, so a portion
   * of it is told from the complementary one by a point strictly inside it
   * and never by the endpoints alone. What locates a point against a
   * collection of two or more surfaces is the union of their boundaries, and
   * a circle contributing one half of itself there bounds nothing: every
   * point of the surface reads as lying outside it. The buffer of a geometry
   * of several parts is such a collection, each part bounded by a circle */
  char pair_covers_patt[10] = "T*****FF*";
  GSERIALIZED *pair_geo = geom_in("MULTIPOINT(0 0,10 0)", -1);
  assert(pair_geo != NULL);
  meos_errno_reset();
  GSERIALIZED *pair_buf = geom_buffer(pair_geo, 1.0, "");
  printf("geom_buffer(a geometry of two points): %d, errno %d\n",
    pair_buf != NULL, meos_errno());
  assert(pair_buf != NULL);
  assert(meos_errno() == 0);
  bool pair_covers = geom_relate_pattern(pair_buf, pair_geo, pair_covers_patt);
  printf("  it covers the geometry it is taken of: %d\n", pair_covers);
  assert(pair_covers == true);
  char *pair_matrix = geom_relate(pair_buf, pair_geo);
  printf("  its matrix against the two points: %s\n", pair_matrix);
  assert(pair_matrix != NULL);
  assert(strcmp(pair_matrix, "0F2FF1FF2") == 0);
  assert(meos_errno() == 0);
  free(pair_geo); free(pair_buf); free(pair_matrix);
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

  /* Contracting a closed ring by more than it encloses carries the contraction
   * through itself, and it re-emerges inverted at the distance it overshot by.
   * That curve lies NEARER the input than the buffer distance, so it bounds
   * nothing and the hole it stands for is gone rather than uncovered; kept, it
   * punches a hole out of the answer and the buffer stops containing the very
   * geometry it is taken of. The closed LINESTRING branch tests for it; the
   * closed CURVE branch is the same shape and asked nothing, so a compound
   * curve buffered past its inradius answered a surface missing its middle.
   * The witness is a closed curve about 10 by 2, so its inradius is near 1,
   * buffered on both sides of that. A buffer contains its own input at EVERY
   * radius, which needs no oracle to check */
  const char *closed_curve =
    "CompoundCurve((0 0,10 0),CircularString(10 0,5 2,0 0))";
  const double closed_radius[] = {0.5, 3.0};
  char closed_patt[10] = "T*****FF*";
  for (int i = 0; i < 2; i++)
  {
    GSERIALIZED *g = geom_in(closed_curve, -1);
    assert(g != NULL);
    meos_errno_reset();
    GSERIALIZED *b = geom_buffer(g, closed_radius[i], "");
    printf("geom_buffer(a closed curve, %s its inradius): answered %d, "
      "errno %d\n", i ? "past" : "within", b != NULL, meos_errno());
    assert(b != NULL);
    assert(meos_errno() == 0);
    bool covers = geom_relate_pattern(b, g, closed_patt);
    printf("  it covers the geometry it is taken of: %d\n", covers);
    assert(covers == true);
    assert(meos_errno() == 0);
    free(b); free(g);
    meos_errno_reset();
  }

  /* The junction between two offsets is settled from the cross product of the
   * directions the two edges meeting there run in, and that product is a sine
   * only where both directions are of unit length. An arc answers a sine and
   * a cosine and is; a straight edge answered its whole chord and was not, so
   * the product was a length where a straight edge met an arc and an area
   * where two straight edges met, and both shrink with the geometry until a
   * genuine turn falls under the tolerance and reads as one edge continuing
   * into the next. The witness is a compound curve of a straight run closing
   * through an arc, whose junctions are exactly those two kinds. A buffer is
   * the same shape wherever the geometry sits and whatever its size, so the
   * two copies below answer the same surface scaled: the copy at unit size,
   * where every tolerance is far from its limits, is what the small one is
   * read against. Before the directions were normalized the small copy lost
   * both junctions and its buffer read zero wide */
  const char *turning[] = {
    "CompoundCurve((0 0,10 0),CircularString(10 0,5 2,0 0))",
    "CompoundCurve((0 0,1e-06 0),CircularString(1e-06 0,5e-07 2e-07,0 0))"
  };
  const double turning_radius[] = {1.0, 1e-07};
  char turning_patt[10] = "T*****FF*";
  for (int i = 0; i < 2; i++)
  {
    GSERIALIZED *g = geom_in(turning[i], -1);
    assert(g != NULL);
    meos_errno_reset();
    GSERIALIZED *b = geom_buffer(g, turning_radius[i], "");
    printf("geom_buffer(a curve turning twice, %s): answered %d, errno %d\n",
      i ? "a millionth the size" : "at unit size", b != NULL, meos_errno());
    assert(b != NULL);
    assert(meos_errno() == 0);
    bool covers = geom_relate_pattern(b, g, turning_patt);
    printf("  it covers the geometry it is taken of: %d\n", covers);
    assert(covers == true);
    assert(meos_errno() == 0);
    free(b); free(g);
    meos_errno_reset();
  }

  /* A boundary piece kept by the resolve lies AT the buffer distance, and the
   * distance deciding that is read from the coordinates, so what it is rounded
   * to is the size of THEIR last bits and not of the radius. At a projected
   * 6.4e6 a piece lying exactly on the offset reads about 1e-09 below the
   * radius, and a bound calibrated to the radius drops it; the ring it belongs
   * to then reaches a point nothing continues and the buffer is refused. The
   * witness is one real protected area of 65 vertices at coordinates near
   * 4.2e5 and 6.5e6, whose buffer exists and covers the geometry it is taken
   * of */
  const char *keepbound =
    "Polygon(("
    "489953.4199511388 6097648.881000896,489920.84889343055 6097658.879044168,489907.729716571 6097663.488146533,"
    "489892.72231515805 6097667.9771457305,489870.4503575071 6097675.7256820435,489859.96157083136 6097676.045579018,"
    "489849.0358225395 6097678.075084536,489837.83791321533 6097680.934604656,489826.9451779598 6097683.58414084,"
    "489810.4288334197 6097687.8132292535,489795.86665023636 6097691.8723328905,489768.812026859 6097699.550729225,"
    "489729.4958934359 6097711.248388912,489704.64294093545 6097714.867743733,489682.1976839783 6097720.606457519,"
    "489648.57111331774 6097724.45570251,489635.85600250604 6097725.905431924,489633.8192796183 6097725.405500338,"
    "489541.375137292 6097976.914247721,489524.2155058065 6098023.974789903,489497.2763028761 6098100.419156248,"
    "489495.32210642466 6098105.838188812,489481.8895747158 6098143.380379892,489456.76452312607 6098217.35544213,"
    "489455.89868972416 6098217.285486622,489293.3978580587 6098139.811078393,489223.5473194059 6098238.461127255,"
    "489164.869909644 6098383.271651909,489198.8262683569 6098399.768292302,489215.57353442116 6098407.666678409,"
    "489216.4889075318 6098408.546475426,489218.7646880301 6098433.77129485,489219.04507516406 6098450.667942926,"
    "489220.33147313143 6098464.345168544,489221.5271011598 6098470.813889468,489223.0030441293 6098476.002770108,"
    "489225.4191173709 6098481.61158111,489228.68442376313 6098487.320390267,489234.86879830575 6098494.898907694,"
    "489252.96025164 6098507.02640009,489266.4669000705 6098515.094794869,489268.29751055 6098518.794133615,"
    "489270.2765390384 6098522.92310304,489269.31174154114 6098523.113149199,489277.21127267333 6098529.961726897,"
    "489358.7049713157 6098514.7148460625,489367.94032394246 6098512.98527085,489524.89995898225 6098481.511698607,"
    "489549.9095684035 6098492.649365734,489565.31286089146 6098494.089168399,489613.4520515562 6098492.719445756,"
    "489628.0142421348 6098493.439323186,489648.85145946336 6098494.578923416,489664.2959753024 6098493.769111497,"
    "489668.2621250196 6098484.301024237,489672.0799681452 6098466.934625475,489797.2519702689 6098449.798070793,"
    "489902.7658147407 6098435.351075934,489914.5326199415 6098433.101463734,489918.3998888076 6098410.596145976,"
    "489972.07205796684 6098409.806260057,490046.7381531224 6098408.666528126,490096.93891947356 6098407.746552396,"
    "490123.7709086013 6098406.296952407,489953.4199511388 6097648.881000896"
    "))";
  char keepbound_patt[10] = "T*****FF*";
  GSERIALIZED *keepbound_geo = geom_in(keepbound, -1);
  assert(keepbound_geo != NULL);
  meos_errno_reset();
  GSERIALIZED *keepbound_buf = geom_buffer(keepbound_geo, 1.0, "");
  printf("geom_buffer(a real area whose offset lies on the buffer distance): "
    "%d, errno %d\n", keepbound_buf != NULL, meos_errno());
  assert(keepbound_buf != NULL);
  assert(meos_errno() == 0);
  bool keepbound_covers = geom_relate_pattern(keepbound_buf, keepbound_geo,
    keepbound_patt);
  printf("  it covers the geometry it is taken of: %d\n", keepbound_covers);
  assert(keepbound_covers == true);
  assert(meos_errno() == 0);
  free(keepbound_geo); free(keepbound_buf);
  meos_errno_reset();

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

  /* An EMPTY geometry draws no line and bounds no area, so it measures 0.
   * That is an ANSWER: nothing is raised, and a caller reading the error
   * return would have no error to read it by */
  const char *emptywkt[] = {
    "LINESTRING EMPTY",
    "POLYGON EMPTY",
    "GEOMETRYCOLLECTION EMPTY",
  };
  for (int i = 0; i < 3; i++)
  {
    GSERIALIZED *e = geom_in(emptywkt[i], -1);
    assert(e != NULL);
    meos_errno_reset();
    double elen = geom_length(e);
    double eper = geom_perimeter(e);
    double ear = geom_area(e);
    printf("%s: length %g, perimeter %g, area %g, errno %d\n", emptywkt[i],
      elen, eper, ear, meos_errno());
    assert(elen == 0.0);
    assert(eper == 0.0);
    assert(ear == 0.0);
    assert(meos_errno() == 0);
    free(e);
    meos_errno_reset();
  }
  /* The measures a NON-empty geometry carries are what says the removed guard
   * took the empty case alone */
  GSERIALIZED *meas = geom_in("POLYGON((0 0,3 0,3 4,0 4,0 0))", -1);
  assert(meas != NULL);
  meos_errno_reset();
  printf("the 3 by 4 rectangle: length %g, perimeter %g, area %g\n",
    geom_length(meas), geom_perimeter(meas), geom_area(meas));
  assert(geom_length(meas) == 0.0);
  assert(geom_perimeter(meas) == 14.0);
  assert(geom_area(meas) == 12.0);
  assert(meos_errno() == 0);
  free(meas);
  meos_errno_reset();

  /* The area a geometry of each dimension carries: a point and a line enclose
   * nothing, a ring encloses what it bounds less its holes, and a geodetic
   * geometry is measured by #geog_area rather than by this one */
  GSERIALIZED *apt = geom_in("POINT(1 1)", -1);
  GSERIALIZED *aln = geom_in("LINESTRING(0 0,3 4)", -1);
  GSERIALIZED *ahole = geom_in(
    "POLYGON((0 0,10 0,10 10,0 10,0 0),(2 2,4 2,4 4,2 4,2 2))", -1);
  GSERIALIZED *amulti = geom_in(
    "MULTIPOLYGON(((0 0,2 0,2 2,0 2,0 0)),((5 5,8 5,8 9,5 9,5 5)))", -1);
  assert(apt && aln && ahole && amulti);
  meos_errno_reset();
  printf("area: point %g, line %g, holed square %g, multipolygon %g\n",
    geom_area(apt), geom_area(aln), geom_area(ahole), geom_area(amulti));
  assert(geom_area(apt) == 0.0);
  assert(geom_area(aln) == 0.0);
  assert(geom_area(ahole) == 96.0);
  assert(geom_area(amulti) == 16.0);
  assert(meos_errno() == 0);
  free(apt); free(aln); free(ahole); free(amulti);
  meos_errno_reset();
  /* A geodetic geometry is refused, as it is by the other planar measures */
  GSERIALIZED *ageog = geog_in("SRID=4326;POLYGON((0 0,1 0,1 1,0 1,0 0))", -1);
  assert(ageog != NULL);
  meos_errno_reset();
  double gar = geom_area(ageog);
  printf("area of a geography: %g, errno %d\n", gar, meos_errno());
  assert(gar == DBL_MAX);
  assert(meos_errno() != 0);
  free(ageog);
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

  /* The clustering entries partition an array of geometries: the three points
   * about the origin and the line string joining two of them meet, the far
   * point stands alone, and a member carrying another SRID is refused */
  meos_errno_reset();
  const GSERIALIZED *cl[4];
  cl[0] = geom_in("SRID=4326;Point(0 0)", -1);
  cl[1] = geom_in("SRID=4326;Point(1 1)", -1);
  cl[2] = geom_in("SRID=4326;Linestring(0 0,1 1)", -1);
  cl[3] = geom_in("SRID=4326;Point(50 50)", -1);
  int nclusters = 0;
  GSERIALIZED **clusters = geo_cluster_intersecting(cl, 4, &nclusters);
  printf("clusterIntersecting of 4 geometries: %d clusters, errno %d\n",
    nclusters, meos_errno());
  assert(clusters != NULL);
  assert(nclusters == 2);
  assert(meos_errno() == 0);
  meos_errno_reset();

  /* The same array within a distance reaching every member is one cluster */
  int nwithin = 0;
  GSERIALIZED **within = geo_cluster_within(cl, 4, 100.0, &nwithin);
  printf("clusterWithin of the same 4 at distance 100: %d cluster(s)\n",
    nwithin);
  assert(within != NULL);
  assert(nwithin == 1);
  assert(meos_errno() == 0);
  /* Both entries answer a fresh array of fresh collections, so the array and
   * every collection in it belong to this caller. The geometries themselves
   * are released once the entries reading them are done, further down */
  for (int i = 0; i < nclusters; i++)
    free(clusters[i]);
  free(clusters);
  for (int i = 0; i < nwithin; i++)
    free(within[i]);
  free(within);
  meos_errno_reset();

  /* Clustering geometries of different SRIDs is an error */
  const GSERIALIZED *mixed[2];
  mixed[0] = geom_in("SRID=4326;Point(0 0)", -1);
  mixed[1] = geom_in("SRID=3812;Point(0 1)", -1);
  int nmixed = 0;
  GSERIALIZED **bad = geo_cluster_intersecting(mixed, 2, &nmixed);
  printf("clusterIntersecting on mixed SRID: %s, errno %d\n",
    bad ? "non-NULL" : "NULL", meos_errno());
  assert(bad == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();

  /* Every entry taking an array of geometries reads one reference system, so
   * each of them reports the mixture rather than answering about it */
  int nbad = 0;
  assert(geo_cluster_kmeans(mixed, 2, 1, &nbad) == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(geo_cluster_dbscan(mixed, 2, 1.0, 1, &nbad) == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(geo_cluster_within(mixed, 2, 1.0, &nbad) == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();
  GSERIALIZED *mixedarr[2];
  mixedarr[0] = (GSERIALIZED *) mixed[0];
  mixedarr[1] = (GSERIALIZED *) mixed[1];
  assert(geom_array_union(mixedarr, 2) == NULL);
  printf("the five array entries all report a mixture of SRIDs\n");
  assert(meos_errno() != 0);
  meos_errno_reset();

  /* The clustering seeds itself from the CENTROID of every input that is not a
   * point, so an array carrying a line reaches an answer a build with no GEOS
   * has to hold on its own. The two clusters here are the pair at the origin
   * and the point far from it */
  int nkmeans = 0;
  int *kmeans = geo_cluster_kmeans(cl, 4, 2, &nkmeans);
  assert(kmeans != NULL);
  assert(nkmeans == 4);
  assert(kmeans[0] == kmeans[1] && kmeans[1] == kmeans[2]);
  assert(kmeans[3] != kmeans[0]);
  printf("clusterKMeans over a line and three points: %d %d %d %d\n",
    kmeans[0], kmeans[1], kmeans[2], kmeans[3]);
  free(kmeans);
  meos_errno_reset();

  /* The k-means clustering reports a missing output parameter, which it reads
   * before the count of clusters is written to it */
  assert(geo_cluster_kmeans(cl, 4, 2, NULL) == NULL);
  printf("clusterKMeans without an output parameter: errno %d\n",
    meos_errno());
  assert(meos_errno() != 0);
  /* The last entry reading them has answered, so the parsed geometries go */
  for (int i = 0; i < 4; i++)
    free((GSERIALIZED *) cl[i]);
  for (int i = 0; i < 2; i++)
    free((GSERIALIZED *) mixed[i]);
  meos_errno_reset();

  /* A ring that encloses no area draws its own linework and bounds no
   * region, so it relates as a line and never as a surface. The square that
   * does enclose area is the control */
  GSERIALIZED *square2 = geom_in("POLYGON((0 0,0 2,2 2,2 0,0 0))", -1);
  GSERIALIZED *noarea = geom_in("POLYGON((3 0,3 1,3 1,3 0,3 0))", -1);
  GSERIALIZED *realsq = geom_in("POLYGON((3 0,3 1,4 1,4 0,3 0))", -1);
  /* The interior of the slit lies in the exterior of the square and is one
   * dimensional; a surface there would make it two */
  assert(geom_relate_pattern(noarea, square2, "**1******") == true);
  assert(geom_relate_pattern(noarea, square2, "**2******") == false);
  /* The control keeps the two dimensions of a surface */
  assert(geom_relate_pattern(realsq, square2, "**2******") == true);
  printf("a ring enclosing no area relates as the linework it draws\n");
  assert(meos_errno() == 0);
  free(square2); free(noarea); free(realsq);
  meos_errno_reset();

  /* Two members of a multipolygon may share a boundary edge, and that edge
   * lies in the INTERIOR of what they cover together. The same region written
   * as one polygon is the control: both must report a line running along that
   * edge as meeting their interior */
  GSERIALIZED *adjmp = geom_in("MULTIPOLYGON(((0 0,0 1,1 1,0 0)),"
    "((0 0,1 1,1 0,0 0)))", -1);
  GSERIALIZED *adjsq = geom_in("POLYGON((0 0,0 1,1 1,1 0,0 0))", -1);
  GSERIALIZED *diag = geom_in("LINESTRING(0 0,2 2)", -1);
  assert(geom_relate_pattern(adjmp, diag, "1********") == true);
  assert(geom_relate_pattern(adjsq, diag, "1********") == true);
  printf("the members of a multipolygon share the interior they cover\n");
  assert(meos_errno() == 0);
  meos_errno_reset();

  /* A TIN is a set of triangles, so it covers what the same triangles cover
   * written any other way. GEOS carries no TIN at all, so what answers for it
   * is that identity rather than a foreign answer */
  GSERIALIZED *adjtin = geom_in("TIN Z (((0 0 0,0 1 0,1 1 0,0 0 0)),"
    "((0 0 0,1 1 0,1 0 0,0 0 0)))", -1);
  assert(geom_relate_pattern(adjtin, diag, "1********") == true);
  printf("a TIN covers what its triangles cover written any other way\n");
  assert(meos_errno() == 0);
  free(adjmp); free(adjtin);
  meos_errno_reset();

  /* A polyhedral surface covers what its faces cover, and the unit cube is the
   * case that says so about a WATERTIGHT SOLID: four of its six faces stand
   * perpendicular to the plane, so each projects to a ring enclosing no area.
   * What the cube covers in the plane is the unit square, which is exactly the
   * control the two assertions above already use, so the identity is read
   * against a geometry this test has independently checked */
  GSERIALIZED *cube = geom_in("POLYHEDRALSURFACE Z ("
    "((0 0 0,0 1 0,1 1 0,1 0 0,0 0 0)),"
    "((0 0 0,0 0 1,0 1 1,0 1 0,0 0 0)),"
    "((0 0 0,1 0 0,1 0 1,0 0 1,0 0 0)),"
    "((1 1 1,1 0 1,0 0 1,0 1 1,1 1 1)),"
    "((1 1 1,1 1 0,1 0 0,1 0 1,1 1 1)),"
    "((1 1 1,0 1 1,0 1 0,1 1 0,1 1 1)))", -1);
  assert(cube != NULL);
  assert(geom_relate_pattern(cube, diag, "1********") == true);
  /* And it is areal, not the linework its perpendicular faces draw */
  assert(geom_relate_pattern(cube, diag, "2********") == false);
  assert(geom_relate_pattern(adjsq, diag, "2********") == false);
  printf("a closed polyhedral surface covers what it projects to\n");
  assert(meos_errno() == 0);
  meos_errno_reset();
  /* The square and the diagonal are read by the cube as well as by the
   * surfaces above, so they go with the last reader of them */
  free(cube); free(adjsq); free(diag);

  /* The overlay answers NULL for a geometry it cannot read -- a polyhedral
   * surface reaches the default arm of LWGEOM2GEOS, whose lwerror the MEOS
   * handler reports and returns from -- so the union has to answer the absence
   * rather than read the null pointer it was handed. Every step after that
   * read one, and the process died in lwgeom_set_geodetic */
  GSERIALIZED *phsurf = geom_in("PolyhedralSurface Z ("
    "((0 0 0,0 1 0,1 1 0,1 0 0,0 0 0)),((0 0 0,0 0 1,0 1 1,0 1 0,0 0 0)))", -1);
  assert(phsurf != NULL);
  meos_errno_reset();
  GSERIALIZED *uu = geom_unary_union(phsurf, -1);
  printf("geom_unary_union of a polyhedral surface: %s, errno %d\n",
    uu ? "answered" : "declined", meos_errno());
  assert(uu == NULL);
  assert(meos_errno() != 0);
  /* The control is a geometry the overlay does read */
  meos_errno_reset();
  GSERIALIZED *mp = geom_in("MULTIPOLYGON(((0 0,0 1,1 1,1 0,0 0)),"
    "((1 0,1 1,2 1,2 0,1 0)))", -1);
  GSERIALIZED *uu2 = geom_unary_union(mp, -1);
  assert(uu2 != NULL);
  assert(meos_errno() == 0);
  printf("the union of a multipolygon is answered as before\n");
  free(phsurf); free(mp); free(uu2);
  meos_errno_reset();

  /* A geography is answered on the spheroid, so it is #geog_array_union() that
   * takes it and #geom_array_union() that turns it away. The union of a set of
   * positions is the set with its duplicates removed, and two positions are
   * the same when their coordinates are, so the answer is read without
   * measuring anything and is the same on the spheroid as on the plane */
  GSERIALIZED *gg1 = geog_in("POINT(1 1)", -1);
  GSERIALIZED *gg2 = geog_in("POINT(2 2)", -1);
  assert(gg1 != NULL); assert(gg2 != NULL);
  GSERIALIZED *ggarr[2] = {gg1, gg2};
  meos_errno_reset();
  GSERIALIZED *ggu = geog_array_union(ggarr, 2);
  assert(ggu != NULL);
  char *ggwkt = geo_as_ewkt(ggu, 6);
  printf("the union of two geography points: %s\n", ggwkt);
  assert(strcmp(ggwkt, "SRID=4326;MULTIPOINT(1 1,2 2)") == 0);
  free(ggu); free(ggwkt);
  meos_errno_reset();

  /* The planar entry turns a geography away rather than reading its degrees as
   * cartesian coordinates */
  GSERIALIZED *ggrefused = geom_array_union(ggarr, 2);
  assert(ggrefused == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  printf("the planar union refuses a geography\n");
  meos_errno_reset();

  /* And the geodetic entry answers no more than the spheroid answers: an array
   * carrying anything but positions awaits an overlay that works there */
  GSERIALIZED *ggp1 = geog_in("POLYGON((0 0,0 1,1 1,1 0,0 0))", -1);
  GSERIALIZED *ggp2 = geog_in("POLYGON((2 2,2 3,3 3,3 2,2 2))", -1);
  GSERIALIZED *ggparr[2] = {ggp1, ggp2};
  meos_errno_reset();
  assert(geog_array_union(ggparr, 2) == NULL);
  assert(meos_errno() == MEOS_ERR_FEATURE_NOT_SUPPORTED);
  printf("the geodetic union answers positions and declines the rest\n");
  free(gg1); free(gg2); free(ggp1); free(ggp2);
  meos_errno_reset();

  /* An array whose members fall on BOTH sides of the areal boundary is
   * answered by the two native arms together: the surfaces are dissolved, the
   * linework and the points are dissolved, and a piece the surfaces COVER is
   * left out of the answer.  A piece they cover only in PART stays WHOLE,
   * which is where the answer differs from the one GEOS gives -- both
   * spellings cover the same points, and keeping the piece whole keeps a
   * circular arc on its own circle */
  const char *msq = "POLYGON((0 0,0 2,2 2,2 0,0 0))";
  struct { const char *piece; const char *result; } mixcases[] = {
    /* the point is outside, so it stands beside the surface */
    { "POINT(5 5)",
      "GEOMETRYCOLLECTION(POINT(5 5),POLYGON((0 0,0 2,2 2,2 0,0 0)))" },
    /* the point is inside, so the surface is the whole answer */
    { "POINT(1 1)", "POLYGON((0 0,0 2,2 2,2 0,0 0))" },
    /* a point ON the boundary is covered as well */
    { "POINT(0 1)", "POLYGON((0 0,0 2,2 2,2 0,0 0))" },
    /* the line is covered, and contributes nothing of its own */
    { "LINESTRING(0.5 0.5,1.5 1.5)", "POLYGON((0 0,0 2,2 2,2 0,0 0))" },
    /* the line is covered only in part and stays whole */
    { "LINESTRING(1 1,5 5)",
      "GEOMETRYCOLLECTION(LINESTRING(1 1,5 5),POLYGON((0 0,0 2,2 2,2 0,0 0)))" },
    /* the arc stays on its own circle rather than being linearized */
    { "CIRCULARSTRING(5 5,6 6,7 5)",
      "GEOMETRYCOLLECTION(CIRCULARSTRING(5 5,6 6,7 5),"
        "POLYGON((0 0,0 2,2 2,2 0,0 0)))" },
  };
  for (size_t i = 0; i < sizeof(mixcases) / sizeof(mixcases[0]); i++)
  {
    GSERIALIZED *mx1 = geom_in(mixcases[i].piece, -1);
    GSERIALIZED *mx2 = geom_in(msq, -1);
    assert(mx1 != NULL); assert(mx2 != NULL);
    GSERIALIZED *mxarr[2] = {mx1, mx2};
    meos_errno_reset();
    GSERIALIZED *mxu = geom_array_union(mxarr, 2);
    assert(mxu != NULL);
    assert(meos_errno() == 0);
    char *mxwkt = geo_as_text(mxu, 3);
    assert(mxwkt != NULL);
    printf("%s united with the square: %s\n", mixcases[i].piece, mxwkt);
    assert(strcmp(mxwkt, mixcases[i].result) == 0);
    free(mx1); free(mx2); free(mxu); free(mxwkt);
  }
  meos_errno_reset();

  /* The members of an array need not share their dimensions, and the answer is
   * read on the ones they do: a member carrying no elevation determines none
   * for the points it contributes, so an answer declaring one would publish a
   * value the array does not hold. Every arm is given a mixture here -- the
   * surfaces, the linework, and an array spanning the areal boundary -- and
   * the answer covers the same points whichever member the array lists first */
  const char *dimcases[][3] = {
    /* the linear arm, whose members are points */
    { "POINT Z(0 0 1)", "POINT(1 1)", "MULTIPOINT((0 0),(1 1))" },
    /* the same members in the other order cover the same points: the answer
     * is read from what the array holds, not from what it lists first */
    { "POINT(1 1)", "POINT Z(0 0 1)", "MULTIPOINT((1 1),(0 0))" },
    /* the linear arm, whose members are curves */
    { "LINESTRING Z(0 0 1,1 1 1)", "LINESTRING(5 5,6 6)",
      "MULTILINESTRING((0 0,1 1),(5 5,6 6))" },
    /* the areal arm, whose surfaces stay apart */
    { "POLYGON Z((0 0 1,0 2 1,2 2 1,2 0 1,0 0 1))",
      "POLYGON((9 9,9 11,11 11,11 9,9 9))",
      "MULTIPOLYGON(((0 0,0 2,2 2,2 0,0 0)),((9 9,9 11,11 11,11 9,9 9)))" },
    /* the areal arm, dissolving a pair whose interiors meet */
    { "POLYGON Z((0 0 1,0 2 1,2 2 1,2 0 1,0 0 1))",
      "POLYGON((1 1,1 3,3 3,3 1,1 1))",
      "POLYGON((1 2,0 2,0 0,2 0,2 1,3 1,3 3,1 3,1 2))" },
    /* both arms together, each half sharing its dimensions and the two halves
     * not sharing them with each other */
    { "POLYGON Z((0 0 1,0 2 1,2 2 1,2 0 1,0 0 1))", "LINESTRING(5 5,6 6)",
      "GEOMETRYCOLLECTION(LINESTRING(5 5,6 6),"
        "POLYGON((0 0,0 2,2 2,2 0,0 0)))" },
    /* a measure is a dimension of the answer as an elevation is, and the two
     * are read one by one: the elevation both members carry is kept */
    { "POINT ZM(0 0 1 2)", "POINT Z(1 1 1)", "MULTIPOINT Z ((0 0 1),(1 1 1))" },
    { "POINT M(0 0 1)", "POINT(1 1)", "MULTIPOINT((0 0),(1 1))" },
    /* an EMPTY member contributes no points, so it takes no ordinate away
     * from the members that do carry one */
    { "POINT Z(0 0 1)", "POLYGON EMPTY", "POINT Z (0 0 1)" },
  };
  for (size_t i = 0; i < sizeof(dimcases) / sizeof(dimcases[0]); i++)
  {
    GSERIALIZED *dm1 = geom_in(dimcases[i][0], -1);
    GSERIALIZED *dm2 = geom_in(dimcases[i][1], -1);
    assert(dm1 != NULL); assert(dm2 != NULL);
    GSERIALIZED *dmarr[2] = {dm1, dm2};
    meos_errno_reset();
    GSERIALIZED *dmu = geom_array_union(dmarr, 2);
    assert(dmu != NULL);
    assert(meos_errno() == 0);
    char *dmwkt = geo_as_text(dmu, 3);
    assert(dmwkt != NULL);
    printf("union of %s with %s: %s\n", dimcases[i][0], dimcases[i][1], dmwkt);
    assert(strcmp(dmwkt, dimcases[i][2]) == 0);
    free(dm1); free(dm2); free(dmu); free(dmwkt);
    meos_errno_reset();
  }

  /* The control is the same array with its members sharing one dimension,
   * which is answered exactly as before */
  GSERIALIZED *dmz1 = geom_in("POINT Z(0 0 1)", -1);
  GSERIALIZED *dmz2 = geom_in("POINT Z(1 1 1)", -1);
  GSERIALIZED *dmzarr[2] = {dmz1, dmz2};
  GSERIALIZED *dmzu = geom_array_union(dmzarr, 2);
  assert(dmzu != NULL);
  assert(meos_errno() == 0);
  char *dmzwkt = geo_as_text(dmzu, 3);
  printf("union of two elevated positions: %s\n", dmzwkt);
  assert(strcmp(dmzwkt, "MULTIPOINT Z ((0 0 1),(1 1 1))") == 0);
  free(dmz1); free(dmz2); free(dmzu); free(dmzwkt);
  meos_errno_reset();

  /* A union is computed on the PLANE while the Z and M its members carry are a
   * lift of that plane: the ordinates are given at their vertices and read
   * between them along the curve joining them. Every vertex of the answer lies
   * on the boundary of a member, so a member determines an ordinate there --
   * unless two of them determine DIFFERENT ones, as two surfaces crossing at
   * different elevations do, when the point has none the members give. The
   * answer therefore carries an ordinate exactly where that lift is unique */
  struct { const char *first; const char *second; const char *result; }
    liftcases[] = {
    /* two surfaces crossing on ONE plane: the nodes the overlay adds are
     * elevated by the edges carrying them, which agree */
    { "POLYGON Z((0 0 1,0 2 1,2 2 1,2 0 1,0 0 1))",
      "POLYGON Z((1 1 1,1 3 1,3 3 1,3 1 1,1 1 1))",
      "POLYGON Z ((1 2 1,0 2 1,0 0 1,2 0 1,2 1 1,3 1 1,3 3 1,1 3 1,1 2 1))" },
    /* the same pair on the sloping plane Z = x, which the added nodes are
     * read onto: (1 2) takes 1 and (2 1) takes 2 */
    { "POLYGON Z((0 0 0,0 2 0,2 2 2,2 0 2,0 0 0))",
      "POLYGON Z((1 1 1,1 3 1,3 3 3,3 1 3,1 1 1))",
      "POLYGON Z ((1 2 1,0 2 0,0 0 0,2 0 2,2 1 2,3 1 3,3 3 3,1 3 1,1 2 1))" },
    /* two surfaces crossing at DIFFERENT elevations: the nodes have none, so
     * the answer is the planar figure rather than one carrying a mean */
    { "POLYGON Z((0 0 1,0 2 1,2 2 1,2 0 1,0 0 1))",
      "POLYGON Z((1 1 7,1 3 7,3 3 7,3 1 7,1 1 7))",
      "POLYGON((1 2,0 2,0 0,2 0,2 1,3 1,3 3,1 3,1 2))" },
    /* two surfaces MEETING along an edge at different elevations: the shared
     * edge is where they disagree, and the shell is traversed counter-clockwise
     * as #buffer_normalize_ring_orientation() leaves every shell it builds */
    { "POLYGON Z((0 0 1,0 2 1,2 2 1,2 0 1,0 0 1))",
      "POLYGON Z((2 0 7,2 2 7,4 2 7,4 0 7,2 0 7))",
      "POLYGON((2 2,0 2,0 0,2 0,4 0,4 2,2 2))" },
    /* linework coinciding over a stretch, dissolved into one curve that keeps
     * the elevation both members carry */
    { "LINESTRING Z(0 0 1,6 6 1)", "LINESTRING Z(3 3 1,10 10 1)",
      "LINESTRING Z (0 0 1,6 6 1,10 10 1)" },
    /* the elevations agree and the measures do not, so the answer keeps the
     * one that is determined and drops the one that is not */
    { "POLYGON ZM((0 0 1 5,0 2 1 5,2 2 1 5,2 0 1 5,0 0 1 5))",
      "POLYGON ZM((1 1 1 9,1 3 1 9,3 3 1 9,3 1 1 9,1 1 1 9))",
      "POLYGON Z ((1 2 1,0 2 1,0 0 1,2 0 1,2 1 1,3 1 1,3 3 1,1 3 1,1 2 1))" },
    /* and the other way round */
    { "POLYGON ZM((0 0 1 5,0 2 1 5,2 2 1 5,2 0 1 5,0 0 1 5))",
      "POLYGON ZM((1 1 7 5,1 3 7 5,3 3 7 5,3 1 7 5,1 1 7 5))",
      "POLYGON M ((1 2 5,0 2 5,0 0 5,2 0 5,2 1 5,3 1 5,3 3 5,1 3 5,1 2 5))" },
  };
  for (size_t i = 0; i < sizeof(liftcases) / sizeof(liftcases[0]); i++)
  {
    GSERIALIZED *lf1 = geom_in(liftcases[i].first, -1);
    GSERIALIZED *lf2 = geom_in(liftcases[i].second, -1);
    assert(lf1 != NULL); assert(lf2 != NULL);
    GSERIALIZED *lfarr[2] = {lf1, lf2};
    meos_errno_reset();
    GSERIALIZED *lfu = geom_array_union(lfarr, 2);
    assert(lfu != NULL);
    assert(meos_errno() == 0);
    char *lfwkt = geo_as_text(lfu, 3);
    assert(lfwkt != NULL);
    printf("%s united with %s: %s\n", liftcases[i].first, liftcases[i].second,
      lfwkt);
    assert(strcmp(lfwkt, liftcases[i].result) == 0);
    free(lf1); free(lf2); free(lfu); free(lfwkt);
    meos_errno_reset();
  }

  /* An arc is a curve, and the lift along it is read by ANGLE rather than by
   * the chord joining its ends. These two discs are mirror images about the
   * line through the points where they cross, and each carries elevations
   * linear in the angle around its own centre, so the two determine the SAME
   * elevation at each crossing node -- 0.2301, the node's 41.41 degrees over
   * 180 -- and the answer carries it. Read along the chord, one disc gives
   * 0.209 there and the other 0.25: they would conflict and the answer would
   * come back flat, so its dimension is what this asserts */
  GSERIALIZED *dsc1 = geom_in(
    "CURVEPOLYGON(CIRCULARSTRING Z(0 0 1,2 2 0.5,4 0 0,2 -2 0.5,0 0 1))", -1);
  GSERIALIZED *dsc2 = geom_in(
    "CURVEPOLYGON(CIRCULARSTRING Z(3 0 0,5 2 0.5,7 0 1,5 -2 0.5,3 0 0))", -1);
  assert(dsc1 != NULL); assert(dsc2 != NULL);
  GSERIALIZED *dscarr[2] = {dsc1, dsc2};
  meos_errno_reset();
  GSERIALIZED *dscu = geom_array_union(dscarr, 2);
  assert(dscu != NULL);
  assert(meos_errno() == 0);
  char *dscwkt = geo_as_text(dscu, 4);
  printf("two mirrored discs elevated by the angle: %s\n", dscwkt);
  assert(strcmp(dscwkt,
    "CURVEPOLYGON Z (COMPOUNDCURVE Z ("
    "CIRCULARSTRING Z (3.5 1.3229 0.2301,1.2929 1.8708 0.615,0 0 1),"
    "CIRCULARSTRING Z (0 0 1,1.2929 -1.8708 0.615,3.5 -1.3229 0.2301),"
    "CIRCULARSTRING Z (3.5 -1.3229 0.2301,5.7071 -1.8708 0.615,7 0 1),"
    "CIRCULARSTRING Z (7 0 1,5.7071 1.8708 0.615,3.5 1.3229 0.2301)))") == 0);
  free(dsc1); free(dsc2); free(dscu); free(dscwkt);
  meos_errno_reset();

  /* The dissolve of ONE geometry reads its ordinates back the same way: its
   * components determine them where they agree and leave the answer planar
   * where they do not */
  GSERIALIZED *uz1 = geom_in(
    "MULTISURFACE(POLYGON Z((0 0 1,0 2 1,2 2 1,2 0 1,0 0 1)),"
    "POLYGON Z((1 1 1,1 3 1,3 3 1,3 1 1,1 1 1)))", -1);
  GSERIALIZED *uz2 = geom_in(
    "MULTISURFACE(POLYGON Z((0 0 1,0 2 1,2 2 1,2 0 1,0 0 1)),"
    "POLYGON Z((1 1 7,1 3 7,3 3 7,3 1 7,1 1 7)))", -1);
  assert(uz1 != NULL); assert(uz2 != NULL);
  GSERIALIZED *uzu1 = geom_unary_union(uz1, -1);
  GSERIALIZED *uzu2 = geom_unary_union(uz2, -1);
  assert(uzu1 != NULL); assert(uzu2 != NULL);
  assert(meos_errno() == 0);
  char *uzwkt1 = geo_as_text(uzu1, 3);
  char *uzwkt2 = geo_as_text(uzu2, 3);
  printf("the dissolve of two surfaces at one elevation: %s\n", uzwkt1);
  printf("the dissolve of two surfaces at two elevations: %s\n", uzwkt2);
  assert(strcmp(uzwkt1,
    "POLYGON Z ((1 2 1,0 2 1,0 0 1,2 0 1,2 1 1,3 1 1,3 3 1,1 3 1,1 2 1))")
    == 0);
  assert(strcmp(uzwkt2,
    "POLYGON((1 2,0 2,0 0,2 0,2 1,3 1,3 3,1 3,1 2))") == 0);
  free(uz1); free(uz2); free(uzu1); free(uzu2); free(uzwkt1); free(uzwkt2);
  meos_errno_reset();

  /* Which positions of a set are equal is read from their coordinates, on the
   * spheroid as on the plane, and an elevation one of them does not carry is
   * no more available there: the geodetic entry reads the shared dimensions
   * as the planar one does */
  GSERIALIZED *ggd1 = geog_in("POINT Z(1 1 1)", -1);
  GSERIALIZED *ggd2 = geog_in("POINT(2 2)", -1);
  assert(ggd1 != NULL); assert(ggd2 != NULL);
  GSERIALIZED *ggdarr[2] = {ggd1, ggd2};
  GSERIALIZED *ggdu = geog_array_union(ggdarr, 2);
  assert(ggdu != NULL);
  assert(meos_errno() == 0);
  char *ggdwkt = geo_as_ewkt(ggdu, 6);
  printf("the union of an elevated geography position with a flat one: %s\n",
    ggdwkt);
  assert(strcmp(ggdwkt, "SRID=4326;MULTIPOINT(1 1,2 2)") == 0);
  free(ggd1); free(ggd2); free(ggdu); free(ggdwkt);
  meos_errno_reset();

  /* Every base type #spatial_basetype admits renders in WKT. A cell index
   * value carries no coordinate, so the digits argument is vacuous and the
   * text is the cell identifier -- which is what the generic output function
   * writes for that same value, an identity independent of the session zone */
  meos_errno_reset();
  Temporal *cell = th3index_in("[831c02fffffffff@2001-01-01, "
    "880326b885fffff@2001-01-02]");
  assert(cell != NULL);
  char *cell_text = tspatial_as_text(cell, 6);
  char *cell_out = tspatial_out(cell, 6);
  assert(cell_text != NULL); assert(cell_out != NULL);
  assert(meos_errno() == 0);
  printf("the WKT of a temporal cell index: %s\n", cell_text);
  assert(strcmp(cell_text, cell_out) == 0);
  free(cell_text); free(cell_out); free(cell);
  meos_errno_reset();

  /* A set reads its elements through that same renderer, and a cell set
   * carries no timestamp, so the three grids are stated exactly */
  Set *h3s = h3index_to_set((H3Index) 0x831c02fffffffffULL);
  Set *qbs = quadbin_to_set((Quadbin) 5209574053332910079ULL);
  Set *s2s = s2cell_to_set((S2CellId) 0x47c3cULL);
  assert(h3s != NULL); assert(qbs != NULL); assert(s2s != NULL);
  char *h3t = spatialset_as_text(h3s, 6);
  char *qbt = spatialset_as_text(qbs, 6);
  char *s2t = spatialset_as_text(s2s, 6);
  assert(h3t != NULL); assert(qbt != NULL); assert(s2t != NULL);
  assert(meos_errno() == 0);
  printf("the WKT of a cell set: %s %s %s\n", h3t, qbt, s2t);
  assert(strcmp(h3t, "{\"831c02fffffffff\"}") == 0);
  assert(strcmp(qbt, "{\"484c1fffffffffff\"}") == 0);
  assert(strcmp(s2t, "{\"0000000000047c3c\"}") == 0);
  free(h3t); free(qbt); free(s2t);
  free(h3s); free(qbs); free(s2s);
  meos_errno_reset();

  /* A second surface lying clear of the first takes nothing away from what
   * the first covers, so a point interior to one member is interior to the
   * union of both. A single component answers without reading the union at
   * all, which makes its answer the reference the two-member spelling is held
   * to. Both members carry arcs at projected coordinates, where the point the
   * engine places on an edge misses it by the rounding unit of those
   * coordinates rather than by nothing */
  meos_errno_reset();
  GSERIALIZED *ubm = geom_in("CURVEPOLYGON(CIRCULARSTRING("
    "448320.7 6180616.3,448330.1 6180626.9,448340.3 6180616.7,"
    "448330.9 6180606.1,448320.7 6180616.3))", -1);
  GSERIALIZED *ubu = geom_in("MULTISURFACE(CURVEPOLYGON(CIRCULARSTRING("
    "448320.7 6180616.3,448330.1 6180626.9,448340.3 6180616.7,"
    "448330.9 6180606.1,448320.7 6180616.3)),CURVEPOLYGON(CIRCULARSTRING("
    "448400.3 6180616.1,448410.7 6180626.3,448420.9 6180616.9,"
    "448410.1 6180606.7,448400.3 6180616.1)))", -1);
  GSERIALIZED *ubp = geom_in("POINT(448330.5 6180616.5)", -1);
  assert(ubm != NULL); assert(ubu != NULL); assert(ubp != NULL);
  char *ubmm = geom_relate(ubm, ubp);
  char *ubum = geom_relate(ubu, ubp);
  assert(ubmm != NULL); assert(ubum != NULL);
  assert(meos_errno() == 0);
  printf("a point interior to one surface, alone and in the union: %s %s\n",
    ubmm, ubum);
  assert(strcmp(ubmm, "0F2FF1FF2") == 0);
  assert(strcmp(ubum, ubmm) == 0);
  /* and the predicates the matrix answers say the same */
  assert(geom_covers(ubu, ubp));
  assert(geom_intersects(ubu, ubp));
  assert(meos_errno() == 0);
  free(ubmm); free(ubum); free(ubm); free(ubu); free(ubp);
  meos_errno_reset();
  /* Whether a geometry is simple is a question about the curves it draws
   * meeting themselves, and an arc meets a curve along an arc as well as at
   * isolated points. A closed circular string passes through no point twice,
   * a compound curve crossing its own earlier segment does, and a multi-curve
   * asks the further question its straight twin asks -- that the members meet
   * only on their boundaries */
  meos_errno_reset();
  const char *simple_wkt[] = {
    "CIRCULARSTRING(0 0,2 2,4 0,2 -2,0 0)",
    "COMPOUNDCURVE((0 0,10 0),CIRCULARSTRING(10 0,5 2,0 0))",
    "CURVEPOLYGON(CIRCULARSTRING(0 0,2 2,4 0,2 -2,0 0))",
    "COMPOUNDCURVE((0 0,4 0),(4 0,2 2),(2 2,2 -2))",
    "MULTICURVE((0 0,4 4),(4 4,4 0),(4 0,0 4))",
  };
  const bool simple_exp[] = { true, true, true, false, false };
  for (int i = 0; i < 5; i++)
  {
    GSERIALIZED *sg = geom_in(simple_wkt[i], -1);
    assert(sg != NULL);
    meos_errno_reset();
    bool simple = geom_is_simple(sg);
    printf("isSimple(%.44s): %d\n", simple_wkt[i], simple);
    assert(meos_errno() == 0);
    assert(simple == simple_exp[i]);
    free(sg);
    meos_errno_reset();
  }

  /* The intersection and the difference of two geometries that are not both
   * polygonal. A point set meets another geometry where that geometry covers
   * a point of it; a line meets one along the stretches the segment kernels
   * answer, which read an arc of the other geometry exactly rather than as
   * the chain of chords a stroke would draw. The circle here has radius 2
   * about (2 2), so a line at y = 3 enters and leaves it at 2 -/+ sqrt(3) */
  meos_errno_reset();
  const char *clip_a[] = {
    "MULTIPOINT((1 1),(9 9),(3 3))",
    "LINESTRING(-2 2,6 2)",
    "LINESTRING(-2 3,6 3)",
  };
  const char *clip_b[] = {
    "POLYGON((0 0,4 0,4 4,0 4,0 0))",
    "POLYGON((0 0,4 0,4 4,0 4,0 0))",
    "CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2))",
  };
  const char *clip_inter[] = {
    "MULTIPOINT((1 1),(3 3))",
    "LINESTRING(0 2,4 2)",
    "LINESTRING(0.267949 3,3.732051 3)",
  };
  for (int i = 0; i < 3; i++)
  {
    GSERIALIZED *ca = geom_in(clip_a[i], -1);
    GSERIALIZED *cb = geom_in(clip_b[i], -1);
    assert(ca != NULL); assert(cb != NULL);
    meos_errno_reset();
    GSERIALIZED *ci = geom_intersection2d(ca, cb);
    assert(ci != NULL);
    assert(meos_errno() == 0);
    char *cw = geo_as_text(ci, 6);
    printf("intersection: %s\n", cw);
    assert(strcmp(cw, clip_inter[i]) == 0);
    free(cw); free(ci);
    /* The difference answers the rest of the first geometry, so the two
     * together give it back */
    meos_errno_reset();
    GSERIALIZED *cd = geom_difference2d(ca, cb);
    assert(cd != NULL);
    assert(meos_errno() == 0);
    free(cd); free(ca); free(cb);
    meos_errno_reset();
  }

  /* A geometry that is ITSELF a curve keeps its arcs through the clip, so the
   * pieces come back as arcs rather than as the chords a stroke would draw.
   * The circle is again the one of radius 2 about (2 2), and the clip cuts it
   * at y = 3, so the two pieces reach 2 -/+ sqrt(3) exactly. The second row
   * meets the clip at a single point, which no stretch of the arc reports */
  const char *arc_a[] = {
    "CIRCULARSTRING(0 2,2 4,4 2)",
    "CIRCULARSTRING(0 2,2 4,4 2)",
    "CIRCULARSTRING(0 2,2 4,4 2)",
    "CIRCULARSTRING(0 2,2 4,4 2)",
  };
  const char *arc_b[] = {
    "POLYGON((0 0,4 0,4 3,0 3,0 0))",
    "LINESTRING(2 0,2 5)",
    /* The quarter of the SAME circle running from (0 2) to (2 4), which the
     * subject runs ALONG rather than crosses */
    "CIRCULARSTRING(0 2.0000000000000004,"
      "0.58578643762690508 3.4142135623730949,2 4)",
    /* A clip the arc MEETS at a point BEFORE the stretch it runs inside, so
     * the two kinds of answer arrive in the opposite of their order along
     * the arc. The difference reads the gaps between them and a wrong order
     * makes those gaps overlap */
    "GEOMETRYCOLLECTION(LINESTRING(0.1 0,0.1 5),"
      "POLYGON((3 -1,5 -1,5 5,3 5,3 -1)))",
  };
  const char *arc_inter[] = {
    "MULTICURVE(CIRCULARSTRING(0 2,0.068148 2.517638,0.267949 3),"
      "CIRCULARSTRING(3.732051 3,3.931852 2.517638,4 2))",
    "POINT(2 4)",
    "CIRCULARSTRING(0 2,0.585786 3.414214,2 4)",
    "GEOMETRYCOLLECTION(CIRCULARSTRING(3 3.732051,3.732051 3,4 2),"
      "POINT(0.1 2.6245))",
  };
  const char *arc_diff[] = {
    "CIRCULARSTRING(0.267949 3,2 4,3.732051 3)",
    "MULTICURVE(CIRCULARSTRING(0 2,0.585786 3.414214,2 4),"
      "CIRCULARSTRING(2 4,3.414214 3.414214,4 2))",
    "CIRCULARSTRING(2 4,3.414214 3.414214,4 2)",
    "MULTICURVE(CIRCULARSTRING(0 2,0.025158 2.316228,0.1 2.6245),"
      "CIRCULARSTRING(0.1 2.6245,1.28644 3.868377,3 3.732051))",
  };
  for (int i = 0; i < 4; i++)
  {
    GSERIALIZED *aa = geom_in(arc_a[i], -1);
    GSERIALIZED *ab = geom_in(arc_b[i], -1);
    assert(aa != NULL); assert(ab != NULL);
    meos_errno_reset();
    GSERIALIZED *ai = geom_intersection2d(aa, ab);
    assert(ai != NULL);
    assert(meos_errno() == 0);
    char *aw = geo_as_text(ai, 6);
    printf("arc intersection: %s\n", aw);
    assert(strcmp(aw, arc_inter[i]) == 0);
    free(aw);
    meos_errno_reset();
    GSERIALIZED *ad = geom_difference2d(aa, ab);
    assert(ad != NULL);
    assert(meos_errno() == 0);
    aw = geo_as_text(ad, 6);
    printf("arc difference: %s\n", aw);
    assert(strcmp(aw, arc_diff[i]) == 0);
    free(aw);
    /* The clip PARTITIONS its subject, so the two pieces give back the length
     * the whole curve draws. That holds of the arcs themselves rather than of
     * a drawing of them, which is what the exact answer buys */
    double whole = geom_length(aa);
    double part = geom_length(ai) + geom_length(ad);
    printf("arc length: %.9f against %.9f\n", part, whole);
    assert(fabs(part - whole) < 1e-9);
    free(ai); free(ad); free(aa); free(ab);
    meos_errno_reset();
  }

  /* An overlay of a geometry the GEOS library cannot read answers NOTHING
   * rather than an empty geometry, and serializing what is absent reads a null
   * pointer. A polyhedral surface is the type it refuses, and the error the
   * refusal raises is what a caller reads the absence by. Under the handler
   * that EXITS these lines are unreachable, so only a binding meets them.
   * Each clip here COVERS AREA, which is what carries the pair to the overlay:
   * a clip of no area is answered from the subject and never reaches it */
  const char *ps = "POLYHEDRALSURFACE(((0 0,4 0,4 4,0 4,0 0)))";
  const char *ovl[] = {
    "POLYGON((1 1,2 1,2 2,1 2,1 1))",
    "POLYHEDRALSURFACE(((0 0,4 0,4 4,0 4,0 0)))",
  };
  for (int i = 0; i < 2; i++)
  {
    GSERIALIZED *pa = geom_in(ps, -1);
    GSERIALIZED *pb = geom_in(ovl[i], -1);
    assert(pa != NULL); assert(pb != NULL);
    meos_errno_reset();
    GSERIALIZED *pd = geom_difference2d(pa, pb);
    int ed = meos_errno();
    printf("difference of a polyhedral surface: %s, errno %d\n",
      pd ? "answered" : "nothing", ed);
    assert(pd == NULL);
    assert(ed != 0);
    if (pd) free(pd);
    meos_errno_reset();
    GSERIALIZED *pi = geom_intersection2d(pa, pb);
    int ei = meos_errno();
    printf("intersection of a polyhedral surface: %s, errno %d\n",
      pi ? "answered" : "nothing", ei);
    if (pi) free(pi);
    free(pa); free(pb);
    meos_errno_reset();
  }
  /* A region loses no area to a clip that covers none, so the difference is
   * the subject itself -- kept as the subject was WRITTEN, where the overlay
   * would return a curve as the chain of chords it reads it by and a triangle
   * as a polygon. The rule needs the subject to enclose something: a flat ring
   * is its own boundary, and the line it lies along removes all of it */
  const char *ar_subj[] = {
    "POLYGON((0 0,4 0,4 4,0 4,0 0))",
    "TRIANGLE((0 0,4 0,2 4,0 0))",
    "CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2))",
    /* A surface of one flat face bounds a region as much as a polygon does,
     * and it is the region the GEOS overlay refuses to read at all, so here
     * the rule answers where reaching the overlay has nothing to answer */
    "POLYHEDRALSURFACE(((0 0,4 0,4 4,0 4,0 0)))",
  };
  const char *ar_clip[] = {
    "POINT(2 2)",
    "LINESTRING(0 2,4 2)",
    "CIRCULARSTRING(0 2,2 4,4 2)",
  };
  for (int i = 0; i < 4; i++)
  {
    GSERIALIZED *as = geom_in(ar_subj[i], -1);
    assert(as != NULL);
    char *aw = geo_as_text(as, 6);
    for (int j = 0; j < 3; j++)
    {
      GSERIALIZED *ac = geom_in(ar_clip[j], -1);
      assert(ac != NULL);
      meos_errno_reset();
      GSERIALIZED *ad = geom_difference2d(as, ac);
      assert(ad != NULL);
      char *dw = geo_as_text(ad, 6);
      printf("%.28s minus %.24s: %s\n", ar_subj[i], ar_clip[j],
        strcmp(dw, aw) == 0 ? "the subject" : dw);
      assert(strcmp(dw, aw) == 0);
      free(dw); free(ad); free(ac);
      meos_errno_reset();
    }
    free(aw); free(as);
  }
  /* THE RULE IS ABOUT EVERY PART, NOT ABOUT A TOTAL. A subject mixing a
   * region with a ring that encloses no area is not a region throughout: the
   * ring traces the segment (3 0)-(5 0), so a clip covering that segment takes
   * it, and only the square is left. Reading a total area instead would answer
   * the subject unchanged and keep a segment the clip covers -- and would
   * answer one point set two ways, since the same members spelled as a
   * collection stay outside the guard and lose the segment */
  const char *pt_subj[] = {
    "MULTIPOLYGON(((0 0,1 0,1 1,0 1,0 0)),((3 0,5 0,5 0,3 0,3 0)))",
    "GEOMETRYCOLLECTION(POLYGON((0 0,1 0,1 1,0 1,0 0)),"
      "POLYGON((3 0,5 0,5 0,3 0,3 0)))",
  };
  for (int i = 0; i < 2; i++)
  {
    GSERIALIZED *ps1 = geom_in(pt_subj[i], -1);
    GSERIALIZED *ps2 = geom_in("LINESTRING(3 0,5 0)", -1);
    assert(ps1 != NULL); assert(ps2 != NULL);
    meos_errno_reset();
    GSERIALIZED *pr = geom_difference2d(ps1, ps2);
    assert(pr != NULL);
    double pa_area = geom_area(pr);
    char *pw = geo_as_text(pr, 6);
    printf("a subject carrying a part of no area, minus that part: %s\n", pw);
    /* The square alone: the part of no area is gone, and both spellings of
     * the same point set answer it the same way */
    assert(strstr(pw, "3 0") == NULL);
    assert(strstr(pw, "5 0") == NULL);
    assert(fabs(pa_area - 1.0) < 1e-12);
    free(pw); free(pr); free(ps1); free(ps2);
    meos_errno_reset();
  }
  /* A hole enclosing no area removes nothing from the surface it sits in, so
   * the subject IS a region and the rule holds of it: the answer is the
   * subject, of the subject's own area */
  GSERIALIZED *dh = geom_in("POLYGON((0 0,10 0,10 10,0 10,0 0),"
    "(3 5,7 5,7 5,3 5,3 5))", -1);
  GSERIALIZED *dl = geom_in("LINESTRING(3 5,7 5)", -1);
  assert(dh != NULL); assert(dl != NULL);
  meos_errno_reset();
  GSERIALIZED *dr = geom_difference2d(dh, dl);
  assert(dr != NULL);
  printf("a shell carrying a hole of no area, minus that hole's line: "
    "area %.6f\n", geom_area(dr));
  assert(fabs(geom_area(dr) - 100.0) < 1e-12);
  free(dr); free(dh); free(dl);
  meos_errno_reset();

  /* Only the DIFFERENCE of such a pair is answered from the subject. The
   * intersection of the same surface with the same line keeps its own arm,
   * which reads the line as the subject it clips and answers it, so the two
   * overlays part company on this pair and each says so here */
  GSERIALIZED *ps_s = geom_in("POLYHEDRALSURFACE(((0 0,4 0,4 4,0 4,0 0)))", -1);
  GSERIALIZED *ps_l = geom_in("LINESTRING(0 2,4 2)", -1);
  assert(ps_s != NULL); assert(ps_l != NULL);
  meos_errno_reset();
  GSERIALIZED *ps_i = geom_intersection2d(ps_s, ps_l);
  int ps_e = meos_errno();
  printf("intersection of a polyhedral surface and a line: %s, errno %d\n",
    ps_i ? "answered" : "nothing", ps_e);
  assert(ps_i != NULL);
  assert(ps_e == 0);
  free(ps_i); free(ps_s); free(ps_l);
  meos_errno_reset();
  /* A subject enclosing nothing is NOT one of them: the line it lies along
   * takes all of it, and the answer is the region of no area */
  GSERIALIZED *flat = geom_in("POLYGON((0 0,2 0,4 0,0 0))", -1);
  GSERIALIZED *along = geom_in("LINESTRING(0 0,4 0)", -1);
  assert(flat != NULL); assert(along != NULL);
  meos_errno_reset();
  GSERIALIZED *fd = geom_difference2d(flat, along);
  assert(fd != NULL);
  char *fw = geo_as_text(fd, 6);
  printf("a flat ring minus the line it lies along: %s\n", fw);
  assert(strstr(fw, "EMPTY") != NULL);
  free(fw); free(fd); free(flat); free(along);
  meos_errno_reset();

  /* An areal pair is overlaid on the circles its operands carry. The two
   * discs of radius 2 about (2,2) and (4,2) cross where both circles pass,
   * at x = 3 and y = 2 +- sqrt(3), and the answer names those points and
   * keeps the two arcs between them -- where reaching the GEOS overlay
   * strokes both circles first and answers the chords */
  const char *d1w = "CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2))";
  const char *d2w = "CURVEPOLYGON(CIRCULARSTRING(2 2,4 4,6 2,4 0,2 2))";
  const char *inw = "CURVEPOLYGON(CIRCULARSTRING(1 2,2 3,3 2,2 1,1 2))";
  const char *awayw = "POLYGON((100 100,101 100,101 101,100 101,100 100))";
  const char *ov_exp[] = {
    /* the lens the two discs share */
    "CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(3 3.732051,2.267949 3,2 2),"
      "CIRCULARSTRING(2 2,2.267949 1,3 0.267949),"
      "CIRCULARSTRING(3 0.267949,3.732051 1,4 2),"
      "CIRCULARSTRING(4 2,3.732051 3,3 3.732051)))",
    /* the first disc with that lens taken out of it */
    "CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(0 2,1 0.267949,3 0.267949),"
      "CIRCULARSTRING(3 0.267949,2.267949 1,2 2),"
      "CIRCULARSTRING(2 2,2.267949 3,3 3.732051),"
      "CIRCULARSTRING(3 3.732051,1 3.732051,0 2)))",
    /* a disc inside a disc: the intersection is the inner one ... */
    "CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(1 2,2 1,3 2),"
      "CIRCULARSTRING(3 2,2 3,1 2)))",
    /* ... and the difference is the outer one carrying it as a HOLE */
    "CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(0 2,2 0,4 2),"
      "CIRCULARSTRING(4 2,2 4,0 2)),"
      "COMPOUNDCURVE(CIRCULARSTRING(1 2,2 3,3 2),"
      "CIRCULARSTRING(3 2,2 1,1 2)))",
    /* a disc inside a disc, the other way round: nothing is left */
    "POLYGON EMPTY",
    /* and two regions that never meet share no area */
    "POLYGON EMPTY",
  };
  const char *ov_a[] = { d1w, d1w, d1w, d1w, inw, d1w };
  const char *ov_b[] = { d2w, d2w, inw, inw, d1w, awayw };
  const int ov_op[] =  { 0,   1,   0,   1,   1,   0 };
  for (int i = 0; i < 6; i++)
  {
    GSERIALIZED *oa = geom_in(ov_a[i], -1);
    GSERIALIZED *ob = geom_in(ov_b[i], -1);
    assert(oa != NULL); assert(ob != NULL);
    meos_errno_reset();
    GSERIALIZED *ores = ov_op[i] ? geom_difference2d(oa, ob) :
      geom_intersection2d(oa, ob);
    assert(ores != NULL);
    char *ow = geo_as_text(ores, 6);
    printf("areal overlay %d: %.60s\n", i, ow);
    assert(strcmp(ow, ov_exp[i]) == 0);
    free(ow); free(ores); free(oa); free(ob);
    meos_errno_reset();
  }
  /* THE REGION OF NO AREA IS SPELLED ONE WAY, whichever operand is empty and
   * whatever type it carries. That is the rule #clip_empty_areal states for
   * the polygonal arm -- what an empty result is drawn as follows from the
   * OPERATION, never from which trivial case reached it -- and an empty
   * CURVEPOLYGON operand is the one route by which the other spelling still
   * came back */
  const char *em_a[] = { "CURVEPOLYGON EMPTY", "POLYGON EMPTY",
    "CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2))", "POLYGON EMPTY" };
  const char *em_b[] = { "CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2))",
    "CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2))",
    "MULTISURFACE EMPTY", "CURVEPOLYGON EMPTY" };
  const int em_op[] = { 0, 0, 0, 1 };
  for (int i = 0; i < 4; i++)
  {
    GSERIALIZED *ea = geom_in(em_a[i], -1);
    GSERIALIZED *eb = geom_in(em_b[i], -1);
    assert(ea != NULL); assert(eb != NULL);
    meos_errno_reset();
    GSERIALIZED *er = em_op[i] ? geom_difference2d(ea, eb) :
      geom_intersection2d(ea, eb);
    assert(er != NULL);
    char *ew = geo_as_text(er, 6);
    printf("an empty areal operand answers: %s\n", ew);
    assert(strcmp(ew, "POLYGON EMPTY") == 0);
    free(ew); free(er); free(ea); free(eb);
    meos_errno_reset();
  }
  /* TWO SURFACES SHARING NO AREA STILL SHARE WHAT THEY MEET AT. Two discs
   * touching at one point share that point, and a point is of lower dimension
   * than the surfaces the arm assembles -- so the meeting is read off the
   * nodes the overlay already solved on the two circles, which is exact,
   * rather than answered as the region of no area, which would drop a point
   * set that is really there */
  GSERIALIZED *tg1 = geom_in("CURVEPOLYGON(CIRCULARSTRING(0 0,1 1,2 0,1 -1,"
    "0 0))", -1);
  GSERIALIZED *tg2 = geom_in("CURVEPOLYGON(CIRCULARSTRING(2 0,3 1,4 0,3 -1,"
    "2 0))", -1);
  assert(tg1 != NULL); assert(tg2 != NULL);
  meos_errno_reset();
  GSERIALIZED *tgi = geom_intersection2d(tg1, tg2);
  assert(tgi != NULL);
  char *tgw = geo_as_text(tgi, 6);
  printf("two discs touching at one point share: %s\n", tgw);
  assert(strcmp(tgw, "POINT(2 0)") == 0);
  free(tgw); free(tgi);
  /* The DIFFERENCE of the same pair takes a point set of no area from a
   * region, so it is the region -- and it keeps its arcs */
  meos_errno_reset();
  GSERIALIZED *tgd = geom_difference2d(tg1, tg2);
  assert(tgd != NULL);
  char *tgdw = geo_as_text(tgd, 6);
  printf("the first of them less the second: %.44s\n", tgdw);
  assert(strstr(tgdw, "CIRCULARSTRING") != NULL);
  free(tgdw); free(tgd); free(tg1); free(tg2);
  meos_errno_reset();
  /* AND THE POINT IS NOT A VERTEX ANY READING KEEPS BY LUCK. Turned off the
   * axes, the tangent point of the pair above is an ordinary point of both
   * arcs rather than a cardinal point of either circle, and a reading that
   * replaces an arc by chords loses it -- the chords of two circles touching
   * at such a point do not meet at all. Solved on the circles it is exactly
   * the midpoint of the segment joining the two centres, which is what this
   * asserts: radius 2 about the origin and about (2*sqrt(3), 2) touch at
   * (sqrt(3), 1) */
  GSERIALIZED *rt1 = geom_in("CURVEPOLYGON(CIRCULARSTRING(-2 0,0 2,2 0,0 -2,"
    "-2 0))", -1);
  GSERIALIZED *rt2 = geom_in("CURVEPOLYGON(CIRCULARSTRING("
    "1.4641016151377544 2,3.4641016151377544 4,5.4641016151377544 2,"
    "3.4641016151377544 0,1.4641016151377544 2))", -1);
  assert(rt1 != NULL); assert(rt2 != NULL);
  meos_errno_reset();
  GSERIALIZED *rti = geom_intersection2d(rt1, rt2);
  assert(rti != NULL);
  char *rtw = geo_as_text(rti, 6);
  printf("two discs touching off the axes share: %s\n", rtw);
  assert(strcmp(rtw, "POINT(1.732051 1)") == 0);
  free(rtw); free(rti); free(rt1); free(rt2);
  meos_errno_reset();
  /* THE MEETING IS NOT ALWAYS A POINT SET, AND THAT IS WHAT THE NODES CANNOT
   * SAY. Two half discs glued along their diameter meet along the whole of
   * it, and the nodes bounding that stretch are its two ENDS -- they state
   * where the meeting begins and ends, never what it draws. So the pair is
   * left to the route below rather than answered from them, and what this
   * asserts is that whatever comes back is not a point set */
  GSERIALIZED *hd1 = geom_in("CURVEPOLYGON(COMPOUNDCURVE("
    "CIRCULARSTRING(-2 0,0 2,2 0),(2 0,-2 0)))", -1);
  GSERIALIZED *hd2 = geom_in("CURVEPOLYGON(COMPOUNDCURVE("
    "CIRCULARSTRING(2 0,0 -2,-2 0),(-2 0,2 0)))", -1);
  assert(hd1 != NULL); assert(hd2 != NULL);
  meos_errno_reset();
  GSERIALIZED *hdi = geom_intersection2d(hd1, hd2);
  char *hdw = hdi ? geo_as_text(hdi, 6) : NULL;
  printf("two half discs sharing their diameter answer: %s\n",
    hdw ? hdw : "nothing");
  assert(hdw == NULL || strstr(hdw, "POINT") == NULL);
  if (hdw)
    free(hdw);
  if (hdi)
    free(hdi);
  free(hd1); free(hd2);
  meos_errno_reset();
  /* A BOUNDARY THE TWO SHARE IS PLACED, NOT DECLINED. Every piece of the
   * stretch they share lies on both boundaries, so which side each interior
   * occupies is what decides it: on the SAME side the two overlap and a union
   * and an intersection are bounded by it, on OPPOSITE sides they merely meet
   * and a difference is. A disc against itself shares the whole of its
   * boundary, and keeps its own circle rather than the chords the route below
   * would put there */
  const char *cw_a[] = {
    "CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2))",
    "POLYGON((0 0,2 0,2 2,0 2,0 0))",
    "POLYGON((0 0,2 0,2 2,0 2,0 0))",
  };
  const char *cw_b[] = {
    "CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2))",
    "POLYGON((2 0,4 0,4 2,2 2,2 0))",
    "POLYGON((2 0,4 0,4 2,2 2,2 0))",
  };
  const int cw_op[] = { 0, 0, 1 };
  const char *cw_exp[] = {
    /* the disc, on its own circle */
    "CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(0 2,2 0,4 2),"
      "CIRCULARSTRING(4 2,2 4,0 2)))",
    /* two squares meeting along an edge share no area ... */
    "POLYGON EMPTY",
    /* ... and neither takes any from the other */
    "POLYGON((2 2,2 0,0 0,0 2,2 2))",
  };
  for (int i = 0; i < 3; i++)
  {
    GSERIALIZED *ca = geom_in(cw_a[i], -1);
    GSERIALIZED *cb = geom_in(cw_b[i], -1);
    assert(ca != NULL); assert(cb != NULL);
    meos_errno_reset();
    GSERIALIZED *cr = cw_op[i] ? geom_difference2d(ca, cb) :
      geom_intersection2d(ca, cb);
    assert(cr != NULL);
    char *cwt = geo_as_text(cr, 6);
    printf("a shared boundary %d: %.58s\n", i, cwt);
    assert(strcmp(cwt, cw_exp[i]) == 0);
    free(cwt); free(cr); free(ca); free(cb);
    meos_errno_reset();
  }
  /* WHERE THE ANSWER LEAVES TWO REGIONS TOUCHING AT ONE POINT, four piece-ends
   * meet on that node and the walk takes the nearest piece round rather than
   * the first it finds: the two regions close as two rings, where the first it
   * finds closes one ring that visits the node twice and is not simple */
  GSERIALIZED *pz_a = geom_in("POLYGON((0 0,4 0,4 4,0 4,0 0))", -1);
  GSERIALIZED *pz_b = geom_in("TRIANGLE((0 0,4 0,2 4,0 0))", -1);
  assert(pz_a != NULL); assert(pz_b != NULL);
  meos_errno_reset();
  GSERIALIZED *pz = geom_difference2d(pz_a, pz_b);
  assert(pz != NULL);
  char *pzw = geo_as_text(pz, 6);
  printf("two regions touching at a point: %.46s\n", pzw);
  assert(strncmp(pzw, "MULTIPOLYGON", 12) == 0);
  assert(fabs(geom_area(pz) - 8.0) < 1e-12);
  free(pzw); free(pz); free(pz_a); free(pz_b);
  meos_errno_reset();

  /* A REGION WHOLLY INSIDE ANOTHER LEAVES NOTHING BEHIND, and it leaves
   * nothing behind whether or not the two boundaries touch on the way: what
   * they share along the touch is of no area, and a difference of regions does
   * not answer one. The pairs below all touch -- a disc inscribed in a square
   * meets it at four points, and a geometry against itself shares the whole of
   * its boundary -- so each is a difference that keeps no piece of either
   * boundary and covers nothing */
  const char *ins_a[] = {
    "TRIANGLE((0 0,4 0,2 4,0 0))",
    "CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2))",
    "CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2))",
    /* A multisurface is a region the route below declines to read at all, so
     * these two are answered where nothing answered them */
    "MULTISURFACE(CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2)),"
      "((3 3,4 3,4 4,3 4,3 3)))",
    "MULTISURFACE(CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2)),"
      "((3 3,4 3,4 4,3 4,3 3)))",
  };
  const char *ins_b[] = {
    "TRIANGLE((0 0,4 0,2 4,0 0))",
    "CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2))",
    "POLYGON((0 0,4 0,4 4,0 4,0 0))",
    "POLYGON((0 0,4 0,4 4,0 4,0 0))",
    "MULTISURFACE(CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2)),"
      "((3 3,4 3,4 4,3 4,3 3)))",
  };
  for (int i = 0; i < 5; i++)
  {
    GSERIALIZED *ia = geom_in(ins_a[i], -1);
    GSERIALIZED *ib = geom_in(ins_b[i], -1);
    assert(ia != NULL); assert(ib != NULL);
    meos_errno_reset();
    GSERIALIZED *ir = geom_difference2d(ia, ib);
    assert(ir != NULL);
    assert(meos_errno() == 0);
    char *iw = geo_as_text(ir, 6);
    printf("a region inside another leaves: %s\n", iw);
    assert(strcmp(iw, "POLYGON EMPTY") == 0);
    free(iw); free(ir); free(ia); free(ib);
    meos_errno_reset();
  }

  /* EVERY MEMBER OF A DISSOLVED COLLECTION IS A TYPE ITS READERS ANSWER.
   * A triangle draws a region a polygon draws, but @c lwmsurface_linearize
   * answers a CURVEPOLYGON and a POLYGON and has no arm for anything else, so
   * a triangle left in the collection leaves its slot of that reader's output
   * array unset and the collection built from it reads whatever the
   * allocation held. The union of surfaces that do not merge into one is
   * where such a collection is built, so a pair that stays apart is the
   * shortest way to ask for it */
  const char *tri_a[] = {
    "TRIANGLE((0 0,1 0,0 1,0 0))",
    "TRIANGLE((0 0,1 0,0 1,0 0))",
    "POLYGON((0 0,1 0,1 1,0 1,0 0))",
  };
  const char *tri_b[] = {
    "TRIANGLE((5 5,6 5,5 6,5 5))",
    "POLYGON((5 5,6 5,6 6,5 6,5 5))",
    "POLYGON((5 5,6 5,6 6,5 6,5 5))",
  };
  const double tri_area[] = { 1.0, 1.5, 2.0 };
  for (int i = 0; i < 3; i++)
  {
    GSERIALIZED *ta[2];
    ta[0] = geom_in(tri_a[i], -1);
    ta[1] = geom_in(tri_b[i], -1);
    assert(ta[0] != NULL); assert(ta[1] != NULL);
    meos_errno_reset();
    GSERIALIZED *tu = geom_array_union(ta, 2);
    assert(tu != NULL);
    char *tuw = geo_as_text(tu, 6);
    printf("surfaces that stay apart dissolve to: %.46s\n", tuw);
    /* Both parts are there, and every member is a polygon */
    assert(fabs(geom_area(tu) - tri_area[i]) < 1e-12);
    assert(strstr(tuw, "TRIANGLE") == NULL);
    free(tuw); free(tu); free(ta[0]); free(ta[1]);
    meos_errno_reset();
  }

  /* ONE REGION, TWO SPELLINGS, AND THEY MUST AGREE. A pair of POLYGONs is
   * overlaid by the Clipper2 arm; the same region spelled TRIANGLE reaches
   * the native one. Neither carries an arc, so the two answer the same
   * polygon and the areas are exact */
  const char *sp_tri = "TRIANGLE((0 0,4 0,2 4,0 0))";
  const char *sp_pol = "POLYGON((0 0,4 0,2 4,0 0))";
  const char *sp_cl  = "POLYGON((1 1,5 1,5 5,1 5,1 1))";
  for (int op = 0; op < 2; op++)
  {
    GSERIALIZED *ta = geom_in(sp_tri, -1);
    GSERIALIZED *pa2 = geom_in(sp_pol, -1);
    GSERIALIZED *cb2 = geom_in(sp_cl, -1);
    assert(ta != NULL); assert(pa2 != NULL); assert(cb2 != NULL);
    meos_errno_reset();
    GSERIALIZED *rn = op ? geom_difference2d(ta, cb2) :
      geom_intersection2d(ta, cb2);
    GSERIALIZED *rc = op ? geom_difference2d(pa2, cb2) :
      geom_intersection2d(pa2, cb2);
    assert(rn != NULL); assert(rc != NULL);
    double an = geom_area(rn), ac = geom_area(rc);
    printf("triangle and polygon spellings of one region, %s: %.12f %.12f\n",
      op ? "minus" : "meet", an, ac);
    assert(fabs(an - ac) < 1e-12);
    free(rn); free(rc); free(ta); free(pa2); free(cb2);
    meos_errno_reset();
  }

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
