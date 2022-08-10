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
 * @brief SP-GiST implementation of 8-dimensional quad-tree over temporal
 * points.
 *
 * This module provides SP-GiST implementation for boxes using an oct-tree
 * analogy in 8-dimensional space. SP-GiST doesn't allow indexing of
 * overlapping objects. We are making 4D objects never-overlapping in
 * 8D space.  This technique has some benefits compared to traditional
 * R-Tree which is implemented as GiST. The performance tests reveal
 * that this technique especially beneficial with too much overlapping
 * objects, so called "spaghetti data".
 *
 * Unlike the original oct-tree, we are splitting the tree into 256
 * quadrants in 8D space.  It is easier to imagine it as splitting space
 * four times into four:
 * @code
 *              |      |                        |      |
 *              |      |                        |      |
 *              | -----+-----                   | -----+-----
 *              |      |                        |      |
 *              |      |                        |      |
 * -------------+------------- -+- -------------+-------------
 *              |                               |
 *              |                               |
 *              |                               |
 *              |                               |
 *              |                               |
 *            FRONT                           BACK
 * @endcode
 * We are using STBOX data type as the prefix, but we are treating them
 * as points in 8-dimensional space, because 4D boxes are not enough
 * to represent the quadrant boundaries in 8D space.  They however are
 * sufficient to point out the additional boundaries of the next
 * quadrant.
 *
 * We are using traversal values provided by SP-GiST to calculate and
 * to store the bounds of the quadrants, while traversing into the tree.
 * Traversal value has all the boundaries in the 8D space, and is is
 * capable of transferring the required boundaries to the following
 * traversal values.  In conclusion, three things are necessary
 * to calculate the next traversal value:
 *
 *  1. the traversal value of the parent
 *  2. the quadrant of the current node
 *  3. the prefix of the current node
 *
 * If we visualize them on our simplified drawing (see the drawing above);
 * transferred boundaries of (1) would be the outer axis, relevant part
 * of (2) would be the up range_y part of the other axis, and (3) would be
 * the inner axis.
 *
 * For example, consider the case of overlapping.  When recursion
 * descends deeper and deeper down the tree, all quadrants in
 * the current node will be checked for overlapping.  The boundaries
 * will be re-calculated for all quadrants.  Overlap check answers
 * the question: can any box from this quadrant overlap with the given
 * box?  If yes, then this quadrant will be walked.  If no, then this
 * quadrant will be skipped.
 *
 * This method provides restrictions for minimum and maximum values of
 * every dimension of every corner of the box on every level of the tree
 * except the root.  For the root node, we are setting the boundaries
 * that we don't yet have as infinity.
 */

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include <access/spgist.h>
#include <access/spgist_private.h>
#include <utils/float.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "point/tpoint_boxops.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/temporal_catalog.h"
#include "pg_general/tnumber_spgist.h"
#include "pg_point/tpoint_gist.h"

/*****************************************************************************
 * Data structures
 *****************************************************************************/

/**
 * Structure to represent the bounding box of a temporal point as a 6- or
 * 8-dimensional point depending on whether the temporal point is in 2D+T or 3D+T.
 */
typedef struct
{
  STBOX left;
  STBOX right;
} STboxNode;

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * Copy a STboxNode
 */
STboxNode *
cubestbox_copy(const STboxNode *box)
{
  STboxNode *result = palloc(sizeof(STboxNode));
  memcpy(result, box, sizeof(STboxNode));
  return result;
}

/**
 * Calculate the quadrant
 *
 * The quadrant is 8 bit unsigned integer with all bits in use.
 * This function accepts 2 STBOX as input.  All 8 bits are set by comparing a
 * corner of the box. This makes 256 quadrants in total.
 */
static uint8
getOctant8D(const STBOX *centroid, const STBOX *inBox)
{
  uint8 quadrant = 0;

  if (MOBDB_FLAGS_GET_Z(centroid->flags))
  {
    if (inBox->zmin > centroid->zmin)
      quadrant |= 0x80;

    if (inBox->zmax > centroid->zmax)
      quadrant |= 0x40;
  }

  if (inBox->ymin > centroid->ymin)
    quadrant |= 0x20;

  if (inBox->ymax > centroid->ymax)
    quadrant |= 0x10;

  if (inBox->xmin > centroid->xmin)
    quadrant |= 0x08;

  if (inBox->xmax > centroid->xmax)
    quadrant |= 0x04;

  if (datum_gt(inBox->period.lower, centroid->period.lower, T_TIMESTAMPTZ))
    quadrant |= 0x02;

  if (datum_gt(inBox->period.upper, centroid->period.upper, T_TIMESTAMPTZ))
    quadrant |= 0x01;

  return quadrant;
}

