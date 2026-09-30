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

/**
 * @file
 * @brief Test the grid sizes the spatial tiling functions read
 * @details A program that tests how the functions laying a spatial grid over
 * a value read the sizes of its dimensions, under the noexit error handler.
 *
 * A grid divides every spatial dimension of the value it is laid over. A
 * `ysize` or `zsize` of 0 is a size the caller leaves out and takes `xsize`,
 * a `zsize` is read only for a value with Z, and the value, never the origin,
 * states whether it has Z. Every size is a finite number that is not
 * negative, and a value with Z needs an origin with Z.
 *
 * The program verifies that each short form answers what the form repeating
 * `xsize` answers, for a box, a temporal point and a point, with and without
 * Z, and that an invalid size or origin is reported by setting #meos_errno.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o grid_sizes_test grid_sizes_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

static int failures = 0;

/* Report a check and count it when it fails */
static void
check(const char *what, bool ok)
{
  printf("%s: %s\n", ok ? "ok  " : "FAIL", what);
  if (! ok)
    failures++;
  meos_errno_reset();
}

/* Whether two arrays of boxes hold the same boxes in the same order */
static bool
same_boxes(const STBox *a, int na, const STBox *b, int nb)
{
  if (! a || ! b || na != nb || na == 0)
    return false;
  for (int i = 0; i < na; i++)
    if (! stbox_eq(&a[i], &b[i]))
      return false;
  return true;
}

/* Whether two arrays of fragments hold the same values in the same order */
static bool
same_fragments(Temporal **a, int na, Temporal **b, int nb)
{
  if (! a || ! b || na != nb || na == 0)
    return false;
  for (int i = 0; i < na; i++)
    if (! temporal_eq(a[i], b[i]))
      return false;
  return true;
}

/* The tiles of a box with the sizes given against those with xsize repeated */
static void
check_tiles(const char *what, const STBox *box, double x, double y, double z,
  double yfull, double zfull, const GSERIALIZED *orig)
{
  int n1 = 0, n2 = 0;
  STBox *t1 = stbox_space_tiles(box, x, y, z, orig, true, &n1);
  STBox *t2 = stbox_space_tiles(box, x, yfull, zfull, orig, true, &n2);
  check(what, meos_errno() == 0 && same_boxes(t1, n1, t2, n2));
  free(t1); free(t2);
}

/* The boxes of a temporal point with the sizes given against the full form */
static void
check_boxes(const char *what, const Temporal *temp, double x, double y,
  double z, double yfull, double zfull, const GSERIALIZED *orig)
{
  int n1 = 0, n2 = 0;
  STBox *b1 = tgeo_space_boxes(temp, x, y, z, orig, true, true, &n1);
  STBox *b2 = tgeo_space_boxes(temp, x, yfull, zfull, orig, true, true, &n2);
  check(what, meos_errno() == 0 && same_boxes(b1, n1, b2, n2));
  free(b1); free(b2);
}

/* The fragments of a temporal point with the sizes given against the full
 * form */
static void
check_split(const char *what, const Temporal *temp, double x, double y,
  double z, double yfull, double zfull, const GSERIALIZED *orig)
{
  int n1 = 0, n2 = 0;
  Temporal **f1 = tgeo_space_split(temp, x, y, z, orig, true, true, NULL, &n1);
  Temporal **f2 = tgeo_space_split(temp, x, yfull, zfull, orig, true, true,
    NULL, &n2);
  check(what, meos_errno() == 0 && same_fragments(f1, n1, f2, n2));
  for (int i = 0; f1 && i < n1; i++)
    free(f1[i]);
  for (int i = 0; f2 && i < n2; i++)
    free(f2[i]);
  free(f1); free(f2);
}

/* The tile of a point with the sizes given against the full form */
static void
check_tile(const char *what, const GSERIALIZED *pt, double x, double y,
  double z, double yfull, double zfull, const GSERIALIZED *orig)
{
  STBox *t1 = stbox_get_space_tile(pt, x, y, z, orig);
  STBox *t2 = stbox_get_space_tile(pt, x, yfull, zfull, orig);
  check(what, meos_errno() == 0 && t1 && t2 && stbox_eq(t1, t2));
  free(t1); free(t2);
}

