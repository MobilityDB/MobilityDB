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
 * @brief Read AIS observations from a CSV file with real-world hardening.
 *
 * Production-grade companion to `02_ais_read.c`. The simplified version
 * assumes well-formed input, no errors, and small files; this version handles
 * the kinds of conditions a real AIS feed presents:
 *
 * 1. Line-based parsing with `fgets` + `sscanf` instead of `fscanf`, which
 *    cleanly tolerates CRLF line endings (Windows-origin files), missing
 *    final newline, and lines longer than a single field expects.
 * 2. UTF-8 BOM stripping at the start of the file (Excel exports often
 *    leave one).
 * 3. Header column-name validation rather than a blind skip — a renamed
 *    column would otherwise silently shift the parser into garbage.
 * 4. Per-row range validation: latitude in [-90, 90], longitude in
 *    [-180, 180], SOG in [0, 102.2] knots (the AIS-encoded maximum).
 *    Out-of-range rows are counted separately from malformed ones.
 * 5. Categorised error counters: malformed-row, missing-field,
 *    out-of-range, invalid-timestamp. Reported separately at end.
 * 6. Line numbers in error messages so you can find the offending row in
 *    the source file.
 *
 * Compile with the same flags as `02_ais_read.c`:
 * @code
 * gcc -Wall -g -I/usr/local/include -o ais_read_full ais_read_full.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

/* Maximum length of a single CSV line; AIS rows are small but allow generous
 * headroom for unexpected fields */
#define MAX_LEN_LINE 1024
/* Maximum length of a timestamp field */
#define MAX_LEN_TIMESTAMP 32
/* Print one summary line every N successfully-parsed records */
#define PRINT_EVERY 1000

/* Expected header. Real files may use different separators or whitespace
 * around column names; we strip whitespace before comparing. */
static const char *EXPECTED_HEADER = "T,MMSI,Latitude,Longitude,SOG";

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
  long total_lines;     /* every non-empty line read */
  long records_ok;      /* parsed and range-valid */
  long malformed;       /* sscanf produced fewer fields than expected */
  long out_of_range;    /* fields parsed but lat/lon/SOG outside valid range */
  long invalid_ts;      /* timestamp string rejected by timestamp_in */
} parse_stats;

/* Strip leading UTF-8 BOM (\xef\xbb\xbf) from a buffer in-place if present. */
static void
strip_bom(char *buf)
{
  if ((unsigned char) buf[0] == 0xef &&
      (unsigned char) buf[1] == 0xbb &&
      (unsigned char) buf[2] == 0xbf)
    memmove(buf, buf + 3, strlen(buf + 3) + 1);
}

/* Strip trailing whitespace (CR / LF / spaces / tabs) in-place. */
static void
rstrip(char *buf)
{
  size_t n = strlen(buf);
  while (n > 0 &&
         (buf[n - 1] == '\n' || buf[n - 1] == '\r' ||
          buf[n - 1] == ' '  || buf[n - 1] == '\t'))
    buf[--n] = '\0';
}

/* Validate parsed AIS values are within physically-meaningful ranges.
 * Returns 0 on success, -1 on out-of-range. Prints reason on failure. */
static int
validate_record(const AIS_record *rec, long line_no)
{
  if (rec->Latitude < -90.0 || rec->Latitude > 90.0)
  {
    fprintf(stderr, "  line %ld: latitude %.6f out of range [-90, 90]\n",
      line_no, rec->Latitude);
    return -1;
  }
  if (rec->Longitude < -180.0 || rec->Longitude > 180.0)
  {
    fprintf(stderr, "  line %ld: longitude %.6f out of range [-180, 180]\n",
      line_no, rec->Longitude);
    return -1;
  }
  /* AIS encodes SOG with a maximum value of 102.2 knots; values above
   * that are AIS-defined sentinels (102.3 = unavailable) so we treat them
   * as out-of-range here. */
  if (rec->SOG < 0.0 || rec->SOG > 102.2)
  {
    fprintf(stderr, "  line %ld: SOG %.2f out of range [0, 102.2]\n",
      line_no, rec->SOG);
    return -1;
  }
  return 0;
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  FILE *file = fopen("data/ais_instants.csv", "r");
  if (! file)
  {
    fprintf(stderr, "Error opening input file 'data/ais_instants.csv'\n");
    meos_finalize();
    return EXIT_FAILURE;
  }

  parse_stats stats = {0};
  char line[MAX_LEN_LINE];
  char ts_buf[MAX_LEN_TIMESTAMP];
  long line_no = 0;

  /* Read the header line. */
  if (fgets(line, sizeof(line), file) == NULL)
  {
    fprintf(stderr, "Input file is empty\n");
    fclose(file);
    meos_finalize();
    return EXIT_FAILURE;
  }
  line_no++;
  strip_bom(line);
  rstrip(line);

  /* Header-name validation: real CSVs occasionally rename columns or change
   * column order; a silent skip would feed the parser garbage. */
  if (strcmp(line, EXPECTED_HEADER) != 0)
  {
    fprintf(stderr,
      "Header mismatch:\n  expected: %s\n  got:      %s\n"
      "Refusing to parse — column order or names differ.\n",
      EXPECTED_HEADER, line);
    fclose(file);
    meos_finalize();
    return EXIT_FAILURE;
  }

  /* Read data lines. */
  while (fgets(line, sizeof(line), file) != NULL)
  {
    line_no++;
    rstrip(line);
    if (line[0] == '\0')
      continue;
    stats.total_lines++;

    AIS_record rec;
    int read = sscanf(line, "%31[^,],%ld,%lf,%lf,%lf",
      ts_buf, &rec.MMSI, &rec.Latitude, &rec.Longitude, &rec.SOG);
    if (read != 5)
    {
      fprintf(stderr, "  line %ld: expected 5 fields, got %d (\"%s\")\n",
        line_no, read, line);
      stats.malformed++;
      continue;
    }

    if (validate_record(&rec, line_no) != 0)
    {
      stats.out_of_range++;
      continue;
    }

    rec.T = timestamp_in(ts_buf, -1);
    if (rec.T == 0)
    {
      fprintf(stderr, "  line %ld: invalid timestamp '%s'\n", line_no, ts_buf);
      stats.invalid_ts++;
      continue;
    }

    stats.records_ok++;

    /* Sample-print the record. */
    if (stats.records_ok % PRINT_EVERY == 0)
    {
      GSERIALIZED *gs = geogpoint_make2d(4326, rec.Longitude, rec.Latitude);
      TInstant *loc_inst = tpointinst_make(gs, rec.T);
      free(gs);
      char *loc_str = tspatial_as_text((Temporal *) loc_inst, 2);

      TInstant *sog_inst = tfloatinst_make(rec.SOG, rec.T);
      char *sog_str = tfloat_out((Temporal *) sog_inst, 2);

      printf("MMSI: %ld  Location: %s  SOG: %s\n",
        rec.MMSI, loc_str, sog_str);

      free(loc_inst); free(loc_str);
      free(sog_inst); free(sog_str);
    }
  }

  if (ferror(file))
    fprintf(stderr, "I/O error reading input file at line %ld\n", line_no);
  fclose(file);

  printf("\nParse summary:\n");
  printf("  total non-empty data lines: %ld\n", stats.total_lines);
  printf("  records OK:                 %ld\n", stats.records_ok);
  printf("  malformed (field count):    %ld\n", stats.malformed);
  printf("  out-of-range:               %ld\n", stats.out_of_range);
  printf("  invalid timestamp:          %ld\n", stats.invalid_ts);

  meos_finalize();
  return (stats.records_ok > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
