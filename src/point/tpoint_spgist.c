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
 * @file tnumber_spgist.c
 * SP-GiST implementation of 8-dimensional oct-tree over temporal points
 *
 * This module provides SP-GiST implementation for boxes using an oct-tree
 * analogy in 8-dimensional space.  SP-GiST doesn't allow indexing of
 * overlapping objects.  We are making 4D objects never-overlapping in
 * 8D space.  This technique has some benefits compared to traditional
 * R-Tree which is implemented as GiST.  The performance tests reveal
 * that this technique especially beneficial with too much overlapping
 * objects, so called "spaghetti data".
 *
 * Unlike the original oct-tree, we are splitting the tree into 256
 * octants in 8D space.  It is easier to imagine it as splitting space
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
 * to represent the octant boundaries in 8D space.  They however are
 * sufficient to point out the additional boundaries of the next
 * octant.
 *
 * We are using traversal values provided by SP-GiST to calculate and
 * to store the bounds of the octants, while traversing into the tree.
 * Traversal value has all the boundaries in the 8D space, and is is
 * capable of transferring the required boundaries to the following
 * traversal values.  In conclusion, three things are necessary
 * to calculate the next traversal value:
 *
 *  1. the traversal value of the parent
 *  2. the octant of the current node
 *  3. the prefix of the current node
 *
 * If we visualize them on our simplified drawing (see the drawing above);
 * transferred boundaries of (1) would be the outer axis, relevant part
 * of (2) would be the up range_y part of the other axis, and (3) would be
 * the inner axis.
 *
 * For example, consider the case of overlapping.  When recursion
 * descends deeper and deeper down the tree, all octants in
 * the current node will be checked for overlapping.  The boundaries
 * will be re-calculated for all octants.  Overlap check answers
 * the question: can any box from this octant overlap with the given
 * box?  If yes, then this octant will be walked.  If no, then this
 * octant will be skipped.
 *
 * This method provides restrictions for minimum and maximum values of
 * every dimension of every corner of the box on every level of the tree
 * except the root.  For the root node, we are setting the boundaries
 * that we don't yet have as infinity.
 */

#include "point/tpoint_spgist.h"

#include <float.h>
#include <access/spgist.h>
#include <utils/timestamp.h>
#include <utils/builtins.h>

#if POSTGRESQL_VERSION_NUMBER >= 120000
#include <utils/float.h>
#endif

#include "general/period.h"
#include "general/timeops.h"
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "general/tempcache.h"
#include "general/tnumber_spgist.h"
#include "point/tpoint.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_distance.h"
#include "point/tpoint_gist.h"

#if POSTGRESQL_VERSION_NUMBER >= 120000
/* To avoid including "access/spgist_private.h" since it conflicts with the
 * EPSILON constant defined there and also in MobilityDB */
extern double *spg_key_orderbys_distances(Datum key, bool isLeaf, ScanKey orderbys,
  int norderbys);
#endif

/*****************************************************************************/

/**
 * Structure to represent the bounding box of a temporal point as a 6- or
 * 8-dimensional point depending on whether the temporal point is in 2D+T or 3D+T.
 */
typedef struct
{
  STBOX left;
  STBOX right;
} CubeSTbox;

/**
 * Calculate the octant
 *
 * The octant is 8 bit unsigned integer with all bits in use.
 * This function accepts 2 STBOX as input.  All 8 bits are set by comparing a
 * corner of the box. This makes 256 octants in total.
 */
static uint8
getOctant8D(const STBOX *centroid, const STBOX *inBox)
{
  uint8 octant = 0;

  if (MOBDB_FLAGS_GET_Z(centroid->flags))
  {
    if (inBox->zmin > centroid->zmin)
      octant |= 0x80;

    if (inBox->zmax > centroid->zmax)
      octant |= 0x40;
  }

  if (inBox->ymin > centroid->ymin)
    octant |= 0x20;

  if (inBox->ymax > centroid->ymax)
    octant |= 0x10;

  if (inBox->xmin > centroid->xmin)
    octant |= 0x08;

  if (inBox->xmax > centroid->xmax)
    octant |= 0x04;

  if (inBox->tmin > centroid->tmin)
    octant |= 0x02;

  if (inBox->tmax > centroid->tmax)
    octant |= 0x01;

  return octant;
}

