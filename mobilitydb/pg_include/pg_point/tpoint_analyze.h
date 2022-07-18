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
 * @brief Functions for gathering statistics from temporal point columns
 */

#ifndef __TPOINT_ANALYZE_H__
#define __TPOINT_ANALYZE_H__

/* PostgreSQL */
#include <postgres.h>
#include <commands/vacuum.h>
/* PostGIS */
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

extern int nd_box_init(ND_BOX *a);
extern int nd_box_overlap(const ND_STATS *nd_stats, const ND_BOX *nd_box, ND_IBOX *nd_ibox);
extern int nd_box_intersects(const ND_BOX *a, const ND_BOX *b, int ndims);
extern double nd_box_ratio_overlaps(const ND_BOX *b1, const ND_BOX *b2, int ndims);
extern int nd_increment(ND_IBOX *ibox, int ndims, int *counter);
extern int nd_stats_value_index(const ND_STATS *stats, const int *indexes);

extern void tpoint_compute_stats(VacAttrStats *stats,
  AnalyzeAttrFetchFunc fetchfunc, int sample_rows, double total_rows);

/*****************************************************************************/

#endif
