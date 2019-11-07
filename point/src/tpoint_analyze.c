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

	if (ia == ib )
		return 0;
	else if (ia > ib )
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
	for ( d = 0; d < ND_DIMS; d++ )
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
	for ( i = 0; i < nvals; i++ )
		total += vals[i];

	return total;
}

/** Expand the bounds of target to include source */
static int
nd_box_merge(const ND_BOX *source, ND_BOX *target)
{
	int d;
	for ( d = 0; d < ND_DIMS; d++ )
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
	for ( d = 0; d < nd_stats->ndims; d++ )
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
	for ( d = 0; d < ndims; d++ )
	{
		if ((a->min[d] > b->max[d]) || (a->max[d] < b->min[d]) )
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

	for ( d = 0 ; d < ndims; d++ )
	{
		if (b1->max[d] <= b2->min[d] || b1->min[d] >= b2->max[d] )
			return 0.0; /* Disjoint */

		if (b1->min[d] > b2->min[d] || b1->max[d] < b2->max[d] )
			covered = false;
	}

	if (covered )
		return 1.0;

	for ( d = 0; d < ndims; d++ )
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

	if (vol2 == 0.0 )
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
	if (FLAGS_GET_GEODETIC(gbox->flags) )
	{
		nd_box->min[d] = gbox->zmin;
		nd_box->max[d] = gbox->zmax;
		return;
	}
	if (FLAGS_GET_Z(gbox->flags) )
	{
		nd_box->min[d] = gbox->zmin;
		nd_box->max[d] = gbox->zmax;
		d++;
	}
	if (FLAGS_GET_M(gbox->flags) )
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
* Given that geodetic boxes are X/Y/Z regardless of the
* underlying geometry dimensionality and other boxes
* are guided by HAS_Z/HAS_M in their dimesionality,
* we have a little utility function to make it easy.
*/
static int
gbox_ndims(const GBOX* gbox)
{
	int dims = 2;
	if (FLAGS_GET_GEODETIC(gbox->flags) )
		return 3;
	if (FLAGS_GET_Z(gbox->flags) )
		dims++;
	if (FLAGS_GET_M(gbox->flags) )
		dims++;
	return dims;
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

	while ( d < ndims )
	{
		if (counter[d] < ibox->max[d] )
		{
			counter[d] += 1;
			break;
		}
		counter[d] = ibox->min[d];
		d++;
	}
	/* That's it, cannot increment any more! */
	if (d == ndims )
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
	for ( d = 0; d < ND_DIMS; d++ )
	{
		size = nd_box->max[d] - nd_box->min[d];
		if (size <= 0 ) continue;
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
	for ( d = 0; d < (int)(stats->ndims); d++ )
	{
		int size = (int)(stats->size[d]);
		if (indexes[d] < 0 || indexes[d] >= size )
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
	double swidth;       /* Spatial width of dimension */
	int   bmin, bmax;   /* Bin min, bin max */
	const ND_BOX *ndb;

	/* For each dimension... */
	for ( d = 0; d < ndims; d++ )
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
		if (swidth < MIN_DIMENSION_WIDTH || swidth > MAX_DIMENSION_WIDTH )
		{
			distribution[d] = 0;
			continue;
		}

		/* Sum up the overlaps of each feature with the dimensional bins */
		for ( i = 0; i < num_boxes; i++ )
		{
			double minoffset, maxoffset;

			/* Skip null entries */
			ndb = nd_boxes[i];
			if (! ndb ) continue;

			/* Where does box fall relative to the working range */
			minoffset = ndb->min[d] - smin;
			maxoffset = ndb->max[d] - smin;

			/* Skip boxes that are outside our working range */
			if (minoffset < 0 || minoffset > swidth ||
			     maxoffset < 0 || maxoffset > swidth )
			{
				continue;
			}

			/* What bins does this range correspond to? */
			bmin = floor(NUM_BINS * minoffset / swidth);
			bmax = floor(NUM_BINS * maxoffset / swidth);

			/* Should only happen when maxoffset==swidth */
			bmax = bmax >= NUM_BINS ? NUM_BINS-1 : bmax;

			/* Increment the counts in all the bins this feature overlaps */
			for ( k = bmin; k <= bmax; k++ )
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

/*****************************************************************************/

static HeapTuple
tpoint_remove_timedim(HeapTuple tuple, TupleDesc tupDesc, int tupattnum, 
	Datum value, Datum *values, bool *isnull)
{
	heap_deform_tuple(tuple, tupDesc, values, isnull);

	SPI_connect();
	Datum replValue = tpoint_values_internal(DatumGetTemporal(value));
	SPI_finish();
	/* tupattnum is 1-based */
	values[tupattnum - 1] = replValue;
	HeapTuple result = heap_form_tuple(tupDesc, values, isnull);
	pfree(DatumGetPointer(replValue));

	/*
	 * Copy the identification info of the old tuple: t_ctid, t_self, and OID
	 * (if any)
	 */
	result->t_data->t_ctid = tuple->t_data->t_ctid;
	result->t_self = tuple->t_self;
	result->t_tableOid = tuple->t_tableOid;
	if (tupDesc->tdhasoid)
		HeapTupleSetOid(result, HeapTupleGetOid(tuple));

	heap_freetuple(tuple);
	return result;
}

static void
tpoint_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
					 int samplerows, double totalrows)
{
	MemoryContext old_context;
	int duration = TYPMOD_GET_DURATION(stats->attrtypmod);
	int stawidth;

	/* Compute statistics for the time component */
	if (duration == TEMPORALINST)
		temporalinst_compute_stats(stats, fetchfunc, samplerows, totalrows);
	else
		temporals_compute_stats(stats, fetchfunc, samplerows, totalrows);

	stawidth = stats->stawidth;

	/* Must copy the target values into anl_context */
	old_context = MemoryContextSwitchTo(stats->anl_context);

	Datum *values = (Datum *) palloc(stats->tupDesc->natts * sizeof(Datum));
	bool *isnull = (bool *) palloc(stats->tupDesc->natts * sizeof(bool));

	/* Remove time component for the tuples */
	for (int i = 0; i < samplerows; i++)
	{
		bool valueisnull;
		Datum value = fetchfunc(stats, i, &valueisnull);
		if (valueisnull)
			continue;

		stats->rows[i] = tpoint_remove_timedim(stats->rows[i], 	
			stats->tupDesc, stats->tupDesc->natts, value, values, isnull);
	}

	/* Compute statistics for the geometry component */
	call_function1(gserialized_analyze_nd, PointerGetDatum(stats));
	stats->compute_stats(stats, fetchfunc, samplerows, totalrows);

	/* Put the total width of the column, variable size */
	stats->stawidth = stawidth;

	pfree(values); pfree(isnull);

	/* Switch back to the previous context */
	MemoryContextSwitchTo(old_context);

	return;
}

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
