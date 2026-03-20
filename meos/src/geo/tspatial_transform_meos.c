/**********************************************************************
 *
 * PostGIS - Spatial Types for PostgreSQL
 * http://postgis.net
 *
 * Copyright (C) 2001-2003 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public Licence. See the COPYING file.
 *
 **********************************************************************/

/**
 * @file
 * @brief Functions for transforming coordinates from one SRID to another one
 * @details PostGIS manages a cache of information obtained from PROJ to speed
 * up transformations (see file file libpgcommon/lw_transform.c).
 * The functions in this file are derived from PostGIS functions by perforning
 * memory allocation with `malloc` instead of using PostreSQL contexts.
 */

#include "geo/meos_transform.h"

/* C */
#include <float.h>
#include <string.h>
#include <stdio.h>
/* PostgreSQL */
#include <postgres.h>
#if ! MEOS
  #include <libpq/pqformat.h>
  #include <executor/spi.h>
#endif /* ! MEOS */
/* PostGIS */
#include <liblwgeom.h>
#if ! MEOS
  #include "lwgeom_pg.h"
#endif /* ! MEOS */
/* MEOS */
#include <meos.h>

#define MAX_PROJ_LEN  512

/*****************************************************************************
 * Data structures
 *****************************************************************************/

/**
 * @brief Size of the Proj cache
 */
#define PROJ_CACHE_ITEMS 128

/* An entry in the PROJ SRS cache */
typedef struct
{
  int32_t srid_from;
  int32_t srid_to;
  LWPROJ *projection;
  uint64_t last_used; /* LRU timestamp */
} MEOSPROJCacheItem;

/**
 * @brief The Proj4 cache holds a fixed number of reprojection entries
 * @details In normal usage we don't expect it to have many entries, so we
 * always linearly scan the list.
 * @note The structure is derived from PostGIS PROJSRSCache
 */
typedef struct
{
  MEOSPROJCacheItem items[PROJ_CACHE_ITEMS];
  uint32_t count;
  uint64_t clock;
  uint32_t last_index;   /* fast-path hint */
} MEOSPROJCache;

/* Global variable to hold the Proj object cache */
static _Thread_local MEOSPROJCache *MEOS_PROJ_CACHE = NULL;

/**
 * @brief Utility structure to get many potential string representations
 * from spatial_ref_sys query
 */
typedef struct
{
  char *authtext; /* auth_name:auth_srid */
  char *srtext;
  char *proj4text;
} PjStrs;

/*****************************************************************************
 * Definitions for reading the spatial_ref_sys.csv file
 *****************************************************************************/

#if MEOS
/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LEN_HEADER 1024
/* Maximum length in characters of a geometry in the input data */
#define MAX_LEN_SRS_RECORD 5120
/* Location of the spatial_ref_sys.csv file */
static char *SPATIAL_REF_SYS_CSV = "/usr/local/share/spatial_ref_sys.csv";

/**
 * @brief Set the location of the SPATIAL_REF_SYS_CSV files
 */
void
meos_set_spatial_ref_sys_csv(const char* path)
{
  SPATIAL_REF_SYS_CSV = strdup(path);
}

typedef struct
{
  char auth_name[256];
  int32_t auth_srid;
  char proj4text[2048];
  char srtext[2048];
} spatial_ref_sys_record;
#endif /* MEOS */

/*****************************************************************************/

#if ! MEOS
/**
 * @brief Return a copy of a string obtained from the database using SPI
 */
static char *
SPI_pstrdup(const char *str)
{
  char *ostr = NULL;
  if (str)
  {
    ostr = SPI_palloc(strlen(str)+1);
    strcpy(ostr, str);
  }
  return ostr;
}
#endif /* ! MEOS */

/**
 * @brief Return the PROJ strings of an SRID either from the CSV file
 * `spatial_ref_sys.csv` (for MEOS) or from the PostGIS table
 * `spatial_ref_sys` (for MobilityDB)
 * @note The PostGIS function is copied here since it is declared as `static`
 */
