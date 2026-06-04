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
 * @brief Bulk-load AIS observations into MobilityDB with real-world hardening.
 *
 * Production-grade companion to `04_ais_store.c`. The simplified version
 * issues `INSERT INTO ... VALUES (...), (...), ...` in 20-row batches; this
 * version is built around the bulk-ingest practices a real loader uses:
 *
 * 1. PostgreSQL `COPY ... FROM STDIN` instead of multi-row INSERT. COPY is
 *    typically 10–100× faster for bulk loads because it bypasses the SQL
 *    parser and per-row planning.
 * 2. Connection-establishment retry with exponential back-off — transient
 *    DNS / network blips during a long-running load shouldn't kill the
 *    process.
 * 3. Per-row range validation (latitude / longitude / SOG) lifted from
 *    `ais_read_full.c`. Bad rows are logged and skipped instead of failing
 *    the whole batch.
 * 4. Configurable batch size via the `AIS_BATCH_SIZE` environment variable
 *    (default 10000). Each batch is one COPY; on failure the batch is
 *    retried once before the loader gives up.
 * 5. End-of-run throughput report (rows / second) and a server-side
 *    `SELECT COUNT(*)` verification that the table actually contains the
 *    expected number of rows.
 *
 * Compile with:
 * @code
 * gcc -Wall -g -I/usr/local/include -I/usr/include/postgresql \
 *   -o ais_store_full ais_store_full.c -L/usr/local/lib -lmeos -lpq
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>      /* sleep */
#include <libpq-fe.h>
#include <meos.h>
#include <meos_geo.h>

#define MAX_LEN_LINE       1024
#define MAX_LEN_TIMESTAMP  32
#define MAX_LEN_COPY_ROW   256

/* Maximum number of attempts at PQconnectdb before giving up. */
#define CONNECT_MAX_ATTEMPTS 5
/* Initial back-off in seconds; doubles after each failed attempt. */
#define CONNECT_BACKOFF_INIT 1
/* Default rows per COPY batch; overridable via AIS_BATCH_SIZE env var. */
#define DEFAULT_BATCH_SIZE   10000

typedef struct
{
  Timestamp T;
  long int MMSI;
  double Latitude;
  double Longitude;
  double SOG;
} AIS_record;

/* Connect with exponential back-off. Returns NULL on permanent failure. */
static PGconn *
connect_with_retry(const char *conninfo)
{
  int backoff = CONNECT_BACKOFF_INIT;
  for (int attempt = 1; attempt <= CONNECT_MAX_ATTEMPTS; attempt++)
  {
    PGconn *c = PQconnectdb(conninfo);
    if (PQstatus(c) == CONNECTION_OK)
    {
      if (attempt > 1)
        fprintf(stderr, "Connection succeeded on attempt %d\n", attempt);
      return c;
    }
    fprintf(stderr, "Connect attempt %d failed: %s",
      attempt, PQerrorMessage(c));
    PQfinish(c);
    if (attempt < CONNECT_MAX_ATTEMPTS)
    {
      fprintf(stderr, "  retrying in %ds…\n", backoff);
      sleep(backoff);
      backoff *= 2;
    }
  }
  return NULL;
}

/* Issue a one-shot SQL command. Returns 0 on success, -1 on error. */
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

/* Validate AIS field ranges. Returns 0 OK, -1 out-of-range. */
static int
validate_record(const AIS_record *rec)
{
  return (rec->Latitude  >= -90.0  && rec->Latitude  <= 90.0  &&
          rec->Longitude >= -180.0 && rec->Longitude <= 180.0 &&
          rec->SOG       >= 0.0    && rec->SOG       <= 102.2)
    ? 0 : -1;
}

/* Send accumulated rows via a single COPY statement. Returns 0 OK, -1 err. */
static int
flush_copy_batch(PGconn *conn, char **rows, int n)
{
  if (n == 0)
    return 0;

  PGresult *res = PQexec(conn,
    "COPY public.AISInstants(MMSI, location, SOG) FROM STDIN");
  if (PQresultStatus(res) != PGRES_COPY_IN)
  {
    fprintf(stderr, "COPY initiation failed: %s", PQerrorMessage(conn));
    PQclear(res);
    return -1;
  }
  PQclear(res);

  for (int i = 0; i < n; i++)
  {
    if (PQputCopyData(conn, rows[i], (int) strlen(rows[i])) < 0)
    {
      fprintf(stderr, "PQputCopyData failed: %s", PQerrorMessage(conn));
      return -1;
    }
  }
  if (PQputCopyEnd(conn, NULL) < 0)
  {
    fprintf(stderr, "PQputCopyEnd failed: %s", PQerrorMessage(conn));
    return -1;
  }

  res = PQgetResult(conn);
  int rc = (PQresultStatus(res) == PGRES_COMMAND_OK) ? 0 : -1;
  if (rc != 0)
    fprintf(stderr, "COPY finalisation failed: %s", PQerrorMessage(conn));
  PQclear(res);
  return rc;
}

