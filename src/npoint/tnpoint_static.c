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
 * @file tnpoint_static.c
 * Network-based static point and segment types
 *
 * Several functions are commented out since they are not currently used.
 * They are kept if needed in the future.
 */

#include "npoint/tnpoint_static.h"

#include <assert.h>
#include <libpq/pqformat.h>
#include <executor/spi.h>
#include <liblwgeom.h>

#include "general/temporaltypes.h"
#include "general/tempcache.h"
#include "general/temporal_util.h"
#include "general/tnumber_mathfuncs.h"

#include "point/tpoint_out.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_parser.h"

/** Buffer size for input and output of npoint values */
#define MAXNPOINTLEN    128

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

/**
 * Convert a C array of int64 values into a PostgreSQL array
 */
ArrayType *
int64arr_to_array(const int64 *int64arr, int count)
{
  return construct_array((Datum *)int64arr, count, INT8OID, 8, true, 'd');
}

/*
ArrayType *
npointarr_to_array(npoint **npointarr, int count)
{
  return construct_array((Datum *)npointarr, count, type_oid(T_NPOINT),
    sizeof(npoint), false, 'd');
}
*/

/**
 * Convert a C array of network segment values into a PostgreSQL array
 */
ArrayType *
nsegmentarr_to_array(nsegment **nsegmentarr, int count)
{
  return construct_array((Datum *)nsegmentarr, count, type_oid(T_NSEGMENT),
    sizeof(nsegment), false, 'd');
}

/**
 * Return the SRID of the routes in the ways table
 */
int32_t
get_srid_ways()
{
  int32_t srid_ways;
  bool isNull = true;
  SPI_connect();
  int ret = SPI_execute("SELECT ST_SRID(the_geom) FROM public.ways LIMIT 1;", true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable != NULL)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    Datum value = SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 1, &isNull);
    if (isNull)
      elog(ERROR, "Cannot determine SRID of the ways table");

    srid_ways = DatumGetInt32(value);
  }
  else
    elog(ERROR, "Cannot determine SRID of the ways table");

  SPI_finish();
  return srid_ways;
}

/*****************************************************************************/

/**
 * Convert a network point array into a geometry
 */
Datum
npointarr_to_geom_internal(npoint **points, int count)
{
  Datum *geoms = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; i++)
  {
    Datum line = route_geom(points[i]->rid);
    geoms[i] = call_function2(LWGEOM_line_interpolate_point, line,
      Float8GetDatum(points[i]->pos));
    pfree(DatumGetPointer(line));
  }
  Datum result;
  if (count == 1)
    result = geoms[0];
  else
  {
    ArrayType *array = datumarr_to_array(geoms, count, type_oid(T_GEOMETRY));
    result = call_function1(pgis_union_geometry_array, PointerGetDatum(array));
    pfree(array);
    for (int i = 0; i < count; i++)
      pfree(DatumGetPointer(geoms[i]));
    pfree(geoms);
  }
  PG_RETURN_DATUM(result);
}

/**
 * Convert a network segment array into a geometry
 */
Datum
nsegmentarr_to_geom_internal(nsegment **segments, int count)
{
  Datum *geoms = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; i++)
  {
    Datum line = route_geom(segments[i]->rid);
    if (segments[i]->pos1 == 0 && segments[i]->pos2 == 1)
      geoms[i] = PointerGetDatum(gserialized_copy((GSERIALIZED *) PG_DETOAST_DATUM(line)));
    else if (segments[i]->pos1 == segments[i]->pos2)
      geoms[i] = call_function2(LWGEOM_line_interpolate_point, line, Float8GetDatum(segments[i]->pos1));
    else
      geoms[i] = call_function3(LWGEOM_line_substring, line,
        Float8GetDatum(segments[i]->pos1), Float8GetDatum(segments[i]->pos2));
    pfree(DatumGetPointer(line));
  }
  Datum result;
  if (count == 1)
    result = geoms[0];
  else
  {
    ArrayType *array = datumarr_to_array(geoms, count, type_oid(T_GEOMETRY));
    result = call_function1(pgis_union_geometry_array, PointerGetDatum(array));
    pfree(array);
    for (int i = 0; i < count; i++)
      pfree(DatumGetPointer(geoms[i]));
    pfree(geoms);
  }
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/

/* Comparator functions */
/*
static int
npoint_sort_cmp(npoint **l, npoint **r)
{
  return npoint_cmp_internal(*l, *r);
}

void
npointarr_sort(npoint **points, int count)
{
  qsort(points, (size_t) count, sizeof(npoint *),
      (qsort_comparator) &npoint_sort_cmp);
}
*/

/**
 * Comparator function for network segments
 */
static int
nsegment_sort_cmp(nsegment **l, nsegment **r)
{
  return nsegment_cmp_internal(*l, *r);
}

/**
 * Sort function for network segments
 */
static void
nsegmentarr_sort(nsegment **segments, int count)
{
  qsort(segments, (size_t) count, sizeof(nsegment *),
      (qsort_comparator) &nsegment_sort_cmp);
}

/**
 * Normalize the array of temporal segments
 */
nsegment **
nsegmentarr_normalize(nsegment **segments, int *count)
{
  assert(*count != 0);
  nsegmentarr_sort(segments, *count);
  int newcount = 0;
  nsegment **result = palloc(sizeof(nsegment *) * *count);
  nsegment *current = segments[0];
  for (int i = 1; i < *count; i++)
  {
    nsegment *seg = segments[i];
    if (current->rid == seg->rid)
    {
      current->pos1 = Min(current->pos1, seg->pos1);
      current->pos2 = Max(current->pos2, seg->pos2);
      pfree(seg);
    }
    else
    {
      result[newcount++] = current;
      current = seg;
    }
  }
  result[newcount++] = current;
  *count = newcount;
  return result;
}

/* Remove duplicates from an array of npoints

int
npoint_remove_duplicates(npoint **values, int count)
{
  assert (count > 0);
  int newcount = 0;
  for (int i = 1; i < count; i++)
    if (npoint_ne_internal(values[newcount], values[i]))
      values[++ newcount] = values[i];
  return newcount + 1;
}
*/

/*****************************************************************************
 * Input/Output functions for npoint
 *****************************************************************************/

PG_FUNCTION_INFO_V1(npoint_in);
/**
 * Input function for network points
 * Example of input:
 *    (1, 0.5)
 */