#if MEOS
static PjStrs
GetProjStringsSPI(int32_t srid)
{
  char header_buffer[MAX_LEN_HEADER];
  char auth_name[256];
  int32_t auth_srid;
  char proj4text[2048];
  char srtext[2048];

  PjStrs strs; // TODO palloc ?
  memset(&strs, 0, sizeof(strs));

  /* Substitute the full file path in the first argument of fopen */
  FILE *file = fopen(SPATIAL_REF_SYS_CSV, "r");
  if (! file)
  {
    printf("Cannot open the spatial_ref_sys.csv file (reading from %s)\n", SPATIAL_REF_SYS_CSV);
    return strs;
  }

  /* Read the first line of the file with the headers */
  int read = fscanf(file, "%1023s\n", header_buffer);

  /* Continue reading the file */
  bool found = false;
  do
  {
    /* Read each line from the file */
    read = fscanf(file, "%255[^,^\n],%d,%2047[^,^\n],%2047[^\n]\n",
      auth_name, &auth_srid, proj4text, srtext);

    if (ferror(file))
    {
      printf("Error reading the spatial_ref_sys.csv file");
      return strs;
    }

    /* Ignore the records with NULL values */
    if (read == 4 && auth_srid == srid)
    {
      char tmp[MAX_PROJ_LEN];
      snprintf(tmp, MAX_PROJ_LEN, "%s:%d", auth_name, auth_srid);
      strs.authtext = strdup(tmp);
      strs.proj4text = strdup(proj4text);
      strs.srtext = strdup(srtext);
      found = true;
      break;
    }
  } while (! feof(file));

  if (! found)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "Cannot find SRID (%d) in spatial_ref_sys", srid);
  }
  fclose(file);
  return strs;
}
#else
static PjStrs
GetProjStringsSPI(int32_t srid)
{
  int spi_result;
  char proj_spi_buffer[spibufferlen];
  PjStrs strs;
  memset(&strs, 0, sizeof(strs));

  /* Connect */
  spi_result = SPI_connect();
  if (spi_result != SPI_OK_CONNECT)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "Could not connect to database using SPI");
  }

  static char *proj_str_tmpl =
      "SELECT proj4text, auth_name, auth_srid, srtext "
      "FROM %s "
      "WHERE srid = %d "
      "LIMIT 1";
  snprintf(proj_spi_buffer, spibufferlen, proj_str_tmpl,
    postgis_spatial_ref_sys(), srid);

  /* Execute the query, noting the readonly status of this SQL */
  spi_result = SPI_execute(proj_spi_buffer, true, 1);

  /* Read back the PROJ text */
  if (spi_result == SPI_OK_SELECT && SPI_processed > 0)
  {
    /* Select the first (and only tuple) */
    TupleDesc tupdesc = SPI_tuptable->tupdesc;
    SPITupleTable *tuptable = SPI_tuptable;
    HeapTuple tuple = tuptable->vals[0];
    /* Always return the proj4text */
    char *proj4text = SPI_getvalue(tuple, tupdesc, 1);
    if (proj4text && strlen(proj4text))
      strs.proj4text = SPI_pstrdup(proj4text);

    /* For Proj >= 6 prefer "AUTHNAME:AUTHSRID" to proj strings */
    /* as proj_create_crs_to_crs() will give us more consistent */
    /* results with authority numbers than with proj strings */
    char *authname = SPI_getvalue(tuple, tupdesc, 2);
    char *authsrid = SPI_getvalue(tuple, tupdesc, 3);
    if (authname && authsrid && strlen(authname) && strlen(authsrid))
    {
      char tmp[MAX_PROJ_LEN];
      snprintf(tmp, MAX_PROJ_LEN, "%s:%s", authname, authsrid);
      strs.authtext = SPI_pstrdup(tmp);
    }

    /* Proj6+ can parse srtext, so return that too */
    char *srtext = SPI_getvalue(tuple, tupdesc, 4);
    if (srtext && strlen(srtext))
      strs.srtext = SPI_pstrdup(srtext);
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "Cannot find SRID (%d) in spatial_ref_sys", srid);
  }

  spi_result = SPI_finish();
  if (spi_result != SPI_OK_FINISH)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "Could not disconnect from database using SPI");
  }
  return strs;
}
#endif /* MEOS */

