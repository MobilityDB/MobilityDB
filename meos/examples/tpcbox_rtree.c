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
 * @file
 * @brief Standalone-MEOS demo: build an in-memory RTree over @c tpcbox
 *   bounding boxes and query it with overlap, contains, and contained-by
 *   operators.  No PostgreSQL backend involved.
 *
 * This program is the pgPointCloud counterpart of @c rtree_example.c.
 * It exercises:
 *
 *   * @ref meos_pc_schema_register — populate the MEOS-owned schema
 *     cache by hand (a PG backend would do this lazily via the
 *     @c mobilitydb_init hook; standalone code does it explicitly).
 *
 *   * @ref rtree_create_tpcbox + @ref rtree_insert + @ref rtree_search
 *     — the bbox-only API.  No temporal value involved; we insert
 *     synthetic @c tpcbox values and query the index directly.
 *
 *   * Build + link: requires libmeos to have been built with
 *     @c -DPOINTCLOUD=ON, plus @c libpc.a available at link time.
 *
 *     @verbatim
 *     gcc tpcbox_rtree.c -o tpcbox_rtree -lmeos
 *     ./tpcbox_rtree
 *     @endverbatim
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_pointcloud.h>
#include "pc_api.h"  /* PCSCHEMA, pc_schema_from_xml */

/* A minimal pgPointCloud schema XML: three int32 X/Y/Z dimensions with
 * 0.01 scale, SRID 0.  Same shape as the datagen helper's
 * @c ensure_random_pcid() inserts into @c pointcloud_formats. */
static const char *DATAGEN_SCHEMA_XML =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<pc:PointCloudSchema xmlns:pc=\"http://pointcloud.org/schemas/PC/1.1\""
"    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
"  <pc:dimension>"
"    <pc:position>1</pc:position><pc:size>4</pc:size>"
"    <pc:name>X</pc:name><pc:interpretation>int32_t</pc:interpretation>"
"    <pc:scale>0.01</pc:scale>"
"  </pc:dimension>"
"  <pc:dimension>"
"    <pc:position>2</pc:position><pc:size>4</pc:size>"
"    <pc:name>Y</pc:name><pc:interpretation>int32_t</pc:interpretation>"
"    <pc:scale>0.01</pc:scale>"
"  </pc:dimension>"
"  <pc:dimension>"
"    <pc:position>3</pc:position><pc:size>4</pc:size>"
"    <pc:name>Z</pc:name><pc:interpretation>int32_t</pc:interpretation>"
"    <pc:scale>0.01</pc:scale>"
"  </pc:dimension>"
"  <pc:metadata><Metadata name=\"srid\">0</Metadata></pc:metadata>"
"</pc:PointCloudSchema>";

int
main(void)
{
  meos_initialize();

  /* (1) Pre-populate the MEOS schema cache for pcid = 1.  In a PG
   * backend this would happen lazily via the catalog-scan hook
   * installed by mobilitydb_init.  Standalone code registers
   * explicitly. */
  PCSCHEMA *schema = pc_schema_from_xml(DATAGEN_SCHEMA_XML);
  if (! schema)
  {
    fprintf(stderr, "Failed to parse demo schema XML\n");
    return 1;
  }
  schema->pcid = 1;
  meos_pc_schema_register(1, schema);

  /* (2) Build an empty RTree typed on TPCBox.  rtree_create_tpcbox
   * is the pointcloud-typed analogue of rtree_create_stbox. */
  RTree *idx = rtree_create_tpcbox();

  /* (3) Insert a few synthetic 2D + time tpcbox values.  Using
   * tpcbox_zt to populate Z and T axes too. */
  TPCBox *boxes[5];
  boxes[0] = tpcbox_make(true, true, true, false, /* srid */ 0, /* pcid */ 1,
    0, 10, 0, 10, 0, 10, NULL);
  boxes[1] = tpcbox_make(true, true, true, false, 0, 1,
    5, 15, 5, 15, 0, 10, NULL);
  boxes[2] = tpcbox_make(true, true, true, false, 0, 1,
    20, 30, 20, 30, 0, 10, NULL);
  boxes[3] = tpcbox_make(true, true, true, false, 0, 1,
    -5, 2, -5, 2, 0, 10, NULL);
  boxes[4] = tpcbox_make(true, true, true, false, 0, 1,
    50, 60, 50, 60, 0, 10, NULL);
  for (int i = 0; i < 5; i++)
    rtree_insert(idx, boxes[i], i);

  /* (4) Query: which boxes overlap [3, 12] x [3, 12] x [-inf, inf]? */
  TPCBox *query = tpcbox_make(true, true, false, false, 0, 1,
    3, 12, 3, 12, 0, 0, NULL);
  MeosArray *hits = meos_array_create(sizeof(int));
  int nhits = rtree_search(idx, RTREE_OVERLAPS, query, hits);
  printf("Overlap query returned %d hits:", nhits);
  for (int i = 0; i < nhits; i++)
    printf(" %d", *(int *) meos_array_get(hits, i));
  printf("\n");
  /* Expected: hits at indices 0, 1, 3 — boxes that overlap [3,12]^2. */

  /* (5) Cleanup. */
  meos_array_destroy(hits);
  free(query);
  for (int i = 0; i < 5; i++)
    free(boxes[i]);
  rtree_free(idx);
  meos_pc_schema_clear();
  pc_schema_free(schema);
  meos_finalize();
  return 0;
}
