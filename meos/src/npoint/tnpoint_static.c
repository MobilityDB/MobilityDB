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
 * @file
 * @brief Network-based static point and segment types.
 *
 * Several functions are commented out since they are not currently used.
 * They are kept if needed in the future.
 */

#include "npoint/tnpoint_static.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <executor/spi.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include "general/pg_types.h"
#include "general/tnumber_mathfuncs.h"
#include "general/type_out.h"
#include "general/type_util.h"
#include "point/pgis_call.h"
#include "point/tpoint_out.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_parser.h"

/** Buffer size for input and output of npoint and nsegment values */
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
  int32_t srid_ways = 0; /* make compiler quiet */
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
 * @brief Convert an array of network points into a geometry
 * @pre The number of network points is greater than 1
 */
GSERIALIZED *
npointarr_geom(Npoint **points, int count)
{
  assert(count > 1);
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * count);
  for (int i = 0; i < count; i++)
  {
    GSERIALIZED *line = route_geom(points[i]->rid);
    int32_t srid = gserialized_get_srid(line);
    LWGEOM *lwline = lwgeom_from_gserialized(line);
    geoms[i] = lwgeom_line_interpolate_point(lwline, points[i]->pos, srid, 0);
    pfree(line); pfree(lwline);
  }
  int newcount;
  LWGEOM **newgeoms = lwpointarr_remove_duplicates(geoms, count, &newcount);
  LWGEOM *lwgeom = lwpointarr_make_trajectory(newgeoms, newcount, STEP);
  GSERIALIZED *result = geo_serialize(lwgeom);
  pfree(newgeoms); pfree(lwgeom);
  pfree_array((void **) geoms, count);
  return result;
}

/**
 * @brief Convert an array of network segments into a geometry
 * @pre The number of network segments is greater than 1
 */
