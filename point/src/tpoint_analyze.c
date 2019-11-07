/*****************************************************************************
 *
 * tpoint_analyze.c
 *	  Functions for gathering statistics from temporal point columns
 *
 * Various kind of statistics are collected for both the value and the time
 * dimensions of temporal types. The kind of statistics depends on the duration
 * of the temporal type, which is defined in the table schema by the typmod
 * attribute. Please refer to the PostgreSQL file pg_statistic_d.h and the
 * PostGIS file gserialized_estimate.c for more information about the 
 * statistics collected.
 * 
 * For the spatial dimension, the statistics collected are the same for all 
 * durations. These statistics are obtained by calling the PostGIS function
 * gserialized_analyze_nd.
 * - Slot 1
 * 		- stakind contains the type of statistics which is STATISTIC_SLOT_2D.
 * 		- stanumbers stores the 2D histrogram of occurrence of features.
 * - Slot 2
 * 		- stakind contains the type of statistics which is STATISTIC_SLOT_ND.
 * 		- stanumbers stores the ND histrogram of occurrence of features.
 * For the time dimension, the statistics collected in Slots 3 and 4 depend on 
 * the duration. Please refer to file temporal_analyze.c for more information.
 * 
 * Portions Copyright (c) 2019, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint_analyze.h"

#include <assert.h>
#include <access/htup_details.h>
#include <executor/spi.h>
#include <float.h>

#include "period.h"
#include "time_analyze.h"
#include "temporal.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "temporal_analyze.h"
#include "postgis.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"

/*****************************************************************************
 * Functions copied from PostGIS file gserialized_estimate.c
 *****************************************************************************/

/*
* Assign a number to the n-dimensional statistics kind
*
* tgl suggested:
*
* 1-100:	reserved for assignment by the core Postgres project
* 100-199: reserved for assignment by PostGIS
* 200-9999: reserved for other globally-known stats kinds
* 10000-32767: reserved for private site-local use
*/
#define STATISTIC_KIND_ND 102
#define STATISTIC_KIND_2D 103
#define STATISTIC_SLOT_ND 0
#define STATISTIC_SLOT_2D 1

/*
* The SD factor restricts the side of the statistics histogram
* based on the standard deviation of the extent of the data.
* SDFACTOR is the number of standard deviations from the mean
* the histogram will extend.
*/
#define SDFACTOR 3.25

/**
* The maximum number of dimensions our code can handle.
* We'll use this to statically allocate a bunch of
* arrays below.
*/
#define ND_DIMS 4

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


/* How many bins shall we use in figuring out the distribution? */
#define NUM_BINS 50

/**
* N-dimensional box type for calculations, to avoid doing
* explicit axis conversions from GBOX in all calculations
* at every step.
*/
typedef struct ND_BOX_T
{
	float4 min[ND_DIMS];
	float4 max[ND_DIMS];
} ND_BOX;

/**
* N-dimensional box index type
*/
typedef struct ND_IBOX_T
{
	int min[ND_DIMS];
	int max[ND_DIMS];
} ND_IBOX;

/**
* N-dimensional statistics structure. Well, actually
* four-dimensional, but set up to handle arbirary dimensions
* if necessary (really, we just want to get the 2,3,4-d cases
* into one shared piece of code).
*/
typedef struct ND_STATS_T
{
	/* Dimensionality of the histogram. */
	float4 ndims;

	/* Size of n-d histogram in each dimension. */
	float4 size[ND_DIMS];

	/* Lower-left (min) and upper-right (max) spatial bounds of histogram. */
	ND_BOX extent;

	/* How many rows in the table itself? */
	float4 table_features;

	/* How many rows were in the sample that built this histogram? */
	float4 sample_features;

	/* How many not-Null/Empty features were in the sample? */
	float4 not_null_features;

	/* How many features actually got sampled in the histogram? */
	float4 histogram_features;

	/* How many cells in histogram? (sizex*sizey*sizez*sizem) */
	float4 histogram_cells;

	/* How many cells did those histogram features cover? */
	/* Since we are pro-rating coverage, this number should */
	/* now always equal histogram_features */
	float4 cells_covered;

	/* Variable length # of floats for histogram */
	float4 value[1];
} ND_STATS;

/**
* Integer comparison function for qsort
*/
static int
cmp_int (const void *a, const void *b)
{
	int ia = *((const int*)a);
	int ib = *((const int*)b);

	if (ia == ib)
		return 0;
	else if (ia > ib)
		return 1;
	else
		return -1;
}

