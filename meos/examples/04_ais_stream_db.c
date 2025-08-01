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
 * @brief A simple program that reads AIS data from a CSV file, accumulates the
 * observations in main memory and send the temporal values to a MobilityDB
 * database when they reach a given number of instants in order to free
 * the memory and ingest the newest observations.
 *
 * This program is similar to `04_ais_store` but illustrates the use of
 * MEOS expandable data structures, which were designed to cope with the
 * requirements of stream applications. In this setting, the expandable data
 * structures accumulate the observations that have been received so far.
 * Depending on application requirements and the available memory, the
 * accumulated temporal values must be sent regularly to the database.
 *
 * This program uses the libpq library
 * https://www.postgresql.org/docs/current/libpq.html
 * for connecting to a PostgreSQL database that has the MobilityDB extension.
 * For this, it is required that `libpq-dev` is installed in your system.
 * For example, in Ubuntu you can install this library as follows
 * @code
 * sudo apt-get install libpq-dev
 * @endcode
 *
 * You should configure PostgreSQL to accept all incoming connections by
 * setting in the `pg_hba.conf` file
 * @code
 * host all all 0.0.0.0/0 md5
 * @endcode
 * Also, make sure that PostgreSQL allows incoming connections on all available
 * IP interfaces by setting in the `postgresql.conf` file
 * @code
 * listen_addresses = '*'
 * @endcode
 *
 * This program is based on the libpq example programs
 * https://www.postgresql.org/docs/current/libpq-example.html
 * Please read the assumptions made about the input file in the file
 * `02_ais_read.c` in the same directory.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -I/usr/include/postgresql -o 04_ais_stream_db 04_ais_stream_db.c -L/usr/local/lib -lmeos -lpq
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libpq-fe.h>
#include <meos.h>
#include <meos_geo.h>
/* The expandable functions are in the internal MEOS API */
#include <meos_internal.h>

/* Number of instants to send in batch to the database */
#define NO_INSTANTS_BATCH 1000
/* Number of instants to keep when restarting a sequence, should keep at least one */
#define NO_INSTANTS_KEEP 2
/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LENGTH_HEADER 1024
/* Maximum length in characters of a point in the input data */
#define MAX_LENGTH_POINT 64
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
  TSequence *trip; /* Latest observations of the trip */
} trip_record;

/**
 * @brief Function that sends a SQL query to the database
 * @returns 0 if OK -1 on error
 */
static int
exec_sql(PGconn *conn, const char *sql, ExecStatusType status)
{
  int result = 0;
  PGresult *res = PQexec(conn, sql);
  if (PQresultStatus(res) != status)
  {
    fprintf(stderr, "SQL command failed:\n%s %s", sql, PQerrorMessage(conn));
    result = -1;
  }
  PQclear(res);
  return result;
}

