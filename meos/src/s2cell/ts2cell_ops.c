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
 * @brief Datum-convention wrappers for the S2 static kernel and the
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
  char *str = s2cell_cell_to_token(DatumGetS2Cell(d));
  text *result = cstring_to_text(str);
  pfree(str);
  return PointerGetDatum(result);
}

/*****************************************************************************
 * Descriptor
 *****************************************************************************/

/**
 * @brief S2 operations descriptor consumed by `dggs_cellops()`.
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
 * @brief Return the temporal S2 cell of a temporal point sequence at a level,
 * or NULL when its positions are not in a lon/lat reference system
 * @details A sequence stating nothing between its instants, discrete or
 * stepwise, holds the cells of its instants. A linear sequence moves between
 * two instants along the great circle of a geodetic point, or the straight
 * line in longitude and latitude of a planar one, and each segment is
 * traversed cell by cell, so the result holds every cell the trajectory
 * crosses and each of its instants marks the time the trajectory enters that
 * cell. The last cell holds to the end of the trajectory.
 */
static TSequence *
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
  if (interp != LINEAR)
  {
    for (int i = 1; i < seq->count; i++)
    {
      inst = TSEQUENCE_INST_N(seq, i);
      const POINT2D *p = tpointinst_point2d(inst);
      ts2cell_entry_append(&instants, &count, &size,
        s2cell_point_to_cell(p->x, p->y, (uint32_t) level), inst->t);
    }
    return tsequence_make_free(instants, count, seq->period.lower_inc,
      seq->period.upper_inc, interp, NORMALIZE);
  }

  /* The walk writes one entry per cell a segment crosses, into arrays that
   * grow until the whole segment fits, so no segment is cut short */
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(seq->flags);
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
        geodetic, (uint32_t) level, cells, enter, maxout)) == maxout)
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
  return tsequence_make_free(instants, count, seq->period.lower_inc,
    seq->period.upper_inc, STEP, NORMALIZE);
}

/**
 * @brief Return the temporal S2 cell of a temporal point sequence set at a
 * level, or NULL when its positions are not in a lon/lat reference system
 */
static TSequenceSet *
tpointseqset_to_ts2cell(const TSequenceSet *ss, int32 level)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * (size_t) ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    sequences[i] = tpointseq_to_ts2cell(TSEQUENCESET_SEQ_N(ss, i), level);
    if (sequences[i] == NULL)
    {
      for (int j = 0; j < i; j++)
        pfree(sequences[j]);
      pfree(sequences);
      return NULL;
    }
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @brief Return the temporal S2 cell of a temporal point at a level, holding
 * every cell the trajectory crosses
 */
static Temporal *
tpoint_to_ts2cell(const Temporal *temp, int32 level)
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
      return (Temporal *) tpointseq_to_ts2cell((const TSequence *) temp,
        level);
    default: /* TSEQUENCESET */
      return (Temporal *) tpointseqset_to_ts2cell(
        (const TSequenceSet *) temp, level);
  }
}

/**
 * @ingroup meos_s2cell_conversion
 * @brief Return the temporal S2 cell of a temporal geodetic point at a level,
 * holding every cell the trajectory crosses
 * @param[in] temp Temporal point
 * @param[in] level S2 level
 * @csqlfn #Tgeogpoint_to_ts2cell()
 */
Temporal *
tgeogpoint_to_ts2cell(const Temporal *temp, int32 level)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOGPOINT(temp, NULL);
  return tpoint_to_ts2cell(temp, level);
}

/**
 * @ingroup meos_s2cell_conversion
 * @brief Return the temporal S2 cell of a temporal planar point in a lon/lat
 * reference system at a level, holding every cell the trajectory crosses
 * along its straight lines in longitude and latitude
 * @param[in] temp Temporal point
 * @param[in] level S2 level
 * @csqlfn #Tgeompoint_to_ts2cell()
 */
Temporal *
tgeompoint_to_ts2cell(const Temporal *temp, int32 level)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(temp, NULL);
  return tpoint_to_ts2cell(temp, level);
}

/*****************************************************************************/
