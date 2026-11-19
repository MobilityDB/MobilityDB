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
 * systems of the SRIDs, shared by the MEOS build, which reads the systems from
 * the file spatial_ref_sys.csv, and the PostgreSQL build, which reads them from
 * the table spatial_ref_sys
 */

#ifndef __TSPATIAL_TRANSFORM_CACHE_H__
#define __TSPATIAL_TRANSFORM_CACHE_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>

#define MAX_PROJ_LEN  512

/* An entry in the PROJ SRS cache */
typedef struct struct_PROJSRSCacheItem
{
  int32_t srid_from;
  int32_t srid_to;
  uint64 hits;
  LWPROJ *projection;
  char *authtext; /* auth_name:auth_srid of srid_from, NULL if it has none */
  char *urn;      /* urn:ogc:def:crs:auth_name::auth_srid of srid_from */
} PROJSRSCacheItem;

/* PROJ 4 lookup transaction cache methods */
#define PROJ_CACHE_ITEMS 128

/**
 * @brief The proj4 cache holds a fixed number of reprojection entries
 * @details In normal usage we don't expect it to have many entries, so we
 * always linearly scan the list.
 * @note The structure removes the context field from PostGIS PROJSRSCache
 */
typedef struct struct_MEOSPROJSRSCache
{
  PROJSRSCacheItem MEOSPROJSRSCache[PROJ_CACHE_ITEMS];
  uint32_t PROJSRSCacheCount;
} MEOSPROJSRSCache;

/**
 * @brief PROJ 4 backend hash table initial hash size
 * @details Since 16 is the default portal hash table size, and there are
 * typically 2 entries per portal, the default size is 256.
 */
#define PROJ_BACKEND_HASH_SIZE 256

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

/* The functions each build defines */
extern MEOSPROJSRSCache *GetMEOSPROJSRSCache(void);
extern PjStrs GetProjStringsSPI(int32_t srid);
extern void *proj_cache_enter(void);
extern void proj_cache_leave(void *state);

/* The functions of the cache */
extern void PROJSRSDestroyPJ(void *projection);

#endif /* __TSPATIAL_TRANSFORM_CACHE_H__ */
