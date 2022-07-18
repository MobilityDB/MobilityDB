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
 * @brief Functions for gathering statistics from temporal point columns.
 *
 * Various kind of statistics are collected for both the value and the time
 * dimensions of temporal types. The kind of statistics depends on the subtype
 * of the temporal type, which is defined in the table schema by the `typmod`
 * attribute. Please refer to the PostgreSQL file `pg_statistic_d.h` and the
 * PostGIS file `gserialized_estimate.c` for more information about the
 * statistics collected.
 *
 * For the spatial dimension, the statistics collected are the same for all
 * subtypes. These statistics are obtained by calling the PostGIS function
 * `gserialized_analyze_nd`.
 * - Slot 1
 *     - `stakind` contains the type of statistics which is `STATISTIC_SLOT_2D`.
 *     - `stanumbers` stores the 2D histogram of occurrence of features.
 * - Slot 2
 *     - `stakind` contains the type of statistics which is `STATISTIC_SLOT_ND`.
 *     - `stanumbers` stores the ND histogram of occurrence of features.
 *
 * For the time dimension, the statistics collected in Slots 3 and 4 depend on
 * the subtype. Please refer to file temporal_analyze.c for more information.
 */

#include "pg_point/tpoint_analyze.h"

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <access/htup_details.h>
#include <executor/spi.h>
#include <utils/lsyscache.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "pg_general/span_analyze.h"
#include "pg_general/temporal_analyze.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Functions copied from PostGIS file gserialized_estimate.c
 *****************************************************************************/

/*
* 1-100: reserved for assignment by the core Postgres project
* 100-199: reserved for assignment by PostGIS
* 200-9999: reserved for other globally-known stats kinds
* 10000-32767: reserved for private site-local use
*/

/**
 * Assign a number to the n-dimensional statistics kind
 */
#define STATISTIC_KIND_ND 102
#define STATISTIC_KIND_2D 103
#define STATISTIC_SLOT_ND 0
#define STATISTIC_SLOT_2D 1

/**
 * The SD factor restricts the side of the statistics histogram
 * based on the standard deviation of the extent of the data.
 * SDFACTOR is the number of standard deviations from the mean
 * the histogram will extend.
 */
#define SDFACTOR 3.25

/**
 * Minimum width of a dimension that we'll bother trying to
 * compute statistics on. Bearing in mind we have no control
 * over units, but noting that for geographics, 10E-5 is in the
 * range of meters, we go lower than that.
 */
#define MIN_DIMENSION_WIDTH 0.000000001

/**
 * Maximum width of a dimension that we'll bother trying to
 * compute statistics on.
 */
#define MAX_DIMENSION_WIDTH 1.0E+20

/** How many bins shall we use in figuring out the distribution? */
#define NUM_BINS 50

/**
 * Integer comparison function for qsort
 */
static int
cmp_int(const void *a, const void *b)
{
  int ia = *((const int *)a);
  int ib = *((const int *)b);

  if (ia == ib)
    return 0;
  else if (ia > ib)
    return 1;
  else
    return -1;
}

/** Zero out an ND_BOX */
int
nd_box_init(ND_BOX *a)
{
  memset(a, 0, sizeof(ND_BOX));
  return true;
}

/**
 * Prepare an ND_BOX for bounds calculation:
 * set the maxes to the smallest thing possible and
 * the mins to the largest.
 */
static int
nd_box_init_bounds(ND_BOX *a)
{
  int d;
  for (d = 0; d < ND_DIMS; d++)
  {
    a->min[d] = FLT_MAX;
    a->max[d] = -1 * FLT_MAX;
  }
  return true;
}

/**
 * Return the sum of values of the double array
 */
static double
total_double(const double *vals, int nvals)
{
  int i;
  float total = 0;
  /* Calculate total */
  for (i = 0; i < nvals; i++)
    total += vals[i];

  return total;
}

/** Expand the bounds of target to include source */
static int
nd_box_merge(const ND_BOX *source, ND_BOX *target)
{
  int d;
  for (d = 0; d < ND_DIMS; d++)
  {
    target->min[d] = Min(target->min[d], source->min[d]);
    target->max[d] = Max(target->max[d], source->max[d]);
  }
  return true;
}

