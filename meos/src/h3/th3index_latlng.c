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
 * @brief MEOS lifting for lat/lng conversions, plus the static adapter bodies
 * that back them
 * @details The static h3 conversions `geo_to_h3index_cell`,
 * `h3index_cell_to_point`, and `h3index_cell_to_boundary` live here
 * alongside the lifted entries that consume them. Point reads use
 * the MobilityDB peek macro `GSERIALIZED_POINT2D_P` rather than
 * `lwgeom_from_gserialized` — approved by the PostGIS team and a
 * meaningful speed-up for point-heavy paths.
 *
 * Both `tgeogpoint` (canonical, geodetic) and `tgeompoint`
 * (SRID 4326, planar-tagged) overloads are provided.
 */

/* C */
#include <math.h>
#include <string.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_cellindex.h>
#include <meos_h3.h>
#include "geo/tgeo_spatialfuncs.h"
#include "meos_internal_geo.h"
#include "temporal/temporal.h"
#include "temporal/meos_catalog.h"
#include "temporal/lifting.h"
#include "temporal/tcellindex.h"
#include "h3/h3index.h"
#include "h3/th3index_internal.h"

/*****************************************************************************
 * Static adapters — lat/lng ↔ cell / cell ↔ boundary
 *****************************************************************************/

/**
 * @ingroup meos_h3_conversion
 * @brief Single H3 cell covering a POINT geometry at the given resolution
 * @param[in] point Point geometry.
 * @param[in] resolution H3 resolution (0..15).
 * @csqlfn #Geo_point_to_h3index()
 */
H3Index
geo_to_h3index_cell(const GSERIALIZED *point, int32 resolution)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(point,  (H3Index) 0);
  if (! ensure_point_type(point) || ! ensure_not_empty(point) ||
      ! ensure_srid_is_latlong(gserialized_get_srid(point)) ||
      ! ensure_valid_cell_resolution(T_TH3INDEX, resolution))
    return (H3Index) 0;
  const POINT2D *p = GSERIALIZED_POINT2D_P(point);
  H3Index cell = h3_point_cell(p->x, p->y,
    FLAGS_GET_GEODETIC(point->gflags), resolution);
  if (cell == (H3Index) 0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "h3 library error");
    return (H3Index) 0;
  }
  return cell;
}

/**
 * @ingroup meos_h3_base_latlng
 * @brief Return the centroid of an H3 cell as a geometry point
 * @param[in] cell H3 cell
 * @csqlfn #H3index_cell_to_point()
 */
GSERIALIZED *
h3index_cell_to_point(H3Index cell)
{
  LatLng ll;
  if (cellToLatLng(cell, &ll) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "h3 library error");
    return NULL;
  }
  /* Planar (non-geodetic) lon/lat point, as the geometry it is */
  return geopoint_make(radsToDegs(ll.lng), radsToDegs(ll.lat), 0.0,
    false, false, SRID_DEFAULT);
}

/**
 * @brief Return an SRID 4326 polygon built from a libh3 CellBoundary,
 * geodetic or planar as its caller states
 * @details The ring closes by repeating vertex 0. The cell and the
 * directed-edge boundary adapters share it.
 * @param[in] bnd Boundary
 * @param[in] geodetic True when the polygon is geodetic
 */
GSERIALIZED *
cell_boundary_to_gs(const CellBoundary *bnd, bool geodetic)
{
  POINTARRAY *pa = ptarray_construct_empty(LW_FALSE, LW_FALSE,
    bnd->numVerts + 1);
  for (int v = 0; v < bnd->numVerts; v++)
  {
    POINT4D pt;
    pt.x = radsToDegs(bnd->verts[v].lng);
    pt.y = radsToDegs(bnd->verts[v].lat);
    pt.z = 0.0;
    pt.m = 0.0;
    ptarray_append_point(pa, &pt, LW_TRUE);
  }
  /* Close the ring. */
  POINT4D pt0;
  pt0.x = radsToDegs(bnd->verts[0].lng);
  pt0.y = radsToDegs(bnd->verts[0].lat);
  pt0.z = 0.0;
  pt0.m = 0.0;
  ptarray_append_point(pa, &pt0, LW_TRUE);

  LWPOLY *poly = lwpoly_construct_empty(SRID_DEFAULT, LW_FALSE, LW_FALSE);
  lwpoly_add_ring(poly, pa);
  lwgeom_set_geodetic(lwpoly_as_lwgeom(poly), geodetic);
  GSERIALIZED *result = geo_serialize(lwpoly_as_lwgeom(poly));
  lwpoly_free(poly);
  return result;
}

