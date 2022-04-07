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
 * @file geography_funcs.h
 * Spatial functions for PostGIS geography.
 */

#ifndef __GEOGRAPHY_FUNCTIONS_H__
#define __GEOGRAPHY_FUNCTIONS_H__

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* PostGIS */
#include <liblwgeom.h>
#if POSTGIS_VERSION_NUMBER < 30000
#include "postgis.h"
#else
#include <lwgeodetic_tree.h>
#endif

/*****************************************************************************/

#if POSTGIS_VERSION_NUMBER < 30000
extern double circ_tree_distance_tree_internal(const CIRC_NODE* n1,
  const CIRC_NODE* n2, double threshold, double* min_dist, double* max_dist,
  GEOGRAPHIC_POINT* closest1, GEOGRAPHIC_POINT* closest2);
#endif

extern Datum geography_closestpoint(PG_FUNCTION_ARGS);
extern Datum geography_shortestline(PG_FUNCTION_ARGS);
extern Datum geography_line_substring(PG_FUNCTION_ARGS);
extern Datum geography_line_interpolate_point(PG_FUNCTION_ARGS);
extern Datum geography_line_locate_point(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
