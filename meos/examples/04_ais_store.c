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
 * @brief A simple program that reads AIS data from a CSV file, converts them
 * into temporal values, and stores them in MobilityDB.
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
 * gcc -Wall -g -I/usr/local/include -I/usr/include/postgresql -o 04_ais_store 04_ais_store.c -L/usr/local/lib -lmeos -lpq
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <libpq-fe.h>
#include <meos.h>
#include <meos_geo.h>

/* Number of instants in a batch for printing a marker */
#define NO_INSTANTS_BATCH 10000
/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LENGTH_HEADER 1024
/* Maximum length for a SQL output query */
#define MAX_LENGTH_SQL 16384
/* Number of inserts that are sent in bulk */
#define NO_BULK_INSERT 20

typedef struct
{
  Timestamp T;
  long int MMSI;
  double Latitude;
  double Longitude;
  double SOG;
} AIS_record;

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
  /* Maximum length in characters of the string for the bulk insert */
  char insert_buffer[MAX_LENGTH_SQL];
  /* Exit value initialized to failure to quickly exit upon error */
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
    PQfinish(conn);
    return EXIT_FAILURE;
  }

  /* Set always-secure search path, so malicious users can't take control. */
  res_sql = exec_sql(conn, "SELECT pg_catalog.set_config('search_path', '', false)",
    PGRES_TUPLES_OK);
  if (res_sql < 0)
  {
    fprintf(stderr, "%s", PQerrorMessage(conn));
    PQfinish(conn);
    return EXIT_FAILURE;
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
   * Section 3: Read input file line by line and save each observation as a
   * temporal point in MobilityDB
   ***************************************************************************/

  /* Create the table that will hold the data */
  printf("Creating the table in the database\n");
  res_sql = exec_sql(conn, "DROP TABLE IF EXISTS public.AISInstants;",
    PGRES_COMMAND_OK);
  if (res_sql < 0)
  {
    fclose(file);
    goto cleanup;
  }
  res_sql = exec_sql(conn, "CREATE TABLE public.AISInstants(MMSI integer, "
    "location public.tgeogpoint, SOG public.tfloat);", PGRES_COMMAND_OK);
  if (res_sql < 0)
  {
    fclose(file);
    goto cleanup;
  }

  /* Start a transaction block */
  res_sql = exec_sql(conn, "BEGIN", PGRES_COMMAND_OK);
  if (res_sql < 0)
  {
    fclose(file);
    goto cleanup;
  }

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", text_buffer);

  printf("Reading the instants (one '*' marker every %d instants)\n",
    NO_INSTANTS_BATCH);

  /* Continue reading the file */
  int len;
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
    if (no_records % NO_INSTANTS_BATCH == 0)
    {
      printf("*");
      fflush(stdout);
    }

    /* Transform the string representing the timestamp into a timestamp value */
    rec.T = timestamp_in(text_buffer, -1);

    /* Create the INSERT command with the values read */
    if ((no_records - 1) % NO_BULK_INSERT == 0)
      len = snprintf(insert_buffer, MAX_LENGTH_SQL - 1,
        "INSERT INTO public.AISInstants(MMSI, location, SOG) VALUES ");

    char *t_out = timestamp_out(rec.T);
    len += snprintf(insert_buffer + len, MAX_LENGTH_SQL - 1 - len,
      "(%ld, 'SRID=4326;Point(%lf %lf)@%s+00', '%lf@%s+00'),",
      rec.MMSI, rec.Longitude, rec.Latitude, t_out, rec.SOG, t_out);
    free(t_out);

    if ((no_records - 1) % NO_BULK_INSERT == NO_BULK_INSERT - 1)
    {
      /* Replace the last comma with a semicolon */
      insert_buffer[len - 1] = ';';
      res_sql = exec_sql(conn, insert_buffer, PGRES_COMMAND_OK);
      if (res_sql < 0)
      {
        fclose(file);
        goto cleanup;
      }
      len = 0;
    }
  } while (!feof(file));

  /* Close the file */
  fclose(file);

  if (len > 0)
  {
    /* Replace the last comma with a semicolon */
    insert_buffer[len - 1] = ';';
    res_sql = exec_sql(conn, insert_buffer, PGRES_COMMAND_OK);
    if (res_sql < 0)
      goto cleanup;
  }

  printf("\n%d records read.\n%d incomplete records ignored.\n",
    no_records, no_nulls);

  snprintf(text_buffer, MAX_LENGTH_SQL - 1, 
    "SELECT COUNT(*) FROM public.AISInstants;");
  PGresult *res = PQexec(conn, text_buffer);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    fprintf(stderr, "SQL command failed:\n%s %s", text_buffer,
      PQerrorMessage(conn));
    PQclear(res);
    goto cleanup;
  }

  /* End the transaction */
  exec_sql(conn, "END", PGRES_COMMAND_OK);

  printf("Query 'SELECT COUNT(*) FROM public.AISInstants' returned %s\n",
    PQgetvalue(res, 0, 0));
  PQclear(res);

  /* State that the program executed successfully */
  exit_value = EXIT_SUCCESS;

  /* Calculate the elapsed time */
  t = clock() - t;
  double time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);

/* Clean up */
cleanup:

  /* Close the connection to the database and cleanup */
  PQfinish(conn);

  /* Finalize MEOS */
  meos_finalize();

  return exit_value;
}
