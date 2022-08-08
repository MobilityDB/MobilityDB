/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief Functions for selectivity estimation of operators on temporal points.
 */

#include "pg_point/tpoint_selfuncs.h"

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <parser/parsetree.h>
#include <utils/syscache.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
/* MobilityDB */
#include "pg_general/span_selfuncs.h"
#include "pg_general/temporal_catalog.h"
#include "pg_general/temporal_selfuncs.h"
#include "pg_point/tpoint_analyze.h"
#include "pg_npoint/tnpoint_selfuncs.h"

/*****************************************************************************
 * Boolean functions for the operators
 * PostGIS provides nd_box_intersects which is copied in tpoint_analyze.c
 *****************************************************************************/

/**
 * Return true if a contains b, false otherwise.
 */
static int
nd_box_contains(const ND_BOX *a, const ND_BOX *b, int ndims)
{
  int d;
  for (d = 0; d < ndims; d++)
  {
    if (! ((a->min[d] < b->min[d]) && (a->max[d] > b->max[d])))
      return false;
  }
  return true;
}

/**
 * Return true if a is strictly left of b, false otherwise.
 */
static bool
nd_box_left(const ND_BOX *a, const ND_BOX *b)
{
  return (a->max[X_DIM] < b->min[X_DIM]);
}

/**
 * Return true if a does not extend to right of b, false otherwise.
 */
static bool
nd_box_overleft(const ND_BOX *a, const ND_BOX *b)
{
  return (a->max[X_DIM] <= b->max[X_DIM]);
}

/**
 * Return true if a is strictly right of b, false otherwise.
 */
static bool
nd_box_right(const ND_BOX *a, const ND_BOX *b)
{
  return (a->min[X_DIM] > b->max[X_DIM]);
}

/**
 * Return true if a does not extend to left of b, false otherwise.
 */
static bool
nd_box_overright(const ND_BOX *a, const ND_BOX *b)
{
  return (a->min[X_DIM] >= b->min[X_DIM]);
}

/**
 * Return true if a is strictly below of b, false otherwise.
 */
static bool
nd_box_below(const ND_BOX *a, const ND_BOX *b)
{
  return (a->max[Y_DIM] < b->min[Y_DIM]);
}

/**
 * Return true if a does not extend above of b, false otherwise.
 */
static bool
nd_box_overbelow(const ND_BOX *a, const ND_BOX *b)
{
  return (a->max[Y_DIM] <= b->max[Y_DIM]);
}

/**
 * Return true if a is strictly above of b, false otherwise.
 */
static bool
nd_box_above(const ND_BOX *a, const ND_BOX *b)
{
  return (a->min[Y_DIM] > b->max[Y_DIM]);
}

/**
 * Return true if a does not extend below of b, false otherwise.
 */
static bool
nd_box_overabove(const ND_BOX *a, const ND_BOX *b)
{
  return (a->min[Y_DIM] >= b->min[Y_DIM]);
}

/**
 * Return true if a is strictly front of b, false otherwise.
 */
static bool
nd_box_front(const ND_BOX *a, const ND_BOX *b)
{
  return (a->max[Z_DIM] < b->min[Z_DIM]);
}

/**
 * Return true if a does not extend to the back of b, false otherwise.
 */
static bool
nd_box_overfront(const ND_BOX *a, const ND_BOX *b)
{
  return (a->max[Z_DIM] <= b->max[Z_DIM]);
}

/**
 * Return true if a strictly back of b, false otherwise.
 */
static bool
nd_box_back(const ND_BOX *a, const ND_BOX *b)
{
  return (a->min[Z_DIM] > b->max[Z_DIM]);
}

/**
 * Return true if a does not extend to the front of b, false otherwise.
 */
static bool
nd_box_overback(const ND_BOX *a, const ND_BOX *b)
{
  return (a->min[Z_DIM] >= b->min[Z_DIM]);
}

/*****************************************************************************
 * Proportion functions for the operators
 * Function nd_box_ratio_overlaps is defined in tpoint_analyze.c and is
 * copied from PostGIS function nd_box_ratio
 *****************************************************************************/

/**
 * Return the proportion of b2 that is left of b1.
 */
static double
nd_box_ratio_left(const ND_BOX *b1, const ND_BOX *b2)
{
  double delta, width;

  if (nd_box_overright(b2, b1))
    return 0.0;
  else if (nd_box_left(b2, b1))
    return 1.0;

  /* b2 is partially to the left of b1 */
  delta = b1->min[X_DIM] - b2->min[X_DIM];
  width = b2->max[X_DIM] - b2->min[X_DIM];
  return delta / width;
}

/**
 * Return the proportion of b2 that is overleft of b1.
 */
static double
nd_box_ratio_overleft(const ND_BOX *b1, const ND_BOX *b2)
{
  double delta, width;

  if (nd_box_right(b2, b1))
    return 0.0;
  else if (nd_box_overleft(b2, b1))
    return 1.0;

  /* b2 is partially to the right of b1 */
  delta = b2->max[X_DIM] - b1->max[X_DIM];
  width = b2->max[X_DIM] - b2->min[X_DIM];
  return delta / width;
}

/**
 * Return the proportion of b2 that is right of b1.
 */
static double
nd_box_ratio_right(const ND_BOX *b1, const ND_BOX *b2)
{
  double delta, width;

  if (nd_box_overleft(b2, b1))
    return 0.0;
  else if (nd_box_right(b2, b1))
    return 1.0;

  /* b2 is partially to the right of b1 */
  delta = b2->max[X_DIM] - b1->max[X_DIM];
  width = b2->max[X_DIM] - b2->min[X_DIM];
  return delta / width;
}

/**
 * Return the proportion of b2 that is overright of b1.
 */
