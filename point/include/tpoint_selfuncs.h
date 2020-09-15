/*****************************************************************************
 *
 * tpoint_selfuncs.h
 *     Selectivity functions for the temporal point types
 * Most definitions come from PostGIS file gserialized_estimate.c
 * 
 * Portions Copyright (c) 2020, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/
#ifndef __TPOINT_SELFUNCS_H__
#define __TPOINT_SELFUNCS_H__

#include <postgres.h>
#include <catalog/pg_statistic.h>
#include <utils/selfuncs.h>

#include "oidcache.h"
#include "tpoint.h"
#include "tpoint_analyze.h"

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

/*****************************************************************************/

extern Datum tpoint_sel(PG_FUNCTION_ARGS);
extern Datum tpoint_joinsel(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif 