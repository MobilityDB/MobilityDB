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
 * @brief A simple program that reads from a CSV file obtained from 
 * 1:10M populated places from Natural Earth.
 * https://www.naturalearthdata.com/downloads/10m-cultural-vectors/10m-populated-places/
 * https://www.naturalearthdata.com/
 * and applies the PostGIS function `ST_ClusterKMeans` to the entire file.
 *
 * This file simply reproduces in MEOS Paul Ramsey's blog
 * https://www.crunchydata.com/blog/postgis-clustering-with-k-means
 * Therefore, the program corresponds to the following SQL query
 * @code
 * SELECT name, pop_max, geom,
 *   ST_ClusterKMeans(geom, 10) OVER () AS cluster
 * FROM popplaces;
 * @endcode
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o popplaces_kmeans popplaces_kmeans.c -L/usr/local/lib -lmeos
 * @endcode
 *
 * @code
   CREATE TABLE popplaces_geographic_meos (
     name text,
    pop_max int,
    geom geometry(Point, 4326),
    cluster integer);
   COPY popplaces_geographic_meos
     FROM '/home/esteban/src/MobilityDB/meos/examples/data/popplaces_new.csv'
     DELIMITER ',' CSV HEADER;
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <meos.h>
#include <meos_geo.h>

/* Maximum number of input rows */
#define MAX_ROWS 50000
/* Number of instants in a batch for printing a marker */
#define NO_INSTS_BATCH 1000
/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LEN_LINE 1024

/* Main program */
int main(void)
{
  /* Get start time */
  clock_t t;
  t = clock();

  /* Initialize MEOS */
  meos_initialize();

  /* Allocate space to save the records with the geometries */
  char *name[MAX_ROWS] = {0};
  int pop_max[MAX_ROWS] = {0};
  GSERIALIZED *geom[MAX_ROWS] = {0};
  /* Iterator variable */
  int i;
  /* Exit value */
  int exit_value = 0;

  /* Open the input file */
  FILE *input_file = fopen("data/popplaces.csv", "r");
  if (! input_file)
  {
    printf("Error opening input file\n");
    meos_finalize();
    return EXIT_FAILURE;
  }

  int no_records = 0;
  int no_nulls = 0;
  int count = 0;
  char line_buffer[MAX_LEN_LINE];

  /* Read the first line of the file with the headers */
  fscanf(input_file, "%1023s\n", line_buffer);
  printf("Reading the instants (one '*' marker every %d instants)\n",
    NO_INSTS_BATCH);

  /* Continue reading the file */
  do
  {
    fscanf(input_file, "%1023[^\n]\n", line_buffer);
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
      /* Column 1 */
      if (column == 0)
        name[count] = strdup(value);
      /* Column 2 */
      else if (column == 1)
        /* Transform the string representing pop_max into an integer */
        pop_max[count] = atoi(value);
      /* Column 3 */
      else if (column == 2)
        geom[count] = geom_in(value, -1);
      value = strtok(NULL, ",");
      column++;
    }

    if (column != 3 && ! feof(input_file))
    {
      printf("Record with missing values ignored\n");
      no_nulls++;
    }

    if (column == 3)
    {
      no_records++;
      if (no_records % NO_INSTS_BATCH == 0)
      {
        printf("*");
        fflush(stdout);
      }
    }

    /* Advance count of records read */
    count++;
  } while (!feof(input_file));

  /* Close the input file */
  fclose(input_file);

  /* Compute the clusters */
  int *cluster = geo_cluster_kmeans((const GSERIALIZED **) geom, count, 10);

  /* Open the output file */
  FILE *output_file = fopen("data/popplaces_new.csv", "w+");
  if (! output_file)
  {
    printf("Error opening output file\n");
    meos_finalize();
    return EXIT_FAILURE;
  }

  /* Write the header line in the output file */
  printf("Clustered geometries written to file 'popplaces_new.csv'\n");
  fprintf(output_file,"name,pop_max,geom,cluster\n");

  for (i = 0; i < count; i++)
  {
    /* Write record in output file */
    char *point_str = geo_out(geom[i]);
    fprintf(output_file,"%s,%d,%s,%d\n", name[i], pop_max[i],
      point_str, cluster[i]);
    free(point_str);
  }

  /* Close the output files */
  fclose(output_file);

  printf("\n%d records read from file 'popplaces.csv'.\n"
    "%d incomplete records ignored.\n", no_records, no_nulls);
  printf("%d points read.\n", count);

  /* Calculate the elapsed time */
  t = clock() - t;
  double time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);

/* Clean up */
cleanup:

 /* Free memory */
  for (i = 0; i < count; i++)
  {
    free(geom[i]);
    free(name[i]);
  }
  free(cluster);
  
  /* Finalize MEOS */
  meos_finalize();

  return exit_value;
}
