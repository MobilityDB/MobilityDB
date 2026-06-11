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

/**
 * @file
 * @brief Transitional compatibility layer for pgPointCloud's varlena ↔
 *   in-memory bridge.
 *
 * pgPointCloud keeps `pc_(point|patch)_(de)serialize` in `pgsql/pc_pgsql.c`,
 * which is compiled into the PG extension `.so` only — they don't reach
 * `libpc.a` and so are unreachable from MEOS, which links the static
 * library. An upstream proposal to move these helpers into `lib/` so
 * they land in `libpc.a` is pending against pgpointcloud/pointcloud.
 *
 * Until that change is upstream and the subtree is bumped, this header
 * declares vendored copies under a `meos_pc_*` prefix and routes call
 * sites via the MEOS_PC_* macros below. When upstream catches up, the
 * CMake probe sets @c PC_API_HAS_SERIALIZE=1 and the macros switch to
 * the upstream symbols; this file and its companion @c pgsql_compat.c
 * can then be removed.
 */

#ifndef __PGSQL_COMPAT_H__
#define __PGSQL_COMPAT_H__

#include <stdint.h>
#include <stddef.h>

#include "pc_api.h"

/**
 * @brief Serialized varlena layout for a single pcpoint.
 *
 * Mirrors @c SERIALIZED_POINT in @c pgsql/pc_pgsql.h. The on-disk byte
 * layout is load-bearing in every existing pgPointCloud database; the
 * upstream PR moves this typedef into @c lib/pc_api.h without changing
 * the layout.
 */
typedef struct
{
  uint32_t size;
  uint32_t pcid;
  uint8_t  data[1];
} SERIALIZED_POINT;

/**
 * @brief Serialized varlena layout for a pcpatch.
 *
 * Mirrors @c SERIALIZED_PATCH in @c pgsql/pc_pgsql.h. As with
 * SERIALIZED_POINT, the layout is load-bearing.
 */
typedef struct
{
  uint32_t size;
  uint32_t pcid;
  uint32_t compression;
  uint32_t npoints;
  PCBOUNDS bounds;
  uint8_t  data[1];
} SERIALIZED_PATCH;

#if !defined(PC_API_HAS_SERIALIZE) || !PC_API_HAS_SERIALIZE

/** @brief Serialize a @c PCPOINT to a fresh varlena. See full contract
 *   in @c pgsql_compat.c. */
extern SERIALIZED_POINT *meos_pc_point_serialize(const PCPOINT *pcpt);
/** @brief Deserialize a @c SERIALIZED_POINT to a fresh @c PCPOINT.
 *   Returns @c NULL on schema/payload size mismatch. */
extern PCPOINT          *meos_pc_point_deserialize(
                            const SERIALIZED_POINT *serpt,
                            const PCSCHEMA *schema);
/** @brief Compute the @c SERIALIZED_PATCH size for an in-memory patch. */
extern size_t            meos_pc_patch_serialized_size(const PCPATCH *patch);
/** @brief Serialize a @c PCPATCH at its schema's target compression. */
extern SERIALIZED_PATCH *meos_pc_patch_serialize(const PCPATCH *patch_in,
                            void *userdata);
/** @brief Serialize a @c PCPATCH forced to uncompressed @c PC_NONE form. */
extern SERIALIZED_PATCH *meos_pc_patch_serialize_to_uncompressed(
                            const PCPATCH *patch_in);
/** @brief Deserialize a @c SERIALIZED_PATCH to a fresh @c PCPATCH
 *   (compression-specific subtype). */
extern PCPATCH          *meos_pc_patch_deserialize(
                            const SERIALIZED_PATCH *serpatch,
                            const PCSCHEMA *schema);

#define MEOS_PC_POINT_SERIALIZE(p)              meos_pc_point_serialize((p))
#define MEOS_PC_POINT_DESERIALIZE(p, s)         meos_pc_point_deserialize((p), (s))
#define MEOS_PC_PATCH_SERIALIZED_SIZE(p)        meos_pc_patch_serialized_size((p))
#define MEOS_PC_PATCH_SERIALIZE(p, u)           meos_pc_patch_serialize((p), (u))
#define MEOS_PC_PATCH_SERIALIZE_TO_UNCOMP(p)    meos_pc_patch_serialize_to_uncompressed((p))
#define MEOS_PC_PATCH_DESERIALIZE(p, s)         meos_pc_patch_deserialize((p), (s))

#else /* PC_API_HAS_SERIALIZE — upstream merged the move; use it directly */

#define MEOS_PC_POINT_SERIALIZE(p)              pc_point_serialize((p))
#define MEOS_PC_POINT_DESERIALIZE(p, s)         pc_point_deserialize((p), (s))
#define MEOS_PC_PATCH_SERIALIZED_SIZE(p)        pc_patch_serialized_size((p))
#define MEOS_PC_PATCH_SERIALIZE(p, u)           pc_patch_serialize((p), (u))
#define MEOS_PC_PATCH_SERIALIZE_TO_UNCOMP(p)    pc_patch_serialize_to_uncompressed((p))
#define MEOS_PC_PATCH_DESERIALIZE(p, s)         pc_patch_deserialize((p), (s))

#endif /* PC_API_HAS_SERIALIZE */

#endif /* __PGSQL_COMPAT_H__ */
