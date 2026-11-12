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
 * @brief Per-point spatial predicates for the tpcpoint temporal type
 * @details Provides the two per-event streaming entry-points (e.g.
 * MobilityNebula NES
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
#include <meos_internal_geo.h>
#include "temporal/temporal.h"  /* Temporal, TInstant */
#include "temporal/meos_catalog.h"  /* T_TPCPOINT */
#include "temporal/type_util.h"  /* pfree_array */
#include "geo/tgeo_spatialfuncs.h"  /* tpointinst_make_coords */
#include "pointcloud/pcpoint.h" /* struct Pcpoint body (.pcid field) */
#include "pointcloud/tpc_boxops.h"  /* tpcpoint_restrict_tpcbox */

/*****************************************************************************
 * Internal helpers
 *****************************************************************************/

/**
 * @brief Return a palloc'd 2D geometry point for the position in @p pt, or NULL
 * when the schema lacks X or Y dimensions
 */
static GSERIALIZED *
pcpoint_to_geompoint2d(const Pcpoint *pt)
{
  PCSCHEMA *schema = meos_pc_schema(pt->pcid);
  if (!schema)
    return NULL;
  double x = 0.0, y = 0.0;
  if (!pcpoint_schema_get_x(pt, schema, &x) || !pcpoint_schema_get_y(pt, schema, &y))
    return NULL;
  return geompoint_make2d((int32_t) schema->srid, x, y);
}

/**
 * @brief Return the Pcpoint payload of a tpcpoint instant
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
 * temporal pointcloud instant, or NULL when a coordinate leaves the doubles
 * @details A coordinate is the stored value of its dimension scaled and
 * offset by the schema, so it is computed and tested as every computed
 * result is (#ensure_finite_result), and the point is made from the
 * coordinates by #tpointinst_make_coords without testing them again
 */
static TInstant *
tpcpointinst_to_tgeompointinst(const TInstant *inst, PCSCHEMA *schema)
{
  const Pcpoint *pt =
    (const Pcpoint *) DatumGetPointer(tinstant_value_p(inst));
  double x, y, z = 0.0;
  if (! pcpoint_schema_get_x(pt, schema, &x) || ! pcpoint_schema_get_y(pt, schema, &y))
    return NULL;
  bool hasz = (schema->zdim != NULL);
  if (hasz && ! pcpoint_schema_get_z(pt, schema, &z))
    return NULL;
  if (! ensure_finite_result(x) || ! ensure_finite_result(y) ||
      (hasz && ! ensure_finite_result(z)))
    return NULL;
  return tpointinst_make_coords(x, y, z, hasz, /* geodetic */ false,
    (int32_t) schema->srid, inst->t);
}

/**
 * @brief Return the temporal geometry point sequence equivalent to a
 * temporal pointcloud sequence
 */
static TSequence *
tpcpointseq_to_tgeompointseq(const TSequence *seq, PCSCHEMA *schema)
{
  TInstant **insts = palloc(sizeof(TInstant *) * seq->count);
  int n = 0;
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *out = tpcpointinst_to_tgeompointinst(
      TSEQUENCE_INST_N(seq, i), schema);
    /* An instant that is not converted fails the conversion */
    if (! out)
    {
      pfree_array((void **) insts, n);
      return NULL;
    }
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
tpcpointseqset_to_tgeompointseqset(const TSequenceSet *ss, PCSCHEMA *schema)
{
  TSequence **seqs = palloc(sizeof(TSequence *) * ss->count);
  int n = 0;
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *out = tpcpointseq_to_tgeompointseq(
      TSEQUENCESET_SEQ_N(ss, i), schema);
    /* A sequence that is not converted fails the conversion */
    if (! out)
    {
      pfree_array((void **) seqs, n);
      return NULL;
    }
    seqs[n++] = out;
  }
  return tsequenceset_make_free(seqs, n, NORMALIZE);
}

/**
 * @ingroup meos_pointcloud_conversion
 * @brief Return a temporal pointcloud value projected onto a temporal
 * geometry point
 * @details The X/Y/[Z] coordinates are extracted from each instant's pcpoint
 * via the schema cache.
 * @param[in] temp Temporal pointcloud value
 * @return NULL if the pcid schema cannot be resolved
 * @csqlfn #Tpcpoint_to_tgeompoint()
 */
Temporal *
tpcpoint_to_tgeompoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPOINT(temp, NULL);
  /* The schema is the one of the first value, read in place */
  const Pcpoint *first = (const Pcpoint *) DatumGetPointer(
    tinstant_value_p(temporal_start_inst(temp)));
  PCSCHEMA *schema = meos_pc_schema(first->pcid);
  if (! schema)
    return NULL;
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tpcpointinst_to_tgeompointinst(
        (const TInstant *) temp, schema);
    case TSEQUENCE:
      return (Temporal *) tpcpointseq_to_tgeompointseq(
        (const TSequence *) temp, schema);
    default: /* TSEQUENCESET */
      return (Temporal *) tpcpointseqset_to_tgeompointseqset(
        (const TSequenceSet *) temp, schema);
  }
}

/**
 * @ingroup meos_pointcloud_conversion
 * @brief Convert a temporal point cloud point into a temporal geography point
 * @details The value converts through #tpcpoint_to_tgeompoint and #tgeompoint_to_tgeogpoint, as every spatial type
 * does
 * @param[in] temp Temporal value
 * @errval NULL
 * @csqlfn #Tpcpoint_to_tgeogpoint()
 */