PGDLLEXPORT Datum
npoint_in(PG_FUNCTION_ARGS)
{
  char *str = PG_GETARG_CSTRING(0);
  npoint *result = npoint_parse(&str);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(npoint_out);
/**
 * Output function for network points
 */
PGDLLEXPORT Datum
npoint_out(PG_FUNCTION_ARGS)
{
  static size_t size = MAXNPOINTLEN + 1;
  char *result = (char *) palloc(size);
  npoint *np = PG_GETARG_NPOINT(0);
  char *rid = call_output(INT8OID, Int64GetDatum(np->rid));
  char *pos = call_output(FLOAT8OID, Float8GetDatum(np->pos));
  snprintf(result, size, "NPoint(%s,%s)", rid, pos);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(npoint_recv);
/**
 * Receive function for network points
 */
PGDLLEXPORT Datum
npoint_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  npoint *result;

  result = (npoint *) palloc(sizeof(npoint));
  result->rid = pq_getmsgint64(buf);
  result->pos = pq_getmsgfloat8(buf);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(npoint_send);
/**
 * Send function for network points
 */
PGDLLEXPORT Datum
npoint_send(PG_FUNCTION_ARGS)
{
  npoint  *np = PG_GETARG_NPOINT(0);
  StringInfoData buf;

  pq_begintypsend(&buf);
  pq_sendint64(&buf, (uint64) np->rid);
  pq_sendfloat8(&buf, np->pos);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * Input/Output functions for nsegment
 *****************************************************************************/

PG_FUNCTION_INFO_V1(nsegment_in);
/**
 * Input function for network segments
 * Example of input:
 *    (1, 0.5, 0.6)
 */
PGDLLEXPORT Datum
nsegment_in(PG_FUNCTION_ARGS)
{
  char *str = PG_GETARG_CSTRING(0);
  nsegment *result = nsegment_parse(&str);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(nsegment_out);
/**
 * Output function for network segments
 */
PGDLLEXPORT Datum
nsegment_out(PG_FUNCTION_ARGS)
{
  nsegment *ns = PG_GETARG_NSEGMENT(0);
  char *result = psprintf("NSegment(%ld,%g,%g)", ns->rid, ns->pos1, ns->pos2);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(nsegment_recv);
/**
 * Receive function for network segments
 */
PGDLLEXPORT Datum
nsegment_recv(PG_FUNCTION_ARGS)
{
  StringInfo  buf = (StringInfo) PG_GETARG_POINTER(0);
  nsegment *result;

  result = (nsegment *) palloc(sizeof(nsegment));
  result->rid = pq_getmsgint64(buf);
  result->pos1 = pq_getmsgfloat8(buf);
  result->pos2 = pq_getmsgfloat8(buf);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(nsegment_send);
/**
 * Send function for network segments
 */
PGDLLEXPORT Datum
nsegment_send(PG_FUNCTION_ARGS)
{
  nsegment *ns = PG_GETARG_NSEGMENT(0);
  StringInfoData buf;

  pq_begintypsend(&buf);
  pq_sendint64(&buf, (uint64) ns->rid);
  pq_sendfloat8(&buf, ns->pos1);
  pq_sendfloat8(&buf, ns->pos2);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * Constructors
 *****************************************************************************/

/* Set an npoint value from arguments */
/*
void
npoint_set(npoint *np, int64 rid, double pos)
{
  if (!route_exists(rid))
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("there is no route with gid value %lu in table ways", rid)));
  if (pos < 0 || pos > 1)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("the relative position must be a real number between 0 and 1")));

  np->rid = rid;
  np->pos = pos;
}
*/

/**
 * Construct an network point value from the arguments
 */
npoint *
npoint_make(int64 rid, double pos)
{
  if (!route_exists(rid))
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("there is no route with gid value %lu in table ways", rid)));
  if (pos < 0 || pos > 1)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("the relative position must be a real number between 0 and 1")));

  npoint *result = (npoint *) palloc(sizeof(npoint));
  result->rid = rid;
  result->pos = pos;
  return result;
}

PG_FUNCTION_INFO_V1(npoint_constructor);
/**
 * Construct an network point value from the arguments
 */
PGDLLEXPORT Datum
npoint_constructor(PG_FUNCTION_ARGS)
{
  int64 rid = PG_GETARG_INT64(0);
  double pos = PG_GETARG_FLOAT8(1);
  npoint *result = npoint_make(rid, pos);
  PG_RETURN_POINTER(result);
}

/* Set an nsegment value from arguments */
/*
void
nsegment_set(nsegment *ns, int64 rid, double pos1, double pos2)
{
  if (!route_exists(rid))
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("there is no route with gid value %lu in table ways", rid)));
  if (pos1 < 0 || pos1 > 1 || pos2 < 0 || pos2 > 1)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("The relative position of a network segment must be a real number between 0 and 1")));

  ns->rid = rid;
  ns->pos1 = Min(pos1, pos2);
  ns->pos2 = Max(pos1, pos2);
}
*/

/**
 * Construct an network segment value from the arguments
 */
nsegment *
nsegment_make(int64 rid, double pos1, double pos2)
{
  if (!route_exists(rid))
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("there is no route with gid value %lu in table ways", rid)));
  if (pos1 < 0 || pos1 > 1 || pos2 < 0 || pos2 > 1)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("The relative position of a network segment must be a real number between 0 and 1")));

  nsegment *result = (nsegment *) palloc(sizeof(nsegment));
  result->rid = rid;
  result->pos1 = Min(pos1, pos2);
  result->pos2 = Max(pos1, pos2);
  return result;
}

PG_FUNCTION_INFO_V1(nsegment_constructor);
/**
 * Construct an network segment value from the arguments
 */