static double
nd_box_ratio_overright(const ND_BOX *b1, const ND_BOX *b2)
{
  double delta, width;

  if (nd_box_left(b2, b1))
    return 0.0;
  else if (nd_box_overright(b2, b1))
    return 1.0;

  /* b2 is partially to the left of b1 */
  delta = b1->min[X_DIM] - b2->min[X_DIM];
  width = b2->max[X_DIM] - b2->min[X_DIM];
  return delta / width;
}

/**
 * Return the proportion of b2 that is below of b1.
 */
static double
nd_box_ratio_below(const ND_BOX *b1, const ND_BOX *b2)
{
  double delta, width;

  if (nd_box_overabove(b2, b1))
    return 0.0;
  else if (nd_box_below(b2, b1))
    return 1.0;

  /* b2 is partially to the below of b1 */
  delta = b1->min[Y_DIM] - b2->min[Y_DIM];
  width = b2->max[Y_DIM] - b2->min[Y_DIM];
  return delta / width;
}

/**
 * Return the proportion of b2 that is overbelow of b1.
 */
static double
nd_box_ratio_overbelow(const ND_BOX *b1, const ND_BOX *b2)
{
  double delta, width;

  if (nd_box_above(b2, b1))
    return 0.0;
  else if (nd_box_overbelow(b2, b1))
    return 1.0;

  /* b2 is partially to the above of b1 */
  delta = b2->max[Y_DIM] - b1->max[Y_DIM];
  width = b2->max[Y_DIM] - b2->min[Y_DIM];
  return delta / width;
}

/**
 * Return the proportion of b2 that is above of b1.
 */
static double
nd_box_ratio_above(const ND_BOX *b1, const ND_BOX *b2)
{
  double delta, width;

  if (nd_box_overbelow(b2, b1))
    return 0.0;
  else if (nd_box_above(b2, b1))
    return 1.0;

  /* b2 is partially to the above of b1 */
  delta = b2->max[Y_DIM] - b1->max[Y_DIM];
  width = b2->max[Y_DIM] - b2->min[Y_DIM];
  return delta / width;
}

/**
 * Return the proportion of b2 that is overabove of b1.
 */
static double
nd_box_ratio_overabove(const ND_BOX *b1, const ND_BOX *b2)
{
  double delta, width;

  if (nd_box_below(b2, b1))
    return 0.0;
  else if (nd_box_overabove(b2, b1))
    return 1.0;

  /* b2 is partially to the below of b1 */
  delta = b1->min[Y_DIM] - b2->min[Y_DIM];
  width = b2->max[Y_DIM] - b2->min[Y_DIM];
  return delta / width;
}

/**
 * Return the proportion of b2 that is front of b1.
 */
static double
nd_box_ratio_front(const ND_BOX *b1, const ND_BOX *b2)
{
  double delta, width;

  if (nd_box_overback(b2, b1))
    return 0.0;
  else if (nd_box_front(b2, b1))
    return 1.0;

  /* b2 is partially to the front of b1 */
  delta = b1->min[Z_DIM] - b2->min[Z_DIM];
  width = b2->max[Z_DIM] - b2->min[Z_DIM];
  return delta / width;
}

/**
 * Return the proportion of b2 that is overfront of b1.
 */
static double
nd_box_ratio_overfront(const ND_BOX *b1, const ND_BOX *b2)
{
  double delta, width;

  if (nd_box_back(b2, b1))
    return 0.0;
  else if (nd_box_overfront(b2, b1))
    return 1.0;

  /* b2 is partially to the back of b1 */
  delta = b2->max[Z_DIM] - b1->max[Z_DIM];
  width = b2->max[Z_DIM] - b2->min[Z_DIM];
  return delta / width;
}

/**
 * Return the proportion of b2 that is back of b1.
 */
static double
nd_box_ratio_back(const ND_BOX *b1, const ND_BOX *b2)
{
  double delta, width;

  if (nd_box_overfront(b2, b1))
    return 0.0;
  else if (nd_box_back(b2, b1))
    return 1.0;

  /* b2 is partially to the back of b1 */
  delta = b2->max[Z_DIM] - b1->max[Z_DIM];
  width = b2->max[Z_DIM] - b2->min[Z_DIM];
  return delta / width;
}

/**
 * Return the proportion of b2 that is overback of b1.
 */
static double
nd_box_ratio_overback(const ND_BOX *b1, const ND_BOX *b2)
{
  double delta, width;

  if (nd_box_front(b2, b1))
    return 0.0;
  else if (nd_box_overback(b2, b1))
    return 1.0;

  /* b2 is partially to the front of b1 */
  delta = b1->min[Z_DIM] - b2->min[Z_DIM];
  width = b2->max[Z_DIM] - b2->min[Z_DIM];
  return delta / width;
}

/**
 * Dispatch function for the position operators
 */
static double
nd_box_ratio_position(const ND_BOX *b1, const ND_BOX *b2, CachedOp op)
{
  if (op == LEFT_OP)
    return nd_box_ratio_left(b1, b2);
  else if (op == OVERLEFT_OP)
    return nd_box_ratio_overleft(b1, b2);
  else if (op == RIGHT_OP)
    return nd_box_ratio_right(b1, b2);
  else if (op == OVERRIGHT_OP)
    return nd_box_ratio_overright(b1, b2);
  else if (op == BELOW_OP)
    return nd_box_ratio_below(b1, b2);
  else if (op == OVERBELOW_OP)
    return nd_box_ratio_overbelow(b1, b2);
  else if (op == ABOVE_OP)
    return nd_box_ratio_above(b1, b2);
  else if (op == OVERABOVE_OP)
    return nd_box_ratio_overabove(b1, b2);
  else if (op == FRONT_OP)
    return nd_box_ratio_front(b1, b2);
  else if (op == OVERFRONT_OP)
    return nd_box_ratio_overfront(b1, b2);
  else if (op == BACK_OP)
    return nd_box_ratio_back(b1, b2);
  else if (op == OVERBACK_OP)
    return nd_box_ratio_overback(b1, b2);
  return FALLBACK_ND_SEL; /* make compiler quiet */
}