/**
 * @brief Given an SRID, return the corresponding proj strings
 * (auth_name:auth_srid/srtext/proj4text)
 * @details If the integer is one of the "well known" projections we support
 * (WGS84 UTM N/S, Polar Stereographic N/S - see SRID_* macros),
 * return the proj4text for those.
 */
static PjStrs
GetProjStrings(int32_t srid)
{
  PjStrs strs;
  memset(&strs, 0, sizeof(strs));

  /* SRIDs in SPATIAL_REF_SYS */
  if (srid < SRID_RESERVE_OFFSET)
  {
    return GetProjStringsSPI(srid);
  }
  /* Automagic SRIDs */
  else
  {
    strs.proj4text = palloc(MAX_PROJ_LEN);
    int id = srid;
    /* UTM North */
    if (id >= SRID_NORTH_UTM_START && id <= SRID_NORTH_UTM_END)
    {
      snprintf(strs.proj4text, MAX_PROJ_LEN,
        "+proj=utm +zone=%d +ellps=WGS84 +datum=WGS84 +units=m +no_defs",
        id - SRID_NORTH_UTM_START + 1);
    }
    /* UTM South */
    else if (id >= SRID_SOUTH_UTM_START && id <= SRID_SOUTH_UTM_END)
    {
      snprintf(strs.proj4text, MAX_PROJ_LEN,
        "+proj=utm +zone=%d +south +ellps=WGS84 +datum=WGS84 +units=m +no_defs",
        id - SRID_SOUTH_UTM_START + 1);
    }
    /* Lambert zones (about 30x30, larger in higher latitudes)
     * There are three latitude zones, divided at -90,-60,-30,0,30,60,90.
     * - In yzones 2,3 (equator) zones, the longitudinal zones are divided
         every 30 degrees (12 of them)
     * - In yzones 1,4 (temperate) zones, the longitudinal zones are every 45
         degrees (8 of them)
     * - In yzones 0,5 (polar) zones, the longitudinal zones are ever 90
         degrees (4 of them) */
    else if (id >= SRID_LAEA_START && id <= SRID_LAEA_END)
    {
      int zone = id - SRID_LAEA_START;
      int xzone = zone % 20;
      int yzone = zone / 20;
      double lat_0 = 30.0 * (yzone - 3) + 15.0;
      double lon_0 = 0.0;

      /* The number of xzones is variable depending on yzone */
      if  ( yzone == 2 || yzone == 3)
        lon_0 = 30.0 * (xzone - 6) + 15.0;
      else if (yzone == 1 || yzone == 4)
        lon_0 = 45.0 * (xzone - 4) + 22.5;
      else if (yzone == 0 || yzone == 5)
        lon_0 = 90.0 * (xzone - 2) + 45.0;
      else
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "Unknown yzone encountered!");

      snprintf(strs.proj4text, MAX_PROJ_LEN,
        "+proj=laea +ellps=WGS84 +datum=WGS84 +lat_0=%g +lon_0=%g +units=m +no_defs",
        lat_0, lon_0);
    }
    /* Lambert Azimuthal Equal Area South Pole */
    else if (id == SRID_SOUTH_LAMBERT)
    {
      strncpy(strs.proj4text,
        "+proj=laea +lat_0=-90 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs",
        MAX_PROJ_LEN);
    }
    /* Polar Sterographic South */
    else if (id == SRID_SOUTH_STEREO)
    {
      strncpy(strs.proj4text,
        "+proj=stere +lat_0=-90 +lat_ts=-71 +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs",
        MAX_PROJ_LEN);
    }
    /* Lambert Azimuthal Equal Area North Pole */
    else if (id == SRID_NORTH_LAMBERT)
    {
      strncpy(strs.proj4text,
        "+proj=laea +lat_0=90 +lon_0=-40 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs",
        MAX_PROJ_LEN);
    }
    /* Polar Stereographic North */
    else if (id == SRID_NORTH_STEREO)
    {
      strncpy(strs.proj4text,
        "+proj=stere +lat_0=90 +lat_ts=71 +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs",
        MAX_PROJ_LEN);
    }
    /* World Mercator */
    else if (id == SRID_WORLD_MERCATOR)
    {
      strncpy(strs.proj4text,
        "+proj=merc +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs",
        MAX_PROJ_LEN);
    }
    else
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Invalid reserved SRID (%d)", srid);
      return strs;
    }
    return strs;
  }
}

