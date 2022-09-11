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
 * @brief A simple program that reads AIS data from a CSV file and outputs
 * a few of these records converted into temporal values.
 *
 * In the input file `aisinput.csv` in the same directory it is assumed that
 * - The coordinates are given in the WGS84 geographic coordinate system, and
 * - The timestamps are given in the GMT time zone.
 * This simple program does not cope with erroneous inputs, such as missing
 * fields or invalid timestamp values.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o meos_read_ais meos_read_ais.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include "meos.h"

typedef struct
{
  Timestamp T;
  long int MMSI;
  double Latitude;
  double Longitude;
  double SOG;
} AIS_record;

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();

  /* You may substitute the full file path in the first argument of fopen */
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

    /* Print only 1 out of 1000 records */
    if (records % 1000 == 0)
    {
      char *t_out = pg_timestamp_out(rec.T);
      /* See above the assumptions made wrt the input data in the file */
      sprintf(buffer, "SRID=4326;Point(%lf %lf)@%s+00", rec.Longitude,
        rec.Latitude, t_out);
      Temporal *inst1 = tgeogpoint_in(buffer);
      char *inst1_out = tpoint_as_text(inst1, 2);

      TInstant *inst2 = tfloatinst_make(rec.SOG, rec.T);
      char *inst2_out = tfloat_out((Temporal *) inst2, 2);
      printf("MMSI: %ld, Location: %s SOG : %s\n",
        rec.MMSI, inst1_out, inst2_out);

      free(inst1); free(t_out); free(inst1_out);
      free(inst2); free(inst2_out);
    }

  } while (!feof(file));

  printf("\n%d records read.\n%d incomplete records ignored.\n",
    records, nulls);

  /* Close the file */
  fclose(file);

  /* Finalize MEOS */
  meos_finish();

  return 0;
}