/* A call that must be refused with an invalid argument value */
static void
check_refused(const char *what, const void *result)
{
  check(what, result == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  STBox *box2 = stbox_in("STBOX X((0,0),(10,10))");
  STBox *box3 = stbox_in("STBOX Z((0,0,0),(10,10,10))");
  Temporal *tpt2 = tgeompoint_in("[Point(0 0)@2001-01-01, "
    "Point(10 10)@2001-01-02]");
  Temporal *tpt3 = tgeompoint_in("[Point(0 0 0)@2001-01-01, "
    "Point(10 10 10)@2001-01-02]");
  GSERIALIZED *pt2 = geom_in("Point(3 4)", -1);
  GSERIALIZED *pt3 = geom_in("Point(3 4 5)", -1);
  GSERIALIZED *o2 = geom_in("Point(0 0)", -1);
  GSERIALIZED *o3 = geom_in("Point(0 0 0)", -1);
  if (! box2 || ! box3 || ! tpt2 || ! tpt3 || ! pt2 || ! pt3 || ! o2 || ! o3)
  {
    printf("FAIL: the inputs\n");
    return EXIT_FAILURE;
  }
  meos_errno_reset();

  /* A size left out takes xsize, and a zsize is read only with Z */
  check_tiles("stbox_space_tiles 2D, one size", box2, 2, 0, 0, 2, 2, o3);
  check_tiles("stbox_space_tiles 2D, zsize ignored", box2, 2, 3, 5, 3, 0, o3);
  check_tiles("stbox_space_tiles 3D, one size", box3, 2, 0, 0, 2, 2, o3);
  check_tiles("stbox_space_tiles 3D, two sizes", box3, 2, 3, 0, 3, 2, o3);
  check_boxes("tgeo_space_boxes 2D, one size", tpt2, 2, 0, 0, 2, 2, o3);
  check_boxes("tgeo_space_boxes 3D, one size", tpt3, 2, 0, 0, 2, 2, o3);
  check_boxes("tgeo_space_boxes 2D, size under one", tpt2, 0.5, 0, 0, 0.5,
    0.5, o3);
  check_split("tgeo_space_split 2D, one size", tpt2, 2, 0, 0, 2, 2, o3);
  check_split("tgeo_space_split 3D, two sizes", tpt3, 2, 3, 0, 3, 2, o3);
  check_tile("stbox_get_space_tile 2D, one size", pt2, 2, 0, 0, 2, 2, o3);
  check_tile("stbox_get_space_tile 3D, one size", pt3, 2, 0, 0, 2, 2, o3);

  /* A size that is negative, not a number or infinite is refused */
  int n = 0;
  check_refused("stbox_space_tiles, negative ysize",
    stbox_space_tiles(box2, 2, -1, 0, o3, true, &n));
  check_refused("tgeo_space_boxes, xsize not a number",
    tgeo_space_boxes(tpt2, NAN, 0, 0, o3, true, true, &n));
  check_refused("tgeo_space_split, infinite zsize",
    tgeo_space_split(tpt3, 2, 2, INFINITY, o3, true, true, NULL, &n));
  check_refused("stbox_get_space_tile, negative zsize",
    stbox_get_space_tile(pt3, 2, 2, -1, o3));

  /* A value with Z needs an origin with Z */
  check_refused("tgeo_space_boxes 3D, origin without Z",
    tgeo_space_boxes(tpt3, 2, 0, 0, o2, true, true, &n));
  check_refused("stbox_space_tiles 3D, origin without Z",
    stbox_space_tiles(box3, 2, 0, 0, o2, true, &n));

  free(box2); free(box3); free(tpt2); free(tpt3);
  free(pt2); free(pt3); free(o2); free(o3);
  meos_finalize();

  printf("%d check(s) failed\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