/*****************************************************************************
 * Internal functions computing selectivity
 *****************************************************************************/

/**
 * Transform the constant into an STBOX
 *
 * @note This function is also used for temporal network points
 */
static bool
tpoint_const_stbox(Node *other, STBOX *box)
{
  Oid consttypid = ((Const *) other)->consttype;
  mobdbType type = oid_type(consttypid);
  if (tgeo_basetype(type))
    geo_set_stbox((GSERIALIZED *) PointerGetDatum(((Const *) other)->constvalue),
      box);
  else if (type == T_TIMESTAMPTZ)
    timestamp_set_stbox(DatumGetTimestampTz(((Const *) other)->constvalue), box);
  else if (type == T_TIMESTAMPSET)
    timestampset_stbox_slice(((Const *) other)->constvalue, box);
  else if (type == T_PERIOD)
    period_set_stbox(DatumGetSpanP(((Const *) other)->constvalue), box);
  else if (type == T_PERIODSET)
    periodset_stbox_slice(((Const *) other)->constvalue, box);
  else if (type == T_STBOX)
    memcpy(box, DatumGetSTboxP(((Const *) other)->constvalue), sizeof(STBOX));
  else if (tspatial_type(type))
    temporal_set_bbox(DatumGetTemporalP(((Const *) other)->constvalue), box);
  else
    return false;
  return true;
}

/**
 * Get the enum value associated to the operator
 */
static bool
tpoint_cachedop(Oid operid, CachedOp *cachedOp)
{
  for (int i = OVERLAPS_OP; i <= OVERAFTER_OP; i++)
  {
    if (operid == oper_oid((CachedOp) i, T_GEOMETRY, T_TGEOMPOINT) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_TGEOMPOINT) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_TGEOMPOINT) ||
        operid == oper_oid((CachedOp) i, T_PERIOD, T_TGEOMPOINT) ||
        operid == oper_oid((CachedOp) i, T_PERIODSET, T_TGEOMPOINT) ||
        operid == oper_oid((CachedOp) i, T_STBOX, T_TGEOMPOINT) ||
        operid == oper_oid((CachedOp) i, T_TGEOMPOINT, T_GEOMETRY) ||
        operid == oper_oid((CachedOp) i, T_TGEOMPOINT, T_TIMESTAMPTZ) ||
        operid == oper_oid((CachedOp) i, T_TGEOMPOINT, T_TIMESTAMPSET) ||
        operid == oper_oid((CachedOp) i, T_TGEOMPOINT, T_PERIOD) ||
        operid == oper_oid((CachedOp) i, T_TGEOMPOINT, T_PERIODSET) ||
        operid == oper_oid((CachedOp) i, T_TGEOMPOINT, T_STBOX) ||
        operid == oper_oid((CachedOp) i, T_TGEOMPOINT, T_TGEOMPOINT) ||

        operid == oper_oid((CachedOp) i, T_GEOGRAPHY, T_TGEOGPOINT) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_TGEOGPOINT) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_TGEOGPOINT) ||
        operid == oper_oid((CachedOp) i, T_PERIOD, T_TGEOGPOINT) ||
        operid == oper_oid((CachedOp) i, T_PERIODSET, T_TGEOGPOINT) ||
        operid == oper_oid((CachedOp) i, T_STBOX, T_TGEOGPOINT) ||
        operid == oper_oid((CachedOp) i, T_TGEOGPOINT, T_GEOGRAPHY) ||
        operid == oper_oid((CachedOp) i, T_TGEOGPOINT, T_TIMESTAMPTZ) ||
        operid == oper_oid((CachedOp) i, T_TGEOGPOINT, T_TIMESTAMPSET) ||
        operid == oper_oid((CachedOp) i, T_TGEOGPOINT, T_PERIOD) ||
        operid == oper_oid((CachedOp) i, T_TGEOGPOINT, T_PERIODSET) ||
        operid == oper_oid((CachedOp) i, T_TGEOGPOINT, T_STBOX) ||
        operid == oper_oid((CachedOp) i, T_TGEOGPOINT, T_TGEOGPOINT))
      {
        *cachedOp = (CachedOp) i;
        return true;
      }
  }
  return false;
}

/**
 * Set the values of an ND_BOX from an STBOX
 * The function only takes into account the x, y, and z dimensions of the box?
 * and assumes that they exist. This is to be ensured by the calling function.
 */
static void
nd_box_from_stbox(const STBOX *box, ND_BOX *nd_box)
{
  int d = 0;

  nd_box_init(nd_box);
  nd_box->min[d] = (float4) box->xmin;
  nd_box->max[d] = (float4) box->xmax;
  d++;
  nd_box->min[d] = (float4) box->ymin;
  nd_box->max[d] = (float4) box->ymax;
  d++;
  if (MOBDB_FLAGS_GET_GEODETIC(box->flags) ||
    MOBDB_FLAGS_GET_Z(box->flags))
  {
    nd_box->min[d] = (float4) box->zmin;
    nd_box->max[d] = (float4) box->zmax;
  }
  return;
}

/**
 * Return an estimate of the selectivity of a spatiotemporal search box by
 * looking at data in the ND_STATS structure. The selectivity is a float in
 * [0,1] that estimates the proportion of the rows in the table that will be
 * returned as a result of the search box.
 *
 * To get our estimate, we need sum up the values * the proportion of each
 * cell in the histogram that satisfies the operator wrt the search box,
 * then divide by the number of features that generated the histogram.
 *
 * This function generalizes PostGIS function estimate_selectivity in file
 * gserialized_estimate.c
 */
