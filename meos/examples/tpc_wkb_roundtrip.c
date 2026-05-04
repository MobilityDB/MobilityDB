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
 *****************************************************************************/

/**
 * @file
 * @brief Standalone-MEOS WKB round-trip test for tpcpoint values.
 *   Builds a hand-coded SERIALIZED_POINT, wraps it in a TInstant,
 *   encodes via @c temporal_as_wkb, decodes via @c temporal_from_wkb,
 *   and asserts byte-equivalent recovery via @c temporal_eq. Pins the
 *   MEOS-side WKB code path independently of the PG roundtrip.
 *
 *   Build:
 *   @code
 *     gcc -Wall -g -I/usr/local/include -o tpc_wkb_roundtrip \
 *       tpc_wkb_roundtrip.c -L/usr/local/lib -lmeos
 *   @endcode
 *
 *   Exit status: 0 on success, 1 on round-trip mismatch.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <meos.h>
#include <meos_pointcloud.h>
#include <meos_internal.h>   /* tinstant_make */

/* postgres.h (transitively from meos.h) macro-substitutes printf →
 * pg_printf, but libpgport's pg_printf is not exported from libmeos.so.
 * Restore the libc versions for the standalone example. */
#undef printf
#undef fprintf

/* WKB variant flags (mirrors meos.h definitions) */
#define WKB_NDR  2
#define WKB_HEX  4

/* Hex of the SERIALIZED_POINT bytes for a pcid=1 pcpoint at (1, 1, 1).
 * Layout: [4-byte vl_len_ slot, will be SET_VARSIZE'd by pcpoint_hex_in]
 *         [4-byte pcid little-endian = 0x00000001]
 *         [3 × 4-byte int32 dimensions = (1, 1, 1)]
 * — matching the int32_t dimension schema the test fixture in
 * mobilitydb/datagen/pointcloud/random_tpcpoint.sql installs at pcid=1.
 *
 * Total: 4 + 4 + 12 = 20 bytes = 40 hex characters. */
#define PCPOINT_HEX_PCID1_111 \
  "0000000001000000010000000100000001000000"

static int
roundtrip_one(const char *label, Pcpoint *pcpt, TimestampTz t)
{
  TInstant *inst = tinstant_make(PointerGetDatum(pcpt), T_TPCPOINT, t);
  if (! inst)
  {
    fprintf(stderr, "[%s] tinstant_make returned NULL\n", label);
    return 1;
  }

  /* Encode */
  size_t wkb_len = 0;
  uint8_t *wkb = temporal_as_wkb((Temporal *) inst, WKB_NDR, &wkb_len);
  if (! wkb || wkb_len == 0)
  {
    fprintf(stderr, "[%s] temporal_as_wkb returned empty buffer\n", label);
    return 1;
  }
  printf("[%s] encoded %zu WKB bytes\n", label, wkb_len);

  /* Decode */
  Temporal *decoded = temporal_from_wkb(wkb, wkb_len);
  if (! decoded)
  {
    fprintf(stderr, "[%s] temporal_from_wkb returned NULL\n", label);
    free(wkb);
    return 1;
  }

  /* Compare */
  bool ok = temporal_eq((Temporal *) inst, decoded);
  printf("[%s] round-trip equality: %s\n", label, ok ? "OK" : "MISMATCH");

  free(wkb);
  free(inst);
  free(decoded);
  return ok ? 0 : 1;
}

int
main(void)
{
  meos_initialize();

  int failures = 0;

  /* Single-instant tpcpoint at (1, 1, 1)@2024-01-01. */
  TimestampTz t1 = pg_timestamptz_in("2024-01-01 00:00:00", -1);
  Pcpoint *pt1 = pcpoint_hex_in(PCPOINT_HEX_PCID1_111);
  if (! pt1)
  {
    fprintf(stderr, "pcpoint_hex_in failed on the seed hex\n");
    meos_finalize();
    return EXIT_FAILURE;
  }
  failures += roundtrip_one("tpcpoint(1, 1, 1, 1)@2024-01-01", pt1, t1);

  /* Same value at a different timestamp — pins that the encoder doesn't
   * accidentally hash the timestamp into the value stream. */
  TimestampTz t2 = pg_timestamptz_in("2024-12-31 23:59:59", -1);
  Pcpoint *pt2 = pcpoint_hex_in(PCPOINT_HEX_PCID1_111);
  failures += roundtrip_one("tpcpoint(1, 1, 1, 1)@2024-12-31", pt2, t2);

  meos_finalize();

  if (failures)
  {
    fprintf(stderr, "\n%d round-trip(s) failed\n", failures);
    return EXIT_FAILURE;
  }
  printf("\nAll round-trips OK\n");
  return EXIT_SUCCESS;
}
