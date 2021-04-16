/*
 * This file has been copied from TimescaleDB.
 * This file and its contents are licensed under the Apache License 2.0.
 * Please see the included NOTICE for copyright information and
 * LICENSE-APACHE for a copy of the license.
 */
#ifndef BUCKET_H
#define BUCKET_H

#include <postgres.h>
#include <fmgr.h>

extern double float_bucket(double value, double width, double origin);
extern int64 get_interval_period_timestamp_units(Interval *interval);

extern Datum timestamptz_bucket(PG_FUNCTION_ARGS);
extern Datum temporal_time_bucket(PG_FUNCTION_ARGS);

#endif /* BUCKET_H */

/*****************************************************************************/