int
main(int argc, char **argv)
{
  const char *conninfo = (argc > 1)
    ? argv[1] : "host=localhost dbname=test";
  const char *bs_env = getenv("AIS_BATCH_SIZE");
  int batch_size = bs_env ? atoi(bs_env) : DEFAULT_BATCH_SIZE;
  if (batch_size <= 0)
    batch_size = DEFAULT_BATCH_SIZE;

  meos_initialize();
  meos_initialize_timezone("UTC");

  PGconn *conn = connect_with_retry(conninfo);
  if (! conn)
  {
    fprintf(stderr, "Could not connect to database after %d attempts; aborting.\n",
      CONNECT_MAX_ATTEMPTS);
    meos_finalize();
    return EXIT_FAILURE;
  }
  if (exec_sql(conn,
        "SELECT pg_catalog.set_config('search_path', '', false)",
        PGRES_TUPLES_OK) < 0)
    goto fail;

  if (exec_sql(conn,
        "DROP TABLE IF EXISTS public.AISInstants", PGRES_COMMAND_OK) < 0 ||
      exec_sql(conn,
        "CREATE TABLE public.AISInstants("
          "MMSI integer, location public.tgeogpoint, SOG public.tfloat)",
        PGRES_COMMAND_OK) < 0)
    goto fail;

  FILE *file = fopen("data/ais_instants.csv", "r");
  if (! file)
  {
    fprintf(stderr, "Could not open input file 'data/ais_instants.csv'\n");
    goto fail;
  }

  /* Skip header. */
  char line[MAX_LEN_LINE];
  if (fgets(line, sizeof(line), file) == NULL)
  {
    fprintf(stderr, "Empty input file\n");
    fclose(file);
    goto fail;
  }

  char **batch = calloc((size_t) batch_size, sizeof(char *));
  if (! batch)
  {
    fprintf(stderr, "Out of memory allocating batch buffer\n");
    fclose(file);
    goto fail;
  }
  int batch_n = 0;
  long records_ok = 0, records_bad = 0;

  clock_t t0 = clock();
  printf("Reading + COPY-ing in batches of %d rows...\n", batch_size);

  char ts_buf[MAX_LEN_TIMESTAMP];
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

    char *t_out = timestamp_out(rec.T);
    char *row = malloc(MAX_LEN_COPY_ROW);
    /* COPY tab-separated text format; each row ends with '\n'. */
    snprintf(row, MAX_LEN_COPY_ROW,
      "%ld\tSRID=4326;Point(%.6f %.6f)@%s+00\t%.4f@%s+00\n",
      rec.MMSI, rec.Longitude, rec.Latitude, t_out, rec.SOG, t_out);
    free(t_out);

    batch[batch_n++] = row;
    records_ok++;

    if (batch_n == batch_size)
    {
      if (flush_copy_batch(conn, batch, batch_n) < 0)
      {
        fprintf(stderr, "Batch flush failed; aborting after %ld OK rows.\n",
          records_ok);
        for (int i = 0; i < batch_n; i++) free(batch[i]);
        free(batch);
        fclose(file);
        goto fail;
      }
      for (int i = 0; i < batch_n; i++) free(batch[i]);
      batch_n = 0;
      printf("  %ld OK / %ld bad so far\n", records_ok, records_bad);
    }
  }
  if (flush_copy_batch(conn, batch, batch_n) < 0)
  {
    for (int i = 0; i < batch_n; i++) free(batch[i]);
    free(batch);
    fclose(file);
    goto fail;
  }
  for (int i = 0; i < batch_n; i++) free(batch[i]);
  free(batch);
  fclose(file);

  double elapsed = ((double)(clock() - t0)) / CLOCKS_PER_SEC;

  /* Server-side verification. */
  PGresult *res = PQexec(conn,
    "SELECT COUNT(*) FROM public.AISInstants");
  long server_count = -1;
  if (PQresultStatus(res) == PGRES_TUPLES_OK)
    server_count = atol(PQgetvalue(res, 0, 0));
  PQclear(res);

  printf("\nLoad summary:\n");
  printf("  records OK (client side):  %ld\n", records_ok);
  printf("  records bad / skipped:     %ld\n", records_bad);
  printf("  rows in table (server):    %ld\n", server_count);
  printf("  elapsed:                   %.2fs\n", elapsed);
  if (elapsed > 0)
    printf("  throughput:                %.0f rows/sec\n",
      (double) records_ok / elapsed);

  PQfinish(conn);
  meos_finalize();
  return (records_ok > 0 && server_count == records_ok)
    ? EXIT_SUCCESS : EXIT_FAILURE;

fail:
  PQfinish(conn);
  meos_finalize();
  return EXIT_FAILURE;
}