static float8
geo_sel(VariableStatData *vardata, const STBOX *box, CachedOp op)
{
  ND_STATS *nd_stats;
  AttStatsSlot sslot;
  int d; /* counter */
  float8 selec;
  ND_BOX nd_box;
  ND_IBOX nd_ibox, search_ibox;
  int at[ND_DIMS];
  double cell_size[ND_DIMS];
  double min[ND_DIMS];
  double max[ND_DIMS];
  double total_count = 0.0;
  int ndims;
  /*
   * The statistics currently collected by PostGIS does not allow us to
   * differentiate between the bounding box operators for computing the
   * selectivity.
   */
  bool bboxop = (op == OVERLAPS_OP || op == CONTAINS_OP ||
    op == CONTAINED_OP || op == SAME_OP);

  /* Get statistics
   * Currently PostGIS does not set the associated staopN so we
   * can pass InvalidOid */
  if (! (HeapTupleIsValid(vardata->statsTuple) &&
      get_attstatsslot(&sslot, vardata->statsTuple, STATISTIC_KIND_ND,
      InvalidOid, ATTSTATSSLOT_NUMBERS)))
    return -1;

  /* Clone the stats here so we can release the attstatsslot immediately */
  nd_stats = palloc(sizeof(float4) * sslot.nnumbers);
  memcpy(nd_stats, sslot.numbers, sizeof(float4) * sslot.nnumbers);
  free_attstatsslot(&sslot);

  /* Calculate the number of common coordinate dimensions  on the histogram */
  ndims = (int) Min(nd_stats->ndims, MOBDB_FLAGS_GET_Z(box->flags) ? 3 : 2);

  /* Initialize nd_box. */
  nd_box_from_stbox(box, &nd_box);

  /* Full histogram extent op box is false? */
  if (bboxop)
  {
     if(! nd_box_intersects(&(nd_stats->extent), &nd_box, ndims))
      return 0.0;
  }
  else
  {
    if ((op == LEFT_OP && nd_box_overright(&(nd_stats->extent), &nd_box)) ||
      (op == OVERLEFT_OP && nd_box_right(&(nd_stats->extent), &nd_box)) ||
      (op == RIGHT_OP && nd_box_overleft(&(nd_stats->extent), &nd_box)) ||
      (op == OVERRIGHT_OP && nd_box_left(&(nd_stats->extent), &nd_box)) ||

      (op == BELOW_OP && nd_box_overabove(&(nd_stats->extent), &nd_box)) ||
      (op == OVERBELOW_OP && nd_box_above(&(nd_stats->extent), &nd_box)) ||
      (op == ABOVE_OP && nd_box_overbelow(&(nd_stats->extent), &nd_box)) ||
      (op == OVERABOVE_OP && nd_box_below(&(nd_stats->extent), &nd_box)) ||

      (op == FRONT_OP && nd_box_overback(&(nd_stats->extent), &nd_box)) ||
      (op == OVERFRONT_OP && nd_box_back(&(nd_stats->extent), &nd_box)) ||
      (op == BACK_OP && nd_box_overfront(&(nd_stats->extent), &nd_box)) ||
      (op == OVERBACK_OP && nd_box_front(&(nd_stats->extent), &nd_box)))
      return 0.0;
  }

  /* Full histogram extent op box is true? */
  if (bboxop)
  {
     if(nd_box_contains(&nd_box, &(nd_stats->extent), ndims))
      return 1.0;
  }
  else
  {
    if ((op == LEFT_OP && nd_box_left(&(nd_stats->extent), &nd_box)) ||
      (op == OVERLEFT_OP && nd_box_overleft(&(nd_stats->extent), &nd_box)) ||
      (op == RIGHT_OP && nd_box_right(&(nd_stats->extent), &nd_box)) ||
      (op == OVERRIGHT_OP && nd_box_overright(&(nd_stats->extent), &nd_box)) ||

      (op == BELOW_OP && nd_box_below(&(nd_stats->extent), &nd_box)) ||
      (op == OVERBELOW_OP && nd_box_overbelow(&(nd_stats->extent), &nd_box)) ||
      (op == ABOVE_OP && nd_box_above(&(nd_stats->extent), &nd_box)) ||
      (op == OVERABOVE_OP && nd_box_overabove(&(nd_stats->extent), &nd_box)) ||

      (op == FRONT_OP && nd_box_front(&(nd_stats->extent), &nd_box)) ||
      (op == OVERFRONT_OP && nd_box_overfront(&(nd_stats->extent), &nd_box)) ||
      (op == BACK_OP && nd_box_back(&(nd_stats->extent), &nd_box)) ||
      (op == OVERBACK_OP && nd_box_overback(&(nd_stats->extent), &nd_box)))
      return 1.0;
  }

  /* Calculate the overlap of the box on the histogram */
  if (! nd_box_overlap(nd_stats, &nd_box, &nd_ibox))
  {
    return FALLBACK_ND_SEL;
  }

  /* Work out some measurements of the histogram */
  for (d = 0; d < ndims; d++)
  {
    /* Cell size in each dim */
    min[d] = nd_stats->extent.min[d];
    max[d] = nd_stats->extent.max[d];
    cell_size[d] = (max[d] - min[d]) / nd_stats->size[d];
  }

  /* Determine the cells to traverse */
  memset(&search_ibox, 0, sizeof(ND_IBOX));
  if (bboxop)
    /* Traverse only the cells that overlap the box */
    for (d = 0; d < ndims; d++)
    {
      search_ibox.min[d] = nd_ibox.min[d];
      search_ibox.max[d] = nd_ibox.max[d];
    }
  else
  {
    /* Initialize to traverse all the cells */
    for (d = 0; d < nd_stats->ndims; d++)
    {
      search_ibox.min[d] = 0;
      search_ibox.max[d] = (int) (nd_stats->size[d] - 1);
      /* Initialize the counter */
      at[d] = search_ibox.min[d];
    }
    /* Restrict the cells according to the position operator */
    if (op == LEFT_OP)
      search_ibox.max[X_DIM] = nd_ibox.min[X_DIM];
    else if (op == OVERLEFT_OP)
      search_ibox.max[X_DIM] = nd_ibox.max[X_DIM];
    else if (op == RIGHT_OP)
      search_ibox.min[X_DIM] = nd_ibox.max[X_DIM];
    else if (op == OVERRIGHT_OP)
      search_ibox.min[X_DIM] = nd_ibox.min[X_DIM];
    else if (op == BELOW_OP)
      search_ibox.max[Y_DIM] = nd_ibox.min[Y_DIM];
    else if (op == OVERBELOW_OP)
      search_ibox.max[Y_DIM] = nd_ibox.max[Y_DIM];
    else if (op == ABOVE_OP)
      search_ibox.min[Y_DIM] = nd_ibox.max[Y_DIM];
    else if (op == OVERABOVE_OP)
      search_ibox.min[Y_DIM] = nd_ibox.min[Y_DIM];
    else if (op == FRONT_OP)
      search_ibox.max[Z_DIM] = nd_ibox.min[Z_DIM];
    else if (op == OVERFRONT_OP)
      search_ibox.max[Z_DIM] = nd_ibox.max[Z_DIM];
    else if (op == BACK_OP)
      search_ibox.min[Z_DIM] = nd_ibox.max[Z_DIM];
    else if (op == OVERBACK_OP)
      search_ibox.min[Z_DIM] = nd_ibox.min[Z_DIM];
  }

  /* Initialize the counter */
  memset(at, 0, sizeof(int) * ND_DIMS);
  for (d = 0; d < ndims; d++)
    at[d] = search_ibox.min[d];

  /* Move through all the overlap values and sum them */
  do
  {
    float cell_count, ratio;
    ND_BOX nd_cell;
    memset(&nd_cell, 0, sizeof(ND_BOX));

    /* We have to pro-rate partially overlapped cells. */
    for (d = 0; d < ndims; d++)
    {
      nd_cell.min[d] = (float4) (min[d] + (at[d]+0) * cell_size[d]);
      nd_cell.max[d] = (float4) (min[d] + (at[d]+1) * cell_size[d]);
    }

    if (bboxop)
      ratio = (float4) (nd_box_ratio_overlaps(&nd_box, &nd_cell, ndims));
    else
      ratio = (float4) (nd_box_ratio_position(&nd_box, &nd_cell, op));
    cell_count = nd_stats->value[nd_stats_value_index(nd_stats, at)];

    /* Add the pro-rated count for this cell to the overall total */
    total_count += cell_count * ratio;
  }
  while (nd_increment(&search_ibox, (int) ndims, at));

  /* Scale by the number of features in our histogram to get the proportion */
  selec = total_count / nd_stats->histogram_features;

  /* Prevent rounding overflows */
  CLAMP_PROBABILITY(selec);

  return selec;
}

