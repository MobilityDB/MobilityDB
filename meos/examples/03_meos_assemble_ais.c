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
 * instants, and the distance travelled. The program also stores in a CSV file
 * the assembled trips.
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
 * gcc -Wall -g -I/usr/local/include -o 03_meos_assemble_ais 03_meos_assemble_ais.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <meos.h>

/* Maximum number of instants in an input trip */
#define MAX_INSTANTS 50000
/* Number of instants in a batch for printing a marker */
#define NO_INSTANTS_BATCH 1000
/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LENGTH_HEADER 1024
/* Maximum length in characters of a point in the input data */
#define MAX_LENGTH_POINT 64
/* Maximum length in characters of a timestamp in the input data */
#define MAX_LENGTH_TIMESTAMP 32
/* Maximum number of trips */
#define MAX_TRIPS 5

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
  int numinstants; /* Number of input instants */
  TInstant *trip_instants[MAX_INSTANTS]; /* Array of instants for the trip */
  TInstant *SOG_instants[MAX_INSTANTS];  /* Array of instants for the SOG */
  Temporal *trip;  /* Trip constructed from the input instants */
  Temporal *SOG;   /* SOG constructed from the input instants */
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
  int i, j;
  /* Return value */
  int return_value = 0;

  /* Substitute the full file path in the first argument of fopen */
  FILE *file = fopen("aisinput.csv", "r");

  if (! file)
  {
    printf("Error opening file\n");
    meos_finalize();
    return 1;
  }

  AIS_record rec;
  int no_records = 0;
  int no_nulls = 0;
  char header_buffer[MAX_LENGTH_HEADER];
  char point_buffer[MAX_LENGTH_POINT];
  char timestamp_buffer[MAX_LENGTH_TIMESTAMP];

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", header_buffer);

  printf("Reading the instants (one marker every %d instants)\n",
    NO_INSTANTS_BATCH);

  /* Continue reading the file */
  do
  {
    int read = fscanf(file, "%31[^,],%ld,%lf,%lf,%lf\n",
      timestamp_buffer, &rec.MMSI, &rec.Latitude, &rec.Longitude, &rec.SOG);
    /* Transform the string representing the timestamp into a timestamp value */
    rec.T = pg_timestamp_in(timestamp_buffer, -1);

    if (read == 5)
    {
      no_records++;
      if (no_records % NO_INSTANTS_BATCH == 0)
      {
        printf("*");
        fflush(stdout);
      }
    }

    if (read != 5 && !feof(file))
    {
      printf("Record with missing values ignored\n");
      no_nulls++;
    }

    if (ferror(file))
    {
      printf("Error reading the input file");
      return_value = 1;
      goto cleanup;
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
        printf("The maximum number of ships in the input file is bigger than %d",
          MAX_TRIPS);
        return_value = 1;
        goto cleanup;
      }
      trips[ship].MMSI = rec.MMSI;
    }

    /*
     * Create the instant and store it in the array of the corresponding ship.
     * In the input file it is assumed that
     * - The coordinates are given in the WGS84 geographic coordinate system
     * - The timestamps are given in GMT time zone
     */
    char *t_out = pg_timestamp_out(rec.T);
    sprintf(point_buffer, "SRID=4326;Point(%lf %lf)@%s+00", rec.Longitude,
      rec.Latitude, t_out);
    TInstant *inst1 = (TInstant *) tgeogpoint_in(point_buffer);
    trips[ship].trip_instants[trips[ship].numinstants] = inst1;
    TInstant *inst2 = (TInstant *) tfloatinst_make(rec.SOG, rec.T);
    trips[ship].SOG_instants[trips[ship].numinstants++] = inst2;
  } while (!feof(file));

  /* Close the input file */
  fclose(file);

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

  /* Open the output file */
  file = fopen("aistrips.csv", "w+");

  /* Write the header line */
  fprintf(file,"mmsi,trip,sog\n");

  /* Loop for each trip */
  for (i = 0; i < numships; i++)
  {
    /* Write line in the CSV file */
    char *trip_str = tpoint_out(trips[i].trip, 6);
    char *sog_str = tfloat_out(trips[i].SOG, 6);
    fprintf(file,"%ld,%s,%s\n", trips[i].MMSI, trip_str, sog_str);
    free(trip_str); free(sog_str);
  }

  /* Close the file */
  fclose(file);

  /* Calculate the elapsed time */
  t = clock() - t;
  double time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);

/* Clean up */
cleanup:

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

  /* Finalize MEOS */
  meos_finalize();

  return return_value;
}
