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
 * @brief A tutorial program that reads one full day of AIS data from a CSV
 * file provided by the Danish Maritime Authority in https://web.ais.dk/aisdata/
 * and constructs for each ship a temporal rigid geometry (`trgeometry`),
 * where the reference geometry is the pentagon-shaped hull derived from
 * the AIS antenna offsets (A, B, C, D) and the pose path is the projected
 * antenna position plus the heading.
 *
 * The program then computes the temporal distance `tdistance_trgeo_trgeo`
 * between every pair of temporally-overlapping trips and prints, for each
 * pair, the moment of closest approach (timestamp + distance in metres).
 *
 * The hull is built in the local frame in metres with antenna at the origin,
 * x-axis pointing forward (bow) and y-axis pointing to the left (port) — the
 * convention used by the maritime dataset. The pentagon has corners
 *
 *     1. (-B,  D)            stern-port
 *     2. (-B, -C)            stern-starboard
 *     3. ((3A-B)/4, -C)      bow-rectangle-starboard
 *     4. (A, D - C/2)        bow tip
 *     5. ((3A-B)/4,  D)      bow-rectangle-port
 *
 * which is a rectangle whose forward quarter is replaced by a triangle so
 * that the bow can be visually distinguished from the stern.
 *
 * Coordinates are projected from WGS84 (EPSG:4326) to EPSG:25832 (UTM 32N)
 * because the trgeometry pose translation must share the metric system of
 * the hull. Heading (degrees clockwise from N) is converted to pose theta
 * (radians counter-clockwise from +x) by `theta = (90° - heading) * π/180`.
 *
 * Please notice that the `data` directory DOES NOT contain the input CSV
 * file; download `aisdk-2026-02-26.csv` from the website above.
 *
 * The program assumes that the input file fits the size constraints below
 * (max 64 ships, max 5000 instants per ship). The defaults are 8 ships and
 * 500 instants per ship; both can be overridden on the command line.
 *
 * The program is intentionally short (single C file, only public MEOS API
 * + one extern for `trgeoseq_make`) so that the canonical pipeline
 *
 *      AIS CSV → trgeometry trips → trgeo/trgeo distance → closest approach
 *
 * fits in one read.
 *
 * Build:
 *   gcc -Wall -O2 -I/usr/local/include -o trgeo_distance \
 *       trgeo_distance.c -L/usr/local/lib -lmeos -lm
 *
 * Run:
 *   ./trgeo_distance [max_trips] [max_instants_per_trip]
 *   defaults: max_trips = 8, max_instants_per_trip = 500
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_rgeo.h>

/* The trgeometry-specific sequence constructor lives in
 * `meos/include/rgeo/trgeo_seq.h`, which pulls in `postgres.h`. We declare
 * the symbol directly so the tutorial stays in pure MEOS-public-API land. */
extern TSequence *trgeoseq_make(const GSERIALIZED *geom, TInstant **instants,
  int count, bool lower_inc, bool upper_inc, interpType interp, bool normalize);

#define CSV_PATH "/home/esteban/src/MobilityDB/meos/examples/data/aisdk-2026-02-26.csv"
#define MAX_LINE 1024
#define MAX_TRIPS_HARD 64
#define MAX_INSTANTS_HARD 5000

typedef struct {
  long mmsi;
  /* Ship dimensions A B C D in metres (antenna offsets to bow / stern /
   * port / starboard, AIS convention). */
  int a, b, c, d;
  char ref_wkt[256];               /* pentagon hull WKT in local frame */
  int n_inst;
  TInstant *insts[MAX_INSTANTS_HARD];
  TimestampTz t_start;
  TimestampTz t_end;
  Temporal *trgeo;                 /* assembled trgeometry */
} trip_t;

/*============================================================================
 * AIS CSV helpers
 *==========================================================================*/

/**
 * @brief Parse an AIS timestamp (`DD/MM/YYYY HH:MM:SS`) into TimestampTz.
 */
static int
parse_ais_timestamp(const char *s, TimestampTz *out)
{
  int dd, mm, yyyy, hh, mi, ss;
  if (sscanf(s, "%d/%d/%d %d:%d:%d", &dd, &mm, &yyyy, &hh, &mi, &ss) != 6)
    return 0;
  char iso[40];
  snprintf(iso, sizeof(iso), "%04d-%02d-%02d %02d:%02d:%02d+00",
    yyyy, mm, dd, hh, mi, ss);
  *out = pg_timestamptz_in(iso, -1);
  return 1;
}

/**
 * @brief In-place split a CSV line into NUL-terminated fields. Returns the
 * field count.
 */
static int
csv_split(char *line, char **fields, int max_fields)
{
  int n = 0;
  char *p = line;
  fields[n++] = p;
  while (*p && n < max_fields) {
    if (*p == ',') { *p = '\0'; fields[n++] = p + 1; }
    p++;
  }
  char *nl = strpbrk(fields[n-1], "\r\n");
  if (nl) *nl = '\0';
  return n;
}

/*============================================================================
 * Hull construction: pentagon from AIS antenna offsets
 *==========================================================================*/

