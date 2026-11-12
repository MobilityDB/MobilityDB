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
 * @brief Geometric representation of the pcpatch and tpcpatch types
 * @details A patch is a cluster of points, so its geometry is the @p
 * MULTIPOINT of the
 * positions its points occupy, read through the schema its pcid names. Lifting
 * that value-level answer over time gives @ref tpcpatch_to_tgeometry, which is
 * what the type's temporal spatial relationships convert to before delegating
 * to the one geometry engine — the same route @ref tpcpoint_to_tgeompoint
 * opens for a tpcpoint, differing only in that a cluster of points is a
 * @p tgeometry where a single point is a @p tgeompoint. The region the
 * patches of a temporal patch sweep is its swath, @ref tpcpatch_swath.
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
#include <pgtypes.h>  /* minus_timestamptz_timestamptz, pg_interval_cmp */
#include "temporal/temporal.h"  /* Temporal, TInstant */
#include "temporal/meos_catalog.h"  /* T_TGEOMETRY */
#include "geo/geo_funcs.h"  /* geo_serialize */
#include "pointcloud/pcpatch.h" /* struct Pcpatch body (.pcid field) */
#include "pointcloud/pcpatch_decompose.h"  /* pcpatch_filter_per_point */
#include "temporal/type_util.h"  /* pfree_array */
#include "pointcloud/pgsql_compat.h"  /* MEOS_PC_PATCH_DESERIALIZE */
#include "pointcloud/meos_schema_hook.h"  /* meos_pc_schema */
#include "pointcloud/tpc_boxops.h"  /* tpcpatch_restrict_tpcbox */
#include "pointcloud/tpcbox.h"  /* pcpatch_srid_to_tpcbox */

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
  const PCSCHEMA *schema = meos_pc_schema(pa->pcid);
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

/**
 * @ingroup meos_pointcloud_base_conversion
 * @brief Convert a point cloud patch into a geography
 * @details The value converts through #pcpatch_to_geom and #geom_to_geog, as every spatial
 * type does
 * @param[in] pa Value
 * @errval NULL
 * @csqlfn #Pcpatch_to_geog()
 */
GSERIALIZED *
pcpatch_to_geog(const Pcpatch *pa)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pa, NULL);
  GSERIALIZED *geom = pcpatch_to_geom(pa);
  if (! geom)
    return NULL;
  GSERIALIZED *result = geom_to_geog(geom);
  pfree(geom);
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

/**
 * @ingroup meos_pointcloud_conversion
 * @brief Convert a temporal point cloud patch into a temporal geography
 * @details The value converts through #tpcpatch_to_tgeometry and #tgeometry_to_tgeography, as every spatial type
 * does
 * @param[in] temp Temporal value
 * @errval NULL
 * @csqlfn #Tpcpatch_to_tgeography()
 */
Temporal *
tpcpatch_to_tgeography(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPATCH(temp, NULL);
  Temporal *value = tpcpatch_to_tgeometry(temp);
  if (! value)
    return NULL;
  Temporal *result = tgeometry_to_tgeography(value);
  pfree(value);
  return result;
}

/*****************************************************************************
 * Swath of a temporal patch
 *****************************************************************************/

/**
 * @brief Return the footprint of a patch, the convex hull of the positions
 * its points occupy on the plane
 * @return @p NULL if the pcid schema cannot be resolved
 */
static GSERIALIZED *
pcpatch_footprint(const Pcpatch *pa)
{
  GSERIALIZED *mpoint = pcpatch_to_geom(pa);
  if (! mpoint)
    return NULL;
  GSERIALIZED *result = geom_convex_hull(mpoint);
  pfree(mpoint);
  return result;
}

/**
 * @brief Return the strip a swath covers between two consecutive footprints,
 * the convex hull of both
 */
