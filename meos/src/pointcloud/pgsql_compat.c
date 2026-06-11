/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
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
 *****************************************************************************/

/*****************************************************************************
 *
 * The function bodies below are vendored verbatim from pgPointCloud's
 * `pgsql/pc_pgsql.c` (commit pinned by the `pointcloud-pg/` subtree).
 * pgPointCloud is provided under the BSD 3-clause license; the original
 * copyright notice follows. The MobilityDB-side wrapping (file headers,
 * the `meos_pc_*` rename, the @c #if !PC_API_HAS_SERIALIZE gating) is
 * under The PostgreSQL License as above.
 *
 *   Copyright (c) 2013 Natural Resources Canada
 *   All rights reserved. See @c pointcloud-pg/COPYRIGHT for the full
 *   BSD 3-clause text.
 *
 * This file is a transitional shim. An upstream proposal is pending
 * against pgpointcloud/pointcloud to move these symbols from
 * @c pgsql/pc_pgsql.c into @c lib/, where they reach @c libpc.a and
 * become linkable from MEOS. Once that lands and the subtree is
 * bumped, the CMake probe sets @c PC_API_HAS_SERIALIZE=1, this entire
 * translation unit compiles to nothing, and the file can be removed
 * alongside the macro indirection in @c pgsql_compat.h.
 *
 *****************************************************************************/

#include "pointcloud/pgsql_compat.h"

#if !defined(PC_API_HAS_SERIALIZE) || !PC_API_HAS_SERIALIZE

/* C */
#include <assert.h>
#include <string.h>
/* PostgreSQL — palloc, SET_VARSIZE, BUFFERALIGN are PG infrastructure
 * already in scope when MEOS is built; this file uses no fmgr machinery. */
#include <postgres.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* pgPointCloud — types + helpers needed by the bodies below. */
#include "pc_api.h"
#include "pc_api_internal.h"

/*****************************************************************************
 * pcpoint serialization (public)
 *****************************************************************************/

/**
 * @brief Serialize a @c PCPOINT into a fresh @c SERIALIZED_POINT varlena.
 * @param pcpt Source in-memory point.
 * @return Newly allocated @c SERIALIZED_POINT (palloc'd, varlena header set).
 *   Caller owns the result.
 */
SERIALIZED_POINT *
meos_pc_point_serialize(const PCPOINT *pcpt)
{
  size_t serpt_size = sizeof(SERIALIZED_POINT) - 1 + pcpt->schema->size;
  SERIALIZED_POINT *serpt = palloc(serpt_size);
  serpt->pcid = pcpt->schema->pcid;
  memcpy(serpt->data, pcpt->data, pcpt->schema->size);
  SET_VARSIZE(serpt, serpt_size);
  return serpt;
}

/**
 * @brief Deserialize a @c SERIALIZED_POINT into a fresh @c PCPOINT.
 * @param serpt  Source varlena.
 * @param schema Schema matching @c serpt->pcid (must not be NULL).
 * @return Newly allocated @c PCPOINT, or @c NULL when @c schema->size
 *   disagrees with the on-disk payload size (a corruption indicator
 *   on which @c pcerror is raised).
 */
PCPOINT *
meos_pc_point_deserialize(const SERIALIZED_POINT *serpt, const PCSCHEMA *schema)
{
  PCPOINT *pcpt;
  size_t pgsize = VARSIZE(serpt) + 1 - sizeof(SERIALIZED_POINT);
  if (schema->size != pgsize)
  {
    pcerror("schema size and disk size mismatch, repair the schema");
    return NULL;
  }
  pcpt = pc_point_from_data(schema, serpt->data);
  return pcpt;
}

/*****************************************************************************
 * pcpatch serialized-size accounting (public)
 *****************************************************************************/

