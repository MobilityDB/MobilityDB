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
 * @brief @c pcpatch_filter_per_point — the decompose / filter / rebuild
 * primitive that per-point operators delegate into.
 */

#include "pointcloud/pcpatch_decompose.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>             /* for MEOS_FLAGS_GET_Z */
#include "pointcloud/pcpatch.h"
#include "pointcloud/pgsql_compat.h"
#include "pointcloud/meos_schema_hook.h"
#include "geo/tgeo_spatialfuncs.h"     /* for geopoint_make */
/* pgPointCloud */
#include "pc_api.h"
#include "pc_api_internal.h"   /* for pc_patch_uncompressed_make / _add_point */

/*****************************************************************************/

/**
 * @brief Decompose @p pa, apply @p pred per point, rebuild a survivor patch.
 *
 * See @ref pcpatch_filter_per_point in @c pcpatch_decompose.h for the
 * full parameter and return-value contract.
 */
Pcpatch *
pcpatch_filter_per_point(const Pcpatch *pa, pcpatch_pointpred_fn pred,
  void *extra, bool keep_when_true)
{
  assert(pa); assert(pred);

  /* Schema is needed both for deserialize and for rebuilding. */
  PCSCHEMA *schema = meos_pc_schema(pa->pcid);
  if (! schema)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "pcpatch_filter_per_point: no schema registered for pcid %u",
      pa->pcid);
    return NULL;
  }

  /* Pcpatch is byte-compatible with SERIALIZED_PATCH (see pcpatch.h). */
  PCPATCH *patch = MEOS_PC_PATCH_DESERIALIZE(
    (const SERIALIZED_PATCH *) pa, schema);
  if (! patch)
    return NULL;

  PCPOINTLIST *pl = pc_pointlist_from_patch(patch);
  if (! pl)
  {
    pc_patch_free(patch);
    return NULL;
  }

  /* Build the survivor patch in uncompressed form. Capacity = pl->npoints
   * is an upper bound; pc_patch_uncompressed_add_point grows internally
   * if exceeded but pre-sizing avoids reallocation churn. */
  PCPATCH_UNCOMPRESSED *out = pc_patch_uncompressed_make(schema, pl->npoints);
  for (uint32_t i = 0; i < pl->npoints; i++)
  {
    PCPOINT *pt = pc_pointlist_get_point(pl, i);
    if (pred(pt, extra) == keep_when_true)
      pc_patch_uncompressed_add_point(out, pt);
  }

  Pcpatch *result = NULL;
  if (out->npoints > 0)
  {
    /* Recompute extent + stats so the serialized header is consistent
     * with the survivor set (bounds matter for downstream bbox prune). */
    pc_patch_uncompressed_compute_extent(out);
    pc_patch_uncompressed_compute_stats(out);
    SERIALIZED_PATCH *ser = MEOS_PC_PATCH_SERIALIZE((PCPATCH *) out, NULL);
    result = (Pcpatch *) ser;
  }

  pc_patch_free((PCPATCH *) out);
  pc_pointlist_free(pl);
  pc_patch_free(patch);
  return result;
}

/*****************************************************************************
 * Existence walk (short-circuit, no rebuild)
 *****************************************************************************/

/**
 * @brief Test whether at least one point of @p pa satisfies @p pred.
 *
 * See @ref pcpatch_any_point_matches in @c pcpatch_decompose.h for the
 * full contract.
 */
bool
pcpatch_any_point_matches(const Pcpatch *pa, pcpatch_pointpred_fn pred,
  void *extra)
{
  assert(pa); assert(pred);

  PCSCHEMA *schema = meos_pc_schema(pa->pcid);
  if (! schema)
    return false;

  PCPATCH *patch = MEOS_PC_PATCH_DESERIALIZE(
    (const SERIALIZED_PATCH *) pa, schema);
  if (! patch)
    return false;

  PCPOINTLIST *pl = pc_pointlist_from_patch(patch);
  bool found = false;
  if (pl)
  {
    for (uint32_t i = 0; i < pl->npoints && ! found; i++)
    {
      PCPOINT *pt = pc_pointlist_get_point(pl, i);
      if (pred(pt, extra))
        found = true;
    }
    pc_pointlist_free(pl);
  }
  pc_patch_free(patch);
  return found;
}

/*****************************************************************************
 * Built-in predicates
 *****************************************************************************/

/**
 * @brief Predicate: keep points inside a @c TPCBox.
 *
 * See @ref pcpoint_in_tpcbox in @c pcpatch_decompose.h for the
 * @c extra argument shape and the strict-vs-inclusive border
 * semantics.
 */
bool
pcpoint_in_tpcbox(const PCPOINT *pt, void *extra)
{
  assert(pt); assert(extra);
  const PcpointInTpcboxArgs *args = (const PcpointInTpcboxArgs *) extra;
  const TPCBox *box = args->box;
  bool inc = args->border_inc;

  double x, y;
  if (! pc_point_get_x(pt, &x) || ! pc_point_get_y(pt, &y))
    return false;

  /* x / y always present in a TPCBox with the X flag — pcpatch lift
   * always produces one. The strict-vs-inclusive cases factor through
   * the inc flag. */
  if (inc)
  {
    if (x < box->xmin || x > box->xmax) return false;
    if (y < box->ymin || y > box->ymax) return false;
  }
  else
  {
    if (x <= box->xmin || x >= box->xmax) return false;
    if (y <= box->ymin || y >= box->ymax) return false;
  }

  /* Z dimension is optional — if the box carries it, the point's
   * schema must be capable of yielding a Z value, otherwise the test
   * cannot be made and we fail closed (drop the point). */
  if (MEOS_FLAGS_GET_Z(box->flags))
  {
    double z;
    if (! pc_point_get_z(pt, &z))
      return false;
    if (inc)
    {
      if (z < box->zmin || z > box->zmax) return false;
    }
    else
    {
      if (z <= box->zmin || z >= box->zmax) return false;
    }
  }

  return true;
}

/**
 * @brief Predicate: keep points whose XY projection intersects a 2D
 *   geometry.
 *
 * See @ref pcpoint_intersects_geometry in @c pcpatch_decompose.h for
 * the @c extra argument shape and SRID-handling notes.
 */
bool
pcpoint_intersects_geometry(const PCPOINT *pt, void *extra)
{
  assert(pt); assert(extra);
  const GSERIALIZED *gs = (const GSERIALIZED *) extra;

  double x, y;
  if (! pc_point_get_x(pt, &x) || ! pc_point_get_y(pt, &y))
    return false;

  /* Probe is a 2D point in the geometry's SRID; the wrapper layer is
   * responsible for SRID-mismatch validation between patch schema and
   * geometry before getting here. */
  GSERIALIZED *probe = geopoint_make(x, y, 0.0, false, false,
    gserialized_get_srid(gs));
  bool result = geom_intersects2d(probe, gs);
  pfree(probe);
  return result;
}

/*****************************************************************************/