static GSERIALIZED *
pcpatch_footprints_strip(const GSERIALIZED *foot1, const GSERIALIZED *foot2)
{
  GSERIALIZED *pair[2] = {(GSERIALIZED *) foot1, (GSERIALIZED *) foot2};
  GSERIALIZED *coll = geo_collect_garray(pair, 2);
  GSERIALIZED *result = geom_convex_hull(coll);
  pfree(coll);
  return result;
}

/**
 * @brief Return true if a swath joins the footprints of two consecutive
 * instants
 * @param[in] t1,t2 Timestamps of the instants
 * @param[in] maxt Largest gap joined @nullable
 */
static bool
tpcpatch_swath_joins(TimestampTz t1, TimestampTz t2, const Interval *maxt)
{
  if (! maxt)
    return true;
  Interval *gap = minus_timestamptz_timestamptz(t2, t1);
  bool result = pg_interval_cmp(gap, maxt) <= 0;
  pfree(gap);
  return result;
}

/**
 * @ingroup meos_pointcloud_accessor
 * @brief Return the swath of a temporal patch, the region of the plane its
 * patches sweep
 * @details The footprint of an instant is the convex hull of the positions
 * of the points of its patch, read through the schema its pcid names and
 * projected on the plane. The footprints of two consecutive instants are
 * joined by the convex hull of both, and the swath is the union of these
 * strips, together with the footprint of every instant joined to neither of
 * its neighbours. Consecutive instants are joined unless they are more than
 * @p maxt apart, so that a pause in the acquisition splits the swath. A
 * pcpatch is the set of points one acquisition captures, such as a ping of a
 * multibeam echosounder, and the swath is then the strip of the seabed the
 * pings of a survey line sound. The union is computed exactly by the areal
 * union of MEOS.
 * @param[in] temp Temporal patch
 * @param[in] maxt Largest gap between two consecutive instants that is
 * joined, every gap if @p NULL @nullable
 * @return @p NULL if the pcid schema cannot be resolved
 * @csqlfn #Tpcpatch_swath()
 */
GSERIALIZED *
tpcpatch_swath(const Temporal *temp, const Interval *maxt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPATCH(temp, NULL);
  int count;
  const TInstant **insts = temporal_insts_p(temp, &count);
  GSERIALIZED **foots = palloc(sizeof(GSERIALIZED *) * count);
  for (int i = 0; i < count; i++)
  {
    foots[i] = pcpatch_footprint(
      (const Pcpatch *) DatumGetPointer(tinstant_value_p(insts[i])));
    if (! foots[i])
    {
      for (int j = 0; j < i; j++)
        pfree(foots[j]);
      pfree(foots); pfree(insts);
      return NULL;
    }
  }

  /* A strip replaces the two footprints it joins, which it contains, so an
   * instant contributes its own footprint only when it is joined to neither
   * neighbour; there are then at most as many pieces as instants */
  GSERIALIZED **pieces = palloc(sizeof(GSERIALIZED *) * count);
  int npieces = 0;
  bool joined_prev = false;
  for (int i = 0; i < count; i++)
  {
    bool joined_next = i + 1 < count &&
      tpcpatch_swath_joins(insts[i]->t, insts[i + 1]->t, maxt);
    if (joined_next)
      pieces[npieces++] = pcpatch_footprints_strip(foots[i], foots[i + 1]);
    else if (! joined_prev)
      pieces[npieces++] = geo_copy(foots[i]);
    joined_prev = joined_next;
  }
  for (int i = 0; i < count; i++)
    pfree(foots[i]);
  pfree(foots); pfree(insts);

  GSERIALIZED *result;
  if (npieces == 1)
    result = pieces[0];
  else
  {
    GSERIALIZED *coll = geo_collect_garray(pieces, npieces);
    for (int i = 0; i < npieces; i++)
      pfree(pieces[i]);
    result = geom_unary_union(coll);
    pfree(coll);
  }
  pfree(pieces);
  return result;
}