/**
 * Initialize the traversal value
 *
 * In the beginning, we don't have any restrictions.  We have to
 * initialize the struct to cover the whole 8D space.
 */
static CubeSTbox *
initCubeSTbox(STBOX *centroid)
{
  CubeSTbox *cube_box = (CubeSTbox *) palloc0(sizeof(CubeSTbox));
  double infinity = get_float8_infinity();

  cube_box->left.xmin = cube_box->right.xmin = -infinity;
  cube_box->left.xmax = cube_box->right.xmax = infinity;

  cube_box->left.ymin = cube_box->right.ymin = -infinity;
  cube_box->left.ymax = cube_box->right.ymax = infinity;

  cube_box->left.zmin = cube_box->right.zmin = -infinity;
  cube_box->left.zmax = cube_box->right.zmax = infinity;

  cube_box->left.tmin = cube_box->right.tmin = DT_NOBEGIN;
  cube_box->left.tmax = cube_box->right.tmax = DT_NOEND;

  cube_box->left.srid = cube_box->right.srid = centroid->srid;
  cube_box->left.flags = cube_box->right.flags = centroid->flags;

  return cube_box;
}

/**
 * Calculate the next traversal value
 *
 * All centroids are bounded by CubeSTbox, but SP-GiST only keeps
 * boxes. When we are traversing the tree, we must calculate CubeSTbox,
 * using centroid and octant.
 */
static CubeSTbox *
nextCubeSTbox(const CubeSTbox *cube_box, const STBOX *centroid, uint8 octant)
{
  CubeSTbox *next_cube_box = (CubeSTbox *) palloc0(sizeof(CubeSTbox));

  memcpy(next_cube_box, cube_box, sizeof(CubeSTbox));

  if (MOBDB_FLAGS_GET_Z(centroid->flags))
  {
    if (octant & 0x80)
      next_cube_box->left.zmin = centroid->zmin;
    else
      next_cube_box->left.zmax = centroid->zmin;

    if (octant & 0x40)
      next_cube_box->right.zmin = centroid->zmax;
    else
      next_cube_box->right.zmax = centroid->zmax;
  }

  if (octant & 0x20)
    next_cube_box->left.ymin = centroid->ymin;
  else
    next_cube_box->left.ymax = centroid->ymin;

  if (octant & 0x10)
    next_cube_box->right.ymin = centroid->ymax;
  else
    next_cube_box->right.ymax = centroid->ymax;

  if (octant & 0x08)
    next_cube_box->left.xmin = centroid->xmin;
  else
    next_cube_box->left.xmax = centroid->xmin;

  if (octant & 0x04)
    next_cube_box->right.xmin = centroid->xmax;
  else
    next_cube_box->right.xmax = centroid->xmax;

  if (octant & 0x02)
    next_cube_box->left.tmin = centroid->tmin;
  else
    next_cube_box->left.tmax = centroid->tmin;

  if (octant & 0x01)
    next_cube_box->right.tmin = centroid->tmax;
  else
    next_cube_box->right.tmax = centroid->tmax;

  return next_cube_box;
}

/**
 * Can any cube from cube_box overlap with query?
 */