/**
 * What stats cells overlap with this ND_BOX? Put the lowest cell
 * addresses in ND_IBOX->min and the highest in ND_IBOX->max
 */
int
nd_box_overlap(const ND_STATS *nd_stats, const ND_BOX *nd_box, ND_IBOX *nd_ibox)
{
  int d;

  /* Initialize ibox */
  memset(nd_ibox, 0, sizeof(ND_IBOX));

  /* In each dimension... */
  for (d = 0; d < nd_stats->ndims; d++)
  {
    double smin = nd_stats->extent.min[d];
    double smax = nd_stats->extent.max[d];
    double width = smax - smin;

    if (width > 0)
    {
      /* ... find cells the box overlaps with in this dimension */
      int size = (int) roundf(nd_stats->size[d]);
      nd_ibox->min[d] = (int) floor(size * (nd_box->min[d] - smin) / width);
      nd_ibox->max[d] = (int) floor(size * (nd_box->max[d] - smin) / width);

      /* Push any out-of range values into range */
      nd_ibox->min[d] = Max(nd_ibox->min[d], 0);
      nd_ibox->max[d] = Min(nd_ibox->max[d], size - 1);
    }
  }
  return true;
}

/**
 * Return true if #ND_BOX a overlaps b, false otherwise.
 */
int
nd_box_intersects(const ND_BOX *a, const ND_BOX *b, int ndims)
{
  int d;
  for (d = 0; d < ndims; d++)
  {
    if ((a->min[d] > b->max[d]) || (a->max[d] < b->min[d]))
      return false;
  }
  return true;
}

/**
 * Return the proportion of b2 that is covered by b1
 */
double
nd_box_ratio_overlaps(const ND_BOX *b1, const ND_BOX *b2, int ndims)
{
  int d;
  bool covered = true;
  double ivol = 1.0;
  double vol2 = 1.0;

  for (d = 0; d < ndims; d++)
  {
    if (b1->max[d] <= b2->min[d] || b1->min[d] >= b2->max[d])
      return 0.0; /* Disjoint */

    if (b1->min[d] > b2->min[d] || b1->max[d] < b2->max[d])
      covered = false;
  }

  if (covered)
    return 1.0;

  for (d = 0; d < ndims; d++)
  {
    double width2 = b2->max[d] - b2->min[d];
    double imin, imax, iwidth;

    vol2 *= width2;

    imin = Max(b1->min[d], b2->min[d]);
    imax = Min(b1->max[d], b2->max[d]);
    iwidth = imax - imin;
    iwidth = Max(0.0, iwidth);

    ivol *= iwidth;
  }

  if (vol2 == 0.0)
    return vol2;

  return ivol / vol2;
}

/** Set the values of an #ND_BOX from a GBOX */
static void
nd_box_from_gbox(const GBOX *gbox, ND_BOX *nd_box)
{
  int d = 0;

  nd_box_init(nd_box);
  nd_box->min[d] = (float4) gbox->xmin;
  nd_box->max[d] = (float4) gbox->xmax;
  d++;
  nd_box->min[d] = (float4) gbox->ymin;
  nd_box->max[d] = (float4) gbox->ymax;
  d++;
  if (FLAGS_GET_GEODETIC(gbox->flags))
  {
    nd_box->min[d] = (float4) gbox->zmin;
    nd_box->max[d] = (float4) gbox->zmax;
    return;
  }
  if (FLAGS_GET_Z(gbox->flags))
  {
    nd_box->min[d] = (float4) gbox->zmin;
    nd_box->max[d] = (float4) gbox->zmax;
    d++;
  }
  if (FLAGS_GET_M(gbox->flags))
  {
    nd_box->min[d] = (float4) gbox->mmin;
    nd_box->max[d] = (float4) gbox->mmax;
  }
}

/**
 * The difference between the fourth and first quintile values,
 * the "inter-quintile range"
 */
static int
range_quintile(int *vals, int nvals)
{
  qsort(vals, (size_t) nvals, sizeof(int), cmp_int);
  return vals[4*nvals/5] - vals[nvals/5];
}

/**
 * Given an n-d index array (counter), and a domain to increment it
 * in (ibox) increment it by one, unless it's already at the max of
 * the domain, in which case return false.
 */