/**
 * Initialize the traversal value
 *
 * In the beginning, we don't have any restrictions.  We have to
 * initialize the struct to cover the whole 8D space.
 */
static void
stboxnode_init(const STBOX *centroid, STboxNode *nodebox)
{
  memset(nodebox, 0, sizeof(STboxNode));
  double infinity = get_float8_infinity();

  nodebox->left.xmin = nodebox->right.xmin = -infinity;
  nodebox->left.xmax = nodebox->right.xmax = infinity;

  nodebox->left.ymin = nodebox->right.ymin = -infinity;
  nodebox->left.ymax = nodebox->right.ymax = infinity;

  nodebox->left.zmin = nodebox->right.zmin = -infinity;
  nodebox->left.zmax = nodebox->right.zmax = infinity;

  nodebox->left.period.lower = nodebox->right.period.lower =
    TimestampTzGetDatum(DT_NOBEGIN);
  nodebox->left.period.upper = nodebox->right.period.upper =
    TimestampTzGetDatum(DT_NOEND);

  nodebox->left.srid = nodebox->right.srid = centroid->srid;
  nodebox->left.flags = nodebox->right.flags = centroid->flags;

  return;
}

/**
 * Calculate the next traversal value
 *
 * All centroids are bounded by STboxNode, but SP-GiST only keeps
 * boxes. When we are traversing the tree, we must calculate STboxNode,
 * using centroid and quadrant.
 */
static void
stboxnode_quadtree_next(const STboxNode *nodebox, const STBOX *centroid,
  uint8 quadrant, STboxNode *next_nodebox)
{
  memcpy(next_nodebox, nodebox, sizeof(STboxNode));

  if (MOBDB_FLAGS_GET_Z(centroid->flags))
  {
    if (quadrant & 0x80)
      next_nodebox->left.zmin = centroid->zmin;
    else
      next_nodebox->left.zmax = centroid->zmin;

    if (quadrant & 0x40)
      next_nodebox->right.zmin = centroid->zmax;
    else
      next_nodebox->right.zmax = centroid->zmax;
  }

  if (quadrant & 0x20)
    next_nodebox->left.ymin = centroid->ymin;
  else
    next_nodebox->left.ymax = centroid->ymin;

  if (quadrant & 0x10)
    next_nodebox->right.ymin = centroid->ymax;
  else
    next_nodebox->right.ymax = centroid->ymax;

  if (quadrant & 0x08)
    next_nodebox->left.xmin = centroid->xmin;
  else
    next_nodebox->left.xmax = centroid->xmin;

  if (quadrant & 0x04)
    next_nodebox->right.xmin = centroid->xmax;
  else
    next_nodebox->right.xmax = centroid->xmax;

  if (quadrant & 0x02)
    next_nodebox->left.period.lower = centroid->period.lower;
  else
    next_nodebox->left.period.upper = centroid->period.lower;

  if (quadrant & 0x01)
    next_nodebox->right.period.lower = centroid->period.upper;
  else
    next_nodebox->right.period.upper = centroid->period.upper;

  return;
}

/**
 * Can any box from nodebox overlap with query?
 */
static bool
overlap8D(const STboxNode *nodebox, const STBOX *query)
{
  bool result = true;
  /* Result value is computed only for the dimensions of the query */
  if (MOBDB_FLAGS_GET_X(query->flags))
    result &= nodebox->left.xmin <= query->xmax &&
      nodebox->right.xmax >= query->xmin &&
      nodebox->left.ymin <= query->ymax &&
      nodebox->right.ymax >= query->ymin;
  if (MOBDB_FLAGS_GET_Z(query->flags))
    result &= nodebox->left.zmin <= query->zmax &&
      nodebox->right.zmax >= query->zmin;
  if (MOBDB_FLAGS_GET_T(query->flags))
    result &=
      datum_le(nodebox->left.period.lower, query->period.upper, T_TIMESTAMPTZ) &&
      datum_ge(nodebox->right.period.upper, query->period.lower, T_TIMESTAMPTZ);
  return result;
}

/**
 * Can any box from nodebox contain query?
 */
