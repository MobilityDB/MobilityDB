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
 * @brief A simple program that exercises the temporal CARTO QUADBIN cell-index
 * type: it parses a temporal quadbin literal, then lifts the shared DGGS
 * cell-index operations (resolution, centroid point) and the quadbin-unique
 * quadkey accessor over time, printing each temporal result.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -DMEOS=1 -I/usr/local/include -o quadbin_smoke quadbin_smoke.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>    /* for printf */
#include <stdlib.h>   /* for free */
/* Include the MEOS API header and the quadbin extension API */
#include <meos.h>
#include <meos_internal.h>
#include <meos_quadbin.h>
#include "temporal/tcellindex.h"

int main(void)
{
  /* Initialize MEOS */
  meos_initialize();

  /* A temporal quadbin value in WKT: two cells (hex of the 64-bit index)
   * sampled at two instants. 480fffffffffffff is the zoom-0 world cell;
   * 48427fffffffffff is the zoom-4 cell of slippy tile (x=3, y=5). */
  char *tquadbin_wkt =
    "{480fffffffffffff@2000-01-01, 48427fffffffffff@2000-01-02}";
  Temporal *tqb = tquadbin_in(tquadbin_wkt);
  char *tqb_str = temporal_out(tqb, 0);
  printf("Temporal quadbin: %s\n", tqb_str);

  /* Shared DGGS op: temporal resolution (tint) */
  Temporal *tres = tcellindex_get_resolution(tqb);
  char *tres_str = temporal_out(tres, 0);
  printf("getResolution:    %s\n", tres_str);

  /* Shared DGGS op: temporal cell centroid (tgeompoint, SRID 4326) */
  Temporal *tpt = tcellindex_cell_to_point(tqb);
  char *tpt_str = temporal_out(tpt, 6);
  printf("cellToPoint:      %s\n", tpt_str);

  /* Quadbin-unique op: temporal quadkey (ttext base-4 slippy-tile string) */
  Temporal *tqk = tquadbin_cell_to_quadkey(tqb);
  char *tqk_str = temporal_out(tqk, 0);
  printf("cellToQuadkey:    %s\n", tqk_str);

  /* Clean up */
  free(tqb_str);
  free(tres_str);
  free(tpt_str);
  free(tqk_str);
  free(tqb);
  free(tres);
  free(tpt);
  free(tqk);

  /* Finalize MEOS */
  meos_finalize();
  return 0;
}
