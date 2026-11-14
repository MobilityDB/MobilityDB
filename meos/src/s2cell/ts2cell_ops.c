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
 * @brief Datum wrappers and cell operations of the S2 index
 * @details Datum-convention wrappers for the S2 static kernel and the
 * `s2_cellops` descriptor that plugs S2 into the shared temporal cell-index
 * machinery (meos/src/temporal/tcellindex.c).
 *
 * An S2 cell is a uint64 carried in a Datum with the int8/bigint convention
 * (Int64GetDatum / DatumGetInt64), holding the face, the position along the
 * Hilbert curve and the level of a cell of the sphere decomposed through a
 * circumscribed cube (https://s2geometry.io/). The cell is defined on the
 * sphere, so its centre and its boundary are geodetic (SRID 4326), and its
 * four edges are geodesics rather than the straight segments a planar grid
 * carries.
 */

#include "s2cell/s2cell.h"

/* C */
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include <meos_s2cell.h>
#include <pgtypes.h>
#include "geo/geo_funcs.h"  /* segment_edge_cross_offset */
#include "temporal/meos_catalog.h"
#include "temporal/tcellindex.h"
#include "temporal/temporal.h"
#include "temporal/lifting.h"
#include "geo/tgeo_spatialfuncs.h"

/*****************************************************************************
 * Datum-convention static-cell wrappers
 *****************************************************************************/

/**
 * @brief Return the level of an S2 cell
 */
static Datum
datum_s2_get_resolution(Datum d)
{
  return Int32GetDatum((int32) s2cell_get_resolution(DatumGetS2Cell(d)));
}

/**
 * @brief Return true if a value encodes a valid S2 cell
 */
static Datum
datum_s2_is_valid_cell(Datum d)
{
  return BoolGetDatum(s2cell_is_valid_cell(DatumGetS2Cell(d)));
}

/**
 * @brief Return the ancestor of an S2 cell at a level
 */
static Datum
datum_s2_cell_to_parent(Datum cell_d, Datum res_d)
{
  S2CellId parent = s2cell_cell_to_parent(DatumGetS2Cell(cell_d),
    (uint32_t) DatumGetInt32(res_d));
  return S2CellGetDatum(parent);
}

/**
 * @brief Return the geodetic centre of an S2 cell
 */
static Datum
datum_s2_cell_to_point(Datum d)
{
  return PointerGetDatum(s2cell_cell_to_geogpoint(DatumGetS2Cell(d)));
}

/**
 * @brief Return the geodetic boundary of an S2 cell
 * @details The four vertices are the corners of the cell on the sphere, and
 * the ring closes on the first of them.
 */
static Datum
datum_s2_cell_to_boundary(Datum d)
{
  return PointerGetDatum(s2cell_cell_to_geog(DatumGetS2Cell(d)));
}

/**
 * @brief Return the area in square metres of an S2 cell
 */
static Datum
datum_s2_cell_area(Datum d)
{
  return Float8GetDatum(s2cell_cell_area(DatumGetS2Cell(d)));
}

/**
 * @brief Return the canonical token of an S2 cell
 */
static Datum
datum_s2_cell_to_token(Datum d)
{
  size_t size_out;
  char *str = s2cell_cell_to_token(DatumGetS2Cell(d), &size_out);
  text *result = cstring_to_text(str);
  pfree(str);
  return PointerGetDatum(result);
}

/*****************************************************************************
 * Descriptor
 *****************************************************************************/

/**
 * @brief S2 operations descriptor consumed by `dggs_cellops()`
 */
const DggsCellOps s2_cellops =
{
  .celltype        = T_S2CELL,
  .settype         = T_S2CELLSET,
  .temptype        = T_TS2CELL,
  .min_resolution  = S2_MIN_LEVEL,
  .max_resolution  = S2_MAX_LEVEL,
  .point_temptype  = T_TGEOGPOINT,
  .point_srid      = SRID_DEFAULT,
  .get_resolution  = &datum_s2_get_resolution,
  .is_valid_cell   = &datum_s2_is_valid_cell,
  .cell_to_parent  = &datum_s2_cell_to_parent,
  .cell_to_point   = &datum_s2_cell_to_point,
  .cell_to_boundary = &datum_s2_cell_to_boundary,
  .cell_area       = &datum_s2_cell_area
};

/*****************************************************************************
 * S2-unique temporal op: cell -> token (ttext)
 *
 * The token has no H3 or QUADBIN analogue, so it is a typed ts2cell function
 * rather than a generic DggsCellOps entry: the descriptor exposes only the
 * operations shared by every DGGS.
 *****************************************************************************/

/**
 * @ingroup meos_s2cell
 * @brief Return the canonical token of each cell in a temporal S2 value
 * @param[in] temp Temporal value
 * @csqlfn #Ts2cell_cell_to_token()
 */
Temporal *
ts2cell_cell_to_token(const Temporal *temp)
{
  VALIDATE_TS2CELL(temp, NULL);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_s2_cell_to_token;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.restype = T_TTEXT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * Grid distance
 *****************************************************************************/

/**
 * @brief Return the grid distance of two S2 cells
 */
static Datum
datum_s2_grid_distance(Datum origin_d, Datum dest_d)
{
  return Int64GetDatum(s2cell_grid_distance(DatumGetS2Cell(origin_d),
    DatumGetS2Cell(dest_d)));
}

/**
 * @ingroup meos_s2cell
 * @brief Return the temporal grid distance of two temporal S2 cells, the
 * number of steps between edge neighbours from one to the other at each
 * instant
 * @details As #th3index_grid_distance is for H3 cells, the procedure of the
 * `<->` operator on `ts2cell`
 * @param[in] origin,dest Temporal values
 * @csqlfn #Ts2cell_grid_distance()
 */
Temporal *
ts2cell_grid_distance(const Temporal *origin, const Temporal *dest)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TS2CELL(origin, NULL); VALIDATE_TS2CELL(dest, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_s2_grid_distance;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TS2CELL;
  lfinfo.argtype[1] = T_TS2CELL;
  lfinfo.restype = T_TBIGINT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal_temporal(origin, dest, &lfinfo);
}

/*****************************************************************************
 * Conversion from a temporal point
 *****************************************************************************/

/**
 * @brief Append the instant of a cell entered at a timestamp
 * @details A cell's entry time is interpolated from the parameter at which the
 * path reaches it, while a timestamp holds whole microseconds, so two
 * crossings closer together than one microsecond round to the same instant.
 * A microsecond is the smallest separation the type can state, so a cell left
 * again before it holds any time is not part of the result: the cell entered
 * at that instant replaces it, as a crossing within #MEOS_EPSILON of the end
 * of a segment is no crossing for #tgeogpointsegm_distance_turnpt()
 */
static void
ts2cell_entry_append(TInstant ***instants, int *count, int *size,
  S2CellId cell, TimestampTz t)
{
  if (*count > 0 && t <= (*instants)[*count - 1]->t)
  {
    /* The cell already stated at this instant is left for this one before it
     * holds any time, so this one is the cell the trajectory holds there */
    TimestampTz tlast = (*instants)[*count - 1]->t;
    pfree((*instants)[--(*count)]);
    t = tlast;
  }
  if (*count == *size)
  {
    *size *= 2;
    *instants = repalloc(*instants, sizeof(TInstant *) * (size_t) *size);
  }
  (*instants)[(*count)++] = tinstant_make(S2CellGetDatum(cell), T_TS2CELL,
    t);
  return;
}

/**
 * @brief Return the position of a temporal point instant
 */
static const POINT2D *
tpointinst_point2d(const TInstant *inst)
{
  return GSERIALIZED_POINT2D_P(DatumGetGserializedP(tinstant_value_p(inst)));
}

/**
 * @brief Append to the visits of a trajectory the cell it enters at a
 * timestamp, the previous visit ending there
 * @details As #th3index_visit_append appends an H3 cell. A cell holds
 * exactly the microseconds whose exact position lies in it. The timestamp is
 * the microsecond of the crossing, truncated: where the trajectory reaches
 * the boundary exactly at it (@p whole), the crossing position belongs to the
 * cell holding it, the one left (@p held_left), the one entered
 * (@p held_entered), or at a vertex a third cell met there, which holds that
 * instant; where it reaches the boundary after it, the trajectory is still
 * in the cell it leaves at that instant, which holds it. A crossing landing
 * before the previous one, which the truncation of two crossings within one
 * microsecond gives, lands on it
 */
static void
ts2cell_visit_append(DggsVisit **visits, int *count, int *size,
  S2CellId cell, TimestampTz t, bool whole, bool held_left, bool held_entered)
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
  v->cell = S2CellGetDatum(cell);
  v->lower = t;
  v->lower_inc = whole && held_entered;
  return;
}

/**
 * @brief Return the temporal S2 cell of a linear temporal planar point
 * sequence at a level, starting in a cell
 * @details As #tpointseq_linear_to_th3index walks the planar H3 cells: each
 * segment is walked on the straight-edged outlines of the cells
 * (#s2cell_segment_cells_planar), and each crossing is dated as the
 * restriction of a trajectory to a region dates it, by the truncation of its
 * exact parameter on the input doubles (#segment_edge_cross_offset); the
 * cell holding the crossing position holds the microsecond where the
 * trajectory lies on the edge exactly then
 */
static Temporal *
tpointseq_planar_to_ts2cell(const TSequence *seq, int32 level, S2CellId cell)
{
  int vsize = seq->count + 1, nvisits = 1;
  DggsVisit *visits = palloc(sizeof(DggsVisit) * (size_t) vsize);
  visits[0].cell = S2CellGetDatum(cell);
  visits[0].lower = TSEQUENCE_INST_N(seq, 0)->t;
  visits[0].lower_inc = seq->period.lower_inc;
  S2CellId last = cell;
  int maxout = 64;
  S2CellId *cells = palloc(sizeof(S2CellId) * (size_t) maxout);
  double *enter = palloc(sizeof(double) * (size_t) maxout);
  DggsEdgeCross *cross = palloc(sizeof(DggsEdgeCross) * (size_t) maxout);
  for (int i = 0; i + 1 < seq->count; i++)
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(seq, i);
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
    const POINT2D *p1 = tpointinst_point2d(inst1);
    const POINT2D *p2 = tpointinst_point2d(inst2);
    int ncells;
    while ((ncells = s2cell_segment_cells_planar(p1->x, p1->y, p2->x, p2->y,
        (uint32_t) level, cells, enter, cross, maxout)) == maxout)
    {
      maxout *= 2;
      cells = repalloc(cells, sizeof(S2CellId) * (size_t) maxout);
      enter = repalloc(enter, sizeof(double) * (size_t) maxout);
      cross = repalloc(cross, sizeof(DggsEdgeCross) * (size_t) maxout);
    }
    /* A segment starts in the cell the previous one ends in */
    for (int k = 0; k < ncells; k++)
    {
      if (cells[k] == last)
        continue;
      TimestampTz tenter = inst1->t;
      bool whole = true, held_left = false, held_entered = true;
      if (k > 0)
      {
        const DggsEdgeCross *x = &cross[k];
        tenter += segment_edge_cross_offset(p1->x, p1->y, p2->x, p2->y,
          inst2->t - inst1->t, x->ax, x->ay, x->oa, x->bx, x->by, x->ob,
          &whole);
        held_left = x->held_left;
        held_entered = x->held_entered;
      }
      ts2cell_visit_append(&visits, &nvisits, &vsize, cells[k], tenter,
        whole, held_left, held_entered);
      last = cells[k];
    }
    /* The endpoint's own cell, which holds it, closes the segment when the
     * traversal stopped short of it */
    S2CellId endcell = s2cell_point_cell_planar(p2->x, p2->y,
      (uint32_t) level);
    if (endcell != 0 && endcell != last)
    {
      ts2cell_visit_append(&visits, &nvisits, &vsize, endcell, inst2->t,
        true, false, true);
      last = endcell;
    }
  }
  pfree(cells); pfree(enter); pfree(cross);

  /* The last cell holds to the end of the trajectory. A crossing dated at
   * the first or the last instant of the trajectory holds that instant only
   * where the trajectory does */
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
  Temporal *result = dggs_visits_to_temporal(visits, nvisits, T_TS2CELL);
  pfree(visits);
  return result;
}

