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
 * @brief Network-based static point and segment types
 */

#include "npoint/tnpoint.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#if ! MEOS
  #include <libpq/pqformat.h>
  #include <executor/spi.h>
#endif /* ! MEOS */
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_npoint.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/span.h"
#include "general/tsequence.h"
#include "general/type_inout.h"
#include "general/type_parser.h"
#include "general/type_util.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo.h"
#include "geo/tgeo_out.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_parser.h"
#include "npoint/tnpoint_parser.h"
#include "npoint/tnpoint.h"

/** Buffer size for input and output of npoint and nsegment values */
#define NPOINT_MAXLEN     128
#define NSEGMENT_MAXLEN   128

/* Global variable saving the SRID of the ways table */
static int32_t SRID_WAYS = SRID_INVALID;

/*****************************************************************************
 * Collinear function
 *****************************************************************************/

/**
 * @brief Return true if the three values are collinear
 * @param[in] np1,np2,np3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the
 * timestamps associated to `np1` and `np2` divided by the duration
 * of the timestamps associated to `np1` and `np3`
 */
bool
npoint_collinear(const Npoint *np1, const Npoint *np2, const Npoint *np3, 
  double ratio)
{
  assert(np1->rid == np2->rid); assert(np1->rid == np3->rid);
  return float_collinear(np1->pos, np2->pos, np3->pos, ratio);
}

/*****************************************************************************
 * Definitions for reading the ways.csv file
 *****************************************************************************/

#if MEOS
/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LENGTH_HEADER 1024
/* Maximum length in characters of a geometry in the input data */
#define MAX_LENGTH_GEOM 100001
/* Location of the ways.csv file */
#define WAYS_CSV "/home/esteban/src/MobilityDB/meos/examples/data/ways.csv"

typedef struct
{
  long int gid;
  GSERIALIZED *the_geom;
} ways_record;
#endif /* MEOS */

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @ingroup meos_npoint_base_accessor
 * @brief Return the SRID of the routes in the ways table
 * @return On error return SRID_INVALID
 */
#if MEOS
int32_t
get_srid_ways()
{
  /* Get the value from the global variable if it has been already set */
  if (SRID_WAYS != SRID_INVALID)
    return SRID_WAYS;

  /* Substitute the full file path in the first argument of fopen */
  FILE *file = fopen(WAYS_CSV, "r");

  if (! file)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Cannot open the ways.csv file");
    return SRID_INVALID;
  }

  bool found = false;
  ways_record rec;
  char header_buffer[MAX_LENGTH_HEADER];
  char geo_buffer[MAX_LENGTH_GEOM];

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", header_buffer);

  /* Continue reading the file */
  do
  {
    int read = fscanf(file, "%ld,%100000[^\n]\n",
      &rec.gid, geo_buffer);

    if (ferror(file))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Error reading the ways.csv file");
      return SRID_INVALID;
    }

    /* Ignore the records with NULL values */
    if (read == 3)
    {
      /* Transform the string representing the geometry into a geometry value */
      rec.the_geom = geom_in(geo_buffer, -1);
      if (geo_is_empty(rec.the_geom))
      {
        free(rec.the_geom);
        continue;
      }
      found = true;
      break;
    }
  } while (!feof(file));

  /* Close the input file */
  fclose(file);
  
  if (! found)
    return SRID_INVALID;
  
  int32_t result = gserialized_get_srid(rec.the_geom);
  free(rec.the_geom);
  /* Save the SRID value in a global variable */
  SRID_WAYS = result;
  return result;  
}
#else
int32_t
get_srid_ways()
{
  /* Get the value from the global variable if it has been already set */
  if (SRID_WAYS != SRID_INVALID)
    return SRID_WAYS;
  
  /* Fetch the SRID value from the table */
  int32_t result = 0; /* make compiler quiet */
  bool isNull = true;
  SPI_connect();
  int ret = SPI_execute("SELECT ST_SRID(the_geom) FROM public.ways LIMIT 1;",
    true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    Datum value = SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 1, &isNull);
    if (isNull)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Cannot determine SRID of the ways table");
      return SRID_INVALID;
    }
    result = DatumGetInt32(value);
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Cannot determine SRID of the ways table");
    return SRID_INVALID;
  }
  SPI_finish();
  /* Save the SRID value in a global variable */
  SRID_WAYS = result;
  return result;
}
#endif /* MEOS */

#define SQL_ROUTE_MAXLEN 64

/**
 * @ingroup meos_npoint_base_accessor
 * @brief Return true if the edge table contains a route with the route
 * identifier
 * @param[in] rid Route identifier
 */
#if MEOS
bool
route_exists(int64 rid)
{
  /* Substitute the full file path in the first argument of fopen */
  FILE *file = fopen(WAYS_CSV, "r");

  if (! file)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Cannot open the ways.csv file");
    return false;
  }

  bool result = false;
  ways_record rec;
  char header_buffer[MAX_LENGTH_HEADER];
  char geo_buffer[MAX_LENGTH_GEOM];

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", header_buffer);

  /* Continue reading the file */
  do
  {
    int read = fscanf(file, "%ld,%100000[^\n]\n",
      &rec.gid, geo_buffer);

    if (ferror(file))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Error reading the ways.csv file");
      return false;
    }

    /* Ignore the records with NULL values */
    if (read == 2)
    {
      /* Transform the string representing the geometry into a geometry value */
      rec.the_geom = geom_in(geo_buffer, -1);
      if (geo_is_empty(rec.the_geom))
      {
        free(rec.the_geom);
        continue;
      }
      if (rec.gid == rid)
      {
        result = true;
        break;
      }
    }
  } while (!feof(file));

  /* Close the input file */
  fclose(file);
  
  return result; 
}
#else
bool
route_exists(int64 rid)
{
  char sql[SQL_ROUTE_MAXLEN];
  snprintf(sql, sizeof(sql),
    "SELECT true FROM public.ways WHERE gid = %ld", rid);
  bool isNull = true;
  bool result = false;
  SPI_connect();
  int ret = SPI_execute(sql, true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    result = DatumGetBool(SPI_getbinval(tuptable->vals[0],
      tuptable->tupdesc, 1, &isNull));
  }
  SPI_finish();
  return result;
}
#endif /* MEOS */

