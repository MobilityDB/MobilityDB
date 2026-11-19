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
 * @brief Cache of the PROJ transformations of the PostgreSQL extension, kept for the
 * life of the backend in a child of the CacheMemoryContext, the systems read
 * from the table spatial_ref_sys
 */

#include "geo/meos_transform.h"

/* C */
#include <string.h>
#include <stdio.h>
/* PostgreSQL */
#include <postgres.h>
#include <executor/spi.h>
#include <utils/memutils.h>
/* PostGIS */
#include <liblwgeom.h>
#include "lwgeom_pg.h"
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"
#include "geo/tspatial_transform_cache.h"

/* Length of the query reading a system from the table spatial_ref_sys, as
 * in lwgeom_transform.c of PostGIS */
#define spibufferlen 512

/* Global variable to hold the Proj object cache */
static MEOSPROJSRSCache *MEOS_PROJ_CACHE = NULL;
/* Context holding the Proj object cache of the backend and its entries */
static MemoryContext MEOS_PROJ_CONTEXT = NULL;

/**
 * @brief Destroy the PROJ objects of the Proj object cache and forget the
 * cache when the context holding it is reset
 * @details The callback of the reset, as #PROJSRSDestroyPortalCache is for
 * the PROJ cache of PostGIS and #ForgetWaysCache for the ways cache. The cache
 * and its entries lie in that context, so the reset releases them; the PROJ
 * objects are allocated by PROJ and are destroyed here.
 */
static void
ForgetMEOSPROJSRSCache(void *arg UNUSED)
{
  MEOSPROJSRSCache *cache = MEOS_PROJ_CACHE;
  for (uint32_t i = 0; cache && i < cache->PROJSRSCacheCount; i++)
  {
    LWPROJ *pj = cache->MEOSPROJSRSCache[i].projection;
    if (pj && pj->pj)
    {
      proj_destroy(pj->pj);
      pj->pj = NULL;
    }
  }
  MEOS_PROJ_CACHE = NULL;
  MEOS_PROJ_CONTEXT = NULL;
  return;
}

/**
 * @brief Get the Proj object cache from the global variable
 * @details If it does not exist, a new blank one is made in a child of the
 * CacheMemoryContext, as #GetWaysCache makes the ways cache, and returned. Its
 * entries are made in that context by #AddToMEOSPROJSRSCache, so the cache
 * serves the backend for its life; the systems are read from the table
 * `spatial_ref_sys`.
 */
MEOSPROJSRSCache *
GetMEOSPROJSRSCache(void)
{
  /* Read the schema of the table spatial_ref_sys before reading it, as
   * lwproj_lookup of PostGIS does */
  postgis_initialize_cache();
  if (! MEOS_PROJ_CACHE)
  {
    MemoryContext context = AllocSetContextCreate(CacheMemoryContext,
      "MEOS Proj Context", ALLOCSET_SMALL_SIZES);
    /* Use this to forget the cache in the event of a MemoryContext reset */
    MemoryContextCallback *callback = (MemoryContextCallback *)
      MemoryContextAlloc(context, sizeof(MemoryContextCallback));
    callback->func = ForgetMEOSPROJSRSCache;
    callback->arg = NULL;
    MemoryContextRegisterResetCallback(context, callback);
    MEOSPROJSRSCache *cache = MemoryContextAllocZero(context,
      sizeof(MEOSPROJSRSCache));
    cache->PROJSRSCacheCount = 0;
    MEOS_PROJ_CACHE = cache;
    MEOS_PROJ_CONTEXT = context;
  }
  return MEOS_PROJ_CACHE;
}
/**
 * @brief Make the entries of the Proj object cache in its context, returning
 * the context to go back to
 */
void *
proj_cache_enter(void)
{
  return MemoryContextSwitchTo(MEOS_PROJ_CONTEXT);
}

/**
 * @brief Go back to the context #proj_cache_enter left
 */
void
proj_cache_leave(void *state)
{
  MemoryContextSwitchTo((MemoryContext) state);
  return;
}

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
/**
 * @brief Return the PROJ strings of an SRID read from the table
 * `spatial_ref_sys` of PostGIS
 */
PjStrs
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
