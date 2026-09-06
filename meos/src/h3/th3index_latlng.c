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
 * @brief MEOS lifting for lat/lng conversions, plus the static
 * adapter bodies that back them.
 *
 * The static h3 conversions `geo_to_h3index_cell`,
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
  if (! ensure_srid_is_latlong(gserialized_get_srid(point)))
    return (H3Index) 0;
  const POINT2D *p = GSERIALIZED_POINT2D_P(point);
  LatLng ll = { .lat = degsToRads(p->y), .lng = degsToRads(p->x) };
  H3Index cell;
  if (latLngToCell(&ll, resolution, &cell) != E_SUCCESS)
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
  return geopoint_make(radsToDegs(ll.lng), radsToDegs(ll.lat), 0.0,
    false, true, SRID_DEFAULT);
}

/**
 * @brief Build a geodetic SRID 4326 LWPOLY from a libh3 CellBoundary
 * and serialise it. The ring is closed by repeating vertex 0. Shared
 * between cell and directed-edge boundary adapters.
 */
GSERIALIZED *
cell_boundary_to_gs(const CellBoundary *bnd)
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
  lwgeom_set_geodetic(lwpoly_as_lwgeom(poly), LW_TRUE);
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
  return cell_boundary_to_gs(&bnd);
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
 * The walker below mirrors the static-side `linestring_to_cells_into`:
 * for each consecutive instant pair (p_a@t_a, p_b@t_b) it walks the
 * segment in lat/lon at Nyquist step (cell-edge / 2) and emits a new
 * th3index instant every time the sampled cell changes. The timestamp
 * of each emitted instant is the linear interpolation of t_a and t_b
 * at the segment parameter where the cell changed.
 *
 * Result is a STEP-interpolation th3index TSequence whose cardinality
 * is data-dependent (one instant per cell entry along the trajectory).
 * For very short / static segments the walker degenerates to one
 * instant per input instant — i.e. the previous per-instant behaviour.
 *
 * Discrete-interpolation input is left at one-instant-per-input
 * (no segment to densify across).
 *****************************************************************************/

/**
 * @brief One-instant conversion. The Datum carries a GSERIALIZED point
 * in SRID 4326 (either tgeompoint or tgeogpoint encoding); the SRID
 * guard lives in the static adapter `geo_to_h3index_cell`.
 */
static TInstant *
tpointinst_to_th3index(const TInstant *inst, int32 resolution)
{
  const GSERIALIZED *gs = DatumGetGserializedP(tinstant_value_p(inst));
  H3Index cell = geo_to_h3index_cell(gs, resolution);
  return tinstant_make(H3IndexGetDatum(cell), T_TH3INDEX, inst->t);
}

/**
 * @brief Densify a tgeompoint / tgeogpoint TSequence to a th3index
 * STEP TSequence. See file-level comment for the algorithm.
 *
 * Discrete-interp input is converted one-instant-per-input (no
 * straight-line segment between instants to walk).
 */
