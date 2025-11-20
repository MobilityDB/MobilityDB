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
 * @brief A simple program that reads from a CSV file obtained from geonames
 * https://download.geonames.org/export/dump/US.zip
 * and applies the PostGIS function `ST_DBSCAN` to the entire file.
 *
 * This file simply reproduces in MEOS Paul Ramsey's blog
 * https://www.crunchydata.com/blog/postgis-clustering-with-k-means
 * Therefore, the program corresponds to the following SQL query
 * @code
   DROP TABLE IF EXISTS geonames_sch;
   CREATE TABLE geonames_sch AS
   SELECT *, ST_ClusterDBScan(geom, 2000, 5)
     OVER (PARTITION BY admin1) AS cluster
   FROM geonames
   WHERE fcode = 'SCH';
 * @endcode
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o geonames_dbscan geonames_dbscan.c -L/usr/local/lib -lmeos
 * @endcode
 * The program expects that the file `data/US.txt` extracted from the ZIP file
 * above is located in the subdirecttory `data\'.
 *
 * @code
   DROP TABLE IF EXISTS geonames_meos;
   CREATE TABLE geonames_meos (
     geonameid int,
     name text,
     admin1 text,
     geom geometry(Point, 5070),
     cluster integer);
   COPY geonames_meos
     FROM '/home/esteban/src/MobilityDB/meos/examples/data/geonames_new.csv'
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
#define MAX_ROWS 250000
/* Maximum number of rows per windinw on which the clustering is performed */
#define MAX_ROWS_WIN 15000
/* Number of instants in a batch for printing a marker */
#define NO_INSTS_BATCH 1000
/* Maximum length in characters of a record in the input CSV file */
#define MAX_LINE_LENGTH 2048
/* Maximum number of fields in the input CSV file */
#define MAX_NO_FIELDS 24
/* Maximum number of admin1 distinct values defining the windows */
#define MAX_NO_ADMIN1 64

/* Function to trim leading and trailing whitespace */
char *trim(char *str)
{
  if (str == NULL)
    return str;
  while (isspace((unsigned char) *str))
    str++;
  if (*str == 0)  /* All spaces?  */
    return str;

  char *end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end))
    end--;

  *(end + 1) = 0;
  return str;
}

/* Function to parse a CSV line considering quotes and escaped quotes */
int parse_csv_line(char *line, char **fields)
{
  size_t len = strlen(line);
  size_t nofields = 0;
  char *field_start = &line[0];
  for (size_t i = 0; i < len; i++)
  {
    char c = line[i];
    if (c == '\t')
    {
      /* End of field */
      line[i] = '\0'; /* Temporarily terminate the field */
      fields[nofields++] = trim(field_start);
      field_start = &line[i + 1];
    }
  }
  return nofields;
}

/* Function to keep track of the distinct admin1 values */
int
get_admin1_no(char *admin1, char *admin1_list[])
{
  int i;
  for (i = 0; i < MAX_NO_ADMIN1; i++)
  {
    if (admin1_list[i] == NULL || strcmp(admin1_list[i], admin1) == 0)
      break;
  }
  /* The value was found */
  if (admin1_list[i])
    return i;
  /* The value is new and there is still space in the list */
  if (i < MAX_NO_ADMIN1)
  {
    admin1_list[i] = strdup(admin1);
    return i;
  }
  /* We only arrive here on error */
  printf("There is no more space to store the list of distinct admin1 values");
  return -1;
}

/* Main program */
int main(void)
{
  /* Get start time */
  clock_t t;
  t = clock();

  /* Initialize MEOS */
  meos_initialize();

  /* Allocate space to save the id, the name, and the geometries */
  int geonameid[MAX_ROWS] = {0};
  char *name[MAX_ROWS] = {0};
  char *admin1[MAX_ROWS] = {0};
  GSERIALIZED *geom[MAX_ROWS] = {0};
  int cluster[MAX_ROWS] = {0};
  char *admin1_list[MAX_NO_ADMIN1] = {0};
  int admin1_count[MAX_NO_ADMIN1] = {0};
  int noadmin1 = 0;
  /* Iterator variables */
  int i, j;
  /* Exit value */
  int exit_value = 0;

  /* Open the input file */
  FILE *input_file = fopen("data/US.txt", "r");
  if (! input_file)
  {
    printf("Error opening input file\n");
    meos_finalize();
    return EXIT_FAILURE;
  }

  int no_records = 0;
  int no_nulls = 0;
  int count = 0;
  char line_buffer[MAX_LINE_LENGTH];
  char *fields[MAX_NO_FIELDS];

  /* Read the file */
  printf("Reading the instants (one '*' marker every %d instants)\n",
    NO_INSTS_BATCH);

  do
  {
    fscanf(input_file, "%2047[^\n]\n", line_buffer);
    if (ferror(input_file))
    {
      printf("Error reading input file");
      exit_value = EXIT_FAILURE;
      goto cleanup;
    }

    /* Remove newline characters */
    line_buffer[strcspn(line_buffer, "\r\n")] = 0;
    /* Split the line ignoring the result of the function below */
    parse_csv_line(line_buffer, fields);

    no_records++;
    if (no_records % NO_INSTS_BATCH == 0)
    {
      printf("*");
      fflush(stdout);
    }
    if (fields[7] && strcmp(fields[7], "SCH") == 0)
    {
      /* Save the name and the geom values read */
      geonameid[count] = atoi(fields[0]);
      name[count] = strdup(fields[1]);
      admin1[count] = strdup(fields[10]);
      double lon = atof(fields[5]);
      double lat = atof(fields[4]);
      GSERIALIZED *gs = geompoint_make2d(4326, lon, lat);
      geom[count] = geo_transform(gs, 5070);
      free(gs);
      int no = get_admin1_no(admin1[count], admin1_list);
      if (no == noadmin1)
        /* Advance count of distinct admin1 values */
        noadmin1++;
      /* Advance count of records to be clustered */
      count++;
    }
  } while (!feof(input_file));
  printf("\n\nClustering geometries per state\n");
  printf("-------------------------------\n");

  /* Close the input file */
  fclose(input_file);

  /* Iterate for each admin1 value and perform the clustering */
  for (i = 0; i < noadmin1; i++)
  {
    int ids[MAX_ROWS_WIN] = {0};
    GSERIALIZED *geoms[MAX_ROWS_WIN] = {0};
    for (j = 0; j < count; j++)
    {
      if (strcmp(admin1_list[i], admin1[j]) == 0)
      {
        ids[admin1_count[i]] = j;
        geoms[admin1_count[i]++] = geom[j];
      }
    }
    printf("%d: %s, %d records\n", i, admin1_list[i], admin1_count[i]);
    /* Compute the clusters */
    uint32_t *clust = geo_cluster_dbscan((const GSERIALIZED **) geoms,
      admin1_count[i], 2000.0, 5, &count);
    for (j = 0; j < admin1_count[i]; j++)
      cluster[ids[j]] = clust[j];
    free(clust);
  }

  /* Open the output file */
  FILE *output_file = fopen("data/geonames_new.csv", "w+");
  if (! output_file)
  {
    printf("Error opening output file\n");
    meos_finalize();
    return EXIT_FAILURE;
  }

  /* Write the header line in the output file */
  printf("\nClustered geometries written to file 'geonames_new.csv'\n");
  fprintf(output_file,"geonameid,name,admin1,geom,cluster\n");
  /* Write the records in the output file */
  for (j = 0; j < count; j++)
  {
    /* Write record in output file */
    char *point_str = geo_out(geom[j]);
    fprintf(output_file,"%d,%s,%s,%s,%d\n",
      geonameid[j], name[j], admin1[j], point_str, cluster[j]);
    free(point_str);
  }

  /* Close the output files */
  fclose(output_file);

  printf(
    "\n%d records read from file 'popplaces.csv'.\n"
    "%d incomplete records ignored.\n"
    "%d records clustered.\n", 
    no_records, no_nulls, count);
  printf("%d distinct admin1 values read\n", noadmin1);

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

  /* Finalize MEOS */
  meos_finalize();

  return exit_value;
}
