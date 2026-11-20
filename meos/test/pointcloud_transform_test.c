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
 * @brief A program that tests the transformation of a point cloud value into
 * the schema of another pcid
 * @details A point cloud value takes its SRID from the schema its pcid names,
 * so it is transformed into another schema, which states the target SRID and
 * how the transformed coordinates are stored. The program registers a schema
 * in WGS84 and two in Web Mercator, transforms a point, a patch, a set, a
 * temporal value and a box between them, and checks the coordinates against
 * the Web Mercator formulas, the round trip back, and each refusal.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o pointcloud_transform_test pointcloud_transform_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>

/* The radius of the sphere Web Mercator projects */
#define WEBMERC_R 6378137.0

static double
webmerc_x(double lon)
{
  return WEBMERC_R * lon * M_PI / 180.0;
}

static double
webmerc_y(double lat)
{
  return WEBMERC_R * log(tan(M_PI / 4.0 + lat * M_PI / 360.0));
}

static double
point_x(const Pcpoint *pt)
{
  double x = 0.0;
  bool found = pcpoint_get_x(pt, &x);
  assert(found);
  return x;
}

static double
point_y(const Pcpoint *pt)
{
  double y = 0.0;
  bool found = pcpoint_get_y(pt, &y);
  assert(found);
  return y;
}

