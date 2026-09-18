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
 * @brief A program that tests that MEOS refuses a NaN where a number enters
 * it: through text, through the generic constructors every temporal value, set
 * and span is built by, through the typed constructors that bypass them, and
 * as a coordinate of a point or a box.
 *
 * A NaN has no position among the numbers, so a value, a bound or a coordinate
 * holding one cannot be compared, bounded or indexed. Every input below must
 * raise an error and answer no value, and each call is paired with a valid one
 * of the same function that must be accepted, so that a refusal for any other
 * reason is not mistaken for the refusal of the NaN.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o nan_input_test nan_input_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

static int errors;
static int failures;

static void
count_error(int level, int code, const char *msg)
{
  (void) level; (void) code; (void) msg;
  errors++;
}

/* A NaN input must raise an error and answer no value */
static void
refused(const char *what, void *result)
{
  if (result || errors == 0)
  {
    printf("FAIL %s: expected the NaN to be refused\n", what);
    failures++;
  }
  errors = 0;
  meos_errno_reset();
  free(result);
}

/* The same function must accept a valid input */
static void
accepted(const char *what, void *result)
{
  if (! result || errors != 0)
  {
    printf("FAIL %s: expected the valid input to be accepted\n", what);
    failures++;
  }
  errors = 0;
  meos_errno_reset();
  free(result);
}

int
main(void)
{
  meos_initialize();
  meos_initialize_error_handler(count_error);

  /* Text */
  refused("tfloat_in", tfloat_in("NaN@2001-01-01"));
  accepted("tfloat_in", tfloat_in("1.5@2001-01-01"));
  refused("floatspan_in", floatspan_in("[NaN, 1]"));
  accepted("floatspan_in", floatspan_in("[0, 1]"));
  refused("floatset_in", floatset_in("{1, NaN}"));
  accepted("floatset_in", floatset_in("{1, 2}"));
  refused("tbox_in", tbox_in("TBOXFLOAT X([NaN,1])"));
  accepted("tbox_in", tbox_in("TBOXFLOAT X([0,1])"));
  refused("stbox_in", stbox_in("STBOX X((NaN,1),(2,2))"));
  accepted("stbox_in", stbox_in("STBOX X((1,1),(2,2))"));
  refused("tgeompoint_in", tgeompoint_in("POINT(NaN 1)@2001-01-01"));
  accepted("tgeompoint_in", tgeompoint_in("POINT(0 1)@2001-01-01"));
  refused("geomset_in", geomset_in("{POINT(NaN 1)}"));
  accepted("geomset_in", geomset_in("{POINT(0 1)}"));

  /* Constructors */
  refused("tfloatinst_make", tfloatinst_make(NAN, 0));
  accepted("tfloatinst_make", tfloatinst_make(1.5, 0));
  refused("floatspan_make", floatspan_make(NAN, 1, true, true));
  accepted("floatspan_make", floatspan_make(0, 1, true, true));
  double values[2] = {1, NAN};
  refused("floatset_make", floatset_make(values, 2));
  values[1] = 2;
  accepted("floatset_make", floatset_make(values, 2));
  refused("float_to_span", float_to_span(NAN));
  accepted("float_to_span", float_to_span(1));
  refused("float_to_tbox", float_to_tbox(NAN));
  accepted("float_to_tbox", float_to_tbox(1));
  refused("stbox_make", stbox_make(true, false, false, 0, NAN, 2, 1, 2, 0, 0,
    NULL));
  accepted("stbox_make", stbox_make(true, false, false, 0, 1, 2, 1, 2, 0, 0,
    NULL));
  refused("geompoint_make2d", geompoint_make2d(0, NAN, 1));
  accepted("geompoint_make2d", geompoint_make2d(0, 0, 1));

  /* A geometry holding a NaN coordinate */
  GSERIALIZED *nanpoint = geom_in("POINT(NaN 1)", -1);
  refused("tpointinst_make", tpointinst_make(nanpoint, 0));
  refused("geo_to_stbox", geo_to_stbox(nanpoint));
  free(nanpoint);

  /* Every coordinate of a point, read in place, and the points of any other
   * geometry, walked */
  static const char *nangeos[] = {"POINT(1 NaN)", "POINT Z(1 2 NaN)",
    "POINT M(1 2 NaN)", "POINT ZM(1 2 3 NaN)", "LINESTRING(0 0,1 NaN)"};
  static const char *geos[] = {"POINT(1 2)", "POINT Z(1 2 3)",
    "POINT M(1 2 3)", "POINT ZM(1 2 3 4)", "LINESTRING(0 0,1 1)"};
  for (int i = 0; i < 5; i++)
  {
    GSERIALIZED *nangeo = geom_in(nangeos[i], -1);
    GSERIALIZED *geo = geom_in(geos[i], -1);
    refused(nangeos[i], geo_to_stbox(nangeo));
    accepted(geos[i], geo_to_stbox(geo));
    free(nangeo); free(geo);
  }
  GSERIALIZED *nanpoint3d = geom_in("POINT Z(1 2 NaN)", -1);
  GSERIALIZED *point3d = geom_in("POINT Z(1 2 3)", -1);
  refused("tpointinst_make Z", tpointinst_make(nanpoint3d, 0));
  accepted("tpointinst_make Z", tpointinst_make(point3d, 0));
  free(nanpoint3d); free(point3d);

  if (failures == 0)
    printf("NaN input test: all tests passed\n");
  meos_finalize();
  return failures ? 1 : 0;
}
