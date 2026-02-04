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
 * into a TSEQUENCE of tgeompoint instants
 * and apply an EKF via temporal_ext_kalman_filter().
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


/* Limits and sizes (kept similar to ais_assemble_full.c) */
/* Maximum number of records read in the CSV file */
#define MAX_NO_RECORDS    20000000
/* Maximum number of trips (ships) */
#define MAX_SHIPS         6500
/* Number of records in a batch for printing a marker */
#define NO_RECORDS_BATCH  100000
/* Initial number of allocated instants for an input trip */
#define INITIAL_INSTANTS  64
/* Maximum length in characters of a record in the input CSV file */
#define MAX_LINE_LEN      1024

typedef struct
{
  TimestampTz T;
  long long MMSI;
  double Latitude;
  double Longitude;
  double SOG;
} AIS_record;

typedef struct
{
  long long MMSI;    /* Identifier of the trip */
  int no_records;    /* Number of input records for the trip */
  int n_inst;        /* Number of input instants for the trip */
  int free_inst;     /* Number of available input instants */
  TInstant **inst;   /* Array of instants for the trip */
} trip_record;

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

  /* Input buffers and per-ship storage (mirroring ais_assemble_full.c) */
  char line_buffer[MAX_LINE_LEN];
  trip_record trips[MAX_SHIPS] = {0};
  AIS_record rec;
  int no_records = 0;
  int no_err_records = 0;
  int no_ships = 0;
  int n_points_ok = 0;
  int i, j;
  int exit_value = EXIT_FAILURE;

  FILE *file = fopen(argv[1], "r");
  if (!file)
  {
    fprintf(stderr, "Error opening input file %s\n", argv[1]);
    meos_finalize();
    return EXIT_FAILURE;
  }

  /* Read the first line of the file with the headers */
  if (fscanf(file, "%1023[^\n]\n", line_buffer) != 1)
  {
    fprintf(stderr, "Empty or invalid header in input file\n");
    fclose(file);
    meos_finalize();
    return EXIT_FAILURE;
  }
  printf("Processing records\n");
  printf("  one '*' marker every %d records\n", NO_RECORDS_BATCH);

  /* Continue reading the file, following ais_assemble_full.c logic */
  do
  {
    if (fscanf(file, "%1023[^\n]\n", line_buffer) != 1)
    {
      if (ferror(file))
        fprintf(stderr, "\nError reading input file\n");
      break;
    }

    no_records++;
    if (no_records % NO_RECORDS_BATCH == 0)
    {
      printf("*");
      fflush(stdout);
    }
    if (no_records == MAX_NO_RECORDS)
      break;

    memset(&rec, 0, sizeof(rec));
    int field = 0;
    char *token = strtok(line_buffer, ",");
    bool has_t = false, has_mmsi = false, has_lat = false,
      has_long = false, has_sog = false;
    while (token)
    {
      if (strlen(token) != 0 && strcmp(token, "Unknown") != 0)
      {
        switch (field)
        {
          case 0:
            if (parse_timestamp_eu(token, &rec.T))
              has_t = true;
            break;
          case 2:
            rec.MMSI = strtoll(token, NULL, 0);
            if (rec.MMSI != 0)
              has_mmsi = true;
            break;
          case 3:
            rec.Latitude = strtold(token, NULL);
            if (rec.Latitude >= 40.18 && rec.Latitude <= 84.17)
              has_lat = true;
            break;
          case 4:
            rec.Longitude = strtold(token, NULL);
            if (rec.Longitude >= -16.1 && rec.Longitude <= 32.88)
              has_long = true;
            break;
          case 7:
            rec.SOG = strtold(token, NULL);
            if (rec.SOG >= 0 && rec.SOG <= 1022)
              has_sog = true;
            break;
          default:
            break;
        }
      }
      token = strtok(NULL, ",");
      field++;
      if (field > 7)
        break;
    }

    if (! has_t || ! has_mmsi || ! ( ( has_lat && has_long ) || has_sog) )
    {
      no_err_records++;
      continue;
    }

    /* Find the place to store the new instant */
    j = -1;
    for (i = 0; i < no_ships; i++)
    {
      if (trips[i].MMSI == rec.MMSI)
      {
        j = i;
        break;
      }
    }
    if (j < 0)
    {
      if (no_ships == MAX_SHIPS)
        continue;
      j = no_ships++;
      trips[j].MMSI = rec.MMSI;
      trips[j].no_records = 0;
      trips[j].n_inst = 0;
      trips[j].free_inst = INITIAL_INSTANTS;
      trips[j].inst = (TInstant **) calloc(INITIAL_INSTANTS, sizeof(TInstant *));
      if (!trips[j].inst)
      {
        fprintf(stderr, "\nMMSI: %lld, there is no more memory to expand the trip\n",
          trips[j].MMSI);
        fclose(file);
        goto cleanup;
      }
    }
    trips[j].no_records++;

    /* Create a Trip instant from the record, only if we have valid position */
    if (has_lat && has_long)
    {
      TInstant *inst, **new_instants;
      const TInstant *last;

      GSERIALIZED *gs = geompoint_make2d(4326, rec.Longitude, rec.Latitude);
      inst = tpointinst_make(gs, rec.T);
      free(gs);

      if (trips[j].free_inst == 0)
      {
        new_instants = realloc(trips[j].inst,
          sizeof(TInstant *) * trips[j].n_inst * 2);
        if (! new_instants)
        {
          fprintf(stderr, "\nMMSI: %lld, there is no more memory to expand the trip\n",
            trips[j].MMSI);
          free(inst);
          fclose(file);
          goto cleanup;
        }
        trips[j].inst = new_instants;
        trips[j].free_inst = trips[j].n_inst;
      }

      last = trips[j].n_inst ?
        trips[j].inst[trips[j].n_inst - 1] : NULL;
      if (last && last->t == inst->t)
      {
        free(inst);
      }
      else
      {
        trips[j].inst[trips[j].n_inst++] = inst;
        trips[j].free_inst--;
        n_points_ok++;
      }
    }
  } while (! feof(file));
  fclose(file);

  printf("\n%d records read.\n%d erroneous records ignored.\n", no_records, no_err_records);
  printf("%d trips read, %d trip instants accepted.\n", no_ships, n_points_ok);

  /************************************************************************/
  /* After creating the trips , we preform EKF cleaning per trip */

  /* Output config */
  const char *outpath = (argc >= 3 ? argv[2] : "ais_ekf_clean_out.csv");
  bool to_drop = true;            /* default: drop */
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
  /* Prepare output CSV file */
  FILE *fout = fopen(outpath, "w");
  if (!fout)
    fprintf(stderr, "Could not open output file %s (printing to stdout only)\n", outpath);
  else
    fprintf(fout, "MMSI,CleanTrajectoryEWKT\n");

  /* Prepare removed-points CSV path (derived from outpath) */
  char removed_path[1024];
  removed_path[0] = '\0';
  FILE *fremoved = NULL;
  if (fout)
  {
    const char *dot = strrchr(outpath, '.');
    size_t base_len = dot ? (size_t)(dot - outpath) : strlen(outpath);
    if (base_len > sizeof(removed_path) - 16) base_len = sizeof(removed_path) - 16;
    memcpy(removed_path, outpath, base_len);
    removed_path[base_len] = '\0';
    strcat(removed_path, "_removed.csv");
    fremoved = fopen(removed_path, "w");
    if (fremoved)
      fprintf(fremoved, "MMSI,Timestamp,Longitude,Latitude\n");
  }

  /* Process each trip_record */
  for (i = 0; i < no_ships; i++)
  {
    if (trips[i].n_inst == 0)
      continue;

    /* Build sequence and run EKF filter ----------------*/
    TSequence *seq = tsequence_make((TInstant **) trips[i].inst, trips[i].n_inst,
      true, true, LINEAR, false);

    /* Free instants and the array after building the sequence */
    for (j = 0; j < trips[i].n_inst; j++)
      free(trips[i].inst[j]);
    free(trips[i].inst);
    trips[i].inst = NULL; trips[i].n_inst = trips[i].free_inst = 0;

    Temporal *clean = temporal_ext_kalman_filter((Temporal *) seq, gate, q, r, to_drop);


    /* Get EWKT of cleaned trajectory */
    GSERIALIZED *traj = tpoint_trajectory(clean ? clean : (Temporal *) seq, false);
    char *ewkt = traj ? geo_as_ewkt(traj, 10) : NULL;
    if (fout)
      fprintf(fout, "%lld,\"%s\"\n", trips[i].MMSI, ewkt ? ewkt : "");
    if (ewkt) free(ewkt);
    if (traj) free(traj);

    /* If drop mode and removed CSV is open, write removed points (we compare the timestamps not present in cleaned) */
    int removed_count = 0;
    if (to_drop && fremoved && clean)
    {
      const TSequence *cseq = (const TSequence *) clean;
      int iraw = 0, icln = 0;
      int nraw = seq ? seq->count : 0;
      int ncln = cseq ? cseq->count : 0;
      while (iraw < nraw)
      {
        TInstant *ir = temporal_instant_n((const Temporal *) seq, iraw + 1);
        TimestampTz tr = ir->t;
        /* Advance cleaned pointer until >= raw time */
        while (icln < ncln)
        {
          TInstant *icpeek = temporal_instant_n((const Temporal *) cseq, icln + 1);
          if (icpeek->t < tr) { free(icpeek); icln++; continue; }
          /* icpeek->t >= tr */
          if (icpeek->t == tr)
          {
            /* kept */
            free(icpeek); free(ir); iraw++; icln++; goto next_raw;
          }
          /* icpeek->t > tr -> removed */
          free(icpeek);
          break;
        }
        {
          /* Removed point: write row */
          char *ts = pg_timestamptz_out(tr);
          /* Get EWKT for the single instant and parse point coords */
          char *wkt = tspatial_as_ewkt((Temporal *) ir, 10); /* e.g., SRID=4326;POINT(x y)@time */
          double lon = 0.0, lat = 0.0;
          if (wkt)
          {
            char *p = strchr(wkt, '(');
            char *q = strchr(wkt, ')');
            if (p && q && q > p) sscanf(p+1, "%lf %lf", &lon, &lat);
          }
          fprintf(fremoved, "%lld,%s,%.10f,%.10f\n", trips[i].MMSI, ts ? ts : "", lon, lat);
          if (ts) free(ts);
          if (wkt) free(wkt);
          free(ir); iraw++;
          removed_count++;
        }
next_raw:
        ;
      }
    }

    if (clean) free(clean);
    if (seq) free(seq);
    if (to_drop)
      printf("MMSI %lld cleaned (removed=%d).\n", trips[i].MMSI, removed_count);
    else
      printf("MMSI %lld cleaned.\n", trips[i].MMSI);
  }

  if (fout) fclose(fout);
  if (fremoved) fclose(fremoved);

  exit_value = EXIT_SUCCESS;

cleanup:

  /* Free any remaining per-ship buffers (in case of early error) */
  for (i = 0; i < no_ships; i++)
  {
    if (trips[i].inst)
    {
      for (j = 0; j < trips[i].n_inst; j++)
        free(trips[i].inst[j]);
      free(trips[i].inst);
    }
  }

  meos_finalize();
  return exit_value;
}