/**
 * @brief Compute the @c SERIALIZED_PATCH size for an in-memory @c PCPATCH.
 * @param patch In-memory patch (compression type may be @c PC_NONE,
 *   @c PC_DIMENSIONAL, or @c PC_LAZPERF).
 * @return Total bytes the serialized form will occupy, including the
 *   varlena header, stats triplet, and compression-specific payload.
 *   Raises @c pcerror on unknown compression types.
 */
size_t
meos_pc_patch_serialized_size(const PCPATCH *patch)
{
  size_t stats_size = pc_stats_size(patch->schema);
  size_t common_size = BUFFERALIGN(sizeof(SERIALIZED_PATCH)) - 1;
  switch (patch->type)
  {
  case PC_NONE:
  {
    PCPATCH_UNCOMPRESSED *pu = (PCPATCH_UNCOMPRESSED *)patch;
    return common_size + stats_size + pu->datasize;
  }
  case PC_DIMENSIONAL:
  {
    return common_size + stats_size +
           pc_patch_dimensional_serialized_size((PCPATCH_DIMENSIONAL *)patch);
  }
  case PC_LAZPERF:
  {
    static size_t lazsize_size = 4;
    PCPATCH_LAZPERF *pg = (PCPATCH_LAZPERF *)patch;
    return common_size + stats_size + lazsize_size + pg->lazperfsize;
  }
  default:
  {
    pcerror("%s: unknown compresed %d", __func__, patch->type);
  }
  }
  return -1;
}

/*****************************************************************************
 * Static helpers — pcpatch stats + per-compression (de)serialize.
 *
 * These mirror the corresponding statics in upstream's pc_pgsql.c
 * (ported verbatim) and are dispatched from the public pcpatch
 * (de)serialize entry points further below.
 *****************************************************************************/

/**
 * @brief Write the (min, max, avg) stats triplet into @p buf.
 * @return Number of bytes written.
 */
static size_t
pc_patch_stats_serialize(uint8_t *buf, const PCSCHEMA *schema,
                         const PCSTATS *stats)
{
  size_t sz = schema->size;
  memcpy(buf, stats->min.data, sz);
  memcpy(buf + sz, stats->max.data, sz);
  memcpy(buf + 2 * sz, stats->avg.data, sz);
  return sz * 3;
}

/**
 * @brief Reconstruct a @c PCSTATS triplet from the leading bytes of
 *   a serialized patch's data area.
 */
static PCSTATS *
pc_patch_stats_deserialize(const PCSCHEMA *schema, const uint8_t *buf)
{
  size_t sz = schema->size;
  const uint8_t *buf_min = buf;
  const uint8_t *buf_max = buf + sz;
  const uint8_t *buf_avg = buf + 2 * sz;
  return pc_stats_new_from_data(schema, buf_min, buf_max, buf_avg);
}

/**
 * @brief Serialize a dimensional-compressed in-memory patch.
 * @details Per-dimension @c PCBYTES blobs are concatenated after the
 *   stats triplet.
 */
static SERIALIZED_PATCH *
pc_patch_dimensional_serialize(const PCPATCH *patch_in)
{
  int i;
  uint8_t *buf;
  size_t serpch_size = meos_pc_patch_serialized_size(patch_in);
  size_t stats_size = pc_stats_size(patch_in->schema);
  const PCPATCH_DIMENSIONAL *patch = (PCPATCH_DIMENSIONAL *)patch_in;
  SERIALIZED_PATCH *serpch = palloc(serpch_size);

  assert(patch_in);
  assert(patch_in->type == PC_DIMENSIONAL);

  /* Copy basic */
  serpch->compression = patch->type;
  serpch->pcid = patch->schema->pcid;
  serpch->npoints = patch->npoints;
  serpch->bounds = patch->bounds;

  /* Get a pointer to the data area */
  buf = serpch->data;

  /* Write stats into the buffer */
  if (patch->stats)
  {
    pc_patch_stats_serialize(buf, patch->schema, patch->stats);
    buf += stats_size;
  }
  else
  {
    pcerror("%s: stats missing!", __func__);
  }

  /* Write each dimension in after the stats */
  for (i = 0; i < patch->schema->ndims; i++)
  {
    size_t bsz;
    PCBYTES *pcb = &(patch->bytes[i]);
    pc_bytes_serialize(pcb, buf, &bsz);
    buf += bsz;
  }

  SET_VARSIZE(serpch, serpch_size);
  return serpch;
}