/*****************************************************************************
 * Restriction selectivity
 *****************************************************************************/

/**
 * Return a default restriction selectivity estimate for a given operator,
 * when we don't have statistics or cannot use them for some reason.
 */
static float8
tpoint_sel_default(CachedOp oper)
{
  switch (oper)
  {
    case OVERLAPS_OP:
      return 0.005;

    case CONTAINS_OP:
    case CONTAINED_OP:
      return 0.002;

    case SAME_OP:
      return 0.001;

    case LEFT_OP:
    case RIGHT_OP:
    case OVERLEFT_OP:
    case OVERRIGHT_OP:
    case ABOVE_OP:
    case BELOW_OP:
    case OVERABOVE_OP:
    case OVERBELOW_OP:
    case FRONT_OP:
    case BACK_OP:
    case OVERFRONT_OP:
    case OVERBACK_OP:
    case AFTER_OP:
    case BEFORE_OP:
    case OVERAFTER_OP:
    case OVERBEFORE_OP:
      /* these are similar to regular scalar inequalities */
      return DEFAULT_INEQ_SEL;

    default:
      /* all operators should be handled above, but just in case */
      return 0.001;
  }
}

/**
 * Get enumeration value associated to the operator according to the family
 */
static bool
tpoint_cachedop_family(Oid operid, CachedOp *cachedOp, TemporalFamily tempfamily)
{
#if NPOINT
  assert(tempfamily == TPOINTTYPE || tempfamily == TNPOINTTYPE);
  if (tempfamily == TPOINTTYPE)
    return tpoint_cachedop(operid, cachedOp);
  else /* tempfamily == TNPOINTTYPE */
    return tnpoint_cachedop(operid, cachedOp);
#else
  assert(tempfamily == TPOINTTYPE);
  return tpoint_cachedop(operid, cachedOp);
#endif /* NPOINT */
}

/**
 * Estimate the restriction selectivity of the operators for temporal points
 * (internal function)
 */
