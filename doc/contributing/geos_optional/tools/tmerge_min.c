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

/* Does merging two temporal geometries reach GEOS when their instants share a
   timestamp? The error handler mirrors #handler of the sweep probe in
   doc/contributing/geos_optional/tools/. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

static char msg[512];
static void handler(int l, int c, const char *m) { (void) l; meos_errno_set(c); snprintf(msg, sizeof msg, "%s", m ? m : ""); }

static void
try(const char *a, const char *b)
{
  Temporal *t1 = tgeometry_in(a), *t2 = tgeometry_in(b);
  msg[0] = 0; meos_errno_reset();
  Temporal *r = temporal_merge(t1, t2);
  printf("merge(%s, %s)\n  -> %s\n", a, b, r ? tspatial_as_text(r, 6) : msg);
}

int
main(void)
{
  meos_initialize();
  meos_initialize_error_handler(handler);
  try("POLYGON((0 0,1 0,1 1,0 1,0 0))@2000-01-01", "POLYGON((5 5,6 5,6 6,5 6,5 5))@2000-01-01");
  try("POLYGON((0 0,1 0,1 1,0 1,0 0))@2000-01-01", "MULTIPOLYGON(((5 5,6 5,6 6,5 6,5 5)))@2000-01-01");
  try("Point(1 1)@2000-01-01", "MULTIPOLYGON(((5 5,6 5,6 6,5 6,5 5)))@2000-01-01");
  meos_finalize();
  return 0;
}
