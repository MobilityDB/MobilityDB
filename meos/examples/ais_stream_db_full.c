/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Stream AIS observations into MobilityDB using expandable structures,
 * with real-world hardening for streaming workloads.
 *
 * Production-grade companion to `04_ais_stream_db.c`. The simplified version
 * accumulates instants per ship in expandable sequences and ships them to the
 * database when a per-ship batch fills up; this version adds the production
 * concerns specific to a *streaming* DB target:
 *
 * 1. **Idempotent inserts** — the table has a unique key on `(MMSI)` and the
 *    UPDATE-on-conflict pattern is `ON CONFLICT (MMSI) DO UPDATE SET trip =
 *    public.append_sequence(public.AISTrips.trip, EXCLUDED.trip)`. Re-running
 *    the loader after a partial failure does not duplicate or re-overwrite
 *    instants — the server-side append handles deduplication.
 * 2. **Connection-establishment retry** with exponential back-off, lifted
 *    from `ais_store_full.c`.
 * 3. **Reconnect on lost connection** — if a write fails because the
 *    connection went bad mid-stream, we re-open the connection and retry
 *    the failing batch once before giving up.
 * 4. **Per-row range validation** lifted from `ais_read_full.c`.
 * 5. **Categorised error counts** at end (malformed, out-of-range, dropped
 *    due to ship-table overflow, write retries, write failures).
 *
 * Compile with:
 * @code
 * gcc -Wall -g -I/usr/local/include -I/usr/include/postgresql \
 *   -o ais_stream_db_full ais_stream_db_full.c -L/usr/local/lib -lmeos -lpq
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <libpq-fe.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>

#define NO_INSTS_BATCH       1000
#define NO_INSTS_KEEP        2
#define MAX_LEN_LINE         1024
#define MAX_LEN_TIMESTAMP    32
#define MAX_LEN_SQL          16384
#define MAX_TRIPS            5

#define CONNECT_MAX_ATTEMPTS 5
#define CONNECT_BACKOFF_INIT 1
/* When a write fails and looks transient, retry the failing batch once. */
#define WRITE_RETRY_ATTEMPTS 1

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
  TSequence *trip;
} trip_record;

static PGconn *
connect_with_retry(const char *conninfo)
{
  int backoff = CONNECT_BACKOFF_INIT;
  for (int attempt = 1; attempt <= CONNECT_MAX_ATTEMPTS; attempt++)
  {
    PGconn *c = PQconnectdb(conninfo);
    if (PQstatus(c) == CONNECTION_OK)
      return c;
    fprintf(stderr, "Connect attempt %d failed: %s",
      attempt, PQerrorMessage(c));
    PQfinish(c);
    if (attempt < CONNECT_MAX_ATTEMPTS)
    {
      sleep(backoff);
      backoff *= 2;
    }
  }
  return NULL;
}

static int
exec_sql(PGconn *conn, const char *sql, ExecStatusType expect)
{
  PGresult *res = PQexec(conn, sql);
  int rc = (PQresultStatus(res) == expect) ? 0 : -1;
  if (rc != 0)
    fprintf(stderr, "SQL command failed:\n  %s\n  %s",
      sql, PQerrorMessage(conn));
  PQclear(res);
  return rc;
}

/* Run a single statement; on failure, if the connection looks bad, reconnect
 * and retry once. Returns the (possibly new) connection on success or NULL
 * if the statement could not be made to succeed. */
static PGconn *
exec_with_reconnect(PGconn *conn, const char *conninfo, const char *sql)
{
  for (int attempt = 0; attempt <= WRITE_RETRY_ATTEMPTS; attempt++)
  {
    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) == PGRES_COMMAND_OK)
    {
      PQclear(res);
      return conn;
    }
    fprintf(stderr,
      "Write failed (attempt %d): %s",
      attempt + 1, PQerrorMessage(conn));
    PQclear(res);

    if (PQstatus(conn) == CONNECTION_BAD &&
        attempt < WRITE_RETRY_ATTEMPTS)
    {
      fprintf(stderr, "  reconnecting…\n");
      PQfinish(conn);
      conn = connect_with_retry(conninfo);
      if (! conn)
      {
        fprintf(stderr, "  reconnect failed\n");
        return NULL;
      }
      /* Restore search_path on the new connection. */
      PGresult *r = PQexec(conn,
        "SELECT pg_catalog.set_config('search_path', '', false)");
      PQclear(r);
    }
    else
    {
      return NULL;
    }
  }
  return NULL;
}

static int
validate_record(const AIS_record *rec)
{
  return (rec->Latitude  >= -90.0  && rec->Latitude  <= 90.0  &&
          rec->Longitude >= -180.0 && rec->Longitude <= 180.0 &&
          rec->SOG       >= 0.0    && rec->SOG       <= 102.2)
    ? 0 : -1;
}