float8
tpoint_sel(PlannerInfo *root, Oid operid, List *args, int varRelid,
  TemporalFamily tempfamily)
{
  VariableStatData vardata;
  Node *other;
  bool varonleft;
  Selectivity selec;
  STBOX box;
  Period period;

  /* Get enumeration value associated to the operator */
  CachedOp cachedOp;
  if (! tpoint_cachedop_family(operid, &cachedOp, tempfamily))
    /* In the case of unknown operator */
    return DEFAULT_TEMP_SEL;

  /*
   * If expression is not (variable op something) or (something op
   * variable), then punt and return a default estimate.
   */
  if (! get_restriction_variable(root, args, varRelid, &vardata, &other,
      &varonleft))
    return tpoint_sel_default(cachedOp);

  /*
   * Can't do anything useful if the something is not a constant, either.
   */
  if (! IsA(other, Const))
  {
    ReleaseVariableStats(vardata);
    return tpoint_sel_default(cachedOp);
  }

  /*
   * All the period operators are strict, so we can cope with a NULL constant
   * right away.
   */
  if (((Const *) other)->constisnull)
  {
    ReleaseVariableStats(vardata);
    return 0.0;
  }

  /*
   * If var is on the right, commute the operator, so that we can assume the
   * var is on the left in what follows.
   */
  if (! varonleft)
  {
    /* we have other Op var, commute to make var Op other */
    operid = get_commutator(operid);
    if (! operid)
    {
      /* Use default selectivity (should we raise an error instead?) */
      ReleaseVariableStats(vardata);
      return tpoint_sel_default(cachedOp);
    }
  }

  /*
   * Transform the constant into an STBOX
   */
  if (! tpoint_const_stbox(other, &box))
    /* In the case of unknown constant */
    return tpoint_sel_default(cachedOp);

  assert(MOBDB_FLAGS_GET_X(box.flags) || MOBDB_FLAGS_GET_T(box.flags));

  /* Enable the multiplication of the selectivity of the spatial and time
   * dimensions since either may be missing */
  selec = 1.0;

  /*
   * Estimate selectivity for the spatial dimension
   */
  if (MOBDB_FLAGS_GET_X(box.flags))
  {
    /* PostGIS does not provide selectivity for the traditional
     * comparisons <, <=, >, >= */
    if (cachedOp == LT_OP || cachedOp == LE_OP || cachedOp == GT_OP ||
      cachedOp == GE_OP)
      selec *= tpoint_sel_default(cachedOp);
    else
      selec *= geo_sel(&vardata, &box, cachedOp);
  }
  /*
   * Estimate selectivity for the time dimension
   */
  if (MOBDB_FLAGS_GET_T(box.flags))
  {
    /* Transform the STBOX into a Period */
    memcpy(&period, &box.period, sizeof(Span));

    /* Compute the selectivity */
    selec *= temporal_sel_period(&vardata, &period, cachedOp);
  }

  ReleaseVariableStats(vardata);
  CLAMP_PROBABILITY(selec);
  return selec;
}

PG_FUNCTION_INFO_V1(Tpoint_sel);
/**
 * Estimate the restriction selectivity of the operators for temporal points
 */
PGDLLEXPORT Datum
Tpoint_sel(PG_FUNCTION_ARGS)
{
  return temporal_sel_ext(fcinfo, TPOINTTYPE);
}

/*****************************************************************************
 * Join selectivity
 *****************************************************************************/

static ND_STATS *
pg_nd_stats_from_tuple(HeapTuple stats_tuple, int mode)
{
  int stats_kind = STATISTIC_KIND_ND;
  int rv;
  ND_STATS *nd_stats;

  /* If we're in 2D mode, set the kind appropriately */
  if ( mode == 2 )
    stats_kind = STATISTIC_KIND_2D;

  /* Then read the geom status histogram from that */
  AttStatsSlot sslot;
  rv = get_attstatsslot(&sslot, stats_tuple, stats_kind, InvalidOid,
             ATTSTATSSLOT_NUMBERS);
  if ( ! rv )
    return NULL;

  /* Clone the stats here so we can release the attstatsslot immediately */
  nd_stats = palloc(sizeof(float4) * sslot.nnumbers);
  memcpy(nd_stats, sslot.numbers, sizeof(float4) * sslot.nnumbers);

  free_attstatsslot(&sslot);

  return nd_stats;
}

/**
* Pull the stats object from the PgSQL system catalogs. Used
* by the selectivity functions and the debugging functions.
*/
static ND_STATS *
pg_get_nd_stats(const Oid tableid, AttrNumber att_num, int mode, bool only_parent)
{
  HeapTuple stats_tuple = NULL;
  ND_STATS *nd_stats;

  /* First pull the stats tuple for the whole tree */
  if ( ! only_parent )
    stats_tuple = SearchSysCache3(STATRELATTINH, ObjectIdGetDatum(tableid), Int16GetDatum(att_num), BoolGetDatum(true));
  /* Fall-back to main table stats only, if not found for whole tree or explicitly ignored */
  if ( only_parent || ! stats_tuple )
    stats_tuple = SearchSysCache3(STATRELATTINH, ObjectIdGetDatum(tableid), Int16GetDatum(att_num), BoolGetDatum(false));
  if ( ! stats_tuple )
    return NULL;

  nd_stats = pg_nd_stats_from_tuple(stats_tuple, mode);
  ReleaseSysCache(stats_tuple);
  return nd_stats;
}

/**
 * Return a default join selectivity estimate for a given operator,
 * when we don't have statistics or cannot use them for some reason.
 */
static float8
tpoint_joinsel_default(CachedOp oper)
{
  switch (oper)
  {
    case OVERLAPS_OP:
      return 0.005;

    case CONTAINS_OP:
    case CONTAINED_OP:
      return 0.002;

    case SAME_OP:
      return 0.001;

    case LEFT_OP:
    case RIGHT_OP:
    case OVERLEFT_OP:
    case OVERRIGHT_OP:
    case ABOVE_OP:
    case BELOW_OP:
    case OVERABOVE_OP:
    case OVERBELOW_OP:
    case FRONT_OP:
    case BACK_OP:
    case OVERFRONT_OP:
    case OVERBACK_OP:
    case AFTER_OP:
    case BEFORE_OP:
    case OVERAFTER_OP:
    case OVERBEFORE_OP:
      /* these are similar to regular scalar inequalities */
      return DEFAULT_INEQ_SEL;

    default:
      /* all operators should be handled above, but just in case */
      return 0.001;
  }
}

