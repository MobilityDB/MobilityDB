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
  /* A geometry the edge decomposition does not reach reports the failure and
   * answers false, so the value a language binding reads is the safe one */
  GSERIALIZED *tin = geom_in(
    "Tin(((0 0,0 1,1 1,0 0)),((0 0,1 0,1 1,0 0)))", -1);
  assert(tin != NULL);
  meos_errno_reset();
  bool rel_tin = geom_relate_pattern(through, tin, patt);
  int errno_tin = meos_errno();
  printf("geom_relate_pattern(line, tin): %d, errno %d\n", rel_tin,
    errno_tin);
  assert(rel_tin == false);
  assert(errno_tin != 0);
  free(coll); free(away); free(through); free(tin);
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