static bool
overlap8D(const CubeSTbox *cube_box, const STBOX *query)
{
  bool result = true;
  /* Result value is computed only for the dimensions of the query */
  if (MOBDB_FLAGS_GET_X(query->flags))
    result &= cube_box->left.xmin <= query->xmax &&
      cube_box->right.xmax >= query->xmin &&
      cube_box->left.ymin <= query->ymax &&
      cube_box->right.ymax >= query->ymin;
  if (MOBDB_FLAGS_GET_Z(query->flags))
    result &= cube_box->left.zmin <= query->zmax &&
      cube_box->right.zmax >= query->zmin;
  if (MOBDB_FLAGS_GET_T(query->flags))
    result &= cube_box->left.tmin <= query->tmax &&
      cube_box->right.tmax >= query->tmin;
  return result;
}

/**
 * Can any cube from cube_box contain query?
 */
static bool
contain8D(const CubeSTbox *cube_box, const STBOX *query)
{
  bool result = true;
  /* Result value is computed only for the dimensions of the query */
  if (MOBDB_FLAGS_GET_X(query->flags))
    result &= cube_box->right.xmax >= query->xmax &&
      cube_box->left.xmin <= query->xmin &&
      cube_box->right.ymax >= query->ymax &&
      cube_box->left.ymin <= query->ymin;
  if (MOBDB_FLAGS_GET_Z(query->flags))
    result &= cube_box->right.zmax >= query->zmax &&
      cube_box->left.zmin <= query->zmin;
  if (MOBDB_FLAGS_GET_T(query->flags))
    result &= cube_box->right.tmax >= query->tmax &&
      cube_box->left.tmin <= query->tmin;
  return result;
}

/**
 * Can any cube from cube_box be left of query?
 */
static bool
left8D(const CubeSTbox *cube_box, const STBOX *query)
{
  return (cube_box->right.xmax < query->xmin);
}

/**
 * Can any cube from cube_box does not extend the right of query?
 */
static bool
overLeft8D(const CubeSTbox *cube_box, const STBOX *query)
{
  return (cube_box->right.xmax <= query->xmax);
}

/**
 * Can any cube from cube_box be right of query?
 */
static bool
right8D(const CubeSTbox *cube_box, const STBOX *query)
{
  return (cube_box->left.xmin > query->xmax);
}

/**
 * Can any cube from cube_box does not extend the left of query?
 */
static bool
overRight8D(const CubeSTbox *cube_box, const STBOX *query)
{
  return (cube_box->left.xmin >= query->xmin);
}

/**
 * Can any cube from cube_box be below of query?
 */
static bool
below8D(const CubeSTbox *cube_box, const STBOX *query)
{
  return (cube_box->right.ymax < query->ymin);
}

/**
 * Can any cube from cube_box does not extend above query?
 */
static bool
overBelow8D(const CubeSTbox *cube_box, const STBOX *query)
{
  return (cube_box->right.ymax <= query->ymax);
}

/**
 * Can any cube from cube_box be above of query?
 */
static bool
above8D(const CubeSTbox *cube_box, const STBOX *query)
{
  return (cube_box->left.ymin > query->ymax);
}

/**
 * Can any cube from cube_box does not extend below of query?
 */
static bool
overAbove8D(const CubeSTbox *cube_box, const STBOX *query)
{
  return (cube_box->left.ymin >= query->ymin);
}

/**
 * Can any cube from cube_box be in front of query?
 */
static bool
front8D(CubeSTbox *cube_box, STBOX *query)
{
  return (cube_box->right.zmax < query->zmin);
}

/**
 * Can any cube from cube_box does not extend the back of query?
 */
static bool
overFront8D(const CubeSTbox *cube_box, const STBOX *query)
{
  return (cube_box->right.zmax <= query->zmax);
}

/**
 * Can any cube from cube_box be back to query?
 */
static bool
back8D(const CubeSTbox *cube_box, const STBOX *query)
{
  return (cube_box->left.zmin > query->zmax);
}

/**
 * Can any cube from cube_box does not extend the front of query?
 */
static bool
overBack8D(const CubeSTbox *cube_box, const STBOX *query)
{
  return (cube_box->left.zmin >= query->zmin);
}

/**
 * Can any cube from cube_box be before of query?
 */
