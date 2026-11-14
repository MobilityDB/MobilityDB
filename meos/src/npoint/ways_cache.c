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
 * @brief Cache of the routes of the ways network
 * @details Both builds keep the routes they read in this cache, the MEOS
 * build in ways_meos.c from the ways CSV file and the PostgreSQL build in
 * ways.c from the ways table, and differ only in how a route is read and in
 * where the cache lives.
 *
 * A route is found by its identifier through a hash table, so that the cost
 * of a lookup does not grow with the size of the network. The cache is bound
 * by the bytes of the geometries it holds, not by its number of routes,
 * since the geometry of a route ranges from a few tens of bytes to several
 * kilobytes. When the bound is reached the geometry of the route used least
 * recently is released, while its entry and its length are kept: an entry
 * costs a few tens of bytes, and keeping it answers the existence and the
 * length of every route already read without reading it again, whatever the
 * size of the network.
 */

#include "npoint/ways_cache.h"

/* C */
#include <assert.h>
#include <string.h>
/* PostgreSQL */
#include <varatt.h>
/* MEOS */
#include "npoint/tnpoint.h"

/* Bytes of route geometries the cache holds */
#define WAYS_CACHE_GEOM_BYTES (8 * 1024 * 1024)
/* Initial number of entries of the cache, a power of two */
#define WAYS_CACHE_INIT_SIZE 64
/* Marker of an empty slot of the hash table and of the end of the list */
#define WAYS_CACHE_NONE (-1)

/**
 * @brief Structure to represent an entry in the ways cache
 */
typedef struct
{
  int64 gid;              /**< Identifier of the route */
  GSERIALIZED *the_geom;  /**< Geometry of the route, NULL when released */
  LWLINE *the_line;       /**< Geometry read as a line, a view over the
                               points of the geometry it is read from, which
                               it is released with */
  double length;          /**< Length of the route */
  double bound;           /**< Bound of the construction of a point on the
                               route, read with the line */
  int32 prev;            /**< Previous entry holding a geometry, in the
                               order of their use */
  int32 next;             /**< Next entry holding a geometry */
} WaysCacheEntry;

/**
 * @brief Structure to represent the ways cache
 * @details The entries are stored in an array that only grows, and the slots
 * of the hash table, whose size is twice the capacity of the array, hold the
 * position of an entry in it. The entries holding a geometry are linked in
 * the order of their use, from the most recent one, the head, to the least
 * recent one, the tail.
 */
struct WaysCache
{
  WaysCacheEntry *entries;  /**< Entries of the cache */
  int32 count;              /**< Number of entries */
  int32 capacity;           /**< Capacity of the array of entries */
  int32 *slots;             /**< Slots of the hash table */
  int32 head;               /**< Entry used most recently */
  int32 tail;               /**< Entry used least recently */
  size_t geombytes;         /**< Bytes of the geometries held */
  bool hassrid;             /**< True when a route was read */
  int32_t srid;             /**< SRID of the routes read, which every route
                                 of the network shares */
};

/*****************************************************************************/

/**
 * @brief Return the slot of the hash table where a route identifier starts
 * its search
 * @details The finalizer of MurmurHash3 spreads consecutive identifiers,
 * the usual ones of a network, over the whole table.
 */
static uint32_t
ways_cache_slot(int64 gid, int32 nslots)
{
  uint64_t h = (uint64_t) gid;
  h ^= h >> 33;
  h *= UINT64_C(0xff51afd7ed558ccd);
  h ^= h >> 33;
  h *= UINT64_C(0xc4ceb9fe1a85ec53);
  h ^= h >> 33;
  return (uint32_t) (h & (uint64_t) (nslots - 1));
}

/**
 * @brief Return the position of the entry of a route in the cache, or
 * `WAYS_CACHE_NONE` when it is not there
 */
static int32
ways_cache_find(const WaysCache *cache, int64 gid)
{
  int32 nslots = cache->capacity * 2;
  uint32_t i = ways_cache_slot(gid, nslots);
  while (cache->slots[i] != WAYS_CACHE_NONE)
  {
    if (cache->entries[cache->slots[i]].gid == gid)
      return cache->slots[i];
    i = (i + 1) & (uint32_t) (nslots - 1);
  }
  return WAYS_CACHE_NONE;
}

/**
 * @brief Put the position of an entry in the slots of the hash table
 */
