/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief A program that reads AIS data from a CSV file containing the temporal
 * values for trip and SOG for the longest 10 trips of ships extracted from one
 * day of observations (aisdk-2023-08-01.csv) provided by the Danish Maritime
 * Authority in https://web.ais.dk/aisdata/, and outputs the size in (KB) and
 * number of instants for various simplification algorithms.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o ais_generalize ais_generalize.c -L/usr/local/lib -lmeos
 * @endcode
 * The output of the program is as follows
 * @code
 * Processing records
 *   one '*' marker every record
 * **********
 * 10 trips read.
 * Generalizing trips
 *   one '*' marker every trip
 * **********
  ---------------------------------------------------------------------------------
  Total size comparison (in KB:#instants) between various generalization algorithms
  ---------------------------------------------------------------------------------
  Original 10346:189174
  DP-ED 1 1266:23139, DP-ED 10 176:3204, DP-SED 1 3905:71401, DP-SED 10 1037:18959
  MaxD-ED 1 1519:27764, MaxD-ED 10 227:4140, MaxD-SED 1 3988:72909, MaxD-SED 10 1141:20846
  MinD 1 8032:146861, MinD 10 5817:106363, MinTD 1 9632:176111, MinTD 10 2963:54169
  Tprec 1 19913:364118, Tprec 10 4312:78844
  -------------------------------------------------------------------------------

  The program took 11.921875 seconds to execute 
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* For memset */
#include <time.h>
#include <meos.h>
#include <meos_internal.h> /* For temporal_mem_size */

/* Maximum number of ships */
#define MAX_SHIPS 10
/* Maximum length in characters of a record in the input CSV file */
#define MAX_LENGTH_LINE 2000001

typedef struct
{
  long int MMSI;      /* Identifier of the trip */
  Temporal *trip;     /* Trip constructed from the instants */
  Temporal *SOG;      /* SOG constructed from the instants */
} trip_record;

/* Main program */
int main(void)
{
  /* Input buffers to read the CSV file */
  char line_buffer[MAX_LENGTH_LINE];
  /* Allocate space to build the trips */
  trip_record trips[MAX_SHIPS] = {0};
  /* Number of ship records read */
  int no_ships = 0;
  /* Iterator variable */
  int i;
  /* Exit value initialized to 1 (i.e., error) to quickly exit upon error */
  int exit_value = 1;

  /* Initialize MEOS */
  meos_initialize("UTC", NULL);

  /* Get start time */
  clock_t t = clock();

  /*
   * Open the input file
   * Substitute the full file path in the first argument of fopen
   */
  FILE *file = fopen("data/ships_10.csv", "r");
  if (! file)
  {
    printf("Error opening input file\n");
    goto cleanup;
  }

  /* Read the first line of the file with the headers */
  fscanf(file, "%2000000[^\n]\n", line_buffer);
  if (ferror(file))
  {
    printf("\nError reading input file\n");
    fclose(file);
    goto cleanup;
  }

  printf("Processing records\n");
  printf("  one '*' marker every record\n");

  /* Continue reading the file */
  do
  {
    fscanf(file, "%2000000[^\n]\n", line_buffer);
    if (ferror(file))
    {
      printf("\nError reading input file\n");
      fclose(file);
      goto cleanup;
    }
    printf("*");

    int field = 0;
    char *token = strtok(line_buffer, ",");
    while (token != NULL)
    {
      switch (field)
      {
        case 0:
          trips[no_ships].MMSI = strtoll(token, NULL, 0);
          break;
        case 1:
          trips[no_ships].trip = temporal_from_hexwkb(token);
          break;
        case 2:
          trips[no_ships].SOG = temporal_from_hexwkb(token);
          break;
        default:
          break;
      }
      token = strtok(NULL, ",");
      field++;
    }
    no_ships++;
  } while (! feof(file));

  /* Close the file */
  fclose(file);

  printf("\n%d trips read.\n", no_ships);

  printf("Generalizing trips\n");
  printf("  one '*' marker every ship record\n");
  /* Compute the size of various generalizations */
  size_t orig = 0, dp_ed_1 = 0, dp_ed_10 = 0, dp_sed_1 = 0, dp_sed_10 = 0,
    max_dist_ed_1 = 0, max_dist_ed_10 = 0, max_dist_sed_1 = 0, max_dist_sed_10 = 0,
    min_dist_1 = 0, min_dist_10 = 0, min_tdelta_1 = 0, min_tdelta_10 = 0,
    tprec_1s = 0, tprec_10s = 0;
  int no_orig = 0, no_dp_ed_1 = 0, no_dp_ed_10 = 0, no_dp_sed_1 = 0, no_dp_sed_10 = 0,
    no_max_dist_ed_1 = 0, no_max_dist_ed_10 = 0, no_max_dist_sed_1 = 0, no_max_dist_sed_10 = 0,
    no_min_dist_1 = 0, no_min_dist_10 = 0, no_min_tdelta_1 = 0, no_min_tdelta_10 = 0,
    no_tprec_1s = 0, no_tprec_10s = 0;
  for (i = 0; i < no_ships; i++)
  {
    /* Print a marker every ship */
    printf("*");
    fflush(stdout);

    /* Input trip */
    orig += temporal_mem_size(trips[i].trip);
    no_orig += temporal_num_instants(trips[i].trip);

    /* Douglas-Peucker simplification */
    Temporal *temp = temporal_simplify_dp(trips[i].trip, 1, false);
    dp_ed_1 += temporal_mem_size(temp);
    no_dp_ed_1 += temporal_num_instants(temp);
    free(temp);
    temp = temporal_simplify_dp(trips[i].trip, 10, false);
    dp_ed_10 += temporal_mem_size(temp);
    no_dp_ed_10 += temporal_num_instants(temp);
    free(temp);
    temp = temporal_simplify_dp(trips[i].trip, 1, true);
    dp_sed_1 += temporal_mem_size(temp);
    no_dp_sed_1 += temporal_num_instants(temp);
    free(temp);
    temp = temporal_simplify_dp(trips[i].trip, 10, true);
    dp_sed_10 += temporal_mem_size(temp);
    no_dp_sed_10 += temporal_num_instants(temp);

    /* Douglas-Peucker maximum distance simplification */
    temp = temporal_simplify_max_dist(trips[i].trip, 1, false);
    max_dist_ed_1 += temporal_mem_size(temp);
    no_max_dist_ed_1 += temporal_num_instants(temp);
    free(temp);
    temp = temporal_simplify_max_dist(trips[i].trip, 10, false);
    max_dist_ed_10 += temporal_mem_size(temp);
    no_max_dist_ed_10 += temporal_num_instants(temp);
    free(temp);
    temp = temporal_simplify_max_dist(trips[i].trip, 1, true);
    max_dist_sed_1 += temporal_mem_size(temp);
    no_max_dist_sed_1 += temporal_num_instants(temp);
    free(temp);
    temp = temporal_simplify_max_dist(trips[i].trip, 10, true);
    max_dist_sed_10 += temporal_mem_size(temp);
    no_max_dist_sed_10 += temporal_num_instants(temp);

    /* Minimum distance simplification */
    temp = temporal_simplify_min_dist(trips[i].trip, 1);
    min_dist_1 += temporal_mem_size(temp);
    no_min_dist_1 += temporal_num_instants(temp);
    free(temp);
    temp = temporal_simplify_min_dist(trips[i].trip, 10);
    min_dist_10 += temporal_mem_size(temp);
    no_min_dist_10 += temporal_num_instants(temp);
    free(temp);

    /* Minimum tdelta simplification */
    Interval *secs1 = pg_interval_in("1 second", -1);
    Interval *secs10 = pg_interval_in("10 seconds", -1);
    temp = temporal_simplify_min_tdelta(trips[i].trip, secs1);
    min_tdelta_1 += temporal_mem_size(temp);
    no_min_tdelta_1 += temporal_num_instants(temp);
    free(temp);
    temp = temporal_simplify_min_tdelta(trips[i].trip, secs10);
    min_tdelta_10 += temporal_mem_size(temp);
    no_min_tdelta_10 += temporal_num_instants(temp);
    free(temp);

    /* Tprecision simplification */
    TimestampTz origin = pg_timestamp_in("2000-01-01", -1);
    temp = temporal_tprecision(trips[i].trip, secs1, origin);
    tprec_1s += temporal_mem_size(temp);
    no_tprec_1s += temporal_num_instants(temp);
    free(temp);
    temp = temporal_tprecision(trips[i].trip, secs10, origin);
    tprec_10s += temporal_mem_size(temp);
    no_tprec_10s += temporal_num_instants(temp);
    free(temp);
  }

  printf("\n---------------------------------------------------------------------------------\n");
  printf("Total size comparison (in KB:#instants) between various generalization algorithms\n");
  printf("---------------------------------------------------------------------------------\n");
  printf("Original %zu:%d\n", orig / 1024, no_orig);
  printf("DP-ED 1 %zu:%d, DP-ED 10 %zu:%d, DP-SED 1 %zu:%d, DP-SED 10 %zu:%d\n",
    dp_ed_1 / 1024, no_dp_ed_1, dp_ed_10 / 1024, no_dp_ed_10, 
    dp_sed_1 / 1024, no_dp_sed_1, dp_sed_10 / 1024, no_dp_sed_10);
  printf("MaxD-ED 1 %zu:%d, MaxD-ED 10 %zu:%d, MaxD-SED 1 %zu:%d, MaxD-SED 10 %zu:%d\n",
    max_dist_ed_1 / 1024, no_max_dist_ed_1, max_dist_ed_10 / 1024, no_max_dist_ed_10, 
    max_dist_sed_1 / 1024, no_max_dist_sed_1, max_dist_sed_10 / 1024, no_max_dist_sed_10);
  printf("MinD 1 %zu:%d, MinD 10 %zu:%d, MinTD 1 %zu:%d, MinTD 10 %zu:%d\n",
    min_dist_1 / 1024, no_min_dist_1, min_dist_10 / 1024, no_min_dist_10, 
    min_tdelta_1 / 1024, no_min_tdelta_1, min_tdelta_10 / 1024, no_min_tdelta_10);
  printf("Tprec 1 %zu:%d, Tprec 10 %zu:%d\n",
    tprec_1s / 1024, no_tprec_1s, tprec_10s / 1024, no_tprec_10s);

  printf("-------------------------------------------------------------------------------\n");

  /* Calculate the elapsed time */
  t = clock() - t;
  double time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);

  /* State that the program executed successfully */
  exit_value = 0;

/* Clean up */
cleanup:

 /* Free memory */
  for (i = 0; i < no_ships; i++)
  {
    free(trips[i].trip);
    free(trips[i].SOG);
  }

  /* Finalize MEOS */
  meos_finalize();

  return exit_value;
}