/**
 * @brief Serialize a LAZ-perf-compressed in-memory patch.
 * @details Layout after the stats triplet is a 4-byte length prefix
 *   followed by the LAZ-perf payload.
 */
static SERIALIZED_PATCH *
pc_patch_lazperf_serialize(const PCPATCH *patch_in)
{
  static size_t lazsize_size = 4;
  uint8_t *buf, *stats_start;
  size_t serpch_size = meos_pc_patch_serialized_size(patch_in);
  size_t stats_size = pc_stats_size(patch_in->schema);
  const PCPATCH_LAZPERF *patch = (PCPATCH_LAZPERF *)patch_in;
  SERIALIZED_PATCH *serpch = palloc(serpch_size);

  assert(patch_in);
  assert(patch_in->type == PC_LAZPERF);

  serpch->pcid = patch->schema->pcid;
  serpch->compression = patch->type;
  serpch->npoints = patch->npoints;
  serpch->bounds = patch->bounds;

  /* Write stats into the buffer */
  stats_start = buf = serpch->data;
  if (patch->stats)
  {
    pc_patch_stats_serialize(buf, patch->schema, patch->stats);
    buf = stats_start + stats_size;
  }
  else
  {
    pcerror("%s: stats missing!", __func__);
  }

  memcpy(buf, &(patch->lazperfsize), lazsize_size);
  buf += lazsize_size;
  memcpy(buf, patch->lazperf, patch->lazperfsize);
  SET_VARSIZE(serpch, serpch_size);
  return serpch;
}

/**
 * @brief Serialize an uncompressed in-memory patch.
 * @details The point payload is a contiguous byte array; copied as-is
 *   after the stats triplet.
 */
static SERIALIZED_PATCH *
pc_patch_uncompressed_serialize(const PCPATCH *patch_in)
{
  uint8_t *buf;
  size_t serpch_size;
  size_t stats_size;
  const PCPATCH_UNCOMPRESSED *patch = (PCPATCH_UNCOMPRESSED *)patch_in;
  SERIALIZED_PATCH *serpch;

  serpch_size = meos_pc_patch_serialized_size(patch_in);
  serpch = palloc(serpch_size);
  stats_size = pc_stats_size(patch_in->schema);

  /* Copy basics */
  serpch->pcid = patch->schema->pcid;
  serpch->npoints = patch->npoints;
  serpch->bounds = patch->bounds;
  serpch->compression = patch->type;

  /* Write stats into the buffer */
  buf = serpch->data;
  if (patch->stats)
  {
    pc_patch_stats_serialize(buf, patch->schema, patch->stats);
    buf += stats_size;
  }
  else
  {
    pcerror("%s: stats missing!", __func__);
  }

  /* Copy point list into data buffer */
  memcpy(buf, patch->data, patch->datasize);
  SET_VARSIZE(serpch, serpch_size);
  return serpch;
}

/*****************************************************************************
 * pcpatch serialization — public dispatch
 *****************************************************************************/

/**
 * @brief Serialize a @c PCPATCH at its schema's target compression.
 * @param patch_in Source in-memory patch (must carry stats).
 * @param userdata Compression-specific side state — typically a
 *   @c PCDIMSTATS for dimensional compression; may be @c NULL.
 * @return Newly allocated @c SERIALIZED_PATCH varlena, or @c NULL on
 *   missing stats / unknown compression type. Caller owns the result.
 */
