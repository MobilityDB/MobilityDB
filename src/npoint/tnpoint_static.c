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
 * @brief Network-based static point and segment types.
 *
 * Several functions are commented out since they are not currently used.
 * They are kept if needed in the future.
 */

#include "npoint/tnpoint_static.h"

/* PostgreSQL */
#include <assert.h>
#include <libpq/pqformat.h>
#include <executor/spi.h>
/* PostGIS */
#include <liblwgeom.h>
/* MobilityDB */
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
 * General functions
 *****************************************************************************/

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
npointarr_geom(Npoint **points, int count)
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
    ArrayType *array = datumarr_to_array(geoms, count, T_GEOMETRY);
    result = call_function1(pgis_union_geometry_array, PointerGetDatum(array));
    pfree(array);
    for (int i = 0; i < count; i++)
      pfree(DatumGetPointer(geoms[i]));
    pfree(geoms);
  }
  return result;
}

/**
 * Convert a network segment array into a geometry
 */
Datum
nsegmentarr_geom(Nsegment **segments, int count)
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
    ArrayType *array = datumarr_to_array(geoms, count, T_GEOMETRY);
    result = call_function1(pgis_union_geometry_array, PointerGetDatum(array));
    pfree(array);
    for (int i = 0; i < count; i++)
      pfree(DatumGetPointer(geoms[i]));
    pfree(geoms);
  }
  return result;
}

/*****************************************************************************/

#if 0 /* not used */
/**
 * Comparator functions
 */
static int
npoint_sort_cmp(Npoint **l, Npoint **r)
{
  return npoint_cmp(*l, *r);
}

void
npointarr_sort(Npoint **points, int count)
{
  qsort(points, (size_t) count, sizeof(Npoint *),
      (qsort_comparator) &npoint_sort_cmp);
}
#endif

/**
 * Comparator function for network segments
 */
static int
nsegment_sort_cmp(Nsegment **l, Nsegment **r)
{
  return nsegment_cmp(*l, *r);
}

/**
 * Sort function for network segments
 */
static void
nsegmentarr_sort(Nsegment **segments, int count)
{
  qsort(segments, (size_t) count, sizeof(Nsegment *),
      (qsort_comparator) &nsegment_sort_cmp);
}

/**
 * Normalize the array of temporal segments
 */