/*****************************************************************************
 * TPCBox-based restriction (patch-level / coarse)
 *
 * Each pcpatch instant is kept-or-dropped based on whether its extent
 * overlaps the box's spatial extent and its timestamp falls in the box's
 * period. The X and Y extent is the one the PCBOUNDS header of the patch
 * states. When the box has Z, the Z extent is the one the points of the
 * patch state, read by @c pcpatch_srid_to_tpcbox, so a patch of a schema
 * without Z fails a box with Z, as each of its points fails it in
 * @c pcpoint_in_tpcbox. The patch payload is preserved verbatim.
 *
 * The @p border_inc flag toggles inclusive (touching counts) vs.
 * strict overlap; the default is inclusive.
 *****************************************************************************/

/**
 * @brief Return true if two extents along one dimension overlap
 * @param min1,max1 First extent
 * @param min2,max2 Second extent
 * @param border_inc Inclusive (@c true) or strict (@c false) overlap
 */
static bool
extent_overlaps(double min1, double max1, double min2, double max2,
  bool border_inc)
{
  if (border_inc)
    return ! (min1 > max2 || max1 < min2);
  return ! (min1 >= max2 || max1 <= min2);
}

/**
 * @brief Per-instant predicate: pcpatch's pcid, spatial extent and timestamp
 * all overlap the supplied @c TPCBox
 * @param pa          Source pcpatch.
 * @param t           Instant timestamp.
 * @param box         Filtering @c TPCBox.
 * @param border_inc  Inclusive (@c true) or strict (@c false) overlap.
 * @return @c true if the instant should be kept under at-semantics.
 */
static bool
pcpatch_passes_tpcbox(const Pcpatch *pa, TimestampTz t, const TPCBox *box,
  bool border_inc)
{
  assert(pa->pcid == box->pcid);
  if (MEOS_FLAGS_GET_T(box->flags) &&
      ! contains_span_timestamptz(&box->period, t))
    return false;
  if (! MEOS_FLAGS_GET_X(box->flags))
    return true;
  /* PCBOUNDS layout: {xmin, xmax, ymin, ymax} — see pc_api.h. */
  if (! extent_overlaps(pa->bounds[0], pa->bounds[1], box->xmin, box->xmax,
        border_inc) ||
      ! extent_overlaps(pa->bounds[2], pa->bounds[3], box->ymin, box->ymax,
        border_inc))
    return false;
  if (! MEOS_FLAGS_GET_Z(box->flags))
    return true;
  /* PCBOUNDS states no Z extent: the one of the points is the one the box
   * of the patch carries */
  TPCBox *pbox = pcpatch_srid_to_tpcbox(pa, box->srid);
  if (! pbox)
    return false;
  bool result = MEOS_FLAGS_GET_Z(pbox->flags) &&
    extent_overlaps(pbox->zmin, pbox->zmax, box->zmin, box->zmax, border_inc);
  pfree(pbox);
  return result;
}

/**
 * @brief Patch-level @c at / @c minus restriction by @c TPCBox
 * @details Walks every instant of @p temp, collects the timestamps whose patch
 * passes the extent-overlap + timestamp-in-period predicate, then defers
 * to @c temporal_restrict_tstzset for the actual restriction. Granularity is
 * patch-level — surviving instants keep their pcpatch payload verbatim.
 *
 * @param temp        Source tpcpatch.
 * @param box         Filtering @c TPCBox.
 * @param border_inc  Inclusive (@c true) or strict (@c false) overlap.
 * @param atfunc      @c REST_AT keeps passing instants; @c REST_MINUS
 *                    keeps the complement.
 * @return Newly allocated @c Temporal of the same subtype as @p temp,
 *   or @c NULL when every instant is dropped (or every instant
 *   survives in @c REST_MINUS).
 */
