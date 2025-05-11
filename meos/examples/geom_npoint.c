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
 * gcc -Wall -g -I/usr/local/include -o geom_npoint geom_npoint.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_npoint.h>

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
  double length;
} ways_record;

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();

  /* Get start time */
  clock_t t;
  t = clock();

  /* Distances */
  double dist, min_dist = DBL_MAX;
  /* Position in the geometry with the shortest distance */
  double pos;
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
  int no_records = 0;
  int no_nulls = 0;
  char header_buffer[MAX_LENGTH_HEADER];
  char geo_buffer[MAX_LENGTH_GEOM];
  const char *geo_str = "SRID=5676;POINT(72.94967061684646 25.156720715884354)";
  GSERIALIZED *point = geom_in(geo_str, -1);
  
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

    if (read != 3 && ! feof(file))
    {
      printf("Record with missing values ignored\n");
      no_nulls++;
      continue;
    }

    no_records++;
    if (no_records % NO_WAYS_BATCH == 0)
    {
      printf("*");
      fflush(stdout);
    }

    /* Transform the string representing the geometry into a geometry value */
    rec.the_geom = geom_in(geo_buffer, -1);
    if (geo_is_empty(rec.the_geom))
    {
      printf("The geometry is empty");
      return_value = 1;
      goto cleanup;
    }

    /* We need to implement the following SQL query for a given geo
     *   SELECT npoint(gid, ST_LineLocatePoint(the_geom, geo))
     *   FROM public.ways WHERE ST_DWithin(the_geom, geo, DIST_EPSILON)
     *   ORDER BY ST_Distance(the_geom, geo) LIMIT 1;
     */
    
    pos = line_locate_point(rec.the_geom, point);
    if (pos < 0)
    {
      free(rec.the_geom);
      continue;
    }

    dist = geom_distance2d(rec.the_geom, point);
    if (dist < min_dist)
      min_dist = dist;
  } while (! feof(file));

  /* Close the input file */
  fclose(file);

  printf("\n%d records read.\n%d incomplete records ignored.\n",
    no_records, no_nulls);

  /* If the point was not found */
  if (! point)
  {
    printf("The geometry point cannot be transformed into a network point");
    return_value = 1;
    goto cleanup;
  }
  
  Npoint *np = npoint_make(rec.gid, pos);
  char *np_str = npoint_out(np, 3);
  printf("Network point: %s\n", np_str);
  free(np); free(np_str);
  
  /* Calculate the elapsed time */
  t = clock() - t;
  double time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);

/* Clean up */
cleanup:

  /* Free memory */
  free(point);

  /* Finalize MEOS */
  meos_finalize();

  return return_value;
}
