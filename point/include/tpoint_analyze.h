/*****************************************************************************
 *
 * tpoint_analyze.h
 *	  Functions for gathering statistics from temporal point columns
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TPOINT_ANALYZE_H__
#define __TPOINT_ANALYZE_H__

#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>
#include <commands/vacuum.h>
#include <liblwgeom.h>

/*****************************************************************************/

/**
* The maximum number of dimensions our code can handle.
* We'll use this to statically allocate a bunch of
* arrays below.
*/
#define ND_DIMS 4

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

extern int nd_box_init(ND_BOX *a);
extern int nd_box_init_bounds(ND_BOX *a, int ndims);
extern int nd_box_merge(const ND_BOX *source, ND_BOX *target, int ndims);
extern void nd_box_from_gbox(const GBOX *gbox, ND_BOX *nd_box);

extern void gserialized_compute_stats(VacAttrStats *stats, int sample_rows, 
    int total_rows, double notnull_cnt, const ND_BOX **sample_boxes, 
    ND_BOX *sum, ND_BOX *sample_extent, int *slot_idx, int ndims);

extern Datum tpoint_analyze(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