/**
 * @brief Write the hull WKT for a ship with antenna offsets A B C D into
 * the per-trip slot. The pentagon is in EPSG:25832 metric units; SRID is
 * applied by the caller when re-parsing it.
 */
static void
build_hull_wkt(trip_t *trip, int A, int B, int C, int D)
{
  trip->a = A; trip->b = B; trip->c = C; trip->d = D;
  double A_rect = (3.0 * A - B) / 4.0;
  double bow_y  = D - C / 2.0;
  snprintf(trip->ref_wkt, sizeof(trip->ref_wkt),
    "POLYGON((-%d %d, -%d -%d, %.3f -%d, %d %.3f, %.3f %d, -%d %d))",
    B, D, B, C, A_rect, C, A, bow_y, A_rect, D, B, D);
}

/*============================================================================
 * Pose construction: project antenna position and convert heading
 *==========================================================================*/

/**
 * @brief Build a Pose from one AIS record. The antenna is projected from
 * WGS84 to EPSG:25832; the heading (degrees clockwise from N) becomes a
 * pose theta in radians counter-clockwise from +x. The boundary value
 * theta = ±π is nudged by 1e-7 to satisfy the half-open MEOS validator.
 */
static Pose *
make_pose_from_ais(double lon, double lat, double heading_deg)
{
  /* Project the antenna position to EPSG:25832 */
  char wgs_pt[80];
  snprintf(wgs_pt, sizeof(wgs_pt),
    "SRID=4326;Point(%.6f %.6f)", lon, lat);
  GSERIALIZED *gs_wgs = geom_in(wgs_pt, -1);
  if (! gs_wgs) return NULL;
  GSERIALIZED *gs_utm = geo_transform(gs_wgs, 25832);
  free(gs_wgs);
  if (! gs_utm) return NULL;
  char *gs_utm_wkt = geo_as_text(gs_utm, 3);
  free(gs_utm);
  double xm = 0, ym = 0;
  if (! gs_utm_wkt || sscanf(gs_utm_wkt, "POINT(%lf %lf)", &xm, &ym) != 2) {
    free(gs_utm_wkt);
    return NULL;
  }
  free(gs_utm_wkt);

  /* Heading → theta */
  double theta = (90.0 - heading_deg) * M_PI / 180.0;
  while (theta > M_PI)  theta -= 2.0 * M_PI;
  while (theta < -M_PI) theta += 2.0 * M_PI;
  const double THETA_EPS = 1e-7;
  if (theta >  M_PI - THETA_EPS) theta =  M_PI - THETA_EPS;
  if (theta < -M_PI + THETA_EPS) theta = -M_PI + THETA_EPS;

  char pose_str[128];
  snprintf(pose_str, sizeof(pose_str),
    "SRID=25832;Pose(Point(%.3f %.3f), %.9f)", xm, ym, theta);
  return pose_in(pose_str);
}

/*============================================================================
 * Main
 *==========================================================================*/

