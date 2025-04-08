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
 * https://web.ais.dk/aisdata/, and transform it from SRID 4326 to SRID 25832
 * writing the result in an output CSV file.
 *
 * Please notice that the `data` directory DOES NOT contain the input CSV file,
 * you must download it from the website above.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o ais_transform_full ais_transform_full.c -L/usr/local/lib -lproj -lmeos
 * @endcode
 */

/*****************************************************************************/

/* C */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>   /* For clock */
#include <string.h> /* For strlen */
/* PROJ */
#include <proj.h>
/* C */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>

/* Maximum length in characters of a record in the input CSV file */
#define MAX_LENGTH_LINE 1024
/* Maximum length in characters of a point in the input data */
#define MAX_LENGTH_POINT 64
/* Maximum number of records read in the CSV file */
#define MAX_NO_RECORDS 20000000
/* Number of instants in a batch for printing a marker */
#define NO_RECORDS_BATCH 100000

typedef struct
{
  Timestamp T;
  long int MMSI;
  double Latitude;
  double Longitude;
  double SOG;
} AIS_record;

int main(void)
{
  /* Input buffers to read the CSV file */
  char line_buffer[MAX_LENGTH_LINE];
  /* Record storing one line read from of the CSV file*/
  AIS_record rec;
  /* Number of records read */
  int no_records = 0;
  /* Number of erroneous records read */
  int no_err_records = 0;
  /* Number of records written */
  int no_writes = 0;

  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Get start time */
  clock_t t = clock();

  /*
   * Open the input file
   * Substitute the full file path in the first argument of fopen
   */
  FILE *file_in = fopen("data/aisdk-2025-03-01.csv", "r");
  if (! file_in)
  {
    printf("Error opening input file\n");
    goto cleanup;
  }

  /* 
   * Open/create the output file 
   * Substitute the full file path in the first argument of fopen
   */
  FILE *file_out = fopen("data/aisdk-2025-03-01_25832.csv", "w+");

  if (! file_out)
  {
    printf("Error creating/opening the output file\n");
    goto cleanup;
  }

  /* Read the first line of the input file with the headers */
  fscanf(file_in, "%1023[^\n]\n", line_buffer);
  printf("Processing records\n");
  printf("  one '*' marker every %d records\n", NO_RECORDS_BATCH);

  /* Write the first line of the output file with the headers */
  fprintf(file_out, "# Timestamp,MMSI,Latitude,Longitude,SOG\n");

  /* Continue reading the input file */
  do
  {
    fscanf(file_in, "%1023[^\n]\n", line_buffer);
    if (ferror(file_in))
    {
      printf("\nError reading input file\n");
      fclose(file_in);
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
    bool has_t = false, has_mmsi = false, has_lat = false,
      has_long = false, has_sog = false;
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
            /* The next line can be used to filter the records to a given MMSI */
            // if (rec.MMSI == -1961112608 )
            if (rec.MMSI != 0)
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
          case 7:
            rec.SOG = strtold(token, NULL);
            /* Speed over ground in 1/10 knot steps (0-102.2 knots)
             * See https://www.navcen.uscg.gov/ais-class-a-reports */
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

    if (! has_t || ! has_mmsi || ! has_lat || ! has_long || ! has_sog)
    {
      /* Uncomment the next lines to display a marker each time
       * an incomplete record or an erroneous field has been read */
      // printf("-");
      // fflush(stdout);
      no_err_records++;
      continue;
    }
    else
    {
      /* Create the point and transform it */
      GSERIALIZED *gs = geogpoint_make2d(4326, rec.Longitude, rec.Latitude);
      GSERIALIZED *transf = geo_transform(gs, 25832);
      const POINT2D *pt = GSERIALIZED_POINT2D_P(transf);
      /* Write the record in the output file */
      fprintf(file_out, "%ld,%lf,%lf,%lf\n", rec.MMSI, pt->x, pt->y, rec.SOG);
      free(gs); 
      free(transf);
      if (ferror(file_out))
      {
        printf("\nError writing to output file\n");
        fclose(file_in);
        goto cleanup;
      }
      no_writes++;
    }
  } while (! feof(file_in));

  printf("\n%d records read\n%d incomplete records ignored\n"
    "%d writes to the output file\n", no_records, no_err_records, no_writes);

  /* Calculate the elapsed time */
  t = clock() - t;
  double time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);

/* Clean up */
cleanup:

  /* Close the input file */
  fclose(file_in);

  /* Close the output file */
  fclose(file_out);

  // free(str_out); free(trip); free(trip_out);

  /* Finalize MEOS */
  meos_finalize();
  return 0;
}

/*****************************************************************************/
