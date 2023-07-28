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
 * @brief A simple program that reads AIS data from a CSV file, constructs
 * trips from these records, and outputs for each trip the MMSI, the number of
 * instants, and the distance travelled.
 *
 * Please read the assumptions made about the input file `aisinput.csv` in the
 * file `02_meos_read_ais.c` in the same directory. Furthermore, this program
 * assumes the input file contains less than 50K observations for at most
 * five ships. Also, the program does not cope with erroneous inputs, such as
 * two or more observations for the same ship with equal timestamp values and
 * supposes that the observations are in increasing timestamp value.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o meos_assemble_ais meos_assemble_ais.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <meos.h>

/* Maximum number of records read in the CSV file
 * IMPORTANT !!!
 * Please fix this value according to the available memory in your computer */
#define MAX_NO_RECORDS 10000000
/* Number of instants in a batch for printing a marker */
#define NO_RECORDS_BATCH 100000
/* Initial number of allocated instants for an input trip and SOG */
#define INITIAL_INSTANTS 1
/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LENGTH_HEADER 1024
/* Maximum length in characters of a point in the input data */
#define MAX_LENGTH_POINT 64
/* Maximum length in characters of a timestamp in the input data */
#define MAX_LENGTH_TIMESTAMP 32
/* Maximum length in characters of all other strings in the input data */
#define MAX_LENGTH_STRING 64
/* Maximum number of trips */
#define MAX_TRIPS 5614

typedef struct
{
  Timestamp T;
  char *TypeOfMobile;
  long int MMSI;
  double Latitude;
  double Longitude;
  char *navigationalStatus;
  double ROT;
  double SOG;
  double COG;
  int Heading;
  char *IMO;
  char *Callsign;
  char *Name;
  char *ShipType;
  char *CargoType;
  double Width;
  double Length;
  char *TypeOfPositionFixingDevice;
  double Draught;
  char *Destination;
  char *ETA;
  char *DataSourceType;
  double SizeA;
  double SizeB;
  double SizeC;
  double SizeD;
} AIS_record;

typedef struct
{
  long int MMSI;    /* Identifier of the trip */
  int numinstants;  /* Number of input instants */
  int no_trip_instants;   /* Number of input instants for the trip */
  int free_trip_instants; /* Number of available input instants for the trip */
  int no_SOG_instants;   /* Number of input instants for the SOG */
  int free_SOG_instants;  /* Number of available input instants for the SOG */
  TInstant **trip_instants; /* Array of instants for the trip */
  TInstant **SOG_instants;  /* Array of instants for the SOG */
  Temporal *trip;   /* Trip constructed from the input instants */
  Temporal *SOG;    /* SOG constructed from the input instants */
} trip_record;