Temporal *
tpcpatch_restrict_tpcbox(const Temporal *temp, const TPCBox *box,
  bool border_inc, bool atfunc)
{
  /* Ensure the validity of the arguments: a box of another schema or
   * reference system raises, as the overlaps of their boxes raises */
  VALIDATE_TPCPATCH(temp, NULL);
  if (! ensure_valid_tpointcloud_tpcbox(temp, box))
    return NULL;

  /* Upper bound on the number of surviving timestamps. */
  int ninst = 0;
  switch (temp->subtype)
  {
    case TINSTANT:
      ninst = 1;
      break;
    case TSEQUENCE:
      ninst = ((const TSequence *) temp)->count;
      break;
    default: /* TSEQUENCESET */
    {
      const TSequenceSet *ss = (const TSequenceSet *) temp;
      for (int i = 0; i < ss->count; i++)
        ninst += TSEQUENCESET_SEQ_N(ss, i)->count;
    }
  }

  Datum *kept = palloc(sizeof(Datum) * ninst);
  int nkept = 0;

  if (temp->subtype == TINSTANT)
  {
    const TInstant *inst = (const TInstant *) temp;
    const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(tinstant_value_p(inst));
    if (pcpatch_passes_tpcbox(pa, inst->t, box, border_inc))
      kept[nkept++] = TimestampTzGetDatum(inst->t);
  }
  else if (temp->subtype == TSEQUENCE)
  {
    const TSequence *seq = (const TSequence *) temp;
    for (int i = 0; i < seq->count; i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(tinstant_value_p(inst));
      if (pcpatch_passes_tpcbox(pa, inst->t, box, border_inc))
        kept[nkept++] = TimestampTzGetDatum(inst->t);
    }
  }
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    for (int i = 0; i < ss->count; i++)
    {
      const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
      for (int j = 0; j < seq->count; j++)
      {
        const TInstant *inst = TSEQUENCE_INST_N(seq, j);
        const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(tinstant_value_p(inst));
        if (pcpatch_passes_tpcbox(pa, inst->t, box, border_inc))
          kept[nkept++] = TimestampTzGetDatum(inst->t);
      }
    }
  }

  /* Edge cases: no survivors / all survivors. */
  if (nkept == 0)
  {
    pfree(kept);
    return atfunc ? NULL : temporal_copy(temp);
  }
  if (nkept == ninst)
  {
    pfree(kept);
    return atfunc ? temporal_copy(temp) : NULL;
  }

  Set *tset = set_make(kept, nkept, T_TIMESTAMPTZ, ORDER);
  pfree(kept);
  Temporal *result = temporal_restrict_tstzset(temp, tset, atfunc);
  pfree(tset);
  return result;
}

/**
 * @ingroup meos_pointcloud_restrict
 * @brief Return a temporal point cloud patch restricted to a point cloud box
 * @param[in] temp Temporal point cloud patch
 * @param[in] box Point cloud box
 * @param[in] border_inc True when the box contains the upper border
 * @csqlfn #Tpcpatch_at_tpcbox()
 */
Temporal *
tpcpatch_at_tpcbox(const Temporal *temp, const TPCBox *box, bool border_inc)
{
  return tpcpatch_restrict_tpcbox(temp, box, border_inc, REST_AT);
}

/**
 * @ingroup meos_pointcloud_restrict
 * @brief Return a temporal point cloud patch restricted to the complement of a point cloud box
 * @param[in] temp Temporal point cloud patch
 * @param[in] box Point cloud box
 * @param[in] border_inc True when the box contains the upper border
 * @csqlfn #Tpcpatch_minus_tpcbox()
 */
Temporal *
tpcpatch_minus_tpcbox(const Temporal *temp, const TPCBox *box, bool border_inc)
{
  return tpcpatch_restrict_tpcbox(temp, box, border_inc, REST_MINUS);
}

/*****************************************************************************
 * Point-level restriction
 *
 * Each point of each patch is kept or dropped by a predicate, a patch keeps
 * the points that survive, and an instant whose patch keeps none is dropped.
 * A period, when given, holds the instants the predicate reads: an instant
 * outside it has no point the predicate selects.
 *****************************************************************************/