int main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* pcid 1: WGS84 longitude, latitude and height, stored as doubles */
  PCDimensionSpec lonlat[3] = {
    { "X", NULL, 1, "double", 1, 0, true },
    { "Y", NULL, 2, "double", 1, 0, true },
    { "Z", NULL, 3, "double", 1, 0, true }
  };
  assert(meos_pc_schema_register_dims(1, 4326, "none", lonlat, 3));
  /* pcid 2: Web Mercator metres, with an intensity the WGS84 schema lacks */
  PCDimensionSpec merc[4] = {
    { "X", NULL, 1, "double", 1, 0, true },
    { "Y", NULL, 2, "double", 1, 0, true },
    { "Z", NULL, 3, "double", 1, 0, true },
    { "Intensity", NULL, 4, "uint16_t", 1, 0, true }
  };
  assert(meos_pc_schema_register_dims(2, 3857, "none", merc, 4));
  /* pcid 3: Web Mercator metres stored as int16, too narrow for Brussels */
  PCDimensionSpec narrow[2] = {
    { "X", NULL, 1, "int16_t", 1, 0, true },
    { "Y", NULL, 2, "int16_t", 1, 0, true }
  };
  assert(meos_pc_schema_register_dims(3, 3857, "none", narrow, 2));
  /* pcid 4: WGS84 again, stored at a centimetre-like scale, the same SRID as
   * pcid 1 so a transformation only stores the coordinates again */
  PCDimensionSpec lonlat_scaled[2] = {
    { "X", NULL, 1, "int32_t", 0.0000001, 0, true },
    { "Y", NULL, 2, "int32_t", 0.0000001, 0, true }
  };
  assert(meos_pc_schema_register_dims(4, 4326, "none", lonlat_scaled, 2));

  /* A point in Brussels */
  double brussels[3] = { 4.35, 50.85, 10.0 };
  Pcpoint *pt = pcpoint_make(1, brussels, 3);
  assert(pt != NULL);

  /* Into Web Mercator: X and Y follow the projection, Z is kept and the
   * intensity the source has not is 0 */
  Pcpoint *pt_merc = pcpoint_transform(pt, 2);
  assert(pt_merc != NULL && meos_errno() == 0);
  assert(pcpoint_get_pcid(pt_merc) == 2);
  printf("Brussels in pcid 2: X %.3f Y %.3f\n", point_x(pt_merc),
    point_y(pt_merc));
  assert(fabs(point_x(pt_merc) - webmerc_x(4.35)) < 1e-3);
  assert(fabs(point_y(pt_merc) - webmerc_y(50.85)) < 1e-3);
  double z = 0.0, intensity = -1.0;
  bool zfound = pcpoint_get_z(pt_merc, &z);
  bool ifound = pcpoint_get_dim(pt_merc, "Intensity", &intensity);
  assert(zfound && z == 10.0);
  assert(ifound && intensity == 0.0);

  /* And back: the round trip returns the longitude and latitude */
  Pcpoint *pt_back = pcpoint_transform(pt_merc, 1);
  assert(pt_back != NULL);
  assert(fabs(point_x(pt_back) - 4.35) < 1e-9);
  assert(fabs(point_y(pt_back) - 50.85) < 1e-9);

  /* A schema stating the same SRID only stores the coordinates again */
  Pcpoint *pt_scaled = pcpoint_transform(pt, 4);
  assert(pt_scaled != NULL);
  assert(fabs(point_x(pt_scaled) - 4.35) < 1e-6);
  assert(fabs(point_y(pt_scaled) - 50.85) < 1e-6);

  /* A coordinate the target dimension cannot hold is refused rather than
   * clamped into it */
  Pcpoint *overflow = pcpoint_transform(pt, 3);
  printf("Into an int16 Web Mercator schema: %s, errno %d\n",
    overflow ? "transformed" : "refused", meos_errno());
  assert(overflow == NULL && meos_errno() != 0);
  meos_errno_reset();

  /* A target pcid naming no schema is refused */
  Pcpoint *unknown = pcpoint_transform(pt, 99);
  assert(unknown == NULL && meos_errno() != 0);
  meos_errno_reset();

  /* A patch transforms each of its points */
  double ghent[3] = { 3.72, 51.05, 5.0 };
  Pcpoint *pt2 = pcpoint_make(1, ghent, 3);
  const Pcpoint *pts[2] = { pt, pt2 };
  Pcpatch *pa = pcpatch_make(pts, 2);
  assert(pa != NULL);
  Pcpatch *pa_merc = pcpatch_transform(pa, 2);
  assert(pa_merc != NULL && pcpatch_npoints(pa_merc) == 2);
  assert(pcpatch_get_pcid(pa_merc) == 2);
  Pcpoint *pa_first = pcpatch_point_n(pa_merc, 1);
  assert(pa_first != NULL);
  assert(fabs(point_x(pa_first) - webmerc_x(4.35)) < 1e-3);

  /* A set transforms each of its values */
  Pcpoint *setvals[2] = { pt, pt2 };
  Set *s = pcpointset_make(setvals, 2);
  Set *s_merc = pointcloudset_transform(s, 2);
  assert(s_merc != NULL && set_num_values(s_merc) == 2);

  /* A temporal value transforms the value of each instant */
  TInstant *inst1 = tpcpointinst_make(pt, 0);
  TInstant *inst2 = tpcpointinst_make(pt2, 86400000000);
  TInstant *insts[2] = { inst1, inst2 };
  TSequence *seq = tsequence_make(insts, 2, true, true, STEP, true);
  assert(seq != NULL);
  Temporal *seq_merc = tpointcloud_transform((Temporal *) seq, 2);
  assert(seq_merc != NULL && temporal_num_instants(seq_merc) == 2);
  assert(tspatial_srid(seq_merc) == 3857);

  /* A box transforms its extent and takes the target pcid */
  TPCBox *box = pcpoint_to_tpcbox(pt);
  TPCBox *box_merc = tpcbox_transform(box, 2);
  assert(box_merc != NULL);
  assert(tpcbox_pcid(box_merc) == 2);
  assert(tpcbox_srid(box_merc) == 3857);
  double xmin = 0.0;
  bool xfound = tpcbox_xmin(box_merc, &xmin);
  assert(xfound);
  assert(fabs(xmin - webmerc_x(4.35)) < 1e-3);

  printf("Point cloud transformations: every check passed\n");

  free(pt); free(pt_merc); free(pt_back); free(pt_scaled); free(pt2);
  free(pa); free(pa_merc); free(pa_first); free(s); free(s_merc);
  free(inst1); free(inst2); free(seq); free(seq_merc);
  free(box); free(box_merc);
  meos_finalize();
  return 0;
}
