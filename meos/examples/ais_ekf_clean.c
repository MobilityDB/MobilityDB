/**
 * ***************************************************************************
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
 * ***************************************************************************
 */

/**
 * @file
 * @brief Example: read AIS CSV, build TSEQUENCE per MMSI, clean with EKF.
 *
 * Input CSV columns, first row is a header:
 * Timestamp,Type of mobile,MMSI,Latitude,Longitude, ... (other columns ignored)
 * Example timestamp format: "01/03/2024 00:00:00" (DD/MM/YYYY HH:MM:SS)
 *
 * For each MMSI we aggregate geographic positions (Longitude, Latitude)
 * into a TSEQUENCE of tgeompoint instants (2D, SRID unknown, geodetic=false),
 * and apply an EKF (constant-velocity) via temporal_ext_kalman_filter().
 *
 * Build:
 *  gcc -Wall -O2 -I/usr/local/include -o ais_ekf_clean meos/examples/ais_ekf_clean.c \
 *    -L/usr/local/lib -lmeos
 *
 * Run:
 *  ./ais_ekf_clean input.csv [output.csv] [drop|fill] [gate_sigma] [q=...] [r=...]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include <meos.h>
#include <meos_geo.h>

/* Fallbacks for Datum helpers when not provided by headers */
#ifndef PointerGetDatum
#define PointerGetDatum(X) ((Datum) (X))
#endif
#ifndef DatumGetPointer
#define DatumGetPointer(X) ((void *) (X))
#endif

/* Limits for this example */
#define MAX_SHIPS         4096
#define INITIAL_INSTANTS  64
#define MAX_LINE_LEN      4096

typedef struct
{
  TimestampTz t;
  long long mmsi;
  double lon;
  double lat;
} Rec;

typedef struct
{
  long long mmsi;
  int n_inst;
  int cap_inst;
  TInstant **inst; /* T_TGEOMPOINT instants */
} Track;

static bool parse_timestamp_eu(const char *s, TimestampTz *out)
{
  /* Expect DD/MM/YYYY HH:MM:SS (no timezone). Interpret as UTC. */
  int d=0,m=0,y=0,hh=0,mm=0,ss=0;
  if (!s || strlen(s) < 10)
    return false;
  int matched = sscanf(s, "%d/%d/%d %d:%d:%d", &d, &m, &y, &hh, &mm, &ss);
  if (matched < 3)
    return false;
  if (matched < 6)
    hh = mm = ss = 0;
  char iso[32];
  snprintf(iso, sizeof(iso), "%04d-%02d-%02d %02d:%02d:%02d", y, m, d, hh, mm, ss);
  *out = pg_timestamptz_in(iso, -1);
  return true;
}