/**
 * @brief Return an instant of a temporal point cloud patch keeping the points
 * a predicate selects, or NULL when it keeps none
 * @param[in] inst Instant
 * @param[in] period Period of the instants the predicate reads, NULL for all
 * @param[in] pred Predicate on a point, NULL when it selects every point
 * @param[in] extra Argument of the predicate
 * @param[in] atfunc True when the points the predicate selects are kept,
 * false when the other ones are
 */
static TInstant *
tpcpatchinst_restrict_points(const TInstant *inst, const Span *period,
  pcpatch_pointpred_fn pred, void *extra, bool atfunc)
{
  bool inperiod = ! period || contains_span_timestamptz(period, inst->t);
  /* The points of the instant are all selected or none is */
  if (! inperiod || ! pred)
    return (inperiod == atfunc) ? tinstant_copy(inst) : NULL;
  const Pcpatch *pa = (const Pcpatch *) DatumGetPointer(tinstant_value_p(inst));
  Pcpatch *filtered = pcpatch_filter_per_point(pa, pred, extra, atfunc);
  if (! filtered)
    return NULL;
  TInstant *result = tinstant_make(PointerGetDatum(filtered), T_TPCPATCH,
    inst->t);
  pfree(filtered);
  return result;
}

/**
 * @brief Restrict the instants of a sequence of a temporal point cloud patch
 * to the points a predicate selects, adding a sequence to an array for each
 * run of instants keeping a point
 * @param[in] seq Sequence
 * @param[in] period Period of the instants the predicate reads, NULL for all
 * @param[in] pred Predicate on a point, NULL when it selects every point
 * @param[in] extra Argument of the predicate
 * @param[in] atfunc True when the points the predicate selects are kept,
 * false when the other ones are
 * @param[out] result Array of sequences, with room for one per instant
 * @return Number of sequences added to the array
 */
static int
tpcpatchseq_restrict_points(const TSequence *seq, const Span *period,
  pcpatch_pointpred_fn pred, void *extra, bool atfunc, TSequence **result)
{
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  TInstant **run = palloc(sizeof(TInstant *) * seq->count);
  int nrun = 0, nseqs = 0;
  bool lower_inc = seq->period.lower_inc;
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = tpcpatchinst_restrict_points(TSEQUENCE_INST_N(seq, i),
      period, pred, extra, atfunc);
    if (inst)
      run[nrun++] = inst;
    else if (nrun > 0)
    {
      result[nseqs++] = tsequence_make(run, nrun,
        lower_inc, true, interp, NORMALIZE);
      pfree_array((void **) run, nrun);
      run = palloc(sizeof(TInstant *) * seq->count);
      nrun = 0;
      lower_inc = true;
    }
  }
  if (nrun > 0)
  {
    result[nseqs++] = tsequence_make(run, nrun,
      lower_inc, seq->period.upper_inc, interp, NORMALIZE);
    pfree_array((void **) run, nrun);
  }
  else
    pfree(run);
  return nseqs;
}

/**
 * @brief Return a temporal point cloud patch restricted to the points a
 * predicate selects, or to the other ones
 * @details Shared by the restrictions to a point cloud box and to a geometry
 * that read each point of a patch
 * @param[in] temp Temporal point cloud patch
 * @param[in] period Period of the instants the predicate reads, NULL for all
 * @param[in] pred Predicate on a point, NULL when it selects every point
 * @param[in] extra Argument of the predicate
 * @param[in] atfunc True when the points the predicate selects are kept,
 * false when the other ones are
 * @return NULL when no instant keeps a point
 */