/**
 * @ingroup meos_npoint_base_accessor
 * @brief Access the edge table to return the route length from the
 * corresponding route identifier
 * @param[in] rid Route identifier
 * @return On error return -1.0
 */
#if MEOS
double
route_length(int64 rid)
{
  /* Substitute the full file path in the first argument of fopen */
  FILE *file = fopen(WAYS_CSV, "r");

  if (! file)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Cannot open the ways.csv file");
    return -1.0;
  }

  bool found = false;
  ways_record rec;
  char header_buffer[MAX_LENGTH_HEADER];
  char geo_buffer[MAX_LENGTH_GEOM];

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", header_buffer);

  /* Continue reading the file */
  do
  {
    int read = fscanf(file, "%ld,%100000[^\n]\n",
      &rec.gid, geo_buffer);

    if (ferror(file))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Error reading the ways.csv file");
      return -1.0;
    }

    /* Ignore the records with NULL values */
    if (read == 2)
    {
      /* Transform the string representing the geometry into a geometry value */
      rec.the_geom = geom_in(geo_buffer, -1);
      if (geo_is_empty(rec.the_geom))
      {
        free(rec.the_geom);
        continue;
      }
      if (rec.gid == rid)
      {
        found = true;
        break;
      }
    }
  } while (!feof(file));

  /* Close the input file */
  fclose(file);
  
  if (! found)
    return -1.0;
  
  double result = geom_length(rec.the_geom);
  free(rec.the_geom);
  return result; 
}
#else
double
route_length(int64 rid)
{
  char sql[SQL_ROUTE_MAXLEN];
  snprintf(sql, sizeof(sql),
    "SELECT length FROM public.ways WHERE gid = %ld", rid);
  bool isNull = true;
  double result = 0.0;
  SPI_connect();
  int ret = SPI_execute(sql, true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    result = DatumGetFloat8(SPI_getbinval(tuptable->vals[0],
      tuptable->tupdesc, 1, &isNull));
  }
  SPI_finish();

  if (isNull)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot get the length for route %ld", rid);
    return -1.0;
  }
  return result;
}
#endif /* MEOS */

/**
 * @ingroup meos_npoint_base_accessor
 * @brief Access the edge table to get the route geometry from corresponding
 * route identifier
 * @param[in] rid Route identifier
 * @return On error return @p NULL
 */
#if MEOS
GSERIALIZED *
route_geom(int64 rid)
{
  /* Substitute the full file path in the first argument of fopen */
  FILE *file = fopen(WAYS_CSV, "r");

  if (! file)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Cannot open the ways.csv file");
    return NULL;
  }

  bool found = false;
  ways_record rec;
  char header_buffer[MAX_LENGTH_HEADER];
  char geo_buffer[MAX_LENGTH_GEOM];

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", header_buffer);

  /* Continue reading the file */
  do
  {
    int read = fscanf(file, "%ld,%100000[^\n]\n",
      &rec.gid, geo_buffer);

    if (ferror(file))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Error reading the ways.csv file");
      return NULL;
    }

    /* Ignore the records with NULL values */
    if (read == 3)
    {
      /* Transform the string representing the geometry into a geometry value */
      rec.the_geom = geom_in(geo_buffer, -1);
      if (geo_is_empty(rec.the_geom))
      {
        free(rec.the_geom);
        continue;
      }
      if (rec.gid == rid)
      {
        found = true;
        break;
      }
    }
  } while (!feof(file));

  /* Close the input file */
  fclose(file);
  
  if (! found)
    return NULL;
  
  GSERIALIZED *result = geo_copy(rec.the_geom);
  free(rec.the_geom);
  return result; 
}
#else
GSERIALIZED *
route_geom(int64 rid)
{
  char sql[SQL_ROUTE_MAXLEN];
  snprintf(sql, sizeof(sql),
    "SELECT the_geom FROM public.ways WHERE gid = %ld", rid);
  bool isNull = true;
  GSERIALIZED *result = NULL;
  SPI_connect();
  int ret = SPI_execute(sql, true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    Datum line = SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 1, &isNull);
    if (! isNull)
    {
      /* Must allocate this in upper executor context to keep it alive after SPI_finish() */
      GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(line);
      result = (GSERIALIZED *) SPI_palloc(gs->size);
      memcpy(result, gs, gs->size);
    }
  }
  SPI_finish();

  if (isNull)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot get the geometry for route %ld", rid);
    return NULL;
  }
  if (! ensure_not_empty(result))
  {
    pfree(result);
    return NULL;
  }
  return result;
}
#endif /* MEOS */

#define SQL_MAXLEN 1024

/**
 * @ingroup meos_npoint_base_conversion
 * @brief Transform a geometry into a network point
 * @param[in] gs Geometry
 * @csqlfn #Geom_to_npoint()
 */
