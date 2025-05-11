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
 * @brief A simple program that get the SRID of the geometries stored in the
 * file `ways.csv` that has the content of the PostgreSQL table ways defined
 * as follows
 * @code
 * CREATE TABLE public.ways (
 *   gid bigint NOT NULL,
 *   the_geom public.geometry NOT NULL,
 *   length double precision NOT NULL
 * );
 * @endcode
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o get_srid_ways get_srid_ways.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <meos.h>
#include <meos_geo.h>

/* Number of ways in a batch for printing a marker */
#define NO_WAYS_BATCH 10
/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LENGTH_HEADER 1024
/* Maximum length in characters of a geometry in the input data */
#define MAX_LENGTH_GEOM 100001

typedef struct
{
  long int gid;
  GSERIALIZED *the_geom;
} ways_record;

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();

  /* Get start time */
  clock_t t;
  t = clock();

  /* SRID */
  int32_t srid = 0;
  /* Return value */
  int return_value = 0;

  /* Substitute the full file path in the first argument of fopen */
  FILE *file = fopen("data/ways.csv", "r");

  if (! file)
  {
    printf("Error opening input file\n");
    meos_finalize();
    return 1;
  }

  ways_record rec;
  char header_buffer[MAX_LENGTH_HEADER];
  char geo_buffer[MAX_LENGTH_GEOM];

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", header_buffer);

  printf("Reading the instants (one '*' marker every %d instants)\n",
    NO_WAYS_BATCH);

  /* Continue reading the file */
  do
  {
    int read = fscanf(file, "%ld,%100000[^\n]\n",
      &rec.gid, geo_buffer);

    if (ferror(file))
    {
      printf("Error reading input file");
      return_value = 1;
      goto cleanup;
    }

    if (read == 2)
    {
      /* Transform the string representing the geometry into a geometry value */
      rec.the_geom = geom_in(geo_buffer, -1);

      /* Find the SRID */
      srid = geo_srid(rec.the_geom);
      break;
    }

    if (read != 2 && ! feof(file))
    {
      printf("Record with missing values ignored\n");
    }
  } while (!feof(file));

  /* Close the input file */
  fclose(file);

  /* Construct the trips */
  printf("SRID: %d\n", srid);

  /* Calculate the elapsed time */
  t = clock() - t;
  double time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);

/* Clean up */
cleanup:

 /* Free memory */
 free(rec.the_geom);

  /* Finalize MEOS */
  meos_finalize();

  return return_value;
}