int
nd_increment(ND_IBOX *ibox, int ndims, int *counter)
{
  int d = 0;
  while (d < ndims)
  {
    if (counter[d] < ibox->max[d])
    {
      counter[d] += 1;
      break;
    }
    counter[d] = ibox->min[d];
    d++;
  }
  /* That's it, cannot increment any more! */
  if (d == ndims)
    return false;

  /* Increment complete! */
  return true;
}

/**
 * Expand an #ND_BOX ever so slightly. Expand parameter is the proportion
 * of total width to add.
 */
static int
nd_box_expand(ND_BOX *nd_box, double expansion_factor)
{
  for (int d = 0; d < ND_DIMS; d++)
  {
    double size = nd_box->max[d] - nd_box->min[d];
    if (size <= 0) continue;
    nd_box->min[d] -= size * expansion_factor / 2;
    nd_box->max[d] += size * expansion_factor / 2;
  }
  return true;
}

/**
 * Given a position in the n-d histogram (i,j,k) return the
 * position in the 1-d values array.
 */
int
nd_stats_value_index(const ND_STATS *stats, const int *indexes)
{
  int accum = 1, vdx = 0;

  /* Calculate the index into the 1-d values array that the (i,j,k,l) */
  /* n-d histogram coordinate implies. */
  /* index = x + y * sizex + z * sizex * sizey + m * sizex * sizey * sizez */
  for (int d = 0; d < (int)(stats->ndims); d++)
  {
    int size = (int)(stats->size[d]);
    if (indexes[d] < 0 || indexes[d] >= size)
    {
      return -1;
    }
    vdx += indexes[d] * accum;
    accum *= size;
  }
  return vdx;
}

/**
 * Calculate how much a set of boxes is homogenously distributed
 * or contentrated within one dimension, returning the range_quintile of
 * of the overlap counts per cell in a uniform
 * partition of the extent of the dimension.
 * A uniform distribution of counts will have a small range
 * and will require few cells in a selectivity histogram.
 * A diverse distribution of counts will have a larger range
 * and require more cells in a selectivity histogram (to
 * distinguish between areas of feature density and areas
 * of feature sparseness. This measurement should help us
 * identify cases like X/Y/Z data where there is lots of variability
 * in density in X/Y (diversely in a multi-kilometer range) and far
 * less in Z (in a few-hundred meter range).
 */
static int
nd_box_array_distribution(const ND_BOX **nd_boxes, int num_boxes, const ND_BOX *extent, int ndims, double *distribution)
{
  /* For each dimension... */
  for (int d = 0; d < ndims; d++)
  {
    int counts[NUM_BINS];
    double swidth;     /* Spatial width of dimension */
    double smin, smax;   /* Spatial min, spatial max */
    /* Initialize counts for this dimension */
    memset(counts, 0, sizeof(counts));

    smin = extent->min[d];
    smax = extent->max[d];
    swidth = smax - smin;

    /* Don't try and calculate distribution of overly narrow */
    /* or overly wide dimensions. Here we're being pretty geographical, */
    /* expecting "normal" planar or geographic coordinates. */
    /* Otherwise we have to "handle" +/- Inf bounded features and */
    /* the assumptions needed for that are as bad as this hack. */
    if (swidth < MIN_DIMENSION_WIDTH || swidth > MAX_DIMENSION_WIDTH)
    {
      distribution[d] = 0;
      continue;
    }

    /* Sum up the overlaps of each feature with the dimensional bins */
    for (int i = 0; i < num_boxes; i++)
    {
      double minoffset, maxoffset;
      int   bmin, bmax;   /* Bin min, bin max */
      const ND_BOX *ndb;

      /* Skip null entries */
      ndb = nd_boxes[i];
      if (! ndb) continue;

      /* Where does box fall relative to the working range */
      minoffset = ndb->min[d] - smin;
      maxoffset = ndb->max[d] - smin;

      /* Skip boxes that are outside our working range */
      if (minoffset < 0 || minoffset > swidth ||
         maxoffset < 0 || maxoffset > swidth)
      {
        continue;
      }

      /* What bins does this range correspond to? */
      bmin = (int) floor(NUM_BINS * minoffset / swidth);
      bmax = (int) floor(NUM_BINS * maxoffset / swidth);

      /* Should only happen when maxoffset==swidth */
      bmax = bmax >= NUM_BINS ? NUM_BINS-1 : bmax;

      /* Increment the counts in all the bins this feature overlaps */
      for (int k = bmin; k <= bmax; k++)
      {
        counts[k] += 1;
      }
    }

    /* How dispersed is the distribution of features across bins? */
    distribution[d] = range_quintile(counts, NUM_BINS);
  }

  return true;
}

