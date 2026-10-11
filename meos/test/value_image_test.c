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
 * @brief Test the size and the copy of the flat memory image of every value
 * type
 * @details A binding can carry a MEOS value as its flat memory image: it
 * writes the number of bytes the value states with its `_mem_size` function
 * and reads them back into a value with its `_copy` function. A value of a
 * fixed-size type has the size of its structure.
 *
 * The program verifies, for a value of each type, that the image of the size
 * the value states is the whole value: the bytes copied out of it answer
 * equal to the value, the copy of those bytes holds the same bytes and states
 * the same size, and a null argument is reported by answering the error value
 * each function documents and setting #meos_errno.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o value_image_test value_image_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_cbuffer.h>
#include <meos_geo.h>
#include <meos_json.h>
#include <meos_npoint.h>
#include <meos_pointcloud.h>
#include <meos_pose.h>
#include <meos_raster.h>

/* A 3x3 raster with a single 32BF band */
static const char *raster_one_band =
  "0100000100000000000000f03f000000000000f0bf000000000000000000000000000008"
  "4000000000000000000000000000000000e6100000030003000a00000000000000000000"
  "000000000000000000000000000000000000000000000000000000000000";

/**
 * @brief Check the image of a value of a variable-length type
 * @details The image of the size the value states is copied out into a buffer
 * as a binding writes it, and the buffer is read back with the copy function
 * as a binding reads it. Both the buffer and its copy answer equal to the
 * value, the copy holds the bytes of the value and states the same size, and
 * a null argument answers INT_MAX and NULL with #meos_errno set
 */
#define CHECK_IMAGE(TYPE, NAME, value, mem_size, copy, eq)                   \
  do {                                                                       \
    int size = mem_size(value);                                              \
    assert(size > 0 && size != INT_MAX);                                     \
    TYPE *image = malloc((size_t) size);                                     \
    memcpy(image, value, (size_t) size);                                     \
    TYPE *result = copy(image);                                              \
    printf("%s: %d bytes, image equal %s, copy equal %s\n", NAME, size,      \
      eq(value, image) ? "true" : "false",                                   \
      eq(value, result) ? "true" : "false");                                 \
    assert(eq(value, image));                                                \
    assert(eq(value, result));                                               \
    assert(memcmp(result, value, (size_t) size) == 0);                       \
    assert(mem_size(result) == size);                                        \
    assert(meos_errno() == 0);                                               \
    free(image); free(result);                                               \
    assert(mem_size(NULL) == INT_MAX);                                       \
    assert(meos_errno() == MEOS_ERR_INVALID_ARG);                            \
    meos_errno_reset();                                                      \
    assert(copy(NULL) == NULL);                                              \
    assert(meos_errno() == MEOS_ERR_INVALID_ARG);                            \
    meos_errno_reset();                                                      \
  } while (0)

/* Equality of two JSON paths, which have no comparison function, read as the
 * equality of their string representations */
static bool
jsonpath_same(const JsonPath *jp1, const JsonPath *jp2)
{
  char *str1 = jsonpath_out(jp1);
  char *str2 = jsonpath_out(jp2);
  bool result = strcmp(str1, str2) == 0;
  free(str1); free(str2);
  return result;
}

/* Equality of two rasters, which have no comparison function, read as the
 * equality of their HexWKB representations */