PGDLLEXPORT Datum
nsegment_constructor(PG_FUNCTION_ARGS)
{
  int64 rid = PG_GETARG_INT64(0);
  double pos1 = PG_GETARG_FLOAT8(1);
  double pos2 = PG_GETARG_FLOAT8(2);
  nsegment *result = nsegment_make(rid, pos1, pos2);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(nsegment_from_npoint);
/**
 * Construct an network segment value from the network point
 */
PGDLLEXPORT Datum
nsegment_from_npoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  nsegment *result = nsegment_make(np->rid, np->pos, np->pos);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessing values
 *****************************************************************************/

PG_FUNCTION_INFO_V1(npoint_route);
/**
 * Returns the route of the network point
 */
PGDLLEXPORT Datum
npoint_route(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  int64 result = np->rid;
  PG_RETURN_INT64(result);
}

PG_FUNCTION_INFO_V1(npoint_position);
/**
 * Returns the position of the network point
 */
PGDLLEXPORT Datum
npoint_position(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  PG_RETURN_FLOAT8(np->pos);
}

PG_FUNCTION_INFO_V1(nsegment_route);
/**
 * Returns the route of the network segment
 */
PGDLLEXPORT Datum
nsegment_route(PG_FUNCTION_ARGS)
{
  nsegment *ns = PG_GETARG_NSEGMENT(0);
  PG_RETURN_INT64(ns->rid);
}

PG_FUNCTION_INFO_V1(nsegment_start_position);
/**
 * Returns the start position of the network segment
 */
PGDLLEXPORT Datum
nsegment_start_position(PG_FUNCTION_ARGS)
{
  nsegment *ns = PG_GETARG_NSEGMENT(0);
  PG_RETURN_FLOAT8(ns->pos1);
}

PG_FUNCTION_INFO_V1(nsegment_end_position);
/**
 * Returns the end position of the network segment
 */
PGDLLEXPORT Datum
nsegment_end_position(PG_FUNCTION_ARGS)
{
  nsegment *ns = PG_GETARG_NSEGMENT(0);
  PG_RETURN_FLOAT8(ns->pos2);
}

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

/**
 * Set the precision of the position of a network point to the number of
 * decimal places
 */
Datum
npoint_round_internal(Datum npt, Datum size)
{
  /* Set precision of position */
  npoint *np = (npoint *) DatumGetPointer(npt);
  double pos = DatumGetFloat8(datum_round_float(Float8GetDatum(np->pos), size));
  return PointerGetDatum(npoint_make(np->rid, pos));
}

PG_FUNCTION_INFO_V1(npoint_round);
/**
 * Set the precision of the position of a network point to the number of
 * decimal places
 */
PGDLLEXPORT Datum
npoint_round(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Datum size = PG_GETARG_DATUM(1);
  /* Set precision of position */
  double pos = DatumGetFloat8(datum_round_float(Float8GetDatum(np->pos), size));
  npoint *result = npoint_make(np->rid, pos);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(nsegment_round);
/**
 * Set the precision of the position of a network point to the number of
 * decimal places
 */
PGDLLEXPORT Datum
nsegment_round(PG_FUNCTION_ARGS)
{
  nsegment *ns = PG_GETARG_NSEGMENT(0);
  Datum size = PG_GETARG_DATUM(1);
  /* Set precision of positions */
  double pos1 = DatumGetFloat8(datum_round_float(Float8GetDatum(ns->pos1), size));
  double pos2 = DatumGetFloat8(datum_round_float(Float8GetDatum(ns->pos2), size));
  nsegment *result = nsegment_make(ns->rid, pos1, pos2);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

/**
 * Returns true if the first network point is equal to the second one
 */
bool
npoint_eq_internal(const npoint *np1, const npoint *np2)
{
  return np1->rid == np2->rid && fabs(np1->pos - np2->pos) < MOBDB_EPSILON;
}

PG_FUNCTION_INFO_V1(npoint_eq);
/**
 * Returns true if the first network point is equal to the second one
 */
PGDLLEXPORT Datum
npoint_eq(PG_FUNCTION_ARGS)
{
  npoint *np1 = PG_GETARG_NPOINT(0);
  npoint *np2 = PG_GETARG_NPOINT(1);
  PG_RETURN_BOOL(npoint_eq_internal(np1, np2));
}

/**
 * Returns true if the first network point is not equal to the second one
 */
bool
npoint_ne_internal(const npoint *np1, const npoint *np2)
{
  return (!npoint_eq_internal(np1, np2));
}

PG_FUNCTION_INFO_V1(npoint_ne);
/**
 * Returns true if the first network point is not equal to the second one
 */
PGDLLEXPORT Datum
npoint_ne(PG_FUNCTION_ARGS)
{
  npoint *np1 = PG_GETARG_NPOINT(0);
  npoint *np2 = PG_GETARG_NPOINT(1);
  PG_RETURN_BOOL(npoint_ne_internal(np1, np2));
}

/**
 * Returns -1, 0, or 1 depending on whether the first network point
 * is less than, equal, or greater than the second one
 *
 * @note Function used for B-tree comparison
 */
int
npoint_cmp_internal(const npoint *np1, const npoint *np2)
{
  if (np1->rid < np2->rid)
    return -1;
  else if (np1->rid > np2->rid)
    return 1;
  /* Both rid are equal */
  else if(np1->pos < np2->pos)
    return -1;
  else if (np1->pos > np2->pos)
    return 1;
  return 0;
}

PG_FUNCTION_INFO_V1(npoint_cmp);
/**
 * Returns -1, 0, or 1 depending on whether the first network point
 * is less than, equal, or greater than the second one
 *
 * @note Function used for B-tree comparison
 */
PGDLLEXPORT Datum
npoint_cmp(PG_FUNCTION_ARGS)
{
  npoint *np1 = PG_GETARG_NPOINT(0);
  npoint *np2 = PG_GETARG_NPOINT(1);
  PG_RETURN_INT32(npoint_cmp_internal(np1, np2));
}

/* Inequality operators using the npoint_cmp function */

/**
 * Returns true if the first network point is less than the second one
 */
bool
npoint_lt_internal(const npoint *np1, const npoint *np2)
{
  int cmp = npoint_cmp_internal(np1, np2);
  return (cmp < 0);
}

PG_FUNCTION_INFO_V1(npoint_lt);
/**
 * Returns true if the first network point is less than the second one
 */
PGDLLEXPORT Datum
npoint_lt(PG_FUNCTION_ARGS)
{
  npoint *np1 = PG_GETARG_NPOINT(0);
  npoint *np2 = PG_GETARG_NPOINT(1);
  int cmp = npoint_cmp_internal(np1, np2);
  PG_RETURN_BOOL(cmp < 0);
}

PG_FUNCTION_INFO_V1(npoint_le);
/**
 * Returns true if the first network point is less than or equal to the
 * second one
 */
PGDLLEXPORT Datum
npoint_le(PG_FUNCTION_ARGS)
{
  npoint *np1 = PG_GETARG_NPOINT(0);
  npoint *np2 = PG_GETARG_NPOINT(1);
  int cmp = npoint_cmp_internal(np1, np2);
  PG_RETURN_BOOL(cmp <= 0);
}

PG_FUNCTION_INFO_V1(npoint_ge);
/**
 * Returns true if the first network point is greater than or equal to the
 * second one
 */
PGDLLEXPORT Datum
npoint_ge(PG_FUNCTION_ARGS)
{
  npoint *np1 = PG_GETARG_NPOINT(0);
  npoint *np2 = PG_GETARG_NPOINT(1);
  int cmp = npoint_cmp_internal(np1, np2);
  PG_RETURN_BOOL(cmp >= 0);
}

PG_FUNCTION_INFO_V1(npoint_gt);
/**
 * Returns true if the first network point is greater than the second one
 */
PGDLLEXPORT Datum
npoint_gt(PG_FUNCTION_ARGS)
{
  npoint *np1 = PG_GETARG_NPOINT(0);
  npoint *np2 = PG_GETARG_NPOINT(1);
  int cmp = npoint_cmp_internal(np1, np2);
  PG_RETURN_BOOL(cmp > 0);
}

/*****************************************************************************/

/**
 * Returns true if the first network segment is equal to the second one
 */
bool
nsegment_eq_internal(const nsegment *ns1, const nsegment *ns2)
{
  return ns1->rid == ns2->rid && fabs(ns1->pos1 - ns2->pos1) < MOBDB_EPSILON &&
    fabs(ns1->pos2 - ns2->pos2) < MOBDB_EPSILON;
}

PG_FUNCTION_INFO_V1(nsegment_eq);
/**
 * Returns true if the first network segment is equal to the second one
 */
PGDLLEXPORT Datum
nsegment_eq(PG_FUNCTION_ARGS)
{
  nsegment *ns1 = PG_GETARG_NSEGMENT(0);
  nsegment *ns2 = PG_GETARG_NSEGMENT(1);
  PG_RETURN_BOOL(nsegment_eq_internal(ns1, ns2));
}

/**
 * Returns true if the first network segment is not equal to the second one
 */
bool
nsegment_ne_internal(const nsegment *ns1, const nsegment *ns2)
{
  return (!nsegment_eq_internal(ns1, ns2));
}

PG_FUNCTION_INFO_V1(nsegment_ne);
/**
 * Returns true if the first network segment is not equal to the second one
 */
PGDLLEXPORT Datum
nsegment_ne(PG_FUNCTION_ARGS)
{
  nsegment *ns1 = PG_GETARG_NSEGMENT(0);
  nsegment *ns2 = PG_GETARG_NSEGMENT(1);
  PG_RETURN_BOOL(nsegment_ne_internal(ns1, ns2));
}

/**
 * Returns -1, 0, or 1 depending on whether the first network segment
 * is less than, equal, or greater than the second one
 *
 * @note Function used for B-tree comparison
 */
int
nsegment_cmp_internal(const nsegment *ns1, const nsegment *ns2)
{
  if (ns1->rid < ns2->rid)
    return -1;
  else if (ns1->rid > ns2->rid)
    return 1;
  /* Both rid are equal */
  else if(ns1->pos1 < ns2->pos1)
    return -1;
  else if (ns1->pos1 > ns2->pos1)
    return 1;
  /* Both pos1 are equal */
  else if(ns1->pos2 < ns2->pos2)
    return -1;
  else if (ns1->pos2 > ns2->pos2)
    return 1;
  return 0;
}

PG_FUNCTION_INFO_V1(nsegment_cmp);
/**
 * Returns -1, 0, or 1 depending on whether the first network segment
 * is less than, equal, or greater than the second one
 */
PGDLLEXPORT Datum
nsegment_cmp(PG_FUNCTION_ARGS)
{
  nsegment *ns1 = PG_GETARG_NSEGMENT(0);
  nsegment *ns2 = PG_GETARG_NSEGMENT(1);
  PG_RETURN_INT32(nsegment_cmp_internal(ns1, ns2));
}

/* Inequality operators using the nsegment_cmp function */

PG_FUNCTION_INFO_V1(nsegment_lt);
/**
 * Returns true if the first network segment is less than the second one
 */
PGDLLEXPORT Datum
nsegment_lt(PG_FUNCTION_ARGS)
{
  nsegment *ns1 = PG_GETARG_NSEGMENT(0);
  nsegment *ns2 = PG_GETARG_NSEGMENT(1);
  int cmp = nsegment_cmp_internal(ns1, ns2);
  PG_RETURN_BOOL(cmp < 0);
}

PG_FUNCTION_INFO_V1(nsegment_le);
/**
 * Returns true if the first network segment is less than or equal to the
 * second one
 */
PGDLLEXPORT Datum
nsegment_le(PG_FUNCTION_ARGS)
{
  nsegment *ns1 = PG_GETARG_NSEGMENT(0);
  nsegment *ns2 = PG_GETARG_NSEGMENT(1);
  int cmp = nsegment_cmp_internal(ns1, ns2);
  PG_RETURN_BOOL(cmp <= 0);
}

PG_FUNCTION_INFO_V1(nsegment_ge);
/**
 * Returns true if the first network segment is greater than or equal to the
 * second one
 */
PGDLLEXPORT Datum
nsegment_ge(PG_FUNCTION_ARGS)
{
  nsegment *ns1 = PG_GETARG_NSEGMENT(0);
  nsegment *ns2 = PG_GETARG_NSEGMENT(1);
  int cmp = nsegment_cmp_internal(ns1, ns2);
  PG_RETURN_BOOL(cmp >= 0);
}

PG_FUNCTION_INFO_V1(nsegment_gt);
/**
 * Returns true if the first network segment is greater than the second one
 */
PGDLLEXPORT Datum
nsegment_gt(PG_FUNCTION_ARGS)
{
  nsegment *ns1 = PG_GETARG_NSEGMENT(0);
  nsegment *ns2 = PG_GETARG_NSEGMENT(1);
  int cmp = nsegment_cmp_internal(ns1, ns2);
  PG_RETURN_BOOL(cmp > 0);
}

/*****************************************************************************
 * Conversions between network and Euclidean space
 *****************************************************************************/

/**
 * Returns true if the edge table contains a route with the route identifier
 */
bool
route_exists(int64 rid)
{
  char sql[64];
  sprintf(sql, "SELECT true FROM public.ways WHERE gid = %ld", rid);
  bool isNull = true;
  bool result = false;
  SPI_connect();
  int ret = SPI_execute(sql, true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable != NULL)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    result = DatumGetBool(SPI_getbinval(tuptable->vals[0],
      tuptable->tupdesc, 1, &isNull));
  }
  SPI_finish();
  return result;
}

/**
 * Access the edge table to return the route length from the corresponding
 * route identifier
 */
double
route_length(int64 rid)
{
  char sql[64];
  sprintf(sql, "SELECT length FROM public.ways WHERE gid = %ld", rid);
  bool isNull = true;
  double result = 0;
  SPI_connect();
  int ret = SPI_execute(sql, true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable != NULL)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    result = DatumGetFloat8(SPI_getbinval(tuptable->vals[0],
      tuptable->tupdesc, 1, &isNull));
  }
  SPI_finish();

  if (isNull)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("cannot get the length for route %ld", rid)));

  return result;
}