/**
* Given two statistics histograms, what is the selectivity
* of a join driven by the && operator?
*
* Join selectivity is defined as the number of rows returned by the
* join operator divided by the number of rows that an
* unconstrained join would return (nrows1*nrows2).
*
* To get the estimate of join rows, we walk through the cells
* of one histogram, and multiply the cell value by the
* proportion of the cells in the other histogram the cell
* overlaps: val += val1 * ( val2 * overlap_ratio )
*/
static float8
geo_joinsel(const ND_STATS *s1, const ND_STATS *s2)
{
  int ncells1, ncells2;
  int ndims1, ndims2, ndims;
  double ntuples_max;
  double ntuples_not_null1, ntuples_not_null2;

  ND_BOX extent1, extent2;
  ND_IBOX ibox1, ibox2;
  int at1[ND_DIMS];
  int at2[ND_DIMS];
  double min1[ND_DIMS];
  double width1[ND_DIMS];
  double cellsize1[ND_DIMS];
  int size2[ND_DIMS];
  double min2[ND_DIMS];
  double width2[ND_DIMS];
  double cellsize2[ND_DIMS];
  int size1[ND_DIMS];
  int d;
  double val = 0;
  float8 selectivity;

  /* Drop out on null inputs */
  if ( ! ( s1 && s2 ) )
  {
    elog(NOTICE, " Join selectivity estimation called with null inputs");
    return FALLBACK_ND_SEL;
  }

  /* We need to know how many cells each side has... */
  ncells1 = (int) roundf(s1->histogram_cells);
  ncells2 = (int) roundf(s2->histogram_cells);

  /* ...so that we can drive the summation loop with the smaller histogram. */
  if ( ncells1 > ncells2 )
  {
    const ND_STATS *stats_tmp = s1;
    s1 = s2;
    s2 = stats_tmp;
  }

  /* Re-read that info after the swap */
  // ncells1 = (int) roundf(s1->histogram_cells);
  // ncells2 = (int) roundf(s2->histogram_cells);

  /* Q: What's the largest possible join size these relations can create? */
  /* A: The product of the # of non-null rows in each relation. */
  ntuples_not_null1 = s1->table_features * (s1->not_null_features / s1->sample_features);
  ntuples_not_null2 = s2->table_features * (s2->not_null_features / s2->sample_features);
  ntuples_max = ntuples_not_null1 * ntuples_not_null2;

  /* Get the ndims as ints */
  ndims1 = (int) roundf(s1->ndims);
  ndims2 = (int) roundf(s2->ndims);
  ndims = Max(ndims1, ndims2);

  /* Get the extents */
  extent1 = s1->extent;
  extent2 = s2->extent;

  /* If relation stats do not intersect, join is very very selective. */
  if ( ! nd_box_intersects(&extent1, &extent2, ndims) )
    return 0.0;

  /*
   * First find the index range of the part of the smaller
   * histogram that overlaps the larger one.
   */
  if ( ! nd_box_overlap(s1, &extent2, &ibox1) )
    return FALLBACK_ND_JOINSEL;

  /* Initialize counters / constants on s1 */
  for ( d = 0; d < ndims1; d++ )
  {
    at1[d] = ibox1.min[d];
    min1[d] = s1->extent.min[d];
    width1[d] = s1->extent.max[d] - s1->extent.min[d];
    size1[d] = (int)roundf(s1->size[d]);
    cellsize1[d] = width1[d] / size1[d];
  }

  /* Initialize counters / constants on s2 */
  for ( d = 0; d < ndims2; d++ )
  {
    min2[d] = s2->extent.min[d];
    width2[d] = s2->extent.max[d] - s2->extent.min[d];
    size2[d] = (int)roundf(s2->size[d]);
    cellsize2[d] = width2[d] / size2[d];
  }

  /* For each affected cell of s1... */
  do
  {
    double val1;
    /* Construct the bounds of this cell */
    ND_BOX nd_cell1;
    nd_box_init(&nd_cell1);
    for ( d = 0; d < ndims1; d++ )
    {
      nd_cell1.min[d] = min1[d] + (at1[d]+0) * cellsize1[d];
      nd_cell1.max[d] = min1[d] + (at1[d]+1) * cellsize1[d];
    }

    /* Find the cells of s2 that cell1 overlaps.. */
    nd_box_overlap(s2, &nd_cell1, &ibox2);

    /* Initialize counter */
    for ( d = 0; d < ndims2; d++ )
    {
      at2[d] = ibox2.min[d];
    }

    /* Get the value at this cell */
    val1 = s1->value[nd_stats_value_index(s1, at1)];

    /* For each overlapped cell of s2... */
    do
    {
      double ratio2;
      double val2;

      /* Construct the bounds of this cell */
      ND_BOX nd_cell2;
      nd_box_init(&nd_cell2);
      for ( d = 0; d < ndims2; d++ )
      {
        nd_cell2.min[d] = min2[d] + (at2[d]+0) * cellsize2[d];
        nd_cell2.max[d] = min2[d] + (at2[d]+1) * cellsize2[d];
      }

      /* Calculate overlap ratio of the cells */
      ratio2 = nd_box_ratio_overlaps(&nd_cell1, &nd_cell2, Max(ndims1, ndims2));

      /* Multiply the cell counts, scaled by overlap ratio */
      val2 = s2->value[nd_stats_value_index(s2, at2)];
      val += val1 * (val2 * ratio2);
    }
    while ( nd_increment(&ibox2, ndims2, at2) );

  }
  while( nd_increment(&ibox1, ndims1, at1) );

  /*
   * In order to compare our total cell count "val" to the
   * ntuples_max, we need to scale val up to reflect a full
   * table estimate. So, multiply by ratio of table size to
   * sample size.
   */
  val *= (s1->table_features / s1->sample_features);
  val *= (s2->table_features / s2->sample_features);

  /*
   * Because the cell counts are over-determined due to
   * double counting of features that overlap multiple cells
   * (see the compute_gserialized_stats routine)
   * we also have to scale our cell count "val" *down*
   * to adjust for the double counting.
   */
//  val /= (s1->cells_covered / s1->histogram_features);
//  val /= (s2->cells_covered / s2->histogram_features);

  /*
   * Finally, the selectivity is the estimated number of
   * rows to be returned divided by the maximum possible
   * number of rows that can be returned.
   */
  selectivity = val / ntuples_max;

  /* Guard against over-estimates and crazy numbers :) */
  if ( isnan(selectivity) || ! isfinite(selectivity) || selectivity < 0.0 )
    selectivity = DEFAULT_ND_JOINSEL;
  else if ( selectivity > 1.0 )
    selectivity = 1.0;

  return selectivity;
}