/**
 * @ingroup meos_h3_base_latlng
 * @brief Return the boundary of an H3 cell as a geometry polygon
 * @param[in] cell H3 cell
 * @csqlfn #H3index_cell_to_boundary()
 */
GSERIALIZED *
h3index_cell_to_boundary(H3Index cell)
{
  CellBoundary bnd;
  if (cellToBoundary(cell, &bnd) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "h3 library error");
    return NULL;
  }
  return cell_boundary_to_gs(&bnd, false);
}

/**
 * @brief Return the centroid of an H3 cell as a geodetic point
 * @param[in] cell H3 cell
 */
GSERIALIZED *
h3index_cell_to_geogpoint(H3Index cell)
{
  LatLng ll;
  if (cellToLatLng(cell, &ll) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "h3 library error");
    return NULL;
  }
  return geopoint_make(radsToDegs(ll.lng), radsToDegs(ll.lat), 0.0,
    false, true, SRID_DEFAULT);
}

/**
 * @brief Return the boundary of an H3 cell as a geodetic polygon
 * @param[in] cell H3 cell
 */
GSERIALIZED *
h3index_cell_to_geog(H3Index cell)
{
  CellBoundary bnd;
  if (cellToBoundary(cell, &bnd) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "h3 library error");
    return NULL;
  }
  return cell_boundary_to_gs(&bnd, true);
}

/*****************************************************************************
 * tgeompoint / tgeogpoint → th3index — densifying conversion
 *
 * The generic `tfunc_temporal` lift samples one cell per input instant.
 * Between two consecutive instants the trip's straight-line segment may
 * traverse cells that neither endpoint instant lies in; those visited-
 * but-not-sampled cells never appear in the result and any prefilter
 * built on the result misses them as false negatives.
 *
 * The walker below traverses, for each consecutive instant pair
 * (p_a@t_a, p_b@t_b), the cells the segment crosses one by one
 * (`h3_segment_cells`), following the great circle of a geodetic segment
 * and the straight line in longitude and latitude of a planar one, and
 * records a visit of every cell it enters. A cell holds exactly the
 * microseconds whose exact position lies in it: a planar crossing is dated
 * by the truncation of its exact parameter, and the cell holding the
 * crossing position holds that microsecond where the segment reaches the
 * boundary exactly at it; a geodetic crossing is dated by the linear
 * interpolation of t_a and t_b at the segment parameter where the segment
 * reaches that cell.
 *
 * Result is a STEP-interpolation th3index sequence, or a sequence set where
 * a crossing instant belongs to the cell left, whose cardinality is
 * data-dependent (one instant per cell entry along the trajectory). A
 * segment that stays within the cell it starts in adds no instant.
 *
 * Discrete-interpolation input is left at one-instant-per-input
 * (no segment to densify across).
 *****************************************************************************/

/**
 * @brief One-instant conversion
 * @details The Datum carries a GSERIALIZED point in SRID 4326 (either
 * tgeompoint or tgeogpoint encoding); the SRID guard lives in the static
 * adapter `geo_to_h3index_cell`.
 */
static TInstant *
tpointinst_to_th3index(const TInstant *inst, int32 resolution)
{
  const GSERIALIZED *gs = DatumGetGserializedP(tinstant_value_p(inst));
  H3Index cell = geo_to_h3index_cell(gs, resolution);
  return tinstant_make(H3IndexGetDatum(cell), T_TH3INDEX, inst->t);
}

/**
 * @brief Convert a discrete or step tgeompoint / tgeogpoint TSequence to a
 * th3index TSequence of the same interpolation, one instant per input
 * instant, since it states nothing between its instants
 */
