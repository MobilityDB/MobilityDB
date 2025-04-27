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
 * @brief A simple program that reads two CSV files, the first one containing
 * temporal circular buffers and the second containing geometries and restricts
 * the temporal circular buffers to the geometries.
 *
 * The corresponding SQL query would be
 * @code
 * SELECT numInstants(atGeometry(temp, g))
 * FROM tbl_tcbuffer, tbl_geometry
 * @endcode
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tbl_tcbuffer_geometry tbl_tcbuffer_geometry.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_cbuffer.h>

/* Maximum length in characters of a header record in the input CSV file1 */
#define MAX_LENGTH_HEADER 1024
/* Maximum length in characters of a geometry in the input data as computed by
 * the following query on the corresponding table
 * SELECT MAX(length(ST_AsHexewkb(g))) FROM tbl_geometry;
 * -- 11572
 */
#define MAX_LENGTH_GEO 12001
/* Maximum length in characters of a temporal circular buffer in the input
 * data as computed by the following query on the corresponding table
 * SELECT MAX(length(temp::text)) FROM tbl_tcbuffer;
 * -- 7449
 */
#define MAX_LENGTH_TCBUFFER 7501

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* You may substitute the full file1 path in the first argument of fopen */
  FILE *file1 = fopen("data/tbl_tcbuffer.csv", "r");

  if (! file1)
  {
    printf("Error opening first input file\n");
    return 1;
  }

  /* You may substitute the full file1 path in the first argument of fopen */
  FILE *file2 = fopen("data/tbl_geometry.csv", "r");

  if (! file2)
  {
    printf("Error opening second input file\n");
    fclose(file2);
    return 1;
  }

  char header_buffer[MAX_LENGTH_HEADER];
  char geo_buffer[MAX_LENGTH_GEO];
  char tcbuffer_buffer[MAX_LENGTH_TCBUFFER];

  /* Read the first line of the first file with the headers */
  fscanf(file1, "%1023s\n", header_buffer);

  /* Continue reading the first file */
  do
  {
    int k1;
    int read1 = fscanf(file1, "%d,\"%7500[^\"\n]\"\n", &k1, tcbuffer_buffer);

    if (ferror(file1) || read1 != 2)
    {
      printf("Error reading input file1\n");
      fclose(file1);
      fclose(file2);
      return 1;
    }

    /* Transform the string read into a tcbuffer value */
    Temporal *temp = tcbuffer_in(tcbuffer_buffer);

    /* Rewind the second file to the beginning */
    rewind(file2);
    /* Read the first line of the second file with the headers */
    fscanf(file2, "%1023s\n", header_buffer);

    /* For each line in the first file loop for every line in the second file */
    do
    {

      int k2;
      int read2 = fscanf(file2, "%d,%12000[^\n]\n", &k2, geo_buffer);

      if (ferror(file2) || read2 != 2)
      {
        printf("Error reading input file2\n");
        fclose(file1);
        fclose(file2);
        return 1;
      }

      /* Print only 1 out of 100 records */
      if (k1 % 10 == 0 && k2 % 10 == 0)
      {

        /* Transform the string read into a geometry value */
        GSERIALIZED *gs = geom_in(geo_buffer, -1);

        /* Compute the atGeometry function */
        Temporal *rest = tintersects_tcbuffer_geo(temp, gs, false, false);
        /* Get the number of instants of the result */
        int count = rest ? temporal_num_instants(rest): 0;
        free(rest);
        free(gs);

        printf("k1: %d, k2: %d: Number of instants of the result: %d\n",
          k1, k2, count);
      }
    } while (! feof(file2));
    free(temp);
  } while (! feof(file1));

  /* Close the files */
  fclose(file1);
  fclose(file2);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