static bool
contain8D(const STboxNode *nodebox, const STBOX *query)
{
  bool result = true;
  /* Result value is computed only for the dimensions of the query */
  if (MOBDB_FLAGS_GET_X(query->flags))
    result &= nodebox->right.xmax >= query->xmax &&
      nodebox->left.xmin <= query->xmin &&
      nodebox->right.ymax >= query->ymax &&
      nodebox->left.ymin <= query->ymin;
  if (MOBDB_FLAGS_GET_Z(query->flags))
    result &= nodebox->right.zmax >= query->zmax &&
      nodebox->left.zmin <= query->zmin;
  if (MOBDB_FLAGS_GET_T(query->flags))
    result &=
      datum_ge(nodebox->right.period.upper, query->period.upper, T_TIMESTAMPTZ) &&
      datum_le(nodebox->left.period.lower, query->period.lower, T_TIMESTAMPTZ);
  return result;
}

/**
 * Can any box from nodebox be left of query?
 */
static bool
left8D(const STboxNode *nodebox, const STBOX *query)
{
  return (nodebox->right.xmax < query->xmin);
}

/**
 * Can any box from nodebox does not extend the right of query?
 */
static bool
overLeft8D(const STboxNode *nodebox, const STBOX *query)
{
  return (nodebox->right.xmax <= query->xmax);
}

/**
 * Can any box from nodebox be right of query?
 */
static bool
right8D(const STboxNode *nodebox, const STBOX *query)
{
  return (nodebox->left.xmin > query->xmax);
}

/**
 * Can any box from nodebox does not extend the left of query?
 */
static bool
overRight8D(const STboxNode *nodebox, const STBOX *query)
{
  return (nodebox->left.xmin >= query->xmin);
}

/**
 * Can any box from nodebox be below of query?
 */
static bool
below8D(const STboxNode *nodebox, const STBOX *query)
{
  return (nodebox->right.ymax < query->ymin);
}

/**
 * Can any box from nodebox does not extend above query?
 */
static bool
overBelow8D(const STboxNode *nodebox, const STBOX *query)
{
  return (nodebox->right.ymax <= query->ymax);
}

/**
 * Can any box from nodebox be above of query?
 */
static bool
above8D(const STboxNode *nodebox, const STBOX *query)
{
  return (nodebox->left.ymin > query->ymax);
}

/**
 * Can any box from nodebox does not extend below of query?
 */
static bool
overAbove8D(const STboxNode *nodebox, const STBOX *query)
{
  return (nodebox->left.ymin >= query->ymin);
}

/**
 * Can any box from nodebox be in front of query?
 */
static bool
front8D(STboxNode *nodebox, STBOX *query)
{
  return (nodebox->right.zmax < query->zmin);
}

/**
 * Can any box from nodebox does not extend the back of query?
 */
static bool
overFront8D(const STboxNode *nodebox, const STBOX *query)
{
  return (nodebox->right.zmax <= query->zmax);
}

/**
 * Can any box from nodebox be back to query?
 */
static bool
back8D(const STboxNode *nodebox, const STBOX *query)
{
  return (nodebox->left.zmin > query->zmax);
}

/**
 * Can any box from nodebox does not extend the front of query?
 */
static bool
overBack8D(const STboxNode *nodebox, const STBOX *query)
{
  return (nodebox->left.zmin >= query->zmin);
}

/**
 * Can any box from nodebox be before of query?
 */
static bool
before8D(const STboxNode *nodebox, const STBOX *query)
{
  return datum_lt(nodebox->right.period.upper, query->period.lower, T_TIMESTAMPTZ);
}

/**
 * Can any box from nodebox does not extend the after of query?
 */
static bool
overBefore8D(const STboxNode *nodebox, const STBOX *query)
{
  return datum_le(nodebox->right.period.upper, query->period.upper, T_TIMESTAMPTZ);
}

/**
 * Can any box from nodebox be after of query?
 */
static bool
after8D(const STboxNode *nodebox, const STBOX *query)
{
  return datum_gt(nodebox->left.period.lower, query->period.upper, T_TIMESTAMPTZ);
}

/**
 * Can any box from nodebox does not extend the before of query?
 */
static bool
overAfter8D(const STboxNode *nodebox, const STBOX *query)
{
  return datum_ge(nodebox->left.period.lower, query->period.lower, T_TIMESTAMPTZ);
}