static TSequence *
tpointseq_instants_to_th3index(const TSequence *seq, int32 resolution)
{
  TInstant **instants = palloc(sizeof(TInstant *) * (size_t) seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tpointinst_to_th3index(TSEQUENCE_INST_N(seq, i),
      resolution);
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
}

/**
 * @brief Return the 2D point of a temporal point instant, as
 * #tpointinst_point2d of tquadbin_ops.c reads it
 */
static const POINT2D *
th3index_inst_point2d(const TInstant *inst)
{
  return GSERIALIZED_POINT2D_P(DatumGetGserializedP(tinstant_value_p(inst)));
}

/**
 * @brief Append to the visits of a trajectory the cell it enters at a
 * timestamp, the previous visit ending there
 * @details As #tquadbin_visit_append appends a tile. A cell holds exactly
 * the microseconds whose exact position lies in it. The timestamp is the
 * microsecond of the crossing, truncated: where the trajectory reaches the
 * boundary exactly at it (@p whole), the crossing position belongs to the
 * cell holding it, the one left (@p held_left), the one entered
 * (@p held_entered), or at a vertex a third cell met there, which holds that
 * instant; where it reaches the boundary after it, the trajectory is still
 * in the cell it leaves at that instant, which holds it. A crossing landing
 * before the previous one, which the truncation of two crossings within one
 * microsecond gives, lands on it: the instants are never moved past one
 * another, and a cell holding no microsecond between them holds nothing
 */
static void
th3index_visit_append(DggsVisit **visits, int *count, int *size,
  H3Index cell, TimestampTz t, bool whole, bool held_left, bool held_entered)
{
  DggsVisit *prev = &(*visits)[*count - 1];
  if (t < prev->lower)
    t = prev->lower;
  prev->upper = t;
  prev->upper_inc = ! whole || held_left;
  if (*count == *size)
  {
    *size *= 2;
    *visits = repalloc(*visits, sizeof(DggsVisit) * (size_t) *size);
  }
  DggsVisit *v = &(*visits)[(*count)++];
  v->cell = H3IndexGetDatum(cell);
  v->lower = t;
  v->lower_inc = whole && held_entered;
  return;
}

/**
 * @brief Densify a linear tgeompoint / tgeogpoint TSequence to a th3index
 * sequence or sequence set holding every cell the trajectory crosses
 * @details See file-level comment for the algorithm, as
 * #tpointseq_to_tquadbin walks the tiles of a trajectory. Return NULL when
 * the first lookup fails, its error raised.
 */
static Temporal *
tpointseq_linear_to_th3index(const TSequence *seq, int32 resolution)
{
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  /* Routing the first lookup through geo_to_h3index_cell applies the lon/lat
   * SRID guard once for the whole sequence: SRID is a type-level property
   * uniform across every instant, so validating it here is sufficient */
  H3Index cell = geo_to_h3index_cell(
    DatumGetGserializedP(tinstant_value_p(inst)), resolution);
  if (cell == (H3Index) 0)
    return NULL;
  int vsize = seq->count + 1, nvisits = 1;
  DggsVisit *visits = palloc(sizeof(DggsVisit) * (size_t) vsize);
  visits[0].cell = H3IndexGetDatum(cell);
  visits[0].lower = inst->t;
  visits[0].lower_inc = seq->period.lower_inc;
  H3Index last = cell;

  /* A geodetic segment follows its great circle, a planar one its straight
   * line in longitude and latitude. The walk writes one entry per cell a
   * segment crosses, into arrays that grow until the whole segment fits, so
   * no segment is cut short */
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(seq->flags);
  int maxout = 64;
  H3Index *cells = palloc(sizeof(H3Index) * (size_t) maxout);
  double *enter = palloc(sizeof(double) * (size_t) maxout);
  DggsEdgeCross *cross = palloc(sizeof(DggsEdgeCross) * (size_t) maxout);
  for (int i = 0; i + 1 < seq->count; i++)
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(seq, i);
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
    const POINT2D *p1 = th3index_inst_point2d(inst1);
    const POINT2D *p2 = th3index_inst_point2d(inst2);
    int ncells;
    while ((ncells = h3_segment_cells(p1->x, p1->y, p2->x, p2->y, geodetic,
        resolution, cells, enter, cross, maxout)) == maxout)
    {
      maxout *= 2;
      cells = repalloc(cells, sizeof(H3Index) * (size_t) maxout);
      enter = repalloc(enter, sizeof(double) * (size_t) maxout);
      cross = repalloc(cross, sizeof(DggsEdgeCross) * (size_t) maxout);
    }
    /* A segment starts in the cell the previous one ends in */
    for (int k = 0; k < ncells; k++)
    {
      if (cells[k] == last)
        continue;
      /* A crossing is dated as the restriction of a trajectory to a region
       * dates it, by the truncation of its exact parameter, and the cell
       * holding the crossing position holds the microsecond where the
       * trajectory lies on the edge exactly then. A planar segment crosses
       * the line of the edge of the outline the walk names, dated on the
       * input doubles by #segment_edge_cross_offset; a geodetic one the
       * great circle of an edge through the crossing, dated on the geodetic
       * positions by #h3_geodetic_edge_offset */
      TimestampTz tenter = inst1->t;
      bool whole = true, held_left = false, held_entered = true;
      if (k > 0)
      {
        const DggsEdgeCross *x = &cross[k];
        int64 D = inst2->t - inst1->t;
        tenter += geodetic ?
          h3_geodetic_edge_offset(p1->x, p1->y, p2->x, p2->y, D, x->ax,
            x->ay, x->bx, x->by, enter[k], &whole) :
          segment_edge_cross_offset(p1->x, p1->y, p2->x, p2->y, D, x->ax,
            x->ay, x->oa, x->bx, x->by, x->ob, &whole);
        held_left = x->held_left;
        held_entered = x->held_entered;
      }
      th3index_visit_append(&visits, &nvisits, &vsize, cells[k], tenter,
        whole, held_left, held_entered);
      last = cells[k];
    }
    /* The endpoint's own cell, which holds it, closes the segment when the
     * traversal stopped short of it */
    H3Index endcell = h3_point_cell(p2->x, p2->y, geodetic, resolution);
    if (endcell != (H3Index) 0 && endcell != last)
    {
      th3index_visit_append(&visits, &nvisits, &vsize, endcell, inst2->t,
        true, false, true);
      last = endcell;
    }
  }
  pfree(cells); pfree(enter); pfree(cross);

  /* The last cell holds to the end of the trajectory. A crossing dated at
   * the first or the last instant of the trajectory holds that instant only
   * where the trajectory does, as the restriction of the trajectory to a
   * region answers no instant outside its period */
  TimestampTz tstart = TSEQUENCE_INST_N(seq, 0)->t;
  TimestampTz tend = TSEQUENCE_INST_N(seq, seq->count - 1)->t;
  visits[nvisits - 1].upper = tend;
  visits[nvisits - 1].upper_inc = seq->period.upper_inc;
  for (int i = 0; i < nvisits; i++)
  {
    if (visits[i].lower == tstart)
      visits[i].lower_inc &= seq->period.lower_inc;
    if (visits[i].upper == tend)
      visits[i].upper_inc &= seq->period.upper_inc;
  }
  Temporal *result = dggs_visits_to_temporal(visits, nvisits, T_TH3INDEX);
  pfree(visits);
  return result;
}