static void
ways_cache_slot_set(WaysCache *cache, int32 pos)
{
  int32 nslots = cache->capacity * 2;
  uint32_t i = ways_cache_slot(cache->entries[pos].gid, nslots);
  while (cache->slots[i] != WAYS_CACHE_NONE)
    i = (i + 1) & (uint32_t) (nslots - 1);
  cache->slots[i] = pos;
  return;
}

/**
 * @brief Make the array of entries and the hash table of the cache for a
 * given capacity
 */
static void
ways_cache_alloc(WaysCache *cache, int32 capacity)
{
  if (cache->entries)
    cache->entries = repalloc(cache->entries,
      sizeof(WaysCacheEntry) * capacity);
  else
    cache->entries = palloc(sizeof(WaysCacheEntry) * capacity);
  if (cache->slots)
    pfree(cache->slots);
  cache->slots = palloc(sizeof(int32) * capacity * 2);
  memset(cache->slots, 0xff, sizeof(int32) * capacity * 2);
  cache->capacity = capacity;
  for (int32 i = 0; i < cache->count; i++)
    ways_cache_slot_set(cache, i);
  return;
}

/**
 * @brief Remove an entry from the list of the entries holding a geometry
 */
static void
ways_cache_unlink(WaysCache *cache, int32 pos)
{
  WaysCacheEntry *entry = &cache->entries[pos];
  if (entry->prev != WAYS_CACHE_NONE)
    cache->entries[entry->prev].next = entry->next;
  else
    cache->head = entry->next;
  if (entry->next != WAYS_CACHE_NONE)
    cache->entries[entry->next].prev = entry->prev;
  else
    cache->tail = entry->prev;
  entry->prev = entry->next = WAYS_CACHE_NONE;
  return;
}

/**
 * @brief Put an entry at the head of the list of the entries holding a
 * geometry
 */
static void
ways_cache_push(WaysCache *cache, int32 pos)
{
  WaysCacheEntry *entry = &cache->entries[pos];
  entry->prev = WAYS_CACHE_NONE;
  entry->next = cache->head;
  if (cache->head != WAYS_CACHE_NONE)
    cache->entries[cache->head].prev = pos;
  cache->head = pos;
  if (cache->tail == WAYS_CACHE_NONE)
    cache->tail = pos;
  return;
}

/**
 * @brief Mark an entry holding a geometry as the one used most recently
 */
static void
ways_cache_touch(WaysCache *cache, int32 pos)
{
  if (cache->head == pos)
    return;
  ways_cache_unlink(cache, pos);
  ways_cache_push(cache, pos);
  return;
}

/**
 * @brief Release the geometry of an entry of the cache and the line read
 * from it
 * @details The line is a view over the points of the geometry, so it is
 * released first, as #relate_ctx_cache_clear_entry releases the geometry of
 * an entry before the copy it reads
 */
static void
ways_cache_release_geom(WaysCacheEntry *entry)
{
  if (entry->the_line)
    lwline_free(entry->the_line);
  pfree(entry->the_geom);
  entry->the_line = NULL;
  entry->the_geom = NULL;
  return;
}

/**
 * @brief Release the geometries used least recently until those held fit
 * in the bound of the cache
 * @details The two geometries used most recently are never released: a
 * caller keeps the geometry of a route while it asks for the next one, and
 * reads it in place, since the geometry belongs to the cache.
 */
static void
ways_cache_evict(WaysCache *cache)
{
  while (cache->geombytes > WAYS_CACHE_GEOM_BYTES &&
    cache->tail != cache->head &&
    cache->tail != cache->entries[cache->head].next)
  {
    int32 pos = cache->tail;
    WaysCacheEntry *entry = &cache->entries[pos];
    ways_cache_unlink(cache, pos);
    cache->geombytes -= VARSIZE(entry->the_geom);
    ways_cache_release_geom(entry);
  }
  return;
}

/**
 * @brief Give a geometry to an entry of the cache
 * @details The entry reads the geometry as a line once, a view over the
 * points of the geometry the entry owns, as #relate_ctx_borrow reads the
 * geometry of an entry of the relate context cache from the copy the entry
 * owns, so that a caller reads the route as a line in place
 */
static void
ways_cache_set_geom(WaysCache *cache, int32 pos, GSERIALIZED *geom)
{
  cache->entries[pos].the_geom = geom;
  cache->entries[pos].the_line = (LWLINE *) lwgeom_from_gserialized(geom);
  cache->entries[pos].bound =
    route_construction_bound(cache->entries[pos].the_line);
  cache->geombytes += VARSIZE(geom);
  ways_cache_push(cache, pos);
  ways_cache_evict(cache);
  return;
}