/**
 * Lower bound for the distance between query and nodebox.
 * @note The temporal dimension is only taken into account for returning
 * +infinity (which will be translated into NULL) if the boxes do not
 * intersect in time. Besides that, it is not possible to mix different
 * units in the computation. As a consequence, the filtering is not very
 * restrictive.
 */
static double
distance_stbox_nodebox(const STBOX *query, const STboxNode *nodebox)
{
  /* The query argument can be an empty geometry */
  if (! MOBDB_FLAGS_GET_X(query->flags))
      return DBL_MAX;

  /* If the boxes do not intersect in the time dimension return infinity */
  bool hast = MOBDB_FLAGS_GET_T(query->flags);
  if (hast && (
      datum_gt(query->period.lower, nodebox->right.period.upper, T_TIMESTAMPTZ) ||
      datum_gt(nodebox->left.period.lower, query->period.upper, T_TIMESTAMPTZ)))
    return DBL_MAX;

  double dx, dy, dz;
  if (query->xmax < nodebox->left.xmin)
    dx = nodebox->left.xmin - query->xmax;
  else if (query->xmin > nodebox->right.xmax)
    dx = query->xmin - nodebox->right.xmax;
  else
    dx = 0;

  if (query->ymax < nodebox->left.ymin)
    dy = nodebox->left.ymin - query->ymax;
  else if (query->ymin > nodebox->right.ymax)
    dy = query->ymin - nodebox->right.ymax;
  else
    dy = 0;

  bool hasz = MOBDB_FLAGS_GET_Z(nodebox->left.flags);
  if (hasz)
  {
    if (query->zmax < nodebox->left.zmin)
      dz = nodebox->left.zmin - query->zmax;
    else if (query->zmin > nodebox->right.zmax)
      dz = query->zmin - nodebox->right.zmax;
    else
      dz = 0;
  }

  return hasz ? hypot3d(dx, dy, dz) : hypot(dx, dy);
}

/**
 * Transform a query argument into an STBOX.
 */
static bool
tpoint_spgist_get_stbox(const ScanKeyData *scankey, STBOX *result)
{
  mobdbType type = oid_type(scankey->sk_subtype);
  if (tgeo_basetype(type))
  {
    GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(scankey->sk_argument);
    /* The geometry can be empty */
    if (! geo_set_stbox(gs, result))
      return false;
  }
  else if (type == T_TIMESTAMPTZ)
  {
    TimestampTz t = DatumGetTimestampTz(scankey->sk_argument);
    timestamp_set_stbox(t, result);
  }
  else if (type == T_TIMESTAMPSET)
  {
    timestampset_stbox_slice(scankey->sk_argument, result);
  }
  else if (type == T_PERIOD)
  {
    Period *p = DatumGetSpanP(scankey->sk_argument);
    period_set_stbox(p, result);
  }
  else if (type == T_PERIODSET)
  {
    periodset_stbox_slice(scankey->sk_argument, result);
  }
  else if (type == T_STBOX)
  {
    memcpy(result, DatumGetSTboxP(scankey->sk_argument), sizeof(STBOX));
  }
  else if (tspatial_type(type))
  {
    temporal_bbox_slice(scankey->sk_argument, result);
  }
  else
    elog(ERROR, "Unsupported type for indexing: %d", type);
  return true;
}

/*****************************************************************************
 * SP-GiST config function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_spgist_config);
/**
 * SP-GiST config function for temporal points
 */
PGDLLEXPORT Datum
Stbox_spgist_config(PG_FUNCTION_ARGS)
{
  spgConfigOut *cfg = (spgConfigOut *) PG_GETARG_POINTER(1);
  Oid stbox_oid = type_oid(T_STBOX);
  cfg->prefixType = stbox_oid;  /* A type represented by its bounding box */
  cfg->labelType = VOIDOID;     /* We don't need node labels. */
  cfg->leafType = stbox_oid;
  cfg->canReturnData = false;
  cfg->longValuesOK = false;
  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST choose function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_quadtree_choose);
/**
 * SP-GiST choose function for temporal points
 */
PGDLLEXPORT Datum
Stbox_quadtree_choose(PG_FUNCTION_ARGS)
{
  spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
  spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
  STBOX *centroid = DatumGetSTboxP(in->prefixDatum),
    *box = DatumGetSTboxP(in->leafDatum);

  out->resultType = spgMatchNode;
  out->result.matchNode.restDatum = PointerGetDatum(box);

  /* nodeN will be set by core, when allTheSame. */
  if (!in->allTheSame)
    out->result.matchNode.nodeN = getOctant8D(centroid, box);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST pick-split function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_quadtree_picksplit);
