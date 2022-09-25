/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * gcc -Wall -g -I/usr/local/include -o meos_assemble_ais meos_assemble_ais.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include "meos.h"

#define MAX_INSTANTS 50000
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
  long int MMSI;
  TInstant *instants[MAX_INSTANTS];
} MMSI_instants;

/* Main program */
int main(void)
{
  /* Allocate space to build the instants for each trip and the trips */
  MMSI_instants trip_instants[MAX_TRIPS];
  TSequence *trips[MAX_TRIPS];
  /* Number of instants for each trip */
  int numinstants[MAX_TRIPS] = {0};
  /* Number of ships */
  int numships = 0;
  /* Iterator variables */
  int i, j;

  /* Initialize MEOS */
  meos_initialize();

  /* Substitute the full file path in the first argument of fopen */
  FILE *file = fopen("aisinput.csv", "r");

  if (! file)
  {
    printf("Error opening file\n");
    return 1;
  }

  AIS_record rec;
  int records = 0;
  int nulls = 0;
  char buffer[1024];

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", buffer);

  /* Continue reading the file */
  do
  {
    int read = fscanf(file, "%32[^,],%ld,%lf,%lf,%lf\n",
      buffer, &rec.MMSI, &rec.Latitude, &rec.Longitude, &rec.SOG);
    /* Transform the string representing the timestamp into a timestamp value */
    rec.T = pg_timestamp_in(buffer, -1);

    if (read == 5)
      records++;

    if (read != 5 && !feof(file))
    {
      printf("Record with missing values ignored\n");
      nulls++;
    }

    if (ferror(file))
    {
      printf("Error reading file\n");
      fclose(file);
      return 1;
    }

    /* Find the place to store the new instant */
    int ship = -1;
    for (i = 0; i < 5; i++)
    {
      if (trip_instants[i].MMSI == rec.MMSI)
      {
        ship = i;
        break;
      }
    }
    if (ship < 0)
    {
      ship = numships;
      numships++;
    }
    trip_instants[ship].MMSI = rec.MMSI;

    /*
     * Create the instant and store it in the array of the corresponding ship.
     * In the input file it is assumed that
     * - The coordinates are given in the WGS84 geographic coordinate system
     * - The timestamps are given in GMT time zone
     */
    char *t_out = pg_timestamp_out(rec.T);
    sprintf(buffer, "SRID=4326;Point(%lf %lf)@%s+00", rec.Longitude,
      rec.Latitude, t_out);
    TInstant *inst = (TInstant *) tgeogpoint_in(buffer);
    trip_instants[ship].instants[numinstants[ship]++] = inst;
  } while (!feof(file));

  printf("\n%d records read.\n%d incomplete records ignored.\n",
    records, nulls);
  printf("%d trips read.\n", numships);

  /* Construct the trips */
  for (i = 0; i < numships; i++)
  {
    trips[i] = tsequence_make((const TInstant **) trip_instants[i].instants,
      numinstants[i], numinstants[i], true, true, true, true);
    printf("MMSI: %ld, Number of input instants: %d, Number of instants: %d, "
      "Distance travelled %lf\n", trip_instants[i].MMSI, numinstants[i],
      trips[i]->count, tpoint_length((Temporal *) trips[i]));
  }

  /* Free memory */
  for (i = 0; i < numships; i++)
    for (j = 0; j < numinstants[i]; j++)
      free(trip_instants[i].instants[j]);

  /* Close the file */
  fclose(file);

  /* Finalize MEOS */
  meos_finish();

  return 0;
}
