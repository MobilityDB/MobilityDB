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
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
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
 * @brief Test that a point cloud schema stated as dimensions is the schema
 * that the same document states
 *
 * A schema stated as rows and the same schema parsed from its XML document
 * are two statements of one thing, so every field of the result must agree,
 * including the ones the engine computes: the size of a dimension, the offset
 * of a dimension within a point, and the width of a point.
 */

#include <stdio.h>
#include <string.h>
#include <meos.h>
#include <meos_pointcloud.h>
#include <pointcloud/pc_api.h>

/* The schema of section 19.1.1 of the manual, as an XML document states it */
static const char *SCHEMA_XML =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
  "<pc:PointCloudSchema xmlns:pc=\"http://pointcloud.org/schemas/PC/1.1\">"
  "  <pc:dimension><pc:position>1</pc:position><pc:size>4</pc:size>"
  "    <pc:name>X</pc:name><pc:interpretation>int32_t</pc:interpretation>"
  "    <pc:scale>1</pc:scale><pc:active>1</pc:active></pc:dimension>"
  "  <pc:dimension><pc:position>2</pc:position><pc:size>4</pc:size>"
  "    <pc:name>Y</pc:name><pc:interpretation>int32_t</pc:interpretation>"
  "    <pc:scale>1</pc:scale><pc:active>1</pc:active></pc:dimension>"
  "  <pc:dimension><pc:position>3</pc:position><pc:size>4</pc:size>"
  "    <pc:name>Z</pc:name><pc:interpretation>int32_t</pc:interpretation>"
  "    <pc:scale>1</pc:scale><pc:active>1</pc:active></pc:dimension>"
  "</pc:PointCloudSchema>";

static int failures = 0;

static void
check(bool ok, const char *what)
{
  if (! ok)
  {
    printf("  FAIL %s\n", what);
    failures++;
  }
}

int
main(void)
{
  meos_initialize();

  /* The same schema, stated as its dimensions */
  PCDimensionSpec dims[3] = {
    { "X", NULL, 1, "int32_t", 1, 0, true },
    { "Y", NULL, 2, "int32_t", 1, 0, true },
    { "Z", NULL, 3, "int32_t", 1, 0, true }
  };
  if (! meos_pc_schema_register_dims(1, 4326, "none", dims, 3))
  {
    printf("FAILED: the schema stated as dimensions is not registered\n");
    return 1;
  }
  PCSCHEMA *stated = meos_pc_schema(1);
  PCSCHEMA *parsed = pc_schema_from_xml(SCHEMA_XML);
  if (! stated || ! parsed)
  {
    printf("FAILED: a schema is missing (stated %p, parsed %p)\n",
      (void *) stated, (void *) parsed);
    return 1;
  }

  printf("The schema stated as dimensions against the same document:\n");
  check(stated->ndims == parsed->ndims, "ndims");
  check(stated->size == parsed->size, "size");
  check(stated->compression == parsed->compression, "compression");
  check(stated->srid == 4326, "srid");
  check(stated->pcid == 1, "pcid");
  for (uint32_t i = 0; i < parsed->ndims; i++)
  {
    PCDIMENSION *a = stated->dims[i], *b = parsed->dims[i];
    check(a && b, "dimension present");
    if (! a || ! b)
      continue;
    check(strcmp(a->name, b->name) == 0, "dimension name");
    check(a->position == b->position, "dimension position");
    check(a->size == b->size, "dimension size");
    check(a->byteoffset == b->byteoffset, "dimension byteoffset");
    check(a->interpretation == b->interpretation, "dimension interpretation");
    check(a->scale == b->scale, "dimension scale");
    check(a->offset == b->offset, "dimension offset");
    check(a->active == b->active, "dimension active");
  }
  /* The X, Y and Z dimensions are resolved from the names */
  check(stated->xdim && strcmp(stated->xdim->name, "X") == 0, "xdim");
  check(stated->ydim && strcmp(stated->ydim->name, "Y") == 0, "ydim");
  check(stated->zdim && strcmp(stated->zdim->name, "Z") == 0, "zdim");
  /* The name hash table answers, as a schema built from a document does */
  check(pc_schema_get_dimension_by_name(stated, "Y") == stated->dims[1],
    "dimension by name");

  printf("%s: %d field(s) disagree\n", failures ? "FAILED" : "PASSED",
    failures);
  meos_finalize();
  return failures ? 1 : 0;
}
