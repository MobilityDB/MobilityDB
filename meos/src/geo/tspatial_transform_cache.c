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
 * @brief Cache of the PROJ transformations and the names of the coordinate reference
 * systems of the SRIDs, in both builds
 * @details PostGIS manages a cache of information obtained from PROJ to speed
 * up transformations (see file libpgcommon/lwgeom_transform.c). The functions
 * of this file are derived from those PostGIS functions; the cache is kept and
 * the systems are read by tspatial_transform_meos.c in MEOS and by
 * tspatial_transform.c in the PostgreSQL extension, as ways_meos.c and ways.c
 * keep the cache of ways_cache.c
 */

#include "geo/meos_transform.h"

/* C */
#include <float.h>
#include <string.h>
#include <stdio.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"
#include "geo/tspatial_transform_cache.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Delete a PROJ structure referenced by a PostGIS projection structure
 */
void
PROJSRSDestroyPJ(void *projection)
{
  LWPROJ *pj = (LWPROJ *) projection;
  if (pj->pj)
  {
    proj_destroy(pj->pj);
    pj->pj = NULL;
  }
  pfree(pj);
}

/*****************************************************************************
 * Per-cache management functions
 *****************************************************************************/

/**
 * @brief Get the entry of a pair of SRIDs from the PROJ cache
 * @errval NULL
 */
static PROJSRSCacheItem *
GetItemFromPROJCache(MEOSPROJSRSCache *cache, int32_t srid_from,
  int32_t srid_to)
{
  for (uint32_t i = 0; i < cache->PROJSRSCacheCount; i++)
  {
    if (cache->MEOSPROJSRSCache[i].srid_from == srid_from &&
        cache->MEOSPROJSRSCache[i].srid_to == srid_to)
    {
      cache->MEOSPROJSRSCache[i].hits++;
      return &cache->MEOSPROJSRSCache[i];
    }
  }
  return NULL;
}

/**
 * @brief Get a PROJ structure from the PROJ cache
 * @errval NULL
 */
static LWPROJ *
GetProjectionFromPROJCache(MEOSPROJSRSCache *cache, int32_t srid_from,
  int32_t srid_to)
{
  PROJSRSCacheItem *item = GetItemFromPROJCache(cache, srid_from, srid_to);
  return item ? item->projection : NULL;
}

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
      {
        /* See doc-comment on meos_error in meos/include/meos.h: handler is
         * not guaranteed to abort. Bail explicitly so we don't silently
         * emit a wrong-but-syntactically-valid projstring with lon_0=0
         * for an unknown yzone. */
        pfree(strs.proj4text);
        strs.proj4text = NULL;
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "Unknown yzone encountered!");
        return strs;
      }

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
      /* The handler is not guaranteed to abort, so return no PROJ string
       * rather than the unfilled buffer */
      pfree(strs.proj4text);
      strs.proj4text = NULL;
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

/**
 * @brief Remove an entry to the PROJ SRS cache
 */
static void
DeleteFromMEOSPROJSRSCache(MEOSPROJSRSCache *PROJCache, uint32_t position)
{
  /* Call PROJSRSDestroyPJ to free the PROJ objects memory */
  PROJSRSDestroyPJ(PROJCache->MEOSPROJSRSCache[position].projection);
  PROJCache->MEOSPROJSRSCache[position].projection = NULL;
  if (PROJCache->MEOSPROJSRSCache[position].authtext)
    pfree(PROJCache->MEOSPROJSRSCache[position].authtext);
  PROJCache->MEOSPROJSRSCache[position].authtext = NULL;
  if (PROJCache->MEOSPROJSRSCache[position].urn)
    pfree(PROJCache->MEOSPROJSRSCache[position].urn);
  PROJCache->MEOSPROJSRSCache[position].urn = NULL;
  PROJCache->MEOSPROJSRSCache[position].srid_from = SRID_UNKNOWN;
  PROJCache->MEOSPROJSRSCache[position].srid_to = SRID_UNKNOWN;
}