/** Zero out an ND_BOX */
static int
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
* Given double array, return sum of values.
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
static inline int
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
		int size = roundf(nd_stats->size[d]);

		/* ... find cells the box overlaps with in this dimension */
		nd_ibox->min[d] = floor(size * (nd_box->min[d] - smin) / width);
		nd_ibox->max[d] = floor(size * (nd_box->max[d] - smin) / width);

		/* Push any out-of range values into range */
		nd_ibox->min[d] = Max(nd_ibox->min[d], 0);
		nd_ibox->max[d] = Min(nd_ibox->max[d], size-1);
	}
	return true;
}

/**
* Return true if #ND_BOX a overlaps b, false otherwise.
*/
static int
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
* Returns the proportion of b2 that is covered by b1.
*/
static inline double
nd_box_ratio(const ND_BOX *b1, const ND_BOX *b2, int ndims)
{
	int d;
	bool covered = true;
	double ivol = 1.0;
	double vol2 = 1.0;
	double vol1 = 1.0;

	for (d = 0 ; d < ndims; d++)
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
		double width1 = b1->max[d] - b1->min[d];
		double width2 = b2->max[d] - b2->min[d];
		double imin, imax, iwidth;

		vol1 *= width1;
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

/** Set the values of an #ND_BOX from a #GBOX */
static void
nd_box_from_gbox(const GBOX *gbox, ND_BOX *nd_box)
{
	int d = 0;

	nd_box_init(nd_box);
	nd_box->min[d] = gbox->xmin;
	nd_box->max[d] = gbox->xmax;
	d++;
	nd_box->min[d] = gbox->ymin;
	nd_box->max[d] = gbox->ymax;
	d++;
	if (FLAGS_GET_GEODETIC(gbox->flags))
	{
		nd_box->min[d] = gbox->zmin;
		nd_box->max[d] = gbox->zmax;
		return;
	}
	if (FLAGS_GET_Z(gbox->flags))
	{
		nd_box->min[d] = gbox->zmin;
		nd_box->max[d] = gbox->zmax;
		d++;
	}
	if (FLAGS_GET_M(gbox->flags))
	{
		nd_box->min[d] = gbox->mmin;
		nd_box->max[d] = gbox->mmax;
		d++;
	}
	return;
}

/**
* The difference between the fourth and first quintile values,
* the "inter-quintile range"
*/
static int
range_quintile(int *vals, int nvals)
{
	qsort(vals, nvals, sizeof(int), cmp_int);
	return vals[4*nvals/5] - vals[nvals/5];
}

/**
* Given an n-d index array (counter), and a domain to increment it
* in (ibox) increment it by one, unless it's already at the max of
* the domain, in which case return false.
*/
static inline int
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
	int d;
	double size;
	for (d = 0; d < ND_DIMS; d++)
	{
		size = nd_box->max[d] - nd_box->min[d];
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
static int
nd_stats_value_index(const ND_STATS *stats, int *indexes)
{
	int d;
	int accum = 1, vdx = 0;

	/* Calculate the index into the 1-d values array that the (i,j,k,l) */
	/* n-d histogram coordinate implies. */
	/* index = x + y * sizex + z * sizex * sizey + m * sizex * sizey * sizez */
	for (d = 0; d < (int)(stats->ndims); d++)
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
	int d, i, k, range;
	int counts[NUM_BINS];
	double smin, smax;   /* Spatial min, spatial max */
	double swidth;	   /* Spatial width of dimension */
	int   bmin, bmax;   /* Bin min, bin max */
	const ND_BOX *ndb;

	/* For each dimension... */
	for (d = 0; d < ndims; d++)
	{
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
		for (i = 0; i < num_boxes; i++)
		{
			double minoffset, maxoffset;

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
			bmin = floor(NUM_BINS * minoffset / swidth);
			bmax = floor(NUM_BINS * maxoffset / swidth);

			/* Should only happen when maxoffset==swidth */
			bmax = bmax >= NUM_BINS ? NUM_BINS-1 : bmax;

			/* Increment the counts in all the bins this feature overlaps */
			for (k = bmin; k <= bmax; k++)
			{
				counts[k] += 1;
			}
		}

		/* How dispersed is the distribution of features across bins? */
		range = range_quintile(counts, NUM_BINS);
		distribution[d] = range;
	}

	return true;
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

static void
gserialized_compute_stats(VacAttrStats *stats, int sample_rows, int total_rows,
	double notnull_cnt, int ndims, const ND_BOX **sample_boxes, 
	ND_BOX *sum, ND_BOX *sample_extent, int *slot_idx)
{
	MemoryContext old_context;
	int d, i;						/* Counters */
	int histogram_features = 0;		/* # rows that actually got counted in the histogram */

	ND_STATS *nd_stats;				/* Our histogram */
	size_t	nd_stats_size;		   /* Size to allocate */

	double total_sample_volume = 0;	/* Area/volume coverage of the sample */
	double total_cell_count = 0;	/* # of cells in histogram affected by sample */

	ND_BOX avg;						/* Avg of extents of sample boxes */
	ND_BOX stddev;					/* StdDev of extents of sample boxes */

	int	histo_size[ND_DIMS];		/* histogram nrows, ncols, etc */
	ND_BOX histo_extent;			/* Spatial extent of the histogram */
	ND_BOX histo_extent_new;		/* Temporary variable */
	int	histo_cells_target;		 	/* Number of cells we will shoot for, given the stats target */
	int	histo_cells;				/* Number of cells in the histogram */
	int	histo_cells_new = 1;		/* Temporary variable */

	int   histo_ndims = 0;			/* Dimensionality of the histogram */
	double sample_distribution[ND_DIMS]; /* How homogeneous is distribution of sample in each axis? */
	double total_distribution;		/* Total of sample_distribution */
	int stats_kind;					/* And this is what? (2D vs ND) */

	/* Initialize boxes */
	nd_box_init(&avg);
	nd_box_init(&stddev);
	nd_box_init(&histo_extent);
	nd_box_init(&histo_extent_new);

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

	/* If there's no useful features, we can't work out stats */
	if (! notnull_cnt)
	{
		elog(NOTICE, "no non-null/empty features, unable to compute statistics");
		stats->stats_valid = false;
		return;
	}

	/*
	 * Second scan:
	 *  o compute standard deviation
	 */
	for (d = 0; d < ndims; d++)
	{
		/* Calculate average bounds values */
		avg.min[d] = sum->min[d] / notnull_cnt;
		avg.max[d] = sum->max[d] / notnull_cnt;

		/* Calculate standard deviation for this dimension bounds */
		for (i = 0; i < notnull_cnt; i++)
		{
			const ND_BOX *ndb = sample_boxes[i];
			stddev.min[d] += (ndb->min[d] - avg.min[d]) * (ndb->min[d] - avg.min[d]);
			stddev.max[d] += (ndb->max[d] - avg.max[d]) * (ndb->max[d] - avg.max[d]);
		}
		stddev.min[d] = sqrt(stddev.min[d] / notnull_cnt);
		stddev.max[d] = sqrt(stddev.max[d] / notnull_cnt);

		/* Histogram bounds for this dimension bounds is avg +/- SDFACTOR * stdev */
		histo_extent.min[d] = Max(avg.min[d] - SDFACTOR * stddev.min[d], sample_extent->min[d]);
		histo_extent.max[d] = Min(avg.max[d] + SDFACTOR * stddev.max[d], sample_extent->max[d]);
	}

	/*
	 * Third scan:
	 *   o skip hard deviants
	 *   o compute new histogram box
	 */
	nd_box_init_bounds(&histo_extent_new);
	for (i = 0; i < notnull_cnt; i++)
	{
		const ND_BOX *ndb = sample_boxes[i];
		/* Skip any hard deviants (boxes entirely outside our histo_extent */
		if (! nd_box_intersects(&histo_extent, ndb, ndims))
		{
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
	for (d = 0; d < ndims; d++)
	{
		if (sample_distribution[d] > 0)
			histo_ndims++;
	}

	if (histo_ndims == 0)
	{
		/* Special case: all our dimensions had low variability! */
		/* We just divide the cells up evenly */
		histo_cells_new = 1;
		for (d = 0; d < ndims; d++)
		{
			histo_size[d] = 1 + (int)pow((double)histo_cells_target, 1/(double)ndims);
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
		total_distribution = total_double(sample_distribution, ndims); /* First get the total */
		histo_cells_new = 1; /* For the number of cells in the final histogram */
		for (d = 0; d < ndims; d++)
		{
			if (sample_distribution[d] == 0) /* Uninteresting dimensions don't get any room */
			{
				histo_size[d] = 1;
			}
			else /* Interesting dimension */
			{
				/* How does this dims variability compare to the total? */
				float edge_ratio = (float)sample_distribution[d] / (float)total_distribution;
				/*
				 * Scale the target cells number by the # of dims and ratio,
				 * then take the appropriate root to get the estimated number of cells
				 * on this axis (eg, pow(0.5) for 2d, pow(0.333) for 3d, pow(0.25) for 4d)
				*/
				histo_size[d] = (int)pow(histo_cells_target * histo_ndims * edge_ratio, 1/(double)histo_ndims);
				/* If something goes awry, just give this dim one slot */
				if (! histo_size[d])
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
	nd_stats->table_features = total_rows;
	nd_stats->not_null_features = notnull_cnt;
	/* Copy in the histogram dimensions */
	for (d = 0; d < ndims; d++)
		nd_stats->size[d] = histo_size[d];

	/*
	 * Fourth scan:
	 *  o fill histogram values with the proportion of
	 *	features' bbox overlaps: a feature's bvol
	 *	can fully overlap (1) or partially overlap
	 *	(fraction of 1) an histogram cell.
	 *
	 * Note that we are filling each cell with the "portion of
	 * the feature's box that overlaps the cell". So, if we sum
	 * up the values in the histogram, we could get the
	 * histogram feature count.
	 *
	 */
	for (i = 0; i < notnull_cnt; i++)
	{
		const ND_BOX *nd_box;
		ND_IBOX nd_ibox;
		int at[ND_DIMS];
		int d;
		double num_cells = 0;
		double tmp_volume = 1.0;
		double min[ND_DIMS] = {0.0, 0.0, 0.0, 0.0};
		double max[ND_DIMS] = {0.0, 0.0, 0.0, 0.0};
		double cellsize[ND_DIMS] = {0.0, 0.0, 0.0, 0.0};

		nd_box = sample_boxes[i];
		if (! nd_box) continue; /* Skip Null'ed out hard deviants */

		/* Give backend a chance of interrupting us */
		vacuum_delay_point();

		/* Find the cells that overlap with this box and put them into the ND_IBOX */
		nd_box_overlap(nd_stats, nd_box, &nd_ibox);
		memset(at, 0, sizeof(int)*ND_DIMS);


		for (d = 0; d < nd_stats->ndims; d++)
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
			for (d = 0; d < nd_stats->ndims; d++)
			{
				nd_cell.min[d] = min[d] + (at[d]+0) * cellsize[d];
				nd_cell.max[d] = min[d] + (at[d]+1) * cellsize[d];
			}

			/*
			 * If a feature box is completely inside one cell the ratio will be
			 * 1.0. If a feature box is 50% in two cells, each cell will get
			 * 0.5 added on.
			 */
			ratio = nd_box_ratio(&nd_cell, nd_box, nd_stats->ndims);
			nd_stats->value[nd_stats_value_index(nd_stats, at)] += ratio;
			num_cells += ratio;
		}
		while (nd_increment(&nd_ibox, nd_stats->ndims, at));

		/* Keep track of overall number of overlaps counted */
		total_cell_count += num_cells;
		/* How many features have we added to this histogram? */
		histogram_features++;
	}

	/* Error out if we got no sample information */
	if (! histogram_features)
	{
		elog(NOTICE, " no features lie in the stats histogram, invalid stats");
		stats->stats_valid = false;
		return;
	}

	nd_stats->histogram_features = histogram_features;
	nd_stats->histogram_cells = histo_cells;
	nd_stats->cells_covered = total_cell_count;

	/* Put this histogram data into the right slot/kind */
	if (ndims == 2)
		stats_kind = STATISTIC_KIND_2D;
	else
		stats_kind = STATISTIC_KIND_ND;

	/* Write the statistics data */
	stats->stakind[*slot_idx] = stats_kind;
	stats->staop[*slot_idx] = InvalidOid;
	stats->stanumbers[*slot_idx] = (float4*) nd_stats;
	stats->numnumbers[*slot_idx] = nd_stats_size/sizeof(float4);

	(*slot_idx)++;

	return;
}

static void
tpoint_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
	int sample_rows, double total_rows)
{
	int d, i;						/* Counters */
	int notnull_cnt = 0;			/* # not null rows in the sample */
	int null_cnt = 0;				/* # null rows in the sample */
	int	slot_idx = 0;				/* slot for storing the statistics */
	double total_width = 0;			/* # of bytes used by sample */
	ND_BOX sum;						/* Sum of extents of sample boxes */
	const ND_BOX **sample_boxes;	/* ND_BOXes for each of the sample features */
	ND_BOX sample_extent;			/* Extent of the raw sample */
	int   ndims = 2;				/* Dimensionality of the sample */
	float8 *time_lengths;
	PeriodBound *time_lowers,
		   *time_uppers;

	/* Initialize sum */
	nd_box_init(&sum);

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

	time_lowers = (PeriodBound *) palloc(sizeof(PeriodBound) * sample_rows);
	time_uppers = (PeriodBound *) palloc(sizeof(PeriodBound) * sample_rows);
	time_lengths = (float8 *) palloc(sizeof(float8) * sample_rows);

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
	for (i = 0; i < sample_rows; i++)
	{
		Datum value;
		Temporal *temp;
		GSERIALIZED *traj;
		Period period;
		PeriodBound period_lower,
				period_upper;
		GBOX gbox;
		ND_BOX *nd_box;
		bool is_null;
		bool is_copy;

		value = fetchfunc(stats, i, &is_null);

		/* Skip all NULLs. */
		if (is_null)
		{
			null_cnt++;
			continue;
		}

		/* Get temporal point */
		temp = DatumGetTemporal(value);

		/* TO VERIFY */
		is_copy = VARATT_IS_EXTENDED(temp);

		/* How many bytes does this sample use? */
		total_width += VARSIZE(temp);

		/* Get trajectory and timespan from temporal point */
		traj = (GSERIALIZED *) DatumGetPointer(tpoint_values_internal(temp));
		temporal_timespan_internal(&period, temp);

		/* Remember time bounds and length for further usage in histograms */
		period_deserialize(&period, &period_lower, &period_upper);
		time_lowers[notnull_cnt] = period_lower;
		time_uppers[notnull_cnt] = period_upper;
		time_lengths[notnull_cnt] = period_duration_secs(period_upper.val, 
			period_lower.val);

		/* Read the bounds from the trajectory. */
		if (LW_FAILURE == gserialized_get_gbox_p(traj, &gbox))
		{
			/* Skip empties too. */
			continue;
		}

		/* Check bounds for validity (finite and not NaN) */
		if (! gbox_is_valid(&gbox))
		{
			continue;
		}

		/* If we're in 2D mode, zero out the higher dimensions for "safety" 
		 * Otherwise set ndims to 3 */
		if (! MOBDB_FLAGS_GET_Z(temp->flags))
			gbox.zmin = gbox.zmax = gbox.mmin = gbox.mmax = 0.0;
		else 
			ndims = 3;

		/* Convert gbox to n-d box */
		nd_box = palloc(sizeof(ND_BOX));
		nd_box_from_gbox(&gbox, nd_box);

		/* Cache n-d bounding box */
		sample_boxes[notnull_cnt] = nd_box;

		/* Initialize sample extent before merging first entry */
		if (! notnull_cnt)
			nd_box_init_bounds(&sample_extent);

		/* Add current sample to overall sample extent */
		nd_box_merge(nd_box, &sample_extent);

		/* Add bounds coordinates to sums for stddev calculation */
		for (d = 0; d < ndims; d++)
		{
			sum.min[d] += nd_box->min[d];
			sum.max[d] += nd_box->max[d];
		}

		/* Increment our "good feature" count */
		notnull_cnt++;

		/* Free up memory if our sample temporal point was copied */
		if (is_copy)
			pfree(temp);
		pfree(traj);

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
		gserialized_compute_stats(stats, sample_rows, total_rows, notnull_cnt,
			ndims, sample_boxes, &sum, &sample_extent, &slot_idx);

		/* Compute statistics for time dimension */
		period_compute_stats1(stats, notnull_cnt, &slot_idx,
			time_lowers, time_uppers, time_lengths);
	}
	else if (null_cnt > 0)
	{
		/* We found only nulls; assume the column is entirely null */
		stats->stats_valid = true;
		stats->stanullfrac = 1.0;
		stats->stawidth = 0;		/* "unknown" */
		stats->stadistinct = 0.0;	/* "unknown" */
	}

	return;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_analyze);

PGDLLEXPORT Datum
tpoint_analyze(PG_FUNCTION_ARGS)
{
	VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);
	int duration;

	/*
	 * Call the standard typanalyze function.  It may fail to find needed
	 * operators, in which case we also can't do anything, so just fail.
	 */
	if (!std_typanalyze(stats))
		PG_RETURN_BOOL(false);

	/* 
	 * Ensure duration is valid and collect extra information about the 
	 * temporal type and its base and time types.
	 */
	duration = TYPMOD_GET_DURATION(stats->attrtypmod);
	temporal_duration_all_is_valid(duration);
	if (duration != TEMPORALINST)
		temporal_extra_info(stats);

	/* Set the callback function to compute statistics. */
	stats->compute_stats = tpoint_compute_stats;
	PG_RETURN_BOOL(true);
}

/*****************************************************************************/