SERIALIZED_PATCH *
meos_pc_patch_serialize(const PCPATCH *patch_in, void *userdata)
{
  PCPATCH *patch = (PCPATCH *)patch_in;
  SERIALIZED_PATCH *serpatch = NULL;
  if (! patch->stats)
  {
    pcerror("%s: patch is missing stats", __func__);
    return NULL;
  }
  if (patch->type != patch->schema->compression)
  {
    patch = pc_patch_compress(patch_in, userdata);
  }

  switch (patch->type)
  {
  case PC_NONE:
    serpatch = pc_patch_uncompressed_serialize(patch);
    break;
  case PC_DIMENSIONAL:
    serpatch = pc_patch_dimensional_serialize(patch);
    break;
  case PC_LAZPERF:
    serpatch = pc_patch_lazperf_serialize(patch);
    break;
  default:
    pcerror("%s: unsupported compression type %d", __func__, patch->type);
  }

  if (patch != patch_in)
    pc_patch_free(patch);

  return serpatch;
}

/**
 * @brief Serialize a @c PCPATCH to its uncompressed @c PC_NONE form,
 *   regardless of the schema's nominal compression target.
 * @param patch_in Source in-memory patch.
 * @return Newly allocated @c SERIALIZED_PATCH carrying compression
 *   @c PC_NONE. Caller owns the result.
 */
SERIALIZED_PATCH *
meos_pc_patch_serialize_to_uncompressed(const PCPATCH *patch_in)
{
  PCPATCH *patch = (PCPATCH *)patch_in;
  SERIALIZED_PATCH *serpatch;

  if (patch->type != PC_NONE)
  {
    patch = pc_patch_uncompress(patch_in);
  }

  serpatch = pc_patch_uncompressed_serialize(patch);

  if (patch != patch_in)
    pc_patch_free(patch);

  return serpatch;
}

/*****************************************************************************
 * Static helpers — per-compression deserialize
 *****************************************************************************/

/**
 * @brief Deserialize an uncompressed @c SERIALIZED_PATCH.
 * @details The returned @c PCPATCH carries @c readonly=true and its
 *   @c data field aliases the input buffer; do not @c pc_patch_free
 *   the data area separately. A consistency check raises @c pcerror
 *   if @c VARSIZE doesn't agree with @c npoints * schema->size.
 */
static PCPATCH *
pc_patch_uncompressed_deserialize(const SERIALIZED_PATCH *serpatch,
                                  const PCSCHEMA *schema)
{
  uint8_t *buf;
  size_t stats_size = pc_stats_size(schema);
  PCPATCH_UNCOMPRESSED *patch = palloc(sizeof(PCPATCH_UNCOMPRESSED));

  patch->type = serpatch->compression;
  patch->schema = schema;
  patch->readonly = true;
  patch->npoints = serpatch->npoints;
  patch->maxpoints = 0;
  patch->bounds = serpatch->bounds;

  buf = (uint8_t *) serpatch->data;

  /* Point into the stats area */
  patch->stats = pc_patch_stats_deserialize(schema, buf);

  /* Advance data pointer past the stats serialization */
  patch->data = buf + stats_size;

  /* Calculate the point data buffer size */
  patch->datasize = VARSIZE(serpatch) - BUFFERALIGN(sizeof(SERIALIZED_PATCH))
                    + 1 - stats_size;
  if (patch->datasize != patch->npoints * schema->size)
    pcerror("%s: calculated patch data sizes don't match (%d != %d)", __func__,
            patch->datasize, patch->npoints * schema->size);

  return (PCPATCH *) patch;
}

/**
 * @brief Deserialize a dimensional-compressed @c SERIALIZED_PATCH.
 * @details Per-dimension @c PCBYTES arrays alias the input buffer
 *   (@c readonly=true). The container array of @c PCBYTES is freshly
 *   palloc'd.
 */