/**
 * @brief Return 1 if a PROJ strings structure has all its strings filled,
 * return 0 otherwise
 */
static int
pjstrs_has_entry(const PjStrs *strs)
{
  if ((strs->proj4text && strlen(strs->proj4text)) ||
    (strs->authtext && strlen(strs->authtext)) ||
    (strs->srtext && strlen(strs->srtext)))
    return 1;
  else
    return 0;
}

/**
 * @brief Delete a PROJ strings structure
 */
static void
pjstrs_pfree(PjStrs *strs)
{
  if (strs->proj4text)
    pfree(strs->proj4text);
  if (strs->authtext)
    pfree(strs->authtext);
  if (strs->srtext)
    pfree(strs->srtext);
}

/**
 * @brief Return the n-th string of a PROJ strings structure
 */
static char *
pgstrs_get_entry(const PjStrs *strs, int n)
{
  switch (n)
  {
    case 0:
      return strs->authtext;
    case 1:
      return strs->srtext;
    case 2:
      return strs->proj4text;
    default:
      return NULL;
  }
}

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Delete a PROJ structure referenced by a PostGIS projection structure
 */
static void
PROJSRSDestroyPJ(void *projection)
{
  LWPROJ *pj = (LWPROJ *) projection;
  if (pj->pj)
    proj_destroy(pj->pj);
  pfree(pj);
}

/**
 * @brief Create a Proj cache .
 */
static MEOSPROJCache *
proj_cache_create(void)
{
  MEOSPROJCache *cache = palloc(sizeof(MEOSPROJCache));
  if (! cache)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "Unable to allocate space for MEOSPROJCache");
    return NULL;
  }
  memset(cache, 0, sizeof(MEOSPROJCache));
  cache->last_index = UINT32_MAX;
  return cache;
}

/**
 * @brief Get the Proj cache entry from the global variable if one exists.
 * If it doesn't exist, make a new blank one and return it.
*/
MEOSPROJCache *
proj_cache_get()
{
  if (! MEOS_PROJ_CACHE)
    MEOS_PROJ_CACHE = proj_cache_create();
  return MEOS_PROJ_CACHE;
}

/**
 * @brief Destroy all the malloc'ed PROJ objects stored in the PROJSRSCache
 */
void
meos_finalize_projsrs(void)
{
  MEOSPROJCache *cache = MEOS_PROJ_CACHE;
  if (! cache)
    return;
  for (uint32_t i = 0; i < cache->count; i++)
  {
    if (cache->items[i].projection)
      PROJSRSDestroyPJ(cache->items[i].projection);
  }
  pfree(cache);
  MEOS_PROJ_CACHE = NULL;
  return;
}

/*****************************************************************************
 * Per-cache management functions
 *****************************************************************************/

/**
 * @brief Get a PROJ structure from the PROJ cache
 * @return On error return `NULL`
 */
static LWPROJ *
proj_cache_lookup(MEOSPROJCache *cache, int32_t srid_from, int32_t srid_to)
{
  /* Fast path testing the last entry */
  if (cache->last_index < cache->count)
  {
    MEOSPROJCacheItem *item = &cache->items[cache->last_index];
    if (item->srid_from == srid_from && item->srid_to == srid_to)
    {
      item->last_used = ++cache->clock;
      return item->projection;
    }
  }

  /* Loop to find the entry */
  for (uint32_t i = 0; i < cache->count; i++)
  {
    MEOSPROJCacheItem *item = &cache->items[i];
    if (item->srid_from == srid_from && item->srid_to == srid_to)
    {
      item->last_used = ++cache->clock;
      return item->projection;
    }
  }
  return NULL;
}

