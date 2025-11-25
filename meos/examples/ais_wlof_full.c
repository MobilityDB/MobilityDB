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
 * @brief A simple program that reads AIS data from a CSV file containing one
 * full day of observations provided by the Danish Maritime Authority in
 * https://web.ais.dk/aisdata/ and computes the weighted local outlier factor 
 * (WLOF) for a given ship identified by an MMSI.
 *
 * Please notice that the `data` directory DOES NOT contain the input CSV file,
 * you must download it from the website above.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o ais_wlof_full ais_wlof_full.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* For memset */
#include <time.h>
#include <meos.h>
#include <meos_geo.h>
/* The expandable functions are in the internal MEOS API */
#include <meos_internal.h>
#include <meos_internal_geo.h>

/*
 * IMPORTANT !!!
 * You need to fix the values MAX_NO_RECORDS and MAX_INSTANTS according to the
 * available memory in your computer, and select a given MMSI in the input file
 * and a value of K for defining the neighborhoud 
 */
/* MMSI in the CSV file for which the WLOF will be computed */
#define MMSI_VALUE 210195000 
/* Number of neighbors to compute the neighborhoud */
#define K 10 
/* Distance for clustering the input geometries */
#define DISTANCE 0.0
/* Maximum number of records read in the CSV file */
#define MAX_NO_RECORDS 20000000
/* Maximum number of trips */
#define MAX_INSTANTS 32000
/* Number of instants in a batch for printing a marker */
#define NO_RECORDS_BATCH 100000
/* Maximum length in characters of a record in the input CSV file */
#define MAX_LENGTH_LINE 1024
/* Maximum length in characters of a point in the input data */
#define MAX_LENGTH_POINT 64
/* Maximum length in characters of a timestamp in the input data */
#define MAX_LENGTH_TIMESTAMP 32
/* Maximum length in characters of all other strings in the input data */
#define MAX_LENGTH_STRING 64

typedef struct
{
  Timestamp T;
  long int MMSI;
  double Latitude;
  double Longitude;
} AIS_record;