/**
 * @brief Densify a tgeompoint / tgeogpoint TSequence to a th3index sequence
 * or sequence set
 * @details See file-level comment for the algorithm. Discrete-interp input is
 * converted one-instant-per-input (no straight-line segment between instants to
 * walk).
 */
static Temporal *
tpointseq_densify_to_th3index(const TSequence *seq, int32 resolution)
{
  if (seq->count == 0)
    return NULL;
  if (MEOS_FLAGS_GET_INTERP(seq->flags) != LINEAR)
    return (Temporal *) tpointseq_instants_to_th3index(seq, resolution);
  return tpointseq_linear_to_th3index(seq, resolution);
}

/**
 * @brief Densify a tgeompoint / tgeogpoint TSequenceSet to a th3index
 * TSequenceSet by per-sequence densification
 * @details As #tpointseqset_to_tquadbin gathers its parts: each sequence
 * answers a sequence, or a sequence set where a crossing instant belongs to
 * the cell it leaves, and the answer holds all their sequences
 */
static TSequenceSet *
tpointseqset_densify_to_th3index(const TSequenceSet *ss, int32 resolution)
{
  int size = ss->count, count = 0;
  TSequence **sequences = palloc(sizeof(TSequence *) * (size_t) size);
  for (int i = 0; i < ss->count; i++)
  {
    Temporal *part = tpointseq_densify_to_th3index(
      TSEQUENCESET_SEQ_N(ss, i), resolution);
    if (part == NULL)
      continue;
    if (part->subtype == TSEQUENCE)
    {
      if (count == size)
      {
        size *= 2;
        sequences = repalloc(sequences, sizeof(TSequence *) * (size_t) size);
      }
      sequences[count++] = (TSequence *) part;
      continue;
    }
    const TSequenceSet *pss = (const TSequenceSet *) part;
    for (int j = 0; j < pss->count; j++)
    {
      if (count == size)
      {
        size *= 2;
        sequences = repalloc(sequences, sizeof(TSequence *) * (size_t) size);
      }
      sequences[count++] = tsequence_copy(TSEQUENCESET_SEQ_N(pss, j));
    }
    pfree(part);
  }
  if (count == 0)
  {
    pfree(sequences);
    return NULL;
  }
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @brief Subtype-dispatching wrapper used by both tgeompoint and tgeogpoint
 * entrypoints
 * @details Every path validates the lon/lat SRID through `geo_to_h3index_cell`
 * (the instant adapter, the non-densify branch, and the first lookup of
 * the densify walker), so non lon/lat input is rejected before any cell
 * is produced and the dispatcher itself needs no separate guard.
 */
static Temporal *
tpoint_to_th3index_dense(const Temporal *temp, int32 resolution)
{
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tpointinst_to_th3index((const TInstant *) temp,
        resolution);
    case TSEQUENCE:
      return tpointseq_densify_to_th3index((const TSequence *) temp,
        resolution);
    default: /* TSEQUENCESET */
      return (Temporal *) tpointseqset_densify_to_th3index(
        (const TSequenceSet *) temp, resolution);
  }
}