/**
 * Given that geodetic boxes are X/Y/Z regardless of the
 * underlying geometry dimensionality and other boxes
 * are guided by HAS_Z/HAS_M in their dimesionality,
 * we have a little utility function to make it easy.
 */
static int
gbox_ndims(const GBOX* gbox)
{
  int dims = 2;
  if ( FLAGS_GET_GEODETIC(gbox->flags) )
    return 3;
  if ( FLAGS_GET_Z(gbox->flags) )
    dims++;
  if ( FLAGS_GET_M(gbox->flags) )
    dims++;
  return dims;
}

/**
 * The gserialized_analyze_nd sets this function as a
 * callback on the stats object when called by the ANALYZE
 * command. ANALYZE then gathers the requisite number of
 * sample rows and then calls this function.
 *
 * We could also pass stats->extra_data in from
 * gserialized_analyze_nd (things like the column type or
 * other stuff from the system catalogs) but so far we
 * don't use that capability.
 *
 * Our job is to build some statistics on the sample data
 * for use by operator estimators.
 *
 * We will populate an n-d histogram using the provided
 * sample rows. The selectivity estimators (sel and j_oinsel)
 * can then use the histogram
 */
void
gserialized_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
  int sample_rows, double total_rows, int mode)
{
  MemoryContext old_context;
  int d, i;                       /* Counters */
  int notnull_cnt = 0;            /* # not null rows in the sample */
  int null_cnt = 0;               /* # null rows in the sample */
  int histogram_features = 0;     /* # rows that actually got counted in the histogram */

  ND_STATS *nd_stats;             /* Our histogram */
  size_t  nd_stats_size;          /* Size to allocate */

  double total_width = 0;         /* # of bytes used by sample */
  double total_sample_volume = 0; /* Area/volume coverage of the sample */
  double total_cell_count = 0;    /* # of cells in histogram affected by sample */

  ND_BOX sum;                     /* Sum of extents of sample boxes */
  ND_BOX avg;                     /* Avg of extents of sample boxes */
  ND_BOX stddev;                  /* StdDev of extents of sample boxes */

  const ND_BOX **sample_boxes;    /* ND_BOXes for each of the sample features */
  ND_BOX sample_extent;           /* Extent of the raw sample */
  int  histo_size[ND_DIMS];       /* histogram nrows, ncols, etc */
  ND_BOX histo_extent;            /* Spatial extent of the histogram */
  ND_BOX histo_extent_new;        /* Temporary variable */
  int  histo_cells_target;        /* Number of cells we will shoot for, given the stats target */
  int  histo_cells;               /* Number of cells in the histogram */
  int  histo_cells_new = 1;       /* Temporary variable */

  int   ndims = 2;                /* Dimensionality of the sample */
  int   histo_ndims = 0;          /* Dimensionality of the histogram */
  double sample_distribution[ND_DIMS]; /* How homogeneous is distribution of sample in each axis? */

  int stats_slot;            /* What slot is this data going into? (2D vs ND) */
  int stats_kind;            /* And this is what? (2D vs ND) */

  /* Initialize sum and stddev */
  nd_box_init(&sum);
  nd_box_init(&stddev);
  nd_box_init(&avg);
  nd_box_init(&histo_extent);
  nd_box_init(&histo_extent_new);

  /*
   * This is where gserialized_analyze_nd
   * should put its' custom parameters.
   */
  /* void *mystats = stats->extra_data; */

  /*
   * We might need less space, but don't think
   * its worth saving...
   */
  sample_boxes = palloc(sizeof(ND_BOX *) * sample_rows);

  /*
   * First scan:
   *  o read boxes
   *  o find dimensionality of the sample
   *  o find extent of the sample
   *  o count null-infinite/not-null values
   *  o compute total_width
   *  o compute total features's box area (for avgFeatureArea)
   *  o sum features box coordinates (for standard deviation)
   */
  for ( i = 0; i < sample_rows; i++ )
  {
    Datum datum;
    Temporal *temp;
    STBOX box;
    GBOX gbox;
    ND_BOX *nd_box;
    bool is_null;
    bool is_copy;

    datum = fetchfunc(stats, i, &is_null);

    /* Skip all NULLs. */
    if ( is_null )
    {
      null_cnt++;
      continue;
    }

    /*
     * This changes wrt the original PostGIS function. We get a temporal
     * point while the original function gets a geometry.
     */
    temp = DatumGetTemporalP(datum);

    /* TO VERIFY */
    is_copy = VARATT_IS_EXTENDED(temp);

    /* Get bounding box from temporal point */
    temporal_set_bbox(temp, &box);
    stbox_set_gbox(&box, &gbox);

    /* If we're in 2D mode, zero out the higher dimensions for "safety" */
    if ( mode == 2 )
      gbox.zmin = gbox.zmax = gbox.mmin = gbox.mmax = 0.0;

    /* Check bounds for validity (finite and not NaN) */
    if ( ! gbox_is_valid(&gbox) )
    {
      continue;
    }

    /*
     * In N-D mode, set the ndims to the maximum dimensionality found
     * in the sample. Otherwise, leave at ndims == 2.
     */
    if ( mode != 2 )
      ndims = Max(gbox_ndims(&gbox), ndims);

    /* Convert gbox to n-d box */
    nd_box = palloc(sizeof(ND_BOX));
    nd_box_from_gbox(&gbox, nd_box);

    /* Cache n-d bounding box */
    sample_boxes[notnull_cnt] = nd_box;

    /* Initialize sample extent before merging first entry */
    if ( ! notnull_cnt )
      nd_box_init_bounds(&sample_extent);

    /* Add current sample to overall sample extent */
    nd_box_merge(nd_box, &sample_extent);

    /* Add bounds coordinates to sums for stddev calculation */
    for ( d = 0; d < ndims; d++ )
    {
      sum.min[d] += nd_box->min[d];
      sum.max[d] += nd_box->max[d];
    }

    /* Increment our "good feature" count */
    notnull_cnt++;

    /* Free up memory if our temporal point was copied */
    if ( is_copy )
      pfree(temp);

    /* Give backend a chance of interrupting us */
    vacuum_delay_point();
  }

  /*
   * We'll build a histogram having stats->attr->attstattarget cells
   * on each side,  within reason... we'll use ndims*10000 as the
   * maximum number of cells.
   * Also, if we're sampling a relatively small table, we'll try to ensure that
   * we have an average of 5 features for each cell so the histogram isn't
   * so sparse.
   */
  histo_cells_target = (int)pow((double)(stats->attr->attstattarget), (double)ndims);
  histo_cells_target = Min(histo_cells_target, ndims * 10000);
  histo_cells_target = Min(histo_cells_target, (int)(total_rows/5));

  /*
   * Second scan:
   *  o compute standard deviation
   */
  for ( d = 0; d < ndims; d++ )
  {
    /* Calculate average bounds values */
    avg.min[d] = sum.min[d] / notnull_cnt;
    avg.max[d] = sum.max[d] / notnull_cnt;

    /* Calculate standard deviation for this dimension bounds */
    for ( i = 0; i < notnull_cnt; i++ )
    {
      const ND_BOX *ndb = sample_boxes[i];
      stddev.min[d] += (ndb->min[d] - avg.min[d]) * (ndb->min[d] - avg.min[d]);
      stddev.max[d] += (ndb->max[d] - avg.max[d]) * (ndb->max[d] - avg.max[d]);
    }
    stddev.min[d] = sqrtf(stddev.min[d] / notnull_cnt);
    stddev.max[d] = sqrtf(stddev.max[d] / notnull_cnt);

    /* Histogram bounds for this dimension bounds is avg +/- SDFACTOR * stdev */
    histo_extent.min[d] = (float4) Max(avg.min[d] - SDFACTOR * stddev.min[d], sample_extent.min[d]);
    histo_extent.max[d] = (float4) Min(avg.max[d] + SDFACTOR * stddev.max[d], sample_extent.max[d]);
  }

  /*
   * Third scan:
   *   o skip hard deviants
   *   o compute new histogram box
   */
  nd_box_init_bounds(&histo_extent_new);
  for ( i = 0; i < notnull_cnt; i++ )
  {
    const ND_BOX *ndb = sample_boxes[i];
    /* Skip any hard deviants (boxes entirely outside our histo_extent */
    if ( ! nd_box_intersects(&histo_extent, ndb, ndims) )
    {
      pfree((void *) sample_boxes[i]);
      sample_boxes[i] = NULL;
      continue;
    }
    /* Expand our new box to fit all the other features. */
    nd_box_merge(ndb, &histo_extent_new);
  }
  /*
   * Expand the box slightly (1%) to avoid edge effects
   * with objects that are on the boundary
   */
  nd_box_expand(&histo_extent_new, 0.01);
  histo_extent = histo_extent_new;

  /*
   * How should we allocate our histogram cells to the
   * different dimensions? We can't do it by raw dimensional width,
   * because in x/y/z space, the z can have different units
   * from the x/y. Similarly for x/y/t space.
   * So, we instead calculate how much features overlap
   * each other in their dimension to figure out which
   *  dimensions have useful selectivity characteristics (more
   * variability in density) and therefor would find
   * more cells useful (to distinguish between dense places and
   * homogeneous places).
   */
  nd_box_array_distribution(sample_boxes, notnull_cnt, &histo_extent, ndims,
                sample_distribution);

  /*
   * The sample_distribution array now tells us how spread out the
   * data is in each dimension, so we use that data to allocate
   * the histogram cells we have available.
   * At this point, histo_cells_target is the approximate target number
   * of cells.
   */

  /*
   * Some dimensions have basically a uniform distribution, we want
   * to allocate no cells to those dimensions, only to dimensions
   * that have some interesting differences in data distribution.
   * Here we count up the number of interesting dimensions
   */
  for ( d = 0; d < ndims; d++ )
  {
    if ( sample_distribution[d] > 0 )
      histo_ndims++;
  }

  if ( histo_ndims == 0 )
  {
    /* Special case: all our dimensions had low variability! */
    /* We just divide the cells up evenly */
    histo_cells_new = 1;
    for ( d = 0; d < ndims; d++ )
    {
      histo_size[d] = (int)pow((double)histo_cells_target, 1/(double)ndims);
      if ( ! histo_size[d] )
        histo_size[d] = 1;
      histo_cells_new *= histo_size[d];
    }
  }
  else
  {
    /*
     * We're going to express the amount of variability in each dimension
     * as a proportion of the total variability and allocate cells in that
     * dimension relative to that proportion.
     */
    double total_distribution;       /* Total of sample_distribution */
    total_distribution = total_double(sample_distribution, ndims); /* First get the total */
    histo_cells_new = 1; /* For the number of cells in the final histogram */
    for ( d = 0; d < ndims; d++ )
    {
      if ( sample_distribution[d] == 0 ) /* Uninteresting dimensions don't get any room */
      {
        histo_size[d] = 1;
      }
      else /* Interesting dimension */
      {
        /* How does this dims variability compare to the total? */
        float edge_ratio = (float) sample_distribution[d] / (float) total_distribution;
        /*
         * Scale the target cells number by the # of dims and ratio,
         * then take the appropriate root to get the estimated number of cells
         * on this axis (eg, pow(0.5) for 2d, pow(0.333) for 3d, pow(0.25) for 4d)
        */
        histo_size[d] = (int)pow(histo_cells_target * histo_ndims * edge_ratio, 1/(double)histo_ndims);
        /* If something goes awry, just give this dim one slot */
        if ( ! histo_size[d] )
          histo_size[d] = 1;
      }
      histo_cells_new *= histo_size[d];
    }
  }

  /* Update histo_cells to the actual number of cells we need to allocate */
  histo_cells = histo_cells_new;

  /*
   * Create the histogram (ND_STATS) in the stats memory context
   */
  old_context = MemoryContextSwitchTo(stats->anl_context);
  nd_stats_size = sizeof(ND_STATS) + ((histo_cells - 1) * sizeof(float4));
  nd_stats = palloc(nd_stats_size);
  memset(nd_stats, 0, nd_stats_size); /* Initialize all values to 0 */
  MemoryContextSwitchTo(old_context);

  /* Initialize the #ND_STATS objects */
  nd_stats->ndims = ndims;
  nd_stats->extent = histo_extent;
  nd_stats->sample_features = sample_rows;
  nd_stats->table_features = (float4) total_rows;
  nd_stats->not_null_features = notnull_cnt;
  /* Copy in the histogram dimensions */
  for ( d = 0; d < ndims; d++ )
    nd_stats->size[d] = histo_size[d];

  /*
   * Fourth scan:
   *  o fill histogram values with the proportion of
   *  features' bbox overlaps: a feature's bvol
   *  can fully overlap (1) or partially overlap
   *  (fraction of 1) an histogram cell.
   *
   * Note that we are filling each cell with the "portion of
   * the feature's box that overlaps the cell". So, if we sum
   * up the values in the histogram, we could get the
   * histogram feature count.
   *
   */
  for ( i = 0; i < notnull_cnt; i++ )
  {
    const ND_BOX *nd_box;
    ND_IBOX nd_ibox;
    int at[ND_DIMS];
    double num_cells = 0;
    double tmp_volume = 1.0;
    double min[ND_DIMS] = {0.0, 0.0, 0.0, 0.0};
    double max[ND_DIMS] = {0.0, 0.0, 0.0, 0.0};
    double cellsize[ND_DIMS] = {0.0, 0.0, 0.0, 0.0};

    nd_box = sample_boxes[i];
    if ( ! nd_box ) continue; /* Skip Null'ed out hard deviants */

    /* Give backend a chance of interrupting us */
    vacuum_delay_point();

    /* Find the cells that overlap with this box and put them into the ND_IBOX */
    nd_box_overlap(nd_stats, nd_box, &nd_ibox);
    memset(at, 0, sizeof(int) * ND_DIMS);

    for ( d = 0; d < nd_stats->ndims; d++ )
    {
      /* Initialize the starting values */
      at[d] = nd_ibox.min[d];
      min[d] = nd_stats->extent.min[d];
      max[d] = nd_stats->extent.max[d];
      cellsize[d] = (max[d] - min[d])/(nd_stats->size[d]);

      /* What's the volume (area) of this feature's box? */
      tmp_volume *= (nd_box->max[d] - nd_box->min[d]);
    }

    /* Add feature volume (area) to our total */
    total_sample_volume += tmp_volume;

    /*
     * Move through all the overlaped histogram cells values and
     * add the box overlap proportion to them.
     */
    do
    {
      ND_BOX nd_cell = { {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0} };
      double ratio;
      /* Create a box for this histogram cell */
      for ( d = 0; d < nd_stats->ndims; d++ )
      {
        nd_cell.min[d] = (float4) (min[d] + (at[d]+0) * cellsize[d]);
        nd_cell.max[d] = (float4) (min[d] + (at[d]+1) * cellsize[d]);
      }

      /*
       * If a feature box is completely inside one cell the ratio will be
       * 1.0. If a feature box is 50% in two cells, each cell will get
       * 0.5 added on.
       */
      ratio = nd_box_ratio_overlaps(&nd_cell, nd_box, (int) nd_stats->ndims);
      nd_stats->value[nd_stats_value_index(nd_stats, at)] += ratio;
      num_cells += ratio;
    }
    while ( nd_increment(&nd_ibox, (int) nd_stats->ndims, at) );

    /* Keep track of overall number of overlaps counted */
    total_cell_count += num_cells;
    /* How many features have we added to this histogram? */
    histogram_features++;
  }

  /* Free memory */
  for ( i = 0; i < notnull_cnt; i++ )
    if ( sample_boxes[i] != NULL )
      pfree((void *) sample_boxes[i]);
  pfree(sample_boxes);

  /* Error out if we got no sample information */
  if ( ! histogram_features )
  {
    elog(NOTICE, " no features lie in the stats histogram, invalid stats");
    stats->stats_valid = false;
    return;
  }

  nd_stats->histogram_features = histogram_features;
  nd_stats->histogram_cells = histo_cells;
  nd_stats->cells_covered = (float4) total_cell_count;

  /* Put this histogram data into the right slot/kind */
  if ( mode == 2 )
  {
    stats_slot = STATISTIC_SLOT_2D;
    stats_kind = STATISTIC_KIND_2D;
  }
  else
  {
    stats_slot = STATISTIC_SLOT_ND;
    stats_kind = STATISTIC_KIND_ND;
  }

  /* Write the statistics data */
  stats->stakind[stats_slot] = (int16) stats_kind;
  stats->staop[stats_slot] = InvalidOid;
  stats->stanumbers[stats_slot] = (float4*)nd_stats;
  stats->numnumbers[stats_slot] = (int) (nd_stats_size/sizeof(float4));
  stats->stanullfrac = (float4)null_cnt/sample_rows;
  stats->stawidth = (int32) (total_width/notnull_cnt);
  stats->stadistinct = -1.0f;
  stats->stats_valid = true;
}

