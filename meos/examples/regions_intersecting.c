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
 * @brief A simple program that reads from the regions.csv file obtained from
 * the BerlinMOD benchmark
 * https://github.com/MobilityDB/MobilityDB-BerlinMOD
 * and applies one of the PostGIS functions `ST_ClusterIntersecting`
 * or `ST_ClusterWithin` to the entire file.
 *
 * The program corresponds to one of the following SQL queries
 * @code
   DROP TABLE IF EXISTS RegionsIntersecting;
   CREATE TABLE RegionsIntersecting AS
   WITH Temp1 AS (
     SELECT unnest(ST_ClusterIntersecting(geom)) geom FROM Regions ),
   Temp2 AS (
     SELECT ROW_NUMBER() OVER () AS cluster, ST_Dump(geom) AS rec FROM Temp1 )
   SELECT cluster, (rec).geom FROM Temp2;
 * @endcode
 * or
 * @code
   DROP TABLE IF EXISTS RegionsWithin;
   CREATE TABLE RegionsWithin AS
   WITH Temp1 AS (
     SELECT unnest(ST_ClusterWithin(geom, 1000)) geom FROM Regions ),
   Temp2 AS (
     SELECT ROW_NUMBER() OVER () AS cluster, ST_Dump(geom) AS rec FROM Temp1 )
   SELECT cluster, (rec).geom FROM Temp2;
 * @endcode
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o regions_intersecting regions_intersecting.c -L/usr/local/lib -lmeos
 * @endcode
 *
 * The program expects the file `data/regions.csv` and produces a new file
 * `data/regions_new.csv` with a new column cluster added to each geometry.
 * This file can be input in PostgreSQL using the following SQL command, after
 * which it can be visualized with QGIS.
 * @code
   DROP TABLE IF EXISTS regions_meos;
   CREATE TABLE regions_meos (
     regionid int,
     geom geometry,
     cluster integer);
   COPY regions_meos
     FROM '/home/esteban/src/MobilityDB/meos/examples/data/regions_new.csv'
     DELIMITER ',' CSV HEADER;
 * @endcode
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <meos.h>
#include <meos_geo.h>

/* Maximum number of input rows */
#define MAX_ROWS 100
/* Maximum length in characters of a record in the input CSV file */
#define MAX_LINE_LENGTH 4096
/* Define whether `ST_ClusterIntersecting` or `ST_ClusterWithin` is applied */
#define CLUSTER_INTERSECTING false

/* Main program */
int main(void)
{
  /* Get start time */
  clock_t t;
  t = clock();

  /* Initialize MEOS */
  meos_initialize();

  /* Allocate space to save the id, the name, and the geometries */
  int ids[MAX_ROWS] = {0};
  GSERIALIZED *geoms[MAX_ROWS] = {0};
  int clusterNos[MAX_ROWS] = {0};
  /* Iterator variables */
  int i, j, k;
  /* Exit value */
  int exit_value = 0;

  /* Open the input file */
  FILE *input_file = fopen("data/regions.csv", "r");
  if (! input_file)
  {
    printf("Error opening input file\n");
    meos_finalize();
    return EXIT_FAILURE;
  }

  int no_records = 0;
  int no_nulls = 0;
  char line_buffer[MAX_LINE_LENGTH];

  /* Read the first line of the file with the headers */
  fscanf(input_file, "%4095[^\n]\n", line_buffer);

  /* Continue reading the file */
  do
  {
    fscanf(input_file, "%4095[^\n]\n", line_buffer);
    if (ferror(input_file))
    {
      printf("Error reading input file");
      exit_value = EXIT_FAILURE;
      goto cleanup;
    }

    /* Splitting the data */
    int column = 0;
    char *value = strtok(line_buffer, ",");
    while (value)
    {
      if (column == 0)
        ids[no_records] = atoi(value);
      else if (column == 1)
        geoms[no_records] = geom_in(value, -1);
      value = strtok(NULL, ",");
      column++;
    }
    /* Advance number of records to be clustered */
    no_records++;
  } while (!feof(input_file));
  printf("\n\nClustering geometries\n");
  printf("---------------------\n");

  /* Close the input file */
  fclose(input_file);

  /* Perform the clustering */
  int no_clusters;
  GSERIALIZED **clusters = CLUSTER_INTERSECTING ?
    geo_cluster_intersecting((const GSERIALIZED **) geoms, no_records,
      &no_clusters) :
    geo_cluster_within((const GSERIALIZED **) geoms, no_records, 1000.0,
      &no_clusters);

  /* Fill the array of cluster numbers */
  for (i = 0; i < no_clusters; i++)
  {
    for (j = 0; j < geo_ngeos(clusters[i]); j++)
    {
      GSERIALIZED *subgeo = geo_geoN(clusters[i], j + 1);
      for (k = 0; k < no_records; k++)
      {
        if (geo_equals(subgeo, geoms[k]))
          clusterNos[k] = i;
      }
      free(subgeo);
    }
  }

  /* Open the output file */
  FILE *output_file = fopen("data/regions_new.csv", "w+");
  if (! output_file)
  {
    printf("Error opening output file\n");
    meos_finalize();
    return EXIT_FAILURE;
  }

  /* Write the header line in the output file */
  printf("\nClustered geometries written to file 'regions_new.csv'\n");
  fprintf(output_file,"id,geom,cluster\n");
  /* Write the records in the output file */
  for (i = 0; i < no_records; i++)
  {
    char *point_str = geo_out(geoms[i]);
    fprintf(output_file,"%d,%s,%d\n", ids[i], point_str, clusterNos[i]);
    free(point_str);
  }

  /* Close the output files */
  fclose(output_file);

  printf(
    "\n%d records read from file 'regions.csv'.\n"
    "%d incomplete records ignored.\n"
    "%d clusters obtained.\n", 
    no_records, no_nulls, no_clusters);

  /* Calculate the elapsed time */
  t = clock() - t;
  double time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);

/* Clean up */
cleanup:

 /* Free memory */
  for (i = 0; i < no_records; i++)
    free(geoms[i]);
  for (i = 0; i < no_clusters; i++)
    free(clusters[i]);
  free(clusters);

  /* Finalize MEOS */
  meos_finalize();

  return exit_value;
}