/*****************************************************************************
 * tgeompoint_to_th3index(tgeompoint, integer) — densifying conversion
 *
 * The adapter `geo_to_h3index_cell` (called from `tpointinst_to_th3index`)
 * verifies SRID 4326 and raises on mismatch.
 *****************************************************************************/

/**
 * @ingroup meos_h3_latlng
 * @brief Return the temporal H3 cell of a temporal planar point (SRID 4326) at
 * the given resolution
 * @details Segments between consecutive instants are densified so every cell
 * the trajectory traverses appears in the result.
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @csqlfn #Tgeompoint_to_th3index()
 */
Temporal *
tgeompoint_to_th3index(const Temporal *temp, int32 resolution, bool border_inc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(temp, NULL);
  return tcellindex_cover_border(tpoint_to_th3index_dense(temp, resolution),
    border_inc);
}

/*****************************************************************************
 * tgeogpoint_to_th3index(tgeogpoint, integer) — densifying conversion
 *****************************************************************************/

/**
 * @ingroup meos_h3_latlng
 * @brief Return the temporal H3 cell of a temporal geodetic point at the given
 * resolution
 * @details Segments between consecutive instants are densified so every cell
 * the trajectory traverses appears in the result.
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @csqlfn #Tgeogpoint_to_th3index()
 */
Temporal *
tgeogpoint_to_th3index(const Temporal *temp, int32 resolution, bool border_inc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOGPOINT(temp, NULL);
  return tcellindex_cover_border(tpoint_to_th3index_dense(temp, resolution),
    border_inc);
}

/*****************************************************************************
 * Split
 *****************************************************************************/

/**
 * @ingroup meos_internal_h3_latlng
 * @brief Return the fragments of a temporal point split by the H3 cells it
 * crosses at a resolution, and the cell of each
 * @details The cover states which cell the trajectory holds and when, and the
 * fragment of a cell is the trajectory over the periods the cover states for
 * it, so a fragment and the cover answer the same periods for a cell
 * @param[in] temp Temporal point
 * @param[in] resolution H3 resolution
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] cells Cell of each fragment
 * @param[out] count Number of fragments
 */