#if MEOS
Npoint *
geom_npoint(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_empty(gs) || ! ensure_point_type(gs))
    return NULL;
  int32_t srid_geom = gserialized_get_srid(gs);
  int32_t srid_ways = get_srid_ways();
  if (srid_ways == SRID_INVALID || ! ensure_same_srid(srid_geom, srid_ways))
    return NULL;

  /* Substitute the full file path in the first argument of fopen */
  FILE *file = fopen(WAYS_CSV, "r");

  if (! file)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Cannot open the ways.csv file");
    return NULL;
  }

  /* Record holding one line of the file */
  ways_record rec;
  /* Buffers for reading one line of the file */
  char header_buffer[MAX_LENGTH_HEADER];
  char geo_buffer[MAX_LENGTH_GEOM];
  /* Distances */
  double dist, min_dist = DBL_MAX;
  /* Position in the geometry with the shortest distance */
  double pos;

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", header_buffer);

  /* Continue reading the file */
  do
  {
    int read = fscanf(file, "%ld,%100000[^\n]\n",
      &rec.gid, geo_buffer);

    if (ferror(file))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Error reading the ways.csv file");
      return NULL;
    }

    /* Ignore the records with NULL values */
    if (read == 3)
    {
      /* Transform the string representing the geometry into a geometry value */
      rec.the_geom = geom_in(geo_buffer, -1);
      if (geo_is_empty(rec.the_geom))
      {
        free(rec.the_geom);
        continue;
      }

      /* We need to implement the following SQL query for a given geo
       *   SELECT npoint(gid, ST_LineLocatePoint(the_geom, geo))
       *   FROM public.ways WHERE ST_DWithin(the_geom, geo, DIST_EPSILON)
       *   ORDER BY ST_Distance(the_geom, geo) LIMIT 1;
       */
      
      pos = line_locate_point(rec.the_geom, gs);
      if (pos < 0)
      {
        free(rec.the_geom);
        continue;
      }

      dist = geom_distance2d(rec.the_geom, gs);
      if (dist < min_dist)
        min_dist = dist;

    }    
  } while (! feof(file));

  /* Close the input file */
  fclose(file);
  
  /* If the point was not found */
  if (! gs)
    return NULL;
  
  Npoint *result = npoint_make(rec.gid, pos);
  free(rec.the_geom);
  return result; 
}
#else
Npoint *
geom_npoint(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_empty(gs) || ! ensure_point_type(gs))
    return NULL;
  int32_t srid_geom = gserialized_get_srid(gs);
  int32_t srid_ways = get_srid_ways();
  if (srid_ways == SRID_INVALID || ! ensure_same_srid(srid_geom, srid_ways))
    return NULL;

  char *geomstr = geo_as_wkt(gs, OUT_DEFAULT_DECIMAL_DIGITS, true);
  char sql[SQL_MAXLEN];
  snprintf(sql, sizeof(sql),
    "SELECT npoint(gid, ST_LineLocatePoint(the_geom, '%s')) "
    "FROM public.ways WHERE ST_DWithin(the_geom, '%s', %lf) "
    "ORDER BY ST_Distance(the_geom, '%s') LIMIT 1", geomstr, geomstr,
    DIST_EPSILON, geomstr);
  pfree(geomstr);
  Npoint *result = palloc(sizeof(Npoint));
  bool isNull = true;
  SPI_connect();
  int ret = SPI_execute(sql, true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    Datum value = SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 1, &isNull);
    if (! isNull)
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
#endif /* MEOS */

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensure that a route exists in the ways table
 */
bool
ensure_route_exists(int64 rid)
{
  if (! route_exists(rid))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "There is no route with gid value %ld in table ways", rid);
    return false;
  }
  return true;
}

/**
 * @brief Ensure valid position
 */
bool
ensure_valid_position(double pos)
{
  if (pos < 0 || pos > 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The relative position must be a real number between 0 and 1");
    return false;
  }
  return true;
}

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Comparator function for network segments
 */
static int
nsegment_sort_cmp(Nsegment **l, Nsegment **r)
{
  return nsegment_cmp(*l, *r);
}

/**
 * @brief Sort function for network segments
 */
static void
nsegmentarr_sort(Nsegment **segments, int count)
{
  qsort(segments, (size_t) count, sizeof(Nsegment *),
      (qsort_comparator) &nsegment_sort_cmp);
  return;
}

/**
 * @brief Normalize an array of temporal segments
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

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @brief Parse a network point from its string representation
 */
Npoint *
npoint_parse(const char **str, bool end)
{
  const char *type_str = meostype_name(T_NPOINT);

  /* Determine whether there is an SRID */
  int32_t srid;
  srid_parse(str, &srid);

  /* Parse prefix */
  p_whitespace(str);
  if (pg_strncasecmp(*str, "NPOINT", 6) != 0)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Missing prefix 'NPoint'", type_str);
    return NULL;
  }

  *str += 6;
  p_whitespace(str);

  /* Parse opening parenthesis */
  if (! ensure_oparen(str, type_str))
    return NULL;

  /* Parse rid */
  p_whitespace(str);
  Datum d;
  if (! basetype_parse(str, T_INT8, ',', &d)) 
    return NULL;
  int64 rid = DatumGetInt64(d);

  p_comma(str);

  /* Parse pos */
  p_whitespace(str);
  if (! basetype_parse(str, T_FLOAT8, ')', &d))
    return NULL;
  double pos = DatumGetFloat8(d);
  if (pos < 0 || pos > 1)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "The relative position must be a real number between 0 and 1");
    return NULL;
  }

  /* Parse closing parenthesis */
  p_whitespace(str);
  if (! ensure_cparen(str, type_str) ||
        (end && ! ensure_end_input(str, type_str)))
    return NULL;

  return npoint_make(rid, pos);
}