/**
 * Depending on the operator and the arguments, determine wheter the space,
 * the time, or both components are taken into account for computing the
 * join selectivity
 */
static bool
tpoint_joinsel_components(CachedOp cachedOp, mobdbType oprleft,
  mobdbType oprright, bool *space, bool *time)
{
  /* Get the argument which may not be a temporal point */
  mobdbType arg = tspatial_type(oprleft) ? oprright : oprleft;

  /* Determine the components */
  if (tspatial_basetype(arg) ||
    cachedOp == LEFT_OP || cachedOp == OVERLEFT_OP ||
    cachedOp == RIGHT_OP || cachedOp == OVERRIGHT_OP ||
    cachedOp == BELOW_OP || cachedOp == OVERBELOW_OP ||
    cachedOp == ABOVE_OP || cachedOp == OVERABOVE_OP ||
    cachedOp == FRONT_OP || cachedOp == OVERFRONT_OP ||
    cachedOp == BACK_OP || cachedOp == OVERBACK_OP)
  {
    *space = true;
    *time = false;
  }
  else if (time_type(arg) ||
    cachedOp == BEFORE_OP || cachedOp == OVERBEFORE_OP ||
    cachedOp == AFTER_OP || cachedOp == OVERAFTER_OP)
  {
    *space = false;
    *time = true;
  }
  else if (tspatial_type(arg) && (cachedOp == OVERLAPS_OP ||
    cachedOp == CONTAINS_OP || cachedOp == CONTAINED_OP ||
    cachedOp == SAME_OP || cachedOp == ADJACENT_OP))
  {
    *space = true;
    *time = true;
  }
  else
  {
    /* By default only the time component is taken into account */
    *space = false;
    *time = true;
  }
  return true;
}

/**
 * Estimate the restriction selectivity of the operators for temporal (network)
 * points (internal function)
 */
float8
tpoint_joinsel(PlannerInfo *root, Oid operid, List *args,
  JoinType jointype, SpecialJoinInfo *sjinfo, int mode,
  TemporalFamily tempfamily)
{
  Node *arg1 = (Node *) linitial(args);
  Node *arg2 = (Node *) lsecond(args);
  Var *var1 = (Var *) arg1;
  Var *var2 = (Var *) arg2;

  /* We only do column joins right now, no functional joins */
  /* TODO: handle t1 <op> expandX(t2) */
  if (!IsA(arg1, Var) || !IsA(arg2, Var))
    return DEFAULT_TEMP_JOINSEL;

  /* Get enumeration value associated to the operator */
  CachedOp cachedOp;
  if (! tpoint_cachedop_family(operid, &cachedOp, tempfamily))
    /* In the case of unknown operator */
    return DEFAULT_TEMP_SEL;

  /*
   * Determine whether the space and/or the time components are
   * taken into account for the selectivity estimation
   */
  mobdbType oprleft = oid_type(var1->vartype);
  mobdbType oprright = oid_type(var2->vartype);
  bool space, time;
  if (! tpoint_joinsel_components(cachedOp, oprleft, oprright,
    &space, &time))
    /* In the case of unknown arguments */
    return tpoint_joinsel_default(cachedOp);

  float8 selec = 1.0;
  if (space)
  {
    /* What are the Oids of our tables/relations? */
    Oid relid1 = rt_fetch(var1->varno, root->parse->rtable)->relid;
    Oid relid2 = rt_fetch(var2->varno, root->parse->rtable)->relid;

    /* Pull the stats from the stats system. */
    ND_STATS *stats1 = pg_get_nd_stats(relid1, var1->varattno, mode, false);
    ND_STATS *stats2 = pg_get_nd_stats(relid2, var2->varattno, mode, false);

    /* If we can't get stats, we have to stop here! */
    if (! stats1 || ! stats2)
      selec *= tpoint_joinsel_default(cachedOp);
    else
      selec *= geo_joinsel(stats1, stats2);
    if (stats1)
      pfree(stats1);
    if (stats2)
      pfree(stats2);
  }
  if (time)
  {
    /*
     * Return default selectivity for the time dimension for the following cases
     * - There is no ~= operator for time types
     * - The support functions for the ever spatial relationships add a
     *   bounding box test with the && operator, but we need to exclude
     *   the dwithin operator since it takes 3 arguments and thus the
     *   PostgreSQL function get_join_variables cannot be invoked.
     */
    if (cachedOp == SAME_OP ||
      (cachedOp == OVERLAPS_OP && list_length(args) != 2))
      selec *= span_joinsel_default(cachedOp);
    else
      /* Estimate join selectivity */
      selec *= span_joinsel(root, cachedOp, args, jointype, sjinfo);
  }
  return selec;
}

PG_FUNCTION_INFO_V1(Tpoint_joinsel);
/**
 * Estimate the join selectivity value of the operators for temporal points.
 *
 * The selectivity is the ratio of the number of rows we think will be
 * returned divided the maximum number of rows the join could possibly
 * return (the full combinatoric join), that is,
 *   joinsel = estimated_nrows / (totalrows1 * totalrows2)
 */
PGDLLEXPORT Datum
Tpoint_joinsel(PG_FUNCTION_ARGS)
{
  return temporal_joinsel_ext(fcinfo, TPOINTTYPE);
}

/*****************************************************************************/