/*****************************************************************************
 * Statistics for temporal points
 *****************************************************************************/

/**
 * Compute the statistics for temporal point columns (callback function)
 */
void
tpoint_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
  int sample_rows, double total_rows)
{
  int notnull_cnt = 0;      /* # not null rows in the sample */
  int null_cnt = 0;         /* # null rows in the sample */
  int slot_idx = 2;         /* Starting slot for storing temporal statistics */
  double total_width = 0;   /* # of bytes used by sample */

  SpanBound *time_lowers = palloc(sizeof(SpanBound) * sample_rows);
  SpanBound *time_uppers = palloc(sizeof(SpanBound) * sample_rows);
  float8 *time_lengths = palloc(sizeof(float8) * sample_rows);

  /*
   * First scan for obtaining the number of nulls and not nulls, the total
   * width and the temporal extents
   */
  for (int i = 0; i < sample_rows; i++)
  {
    Datum value;
    Temporal *temp;
    Period period;
    SpanBound period_lower, period_upper;
    bool is_null, is_copy;

    value = fetchfunc(stats, i, &is_null);

    /* Skip all NULLs. */
    if (is_null)
    {
      null_cnt++;
      continue;
    }

    /* Get temporal point */
    temp = DatumGetTemporalP(value);

    /* TO VERIFY */
    is_copy = VARATT_IS_EXTENDED(temp);

    /* How many bytes does this sample use? */
    total_width += VARSIZE(temp);

    /* Get period from temporal point */
    temporal_set_period(temp, &period);

    /* Remember time bounds and length for further usage in histograms */
    span_deserialize((Span *) &period, &period_lower, &period_upper);
    time_lowers[notnull_cnt] = period_lower;
    time_uppers[notnull_cnt] = period_upper;
    time_lengths[notnull_cnt] = distance_elem_elem(period_upper.val,
      period_lower.val, T_TIMESTAMPTZ, T_TIMESTAMPTZ);

    /* Increment our "good feature" count */
    notnull_cnt++;

    /* Free up memory if our sample temporal point was copied */
    if (is_copy)
      pfree(temp);

    /* Give backend a chance of interrupting us */
    vacuum_delay_point();
  }

  /* We can only compute real stats if we found some non-null values. */
  if (notnull_cnt > 0)
  {
    stats->stats_valid = true;
    /* Do the simple null-frac and width stats */
    stats->stanullfrac = (float4) null_cnt / (float4) sample_rows;
    stats->stawidth = (int) (total_width / notnull_cnt);

    /* Estimate that non-null values are unique */
    stats->stadistinct = (float4) (-1.0 * (1.0 - stats->stanullfrac));

    /* Compute statistics for spatial dimension */
    /* 2D Mode */
    gserialized_compute_stats(stats, fetchfunc, sample_rows, total_rows, 2);
    /* ND Mode */
    gserialized_compute_stats(stats, fetchfunc, sample_rows, total_rows, 0);

    /* Compute statistics for time dimension */
    span_compute_stats(stats, notnull_cnt, &slot_idx, time_lowers, time_uppers,
      time_lengths, T_PERIOD);
  }
  else if (null_cnt > 0)
  {
    /* We found only nulls; assume the column is entirely null */
    stats->stats_valid = true;
    stats->stanullfrac = 1.0;
    stats->stawidth = 0;    /* "unknown" */
    stats->stadistinct = 0.0;  /* "unknown" */
  }

  pfree(time_lowers);
  pfree(time_uppers);
  pfree(time_lengths);
  return;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_analyze);
/**
 * Compute the statistics for temporal point columns
 */
PGDLLEXPORT Datum
Tpoint_analyze(PG_FUNCTION_ARGS)
{
  return generic_analyze(fcinfo, &tpoint_compute_stats);
}

/*****************************************************************************/
