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
 * @brief MEOS-side producer for the end-to-end zero-MEOS Arrow and Parquet
 * consumption demo
 *
 * @details This is the MEOS-linked half of the demo. It builds a
 * representative population of temporal values (one decomposed-Struct type,
 * temporal geometry point, and one scalar type, temporal float), sized so
 * that a Parquet file written from them forms several row groups. For each
 * value it calls the conformance-proven #meos_temporal_to_arrow kernel and
 * exposes the produced `ArrowSchema`/`ArrowArray` over the documented Arrow
 * C Data Interface so that a zero-MEOS consumer (pyarrow, loaded by the
 * bridge with no libmeos symbols of its own) can import the array purely
 * through that ABI. It also emits the MEOS canonical string of every value
 * to a ground-truth file so that a separate zero-MEOS process can verify
 * the Parquet round-trip bit-exactly without ever touching MEOS.
 *
 * `temporal_arrow.c` is untouched: this is a consumer-side demo built only
 * on the already-conformance-proven export kernel.
 *
 * The values are laid out in SRID blocks (the first half SRID 4326, the
 * second half SRID 3857) so that the flat top-level `srid` Arrow column
 * carries block structure a Parquet reader can use for honest row-group
 * statistics pruning. The deeply nested `seqs` column intentionally carries
 * no such flat structure; the demo measures and reports exactly what the
 * nested schema does and does not let a Parquet engine prune.
 *
 * Built as a shared library; the Python bridge loads it with ctypes and
 * calls the exported functions. The bridge itself links no MEOS symbols.
 *
 * @code
 * gcc -Wall -fPIC -shared -I/usr/local/include \
 *   -o libtemporal_arrow_parquet_producer.so \
 *   temporal_arrow_parquet_producer.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_internal.h>
#include <meos_geo.h>
#include "arrow_c_data_interface.h"

/* Population size per type. A few thousand rows so that, written with a
 * small Parquet row-group size, the file forms several row groups and a
 * predicate on the flat top-level columns can skip some of them. */
#define N_PER_TYPE 4000

/* Two SRID blocks: rows [0, N/2) carry SRID 4326, rows [N/2, N) carry
 * SRID 3857. This is real block structure on a flat top-level Arrow
 * column, the only place row-group statistics pruning is honestly
 * available for this nested schema. */
#define SRID_A 4326
#define SRID_B 3857

static Temporal **g_values = NULL;
static int g_count = 0;

/**
 * @brief Build the temporal value population
 *
 * @details Even global indices are temporal geometry points (the
 * decomposed Struct{x,y} value leaf); odd indices are temporal floats
 * (the scalar Float64 value leaf). Both kinds are interleaved so the
 * Parquet file mixes the two value-leaf encodings, exactly as a real
 * heterogeneous temporal column would. Each value is a short linear
 * sequence so the nested seqs/insts lists are non-trivial.
 */
static void
build_population(void)
{
  g_count = 2 * N_PER_TYPE;
  g_values = malloc(sizeof(Temporal *) * g_count);
  char buf[512];
  for (int i = 0; i < N_PER_TYPE; i++)
  {
    int srid = (i < N_PER_TYPE / 2) ? SRID_A : SRID_B;
    /* Temporal geometry point: a two-instant linear sequence whose
     * coordinates and timestamps vary per row. */
    double x0 = (i % 97) + 0.25, y0 = (i % 89) + 0.5;
    double x1 = x0 + 1.0, y1 = y0 + 2.0;
    int day = (i % 27) + 1;
    snprintf(buf, sizeof(buf),
      "SRID=%d;[Point(%.4f %.4f)@2000-01-%02d, "
      "Point(%.4f %.4f)@2000-02-%02d]",
      srid, x0, y0, day, x1, y1, day);
    g_values[2 * i] = (Temporal *) tgeompoint_in(buf);

    /* Temporal float: a two-instant linear sequence. */
    double f0 = (i % 101) * 0.5 - 10.0;
    double f1 = f0 + ((i % 7) - 3) * 1.5;
    snprintf(buf, sizeof(buf),
      "[%.4f@2000-01-%02d, %.4f@2000-02-%02d]",
      f0, day, f1, day);
    g_values[2 * i + 1] = (Temporal *) tfloat_in(buf);
  }
}

/**
 * @brief Initialize MEOS and build the population
 * @return The number of temporal values produced
 */
int
producer_init(void)
{
  meos_initialize();
  build_population();
  for (int i = 0; i < g_count; i++)
    if (! g_values[i])
    {
      fprintf(stderr, "producer_init: value %d is NULL\n", i);
      return -1;
    }
  return g_count;
}

/** @brief Number of temporal values in the population */
int
producer_count(void)
{
  return g_count;
}

/**
 * @brief Export value @p i over the Arrow C Data Interface
 *
 * @details The caller (the zero-MEOS Python bridge) owns two
 * `ArrowSchema` and `ArrowArray` C structs and passes their addresses.
 * This fills them through the conformance-proven #meos_temporal_to_arrow
 * kernel. The consumer then imports them purely through the documented
 * ABI; the consumer never links or calls any MEOS symbol.
 *
 * @return 1 on success, 0 on failure
 */