/**
 * Access the edge table to get the route geometry from corresponding route
 * identifier
 */
Datum
route_geom(int64 rid)
{
  char sql[64];
  sprintf(sql, "SELECT the_geom FROM public.ways WHERE gid = %ld", rid);
  bool isNull = true;
  GSERIALIZED *result = NULL;
  SPI_connect();
  int ret = SPI_execute(sql, true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable != NULL)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    Datum line = SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 1, &isNull);
    if (!isNull)
    {
      /* Must allocate this in upper executor context to keep it alive after SPI_finish() */
      GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(line);
      result = (GSERIALIZED *)SPI_palloc(gs->size);
      memcpy(result, gs, gs->size);
    }
  }
  SPI_finish();

  if (isNull)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("cannot get the geometry for route %ld", rid)));

  ensure_non_empty(result);

  return PointerGetDatum(result);
}

/* Access edge table to get the rid from corresponding geometry */
/*
int64
rid_from_geom(Datum geom)
{
  char *geomstr = ewkt_out(ANYOID, geom);
  char sql[128];
  sprintf(sql, "SELECT gid FROM public.ways WHERE ST_DWithin(the_geom, '%s', %lf) "
    "ORDER BY ST_Distance(the_geom, '%s') LIMIT 1", geomstr, DIST_EPSILON, geomstr);
  pfree(geomstr);
  bool isNull = true;
  int64 result = 0; / * make compiler quiet * /
  SPI_connect();
  int ret = SPI_execute(sql, true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable != NULL)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    result = DatumGetInt64(SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 1, &isNull));
  }
  SPI_finish();
  if (isNull)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("cannot get route identifier from geometry point")));

  return result;
}
*/
/*****************************************************************************/