#if MEOS
/**
 * @ingroup meos_npoint_base_inout
 * @brief Return a network point from its string representation
 * @param[in] str String
 * @csqlfn #Npoint_in()
 */
Npoint *
npoint_in(const char *str)
{
  VALIDATE_NOT_NULL(str, NULL);
  return npoint_parse(&str, true);
}
#endif /* MEOS */

/**
 * @ingroup meos_npoint_base_inout
 * @brief Return the string representation of a network point
 * @param[in] np Network point
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Npoint_out()
 */
char *
npoint_out(const Npoint *np, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  char *result = palloc(NPOINT_MAXLEN);
  char *rid = int8_out(np->rid);
  char *pos = float8_out(np->pos, maxdd);
  snprintf(result, NPOINT_MAXLEN, "NPoint(%s,%s)", rid, pos);
  pfree(rid); pfree(pos);
  return result;
}

/*****************************************************************************/

/**
 * @brief Parse a network segment from its string representation
 */
Nsegment *
nsegment_parse(const char **str)
{
  const char *type_str = meostype_name(T_NSEGMENT);
  p_whitespace(str);

  if (pg_strncasecmp(*str, "NSEGMENT", 8) != 0)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Missing prefix 'NSegment'", type_str);
    return NULL;
  }

  *str += 8;
  p_whitespace(str);

  /* Parse opening parenthesis */
  if (! ensure_oparen(str, type_str))
    return NULL;

  /* Parse rid */
  p_whitespace(str);
  Datum d;
  if (! basetype_parse(str, T_INT8, ',', &d))
    return NULL;
  int64 rid = DatumGetInt64(d);

  p_comma(str);

  /* Parse pos1 */
  p_whitespace(str);
  if (! basetype_parse(str, T_FLOAT8, ',', &d))
    return NULL;
  double pos1 = DatumGetFloat8(d);
  if (pos1 < 0 || pos1 > 1)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "The relative position must be a real number between 0 and 1");
    return NULL;
  }
  p_comma(str);

  /* Parse pos2 */
  p_whitespace(str);
  if (! basetype_parse(str, T_FLOAT8, ')', &d))
    return NULL;
  double pos2 = DatumGetFloat8(d);
  if (pos2 < 0 || pos2 > 1)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "The relative position must be a real number between 0 and 1");
    return NULL;
  }

  /* Parse closing parenthesis */
  p_whitespace(str);
  if (! ensure_cparen(str, type_str) || ! ensure_end_input(str, type_str))
    return NULL;

  return nsegment_make(rid, pos1, pos2);
}

/**
 * @ingroup meos_npoint_base_inout
 * @brief Return a network point from its string representation
 * @param[in] str String
 * @csqlfn #Nsegment_in()
 */
Nsegment *
nsegment_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return nsegment_parse(&str);
}

/**
 * @ingroup meos_npoint_base_inout
 * @brief Return the string representation of a network segment
 * @param[in] ns Network segment
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Nsegment_out()
 */
char *
nsegment_out(const Nsegment *ns, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ns, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  char *result = palloc(NSEGMENT_MAXLEN);
  char *rid = int8_out(ns->rid);
  char *pos1 = float8_out(ns->pos1, maxdd);
  char *pos2 = float8_out(ns->pos2, maxdd);
  snprintf(result, NSEGMENT_MAXLEN, "NSegment(%s,%s,%s)", rid, pos1, pos2);
  pfree(rid); pfree(pos1); pfree(pos2);
  return result;
}

/*****************************************************************************
 * WKT and EWKT output functions for network points
 *****************************************************************************/

/**
 * @brief Output a network point in the Well-Known Text (WKT) representation
 * (internal function)
 */
char *
npoint_wkt_out(Datum value, int maxdd)
{
  Npoint *np = DatumGetNpointP(value);
  char *rid = int8_out(np->rid);
  char *pos = float8_out(np->pos, maxdd);
  size_t len = strlen(rid) + strlen(pos) + 10; // Npoint(,) + end NULL
  char *result = palloc(len);
  snprintf(result, len, "NPoint(%s,%s)", rid, pos);
  pfree(rid); pfree(pos);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_npoint_base_inout
 * @brief Return the Well-Known Text (WKT) representation of a network point
 * @param[in] np Network point
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Npoint_as_text()
 */
char *
npoint_as_text(const Npoint *np, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  return npoint_wkt_out(PointerGetDatum(np), maxdd);
}

/**
 * @ingroup meos_npoint_base_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * network point
 * @param[in] np Network point
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Npoint_as_ewkt()
 */
char *
npoint_as_ewkt(const Npoint *np, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  int32_t srid = npoint_srid(np);
  char str1[18];
  if (srid > 0)
    /* SRID_MAXIMUM is defined by PostGIS as 999999 */
    snprintf(str1, sizeof(str1), "SRID=%d;", srid);
  else
    str1[0] = '\0';
  char *str2 = npoint_wkt_out(PointerGetDatum(np), maxdd);
  char *result = palloc(strlen(str1) + strlen(str2) + 1);
  strcpy(result, str1);
  strcat(result, str2);
  pfree(str2);
  return result;
}

/*****************************************************************************
 * WKB and HexWKB output functions for network points
 *****************************************************************************/

/**
 * @ingroup meos_npoint_base_inout
 * @brief Return a network point from its Well-Known Binary (WKB) 
 * representation
 * @param[in] wkb WKB string
 * @param[in] size Size of the string
 * @csqlfn #Npoint_recv(), #Npoint_from_wkb()
 */
Npoint *
npoint_from_wkb(const uint8_t *wkb, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb, NULL);
  return DatumGetNpointP(type_from_wkb(wkb, size, T_NPOINT));
}