/**
 * SP-GiST pick-split function for temporal points
 *
 * It splits a list of boxes into quadrants by choosing a central 8D
 * point as the median of the coordinates of the boxes.
 */
PGDLLEXPORT Datum
Stbox_quadtree_picksplit(PG_FUNCTION_ARGS)
{
  spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
  spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
  STBOX *box = DatumGetSTboxP(in->datums[0]);
  bool hasz = MOBDB_FLAGS_GET_Z(box->flags);
  STBOX *centroid = palloc0(sizeof(STBOX));
  centroid->srid = box->srid;
  centroid->flags = box->flags;
  int  median, i;
  double *lowXs = palloc(sizeof(double) * in->nTuples);
  double *highXs = palloc(sizeof(double) * in->nTuples);
  double *lowYs = palloc(sizeof(double) * in->nTuples);
  double *highYs = palloc(sizeof(double) * in->nTuples);
  double *lowZs;
  double *highZs;
  if (hasz)
  {
    lowZs = palloc(sizeof(double) * in->nTuples);
    highZs = palloc(sizeof(double) * in->nTuples);
  }
  double *lowTs = palloc(sizeof(double) * in->nTuples);
  double *highTs = palloc(sizeof(double) * in->nTuples);

  /* Calculate median of all 8D coordinates */
  for (i = 0; i < in->nTuples; i++)
  {
    box = DatumGetSTboxP(in->datums[i]);
    lowXs[i] = box->xmin;
    highXs[i] = box->xmax;
    lowYs[i] = box->ymin;
    highYs[i] = box->ymax;
    if (hasz)
    {
      lowZs[i] = box->zmin;
      highZs[i] = box->zmax;
    }
    lowTs[i] = (double) DatumGetTimestampTz(box->period.lower);
    highTs[i] = (double) DatumGetTimestampTz(box->period.upper);
  }

  qsort(lowXs, (size_t) in->nTuples, sizeof(double), compareDoubles);
  qsort(highXs, (size_t) in->nTuples, sizeof(double), compareDoubles);
  qsort(lowYs, (size_t) in->nTuples, sizeof(double), compareDoubles);
  qsort(highYs, (size_t) in->nTuples, sizeof(double), compareDoubles);
  if (hasz)
  {
    qsort(lowZs, (size_t) in->nTuples, sizeof(double), compareDoubles);
    qsort(highZs, (size_t) in->nTuples, sizeof(double), compareDoubles);
  }
  qsort(lowTs, (size_t) in->nTuples, sizeof(double), compareDoubles);
  qsort(highTs, (size_t) in->nTuples, sizeof(double), compareDoubles);

  median = in->nTuples / 2;

  centroid->xmin = lowXs[median];
  centroid->xmax = highXs[median];
  centroid->ymin = lowYs[median];
  centroid->ymax = highYs[median];
  if (hasz)
  {
    centroid->zmin = lowZs[median];
    centroid->zmax = highZs[median];
  }
  centroid->period.lower = TimestampTzGetDatum((TimestampTz) lowTs[median]);
  centroid->period.upper = TimestampTzGetDatum((TimestampTz) highTs[median]);

  /* Fill the output */
  out->hasPrefix = true;
  out->prefixDatum = STboxPGetDatum(centroid);
  out->nNodes = hasz ? 256 : 128;
  out->nodeLabels = NULL;    /* We don't need node labels. */
  out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
  out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);

  /*
   * Assign ranges to corresponding nodes according to quadrants relative to
   * the "centroid" range
   */
  for (i = 0; i < in->nTuples; i++)
  {
    box = DatumGetSTboxP(in->datums[i]);
    uint8 quadrant = getOctant8D(centroid, box);
    out->leafTupleDatums[i] = STboxPGetDatum(box);
    out->mapTuplesToNodes[i] = quadrant;
  }

  pfree(lowXs); pfree(highXs);
  pfree(lowYs); pfree(highYs);
  if (hasz)
  {
    pfree(lowZs); pfree(highZs);
  }
  pfree(lowTs); pfree(highTs);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST inner consistent functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_quadtree_inner_consistent);
/**
 * SP-GiST inner consistent functions for temporal points
 */