/**
 * Returns the SRID of the network point
 */
int
npoint_srid_internal(const npoint *np)
{
  Datum line = route_geom(np->rid);
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(line);
  int result = gserialized_get_srid(gs);
  pfree(DatumGetPointer(line));
  return result;
}

PG_FUNCTION_INFO_V1(npoint_srid);
/**
 * Returns the SRID of the network point
 */
PGDLLEXPORT Datum
npoint_srid(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  int result = npoint_srid_internal(np);
  PG_RETURN_INT32(result);
}

/**
 * Transforms the network point into a geometry
 */
Datum
npoint_as_geom_internal(const npoint *np)
{
  Datum line = route_geom(np->rid);
  Datum result = call_function2(LWGEOM_line_interpolate_point, line, Float8GetDatum(np->pos));
  pfree(DatumGetPointer(line));
  return result;
}

PG_FUNCTION_INFO_V1(npoint_as_geom);
/**
 * Transforms the network point into a geometry
 */
PGDLLEXPORT Datum
npoint_as_geom(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Datum result = npoint_as_geom_internal(np);
  PG_RETURN_DATUM(result);
}

/**
 * Transforms the geometry into a network point
 */
npoint *
geom_as_npoint_internal(Datum geom)
{
  char *geomstr = ewkt_out(ANYOID, geom);
  char sql[512];
  sprintf(sql, "SELECT npoint(gid, ST_LineLocatePoint(the_geom, '%s')) "
    "FROM public.ways WHERE ST_DWithin(the_geom, '%s', %lf) "
    "ORDER BY ST_Distance(the_geom, '%s') LIMIT 1", geomstr, geomstr,
    DIST_EPSILON, geomstr);
  pfree(geomstr);
  npoint *result = (npoint *) palloc(sizeof(npoint));
  bool isNull = true;
  SPI_connect();
  int ret = SPI_execute(sql, true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable != NULL)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    Datum value = SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 1, &isNull);
    if (!isNull)
    {
      /* Must allocate this in upper executor context to keep it alive after SPI_finish() */
      npoint *np = DatumGetNpoint(value);
      memcpy(result, np, sizeof(npoint));
    }
  }
  SPI_finish();
  if (isNull)
  {
    pfree(result);
    return NULL;
  }
  return result;
}

