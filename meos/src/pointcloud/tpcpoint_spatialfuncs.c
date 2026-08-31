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
 * @brief Per-point spatial predicates for the tpcpoint temporal type.
 *
 * Provides the two per-event streaming entry-points (e.g. MobilityNebula NES
 * operators): @ref nad_tpcpoint_geo and @ref eintersects_tpcpoint_geo. The
 * instant constructor @ref tpcpointinst_make lives with the rest of the
 * Temporal<T> value surface in @ref tpcpoint.c.
 */

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include <varatt.h>
/* pgPointCloud */
#include <pc_api.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"  /* Temporal, TInstant */
#include "temporal/meos_catalog.h"  /* T_TPCPOINT */
#include "geo/tgeo_spatialfuncs.h"  /* geopoint_make */
#include "pointcloud/pcpoint.h" /* struct Pcpoint body (.pcid field) */

/*****************************************************************************
 * Internal helpers
 *****************************************************************************/

/**
 * @brief Return a palloc'd 2D geometry point for the position in @p pt, or
 * NULL when the schema lacks X or Y dimensions.
 */
static GSERIALIZED *
pcpoint_to_geompoint2d(const Pcpoint *pt)
{
  PCSCHEMA *schema = meos_pc_schema(pt->pcid);
  if (!schema)
    return NULL;
  double x = 0.0, y = 0.0;
  if (!pcpoint_get_x(pt, schema, &x) || !pcpoint_get_y(pt, schema, &y))
    return NULL;
  return geompoint_make2d((int32_t) schema->srid, x, y);
}

/**
 * @brief Return the Pcpoint payload of a tpcpoint instant.
 */
static const Pcpoint *
tpointcloudinst_pcpoint(const Temporal *temp)
{
  assert(temp->temptype == T_TPCPOINT);
  return (const Pcpoint *) DatumGetPointer(
    tinstant_value_p((const TInstant *) temp));
}

/*****************************************************************************
 * Projection to tgeompoint
 *
 * Per-instant mapper: pcpoint -> POINT gserialized with the schema's SRID
 * and Z-presence, read through the schema-aware public accessors
 * (pcpoint_get_x/y/z). The schema is resolved once from the value's
 * common pcid (enforced at construction time by set_make_exp's
 * same-pcid check) and shared across every instant.
 *****************************************************************************/

/**
 * @brief Return the temporal geometry point instant equivalent to a
 * temporal pointcloud instant
 */
static TInstant *
tpointcloudinst_to_tgeompointinst(const TInstant *inst, PCSCHEMA *schema)
{
  const Pcpoint *pt =
    (const Pcpoint *) DatumGetPointer(tinstant_value_p(inst));
  double x, y, z = 0.0;
  if (! pcpoint_get_x(pt, schema, &x) || ! pcpoint_get_y(pt, schema, &y))
    return NULL;
  bool hasz = (schema->zdim != NULL);
  if (hasz && ! pcpoint_get_z(pt, schema, &z))
    return NULL;
  GSERIALIZED *gs = geopoint_make(x, y, z, hasz, /* geodetic */ false,
    (int32_t) schema->srid);
  TInstant *result = tinstant_make(PointerGetDatum(gs), T_TGEOMPOINT, inst->t);
  pfree(gs);
  return result;
}

/**
 * @brief Return the temporal geometry point sequence equivalent to a
 * temporal pointcloud sequence
 */
static TSequence *
tpointcloudseq_to_tgeompointseq(const TSequence *seq, PCSCHEMA *schema)
{
  TInstant **insts = palloc(sizeof(TInstant *) * seq->count);
  int n = 0;
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *out = tpointcloudinst_to_tgeompointinst(
      TSEQUENCE_INST_N(seq, i), schema);
    if (! out) continue;
    insts[n++] = out;
  }
  /* Step-interpolated in; linear is semantically meaningful for the
   * projected XY path (the sensor physically moved), so we promote. */
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  if (interp == STEP) interp = LINEAR;
  return tsequence_make_free(insts, n, seq->period.lower_inc,
    seq->period.upper_inc, interp, NORMALIZE);
}

/**
 * @brief Return the temporal geometry point sequence set equivalent to a
 * temporal pointcloud sequence set
 */
static TSequenceSet *
tpointcloudseqset_to_tgeompointseqset(const TSequenceSet *ss, PCSCHEMA *schema)
{
  TSequence **seqs = palloc(sizeof(TSequence *) * ss->count);
  int n = 0;
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *out = tpointcloudseq_to_tgeompointseq(
      TSEQUENCESET_SEQ_N(ss, i), schema);
    if (! out) continue;
    seqs[n++] = out;
  }
  return tsequenceset_make_free(seqs, n, NORMALIZE);
}

/**
 * @ingroup meos_pointcloud_conversion
 * @brief Return a temporal pointcloud value projected onto a temporal
 * geometry point by extracting X/Y/[Z] from each instant's pcpoint via
 * the schema cache
 * @param[in] temp Temporal pointcloud value
 * @return NULL if the pcid schema cannot be resolved
 * @csqlfn #Tpcpoint_to_tgeompoint()
 */
Temporal *
tpointcloud_to_tgeompoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPOINT(temp, NULL);
  const Pcpoint *first =
    (const Pcpoint *) DatumGetPointer(temporal_start_value(temp));
  PCSCHEMA *schema = meos_pc_schema(first->pcid);
  if (! schema)
    return NULL;
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tpointcloudinst_to_tgeompointinst(
        (const TInstant *) temp, schema);
    case TSEQUENCE:
      return (Temporal *) tpointcloudseq_to_tgeompointseq(
        (const TSequence *) temp, schema);
    default: /* TSEQUENCESET */
      return (Temporal *) tpointcloudseqset_to_tgeompointseqset(
        (const TSequenceSet *) temp, schema);
  }
}

/*****************************************************************************
 * Spatial predicates and nearest-approach distance
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_ever
 * @brief Return true if the tpcpoint instant position intersects the geometry
 * @param[in] temp Temporal pointcloud value (single instant)
 * @param[in] gs Geometry
 */
bool
eintersects_tpcpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, false); VALIDATE_NOT_NULL(gs, false);
  GSERIALIZED *probe = pcpoint_to_geompoint2d(tpointcloudinst_pcpoint(temp));
  if (!probe)
    return false;
  bool result = geom_intersects2d(probe, gs);
  pfree(probe);
  return result;
}

/**
 * @ingroup meos_pointcloud_dist
 * @brief Return the nearest-approach distance between a tpcpoint instant and
 *   a geometry
 * @param[in] temp Temporal pointcloud value (single instant)
 * @param[in] gs Geometry
 * @return @p DBL_MAX on error (missing X/Y dimensions or NULL input)
 */
double
nad_tpcpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, DBL_MAX); VALIDATE_NOT_NULL(gs, DBL_MAX);
  GSERIALIZED *probe = pcpoint_to_geompoint2d(tpointcloudinst_pcpoint(temp));
  if (!probe)
    return DBL_MAX;
  double result = geom_distance2d(probe, gs);
  pfree(probe);
  return result;
}

/*****************************************************************************/