int
main(int argc, char **argv)
{
  const char *conninfo = (argc > 1)
    ? argv[1] : "host=localhost dbname=test";

  meos_initialize();
  meos_initialize_timezone("UTC");

  PGconn *conn = connect_with_retry(conninfo);
  if (! conn)
  {
    fprintf(stderr, "Could not establish initial connection; aborting.\n");
    meos_finalize();
    return EXIT_FAILURE;
  }
  if (exec_sql(conn,
        "SELECT pg_catalog.set_config('search_path', '', false)",
        PGRES_TUPLES_OK) < 0)
    goto fail;

  /* Schema: PRIMARY KEY on MMSI lets us use ON CONFLICT DO UPDATE for
   * idempotent server-side appends across loader restarts. */
  if (exec_sql(conn,
        "DROP TABLE IF EXISTS public.AISTrips", PGRES_COMMAND_OK) < 0 ||
      exec_sql(conn,
        "CREATE TABLE public.AISTrips("
          "MMSI integer PRIMARY KEY, trip public.tgeogpoint)",
        PGRES_COMMAND_OK) < 0)
    goto fail;

  FILE *file = fopen("data/ais_instants.csv", "r");
  if (! file)
  {
    fprintf(stderr, "Could not open input file\n");
    goto fail;
  }

  trip_record trips[MAX_TRIPS] = {0};
  long records_ok = 0, records_bad = 0, dropped_overflow = 0, writes = 0;
  long write_retries = 0, write_failures = 0;
  int no_ships = 0;

  char line[MAX_LEN_LINE], ts_buf[MAX_LEN_TIMESTAMP];
  if (fgets(line, sizeof(line), file) == NULL)
  {
    fprintf(stderr, "Empty input file\n");
    fclose(file);
    goto fail;
  }

  printf("Streaming with batch=%d kept=%d, max-trips=%d\n",
    NO_INSTS_BATCH, NO_INSTS_KEEP, MAX_TRIPS);
  clock_t t0 = clock();

  while (fgets(line, sizeof(line), file) != NULL)
  {
    AIS_record rec;
    int read = sscanf(line, "%31[^,],%ld,%lf,%lf,%lf",
      ts_buf, &rec.MMSI, &rec.Latitude, &rec.Longitude, &rec.SOG);
    if (read != 5 || validate_record(&rec) != 0)
    {
      records_bad++;
      continue;
    }
    rec.T = timestamp_in(ts_buf, -1);
    if (rec.T == 0)
    {
      records_bad++;
      continue;
    }
    records_ok++;

    /* Slot lookup. */
    int j = -1;
    for (int i = 0; i < MAX_TRIPS; i++)
    {
      if (trips[i].MMSI == rec.MMSI)
      {
        j = i;
        break;
      }
    }
    if (j < 0)
    {
      if (no_ships >= MAX_TRIPS)
      {
        dropped_overflow++;
        continue;
      }
      j = no_ships++;
      trips[j].MMSI = rec.MMSI;
    }

    /* Flush a full-batch ship to the DB before appending the new instant
     * so the in-memory sequence has room. The DB-side UPSERT keeps the
     * append idempotent across loader restarts. */
    if (trips[j].trip != NULL && trips[j].trip->count >= NO_INSTS_BATCH)
    {
      char *seq_str = tspatial_as_text((Temporal *) trips[j].trip, 6);
      char sql[MAX_LEN_SQL];
      snprintf(sql, sizeof(sql),
        "INSERT INTO public.AISTrips(MMSI, trip) VALUES (%ld, '%s') "
        "ON CONFLICT (MMSI) DO UPDATE "
        "SET trip = public.update(public.AISTrips.trip, EXCLUDED.trip)",
        trips[j].MMSI, seq_str);
      free(seq_str);

      PGconn *new_conn = exec_with_reconnect(conn, conninfo, sql);
      if (! new_conn)
      {
        write_failures++;
        fprintf(stderr,
          "  giving up on this batch for MMSI %ld; continuing with next\n",
          trips[j].MMSI);
      }
      else
      {
        if (new_conn != conn)
        {
          write_retries++;
          conn = new_conn;
        }
        writes++;
      }

      /* In-place restart keeping the last NO_INSTS_KEEP instants so we
       * still have continuity for the next batch. */
      tsequence_restart(trips[j].trip, NO_INSTS_KEEP);
    }

    /* Append the new instant. */
    GSERIALIZED *gs = geogpoint_make2d(4326, rec.Longitude, rec.Latitude);
    TInstant *inst = tpointinst_make(gs, rec.T);
    free(gs);
    if (! trips[j].trip)
      trips[j].trip = tsequence_make_exp(&inst, 1,
        NO_INSTS_BATCH, true, true, LINEAR, false);
    else
      tsequence_append_tinstant(trips[j].trip, inst, 0.0, NULL, true);
    free(inst);
  }

  /* Flush whatever remains for each ship. */
  for (int i = 0; i < no_ships; i++)
  {
    if (trips[i].trip->count == 0)
      continue;
    char *seq_str = tspatial_as_text((Temporal *) trips[i].trip, 6);
    char sql[MAX_LEN_SQL];
    snprintf(sql, sizeof(sql),
      "INSERT INTO public.AISTrips(MMSI, trip) VALUES (%ld, '%s') "
      "ON CONFLICT (MMSI) DO UPDATE "
      "SET trip = public.update(public.AISTrips.trip, EXCLUDED.trip)",
      trips[i].MMSI, seq_str);
    free(seq_str);
    PGconn *new_conn = exec_with_reconnect(conn, conninfo, sql);
    if (! new_conn)
      write_failures++;
    else
    {
      if (new_conn != conn) { write_retries++; conn = new_conn; }
      writes++;
    }
    free(trips[i].trip);
  }
  fclose(file);

  double elapsed = ((double)(clock() - t0)) / CLOCKS_PER_SEC;

  printf("\nStream summary:\n");
  printf("  records OK:                %ld\n", records_ok);
  printf("  records bad:               %ld\n", records_bad);
  printf("  dropped (overflow):        %ld\n", dropped_overflow);
  printf("  DB writes successful:      %ld\n", writes);
  printf("  DB write retries:          %ld\n", write_retries);
  printf("  DB write failures:         %ld\n", write_failures);
  printf("  elapsed:                   %.2fs\n", elapsed);

  PQfinish(conn);
  meos_finalize();
  return (write_failures == 0 && records_ok > 0)
    ? EXIT_SUCCESS : EXIT_FAILURE;

fail:
  PQfinish(conn);
  meos_finalize();
  return EXIT_FAILURE;
}