/**
 * @brief Add an entry to the PROJ SRS cache
 * @details If we need to wrap around then we must make sure the entry we
 * choose to delete does not contain other_srid which is the definition for
 * the other half of the transformation.
 */
static LWPROJ *
AddToMEOSPROJSRSCache(MEOSPROJSRSCache *PROJCache, int32_t srid_from,
  int32_t srid_to)
{
  PjStrs from_strs, to_strs;

  /* Turn the SRID number into a proj4 string, by reading from spatial_ref_sys
   * or instantiating a magical value from a negative srid */
  from_strs = GetProjStrings(srid_from);
  if (! pjstrs_has_entry(&from_strs))
  {
    /* See doc-comment on meos_error in meos/include/meos.h: handler is
     * not guaranteed to abort. Free and return so we don't fall through
     * to a downstream lwproj_from_str with empty PjStrs. */
    pjstrs_pfree(&from_strs);
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "got NULL for SRID (%d)", srid_from);
    return NULL;
  }
  to_strs = GetProjStrings(srid_to);
  if (! pjstrs_has_entry(&to_strs))
  {
    pjstrs_pfree(&from_strs);
    pjstrs_pfree(&to_strs);
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "got NULL for SRID (%d)", srid_to);
    return NULL;
  }

  /* The projection and the entry are made where the build keeps the cache, the
   * strings read staying where they were read */
  void *oldcontext = proj_cache_enter();
  LWPROJ *projection = NULL;
  /* Try combinations of AUTH_NAME:AUTH_SRID/SRTEXT/PROJ4TEXT until we find
   * one that gives us a usable transform. Note that we prefer
   * AUTH_NAME:AUTH_SRID over SRTEXT and SRTEXT over PROJ4TEXT
   * (3 entries * 3 entries = 9 combos) */
  uint32_t i;
  for (i = 0; i < 9; i++)
  {
    char *pj_from_str = pgstrs_get_entry(&from_strs, i / 3);
    char *pj_to_str = pgstrs_get_entry(&to_strs, i % 3);
    if (! (pj_from_str && pj_to_str))
      continue;

    projection = lwproj_from_str(pj_from_str, pj_to_str);
    if (projection)
      break;
  }
  if (! projection)
  {
    proj_cache_leave(oldcontext);
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "could not form projection (LWPROJ) from 'srid=%d' to 'srid=%d'", srid_from, srid_to);
    return NULL;
  }

  /* If the cache is already full then find the least used element and delete it */
  uint32_t cache_position = PROJCache->PROJSRSCacheCount;
  uint32_t hits = 1;
  if (cache_position == PROJ_CACHE_ITEMS)
  {
    cache_position = 0;
    hits = PROJCache->MEOSPROJSRSCache[0].hits;
    for (uint32_t k = 1; k < PROJ_CACHE_ITEMS; k++)
    {
      if (PROJCache->MEOSPROJSRSCache[k].hits < hits)
      {
        cache_position = k;
        hits = PROJCache->MEOSPROJSRSCache[k].hits;
      }
    }
    DeleteFromMEOSPROJSRSCache(PROJCache, cache_position);
    /* To avoid the element we are introduced now being evicted next (as
     * it would have 1 hit, being most likely the lower one) we reuse the
     * hits from the evicted position and add some extra buffer
     */
    hits += 5;
  }
  else
  {
    PROJCache->PROJSRSCacheCount++;
  }

  /* Keep the authority of the source SRID and its OGC URN, the short and the
   * long names of its coordinate reference system in GeoJSON and MF-JSON, and
   * free the other strings */
  /* The authority is read where the strings are read: copy it where the cache
   * is kept */
  char *authtext = from_strs.authtext ? pstrdup(from_strs.authtext) : NULL;
  pjstrs_pfree(&from_strs);
  pjstrs_pfree(&to_strs);
  char *urn = NULL;
  const char *colon = authtext ? strchr(authtext, ':') : NULL;
  if (colon)
  {
    /* The long name separates the authority from its code by two colons */
    size_t len = strlen(authtext) + sizeof("urn:ogc:def:crs::");
    urn = palloc(len);
    snprintf(urn, len, "urn:ogc:def:crs:%.*s::%s", (int) (colon - authtext),
      authtext, colon + 1);
  }

  /* Store everything in new cache entry */
  PROJCache->MEOSPROJSRSCache[cache_position].srid_from = srid_from;
  PROJCache->MEOSPROJSRSCache[cache_position].srid_to = srid_to;
  PROJCache->MEOSPROJSRSCache[cache_position].projection = projection;
  PROJCache->MEOSPROJSRSCache[cache_position].authtext = authtext;
  PROJCache->MEOSPROJSRSCache[cache_position].urn = urn;
  PROJCache->MEOSPROJSRSCache[cache_position].hits = hits;

  proj_cache_leave(oldcontext);
  return projection;
}