/**
 * @ingroup meos_npoint_base_inout
 * @brief Return a network point from its hex-encoded ASCII Well-Known Binary
 * (WKB) representation
 * @param[in] hexwkb HexWKB string
 * @csqlfn #Npoint_from_hexwkb()
 */
Npoint *
npoint_from_hexwkb(const char *hexwkb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(hexwkb, NULL);
  size_t size = strlen(hexwkb);
  return DatumGetNpointP(type_from_hexwkb(hexwkb, size, T_NPOINT));
}

/*****************************************************************************/

#if MEOS
/**
 * @ingroup meos_npoint_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a circular
 * buffer
 * @param[in] np Network point
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Npoint_recv(), #Npoint_as_wkb()
 */
uint8_t *
npoint_as_wkb(const Npoint *np, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return datum_as_wkb(PointerGetDatum(np), T_NPOINT, variant, size_out);
}

/**
 * @ingroup meos_npoint_base_inout
 * @brief Return the hex-encoded ASCII Well-Known Binary (HexWKB)
 * representation of a network point
 * @param[in] np Network point
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Npoint_as_hexwkb()
 */
char *
npoint_as_hexwkb(const Npoint *np, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return (char *) datum_as_wkb(PointerGetDatum(np), T_NPOINT,
    variant | (uint8_t) WKB_HEX, size_out);
}
#endif /* MEOS */

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_npoint_base_constructor
 * @brief Return a network point from a route identifier and a position
 * @param[in] rid Route identifier
 * @param[in] pos Position
 * @csqlfn #Npoint_constructor()
 */
Npoint *
npoint_make(int64 rid, double pos)
{
  /* Ensure the validity of the arguments */
  if (! ensure_route_exists(rid) || ! ensure_valid_position(pos))
    return NULL;

  /* Note: zero-fill is done in the npoint_set function */
  Npoint *result = palloc(sizeof(Npoint));
  npoint_set(rid, pos, result);
  return result;
}

/**
 * @brief Return in the last argument a network point constructed from a route
 * identifier and a position
 */
void
npoint_set(int64 rid, double pos, Npoint *np)
{
  assert(route_exists(rid)); assert(pos >=0 && pos <= 1);
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(np, 0, sizeof(Npoint));
  /* Fill in the network point */
  np->rid = rid;
  np->pos = pos;
  return;
}

/**
 * @ingroup meos_npoint_base_constructor
 * @brief Return a network segment from a route identifier and two positions
 * @param[in] rid Route identifier
 * @param[in] pos1, pos2 Positions
 * @csqlfn #Nsegment_constructor()
 */
Nsegment *
nsegment_make(int64 rid, double pos1, double pos2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_route_exists(rid) || ! ensure_valid_position(pos1) ||
      ! ensure_valid_position(pos2))
    return NULL;

  /* Note: zero-fill is done in the nsegment_set function */
  Nsegment *result = palloc(sizeof(Nsegment));
  nsegment_set(rid, pos1, pos2, result);
  return result;
}

/**
 * @brief Return in the last argument a network segment constructed from a
 * route identifier and two positions
 */
void
nsegment_set(int64 rid, double pos1, double pos2, Nsegment *ns)
{
  assert(route_exists(rid)); 
  assert(pos1 >= 0 && pos1 <= 1 && pos2 >= 0 && pos2 <= 1);

  ns->rid = rid;
  ns->pos1 = Min(pos1, pos2);
  ns->pos2 = Max(pos1, pos2);
  return;
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_npoint_base_conversion
 * @brief Convert a network point into a network segment
 * @param[in] np Network point
 * @csqlfn #Npoint_to_nsegment()
 */
Nsegment *
npoint_nsegment(const Npoint *np)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np, NULL);
  return nsegment_make(np->rid, np->pos, np->pos);
}

/*****************************************************************************
 * Transform a temporal network point to a STBox
 *****************************************************************************/

/**
 * @ingroup meos_internal_box_constructor
 * @brief Return in the last argument the spatiotemporal box of a network point
 * @param[in] np Network point
 * @param[out] box Spatiotemporal box
 */
bool
npoint_set_stbox(const Npoint *np, STBox *box)
{
  GSERIALIZED *geom = npoint_geom(np);
  bool result = geo_set_stbox(geom, box);
  pfree(geom);
  return result;
}

/**
 * @ingroup meos_npoint_base_conversion
 * @brief Convert a network point into a spatiotemporal box
 * @param[in] np Network point
 * @csqlfn #Npoint_to_stbox()
 */
STBox *
npoint_stbox(const Npoint *np)
{
  STBox box;
  if (! npoint_set_stbox(np, &box))
    return NULL;
  return stbox_copy(&box);
}

/**
 * @ingroup meos_internal_box_constructor
 * @brief Return in the last argument a spatiotemporal box constructed from
 * an array of network points
 * @param[in] values Network points
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
npointarr_set_stbox(const Datum *values, int count, STBox *box)
{
  npoint_set_stbox(DatumGetNpointP(values[0]), box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    npoint_set_stbox(DatumGetNpointP(values[i]), &box1);
    stbox_expand(&box1, box);
  }
  return;
}

/**
 * @ingroup meos_internal_npoint_accessor
 * @brief Return the bounding box of the network segment value
 * @param[in] ns Network segment
 * @param[out] box Spatiotemporal box
 */
bool
nsegment_set_stbox(const Nsegment *ns, STBox *box)
{
  GSERIALIZED *geom = nsegment_geom(ns);
  bool result = geo_set_stbox(geom, box);
  pfree(geom);
  return result;
}

