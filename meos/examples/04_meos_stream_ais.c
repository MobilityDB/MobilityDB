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
 * observations in main memory and send the temporal values to a MobilityDB
 * database when they reach a given number of instants in order to free
 * the memory and ingest the newest observations.
 *
 * This program is similar to `04_meos_store_ais` but illustrates the use of
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
 * Please read the assumptions made about the input file `aisinput.csv` in the
 * file `02_meos_read_ais.c` in the same directory.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -I/usr/include/postgresql -o 04_meos_stream_ais 04_meos_stream_ais.c -L/usr/local/lib -lmeos -lpq
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include <meos.h>
#include <meos_internal.h>

/* Number of instants to send in batch to the database  */
#define NO_INSTANTS_BATCH 500
/* Number of instants to keep when restarting a sequence, should keep at least one */
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
  TSequence *trip; /* Latest observations of the trip */
} trip_record;

static void
exit_nicely(PGconn *conn)
{
  PQfinish(conn);
  exit(1);
}

static void
exec_sql(PGconn *conn, const char *sql, ExecStatusType status)
{
  PGresult *res = PQexec(conn, sql);
  if (PQresultStatus(res) != status)
  {
    fprintf(stderr, "SQL command failed:\n%s %s", sql, PQerrorMessage(conn));
    PQclear(res);
    exit_nicely(conn);
  }
  PQclear(res);
}

/* Send the trip to the database when its size is greater than the max size */
int
windowManager(trip_record *trips, int ship, int size, PGconn *conn)
{
  if (trips[ship].trip && trips[ship].trip->count == NO_INSTANTS_BATCH)
  {

    char *temp_out = tsequence_out(trips[ship].trip, 15);
    //printf("%d\n",trips[ship].trip->count);

    char *query_buffer = malloc(sizeof(char) * (strlen(temp_out) + 256));
    /* Send to the database the trip if reached the maximum number of instants */
    sprintf(query_buffer, "INSERT INTO public.AISTrips(MMSI, trip) "
      "VALUES (%ld, '%s') ON CONFLICT (MMSI) DO "
      "UPDATE SET trip = public.update(AISTrips.trip, EXCLUDED.trip, true);",
        trips[ship].MMSI, temp_out);

    PGresult *res = PQexec(conn, query_buffer);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
      fprintf(stderr, "SQL command failed:\n%s %s", query_buffer,
      PQerrorMessage(conn));
      PQclear(res);
      exit_nicely(conn);
    }

    /* Free memory */
    free(temp_out);
    free(query_buffer);
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
  const char *conninfo;
  PGconn *conn;
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
    exit_nicely(conn);
  }

  /* Set always-secure search path, so malicious users can't take control. */
  exec_sql(conn, "SELECT pg_catalog.set_config('search_path', '', false)",
    PGRES_TUPLES_OK);

  /* Create the table that will hold the data */
  printf("Creating the table AISTrips in the database\n");
  exec_sql(conn, "DROP TABLE IF EXISTS public.AISTrips;", PGRES_COMMAND_OK);
  exec_sql(conn, "CREATE TABLE public.AISTrips(MMSI integer PRIMARY KEY, "
    "trip public.tgeogpoint);", PGRES_COMMAND_OK);

  /***************************************************************************
   * Section 2: Initialize MEOS and open the input AIS file
   ***************************************************************************/

  /* Initialize MEOS */
  meos_initialize(NULL);

  /* You may substitute the full file path in the first argument of fopen */
  FILE *file = fopen("aisinput.csv", "r");

  if (! file)
  {
    printf("Error opening file\n");
    return_value = 1;
    goto cleanup;
  }

  /***************************************************************************
   * Section 3: Read input file line by line and append each observation as a
   * temporal point in MEOS
   ***************************************************************************/

  printf("Accumulating %d instants before sending them to the database\n"
    "(one marker every database update)\n", NO_INSTANTS_BATCH);

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", text_buffer);

  /* Continue reading the file */
  do
  {
    int read = fscanf(file, "%32[^,],%ld,%lf,%lf,%lf\n",
      text_buffer, &rec.MMSI, &rec.Latitude, &rec.Longitude, &rec.SOG);
    /* Transform the string representing the timestamp into a timestamp value */
    rec.T = pg_timestamp_in(text_buffer, -1);

    if (read == 5)
      no_records++;

    if (read != 5 && ! feof(file))
    {
      printf("Record with missing values ignored\n");
      no_nulls++;
    }

    if (ferror(file))
    {
      printf("Error reading file\n");
      fclose(file);
      exit_nicely(conn);
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
     * Append the latest observation to the corresponding ship.
     * In the input file it is assumed that
     * - The coordinates are given in the WGS84 geographic coordinate system
     * - The timestamps are given in GMT time zone
     */
    char *t_out = pg_timestamp_out(rec.T);
    sprintf(point_buffer, "SRID=4326;Point(%lf %lf)@%s+00", rec.Longitude,
      rec.Latitude, t_out);

    /* Send the trip to the database when its size reaches the maximum size */
    windowManager(trips, ship, NO_INSTANTS_BATCH, conn);

    /* Append the last observation */
    TInstant *inst = (TInstant *) tgeogpoint_in(point_buffer);
    if (! trips[ship].trip)
      trips[ship].trip = tsequence_make_exp((const TInstant **) &inst, 1,
        NO_INSTANTS_BATCH, true, true, LINEAR, false);
    else
      tsequence_append_tinstant(trips[ship].trip, inst, 0.0, NULL, true);
  } while (!feof(file));

  printf("\n%d records read.\n%d incomplete records ignored. %d writes to the database\n",
    no_records, no_nulls,count);

  sprintf(text_buffer, "SELECT MMSI, public.numInstants(trip) FROM public.AISTrips;");
  PGresult *res = PQexec(conn, text_buffer);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    fprintf(stderr, "SQL command failed:\n%s %s", text_buffer, PQerrorMessage(conn));
    PQclear(res);
    exit_nicely(conn);
  }

  int numrows = PQntuples(res);
  printf("Result of the query '%s'\n\n", text_buffer);
  printf("   mmsi    | numinstants\n");
  printf("-----------+-------------\n");
  for (i = 0; i < numrows; i++)
    printf(" %s |     %s\n", PQgetvalue(res, i, 0), PQgetvalue(res, i, 1));

  /* Close the file */
  fclose(file);

/* Clean up */
cleanup:

 /* Free memory */
  for (i = 0; i < numships; i++)
    free(trips[i].trip);

  /* Finalize MEOS */
  meos_finalize();

  /* Close the connection to the database and cleanup */
  PQfinish(conn);

  return return_value;
}