static PCPATCH *
pc_patch_dimensional_deserialize(const SERIALIZED_PATCH *serpatch,
                                 const PCSCHEMA *schema)
{
  PCPATCH_DIMENSIONAL *patch;
  int i;
  const uint8_t *buf;
  int ndims = schema->ndims;
  int npoints = serpatch->npoints;
  size_t stats_size = pc_stats_size(schema);

  /* Reference the external data */
  patch = palloc(sizeof(PCPATCH_DIMENSIONAL));

  /* Set up basic info */
  patch->type = serpatch->compression;
  patch->schema = schema;
  patch->readonly = true;
  patch->npoints = npoints;
  patch->bounds = serpatch->bounds;

  /* Point into the stats area */
  patch->stats = pc_patch_stats_deserialize(schema, serpatch->data);

  /* Set up dimensions */
  patch->bytes = palloc(ndims * sizeof(PCBYTES));
  memset(patch->bytes, 0, ndims * sizeof(PCBYTES));
  buf = serpatch->data + stats_size;

  for (i = 0; i < ndims; i++)
  {
    PCBYTES *pcb = &(patch->bytes[i]);
    PCDIMENSION *dim = schema->dims[i];
    pc_bytes_deserialize(buf, dim, pcb,
                         true /*readonly*/, false /*flipendian*/);
    pcb->npoints = npoints;
    buf += pc_bytes_serialized_size(pcb);
  }

  return (PCPATCH *) patch;
}

/**
 * @brief Deserialize a LAZ-perf-compressed @c SERIALIZED_PATCH.
 * @details Unlike the uncompressed and dimensional variants, the
 *   LAZ-perf payload is copied into a fresh palloc'd buffer (the
 *   decoder needs its own), not aliased.
 */
static PCPATCH *
pc_patch_lazperf_deserialize(const SERIALIZED_PATCH *serpatch,
                             const PCSCHEMA *schema)
{
  PCPATCH_LAZPERF *patch;
  uint32_t lazperfsize;
  int npoints = serpatch->npoints;
  size_t stats_size = pc_stats_size(schema);
  uint8_t *buf = (uint8_t *) serpatch->data + stats_size;

  /* Reference the external data */
  patch = palloc(sizeof(PCPATCH_LAZPERF));

  /* Set up basic info */
  patch->type = serpatch->compression;
  patch->schema = schema;
  patch->readonly = true;
  patch->npoints = npoints;
  patch->bounds = serpatch->bounds;

  /* Point into the stats area */
  patch->stats = pc_patch_stats_deserialize(schema, serpatch->data);

  /* Set up buffer */
  memcpy(&lazperfsize, buf, 4);
  patch->lazperfsize = lazperfsize;
  buf += 4;

  patch->lazperf = palloc(patch->lazperfsize);
  memcpy(patch->lazperf, buf, patch->lazperfsize);

  return (PCPATCH *) patch;
}

/*****************************************************************************
 * pcpatch deserialization — public dispatch
 *****************************************************************************/

/**
 * @brief Deserialize a @c SERIALIZED_PATCH into a fresh in-memory @c PCPATCH.
 * @param serpatch Source varlena.
 * @param schema   Schema matching @c serpatch->pcid (must not be NULL).
 * @return Newly allocated @c PCPATCH (compression-specific subtype),
 *   or @c NULL on unknown compression type. Caller releases via
 *   @c pc_patch_free.
 */
PCPATCH *
meos_pc_patch_deserialize(const SERIALIZED_PATCH *serpatch,
                          const PCSCHEMA *schema)
{
  switch (serpatch->compression)
  {
  case PC_NONE:
    return pc_patch_uncompressed_deserialize(serpatch, schema);
  case PC_DIMENSIONAL:
    return pc_patch_dimensional_deserialize(serpatch, schema);
  case PC_LAZPERF:
    return pc_patch_lazperf_deserialize(serpatch, schema);
  }
  pcerror("%s: unsupported compression type", __func__);
  return NULL;
}

#endif /* !PC_API_HAS_SERIALIZE */