Temporal *
tpcpatch_restrict_points(const Temporal *temp, const Span *period,
  pcpatch_pointpred_fn pred, void *extra, bool atfunc)
{
  assert(temp); assert(temp->temptype == T_TPCPATCH);
  if (temp->subtype == TINSTANT)
    return (Temporal *) tpcpatchinst_restrict_points((const TInstant *) temp,
      period, pred, extra, atfunc);

  int nseqs;
  TSequence **seqs;
  if (temp->subtype == TSEQUENCE)
  {
    const TSequence *seq = (const TSequence *) temp;
    seqs = palloc(sizeof(TSequence *) * seq->count);
    nseqs = tpcpatchseq_restrict_points(seq, period, pred, extra, atfunc,
      seqs);
    if (nseqs == 1)
    {
      Temporal *result = (Temporal *) seqs[0];
      pfree(seqs);
      return result;
    }
  }
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    /* At most one sequence for each instant */
    int count = 0;
    for (int i = 0; i < ss->count; i++)
      count += TSEQUENCESET_SEQ_N(ss, i)->count;
    seqs = palloc(sizeof(TSequence *) * count);
    nseqs = 0;
    for (int i = 0; i < ss->count; i++)
      nseqs += tpcpatchseq_restrict_points(TSEQUENCESET_SEQ_N(ss, i), period,
        pred, extra, atfunc, &seqs[nseqs]);
  }
  if (nseqs == 0)
  {
    pfree(seqs);
    return NULL;
  }
  return (Temporal *) tsequenceset_make_free(seqs, nseqs, NORMALIZE);
}

/**
 * @brief Return a temporal point cloud patch restricted to the points in a
 * point cloud box or to the other ones
 * @details An instant outside the period of the box has no point in it, and
 * when the box has no X dimension every point of an instant in its period is
 * in it
 * @param[in] temp Temporal point cloud patch
 * @param[in] box Point cloud box
 * @param[in] border_inc True when the box contains the upper border
 * @param[in] atfunc True if the restriction is at, false for minus
 */
Temporal *
tpcpatch_restrict_tpcbox_fine(const Temporal *temp, const TPCBox *box,
  bool border_inc, bool atfunc)
{
  /* Ensure the validity of the arguments: a box of another schema or
   * reference system raises, as the overlaps of their boxes raises */
  VALIDATE_TPCPATCH(temp, NULL);
  if (! ensure_valid_tpointcloud_tpcbox(temp, box))
    return NULL;
  PcpointInTpcboxArgs args = { .box = box, .border_inc = border_inc };
  return tpcpatch_restrict_points(temp,
    MEOS_FLAGS_GET_T(box->flags) ? &box->period : NULL,
    MEOS_FLAGS_GET_X(box->flags) ? pcpoint_in_tpcbox : NULL, &args, atfunc);
}

/**
 * @ingroup meos_pointcloud_restrict
 * @brief Return a temporal point cloud patch restricted to the points in a
 * point cloud box
 * @param[in] temp Temporal point cloud patch
 * @param[in] box Point cloud box
 * @param[in] border_inc True when the box contains the upper border
 * @csqlfn #Tpcpatch_at_tpcbox_fine()
 */
Temporal *
tpcpatch_at_tpcbox_fine(const Temporal *temp, const TPCBox *box,
  bool border_inc)
{
  return tpcpatch_restrict_tpcbox_fine(temp, box, border_inc, REST_AT);
}

/**
 * @ingroup meos_pointcloud_restrict
 * @brief Return a temporal point cloud patch restricted to the points outside
 * a point cloud box
 * @param[in] temp Temporal point cloud patch
 * @param[in] box Point cloud box
 * @param[in] border_inc True when the box contains the upper border
 * @csqlfn #Tpcpatch_minus_tpcbox_fine()
 */
Temporal *
tpcpatch_minus_tpcbox_fine(const Temporal *temp, const TPCBox *box,
  bool border_inc)
{
  return tpcpatch_restrict_tpcbox_fine(temp, box, border_inc, REST_MINUS);
}

/*****************************************************************************/