GSERIALIZED *
nsegmentarr_geom(Nsegment **segments, int count)
{
  assert(count > 1);
  GSERIALIZED **geoms = palloc(sizeof(GSERIALIZED *) * count);
  for (int i = 0; i < count; i++)
  {
    GSERIALIZED *line = route_geom(segments[i]->rid);
    if (segments[i]->pos1 == 0 && segments[i]->pos2 == 1)
      geoms[i] = gserialized_copy(line);
    else if (segments[i]->pos1 == segments[i]->pos2)
      geoms[i] = gserialized_line_interpolate_point(line, segments[i]->pos1, 0);
    else
      geoms[i] = gserialized_line_substring(line, segments[i]->pos1,
        segments[i]->pos2);
    pfree(line);
  }
  GSERIALIZED *result = gserialized_array_union(geoms, count);
  pfree_array((void **) geoms, count);
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
 * @brief Return a network point from its string representation.
 */
Npoint *
npoint_in(const char *str, bool end)
{
  return npoint_parse(&str, end);
}

/**
 * @brief Return the string representation of a network point
 */
char *
npoint_out(const Npoint *np, int maxdd)
{
  char *result = palloc(MAXNPOINTLEN);
  char *rid = int8_out(np->rid);
  char *pos = float8_out(np->pos, maxdd);
  snprintf(result, MAXNPOINTLEN - 1, "NPoint(%s,%s)", rid, pos);
  pfree(rid); pfree(pos);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return a network point from its string representation.
 */
Nsegment *
nsegment_in(const char *str)
{
  return nsegment_parse(&str);
}

/**
 * @brief Output function for network segments
 */
char *
nsegment_out(const Nsegment *ns, int maxdd)
{
  char *result = palloc(MAXNPOINTLEN);
  char *rid = int8_out(ns->rid);
  char *pos1 = float8_out(ns->pos1, maxdd);
  char *pos2 = float8_out(ns->pos2, maxdd);
  snprintf(result, MAXNPOINTLEN - 1, "NSegment(%s,%s,%s)", rid, pos1, pos2);
  pfree(rid); pfree(pos1); pfree(pos2);
  return result;
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @brief Construct a network segment from the arguments
 */
Npoint *
npoint_make(int64 rid, double pos)
{
  /* Note: zero-fill is done in the npoint_set function */
  Npoint *result = palloc(sizeof(Npoint));
  npoint_set(rid, pos, result);
  return result;
}

/**
 * @brief Set a network segment from the arguments.
 */
void
npoint_set(int64 rid, double pos, Npoint *np)
{
  if (!route_exists(rid))
    elog(ERROR, "There is no route with gid value %ld in table ways", rid);
  if (pos < 0 || pos > 1)
    elog(ERROR, "The relative position must be a real number between 0 and 1");

  /* Note: zero-fill is required here, just as in heap tuples */
  memset(np, 0, sizeof(Npoint));
  /* Fill in the network point */
  np->rid = rid;
  np->pos = pos;
}

/**
 * @brief Construct a network segment from the arguments
 */
Nsegment *
nsegment_make(int64 rid, double pos1, double pos2)
{
  /* Note: zero-fill is done in the nsegment_set function */
  Nsegment *result = palloc(sizeof(Nsegment));
  nsegment_set(rid, pos1, pos2, result);
  return result;
}

/**
 * @brief Set a network segment from the arguments.
 */
void
nsegment_set(int64 rid, double pos1, double pos2, Nsegment *ns)
{
  if (! route_exists(rid))
    elog(ERROR, "There is no route with gid value %ld in table ways", rid);
  if (pos1 < 0 || pos1 > 1 || pos2 < 0 || pos2 > 1)
    elog(ERROR, "The relative position of a network segment must be a real number between 0 and 1");

  ns->rid = rid;
  ns->pos1 = Min(pos1, pos2);
  ns->pos2 = Max(pos1, pos2);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * @brief Cast a network point as a network segment
 */
Nsegment *
npoint_to_nsegment(const Npoint *np)
{
  return nsegment_make(np->rid, np->pos, np->pos);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @brief Return the route of the network point
 */
int64
npoint_route(const Npoint *np)
{
  return np->rid;
}

/**
 * @brief Return the position of the network point
 */
double
npoint_position(const Npoint *np)
{
  return np->pos;
}

/**
 * @brief Return the route of the network segment
 */
int64
nsegment_route(const Nsegment *ns)
{
  return ns->rid;
}

/**
 * @brief Return the start position of the network segment
 */
double
nsegment_start_position(const Nsegment *ns)
{
  return ns->pos1;
}

/**
 * @brief Return the end position of the network segment
 */
double
nsegment_end_position(const Nsegment *ns)
{
  return ns->pos2;
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
    elog(ERROR, "Cannot get the length for route %ld", rid);

  return result;
}

/**
 * Access the edge table to get the route geometry from corresponding route
 * identifier
 */
GSERIALIZED *
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
      result = (GSERIALIZED *) SPI_palloc(gs->size);
      memcpy(result, gs, gs->size);
    }
  }
  SPI_finish();

  if (isNull)
    elog(ERROR, "Cannot get the geometry for route %ld", rid);

  ensure_non_empty(result);

  return result;
}

#if 0 /* not used */
/**
 * Access edge table to get the rid from corresponding geometry
 */
int64
rid_from_geom(Datum geom)
{
  char *geomstr = ewkt_out(geom, 0, OUT_DEFAULT_DECIMAL_DIGITS);
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
    elog(ERROR, "Cannot get route identifier from geometry point");

  return result;
}
#endif

/**
 * @brief Transforms the network point into a geometry
 */
GSERIALIZED *
npoint_geom(const Npoint *np)
{
  GSERIALIZED *line = route_geom(np->rid);
  GSERIALIZED *result = gserialized_line_interpolate_point(line, np->pos, 0);
  pfree(line);
  return result;
}

/**
 * @brief Transforms the geometry into a network point
 */
Npoint *
geom_npoint(const GSERIALIZED *gs)
{
  /* Ensure validity of operation */
  ensure_non_empty(gs);
  ensure_point_type(gs);
  int32_t srid_geom = gserialized_get_srid(gs);
  int32_t srid_ways = get_srid_ways();
  ensure_same_srid(srid_geom, srid_ways);

  char *geomstr = ewkt_out(PointerGetDatum(gs), 0, OUT_DEFAULT_DECIMAL_DIGITS);
  char sql[512];
  sprintf(sql, "SELECT npoint(gid, ST_LineLocatePoint(the_geom, '%s')) "
    "FROM public.ways WHERE ST_DWithin(the_geom, '%s', %lf) "
    "ORDER BY ST_Distance(the_geom, '%s') LIMIT 1", geomstr, geomstr,
    DIST_EPSILON, geomstr);
  pfree(geomstr);
  Npoint *result = palloc(sizeof(Npoint));
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
 * @brief Transforms the network segment into a geometry
 */
GSERIALIZED *
nsegment_geom(const Nsegment *ns)
{
  GSERIALIZED *line = route_geom(ns->rid);
  GSERIALIZED *result;
  if (fabs(ns->pos1 - ns->pos2) < MEOS_EPSILON)
    result = gserialized_line_interpolate_point(line, ns->pos1, 0);
  else
    result = gserialized_line_substring(line, ns->pos1, ns->pos2);
  pfree(line);
  return result;
}

/**
 * @brief Transforms the geometry into a network segment
 */
Nsegment *
geom_nsegment(const GSERIALIZED *gs)
{
  /* Ensure validity of operation */
  ensure_non_empty(gs);
  int geomtype = gserialized_get_type(gs);
  if (geomtype != POINTTYPE && geomtype != LINETYPE)
    elog(ERROR, "Only point or line geometries accepted");

  Npoint **points;
  Npoint *np;
  int npoints = 0;
  if (geomtype == POINTTYPE)
  {
    points = palloc0(sizeof(Npoint *));
    np = geom_npoint(gs);
    if (np != NULL)
      points[npoints++] = np;
  }
  else /* geomtype == LINETYPE */
  {
    int numpoints = gserialized_numpoints_linestring(gs);
    points = palloc0(sizeof(Npoint *) * numpoints);
    for (int i = 0; i < numpoints; i++)
    {
      /* The composing points are from 1 to numcount */
      GSERIALIZED *point = gserialized_pointn_linestring(gs, i + 1);
      np = geom_npoint(point);
      if (np != NULL)
        points[npoints++] = np;
      /* Cannot pfree(point); */
    }
  }

  if (npoints == 0)
  {
    pfree(points);
    return NULL;
  }
  int64 rid = points[0]->rid;
  double minPos = points[0]->pos, maxPos = points[0]->pos;
  for (int i = 1; i < npoints; i++)
  {
    if (points[i]->rid != rid)
    {
      pfree_array((void **) points, npoints);
      return NULL;
    }
    minPos = Min(minPos, points[i]->pos);
    maxPos = Max(maxPos, points[i]->pos);
  }
  Nsegment *result = nsegment_make(rid, minPos, maxPos);
  pfree_array((void **) points, npoints);
  return result;
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

/**
 * @brief Return the SRID of the network point
 */
int
npoint_srid(const Npoint *np)
{
  GSERIALIZED *line = route_geom(np->rid);
  int result = gserialized_get_srid(line);
  pfree(line);
  return result;
}

/**
 * @brief Return the SRID of the network segment.
 */
int
nsegment_srid(const Nsegment *ns)
{
  GSERIALIZED *line = route_geom(ns->rid);
  int result = gserialized_get_srid(line);
  pfree(line);
  return result;
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/**
 * @brief Return true if the first network point is equal to the second one
 */
bool
npoint_eq(const Npoint *np1, const Npoint *np2)
{
  return np1->rid == np2->rid && fabs(np1->pos - np2->pos) < MEOS_EPSILON;
}

/**
 * @brief Return true if the first network point is not equal to the second one
 */
bool
npoint_ne(const Npoint *np1, const Npoint *np2)
{
  return (!npoint_eq(np1, np2));
}

/**
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
 * @brief Return true if the first network point is less than the second one
 */
bool
npoint_lt(const Npoint *np1, const Npoint *np2)
{
  int cmp = npoint_cmp(np1, np2);
  return (cmp < 0);
}

/**
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
 * @brief Return true if the first network point is greater than the second one
 */
bool
npoint_gt(const Npoint *np1, const Npoint *np2)
{
  int cmp = npoint_cmp(np1, np2);
  return (cmp > 0);
}

/**
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
 * @brief Return true if the first network segment is equal to the second one
 */
bool
nsegment_eq(const Nsegment *ns1, const Nsegment *ns2)
{
  return ns1->rid == ns2->rid && fabs(ns1->pos1 - ns2->pos1) < MEOS_EPSILON &&
    fabs(ns1->pos2 - ns2->pos2) < MEOS_EPSILON;
}

/**
 * @brief Return true if the first network segment is not equal to the second one
 */
bool
nsegment_ne(const Nsegment *ns1, const Nsegment *ns2)
{
  return (!nsegment_eq(ns1, ns2));
}

/**
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
 * @brief Return true if the first network segment is less than the second one
 */
bool
nsegment_lt(const Nsegment *ns1, const Nsegment *ns2)
{
  int cmp = nsegment_cmp(ns1, ns2);
  return (cmp < 0);
}

/**
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
 * @brief Return true if the first network segment is greater than the second one
 */
bool
nsegment_gt(const Nsegment *ns1, const Nsegment *ns2)
{
  int cmp = nsegment_cmp(ns1, ns2);
  return (cmp > 0);
}

/**
 * @brief Return true if the first network segment is greater than or equal to
 * the second one
 */
bool
nsegment_ge(const Nsegment *ns1, const Nsegment *ns2)
{
  int cmp = nsegment_cmp(ns1, ns2);
  return (cmp >= 0);
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for span types for combining the hash of
 * the lower and upper bounds.
 *****************************************************************************/

/**
 * @brief Return the 32-bit hash value of a network point.
 */
uint32
npoint_hash(const Npoint *np)
{
  /* Compute hashes of value and position */
  uint32 rid_hash = pg_hashint8(np->rid);
  uint32 pos_hash = pg_hashfloat8(np->pos);

  /* Merge hashes of value and position */
  uint32 result = rid_hash;
  result = (result << 1) | (result >> 31);
  result ^= pos_hash;
  return result;
}

/**
 * @brief Return the 32-bit hash value of a network point.
 */
uint64
npoint_hash_extended(const Npoint *np, uint64 seed)
{
  /* Compute hashes of value and position */
  uint64 rid_hash = pg_hashint8extended(np->rid, seed);
  uint64 pos_hash = pg_hashfloat8extended(np->pos, seed);

  /* Merge hashes of value and position */
  uint64 result = rid_hash;
  result = (result << 1) | (result >> 31);
  result ^= pos_hash;
  return result;
}

/*****************************************************************************/