PG_FUNCTION_INFO_V1(geom_as_npoint);
/**
 * Transforms the geometry into a network point
 */
PGDLLEXPORT Datum
geom_as_npoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  /* Ensure validity of operation */
  ensure_non_empty(gs);
  ensure_point_type(gs);
  int32_t srid_geom = gserialized_get_srid(gs);
  int32_t srid_ways = get_srid_ways();
  ensure_same_srid(srid_geom, srid_ways);

  npoint *result = geom_as_npoint_internal(PointerGetDatum(gs));
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Returns the SRID of the network segment
 */
int
nsegment_srid_internal(const nsegment *ns)
{
  Datum line = route_geom(ns->rid);
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(line);
  int result = gserialized_get_srid(gs);
  pfree(DatumGetPointer(line));
  return result;
}

PG_FUNCTION_INFO_V1(nsegment_srid);
/**
 * Returns the SRID of the network segment
 */
PGDLLEXPORT Datum
nsegment_srid(PG_FUNCTION_ARGS)
{
  nsegment *ns = PG_GETARG_NSEGMENT(0);
  int result = nsegment_srid_internal(ns);
  PG_RETURN_INT32(result);
}

/**
 * Transforms the network segment into a geometry
 */
Datum
nsegment_as_geom_internal(const nsegment *ns)
{
  Datum line = route_geom(ns->rid);
  Datum result;
  if (fabs(ns->pos1 - ns->pos2) < MOBDB_EPSILON)
    result = call_function2(LWGEOM_line_interpolate_point, line,
      Float8GetDatum(ns->pos1));
  else
    result = call_function3(LWGEOM_line_substring, line,
      Float8GetDatum(ns->pos1), Float8GetDatum(ns->pos2));
  pfree(DatumGetPointer(line));
  return result;
}