/**
 * @ingroup meos_npoint_base_conversion
 * @brief Convert a network segment into a spatiotemporal box
 * @param[in] ns Network segment
 * @csqlfn #Nsegment_to_stbox()
 */
STBox *
nsegment_stbox(const Nsegment *ns)
{
  STBox box;
  if (! nsegment_set_stbox(ns, &box))
    return NULL;
  return stbox_copy(&box);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_box_constructor
 * @brief Return in the last argument a spatiotemporal box constructed from a
 * network point and a timestamptz
 * @param[in] np Network point
 * @param[in] t Timestamp
 * @param[out] box Spatiotemporal box
 */
bool
npoint_timestamptz_set_stbox(const Npoint *np, TimestampTz t, STBox *box)
{
  npoint_set_stbox(np, box);
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return true;
}

/**
 * @ingroup meos_box_constructor
 * @brief Return a spatiotemporal box constructed from a network point and a
 * timestamptz
 * @param[in] np Network point
 * @param[in] t Timestamp
 * @csqlfn #Npoint_timestamptz_to_stbox()
 */
STBox *
npoint_timestamptz_to_stbox(const Npoint *np, TimestampTz t)
{
  VALIDATE_NOT_NULL(np, NULL);
  STBox box;
  if (! npoint_timestamptz_set_stbox(np, t, &box))
    return NULL;
  return stbox_copy(&box);
}

/**
 * @ingroup meos_internal_box_constructor
 * @brief Return in the last argument a spatiotemporal box constructed from a
 * network point and a timestamptz span
 * @param[in] np Network point
 * @param[in] s Timestamptz span
 * @param[out] box Spatiotemporal box
 */
bool
npoint_tstzspan_set_stbox(const Npoint *np, const Span *s, STBox *box)
{
  npoint_set_stbox(np, box);
  memcpy(&box->period, s, sizeof(Span));
  MEOS_FLAGS_SET_T(box->flags, true);
  return true;
}

/**
 * @ingroup meos_box_constructor
 * @brief Return a spatiotemporal box constructed from a network point and a
 * timestamptz
 * @param[in] np Network point
 * @param[in] s Timestamptz span
 * @csqlfn #Npoint_tstzspan_to_stbox()
 */
STBox *
npoint_tstzspan_to_stbox(const Npoint *np, const Span *s)
{
  VALIDATE_NOT_NULL(np, NULL); VALIDATE_TSTZSPAN(s, NULL);
  STBox box;
  if (! npoint_tstzspan_set_stbox(np, s, &box))
    return NULL;
  return stbox_copy(&box);
}

/*****************************************************************************
 * Conversion functions between network and Euclidean space
 *****************************************************************************/

/**
 * @ingroup meos_npoint_base_conversion
 * @brief Transform a network point into a geometry
 * @param[in] np Network point
 * @csqlfn #Npoint_to_geom()
 */
GSERIALIZED *
npoint_geom(const Npoint *np)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np, NULL);
  GSERIALIZED *line = route_geom(np->rid);
  GSERIALIZED *result = line_interpolate_point(line, np->pos, 0);
  pfree(line);
  return result;
}

/**
 * @ingroup meos_npoint_base_conversion
 * @brief Transform a network segment into a geometry
 * @param[in] ns Network segment
 * @csqlfn #Nsegment_to_geom()
 */
GSERIALIZED *
nsegment_geom(const Nsegment *ns)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ns, NULL);
  GSERIALIZED *line = route_geom(ns->rid);
  GSERIALIZED *result;
  if (fabs(ns->pos1 - ns->pos2) < MEOS_EPSILON)
    result = line_interpolate_point(line, ns->pos1, 0);
  else
    result = line_substring(line, ns->pos1, ns->pos2);
  pfree(line);
  return result;
}

/**
 * @ingroup meos_npoint_base_conversion
 * @brief Transform a geometry into a network segment
 * @return On error return @p NULL
 * @param[in] gs Geometry
 * @csqlfn #Geom_to_nsegment()
 */
