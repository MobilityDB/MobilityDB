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
 * @brief A program that tests the temporal point cloud types in a program that
 * states the pgPointCloud schema of the pcid its values name itself
 * @details A pcpoint and a pcpatch carry a pcid and nothing else about their
 * layout, and the schema that pcid resolves to lives in a catalog table only
 * a PostgreSQL backend can scan. A standalone program has neither that
 * catalog nor, until it registers one, any schema at all, which is the state
 * every binding starts in. The text of a value is the pgPointCloud
 * Well-Known Binary (WKB) of its points, whose data the schema lays out, so
 * the program checks that a value is not read before its schema is
 * registered, as the type input function of pgPointCloud refuses it, that it
 * is read and written back once the schema is registered, and that a
 * question that must decode a coordinate reports a schema that is gone.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tpointcloud_test tpointcloud_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>

/* A pcpoint of pcid 1 holding X=1.0 Y=2.0 Z=3.0, and a pcpatch of pcid 1
 * holding the two points (1,1,1) and (2,2,2), each at one timestamp. Both
 * are the hex WKB the type output functions of pgPointCloud write, the data
 * laid out by schema 1 of meos/src/pointcloud/pointcloud_schemas.xml: three
 * int32 dimensions at scale 0.01 */
#define TPCPOINT_IN \
  "010100000064000000C80000002C010000@2024-01-01"
