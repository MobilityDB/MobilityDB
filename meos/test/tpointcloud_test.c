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
 * @brief A program that tests the temporal point cloud types in a program
 * that has no pgPointCloud schema for the pcid its values name.
 *
 * A pcpoint and a pcpatch carry a pcid and nothing else about their layout,
 * and the schema that pcid resolves to lives in a catalog table only a
 * PostgreSQL backend can scan. A standalone program has neither that catalog
 * nor, until it registers one, any schema at all, which is the state every
 * binding starts in. What such a program can still do is what needs no
 * schema: read a value from its serialized form, write it back, and build a
 * temporal value out of it. What it cannot do is read a coordinate, and the
 * program checks that the two are separated — the value is built, and the
 * question that must decode a coordinate reports the missing schema.
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
 * are the hex WKB pgPointCloud serializes, the form that carries no schema */
#define TPCPOINT_IN \
  "2300000001000000000000000000F03F0000000000000040000000000000084000" \
  "0000@2024-01-01"
#define TPCPATCH_IN \
  "4F000000010000000000000002000000000000000000F03F000000000000F03F00" \
  "0000000000F03F0000000000000040000000000000004000000000000000400000" \
  "00000000000000000000000000@2024-01-01"

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

  /* A temporal point cloud point is built from its serialized form and
   * written back to it, neither of which reads the schema */
  Temporal *tpcpoint = temporal_in(TPCPOINT_IN, T_TPCPOINT);
  assert(tpcpoint != NULL);
  assert(meos_errno() == 0);
  char *tpcpoint_out = temporal_out(tpcpoint, 15);
  printf("tpcpoint: %s\n", tpcpoint_out);
  assert(strchr(tpcpoint_out, '@') != NULL);

  /* And so is a temporal point cloud patch, whose bounds its own serialized
   * header carries */
  Temporal *tpcpatch = temporal_in(TPCPATCH_IN, T_TPCPATCH);
  assert(tpcpatch != NULL);
  assert(meos_errno() == 0);
  char *tpcpatch_out = temporal_out(tpcpatch, 15);
  printf("tpcpatch: %s\n", tpcpatch_out);
  assert(strchr(tpcpatch_out, '@') != NULL);

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

  free(tpcpoint); free(tpcpoint_out);
  free(tpcpatch); free(tpcpatch_out);
  free(box);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
