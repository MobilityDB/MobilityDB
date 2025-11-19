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
 * @brief A simple program that reads a CSV file containig geometries and
 * compute the total number of points in all geometries.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tbl_geometry_read tbl_geometry_read.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

/* Maximum length in characters of a header record in the input CSV file1 */
#define MAX_LENGTH_HEADER 1024
/* Maximum length in characters of a geometry in the input data as computed by
 * the following query on the corresponding table
 * SELECT MAX(length(g::text)) FROM tbl_geometry;
 * -- 11572
 */
#define MAX_LENGTH_GEO 12001

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* You may substitute the full file1 path in the first argument of fopen */
  FILE *file = fopen("data/tbl_geometry.csv", "r");

  if (! file)
  {
    printf("Error opening input file\n");
    fclose(file);
    return 1;
  }

  char header_buffer[MAX_LENGTH_HEADER];
  char geo_buffer[MAX_LENGTH_GEO];

  /* Read the first line of the first file with the headers */
  fscanf(file, "%1023s\n", header_buffer);

  /* Continue reading the file */
  int total_points = 0;
  do
  {
    int k;
    int read = fscanf(file, "%d,%12000[^\n]\n", &k, geo_buffer);

    if (ferror(file))
    {
      printf("Error reading input file\n");
      return 1;
    }
    if (read != 2)
    {
      printf("Error reading input file\n");
      fclose(file);
      return 1;
    }

    /* Transform the string representing the tcbuffer into a tcbuffer value */
    GSERIALIZED *gs = geom_in(geo_buffer, -1);

    /* Add the number of points of the geometry to the total number */
    int npoints = geo_npoints(gs);
    total_points += npoints;
    free(gs);

    /* Print only 1 out of 100 records */
    if (k % 10 == 0)
    {
      printf("k: %d, Number of points in the geometry: %d\n",
        k, npoints);
    }
  } while (!feof(file));


  printf("------------------------------\n");
  printf("Total number of points: %d\n", total_points);
  printf("------------------------------\n");

  /* Close the files */
  fclose(file);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
