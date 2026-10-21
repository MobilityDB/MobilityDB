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

/* The same square, 0.02 on a side, buffered by 0.001, 1 and 1000 at the origin,
   at 1 000 000 and at 6 400 000: the area of each buffer, or DECLINED. The
   area of the buffer of a square of side s at distance r is s^2 + 4sr + pi r^2.
   Run with an error handler that returns, so every case prints. */
#include <stdio.h>
#include <meos.h>
#include <meos_geo.h>
int main(void) {
  meos_initialize();
  meos_initialize_noexit_error_handler();
  double bases[] = {0, 1e6, 6.4e6}; double rs[] = {0.001, 1, 1000};
  for (int j = 0; j < 3; j++) for (int i = 0; i < 3; i++) {
    double b = bases[i], s = 0.02; char wkt[256];
    snprintf(wkt, sizeof wkt, "POLYGON((%.17g %.17g,%.17g %.17g,%.17g %.17g,%.17g %.17g,%.17g %.17g))",
      b,b, b+s,b, b+s,b+s, b,b+s, b,b);
    GSERIALIZED *g = geom_in(wkt, -1);
    GSERIALIZED *r = geom_buffer(g, rs[j], "");
    if (r) printf("r=%g base=%g area=%.15g\n", rs[j], b, geom_area(r));
    else printf("r=%g base=%g DECLINED\n", rs[j], b);
  }
  meos_finalize(); return 0;
}