static bool is_nonempty_token(const char *tok)
{
  return tok && *tok && strcmp(tok, "Unknown") != 0;
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    fprintf(stderr, "Usage: %s input.csv [output.csv] [drop|fill] [gate] [q=...] [r=...]\n", argv[0]);
    return EXIT_FAILURE;
  }

  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  FILE *fp = fopen(argv[1], "r");
  if (!fp)
  {
    fprintf(stderr, "Could not open %s\n", argv[1]);
    meos_finalize();
    return EXIT_FAILURE;
  }

  /* Skip header */
  char line[MAX_LINE_LEN];
  if (!fgets(line, sizeof(line), fp))
  {
    fprintf(stderr, "Empty file\n");
    fclose(fp);
    meos_finalize();
    return EXIT_FAILURE;
  }

  Track tracks[MAX_SHIPS] = {0};
  int n_tracks = 0;
  int n_rows = 0, n_rows_ok = 0;

  while (fgets(line, sizeof(line), fp))
  {
    n_rows++;
    /* Extract required columns */
    char *tokens[5] = {0};
    int idx = 0;
    for (char *tok = strtok(line, ","); tok && idx < 5; tok = strtok(NULL, ","))
      tokens[idx++] = tok;
    if (idx < 5) continue;

    if (!is_nonempty_token(tokens[0]) || !is_nonempty_token(tokens[2]) ||
        !is_nonempty_token(tokens[3]) || !is_nonempty_token(tokens[4]))
      continue;

    /* Optional: skip Base Station rows */
    if (tokens[1] && strncmp(tokens[1], "Base Station", 12) == 0)
      continue;

    Rec r = {0};
    if (!parse_timestamp_eu(tokens[0], &r.t))
      continue;
    r.mmsi = strtoll(tokens[2], NULL, 10);
    r.lat = strtod(tokens[3], NULL);
    r.lon = strtod(tokens[4], NULL);

    /* Find track */
    int j = -1;
    for (int i = 0; i < n_tracks; i++)
      if (tracks[i].mmsi == r.mmsi) { j = i; break; }
    if (j == -1)
    {
      if (n_tracks == MAX_SHIPS)
        continue;
      j = n_tracks++;
      tracks[j].mmsi = r.mmsi;
      tracks[j].n_inst = 0;
      tracks[j].cap_inst = INITIAL_INSTANTS;
      tracks[j].inst = (TInstant **) calloc((size_t)INITIAL_INSTANTS, sizeof(TInstant *));
      if (!tracks[j].inst)
      {
        fprintf(stderr, "Out of memory\n");
        fclose(fp);
        meos_finalize();
        return EXIT_FAILURE;
      }
    }

    /* Make a 2D tgeompoint instant with SRID unknown (0), geodetic=false */
    GSERIALIZED *gs = geompoint_make2d(4326, r.lon, r.lat);
    TInstant *ti = tpointinst_make(gs, r.t);
    free(gs);

    int n = tracks[j].n_inst;
    if (n > 0 && tracks[j].inst[n-1]->t == ti->t)
    { free(ti); continue; }
    if (n == tracks[j].cap_inst)
    {
      int newcap = tracks[j].cap_inst * 2;
      TInstant **tmp = (TInstant **) realloc(tracks[j].inst, (size_t)newcap * sizeof(TInstant *));
      if (!tmp) { fprintf(stderr, "Out of memory (expand)\n"); free(ti); break; }
      tracks[j].inst = tmp; tracks[j].cap_inst = newcap;
    }
    tracks[j].inst[tracks[j].n_inst++] = ti;
    n_rows_ok++;
  }
  fclose(fp);

  printf("Read %d rows, accepted %d points, built %d tracks.\n", n_rows, n_rows_ok, n_tracks);

  /* Output config */
  const char *outpath = (argc >= 3 ? argv[2] : "ais_ekf_clean_out.csv");
  bool to_drop = false;           /* default: fill */
  double gate = 8.0;              /* Mahalanobis threshold in sigmas */
  double q = 5e-10;               /* process noise (deg^2/s^4) default */
  double r = 4e-6;                /* measurement noise (deg^2) default */
  for (int ai = 3; ai < argc; ai++)
  {
    if (strcmp(argv[ai], "drop") == 0) to_drop = true;
    else if (strcmp(argv[ai], "fill") == 0) to_drop = false;
    else if (strncmp(argv[ai], "q=", 2) == 0) q = strtod(argv[ai]+2, NULL);
    else if (strncmp(argv[ai], "r=", 2) == 0) r = strtod(argv[ai]+2, NULL);
    else {
      char *endp = NULL; double v = strtod(argv[ai], &endp);
      if (endp && endp != argv[ai]) gate = v;
    }
  }

  FILE *fout = fopen(outpath, "w");
  if (!fout)
    fprintf(stderr, "Could not open output file %s (printing to stdout only)\n", outpath);
  else
    fprintf(fout, "MMSI,CleanTrajectoryEWKT\n");

  for (int i = 0; i < n_tracks; i++)
  {
    if (tracks[i].n_inst == 0)
      continue;

    /* Build sequence and run EKF filter */
    TSequence *seq = tsequence_make((const TInstant **) tracks[i].inst, tracks[i].n_inst,
      true, true, LINEAR, false);
    /* Free instants and the array after building the sequence */
    for (int j = 0; j < tracks[i].n_inst; j++)
      free(tracks[i].inst[j]);
    free(tracks[i].inst);
    tracks[i].inst = NULL; tracks[i].n_inst = tracks[i].cap_inst = 0;

    Temporal *clean = temporal_ext_kalman_filter((Temporal *) seq, gate, q, r, to_drop);

    GSERIALIZED *traj = tpoint_trajectory(clean ? clean : (Temporal *) seq, false);
    char *ewkt = traj ? geo_as_ewkt(traj, 10) : NULL;
    if (fout)
      fprintf(fout, "%lld,\"%s\"\n", tracks[i].mmsi, ewkt ? ewkt : "");
    if (ewkt) free(ewkt);
    if (traj) free(traj);

    if (clean) free(clean);
    if (seq) free(seq);
    printf("MMSI %lld cleaned.\n", tracks[i].mmsi);
  }

  if (fout) fclose(fout);

  meos_finalize();
  return EXIT_SUCCESS;
}