static bool
before8D(const CubeSTbox *cube_box, const STBOX *query)
{
  return (cube_box->right.tmax < query->tmin);
}

/**
 * Can any cube from cube_box does not extend the after of query?
 */
static bool
overBefore8D(const CubeSTbox *cube_box, const STBOX *query)
{
  return (cube_box->right.tmax <= query->tmax);
}

/**
 * Can any cube from cube_box be after of query?
 */
static bool
after8D(const CubeSTbox *cube_box, const STBOX *query)
{
  return (cube_box->left.tmin > query->tmax);
}

/**
 * Can any cube from cube_box does not extend the before of query?
 */
static bool
overAfter8D(const CubeSTbox *cube_box, const STBOX *query)
{
  return (cube_box->left.tmin >= query->tmin);
}

#if POSTGRESQL_VERSION_NUMBER >= 120000
/**
 * Lower bound for the distance between query and cube_box.
 * @note The temporal dimension is only taken into account for returning
 * +infinity (which will be translated into NULL) if the boxes do not
 * intersect in time. Besides that, it is not possible to mix different
 * units in the computation. As a consequence, the filtering is not very
 * restrictive.
 */
static double
distanceBoxCubeBox(const STBOX *query, const CubeSTbox *cube_box)
{
  /* The query argument can be an empty geometry */
  if (! MOBDB_FLAGS_GET_X(query->flags))
      return DBL_MAX;
  bool hast = MOBDB_FLAGS_GET_T(query->flags) &&
     MOBDB_FLAGS_GET_T(cube_box->left.flags);
  Period p1, p2;
  Period *inter = NULL;
  /* Project the boxes to their common timespan */
  if (hast)
  {
    period_set(query->tmin, query->tmax, true, true, &p1);
    period_set(cube_box->left.tmin, cube_box->right.tmax, true, true, &p2);
    inter = intersection_period_period_internal(&p1, &p2);
    if (!inter)
      return DBL_MAX;
    pfree(inter);
  }

  double dx, dy, dz;
  if (query->xmax < cube_box->left.xmin)
    dx = cube_box->left.xmin - query->xmax;
  else if (query->xmin > cube_box->right.xmax)
    dx = query->xmin - cube_box->right.xmax;
  else
    dx = 0;

  if (query->ymax < cube_box->left.ymin)
    dy = cube_box->left.ymin - query->ymax;
  else if (query->ymin > cube_box->right.ymax)
    dy = query->ymin - cube_box->right.ymax;
  else
    dy = 0;

  bool hasz = MOBDB_FLAGS_GET_Z(cube_box->left.flags);
  if (hasz)
  {
    if (query->zmax < cube_box->left.zmin)
      dz = cube_box->left.zmin - query->zmax;
    else if (query->zmin > cube_box->right.zmax)
      dz = query->zmin - cube_box->right.zmax;
    else
      dz = 0;
  }

  return hasz ? hypot3d(dx, dy, dz) : hypot(dx, dy);
}
#endif

/**
 * Transform a query argument into an STBOX.
 */
