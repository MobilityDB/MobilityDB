/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * The static h3 conversions `h3_gs_point_to_cell`,
 * `h3_cell_to_gs_point`, and `h3_cell_to_gs_boundary` live here
 * alongside the lifted entries that consume them. Point reads use
 * the MobilityDB peek macro `GSERIALIZED_POINT2D_P` rather than
 * `lwgeom_from_gserialized` — approved by the PostGIS team and a
 * meaningful speed-up for point-heavy paths.
 *
 * Both `tgeogpoint` (canonical, geodetic) and `tgeompoint`
 * (SRID 4326, planar-tagged) overloads are provided.
 */

#include <math.h>
#include <string.h>

#include <liblwgeom.h>

#include <meos.h>
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

H3Index
h3_gs_point_to_cell(const GSERIALIZED *point, int32 resolution)
{
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

GSERIALIZED *
h3_cell_to_gs_point(H3Index cell)
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

GSERIALIZED *
h3_cell_to_gs_boundary(H3Index cell)
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
 * guard lives in the static adapter `h3_gs_point_to_cell`.
 */
static TInstant *
tpointinst_to_th3index(const TInstant *inst, int32 resolution)
{
  const GSERIALIZED *gs = DatumGetGserializedP(tinstant_value(inst));
  H3Index cell = h3_gs_point_to_cell(gs, resolution);
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
      const GSERIALIZED *gs = DatumGetGserializedP(tinstant_value(inst));
      H3Index cell = h3_gs_point_to_cell(gs, resolution);
      PUSH_INSTANT(cell, inst->t);
    }
  }
  else
  {
    double step_deg = h3_sample_step_deg(resolution);
    if (step_deg <= 0.0)
      step_deg = 1e-5;   /* defensive */

    /* Emit the first instant's cell. Routing the first lookup through
     * h3_gs_point_to_cell applies the lon/lat SRID guard once for the
     * whole sequence: SRID is a type-level property uniform across every
     * instant, so validating it here is sufficient and the interpolated
     * per-sample lookups below can use the cheaper raw-coordinate path. */
    {
      const TInstant *inst0 = TSEQUENCE_INST_N(seq, 0);
      const GSERIALIZED *gs0 = DatumGetGserializedP(tinstant_value(inst0));
      H3Index cell0 = h3_gs_point_to_cell(gs0, resolution);
      PUSH_INSTANT(cell0, inst0->t);
      last_cell = cell0;
      have_last = true;
    }

    /* For each segment, walk in Nyquist steps and emit cell-entry
     * instants. */
    for (int i = 0; i + 1 < seq->count; i++)
    {
      const TInstant *inst_a = TSEQUENCE_INST_N(seq, i);
      const TInstant *inst_b = TSEQUENCE_INST_N(seq, i + 1);
      const POINT2D *pa = GSERIALIZED_POINT2D_P(
        DatumGetGserializedP(tinstant_value(inst_a)));
      const POINT2D *pb = GSERIALIZED_POINT2D_P(
        DatumGetGserializedP(tinstant_value(inst_b)));
      double dx = pb->x - pa->x;
      double dy = pb->y - pa->y;
      double seg_deg = sqrt(dx * dx + dy * dy);
      int nsamples = (int) ceil(seg_deg / step_deg);
      if (nsamples < 1)
        nsamples = 1;

      /* Skip s=0 (already emitted as endpoint of the previous segment
       * or as the very first instant); walk s = 1..nsamples inclusive
       * so the last sample lands exactly on inst_b. Emit only when the
       * cell changes. */
      for (int s = 1; s <= nsamples; s++)
      {
        double t = (double) s / (double) nsamples;
        double lat = pa->y + t * dy;
        double lng = pa->x + t * dx;
        H3Index cell = h3_latlng_deg_to_cell(lat, lng, resolution);
        if (cell == (H3Index) 0)
          continue;
        if (have_last && cell == last_cell && s < nsamples)
          continue;
        TimestampTz ts;
        if (s == nsamples)
          ts = inst_b->t;   /* land on the segment endpoint exactly */
        else
          ts = inst_a->t
             + (TimestampTz) ((double) (inst_b->t - inst_a->t) * t);
        if (have_last && cell == last_cell)
          continue;  /* same cell at endpoint: drop duplicate, will be
                      * re-emitted as the first sample of the next
                      * segment if different from then-current. */
        PUSH_INSTANT(cell, ts);
        last_cell = cell;
        have_last = true;
      }
    }
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
 *
 * Every path validates the lon/lat SRID through `h3_gs_point_to_cell`
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
 * h3_latlng_to_cell(tgeompoint, integer) — densifying conversion
 *
 * The adapter `h3_gs_point_to_cell` (called from `tpointinst_to_th3index`)
 * verifies SRID 4326 and raises on mismatch.
 *****************************************************************************/

/**
 * @ingroup meos_h3_latlng
 * @brief Return the temporal H3 cell of a temporal planar point (SRID 4326)
 * at the given resolution; segments between consecutive instants are
 * densified so every cell the trajectory traverses appears in the result.
 */
Temporal *
tgeompoint_to_th3index(const Temporal *temp, int32 resolution)
{
  assert(temp); assert(temp->temptype == T_TGEOMPOINT);
  return tpoint_to_th3index_dense(temp, resolution);
}

/*****************************************************************************
 * h3_latlng_to_cell(tgeogpoint, integer) — densifying conversion
 *****************************************************************************/

/**
 * @ingroup meos_h3_latlng
 * @brief Return the temporal H3 cell of a temporal geodetic point at the
 * given resolution; segments between consecutive instants are densified
 * so every cell the trajectory traverses appears in the result.
 */
Temporal *
tgeogpoint_to_th3index(const Temporal *temp, int32 resolution)
{
  assert(temp); assert(temp->temptype == T_TGEOGPOINT);
  return tpoint_to_th3index_dense(temp, resolution);
}

/*****************************************************************************
 * h3_cell_to_latlng (geodetic output)
 *****************************************************************************/

/**
 * @ingroup meos_h3_latlng
 * @brief Return the geodetic centroid trajectory of a temporal H3 cell.
 */
Temporal *
th3index_to_tgeogpoint(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_cell_to_latlng;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TGEOGPOINT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_cell_to_latlng (planar output, SRID 4326 overload)
 *
 * Both overloads share the same static adapter `h3_cell_to_gs_point`,
 * which emits an SRID-4326 point. The geography-vs-geometry nature
 * of the result is disambiguated at the lifting layer via the
 * `restype` setting — downstream consumers see the intended type.
 *****************************************************************************/

/**
 * @ingroup meos_h3_latlng
 * @brief Return the planar centroid trajectory (SRID 4326) of a temporal
 * H3 cell.
 */
Temporal *
th3index_to_tgeompoint(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

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
 * h3_cell_to_boundary — polygon per instant, emitted as tgeography
 *****************************************************************************/

/**
 * @ingroup meos_h3_latlng
 * @brief Return the per-instant polygon boundary of a temporal H3 cell as
 * a temporal geography.
 */
Temporal *
th3index_cell_to_boundary(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_cell_to_boundary;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TGEOGRAPHY;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************/