/**
 * @brief Return the temporal S2 cell of a temporal point sequence at a level,
 * or NULL when its positions are not in a lon/lat reference system
 * @details A sequence stating nothing between its instants, discrete or
 * stepwise, holds the cells of its instants. A linear sequence moves between
 * two instants along the great circle of a geodetic point, or the straight
 * line in longitude and latitude of a planar one, and each segment is
 * traversed cell by cell, so the result holds every cell the trajectory
 * crosses and each of its instants marks the time the trajectory enters that
 * cell. The last cell holds to the end of the trajectory. A planar sequence
 * answers a sequence set where a crossing instant belongs to the cell it
 * leaves (#tpointseq_planar_to_ts2cell).
 */
static Temporal *
tpointseq_to_ts2cell(const TSequence *seq, int32 level)
{
  /* The reference system is the one every instant carries, so the adapter
   * testing it reads the first instant alone */
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  S2CellId cell = geo_to_s2cell_cell(
    DatumGetGserializedP(tinstant_value_p(inst)), level);
  if (cell == (S2CellId) 0)
    return NULL;
  int size = seq->count + 1, count = 0;
  TInstant **instants = palloc(sizeof(TInstant *) * (size_t) size);
  ts2cell_entry_append(&instants, &count, &size, cell, inst->t);
  S2CellId last = cell;

  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(seq->flags);
  if (interp != LINEAR)
  {
    for (int i = 1; i < seq->count; i++)
    {
      inst = TSEQUENCE_INST_N(seq, i);
      const POINT2D *p = tpointinst_point2d(inst);
      ts2cell_entry_append(&instants, &count, &size, geodetic ?
        s2cell_point_to_cell(p->x, p->y, (uint32_t) level) :
        s2cell_point_cell_planar(p->x, p->y, (uint32_t) level), inst->t);
    }
    return (Temporal *) tsequence_make_free(instants, count,
      seq->period.lower_inc, seq->period.upper_inc, interp, NORMALIZE);
  }
  if (! geodetic)
  {
    pfree(instants[0]);
    pfree(instants);
    return tpointseq_planar_to_ts2cell(seq, level, cell);
  }

  /* The walk writes one entry per cell a segment crosses, into arrays that
   * grow until the whole segment fits, so no segment is cut short */
  int maxout = 64;
  S2CellId *cells = palloc(sizeof(S2CellId) * (size_t) maxout);
  double *enter = palloc(sizeof(double) * (size_t) maxout);
  for (int i = 0; i + 1 < seq->count; i++)
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(seq, i);
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
    const POINT2D *p1 = tpointinst_point2d(inst1);
    const POINT2D *p2 = tpointinst_point2d(inst2);
    int ncells;
    while ((ncells = s2cell_segment_cells(p1->x, p1->y, p2->x, p2->y,
        true, (uint32_t) level, cells, enter, maxout)) == maxout)
    {
      maxout *= 2;
      cells = repalloc(cells, sizeof(S2CellId) * (size_t) maxout);
      enter = repalloc(enter, sizeof(double) * (size_t) maxout);
    }
    /* A segment starts in the cell the previous one ends in */
    for (int k = 0; k < ncells; k++)
    {
      if (cells[k] == last)
        continue;
      /* A crossing the last microsecond of the segment holds is the end of
       * the segment at the resolution a timestamp states, so it enters at
       * that instant, as the clip of a segment by a geometry states a
       * parameter of 1 as the instant of the second position. A cell is then
       * entered at the same instant however the segment is cut */
      TimestampTz tenter = inst1->t +
        (TimestampTz) ((double) (inst2->t - inst1->t) * enter[k]);
      if (inst2->t - tenter <= 1)
        tenter = inst2->t;
      ts2cell_entry_append(&instants, &count, &size, cells[k], tenter);
      last = cells[k];
    }
    /* The endpoint's own cell closes the segment when the traversal stopped
     * short of it, as for an endpoint lying on a cell boundary within the
     * rounding of the crossing */
    S2CellId endcell = s2cell_point_to_cell(p2->x, p2->y, (uint32_t) level);
    if (endcell != (S2CellId) 0 && endcell != last)
    {
      ts2cell_entry_append(&instants, &count, &size, endcell, inst2->t);
      last = endcell;
    }
  }
  pfree(cells); pfree(enter);

  /* The last cell holds to the end of the trajectory, which the closing
   * instant states, since a sequence reaches no further than its last
   * instant. Under an exclusive upper bound, a cell the trajectory reaches at
   * its end is held for no time and is no part of the value */
  TimestampTz tend = TSEQUENCE_INST_N(seq, seq->count - 1)->t;
  if (! seq->period.upper_inc)
    while (count > 1 && instants[count - 1]->t >= tend)
      pfree(instants[--count]);
  if (instants[count - 1]->t < tend)
    ts2cell_entry_append(&instants, &count, &size,
      DatumGetS2Cell(tinstant_value_p(instants[count - 1])), tend);
  return (Temporal *) tsequence_make_free(instants, count,
    seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE);
}

