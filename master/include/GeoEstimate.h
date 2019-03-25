/*****************************************************************************
 *
 * GeoEstimate.h
 * 		Selectivity functions for the temporal point types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Anas Al Bassit
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __GEO_ESTIMATE_H__
#define __GEO_ESTIMATE_H__

#include <postgres.h>
#include <datatype/timestamp.h>

#define XDIM_VALUES		0x00001
#define YDIM_VALUES	    0x00010
#define ZDIM_VALUES	    0x00100
#define MDIM_VALUES	    0x01000
#define ALLDIM_VALUES	0x10000

/**
* The maximum number of dimensions our code can handle.
* We'll use this to statically allocate a bunch of
* arrays below.
*/
#define ND_DIMS 4
#define STATISTIC_SLOT_XD 0
#define STATISTIC_SLOT_YD 1
#define STATISTIC_SLOT_ZD 3


extern int gbox_ndims(const GBOX* gbox);
extern GBOX get_gbox(Node *node);
extern Selectivity geometry_sel(PlannerInfo *root, VariableStatData *vardata, GBOX *box, Oid op, int flags);
extern Selectivity estimate_selectivity_temporal_dimension(PlannerInfo *root, VariableStatData vardata, Node *other,
                                                           Oid operator);
extern Selectivity calc_hist_selectivty_geometry(PlannerInfo *root, VariableStatData *vardata, TypeCacheEntry *typcache,
                                                 RangeType * constRange, Oid op, StatisticsStrategy strategy);

#endif