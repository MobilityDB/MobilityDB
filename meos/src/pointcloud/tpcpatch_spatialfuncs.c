/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Geometric representation of the pcpatch and tpcpatch types
 *
 * A patch is a cluster of points, so its geometry is the @p MULTIPOINT of the
 * positions its points occupy, read through the schema its pcid names. Lifting
 * that value-level answer over time gives @ref tpcpatch_to_tgeometry, which is
 * what the type's temporal spatial relationships convert to before delegating
 * to the one geometry engine — the same route @ref tpointcloud_to_tgeompoint
 * opens for a tpcpoint, differing only in that a cluster of points is a
 * @p tgeometry where a single point is a @p tgeompoint.
 */

/* C */
#include <assert.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* pgPointCloud */
#include <pc_api.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"  /* Temporal, TInstant */
#include "temporal/meos_catalog.h"  /* T_TGEOMETRY */
#include "geo/geo_funcs.h"  /* geo_serialize */
#include "pointcloud/pcpatch.h" /* struct Pcpatch body (.pcid field) */
#include "pointcloud/pgsql_compat.h"  /* MEOS_PC_PATCH_DESERIALIZE */
#include "pointcloud/meos_schema_hook.h"  /* meos_pc_schema */

/*****************************************************************************
 * Conversion of a patch into its geometry
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_base_conversion
 * @brief Convert a patch into the multipoint of the positions its points
 * occupy
 * @details The schema the patch's pcid names decides the SRID of the result
 * and whether it carries a Z dimension. A point whose coordinates that schema
 * cannot yield is left out of the multipoint rather than placed at the origin.
 * @param[in] pa Patch
 * @return @p NULL if the pcid schema cannot be resolved
 * @csqlfn #Pcpatch_to_geom()
 */
GSERIALIZED *
pcpatch_to_geom(const Pcpatch *pa)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pa, NULL);
  PCSCHEMA *schema = meos_pc_schema(pa->pcid);
  if (! schema)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "No schema registered for pcid %u", pa->pcid);
    return NULL;
  }

  /* Pcpatch is byte-compatible with SERIALIZED_PATCH (see pcpatch.h) */
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

  int32_t srid = (int32_t) schema->srid;
  uint8_t hasz = (schema->zdim != NULL) ? 1 : 0;
  LWMPOINT *mpoint = lwmpoint_construct_empty(srid, hasz, /* hasm */ 0);
  for (uint32_t i = 0; i < pl->npoints; i++)
  {
    const PCPOINT *pt = pc_pointlist_get_point(pl, i);
    POINT4D p4d;
    memset(&p4d, 0, sizeof(POINT4D));
    if (! pc_point_get_x(pt, &p4d.x) || ! pc_point_get_y(pt, &p4d.y))
      continue;
    if (hasz && ! pc_point_get_z(pt, &p4d.z))
      continue;
    lwmpoint_add_lwpoint(mpoint, lwpoint_make(srid, hasz, /* hasm */ 0, &p4d));
  }

  GSERIALIZED *result = geo_serialize((LWGEOM *) mpoint);
  lwmpoint_free(mpoint);
  pc_pointlist_free(pl);
  pc_patch_free(patch);
  return result;
}

/*****************************************************************************
 * Conversion of a temporal patch into a temporal geometry
 *****************************************************************************/

/**
 * @brief Return the temporal geometry instant equivalent to a temporal patch
 * instant
 */
static TInstant *
tpcpatchinst_to_tgeometryinst(const TInstant *inst)
{
  const Pcpatch *pa =
    (const Pcpatch *) DatumGetPointer(tinstant_value_p(inst));
  GSERIALIZED *gs = pcpatch_to_geom(pa);
  if (! gs)
    return NULL;
  TInstant *result = tinstant_make(PointerGetDatum(gs), T_TGEOMETRY, inst->t);
  pfree(gs);
  return result;
}

/**
 * @brief Return the temporal geometry sequence equivalent to a temporal patch
 * sequence
 */
static TSequence *
tpcpatchseq_to_tgeometryseq(const TSequence *seq)
{
  TInstant **insts = palloc(sizeof(TInstant *) * seq->count);
  int ninsts = 0;
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = tpcpatchinst_to_tgeometryinst(TSEQUENCE_INST_N(seq, i));
    if (inst)
      insts[ninsts++] = inst;
  }
  /* The interpolation of the source is kept: consecutive patches hold no
   * correspondence between their points that a reading along time could
   * interpolate, which is why a tpcpatch carries step interpolation */
  return tsequence_make_free(insts, ninsts, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
}

/**
 * @brief Return the temporal geometry sequence set equivalent to a temporal
 * patch sequence set
 */
static TSequenceSet *
tpcpatchseqset_to_tgeometryseqset(const TSequenceSet *ss)
{
  TSequence **seqs = palloc(sizeof(TSequence *) * ss->count);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *seq = tpcpatchseq_to_tgeometryseq(TSEQUENCESET_SEQ_N(ss, i));
    if (seq)
      seqs[nseqs++] = seq;
  }
  return tsequenceset_make_free(seqs, nseqs, NORMALIZE);
}

/**
 * @ingroup meos_pointcloud_conversion
 * @brief Convert a temporal patch into the temporal geometry of the
 * multipoints its patches occupy
 * @param[in] temp Temporal patch
 * @return @p NULL if the pcid schema cannot be resolved
 * @csqlfn #Tpcpatch_to_tgeometry()
 */
Temporal *
tpcpatch_to_tgeometry(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPATCH(temp, NULL);
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tpcpatchinst_to_tgeometryinst(
        (const TInstant *) temp);
    case TSEQUENCE:
      return (Temporal *) tpcpatchseq_to_tgeometryseq(
        (const TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tpcpatchseqset_to_tgeometryseqset(
        (const TSequenceSet *) temp);
  }
}

/*****************************************************************************/