Nsegment *
geom_nsegment(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_empty(gs))
    return NULL;
  int geomtype = gserialized_get_type(gs);
  if (geomtype != POINTTYPE && geomtype != LINETYPE)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Only point or line geometries accepted");
    return NULL;
  }

  Npoint **points;
  Npoint *np;
  int npoints = 0;
  if (geomtype == POINTTYPE)
  {
    points = palloc0(sizeof(Npoint *));
    np = geom_npoint(gs);
    if (np)
      points[npoints++] = np;
  }
  else /* geomtype == LINETYPE */
  {
    int numpoints = line_numpoints(gs);
    points = palloc0(sizeof(Npoint *) * numpoints);
    for (int i = 0; i < numpoints; i++)
    {
      /* The composing points are from 1 to numcount */
      GSERIALIZED *point = line_point_n(gs, i + 1);
      np = geom_npoint(point);
      if (np)
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
  double minPos, maxPos;
  minPos = maxPos = points[0]->pos;
  for (int i = 1; i < npoints; i++)
  {
    if (points[i]->rid != rid)
    {
      pfree_array((void **) points, i);
      return NULL;
    }
    minPos = Min(minPos, points[i]->pos);
    maxPos = Max(maxPos, points[i]->pos);
  }
  Nsegment *result = nsegment_make(rid, minPos, maxPos);
  pfree_array((void **) points, npoints);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_npoint_conversion
 * @brief Return a geometry converted from an array of network points
 * @param[in] points Array of network points
 * @param[in] count Number of elements in the input array
 * @pre The argument @p count is greater than 1, all points have the same SRID
 */
GSERIALIZED *
npointarr_geom(Npoint **points, int count)
{
  assert(count > 1);
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * count);
  int32_t srid = npoint_srid(points[0]);
  for (int i = 0; i < count; i++)
  {
    GSERIALIZED *gsline = route_geom(points[i]->rid);
    assert(gserialized_get_srid(gsline) == srid);
    LWGEOM *line = lwgeom_from_gserialized(gsline);
    geoms[i] = lwgeom_line_interpolate_point(line, points[i]->pos, srid, 0);
    pfree(gsline); pfree(line);
  }
  int newcount;
  LWGEOM **newgeoms = lwpointarr_remove_duplicates(geoms, count, &newcount);
  LWGEOM *geom = lwpointarr_make_trajectory(newgeoms, newcount, STEP);
  GSERIALIZED *result = geo_serialize(geom);
  pfree(newgeoms); pfree(geom);
  pfree_array((void **) geoms, count);
  return result;
}

/**
 * @ingroup meos_internal_npoint_conversion
 * @brief Return a geometry converted from an array of network segments
 * @param[in] segments Array of network segments
 * @param[in] count Number of elements in the input array
 * @pre The argument @p count is greater than 1
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
      geoms[i] = geo_copy(line);
    else if (segments[i]->pos1 == segments[i]->pos2)
      geoms[i] = line_interpolate_point(line, segments[i]->pos1, 0);
    else
      geoms[i] = line_substring(line, segments[i]->pos1, segments[i]->pos2);
    pfree(line);
  }
  GSERIALIZED *result = geom_array_union(geoms, count);
  pfree_array((void **) geoms, count);
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_npoint_base_transf
 * @brief Return a network point with the precision of the position set to a
 * number of decimal places
 */
Npoint *
npoint_round(const Npoint *np, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np, NULL);
  /* Set precision of position */
  double pos = float_round(np->pos, maxdd);
  return npoint_make(np->rid, pos);
}

/**
 * @brief Return a network point with the precision of the position set to a
 * number of decimal places
 * @note Funcion used by the lifting infrastructure
 */
Datum
datum_npoint_round(Datum npoint, Datum size)
{
  /* Set precision of position */
  return PointerGetDatum(npoint_round(DatumGetNpointP(npoint),
    DatumGetInt32(size)));
}

/**
 * @ingroup meos_npoint_base_transf
 * @brief Return a network segment with the precision of the positions set to a
 * number of decimal places
 */
Nsegment *
nsegment_round(const Nsegment *ns, int maxdd)
{
  VALIDATE_NOT_NULL(ns, NULL);
  /* Set precision of positions */
  double pos1 = float_round(ns->pos1, maxdd);
  double pos2 = float_round(ns->pos2, maxdd);
  return nsegment_make(ns->rid, pos1, pos2);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_npoint_base_accessor
 * @brief Return the route of a network point
 * @param[in] np Network point
 * @csqlfn #Npoint_route()
 */
int64
npoint_route(const Npoint *np)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np, LONG_MAX);
  return np->rid;
}

/**
 * @ingroup meos_npoint_base_accessor
 * @brief Return the position of a network point
 * @param[in] np Network point
 * @return On error return -1.0
 * @csqlfn #Npoint_position()
 */
double
npoint_position(const Npoint *np)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np, -1.0);
  return np->pos;
}

/**
 * @ingroup meos_npoint_base_accessor
 * @brief Return the route of a network segment
 * @param[in] ns Network segment
 * @csqlfn #Nsegment_route()
 */
int64
nsegment_route(const Nsegment *ns)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ns, LONG_MAX);
  return ns->rid;
}

/**
 * @ingroup meos_npoint_base_accessor
 * @brief Return the start position of a network segment
 * @param[in] ns Network segment
 * @csqlfn #Nsegment_start_position()
 */
double
nsegment_start_position(const Nsegment *ns)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ns, -1.0);
  return ns->pos1;
}

/**
 * @ingroup meos_npoint_base_accessor
 * @brief Return the end position of a network segment
 * @param[in] ns Network segment
 * @csqlfn #Nsegment_end_position()
 */
double
nsegment_end_position(const Nsegment *ns)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ns, -1.0);
  return ns->pos2;
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

/**
 * @ingroup meos_npoint_base_spatial
 * @brief Return the SRID of a network point
 * @param[in] np Network point
 * @csqlfn #Npoint_srid()
 * @note Since it is assumed that all network points share the same SRID which
 * is the one from the @p ways table, for performance reasons we simply get
 * the SRID of the table
 */
inline int32_t
npoint_srid(const Npoint *np __attribute__((unused)))
{
  return get_srid_ways();
}

/**
 * @ingroup meos_npoint_base_spatial
 * @brief Return the SRID of a network segment
 * @param[in] ns Network segment
 * @csqlfn #Nsegment_srid()
 * @note Since it is assumed that all network points share the same SRID which
 * is the one from the @p ways table, for performance reasons we simply get
 * the SRID of the table
 */
inline int32_t
nsegment_srid(const Nsegment *ns __attribute__((unused)))
{
  return get_srid_ways();
}

/*****************************************************************************
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

/**
 * @ingroup meos_npoint_base_comp
 * @brief Return true if the first network point is equal to the second one
 * @param[in] np1,np2 Network points
 * @csqlfn #Npoint_eq()
 */
bool
npoint_eq(const Npoint *np1, const Npoint *np2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np1, false); VALIDATE_NOT_NULL(np2, false);
  return np1->rid == np2->rid && fabs(np1->pos - np2->pos) < MEOS_EPSILON;
}