/* Main program */
int main(void)
{
  /* Input buffer to read the CSV file */
  char line_buffer[MAX_LENGTH_LINE];
  /* Allocate space to build the trips */
  GSERIALIZED *geoms[MAX_INSTANTS] = {0};
  /* Record storing one line read from of the CSV file*/
  AIS_record rec;
  /* Number of records read */
  int no_records = 0;
  /* Number of instants found for the given MMSI */
  int no_instants = 0;
  /* Iterator variables */
  int i;
  /* Exit value initialized to 1 (i.e., error) to quickly exit upon error */
  int exit_value = 1;

  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Get start time */
  clock_t t = clock();

  /*
   * Open the input file
   * Substitute the full file path in the first argument of fopen
   */
  FILE *input_file = fopen("data/aisdk-2024-03-01.csv", "r");
  if (! input_file)
  {
    printf("Error opening input file\n");
    goto cleanup;
  }

  /* Read the first line of the input file with the headers */
  fscanf(input_file, "%1023[^\n]\n", line_buffer);
  printf("Processing records\n");
  printf("  one '*' marker every %d records\n", NO_RECORDS_BATCH);
  /* Uncomment the next lines to display a marker each time and incomplete
   * record or an erroneous field has been read */
  // printf("  one '-' marker every incomplete or erroneous records\n");
  // printf("  one 'T' marker every record with duplicate timestamp and position\n");
  // printf("  one 'S' marker every record with duplicate timestamp and SGO\n");

  /* Continue reading the input file */
  do
  {
    fscanf(input_file, "%1023[^\n]\n", line_buffer);
    if (ferror(input_file))
    {
      printf("\nError reading input file\n");
      fclose(input_file);
      goto cleanup;
    }

    no_records++;
    /* Print a marker every X records read */
    if (no_records % NO_RECORDS_BATCH == 0)
    {
      printf("*");
      fflush(stdout);
    }
    /* Break if maximum number of records read */
    if (no_records == MAX_NO_RECORDS)
      break;

    /* Initialize record to 0 */
    memset(&rec, 0, sizeof(rec));
    int field = 0;
    char *token = strtok(line_buffer, ",");
    bool has_t = false, has_mmsi = false, has_lat = false, has_long = false;
    while (token)
    {
      if (strlen(token) != 0 && strcmp(token, "Unknown") != 0)
      {
        switch (field)
        {
          case 0:
            rec.T = timestamp_in(token, -1);
            if (rec.T != 0)
              has_t = true;
            break;
          case 2:
            rec.MMSI = strtoll(token, NULL, 0);
            /* Filter the records to the given MMSI */
            if (rec.MMSI == MMSI_VALUE)
              has_mmsi = true;
            break;
          case 3:
            rec.Latitude = strtold(token, NULL);
            /* The next line ensures the validity of the latitude value */
            // if (rec.Latitude >= -90.0 && rec.Latitude <= 90.0)
            /* The next line ensures the validity of the latitude value given
             * that the input file reports observations around Denmark */
            if (rec.Latitude >= 40.18 && rec.Latitude <= 84.17)
              has_lat = true;
            break;
          case 4:
            rec.Longitude = strtold(token, NULL);
            /* The next line ensures the validity of the longitude value */
            // if (rec.Longitude >= -180.0 && rec.Longitude <= 180.0)
            /* The next line ensures the validity of the longitude value given
             * that the input file reports observations around Denmark */
            if (rec.Longitude >= -16.1 && rec.Longitude <= 32.88)
              has_long = true;
            break;
          default:
            break;
        }
      }
      token = strtok(NULL, ",");
      field++;
      if (field > 4)
        break;
    }

    if (! has_t || ! has_mmsi || ! ( has_lat && has_long ))
    {
      /* Uncomment the next lines to display a marker each time
       * an incomplete record or an erroneous field has been read */
      // printf("-");
      // fflush(stdout);
      continue;
    }

    /* Store the new instant */
    if (has_lat && has_long)
    {
      GSERIALIZED *gs = geogpoint_make2d(4326, rec.Longitude, rec.Latitude);
      geoms[no_instants++] = geo_transform(gs, 25832);
      free(gs);
    }
  } while (! feof(input_file));

  /* Close the file */
  fclose(input_file);
  printf("\n%d records read.\n%d records for MMSI %d.\n",
    no_records, no_instants, MMSI_VALUE);
 
 /* Calculate the elapsed time for reading the input file */
  t = clock() - t;
  double time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to read the input file \n", time_taken);

  /* Compute the WLOF */
  uint32_t count = 0;
  GSERIALIZED **clusters;
  double *scores = geo_wlof((const GSERIALIZED **) geoms, no_instants, K,
    DISTANCE, &count, &clusters);
      
  /* Calculate the elapsed time */
  t = clock() - t;
  time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to compute the scores\n", time_taken);

  /* Open/create the output file */
  /* You may substitute the full file path in the first argument of fopen */
  char *output_file_str = malloc(64);
  sprintf(output_file_str, "data/ais_wlof_%ld_%d.csv", (int64) MMSI_VALUE, K);
  FILE *output_file = fopen(output_file_str, "w+");
  free(output_file_str);
  if (! output_file)
  {
    printf("Error creating/opening the output file\n");
    goto cleanup;
  }

  /* Write the header line in the output file */
  fprintf(output_file,"MMSI,geom,score\n");
  for (i = 0; i < count; i++)
  {
    /* Write record in output file */
    char *point_str = geo_out(geoms[i]);
    fprintf(output_file,"%ld,%s,%lf\n", (int64) MMSI_VALUE, point_str,
      scores[i]);
    free(point_str);
  }

  /* Close the output files */
  fclose(output_file);

  /* Calculate the elapsed time */
  t = clock() - t;
  time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to write the scores in the output file\n", time_taken);

  /* State that the program executed successfully */
  exit_value = 0;

/* Clean up */
cleanup:

 /* Free memory */
  for (i = 0; i < no_instants; i++)
    free(geoms[i]);
  free(clusters); free(scores);

  /* Finalize MEOS */
  meos_finalize();

  return exit_value;
}