Temporal *
tpcpoint_to_tgeogpoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPOINT(temp, NULL);
  Temporal *value = tpcpoint_to_tgeompoint(temp);
  if (! value)
    return NULL;
  Temporal *result = tgeompoint_to_tgeogpoint(value);
  pfree(value);
  return result;
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
  VALIDATE_TPCPOINT(temp, false); VALIDATE_NOT_NULL(gs, false);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return false;
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
 * @errval -1.0
 */
double
nad_tpcpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPOINT(temp, -1.0); VALIDATE_NOT_NULL(gs, -1.0);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return -1.0;
  GSERIALIZED *probe = pcpoint_to_geompoint2d(tpointcloudinst_pcpoint(temp));
  if (!probe)
    return -1.0;
  double result = geom_distance2d(probe, gs);
  pfree(probe);
  return result;
}

/*****************************************************************************
 * Bbox-based restrictions — at_tpcbox / minus_tpcbox
 *
 * Strategy: project the tpcpoint onto a tgeompoint via the schema
 * cache, restrict the projection by the equivalent stbox (TPCBox shares
 * the STBox prefix layout — copy the first 80 bytes), extract the time
 * span of the result, and restrict the ORIGINAL tpcpoint by that time
 * span.  Restricting the original (not the projected tgeompoint and
 * casting back) preserves the full pcpoint values, including
 * non-spatial dimensions like Intensity / Classification.
 *****************************************************************************/

/**
 * @brief Restrict (or remove) a tpcpoint by a TPCBox
 * @details A box of another schema or reference system raises an error
 * @return Newly-palloc'd Temporal, or @p NULL when the result is empty.
 */
Temporal *
tpcpoint_restrict_tpcbox(const Temporal *temp, const TPCBox *box,
  bool border_inc, bool atfunc)
{
  /* Ensure the validity of the arguments: a box of another schema or
   * reference system raises, as the overlaps of their boxes raises */
  VALIDATE_TPCPOINT(temp, NULL);
  if (! ensure_valid_tpointcloud_tpcbox(temp, box))
    return NULL;

  /* Project tpcpoint -> tgeompoint via the shared MEOS conversion. */
  Temporal *tpoint = tpcpoint_to_tgeompoint(temp);
  if (! tpoint)
    return atfunc ? NULL : temporal_copy(temp);

  /* TPCBox shares the period + xyz + srid prefix of STBox, but inserts
   * pcid before flags, so the flags byte offsets differ.  Build the
   * STBox explicitly, mirroring the box's own dimension flags, with
   * GEODETIC cleared. */
  STBox sbox;
  memset(&sbox, 0, sizeof(STBox));
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  if (hast)
    sbox.period = box->period;
  sbox.xmin = box->xmin; sbox.ymin = box->ymin; sbox.zmin = box->zmin;
  sbox.xmax = box->xmax; sbox.ymax = box->ymax; sbox.zmax = box->zmax;
  sbox.srid = box->srid;
  MEOS_FLAGS_SET_X(sbox.flags, MEOS_FLAGS_GET_X(box->flags));
  MEOS_FLAGS_SET_Z(sbox.flags, MEOS_FLAGS_GET_Z(box->flags));
  MEOS_FLAGS_SET_T(sbox.flags, hast);
  MEOS_FLAGS_SET_GEODETIC(sbox.flags, false);

  /* Restrict projection by the equivalent stbox.  An empty restriction
   * of the pcid-matched projection is an empty result for at and minus
   * alike: a NULL minus means no part of the value lies outside the
   * box. */
  Temporal *tpoint_rest = tgeo_restrict_stbox(tpoint, &sbox, border_inc, atfunc);
  pfree(tpoint);
  if (! tpoint_rest)
    return NULL;

  /* Lift the result's time span back onto the ORIGINAL tpcpoint to
   * recover the full pcpoint values at each surviving instant. */
  SpanSet *ss = temporal_time(tpoint_rest);
  Temporal *result = temporal_restrict_tstzspanset(temp, ss, REST_AT);
  pfree(ss);
  pfree(tpoint_rest);
  return result;
}

/**
 * @ingroup meos_pointcloud_restrict
 * @brief Return a temporal point cloud point restricted to a point cloud box
 * @param[in] temp Temporal point cloud point
 * @param[in] box Point cloud box
 * @param[in] border_inc True when the box contains the upper border
 * @csqlfn #Tpcpoint_at_tpcbox()
 */
Temporal *
tpcpoint_at_tpcbox(const Temporal *temp, const TPCBox *box, bool border_inc)
{
  return tpcpoint_restrict_tpcbox(temp, box, border_inc, REST_AT);
}

/**
 * @ingroup meos_pointcloud_restrict
 * @brief Return a temporal point cloud point restricted to the complement of a point cloud box
 * @param[in] temp Temporal point cloud point
 * @param[in] box Point cloud box
 * @param[in] border_inc True when the box contains the upper border
 * @csqlfn #Tpcpoint_minus_tpcbox()
 */
Temporal *
tpcpoint_minus_tpcbox(const Temporal *temp, const TPCBox *box, bool border_inc)
{
  return tpcpoint_restrict_tpcbox(temp, box, border_inc, REST_MINUS);
}

/*****************************************************************************/