Temporal **
tpoint_h3index_split(const Temporal *temp, int32 resolution, bool border_inc,
  Datum **cells, int *count)
{
  assert(temp); assert(cells); assert(count);
  *count = 0;
  Temporal *cover = tcellindex_cover_border(
    tpoint_to_th3index_dense(temp, resolution), border_inc);
  if (! cover)
    return NULL;
  int ncells;
  SpanSet **spansets = temporal_unnest(cover, cells, &ncells);
  if (! spansets)
  {
    pfree(cover);
    return NULL;
  }
  Temporal **result = palloc(sizeof(Temporal *) * ncells);
  int nfrags = 0;
  for (int i = 0; i < ncells; i++)
  {
    Temporal *frag = temporal_restrict_tstzspanset(temp, spansets[i], REST_AT);
    /* A cell the cover holds over periods the trajectory no longer has leaves
     * nothing to state for it */
    if (frag)
    {
      (*cells)[nfrags] = (*cells)[i];
      result[nfrags++] = frag;
    }
    pfree(spansets[i]);
  }
  pfree(spansets); pfree(cover);
  if (nfrags == 0)
  {
    pfree(result);
    return NULL;
  }
  *count = nfrags;
  return result;
}

/**
 * @brief Return the H3 cells of an array of datums, freeing the array
 */
static H3Index *
h3index_cells(Datum *datums, int count)
{
  H3Index *result = palloc(sizeof(H3Index) * count);
  for (int i = 0; i < count; i++)
    result[i] = DatumGetH3Index(datums[i]);
  pfree(datums);
  return result;
}

/**
 * @ingroup meos_h3_latlng
 * @brief Return the fragments of a temporal planar point split by the H3
 * cells it crosses at a resolution, and the cell of each
 * @param[in] temp Temporal point
 * @param[in] resolution H3 resolution
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] cells Cell of each fragment
 * @param[out] count Number of fragments
 * @csqlfn #Tgeompoint_h3index_split()
 */
Temporal **
tgeompoint_h3index_split(const Temporal *temp, int32 resolution,
  bool border_inc, H3Index **cells, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(temp, NULL); VALIDATE_NOT_NULL(cells, NULL);

  Datum *datum_cells;
  Temporal **result = tpoint_h3index_split(temp, resolution, border_inc,
    &datum_cells, count);
  if (! result)
    return NULL;
  *cells = h3index_cells(datum_cells, *count);
  return result;
}

/**
 * @ingroup meos_h3_latlng
 * @brief Return the fragments of a temporal geodetic point split by the H3
 * cells it crosses at a resolution, and the cell of each
 * @param[in] temp Temporal point
 * @param[in] resolution H3 resolution
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] cells Cell of each fragment
 * @param[out] count Number of fragments
 * @csqlfn #Tgeogpoint_h3index_split()
 */
Temporal **
tgeogpoint_h3index_split(const Temporal *temp, int32 resolution,
  bool border_inc, H3Index **cells, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOGPOINT(temp, NULL); VALIDATE_NOT_NULL(cells, NULL);

  Datum *datum_cells;
  Temporal **result = tpoint_h3index_split(temp, resolution, border_inc,
    &datum_cells, count);
  if (! result)
    return NULL;
  *cells = h3index_cells(datum_cells, *count);
  return result;
}

/*****************************************************************************
 * cellToPoint (geodetic output)
 *****************************************************************************/

/**
 * @ingroup meos_h3_latlng
 * @brief Return the geodetic centroid trajectory of a temporal H3 cell
 * @csqlfn #Th3index_cell_to_tgeogpoint()
 */
Temporal *
th3index_to_tgeogpoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TH3INDEX(temp, NULL);
  return tcellindex_cell_to_point(temp);
}

/*****************************************************************************
 * cellToPoint (planar output, SRID 4326 overload)
 *
 * The planar trajectory lifts the planar centroid `h3index_cell_to_point`
 * answers, and the geodetic trajectory above lifts the geodetic centroid
 * `h3index_cell_to_geogpoint` answers through the shared cell operations.
 *****************************************************************************/

/**
 * @brief Return the planar centroid of an H3 cell
 */
static Datum
datum_h3_cell_to_geompoint(Datum d)
{
  return PointerGetDatum(h3index_cell_to_point(DatumGetH3Index(d)));
}

/**
 * @ingroup meos_h3_latlng
 * @brief Return the planar centroid trajectory (SRID 4326) of a temporal H3
 * cell
 * @csqlfn #Th3index_cell_to_tgeompoint()
 */
Temporal *
th3index_to_tgeompoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TH3INDEX(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_cell_to_geompoint;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TGEOMPOINT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * cellToBoundary — polygon per instant, emitted as tgeography
 *****************************************************************************/

/*****************************************************************************/
