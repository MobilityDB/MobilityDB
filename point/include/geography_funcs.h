/*****************************************************************************
 *
 * geography_functions.h
 *	  Spatial functions for PostGIS geography.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __GEOGRAPHY_FUNCTIONS_H__
#define __GEOGRAPHY_FUNCTIONS_H__

#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H 1

#include <postgres.h>
#include <liblwgeom.h>
#include "postgis.h"

/*****************************************************************************/

extern double circ_tree_distance_tree_internal(const CIRC_NODE* n1, const CIRC_NODE* n2, double threshold,
		double* min_dist, double* max_dist, GEOGRAPHIC_POINT* closest1, GEOGRAPHIC_POINT* closest2);

/*****************************************************************************/

#endif
