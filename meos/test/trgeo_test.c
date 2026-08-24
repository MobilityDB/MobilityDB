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
 * @brief A simple program that tests trgeometry_in(), the public MEOS entry
 * point for reading a temporal rigid geometry from its text representation
 *
 * A trgeometry value carries a leading reference geometry ahead of its
 * temporal (pose) part. This test exercises trgeometry_in() directly on a
 * well-formed value and checks that the result round-trips through
 * trgeometry_out(). It also exercises the restriction of a trgeometry to a
 * base value and to a set of base values, whose base values are poses.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o trgeo_test trgeo_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_pose.h>
#include <meos_rgeo.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS, using the noexit error handler so a parse failure
   * returns NULL instead of exiting the process */
  meos_initialize();
  meos_initialize_noexit_error_handler();
  meos_initialize_timezone("UTC");

  int result = 0;

  /* Well-formed trgeometry value: a reference geometry followed by the
   * ';'-delimited temporal (pose) part */
  const char *trgeo1_in =
    "Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 2),0.5)@2000-01-01";

  /* Temporal *trgeometry_in(const char *str); */
  Temporal *trgeo1 = trgeometry_in(trgeo1_in);
  if (! trgeo1)
  {
    printf("FAILED: trgeometry_in(%s) returned NULL\n", trgeo1_in);
    result = 1;
  }
  else
  {
    /* char *trgeometry_out(const Temporal *temp); */
    char *trgeo1_out = trgeometry_out(trgeo1);
    printf("trgeometry_in(%s): %s\n", trgeo1_in, trgeo1_out);

    /* Round-trip: parse the rendered representation again and check that
     * rendering it a second time yields the same string */
    Temporal *trgeo2 = trgeometry_in(trgeo1_out);
    if (! trgeo2)
    {
      printf("FAILED: trgeometry_in(%s) (round-trip) returned NULL\n",
        trgeo1_out);
      result = 1;
    }
    else
    {
      char *trgeo2_out = trgeometry_out(trgeo2);
      if (strcmp(trgeo1_out, trgeo2_out) != 0)
      {
        printf("FAILED: round-trip mismatch: %s != %s\n", trgeo1_out,
          trgeo2_out);
        result = 1;
      }
      else
        printf("OK: trgeometry_in/trgeometry_out round-trip matches\n");
      free(trgeo2); free(trgeo2_out);
    }
    free(trgeo1); free(trgeo1_out);
  }

  /* The base values of a trgeometry are poses, so it is restricted to a set
   * of poses */
  static const char *trgeo3_in =
    "Polygon((0 0,1 0,1 1,0 1,0 0));"
    "[Pose(Point(0 0),0)@2001-01-01, Pose(Point(10 0),0)@2001-01-02, "
    "Pose(Point(20 0),0)@2001-01-03]";
  static const char *at_expected =
    "POLYGON((0 0,1 0,1 1,0 1,0 0));"
    "{[Pose(POINT(10 0),0)@2001-01-02 00:00:00+00]}";

  Temporal *trgeo3 = trgeometry_in(trgeo3_in);
  Set *poseset1 = poseset_in("{\"Pose(Point(10 0),0)\"}");
  if (! trgeo3 || ! poseset1)
  {
    printf("FAILED: could not build the trgeometry and the poseset\n");
    result = 1;
  }
  else
  {
    /* Restricting to a pose the value takes keeps the instant at which it
     * takes it, with the reference geometry back on the result */
    meos_errno_reset();
    Temporal *at = trgeometry_at_values(trgeo3, poseset1);
    if (! at)
    {
      printf("FAILED: trgeometry_at_values returned NULL, errno %d\n",
        meos_errno());
      result = 1;
    }
    else
    {
      char *at_out = trgeometry_as_text(at, 6);
      if (strcmp(at_out, at_expected) != 0)
      {
        printf("FAILED: trgeometry_at_values: %s != %s\n", at_out,
          at_expected);
        result = 1;
      }
      else
        printf("OK: trgeometry_at_values(trgeometry, poseset): %s\n", at_out);
      free(at); free(at_out);
    }

    /* The complement keeps everything else, and the two together give back the
     * time of the value */
    meos_errno_reset();
    Temporal *minus = trgeometry_minus_values(trgeo3, poseset1);
    if (! minus)
    {
      printf("FAILED: trgeometry_minus_values returned NULL, errno %d\n",
        meos_errno());
      result = 1;
    }
    else
    {
      char *minus_out = trgeometry_as_text(minus, 6);
      printf("OK: trgeometry_minus_values(trgeometry, poseset): %s\n",
        minus_out);
      free(minus); free(minus_out);
    }

    /* A set of another base type is rejected, rather than reaching the
     * restriction and being rejected there */
    Set *geomset1 = geomset_in("{\"Point(0 0)\", \"Point(1 1)\"}");
    meos_errno_reset();
    Temporal *bad = trgeometry_at_values(trgeo3, geomset1);
    if (bad || meos_errno() == 0)
    {
      printf("FAILED: trgeometry_at_values accepted a geomset\n");
      result = 1;
    }
    else
      printf("OK: trgeometry_at_values(trgeometry, geomset) rejected, "
        "errno %d\n", meos_errno());
    free(bad); free(geomset1);
    meos_errno_reset();

    /* The singular restrictions take a base value, that is, a pose */
    Pose *pose1 = pose_in("Pose(Point(10 0),0)");
    meos_errno_reset();
    Temporal *at_value = trgeometry_at_value(trgeo3, pose1);
    if (! at_value)
    {
      printf("FAILED: trgeometry_at_value returned NULL, errno %d\n",
        meos_errno());
      result = 1;
    }
    else
    {
      char *at_value_out = trgeometry_as_text(at_value, 6);
      if (strcmp(at_value_out, at_expected) != 0)
      {
        printf("FAILED: trgeometry_at_value: %s != %s\n", at_value_out,
          at_expected);
        result = 1;
      }
      else
        printf("OK: trgeometry_at_value(trgeometry, pose): %s\n",
          at_value_out);
      free(at_value); free(at_value_out);
    }

    /* The complement keeps everything else */
    meos_errno_reset();
    Temporal *minus_value = trgeometry_minus_value(trgeo3, pose1);
    if (! minus_value)
    {
      printf("FAILED: trgeometry_minus_value returned NULL, errno %d\n",
        meos_errno());
      result = 1;
    }
    else
    {
      char *minus_value_out = trgeometry_as_text(minus_value, 6);
      printf("OK: trgeometry_minus_value(trgeometry, pose): %s\n",
        minus_value_out);
      free(minus_value); free(minus_value_out);
    }

    /* A temporal value of another type is rejected */
    Temporal *tpose1 = trgeometry_to_tpose(trgeo3);
    meos_errno_reset();
    Temporal *bad_value = trgeometry_at_value(tpose1, pose1);
    if (bad_value || meos_errno() == 0)
    {
      printf("FAILED: trgeometry_at_value accepted a temporal pose\n");
      result = 1;
    }
    else
      printf("OK: trgeometry_at_value(tpose, pose) rejected, errno %d\n",
        meos_errno());
    free(bad_value); free(tpose1); free(pose1);
    meos_errno_reset();
  }
  free(trgeo3); free(poseset1);

  /* The nearest approach distance answers the sentinel of its return type
   * where the temporal distance underneath it is not computed, rather than
   * reading a value that is not there. Three inputs reach that: a geometry
   * whose type the distance does not support, and a three-dimensional body
   * against a point and against a box, which the planar computation declines */
  static const char *trgeo4_in =
    "Polygon((0 0,1 0,1 1,0 1,0 0));"
    "[Pose(Point(0 0),0)@2001-01-01, Pose(Point(2 0),0)@2001-01-02]";
  static const char *trgeo5_in =
    "PolyhedralSurface(((0 0 0,0 1 0,1 1 0,1 0 0,0 0 0)));"
    "[Pose(Point(0 0 0),1,0,0,0)@2001-01-01, Pose(Point(2 0 0),1,0,0,0)@2001-01-02]";

  Temporal *trgeo4 = trgeometry_in(trgeo4_in);
  Temporal *trgeo5 = trgeometry_in(trgeo5_in);
  GSERIALIZED *coll = geom_in("GeometryCollection(Point(9 9))", -1);
  GSERIALIZED *point3d = geom_in("Point(9 9 9)", -1);
  GSERIALIZED *point2d = geom_in("Point(9 9)", -1);
  STBox *box3d = stbox_in("STBOX ZT(((5,5,5),(6,6,6)),[2001-01-01, 2001-01-02])");
  if (! trgeo4 || ! trgeo5 || ! coll || ! point3d || ! point2d || ! box3d)
  {
    printf("FAILED: could not build the nearest approach distance arguments\n");
    result = 1;
  }
  else
  {
    meos_errno_reset();
    if (nad_trgeometry_geo(trgeo4, coll) != DBL_MAX || meos_errno() == 0)
    {
      printf("FAILED: nad_trgeometry_geo(trgeometry, collection)\n");
      result = 1;
    }
    else
      printf("OK: nad_trgeometry_geo(trgeometry, collection) answers the "
        "sentinel, errno %d\n", meos_errno());

    meos_errno_reset();
    if (nad_trgeometry_geo(trgeo5, point3d) != DBL_MAX || meos_errno() == 0)
    {
      printf("FAILED: nad_trgeometry_geo(3D trgeometry, 3D point)\n");
      result = 1;
    }
    else
      printf("OK: nad_trgeometry_geo(3D trgeometry, 3D point) answers the "
        "sentinel, errno %d\n", meos_errno());

    meos_errno_reset();
    if (nad_trgeometry_stbox(trgeo5, box3d) != DBL_MAX || meos_errno() == 0)
    {
      printf("FAILED: nad_trgeometry_stbox(3D trgeometry, 3D box)\n");
      result = 1;
    }
    else
      printf("OK: nad_trgeometry_stbox(3D trgeometry, 3D box) answers the "
        "sentinel, errno %d\n", meos_errno());

    /* A computable distance still answers it */
    meos_errno_reset();
    double nad = nad_trgeometry_geo(trgeo4, point2d);
    if (nad == DBL_MAX || meos_errno() != 0)
    {
      printf("FAILED: nad_trgeometry_geo(trgeometry, point) answered the "
        "sentinel, errno %d\n", meos_errno());
      result = 1;
    }
    else
      printf("OK: nad_trgeometry_geo(trgeometry, point): %g\n", nad);
    meos_errno_reset();
  }
  free(trgeo4); free(trgeo5); free(coll); free(point3d); free(point2d);
  free(box3d);

  /* The constructor accepts a polyhedral surface as a reference geometry, and
   * the closest-feature walk the sequence kernels run reads the body as a
   * polygon: the cast answered NULL and the walk read it, which ended the
   * process.  A pair it cannot walk is reported instead */
  {
    Temporal *psurf = trgeometry_in("PolyhedralSurface(((0 0,0 1,1 1,1 0,"
      "0 0)));[Pose(Point(0 0),0)@2001-01-01, Pose(Point(2 0),0.5)@2001-01-02]");
    GSERIALIZED *away = geom_in("Point(9 9)", -1);
    if (! psurf || ! away)
    {
      printf("FAILED: the polyhedral surface witness did not parse\n");
      result = 1;
    }
    else
    {
      meos_errno_reset();
      Temporal *dist = tdistance_trgeometry_geo(psurf, away);
      if (dist != NULL || meos_errno() == 0)
      {
        printf("FAILED: tdistance_trgeometry_geo(polyhedral surface, point) "
          "answered, errno %d\n", meos_errno());
        result = 1;
      }
      else
        printf("OK: tdistance_trgeometry_geo(polyhedral surface, point) is "
          "reported, errno %d\n", meos_errno());
      /* The instant path needs no polygon -- it places the body and measures
       * it whole -- so it must keep answering */
      meos_errno_reset();
      Temporal *inst = trgeometry_in("PolyhedralSurface(((0 0,0 1,1 1,1 0,"
        "0 0)));Pose(Point(0 0),0)@2001-01-01");
      Temporal *idist = inst ? tdistance_trgeometry_geo(inst, away) : NULL;
      if (! idist || meos_errno() != 0)
      {
        printf("FAILED: the instant path stopped answering, errno %d\n",
          meos_errno());
        result = 1;
      }
      else
        printf("OK: the instant path still measures a polyhedral body\n");
      free(idist); free(inst);
    }
    free(psurf); free(away);
    meos_errno_reset();
  }

  /* Finalize MEOS */
  meos_finalize();

  return result;
}
