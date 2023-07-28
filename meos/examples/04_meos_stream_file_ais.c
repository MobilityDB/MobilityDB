/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief A simple program that reads AIS data from a CSV file, accumulates the
 * observations in main memory and send the temporal values to a logfile
 * when they reach a given number of instants in order to free
 * the memory and ingest the newest observations.
 *
 * This program is similar to `04_meos_stream_ais` but illustrates the use of
 * a logfile. In this setting, the expandable data structures accumulate the
 * observations that have been received so far and appends to the file.
 * Depending on application requirements and the available memory, the
 * accumulated temporal values must be sent regularly to the file.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o 04_meos_stream_file_ais 04_meos_stream_file_ais.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_internal.h>

/* Number of instants to send in batch to the file  */
#define NO_INSTANTS_BATCH 500
/* Number of instants to keep when restarting a sequence */
#define NO_INSTANTS_KEEP 2
/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LENGTH_HEADER 1024
/* Maximum length in characters of a point in the input data */
#define MAX_LENGTH_POINT 64
/* Maximum length for a SQL output query */
#define MAX_LENGTH_SQL 16384
/* Number of inserts that are sent in bulk */
#define NO_BULK_INSERT 20
/* Maximum number of trips */
#define MAX_TRIPS 5

int count=0;

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
  long int MMSI;   /* Identifier of the trip */
  TSequence *trip; /* Array of Latest observations of the trip, by MMSI */
} trip_record;

//verifies if the size if greater than the max size, of so, it sends the data to the file
int windowManager(int size, trip_record *trips, int ship ,FILE *fileOut)
{
  if (trips[ship].trip && trips[ship].trip->count == NO_INSTANTS_BATCH)
  {

    char *temp_out = tsequence_out(trips[ship].trip, 15);
    fprintf(fileOut, "%ld, %s\n",trips[ship].MMSI, temp_out);
    /* Free memory */
    free(temp_out);
    count++;
    printf("*");
    fflush(stdout);
    /* Restart the sequence by only keeping the last instants */
    tsequence_restart(trips[ship].trip, NO_INSTANTS_KEEP);

  }
  return 1;
}



int
main(int argc, char **argv)
{
  AIS_record rec;
  int no_records = 0;
  int no_nulls = 0;
  char point_buffer[MAX_LENGTH_POINT];
  char text_buffer[MAX_LENGTH_HEADER];
  /* Allocate space to build the trips */
  trip_record trips[MAX_TRIPS] = {0};
  /* Number of ships */
  int numships = 0;
  /* Iterator variable */
  int i;
  /* Return value */
  int return_value = 0;

  /***************************************************************************
   * Section 1: Open the output file
   ***************************************************************************/

  /* Open/create the output file */
  /* You may substitute the full file path in the first argument of fopen */
  FILE *fileOut = fopen("aisoutput.csv", "w+");

  if (! fileOut)
  {
    printf("Error creating/opening logfile\n");
    return_value = 1;
    goto cleanup;
  }


  /***************************************************************************
   * Section 2: Initialize MEOS and open the input AIS file
   ***************************************************************************/

  /* Initialize MEOS */
  meos_initialize(NULL);

  /* You may substitute the full file path in the first argument of fopen */
  FILE *fileIn = fopen("aisinput.csv", "r");

  if (! fileIn)
  {
    printf("Error opening input file\n");
    return_value = 1;
    goto cleanup;
  }

  /***************************************************************************
   * Section 3: Read input file line by line and append each observation as a
   * temporal point in MEOS
   ***************************************************************************/

  printf("Accumulating %d instants before sending them to the logfile\n"
    "(one marker every logfile update)\n", NO_INSTANTS_BATCH);

  /* Read the first line of the file with the headers */
  fscanf(fileIn, "%1023s\n", text_buffer);

  /* Continue reading the file */
  do
  {
    int read = fscanf(fileIn, "%32[^,],%ld,%lf,%lf,%lf\n",
      text_buffer, &rec.MMSI, &rec.Latitude, &rec.Longitude, &rec.SOG);
    /* Transform the string representing the timestamp into a timestamp value */
    rec.T = pg_timestamp_in(text_buffer, -1);

    if (read == 5)
      no_records++;

    if (read != 5 && ! feof(fileIn))
    {
      printf("Record with missing values ignored\n");
      no_nulls++;
    }

    if (ferror(fileIn))
    {
      printf("Error reading file\n");
      fclose(fileIn);
      fclose(fileOut);
    }

    /* Find the place to store the new instant */
    int ship = -1;
    for (i = 0; i < MAX_TRIPS; i++)
    {
      if (trips[i].MMSI == rec.MMSI)
      {
        ship = i;
        break;
      }
    }
    if (ship < 0)
    {
      ship = numships++;
      if (ship == MAX_TRIPS)
      {
        printf("The maximum number of ships in the input file is bigger than %d",MAX_TRIPS);
        return_value = 1;
        goto cleanup;
      }
      trips[ship].MMSI = rec.MMSI;
    }
    /*
     * Append the latest observation to the corresponding ship.
     * In the input file it is assumed that
     * - The coordinates are given in the WGS84 geographic coordinate system
     * - The timestamps are given in GMT time zone
     */
    char *t_out = pg_timestamp_out(rec.T);
    sprintf(point_buffer, "SRID=4326;Point(%lf %lf)@%s+00", rec.Longitude,
      rec.Latitude, t_out);


    /* Send to the logfile the trip if reached the maximum number of instants */
    windowManager(NO_INSTANTS_BATCH,trips, ship,fileOut);

    /* Append the last observation */
    TInstant *inst = (TInstant *) tgeogpoint_in(point_buffer);
    if (! trips[ship].trip)
      trips[ship].trip = tsequence_make_exp((const TInstant **) &inst, 1,
        NO_INSTANTS_BATCH, true, true, LINEAR, false);
    else
      tsequence_append_tinstant(trips[ship].trip, inst, true);
  } while (!feof(fileIn));

  printf("\n%d records read.\n%d incomplete records ignored. %d writes to the logfile\n",
    no_records, no_nulls,count);

  /* Close the file */
  fclose(fileIn);

/* Clean up */
cleanup:

 /* Free memory */
  for (i = 0; i < numships; i++)
    free(trips[i].trip);

  /* Finalize MEOS */
  meos_finalize();

  /* Close the connection to the logfile */
  fclose(fileOut);

  return return_value;
}