static TSequence *
tpointseq_densify_to_th3index(const TSequence *seq, int32 resolution)
{
  if (seq->count == 0)
    return NULL;

  interpType in_interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  bool densify = (in_interp == LINEAR);

  /* Initial capacity: at least the input cardinality. Grow as needed. */
  int maxcount = seq->count;
  TInstant **instants = palloc(sizeof(TInstant *) * (size_t) maxcount);
  int ninsts = 0;
  H3Index last_cell = (H3Index) 0;
  bool have_last = false;

  /* Helper: push (cell, ts) into the result, growing if needed. */
  #define PUSH_INSTANT(_cell, _ts)                                    \
    do {                                                              \
      if (ninsts >= maxcount)                                         \
      {                                                               \
        maxcount = (maxcount * 2 > ninsts + 1)                        \
                   ? maxcount * 2 : ninsts + 1;                       \
        instants = repalloc(instants, sizeof(TInstant *)              \
                                      * (size_t) maxcount);           \
      }                                                               \
      instants[ninsts++] = tinstant_make(H3IndexGetDatum(_cell),      \
                                         T_TH3INDEX, (_ts));          \
    } while (0)

  if (! densify)
  {
    for (int i = 0; i < seq->count; i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      const GSERIALIZED *gs = DatumGetGserializedP(tinstant_value_p(inst));
      H3Index cell = geo_to_h3index_cell(gs, resolution);
      PUSH_INSTANT(cell, inst->t);
    }
  }
  else
  {
    /* Emit the first instant's cell. Routing the first lookup through
     * geo_to_h3index_cell applies the lon/lat SRID guard once for the
     * whole sequence: SRID is a type-level property uniform across every
     * instant, so validating it here is sufficient and the interpolated
     * per-sample lookups below can use the cheaper raw-coordinate path. */
    {
      const TInstant *inst0 = TSEQUENCE_INST_N(seq, 0);
      const GSERIALIZED *gs0 = DatumGetGserializedP(tinstant_value_p(inst0));
      H3Index cell0 = geo_to_h3index_cell(gs0, resolution);
      PUSH_INSTANT(cell0, inst0->t);
      last_cell = cell0;
      have_last = true;
    }

    /* The traversal writes one entry per cell a segment crosses, so the
     * longest segment sizes the buffer for every one of them: a segment
     * spans at most its own length in cell widths, and a cell is never
     * narrower than its own edge */
    double edge_m;
    if (getHexagonEdgeLengthAvgM(resolution, &edge_m) != E_SUCCESS)
      edge_m = 1000.0;
    double half_edge_deg = (edge_m / 111320.0) / 2.0;
    double longest = 0.0;
    for (int i = 0; i + 1 < seq->count; i++)
    {
      const POINT2D *qa = GSERIALIZED_POINT2D_P(DatumGetGserializedP(
        tinstant_value_p(TSEQUENCE_INST_N(seq, i))));
      const POINT2D *qb = GSERIALIZED_POINT2D_P(DatumGetGserializedP(
        tinstant_value_p(TSEQUENCE_INST_N(seq, i + 1))));
      double ddx = qb->x - qa->x, ddy = qb->y - qa->y;
      double d = sqrt(ddx * ddx + ddy * ddy);
      if (d > longest)
        longest = d;
    }
    int xcap = (half_edge_deg > 0.0)
      ? (int) (longest / half_edge_deg) + 8 : 8;
    H3Index *xcells = palloc(sizeof(H3Index) * (size_t) xcap);
    double *xenter = palloc(sizeof(double) * (size_t) xcap);

    /* For each segment, traverse the cells it crosses and emit a
     * cell-entry instant for each. */
    for (int i = 0; i + 1 < seq->count; i++)
    {
      const TInstant *inst_a = TSEQUENCE_INST_N(seq, i);
      const TInstant *inst_b = TSEQUENCE_INST_N(seq, i + 1);
      const POINT2D *pa = GSERIALIZED_POINT2D_P(
        DatumGetGserializedP(tinstant_value_p(inst_a)));
      const POINT2D *pb = GSERIALIZED_POINT2D_P(
        DatumGetGserializedP(tinstant_value_p(inst_b)));
      /* The segment is TRAVERSED cell by cell, so the result holds every
       * cell it crosses; `enter[k]` is the parameter at which it reaches
       * cells[k], which the timestamp is interpolated from. The first
       * entry repeats the cell the previous segment ended in and is
       * dropped by the same-cell test below. */
      int nx = h3_segment_cells(pa->x, pa->y, pb->x, pb->y, resolution,
        xcells, xenter, xcap);
      for (int k = 0; k < nx; k++)
      {
        H3Index cell = xcells[k];
        if (have_last && cell == last_cell)
          continue;
        TimestampTz ts = (k == 0) ? inst_a->t
          : inst_a->t + (TimestampTz) ((double) (inst_b->t - inst_a->t)
              * xenter[k]);
        PUSH_INSTANT(cell, ts);
        last_cell = cell;
        have_last = true;
      }
      /* The endpoint's own cell closes the segment when the traversal
       * stopped inside it */
      H3Index endcell = h3_latlng_deg_to_cell(pb->y, pb->x, resolution);
      if (endcell != (H3Index) 0 && ! (have_last && endcell == last_cell))
      {
        PUSH_INSTANT(endcell, inst_b->t);
        last_cell = endcell;
        have_last = true;
      }
    }
    pfree(xcells); pfree(xenter);
  }

  #undef PUSH_INSTANT

  /* STEP interpolation: each instant marks the time the trajectory
   * entered that cell. lower_inc / upper_inc inherited from the input. */
  return tsequence_make_free(instants, ninsts, seq->period.lower_inc,
    seq->period.upper_inc, STEP, NORMALIZE);
}

/**
 * @brief Densify a tgeompoint / tgeogpoint TSequenceSet to a th3index
 * STEP TSequenceSet by per-sequence densification.
 */
static TSequenceSet *
tpointseqset_densify_to_th3index(const TSequenceSet *ss, int32 resolution)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * (size_t) ss->count);
  int nseq = 0;
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *out = tpointseq_densify_to_th3index(
      TSEQUENCESET_SEQ_N(ss, i), resolution);
    if (out != NULL)
      sequences[nseq++] = out;
  }
  return tsequenceset_make_free(sequences, nseq, NORMALIZE);
}

/**
 * @brief Subtype-dispatching wrapper used by both tgeompoint and
 * tgeogpoint entrypoints.
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
      return (Temporal *) tpointseq_densify_to_th3index(
        (const TSequence *) temp, resolution);
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
 * @brief Return the temporal H3 cell of a temporal planar point (SRID 4326)
 * at the given resolution; segments between consecutive instants are
 * densified so every cell the trajectory traverses appears in the result.
 * @csqlfn #Tgeompoint_to_th3index()
 */
Temporal *
tgeompoint_to_th3index(const Temporal *temp, int32 resolution)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(temp, NULL);
  return tpoint_to_th3index_dense(temp, resolution);
}

/*****************************************************************************
 * tgeogpoint_to_th3index(tgeogpoint, integer) — densifying conversion
 *****************************************************************************/

/**
 * @ingroup meos_h3_latlng
 * @brief Return the temporal H3 cell of a temporal geodetic point at the
 * given resolution; segments between consecutive instants are densified
 * so every cell the trajectory traverses appears in the result.
 * @csqlfn #Tgeogpoint_to_th3index()
 */
Temporal *
tgeogpoint_to_th3index(const Temporal *temp, int32 resolution)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOGPOINT(temp, NULL);
  return tpoint_to_th3index_dense(temp, resolution);
}

/*****************************************************************************
 * cellToPoint (geodetic output)
 *****************************************************************************/

/**
 * @ingroup meos_h3_latlng
 * @brief Return the geodetic centroid trajectory of a temporal H3 cell.
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
 * Both overloads share the same static adapter `h3index_cell_to_point`,
 * which emits an SRID-4326 point. The geography-vs-geometry nature
 * of the result is disambiguated at the lifting layer via the
 * `restype` setting — downstream consumers see the intended type.
 *****************************************************************************/

/**
 * @ingroup meos_h3_latlng
 * @brief Return the planar centroid trajectory (SRID 4326) of a temporal
 * H3 cell.
 * @csqlfn #Th3index_cell_to_tgeompoint()
 */
Temporal *
th3index_to_tgeompoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TH3INDEX(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_cell_to_latlng;
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