/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize("UTC");

  /* Get start time */
  clock_t t;
  t = clock();

  /* Allocate space to build the trips */
  trip_record trips[MAX_TRIPS] = {0};
  /* Number of ships */
  int numships = 0;
  /* Iterator variables */
  int i, j, ship;

  /* Substitute the full file path in the first argument of fopen */
  FILE *file = fopen("aisdk-2022-09-01.csv", "r");

  if (! file)
  {
    printf("Error opening file\n");
    return 1;
  }

  AIS_record rec;
  int no_records = 0;
  int no_nulls = 0;
  char header_buffer[MAX_LENGTH_HEADER];
  char point_buffer[MAX_LENGTH_POINT];
  char timestamp_buffer[MAX_LENGTH_TIMESTAMP];
  char string_buffer[MAX_LENGTH_STRING];

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023[^\n]\n", header_buffer);
  printf("Processing records (one marker every %d records)\n",
    NO_RECORDS_BATCH);

  /* Continue reading the file */
  do
  {
    // int read = fscanf(file,
    fscanf(file,
      /* T,TypeOfMobile,MMSI,Latitude,Longitude, */
      "%31[^,],%63[^,],%ld,%lf,%lf,"
      /* navigationalStatus,ROT,SOG,COG,Heading,IMO, */
      "%63[^,],%lf,%lf,%lf,%d,%63[^,],"
      /* Callsign,Name,ShipType,CargoType,Width, */
      "%63[^,],%63[^,],%63[^,],%63[^,],%lf,"
      /* Length, TypeOfPositionFixingDevice,Draught,Destination,ETA, */
      "%lf,%63[^,],%lf,%63[^,],%63[^,],"
      /* DataSourceType,SizeA,SizeB,SizeC,SizeD */
      "%63[^,],%lf,%lf,%lf,%lf\n",
      timestamp_buffer, string_buffer, &rec.MMSI, &rec.Latitude, &rec.Longitude,
      string_buffer, &rec.ROT, &rec.SOG, &rec.COG, &rec.Heading, string_buffer,
      string_buffer, string_buffer, string_buffer, string_buffer, &rec.Width,
      &rec.Length, string_buffer, &rec.Draught, string_buffer, string_buffer,
      string_buffer, &rec.SizeA, &rec.SizeB, &rec.SizeC, &rec.SizeD);
    /* Transform the string representing the timestamp into a timestamp value */
    rec.T = pg_timestamp_in(timestamp_buffer, -1);

    no_records++;
    /* Break if maximum number of records read */
    if (no_records == MAX_NO_RECORDS)
      break;
    /* Print a marker every X records read */
    if (no_records % NO_RECORDS_BATCH == 0)
    {
      printf("*");
      fflush(stdout);
    }

    if (ferror(file))
    {
      printf("Error reading file\n");
      fclose(file);
      /* Free memory */
      for (i = 0; i < numships; i++)
      {
        for (j = 0; j < trips[i].numinstants; j++)
        {
          free(trips[i].trip_instants[j]);
          free(trips[i].SOG_instants[j]);
        }
      }
      return 1;
    }

    /* Find the place to store the new instant */
    ship = -1;
    i = 0;
    while (i < MAX_TRIPS)
    {
      if (trips[i].MMSI == 0)
        break;
      if (trips[i].MMSI == rec.MMSI)
      {
        ship = i;
        break;
      }
    }
    if (ship < 0)
    {
      ship = numships++;
      trips[ship].MMSI = rec.MMSI;
      /* Allocate initial space for storing the instants */
      trips[ship].trip_instants = malloc(sizeof(TInstant *) * INITIAL_INSTANTS);
      trips[ship].SOG_instants = malloc(sizeof(TInstant *) * INITIAL_INSTANTS);
      trips[ship].no_trip_instants = INITIAL_INSTANTS;
      trips[ship].free_trip_instants = INITIAL_INSTANTS;
      trips[ship].no_SOG_instants = INITIAL_INSTANTS;
      trips[ship].free_SOG_instants = INITIAL_INSTANTS;
    }

    /*
     * Create the instants and store them in the arrays of the ship.
     * In the input file it is assumed that
     * - The coordinates are given in the WGS84 geographic coordinate system
     * - The timestamps are given in GMT time zone
     */
    char *t_out = pg_timestamp_out(rec.T);
    sprintf(point_buffer, "SRID=4326;Point(%lf %lf)@%s+00", rec.Longitude,
      rec.Latitude, t_out);
    TInstant *inst1 = (TInstant *) tgeogpoint_in(point_buffer);
    /* Ensure there is still space for storing the instant */
    if (trips[ship].free_trip_instants == 0)
    {
      trips[ship].trip_instants = realloc(trips[ship].trip_instants,
        sizeof(TInstant *) * trips[ship].no_trip_instants * 2);
      trips[ship].free_trip_instants = trips[ship].no_trip_instants;
      trips[ship].no_trip_instants *= 2;
    }
    trips[ship].trip_instants[trips[ship].numinstants] = inst1;
    trips[ship].free_trip_instants--;
    TInstant *inst2 = (TInstant *) tfloatinst_make(rec.SOG, rec.T);
    /* Ensure there is still space for storing the instant */
    if (trips[ship].free_SOG_instants == 0)
    {
      trips[ship].SOG_instants = realloc(trips[ship].SOG_instants,
        sizeof(TInstant *) * trips[ship].no_SOG_instants * 2);
      trips[ship].free_SOG_instants = trips[ship].no_SOG_instants;
      trips[ship].no_SOG_instants *= 2;
    }
    trips[ship].SOG_instants[trips[ship].numinstants++] = inst2;
    trips[ship].free_SOG_instants--;
  } while (!feof(file));

  printf("\n%d records read.\n%d incomplete records ignored.\n",
    no_records, no_nulls);
  printf("%d trips read.\n", numships);

  /* Construct the trips */
  for (i = 0; i < numships; i++)
  {
    trips[i].trip = (Temporal *) tsequence_make(
      (const TInstant **) trips[i].trip_instants,
      trips[i].numinstants, true, true, LINEAR, true);
    trips[i].SOG = (Temporal *) tsequence_make(
      (const TInstant **) trips[i].SOG_instants,
      trips[i].numinstants, true, true, LINEAR, true);
    printf("MMSI: %ld, Number of input instants: %d\n", trips[i].MMSI,
      trips[i].numinstants);
    printf("  Trip -> Number of instants: %d, Distance travelled %lf\n",
      temporal_num_instants(trips[i].trip), tpoint_length(trips[i].trip));
    printf("  SOG -> Number of instants: %d, Time-weighted average %lf\n",
      temporal_num_instants(trips[i].SOG), tnumber_twavg(trips[i].SOG));
  }

 /* Free memory */
  for (i = 0; i < numships; i++)
  {
    free(trips[i].trip);
    free(trips[i].SOG);
    for (j = 0; j < trips[i].numinstants; j++)
    {
      free(trips[i].trip_instants[j]);
      free(trips[i].SOG_instants[j]);
    }
  }

  /* Close the file */
  fclose(file);

  /* Calculate the elapsed time */
  t = clock() - t;
  double time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