/* Main program */
int
main(int argc, char **argv)
{
  const char *conninfo;
  PGconn *conn;
  int res_sql;
  AIS_record rec;
  int no_records = 0;
  int no_nulls = 0;
  char text_buffer[MAX_LENGTH_HEADER];
  /* Allocate space to build the trips */
  trip_record trips[MAX_TRIPS] = {0};
  /* Number of ships */
  int no_ships = 0;
  /* Number of writes */
  int no_writes = 0;
  /* Iterator variables */
  int i, j;
  /* Exit value initialized to 1 (i.e., error) to quickly exit upon error */
  int exit_value = EXIT_FAILURE;

  /* Get start time */
  clock_t t;
  t = clock();
  
  /***************************************************************************
   * Section 1: Connexion to the database
   ***************************************************************************/

  /*
   * If the user supplies a parameter on the command line, use it as the
   * conninfo string; otherwise default to setting dbname=postgres and using
   * environment variables or defaults for all other connection parameters.
   */
  if (argc > 1)
    conninfo = argv[1];
  else
    conninfo = "host=localhost user=esteban dbname=test";

  /* Make a connection to the database */
  conn = PQconnectdb(conninfo);

  /* Check to see that the backend connection was successfully made */
  if (PQstatus(conn) != CONNECTION_OK)
  {
    fprintf(stderr, "%s", PQerrorMessage(conn));
    goto cleanup;
  }

  /* Set always-secure search path, so malicious users can't take control. */
  res_sql = exec_sql(conn, "SELECT pg_catalog.set_config('search_path', '', false)",
    PGRES_TUPLES_OK);
  if (res_sql < 0)
  {
    fprintf(stderr, "%s", PQerrorMessage(conn));
    goto cleanup;
  }

  /* Create the table that will hold the data */
  printf("Creating the table AISTrips in the database\n");
  res_sql = exec_sql(conn, "DROP TABLE IF EXISTS public.AISTrips;",
    PGRES_COMMAND_OK);
  if (res_sql < 0)
  {
    fprintf(stderr, "%s", PQerrorMessage(conn));
    goto cleanup;
  }
  res_sql = exec_sql(conn, "CREATE TABLE public.AISTrips("
    "MMSI integer PRIMARY KEY, trip public.tgeogpoint);", PGRES_COMMAND_OK);
  if (res_sql < 0)
  {
    fprintf(stderr, "%s", PQerrorMessage(conn));
    goto cleanup;
  }

  /***************************************************************************
   * Section 2: Initialize MEOS and open the input AIS file
   ***************************************************************************/

  /* Initialize MEOS */
  meos_initialize();

  /* You may substitute the full file path in the first argument of fopen */
  FILE *file = fopen("data/ais_instants.csv", "r");
  if (! file)
  {
    printf("Error opening input file\n");
    goto cleanup;
  }

  /***************************************************************************
   * Section 3: Read input file line by line and append each observation as a
   * temporal point in MEOS
   ***************************************************************************/

  printf("Accumulating %d instants before sending them to the database\n"
    "(one '*' marker every database update)\n", NO_INSTANTS_BATCH);

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", text_buffer);

  /* Continue reading the file */
  do
  {
    int read = fscanf(file, "%32[^,],%ld,%lf,%lf,%lf\n",
      text_buffer, &rec.MMSI, &rec.Latitude, &rec.Longitude, &rec.SOG);
    if (ferror(file))
    {
      printf("Error reading input file\n");
      fclose(file);
      goto cleanup;
    }
    if (read != 5)
    {
      printf("Record with missing values ignored\n");
      no_nulls++;
      continue;
    }

    no_records++;
    
    /* Transform the string representing the timestamp into a timestamp value */
    rec.T = timestamp_in(text_buffer, -1);

    /* Find the place to store the new instant */
    j = -1;
    for (i = 0; i < MAX_TRIPS; i++)
    {
      if (trips[i].MMSI == rec.MMSI)
      {
        j = i;
        break;
      }
    }
    if (j < 0)
    {
      j = no_ships++;
      if (j == MAX_TRIPS)
      {
        printf("The maximum number of ships in the input file is bigger than %d",
          MAX_TRIPS);
        fclose(file);
        goto cleanup;
      }
      trips[j].MMSI = rec.MMSI;
    }

    /* Send the trip to the database when its size reaches the maximum size */
    if (trips[j].trip && trips[j].trip->count == NO_INSTANTS_BATCH)
    {
      /* Construct the query to be sent to the database */
      char *temp_out = tsequence_out(trips[j].trip, 15);
      size_t len = sizeof(char) * (strlen(temp_out) + 256);
      char *query_buffer = malloc(len + 1);
      snprintf(query_buffer, len, "INSERT INTO public.AISTrips(MMSI, trip) "
        "VALUES (%ld, '%s') ON CONFLICT (MMSI) DO "
        "UPDATE SET trip = public.update(AISTrips.trip, EXCLUDED.trip, true);",
        trips[j].MMSI, temp_out);

      res_sql = exec_sql(conn, query_buffer, PGRES_COMMAND_OK);
      if (res_sql < 0)
      {
        fclose(file);
        goto cleanup;
      }

      /* Free memory */
      free(temp_out);
      free(query_buffer);
      no_writes++;
      printf("*");
      fflush(stdout);
      /* Restart the sequence by only keeping the last instants */
      tsequence_restart(trips[j].trip, NO_INSTANTS_KEEP);
    }

    /* Append the last observation to the corresponding ship.
     * In the input file it is assumed that
     * - The coordinates are given in the WGS84 geographic coordinate system
     * - The timestamps are given in GMT time zone */
    GSERIALIZED *gs = geogpoint_make2d(4326, rec.Longitude, rec.Latitude);
    TInstant *inst = tpointinst_make(gs, rec.T);
    free(gs);
    if (! trips[j].trip)
      trips[j].trip = tsequence_make_exp((const TInstant **) &inst, 1,
        NO_INSTANTS_BATCH, true, true, LINEAR, false);
    else
      tsequence_append_tinstant(trips[j].trip, inst, 0.0, NULL, true);
    free(inst);
  } while (! feof(file));

  /* Close the file */
  fclose(file);

  printf("\n%d records read\n%d incomplete records ignored\n"
    "%d writes to the database\n", no_records, no_nulls, no_writes);

  snprintf(text_buffer, MAX_LENGTH_HEADER - 1,
    "SELECT MMSI, public.numInstants(trip) FROM public.AISTrips;");
  PGresult *res = PQexec(conn, text_buffer);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    fprintf(stderr, "SQL command failed:\n%s %s", text_buffer,
      PQerrorMessage(conn));
    PQclear(res);
    goto cleanup;
  }

  int numrows = PQntuples(res);
  printf("Result of the query '%s'\n\n", text_buffer);
  printf("   mmsi    | numinstants\n");
  printf("-----------+-------------\n");
  for (i = 0; i < numrows; i++)
    printf(" %s |     %s\n", PQgetvalue(res, i, 0), PQgetvalue(res, i, 1));
  PQclear(res);

  /* State that the program executed successfully */
  exit_value = EXIT_SUCCESS;
  
  /* Calculate the elapsed time */
  t = clock() - t;
  double time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);

/* Clean up */
cleanup:

 /* Free memory */
  for (i = 0; i < no_ships; i++)
    free(trips[i].trip);

  /* Close the connection to the database and cleanup */
  PQfinish(conn);

  /* Finalize MEOS */
  meos_finalize();

  return exit_value;
}
