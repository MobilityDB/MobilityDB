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
 * @brief Per-point decomposition primitive for pcpatch.
 *
 * Provides a single C-side entry point that other MEOS pointcloud
 * operations (per-point @c atTpcbox, @c eIntersects, fast
 * @c points(tpcpatch) SRF, …) delegate into. Decomposes a serialized
 * @c Pcpatch into its constituent points, applies a caller-supplied
 * predicate, and rebuilds a new @c Pcpatch from the survivors — all
 * in C, no SQL roundtrip via @c PC_Explode.
 *
 * Depends on the transitional @c pgsql_compat shim while pgPointCloud's
 * @c pc_(point|patch)_(de)serialize helpers remain in @c pgsql/ rather
 * than @c lib/. See @c pgsql_compat.h for the kill-switch story.
 */

#ifndef __PCPATCH_DECOMPOSE_H__
#define __PCPATCH_DECOMPOSE_H__

#include <stdbool.h>

#include <liblwgeom.h>           /* for GSERIALIZED (predicate args) */
#include <meos_pointcloud.h>     /* for Pcpatch, TPCBox */
#include "pc_api.h"              /* for PCPOINT */

/**
 * @brief Per-point predicate signature.
 *
 * Returns @c true to keep the point in the rebuilt patch, @c false to
 * drop it. The opaque @c extra argument is whatever the caller of
 * @c pcpatch_filter_per_point passes through verbatim — typically a
 * bbox struct, a geometry pointer, or a small parameter pack.
 */
typedef bool (*pcpatch_pointpred_fn)(const PCPOINT *pt, void *extra);

/**
 * @brief Decompose a pcpatch, apply a predicate per point, rebuild.
 *
 * @param pa              Source pcpatch (must not be NULL).
 * @param pred            Predicate (must not be NULL).
 * @param extra           Caller-supplied state passed verbatim to @c pred.
 * @param keep_when_true  When @c true (atfunc semantics), keep points
 *                        for which @c pred returns true; when @c false
 *                        (minus semantics), keep the complement.
 * @return Newly allocated pcpatch holding only the surviving points
 *         (always uncompressed-form output), or @c NULL when every
 *         point was dropped or the schema for @c pa->pcid cannot be
 *         resolved via the schema hook. Caller owns the result.
 */
extern Pcpatch *pcpatch_filter_per_point(const Pcpatch *pa,
  pcpatch_pointpred_fn pred, void *extra, bool keep_when_true);

/**
 * @brief Test whether at least one point of @p pa satisfies @p pred.
 *
 * Walks the patch's points in order and returns @c true on the first
 * match — no patch rebuild, no allocations beyond the temporary
 * decompression. Useful for ever / any-style predicates that don't
 * need the filtered output.
 *
 * @return @c true if at least one point matched, @c false if none did
 *         or the schema cannot be resolved.
 */
extern bool pcpatch_any_point_matches(const Pcpatch *pa,
  pcpatch_pointpred_fn pred, void *extra);

/*****************************************************************************
 * Built-in predicates
 *
 * Predicate adapters that pcpatch_filter_per_point can use directly via
 * its @c pred argument. Each pairs with a small @c *Args struct defining
 * what to pass through @c extra.
 *****************************************************************************/

/**
 * @brief Closure for #pcpoint_in_tpcbox.
 *
 * @c border_inc=true makes the bbox membership test inclusive on every
 * face (the typical SQL semantics); @c false uses strict @c < / @c >.
 */
typedef struct
{
  const TPCBox *box;
  bool border_inc;
} PcpointInTpcboxArgs;

/**
 * @brief Predicate: keep points whose XY (and Z, if @c box->flags has Z)
 *   coordinates fall inside the supplied @c TPCBox.
 *
 * @param pt    pcpoint under test.
 * @param extra Pointer to a #PcpointInTpcboxArgs.
 */
extern bool pcpoint_in_tpcbox(const PCPOINT *pt, void *extra);

/**
 * @brief Predicate: keep points whose XY projection intersects the supplied
 *   2D geometry (@c GSERIALIZED *).
 *
 * @param pt    pcpoint under test.
 * @param extra Pointer to a @c GSERIALIZED * (the clipping geometry).
 *              Its SRID is used when constructing the per-point probe;
 *              MobilityDB-side wrappers should validate SRID
 *              compatibility with the patch schema before calling.
 */
extern bool pcpoint_intersects_geometry(const PCPOINT *pt, void *extra);

#endif /* __PCPATCH_DECOMPOSE_H__ */
