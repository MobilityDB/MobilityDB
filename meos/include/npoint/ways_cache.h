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
 * @brief Cache of the routes of the ways network, shared by the MEOS build,
 * which reads them from a CSV file, and the PostgreSQL build, which reads
 * them from the ways table
 */

#ifndef __WAYS_CACHE_H__
#define __WAYS_CACHE_H__

/* C */
#include <stdbool.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>

/*****************************************************************************/

/**
 * @brief Structure to represent a record of the ways network
 */
typedef struct
{
  int64 gid;              /**< Identifier of the route */
  GSERIALIZED *the_geom;  /**< Geometry of the route */
  const LWLINE *the_line; /**< Geometry of the route read as a line, set by
                               the cache and not by a fetch function */
  double length;          /**< Length of the route, DBL_MAX when it states
                               none */
  double bound;           /**< Bound of the construction of a point on the
                               route (#route_construction_bound), set by the
                               cache and not by a fetch function */
} ways_record;

/**
 * @brief Function reading the record of a route from the ways network
 * @details It is given the identifier of the route, or true in its second
 * argument when the record of any route is asked for, and allocates the
 * geometry of the record in the memory of the cache, which the cache then
 * releases with `pfree`. It returns false when there is no such route.
 */
typedef bool (*ways_fetch_fn)(int64 rid, bool any_gid, ways_record *rec);

/* Opaque structure of the ways cache */
typedef struct WaysCache WaysCache;

/*****************************************************************************/

extern WaysCache *ways_cache_make(void);
extern void ways_cache_free(WaysCache *cache);
extern bool ways_cache_lookup(WaysCache *cache, int64 gid, bool any_gid,
  ways_fetch_fn fetch, ways_record *rec);

/*****************************************************************************/

#endif /* __WAYS_CACHE_H__ */
