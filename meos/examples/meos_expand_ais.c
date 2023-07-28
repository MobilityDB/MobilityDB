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
 * file `meos_read_ais.c` in the same directory. Furthermore, this program
 * assumes the input file contains less than 50K observations for at most
 * five ships. Also, the program does not cope with erroneous inputs, such as
 * two or more observations for the same ship with equal timestamp values and
 * supposes that the observations are in increasing timestamp value.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o meos_expand_ais meos_expand_ais.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <meos.h>

/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LENGTH_HEADER 1024
/* Maximum length in characters of a point in the input data */
#define MAX_LENGTH_INST 64
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
  Temporal *trip;  /* Trip constructed with expandable structures */
  Temporal *SOG;   /* SOG constructed with expandable structures */
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
  /* Iterator variable */
  int i;

  /* Substitute the full file path in the first argument of fopen */
  FILE *file = fopen("aisinput_one.csv", "r"); // TO CHANGE

  if (! file)
  {
    printf("Error opening file\n");
    return 1;
  }

  AIS_record rec;
  int no_records = 0;
  int no_nulls = 0;
  char header_buffer[MAX_LENGTH_HEADER];
  char inst_buffer[MAX_LENGTH_INST];
  char timestamp_buffer[MAX_LENGTH_TIMESTAMP];

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", header_buffer);

  /* Continue reading the file */
  do
  {
    int read = fscanf(file, "%31[^,],%ld,%lf,%lf,%lf\n",
      timestamp_buffer, &rec.MMSI, &rec.Latitude, &rec.Longitude, &rec.SOG);
    /* Transform the string representing the timestamp into a timestamp value */
    rec.T = pg_timestamp_in(timestamp_buffer, -1);

    if (read == 5)
      no_records++;

    if (read != 5 && !feof(file))
    {
      printf("Record with missing values ignored\n");
      no_nulls++;
    }

    if (ferror(file))
    {
      printf("Error reading file\n");
      fclose(file);
      /* Free memory */
      for (i = 0; i < numships; i++)
      {
        // free(trips[i].trip);
        free(trips[i].SOG);
      }
      return 1;
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
      trips[ship].MMSI = rec.MMSI;
    }
    trips[ship].numinstants++;

    /*
     * Create the instants and append them in the corresponding ship record.
     * In the input file it is assumed that
     * - The coordinates are given in the WGS84 geographic coordinate system
     * - The timestamps are given in GMT time zone
     */
    char *t_out = pg_timestamp_out(rec.T);
    sprintf(inst_buffer, "SRID=4326;Point(%lf %lf)@%s", rec.Longitude,
      rec.Latitude, t_out);
    // TInstant *inst1 = (TInstant *) tgeogpoint_in(inst_buffer);
    // if (! trips[ship].trip)
      // trips[ship].trip = (Temporal *) tsequence_make_exp(
        // (const TInstant **) &inst1, 1, 2, true, true, LINEAR, false);
    // else
      // trips[ship].trip = temporal_append_tinstant(trips[ship].trip, inst1, true);
    TInstant *inst2 = (TInstant *) tfloatinst_make(rec.SOG, rec.T);
    if (! trips[ship].SOG)
      trips[ship].SOG = (Temporal *) tsequence_make_exp(
        (const TInstant **) &inst2, 1, 2, true, true, LINEAR, false);
    else
      trips[ship].SOG = temporal_append_tinstant(trips[ship].SOG, inst2, true);
    // free(inst1);
    free(inst2);
  } while (!feof(file));

  printf("\n%d records read.\n%d incomplete records ignored.\n",
    no_records, no_nulls);
  printf("%d trips read.\n", numships);

  /* Print information about the trips */
  for (i = 0; i < numships; i++)
  {
    printf("MMSI: %ld, Number of input instants: %d\n", trips[i].MMSI,
      trips[i].numinstants);
    // printf("  Trip -> Number of instants: %d, Distance travelled %lf\n",
      // temporal_num_instants(trips[i].trip), tpoint_length(trips[i].trip));
    printf("  SOG -> Number of instants: %d, Time-weighted average %lf\n",
      temporal_num_instants(trips[i].SOG), tnumber_twavg(trips[i].SOG));
  }

  /* Free memory */
  for (i = 0; i < numships; i++)
  {
    // free(trips[i].trip);
    free(trips[i].SOG);
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
