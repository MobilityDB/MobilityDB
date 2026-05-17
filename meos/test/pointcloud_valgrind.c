/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @file pointcloud_valgrind.c
 * @brief Allocation-balanced exercise of the pgsql_compat shim entry
 *        points. Run under valgrind --leak-check=full to confirm zero
 *        leaks across (de)serialize round-trips for PC_NONE,
 *        PC_DIMENSIONAL, and (when --with-lazperf is in effect)
 *        PC_LAZPERF compression types.
 *
 * Build (assumes a MEOS build dir at ../../build-meos and the vendored
 * pgPointCloud subtree at ../../pointcloud-pg):
 *
 *   PCDIR=../../build-meos/meos/src/pointcloud/CMakeFiles/pointcloud.dir
 *   gcc -O0 -g -DMEOS=1 -DPOINTCLOUD=1 -DPOSTGRESQL_VERSION_NUMBER=140200 \
 *     -I../include \
 *     -isystem /usr/include/json-c \
 *     -isystem ../../postgres -isystem ../../build-meos/postgres \
 *     -isystem ../../postgis/liblwgeom -isystem ../../postgis \
 *     -I../../pointcloud-pg/lib \
 *     pointcloud_valgrind.c \
 *     $PCDIR/pgsql_compat.c.o \
 *     ../../pointcloud-pg/lib/libpc.a \
 *     ../../pointcloud-pg/lib/liblazperf.a \
 *     -o pcv -L../../build-meos/meos -lmeos \
 *     -Wl,-rpath,../../build-meos/meos -lxml2 -lz -lstdc++ -lm
 *
 * Run:
 *   valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=2 ./pcv
 *
 * Expected output: "All heap blocks were freed -- no leaks are
 * possible" and "ERROR SUMMARY: 0 errors from 0 contexts".
 */

#include <meos.h>
#include <meos_pointcloud.h>
#include <pointcloud/pgsql_compat.h>

#include <pc_api.h>
#include <pc_api_internal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ITERATIONS  64
#define POINTS_PER  100
#define PCID        7777

static const char SCHEMA_XML[] =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
  "<pc:PointCloudSchema xmlns:pc=\"http://pointcloud.org/schemas/PC/1.1\">"
  "<pc:dimension><pc:position>1</pc:position><pc:size>8</pc:size>"
  "<pc:name>X</pc:name><pc:interpretation>double</pc:interpretation>"
  "<pc:scale>1</pc:scale></pc:dimension>"
  "<pc:dimension><pc:position>2</pc:position><pc:size>8</pc:size>"
  "<pc:name>Y</pc:name><pc:interpretation>double</pc:interpretation>"
  "<pc:scale>1</pc:scale></pc:dimension>"
  "<pc:dimension><pc:position>3</pc:position><pc:size>8</pc:size>"
  "<pc:name>Z</pc:name><pc:interpretation>double</pc:interpretation>"
  "<pc:scale>1</pc:scale></pc:dimension>"
  "<pc:metadata><Metadata name=\"compression\">none</Metadata>"
  "</pc:metadata></pc:PointCloudSchema>";

static PCSCHEMA *
build_schema(void)
{
  PCSCHEMA *s = pc_schema_from_xml(SCHEMA_XML);
  if (!s) { fprintf(stderr, "pc_schema_from_xml failed\n"); exit(1); }
  s->pcid = PCID;
  return s;
}

static PCPATCH *
build_patch(const PCSCHEMA *schema, uint32_t npoints)
{
  PCPOINTLIST *pl = pc_pointlist_make(npoints);
  for (uint32_t i = 0; i < npoints; ++i)
  {
    PCPOINT *pt = pc_point_make(schema);
    PCDIMENSION *xd = pc_schema_get_dimension(schema, 0);
    PCDIMENSION *yd = pc_schema_get_dimension(schema, 1);
    PCDIMENSION *zd = pc_schema_get_dimension(schema, 2);
    pc_double_to_ptr(pt->data + xd->byteoffset, xd->interpretation,
                     (double) i);
    pc_double_to_ptr(pt->data + yd->byteoffset, yd->interpretation,
                     (double) (i * 2));
    pc_double_to_ptr(pt->data + zd->byteoffset, zd->interpretation,
                     (double) (i * 3));
    pc_pointlist_add_point(pl, pt);
  }
  PCPATCH *patch = (PCPATCH *) pc_patch_uncompressed_from_pointlist(pl);
  pc_pointlist_free(pl);
  return patch;
}

static void
run_point_round_trip(const PCSCHEMA *schema)
{
  PCPOINT *pt = pc_point_make(schema);
  PCDIMENSION *xd = pc_schema_get_dimension(schema, 0);
  pc_double_to_ptr(pt->data + xd->byteoffset, xd->interpretation, 42.0);

  SERIALIZED_POINT *ser = meos_pc_point_serialize(pt);
  PCPOINT *got = meos_pc_point_deserialize(ser, schema);

  pc_point_free(got);
  pcfree(ser);
  pc_point_free(pt);
}

static void
run_patch_round_trip(const PCSCHEMA *schema, int compression)
{
  PCPATCH *patch = build_patch(schema, POINTS_PER);

  /* Drive compression by overriding schema->compression (the shim's
   * meos_pc_patch_serialize dispatches on it). */
  uint32_t saved = schema->compression;
  ((PCSCHEMA *) schema)->compression = compression;

  SERIALIZED_PATCH *ser = meos_pc_patch_serialize(patch, NULL);
  if (!ser) { fprintf(stderr, "serialize failed\n"); exit(1); }

  PCPATCH *got = meos_pc_patch_deserialize(ser, schema);
  if (!got) { fprintf(stderr, "deserialize failed\n"); exit(1); }

  pc_patch_free(got);
  pcfree(ser);
  pc_patch_free(patch);

  ((PCSCHEMA *) schema)->compression = saved;
}

int
main(int argc, char **argv)
{
  (void) argc; (void) argv;
  meos_initialize();
  pc_install_default_handlers();

  PCSCHEMA *schema = build_schema();

  for (int i = 0; i < ITERATIONS; ++i)
  {
    run_point_round_trip(schema);
    run_patch_round_trip(schema, PC_NONE);
    run_patch_round_trip(schema, PC_DIMENSIONAL);
#ifdef HAVE_LAZPERF
    run_patch_round_trip(schema, PC_LAZPERF);
#endif
  }

  pc_schema_free(schema);
  meos_finalize();
  return 0;
}