PGDLLEXPORT Datum
Stbox_quadtree_inner_consistent(PG_FUNCTION_ARGS)
{
  spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
  spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
  int i;
  MemoryContext old_ctx;
  STboxNode *nodebox, infbox, next_nodebox;
  uint16 quadrant;
  STBOX *centroid, *queries, *orderbys;

  /* Fetch the centroid of this node. */
  assert(in->hasPrefix);
  centroid = DatumGetSTboxP(in->prefixDatum);

  /*
   * We are saving the traversal value or initialize it an unbounded one, if
   * we have just begun to walk the tree.
   */
  if (in->traversalValue)
    nodebox = in->traversalValue;
  else
  {
    stboxnode_init(centroid, &infbox);
    nodebox = &infbox;
  }

  /*
   * Transform the orderbys into bounding boxes initializing the dimensions
   * that must not be taken into account for the operators to infinity.
   * This transformation is done here to avoid doing it for all quadrants
   * in the loop below.
   */
  if (in->norderbys > 0)
  {
    orderbys = palloc0(sizeof(STBOX) * in->norderbys);
    for (i = 0; i < in->norderbys; i++)
      /* If the argument is an empty geometry the following call will do nothing */
      tpoint_spgist_get_stbox(&in->orderbys[i], &orderbys[i]);
  }

  if (in->allTheSame)
  {
    /* Report that all nodes should be visited */
    out->nNodes = in->nNodes;
    out->nodeNumbers = palloc(sizeof(int) * in->nNodes);
    for (i = 0; i < in->nNodes; i++)
    {
      out->nodeNumbers[i] = i;
      if (in->norderbys > 0)
      {
        /* Use parent quadrant box as traversalValue */
        old_ctx = MemoryContextSwitchTo(in->traversalMemoryContext);
        out->traversalValues[i] = cubestbox_copy(nodebox);
        MemoryContextSwitchTo(old_ctx);

        /* Compute the distances */
        double *distances = palloc0(sizeof(double) * in->norderbys);
        out->distances[i] = distances;
        for (int j = 0; j < in->norderbys; j++)
          distances[j] = distance_stbox_nodebox(&orderbys[i], nodebox);

        pfree(orderbys);
      }
    }

    PG_RETURN_VOID();
  }

  /*
   * Transform the queries into bounding boxes initializing the dimensions
   * that must not be taken into account for the operators to infinity.
   * This transformation is done here to avoid doing it for all quadrants
   * in the loop below.
   */
  if (in->nkeys > 0)
  {
    queries = palloc0(sizeof(STBOX) * in->nkeys);
    for (i = 0; i < in->nkeys; i++)
      /* If the argument is an empty geometry the following call will do nothing */
      tpoint_spgist_get_stbox(&in->scankeys[i], &queries[i]);
  }

  /* Allocate enough memory for nodes */
  out->nNodes = 0;
  out->nodeNumbers = palloc(sizeof(int) * in->nNodes);
  out->traversalValues = palloc(sizeof(void *) * in->nNodes);
  if (in->norderbys > 0)
    out->distances = palloc(sizeof(double *) * in->nNodes);

  for (quadrant = 0; quadrant < in->nNodes; quadrant++)
  {
    stboxnode_quadtree_next(nodebox, centroid, (uint8) quadrant, &next_nodebox);
    bool flag = true;
    for (i = 0; i < in->nkeys; i++)
    {
      StrategyNumber strategy = in->scankeys[i].sk_strategy;
      switch (strategy)
      {
        case RTOverlapStrategyNumber:
        case RTContainedByStrategyNumber:
        case RTAdjacentStrategyNumber:
          flag = overlap8D(&next_nodebox, &queries[i]);
          break;
        case RTContainsStrategyNumber:
        case RTSameStrategyNumber:
          flag = contain8D(&next_nodebox, &queries[i]);
          break;
        case RTLeftStrategyNumber:
          flag = !overRight8D(&next_nodebox, &queries[i]);
          break;
        case RTOverLeftStrategyNumber:
          flag = !right8D(&next_nodebox, &queries[i]);
          break;
        case RTRightStrategyNumber:
          flag = !overLeft8D(&next_nodebox, &queries[i]);
          break;
        case RTOverRightStrategyNumber:
          flag = !left8D(&next_nodebox, &queries[i]);
          break;
        case RTFrontStrategyNumber:
          flag = !overBack8D(&next_nodebox, &queries[i]);
          break;
        case RTOverFrontStrategyNumber:
          flag = !back8D(&next_nodebox, &queries[i]);
          break;
        case RTBackStrategyNumber:
          flag = !overFront8D(&next_nodebox, &queries[i]);
          break;
        case RTOverBackStrategyNumber:
          flag = !front8D(&next_nodebox, &queries[i]);
          break;
        case RTAboveStrategyNumber:
          flag = !overBelow8D(&next_nodebox, &queries[i]);
          break;
        case RTOverAboveStrategyNumber:
          flag = !below8D(&next_nodebox, &queries[i]);
          break;
        case RTBelowStrategyNumber:
          flag = !overAbove8D(&next_nodebox, &queries[i]);
          break;
        case RTOverBelowStrategyNumber:
          flag = !above8D(&next_nodebox, &queries[i]);
          break;
        case RTAfterStrategyNumber:
          flag = !overBefore8D(&next_nodebox, &queries[i]);
          break;
        case RTOverAfterStrategyNumber:
          flag = !before8D(&next_nodebox, &queries[i]);
          break;
        case RTBeforeStrategyNumber:
          flag = !overAfter8D(&next_nodebox, &queries[i]);
          break;
        case RTOverBeforeStrategyNumber:
          flag = !after8D(&next_nodebox, &queries[i]);
          break;
        default:
          elog(ERROR, "unrecognized strategy: %d", strategy);
      }

      /* If any check is failed, we have found our answer. */
      if (!flag)
        break;
    }

    if (flag)
    {
      /* Pass traversalValue and quadrant */
      old_ctx = MemoryContextSwitchTo(in->traversalMemoryContext);
      out->traversalValues[out->nNodes] = cubestbox_copy(&next_nodebox);
      MemoryContextSwitchTo(old_ctx);
      out->nodeNumbers[out->nNodes] = quadrant;
      /* Pass distances */
      if (in->norderbys > 0)
      {
        double *distances = palloc(sizeof(double) * in->norderbys);
        out->distances[out->nNodes] = distances;
        for (int j = 0; j < in->norderbys; j++)
          distances[j] = distance_stbox_nodebox(&orderbys[j], &next_nodebox);
      }
      out->nNodes++;
    }
  }

  if (in->nkeys > 0)
    pfree(queries);
  if (in->norderbys > 0)
    pfree(orderbys);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST leaf-level consistency function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_spgist_leaf_consistent);
