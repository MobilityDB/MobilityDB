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
#include <float.h>
#include <limits.h>
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

  /* The position of a temporal point cloud point instant is compared with a
   * geometry, and a value of another temporal type, or a temporal point
   * cloud point that is not an instant, is refused with an error */
  GSERIALIZED *line = geom_in("Linestring(0 0, 3 3)", -1);
  Temporal *tgp = tgeompoint_in("[Point(1 1)@2001-01-01, "
    "Point(2 2)@2001-01-02]");
  Temporal *pcseq = temporal_in("{" TPCPOINT_IN ", "
    "010100000064000000C80000002C010000@2024-01-02}", T_TPCPOINT);
  assert(line != NULL && tgp != NULL && pcseq != NULL);
  double nad = nad_tpcpoint_geo(tpcpoint, line);
  bool inter = eintersects_tpcpoint_geo(tpcpoint, line);
  printf("nad_tpcpoint_geo: %g, eintersects_tpcpoint_geo: %d\n", nad, inter);
  assert(nad != DBL_MAX && meos_errno() == 0);
  const Temporal *refused[3] = { tgp, tpcpatch, pcseq };
  for (int i = 0; i < 3; i++)
  {
    assert(nad_tpcpoint_geo(refused[i], line) == DBL_MAX);
    assert(meos_errno() != 0);
    meos_errno_reset();
    assert(! eintersects_tpcpoint_geo(refused[i], line));
    assert(meos_errno() != 0);
    meos_errno_reset();
  }
  free(line); free(tgp); free(pcseq);

  /* The point (1,2,3) is to the left of, below, in front of and before a box
   * spanning 5 to 6 on each axis at a later time, the box answers the
   * converse, and the point only does not extend past itself */
  TPCBox *far = tpcbox_in("TPCBOX(ZT(((5,5,5),(6,6,6)),"
    "[2030-01-01,2030-01-02]), 1)");
  assert(far != NULL);
  struct {
    const char *op;
    bool (*box_temp)(const TPCBox *, const Temporal *);
    bool (*temp_box)(const Temporal *, const TPCBox *);
    bool (*temp_temp)(const Temporal *, const Temporal *);
    bool point_first, self;
  } posops[] = {
    { "left", left_tpcbox_tpointcloud, left_tpointcloud_tpcbox,
      left_tpointcloud_tpointcloud, true, false },
    { "overleft", overleft_tpcbox_tpointcloud, overleft_tpointcloud_tpcbox,
      overleft_tpointcloud_tpointcloud, true, true },
    { "right", right_tpcbox_tpointcloud, right_tpointcloud_tpcbox,
      right_tpointcloud_tpointcloud, false, false },
    { "overright", overright_tpcbox_tpointcloud, overright_tpointcloud_tpcbox,
      overright_tpointcloud_tpointcloud, false, true },
    { "below", below_tpcbox_tpointcloud, below_tpointcloud_tpcbox,
      below_tpointcloud_tpointcloud, true, false },
    { "overbelow", overbelow_tpcbox_tpointcloud, overbelow_tpointcloud_tpcbox,
      overbelow_tpointcloud_tpointcloud, true, true },
    { "above", above_tpcbox_tpointcloud, above_tpointcloud_tpcbox,
      above_tpointcloud_tpointcloud, false, false },
    { "overabove", overabove_tpcbox_tpointcloud, overabove_tpointcloud_tpcbox,
      overabove_tpointcloud_tpointcloud, false, true },
    { "front", front_tpcbox_tpointcloud, front_tpointcloud_tpcbox,
      front_tpointcloud_tpointcloud, true, false },
    { "overfront", overfront_tpcbox_tpointcloud, overfront_tpointcloud_tpcbox,
      overfront_tpointcloud_tpointcloud, true, true },
    { "back", back_tpcbox_tpointcloud, back_tpointcloud_tpcbox,
      back_tpointcloud_tpointcloud, false, false },
    { "overback", overback_tpcbox_tpointcloud, overback_tpointcloud_tpcbox,
      overback_tpointcloud_tpointcloud, false, true },
    { "before", before_tpcbox_tpointcloud, before_tpointcloud_tpcbox,
      before_tpointcloud_tpointcloud, true, false },
    { "overbefore", overbefore_tpcbox_tpointcloud,
      overbefore_tpointcloud_tpcbox, overbefore_tpointcloud_tpointcloud,
      true, true },
    { "after", after_tpcbox_tpointcloud, after_tpointcloud_tpcbox,
      after_tpointcloud_tpointcloud, false, false },
    { "overafter", overafter_tpcbox_tpointcloud, overafter_tpointcloud_tpcbox,
      overafter_tpointcloud_tpointcloud, false, true },
  };
  for (size_t i = 0; i < sizeof(posops) / sizeof(posops[0]); i++)
  {
    bool temp_box = posops[i].temp_box(tpcpoint, far);
    bool box_temp = posops[i].box_temp(far, tpcpoint);
    bool temp_temp = posops[i].temp_temp(tpcpoint, tpcpoint);
    printf("%s: point-box %d, box-point %d, point-point %d\n", posops[i].op,
      temp_box, box_temp, temp_temp);
    assert(temp_box == posops[i].point_first);
    assert(box_temp == ! posops[i].point_first);
    assert(temp_temp == posops[i].self);
    assert(meos_errno() == 0);
  }
  free(far);

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

  /* With no schema registered a box still writes its plain WKB and its EWKB,
   * as every spatial type does, and reads either back: reading a value needs
   * no catalog */
  size_t wkb_size, ewkb_size;
  uint8_t *wkb = tpcbox_as_wkb(box, 0, &wkb_size);
  uint8_t *ewkb = tpcbox_as_ewkb(box, 0, &ewkb_size);
  assert(wkb != NULL && ewkb != NULL);
  TPCBox *from_wkb = tpcbox_from_wkb(wkb, wkb_size);
  TPCBox *from_ewkb = tpcbox_from_wkb(ewkb, ewkb_size);
  assert(from_wkb != NULL && from_ewkb != NULL);
  char *from_wkb_out = tpcbox_out(from_wkb, 6);
  char *from_ewkb_out = tpcbox_out(from_ewkb, 6);
  printf("tpcbox_as_wkb read back with no schema: %s\n", from_wkb_out);
  printf("tpcbox_as_ewkb read back with no schema: %s\n", from_ewkb_out);
  assert(meos_errno() == 0);
  free(wkb); free(ewkb); free(from_wkb); free(from_ewkb);
  free(from_wkb_out); free(from_ewkb_out);

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

  /* A point and a patch are read in the schema their pcid names, so the
   * coordinates of a point and the SRID of a box are the ones that schema
   * states */
  double x, y, z, dim;
  assert(pcpoint_get_x(p3, &x) && pcpoint_get_y(p3, &y) &&
    pcpoint_get_z(p3, &z) && pcpoint_get_dim(p3, "Z", &dim));
  printf("pcpoint_get_x/y/z/dim: %g %g %g %g\n", x, y, z, dim);
  assert(x == 3 && y == 3 && z == 3 && dim == 3);
  TPCBox *ptbox = pcpoint_to_tpcbox(p1);
  TPCBox *pabox = pcpatch_to_tpcbox(pa);
  assert(ptbox != NULL && pabox != NULL);
  assert(tpcbox_srid(ptbox) == 4326 && tpcbox_srid(pabox) == 4326);
  free(ptbox); free(pabox);

  /* A box built from a pcid takes the SRID the schema of the pcid states;
   * pcid 0 names no schema and states SRID 0, and a box with no spatial
   * dimension states SRID 0 whatever its pcid */
  Span *period = tstzspan_in("[2024-01-01, 2024-01-02]");
  TPCBox *mkbox = tpcbox_make(true, false, false, 2, 1, 2, 1, 2, 0, 0,
    NULL);
  TPCBox *mkbox0 = tpcbox_make(true, false, false, 0, 1, 2, 1, 2, 0, 0,
    NULL);
  TPCBox *tbox7 = tpcbox_make(false, false, false, 7, 0, 0, 0, 0, 0, 0,
    period);
  assert(mkbox != NULL && mkbox0 != NULL && tbox7 != NULL);
  printf("tpcbox_make SRIDs: pcid 2 %d, pcid 0 %d, time only %d\n",
    tpcbox_srid(mkbox), tpcbox_srid(mkbox0), tpcbox_srid(tbox7));
  assert(tpcbox_srid(mkbox) == 4326 && tpcbox_srid(mkbox0) == 0 &&
    tpcbox_srid(tbox7) == 0);
  assert(meos_errno() == 0);
  free(mkbox); free(mkbox0); free(tbox7);

  /* A box with a spatial dimension built from a pcid naming no registered
   * schema is an error rather than a box with an assumed SRID */
  TPCBox *unknown = tpcbox_make(true, false, false, 7, 1, 2, 1, 2, 0, 0,
    NULL);
  printf("errno building a box of a pcid with no schema: %d\n", meos_errno());
  assert(unknown == NULL && meos_errno() != 0);
  meos_errno_reset();
  free(period);

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

  /* The pcid of a temporal value is the pcid of its values, and the point
   * counts of a temporal patch are those of its start and end values and
   * their sum over every instant, here a sequence of a patch of 2 points
   * then one of 3 */
  assert(tpcpoint_pcid(tpcpoint) == 1);
  assert(tpcpatch_pcid(tpcpatch) == 1);
  assert(tpcpatch_start_npoints(tpcpatch) == 2);
  assert(tpcpatch_end_npoints(tpcpatch) == 2);
  assert(tpcpatch_npoints(tpcpatch) == 2);
  assert(meos_errno() == 0);
  pts[0] = p1 = pcpoint_make(2, v1, 3);
  pts[1] = p2 = pcpoint_make(2, v2, 3);
  pts[2] = p3 = pcpoint_make(2, v3, 3);
  Pcpatch *pa2 = pcpatch_make(pts, 2);
  Pcpatch *pa3 = pcpatch_make(pts, 3);
  assert(pa2 != NULL && pa3 != NULL);
  TInstant *pinsts[2] = {
    tpcpatchinst_make(pa2, timestamptz_in("2024-01-01", -1)),
    tpcpatchinst_make(pa3, timestamptz_in("2024-01-02", -1))
  };
  TSequence *pseq = tsequence_make(pinsts, 2, true, true, STEP, false);
  assert(pseq != NULL);
  uint32_t seq_pcid = tpcpatch_pcid((Temporal *) pseq);
  uint32_t seq_start = tpcpatch_start_npoints((Temporal *) pseq);
  uint32_t seq_end = tpcpatch_end_npoints((Temporal *) pseq);
  int64 seq_total = tpcpatch_npoints((Temporal *) pseq);
  printf("tpcpatch sequence: pcid %u, start %u, end %u, total %ld points\n",
    seq_pcid, seq_start, seq_end, (long) seq_total);
  assert(seq_pcid == 2 && seq_start == 2 && seq_end == 3 && seq_total == 5);
  assert(meos_errno() == 0);

  /* A value of the other temporal point cloud type is refused */
  assert(tpcpoint_pcid(tpcpatch) == INT_MAX);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(tpcpatch_start_npoints(tpcpoint) == INT_MAX);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(tpcpatch_npoints(tpcpoint) == INT64_MAX);
  assert(meos_errno() != 0);
  meos_errno_reset();

  /* The pcid and the point counts are read from the values themselves, so
   * with no schema registered they are answered all the same */
  meos_pc_schema_clear();
  assert(meos_errno() == 0);
  assert(tpcpoint_pcid(tpcpoint) == 1);
  assert(tpcpatch_pcid((Temporal *) pseq) == 2);
  assert(tpcpatch_start_npoints((Temporal *) pseq) == 2);
  assert(tpcpatch_end_npoints((Temporal *) pseq) == 3);
  assert(tpcpatch_npoints((Temporal *) pseq) == 5);
  printf("errno reading the pcid and point counts with no schema "
    "registered: %d\n", meos_errno());
  assert(meos_errno() == 0);
  free(pinsts[0]); free(pinsts[1]); free(pseq);
  free(pa2); free(pa3); free(p1); free(p2); free(p3);

  free(tpcpoint); free(tpcpoint2); free(tpcpoint_out);
  free(tpcpatch); free(tpcpatch2); free(tpcpatch_out);
  free(box);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