static bool
tpoint_spgist_get_stbox(STBOX *result, ScanKeyData *scankey)
{
  if (tgeo_base_type(scankey->sk_subtype))
  {
    GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(scankey->sk_argument);
    /* The geometry can be empty */
    if (!geo_stbox(gs, result))
      return false;
  }
  else if (scankey->sk_subtype == TIMESTAMPTZOID)
  {
    TimestampTz t = DatumGetTimestampTz(scankey->sk_argument);
    timestamp_stbox(t, result);
  }
  else if (scankey->sk_subtype == type_oid(T_TIMESTAMPSET))
  {
    // TimestampSet *ts = DatumGetTimestampSet(scankey->sk_argument);
    // timestampset_stbox(ts, result);
    timestampset_stbox_slice(scankey->sk_argument, result);
  }
  else if (scankey->sk_subtype == type_oid(T_PERIOD))
  {
    Period *p = DatumGetPeriod(scankey->sk_argument);
    period_stbox(p, result);
  }
  else if (scankey->sk_subtype == type_oid(T_PERIODSET))
  {
    // PeriodSet *ps = DatumGetPeriodSet(scankey->sk_argument);
    // periodset_stbox(ps, result);
    periodset_stbox_slice(scankey->sk_argument, result);
  }
  else if (scankey->sk_subtype == type_oid(T_STBOX))
  {
    memcpy(result, DatumGetSTboxP(scankey->sk_argument), sizeof(STBOX));
  }
  else if (tspatial_type(scankey->sk_subtype))
  {
    // temporal_bbox(DatumGetTemporalP(scankey->sk_argument), result);
    temporal_bbox_slice(scankey->sk_argument, result);
  }
  else
    elog(ERROR, "Unsupported subtype for indexing: %d", scankey->sk_subtype);
  return true;
}

/*****************************************************************************
 * SP-GiST config function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(stbox_spgist_config);
/**
 * SP-GiST config function for temporal points
 */
