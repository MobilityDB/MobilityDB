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
 * @brief Test of the geodetic cover of a temporal point by H3 cells of
 * meos/src/h3/th3index_latlng.c
 * @details A cell holds exactly the microseconds whose exact position lies
 * in it. A geodetic cell is the region of the sphere its outline bounds with
 * great-circle arcs, and a geodetic position moves at constant angular speed
 * along the great circle of its segment. The geodetic walk states an edge
 * through each crossing and which cell holds the crossing position, and the
 * cover dates the crossing by the truncation of its exact fraction
 * (#h3_geodetic_edge_offset): where the trip meets the edge exactly at that
 * microsecond, the cell holding the crossing position holds it, the one
 * left, the one entered, or at a vertex a third cell met there; where it
 * reaches the edge after it, the cell left holds it; and a cell crossed
 * between two microseconds holds none. The rows are trips of the acceptance
 * set of row CW3 whose cover, as #tgeogpoint_to_th3index answers it and
 * read as #th3index_value_at_timestamptz reads each instant, an exact oracle
 * sharing no code with MEOS states piece by piece, each a trip the cover
 * dated by the fraction of the walk in doubles answers otherwise.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <meos.h>
#include <meos_geo.h>
#include <meos_h3.h>
#include <meos_internal.h>

typedef struct
{
  int res;              /* Resolution */
  int inc;              /* Whether the grid holds the upper border */
  int n;                /* Number of positions */
  double x[4], y[4];    /* Positions */
  long long t[4];       /* Times in microseconds */
  const char *cover;    /* The pieces of the cover, by the oracle */
} CoverRow;

static const CoverRow rows[] =
{
#include "h3_geog_cover_rows.inc"
};

/* The cell of the instant k of a sequence of the cover */
static H3Index
inst_cell(const TSequence *seq, int k)
{
  const TInstant *inst = TSEQUENCE_INST_N(seq, k);
  H3Index h = 0;
  th3index_value_at_timestamptz((const Temporal *) inst, inst->t, true, &h);
  return h;
}

/* Write the pieces of a cover into a buffer, as the oracle states them:
 * each run of instants of one cell, its period, ( ) for an exclusive
 * bound, as h3_cover_dump of the plan of the cell walks reads them */
static void
cover_text(const Temporal *cover, char *buf, size_t size)
{
  size_t len = 0;
  buf[0] = '\0';
  int count;
  TSequence **seqs = temporal_sequences(cover, &count);
  for (int s = 0; s < count; s++)
  {
    const TSequence *seq = seqs[s];
    int k = 0;
    while (k < seq->count)
    {
      H3Index h = inst_cell(seq, k);
      int j = k + 1;
      while (j < seq->count && inst_cell(seq, j) == h)
        j++;
      bool li = k == 0 ? seq->period.lower_inc : true;
      TimestampTz lo = TSEQUENCE_INST_N(seq, k)->t, hi;
      bool ui;
      if (j < seq->count)
      {
        hi = TSEQUENCE_INST_N(seq, j)->t;
        ui = false;
      }
      else
      {
        hi = TSEQUENCE_INST_N(seq, seq->count - 1)->t;
        ui = seq->period.upper_inc;
      }
      len += (size_t) snprintf(buf + len, size - len, "%s%llx %s%lld,%lld%s",
        len ? " " : "", (unsigned long long) h, li ? "[" : "(",
        (long long) lo, (long long) hi, ui ? "]" : ")");
      k = j;
    }
    free(seqs[s]);
  }
  free(seqs);
  return;
}

int
main(void)
{
  meos_initialize();
  int failures = 0, nrows = (int) (sizeof(rows) / sizeof(rows[0]));
  /* A trip crossing many cells in a few microseconds states some hundred
   * pieces */
  char buf[16384];
  for (int i = 0; i < nrows; i++)
  {
    const CoverRow *r = &rows[i];
    TInstant *insts[4];
    for (int k = 0; k < r->n; k++)
    {
      char wkt[128];
      snprintf(wkt, sizeof(wkt), "SRID=4326;POINT(%.17g %.17g)", r->x[k],
        r->y[k]);
      GSERIALIZED *g = geog_in(wkt, -1);
      insts[k] = tpointinst_make(g, r->t[k]);
      free(g);
    }
    Temporal *temp = (Temporal *) tsequence_make(insts, r->n, true, true,
      LINEAR, false);
    Temporal *cover = tgeogpoint_to_th3index(temp, r->res, r->inc != 0);
    if (cover)
      cover_text(cover, buf, sizeof(buf));
    else
      buf[0] = '\0';
    if (strcmp(buf, r->cover) != 0)
    {
      printf("FAIL row %d at resolution %d:\n  got  %s\n  want %s\n", i,
        r->res, buf, r->cover);
      failures++;
    }
    free(cover);
    free(temp);
    for (int k = 0; k < r->n; k++)
      free(insts[k]);
  }
  meos_finalize();
  printf("%d rows, %d failures\n", nrows, failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
