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
 * Please read the assumptions made about the input file `aisinput.csv` in the
 * file `meos_read_ais.c` in the same directory.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -I/usr/include/postgresql -o meos_store_ais meos_store_ais.c -L/usr/local/lib -lmeos -lpq
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

#include "meos.h"

#define NO_BULK_INSERT 20

typedef struct
{
  Timestamp T;
  long int MMSI;
  double Latitude;
  double Longitude;
  double SOG;
} AIS_record;

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

int
main(int argc, char **argv)
{
  const char *conninfo;
  PGconn *conn;
  AIS_record rec;
  int records = 0;
  int nulls = 0;
  /* Maximum length in characters of a line in the input data */
  char buffer[1024];
  /* Maximum length in characters of the string for the bulk insert */
  char insert_buffer[16384];

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

  /***************************************************************************
   * Section 2: Initialize MEOS and open the input AIS file
   ***************************************************************************/

  /* Initialize MEOS */
  meos_initialize();

  /* You may substitute the full file path in the first argument of fopen */
  FILE *file = fopen("aisinput.csv", "r");

  if (! file)
  {
    printf("Error opening file\n");
    return 1;
  }

  /***************************************************************************
   * Section 3: Read input file line by line and save each observation as a
   * temporal point in MobilityDB
   ***************************************************************************/

  /* Create the table that will hold the data */
  printf("Creating the table in the database\n");
  exec_sql(conn, "DROP TABLE IF EXISTS public.MEOS_demo;", PGRES_COMMAND_OK);
  exec_sql(conn, "CREATE TABLE public.MEOS_demo(MMSI integer, "
    "location public.tgeogpoint, SOG public.tfloat);", PGRES_COMMAND_OK);

  /* Start a transaction block */
  exec_sql(conn, "BEGIN", PGRES_COMMAND_OK);

  printf("Start processing the file\n");

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", buffer);

  /* Continue reading the file */
  int len;
  do
  {
    int read = fscanf(file, "%32[^,],%ld,%lf,%lf,%lf\n",
      buffer, &rec.MMSI, &rec.Latitude, &rec.Longitude, &rec.SOG);
    /* Transform the string representing the timestamp into a timestamp value */
    rec.T = pg_timestamp_in(buffer, -1);

    if (read == 5)
      records++;

    if (read != 5 && ! feof(file))
    {
      printf("Record with missing values ignored\n");
      nulls++;
    }

    if (ferror(file))
    {
      printf("Error reading file\n");
      fclose(file);
      exit_nicely(conn);
    }

    /* Create the INSERT command with the values read */
    if ((records - 1) % NO_BULK_INSERT == 0)
      len = sprintf(insert_buffer,
        "INSERT INTO public.MEOS_demo(MMSI, location, SOG) VALUES ");

    char *t_out = pg_timestamp_out(rec.T);
    len += sprintf(insert_buffer + len,
      "(%ld, 'SRID=4326;Point(%lf %lf)@%s+00', '%lf@%s+00'),",
      rec.MMSI, rec.Longitude, rec.Latitude, t_out, rec.SOG, t_out);
    free(t_out);

    if ((records - 1) % NO_BULK_INSERT == NO_BULK_INSERT - 1)
    {
      /* Replace the last comma with a semicolon */
      insert_buffer[len - 1] = ';';
      exec_sql(conn, insert_buffer, PGRES_COMMAND_OK);
      len = 0;
    }
  } while (!feof(file));

  if (len > 0)
  {
    /* Replace the last comma with a semicolon */
    insert_buffer[len - 1] = ';';
    exec_sql(conn, insert_buffer, PGRES_COMMAND_OK);
  }

  printf("%d records read.\n%d incomplete records ignored.\n",
    records, nulls);

  sprintf(buffer, "SELECT COUNT(*) FROM public.MEOS_demo;");
  PGresult *res = PQexec(conn, buffer);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    fprintf(stderr, "SQL command failed:\n%s %s", buffer, PQerrorMessage(conn));
    PQclear(res);
    exit_nicely(conn);
  }

  printf("Query 'SELECT COUNT(*) FROM public.MEOS_demo' returned %s\n",
    PQgetvalue(res, 0, 0));

  /* End the transaction */
  exec_sql(conn, "END", PGRES_COMMAND_OK);

  /* Close the file */
  fclose(file);

  /* Finalize MEOS */
  meos_finish();

  /* Close the connection to the database and cleanup */
  PQfinish(conn);

  return 0;
}