Nsegment **
nsegmentarr_normalize(Nsegment **segments, int *count)
{
  assert(*count != 0);
  nsegmentarr_sort(segments, *count);
  int newcount = 0;
  Nsegment **result = palloc(sizeof(Nsegment *) * *count);
  Nsegment *current = segments[0];
  for (int i = 1; i < *count; i++)
  {
    Nsegment *seg = segments[i];
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

#if 0 /* not used */
/**
 * Remove duplicates from an array of npoints
 */
int
npoint_remove_duplicates(Npoint **values, int count)
{
  assert (count > 0);
  int newcount = 0;
  for (int i = 1; i < count; i++)
    if (npoint_ne(values[newcount], values[i]))
      values[++ newcount] = values[i];
  return newcount + 1;
}
#endif

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Output function for network points
 */
char *
npoint_to_string(const Npoint *np)
{
  static size_t size = MAXNPOINTLEN + 1;
  char *result = (char *) palloc(size);
  char *rid = call_output(INT8OID, Int64GetDatum(np->rid));
  char *pos = call_output(FLOAT8OID, Float8GetDatum(np->pos));
  snprintf(result, size, "NPoint(%s,%s)", rid, pos);
  return result;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a new network point value from its binary representation read
 * from the buffer.
 */
Npoint *
npoint_read(StringInfo buf)
{
  Npoint *result = (Npoint *) palloc0(sizeof(Npoint));
  result->rid = pq_getmsgint64(buf);
  result->pos = pq_getmsgfloat8(buf);
  return result;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Write the binary representation of the network point value into the
 * buffer.
 *
 * @param[in] np Network point value
 * @param[in] buf Buffer
 */
void
npoint_write(const Npoint *np, StringInfo buf)
{
  pq_sendint64(buf, (uint64) np->rid);
  pq_sendfloat8(buf, np->pos);
  return;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Output function for network segments
 */
char *
nsegment_to_string(Nsegment *ns)
{
  char *result = psprintf("NSegment(%ld,%g,%g)", ns->rid, ns->pos1, ns->pos2);
  return result;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Receive function for network segments
 */
Nsegment *
nsegment_read(StringInfo buf)
{
  Nsegment *result = (Nsegment *) palloc(sizeof(Nsegment));
  result->rid = pq_getmsgint64(buf);
  result->pos1 = pq_getmsgfloat8(buf);
  result->pos2 = pq_getmsgfloat8(buf);
  return result;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Send function for network segments
 */
void
nsegment_write(Nsegment *ns, StringInfo buf)
{
  pq_sendint64(buf, (uint64) ns->rid);
  pq_sendfloat8(buf, ns->pos1);
  pq_sendfloat8(buf, ns->pos2);
  return;
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct an network point value from the arguments
 */
Npoint *
npoint_make(int64 rid, double pos)
{
  /* Note: zero-fill is done in the npoint_set function */
  Npoint *result = (Npoint *) palloc(sizeof(Npoint));
  npoint_set(rid, pos, result);
  return result;
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Set a network point value from the arguments.
 */
void
npoint_set(int64 rid, double pos, Npoint *np)
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

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct an network segment value from the arguments
 */
Nsegment *
nsegment_make(int64 rid, double pos1, double pos2)
{
  /* Note: zero-fill is done in the nsegment_set function */
  Nsegment *result = (Nsegment *) palloc(sizeof(Nsegment));
  nsegment_set(rid, pos1, pos2, result);
  return result;
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Set a network segment value from the arguments.
 */
void
nsegment_set(int64 rid, double pos1, double pos2, Nsegment *ns)
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

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_cast
 * @brief Construct an network segment value from the arguments
 */
Nsegment *
npoint_nsegment(const Npoint *np)
{
  return nsegment_make(np->rid, np->pos, np->pos);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_accesor
 * @brief Return the route of the network point
 */
int64
npoint_route(Npoint *np)
{
  return np->rid;
}

/**
 * @ingroup libmeos_temporal_accesor
 * @brief Return the position of the network point
 */
double
npoint_position(Npoint *np)
{
  return np->pos;
}

/**
 * @ingroup libmeos_temporal_accesor
 * @brief Return the route of the network segment
 */
int64
nsegment_route(Nsegment *ns)
{
  return ns->rid;
}

/**
 * @ingroup libmeos_temporal_accesor
 * @brief Return the start position of the network segment
 */
double
nsegment_start_position(Nsegment *ns)
{
  return ns->pos1;
}

/**
 * @ingroup libmeos_temporal_accesor
 * @brief Return the end position of the network segment
 */
double
nsegment_end_position(Nsegment *ns)
{
  return ns->pos2;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/


/**
 * @brief Set the precision of the position of a network point to the number of
 * decimal places
 * @note Funcion used by the lifting infrastructure
 */
Datum
datum_npoint_round(Datum npoint, Datum size)
{
  /* Set precision of position */
  Npoint *np = (Npoint *) DatumGetPointer(npoint);
  Npoint *result = npoint_round(np, size);
  return PointerGetDatum(result);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Set the precision of the position of a network point to the number of
 * decimal places
 */
Npoint *
npoint_round(Npoint *np, Datum size)
{
  /* Set precision of position */
  double pos = DatumGetFloat8(datum_round_float(Float8GetDatum(np->pos), size));
  Npoint *result = npoint_make(np->rid, pos);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Set the precision of the position of a network point to the number of
 * decimal places
 */
Nsegment *
nsegment_round(Nsegment *ns, Datum size)
{
  /* Set precision of positions */
  double pos1 = DatumGetFloat8(datum_round_float(Float8GetDatum(ns->pos1),
    size));
  double pos2 = DatumGetFloat8(datum_round_float(Float8GetDatum(ns->pos2),
    size));
  Nsegment *result = nsegment_make(ns->rid, pos1, pos2);
  return result;
}

/*****************************************************************************
 * Conversions between network and Euclidean space
 *****************************************************************************/

/**
 * Return true if the edge table contains a route with the route identifier
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

#if 0 /* not used */
/**
 * Access edge table to get the rid from corresponding geometry
 */
int64
rid_from_geom(Datum geom)
{
  char *geomstr = ewkt_out(ANYOID, geom);
  char sql[128];
  sprintf(sql, "SELECT gid FROM public.ways WHERE ST_DWithin(the_geom, '%s', %lf) "
    "ORDER BY ST_Distance(the_geom, '%s') LIMIT 1", geomstr, DIST_EPSILON, geomstr);
  pfree(geomstr);
  bool isNull = true;
  int64 result = 0; /* make compiler quiet */
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
#endif

/**
 * @ingroup libmeos_temporal_cast
 * @brief Transforms the network point into a geometry
 */
Datum
npoint_geom(const Npoint *np)
{
  Datum line = route_geom(np->rid);
  Datum result = call_function2(LWGEOM_line_interpolate_point, line, Float8GetDatum(np->pos));
  pfree(DatumGetPointer(line));
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Transforms the geometry into a network point
 */
Npoint *
geom_npoint(Datum geom)
{
  /* Ensure validity of operation */
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
  ensure_non_empty(gs);
  ensure_point_type(gs);
  int32_t srid_geom = gserialized_get_srid(gs);
  int32_t srid_ways = get_srid_ways();
  ensure_same_srid(srid_geom, srid_ways);

  char *geomstr = ewkt_out(ANYOID, geom);
  char sql[512];
  sprintf(sql, "SELECT npoint(gid, ST_LineLocatePoint(the_geom, '%s')) "
    "FROM public.ways WHERE ST_DWithin(the_geom, '%s', %lf) "
    "ORDER BY ST_Distance(the_geom, '%s') LIMIT 1", geomstr, geomstr,
    DIST_EPSILON, geomstr);
  pfree(geomstr);
  Npoint *result = (Npoint *) palloc(sizeof(Npoint));
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
      Npoint *np = DatumGetNpointP(value);
      memcpy(result, np, sizeof(Npoint));
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

/**
 * @ingroup libmeos_temporal_cast
 * @brief Transforms the network segment into a geometry
 */
Datum
nsegment_geom(const Nsegment *ns)
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

/**
 * @ingroup libmeos_temporal_cast
 * @brief Transforms the geometry into a network segment
 */
Nsegment *
geom_nsegment(Datum geom)
{
  /* Ensure validity of operation */
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
  ensure_non_empty(gs);
  int geomtype = gserialized_get_type(gs);
  if (geomtype != POINTTYPE && geomtype != LINETYPE)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Only point or line geometries accepted")));

  Npoint **points;
  Npoint *np;
  int k = 0;
  if (geomtype == POINTTYPE)
  {
    points = palloc0(sizeof(Npoint *));
    np = geom_npoint(geom);
    if (np != NULL)
      points[k++] = np;
  }
  else /* geomtype == LINETYPE */
  {
    int numpoints = DatumGetInt32(call_function1(LWGEOM_numpoints_linestring, geom));
    points = palloc0(sizeof(Npoint *) * numpoints);
    for (int i = 0; i < numpoints; i++)
    {
      /* The composing points are from 1 to numcount */
      Datum point = call_function2(LWGEOM_pointn_linestring, geom, Int32GetDatum(i + 1));
      np = geom_npoint(point);
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
      pfree_array((void **) points, k);
      return NULL;
    }
    minPos = Min(minPos, points[i]->pos);
    maxPos = Max(maxPos, points[i]->pos);
  }
  Nsegment *result = nsegment_make(rid, minPos, maxPos);
  pfree_array((void **) points, k);
  return result;
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_accesor
 * @brief Return the SRID of the network point
 */
int
npoint_srid(const Npoint *np)
{
  Datum line = route_geom(np->rid);
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(line);
  int result = gserialized_get_srid(gs);
  pfree(DatumGetPointer(line));
  return result;
}

/**
 * @ingroup libmeos_temporal_accesor
 * @brief Return the SRID of the network segment.
 */
int
nsegment_srid(const Nsegment *ns)
{
  Datum line = route_geom(ns->rid);
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(line);
  int result = gserialized_get_srid(gs);
  pfree(DatumGetPointer(line));
  return result;
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the first network point is equal to the second one
 */
bool
npoint_eq(const Npoint *np1, const Npoint *np2)
{
  return np1->rid == np2->rid && fabs(np1->pos - np2->pos) < MOBDB_EPSILON;
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the first network point is not equal to the second one
 */
bool
npoint_ne(const Npoint *np1, const Npoint *np2)
{
  return (!npoint_eq(np1, np2));
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return -1, 0, or 1 depending on whether the first network point
 * is less than, equal, or greater than the second one
 *
 * @note Function used for B-tree comparison
 */
int
npoint_cmp(const Npoint *np1, const Npoint *np2)
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

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the first network point is less than the second one
 */
bool
npoint_lt(const Npoint *np1, const Npoint *np2)
{
  int cmp = npoint_cmp(np1, np2);
  return (cmp < 0);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the first network point is less than or equal to the
 * second one
 */
bool
npoint_le(const Npoint *np1, const Npoint *np2)
{
  int cmp = npoint_cmp(np1, np2);
  return (cmp <= 0);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the first network point is greater than the second one
 */
bool
npoint_gt(const Npoint *np1, const Npoint *np2)
{
  int cmp = npoint_cmp(np1, np2);
  return (cmp > 0);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the first network point is greater than or equal to
 * the second one
 */
bool
npoint_ge(const Npoint *np1, const Npoint *np2)
{
  int cmp = npoint_cmp(np1, np2);
  return (cmp >= 0);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the first network segment is equal to the second one
 */
bool
nsegment_eq(const Nsegment *ns1, const Nsegment *ns2)
{
  return ns1->rid == ns2->rid && fabs(ns1->pos1 - ns2->pos1) < MOBDB_EPSILON &&
    fabs(ns1->pos2 - ns2->pos2) < MOBDB_EPSILON;
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the first network segment is not equal to the second one
 */
bool
nsegment_ne(const Nsegment *ns1, const Nsegment *ns2)
{
  return (!nsegment_eq(ns1, ns2));
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return -1, 0, or 1 depending on whether the first network segment
 * is less than, equal, or greater than the second one
 *
 * @note Function used for B-tree comparison
 */
int
nsegment_cmp(const Nsegment *ns1, const Nsegment *ns2)
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

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the first network segment is less than the second one
 */
bool
nsegment_lt(const Nsegment *ns1, const Nsegment *ns2)
{
  int cmp = nsegment_cmp(ns1, ns2);
  return (cmp < 0);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the first network segment is less than or equal to the
 * second one
 */
bool
nsegment_le(const Nsegment *ns1, const Nsegment *ns2)
{
  int cmp = nsegment_cmp(ns1, ns2);
  return (cmp <= 0);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the first network segment is greater than the second one
 */
bool
nsegment_gt(const Nsegment *ns1, const Nsegment *ns2)
{
  int cmp = nsegment_cmp(ns1, ns2);
  return (cmp > 0);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the first network segment is greater than or equal to
 * the second one
 */
bool
nsegment_ge(const Nsegment *ns1, const Nsegment *ns2)
{
  int cmp = nsegment_cmp(ns1, ns2);
  return (cmp >= 0);
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/*****************************************************************************
 * Input/Output functions for network point
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Npoint_in);
/**
 * Input function for network points
 * Example of input:
 *    (1, 0.5)
 */
PGDLLEXPORT Datum
Npoint_in(PG_FUNCTION_ARGS)
{
  char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_POINTER(npoint_parse(&str));
}

PG_FUNCTION_INFO_V1(Npoint_out);
/**
 * Output function for network points
 */
PGDLLEXPORT Datum
Npoint_out(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_CSTRING(npoint_to_string(np));
}

PG_FUNCTION_INFO_V1(Npoint_recv);
/**
 * Receive function for network points
 */
PGDLLEXPORT Datum
Npoint_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PG_RETURN_POINTER(npoint_read(buf));
}

PG_FUNCTION_INFO_V1(Npoint_send);
/**
 * Send function for network points
 */
PGDLLEXPORT Datum
Npoint_send(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  npoint_write(np, &buf) ;
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * Input/Output functions for network segment
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Nsegment_in);
/**
 * Input function for network segments
 * Example of input:
 *    (1, 0.5, 0.6)
 */
PGDLLEXPORT Datum
Nsegment_in(PG_FUNCTION_ARGS)
{
  char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_POINTER(nsegment_parse(&str));
}

PG_FUNCTION_INFO_V1(Nsegment_out);
/**
 * Output function for network segments
 */
PGDLLEXPORT Datum
Nsegment_out(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_CSTRING(nsegment_to_string(ns));
}

PG_FUNCTION_INFO_V1(Nsegment_recv);
/**
 * Receive function for network segments
 */
PGDLLEXPORT Datum
Nsegment_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PG_RETURN_POINTER(nsegment_read(buf));
}

PG_FUNCTION_INFO_V1(Nsegment_send);
/**
 * Send function for network segments
 */
PGDLLEXPORT Datum
Nsegment_send(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  nsegment_write(ns, &buf);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Npoint_constructor);
/**
 * Construct an network point value from the arguments
 */
PGDLLEXPORT Datum
Npoint_constructor(PG_FUNCTION_ARGS)
{
  int64 rid = PG_GETARG_INT64(0);
  double pos = PG_GETARG_FLOAT8(1);
  PG_RETURN_POINTER(npoint_make(rid, pos));
}

PG_FUNCTION_INFO_V1(Nsegment_constructor);
/**
 * Construct an network segment value from the arguments
 */
PGDLLEXPORT Datum
Nsegment_constructor(PG_FUNCTION_ARGS)
{
  int64 rid = PG_GETARG_INT64(0);
  double pos1 = PG_GETARG_FLOAT8(1);
  double pos2 = PG_GETARG_FLOAT8(2);
  PG_RETURN_POINTER(nsegment_make(rid, pos1, pos2));
}

PG_FUNCTION_INFO_V1(Npoint_to_nsegment);
/**
 * Construct an network segment value from the network point
 */
PGDLLEXPORT Datum
Npoint_to_nsegment(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_POINTER(npoint_nsegment(np));
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Npoint_route);
/**
 * Return the route of the network point
 */
PGDLLEXPORT Datum
Npoint_route(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_INT64(npoint_route(np));
}

PG_FUNCTION_INFO_V1(Npoint_position);
/**
 * Return the position of the network point
 */
PGDLLEXPORT Datum
Npoint_position(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_FLOAT8(npoint_position(np));
}

PG_FUNCTION_INFO_V1(Nsegment_route);
/**
 * Return the route of the network segment
 */
PGDLLEXPORT Datum
Nsegment_route(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_INT64(nsegment_route(ns));
}

PG_FUNCTION_INFO_V1(Nsegment_start_position);
/**
 * Return the start position of the network segment
 */
PGDLLEXPORT Datum
Nsegment_start_position(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_FLOAT8(nsegment_start_position(ns));
}

PG_FUNCTION_INFO_V1(Nsegment_end_position);
/**
 * Return the end position of the network segment
 */
PGDLLEXPORT Datum
Nsegment_end_position(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_FLOAT8(nsegment_end_position(ns));
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Npoint_round);
/**
 * Set the precision of the position of a network point to the number of
 * decimal places
 */
PGDLLEXPORT Datum
Npoint_round(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Datum size = PG_GETARG_DATUM(1);
  PG_RETURN_POINTER(npoint_round(np, size));
}

PG_FUNCTION_INFO_V1(Nsegment_round);
/**
 * Set the precision of the position of a network point to the number of
 * decimal places
 */
PGDLLEXPORT Datum
Nsegment_round(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  Datum size = PG_GETARG_DATUM(1);
  PG_RETURN_POINTER(nsegment_round(ns, size));
}

/*****************************************************************************
 * Conversions between network and Euclidean space
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Npoint_to_geom);
/**
 * Transforms the network point into a geometry
 */
PGDLLEXPORT Datum
Npoint_to_geom(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Datum result = npoint_geom(np);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Geom_to_npoint);
/**
 * Transforms the geometry into a network point
 */
PGDLLEXPORT Datum
Geom_to_npoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Npoint *result = geom_npoint(PointerGetDatum(gs));
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Nsegment_to_geom);
/**
 * Transforms the network segment into a geometry
 */
PGDLLEXPORT Datum
Nsegment_to_geom(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  Datum result = nsegment_geom(ns);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Geom_to_nsegment);
/**
 * Transforms the geometry into a network segment
 */
PGDLLEXPORT Datum
Geom_to_nsegment(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Nsegment *result = geom_nsegment(PointerGetDatum(gs));
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Npoint_get_srid);
/**
 * Return the SRID of the network point
 */
PGDLLEXPORT Datum
Npoint_get_srid(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  int result = npoint_srid(np);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(Nsegment_get_srid);
/**
 * Return the SRID of the network segment
 */
PGDLLEXPORT Datum
Nsegment_get_srid(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  int result = nsegment_srid(ns);
  PG_RETURN_INT32(result);
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Npoint_eq);
/**
 * Return true if the first network point is equal to the second one
 */
PGDLLEXPORT Datum
Npoint_eq(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_eq(np1, np2));
}

PG_FUNCTION_INFO_V1(Npoint_ne);
/**
 * Return true if the first network point is not equal to the second one
 */
PGDLLEXPORT Datum
Npoint_ne(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_ne(np1, np2));
}

PG_FUNCTION_INFO_V1(Npoint_cmp);
/**
 * Return -1, 0, or 1 depending on whether the first network point
 * is less than, equal, or greater than the second one
 *
 * @note Function used for B-tree comparison
 */
PGDLLEXPORT Datum
Npoint_cmp(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_INT32(npoint_cmp(np1, np2));
}

PG_FUNCTION_INFO_V1(Npoint_lt);
/**
 * Return true if the first network point is less than the second one
 */
PGDLLEXPORT Datum
Npoint_lt(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_lt(np1, np2));
}

PG_FUNCTION_INFO_V1(Npoint_le);
/**
 * Return true if the first network point is less than or equal to the
 * second one
 */
PGDLLEXPORT Datum
Npoint_le(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_le(np1, np2));
}

PG_FUNCTION_INFO_V1(Npoint_ge);
/**
 * Return true if the first network point is greater than or equal to the
 * second one
 */
PGDLLEXPORT Datum
Npoint_ge(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_ge(np1, np2));
}

PG_FUNCTION_INFO_V1(Npoint_gt);
/**
 * Return true if the first network point is greater than the second one
 */
PGDLLEXPORT Datum
Npoint_gt(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_gt(np1, np2));
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Nsegment_eq);
/**
 * Return true if the first network segment is equal to the second one
 */
PGDLLEXPORT Datum
Nsegment_eq(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_eq(ns1, ns2));
}

PG_FUNCTION_INFO_V1(Nsegment_ne);
/**
 * Return true if the first network segment is not equal to the second one
 */
PGDLLEXPORT Datum
Nsegment_ne(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_ne(ns1, ns2));
}

PG_FUNCTION_INFO_V1(Nsegment_cmp);
/**
 * Return -1, 0, or 1 depending on whether the first network segment
 * is less than, equal, or greater than the second one
 */
PGDLLEXPORT Datum
Nsegment_cmp(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_INT32(nsegment_cmp(ns1, ns2));
}

/* Inequality operators using the nsegment_cmp function */

PG_FUNCTION_INFO_V1(Nsegment_lt);
/**
 * Return true if the first network segment is less than the second one
 */
PGDLLEXPORT Datum
Nsegment_lt(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_lt(ns1, ns2));
}

PG_FUNCTION_INFO_V1(Nsegment_le);
/**
 * Return true if the first network segment is less than or equal to the
 * second one
 */
PGDLLEXPORT Datum
Nsegment_le(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_le(ns1, ns2));
}

PG_FUNCTION_INFO_V1(Nsegment_ge);
/**
 * Return true if the first network segment is greater than or equal to the
 * second one
 */
PGDLLEXPORT Datum
Nsegment_ge(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_ge(ns1, ns2));
}

PG_FUNCTION_INFO_V1(Nsegment_gt);
/**
 * Return true if the first network segment is greater than the second one
 */
PGDLLEXPORT Datum
Nsegment_gt(PG_FUNCTION_ARGS)
{
  Nsegment *ns1 = PG_GETARG_NSEGMENT_P(0);
  Nsegment *ns2 = PG_GETARG_NSEGMENT_P(1);
  PG_RETURN_BOOL(nsegment_gt(ns1, ns2));
}

#endif /* #ifndef MEOS */

/*****************************************************************************/