PGDLLEXPORT Datum
stbox_spgist_config(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(stbox_spgist_choose);
/**
 * SP-GiST choose function for temporal points
 */
PGDLLEXPORT Datum
stbox_spgist_choose(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(stbox_spgist_picksplit);
/**
 * SP-GiST pick-split function for temporal points
 *
 * It splits a list of boxes into octants by choosing a central 8D
 * point as the median of the coordinates of the boxes.
 */
PGDLLEXPORT Datum
stbox_spgist_picksplit(PG_FUNCTION_ARGS)
{
  spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
  spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
  STBOX *box = DatumGetSTboxP(in->datums[0]);
  bool hasz = MOBDB_FLAGS_GET_Z(box->flags);
  STBOX *centroid = (STBOX *) palloc0(sizeof(STBOX));
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
    lowTs[i] = (double) box->tmin;
    highTs[i] = (double) box->tmax;
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
  centroid->tmin = (TimestampTz) lowTs[median];
  centroid->tmax = (TimestampTz) highTs[median];

  /* Fill the output */
  out->hasPrefix = true;
  out->prefixDatum = STboxPGetDatum(centroid);

  out->nNodes = hasz ? 256 : 128;
  out->nodeLabels = NULL;    /* We don't need node labels. */

  out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
  out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);

  /*
   * Assign ranges to corresponding nodes according to octants relative to
   * the "centroid" range
   */
  for (i = 0; i < in->nTuples; i++)
  {
    box = DatumGetSTboxP(in->datums[i]);
    uint8 octant = getOctant8D(centroid, box);
    out->leafTupleDatums[i] = STboxPGetDatum(box);
    out->mapTuplesToNodes[i] = octant;
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

PG_FUNCTION_INFO_V1(stbox_spgist_inner_consistent);
/**
 * SP-GiST inner consistent functions for temporal points
 */
PGDLLEXPORT Datum
stbox_spgist_inner_consistent(PG_FUNCTION_ARGS)
{
  spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
  spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
  int i;
  MemoryContext old_ctx;
  CubeSTbox *cube_box;
  uint16 octant;
  STBOX *queries, *centroid = DatumGetSTboxP(in->prefixDatum);

  /*
   * We are saving the traversal value or initialize it an unbounded one, if
   * we have just begun to walk the tree.
   */
  if (in->traversalValue)
    cube_box = in->traversalValue;
  else
    cube_box = initCubeSTbox(centroid);

#if POSTGRESQL_VERSION_NUMBER >= 120000
  /*
   * Transform the orderbys into bounding boxes initializing the dimensions
   * that must not be taken into account for the operators to infinity.
   * This transformation is done here to avoid doing it for all octants
   * in the loop below.
   */
  STBOX *orderbys = (STBOX *) palloc0(sizeof(STBOX) * in->norderbys);
  for (i = 0; i < in->norderbys; i++)
    tpoint_spgist_get_stbox(&orderbys[i], &in->orderbys[i]);
#endif

  if (in->allTheSame)
  {
    /* Report that all nodes should be visited */
    out->nNodes = in->nNodes;
    out->nodeNumbers = (int *) palloc(sizeof(int) * in->nNodes);
    for (i = 0; i < in->nNodes; i++)
      out->nodeNumbers[i] = i;

#if POSTGRESQL_VERSION_NUMBER >= 120000
    if (in->norderbys > 0 && in->nNodes > 0)
    {
      double *distances = palloc0(sizeof(double) * in->norderbys);
      for (i = 0; i < in->norderbys; i++)
        distances[i] = distanceBoxCubeBox(&orderbys[i], cube_box);
      out->distances = (double **) palloc(sizeof(double *) * in->nNodes);
      out->distances[0] = distances;
      for (i = 1; i < in->nNodes; i++)
      {
        out->distances[i] = palloc(sizeof(double) * in->norderbys);
        memcpy(out->distances[i], distances, sizeof(double) * in->norderbys);
      }
    }
    pfree(orderbys);
#endif

    PG_RETURN_VOID();
  }

  /*
   * Transform the queries into bounding boxes initializing the dimensions
   * that must not be taken into account for the operators to infinity.
   * This transformation is done here to avoid doing it for all octants
   * in the loop below.
   */
  queries = (STBOX *) palloc0(sizeof(STBOX) * in->nkeys);
  for (i = 0; i < in->nkeys; i++)
    tpoint_spgist_get_stbox(&queries[i], &in->scankeys[i]);

  /* Allocate enough memory for nodes */
  out->nNodes = 0;
  out->nodeNumbers = (int *) palloc(sizeof(int) * in->nNodes);
  out->traversalValues = (void **) palloc(sizeof(void *) * in->nNodes);
#if POSTGRESQL_VERSION_NUMBER >= 120000
  if (in->norderbys > 0)
    out->distances = (double **) palloc(sizeof(double *) * in->nNodes);
#endif
  /*
   * We switch memory context, because we want to allocate memory for new
   * traversal values (next_cube_box) and pass these pieces of memory to
   * further calls of this function.
   */
  old_ctx = MemoryContextSwitchTo(in->traversalMemoryContext);

  for (octant = 0; octant < in->nNodes; octant++)
  {
    CubeSTbox *next_cube_box = nextCubeSTbox(cube_box, centroid, (uint8) octant);
    bool flag = true;
    for (i = 0; i < in->nkeys; i++)
    {
      StrategyNumber strategy = in->scankeys[i].sk_strategy;
      switch (strategy)
      {
        case RTOverlapStrategyNumber:
        case RTContainedByStrategyNumber:
        case RTAdjacentStrategyNumber:
          flag = overlap8D(next_cube_box, &queries[i]);
          break;
        case RTContainsStrategyNumber:
        case RTSameStrategyNumber:
          flag = contain8D(next_cube_box, &queries[i]);
          break;
        case RTLeftStrategyNumber:
          flag = !overRight8D(next_cube_box, &queries[i]);
          break;
        case RTOverLeftStrategyNumber:
          flag = !right8D(next_cube_box, &queries[i]);
          break;
        case RTRightStrategyNumber:
          flag = !overLeft8D(next_cube_box, &queries[i]);
          break;
        case RTOverRightStrategyNumber:
          flag = !left8D(next_cube_box, &queries[i]);
          break;
        case RTFrontStrategyNumber:
          flag = !overBack8D(next_cube_box, &queries[i]);
          break;
        case RTOverFrontStrategyNumber:
          flag = !back8D(next_cube_box, &queries[i]);
          break;
        case RTBackStrategyNumber:
          flag = !overFront8D(next_cube_box, &queries[i]);
          break;
        case RTOverBackStrategyNumber:
          flag = !front8D(next_cube_box, &queries[i]);
          break;
        case RTAboveStrategyNumber:
          flag = !overBelow8D(next_cube_box, &queries[i]);
          break;
        case RTOverAboveStrategyNumber:
          flag = !below8D(next_cube_box, &queries[i]);
          break;
        case RTBelowStrategyNumber:
          flag = !overAbove8D(next_cube_box, &queries[i]);
          break;
        case RTOverBelowStrategyNumber:
          flag = !above8D(next_cube_box, &queries[i]);
          break;
        case RTAfterStrategyNumber:
          flag = !overBefore8D(next_cube_box, &queries[i]);
          break;
        case RTOverAfterStrategyNumber:
          flag = !before8D(next_cube_box, &queries[i]);
          break;
        case RTBeforeStrategyNumber:
          flag = !overAfter8D(next_cube_box, &queries[i]);
          break;
        case RTOverBeforeStrategyNumber:
          flag = !after8D(next_cube_box, &queries[i]);
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
      out->traversalValues[out->nNodes] = next_cube_box;
      out->nodeNumbers[out->nNodes] = octant;
#if POSTGRESQL_VERSION_NUMBER >= 120000
      if (in->norderbys > 0)
      {
        double *distances = palloc(sizeof(double) * in->norderbys);
        out->distances[out->nNodes] = distances;
        for (int j = 0; j < in->norderbys; j++)
          distances[j] = distanceBoxCubeBox(&orderbys[j], next_cube_box);
      }
#endif
      out->nNodes++;
    }
    else
    {
      /*
       * If this node is not selected, we don't need to keep the next
       * traversal value in the memory context.
       */
      pfree(next_cube_box);
    }
  }

  /* Switch after */
  MemoryContextSwitchTo(old_ctx);

  pfree(queries);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST leaf-level consistency function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(stbox_spgist_leaf_consistent);
/**
 * SP-GiST leaf-level consistency function for temporal points
 */
PGDLLEXPORT Datum
stbox_spgist_leaf_consistent(PG_FUNCTION_ARGS)
{
  spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
  spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
#if POSTGRESQL_VERSION_NUMBER >= 120000
  Datum leaf = in->leafDatum;
#endif
  STBOX *key = DatumGetSTboxP(in->leafDatum);
  bool res = true;

  /* Initialize the value to do not recheck, will be updated below */
  out->recheck = false;

  /* leafDatum is what it is... */
  out->leafValue = in->leafDatum;

  /* Perform the required comparison(s) */
  for (int i = 0; i < in->nkeys; i++)
  {
    StrategyNumber strategy = in->scankeys[i].sk_strategy;
    STBOX query;

    /* Update the recheck flag according to the strategy */
    out->recheck |= tpoint_index_recheck(strategy);

    if (tpoint_spgist_get_stbox(&query, &in->scankeys[i]))
      res = stbox_index_consistent_leaf(key, &query, strategy);
    else
      res = false;
    /* If any check is failed, we have found our answer. */
    if (!res)
      break;
  }

#if POSTGRESQL_VERSION_NUMBER >= 120000
  if (res && in->norderbys > 0)
  {
    out->distances = spg_key_orderbys_distances(leaf, true, in->orderbys,
      in->norderbys);
    /* Recheck is necessary when computing distance with bounding boxes */
    out->recheckDistances = true;
  }
#endif

  PG_RETURN_BOOL(res);
}

/*****************************************************************************
 * SP-GiST compress functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_spgist_compress);
/**
 * SP-GiST compress functions for temporal points
 */
PGDLLEXPORT Datum
tpoint_spgist_compress(PG_FUNCTION_ARGS)
{
  Datum tempdatum = PG_GETARG_DATUM(0);
  STBOX *result = palloc(sizeof(STBOX));
  temporal_bbox_slice(tempdatum, result);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************/
