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
 * @brief A program that tests that the sort keys of network segments and of
 * Raquet tiles order and equate them as their comparison and their equality do
 * @details A sort key is the string of bytes whose unsigned byte order is the
 * order of a comparison and whose byte equality is the equality of the type,
 * so that an engine that sorts, groups or joins values by their bytes answers
 * as MobilityDB does. Over a corpus of network segments (four routes, the
 * positions 0, 0.25, 0.5 and 1 in either order of the two ends) and of Raquet
 * tiles (four tiles, two pixel types, three widths and two heights, pixels of
 * small and large bytes, the same pixels with and without a nodata value,
 * which #raquet_cmp does not read), the program checks:
 * - over every pair, that the comparison answers 0 exactly when the equality
 *   holds, and that swapping the operands swaps the sign;
 * - over every triple, that the comparison is transitive;
 * - over every pair, that the byte order of the keys is the comparison and
 *   that the keys are identical exactly when the equality holds.
 * It also counts the pairs whose stored bytes order otherwise than the
 * comparison, which is what an engine comparing the stored bytes answers.
 *
 * The routes of the network segments come from the ways file the environment
 * variable MEOS_WAYS_CSV names, by default `../examples/data/ways1000.csv`,
 * the one of the source tree read from `meos/test`, where the tests run, as
 * `spatialset_distance_test.c` reads it. A corpus that is empty, as one of
 * network segments is when no ways file holds their routes, is a failure, so
 * that the checks are never made over nothing.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o raquet_nsegment_sortkey_test raquet_nsegment_sortkey_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_npoint.h>
#include <meos_quadbin.h>
#include <meos_raster.h>

#define MAXV 512

/* The functions a family offers to the checks */
typedef struct
{
  const char *name;
  int (*cmp)(const void *, const void *);
  bool (*eq)(const void *, const void *);
  uint8_t *(*key)(const void *, size_t *);
  size_t (*bytes)(const void *);
} family;

static int sign(int c) { return (c > 0) - (c < 0); }

static int ns_cmp(const void *a, const void *b) { return nsegment_cmp(a, b); }
static bool ns_eq(const void *a, const void *b) { return nsegment_eq(a, b); }
static uint8_t *ns_key(const void *a, size_t *n) { return nsegment_sortkey(a, n); }
static size_t ns_bytes(const void *a) { (void) a; return 24; }

static int rq_cmp(const void *a, const void *b) { return raquet_cmp(a, b); }
static bool rq_eq(const void *a, const void *b) { return raquet_eq(a, b); }
static uint8_t *rq_key(const void *a, size_t *n) { return raquet_sortkey(a, n); }
static size_t rq_bytes(const void *a) { return (size_t) raquet_mem_size(a); }

static long failures = 0;

static void
check(const family *f, void **v, int n)
{
  if (n == 0)
  {
    printf("%s: no value was built\n", f->name);
    failures++;
    return;
  }
  long pairs = 0, inconsistent = 0, asym = 0, keyless = 0, badkey = 0,
    bytes_otherwise = 0, triples = 0, intransitive = 0;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      int c = sign(f->cmp(v[i], v[j]));
      if ((c == 0) != f->eq(v[i], v[j]))
        inconsistent++;
      if (c != -sign(f->cmp(v[j], v[i])))
        asym++;
      size_t na, nb;
      uint8_t *ka = f->key(v[i], &na), *kb = f->key(v[j], &nb);
      if (! ka || ! kb)
        keyless++;
      else
      {
        size_t m = na < nb ? na : nb;
        int k = memcmp(ka, kb, m);
        k = k ? sign(k) : (na > nb) - (na < nb);
        if (k != c)
          badkey++;
      }
      free(ka); free(kb);
      size_t sa = f->bytes(v[i]), sb = f->bytes(v[j]);
      size_t m = sa < sb ? sa : sb;
      int b = memcmp(v[i], v[j], m);
      b = b ? sign(b) : (sa > sb) - (sa < sb);
      if (b != c)
        bytes_otherwise++;
      pairs++;
    }
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      if (f->cmp(v[i], v[j]) >= 0)
        continue;
      for (int k = 0; k < n; k++)
        if (f->cmp(v[j], v[k]) < 0)
        {
          triples++;
          if (f->cmp(v[i], v[k]) >= 0)
            intransitive++;
        }
    }
  printf("%s: %ld pairs, %ld whose comparison answers 0 otherwise than their "
    "equality, %ld whose swapped comparison is not the opposite, %ld without "
    "a key, %ld ordered or equated otherwise than the comparison by their "
    "keys, %ld ordered otherwise by their stored bytes; %ld ordered triples, "
    "%ld not transitive\n", f->name, pairs, inconsistent, asym, keyless, badkey,
    bytes_otherwise, triples, intransitive);
  failures += inconsistent + asym + keyless + badkey + intransitive;
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();
  const char *ways = getenv("MEOS_WAYS_CSV");
  meos_set_ways_csv(ways ? ways : "../examples/data/ways1000.csv");

  /* Network segments */
  void *segs[MAXV];
  int nsegs = 0;
  const int64 rids[] = {1, 2, 256, 300};
  const double pos[] = {0.0, 0.25, 0.5, 1.0};
  for (int r = 0; r < 4; r++)
    for (int a = 0; a < 4; a++)
      for (int b = 0; b < 4; b++)
      {
        Nsegment *ns = nsegment_make(rids[r], pos[a], pos[b]);
        meos_errno_reset();
        if (ns)
          segs[nsegs++] = ns;
      }
  family fns = {"nsegment", ns_cmp, ns_eq, ns_key, ns_bytes};
  check(&fns, segs, nsegs);

  /* Raquet tiles */
  void *tiles[MAXV];
  int ntiles = 0;
  const Quadbin cells[] = {quadbin_tile_to_cell(0, 0, 1),
    quadbin_tile_to_cell(1, 0, 1), quadbin_tile_to_cell(3, 2, 2),
    quadbin_tile_to_cell(255, 1, 8)};
  const MeosPixType types[] = {MEOS_PT_UINT8, MEOS_PT_INT16};
  const uint8_t fill[] = {0x00, 0x01, 0x7F, 0xFF};
  uint8_t pixels[64];
  for (int c = 0; c < 4; c++)
    for (int t = 0; t < 2; t++)
      for (int w = 1; w <= 3; w++)
        for (int h = 1; h <= 2; h++)
          for (int p = 0; p < 4; p += (w == 1 && h == 1) ? 1 : 3)
            for (int nd = 0; nd < 2; nd++)
            {
              size_t size = (size_t) (w * h * (t == 0 ? 1 : 2));
              for (size_t i = 0; i < size; i++)
                pixels[i] = (uint8_t) (fill[p] + i);
              Raquet *rq = raquet_make(cells[c], w, h, types[t],
                nd ? 7.0 : 0.0, nd == 1, pixels, size);
              meos_errno_reset();
              if (rq && ntiles < MAXV)
                tiles[ntiles++] = rq;
            }
  family frq = {"raquet", rq_cmp, rq_eq, rq_key, rq_bytes};
  check(&frq, tiles, ntiles);

  for (int i = 0; i < nsegs; i++)
    free(segs[i]);
  for (int i = 0; i < ntiles; i++)
    free(tiles[i]);
  printf("%ld failure(s)\n", failures);
  meos_finalize();
  return failures ? 1 : 0;
}