PG_FUNCTION_INFO_V1(nsegment_as_geom);
/**
 * Transforms the network segment into a geometry
 */
PGDLLEXPORT Datum
nsegment_as_geom(PG_FUNCTION_ARGS)
{
  nsegment *ns = PG_GETARG_NSEGMENT(0);
  Datum result = nsegment_as_geom_internal(ns);
  PG_RETURN_DATUM(result);
}

/**
 * Transforms the geometry into a network segment
 */
nsegment *
geom_as_nsegment_internal(Datum geom)
{
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(geom);
  int geomtype = gserialized_get_type(gs);
  assert(geomtype == POINTTYPE || geomtype == LINETYPE);
  npoint **points;
  npoint *np;
  int k = 0;
  if (geomtype == POINTTYPE)
  {
    points = palloc0(sizeof(npoint *));
    np = geom_as_npoint_internal(geom);
    if (np != NULL)
      points[k++] = np;
  }
  else /* geomtype == LINETYPE */
  {
    int numpoints = DatumGetInt32(call_function1(LWGEOM_numpoints_linestring, geom));
    points = palloc0(sizeof(npoint *) * numpoints);
    for (int i = 0; i < numpoints; i++)
    {
      /* The composing points are from 1 to numcount */
      Datum point = call_function2(LWGEOM_pointn_linestring, geom, Int32GetDatum(i + 1));
      np = geom_as_npoint_internal(point);
      if (np != NULL)
        points[k++] = np;
      /* Cannot pfree(DatumGetPointer(point)); */
    }
  }

  if (k == 0)
  {
    pfree(points);
    return NULL;
  }
  int64 rid = points[0]->rid;
  double minPos = points[0]->pos, maxPos = points[0]->pos;
  for (int i = 1; i < k; i++)
  {
    if (points[i]->rid != rid)
    {
      for (int j = 0; j < k; j++)
        pfree(points[j]);
      pfree(points);
      return NULL;
    }
    minPos = Min(minPos, points[i]->pos);
    maxPos = Max(maxPos, points[i]->pos);
  }
  nsegment *result = nsegment_make(rid, minPos, maxPos);
  for (int i = 0; i < k; i++)
    pfree(points[i]);
  pfree(points);
  return result;
}

PG_FUNCTION_INFO_V1(geom_as_nsegment);
/**
 * Transforms the geometry into a network segment
 */
PGDLLEXPORT Datum
geom_as_nsegment(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  ensure_non_empty(gs);
  if (gserialized_get_type(gs) != POINTTYPE && gserialized_get_type(gs) != LINETYPE)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Only point or line geometries accepted")));
  nsegment *result = geom_as_nsegment_internal(PointerGetDatum(gs));
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