#define TPCPATCH_IN \
  "01010000000000000002000000640000006400000064000000C8000000C8000000" \
  "C8000000@2024-01-01"

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting, the handler every binding uses */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* No schema is registered and no hook is installed: the state a program
   * outside a PostgreSQL backend starts in */
  meos_pc_schema_clear();
  assert(meos_errno() == 0);

  /* The data of a value is laid out by the schema of its pcid, so with no
   * schema registered a value is not read */
  Temporal *unread = temporal_in(TPCPOINT_IN, T_TPCPOINT);
  printf("errno reading a value with no schema registered: %d\n",
    meos_errno());
  assert(unread == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();

  /* With the schema of pcid 1 registered, a temporal point cloud point is
   * read and written back to the text it was read from */
  PCDimensionSpec dims1[3] = {
    { "X", NULL, 1, "int32_t", 0.01, 0, true },
    { "Y", NULL, 2, "int32_t", 0.01, 0, true },
    { "Z", NULL, 3, "int32_t", 0.01, 0, true }
  };
  assert(meos_pc_schema_register_dims(1, 0, "none", dims1, 3));
  Temporal *tpcpoint = temporal_in(TPCPOINT_IN, T_TPCPOINT);
  assert(tpcpoint != NULL);
  assert(meos_errno() == 0);
  char *tpcpoint_out = temporal_out(tpcpoint, 15);
  printf("tpcpoint: %s\n", tpcpoint_out);
  Temporal *tpcpoint2 = temporal_in(tpcpoint_out, T_TPCPOINT);
  assert(tpcpoint2 != NULL && temporal_eq(tpcpoint, tpcpoint2));

  /* And so is a temporal point cloud patch */
  Temporal *tpcpatch = temporal_in(TPCPATCH_IN, T_TPCPATCH);
  assert(tpcpatch != NULL);
  assert(meos_errno() == 0);
  char *tpcpatch_out = temporal_out(tpcpatch, 15);
  printf("tpcpatch: %s\n", tpcpatch_out);
  Temporal *tpcpatch2 = temporal_in(tpcpatch_out, T_TPCPATCH);
  assert(tpcpatch2 != NULL && temporal_eq(tpcpatch, tpcpatch2));

  /* The schema is cleared again, the state a value read earlier meets when
   * its schema is gone */
  meos_pc_schema_clear();
  assert(meos_errno() == 0);

  /* The reference system of a point cloud value is stated by its schema, so
   * with none registered the value reports the SRID that names none, and
   * reporting it is not itself an error */
  int32_t srid = tspatial_srid(tpcpoint);
  printf("SRID with no schema registered: %d\n", srid);
  assert(meos_errno() == 0);

  /* A question that must decode a coordinate reports the missing schema
   * rather than answering from one that does not exist */
  TPCBox *box = tpcbox_in("TPCBOX(XT(((1,1),(3,3)),[2024-01-01,2024-01-02]), 1)");
  assert(box != NULL);
  meos_errno_reset();
  (void) same_tpointcloud_tpcbox(tpcpoint, box);
  printf("errno after a question needing the schema: %d\n", meos_errno());
  assert(meos_errno() != 0);
  meos_errno_reset();

  /* The extent of two boxes of one schema expands a copy of the first */
  TPCBox *box2 = tpcbox_in("TPCBOX(XT(((2,2),(5,5)),[2024-01-02,2024-01-03]), 1)");
  TPCBox *ext = tpcbox_extent_transfn(NULL, box);
  ext = tpcbox_extent_transfn(ext, box2);
  assert(ext != NULL);
  char *ext_out = tpcbox_out(ext, 6);
  printf("tpcbox_extent_transfn: %s\n", ext_out);
  assert(meos_errno() == 0);
  free(ext); free(ext_out); free(box2);

  /* With a schema registered, a patch built from its points answers which
   * point stands at a given position */
  PCDimensionSpec dims[3] = {
    { "X", NULL, 1, "int32_t", 1, 0, true },
    { "Y", NULL, 2, "int32_t", 1, 0, true },
    { "Z", NULL, 3, "int32_t", 1, 0, true }
  };
  assert(meos_pc_schema_register_dims(2, 4326, "none", dims, 3));
  double v1[3] = {1, 1, 1}, v2[3] = {2, 2, 2}, v3[3] = {3, 3, 3};
  Pcpoint *p1 = pcpoint_make(2, v1, 3);
  Pcpoint *p2 = pcpoint_make(2, v2, 3);
  Pcpoint *p3 = pcpoint_make(2, v3, 3);
  assert(p1 != NULL && p2 != NULL && p3 != NULL);
  const Pcpoint *pts[3] = { p1, p2, p3 };
  Pcpatch *pa = pcpatch_make(pts, 3);
  assert(pa != NULL);
  assert(pcpatch_npoints(pa) == 3);

  /* The position is one-based, and a negative one counts from the end, which
   * is the indexing pgPointCloud defines for PC_PointN */
  Pcpoint *first = pcpatch_point_n(pa, 1);
  Pcpoint *last = pcpatch_point_n(pa, 3);
  Pcpoint *last_from_end = pcpatch_point_n(pa, -1);
  Pcpoint *first_from_end = pcpatch_point_n(pa, -3);
  assert(first != NULL && last != NULL);
  assert(last_from_end != NULL && first_from_end != NULL);
  assert(pcpoint_eq(first, p1));
  assert(pcpoint_eq(last, p3));
  assert(pcpoint_eq(last_from_end, last));
  assert(pcpoint_eq(first_from_end, first));

  /* A position addressing no point of the patch is answered with NULL, and
   * asking for one is not an error */
  assert(pcpatch_point_n(pa, 0) == NULL);
  assert(pcpatch_point_n(pa, 4) == NULL);
  assert(pcpatch_point_n(pa, -4) == NULL);
  assert(meos_errno() == 0);

  /* The array form holds the same points in the same order */
  int count;
  Pcpoint **points = pcpatch_points(pa, &count);
  assert(points != NULL);
  assert(count == (int) pcpatch_npoints(pa));
  for (int i = 0; i < count; i++)
  {
    Pcpoint *nth = pcpatch_point_n(pa, i + 1);
    assert(pcpoint_eq(points[i], nth));
    free(nth); free(points[i]);
  }
  printf("a patch of %d points answers each of them by its position\n", count);
  free(points);
  free(first); free(last); free(last_from_end); free(first_from_end);
  free(p1); free(p2); free(p3); free(pa);

  /* The point count and density aggregates sum the patches instant by
   * instant */
  SkipList *npstate = tpcpatch_tnpoints_transfn(NULL, tpcpatch);
  npstate = tpcpatch_tnpoints_transfn(npstate, tpcpatch2);
  Temporal *np = temporal_tagg_finalfn(npstate);
  assert(np != NULL);
  char *np_out = temporal_out(np, 6);
  printf("tpcpatch_tnpoints_transfn: %s\n", np_out);
  SkipList *dstate = tpcpatch_tdensity_transfn(NULL, tpcpatch);
  Temporal *dens = temporal_tagg_finalfn(dstate);
  assert(dens != NULL);
  char *dens_out = temporal_out(dens, 6);
  printf("tpcpatch_tdensity_transfn: %s\n", dens_out);
  assert(meos_errno() == 0);
  free(np); free(np_out); free(dens); free(dens_out);

  free(tpcpoint); free(tpcpoint2); free(tpcpoint_out);
  free(tpcpatch); free(tpcpatch2); free(tpcpatch_out);
  free(box);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