static bool
raster_same(const Raster *rast1, const Raster *rast2)
{
  size_t size1, size2;
  char *hex1 = raster_as_hexwkb(rast1, 0, &size1);
  char *hex2 = raster_as_hexwkb(rast2, 0, &size2);
  bool result = size1 == size2 && strcmp(hex1, hex2) == 0;
  free(hex1); free(hex2);
  return result;
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* Temporal values, sets and span sets */
  Temporal *temp = tfloat_in("{[1@2001-01-01, 2@2001-01-02], "
    "[3@2001-01-03, 1@2001-01-05]}");
  Set *set = intset_in("{1, 2, 3}");
  SpanSet *spanset = intspanset_in("{[1, 2], [4, 5]}");
  assert(temp && set && spanset);
  CHECK_IMAGE(Temporal, "temporal", temp, temporal_mem_size, temporal_copy,
    temporal_eq);
  CHECK_IMAGE(Set, "set", set, set_mem_size, set_copy, set_eq);
  CHECK_IMAGE(SpanSet, "spanset", spanset, spanset_mem_size, spanset_copy,
    spanset_eq);
  free(temp); free(set); free(spanset);

  /* Geometries */
  GSERIALIZED *gs = geom_in("SRID=4326;Linestring(1 1, 2 2, 3 1)", -1);
  assert(gs);
  CHECK_IMAGE(GSERIALIZED, "geo", gs, geo_mem_size, geo_copy, geo_same);
  free(gs);

  /* Circular buffers */
  Cbuffer *cb = cbuffer_in("Cbuffer(Point(1 1), 0.5)");
  assert(cb);
  CHECK_IMAGE(Cbuffer, "cbuffer", cb, cbuffer_mem_size, cbuffer_copy,
    cbuffer_eq);
  free(cb);

  /* JSONB values and JSON paths */
  Jsonb *jb = jsonb_in("{\"a\": 1, \"b\": [true, \"x\"]}");
  JsonPath *jp = jsonpath_in("$.b[*] ? (@ == \"x\")");
  assert(jb && jp);
  CHECK_IMAGE(Jsonb, "jsonb", jb, jsonb_mem_size, jsonb_copy, jsonb_eq);
  CHECK_IMAGE(JsonPath, "jsonpath", jp, jsonpath_mem_size, jsonpath_copy,
    jsonpath_same);
  free(jb); free(jp);

  /* Point cloud points and patches of a registered schema */
  PCDimensionSpec dims[3] = {
    { "X", NULL, 1, "int32_t", 1, 0, true },
    { "Y", NULL, 2, "int32_t", 1, 0, true },
    { "Z", NULL, 3, "int32_t", 1, 0, true }
  };
  assert(meos_pc_schema_register_dims(1, 4326, "none", dims, 3));
  double v1[3] = {1, 2, 3}, v2[3] = {4, 5, 6};
  Pcpoint *pt1 = pcpoint_make(1, v1, 3);
  Pcpoint *pt2 = pcpoint_make(1, v2, 3);
  assert(pt1 && pt2);
  const Pcpoint *pts[2] = {pt1, pt2};
  Pcpatch *pa = pcpatch_make(pts, 2);
  assert(pa);
  CHECK_IMAGE(Pcpoint, "pcpoint", pt1, pcpoint_mem_size, pcpoint_copy,
    pcpoint_eq);
  CHECK_IMAGE(Pcpatch, "pcpatch", pa, pcpatch_mem_size, pcpatch_copy,
    pcpatch_eq);
  free(pt1); free(pt2); free(pa);
  meos_pc_schema_clear();

  /* Poses and pose chains */
  Pose *pose = pose_in("Pose(Point(1 1), 0.5)");
  PoseChain *pc = posechain_in("PoseChain(Pose(Point(0 0), 0), "
    "Pose(Point(1 0), 1))");
  assert(pose && pc);
  CHECK_IMAGE(Pose, "pose", pose, pose_mem_size, pose_copy, pose_eq);
  CHECK_IMAGE(PoseChain, "posechain", pc, posechain_mem_size, posechain_copy,
    posechain_eq);
  free(pose); free(pc);

  /* Raquet tiles and rasters */
  Temporal *tpoint = tgeompoint_in(
    "SRID=4326;[Point(0.5 2.5)@2020-01-01, Point(2.5 0.5)@2020-01-02]");
  int ntiles = 0;
  Temporal *ttiles = tgeompoint_to_tquadbin(tpoint, 8, true);
  Quadbin *tiles = tquadbin_values(ttiles, &ntiles);
  assert(tiles && ntiles > 0);
  uint8_t pixels[4] = {7, 8, 9, 10};
  Raquet *rq = raquet_make(tiles[0], 2, 2, MEOS_PT_UINT8, 0.0, false, pixels,
    4);
  Raster *rast = raster_from_hexwkb(raster_one_band);
  assert(rq && rast);
  CHECK_IMAGE(Raquet, "raquet", rq, raquet_mem_size, raquet_copy, raquet_eq);
  CHECK_IMAGE(Raster, "raster", rast, raster_mem_size, raster_copy,
    raster_same);
  free(tpoint); free(ttiles); free(tiles); free(rq); free(rast);

  /* A network point and a network segment have the size of their structure,
   * and their copy holds the same bytes */
  Npoint *np = npoint_in("Npoint(1, 0.5)");
  Nsegment *ns = nsegment_in("Nsegment(1, 0.25, 0.75)");
  assert(np && ns);
  Npoint *npcopy = npoint_copy(np);
  Nsegment *nscopy = nsegment_copy(ns);
  printf("npoint: copy equal %s\n", npoint_eq(np, npcopy) ? "true" : "false");
  printf("nsegment: copy equal %s\n",
    nsegment_eq(ns, nscopy) ? "true" : "false");
  assert(npoint_eq(np, npcopy));
  assert(memcmp(np, npcopy, sizeof(Npoint)) == 0);
  assert(nsegment_eq(ns, nscopy));
  assert(memcmp(ns, nscopy, sizeof(Nsegment)) == 0);
  assert(meos_errno() == 0);
  assert(npoint_copy(NULL) == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  assert(nsegment_copy(NULL) == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  free(np); free(ns); free(npcopy); free(nscopy);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