int
main(int argc, char **argv)
{
  int max_trips = (argc > 1) ? atoi(argv[1]) : 8;
  int max_inst  = (argc > 2) ? atoi(argv[2]) : 500;
  if (max_trips < 2 || max_trips > MAX_TRIPS_HARD) max_trips = 8;
  if (max_inst < 10 || max_inst > MAX_INSTANTS_HARD) max_inst = 500;

  meos_initialize();
  meos_initialize_timezone("UTC");

  FILE *f = fopen(CSV_PATH, "r");
  if (! f) {
    fprintf(stderr, "open %s: failed\n", CSV_PATH);
    return 1;
  }

  trip_t trips[MAX_TRIPS_HARD] = {0};
  int n_trips = 0;
  char line[MAX_LINE];
  char *fields[32];
  int rec_in = 0, rec_used = 0;

  /* Skip header */
  if (! fgets(line, sizeof(line), f))
    return 1;

  /*--- 1. Read AIS records and build trgeometry instants ----------------*/
  while (fgets(line, sizeof(line), f)) {
    rec_in++;
    int nf = csv_split(line, fields, 32);
    if (nf < 26) continue;
    /* Class A only, with valid lat/lon, heading, and ABCD */
    if (strcmp(fields[1], "Class A") != 0) continue;
    if (! fields[3][0] || ! fields[4][0] || ! fields[9][0]) continue;
    if (! fields[22][0] || ! fields[23][0] ||
        ! fields[24][0] || ! fields[25][0]) continue;
    long mmsi = strtol(fields[2], NULL, 10);
    if (mmsi <= 0) continue;
    double lat = strtod(fields[3], NULL);
    double lon = strtod(fields[4], NULL);
    double heading_deg = strtod(fields[9], NULL);
    if (heading_deg >= 360.0) continue;            /* 511 = unknown */
    int A = atoi(fields[22]), B = atoi(fields[23]);
    int C = atoi(fields[24]), D = atoi(fields[25]);
    if (A + B < 5 || C + D < 2) continue;

    /* Find or create per-MMSI slot */
    int slot = -1;
    for (int i = 0; i < n_trips; i++)
      if (trips[i].mmsi == mmsi) { slot = i; break; }
    if (slot == -1) {
      if (n_trips >= max_trips) continue;
      slot = n_trips++;
      trips[slot].mmsi = mmsi;
      build_hull_wkt(&trips[slot], A, B, C, D);
    }
    if (trips[slot].n_inst >= max_inst) continue;

    TimestampTz t;
    if (! parse_ais_timestamp(fields[0], &t)) continue;

    /* Build the trgeometry instant */
    char ref_with_srid[300];
    snprintf(ref_with_srid, sizeof(ref_with_srid), "SRID=25832;%s",
      trips[slot].ref_wkt);
    GSERIALIZED *refgeom = geom_in(ref_with_srid, -1);
    if (! refgeom) continue;
    Pose *pose = make_pose_from_ais(lon, lat, heading_deg);
    if (! pose) { free(refgeom); continue; }
    TInstant *inst = trgeoinst_make(refgeom, pose, t);
    free(refgeom); free(pose);
    if (! inst) continue;

    /* Drop instants that don't strictly advance time. */
    if (trips[slot].n_inst > 0 && t <= trips[slot].t_end) {
      free(inst);
      continue;
    }
    trips[slot].insts[trips[slot].n_inst++] = inst;
    if (trips[slot].n_inst == 1) trips[slot].t_start = t;
    trips[slot].t_end = t;
    rec_used++;

    /* Stop reading once every slot is full */
    int all_full = (n_trips >= max_trips);
    for (int i = 0; i < n_trips && all_full; i++)
      if (trips[i].n_inst < max_inst) all_full = 0;
    if (all_full) break;
  }
  fclose(f);

  fprintf(stderr, "Read %d AIS records, used %d, built %d trip(s).\n\n",
    rec_in, rec_used, n_trips);

  /*--- 2. Assemble each instant array into a trgeometry sequence -------*/
  for (int i = 0; i < n_trips; i++) {
    if (trips[i].n_inst < 2) continue;
    char ref_with_srid[300];
    snprintf(ref_with_srid, sizeof(ref_with_srid), "SRID=25832;%s",
      trips[i].ref_wkt);
    GSERIALIZED *refgeom = geom_in(ref_with_srid, -1);
    /* The trgeometry-specific constructor wires the per-sequence
     * reference-geometry pointer (the generic tsequence_make does not). */
    trips[i].trgeo = (Temporal *) trgeoseq_make(refgeom, trips[i].insts,
      trips[i].n_inst, true, true, LINEAR, true);
    free(refgeom);
    fprintf(stderr,
      "  trip mmsi=%ld n_inst=%d  hull %dx%d m  (A=%d B=%d C=%d D=%d)\n",
      trips[i].mmsi, trips[i].n_inst, trips[i].a + trips[i].b,
      trips[i].c + trips[i].d, trips[i].a, trips[i].b, trips[i].c, trips[i].d);
  }

  /*--- 3. Pairwise tdistance + closest-approach summary ----------------*/
  printf("\n%-12s %-12s %8s %14s %20s %10s\n",
    "ship A", "ship B", "marks", "min dist (m)",
    "closest-approach time", "elapsed");
  printf("------------ ------------ -------- -------------- "
    "-------------------- ----------\n");

  int pairs_total = 0, pairs_overlap = 0;
  long total_marks = 0;
  double total_secs = 0.0;
  for (int i = 0; i < n_trips; i++) for (int j = i + 1; j < n_trips; j++) {
    pairs_total++;
    if (! trips[i].trgeo || ! trips[j].trgeo) continue;
    if (trips[i].t_end < trips[j].t_start ||
        trips[j].t_end < trips[i].t_start) continue;
    pairs_overlap++;

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    Temporal *d = tdistance_trgeo_trgeo(trips[i].trgeo, trips[j].trgeo);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double secs = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;

    int n_marks = d ? temporal_num_instants(d) : 0;
    double dmin = d ? tfloat_min_value(d) : 0.0;
    char *tmin_str = NULL;
    if (d) {
      Temporal *dat_min = temporal_at_min(d);
      if (dat_min) {
        TimestampTz tmin = temporal_start_timestamptz(dat_min);
        tmin_str = pg_timestamptz_out(tmin);
        free(dat_min);
      }
    }
    printf("%-12ld %-12ld %8d %14.2f %20s %8.4f s\n",
      trips[i].mmsi, trips[j].mmsi, n_marks, dmin,
      tmin_str ? tmin_str : "-", secs);
    free(tmin_str);
    total_marks += n_marks;
    total_secs  += secs;
    if (d) free(d);
  }

  printf("\n%d trip(s), %d pair(s) total, %d overlapping.\n",
    n_trips, pairs_total, pairs_overlap);
  if (pairs_overlap > 0)
    printf("avg per-pair: %.0f marks, %.4f s\n",
      (double) total_marks / pairs_overlap, total_secs / pairs_overlap);

  meos_finalize();
  return 0;
}
