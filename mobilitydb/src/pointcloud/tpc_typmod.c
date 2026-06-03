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
 *****************************************************************************/

/**
 * @file
 * @brief Typmod plumbing for tpcpoint and tpcpatch — column-level pcid
 *   pinning. Mirrors the tgeompoint(Point, SRID) typmod pattern in
 *   mobilitydb/src/geo/tgeo.c, but the only constrainable field is the
 *   pcid (positive int32). Typmod -1 means unconstrained; a non-negative
 *   value is the required pcid for every value stored in the column.
 *
 * @code
 *   CREATE TABLE scans (id int, traj tpcpoint(1));   -- pinned to pcid 1
 *   INSERT INTO scans VALUES (1, tpcpoint(pcpoint(2, ...), '...'));
 *   -- ERROR: tpcpoint pcid 2 does not match column typmod pcid 1
 * @endcode
 */

#include <postgres.h>
#include <fmgr.h>
#include <stdlib.h>              /* strtol */
#include <errno.h>
#include <utils/array.h>
#include <catalog/pg_type.h>     /* CSTRINGOID */
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif

/* meos_internal.h pulls in <json-c/json.h>, whose `struct json_object`
 * collides with PG's `Datum json_object(PG_FUNCTION_ARGS)` in
 * utils/fmgrprotos.h (transitively via utils/builtins.h). We only need
 * MEOS public API + a hand-rolled strtoint, so we skip both. */
#include <meos.h>
#include <meos_pointcloud.h>     /* pcpoint_get_pcid, pcpatch_get_pcid */
#include "temporal/temporal.h"
#include "pointcloud/pcpoint.h"
#include "pointcloud/pcpatch.h"

#define TPC_TYPMOD_UNCONSTRAINED  (-1)
#define TPC_MAX_TYPMOD_LEN         32  /* "(2147483647)" + slack */

/*****************************************************************************
 * typmod_in
 *
 * Accepts a single-element cstring[] containing the pcid as a decimal
 * integer. Anything else is an error. The shape is symmetrical with
 * pgPointCloud's `pcpoint(pcid)` typmod on the static type.
 *****************************************************************************/

static int32
tpc_typmod_in_array(ArrayType *arr)
{
  Datum *elem_values;
  int n = 0;

  if (ARR_ELEMTYPE(arr) != CSTRINGOID)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_ELEMENT_ERROR),
      errmsg("typmod array must be type cstring[]")));
  if (ARR_NDIM(arr) != 1)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
      errmsg("typmod array must be one-dimensional")));
  if (ARR_HASNULL(arr))
    ereport(ERROR, (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
      errmsg("typmod array must not contain nulls")));

  deconstruct_array(arr, CSTRINGOID, -2, false, 'c', &elem_values, NULL, &n);
  if (n != 1)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Invalid pcid typmod: expected one integer, got %d arguments", n)));

  char *s = DatumGetCString(elem_values[0]);
  if (s == NULL || *s == '\0')
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Empty pcid typmod")));

  /* Hand-rolled int32 parse — we can't include utils/builtins.h here
   * (json-c collision), so don't use pg_strtoint32 / pg_atoi. */
  errno = 0;
  char *end = NULL;
  long val = strtol(s, &end, 10);
  if (errno != 0 || end == s || *end != '\0' ||
      val < INT32_MIN || val > INT32_MAX)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Invalid pcid typmod: %s (must be a positive integer)", s)));
  int32 pcid = (int32) val;
  if (pcid <= 0)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Invalid pcid typmod: %d (must be a positive integer)", pcid)));
  pfree(elem_values);
  return pcid;
}

PGDLLEXPORT Datum Tpc_typmod_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpc_typmod_in);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Input typmod for tpcpoint / tpcpatch (pcid as a single int)
 * @sqlfn tpc_typmod_in()
 */
Datum
Tpc_typmod_in(PG_FUNCTION_ARGS)
{
  ArrayType *arr = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
  PG_RETURN_INT32(tpc_typmod_in_array(arr));
}

/*****************************************************************************
 * typmod_out
 *****************************************************************************/

PGDLLEXPORT Datum Tpc_typmod_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpc_typmod_out);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Output typmod for tpcpoint / tpcpatch as `(pcid)`. Returns an
 *   empty string when typmod is -1 (unconstrained).
 * @sqlfn tpc_typmod_out()
 */
Datum
Tpc_typmod_out(PG_FUNCTION_ARGS)
{
  int32 typmod = PG_GETARG_INT32(0);
  char *s = palloc(TPC_MAX_TYPMOD_LEN);
  if (typmod == TPC_TYPMOD_UNCONSTRAINED)
    s[0] = '\0';
  else
    snprintf(s, TPC_MAX_TYPMOD_LEN, "(%d)", typmod);
  PG_RETURN_CSTRING(s);
}

/*****************************************************************************
 * enforce_typmod — runtime check on cast/INSERT
 *****************************************************************************/

/**
 * @brief Read the pcid from a temporal pcpoint/pcpatch's first instant.
 *   All instants share the same pcid by construction, so the first
 *   instant is authoritative.
 */
static uint32_t
tpc_pcid(const Temporal *temp)
{
  Datum first = temporal_start_value(temp);
  if (temp->temptype == T_TPCPOINT)
    return pcpoint_get_pcid((const Pcpoint *) DatumGetPointer(first));
  return pcpatch_get_pcid((const Pcpatch *) DatumGetPointer(first));
}

PGDLLEXPORT Datum Tpc_enforce_typmod(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpc_enforce_typmod);
/**
 * @ingroup mobilitydb_pointcloud_temp
 * @brief Enforce the typmod's pcid pinning on a tpcpoint/tpcpatch value
 * @details Called by PG as the cast function bound to
 *   `CREATE CAST (tpcpoint AS tpcpoint) WITH FUNCTION tpcpoint(tpcpoint, integer)`.
 *   Returns the value unchanged if the column is unconstrained or the
 *   pcid matches; raises if the pcid disagrees.
 * @sqlfn tpcpoint(), tpcpatch()
 */
Datum
Tpc_enforce_typmod(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 typmod = PG_GETARG_INT32(1);
  if (typmod == TPC_TYPMOD_UNCONSTRAINED)
    PG_RETURN_TEMPORAL_P(temp);
  uint32_t pcid = tpc_pcid(temp);
  if ((int32) pcid != typmod)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Pcid of %s value (%u) does not match column typmod pcid (%d)",
        (temp->temptype == T_TPCPOINT) ? "tpcpoint" : "tpcpatch",
        pcid, typmod)));
  PG_RETURN_TEMPORAL_P(temp);
}

/*****************************************************************************/