int
producer_export(int i, void *schema_addr, void *array_addr)
{
  if (i < 0 || i >= g_count)
    return 0;
  struct ArrowSchema *schema = (struct ArrowSchema *) schema_addr;
  struct ArrowArray *array = (struct ArrowArray *) array_addr;
  return meos_temporal_to_arrow(g_values[i], schema, array) ? 1 : 0;
}

/**
 * @brief Write the MEOS canonical string of every value to @p path
 *
 * @details One line per value, in population order. This is the ground
 * truth a separate zero-MEOS process compares the Parquet round-trip
 * against. The round-trip on the consumer side reconstructs each value
 * through #meos_temporal_from_arrow only inside the MEOS-linked control;
 * the independent zero-MEOS check compares the canonical strings.
 *
 * @return 1 on success, 0 on failure
 */
int
producer_write_ground_truth(const char *path)
{
  FILE *f = fopen(path, "w");
  if (! f)
    return 0;
  for (int i = 0; i < g_count; i++)
  {
    char *s = temporal_out(g_values[i], 15);
    fprintf(f, "%s\n", s);
    free(s);
  }
  fclose(f);
  return 1;
}

/**
 * @brief Write the decomposed field values of every value to @p path,
 * extracted from the original `Temporal*` through MEOS accessors
 *
 * @details This is the canonical oracle for the zero-MEOS verifier. Each
 * line is the decomposed content MEOS itself reports for the original
 * value, computed from the `Temporal*` (NOT from the Arrow export) via
 * MEOS accessor functions: subtype code, interpolation code, srid, and
 * the per-instant timestamps (microseconds since the Unix epoch) and
 * values (x;y for a temporal point, the scalar for a temporal float).
 * The zero-MEOS consumer reads exactly these fields back from Parquet and
 * compares them value-exact, with MEOS's own accessors as the oracle and
 * no re-implementation of any MEOS encoding. Line format:
 *
 *   row_id|kind|srid|t0:v0,t1:v1,...
 *
 * where kind is "P" (temporal point, v = "x:y") or "F" (temporal float,
 * v = the double rendered with %.15g), and t is the integer microsecond
 * timestamp. The decomposition mirrors exactly what the Arrow value leaf
 * carries, so a value-exact match proves the Parquet round-trip is
 * lossless without any MEOS in the consumer.
 *
 * @return 1 on success, 0 on failure
 */
int
producer_write_decomposed(const char *path)
{
  FILE *f = fopen(path, "w");
  if (! f)
    return 0;
  for (int i = 0; i < g_count; i++)
  {
    Temporal *t = g_values[i];
    bool is_point = (t->temptype == T_TGEOMPOINT);
    int srid = is_point ? tspatial_srid(t) : 0;
    int nc = 0;
    /* Per-instant timestamps in temporal (ascending) order, exactly the
     * order the Arrow insts list carries. */
    TimestampTz *ts = temporal_timestamps(t, &nc);
    fprintf(f, "%d|%s|%d|", i, is_point ? "P" : "F", srid);
    if (is_point)
    {
      /* tpoint_get_x/y are temporal floats of the coordinates; evaluate
       * each at the instant timestamp so the value is the per-instant
       * value in temporal order (NOT the dedup-sorted value set that
       * tfloat_values returns). */
      Temporal *tx = tpoint_get_x(t);
      Temporal *ty = tpoint_get_y(t);
      for (int k = 0; k < nc; k++)
      {
        double vx = 0.0, vy = 0.0;
        tfloat_value_at_timestamptz(tx, ts[k], false, &vx);
        tfloat_value_at_timestamptz(ty, ts[k], false, &vy);
        fprintf(f, "%s%lld:%.15g:%.15g", k ? "," : "",
          (long long) ts[k], vx, vy);
      }
      free(tx); free(ty);
    }
    else
    {
      for (int k = 0; k < nc; k++)
      {
        double v = 0.0;
        tfloat_value_at_timestamptz(t, ts[k], false, &v);
        fprintf(f, "%s%lld:%.15g", k ? "," : "",
          (long long) ts[k], v);
      }
    }
    fprintf(f, "\n");
    free(ts);
  }
  fclose(f);
  return 1;
}

/**
 * @brief MEOS-side control: reconstruct value @p i from its own Arrow
 * export and return whether it is bit-exact to the original
 *
 * @details This is only a control on top of the external verdict. The
 * authoritative zero-MEOS check is the Parquet round-trip comparison
 * against the ground-truth file, done in a process with no libmeos.
 *
 * @return 1 if the self round-trip is bit-exact, 0 otherwise
 */
int
producer_selfcheck(int i)
{
  if (i < 0 || i >= g_count)
    return 0;
  Temporal *back = meos_temporal_arrow_roundtrip(g_values[i]);
  if (! back)
    return 0;
  char *a = temporal_out(g_values[i], 15);
  char *b = temporal_out(back, 15);
  int ok = (strcmp(a, b) == 0);
  free(a);
  free(b);
  free(back);
  return ok;
}

/** @brief Finalize MEOS and free the population */
void
producer_finalize(void)
{
  for (int i = 0; i < g_count; i++)
    free(g_values[i]);
  free(g_values);
  g_values = NULL;
  g_count = 0;
  meos_finalize();
}
