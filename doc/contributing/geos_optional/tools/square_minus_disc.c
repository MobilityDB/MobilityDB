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

/* The square from (-2,-2) to (2,2) minus the disc of radius 2 at the origin,
   written as a curve polygon: the native answer, printed as text, shows whether
   the arc is kept. Its exact area is 16 - 4 pi. */
#include <stdio.h>
#include <meos.h>
#include <meos_geo.h>
int main(void) {
  meos_initialize();
  GSERIALIZED *sq = geom_in("POLYGON((-2 -2,2 -2,2 2,-2 2,-2 -2))", -1);
  GSERIALIZED *disc = geom_in("CURVEPOLYGON(CIRCULARSTRING(2 0,0 2,-2 0,0 -2,2 0))", -1);
  GSERIALIZED *d = geom_difference2d(sq, disc);
  if (! d) { printf("DECLINED\n"); return 1; }
  printf("%s\n", geo_as_text(d, 9));
  meos_finalize(); return 0;
}