static uint32_t
proj_cache_lru_index(MEOSPROJCache *cache)
{
  uint32_t lru = 0;
  uint64_t oldest = cache->items[0].last_used;
  for (uint32_t i = 1; i < cache->count; i++)
  {
    if (cache->items[i].last_used < oldest)
    {
      oldest = cache->items[i].last_used;
      lru = i;
    }
  }
  return lru;
}

/**
 * @brief Add an entry to the PROJ SRS cache
 * @details If we need to wrap around then we must make sure the entry we
 * choose to delete does not contain other_srid which is the definition for
 * the other half of the transformation.
 */
static LWPROJ *
proj_cache_insert(MEOSPROJCache *cache, int32_t srid_from, int32_t srid_to)
{
  /* Turn the SRID number into a Proj4 string, by reading from spatial_ref_sys
   * or instantiating a magical value from a negative srid */
  PjStrs from = GetProjStrings(srid_from);
  if (! pjstrs_has_entry(&from))
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "got NULL for SRID (%d)", srid_from);
  PjStrs to = GetProjStrings(srid_to);
  if (! pjstrs_has_entry(&to))
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "got NULL for SRID (%d)", srid_to);

  /* Try combinations of AUTH_NAME:AUTH_SRID/SRTEXT/PROJ4TEXT until we find
   * one that gives us a usable transform. Note that we prefer
   * AUTH_NAME:AUTH_SRID over SRTEXT and SRTEXT over PROJ4TEXT
   * (3 entries * 3 entries = 9 combos) */
  char *from_str, *to_str;
  LWPROJ *projection = NULL;
  for (uint32_t i = 0; i < 9; i++)
  {
    from_str = pgstrs_get_entry(&from, i / 3);
    to_str = pgstrs_get_entry(&to, i % 3);
    if (! from_str || ! to_str)
      continue;
    projection = lwproj_from_str(from_str, to_str);
    if (projection)
      break;
  }
  if (! projection)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "could not form projection (LWPROJ) from 'srid=%d' to 'srid=%d'",
      srid_from, srid_to);
    return NULL;
  }

  /* If the cache is already full then find the least used element and delete it */
  uint32_t pos;
  if (cache->count < PROJ_CACHE_ITEMS)
    pos = cache->count++;
  else
  {
    pos = proj_cache_lru_index(cache);
    PROJSRSDestroyPJ(cache->items[pos].projection);
  }
  cache->items[pos].srid_from = srid_from;
  cache->items[pos].srid_to = srid_to;
  cache->items[pos].projection = projection;
  cache->items[pos].last_used = ++cache->clock;

  pjstrs_pfree(&from);
  pjstrs_pfree(&to);

  return projection;
}

/*****************************************************************************/

#if MEOS
/**
 * @brief Return 1 if the projection structure was read from the PROJ cache,
 * return 0 otherwise
 */
int
lwproj_lookup(int32_t srid_from, int32_t srid_to, LWPROJ **pj)
{
  /* get or initialize the cache */
  MEOSPROJCache* cache = proj_cache_get();
  if (! cache)
    return LW_FAILURE;

  /* Add the output SRID to the cache if it is not already there */
  *pj = proj_cache_lookup(cache, srid_from, srid_to);
  if (*pj == NULL)
  {
    *pj = proj_cache_insert(cache, srid_from, srid_to);
  }
  return (*pj != NULL);
}
#endif /* MEOS */

#if MEOS
/**
 * @brief Return 1 if the spheroid in the last argument was initialized
 * from an SRID, return 0 otherwise
 */
int
spheroid_init_from_srid(int32_t srid, SPHEROID *s)
{
  LWPROJ *pj;
  if (lwproj_lookup(srid, srid, &pj) == LW_FAILURE)
    return LW_FAILURE;
  if (! pj->source_is_latlong)
    return LW_FAILURE;
  spheroid_init(s, pj->source_semi_major_metre, pj->source_semi_minor_metre);
  return LW_SUCCESS;
}
#endif /* MEOS */

/*****************************************************************************/