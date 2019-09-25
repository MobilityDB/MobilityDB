/*****************************************************************************
 *
 * tpoint_selfuncs.h
 * 		Selectivity functions for the temporal point types
 * Most definitions come from PostGIS file gserialized_estimate.c
 * 
 * Portions Copyright (c) 2019, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/
#ifndef __TPOINT_SELFUNCS_H__
#define __TPOINT_SELFUNCS_H__

#include <postgres.h>
#include <catalog/pg_statistic.h>
#include <nodes/relation.h>
#include <utils/selfuncs.h>

#include "oidcache.h"
#include "tpoint.h"

/**
* The maximum number of dimensions our code can handle.
* We'll use this to statically allocate a bunch of
* arrays below.
*/
#define X_DIM  0
#define Y_DIM  1
#define Z_DIM  2
#define T_DIM  3
#define ND_DIMS 4

/*****************************************************************************
 * Definitions copied from PostGIS file gserialized_estimate.c
 *****************************************************************************/

/*
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

#define STATISTIC_KIND_ND 102
#define STATISTIC_KIND_2D 103
#define STATISTIC_SLOT_ND 0
#define STATISTIC_SLOT_2D 1

/**
* More modest fallafter selectivity factor
*/
#define FALLBACK_ND_SEL 0.2
#define FALLBACK_ND_JOINSEL 0.3

/*
 * N-dimensional box type for calculations, to avoid doing
 * explicit axis conversions from STBOX in all calculations
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

/*
 * N-dimensional statistics structure. Well, actually
 * four-dimensional, but set up to handle arbirary dimensions
 * if necessary (really, we just want to get the 2,3,4-d cases
 * into one shared piece of code).
 * 
 * Definition copied from PostGIS file gserialized_estimate.c
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

/*****************************************************************************/

extern Datum tpoint_sel(PG_FUNCTION_ARGS);
extern Datum tpoint_joinsel(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif 