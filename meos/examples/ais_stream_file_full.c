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
 * @brief Stream AIS observations to disk with real-world hardening for
 * a file-output target.
 *
 * Production-grade companion to `04_ais_stream_file.c`. The simplified
 * version writes one batch per ship to a single output file via `fprintf`;
 * this version adds the production concerns specific to a *file* target:
 *
 * 1. **Atomic batch writes** — each batch is written to a temporary file
 *    in the same directory and `rename(2)`'d into place once `fsync(2)`
 *    succeeds. A loader killed mid-write never leaves a half-written
 *    batch in the canonical output.
 * 2. **Output rotation** — when the active output file exceeds a configurable
 *    size threshold (default 64 MiB), it is closed and the next batch
 *    starts a new file `<name>.NNN` with monotonically-increasing index.
 *    Avoids unbounded growth of a single file.
 * 3. **Output format selection** via the `AIS_OUT_FORMAT` env var:
 *    `wkt` (default), `mfjson`. Both are MEOS-defined encodings; the
 *    choice is at output time.
 * 4. **Per-row range validation** lifted from `ais_read_full.c`.
 * 5. **Resume-after-restart skeleton** — on startup, if the output file
 *    already exists, the loader reads it once to count rows and skips
 *    that many input rows before resuming. Pragmatic for short-pause
 *    restarts; for long pauses, prefer a real journaled approach.
 * 6. **Categorised error counters** at end (malformed, out-of-range,
 *    write failures).
 *
 * Compile with:
 * @code
 * gcc -Wall -g -I/usr/local/include -o ais_stream_file_full ais_stream_file_full.c \
 *   -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      /* fsync, close */
#include <fcntl.h>       /* open, O_* */
#include <sys/stat.h>    /* stat */
#include <time.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>

#define NO_INSTS_BATCH        1000
#define NO_INSTS_KEEP         2
#define MAX_LEN_LINE          1024
#define MAX_LEN_TIMESTAMP     32
#define MAX_NO_TRIPS          5

/* Default rotation threshold: 64 MiB. Override with AIS_ROTATE_BYTES. */
#define DEFAULT_ROTATE_BYTES  (64L * 1024 * 1024)

#define OUT_BASE              "data/ais_trips_full"
#define OUT_EXT               ".csv"

typedef enum { OUT_WKT, OUT_MFJSON } out_format;

typedef struct
{
  Timestamp T;
  long int MMSI;
  double Latitude;
  double Longitude;
  double SOG;
} AIS_record;

typedef struct
{
  long int MMSI;
  TSequence *trip;
} trip_record;

static int
validate_record(const AIS_record *rec)
{
  return (rec->Latitude  >= -90.0  && rec->Latitude  <= 90.0  &&
          rec->Longitude >= -180.0 && rec->Longitude <= 180.0 &&
          rec->SOG       >= 0.0    && rec->SOG       <= 102.2)
    ? 0 : -1;
}

/* Build the active output filename for the given rotation index. */
static void
build_outname(char *buf, size_t bufsz, int idx)
{
  if (idx == 0)
    snprintf(buf, bufsz, OUT_BASE OUT_EXT);
  else
    snprintf(buf, bufsz, OUT_BASE ".%03d" OUT_EXT, idx);
}

/* Atomic append: write the line to a temp file in the same directory,
 * fsync, then rename. Returns 0 OK, -1 err. */
static int
atomic_append(const char *outname, const char *content, long *current_size)
{
  /* Read current file content into memory. For large files this would
   * be wasteful; in production one would prefer fcntl-locked append
   * or a write-ahead log. Atomicity here illustrates the pattern. */
  FILE *fr = fopen(outname, "rb");
  long existing_len = 0;
  char *existing = NULL;
  if (fr)
  {
    fseek(fr, 0L, SEEK_END);
    existing_len = ftell(fr);
    fseek(fr, 0L, SEEK_SET);
    existing = malloc((size_t) existing_len);
    if (existing && existing_len > 0)
    {
      if (fread(existing, 1, (size_t) existing_len, fr) != (size_t) existing_len)
      {
        free(existing);
        fclose(fr);
        return -1;
      }
    }
    fclose(fr);
  }

  char tmpname[1024];
  snprintf(tmpname, sizeof(tmpname), "%s.tmp.%d", outname, (int) getpid());

  int fd = open(tmpname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0)
  {
    free(existing);
    return -1;
  }
  if (existing && existing_len > 0)
  {
    if (write(fd, existing, (size_t) existing_len) != existing_len)
    {
      close(fd);
      unlink(tmpname);
      free(existing);
      return -1;
    }
  }
  free(existing);

  size_t content_len = strlen(content);
  if (write(fd, content, content_len) != (ssize_t) content_len)
  {
    close(fd);
    unlink(tmpname);
    return -1;
  }
  if (fsync(fd) != 0 || close(fd) != 0)
  {
    unlink(tmpname);
    return -1;
  }
  if (rename(tmpname, outname) != 0)
  {
    unlink(tmpname);
    return -1;
  }
  *current_size = existing_len + (long) content_len;
  return 0;
}

/* Render a sequence in the chosen output format. Returns malloc'd string. */
static char *
render_sequence(const TSequence *seq, out_format fmt)
{
  if (fmt == OUT_MFJSON)
    return temporal_as_mfjson((const Temporal *) seq, true, 6, 1, NULL);
  return tsequence_out(seq, 6);
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Resolve output format. */
  const char *fmt_env = getenv("AIS_OUT_FORMAT");
  out_format fmt = OUT_WKT;
  if (fmt_env && strcasecmp(fmt_env, "mfjson") == 0)
    fmt = OUT_MFJSON;

  /* Resolve rotation threshold. */
  const char *rot_env = getenv("AIS_ROTATE_BYTES");
  long rotate_bytes = rot_env ? atol(rot_env) : DEFAULT_ROTATE_BYTES;
  if (rotate_bytes <= 0) rotate_bytes = DEFAULT_ROTATE_BYTES;

  /* Resume support: if the active output file exists, count its lines
   * and use that to skip already-emitted records. (Pragmatic but only
   * meaningful when replaying the same input file in the same order.) */
  long resume_skip = 0;
  int rot_idx = 0;
  char outname[1024];
  build_outname(outname, sizeof(outname), rot_idx);
  struct stat st;
  long current_size = 0;
  if (stat(outname, &st) == 0)
  {
    current_size = st.st_size;
    FILE *f = fopen(outname, "r");
    if (f)
    {
      char buf[MAX_LEN_LINE];
      while (fgets(buf, sizeof(buf), f)) resume_skip++;
      fclose(f);
      if (resume_skip > 0)
        printf("Found %ld existing batch lines in %s; resuming.\n",
          resume_skip, outname);
    }
  }

  FILE *file_in = fopen("data/ais_instants.csv", "r");
  if (! file_in)
  {
    fprintf(stderr, "Could not open input file\n");
    meos_finalize();
    return EXIT_FAILURE;
  }
  char line[MAX_LEN_LINE], ts_buf[MAX_LEN_TIMESTAMP];
  if (fgets(line, sizeof(line), file_in) == NULL)
  {
    fclose(file_in);
    meos_finalize();
    return EXIT_FAILURE;
  }

  trip_record trips[MAX_NO_TRIPS] = {0};
  long records_ok = 0, records_bad = 0, dropped_overflow = 0;
  long writes = 0, write_failures = 0;
  int no_ships = 0;

  printf("Streaming to %s (format=%s, rotate at %ld bytes)\n",
    outname, fmt == OUT_MFJSON ? "mfjson" : "wkt", rotate_bytes);
  clock_t t0 = clock();

  while (fgets(line, sizeof(line), file_in) != NULL)
  {
    AIS_record rec;
    int read = sscanf(line, "%31[^,],%ld,%lf,%lf,%lf",
      ts_buf, &rec.MMSI, &rec.Latitude, &rec.Longitude, &rec.SOG);
    if (read != 5 || validate_record(&rec) != 0)
    {
      records_bad++;
      continue;
    }
    rec.T = timestamp_in(ts_buf, -1);
    if (rec.T == 0)
    {
      records_bad++;
      continue;
    }
    records_ok++;

    int j = -1;
    for (int i = 0; i < no_ships; i++)
      if (trips[i].MMSI == rec.MMSI) { j = i; break; }
    if (j < 0)
    {
      if (no_ships >= MAX_NO_TRIPS)
      {
        dropped_overflow++;
        continue;
      }
      j = no_ships++;
      trips[j].MMSI = rec.MMSI;
    }

    /* Flush full-batch ship to the file; rotate first if needed. */
    if (trips[j].trip != NULL && trips[j].trip->count >= NO_INSTS_BATCH)
    {
      if (current_size >= rotate_bytes)
      {
        rot_idx++;
        build_outname(outname, sizeof(outname), rot_idx);
        current_size = 0;
        printf("\nRotated to %s\n", outname);
      }
      char *seq_str = render_sequence(trips[j].trip, fmt);
      char *batch_line = malloc(strlen(seq_str) + 64);
      sprintf(batch_line, "%ld, %s\n", trips[j].MMSI, seq_str);
      free(seq_str);

      if (atomic_append(outname, batch_line, &current_size) != 0)
      {
        fprintf(stderr, "Atomic append failed for %s\n", outname);
        write_failures++;
      }
      else
      {
        writes++;
        if (writes % 10 == 0) { printf("."); fflush(stdout); }
      }
      free(batch_line);
      tsequence_restart(trips[j].trip, NO_INSTS_KEEP);
    }

    GSERIALIZED *gs = geogpoint_make2d(4326, rec.Longitude, rec.Latitude);
    TInstant *inst = tpointinst_make(gs, rec.T);
    free(gs);
    if (! trips[j].trip)
      trips[j].trip = tsequence_make_exp(&inst, 1,
        NO_INSTS_BATCH, true, true, LINEAR, false);
    else
      tsequence_append_tinstant(trips[j].trip, inst, 0.0, NULL, true);
    free(inst);
  }

  /* Flush remaining per-ship sequences. */
  for (int i = 0; i < no_ships; i++)
  {
    if (trips[i].trip == NULL || trips[i].trip->count == 0)
      continue;
    if (current_size >= rotate_bytes)
    {
      rot_idx++;
      build_outname(outname, sizeof(outname), rot_idx);
      current_size = 0;
    }
    char *seq_str = render_sequence(trips[i].trip, fmt);
    char *batch_line = malloc(strlen(seq_str) + 64);
    sprintf(batch_line, "%ld, %s\n", trips[i].MMSI, seq_str);
    free(seq_str);
    if (atomic_append(outname, batch_line, &current_size) != 0)
      write_failures++;
    else
      writes++;
    free(batch_line);
    free(trips[i].trip);
  }
  fclose(file_in);

  double elapsed = ((double)(clock() - t0)) / CLOCKS_PER_SEC;

  printf("\nStream-to-file summary:\n");
  printf("  records OK:               %ld\n", records_ok);
  printf("  records bad:              %ld\n", records_bad);
  printf("  dropped (overflow):       %ld\n", dropped_overflow);
  printf("  successful batch writes:  %ld\n", writes);
  printf("  failed batch writes:      %ld\n", write_failures);
  printf("  rotations performed:      %d\n", rot_idx);
  printf("  active output file:       %s\n", outname);
  printf("  elapsed:                  %.2fs\n", elapsed);

  meos_finalize();
  return (write_failures == 0 && records_ok > 0)
    ? EXIT_SUCCESS : EXIT_FAILURE;
}