/**
 * SP-GiST leaf-level consistency function for temporal points
 */
PGDLLEXPORT Datum
Stbox_spgist_leaf_consistent(PG_FUNCTION_ARGS)
{
  spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
  spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
  STBOX *key = DatumGetSTboxP(in->leafDatum), box;
  bool result = true;
  int i;

  /* Initialize the value to do not recheck, will be updated below */
  out->recheck = false;

  /* leafDatum is what it is... */
  out->leafValue = in->leafDatum;

  /* Perform the required comparison(s) */
  for (i = 0; i < in->nkeys; i++)
  {
    StrategyNumber strategy = in->scankeys[i].sk_strategy;
    /* Update the recheck flag according to the strategy */
    out->recheck |= tpoint_index_recheck(strategy);

    if (tpoint_spgist_get_stbox(&in->scankeys[i], &box))
      result = stbox_index_consistent_leaf(key, &box, strategy);
    else
      result = false;
    /* If any check is failed, we have found our answer. */
    if (! result)
      break;
  }

  if (result && in->norderbys > 0)
  {
    /* Recheck is necessary when computing distance with bounding boxes */
    out->recheckDistances = true;
    double *distances = palloc(sizeof(double) * in->norderbys);
    out->distances = distances;
    for (i = 0; i < in->norderbys; i++)
    {
      /* Cast the order by argument to a box and perform the test */
      if (tpoint_spgist_get_stbox(&in->orderbys[i], &box))
        distances[i] = nad_stbox_stbox(&box, key);
      else
        /* If empty geometry */
        distances[i] = DBL_MAX;
    }
  }

  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * SP-GiST compress functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_spgist_compress);
/**
 * SP-GiST compress functions for temporal points
 */
PGDLLEXPORT Datum
Tpoint_spgist_compress(PG_FUNCTION_ARGS)
{
  Datum tempdatum = PG_GETARG_DATUM(0);
  STBOX *result = palloc(sizeof(STBOX));
  temporal_bbox_slice(tempdatum, result);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************/