/**
 * @brief Add the record of a route to the cache and return its position
 */
static int32
ways_cache_add(WaysCache *cache, const ways_record *rec)
{
  if (cache->count == cache->capacity)
    ways_cache_alloc(cache, cache->capacity * 2);
  int32 pos = cache->count++;
  WaysCacheEntry *entry = &cache->entries[pos];
  entry->gid = rec->gid;
  entry->the_geom = NULL;
  entry->the_line = NULL;
  entry->length = rec->length;
  entry->bound = 0.0;
  entry->prev = entry->next = WAYS_CACHE_NONE;
  ways_cache_slot_set(cache, pos);
  ways_cache_set_geom(cache, pos, rec->the_geom);
  return pos;
}

/*****************************************************************************/

/**
 * @brief Return a new empty ways cache
 * @details The cache and its arrays are allocated with `palloc`, so that a
 * caller giving the cache a memory context of its own makes it the current
 * context around every call of this file.
 */
WaysCache *
ways_cache_make(void)
{
  WaysCache *result = palloc0(sizeof(WaysCache));
  result->head = result->tail = WAYS_CACHE_NONE;
  ways_cache_alloc(result, WAYS_CACHE_INIT_SIZE);
  return result;
}

/**
 * @brief Free a ways cache and the geometries it holds
 */
void
ways_cache_free(WaysCache *cache)
{
  if (! cache)
    return;
  for (int32 i = 0; i < cache->count; i++)
  {
    if (cache->entries[i].the_geom)
      ways_cache_release_geom(&cache->entries[i]);
  }
  pfree(cache->entries);
  pfree(cache->slots);
  pfree(cache);
  return;
}

/**
 * @brief Return true when the geometry of a route read from the ways network
 * is valid, and otherwise free it and return false
 * @details Every route of the network shares the SRID of the first route the
 * cache reads, as #npoint_srid presupposes
 */
static bool
ways_cache_route_valid(WaysCache *cache, ways_record *rec)
{
  int32_t srid = cache->hassrid ? cache->srid :
    gserialized_get_srid(rec->the_geom);
  if (! ensure_valid_route(rec->the_geom, srid))
  {
    pfree(rec->the_geom);
    rec->the_geom = NULL;
    return false;
  }
  cache->hassrid = true;
  cache->srid = srid;
  return true;
}

/**
 * @brief Return in the last argument the record of a route, read from the
 * ways cache or else from the ways network, and return false when there is
 * no such route
 * @details The geometry of the record and the line read from it belong to
 * the cache. A route whose geometry the cache released is read again from
 * the network, and only its geometry is kept, since its entry is still there.
 * @param[in] cache Ways cache
 * @param[in] gid Route identifier
 * @param[in] any_gid True when the record of any route is asked for
 * @param[in] fetch Function reading a route from the ways network, which
 * allocates the geometry it returns in the context of the cache
 * @param[out] rec Record of the route
 */
bool
ways_cache_lookup(WaysCache *cache, int64 gid, bool any_gid,
  ways_fetch_fn fetch, ways_record *rec)
{
  assert(cache); assert(fetch); assert(rec);
  int32 pos = any_gid ? cache->head : ways_cache_find(cache, gid);
  if (pos == WAYS_CACHE_NONE)
  {
    if (! fetch(gid, any_gid, rec) || ! ways_cache_route_valid(cache, rec))
      return false;
    /* Asked for any route, the network may answer one already there */
    pos = any_gid ? ways_cache_find(cache, rec->gid) : WAYS_CACHE_NONE;
    if (pos == WAYS_CACHE_NONE)
      pos = ways_cache_add(cache, rec);
    else if (! cache->entries[pos].the_geom)
      ways_cache_set_geom(cache, pos, rec->the_geom);
    else
      pfree(rec->the_geom);
  }
  else if (! cache->entries[pos].the_geom)
  {
    /* The entry is there but its geometry was released */
    if (! fetch(gid, false, rec) || ! ways_cache_route_valid(cache, rec))
      return false;
    ways_cache_set_geom(cache, pos, rec->the_geom);
  }
  else
    ways_cache_touch(cache, pos);

  WaysCacheEntry *entry = &cache->entries[pos];
  rec->gid = entry->gid;
  rec->the_geom = entry->the_geom;
  rec->the_line = entry->the_line;
  rec->length = entry->length;
  rec->bound = entry->bound;
  return true;
}

/*****************************************************************************/
