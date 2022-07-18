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
 * @brief Selectivity functions for temporal point types.
 */

#ifndef __TPOINT_SELFUNCS_H__
#define __TPOINT_SELFUNCS_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_statistic.h>
#include <utils/selfuncs.h>
/* MobilityDB */
#include "general/temporal_catalog.h"
#include "point/tpoint.h"
#include "pg_point/tpoint_analyze.h"

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
* Default geometry selectivity factor
*/
#define DEFAULT_ND_SEL 0.0001
#define DEFAULT_ND_JOINSEL 0.001

/**
* More modest fallafter selectivity factor
*/
#define FALLBACK_ND_SEL 0.2
#define FALLBACK_ND_JOINSEL 0.3

/*****************************************************************************/

extern float8 tpoint_sel(PlannerInfo *root, Oid operid, List *args,
  int varRelid, TemporalFamily tempfamily);

extern float8 tpoint_joinsel(PlannerInfo *root, Oid operid, List *args,
  JoinType jointype, SpecialJoinInfo *sjinfo, int mode,
  TemporalFamily tempFamily);

/*****************************************************************************/

#endif