/**
 * @ingroup meos_npoint_base_comp
 * @brief Return true if the first network point is not equal to the second one
 * @param[in] np1,np2 Network points
 * @csqlfn #Npoint_ne()
 */
inline bool
npoint_ne(const Npoint *np1, const Npoint *np2)
{
  return ! npoint_eq(np1, np2);
}

/**
 * @ingroup meos_npoint_base_comp
 * @brief Return -1, 0, or 1 depending on whether the first network point
 * is less than, equal to, or greater than the second one
 * @param[in] np1,np2 Network points
 * @csqlfn #Npoint_cmp()
 */
int
npoint_cmp(const Npoint *np1, const Npoint *np2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np1, false); VALIDATE_NOT_NULL(np2, false);

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
 * @ingroup meos_npoint_base_comp
 * @brief Return true if the first network point is less than the second one
 * @param[in] np1,np2 Network points
 * @csqlfn #Npoint_lt()
 */
inline bool
npoint_lt(const Npoint *np1, const Npoint *np2)
{
  return npoint_cmp(np1, np2) < 0;
}

/**
 * @ingroup meos_npoint_base_comp
 * @brief Return true if the first network point is less than or equal to the
 * second one
 * @param[in] np1,np2 Network points
 * @csqlfn #Npoint_le()
 */
inline bool
npoint_le(const Npoint *np1, const Npoint *np2)
{
  return npoint_cmp(np1, np2) <= 0;
}

/**
 * @ingroup meos_npoint_base_comp
 * @brief Return true if the first network point is greater than the second one
 * @param[in] np1,np2 Network points
 * @csqlfn #Npoint_gt()
 */
inline bool
npoint_gt(const Npoint *np1, const Npoint *np2)
{
  return npoint_cmp(np1, np2) > 0;
}

/**
 * @ingroup meos_npoint_base_comp
 * @brief Return true if the first network point is greater than or equal to
 * the second one
 * @param[in] np1,np2 Network points
 * @csqlfn #Npoint_ge()
 */
inline bool
npoint_ge(const Npoint *np1, const Npoint *np2)
{
  return npoint_cmp(np1, np2) >= 0;
}

/*****************************************************************************/

/**
 * @ingroup meos_npoint_base_comp
 * @brief Return true if the first network segment is equal to the second one
 * @param[in] ns1,ns2 Network segments
 * @csqlfn #Nsegment_eq()
 */
bool
nsegment_eq(const Nsegment *ns1, const Nsegment *ns2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ns1, false); VALIDATE_NOT_NULL(ns2, false);
  return ns1->rid == ns2->rid && fabs(ns1->pos1 - ns2->pos1) < MEOS_EPSILON &&
    fabs(ns1->pos2 - ns2->pos2) < MEOS_EPSILON;
}

/**
 * @ingroup meos_npoint_base_comp
 * @brief Return true if the first network segment is not equal to the second
 * one
 * @param[in] ns1,ns2 Network segments
 * @csqlfn #Nsegment_ne()
 */
inline bool
nsegment_ne(const Nsegment *ns1, const Nsegment *ns2)
{
  return ! nsegment_eq(ns1, ns2);
}

/**
 * @ingroup meos_npoint_base_comp
 * @brief Return -1, 0, or 1 depending on whether the first network segment
 * is less than, equal to, or greater than the second one
 * @param[in] ns1,ns2 Network segments
 * @csqlfn #Nsegment_cmp()
 */
int
nsegment_cmp(const Nsegment *ns1, const Nsegment *ns2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ns1, false); VALIDATE_NOT_NULL(ns2, false);

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
 * @ingroup meos_npoint_base_comp
 * @brief Return true if the first network segment is less than the second one
 * @param[in] ns1,ns2 Network segments
 * @csqlfn #Nsegment_lt()
 */
inline bool
nsegment_lt(const Nsegment *ns1, const Nsegment *ns2)
{
  return nsegment_cmp(ns1, ns2) < 0;
}

/**
 * @ingroup meos_npoint_base_comp
 * @brief Return true if the first network segment is less than or equal to the
 * second one
 * @param[in] ns1,ns2 Network segments
 * @csqlfn #Nsegment_le()
 */
inline bool
nsegment_le(const Nsegment *ns1, const Nsegment *ns2)
{
  return nsegment_cmp(ns1, ns2) <= 0;
}

/**
 * @ingroup meos_npoint_base_comp
 * @brief Return true if the first network segment is greater than the second
 * one
 * @param[in] ns1,ns2 Network segments
 * @csqlfn #Nsegment_gt()
 */
inline bool
nsegment_gt(const Nsegment *ns1, const Nsegment *ns2)
{
  return nsegment_cmp(ns1, ns2) > 0;
}

/**
 * @ingroup meos_npoint_base_comp
 * @brief Return true if the first network segment is greater than or equal to
 * the second one
 * @param[in] ns1,ns2 Network segments
 * @csqlfn #Nsegment_ge()
 */
inline bool
nsegment_ge(const Nsegment *ns1, const Nsegment *ns2)
{
  return nsegment_cmp(ns1, ns2) >= 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for span types for combining the hash of
 * the lower and upper bounds.
 *****************************************************************************/

/**
 * @ingroup meos_npoint_base_accessor
 * @brief Return the 32-bit hash value of a network point
 * @param[in] np Network point
 */
uint32
npoint_hash(const Npoint *np)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np, INT_MAX);

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
 * @ingroup meos_npoint_base_accessor
 * @brief Return the 64-bit hash value of a network point using a seed
 * @param[in] np Network point
 * @param[in] seed Seed
 */
uint64
npoint_hash_extended(const Npoint *np, uint64 seed)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(np, LONG_MAX);

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