/**
 * @brief Return the temporal S2 cell of a temporal point sequence set at a
 * level, or NULL when its positions are not in a lon/lat reference system
 * @details Each sequence answers a sequence, or a sequence set where a
 * crossing instant belongs to the cell it leaves, and the answer holds all
 * their sequences, as #tpointseqset_densify_to_th3index gathers them
 */
static TSequenceSet *
tpointseqset_to_ts2cell(const TSequenceSet *ss, int32 level)
{
  int size = ss->count, count = 0;
  TSequence **sequences = palloc(sizeof(TSequence *) * (size_t) size);
  for (int i = 0; i < ss->count; i++)
  {
    Temporal *part = tpointseq_to_ts2cell(TSEQUENCESET_SEQ_N(ss, i), level);
    if (part == NULL)
    {
      for (int j = 0; j < count; j++)
        pfree(sequences[j]);
      pfree(sequences);
      return NULL;
    }
    int n = part->subtype == TSEQUENCE ? 1 : ((TSequenceSet *) part)->count;
    if (count + n > size)
    {
      size = 2 * (count + n);
      sequences = repalloc(sequences, sizeof(TSequence *) * (size_t) size);
    }
    if (part->subtype == TSEQUENCE)
    {
      sequences[count++] = (TSequence *) part;
      continue;
    }
    const TSequenceSet *pss = (const TSequenceSet *) part;
    for (int j = 0; j < pss->count; j++)
      sequences[count++] = tsequence_copy(TSEQUENCESET_SEQ_N(pss, j));
    pfree(part);
  }
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @brief Return the temporal S2 cell of a temporal point at a level, holding
 * every cell the trajectory crosses
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 */
static Temporal *
tpoint_to_ts2cell(const Temporal *temp, int32 level, bool border_inc)
{
  if (! ensure_valid_cell_resolution(T_TS2CELL, level))
    return NULL;

  switch (temp->subtype)
  {
    case TINSTANT:
    {
      const TInstant *inst = (const TInstant *) temp;
      S2CellId cell = geo_to_s2cell_cell(
        DatumGetGserializedP(tinstant_value_p(inst)), level);
      return (cell == (S2CellId) 0) ? NULL :
        (Temporal *) tinstant_make(S2CellGetDatum(cell), T_TS2CELL, inst->t);
    }
    case TSEQUENCE:
      return tcellindex_cover_border(tpointseq_to_ts2cell(
        (const TSequence *) temp, level), border_inc);
    default: /* TSEQUENCESET */
      return tcellindex_cover_border((Temporal *) tpointseqset_to_ts2cell(
        (const TSequenceSet *) temp, level), border_inc);
  }
}

/**
 * @ingroup meos_s2cell_conversion
 * @brief Return the temporal S2 cell of a temporal geodetic point at a level,
 * holding every cell the trajectory crosses
 * @param[in] temp Temporal point
 * @param[in] level S2 level
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @csqlfn #Tgeogpoint_to_ts2cell()
 */
Temporal *
tgeogpoint_to_ts2cell(const Temporal *temp, int32 level, bool border_inc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOGPOINT(temp, NULL);
  return tpoint_to_ts2cell(temp, level, border_inc);
}

/**
 * @ingroup meos_s2cell_conversion
 * @brief Return the temporal S2 cell of a temporal planar point in a lon/lat
 * reference system at a level
 * @details The result holds every cell the trajectory crosses along its
 * straight lines in longitude and latitude.
 * @param[in] temp Temporal point
 * @param[in] level S2 level
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @csqlfn #Tgeompoint_to_ts2cell()
 */
Temporal *
tgeompoint_to_ts2cell(const Temporal *temp, int32 level, bool border_inc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(temp, NULL);
  return tpoint_to_ts2cell(temp, level, border_inc);
}

/*****************************************************************************
 * Split
 *****************************************************************************/

/**
 * @ingroup meos_internal_s2cell_conversion
 * @brief Return the fragments of a temporal point split by the S2 cells it
 * crosses at a level, and the cell of each
 * @details The cover states which cell the trajectory holds and when, and the
 * fragment of a cell is the trajectory over the periods the cover states for
 * it, so a fragment and the cover answer the same periods for a cell
 * @param[in] temp Temporal point
 * @param[in] level S2 level
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] cells Cell of each fragment
 * @param[out] count Number of fragments
 */
Temporal **
tpoint_s2cell_split(const Temporal *temp, int32 level, bool border_inc,
  Datum **cells, int *count)
{
  assert(temp); assert(cells); assert(count);
  *count = 0;
  Temporal *cover = tpoint_to_ts2cell(temp, level, border_inc);
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
 * @brief Return the S2 cells of an array of datums, freeing the array
 */
static S2CellId *
s2cell_cells(Datum *datums, int count)
{
  S2CellId *result = palloc(sizeof(S2CellId) * count);
  for (int i = 0; i < count; i++)
    result[i] = DatumGetS2Cell(datums[i]);
  pfree(datums);
  return result;
}

/**
 * @ingroup meos_s2cell_conversion
 * @brief Return the fragments of a temporal planar point split by the S2
 * cells it crosses at a level, and the cell of each
 * @param[in] temp Temporal point
 * @param[in] level S2 level
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] cells Cell of each fragment
 * @param[out] count Number of fragments
 * @csqlfn #Tgeompoint_s2cell_split()
 */
Temporal **
tgeompoint_s2cell_split(const Temporal *temp, int32 level, bool border_inc,
  S2CellId **cells, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(temp, NULL); VALIDATE_NOT_NULL(cells, NULL);

  Datum *datum_cells;
  Temporal **result = tpoint_s2cell_split(temp, level, border_inc,
    &datum_cells, count);
  if (! result)
    return NULL;
  *cells = s2cell_cells(datum_cells, *count);
  return result;
}

/**
 * @ingroup meos_s2cell_conversion
 * @brief Return the fragments of a temporal geodetic point split by the S2
 * cells it crosses at a level, and the cell of each
 * @param[in] temp Temporal point
 * @param[in] level S2 level
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] cells Cell of each fragment
 * @param[out] count Number of fragments
 * @csqlfn #Tgeogpoint_s2cell_split()
 */
Temporal **
tgeogpoint_s2cell_split(const Temporal *temp, int32 level, bool border_inc,
  S2CellId **cells, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOGPOINT(temp, NULL); VALIDATE_NOT_NULL(cells, NULL);

  Datum *datum_cells;
  Temporal **result = tpoint_s2cell_split(temp, level, border_inc,
    &datum_cells, count);
  if (! result)
    return NULL;
  *cells = s2cell_cells(datum_cells, *count);
  return result;
}

/*****************************************************************************/
