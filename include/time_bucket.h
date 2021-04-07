/*
 * This file has been copied from TimescaleDB.
 * This file and its contents are licensed under the Apache License 2.0.
 * Please see the included NOTICE for copyright information and
 * LICENSE-APACHE for a copy of the license.
 */
#ifndef TIME_BUCKET_H
#define TIME_BUCKET_H

#include <postgres.h>
#include <fmgr.h>

extern TimestampTz timestamptz_bucket_internal(TimestampTz timestamp,
  int64 period, TimestampTz origin);

extern Datum timestamptz_bucket(PG_FUNCTION_ARGS);
extern Datum temporal_bucket(PG_FUNCTION_ARGS);

#endif /* TIME_BUCKET_H */

/*****************************************************************************/
