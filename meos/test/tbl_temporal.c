/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief A simple program that reads the CSV file of every temporal type and
 * applies a function to the values it holds
 *
 * The corresponding SQL query would be, for each temporal type in turn,
 * @code
 * SELECT k, numInstants(tprecision(temp, interval '5 minutes', timestamptz
     '2000-01-03'))
   FROM tbl_tfloat;
 * @endcode
 *
 * The function is one every temporal type answers, so one run reports every
 * type rather than the one a reader uncomments. To exercise a type-specific
 * function instead, apply it to the value the loop parses.
 *
 * The fixtures are written by `tools/gen_test_csv.py` from the archives the
 * PostgreSQL suite loads.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tbl_temporal tbl_temporal.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_cbuffer.h>
#include <meos_geo.h>
#include <meos_npoint.h>
#include <meos_pose.h>
#include <meos_rgeo.h>
#include <meos_catalog.h>
#include <meos_internal.h>

/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LEN_HEADER 1024
/* Maximum length in characters of a temporal value in the input data. The
 * program reads every temporal table the archives carry, so the bound is the
 * longest value across all of them rather than one table's:
 * SELECT MAX(length(temp::text)) FROM tbl_tposechain;
 * -- 20899
 */
#define MAX_LEN_TEMP 32768
/* The scanf width, one below the buffer so a value reaching it is a value the
 * buffer truncated rather than one that happened to fit exactly */
#define SCAN_LEN_TEMP 32766

/**
 * @brief Return the temporal type whose name a fixture file carries
 * @details A fixture is named after the table it exports, `tbl_<type>` for a
 * temporal type, and the spatial tables that separate the dimensions add a
 * `2d` or `3d` suffix to it. The name is looked up in the catalog, so what a
 * file may be read as is what the library registers, and a file naming no
 * type is refused rather than read as the wrong one.
 */
static MeosType
fixture_temptype(const char *path)
{
  const char *base = strrchr(path, '/');
  base = base ? base + 1 : path;
  if (strncmp(base, "tbl_", 4) != 0)
    return T_UNKNOWN;
  base += 4;
  size_t len = strlen(base);
  if (len > 4 && strcmp(base + len - 4, ".csv") == 0)
    len -= 4;
  if (len > 2 && (strncmp(base + len - 2, "2d", 2) == 0 ||
      strncmp(base + len - 2, "3d", 2) == 0))
    len -= 2;
  for (MeosType temptype = 0; temptype < NUM_MEOS_TYPES; temptype++)
  {
    if (! temporal_type(temptype))
      continue;
    const char *name = meostype_name(temptype);
    if (strlen(name) == len && strncmp(name, base, len) == 0)
      return temptype;
  }
  return T_UNKNOWN;
}

/* Main program */
int
main(int argc, char **argv)
{
  if (argc != 2)
  {
    printf("Usage: %s <csv-file>\n", argv[0]);
    return 1;
  }
  const char *path = argv[1];

  /* Initialize MEOS. The handler is installed after meos_initialize, which
   * installs the exiting default itself, so the order is what makes it take
   * effect: a value the library declines then ends its own row rather than
   * the run, and the loop reads the rest of the table. */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  MeosType temptype = fixture_temptype(path);
  if (temptype == T_UNKNOWN)
  {
    /* The caller hands over every fixture the archives carry, so a table of
     * some other type is an answer rather than a failure: say which file names
     * no temporal type, and leave the exit status to the files that do. */
    printf("%s names no temporal type\n", path);
    meos_finalize();
    return 0;
  }

  FILE *file = fopen(path, "r");
  if (! file)
  {
    printf("Error opening input file %s\n", path);
    meos_finalize();
    return 1;
  }

  /* The function applied, which every temporal type answers because it moves
   * the time axis and leaves the values alone */
  Interval *interv = interval_in("5 minutes", -1);

  char header_buffer[MAX_LEN_HEADER];
  char temporal_buffer[MAX_LEN_TEMP];

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", header_buffer);

  /* Continue reading the file */
  int nrows = 0;
  do
  {
    int k;
    int read = fscanf(file, "%d,%32766[^\n]\n", &k, temporal_buffer);

    /* A value as long as the width is one the read cut short, and a cut value
     * parses as garbage or not at all. Say so rather than answer for it. */
    if (read == 2 && strlen(temporal_buffer) >= SCAN_LEN_TEMP)
    {
      printf("Value of k %d in %s exceeds %d characters\n", k, path,
        SCAN_LEN_TEMP);
      fclose(file);
      free(interv);
      meos_finalize();
      return 1;
    }

    if (ferror(file))
    {
      printf("Error reading input file %s\n", path);
      fclose(file);
      free(interv);
      meos_finalize();
      return 1;
    }
    /* Ignore records with NULL values and continue reading */
    if (read != 2)
      continue;

    /* Transform the string read into a temporal value of the fixture type */
    Temporal *temp = temporal_in(temporal_buffer, temptype);
    if (! temp)
      continue;

    Temporal *rest = temporal_shift_time(temp, interv);
    if (rest)
    {
      /* Get the number of instants of the result */
      int count = temporal_num_instants(rest);
      printf("k: %d, Number of instants: %d\n", k, count);
      free(rest);
      nrows++;
    }
    free(temp);
  } while (! feof(file));

  printf("%s (%s): number of non-empty answers: %d\n", path,
    meostype_name(temptype), nrows);

  free(interv);
  fclose(file);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
