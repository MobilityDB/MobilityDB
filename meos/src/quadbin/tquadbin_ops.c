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
 * @brief Datum wrappers and cell operations of the QUADBIN index
 * @details Datum-convention wrappers for the quadbin static kernel and the
 * `quadbin_cellops` descriptor that plugs quadbin into the shared temporal
 * cell-index machinery (meos/src/temporal/tcellindex.c).
 *
 * A quadbin cell is a uint64 carried in a Datum with the int8/bigint
 * convention (Int64GetDatum / DatumGetInt64), holding the Bing Maps Tile
 * System (quadkey) cell of a map uniformly subdivided in the MERCATOR
 * projection into four squares at each resolution from 0 to 26
 * (https://docs.carto.com/data-and-analysis/analytics-toolbox-for-bigquery/key-concepts/spatial-indexes).
 * The cell is defined on that projection, and its centroid is emitted as a
 * planar tgeompoint converted to lon/lat (SRID 4326); a Web-Mercator
 * (SRID 3857) emission, which is the cell's own system, would set
 * point_srid = 3857 and skip that conversion.
 */

#include "quadbin/quadbin.h"
#include "quadbin/tquadbin.h"

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
#include <meos_quadbin.h>
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
 * @brief Return the resolution of a quadbin cell
 */
static Datum
datum_quadbin_get_resolution(Datum d)
{
  return Int32GetDatum((int32) quadbin_get_resolution((Quadbin) DatumGetInt64(d)));
}

/**
 * @brief Return true if a value is a valid quadbin cell
 */
static Datum
datum_quadbin_is_valid_cell(Datum d)
{
  return BoolGetDatum(quadbin_is_valid_cell((Quadbin) DatumGetInt64(d)));
}

/**
 * @brief Return the parent of a quadbin cell at a given resolution
 */
static Datum
datum_quadbin_cell_to_parent(Datum cell_d, Datum res_d)
{
  Quadbin parent = quadbin_cell_to_parent((Quadbin) DatumGetInt64(cell_d),
    (uint32_t) DatumGetInt32(res_d));
  return Int64GetDatum((int64) parent);
}

/**
 * @brief Return the center point of a quadbin cell
 */
static Datum
datum_quadbin_cell_to_point(Datum d)
{
  double lon, lat;
  quadbin_cell_point((Quadbin) DatumGetInt64(d), &lon, &lat);
  /* Planar (non-geodetic) lon/lat point, SRID 4326. geopoint_make is
   * available in both the MEOS and the MEOS=OFF extension build, unlike the
   * MEOS-only geompoint_make2d. */
  GSERIALIZED *gs = geopoint_make(lon, lat, 0.0, false, false, 4326);
  return PointerGetDatum(gs);
}

/**
 * @brief Return the boundary of a quadbin cell as a geometry
 */
static Datum
datum_quadbin_cell_to_boundary(Datum d)
{
  double xmin, ymin, xmax, ymax;
  quadbin_cell_bounding_box((Quadbin) DatumGetInt64(d), &xmin, &ymin,
    &xmax, &ymax);
  POINTARRAY *pa = ptarray_construct_empty(LW_FALSE, LW_FALSE, 5);
  POINT4D pt;
  pt.z = 0.0; pt.m = 0.0;
  pt.x = xmin; pt.y = ymin; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmax; pt.y = ymin; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmax; pt.y = ymax; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmin; pt.y = ymax; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmin; pt.y = ymin; ptarray_append_point(pa, &pt, LW_TRUE); /* close */
  LWPOLY *poly = lwpoly_construct_empty(4326, LW_FALSE, LW_FALSE);
  lwpoly_add_ring(poly, pa);
  GSERIALIZED *gs = geo_serialize(lwpoly_as_lwgeom(poly));
  lwpoly_free(poly);
  return PointerGetDatum(gs);
}

/**
 * @brief Return the quadkey of a quadbin cell
 */
static Datum
datum_quadbin_cell_to_quadkey(Datum d)
{
  char *str = quadbin_cell_to_quadkey((Quadbin) DatumGetInt64(d));
  text *result = cstring_to_text(str);
  pfree(str);
  return PointerGetDatum(result);
}

/**
 * @brief Return the area in square meters of a quadbin cell
 */
static Datum
datum_quadbin_cell_area(Datum d)
{
  return Float8GetDatum(quadbin_cell_area((Quadbin) DatumGetInt64(d)));
}

/*****************************************************************************
 * Descriptor
 *****************************************************************************/

/**
 * @brief Quadbin operations descriptor consumed by `dggs_cellops()`
 */
const DggsCellOps quadbin_cellops =
{
  .celltype        = T_QUADBIN,
  .settype         = T_QUADBINSET,
  .temptype        = T_TQUADBIN,
  .min_resolution  = 0,
  .max_resolution  = 26,
  .point_temptype  = T_TGEOMPOINT,
  .point_srid      = 4326,
  .get_resolution  = &datum_quadbin_get_resolution,
  .is_valid_cell   = &datum_quadbin_is_valid_cell,
  .cell_to_parent  = &datum_quadbin_cell_to_parent,
  .cell_to_point   = &datum_quadbin_cell_to_point,
  .cell_to_boundary = &datum_quadbin_cell_to_boundary,
  .cell_area       = &datum_quadbin_cell_area
};

/*****************************************************************************
 * Quadbin-unique temporal op: cell -> quadkey (ttext)
 *
 * The quadkey (base-4 slippy-tile string) has no H3 analogue, so it is a typed
 * tquadbin function rather than a generic DggsCellOps entry: the descriptor
 * exposes only the operations shared by every DGGS.
 *****************************************************************************/

/**
 * @ingroup meos_cellindex
 * @brief Return the temporal quadkey (ttext) of a temporal quadbin cell
 * @csqlfn #Tquadbin_cell_to_quadkey()
 */
Temporal *
tquadbin_cell_to_quadkey(const Temporal *temp)
{
  /* Ensure the validity of the parameters */
  VALIDATE_TQUADBIN(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_quadbin_cell_to_quadkey;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TQUADBIN;
  lfinfo.restype = T_TTEXT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * tgeompoint -> tquadbin
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
tquadbin_entry_append(TInstant ***instants, int *count, int *size,
  Quadbin cell, TimestampTz t)
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
  (*instants)[(*count)++] = tinstant_make(QuadbinGetDatum(cell), T_TQUADBIN,
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
 * @brief Return true when the position where a trajectory crosses from a cell
 * into the next belongs to the next one
 * @details A tile holds the lower bounds of its tile coordinates, the column
 * growing east and the row growing south, so the boundary between two tiles
 * belongs to the one of higher index along the axis the crossing changes: a
 * trajectory moving east or south enters the tile holding the boundary, and
 * one moving west or north leaves it. A crossing through a corner reaches the
 * tile holding the corner first, and the two tiles it steps between agree on
 * which of them holds the crossing, since both coordinates move the same way
 * relative to it
 */
static bool
tquadbin_entered_holds(Quadbin cell, Quadbin next)
{
  uint32_t x1, y1, z1, x2, y2, z2;
  quadbin_cell_tile(cell, &x1, &y1, &z1);
  quadbin_cell_tile(next, &x2, &y2, &z2);
  return x2 > x1 || (x2 == x1 && y2 > y1);
}

/**
 * @brief Append to the visits of a trajectory the cell it enters at a
 * timestamp, the previous visit ending there
 * @details The crossing position belongs to one of the two cells
 * (#tquadbin_entered_holds), which holds the crossing instant, the other one
 * leaving it out. A crossing landing before the previous one, which the
 * truncation of two crossings closer than a microsecond can give, lands on
 * it: the instants are never moved past one another
 */
static void
tquadbin_visit_append(DggsVisit **visits, int *count, int *size, Quadbin cell,
  TimestampTz t)
{
  DggsVisit *prev = &(*visits)[*count - 1];
  if (t < prev->lower)
    t = prev->lower;
  bool entered = tquadbin_entered_holds(DatumGetQuadbin(prev->cell), cell);
  prev->upper = t;
  prev->upper_inc = ! entered;
  if (*count == *size)
  {
    *size *= 2;
    *visits = repalloc(*visits, sizeof(DggsVisit) * (size_t) *size);
  }
  DggsVisit *v = &(*visits)[(*count)++];
  v->cell = QuadbinGetDatum(cell);
  v->lower = t;
  v->lower_inc = entered;
  return;
}

/**
 * @brief Column and row of the tiles a grid leaves out of a trajectory whose
 * extent does not contain its upper border, -1 where none is left out
 */
typedef struct
{
  long x;             /**< Column left out, or -1 */
  long y;             /**< Row left out, or -1 */
} QuadbinBorder;

/**
 * @brief Set the column and the row of the tiles holding only the upper border
 * of the extent of a trajectory, which a grid not containing that border
 * leaves out
 * @details A tile holds the lower bounds of its tile coordinates, the column
 * growing east and the row growing south, so the upper border of the extent
 * is its easternmost longitude and its southernmost latitude. As the split of
 * a temporal point by a space grid lays its tiles (#tile_dim_count), the tile
 * starting at that border holds nothing of the extent but the border itself
 * where the border lies exactly on a tile boundary and the extent spreads
 * along that axis, and the grid leaves it out when it does not contain the
 * border. The extent is the bounding box of the trajectory, which for a
 * geodetic one holds its great circles
 */
static void
tquadbin_border(const Temporal *temp, int32 resolution, bool border_inc,
  QuadbinBorder *border)
{
  border->x = border->y = -1;
  if (border_inc)
    return;
  STBox box;
  tspatial_set_stbox(temp, &box);
  double n = (double) (UINT64_C(1) << resolution);
  double xwest, ynorth, xeast, ysouth;
  quadbin_tile_coords(box.xmin, box.ymax, n, &xwest, &ynorth);
  quadbin_tile_coords(box.xmax, box.ymin, n, &xeast, &ysouth);
  if (xeast > xwest && xeast == floor(xeast) && xeast < n)
    border->x = (long) xeast;
  if (ysouth > ynorth && ysouth == floor(ysouth) && ysouth < n)
    border->y = (long) ysouth;
  return;
}

/**
 * @brief Return true when a cell is a tile the grid leaves out
 */
static bool
tquadbin_in_border(Quadbin cell, const QuadbinBorder *border)
{
  if (border->x < 0 && border->y < 0)
    return false;
  uint32_t x, y, z;
  quadbin_cell_tile(cell, &x, &y, &z);
  return (long) x == border->x || (long) y == border->y;
}

/**
 * @brief Return the temporal quadbin cell of a temporal point sequence at a
 * resolution
 * @details The result is NULL when the positions of the sequence are not in a
 * lon/lat reference system. A sequence stating nothing between its instants,
 * discrete or stepwise, holds the cells of its instants. A linear sequence
 * moves between two instants along the straight line in longitude and
 * latitude of a planar point, or the great circle of a geodetic one, and each
 * segment is traversed tile by tile, so the result holds every cell the
 * trajectory crosses, each over the period the trajectory spends in it, as
 * the split of a temporal point by a space grid states the fragment of each
 * tile. A crossing instant belongs to the cell the grid assigns the crossing
 * position to, so a trajectory crossing into a tile of lower index states the
 * tile it leaves at that instant and the next one after it, which a sequence
 * set holds.
 */
static Temporal *
tpointseq_to_tquadbin(const TSequence *seq, int32 resolution,
  const QuadbinBorder *border)
{
  /* The reference system is the one every instant carries, so the adapter
   * testing it reads the first instant alone */
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  Quadbin cell = geo_to_quadbin_cell(
    DatumGetGserializedP(tinstant_value_p(inst)), resolution);
  if (cell == (Quadbin) 0)
    return NULL;

  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  if (interp == DISCRETE)
  {
    /* The instants of the trajectory, those in a tile the grid leaves out
     * dropped */
    int size = seq->count + 1, count = 0;
    TInstant **instants = palloc(sizeof(TInstant *) * (size_t) size);
    for (int i = 0; i < seq->count; i++)
    {
      inst = TSEQUENCE_INST_N(seq, i);
      const POINT2D *p = tpointinst_point2d(inst);
      Quadbin c = quadbin_point_to_cell(p->x, p->y, (uint32_t) resolution);
      if (! tquadbin_in_border(c, border))
        tquadbin_entry_append(&instants, &count, &size, c, inst->t);
    }
    if (count == 0)
    {
      pfree(instants);
      return NULL;
    }
    return (Temporal *) tsequence_make_free(instants, count, true, true,
      DISCRETE, NORMALIZE);
  }
  if (interp == STEP)
  {
    /* A stepwise trajectory holds the cell of each instant up to the next
     * one, and the cell of its last instant at that instant */
    DggsVisit *visits = palloc(sizeof(DggsVisit) * (size_t) seq->count);
    for (int i = 0; i < seq->count; i++)
    {
      inst = TSEQUENCE_INST_N(seq, i);
      const POINT2D *p = tpointinst_point2d(inst);
      visits[i].cell = QuadbinGetDatum(quadbin_point_to_cell(p->x, p->y,
        (uint32_t) resolution));
      visits[i].lower = inst->t;
      visits[i].lower_inc = (i == 0) ? seq->period.lower_inc : true;
      if (i + 1 < seq->count)
      {
        visits[i].upper = TSEQUENCE_INST_N(seq, i + 1)->t;
        visits[i].upper_inc = false;
      }
      else
      {
        visits[i].upper = inst->t;
        visits[i].upper_inc = seq->period.upper_inc;
      }
      /* A tile the grid leaves out holds no time */
      if (tquadbin_in_border(DatumGetQuadbin(visits[i].cell), border))
      {
        visits[i].upper = visits[i].lower;
        visits[i].lower_inc = false;
      }
    }
    Temporal *result = dggs_visits_to_temporal(visits, seq->count,
      T_TQUADBIN);
    pfree(visits);
    return result;
  }

  int vsize = seq->count + 1, nvisits = 1;
  DggsVisit *visits = palloc(sizeof(DggsVisit) * (size_t) vsize);
  visits[0].cell = QuadbinGetDatum(cell);
  visits[0].lower = inst->t;
  visits[0].lower_inc = seq->period.lower_inc;
  Quadbin last = cell;

  /* The walk writes one entry per cell a segment crosses, into arrays that
   * grow until the whole segment fits, so no segment is cut short */
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(seq->flags);
  int maxout = 64;
  Quadbin *cells = palloc(sizeof(Quadbin) * (size_t) maxout);
  double *enter = palloc(sizeof(double) * (size_t) maxout);
  for (int i = 0; i + 1 < seq->count; i++)
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(seq, i);
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
    const POINT2D *p1 = tpointinst_point2d(inst1);
    const POINT2D *p2 = tpointinst_point2d(inst2);
    int ncells;
    while ((ncells = quadbin_segment_cells(p1->x, p1->y, p2->x, p2->y,
        geodetic, (uint32_t) resolution, cells, enter, maxout)) == maxout)
    {
      maxout *= 2;
      cells = repalloc(cells, sizeof(Quadbin) * (size_t) maxout);
      enter = repalloc(enter, sizeof(double) * (size_t) maxout);
    }
    /* A segment starts in the cell the previous one ends in */
    for (int k = 0; k < ncells; k++)
    {
      if (cells[k] == last)
        continue;
      /* The crossing is dated as the restriction of a trajectory to a tile
       * dates it, truncating its parameter to the microsecond */
      TimestampTz tenter = inst1->t +
        (TimestampTz) ((double) (inst2->t - inst1->t) * enter[k]);
      tquadbin_visit_append(&visits, &nvisits, &vsize, cells[k], tenter);
      last = cells[k];
    }
    /* The endpoint's own cell closes the segment when the traversal stopped
     * short of it, as for an endpoint lying on a tile boundary within the
     * rounding of the crossing */
    Quadbin endcell = quadbin_point_to_cell(p2->x, p2->y,
      (uint32_t) resolution);
    if (endcell != last)
    {
      tquadbin_visit_append(&visits, &nvisits, &vsize, endcell, inst2->t);
      last = endcell;
    }
  }
  pfree(cells); pfree(enter);

  /* The last cell holds to the end of the trajectory. A crossing dated at
   * the first or the last instant of the trajectory holds that instant only
   * where the trajectory does, as the restriction of the trajectory to a
   * tile answers no instant outside its period */
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
    /* A tile the grid leaves out holds no time */
    if (tquadbin_in_border(DatumGetQuadbin(visits[i].cell), border))
    {
      visits[i].upper = visits[i].lower;
      visits[i].lower_inc = false;
    }
  }
  Temporal *result = dggs_visits_to_temporal(visits, nvisits, T_TQUADBIN);
  pfree(visits);
  return result;
}

/**
 * @brief Return the temporal quadbin cell of a temporal point sequence set at
 * a resolution
 * @details Return NULL when its positions are not in a lon/lat reference
 * system. Each sequence answers a sequence, or a sequence set where a
 * crossing instant belongs to the cell it leaves, and the answer holds all
 * their sequences.
 */
static TSequenceSet *
tpointseqset_to_tquadbin(const TSequenceSet *ss, int32 resolution,
  const QuadbinBorder *border)
{
  int size = ss->count, count = 0;
  TSequence **sequences = palloc(sizeof(TSequence *) * (size_t) size);
  for (int i = 0; i < ss->count; i++)
  {
    Temporal *part = tpointseq_to_tquadbin(TSEQUENCESET_SEQ_N(ss, i),
      resolution, border);
    /* A sequence lying in the tiles the grid leaves out holds none */
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
 * @brief Return the temporal quadbin cell of a temporal point at a resolution,
 * holding every cell the trajectory crosses
 */
static Temporal *
tpoint_to_tquadbin(const Temporal *temp, int32 resolution, bool border_inc)
{
  if (! ensure_valid_cell_resolution(T_TQUADBIN, resolution))
    return NULL;
  QuadbinBorder border;
  tquadbin_border(temp, resolution, border_inc, &border);

  switch (temp->subtype)
  {
    case TINSTANT:
    {
      const TInstant *inst = (const TInstant *) temp;
      Quadbin cell = geo_to_quadbin_cell(
        DatumGetGserializedP(tinstant_value_p(inst)), resolution);
      return (cell == (Quadbin) 0) ? NULL :
        (Temporal *) tinstant_make(QuadbinGetDatum(cell), T_TQUADBIN, inst->t);
    }
    case TSEQUENCE:
      return tpointseq_to_tquadbin((const TSequence *) temp, resolution,
        &border);
    default: /* TSEQUENCESET */
      return (Temporal *) tpointseqset_to_tquadbin(
        (const TSequenceSet *) temp, resolution, &border);
  }
}

/**
 * @ingroup meos_quadbin_conversion
 * @brief Return the temporal quadbin cell of a temporal planar point in a
 * lon/lat reference system at a resolution
 * @details The result holds every cell the trajectory crosses, each over the
 * period the split of the trajectory by a space grid states for its tile.
 * @param[in] temp Temporal point
 * @param[in] resolution Quadbin resolution
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent of the trajectory, its easternmost longitude and its southernmost
 * latitude
 * @csqlfn #Tgeompoint_to_tquadbin()
 */
Temporal *
tgeompoint_to_tquadbin(const Temporal *temp, int32 resolution,
  bool border_inc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(temp, NULL);
  return tpoint_to_tquadbin(temp, resolution, border_inc);
}

/**
 * @ingroup meos_quadbin_conversion
 * @brief Return the temporal quadbin cell of a temporal geodetic point at a
 * resolution
 * @details The result holds every cell the trajectory crosses along its great
 * circles, each over the period the split of the trajectory by a space grid
 * states for its tile.
 * @param[in] temp Temporal point
 * @param[in] resolution Quadbin resolution
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent of the trajectory, its easternmost longitude and its southernmost
 * latitude
 * @csqlfn #Tgeogpoint_to_tquadbin()
 */
Temporal *
tgeogpoint_to_tquadbin(const Temporal *temp, int32 resolution,
  bool border_inc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOGPOINT(temp, NULL);
  return tpoint_to_tquadbin(temp, resolution, border_inc);
}

/*****************************************************************************
 * Split
 *****************************************************************************/

/**
 * @brief Return the fragments of a temporal point split by the quadbin cells
 * it crosses at a resolution, and the cell of each
 * @details The cover states which cell the trajectory holds and when, and the
 * fragment of a cell is the trajectory over the periods the cover states for
 * it, so a fragment and the cover answer the same periods for a cell
 */
static Temporal **
tpoint_quadbin_split(const Temporal *temp, int32 resolution, bool border_inc,
  Datum **cells, int *count)
{
  assert(temp); assert(cells); assert(count);
  *count = 0;
  Temporal *cover = tpoint_to_tquadbin(temp, resolution, border_inc);
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
    /* A cell the cover holds for an instant alone under an exclusive upper
     * bound leaves the trajectory nothing over its periods */
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
 * @ingroup meos_quadbin_conversion
 * @brief Return the fragments of a temporal planar point split by the quadbin
 * cells it crosses at a resolution, and the cell of each
 * @param[in] temp Temporal point
 * @param[in] resolution Quadbin resolution
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent of the trajectory, its easternmost longitude and its southernmost
 * latitude
 * @param[out] cells Cell of each fragment
 * @param[out] count Number of fragments
 * @csqlfn #Tgeompoint_quadbin_split()
 */
Temporal **
tgeompoint_quadbin_split(const Temporal *temp, int32 resolution,
  bool border_inc, Datum **cells, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(temp, NULL); VALIDATE_NOT_NULL(cells, NULL);
  VALIDATE_NOT_NULL(count, NULL);
  return tpoint_quadbin_split(temp, resolution, border_inc, cells, count);
}

/**
 * @ingroup meos_quadbin_conversion
 * @brief Return the fragments of a temporal geodetic point split by the
 * quadbin cells it crosses at a resolution, and the cell of each
 * @param[in] temp Temporal point
 * @param[in] resolution Quadbin resolution
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent of the trajectory, its easternmost longitude and its southernmost
 * latitude
 * @param[out] cells Cell of each fragment
 * @param[out] count Number of fragments
 * @csqlfn #Tgeogpoint_quadbin_split()
 */
Temporal **
tgeogpoint_quadbin_split(const Temporal *temp, int32 resolution,
  bool border_inc, Datum **cells, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOGPOINT(temp, NULL); VALIDATE_NOT_NULL(cells, NULL);
  VALIDATE_NOT_NULL(count, NULL);
  return tpoint_quadbin_split(temp, resolution, border_inc, cells, count);
}

/*****************************************************************************/
