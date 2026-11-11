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
 * @brief Test of the signs on coordinates shifted by exact offsets of
 * meos/src/geo/geo_funcs.c
 * @details #cross_product_sign_offset is checked on orientations whose x
 * coordinates are shifted by multiples of 360, as a cell of a grid is read
 * across the antimeridian, against the signs computed in exact rationals: on
 * the witness rows the sign on the doubles the shift rounds to differs from
 * the exact one, and the test fails if it does not, so that the rows keep
 * exercising the rounding; on the control rows the two agree. Then
 * #point_in_ring_offset_exact locates points in a ring straddling the
 * antimeridian, inside, outside, on an edge and at a vertex.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <meos.h>
#include <meos_geo.h>
#include "geo/geo_funcs.h"

typedef struct
{
  double ax, ay;
  int ka;
  double bx, by;
  int kb;
  double px, py;
  int kp;
  int sign;
} Row;

/* Generated in exact rationals by gen_witness.py of the oracle kit of the
 * cell walks: A, B and P with their x shifted by k * 360, and the exact sign
 * of the turn from A->B to A->P */
static const Row witness[] = {
#include "shift_witness_rows.inc"
};

int
main(void)
{
  meos_initialize();
  int failures = 0, nwitness = 0, nrows = 0;
  for (size_t i = 0; i < sizeof(witness) / sizeof(witness[0]); i++)
  {
    const Row *r = &witness[i];
    /* the rows before the first control are the witness: the generator
     * writes 6 of each */
    const bool in_witness = i < sizeof(witness) / sizeof(witness[0]) / 2;
    double oab = 360.0 * (r->kb - r->ka), oap = 360.0 * (r->kp - r->ka);
    int s = cross_product_sign_offset(r->ax, r->ay, r->bx, r->by, r->ax,
      r->ay, r->px, r->py, oab, oap);
    if (s != r->sign)
    {
      printf("FAIL row %zu: sign %d, exact %d\n", i, s, r->sign);
      failures++;
    }
    double rounded = cross_product_sign_exact(r->ax + 360.0 * r->ka, r->ay,
      r->bx + 360.0 * r->kb, r->by, r->ax + 360.0 * r->ka, r->ay,
      r->px + 360.0 * r->kp, r->py);
    if (in_witness)
    {
      nwitness++;
      if (rounded == r->sign)
      {
        printf("FAIL row %zu: the shift in doubles no longer rounds the sign\n", i);
        failures++;
      }
    }
    nrows++;
  }

  /* A ring straddling the antimeridian: its eastern vertices are read 360
   * further east */
  const double x[4] = {179.0, -179.0, -179.0, 179.0};
  const double y[4] = {0.0, 0.0, 2.0, 2.0};
  const double off[4] = {0.0, 360.0, 360.0, 0.0};
  struct { double px, py; int expect; } pts[] = {
    {179.5, 1.0, 1},      /* inside, west of the antimeridian */
    {180.0, 1.0, 1},      /* inside, on the antimeridian */
    {180.5, 1.0, 1},      /* inside, read 360 east of -179.5 */
    {178.0, 1.0, 0},      /* outside */
    {182.0, 1.0, 0},      /* outside, east */
    {179.0, 1.0, -1},     /* on the western edge */
    {181.0, 1.0, -1},     /* on the eastern edge, -179 + 360 */
    {180.0, 0.0, -1},     /* on the southern edge */
    {181.0, 2.0, -1},     /* at a vertex */
  };
  for (size_t i = 0; i < sizeof(pts) / sizeof(pts[0]); i++)
  {
    int in = point_in_ring_offset_exact(x, y, off, 4, pts[i].px, pts[i].py);
    if (in != pts[i].expect)
    {
      printf("FAIL point %zu (%g %g): %d, expected %d\n", i, pts[i].px,
        pts[i].py, in, pts[i].expect);
      failures++;
    }
  }
  meos_finalize();
  printf("%d rows (%d witness), %zu points, %d failures\n", nrows, nwitness,
    sizeof(pts) / sizeof(pts[0]), failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