/*****************************************************************************/

/**
 * @brief Return 1 if the projection structure was read from the PROJ cache,
 * return 0 otherwise
 */
int
lwproj_lookup(int32_t srid_from, int32_t srid_to, LWPROJ **pj)
{
  /* get or initialize the cache for this round */
  MEOSPROJSRSCache* proj_cache = GetMEOSPROJSRSCache();
  if (! proj_cache)
    return LW_FAILURE;

  /* Add the output SRID to the cache if it is not already there */
  *pj = GetProjectionFromPROJCache(proj_cache, srid_from, srid_to);
  if (*pj == NULL)
  {
    *pj = AddToMEOSPROJSRSCache(proj_cache, srid_from, srid_to);
  }
  return *pj != NULL;
}

/**
 * @brief Return 1 if the spheroid in the last argument was initialized
 * from an SRID, return 0 otherwise
 */
int
spheroid_init_from_srid(int32_t srid, SPHEROID *s)
{
  /* Answer the geographic SRIDs in common use from the built-in table, so that
   * the geodetic surface neither queries PROJ nor reads spatial_ref_sys */
  double a, rf;
  if (srid_builtin_ellipsoid(srid, &a, &rf) == LW_SUCCESS)
  {
    spheroid_init(s, a, a - a / rf);
    return LW_SUCCESS;
  }

  LWPROJ *pj;
  if (lwproj_lookup(srid, srid, &pj) == LW_FAILURE)
    return LW_FAILURE;
  if (! pj->source_is_latlong)
    return LW_FAILURE;
  spheroid_init(s, pj->source_semi_major_metre, pj->source_semi_minor_metre);
  return LW_SUCCESS;
}

/**
 * @brief Return the name of the coordinate reference system of an SRID, as
 * the GeoJSON and MF-JSON representations state it
 * @details The short name is the authority and its code, as in `EPSG:3857`,
 * and the long one is the OGC URN, as in `urn:ogc:def:crs:EPSG::3857`. Both
 * are read, from `spatial_ref_sys.csv` in MEOS and from the table
 * `spatial_ref_sys` in the PostgreSQL extension, together with the projection
 * of the SRID onto itself and kept with it in the PROJ cache, so the systems
 * are read once per SRID
 * @param[in] srid SRID
 * @param[in] short_crs True for the short name, false for the long one
 * @return The name, owned by the PROJ cache and valid until the cache stores
 * another projection, or @p NULL with an error where the SRID has no
 * authority in the systems read
 */
const char *
srid_srs(int32_t srid, bool short_crs)
{
  MEOSPROJSRSCache *cache = GetMEOSPROJSRSCache();
  if (! cache)
    return NULL;
  const PROJSRSCacheItem *item = GetItemFromPROJCache(cache, srid, srid);
  LWPROJ *pj;
  if (! item && lwproj_lookup(srid, srid, &pj) == LW_SUCCESS)
    item = GetItemFromPROJCache(cache, srid, srid);
  const char *result = ! item ? NULL : short_crs ? item->authtext : item->urn;
  if (! result)
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "SRID %d unknown in spatial_ref_sys", srid);
  return result;
}

/*****************************************************************************/